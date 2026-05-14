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
#include "audio_capturer.h"
#include "audio_policy_manager.h"
#include "audio_renderer.h"
#include "audio_device_info.h"
#include "audio_service_proxy.h"

namespace OHOS {
namespace AudioStandard {
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
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "device descriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(IsOutputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
        "invalid output device type");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { desc };
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_OPERATION_FAILED, "create renderer filter failed");
    int32_t uid = static_cast<int32_t>(getuid());
    audioRendererFilter->uid = uid;
    
    return AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors,
        SELECT_STRATEGY_INDEPENDENT);
}

int32_t AudioDeviceEnhanceManager::SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "device descriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(IsInputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
        "invalid input device type");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { desc };
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr, ERR_OPERATION_FAILED, "create capturer filter failed");
    int32_t uid = static_cast<int32_t>(getuid());
    audioCapturerFilter->uid = uid;
    audioCapturerFilter->audioDeviceSelectMode = SELECT_STRATEGY_INDEPENDENT;
    return AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

int32_t AudioDeviceEnhanceManager::SelectOutputDeviceForAudioRenderer(
    std::shared_ptr<AudioRenderer> &renderer, const std::shared_ptr<AudioDeviceDescriptor> &desc) const
{
    CHECK_AND_RETURN_RET_LOG(renderer != nullptr, ERR_INVALID_PARAM, "renderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(renderer->GetStatus() != RENDERER_RELEASED, ERR_INVALID_PARAM, "renderer is released");
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "device descriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(IsOutputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
        "invalid output device type");
    return renderer->SelectOutputDevice(desc);
}

int32_t AudioDeviceEnhanceManager::SelectInputDeviceForAudioCapturer(
    std::shared_ptr<AudioCapturer> &capturer, const std::shared_ptr<AudioDeviceDescriptor> &desc) const
{
    CHECK_AND_RETURN_RET_LOG(capturer != nullptr, ERR_INVALID_PARAM, "capturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(capturer->GetStatus() != CAPTURER_RELEASED, ERR_INVALID_PARAM, "capturer is released");
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "device descriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(IsInputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
        "invalid input device type");
    return capturer->SelectInputDevice(desc);
}

SoundCardInfo ParseSoundCardInfo(const std::string &halResponse)
{
    SoundCardInfo info;
    if (halResponse.empty()) {
        AUDIO_INFO_LOG("Sound card info response is empty");
        return info;
    }

    std::istringstream stream(halResponse);
    std::string line;
    std::string key;
    std::string value;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        size_t pos = line.find(":");
        if (pos == std::string::npos) {
            continue;
        }

        key = line.substr(0, pos);
        value = line.substr(pos + 1);
        size_t first = value.find_first_not_of(" \t\r\n");
        size_t last = value.find_last_not_of(" \t\r\n");
        if (first != std::string::npos) {
            value = value.substr(first, (last - first + 1));
        } else {
            value = "";
        }

        if (key == "Name") {
            info.name = value;
        } else if (key == "Vendor") {
            info.vendor = value;
        } else if (key == "Model") {
            info.model = value;
        } else if (key == "BusAddress") {
            info.busAddress = value;
        } else if (key == "Driver") {
            info.driver = value;
        }
    }
    return info;
}

SoundCardInfo AudioDeviceEnhanceManager::GetSoundCardInfo() const
{
    std::string key = "sound_card_info";
    std::string value = "";

    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("Audio service proxy unavailable.");
        return SoundCardInfo();
    }
    int32_t ret = gasp->GetAudioParameter(key, value);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetAudioParameter failed, ret: %{public}d", ret);
        return SoundCardInfo();
    }
    return ParseSoundCardInfo(value);
}
} // namespace AudioStandard
} // namespace OHOS
