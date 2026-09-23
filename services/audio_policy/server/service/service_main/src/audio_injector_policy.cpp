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
#define LOG_TAG "AudioInjectorPolicy"
#endif

#include "audio_injector_policy.h"
#include "audio_core_service.h"
#include "audio_device_info.h"
#include "audio_policy_manager_factory.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
AudioInjectorPolicy::AudioInjectorPolicy()
    :audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
     audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())
{
    pipeManager_ = AudioPipeManager::GetPipeManager();
    isConnected_ = false;
    isOpened_ = false;
}

int32_t AudioInjectorPolicy::Init()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        AudioModuleInfo moduleInfo = {};
        moduleInfo.lib = "libmodule-hdi-sink.z.so";
        std::string name = VIRTUAL_INJECTOR;
        moduleInfo.name = name;
        moduleInfo.deviceType = std::to_string(static_cast<int32_t>(DEVICE_TYPE_SYSTEM_PRIVATE));
        moduleInfo.className = VIRTUAL_INJECTOR;
        moduleInfo.role = AudioAdapterManager::HDI_AUDIO_PORT_SINK_ROLE;
        moduleInfo.format = "s16le";
        moduleInfo.channels = "2"; // 2 channel
        moduleInfo.rate = "48000";
        moduleInfo.bufferSize = "3840"; // 20ms

        uint32_t paIndex = 0;
        ioHandle_ = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
        CHECK_AND_RETURN_RET_LOG(paIndex != HDI_INVALID_ID, ERR_OPERATION_FAILED,
            "Injector::OpenAudioPort failed paId[%{public}u]", paIndex);

        std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
        pipeInfo->id_ = ioHandle_;
        pipeInfo->paIndex_ = paIndex;
        pipeInfo->name_ = VIRTUAL_INJECTOR;
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        pipeInfo->adapterName_ = moduleInfo.name;
        pipeInfo->moduleInfo_ = moduleInfo;
        pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
        pipeInfo->InitAudioStreamInfo();
        AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.AddIOHandleInfo(VIRTUAL_INJECTOR, ioHandle_);
        isOpened_ = true;
        this->moduleInfo_ = moduleInfo;
        renderPortIdx_ = paIndex;
    }
    return SUCCESS;
}

int32_t AudioInjectorPolicy::DeInit()
{
    if (isOpened_ && rendererStreamMap_.size() == 0) {
        int32_t ret = audioPolicyManager_.CloseAudioPort(ioHandle_, renderPortIdx_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::close port failed");
        std::shared_ptr<AudioPipeInfo> pipeInfo = pipeManager_->GetPipeinfoByNameAndFlag(
            VIRTUAL_INJECTOR, AUDIO_OUTPUT_FLAG_NORMAL);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.DelIOHandleInfo(VIRTUAL_INJECTOR);
        if (pipeInfo != nullptr) {
            audioPolicyManager_.RemoveAudioPipeVolume(pipeInfo->id_);
        }
        isOpened_ = false;
        renderPortIdx_ = HDI_INVALID_ID;
    }
    return SUCCESS;
}

void AudioInjectorPolicy::UpdateAudioInfo(AudioModuleInfo &info)
{
    audioPolicyManager_.UpdateAudioPortInfo(renderPortIdx_, info);
}

int32_t AudioInjectorPolicy::AddStreamDescriptor(uint32_t renderId, std::shared_ptr<AudioStreamDescriptor> desc)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    rendererStreamMap_[renderId] = desc;
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveStreamDescriptor(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    AUDIO_INFO_LOG("Injector:: renderId: %{public}u", renderId);
    rendererStreamMap_.erase(renderId);
    rendererMuteStreamMap_.erase(renderId);
    injectCaptureTargetUidMap_.erase(renderId);
    injectCaptureTargetStreamIdMap_.erase(renderId);
    injectorStreamIds_.erase(renderId);
    if (rendererStreamMap_.size() == 0) {
        RemoveCaptureInjectorInner(false);
    }
    ResetInjectorModeIfNeeded();
    return SUCCESS;
}

bool AudioInjectorPolicy::IsContainStream(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto streamIt = rendererStreamMap_.find(renderId);
    if (streamIt != rendererStreamMap_.end()) {
        return true;
    }
    return false;
}

std::string AudioInjectorPolicy::GetAdapterName()
{
    return moduleInfo_.name;
}

// get the number of rendererStream moved in Injector
int32_t AudioInjectorPolicy::GetRendererStreamCount()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return rendererStreamMap_.size();
}

