/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteManager"
#endif

#include <mutex>
#include <string>
#include <atomic>
#include <memory>
#include <sstream>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_engine.h"
#include "audio_suite_manager_private.h"
#include "audio_suite_manager_callback.h"
#include "media_monitor_manager.h"
#include "media_monitor_info.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static const int32_t OPERATION_TIMEOUT_IN_MS = 10000;  // 10s
enum ErrorScene : uint32_t {
    PIPELINE_SCENE = 0,
    NODE_SCENE = 1,
};
enum PipelineErrorCase : uint32_t {
    CREATE_PIPELINE_ERROR = 0,
    DESTROY_PIPELINE_ERROR = 1,
    RENDER_PIPELINE_ERROR = 2,
};
enum NodeErrorCase : uint32_t {
    CREATE_NODE_ERROR = 0,
    DESTROY_NODE_ERROR = 1,
    CONNECT_NODE_ERROR = 2,
    DISCONNECT_NODE_ERROR = 3,
};
static const std::map<AudioNodeType, std::string> NODETYPE_TOSTRING_MAP = {
    {NODE_TYPE_EMPTY, "NODE_TYPE_EMPTY"},
    {NODE_TYPE_INPUT, "NODE_TYPE_INPUT"},
    {NODE_TYPE_OUTPUT, "NODE_TYPE_OUTPUT"},
    {NODE_TYPE_EQUALIZER, "NODE_TYPE_EQUALIZER"},
    {NODE_TYPE_NOISE_REDUCTION, "NODE_TYPE_NOISE_REDUCTION"},
    {NODE_TYPE_SOUND_FIELD, "NODE_TYPE_SOUND_FIELD"},
    {NODE_TYPE_AUDIO_SEPARATION, "NODE_TYPE_AUDIO_SEPARATION"},
    {NODE_TYPE_VOICE_BEAUTIFIER, "NODE_TYPE_VOICE_BEAUTIFIER"},
    {NODE_TYPE_ENVIRONMENT_EFFECT, "NODE_TYPE_ENVIRONMENT_EFFECT"},
    {NODE_TYPE_AUDIO_MIXER, "NODE_TYPE_AUDIO_MIXER"}
};
}

IAudioSuiteManager& IAudioSuiteManager::GetAudioSuiteManager()
{
    static AudioSuiteManager audioSuiteManager;
    return audioSuiteManager;
}

int32_t AudioSuiteManager::Init()
{
    AUDIO_INFO_LOG("Init enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ == nullptr, ERR_ILLEGAL_STATE, "suite engine aleay inited");

    suiteEngine_ = std::make_shared<AudioSuiteEngine>(*this);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr,
        ERR_MEMORY_ALLOC_FAILED, "Create suite engine failed, mallocl error.");

    int32_t ret = suiteEngine_->Init();
    if (ret != SUCCESS) {
        suiteEngine_ = nullptr;
        AUDIO_INFO_LOG("Aduio suite engine init failed. ret = %{public}d.", ret);
        return ret;
    }

    AUDIO_INFO_LOG("Init leave");
    return ret;
}

int32_t AudioSuiteManager::DeInit()
{
    AUDIO_INFO_LOG("DeInit enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_ILLEGAL_STATE, "suite engine not inited");
    std::vector<std::unique_lock<std::mutex>> pipelineLocks;
    for (auto& [id, lock] : pipelineLockMap_) {
        pipelineLocks.emplace_back(*lock);
    }
    int32_t ret = suiteEngine_->DeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "suite engine deinit failed, ret = %{public}d.", ret);

    suiteEngine_ = nullptr;
    AUDIO_INFO_LOG("DeInit leave");
    return ret;
}

