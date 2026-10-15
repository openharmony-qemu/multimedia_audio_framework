/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyManager"
#endif

#include "audio_policy_manager.h"
#include "audio_errors.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "iservice_registry.h"
#include "va_device_broker_wrapper_impl.h"
#include "system_ability_definition.h"
#include "sle_audio_operation_callback_stub_impl.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const unsigned int TIME_OUT_SECONDS = 10;

int32_t AudioPolicyManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors, const int32_t audioDeviceSelectMode)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);

    int32_t validSize = 20; // Use 20 as limit.
    int32_t size = static_cast<int32_t>(audioDeviceDescriptors.size());
    if (size <= 0 || size > validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice get invalid device size.");
        return -1;
    }

    return gsp->SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors, audioDeviceSelectMode);
}

bool AudioPolicyManager::IsEnhancedRoutingSupported()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, false);

    bool supported = false;
    gsp->IsEnhancedRoutingSupported(supported);
    return supported;
}

int32_t AudioPolicyManager::SelectPrivateDevice(DeviceType devType, const std::string &macAddress)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "No system permission");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->SelectPrivateDevice(devType, macAddress);
}

int32_t AudioPolicyManager::ForceSelectDevice(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "No system permission");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->ForceSelectDevice(devType, macAddress, filter);
}

int32_t AudioPolicyManager::SetActiveHfpDevice(const std::string &macAddress)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->SetActiveHfpDevice(macAddress);
}


int32_t AudioPolicyManager::RestoreOutputDevice(sptr<AudioRendererFilter> audioRendererFilter)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "No system permission");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);

    return gsp->RestoreOutputDevice(audioRendererFilter);
}

int32_t AudioPolicyManager::SetMediaOutputDevice(DeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    CHECK_AND_RETURN_RET_LOG(deviceType == DEVICE_TYPE_SPEAKER || deviceType == DEVICE_TYPE_DEFAULT,
        ERROR, "invaild deviceType.");
    int32_t result = CheckAudioPolicyClientRegisted();
    CHECK_AND_RETURN_RET(result == SUCCESS, result);
    return gsp->SetMediaOutputDevice(deviceType);
}

int32_t AudioPolicyManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);

    int32_t validSize = 10; // Use 10 as limit.
    int32_t size = static_cast<int32_t>(audioDeviceDescriptors.size());
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= validSize, -1, "SelectInputDevice get invalid device size.");
    return gsp->SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

int32_t AudioPolicyManager::SelectInputDevice(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptor)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor != nullptr, -1,
        "SelectInputDevice get null device.");
    int32_t result = CheckAudioPolicyClientRegisted();
    CHECK_AND_RETURN_RET(result == SUCCESS, result);
    return gsp->SelectInputDevice(audioDeviceDescriptor);
}

int32_t AudioPolicyManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);

    int32_t validSize = 20; // Use 20 as limit.
    int32_t size = static_cast<int32_t>(audioDeviceDescriptors.size());
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= validSize, -1, "ExcludeOutputDevices get invalid device size.");
    return gsp->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioPolicyManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);

    int32_t validSize = 20; // Use 20 as limit.
    int32_t size = static_cast<int32_t>(audioDeviceDescriptors.size());
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= validSize, -1, "UnexcludeOutputDevices get invalid device size.");
    return gsp->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetExcludedDevices: audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetExcludedDevices(audioDevUsage, deviceInfo);
    return deviceInfo;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetDevices(DeviceFlag deviceFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetDevices: audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetDevices(deviceFlag, deviceInfo);
    return deviceInfo;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetDevicesInner(DeviceFlag deviceFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetDevicesInner(deviceFlag, deviceInfo);
    return deviceInfo;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, bool forceNoBTPermission)
{
    AudioXCollie audioXCollie("AudioPolicyManager::GetPreferredOutputDeviceDescriptors", TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetPreferredOutputDeviceDescriptors: audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetPreferredOutputDeviceDescriptors(rendererInfo, forceNoBTPermission, deviceInfo);
    return deviceInfo;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetPreferredInputDeviceDescriptors(captureInfo, deviceInfo);
    return deviceInfo;
}

int32_t AudioPolicyManager::SetDeclaredDeviceTypes(const std::vector<DeviceType> &deviceTypes)
{
    std::set<int32_t> intDeviceTypeSet;
    for (const auto &deviceType : deviceTypes) {
        CHECK_AND_RETURN_RET_LOG(deviceType > DEVICE_TYPE_NONE && deviceType < DEVICE_TYPE_MAX, -1,
            "device type is invalid");
        intDeviceTypeSet.insert(static_cast<int32_t>(deviceType));
    }
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->SetDeclaredDeviceTypes(intDeviceTypeSet);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetOutputDevice(audioRendererFilter, deviceInfo);
    return deviceInfo;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return deviceInfo;
    }
    gsp->GetInputDevice(audioCapturerFilter, deviceInfo);
    return deviceInfo;
}

int32_t AudioPolicyManager::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    int32_t uidIn = uid;
    return gsp->SetDeviceActive(deviceType, active, uidIn);
}

