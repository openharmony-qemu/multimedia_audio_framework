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

#include "audio_utils.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_devices_client_manager.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioDevicesClientManager &AudioDevicesClientManager::GetInstance()
{
    static AudioDevicesClientManager instance;
    return instance;
}

int32_t AudioDevicesClientManager::SelectOutputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() == 1 && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_PARAM, "invalid parameter");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an output device.");
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_OPERATION_FAILED, "create renderer filter failed");
    audioRendererFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
    return ret;
}

int32_t AudioDevicesClientManager::SelectInputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() == 1 && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_PARAM, "invalid parameter");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::INPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an input device.");
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr, ERR_OPERATION_FAILED, "create capturer filter failed");
    audioCapturerFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    return ret;
}

std::string AudioDevicesClientManager::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const
{
    return AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioDevicesClientManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    // basic check
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr && audioDeviceDescriptors.size() != 0,
        ERR_INVALID_PARAM, "invalid parameter");

    size_t validDeviceSize = 1;
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() <= validDeviceSize && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_OPERATION, "device error.");
    // operation chack
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::INPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an input device");
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter->uid >= 0 || (audioCapturerFilter->uid == -1),
        ERR_INVALID_PARAM, "invalid uid.");
    AUDIO_DEBUG_LOG("[%{public}d] SelectInputDevice: uid<%{public}d> device<type:%{public}d>",
        getpid(), audioCapturerFilter->uid, static_cast<int32_t>(audioDeviceDescriptors[0]->deviceType_));

    return AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

// LCOV_EXCL_START
int32_t AudioDevicesClientManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES ||
        audioDevUsage == ALL_MEDIA_DEVICES || audioDevUsage == ALL_CALL_DEVICES, ERR_INVALID_PARAM,
        "invalid parameter: only support output device");
    CHECK_AND_RETURN_RET_LOG(!audioDeviceDescriptors.empty(), ERR_INVALID_PARAM, "invalid parameter: empty list");
    for (const auto &devDesc : audioDeviceDescriptors) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter: mull pointer in list");
        CHECK_AND_RETURN_RET_LOG(!(devDesc->deviceType_ == DEVICE_TYPE_SPEAKER &&
            devDesc->networkId_ == LOCAL_NETWORK_ID),
            ERR_INVALID_PARAM, "invalid parameter: speaker can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->deviceType_ != DEVICE_TYPE_EARPIECE, ERR_INVALID_PARAM,
            "invalid parameter: earpiece can not be excluded.");
    }
    return AudioPolicyManager::GetInstance().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioDevicesClientManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES ||
        audioDevUsage == ALL_MEDIA_DEVICES || audioDevUsage == ALL_CALL_DEVICES, ERR_INVALID_PARAM,
        "invalid parameter: only support output device");
    CHECK_AND_RETURN_RET_LOG(!audioDeviceDescriptors.empty(), ERR_INVALID_PARAM, "invalid parameter: empty list");
    for (const auto &devDesc : audioDeviceDescriptors) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter: mull pointer in list");
        CHECK_AND_RETURN_RET_LOG(!(devDesc->deviceType_ == DEVICE_TYPE_SPEAKER &&
            devDesc->networkId_ == LOCAL_NETWORK_ID),
            ERR_INVALID_PARAM, "invalid parameter: speaker can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->deviceType_ != DEVICE_TYPE_EARPIECE, ERR_INVALID_PARAM,
            "invalid parameter: earpiece can not be excluded.");
    }
    return AudioPolicyManager::GetInstance().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioDevicesClientManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage) const
{
    CHECK_AND_RETURN_RET_LOG(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES ||
        audioDevUsage == ALL_MEDIA_DEVICES || audioDevUsage == ALL_CALL_DEVICES, ERR_INVALID_PARAM,
        "invalid parameter: only support output device");
    auto unexcludeOutputDevices = GetExcludedDevices(audioDevUsage);
    if (unexcludeOutputDevices.empty()) {
        return SUCCESS;
    }
    for (const auto &devDesc : unexcludeOutputDevices) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter: mull pointer in list");
        CHECK_AND_RETURN_RET_LOG(!(devDesc->deviceType_ == DEVICE_TYPE_SPEAKER &&
            devDesc->networkId_ == LOCAL_NETWORK_ID),
            ERR_INVALID_PARAM, "invalid parameter: speaker can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->deviceType_ != DEVICE_TYPE_EARPIECE, ERR_INVALID_PARAM,
            "invalid parameter: earpiece can not be excluded.");
    }
    return AudioPolicyManager::GetInstance().UnexcludeOutputDevices(audioDevUsage, unexcludeOutputDevices);
}
// LCOV_EXCL_STOP

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDevicesClientManager::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage) const
{
    return AudioPolicyManager::GetInstance().GetExcludedDevices(audioDevUsage);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDevicesClientManager::GetDevices(DeviceFlag deviceFlag)
{
    return AudioPolicyManager::GetInstance().GetDevices(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDevicesClientManager::GetDevicesInner(DeviceFlag deviceFlag)
{
    return AudioPolicyManager::GetInstance().GetDevicesInner(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDevicesClientManager::GetActiveOutputDeviceDescriptors()
{
    AudioRendererInfo rendererInfo;
    return AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
}

int32_t AudioDevicesClientManager::GetPreferredInputDeviceDescriptors()
{
    AudioCapturerInfo capturerInfo;
    auto dec = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    CHECK_AND_RETURN_RET(dec.size() > 0, ERROR_INVALID_PARAM);
    return SUCCESS;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDevicesClientManager::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    return AudioPolicyManager::GetInstance().GetOutputDevice(audioRendererFilter);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDevicesClientManager::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    return AudioPolicyManager::GetInstance().GetInputDevice(audioCapturerFilter);
}

int32_t AudioDevicesClientManager::SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientUid) const
{
    int32_t uid = clientUid == -1 ? getuid() : clientUid;
    if (!IsActiveDeviceType(deviceType)) {
        AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
        return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetDeviceActive */
    return (AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag, uid));
}

bool AudioDevicesClientManager::IsDeviceActive(DeviceType deviceType) const
{
    if (!IsActiveDeviceType(deviceType)) {
        AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
        return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy IsDeviceActive */
    return (AudioPolicyManager::GetInstance().IsDeviceActive(static_cast<InternalDeviceType>(deviceType)));
}

DeviceType AudioDevicesClientManager::GetActiveOutputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveOutputDevice();
}

DeviceType AudioDevicesClientManager::GetActiveInputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveInputDevice();
}

int32_t AudioDevicesClientManager::SetDeviceChangeCallback(const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
}

int32_t AudioDevicesClientManager::UnsetDeviceChangeCallback(DeviceFlag flag,
    std::shared_ptr<AudioManagerDeviceChangeCallback> cb)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().UnsetDeviceChangeCallback(clientId, flag, cb);
}
} // namespace AudioStandard
} // namespace OHOS
