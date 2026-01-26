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
#define LOG_TAG "HWDecodingStreamManager"
#endif

#include "hw_decoding_adapter_manager.h"

#include <sstream>
#include <atomic>
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_info.h"
#include "hw_decoding_renderer_impl.h"

// placed at the end to take effect
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {

HWDecodingStreamManager::HWDecodingStreamManager()
{
    AUDIO_INFO_LOG("Constructor");
}

int32_t HWDecodingStreamManager::CreateRender(AudioProcessConfig processConfig,
    std::shared_ptr<IRendererStream> &stream, std::optional<std::string_view> originDeviceName)
{
    uint32_t sessionId = processConfig.originalSessionId;

    AUDIO_INFO_LOG("Create renderer:[%{public}u]", sessionId);

    auto rendererStream = std::make_shared<HWDecodingRendererStream>(processConfig);
    CHECK_AND_RETURN_RET_LOG(rendererStream != nullptr, ERR_OPERATION_FAILED, "Failed to init stream!");

    int32_t ret = rendererStream->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Create rendererStream Failed!");

    rendererStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    rendererStreamMap_[sessionId] = rendererStream;
    stream = rendererStream;

    return SUCCESS;
}

int32_t HWDecodingStreamManager::ReleaseRender(uint32_t streamIndex)
{
    std::unique_lock<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    CHECK_AND_RETURN_RET_PRELOG(it != rendererStreamMap_.end(), SUCCESS, "stream is released:%{public}d", streamIndex);
    std::shared_ptr<IRendererStream> tempStream = rendererStreamMap_[streamIndex];
    rendererStreamMap_[streamIndex] = nullptr;
    rendererStreamMap_.erase(streamIndex);
    AUDIO_INFO_LOG("remained: %{public}zu, released:%{public}d", rendererStreamMap_.size(), streamIndex);
    lock.unlock();

    tempStream->Release();

    return SUCCESS;
}

int32_t HWDecodingStreamManager::StartRender(uint32_t streamIndex)
{
    Trace trace("HWDecodingStreamManager::StartRenderWithSyncId" + std::to_string(streamIndex));
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    CHECK_AND_RETURN_RET_PRELOG(it != rendererStreamMap_.end(), SUCCESS, "No matching stream:%{public}d", streamIndex);

    return rendererStreamMap_[streamIndex]->Start();
}

int32_t HWDecodingStreamManager::StopRender(uint32_t streamIndex)
{
    Trace trace("HWDecodingStreamManager::StopRender" + std::to_string(streamIndex));
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    CHECK_AND_RETURN_RET_PRELOG(it != rendererStreamMap_.end(), SUCCESS, "No matching stream:%{public}d", streamIndex);
    return rendererStreamMap_[streamIndex]->Stop();
}

int32_t HWDecodingStreamManager::PauseRender(uint32_t streamIndex, bool isStandby)
{
    Trace trace("HWDecodingStreamManager::PauseRender" + std::to_string(streamIndex));
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    CHECK_AND_RETURN_RET_PRELOG(it != rendererStreamMap_.end(), SUCCESS, "No matching stream:%{public}d", streamIndex);
    rendererStreamMap_[streamIndex]->Pause();
    return SUCCESS;
}

int32_t HWDecodingStreamManager::TriggerStartIfNecessary()
{
    // do nothing
    return SUCCESS;
}

int32_t HWDecodingStreamManager::GetStreamCount() const noexcept
{
    return rendererStreamMap_.size();
}

int32_t HWDecodingStreamManager::CreateCapturer(AudioProcessConfig processConfig,
    std::shared_ptr<ICapturerStream> &stream)
{
    AUDIO_WARNING_LOG("Not supported");
    return ERR_OPERATION_FAILED;
}

int32_t HWDecodingStreamManager::AddUnprocessStream(int32_t appUid)
{
    AUDIO_WARNING_LOG("Not supported");
    return ERR_OPERATION_FAILED;
}

int32_t HWDecodingStreamManager::ReleaseCapturer(uint32_t streamIndex)
{
    AUDIO_WARNING_LOG("Not supported");
    // do nothing
    return SUCCESS;
}

int32_t HWDecodingStreamManager::GetSessionIdAndRemove(uint32_t paIndex, uint32_t &sessionId)
{
    AUDIO_INFO_LOG("get sessionId: %{public}d for paIndex: %{public}d", sessionId, paIndex);
    return SUCCESS;
}

uint64_t HWDecodingStreamManager::GetLatency() noexcept
{
    return 0;
}

void HWDecodingStreamManager::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    AUDIO_WARNING_LOG("Not supported");
    return;
}
} // namespace AudioStandard
} // namespace OHOS