void AudioInjectorPolicy::SetCapturePortIdx(uint32_t idx)
{
    capturePortIdx_ = idx;
}

uint32_t AudioInjectorPolicy::GetCapturePortIdx()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return capturePortIdx_;
}

void AudioInjectorPolicy::SetRendererPortIdx(uint32_t idx)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    renderPortIdx_ = idx;
}

uint32_t AudioInjectorPolicy::GetRendererPortIdx()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return renderPortIdx_;
}

AudioModuleInfo& AudioInjectorPolicy::GetAudioModuleInfo()
{
    return moduleInfo_;
}

bool AudioInjectorPolicy::GetIsConnected()
{
    return isConnected_;
}

void AudioInjectorPolicy::SetVoipType(VoipType type)
{
    voipType_ = type;
}

void AudioInjectorPolicy::ResetInjectorModeIfNeeded()
{
    if (injectorStreamIds_.empty() && rendererStreamMap_.empty() && injectCaptureTargetStreamIdMap_.empty()) {
        injectorWorkMode_ = INJECTOR_MODE_NONE;
        activeTargetUid_ = INVALID_UID;
        activeTargetStreamId_ = INVALID_STREAM_ID;
    }
}

int32_t AudioInjectorPolicy::PrepareLegacyRenderer(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    CHECK_AND_RETURN_RET_LOG(injectorWorkMode_ != INJECTOR_MODE_STREAM_ID_TARGET, ERR_NOT_SUPPORTED,
        "Injector::precise target mode is active");
    injectorWorkMode_ = INJECTOR_MODE_LEGACY_TARGET;
    injectCaptureTargetUidMap_.erase(renderId);
    injectCaptureTargetStreamIdMap_.erase(renderId);
    injectorStreamIds_.insert(renderId);
    return SUCCESS;
}

int32_t AudioInjectorPolicy::PreparePreciseRenderer(uint32_t renderId, int32_t targetUid, uint32_t targetStreamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    CHECK_AND_RETURN_RET_LOG(targetUid >= 0, ERR_INVALID_PARAM, "Injector::invalid targetUid");
    CHECK_AND_RETURN_RET_LOG(ValidStreamId(targetStreamId), ERR_INVALID_PARAM,
        "Injector::invalid targetStreamId");
    CHECK_AND_RETURN_RET_LOG(injectorWorkMode_ != INJECTOR_MODE_LEGACY_TARGET, ERR_NOT_SUPPORTED,
        "Injector::legacy target mode is active");
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET &&
        (activeTargetUid_ != targetUid || activeTargetStreamId_ != targetStreamId)) {
        AUDIO_ERR_LOG("Injector::only one precise target stream is supported");
        return ERR_NOT_SUPPORTED;
    }
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
    VoipType type = NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto pipe = FindCaptureVoipPipeByTarget(pipeManager_->GetPipeList(), targetUid, targetStreamId, type, ret);
    CHECK_AND_RETURN_RET_LOG(pipe != nullptr, ret, "Injector::target capturer is invalid: %{public}d", ret);
    (void)pipe;
    injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    activeTargetUid_ = targetUid;
    activeTargetStreamId_ = targetStreamId;
    injectCaptureTargetUidMap_[renderId] = targetUid;
    injectCaptureTargetStreamIdMap_[renderId] = targetStreamId;
    injectorStreamIds_.insert(renderId);
    return SUCCESS;
}

void AudioInjectorPolicy::ClearRendererTarget(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    injectCaptureTargetUidMap_.erase(renderId);
    injectCaptureTargetStreamIdMap_.erase(renderId);
    injectorStreamIds_.erase(renderId);
    ResetInjectorModeIfNeeded();
}

bool AudioInjectorPolicy::IsPreciseRenderer(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return injectCaptureTargetStreamIdMap_.count(renderId) > 0;
}

