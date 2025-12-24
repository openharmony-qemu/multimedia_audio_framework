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

#include "audio_wakeup_client_manager.h"
#include "audio_service_proxy.h"

#include <memory>
#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_manager_listener_stub_impl.h"

namespace OHOS {
namespace AudioStandard {
AudioWakeupClientManager &AudioWakeupClientManager::GetInstance()
{
    static AudioWakeupClientManager instance;
    return instance;
}

class WakeUpCallbackImpl : public WakeUpSourceCallback {
    public:
        WakeUpCallbackImpl()
        {
        }
        void OnCapturerState(bool isActive) override
        {
            AudioWakeupClientManager::GetInstance().OnCapturerState(isActive);
        }
        void OnWakeupClose() override
        {
            AudioWakeupClientManager::GetInstance().OnWakeupClose();
        }
};

int32_t AudioWakeupClientManager::RegisterWakeupSourceCallback()
{
    AUDIO_INFO_LOG("RegisterWakeupSourceCallback");
    remoteWakeUpCallback_ = std::make_shared<WakeUpCallbackImpl>();

    sptr<AudioManagerListenerStubImpl> wakeupCloseCbStub = new(std::nothrow) AudioManagerListenerStubImpl();
    CHECK_AND_RETURN_RET_LOG(wakeupCloseCbStub != nullptr, ERROR, "wakeupCloseCbStub is null");
    wakeupCloseCbStub->SetWakeupSourceCallback(remoteWakeUpCallback_);

    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERROR, "GetAudioParameter::Audio service unavailable.");

    sptr<IRemoteObject> object = wakeupCloseCbStub->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "SetWakeupCloseCallback listenerStub object is nullptr");

    return gasp->SetWakeupSourceCallback(object);
}

int32_t AudioWakeupClientManager::SetAudioCapturerSourceCallback(
    const std::shared_ptr<AudioCapturerSourceCallback> &callback)
{
    audioCapturerSourceCallback_ = callback;
    return RegisterWakeupSourceCallback();
}

int32_t AudioWakeupClientManager::SetWakeUpSourceCloseCallback(
    const std::shared_ptr<WakeUpSourceCloseCallback> &callback)
{
    audioWakeUpSourceCloseCallback_ = callback;
    return RegisterWakeupSourceCallback();
}

void AudioWakeupClientManager::OnCapturerState(bool isActive)
{
    if (audioCapturerSourceCallback_ != nullptr) {
        audioCapturerSourceCallback_ -> OnCapturerState(isActive);
    }
}
void AudioWakeupClientManager::OnWakeupClose()
{
    if (audioWakeUpSourceCloseCallback_ != nullptr) {
        audioWakeUpSourceCloseCallback_ -> OnWakeupClose();
    }
}
} // namespace AudioStandard
} // namespace OHOS
