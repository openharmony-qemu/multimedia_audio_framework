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
#ifndef LOG_TAG
#define LOG_TAG "AudioSelectInterfaceService"
#endif

#include "audio_select_interface_service.h"

#include "audio_active_device.h"
#include "audio_core_service.h"
#include "audio_device_factory.h"
#include "audio_errors.h"
#include "audio_policy_utils.h"
#include "audio_router_center.h"
#include "audio_router_select_strategy.h"
#include "audio_server_proxy.h"
#include "audio_bundle_manager.h"
#include "ipc_skeleton.h"
#include "sle_audio_device_manager.h"
#include "audio_affinity_manager.h"
#include "audio_stream_collector.h"
#include "audio_router_map.h"
#include "bluetooth_device_manager.h"
#include "audio_bluetooth_manager.h"
#include "audio_scene_manager.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioSelectInterfaceService::SetDeviceActive(DeviceType deviceType, bool active,
    const std::string &address, const int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    auto isPresent = [&deviceType, &address] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "SetDeviceActive::Invalid device descriptor");
        bool typeMatch = (deviceType == desc->deviceType_) || (address.empty() && deviceType == DEVICE_TYPE_FILE_SINK);
        bool addrMatch = address.empty() || (address == desc->macAddress_);
        return typeMatch && addrMatch;
    };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    auto itr = std::find_if(callDevices.begin(), callDevices.end(), isPresent);
    CHECK_AND_RETURN_RET_LOG(itr != callDevices.end(), ERR_OPERATION_FAILED,
        "Requested device not available %{public}d", deviceType);

    int32_t ownerUid = AudioSceneManager::GetInstance().GetAudioSceneOwnerUid();
    int32_t callerUid = GetPreferredUid(uid);
    if (!active) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), uid, "SetDeviceActive");
#ifdef BLUETOOTH_ENABLE
        CHECK_AND_RETURN_RET(callerUid == SYSTEM_UID || callerUid == ownerUid, SUCCESS);
        HandleNegtiveBt(deviceType);
#endif
    } else {
        if (!address.empty() && deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
            (*itr)->isEnable_ = true;
            AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
            AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(
                std::make_shared<AudioDeviceDescriptor>(**itr), ENABLE_UPDATE, reason);
            AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(address);
        }
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER, *itr, uid, "SetDeviceActive");
#ifdef BLUETOOTH_ENABLE
        CHECK_AND_RETURN_RET(callerUid == SYSTEM_UID || callerUid == ownerUid, SUCCESS);
        HandleActiveBt(deviceType, (*itr)->macAddress_);
#endif
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetAudioClientInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback)
{
    audioClientInfoMgrCallback_ = callback;
    return 0;
}

int32_t AudioSelectInterfaceService::GetPreferredUid(int32_t uid)
{
    int32_t callerUid = uid;
    if (audioClientInfoMgrCallback_ != nullptr) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        std::string bundleName = AudioBundleManager::GetBundleNameFromUid(callerUid);
        bool ret = false;
        audioClientInfoMgrCallback_->OnCheckClientInfo(bundleName, callerUid, callerPid, ret);
    }
    return callerUid;
}

void AudioSelectInterfaceService::HandleNegtiveBt(DeviceType deviceType)
{
    auto currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    if (currentOutputDevice.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::SendUserSelectionEvent(DEVICE_TYPE_BLUETOOTH_SCO,
            currentOutputDevice.macAddress_, USER_NOT_SELECT_SLE);
        Bluetooth::AudioHfpManager::DisconnectSco();
    }
    if (currentOutputDevice.deviceType_ == DEVICE_TYPE_NEARLINK &&
        deviceType == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(currentOutputDevice,
            STREAM_USAGE_VOICE_COMMUNICATION, USER_NOT_SELECT_SLE);
    }
}

