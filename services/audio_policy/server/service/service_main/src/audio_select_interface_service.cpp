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
#include "audio_stream_collector.h"
#include "audio_router_map.h"
#include "bluetooth_device_manager.h"
#include "audio_bluetooth_manager.h"
#include "audio_scene_manager.h"
#include "hisysevent.h"
#include "stream_dfx_manager.h"

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

bool AudioSelectInterfaceService::GetEnhancedRoutingSupported() const
{
    return AudioRouterSelectStrategy::GetInstance().GetEnhancedRoutingSupported();
}

void AudioSelectInterfaceService::HandleNegtiveBt(DeviceType deviceType)
{
    auto descs = AudioRouterSelectStrategy::GetInstance().FindCurrentOutputDevice(
        {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_NEARLINK});
    auto currentOutputDevice = !descs.empty() && *descs.begin() ? *(*descs.begin()) : AudioDeviceDescriptor();
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
    auto descs = AudioRouterSelectStrategy::GetInstance().FindCurrentOutputDevice(
        {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_NEARLINK});
    auto currentOutputDevice = !descs.empty() && *descs.begin() ? *(*descs.begin()) : AudioDeviceDescriptor();
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
    SetDeviceEnableAndUsage(selectedDesc[0]);

    if (audioDeviceSelectMode == SELECT_STRATEGY_STREAM || audioDeviceSelectMode == SELECT_STRATEGY_INDEPENDENT) {
        CHECK_AND_CALL_FUNC_RETURN_RET(audioRendererFilter->uid > 0, ERR_INVALID_OPERATION,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
                ERR_PLAY_DEVICE_SWITCH_INVALID_PARAM, "SelectOutputDevice: invalid uid", false));
        AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(audioRendererFilter->uid,
            audioRendererFilter->streamId, selectedDesc[0]);
    } else if (audioRendererFilter->rendererInfo.rendererFlags == STREAM_FLAG_FAST) {
        return SelectOutputDeviceForFastInner(audioRendererFilter, selectedDesc);
    } else {
        res = SetRenderDeviceForUsage(strUsage, selectedDesc[0], audioRendererFilter->uid);
        CHECK_AND_CALL_FUNC_RETURN_RET(res == SUCCESS, res,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
                ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "SelectOutputDevice: SetRenderDeviceForUsage failed", false));
    }

    if (selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(selectedDesc[0]->macAddress_);
    }

    // If the selected device is virtual device, connect it.
    if (AudioDeviceManager::GetAudioDeviceManager().IsVirtualConnectedDevice(selectedDesc[0])) {
        int32_t ret = AudioRecoveryDevice::GetInstance().ConnectVirtualDevice(selectedDesc[0]);
        CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ret, StreamDfxManager::GetInstance().SendAudioErrorEvent(
            IPCSkeleton::GetCallingUid(), ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "ConnectVirtualDevice fail", false));
        return SUCCESS;
    }

    if (isNeedNotifyBt) {
        AudioActiveDevice::GetInstance().NotifyUserSelectionEventToBt(selectedDesc[0], strUsage);
    }
    HandleFetchDeviceChange(AudioStreamDeviceChangeReason::OVERRODE, "SelectOutputDevice");
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->OnPreferredOutputDeviceUpdated(AudioRouterSelectStrategy::GetInstance()
        .Get1stCurrentOutputDevice(audioRendererFilter->uid), AudioStreamDeviceChangeReason::OVERRODE);
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
    CHECK_AND_CALL_FUNC_RETURN(selectedDesc.size() == 1 && selectedDesc[0] &&
        selectedDesc[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_PLAY_DEVICE_SWITCH_INVALID_PARAM, "SelectOutputDevice: DeviceCheck no success", false));
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
    return SUCCESS;
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

    auto callerUid = uid == INVALID_UID ? IPCSkeleton::GetCallingUid() : uid;
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
    auto currentInputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice();
    auto currentOutputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice();
    coreService->ReloadSourceForDeviceChange(currentInputDevice, currentOutputDevice, caller);
    CHECK_AND_RETURN_LOG(audioA2dpOffloadManager_ != nullptr, "audioA2dpOffloadManager_ is nullptr");
    if ((currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
        (currentOutputDevice.networkId_ != LOCAL_NETWORK_ID)) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
    } else {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(currentOutputDevice.deviceType_);
    }
}

