/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioManagerListenerStub"
#endif

#include "audio_manager_listener_stub.h"
#include "audio_service_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr int32_t MAX_REGISTER_COUNT = 10;
}

AudioManagerListenerStub::AudioManagerListenerStub()
{
}
AudioManagerListenerStub::~AudioManagerListenerStub()
{
}
int AudioManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "AudioManagerStub: ReadInterfaceToken failed");

    switch (code) {
        case ON_PARAMETER_CHANGED: {
            string networkId = data.ReadString();
            AudioParamKey key = static_cast<AudioParamKey>(data.ReadInt32());
            string condition = data.ReadString();
            string value = data.ReadString();
            OnAudioParameterChange(networkId, key, condition, value);
            return AUDIO_OK;
        }
        case ON_WAKEUP_CLOSE: {
            OnWakeupClose();
            return AUDIO_OK;
        }
        case ON_CAPTURER_STATE: {
            bool isActive = data.ReadBool();
            OnCapturerState(isActive);
            return AUDIO_OK;
        }
        case ON_DATATRANSFER_STATE_CHANGE: {
            int32_t callbackId = data.ReadInt32();
            AudioRendererDataTransferStateChangeInfo info;
            info.Unmarshalling(data);
            OnDataTransferStateChange(callbackId, info);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioManagerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioManagerListenerStub::SetParameterCallback(const std::weak_ptr<AudioParameterCallback>& callback)
{
    callback_ = callback;
}

void AudioManagerListenerStub::SetWakeupSourceCallback(const std::weak_ptr<WakeUpSourceCallback>& callback)
{
    wakeUpCallback_ = callback;
}

void AudioManagerListenerStub::OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
    const std::string& condition, const std::string& value)
{
    std::shared_ptr<AudioParameterCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnAudioParameterChange(networkId, key, condition, value);
    } else {
        AUDIO_WARNING_LOG("AudioRingerModeUpdateListenerStub: callback_ is nullptr");
    }
}

void AudioManagerListenerStub::OnCapturerState(bool isActive)
{
    std::shared_ptr<WakeUpSourceCallback> cb = wakeUpCallback_.lock();
    if (cb != nullptr) {
        cb->OnCapturerState(isActive);
    } else {
        AUDIO_WARNING_LOG("AudioManagerListenerStub: OnWakeupClose error");
    }
}

void AudioManagerListenerStub::OnWakeupClose()
{
    std::shared_ptr<WakeUpSourceCallback> cb = wakeUpCallback_.lock();
    if (cb != nullptr) {
        cb->OnWakeupClose();
    } else {
        AUDIO_WARNING_LOG("AudioManagerListenerStub: OnWakeupClose error");
    }
}

void AudioManagerListenerStub::OnDataTransferStateChange(const int32_t &callbackId,
    const AudioRendererDataTransferStateChangeInfo &info)
{
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateChangeMutex_);
        if (stateChangeCallbackMap_.count(callbackId) > 0) {
            callback = stateChangeCallbackMap_[callbackId].second;
        }
    }

    if (callback == nullptr) {
        return;
    }

    callback->OnDataTransferStateChange(info);
}

int32_t AudioManagerListenerStub::AddDataTransferStateChangeCallback(const DataTransferMonitorParam &param,
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

std::vector<int32_t> AudioManagerListenerStub::RemoveDataTransferStateChangeCallback(
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