void AudioSelectInterfaceService::HandleActiveBt(DeviceType deviceType, std::string macAddress)
{
    auto currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    if (currentOutputDevice.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceType != DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::SendUserSelectionEvent(DEVICE_TYPE_BLUETOOTH_SCO,
            currentOutputDevice.macAddress_, USER_NOT_SELECT_SLE);
        Bluetooth::AudioHfpManager::DisconnectSco();
    }
    if (currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::SendUserSelectionEvent(DEVICE_TYPE_BLUETOOTH_SCO,
            macAddress, USER_SELECT_SLE);
    }
    if (currentOutputDevice.deviceType_ == DEVICE_TYPE_NEARLINK &&
        deviceType != DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(currentOutputDevice,
            STREAM_USAGE_VOICE_COMMUNICATION, USER_NOT_SELECT_SLE);
    }
    if (deviceType == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(currentOutputDevice,
            STREAM_USAGE_VOICE_COMMUNICATION, USER_SELECT_SLE);
    }
}

int32_t AudioSelectInterfaceService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode,
    const bool isNeedNotifyBt)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr && !selectedDesc.empty() && selectedDesc[0] != nullptr,
        ERROR, "ptr exception");
    SelectOutputDeviceLog(audioRendererFilter, selectedDesc, audioDeviceSelectMode);
    CHECK_AND_RETURN_RET_LOG(selectedDesc.size() == 1 && selectedDesc[0] &&
        selectedDesc[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE, ERR_INVALID_OPERATION, "DeviceCheck no success");

    int32_t res = SUCCESS;
    StreamUsage strUsage = audioRendererFilter->rendererInfo.streamUsage;
    auto audioDevUsage = AudioPolicyUtils::GetInstance().GetAudioDeviceUsageByStreamUsage(strUsage);
    vector<shared_ptr<AudioDeviceDescriptor>> unexcludeDevice = {
        make_shared<AudioDeviceDescriptor>(*selectedDesc[0])};
    if (AudioRouterSelectStrategy::GetInstance().IsDeviceExcluded(unexcludeDevice[0], audioDevUsage)) {
        res = UnexcludeOutputDevicesInner(audioDevUsage, unexcludeDevice);
        CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "UnexcludeOutputDevicesInner fail");
    }

    AudioActiveDevice::GetInstance().NotifyUserSelectionEventToRemote(selectedDesc[0]);

    bool isVirtualDevice = AudioDeviceManager::GetAudioDeviceManager().IsVirtualConnectedDevice(selectedDesc[0]);
    if (isVirtualDevice == true) {
        selectedDesc[0]->connectState_ = VIRTUAL_CONNECTED;
    }

    SetDeviceEnableAndUsage(selectedDesc[0]);

    RestoreSelectRendererDevice(selectedDesc[0], audioRendererFilter->uid);
    if (audioDeviceSelectMode == SELECT_STRATEGY_INDEPENDENT && audioRendererFilter->uid >= 0) {
        return SelectOutputDeviceByFilterInner(audioRendererFilter, selectedDesc);
    }
    if (audioRendererFilter->rendererInfo.rendererFlags == STREAM_FLAG_FAST) {
        return SelectOutputDeviceForFastInner(audioRendererFilter, selectedDesc);
    }

    if (selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(selectedDesc[0]->macAddress_);
    }

    res = SetRenderDeviceForUsage(strUsage, selectedDesc[0], audioRendererFilter->uid);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "SetRenderDeviceForUsage fail");

    // If the selected device is virtual device, connect it.
    if (isVirtualDevice) {
        int32_t ret = AudioRecoveryDevice::GetInstance().ConnectVirtualDevice(selectedDesc[0]);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Connect device [%{public}s] failed",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(selectedDesc[0]->macAddress_).c_str());
        return SUCCESS;
    }

    if (isNeedNotifyBt) {
        AudioActiveDevice::GetInstance().NotifyUserSelectionEventToBt(selectedDesc[0], strUsage);
    }
    HandleFetchDeviceChange(AudioStreamDeviceChangeReason::OVERRODE, "SelectOutputDevice");
    auto coreService = AudioCoreService::GetCoreService();
        CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->OnPreferredOutputDeviceUpdated(AudioActiveDevice::GetInstance().GetCurrentOutputDevice(),
        AudioStreamDeviceChangeReason::OVERRODE);
    return SUCCESS;
}