int32_t AudioSuiteManager::CreatePipeline(uint32_t &pipelineId, PipelineWorkMode workMode)
{
    AUDIO_INFO_LOG("CreatePipeline enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    isFinishCreatePipeline_ = false;
    engineCreateResult_ = 0;
    int32_t ret = suiteEngine_->CreatePipeline(workMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine CreatePipeline failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCreatePipeline_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, CREATE_PIPELINE_ERROR, "CreatePipeline timeout");
        AUDIO_ERR_LOG("CreatePipeline timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("CreatePipeline leave");
    pipelineId = engineCreatePipelineId_;
    engineCreatePipelineId_ = INVALID_PIPELINE_ID;

    pipelineLockMap_[pipelineId] = std::make_unique<std::mutex>();
    pipelineCallbackMutexMap_[pipelineId] = std::make_unique<std::mutex>();
    pipelineCallbackCVMap_[pipelineId] = std::make_unique<std::condition_variable>();
    return engineCreateResult_;
}

int32_t AudioSuiteManager::DestroyPipeline(uint32_t pipelineId)
{
    AUDIO_INFO_LOG("DestroyPipeline enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    auto it = pipelineLockMap_.find(pipelineId);
    CHECK_AND_RETURN_RET_LOG(it != pipelineLockMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                             "pipeline lock not exist");
    auto &pipelineLock = it->second;
    CHECK_AND_RETURN_RET_LOG(pipelineLock != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                             "pipeline lock is null");
    std::lock_guard<std::mutex> pipelineLockGuard(*pipelineLock);

    isFinishDestroyPipeline_ = false;
    int32_t ret = suiteEngine_->DestroyPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine DestroyPipeline failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishDestroyPipeline_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, DESTROY_PIPELINE_ERROR, "DestroyPipeline timeout");
        AUDIO_ERR_LOG("DestroyPipeline timeout");
        return ERROR;
    }
    isFinishRenderFrameMap_.erase(pipelineId);
    renderFrameResultMap_.erase(pipelineId);
    isFinishMultiRenderFrameMap_.erase(pipelineId);
    multiRenderFrameResultMap_.erase(pipelineId);
    pipelineLockMap_.erase(pipelineId);
    pipelineCallbackMutexMap_.erase(pipelineId);
    pipelineCallbackCVMap_.erase(pipelineId);

    AUDIO_INFO_LOG("DestroyPipeline leave");
    return destroyPipelineResult_;
}

int32_t AudioSuiteManager::StartPipeline(uint32_t pipelineId)
{
    AUDIO_INFO_LOG("StartPipeline enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    isFinishStartPipeline_ = false;
    int32_t ret = suiteEngine_->StartPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine StartPipeline failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishStartPipeline_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "StartPipeline timeout");

    AUDIO_INFO_LOG("StartPipeline leave");
    return startPipelineResult_;
}

int32_t AudioSuiteManager::StopPipeline(uint32_t pipelineId)
{
    AUDIO_INFO_LOG("StopPipeline enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    isFinishStopPipeline_ = false;
    int32_t ret = suiteEngine_->StopPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine StopPipeline failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishStopPipeline_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "StopPipeline timeout");

    AUDIO_INFO_LOG("StopPipeline leave");
    return stopPipelineResult_;
}

int32_t AudioSuiteManager::GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state)
{
    AUDIO_INFO_LOG("GetPipelineState enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    isFinishGetPipelineState_ = false;
    getPipelineState_ = PIPELINE_STOPPED;
    int32_t ret = suiteEngine_->GetPipelineState(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetPipelineState failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetPipelineState_;  // will be true when got notified.
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "GetPipelineState timeout");

    AUDIO_INFO_LOG("GetPipelineState leave");
    state = getPipelineState_;
    return SUCCESS;
}

int32_t AudioSuiteManager::CreateNode(uint32_t pipelineId, AudioNodeBuilder &builder, uint32_t &nodeId)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    isFinishCreateNode_ = false;
    engineCreateNodeResult_ = 0;
    int32_t ret = suiteEngine_->CreateNode(pipelineId, builder);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, INVALID_NODE_ID, "engine CreateNode failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCreateNode_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CREATE_NODE_ERROR, "CreateNode timeout");
        AUDIO_ERR_LOG("CreateNode timeout");
        return INVALID_NODE_ID;
    }

    AUDIO_INFO_LOG("CreateNode leave");
    WriteSuiteEngineUtilizationStatsEvent(builder.nodeType);
    nodeId = engineCreateNodeId_;
    engineCreateNodeId_ = INVALID_NODE_ID;
    return engineCreateNodeResult_;
}

