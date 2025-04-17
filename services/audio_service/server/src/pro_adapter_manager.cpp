/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "ProAdapterManager"
#endif

#include "pro_adapter_manager.h"
#include <sstream>
#include <atomic>
#include "audio_common_log.h"
#include "audio_errors.h"
#include "hpae_renderer_stream_impl.h"
#include "hpae_capturer_stream_impl.h"
#include "audio_utils.h"
#include "audio_info.h"
#include "policy_handler.h"
namespace OHOS {
namespace AudioStandard {

const char* PRO_INNER_CAPTURER_SOURCE = "Speaker.monitor";
const char* PRO_NEW_INNER_CAPTURER_SOURCE = "InnerCapturerSink.monitor";
const char* PRO_MONITOR_SOURCE_SUFFIX = ".monitor";

ProAdapterManager::ProAdapterManager(ManagerType type)
{
    AUDIO_INFO_LOG("Constructor with type:%{public}d", type);
    managerType_ = type;
}

int32_t ProAdapterManager::CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_DEBUG_LOG("Create renderer start");
    uint32_t sessionId = 0;
    sessionId = processConfig.originalSessionId;
    if (managerType_ == DUP_PLAYBACK ||
        processConfig.originalSessionId < MIN_STREAMID || processConfig.originalSessionId > MAX_STREAMID) {
        sessionId = PolicyHandler::GetInstance().GenerateSessionId(processConfig.appInfo.appUid);
        AUDIO_ERR_LOG("Create [%{public}d] type renderer:[%{public}u] error",
            managerType_, processConfig.originalSessionId);

    } 
    AUDIO_INFO_LOG("Create [%{public}d] type renderer:[%{public}u]", managerType_, sessionId);
    std::string deviceName;
    int32_t ret = GetDeviceNameForConnect(processConfig, processConfig.originalSessionId, deviceName);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "getdevicename err: %{public}d", ret);

    processConfig.originalSessionId = sessionId;
    // ProAdapterManager is solely responsible for creating paStream objects
    std::shared_ptr<IRendererStream> rendererStream = CreateRendererStream(processConfig, deviceName);
    CHECK_AND_RETURN_RET_LOG(rendererStream != nullptr, ERR_DEVICE_INIT, "Failed to init pa stream");
    rendererStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    rendererStreamMap_[sessionId] = rendererStream;
    stream = rendererStream;

    std::lock_guard<std::mutex> mutex(sinkInputsMutex_);
    SinkInput sinkInput;
    sinkInput.streamId = sessionId;
    sinkInput.streamType = processConfig.streamType;
    sinkInput.uid = processConfig.appInfo.appUid;
    sinkInput.pid = processConfig.appInfo.appPid;
    sinkInput.paStreamId = sessionId;
    sinkInputs_.push_back(sinkInput);
    return SUCCESS;
}

int32_t ProAdapterManager::ReleaseRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Release [%{public}d] type render:[%{public}u]", managerType_, streamIndex);
    std::unique_lock<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    std::shared_ptr<IRendererStream> currentRender = rendererStreamMap_[streamIndex];
    rendererStreamMap_[streamIndex] = nullptr;
    rendererStreamMap_.erase(streamIndex);
    lock.unlock();

    if (currentRender->Release() < 0) {
        AUDIO_WARNING_LOG("Release stream %{public}d failed", streamIndex);
        return ERR_OPERATION_FAILED;
    }

    AUDIO_INFO_LOG("rendererStreamMap_.size() : %{public}zu", rendererStreamMap_.size());
    if (rendererStreamMap_.size() == 0) {
        AUDIO_INFO_LOG("Release the last stream");
    }

    std::lock_guard<std::mutex> mutex(sinkInputsMutex_);
    sinkInputs_.erase(
        std::remove_if(sinkInputs_.begin(),
            sinkInputs_.end(),
            [&](const SinkInput &sinkInput) { return static_cast<uint32_t>(sinkInput.streamId) == streamIndex; }),
        sinkInputs_.end());
    return SUCCESS;
}

int32_t ProAdapterManager::StartRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter StartRender");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    return rendererStreamMap_[streamIndex]->Start();
}

int32_t ProAdapterManager::StopRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter StopRender");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    return rendererStreamMap_[streamIndex]->Stop();
}

int32_t ProAdapterManager::PauseRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter PauseRender");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    rendererStreamMap_[streamIndex]->Pause();
    return SUCCESS;
}

int32_t ProAdapterManager::TriggerStartIfNecessary()
{
    return SUCCESS;
}

int32_t ProAdapterManager::GetStreamCount() const noexcept
{
    if (managerType_ == RECORDER) {
        return capturerStreamMap_.size();
    } else {
        return rendererStreamMap_.size();
    }
}

