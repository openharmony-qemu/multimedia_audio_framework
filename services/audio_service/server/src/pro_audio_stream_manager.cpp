/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "ProAudioStreamManager"
#endif

#include "pro_audio_stream_manager.h"
#include <sstream>
#include <atomic>
#include "audio_service_log.h"
#include "audio_errors.h"
#include "direct_playback_engine.h"
#include "policy_handler.h"
#include "pro_renderer_stream_impl.h"
#include "audio_engine_manager.h"
#include "none_mix_engine.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

ProAudioStreamManager::ProAudioStreamManager(ManagerType type)
    : managerType_(type)
{
    if (type == EAC3_PLAYBACK) {
        playbackEngine_ = std::make_unique<DirectPlayBackEngine>();
    } else {
        playbackEngine_ = std::make_unique<NoneMixEngine>();
    }
    AUDIO_DEBUG_LOG("ProAudioStreamManager");
}

ProAudioStreamManager::~ProAudioStreamManager()
{
    playbackEngine_ = nullptr;
    AUDIO_DEBUG_LOG("~ProAudioStreamManager");
}

int32_t ProAudioStreamManager::CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream)
{
    Trace trace("ProAudioStreamManager::CreateRender");
    AUDIO_DEBUG_LOG("Create renderer start,manager type:%{public}d", managerType_);
    uint32_t sessionId = 0;
    if (processConfig.originalSessionId < MIN_STREAMID || processConfig.originalSessionId > MAX_STREAMID) {
        sessionId = PolicyHandler::GetInstance().GenerateSessionId(processConfig.appInfo.appUid);
    } else {
        sessionId = processConfig.originalSessionId;
    }
    std::shared_ptr<IRendererStream> rendererStream = CreateRendererStream(processConfig);
    CHECK_AND_RETURN_RET_LOG(rendererStream != nullptr, ERR_DEVICE_INIT, "Failed to init rendererStream");
    int32_t ret = CreatePlayBackEngine(rendererStream);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Create play back engine failed. ret:%{public}d", ret);
        rendererStream = nullptr;
        return ret;
    }
    rendererStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    rendererStreamMap_[sessionId] = rendererStream;
    stream = rendererStream;
    return SUCCESS;
}

int32_t ProAudioStreamManager::StartRender(uint32_t streamIndex)
{
    Trace trace("ProAudioStreamManager::StartRender");
    AUDIO_DEBUG_LOG("Start renderer enter");
    std::shared_ptr<IRendererStream> currentRender;
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    currentRender = rendererStreamMap_[streamIndex];
    int32_t result = currentRender->Start();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Failed to start rendererStream");
    if (playbackEngine_) {
        playbackEngine_->Start();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::StopRender(uint32_t streamIndex)
{
    Trace trace("ProAudioStreamManager::StopRender");
    AUDIO_DEBUG_LOG("Stop renderer enter");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    rendererStreamMap_[streamIndex]->Stop();
    if (playbackEngine_) {
        playbackEngine_->Stop();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::PauseRender(uint32_t streamIndex)
{
    Trace trace("ProAudioStreamManager::PauseRender");
    AUDIO_DEBUG_LOG("Pause renderer enter");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    rendererStreamMap_[streamIndex]->Pause();
    if (playbackEngine_) {
        playbackEngine_->Pause();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::ReleaseRender(uint32_t streamIndex)
{
    Trace trace("ProAudioStreamManager::ReleaseRender");
    AUDIO_DEBUG_LOG("Release renderer start");
    std::shared_ptr<IRendererStream> currentRender;
    {
        std::lock_guard<std::mutex> lock(streamMapMutex_);
        auto it = rendererStreamMap_.find(streamIndex);
        if (it == rendererStreamMap_.end()) {
            AUDIO_WARNING_LOG("No matching stream");
            return SUCCESS;
        }
        currentRender = rendererStreamMap_[streamIndex];
        rendererStreamMap_[streamIndex] = nullptr;
        rendererStreamMap_.erase(streamIndex);
        if (playbackEngine_) {
            playbackEngine_->Stop();
            playbackEngine_->RemoveRenderer(currentRender);
        }
    }
    if (currentRender->Release() < 0) {
        AUDIO_WARNING_LOG("Release stream %{public}d failed", streamIndex);
        return ERR_OPERATION_FAILED;
    }
    AUDIO_INFO_LOG("rendererStreamMap_.size() : %{public}zu", rendererStreamMap_.size());
    if (rendererStreamMap_.size() == 0) {
        AUDIO_INFO_LOG("Release the last stream");
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::TriggerStartIfNecessary()
{
    if (playbackEngine_ && !playbackEngine_->IsPlaybackEngineRunning()) {
        AUDIO_INFO_LOG("trigger re-start thread");
        playbackEngine_->Start();
    }
    return SUCCESS;
}

int32_t ProAudioStreamManager::GetStreamCount() const noexcept
{
    return rendererStreamMap_.size();
}

int32_t ProAudioStreamManager::CreatePlayBackEngine(const std::shared_ptr<IRendererStream> &stream)
{
    Trace trace("ProAudioStreamManager::CreatePlayBackEngine");
    int32_t ret = SUCCESS;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioProcessConfig config = stream->GetAudioProcessConfig();
    bool result = PolicyHandler::GetInstance().GetProcessDeviceInfo(config, true, deviceInfo);
    CHECK_AND_RETURN_RET_LOG(result, ERR_DEVICE_INIT, "GetProcessDeviceInfo failed.");
    CHECK_AND_RETURN_RET_LOG(playbackEngine_ != nullptr, ERR_NOT_SUPPORTED, "engine not init!");
    playbackEngine_->Init(deviceInfo, managerType_ == VOIP_PLAYBACK);
    ret = playbackEngine_->AddRenderer(stream);
    return ret;
}

std::shared_ptr<IRendererStream> ProAudioStreamManager::CreateRendererStream(AudioProcessConfig processConfig)
{
    Trace trace("ProAudioStreamManager::CreateRendererStream");
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    bool isDirectStream = managerType_ == DIRECT_PLAYBACK; // direct stream (high resolution) or direct VoIP stream
    std::shared_ptr<ProRendererStreamImpl> rendererStream =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirectStream);
    if (rendererStream->InitParams() != SUCCESS) {
        AUDIO_ERR_LOG("Create rendererStream Failed!");
        return nullptr;
    }
    return rendererStream;
}

int32_t ProAudioStreamManager::CreateCapturer(AudioProcessConfig processConfig,
                                              std::shared_ptr<ICapturerStream> &stream)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t ProAudioStreamManager::ReleaseCapturer(uint32_t streamIndex)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t ProAudioStreamManager::AddUnprocessStream(int32_t appUid)
{
    AUDIO_ERR_LOG("Unsupported operation!");
    return SUCCESS;
}

uint64_t ProAudioStreamManager::GetLatency() noexcept
{
    CHECK_AND_RETURN_RET_LOG(playbackEngine_ != nullptr, 0, "engine not init!");
    return playbackEngine_->GetLatency();
}

void ProAudioStreamManager::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    // not supported
}

} // namespace AudioStandard
} // namespace OHOS