int32_t AudioSuiteManager::DestroyNode(uint32_t nodeId)
{
    AUDIO_INFO_LOG("DestroyNode enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    isFinishDestroyNode_ = false;
    int32_t ret = suiteEngine_->DestroyNode(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine DestroyNode failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishDestroyNode_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DESTROY_NODE_ERROR, "DestroyNode timeout");
        AUDIO_ERR_LOG("DestroyNode timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("DestroyNode leave");
    return destroyNodeResult_;
}

int32_t AudioSuiteManager::BypassEffectNode(uint32_t nodeId, bool bypass)
{
    AUDIO_INFO_LOG("BypassEffectNode enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    isFinishBypassEffectNode_ = false;
    int32_t ret = suiteEngine_->BypassEffectNode(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine BypassEffectNode failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishBypassEffectNode_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "BypassEffectNode timeout");

    AUDIO_INFO_LOG("BypassEffectNode leave");
    return bypassEffectNodeResult_;
}

int32_t AudioSuiteManager::GetNodeBypassStatus(uint32_t nodeId, bool &bypass)
{
    AUDIO_INFO_LOG("GetNodeBypassStatus enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    isFinishGetNodeBypassStatus_ = false;
    getNodeBypassResult_ = false;
    int32_t ret = suiteEngine_->GetNodeBypassStatus(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetNodeBypassStatus failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetNodeBypassStatus_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "GetNodeBypassStatus timeout");

    AUDIO_INFO_LOG("GetNodeBypassStatus leave");
    bypass = getNodeBypassResult_;
    return SUCCESS;
}

int32_t AudioSuiteManager::SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat)
{
    AUDIO_INFO_LOG("SetAudioFormat enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    isFinishSetFormat_ = false;
    int32_t ret = suiteEngine_->SetAudioFormat(nodeId, audioFormat);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetAudioFormat failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetFormat_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "SetAudioFormat timeout");

    AUDIO_INFO_LOG("SetAudioFormat leave");
    return setFormatResult_;
}

int32_t AudioSuiteManager::SetRequestDataCallback(uint32_t nodeId,
    std::shared_ptr<InputNodeRequestDataCallBack> callback)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    isFinishSetWriteData_ = false;
    int32_t ret = suiteEngine_->SetRequestDataCallback(nodeId, callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetRequestDataCallback failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetWriteData_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "SetRequestDataCallback timeout");

    AUDIO_INFO_LOG("SetRequestDataCallback leave");
    return setWriteDataResult_;
}

int32_t AudioSuiteManager::ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    AUDIO_INFO_LOG("ConnectNodes enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    isFinishConnectNodes_ = false;
    int32_t ret = suiteEngine_->ConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine ConnectNodes failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishConnectNodes_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CONNECT_NODE_ERROR, "ConnectNodes timeout");
        AUDIO_ERR_LOG("ConnectNodes timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("ConnectNodes leave");
    return connectNodesResult_;
}

int32_t AudioSuiteManager::DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    AUDIO_INFO_LOG("DisConnectNodes enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    isFinishDisConnectNodes_ = false;
    int32_t ret = suiteEngine_->DisConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine DisConnectNodes failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishDisConnectNodes_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DISCONNECT_NODE_ERROR, "DisConnectNodes timeout");
        AUDIO_ERR_LOG("DisConnectNodes timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("DisConnectNodes leave");
    return disConnectNodesResult_;
}

int32_t AudioSuiteManager::SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains)
{
    AUDIO_INFO_LOG("SetEqualizerFrequencyBandGains enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    // check
    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "";
    for (size_t idx = 0; idx < sizeof(gains.gains) / sizeof(gains.gains[0]); idx++) {
        value += std::to_string(gains.gains[idx]);
        value += ":";
    }
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "engine SetEqualizerFrequencyBandGains failed, ret = %{public}d", ret);
    return ret;
}

int32_t AudioSuiteManager::SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType)
{
    AUDIO_INFO_LOG("SetSoundFieldType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    // check
    std::string name = "SoundFieldType";
    std::string value = std::to_string(static_cast<int32_t>(soundFieldType));
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetSoundFieldType failed, ret = %{public}d", ret);
    return ret;
}

int32_t AudioSuiteManager::SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType)
{
    AUDIO_INFO_LOG("EnvironmentType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    // check
    std::string name = "EnvironmentType";
    std::string value = std::to_string(static_cast<int32_t>(environmentType));
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine EnvironmentType failed, ret = %{public}d", ret);
    return ret;
}

int32_t AudioSuiteManager::SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType)
{
    AUDIO_INFO_LOG("SetVoiceBeautifierType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    // check
    std::string name = "VoiceBeautifierType";
    std::string value = std::to_string(static_cast<int32_t>(voiceBeautifierType));
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetVoiceBeautifierType failed, ret = %{public}d", ret);
    return ret;
}

int32_t AudioSuiteManager::GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType)
{
    AUDIO_INFO_LOG("GetEnvironmentType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    // check
    isFinishGetOptions_ = false;
    std::string name = "EnvironmentType";
    std::string value = "";
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine EnvironmentType failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    if (!stopWaiting || getOptionsResult_ != SUCCESS) {
        AUDIO_ERR_LOG("GetEnvironmentType Error!");
        return ERROR;
    }
    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(EnvironmentType::AUDIO_SUITE_ENVIRONMENT_TYPE_CLOSE)
        || parseValue > static_cast<int32_t>(EnvironmentType::AUDIO_SUITE_ENVIRONMENT_TYPE_GRAMOPHONE)) {
        return ERROR;
    }
    environmentType = static_cast<EnvironmentType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::GetSoundFiledType(uint32_t nodeId, SoundFieldType &soundFieldType)
{
    AUDIO_INFO_LOG("GetSoundFiledType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    // check
    isFinishGetOptions_ = false;
    std::string name = "SoundFieldType";
    std::string value = "";
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetSoundFiledType failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    if (!stopWaiting || getOptionsResult_ != SUCCESS) {
        AUDIO_ERR_LOG("GetSoundFiledType Error!");
        return ERROR;
    }
    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(SoundFieldType::AUDIO_SUITE_SOUND_FIELD_CLOSE)
        || parseValue > static_cast<int32_t>(SoundFieldType::AUDIO_SUITE_SOUND_FIELD_WIDE)) {
        return ERROR;
    }
    soundFieldType = static_cast<SoundFieldType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::GetEqualizerFrequencyBandGains(uint32_t nodeId,
    AudioEqualizerFrequencyBandGains &frequencyBandGains)
{
    AUDIO_INFO_LOG("GetEqualizerFrequencyBandGains enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    // check
    isFinishGetOptions_ = false;
    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "";
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "engine GetEqualizerFrequencyBandGains failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    if (!stopWaiting || getOptionsResult_ != SUCCESS) {
        AUDIO_ERR_LOG("GetEqualizerFrequencyBandGains Error!");
        return ERROR;
    }
    int32_t parseValue[EQUALIZER_BAND_NUM] = {0};
    ParseValue(value, parseValue);
    for (size_t idx = 0; idx < sizeof(parseValue) / sizeof(parseValue[0]); idx++) {
        frequencyBandGains.gains[idx] = parseValue[idx];
    }
    return SUCCESS;
}

int32_t AudioSuiteManager::GetVoiceBeautifierType(uint32_t nodeId,
    VoiceBeautifierType &voiceBeautifierType)
{
    AUDIO_INFO_LOG("GetVoiceBeautifierType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    // check
    isFinishGetOptions_ = false;
    std::string name = "VoiceBeautifierType";
    std::string value = "";
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "engine GetVoiceBeautifierType failed, ret = %{public}d", ret);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    if (!stopWaiting || getOptionsResult_ != SUCCESS) {
        AUDIO_ERR_LOG("GetVoiceBeautifierType Error!");
        return ERROR;
    }
    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(VoiceBeautifierType::AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR)
        || parseValue > static_cast<int32_t>(VoiceBeautifierType::AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO)) {
        return ERROR;
    }
    voiceBeautifierType = static_cast<VoiceBeautifierType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::RenderFrame(uint32_t pipelineId,
    uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag)
{
    std::mutex* pipelineLock = nullptr;
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = pipelineLockMap_.find(pipelineId);
        CHECK_AND_RETURN_RET_LOG(it != pipelineLockMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock not exist");
        pipelineLock = it->second.get();
        CHECK_AND_RETURN_RET_LOG(pipelineLock != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock is null");
    }
    std::lock_guard<std::mutex> lock(*pipelineLock);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    isFinishRenderFrameMap_[pipelineId] = false;
    int32_t ret = suiteEngine_->RenderFrame(pipelineId, audioData, frameSize, writeLen, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine RenderFrame failed, ret = %{public}d", ret);

    auto& callbackMutex = pipelineCallbackMutexMap_[pipelineId];
    auto& callbackCV = pipelineCallbackCVMap_[pipelineId];
    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    bool stopWaiting = callbackCV->wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS),
        [this, pipelineId] { return isFinishRenderFrameMap_[pipelineId]; });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, "RenderFrame timeout");
        AUDIO_ERR_LOG("RenderFrame timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("RenderFrame leave");
    return renderFrameResultMap_[pipelineId];
}

int32_t AudioSuiteManager::MultiRenderFrame(uint32_t pipelineId,
    AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    std::mutex* pipelineLock = nullptr;
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = pipelineLockMap_.find(pipelineId);
        CHECK_AND_RETURN_RET_LOG(it != pipelineLockMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock not exist");
        pipelineLock = it->second.get();
        CHECK_AND_RETURN_RET_LOG(pipelineLock != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock is null");
    }
    std::lock_guard<std::mutex> lock(*pipelineLock);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    isFinishMultiRenderFrameMap_[pipelineId] = false;
    int32_t ret = suiteEngine_->MultiRenderFrame(
        pipelineId, audioDataArray, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine RenderFrame failed, ret = %{public}d", ret);

    auto& callbackMutex = pipelineCallbackMutexMap_[pipelineId];
    auto& callbackCV = pipelineCallbackCVMap_[pipelineId];
    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    bool stopWaiting = callbackCV->wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS),
        [this, pipelineId] { return isFinishMultiRenderFrameMap_[pipelineId]; });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, "MultiRenderFrame timeout");
        AUDIO_ERR_LOG("MultiRenderFrame timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("MultiRenderFrame leave");
    return multiRenderFrameResultMap_[pipelineId];
}

void AudioSuiteManager::OnCreatePipeline(int32_t result, uint32_t pipelineId)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS) {
        std::ostringstream errorDescription;
        errorDescription << "engine CreatePipeline failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, CREATE_PIPELINE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnCreatePipeline enter");
    isFinishCreatePipeline_ = true;
    engineCreateResult_ = result;
    engineCreatePipelineId_ = pipelineId;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnDestroyPipeline(int32_t result)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST) {
        std::ostringstream errorDescription;
        errorDescription << "engine DestroyPipeline failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, DESTROY_PIPELINE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnDestroyPipeline result: %{public}d", result);
    isFinishDestroyPipeline_ = true;
    destroyPipelineResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnStartPipeline(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnStartPipeline enter");
    isFinishStartPipeline_ = true;
    startPipelineResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnStopPipeline(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnStopPipeline enter");
    isFinishStopPipeline_ = true;
    stopPipelineResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnGetPipelineState(AudioSuitePipelineState state)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetPipelineState enter");
    isFinishGetPipelineState_ = true;
    getPipelineState_ = state;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnCreateNode(int32_t result, uint32_t nodeId)
{
    if (result != SUCCESS) {
        std::ostringstream errorDescription;
        errorDescription << "engine CreateNode failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CREATE_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnCreateNode enter");
    isFinishCreateNode_ = true;
    engineCreateNodeResult_ = result;
    engineCreateNodeId_ = nodeId;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnDestroyNode(int32_t result)
{
    if (result != SUCCESS) {
        std::ostringstream errorDescription;
        errorDescription << "engine DestroyNode failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DESTROY_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnDestroyNode enter");
    isFinishDestroyNode_ = true;
    destroyNodeResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnBypassEffectNode(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnEnableNode enter");
    isFinishBypassEffectNode_ = true;
    bypassEffectNodeResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnGetNodeBypass(int32_t result, bool bypassStatus)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetNodeBypass enter");
    isFinishGetNodeBypassStatus_ = true;
    getNodeBypassResult_ = bypassStatus;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnSetAudioFormat(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnSetAudioFormat enter");
    isFinishSetFormat_ = true;
    setFormatResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnWriteDataCallback(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnWriteDataCallback enter");
    isFinishSetWriteData_ = true;
    setWriteDataResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnConnectNodes(int32_t result)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_UNSUPPORT_CONNECT) {
        std::ostringstream errorDescription;
        errorDescription << "engine ConnectNodes failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CONNECT_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnConnectNodes enter");
    isFinishConnectNodes_ = true;
    connectNodesResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnDisConnectNodes(int32_t result)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_UNSUPPORT_CONNECT) {
        std::ostringstream errorDescription;
        errorDescription << "engine DisConnectNodes failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DISCONNECT_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnDisConnectNodes enter");
    isFinishDisConnectNodes_ = true;
    disConnectNodesResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnRenderFrame(int32_t result, uint32_t pipelineId)
{
    if (result != SUCCESS &&
        result != ERR_NOT_SUPPORTED &&
        result != ERR_ILLEGAL_STATE) {
        std::ostringstream errorDescription;
        errorDescription << "engine RenderFrame failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, errorDescription.str());
    }
    auto& callbackMutex = pipelineCallbackMutexMap_[pipelineId];
    auto& callbackCV = pipelineCallbackCVMap_[pipelineId];
    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    AUDIO_INFO_LOG("OnRenderFrame callback");
    isFinishRenderFrameMap_[pipelineId] = true;
    renderFrameResultMap_[pipelineId] = result;
    callbackCV->notify_all();
}

void AudioSuiteManager::OnMultiRenderFrame(int32_t result, uint32_t pipelineId)
{
    if (result != SUCCESS &&
        result != ERR_NOT_SUPPORTED &&
        result != ERR_ILLEGAL_STATE) {
        std::ostringstream errorDescription;
        errorDescription << "engine MultiRenderFrame failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, errorDescription.str());
    }
    auto& callbackMutex = pipelineCallbackMutexMap_[pipelineId];
    auto& callbackCV = pipelineCallbackCVMap_[pipelineId];
    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    AUDIO_INFO_LOG("OnMultiRenderFrame callback");
    isFinishMultiRenderFrameMap_[pipelineId] = true;
    multiRenderFrameResultMap_[pipelineId] = result;
    callbackCV->notify_all();
}

void AudioSuiteManager::OnGetOptions(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = true;
    getOptionsResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::WriteSuiteEngineUtilizationStatsEvent(AudioNodeType nodeType)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::SUITE_ENGINE_UTILIZATION_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    std::string nodeTypeStr = "";
    auto it = NODETYPE_TOSTRING_MAP.find(nodeType);
    if (it != NODETYPE_TOSTRING_MAP.end()) {
        nodeTypeStr = it->second;
    }
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("AUDIO_NODE_TYPE", nodeTypeStr);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioSuiteManager::WriteSuiteEngineExceptionEvent(uint32_t scene, uint32_t result, std::string description)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::SUITE_ENGINE_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("ERROR_SCENE", static_cast<int32_t>(scene));
    bean->Add("ERROR_CASE", static_cast<int32_t>(result));
    bean->Add("ERROR_DESCRIPTION", description);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