void AudioSelectInterfaceService::SelectOutputDeviceLog(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode)
{
    AUDIO_WARNING_LOG("[ADeviceEvent] uid[%{public}d] type[%{public}d] islocal [%{public}d] mac[%{public}s] " \
        "streamUsage[%{public}d] callerUid[%{public}d] audioDeviceSelectMode[%{public}d]", audioRendererFilter->uid,
        selectedDesc[0]->deviceType_, selectedDesc[0]->networkId_ == LOCAL_NETWORK_ID,
        AudioPolicyUtils::GetInstance().GetEncryptAddr(selectedDesc[0]->macAddress_).c_str(),
        audioRendererFilter->rendererInfo.streamUsage, IPCSkeleton::GetCallingUid(), audioDeviceSelectMode);
}

void AudioSelectInterfaceService::SetDeviceEnableAndUsage(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    deviceDesc->isEnable_ = true;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(deviceDesc, ENABLE_UPDATE, reason);
    deviceDesc->deviceUsage_ = ALL_USAGE;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(deviceDesc, USAGE_UPDATE, reason);
    deviceDesc->exceptionFlag_ = false;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(deviceDesc, EXCEPTION_FLAG_UPDATE, reason);
}

void AudioSelectInterfaceService::RestoreSelectRendererDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, const int32_t uid)
{
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
    if (deviceDesc->deviceType_ == DEVICE_TYPE_NONE && uid >= 0) {
        AudioAffinityManager::GetAudioAffinityManager().DelSelectRendererDevice(uid);
    }
}

int32_t AudioSelectInterfaceService::SelectOutputDeviceByFilterInner(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    AudioAffinityManager::GetAudioAffinityManager().AddSelectRendererDevice(audioRendererFilter->uid, selectedDesc[0]);
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    AudioStreamCollector::GetAudioStreamCollector().GetCurrentRendererChangeInfos(rendererChangeInfos);
    for (auto &changeInfo : rendererChangeInfos) {
        if (changeInfo->clientUID == audioRendererFilter->uid && changeInfo->sessionId != 0) {
            RestoreInfo restoreInfo;
            restoreInfo.restoreReason = STREAM_SPLIT;
            AudioServerProxy::GetInstance().RestoreSessionProxy(changeInfo->sessionId, restoreInfo);
        }
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectOutputDeviceForFastInner(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    int32_t res = SetRenderDeviceForUsage(audioRendererFilter->rendererInfo.streamUsage, selectedDesc[0]);
    CheckAndWriteDeviceChangeExceptionEvent(res == SUCCESS, AudioStreamDeviceChangeReason::OVERRODE,
        selectedDesc[0]->deviceType_, selectedDesc[0]->deviceRole_, res, "SetRenderDeviceForUsage fail");
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "SetRenderDeviceForUsage fail");
    SetRenderDeviceForUsage(audioRendererFilter->rendererInfo.streamUsage, selectedDesc[0]);
    res = SelectFastOutputDevice(audioRendererFilter, selectedDesc[0]);
    CheckAndWriteDeviceChangeExceptionEvent(res == SUCCESS, AudioStreamDeviceChangeReason::OVERRODE,
        selectedDesc[0]->deviceType_, selectedDesc[0]->deviceRole_, res, "AddFastRouteMapInfo failed");
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res,
        "AddFastRouteMapInfo failed! fastRouteMap is too large!");
    auto coreService = AudioCoreService::GetCoreService();
        CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchOutputDeviceAndRoute("SelectOutputDeviceForFastInner",
        AudioStreamDeviceChangeReason::OVERRODE);
    return true;
}