void AudioInjectorPolicy::ReleaseCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "Injector::pipeManager_ is null");
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET) {
        VoipType type = NO_VOIP;
        int32_t ret = ERR_INVALID_PARAM;
        auto pipe = FindCaptureVoipPipeByTarget(pipeManager_->GetPipeList(), activeTargetUid_,
            activeTargetStreamId_, type, ret);
        if (pipe == nullptr) {
            RemoveCaptureInjectorByTargetStreamInner(true);
        }
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    auto pipeList = pipeManager_->GetPipeList();
    for (auto it = pipeList.rbegin(); it != pipeList.rend(); ++it) {
        CHECK_AND_CONTINUE_LOG((*it) != nullptr, "Injector::it is null");
        // because the paindex of all low_latency pipe is 0, distinguishing between types of low-latency paths
        // requires checking Voip-specific scenarios to trigger the following processing
        bool isPortIdxUsed = (voipType_ != FAST_VOIP ||
            ((*it)->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)));
        CHECK_AND_CONTINUE(isPortIdxUsed);
        if ((*it)->paIndex_ == capturePortIdx_ && (*it)->pipeRole_ == PIPE_ROLE_INPUT) {
            streamVec = (*it)->streamDescriptors_;
            break;
        }
    }
    if (streamVec.size() == 0) {
        RemoveCaptureInjectorInner(true);
        return ;
    }
}

void AudioInjectorPolicy::RebuildCaptureInjector(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        return;
    }
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET) {
        RebuildCaptureInjectorByTargetStream(streamId);
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    uint32_t paIndex = 0;
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "Injector::pipeManager_ is null");
    auto pipeList = pipeManager_->GetPipeList();
    for (const auto &pipe : pipeList) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            if (stream->sessionId_ == streamId) {
                paIndex = pipe->paIndex_;
                streamDesc = stream;
            }
        }
        // because the paindex of all low_latency pipe is 0, distinguishing between types of low-latency paths
        // requires checking Voip-specific scenarios to trigger the following processing
        bool isPortIdxUsed = (voipType_ != FAST_VOIP ||
            (pipe->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)));
        CHECK_AND_CONTINUE(isPortIdxUsed);
        if (pipe->paIndex_ == capturePortIdx_ && pipe->pipeRole_ == PIPE_ROLE_INPUT) {
            streamVec = pipe->streamDescriptors_;
        }
    }
    bool isRunning = HasRunningVoipStream(streamVec);
    if (isRunning) {
        return;
    }
    RemoveCaptureInjectorInner(true);
    if (rendererStreamMap_.size() == 0) {
        return;
    }
    capturePortIdx_ = paIndex;
    CHECK_AND_RETURN(streamDesc != nullptr);
    if ((streamDesc->routeFlag_ & AUDIO_INPUT_FLAG_NORMAL) &&
            streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        voipType_ = NORMAL_VOIP;
    } else if (streamDesc->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)) {
        voipType_ = FAST_VOIP;
    }
    AddCaptureInjectorInner();
}

void AudioInjectorPolicy::RebuildCaptureInjectorByTargetStream(uint32_t streamId)
{
    CHECK_AND_RETURN_LOG(streamId == activeTargetStreamId_, "Injector::not active target stream");
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "Injector::pipeManager_ is null");
    VoipType type = NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto pipe = FindCaptureVoipPipeByTarget(pipeManager_->GetPipeList(), activeTargetUid_,
        activeTargetStreamId_, type, ret);
    if (pipe == nullptr) {
        RemoveCaptureInjectorByTargetStreamInner(true);
        return;
    }
    if (!isConnected_ || pipe->paIndex_ != capturePortIdx_ || type != voipType_) {
        RemoveCaptureInjectorByTargetStreamInner(true);
        capturePortIdx_ = pipe->paIndex_;
        voipType_ = type;
        AddCaptureInjectorByTargetStreamInner(activeTargetStreamId_);
    }
}

bool AudioInjectorPolicy::HasRunningVoipStream(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamVec)
{
    for (const auto &stream : streamVec) {
        if (stream->IsRunning() && stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            return true;
        }
    }
    return false;
}

int32_t AudioInjectorPolicy::AddCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return AddCaptureInjectorInner();
}

