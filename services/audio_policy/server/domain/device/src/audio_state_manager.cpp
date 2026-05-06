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
#define LOG_TAG "AudioStateManager"
#endif

#include "audio_state_manager.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"
#include "audio_bundle_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr int32_t AUDIO_UID = 1041;
constexpr int32_t ANCO_SERVICE_BROKER_UID = 5557;
constexpr AudioDeviceUsage EXCLUDED_DEVICE_USAGES[] {
    MEDIA_OUTPUT_DEVICES, MEDIA_INPUT_DEVICES, CALL_OUTPUT_DEVICES, CALL_INPUT_DEVICES};

void AudioStateManager::SetPreferredRingRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredRingRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredToneRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredToneRenderDevice_ = deviceDescriptor;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRingRenderDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredRingRenderDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredToneRenderDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredToneRenderDevice_);
    return devDesc;
}

void AudioStateManager::SetPreferredRecognitionCaptureDevice(const shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredRecognitionCaptureDevice_ = desc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRecognitionCaptureDevice()
{
    lock_guard<std::mutex> lock(mutex_);
    return preferredRecognitionCaptureDevice_;
}

int32_t AudioStateManager::SetAudioVKBInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioVKBInfoMgrCallback_ = callback;
    AUDIO_INFO_LOG("VKB audioVKBInfoMgrCallback_ is nullptr:%{public}s",
        audioVKBInfoMgrCallback_ == nullptr ? "T" : "F");
    return 0;
}

int32_t AudioStateManager::CheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (audioVKBInfoMgrCallback_ != nullptr) {
        audioVKBInfoMgrCallback_->OnCheckVKBInfo(bundleName, isValid);
    }
    AUDIO_INFO_LOG("isVKB:%{public}s", isValid ? "T" : "F");
    return 0;
}
} // namespace AudioStandard
} // namespace OHOS