bool AudioPolicyManager::IsDeviceActive(InternalDeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, false);
    bool active = false;
    gsp->IsDeviceActive(deviceType, active);
    return active;
}

DeviceType AudioPolicyManager::GetActiveOutputDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, DEVICE_TYPE_INVALID);
    int32_t out = DEVICE_TYPE_INVALID;
    gsp->GetActiveOutputDevice(out);
    return static_cast<DeviceType>(out);
}

uint16_t AudioPolicyManager::GetDmDeviceType()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, DEVICE_TYPE_INVALID);
    uint16_t out = DEVICE_TYPE_INVALID;
    gsp->GetDmDeviceType(out);
    return out;
}

DeviceType AudioPolicyManager::GetActiveInputDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, DEVICE_TYPE_INVALID);
    int32_t out = DEVICE_TYPE_INVALID;
    gsp->GetActiveInputDevice(out);
    return static_cast<DeviceType>(out);
}

// LCOV_EXCL_START
int32_t AudioPolicyManager::SetDeviceChangeCallback(const int32_t clientId, const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetDeviceChangeCallback");
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (flag) {
        case NONE_DEVICES_FLAG:
        case DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DISTRIBUTED_INPUT_DEVICES_FLAG:
        case ALL_DISTRIBUTED_DEVICES_FLAG:
        case ALL_L_D_DEVICES_FLAG:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("SetDeviceChangeCallback: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            break;
    }

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "SetDeviceChangeCallback: callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET(gsp != nullptr, -1);
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddDeviceChangeCallback(flag, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetDeviceChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_DEVICE_CHANGE, true);
        }
    }
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyManager::UnsetDeviceChangeCallback(const int32_t clientId, DeviceFlag flag,
    std::shared_ptr<AudioManagerDeviceChangeCallback> &cb)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetDeviceChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveDeviceChangeCallback(flag, cb);
        if (audioPolicyClientStubCB_->GetDeviceChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_DEVICE_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetDeviceInfoUpdateCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetDeviceInfoUpdateCallback");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetDeviceInfoUpdateCallback: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetDeviceInfoUpdateCallback: callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET(gsp != nullptr, -1);
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_DEVICE_INFO_UPDATE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddDeviceInfoUpdateCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetDeviceInfoUpdateCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_DEVICE_INFO_UPDATE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_DEVICE_INFO_UPDATE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetDeviceInfoUpdateCallback(const int32_t clientId,
    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> &cb)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetDeviceInfoUpdateCallback");
    int32_t ret = SUCCESS;
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_DEVICE_INFO_UPDATE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveDeviceInfoUpdateCallback(cb);
        if (audioPolicyClientStubCB_->GetDeviceInfoUpdateCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_DEVICE_INFO_UPDATE].isEnable = false;
            ret = SetClientCallbacksEnable(CALLBACK_SET_DEVICE_INFO_UPDATE, false);
        }
    }
    return ret;
}