int32_t ProAdapterManager::GetDeviceNameForConnect(AudioProcessConfig processConfig, uint32_t sessionId,
    std::string &deviceName)
{
    deviceName = "";
    if (processConfig.audioMode == AUDIO_MODE_RECORD) {
        if (processConfig.isWakeupCapturer) {
            int32_t ret = PolicyHandler::GetInstance().SetWakeUpAudioCapturerFromAudioServer(processConfig);
            if (ret < 0) {
                AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
                return ERROR;
            }
            deviceName = PRIMARY_WAKEUP;
        }
        if (processConfig.isInnerCapturer) {
            if (processConfig.innerCapMode == MODERN_INNER_CAP) {
                deviceName = std::string(INNER_CAPTURER_SINK) + std::to_string(processConfig.innerCapId);
            } else {
                deviceName = PRO_INNER_CAPTURER_SOURCE;
            }
        } else if (processConfig.capturerInfo.sourceType == SOURCE_TYPE_REMOTE_CAST) {
            deviceName = std::string(REMOTE_CAST_INNER_CAPTURER_SINK_NAME) + std::string(PRO_MONITOR_SOURCE_SUFFIX);
        }
        return PolicyHandler::GetInstance().NotifyCapturerAdded(processConfig.capturerInfo,
            processConfig.streamInfo, sessionId);
    } else if (managerType_ == DUP_PLAYBACK) {
        deviceName = std::string(INNER_CAPTURER_SINK) + std::to_string(processConfig.innerCapId);
    }
    return SUCCESS;
}

int32_t ProAdapterManager::CreateCapturer(AudioProcessConfig processConfig, std::shared_ptr<ICapturerStream> &stream)
{
    AUDIO_DEBUG_LOG("Create capturer start");
    CHECK_AND_RETURN_RET_LOG(managerType_ == RECORDER, ERROR, "Invalid managerType:%{public}d", managerType_);
    uint32_t sessionId = 0;
    if (processConfig.originalSessionId < MIN_STREAMID || processConfig.originalSessionId > MAX_STREAMID) {
        sessionId = PolicyHandler::GetInstance().GenerateSessionId(processConfig.appInfo.appUid);
        AUDIO_ERR_LOG("Create capturer originalSessionId is error %{public}d", processConfig.originalSessionId);
    } else {
        sessionId = processConfig.originalSessionId;
    }
    processConfig.originalSessionId = sessionId;
    std::string deviceName;
    int32_t ret = GetDeviceNameForConnect(processConfig, processConfig.originalSessionId, deviceName);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "getdevicename err: %{public}d", ret);

    // ProAdapterManager is solely responsible for creating paStream objects
    std::shared_ptr<ICapturerStream> capturerStream = CreateCapturerStream(processConfig, deviceName);
    CHECK_AND_RETURN_RET_LOG(capturerStream != nullptr, ERR_DEVICE_INIT, "Failed to init pa stream");
    capturerStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    capturerStreamMap_[sessionId] = capturerStream;
    stream = capturerStream;
    return SUCCESS;
}

int32_t ProAdapterManager::AddUnprocessStream(int32_t appUid)
{
    return SUCCESS;
}

int32_t ProAdapterManager::ReleaseCapturer(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter ReleaseCapturer");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = capturerStreamMap_.find(streamIndex);
    if (it == capturerStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }

    if (capturerStreamMap_[streamIndex]->Release() < 0) {
        AUDIO_WARNING_LOG("Release stream %{public}d failed", streamIndex);
        return ERR_OPERATION_FAILED;
    }

    capturerStreamMap_[streamIndex] = nullptr;
    capturerStreamMap_.erase(streamIndex);
    if (capturerStreamMap_.size() == 0) {
        AUDIO_INFO_LOG("Release the last stream");
    }
    return SUCCESS;
}

std::shared_ptr<IRendererStream> ProAdapterManager::CreateRendererStream(AudioProcessConfig processConfig,
    const std::string &deviceName)
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    if (managerType_ == DUP_PLAYBACK) {
        // todo check
        processConfig.isInnerCapturer = true;
        AUDIO_INFO_LOG("Create dup playback renderer stream");
    }
    std::shared_ptr<HpaeRendererStreamImpl> rendererStream =
        std::make_shared<HpaeRendererStreamImpl>(processConfig);
    if (rendererStream->InitParams(deviceName) != SUCCESS) {
        AUDIO_ERR_LOG("Create rendererStream Failed");
        return nullptr;
    }
    return rendererStream;
}

std::shared_ptr<ICapturerStream> ProAdapterManager::CreateCapturerStream(AudioProcessConfig processConfig,
    const std::string &deviceName)
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    std::shared_ptr<HpaeCapturerStreamImpl> capturerStream =
        std::make_shared<HpaeCapturerStreamImpl>(processConfig);
    if (capturerStream->InitParams(deviceName) != SUCCESS) {
        AUDIO_ERR_LOG("Create capturerStream Failed, error");
        return nullptr;
    }
    return capturerStream;
}

uint64_t ProAdapterManager::GetLatency() noexcept
{
    return 0;
}

void ProAdapterManager::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    sinkInputs = sinkInputs_;
    return;
}
} // namespace AudioStandard
} // namespace OHOS
