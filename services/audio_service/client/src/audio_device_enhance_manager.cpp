/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_device_enhance_manager.h"

#include <unistd.h>
#include "audio_errors.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t APP_SELECT_INDEPENDENT = 1;
}

AudioDeviceEnhanceManager &AudioDeviceEnhanceManager::GetInstance()
{
    static AudioDeviceEnhanceManager manager;
    return manager;
}

int32_t AudioDeviceEnhanceManager::IsEnhancedRoutingSupported(bool &supported)
{
    supported = AudioPolicyManager::GetInstance().IsEnhancedRoutingSupported();
    return SUCCESS;
}

int32_t AudioDeviceEnhanceManager::SelectOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { desc };
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_OPERATION_FAILED, "create renderer filter failed");
    int32_t uid = static_cast<int32_t>(getuid());
    audioRendererFilter->uid = uid;
    
    return AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors,
        APP_SELECT_INDEPENDENT);
}

int32_t AudioDeviceEnhanceManager::SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { desc };
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr, ERR_OPERATION_FAILED, "create capturer filter failed");
    int32_t uid = static_cast<int32_t>(getuid());
    audioCapturerFilter->uid = uid;
    audioCapturerFilter->audioDeviceSelectMode = APP_SELECT_INDEPENDENT;
    return AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}
} // namespace AudioStandard
} // namespace OHOS