int32_t AudioPolicyManager::SetPreferredOutputDeviceChangeCallback(const AudioRendererInfo &rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback, const int32_t uid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetPreferredOutputDeviceChangeCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET(gsp != nullptr, -1);
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    int32_t targetUid = (uid == INVALID_UID) ? static_cast<int32_t>(getuid()) : uid;
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddPreferredOutputDeviceChangeCallback(rendererInfo, targetUid, callback);
        // Check if (rendererInfo, uid) pair already exists before adding
        auto it = std::find_if(rendererInfosWithUid_.begin(), rendererInfosWithUid_.end(),
            [&rendererInfo, targetUid](const std::pair<AudioRendererInfo, int32_t> &item) {
                return item.first.streamUsage == rendererInfo.streamUsage && item.second == targetUid;
            });
        if (it == rendererInfosWithUid_.end()) {
            rendererInfosWithUid_.push_back({rendererInfo, targetUid});
        }
        SetCallbackRendererInfo(rendererInfo, targetUid);
        size_t callbackSize = audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetPreferredInputDeviceChangeCallback(const AudioCapturerInfo &capturerInfo,
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback, const int32_t uid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetPreferredInputDeviceChangeCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET(gsp != nullptr, -1);
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    int32_t targetUid = (uid == INVALID_UID) ? static_cast<int32_t>(getuid()) : uid;
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddPreferredInputDeviceChangeCallback(capturerInfo, targetUid, callback);
        // Check if (capturerInfo, targetUid) pair already exists before adding
        auto it = std::find_if(capturerInfosWithUid_.begin(), capturerInfosWithUid_.end(),
            [&capturerInfo, targetUid](const std::pair<AudioCapturerInfo, int32_t> &item) {
                return item.first.sourceType == capturerInfo.sourceType && item.second == targetUid;
            });
        if (it == capturerInfosWithUid_.end()) {
            capturerInfosWithUid_.push_back({capturerInfo, targetUid});
        }
        SetCallbackCapturerInfo(capturerInfo, targetUid);
        size_t callbackSize = audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetPreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback,
    const AudioRendererInfo &rendererInfo,
    const int32_t uid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetPreferredOutputDeviceChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        int32_t targetUid = (uid == INVALID_UID) ? static_cast<int32_t>(getuid()) : uid;
        audioPolicyClientStubCB_->RemovePreferredOutputDeviceChangeCallback(targetUid, rendererInfo.streamUsage,
            callback);
        if (audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, false, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback,
    const AudioCapturerInfo &capturerInfo,
    const int32_t uid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetPreferredInputDeviceChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        int32_t targetUid = (uid == INVALID_UID) ? static_cast<int32_t>(getuid()) : uid;
        audioPolicyClientStubCB_->RemovePreferredInputDeviceChangeCallback(targetUid, capturerInfo.sourceType,
            callback);
        if (audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterDeviceChangeWithInfoCallback(
    const uint32_t sessionID, const std::weak_ptr<DeviceChangeWithInfoCallback> &callback)
{
    AUDIO_DEBUG_LOG("In");

    if (callback.expired()) {
        AUDIO_ERR_LOG("callback is expired");
        return ERR_INVALID_PARAM;
    }

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddDeviceChangeWithInfoCallback(sessionID, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetDeviceChangeWithInfoCallbackkSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_DEVICE_CHANGE_WITH_INFO, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterDeviceChangeWithInfoCallback(const uint32_t sessionID)
{
    AUDIO_DEBUG_LOG("In");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveDeviceChangeWithInfoCallback(sessionID);
        if (audioPolicyClientStubCB_->GetDeviceChangeWithInfoCallbackkSize() == 0) {
            callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_DEVICE_CHANGE_WITH_INFO, false);
        }
    }
    return SUCCESS;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> descs;
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetAvailableMicrophones: audio policy manager proxy is NULL.");
        return descs;
    }
    gsp->GetAvailableDevices(usage, descs);
    return descs;
}

std::shared_ptr<AudioDeviceDescriptor> AudioPolicyManager::GetSelectedInputDevice()
{
    std::shared_ptr<AudioDeviceDescriptor> descriptor =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, descriptor);

    gsp->GetSelectedInputDevice(descriptor);
    return descriptor;
}

int32_t AudioPolicyManager::ClearSelectedInputDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->ClearSelectedInputDevice();
}

int32_t AudioPolicyManager::PreferBluetoothAndNearlinkRecord(BluetoothAndNearlinkPreferredRecordCategory category)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    int32_t result = CheckAudioPolicyClientRegisted();
    CHECK_AND_RETURN_RET(result == SUCCESS, result);
    return gsp->PreferBluetoothAndNearlinkRecord(category);
}