int32_t AudioInjectorPolicy::AddCaptureInjectorInner()
{
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET) {
        return AddCaptureInjectorByTargetStreamInner(activeTargetStreamId_);
    }
    if (!isConnected_) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
        int32_t enableNewRoute = 0;
        GetSysPara("persist.multimedia.audio.enable_new_route", enableNewRoute);
        if (voipType_ == NORMAL_VOIP || voipType_ == FAST_VOIP) {
            audioPolicyManager_.AddCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
            isConnected_ = true;
        }
    }
    return SUCCESS;
}

int32_t AudioInjectorPolicy::AddCaptureInjectorByTargetStream(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto targetUidIt = injectCaptureTargetUidMap_.find(renderId);
    auto targetStreamIdIt = injectCaptureTargetStreamIdMap_.find(renderId);
    CHECK_AND_RETURN_RET_LOG(targetUidIt != injectCaptureTargetUidMap_.end() &&
        targetStreamIdIt != injectCaptureTargetStreamIdMap_.end(), ERR_INVALID_PARAM,
        "Injector::renderer has no precise target");
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
    VoipType type = NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto pipe = FindCaptureVoipPipeByTarget(pipeManager_->GetPipeList(), targetUidIt->second,
        targetStreamIdIt->second, type, ret);
    CHECK_AND_RETURN_RET_LOG(pipe != nullptr, ret, "Injector::target capturer is invalid: %{public}d", ret);
    capturePortIdx_ = pipe->paIndex_;
    voipType_ = type;
    activeTargetUid_ = targetUidIt->second;
    activeTargetStreamId_ = targetStreamIdIt->second;
    return AddCaptureInjectorByTargetStreamInner(targetStreamIdIt->second);
}

int32_t AudioInjectorPolicy::AddCaptureInjectorByTargetStreamInner(uint32_t targetStreamId)
{
    if (!isConnected_) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
        CHECK_AND_RETURN_RET_LOG(ValidStreamId(targetStreamId), ERR_INVALID_PARAM,
            "Injector::targetStreamId is invalid");
        audioPolicyManager_.AddCaptureInjectorByTargetStream(renderPortIdx_, capturePortIdx_, targetStreamId);
        if (voipType_ == FAST_VOIP) {
            UpdateAudioInfo(moduleInfo_);
        }
        isConnected_ = true;
    }
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveCaptureInjector(bool noCapturer)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return RemoveCaptureInjectorInner(noCapturer);
}

int32_t AudioInjectorPolicy::RemoveCaptureInjectorInner(bool noCapturer)
{
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET) {
        return RemoveCaptureInjectorByTargetStreamInner(noCapturer);
    }
    bool flag = (rendererStreamMap_.size() == 0 || noCapturer);
    if (isConnected_ && flag) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
        if (voipType_ == NORMAL_VOIP || voipType_ == FAST_VOIP) {
            audioPolicyManager_.RemoveCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
            isConnected_ = false;
            capturePortIdx_ = HDI_INVALID_ID;
            voipType_ = NO_VOIP;
        }
    }
    // release capturer stream does not require executing Deinit();
    if (!noCapturer) {
        DeInit();
    }
    return SUCCESS;
}

int32_t AudioInjectorPolicy::RemoveCaptureInjectorByTargetStreamInner(bool noCapturer)
{
    bool flag = (rendererStreamMap_.size() == 0 || noCapturer);
    if (isConnected_ && flag) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
        audioPolicyManager_.RemoveCaptureInjectorByTargetStream(renderPortIdx_, capturePortIdx_,
            activeTargetStreamId_);
        isConnected_ = false;
        capturePortIdx_ = HDI_INVALID_ID;
        voipType_ = NO_VOIP;
    }
    if (!noCapturer) {
        DeInit();
    }
    return SUCCESS;
}