int32_t AudioSelectInterfaceService::SetMediaOutputDeviceByUid(DeviceType deviceType, const int32_t uid)
{
    int32_t callerUid = GetPreferredUid(uid);
    if (deviceType == DEVICE_TYPE_DEFAULT) {
        AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(
            callerUid, INVALID_STREAM_ID, std::make_shared<AudioDeviceDescriptor>());
        return SUCCESS;
    }
    auto isPresent = [&deviceType] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return deviceType == desc->deviceType_;
    };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> mediaDevices = {
        AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice()
    };
    auto itr = std::find_if(mediaDevices.begin(), mediaDevices.end(), isPresent);
    CHECK_AND_RETURN_RET_LOG(itr != mediaDevices.end(), SUCCESS,
        "Requested device not available %{public}d", deviceType);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(callerUid, INVALID_STREAM_ID, *itr);
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr && !selectedDesc.empty() && selectedDesc[0] != nullptr,
        ERROR, "ptr exception");
    AUDIO_WARNING_LOG("uid[%{public}d] type[%{public}d] mac[%{public}s] pid[%{public}d]",
        audioCapturerFilter->uid, selectedDesc[0]->deviceType_, AudioPolicyUtils::GetInstance().GetEncryptAddr(
            selectedDesc[0]->macAddress_).c_str(), IPCSkeleton::GetCallingPid());
    CHECK_AND_RETURN_RET_LOG(selectedDesc.size() == 1 &&
        AudioDeviceManager::GetAudioDeviceManager().IsDeviceConnected(selectedDesc[0]) &&
        IsInputDevice(selectedDesc[0]->deviceType_, selectedDesc[0]->deviceRole_), ERR_INVALID_OPERATION, "err");

    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    int32_t selectUid = audioCapturerFilter->uid == -1 ? SYSTEM_UID : audioCapturerFilter->uid;
    uint32_t selectStreamId = audioCapturerFilter->audioDeviceSelectMode == SELECT_STRATEGY_STREAM ?
        static_cast<uint32_t>(audioCapturerFilter->streamId) : INVALID_STREAM_ID;
    SourceType srcType = audioCapturerFilter->capturerInfo.sourceType;
    AudioScene scene = AudioSceneManager::GetInstance().GetAudioScene(true);
    if (audioCapturerFilter->capturerInfo.capturerFlags == STREAM_FLAG_FAST) {
        if (scene == AUDIO_SCENE_PHONE_CALL || scene == AUDIO_SCENE_PHONE_CHAT ||
            srcType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(selectUid, selectStreamId, selectedDesc[0]);
        } else {
            AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(selectUid, selectStreamId, selectedDesc[0]);
        }
        int32_t result = AudioRouteMap::GetInstance().AddFastRouteMapInfo(audioCapturerFilter->uid,
            selectedDesc[0]->networkId_, INPUT_DEVICE);
        CheckAndWriteDeviceChangeExceptionEvent(result == SUCCESS, AudioStreamDeviceChangeReason::OVERRODE,
            selectedDesc[0]->deviceType_, selectedDesc[0]->deviceRole_, result, "AddFastRouteMapInfo failed!");
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "AddFastRouteMapInfo failed! fastRouteMap is too large!");
        AUDIO_INFO_LOG("Success for uid[%{public}d] device[%{public}s]",
            audioCapturerFilter->uid, GetEncryptStr(selectedDesc[0]->networkId_).c_str());
        coreService->FetchInputDeviceAndRoute("SelectInputDevice_1");
        coreService->ReloadSourceForDeviceChange(AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice(),
            AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(), "SelectInputDevice fast");
        return SUCCESS;
    }
    auto ret = RefreshVirtualDevice(selectedDesc[0]);
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ret, StreamDfxManager::GetInstance().SendAudioErrorEvent(
        IPCSkeleton::GetCallingUid(), ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED, "RefreshVirtualDevice failed", false));

    if (scene == AUDIO_SCENE_PHONE_CALL || scene == AUDIO_SCENE_PHONE_CHAT ||
        srcType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(selectUid, selectStreamId, selectedDesc[0]);
    } else {
        AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(selectUid, selectStreamId, selectedDesc[0]);
    }

    AudioActiveDevice::GetInstance().NotifyUserSelectionEventForInput(selectedDesc[0], srcType);
    coreService->FetchInputDeviceAndRoute("SelectInputDevice_2");

    coreService->ReloadSourceForDeviceChange(AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice(),
        AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(), "SelectInputDevice");
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::RefreshVirtualDevice(shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_RET_LOG(desc, ERR_NULL_POINTER, "desc is nullptr");
    bool isVirtualDevice = AudioDeviceManager::GetAudioDeviceManager().IsVirtualConnectedDevice(desc);
    if (isVirtualDevice == true) {
        desc->connectState_ = VIRTUAL_CONNECTED;
    }
    SetDeviceEnableAndUsage(desc);
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(desc->macAddress_);
    }
    // If the selected device is virtual device, connect it.
    if (isVirtualDevice) {
        int32_t ret = AudioRecoveryDevice::GetInstance().ConnectVirtualDevice(desc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Connect device [%{public}s] failed",
            GetEncryptStr(desc->macAddress_).c_str());
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    int32_t uid)
{
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(uid, INVALID_STREAM_ID, deviceDesc);
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchInputDeviceAndRoute("SelectInputDeviceByUid");
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

    ret = FetchInputAndOutputDevice("ExcludeOutputDevices", AudioStreamDeviceChangeReason::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchInputAndOutputDevice failed");
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    AudioDeviceDescriptor currentOutputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice();
    AudioDeviceDescriptor currentInputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice();
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
    CHECK_AND_RETURN_RET_LOG(deviceDescs.size() > 0 && deviceDescs.front() != nullptr,
        ERR_INVALID_PARAM, "No device to exclude");
    if (deviceDescs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceDescs.front()->macAddress_.empty()) {
        AudioRouterSelectStrategy::GetInstance().SetScoExcluded(false);
        return SUCCESS;
    }
    int32_t ret = UnexcludeOutputDevicesInner(audioDevUsage, deviceDescs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Unexclude devices failed");

    ret = FetchInputAndOutputDevice("UnexcludeOutputDevices", AudioStreamDeviceChangeReason::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchInputAndOutputDevice failed");
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    AudioDeviceDescriptor currentOutputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice();
    AudioDeviceDescriptor currentInputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice();
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

int32_t AudioSelectInterfaceService::SetDefaultOutputDevice(const DeviceType deviceType,
    const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning, bool skipForce)
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager->GetHasEarpiece(), ERR_NOT_SUPPORTED, "the device has no earpiece");
    CHECK_AND_RETURN_RET_LOG(pipeManager->GetStreamDescById(sessionID) != nullptr, ERR_NOT_SUPPORTED,
        "sessionId is not exist");

    auto &audioSessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    if (!audioSessionService.IsStreamAllowedToSetDevice(sessionID)) {
        AUDIO_ERR_LOG("current stream is contained in a session which had set default output device");
        return ERR_NOT_SUPPORTED;
    }

    AUDIO_INFO_LOG("[ADeviceEvent] device %{public}d for %{public}s stream %{public}u", deviceType,
        isRunning ? "running" : "not running", sessionID);
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto &streamCollector = AudioStreamCollector::GetAudioStreamCollector();
    streamCollector.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    bool forceFetch = false;
    int32_t clientUid = INVALID_UID;
    for (auto &changeInfo : audioRendererChangeInfos) {
        if (changeInfo->sessionId != static_cast<int32_t>(sessionID)) {
            continue;
        }
        if (changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
            changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
            changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
            changeInfo->rendererInfo.streamUsage == STREAM_USAGE_INTERPHONE) {
            CHECK_AND_CONTINUE(!skipForce);
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                std::make_shared<AudioDeviceDescriptor>(), changeInfo->clientUID, "SetDefaultOutputDevice");
            forceFetch = true;
        }
        clientUid = changeInfo->clientUID;
    }
    if (clientUid == INVALID_UID) {
        return SUCCESS;
    }

    int32_t ret = SUCCESS;
    ret = AudioRouterSelectStrategy::GetInstance().UpdateDefaultOutputDevice(deviceType, clientUid,
        sessionID, streamUsage);
    if ((isRunning && ret == NEED_TO_FETCH) || forceFetch) {
        AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("SetDefaultOutputDevice",
            AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
        return SUCCESS;
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetInputDevice(
    const DeviceType deviceType, const uint32_t sessionID, int32_t uid)
{
    auto connectedDevices_ = AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();
    shared_ptr<AudioDeviceDescriptor> target = nullptr;
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == deviceType) {
            AUDIO_WARNING_LOG("sessionid %{public}d has selected device", sessionID);
            target = std::make_shared<AudioDeviceDescriptor>(*desc);
            break;
        }
    }

    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(uid, sessionID, target);

    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchInputDeviceAndRoute("SetInputDevice");
    coreService->FetchOutputDeviceAndRoute("SetInputDevice");
    return SUCCESS;
}

void AudioSelectInterfaceService::SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    AudioDeviceDescriptor preferredDevice = streamDesc->preferredInputDevice;
    CHECK_AND_RETURN_LOG(preferredDevice.deviceType_ > DEVICE_TYPE_INVALID, "invalid deviceType");
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(streamDesc->appInfo_.appUid,
        streamDesc->sessionId_, std::make_shared<AudioDeviceDescriptor>(preferredDevice));

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION) {
        WriteDesignateAudioCaptureDeviceEvent(streamDesc->capturerInfo_.sourceType, preferredDevice.deviceType_, true);
    } else if (preferredDevice.deviceType_ == DEVICE_TYPE_BT_SPP) {
        AUDIO_WARNING_LOG("BT SPP incorrectly selected as preferred input device in non-recognition session");
        WriteDesignateAudioCaptureDeviceEvent(streamDesc->capturerInfo_.sourceType, preferredDevice.deviceType_, false);
    }
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
    FetchDeviceInfo info = { STREAM_USAGE_VOICE_COMMUNICATION, "OnPrivacyDeviceSelected", ROUTER_TYPE_USER_SELECT};
    auto devs = AudioRouterCenter::GetAudioRouterCenter().FetchOutputDevices(info);
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
    int32_t ret = FetchInputAndOutputDevice("OnPrivacyDeviceSelected", AudioStreamDeviceChangeReason::OVERRODE,
        AudioStreamDeviceChangeReason::OVERRODE);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "FetchInputAndOutputDevice failed");
}

