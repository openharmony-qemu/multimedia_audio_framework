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
#define LOG_TAG "AppSelectRouter"
#endif

#include "app_select_router.h"

#include "audio_usr_select_manager.h"
#include "audio_policy_service.h"
#include "audio_bundle_manager.h"
#include "hisysevent.h"
#include "media_monitor_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetRendererDevice(clientUID);
    return device;
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetRendererDevice(clientUID);
    return device;
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> allDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();
    shared_ptr<AudioDeviceDescriptor> device =
        AudioDeviceManager::GetAudioDeviceManager().GetSelectedCaptureDevice(sessionID);
    CHECK_AND_RETURN_RET(device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE,
        RouterBase::GetPairDevice(device, allDevices));
    device = AudioAffinityManager::GetAudioAffinityManager().GetCapturerDevice(clientUID);
    return RouterBase::GetPairDevice(device, allDevices);
}

vector<std::shared_ptr<AudioDeviceDescriptor>> AppSelectRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> allDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();
    shared_ptr<AudioDeviceDescriptor> device =
        AudioDeviceManager::GetAudioDeviceManager().GetSelectedCaptureDevice(sessionID);
    CHECK_AND_RETURN_RET(device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE,
        RouterBase::GetPairDevice(device, allDevices));
    device = AudioDeviceManager::GetAudioDeviceManager().GetOnlinePreferredInputDevice(sessionID);
    if (IsInSpecialScenario(sourceType)) {
        ConfigureDeviceForSpecialScenario(sourceType, device);
    }
    CHECK_AND_RETURN_RET(device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE,
        RouterBase::GetPairDevice(device, allDevices));
    device = AudioUsrSelectManager::GetAudioUsrSelectManager().GetCapturerDevice(clientUID, sourceType);
    CHECK_AND_RETURN_RET(device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE,
        RouterBase::GetPairDevice(device, allDevices));
    device = AudioAffinityManager::GetAudioAffinityManager().GetCapturerDevice(clientUID);
    return RouterBase::GetPairDevice(device, allDevices);
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetRendererDevice(clientUID);
    return device;
}

bool AppSelectRouter::IsInSpecialScenario(SourceType sourceType)
{
    return sourceType == SOURCE_TYPE_VOICE_RECOGNITION && IsCurrentOutPutDevicePublic();
}

bool AppSelectRouter::IsCurrentOutPutDevicePublic()
{
    std::shared_ptr<AudioDeviceDescriptor> activeOutPutDevice =
        AudioPolicyService::GetAudioPolicyService().GetActiveOutputDeviceDescriptor();
    CHECK_AND_RETURN_RET_LOG(activeOutPutDevice != nullptr, false, "activeOutPutDevice is nullptr");
    AUDIO_INFO_LOG("activeOutPutDevice deviceType_: %{public}d", activeOutPutDevice->deviceType_);

    auto devicePrivacyType = AudioDeviceManager::GetAudioDeviceManager().GetDevicePrivacyType(activeOutPutDevice);
    AUDIO_INFO_LOG("current output device devicePrivacyType: %{public}d", devicePrivacyType);
    return devicePrivacyType == AudioDevicePrivacyType::TYPE_PUBLIC;
}

void AppSelectRouter::ConfigureDeviceForSpecialScenario(
    SourceType sourceType, shared_ptr<AudioDeviceDescriptor> &device)
{
    AUDIO_INFO_LOG("configure input device for special scenario");
    DeviceType beforeType = device != nullptr ? device->deviceType_ : DEVICE_TYPE_NONE;

    auto activeOutPutDevice = AudioPolicyService::GetAudioPolicyService().GetActiveOutputDeviceDescriptor();
    CHECK_AND_RETURN_LOG(activeOutPutDevice != nullptr, "activeOutPutDevice is nullptr");

    auto pairDevice = activeOutPutDevice->pairDeviceDescriptor_;
    auto scoDevice = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(
        activeOutPutDevice->macAddress_, DeviceRole::INPUT_DEVICE);

    if (pairDevice != nullptr) {
        AUDIO_INFO_LOG("find pairDevice deviceType_: %{public}d", pairDevice->deviceType_);
        device = pairDevice;
    } else if (scoDevice != nullptr) {
        AUDIO_INFO_LOG("find scoDevice deviceType_: %{public}d", scoDevice->deviceType_);
        device = scoDevice;
    }

    DeviceType afterType = device != nullptr ? device->deviceType_ : DEVICE_TYPE_NONE;
    WriteModifyCaptureDeviceSpecially(sourceType, beforeType, afterType);
}

void AppSelectRouter::WriteModifyCaptureDeviceSpecially(
    SourceType sourceType, DeviceType deviceBefore, DeviceType deviceAfter)
{
    CHECK_AND_RETURN_LOG(deviceBefore != deviceAfter, "device type unchanged");
    std::string appName = AudioBundleManager::GetBundleName();
    AUDIO_INFO_LOG("WriteModifyCaptureDeviceSpecially appName = %{public}s, sourceType = %{public}d, deviceBefore = "
        "%{public}d, deviceAfter = %{public}d", appName.c_str(), sourceType, deviceBefore, deviceAfter);

    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO,
        "MODIFY_CAPTURE_DEVICE_SPECIALLY", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "APP_NAME", appName.c_str(),
        "STREAM_TYPE", sourceType,
        "DEVICE_TYPE_BEFORE_CHANGE", deviceBefore,
        "DEVICE_TYPE_AFTER_CHANGE", deviceAfter);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "write event fail: MODIFY_CAPTURE_DEVICE_SPECIALLY, ret = %{public}d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