std::shared_ptr<AudioPipeInfo> AudioInjectorPolicy::FindCaptureVoipPipe(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, uint32_t &streamId)
{
    std::shared_ptr<AudioPipeInfo> voipPipe = nullptr;
    for (const auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            bool isRunning = stream->IsRunning();
            CHECK_AND_CONTINUE_LOG(isRunning == true, "Injector::isRunning is false");
            AudioStreamAction action = stream->streamAction_;
            bool actionFlag = (action == AUDIO_STREAM_ACTION_MOVE);
            CHECK_AND_CONTINUE_LOG(actionFlag, "Injector::streamAction is not right");
            if ((stream->routeFlag_ & AUDIO_INPUT_FLAG_NORMAL) &&
                    stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
                voipPipe = pipe;
                streamId = stream->sessionId_;
                break;
            } else if (stream->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)) {
                voipPipe = pipe;
            }
        }
    }
    return voipPipe;
}

std::shared_ptr<AudioPipeInfo> AudioInjectorPolicy::FindCaptureVoipPipeByTarget(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, int32_t targetUid, uint32_t targetStreamId,
    VoipType &type, int32_t &ret)
{
    ret = ERR_INVALID_PARAM;
    for (const auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            CHECK_AND_CONTINUE(stream->sessionId_ == targetStreamId);
            CHECK_AND_RETURN_RET_LOG(stream->IsRecording() && pipe->pipeRole_ == PIPE_ROLE_INPUT, nullptr,
                "Injector::target stream is not a capture stream");
            CHECK_AND_RETURN_RET_LOG(stream->GetRealUid() == targetUid, nullptr,
                "Injector::target uid mismatch");
            if (stream->capturerInfo_.sourceType != SOURCE_TYPE_VOICE_COMMUNICATION) {
                ret = ERR_NOT_SUPPORTED;
                AUDIO_ERR_LOG("Injector::target source type is not voice communication");
                return nullptr;
            }
            if (!stream->IsRunning()) {
                ret = ERR_ILLEGAL_STATE;
                AUDIO_ERR_LOG("Injector::target capturer is not running");
                return nullptr;
            }
            if (stream->routeFlag_ & AUDIO_INPUT_FLAG_NORMAL) {
                type = NORMAL_VOIP;
                ret = SUCCESS;
                return pipe;
            }
            if (stream->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)) {
                type = FAST_VOIP;
                ret = SUCCESS;
                return pipe;
            }
            ret = ERR_NOT_SUPPORTED;
            AUDIO_ERR_LOG("Injector::target route flag is not supported");
            return nullptr;
        }
    }
    return nullptr;
}

std::shared_ptr<AudioPipeInfo> AudioInjectorPolicy::FindPipeByStreamId(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, VoipType &type, uint32_t &streamId)
{
    std::shared_ptr<AudioPipeInfo> voipPipe = nullptr;
    CHECK_AND_RETURN_RET_LOG(streamId != UINT32_INVALID_VALUE, voipPipe, "Injector:: streamId is wrong!");
    for (const auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            bool isRunning = stream->IsRunning();
            CHECK_AND_CONTINUE_LOG(isRunning == true, "Injector::isRunning is false");
            if (streamId == stream->sessionId_) {
                voipPipe = pipe;
                type = NORMAL_VOIP;
                break;
            }
        }
    }
    return voipPipe;
}

void AudioInjectorPolicy::FetchCapDeviceInjectPreProc(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, bool &removeFlag, uint32_t &streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        return;
    }
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET) {
        VoipType type = NO_VOIP;
        int32_t ret = ERR_INVALID_PARAM;
        std::shared_ptr<AudioPipeInfo> tempPipe = FindCaptureVoipPipeByTarget(pipeInfos, activeTargetUid_,
            activeTargetStreamId_, type, ret);
        if (tempPipe == nullptr || tempPipe->paIndex_ != capturePortIdx_ || type != voipType_) {
            RemoveCaptureInjectorByTargetStreamInner(true);
            removeFlag = true;
            streamId = activeTargetStreamId_;
        }
        return;
    }
    VoipType type = VoipType::NO_VOIP;
    std::shared_ptr<AudioPipeInfo> tempPipe = FindCaptureVoipPipe(pipeInfos, streamId);
    if (tempPipe != nullptr && tempPipe->paIndex_ != capturePortIdx_) {
        RemoveCaptureInjectorInner(true);
        removeFlag = true;
    }
}