int32_t AudioSelectInterfaceService::FetchInputAndOutputDevice(const std::string &caller,
    const AudioStreamDeviceChangeReasonExt outputReason, const AudioStreamDeviceChangeReasonExt inputReason)
{
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchOutputDeviceAndRoute(caller, outputReason);
    coreService->FetchInputDeviceAndRoute(caller, inputReason);
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetA2dpDeviceOffloadManager(
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager)
{
    CHECK_AND_RETURN_RET_LOG(audioA2dpOffloadManager != nullptr, ERR_INVALID_PARAM,
        "audioA2dpOffloadManager is nullptr");
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
    return SUCCESS;
}

void AudioSelectInterfaceService::WriteDesignateAudioCaptureDeviceEvent(
    SourceType sourceType, int32_t deviceType, bool isNormalSelection)
{
    std::string appName = AudioBundleManager::GetBundleName();
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO,
        "DESIGNATE_AUDIO_CAPTURE_DEVICE", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "APP_NAME", appName.c_str(),
        "STREAM_TYPE", sourceType,
        "DEVICE_TYPE", deviceType,
        "ERROR_CODE", isNormalSelection ? 0 : 1);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "write event fail: DESIGNATE_AUDIO_CAPTURE_DEVICE, ret = %{public}d", ret);
}
} // namespace AudioStandard
} // namespace OHOS
