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
#ifdef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
#include "ipc_skeleton.h"
#include "parameters.h"
#endif

namespace OHOS {
namespace AudioStandard {
#ifdef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
constexpr char GET_AUDIO_ADAPTER_INFO[] = "ohos.permission.GET_AUDIO_ADAPTER_INFO";
constexpr char ONBOARD_SOUND_CARD_PATH[] = "/proc/asound/card0";
constexpr char SOUND_CARD_INFO_KEY[] = "sound_card_info";
#endif
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
    std::shared_ptr<AudioDeviceDescriptor> descriptor = desc;
    if (desc == nullptr) {
        descriptor = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    } else {
        CHECK_AND_RETURN_RET_LOG(IsOutputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
            "invalid output device type");
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { descriptor };
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_OPERATION_FAILED, "create renderer filter failed");
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().IsEnhancedRoutingSupported(), SUCCESS, "not support");
    int32_t uid = static_cast<int32_t>(getuid());
    audioRendererFilter->uid = uid;
    return AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors,
        SELECT_STRATEGY_INDEPENDENT);
}

int32_t AudioDeviceEnhanceManager::SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::shared_ptr<AudioDeviceDescriptor> descriptor = desc;
    if (desc == nullptr) {
        descriptor = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NONE, DeviceRole::INPUT_DEVICE);
    } else {
        CHECK_AND_RETURN_RET_LOG(IsInputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
            "invalid input device type");
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { descriptor };
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr, ERR_OPERATION_FAILED, "create capturer filter failed");
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().IsEnhancedRoutingSupported(), SUCCESS, "not support");
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
    std::shared_ptr<AudioDeviceDescriptor> descriptor = desc;
    if (desc == nullptr) {
        descriptor = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    } else {
        CHECK_AND_RETURN_RET_LOG(IsOutputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
            "invalid output device type");
    }
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().IsEnhancedRoutingSupported(), SUCCESS, "not support");
    return renderer->SelectOutputDevice(descriptor);
}

int32_t AudioDeviceEnhanceManager::SelectInputDeviceForAudioCapturer(
    std::shared_ptr<AudioCapturer> &capturer, const std::shared_ptr<AudioDeviceDescriptor> &desc) const
{
    CHECK_AND_RETURN_RET_LOG(capturer != nullptr, ERR_INVALID_PARAM, "capturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(capturer->GetStatus() != CAPTURER_RELEASED, ERR_INVALID_PARAM, "capturer is released");
    std::shared_ptr<AudioDeviceDescriptor> descriptor = desc;
    if (desc == nullptr) {
        descriptor = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NONE, DeviceRole::INPUT_DEVICE);
    } else {
        CHECK_AND_RETURN_RET_LOG(IsInputDevice(desc->deviceType_, desc->deviceRole_), ERR_INVALID_PARAM,
            "invalid input device type");
    }
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().IsEnhancedRoutingSupported(), SUCCESS, "not support");
    return capturer->SelectInputDevice(descriptor);
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

int32_t AudioDeviceEnhanceManager::GetSoundCardInfo(SoundCardInfo &soundCardInfo) const
{
#ifndef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
    AUDIO_ERR_LOG("Sound card info feature is not supported on this device.");
    return ERR_NOT_SUPPORTED;
#else
    if (!OHOS::AudioStandard::PermissionUtil::VerifyIsSystemApp()) {
        AUDIO_ERR_LOG("Permission denied: Caller is not a system app.");
        return ERR_PERMISSION_DENIED;
    }
    return GetOnboardSoundCardInfo(soundCardInfo);
#endif
}

#ifdef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
int32_t AudioDeviceEnhanceManager::GetOnboardSoundCardInfo(SoundCardInfo &onboardInfo) const
{
    std::string key = SOUND_CARD_INFO_KEY;
    std::string value = "";

    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("Audio service proxy unavailable.");
        return ERROR;
    }
    int32_t ret = gasp->GetAudioParameter(key, value);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetAudioParameter failed, ret: %{public}d", ret);
        return ERROR;
    }
    onboardInfo = ParseSoundCardInfo(value);
    if (!onboardInfo.IsEmpty()) {
        onboardInfo.cardId = BuildOnboardCardId(onboardInfo.vendor, ONBOARD_SOUND_CARD_PATH);
        onboardInfo.cardType = SoundCardType::ONBOARD;
    }
    return SUCCESS;
}

std::string AudioDeviceEnhanceManager::BuildOnboardCardId(const std::string &vendor, const std::string &devPath) const
{
    // Card ID format: ONBOARD\VEN_<vendor>&<platform>&<devPath>, see the design doc cardId format table.
    std::string platform = system::GetParameter("ohos.boot.hardware", "unknown");
    return "ONBOARD\\VEN_" + vendor + "&" + platform + "&" + devPath;
}

#ifdef USB_ENABLE
int32_t AudioDeviceEnhanceManager::GetUsbSoundCardInfo(std::vector<SoundCardInfo> &soundCardList) const
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("Audio service proxy unavailable.");
        return ERROR;
    }
    int32_t ret = ERROR;
    gasp->GetUsbSoundCardInfoList(soundCardList, ret);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetUsbSoundCardInfoList failed, ret: %{public}d", ret);
        return ERROR;
    }
    return SUCCESS;
}
#endif // USB_ENABLE
#endif // AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO

int32_t AudioDeviceEnhanceManager::GetSoundCardInfoList(std::vector<SoundCardInfo> &soundCardList) const
{
#ifndef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
    AUDIO_ERR_LOG("Sound card info feature is not supported on this device.");
    return ERR_NOT_SUPPORTED;  // return error code 801
#else
    if (!PermissionUtil::VerifyPermission(GET_AUDIO_ADAPTER_INFO, IPCSkeleton::GetCallingTokenID())) {
        AUDIO_ERR_LOG("Permission denied: Caller does not have GET_AUDIO_ADAPTER_INFO permission.");
        return ERR_NO_PERMISSION;
    }

    SoundCardInfo onboardInfo;
    int32_t onboardRet = GetOnboardSoundCardInfo(onboardInfo);
    if (onboardRet != SUCCESS) {
        AUDIO_ERR_LOG("Get onboard sound card info failed, onboardRet: %{public}d", onboardRet);
        return onboardRet;
    }
    if (!onboardInfo.IsEmpty()) {
        soundCardList.push_back(onboardInfo);
        AUDIO_INFO_LOG("found onboard sound card, cardId=%{public}s", onboardInfo.cardId.c_str());
    }

#ifdef USB_ENABLE
    int32_t usbRet = GetUsbSoundCardInfo(soundCardList);
    if (usbRet != SUCCESS) {
        // Keep the onboard sound card info already collected and return SUCCESS.
        // A USB service failure should not fail the whole enumeration.
        AUDIO_ERR_LOG("Get USB sound card info failed, usbRet: %{public}d", usbRet);
    }
#endif // USB_ENABLE
    AUDIO_INFO_LOG("total sound cards found: %{public}zu", soundCardList.size());
    return SUCCESS;
#endif // AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
}
} // namespace AudioStandard
} // namespace OHOS
