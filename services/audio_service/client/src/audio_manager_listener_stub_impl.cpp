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
#define LOG_TAG "AudioManagerListenerStubImpl"
#endif

#include "audio_errors.h"
#include "audio_manager_listener_stub_impl.h"
#include "audio_service_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr int32_t MAX_REGISTER_COUNT = 10;
}

void AudioManagerListenerStubImpl::SetParameterCallback(const std::weak_ptr<AudioParameterCallback>& callback)
{
    callback_ = callback;
}

void AudioManagerListenerStubImpl::SetWakeupSourceCallback(const std::weak_ptr<WakeUpSourceCallback>& callback)
{
    wakeUpCallback_ = callback;
}

int32_t AudioManagerListenerStubImpl::OnAudioParameterChange(const std::string &networkId, int32_t key,
    const std::string& condition, const std::string& value)
{
    std::shared_ptr<AudioParameterCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnAudioParameterChange(networkId, static_cast<AudioParamKey>(key), condition, value);
    } else {
        AUDIO_WARNING_LOG("AudioRingerModeUpdateListenerStub: callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioManagerListenerStubImpl::OnCapturerState(bool isActive)
{
    std::shared_ptr<WakeUpSourceCallback> cb = wakeUpCallback_.lock();
    if (cb != nullptr) {
        cb->OnCapturerState(isActive);
    } else {
        AUDIO_WARNING_LOG("OnCapturerState error");
    }
    return SUCCESS;
}

int32_t AudioManagerListenerStubImpl::OnWakeupClose()
{
    std::shared_ptr<WakeUpSourceCallback> cb = wakeUpCallback_.lock();
    if (cb != nullptr) {
        cb->OnWakeupClose();
    } else {
        AUDIO_WARNING_LOG("OnWakeupClose error");
    }
    return SUCCESS;
}

int32_t AudioManagerListenerStubImpl::OnDataTransferStateChange(int32_t callbackId,
    const AudioRendererDataTransferStateChangeInfo &info)
{
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> callback = nullptr;
    std::unique_lock<std::mutex> lock(stateChangeMutex_);
    if (stateChangeCallbackMap_.count(callbackId) > 0) {
        callback = stateChangeCallbackMap_[callbackId].second;
    }
    lock.unlock();
    if (callback) {
        callback->OnDataTransferStateChange(info);
    }
    return SUCCESS;
}

int32_t AudioManagerListenerStubImpl::OnMuteStateChange(int32_t callbackId, int32_t uid,
    uint32_t sessionId, bool isMuted)
{
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> callback = nullptr;
    std::unique_lock<std::mutex> lock(stateChangeMutex_);
    auto it = stateChangeCallbackMap_.find(callbackId);
    CHECK_AND_RETURN_RET(it != stateChangeCallbackMap_.end(), SUCCESS);
    callback = stateChangeCallbackMap_[callbackId].second;
    lock.unlock();
    CHECK_AND_RETURN_RET(callback != nullptr, SUCCESS);
    callback->OnMuteStateChange(uid, sessionId, isMuted);

    return SUCCESS;
}

int32_t AudioManagerListenerStubImpl::AddDataTransferStateChangeCallback(const DataTransferMonitorParam &param,
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> cb)
{
    std::lock_guard<std::mutex> lock(stateChangeMutex_);
    if (stateChangeCallbackMap_.size() >= MAX_REGISTER_COUNT) {
        return -1;
    }

    for (auto it = stateChangeCallbackMap_.begin(); it != stateChangeCallbackMap_.end(); ++it) {
        if (it->second.first == param && it->second.second == cb) {
            return it->first;
        }
    }

    ++callbackId_;
    stateChangeCallbackMap_[callbackId_] = std::make_pair(param, cb);
    return callbackId_;
}

std::vector<int32_t> AudioManagerListenerStubImpl::RemoveDataTransferStateChangeCallback(
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> cb)
{
    std::lock_guard<std::mutex> lock(stateChangeMutex_);
    std::vector<int32_t> callbackIds;
    for (auto it = stateChangeCallbackMap_.begin(); it != stateChangeCallbackMap_.end();) {
        if (it->second.second == cb) {
            callbackIds.push_back(it->first);
            it = stateChangeCallbackMap_.erase(it);
        } else {
            ++it;
        }
    }

    return callbackIds;
}

} // namespace AudioStandard
} // namespace OHOS