void AudioInjectorPolicy::FetchCapDeviceInjectPostProc(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, bool &removeFlag, uint32_t &streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        return;
    }
    if (injectorWorkMode_ == INJECTOR_MODE_STREAM_ID_TARGET) {
        CHECK_AND_RETURN(removeFlag);
        VoipType type = NO_VOIP;
        int32_t ret = ERR_INVALID_PARAM;
        std::shared_ptr<AudioPipeInfo> tempPipe = FindCaptureVoipPipeByTarget(pipeInfos, activeTargetUid_,
            activeTargetStreamId_, type, ret);
        CHECK_AND_RETURN_LOG(tempPipe != nullptr, "Injector::precise target pipe not found");
        capturePortIdx_ = tempPipe->paIndex_;
        voipType_ = type;
        AddCaptureInjectorByTargetStreamInner(activeTargetStreamId_);
        return;
    }
    VoipType type = VoipType::NO_VOIP;
    std::shared_ptr<AudioPipeInfo> tempPipe = FindPipeByStreamId(pipeInfos, type, streamId);
    if (tempPipe != nullptr && removeFlag) {
        capturePortIdx_ = tempPipe->paIndex_;
        voipType_ = type;
        AddCaptureInjectorInner();
    }
}

void AudioInjectorPolicy::AddInjectorStreamId(const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    CHECK_AND_RETURN(injectorWorkMode_ != INJECTOR_MODE_STREAM_ID_TARGET);
    injectorStreamIds_.insert(streamId);
}

void AudioInjectorPolicy::DeleteInjectorStreamId(const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    injectorStreamIds_.erase(streamId);
    ResetInjectorModeIfNeeded();
}

bool AudioInjectorPolicy::IsActivateInterruptStreamId(const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return injectorStreamIds_.count(streamId) > 0;
}

void AudioInjectorPolicy::SendInterruptEventToInjectorStreams(const std::shared_ptr<AudioPolicyServerHandler> &handler)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
        INTERRUPT_HINT_STOP, 1.0f};
    for (const auto& pair : rendererStreamMap_) {
        if (handler != nullptr) {
            handler->SendInterruptEventWithStreamIdCallback(interruptEvent, pair.first);
        }
    }
}

void AudioInjectorPolicy::SetInjectStreamsMuteForInjection(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto mute = rendererMuteStreamMap_.find(streamId);
    AUDIO_INFO_LOG("Injector:: streamId: %{public}u, mapIsExist: %{public}d, mute: %{public}d",
        streamId, mute == rendererMuteStreamMap_.end(), isNeedMuteRenderer_);
    if (mute == rendererMuteStreamMap_.end() && isNeedMuteRenderer_) {
        rendererMuteStreamMap_.insert(std::make_pair(streamId, isNeedMuteRenderer_));
        AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(streamId, isNeedMuteRenderer_);
    }
}

void AudioInjectorPolicy::SetInjectStreamsMuteForPlayback(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto mute = rendererMuteStreamMap_.find(streamId);
    AUDIO_INFO_LOG("Injector:: streamId: %{public}u, mapIsExist: %{public}d", streamId,
        mute != rendererMuteStreamMap_.end());
    if (mute != rendererMuteStreamMap_.end() && mute->second == true) {
        AUDIO_INFO_LOG("Injector:: setMuteFalse: %{public}u", streamId);
        AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(streamId, false);
        rendererMuteStreamMap_[streamId] = false;
    }
}

void AudioInjectorPolicy::SetInjectorStreamsMute(bool newMicrophoneMute)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    isNeedMuteRenderer_ = newMicrophoneMute;
    AUDIO_INFO_LOG("Injector:: %{public}d", newMicrophoneMute);
    for (const auto& streamId : injectorStreamIds_) {
        auto mute = rendererMuteStreamMap_.find(streamId);
        AUDIO_INFO_LOG("Injector:: streamId: %{public}u, mapIsExist: %{public}d", streamId,
            mute == rendererMuteStreamMap_.end());
        if (mute == rendererMuteStreamMap_.end()) {
            rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
        } else {
            rendererMuteStreamMap_[streamId] = newMicrophoneMute;
        }
        AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(streamId, newMicrophoneMute);
    }
}
}  //  namespace AudioStandard
}  //  namespace OHOS