int32_t AudioSelectInterfaceService::SetRenderDeviceForUsage(StreamUsage streamUsage,
    std::shared_ptr<AudioDeviceDescriptor> desc, const int32_t uid)
{
    // get deviceUsage and preferredType
    auto deviceUsage = AudioPolicyUtils::GetInstance().GetAudioDeviceUsageByStreamUsage(streamUsage);
    auto preferredType = AudioPolicyUtils::GetInstance().GetPreferredTypeByStreamUsage(streamUsage);
    auto tempId = desc->deviceId_;

    // find device
    auto devices = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(deviceUsage);
    auto itr = std::find_if(devices.begin(), devices.end(), [&desc](const auto &device) {
        return (desc->deviceType_ == device->deviceType_) &&
            (desc->macAddress_ == device->macAddress_) &&
            (desc->networkId_ == device->networkId_) &&
            (!IsUsb(desc->deviceType_) || desc->deviceRole_ == device->deviceRole_);
    });
    CHECK_AND_RETURN_RET_LOG(itr != devices.end() || desc->deviceType_ == DEVICE_TYPE_NONE, ERR_INVALID_OPERATION,
        "device not available type:%{public}d macAddress:%{public}s id:%{public}d networkId:%{public}s",
        desc->deviceType_, AudioPolicyUtils::GetInstance().GetEncryptAddr(desc->macAddress_).c_str(),
        tempId, GetEncryptStr(desc->networkId_).c_str());
    // set preferred device
    std::shared_ptr<AudioDeviceDescriptor> descriptor = (desc->deviceType_ == DEVICE_TYPE_NONE ?
        desc : std::make_shared<AudioDeviceDescriptor>(**itr));
    CHECK_AND_RETURN_RET_LOG(descriptor != nullptr, ERR_INVALID_OPERATION, "Create device descriptor failed");

    auto callerUid = uid == -1 ? IPCSkeleton::GetCallingUid() : uid;
    if (preferredType == AUDIO_CALL_RENDER) {
        int32_t PreferUid = GetPreferredUid(callerUid);
        AudioPolicyUtils::GetInstance().SetPreferredDevice(preferredType, descriptor, callerUid, "SelectOutputDevice");
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(preferredType, descriptor);
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectFastOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    AUDIO_INFO_LOG("Start for uid[%{public}d] device[%{public}s]", audioRendererFilter->uid,
        GetEncryptStr(deviceDescriptor->networkId_).c_str());
    // note: check if stream is already running
    // if is running, call moveProcessToEndpoint.

    // otherwises, keep router info in the map
    int32_t res = AudioRouteMap::GetInstance().AddFastRouteMapInfo(audioRendererFilter->uid,
        deviceDescriptor->networkId_, OUTPUT_DEVICE);
    return res;
}

void AudioSelectInterfaceService::HandleFetchDeviceChange(const AudioStreamDeviceChangeReason &reason,
    const std::string &caller)
{
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_LOG(coreService != nullptr, "coreService is null");
    coreService->FetchOutputDeviceAndRoute("HandleFetchDeviceChange", reason);
    coreService->FetchInputDeviceAndRoute("HandleFetchDeviceChange");
    auto currentInputDevice = AudioActiveDevice::GetInstance().GetCurrentInputDevice();
    auto currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    coreService->ReloadSourceForDeviceChange(currentInputDevice, currentOutputDevice, caller);
    CHECK_AND_RETURN_LOG(audioA2dpOffloadManager_ != nullptr, "audioA2dpOffloadManager_ is nullptr");
    if ((currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
        (currentOutputDevice.networkId_ != LOCAL_NETWORK_ID)) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
    } else {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(currentOutputDevice.deviceType_);
    }
}

int32_t AudioSelectInterfaceService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    int32_t uid)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AUDIO_WARNING_LOG("audioDevUsage[%{public}d], Exclude devices list size [%{public}zu], %{public}s",
        audioDevUsage, deviceDescs.size(),
        AudioPolicyUtils::GetInstance().GetDevicesStr(deviceDescs).c_str());

    CHECK_AND_RETURN_RET_LOG(deviceDescs.size() > 0, ERR_INVALID_PARAM, "No device to exclude");
    for (const auto& deviceDesc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERROR_INVALID_PARAM, "deviceDesc is null");
        AUDIO_INFO_LOG("deviceDesc->dmDeviceType_ %{public}d", deviceDesc->dmDeviceType_);
        if (deviceDesc->dmDeviceType_ == DM_DEVICE_TYPE_MUSIC_HOST) {
            std::string taskId = "{\"taskId\":\"0\"}";
            AudioServerProxy::GetInstance().NotifyTaskIdInfoProxy(taskId, false);
            AUDIO_INFO_LOG("taskId clean success!, is not connected");
        }
    }
    if (deviceDescs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceDescs.front()->macAddress_.empty()) {
        AudioRouterSelectStrategy::GetInstance().SetScoExcluded(true);
        return SUCCESS;
    }

    int32_t ret = ExcludeOutputDevicesInner(audioDevUsage, deviceDescs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Exclude devices failed");

    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchOutputDeviceAndRoute("ExcludeOutputDevices",
        AudioStreamDeviceChangeReason::OVERRODE);
    coreService->FetchInputDeviceAndRoute("ExcludeOutputDevices");
    AudioDeviceDescriptor currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    AudioDeviceDescriptor currentInputDevice = AudioActiveDevice::GetInstance().GetCurrentInputDevice();
    coreService->ReloadSourceForDeviceChange(
        currentInputDevice, currentOutputDevice, "ExcludeOutputDevices");

    if (audioA2dpOffloadManager_ != nullptr) {
        if ((currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
            (currentOutputDevice.networkId_ != LOCAL_NETWORK_ID)) {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
        } else {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(currentOutputDevice.deviceType_);
        }
    }

    for (const auto &deviceDesc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "Invalid device descriptor");
        coreService->NotifyRemoteDeviceStatusUpdate(deviceDesc);
    }
    coreService->OnPreferredOutputDeviceUpdated(currentOutputDevice,
        AudioStreamDeviceChangeReason::OVERRODE);
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ExcludeOutputDevicesInner(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    const std::string macAddress = deviceDescs.front()->macAddress_;
    auto allDevices = AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();

    std::unordered_set<std::shared_ptr<AudioDeviceDescriptor>> excludeSet;
    if (!(audioDevUsage == ALL_MEDIA_DEVICES && !macAddress.empty())) {
        excludeSet.insert(deviceDescs.begin(), deviceDescs.end());
    }

    auto matchCondition = [&](const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc) -> bool {
        if (audioDevUsage == ALL_MEDIA_DEVICES && !macAddress.empty()) {
            // Find Sco device when exclude all media usage for A2dp device
            return !deviceDesc->macAddress_.empty() &&
                deviceDesc->macAddress_ == macAddress &&
                deviceDesc->deviceRole_ == OUTPUT_DEVICE;
        }
        return std::any_of(excludeSet.begin(), excludeSet.end(),
            [&](const auto &excludeDesc) {
                return deviceDesc->IsSameDeviceDescPtr(excludeDesc);
            });
    };

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors;
    std::copy_if(allDevices.begin(), allDevices.end(),
        std::back_inserter(deviceDescriptors), matchCondition);

    AudioRouterSelectStrategy::GetInstance().ExcludeDevices(deviceDescriptors, audioDevUsage);
    for (const auto &deviceDesc : deviceDescriptors) {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "Invalid device descriptor");
        ClearActiveHfpDevice(deviceDesc);
        AudioActiveDevice::GetInstance().NotifyUserDisSelectionEventToBt(deviceDesc);
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ClearActiveHfpDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    if (deviceDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        auto &audioDeviceFactory = AudioDeviceFactory::GetInstance();
        auto deviceDescs = {deviceDesc};
        return audioDeviceFactory.DeactivateDevice(deviceDescs);
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AUDIO_WARNING_LOG("audioDevUsage[%{public}d], Unexclude devices list size [%{public}zu], %{public}s",
        audioDevUsage, deviceDescs.size(),
        AudioPolicyUtils::GetInstance().GetDevicesStr(deviceDescs).c_str());
    if (deviceDescs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceDescs.front()->macAddress_.empty()) {
        AudioRouterSelectStrategy::GetInstance().SetScoExcluded(false);
        return SUCCESS;
    }
    int32_t ret = UnexcludeOutputDevicesInner(audioDevUsage, deviceDescs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Unexclude devices failed");

    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchOutputDeviceAndRoute("UnexcludeOutputDevices",
        AudioStreamDeviceChangeReason::OVERRODE);
    coreService->FetchInputDeviceAndRoute("UnexcludeOutputDevices");
    AudioDeviceDescriptor currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    AudioDeviceDescriptor currentInputDevice = AudioActiveDevice::GetInstance().GetCurrentInputDevice();
    coreService->ReloadSourceForDeviceChange(
        currentInputDevice, currentOutputDevice, "UnexcludeOutputDevices");
    if (audioA2dpOffloadManager_ != nullptr) {
        if ((currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
            (currentOutputDevice.networkId_ != LOCAL_NETWORK_ID)) {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
        } else {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(currentOutputDevice.deviceType_);
        }
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::UnexcludeOutputDevicesInner(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    CHECK_AND_RETURN_RET_LOG(deviceDescs.size() > 0, ERR_INVALID_PARAM, "No device to exclude");
    AudioRouterSelectStrategy::GetInstance().UnexcludeDevices(deviceDescs, audioDevUsage);
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetSessionDefaultOutputDevice(const int32_t callerPid,
    const DeviceType &deviceType)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetDefaultOutputDevice(
    const DeviceType deviceType, const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning,
    bool skipForce)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const SourceType sourceType, bool isRunning)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    return SUCCESS;
}

void AudioSelectInterfaceService::SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    return;
}

void AudioSelectInterfaceService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter, const std::string &caller)
{
    if (!filter) {
        filter = new AudioRendererFilter();
        CHECK_AND_RETURN_LOG(filter, "filter is nullptr");
    }
    filter->uid = SYSTEM_UID;
    AUDIO_INFO_LOG("Entry. devType=%{public}d, addr=%{public}s, streamUsage=%{public}d",
        devType, AudioPolicyUtils::GetInstance().GetEncryptAddr(macAddress).c_str(), filter->rendererInfo.streamUsage);
    auto devDescs = AudioDeviceManager::GetAudioDeviceManager().GetAvailableBluetoothDevice(devType, macAddress);
    bool isNeedNotifyBt = (caller == "Bluetooth") ? false : true;
    for (auto &devDesc : devDescs) {
        if (devDesc->deviceRole_ == OUTPUT_DEVICE) {
            SelectOutputDevice(filter, {devDesc}, 0, isNeedNotifyBt);
        }
    }
}

void AudioSelectInterfaceService::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
    const std::string &caller)
{
    AUDIO_INFO_LOG("Entry");
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID,
        std::make_shared<AudioDeviceDescriptor>(), caller);
    auto type = static_cast<Media::MediaMonitor::PreferredType>(AUDIO_CALL_RENDER);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().ErasePreferredDeviceByType(type);

    bool hasUsablePrivateCallDevice {false};
    auto devs = AudioRouterCenter::GetAudioRouterCenter().FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION,
        -1, "OnPrivacyDeviceSelected", ROUTER_TYPE_USER_SELECT);
    auto pDevs = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    for (auto &dev : devs) {
        auto it = find_if(pDevs.cbegin(), pDevs.cend(), [&dev](auto &item) {
            return dev && item && dev->IsSameDeviceDescPtr(item);
        });
        if (it != pDevs.cend()) {
            hasUsablePrivateCallDevice = true;
            break;
        }
    }
    if (!hasUsablePrivateCallDevice) {
        sptr<AudioRendererFilter> filter = new AudioRendererFilter();
        CHECK_AND_RETURN_LOG(filter, "filter is nullptr");
        filter->rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        OnForcedDeviceSelected(devType, macAddress, filter, caller);
        return;
    }
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_LOG(coreService, "coreService is nullptr");
    coreService->FetchOutputDeviceAndRoute("OnPrivacyDeviceSelected", AudioStreamDeviceChangeReason::OVERRODE);
    coreService->FetchInputDeviceAndRoute("OnPrivacyDeviceSelected", AudioStreamDeviceChangeReason::OVERRODE);
}

int32_t AudioSelectInterfaceService::SetA2dpDeviceOffloadManager(
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager)
{
    CHECK_AND_RETURN_RET_LOG(audioA2dpOffloadManager != nullptr, ERR_INVALID_PARAM,
        "audioA2dpOffloadManager is nullptr");
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