BluetoothAndNearlinkPreferredRecordCategory AudioPolicyManager::GetPreferBluetoothAndNearlinkRecord()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, PREFERRED_NONE);
    uint32_t category = PREFERRED_NONE;
    gsp->GetPreferBluetoothAndNearlinkRecord(category);
    return static_cast<BluetoothAndNearlinkPreferredRecordCategory>(category);
}

int32_t AudioPolicyManager::SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    }

    auto deviceChangeCbStub = new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    CHECK_AND_RETURN_RET_LOG(deviceChangeCbStub != nullptr, ERROR, "object null");

    deviceChangeCbStub->SetAvailableDeviceChangeCallback(callback);

    sptr<IRemoteObject> object = deviceChangeCbStub->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub->AsObject is nullptr..");
        delete deviceChangeCbStub;
        return ERROR;
    }

    {
        std::lock_guard<std::mutex> lock(handleAvailableDeviceChangeCbsMapMutex_);
        availableDeviceChangeCbsMap_[{clientId, usage}] = object;
    }

    return gsp->SetAvailableDeviceChangeCallback(clientId, usage, object);
}

int32_t AudioPolicyManager::UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);

    {
        std::lock_guard<std::mutex> lock(handleAvailableDeviceChangeCbsMapMutex_);
        availableDeviceChangeCbsMap_.erase({clientId, usage});

        if (usage == AudioDeviceUsage::D_ALL_DEVICES) {
            for (auto it = availableDeviceChangeCbsMap_.begin(); it != availableDeviceChangeCbsMap_.end();) {
                if ((*it).first.first == clientId) {
                    it = availableDeviceChangeCbsMap_.erase(it);
                } else {
                    it++;
                }
            }
        }
    }

    return gsp->UnsetAvailableDeviceChangeCallback(clientId, static_cast<int32_t>(usage));
}

int32_t AudioPolicyManager::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return -1;
    }
    return gsp->SetCallDeviceActive(static_cast<int32_t>(deviceType), active, address, uid);
}

std::shared_ptr<AudioDeviceDescriptor> AudioPolicyManager::GetActiveBluetoothDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return make_shared<AudioDeviceDescriptor>();
    }
    std::shared_ptr<AudioDeviceDescriptor> descs = make_shared<AudioDeviceDescriptor>();
    gsp->GetActiveBluetoothDevice(descs);
    return descs;
}

int32_t AudioPolicyManager::SkipDeviceTypes(bool skipped, const std::vector<DeviceType> &deviceTypes)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    sptr<IRemoteObject> object = GetAudioPolicyClientStubObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "audio policy client stub object is NULL.");
    std::vector<int32_t> deviceTypeVec;
    deviceTypeVec.reserve(deviceTypes.size());
    for (auto deviceType : deviceTypes) {
        deviceTypeVec.push_back(static_cast<int32_t>(deviceType));
    }

    return gsp->SkipDeviceTypes(skipped, deviceTypeVec, object);
}

int32_t AudioPolicyManager::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->TriggerFetchDevice(reason);
}

int32_t AudioPolicyManager::SetPreferredDevice(const PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->SetPreferredDevice(preferredType, desc, uid);
}

int32_t AudioPolicyManager::SetAudioDeviceAnahsCallback(const std::shared_ptr<AudioDeviceAnahs> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    if (callback == nullptr) {
        return ERR_INVALID_PARAM;
    };

    std::unique_lock<std::mutex> lock(listenerStubMutex_);
    auto activeDistributedAnahsRoleCb = new (std::nothrow) AudioAnahsManagerListener();
    if (activeDistributedAnahsRoleCb == nullptr) {
        AUDIO_ERR_LOG("object is nullptr");
        return ERROR;
    }
    activeDistributedAnahsRoleCb->SetAudioDeviceAnahsCallback(callback);
    sptr<IRemoteObject> object = activeDistributedAnahsRoleCb->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub is nullptr");
        delete activeDistributedAnahsRoleCb;
        return ERROR;
    }
    return gsp->SetAudioDeviceAnahsCallback(object);
}

int32_t AudioPolicyManager::UnsetAudioDeviceAnahsCallback()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->UnsetAudioDeviceAnahsCallback();
}

int32_t AudioPolicyManager::SetDeviceVolumeBehavior(const std::string &networkId,
    DeviceType deviceType, VolumeBehavior volumeBehavior)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

int32_t AudioPolicyManager::SetStopStreamInfoWhenDisconnect(const std::string &networkId,
    const std::vector<StreamUsage> &usages, const std::vector<int32_t> &uids)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    std::vector<int32_t> usagesNeedStop = {};
    for (const auto usage : usages) {
        usagesNeedStop.emplace_back(static_cast<int32_t>(usage));
    }
    return gsp->SetStopStreamInfoWhenDisconnect(networkId, usagesNeedStop, uids);
}

int32_t AudioPolicyManager::SetQueryDeviceVolumeBehaviorCallback(
    const std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    sptr<AudioPolicyManagerListenerStubImpl> listener = new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetQueryDeviceVolumeBehaviorCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr.");

    return gsp->SetQueryDeviceVolumeBehaviorCallback(object);
}

int32_t AudioPolicyManager::SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    const bool isConnected)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->SetDeviceConnectionStatus(desc, isConnected);
}

int32_t AudioPolicyManager::UpdateDeviceInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    const DeviceInfoUpdateCommand command)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->UpdateDeviceInfo(deviceDesc, command);
}

int32_t AudioPolicyManager::SetSleAudioOperationCallback(const std::shared_ptr<SleAudioOperationCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    std::unique_lock<std::mutex> lock(listenerStubMutex_);
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    CHECK_AND_RETURN_RET_LOG(audioSleCb != nullptr, ERROR, "object is nullptr");

    audioSleCb->SetSleAudioOperationCallback(callback);
    sptr<IRemoteObject> object = audioSleCb->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub is nullptr");
        delete audioSleCb;
        return ERROR;
    }

    return gsp->SetSleAudioOperationCallback(object);
}

int32_t AudioPolicyManager::RegisterPreferredDeviceSetCallback(
    const std::shared_ptr<PreferredDeviceSetCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(),
        ERR_PERMISSION_DENIED, "No system permission");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_DEVICE_SET].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddPreferredDeviceSetCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetPreferredDeviceSetCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_PREFERRED_DEVICE_SET].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_DEVICE_SET, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterPreferredDeviceSetCallback(
    const std::shared_ptr<PreferredDeviceSetCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_DEVICE_SET].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemovePreferredDeviceSetCallback(callback);
        if (audioPolicyClientStubCB_->GetPreferredDeviceSetCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_PREFERRED_DEVICE_SET].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_DEVICE_SET, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::GetVADeviceBroker(std::shared_ptr<VADeviceBrokerWrapper> &vaBroker)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is null");

    auto vaBrokerWrapper = std::make_shared<VADeviceBrokerWrapperImpl>();
    CHECK_AND_RETURN_RET_LOG(vaBrokerWrapper != nullptr, ERROR, "vaBrokerWrapper is null");
    vaBroker = vaBrokerWrapper;

    return SUCCESS;
}

int32_t AudioPolicyManager::SetVADeviceExceptionFlag(const std::string &macAddress, bool exceptionFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is null");
    gsp->SetVADeviceExceptionFlag(macAddress, exceptionFlag);
    return SUCCESS;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyManager::FetchOutputDevicesByFilter(
    StreamUsage usage, RouterType bypass)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> descs;
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, descs, "audio policy manager proxy is null");
    
    gsp->FetchOutputDevicesByFilter(usage, bypass, descs);
    return descs;
}

} // namespace AudioStandard
} // namespace OHOS
