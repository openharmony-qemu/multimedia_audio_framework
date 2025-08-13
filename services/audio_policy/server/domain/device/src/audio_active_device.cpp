
/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioActiveDevice"
#endif

#include "audio_active_device.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"
#include "sle_audio_device_manager.h"
#include "audio_pipe_manager.h"

namespace OHOS {
namespace AudioStandard {

#ifdef BLUETOOTH_ENABLE
const uint32_t USER_NOT_SELECT_BT = 1;
const uint32_t USER_SELECT_BT = 2;
#endif

bool AudioActiveDevice::GetActiveA2dpDeviceStreamInfo(DeviceType deviceType, AudioStreamInfo &streamInfo)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        A2dpDeviceConfigInfo info;
        if (audioA2dpDevice_.GetA2dpDeviceInfo(activeBTDevice_, info)) {
            streamInfo.samplingRate = *info.streamInfo.samplingRate.rbegin();
            streamInfo.format = info.streamInfo.format;
            streamInfo.channels = *info.streamInfo.GetChannels().rbegin();
            return true;
        }
    } else if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        A2dpDeviceConfigInfo info;
        if (audioA2dpDevice_.GetA2dpInDeviceInfo(activeBTInDevice_, info)) {
            streamInfo.samplingRate = *info.streamInfo.samplingRate.rbegin();
            streamInfo.format = info.streamInfo.format;
            streamInfo.channels = *info.streamInfo.GetChannels().rbegin();
            return true;
        }
    }
    return false;
}

std::string AudioActiveDevice::GetActiveBtDeviceMac()
{
    return activeBTDevice_;
}

void AudioActiveDevice::SetActiveBtDeviceMac(const std::string macAddress)
{
    activeBTDevice_ = macAddress;
}

void AudioActiveDevice::SetActiveBtInDeviceMac(const std::string macAddress)
{
    activeBTInDevice_ = macAddress;
}

bool AudioActiveDevice::IsDirectSupportedDevice()
{
    DeviceType dev = GetCurrentOutputDeviceType();
    return dev == DEVICE_TYPE_WIRED_HEADSET || dev == DEVICE_TYPE_USB_HEADSET;
}

bool AudioActiveDevice::CheckActiveOutputDeviceSupportOffload()
{
    DeviceType dev = GetCurrentOutputDeviceType();
    if (GetCurrentOutputDeviceNetworkId() != LOCAL_NETWORK_ID || dev == DEVICE_TYPE_REMOTE_CAST) {
        return false;
    }

    return dev == DEVICE_TYPE_SPEAKER ||
        (dev == DEVICE_TYPE_BLUETOOTH_A2DP && audioA2dpOffloadFlag_.GetA2dpOffloadFlag() == A2DP_OFFLOAD) ||
        dev == DEVICE_TYPE_USB_HEADSET;
}

void AudioActiveDevice::SetCurrentInputDevice(const AudioDeviceDescriptor &desc)
{
    std::lock_guard<std::mutex> lock(curInputDevice_);
    AUDIO_INFO_LOG("Set as type: %{public}d id: %{public}d", desc.deviceType_, desc.deviceId_);
    currentActiveInputDevice_ = AudioDeviceDescriptor(desc);
}

const AudioDeviceDescriptor AudioActiveDevice::GetCurrentInputDevice()
{
    std::lock_guard<std::mutex> lock(curInputDevice_);
    return currentActiveInputDevice_;
}


DeviceType AudioActiveDevice::GetCurrentInputDeviceType()
{
    std::lock_guard<std::mutex> lock(curInputDevice_);
    return currentActiveInputDevice_.deviceType_;
}

std::string AudioActiveDevice::GetCurrentInputDeviceMacAddr()
{
    std::lock_guard<std::mutex> lock(curInputDevice_);
    return currentActiveDevice_.macAddress_;
}

void AudioActiveDevice::SetCurrentOutputDevice(const AudioDeviceDescriptor &desc)
{
    std::lock_guard<std::mutex> lock(curOutputDevice_);
    AUDIO_INFO_LOG("Set as type: %{public}d id: %{public}d", desc.deviceType_, desc.deviceId_);
    currentActiveDevice_ = AudioDeviceDescriptor(desc);
}

const AudioDeviceDescriptor AudioActiveDevice::GetCurrentOutputDevice()
{
    std::lock_guard<std::mutex> lock(curOutputDevice_);
    return currentActiveDevice_;
}

DeviceType AudioActiveDevice::GetCurrentOutputDeviceType()
{
    std::lock_guard<std::mutex> lock(curOutputDevice_);
    return currentActiveDevice_.deviceType_;
}

DeviceCategory AudioActiveDevice::GetCurrentOutputDeviceCategory()
{
    std::lock_guard<std::mutex> lock(curOutputDevice_);
    return currentActiveDevice_.deviceCategory_;
}

std::string AudioActiveDevice::GetCurrentOutputDeviceNetworkId()
{
    std::lock_guard<std::mutex> lock(curOutputDevice_);
    return currentActiveDevice_.networkId_;
}

std::string AudioActiveDevice::GetCurrentOutputDeviceMacAddr()
{
    std::lock_guard<std::mutex> lock(curOutputDevice_);
    return currentActiveDevice_.macAddress_;
}

float AudioActiveDevice::GetMaxAmplitude(const int32_t deviceId, AudioInterrupt audioInterrupt)
{
    AudioDeviceDescriptor descriptor = GetCurrentOutputDevice();
    if (deviceId == descriptor.deviceId_) {
        uint32_t sessionId = audioInterrupt.streamId;
        std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkName(descriptor, static_cast<int32_t>(sessionId));
        std::string deviceClass = AudioPolicyUtils::GetInstance().GetOutputDeviceClassBySinkPortName(sinkName);
        return AudioServerProxy::GetInstance().GetMaxAmplitudeProxy(true, deviceClass);
    }

    descriptor = GetCurrentInputDevice();
    if (deviceId == descriptor.deviceId_) {
        std::string sourceName = AudioPolicyUtils::GetInstance().GetSourcePortName(GetCurrentInputDeviceType());
        std::string deviceClass = AudioPolicyUtils::GetInstance().GetInputDeviceClassBySourcePortName(sourceName);
        return AudioServerProxy::GetInstance().GetMaxAmplitudeProxy(false, deviceClass,
            audioInterrupt.audioFocusType.sourceType);
    }

    return 0;
}

void AudioActiveDevice::NotifyUserSelectionEventToBt(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
    StreamUsage streamUsage)
{
    Trace trace("AudioActiveDevice::NotifyUserSelectionEventToBt");
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "audioDeviceDescriptor is nullptr");
#ifdef BLUETOOTH_ENABLE
    auto currentOutputDevice = std::make_shared<AudioDeviceDescriptor>(GetCurrentOutputDevice());
    CHECK_AND_RETURN_LOG(currentOutputDevice != nullptr, "currentOutputDevice is nullptr");

    bool isSameDevice = audioDeviceDescriptor->IsSameDeviceDesc(*currentOutputDevice);
    NotifyUserDisSelectionEventToBt(currentOutputDevice, isSameDevice);

    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO ||
        audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        Bluetooth::SendUserSelectionEvent(audioDeviceDescriptor->deviceType_,
            audioDeviceDescriptor->macAddress_, USER_SELECT_BT);
    }
    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(*audioDeviceDescriptor,
            streamUsage);
    }
#endif
}

void AudioActiveDevice::NotifyUserDisSelectionEventToBt(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
    bool isSameDevice)
{
    CHECK_AND_RETURN_LOG(!isSameDevice, "isSameDevice is true, do not notify");
    AUDIO_INFO_LOG("UserDisSelection start");
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "deviceDesc is nullptr");
#ifdef BLUETOOTH_ENABLE
    Bluetooth::SendUserSelectionEvent(
        audioDeviceDescriptor->deviceType_, audioDeviceDescriptor->macAddress_, USER_NOT_SELECT_BT);
    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        audioDeviceDescriptor->IsSameDeviceDesc(GetCurrentOutputDevice())) {
        Bluetooth::AudioHfpManager::DisconnectSco();
    }
#endif
    SleAudioDeviceManager::GetInstance().SetActiveDevice(audioDeviceDescriptor, STREAM_USAGE_INVALID);
    SleAudioDeviceManager::GetInstance().SendUserSelection(*audioDeviceDescriptor, STREAM_USAGE_INVALID);
}

void AudioActiveDevice::NotifyUserSelectionEventForInput(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
    SourceType sourceType)
{
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "audioDeviceDescriptor is nullptr");
#ifdef BLUETOOTH_ENABLE
    auto curInputDevice = std::make_shared<AudioDeviceDescriptor>(GetCurrentInputDevice());
    CHECK_AND_RETURN_LOG(curInputDevice != nullptr, "curInputDevice is nullptr");

    bool isSameDevice = audioDeviceDescriptor->IsSameDeviceDesc(*curInputDevice);
    NotifyUserDisSelectionEventToBt(curInputDevice, isSameDevice);

    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO ||
        audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        Bluetooth::SendUserSelectionEvent(audioDeviceDescriptor->deviceType_,
            audioDeviceDescriptor->macAddress_, USER_SELECT_BT);
    }
    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_NEARLINK_IN) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(*audioDeviceDescriptor,
            sourceType);
    }
#endif
}

void AudioActiveDevice::WriteOutputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const AudioStreamDeviceChangeReason reason)
{
    int64_t timeStamp = AudioPolicyUtils::GetInstance().GetCurrentTimeMS();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_ROUTE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    DeviceType curOutputDeviceType = GetCurrentOutputDeviceType();
    bean->Add("REASON", static_cast<int32_t>(reason));
    bean->Add("TIMESTAMP", static_cast<uint64_t>(timeStamp));
    bean->Add("DEVICE_TYPE_BEFORE_CHANGE", curOutputDeviceType);
    bean->Add("DEVICE_TYPE_AFTER_CHANGE", desc->deviceType_);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

bool AudioActiveDevice::UpdateDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const AudioStreamDeviceChangeReasonExt reason, const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo)
{
    std::shared_ptr<AudioDeviceDescriptor> preferredDesc =
        audioAffinityManager_.GetRendererDevice(rendererChangeInfo->clientUID);
    AudioDeviceDescriptor tmpOutputDeviceDesc = GetCurrentOutputDevice();
    if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) && !desc->IsSameDeviceInfo(tmpOutputDeviceDesc)
        && desc->deviceType_ != preferredDesc->deviceType_)
        || ((preferredDesc->deviceType_ == DEVICE_TYPE_NONE) && !desc->IsSameDeviceInfo(tmpOutputDeviceDesc))) {
        WriteOutputRouteChangeEvent(desc, reason);
        SetCurrentOutputDevice(*desc);
        AUDIO_DEBUG_LOG("currentActiveDevice update %{public}d", GetCurrentOutputDeviceType());
        return true;
    }
    return false;
}

void AudioActiveDevice::HandleActiveBt(DeviceType deviceType, std::string macAddress)
{
    if (GetCurrentOutputDeviceType() == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceType != DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::SendUserSelectionEvent(DEVICE_TYPE_BLUETOOTH_SCO,
            GetCurrentOutputDeviceMacAddr(), USER_NOT_SELECT_BT);
        Bluetooth::AudioHfpManager::DisconnectSco();
    }
    if (GetCurrentOutputDeviceType() != DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::SendUserSelectionEvent(DEVICE_TYPE_BLUETOOTH_SCO,
            macAddress, USER_SELECT_BT);
    }
    if (GetCurrentOutputDeviceType() == DEVICE_TYPE_NEARLINK &&
        deviceType != DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SetActiveDevice(GetCurrentOutputDevice(), STREAM_USAGE_INVALID);
        SleAudioDeviceManager::GetInstance().SendUserSelection(GetCurrentOutputDevice(), STREAM_USAGE_INVALID);
    }
    if (deviceType == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(GetCurrentOutputDevice(),
            STREAM_USAGE_VOICE_COMMUNICATION);
    }
}

void AudioActiveDevice::HandleNegtiveBt(DeviceType deviceType)
{
    if (GetCurrentOutputDeviceType() == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::SendUserSelectionEvent(DEVICE_TYPE_BLUETOOTH_SCO,
            GetCurrentOutputDeviceMacAddr(), USER_NOT_SELECT_BT);
        Bluetooth::AudioHfpManager::DisconnectSco();
    }
    if (GetCurrentOutputDeviceType() == DEVICE_TYPE_NEARLINK &&
        deviceType == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SetActiveDevice(GetCurrentOutputDevice(), STREAM_USAGE_INVALID);
        SleAudioDeviceManager::GetInstance().SendUserSelection(GetCurrentOutputDevice(), STREAM_USAGE_INVALID);
    }
}

bool AudioActiveDevice::IsDeviceActive(DeviceType deviceType)
{
    AUDIO_DEBUG_LOG("type [%{public}d]", deviceType);
    CHECK_AND_RETURN_RET(GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID, false);
    return GetCurrentOutputDeviceType() == deviceType;
}

int32_t AudioActiveDevice::SetDeviceActive(DeviceType deviceType, bool active, const int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    // Activate new device if its already connected
    auto isPresent = [&deviceType] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "SetDeviceActive::Invalid device descriptor");
        return ((deviceType == desc->deviceType_) || (deviceType == DEVICE_TYPE_FILE_SINK));
    };

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices
        = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    for (const auto &desc : callDevices) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
        deviceList.push_back(devDesc);
    }

    auto itr = std::find_if(deviceList.begin(), deviceList.end(), isPresent);
    CHECK_AND_RETURN_RET_LOG(itr != deviceList.end(), ERR_OPERATION_FAILED,
        "Requested device not available %{public}d ", deviceType);
    if (!active) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), uid, "SetDeviceActive");
#ifdef BLUETOOTH_ENABLE
        HandleNegtiveBt(deviceType);
#endif
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER, *itr, uid, "SetDeviceActive");
#ifdef BLUETOOTH_ENABLE
        HandleActiveBt(deviceType, (*itr)->macAddress_);
#endif
    }
    return SUCCESS;
}

int32_t AudioActiveDevice::SetCallDeviceActive(DeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
    // Activate new device if its already connected
    auto isPresent = [&deviceType, &address] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return ((deviceType == desc->deviceType_) && (address == desc->macAddress_));
    };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices
        = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);

    auto itr = std::find_if(callDevices.begin(), callDevices.end(), isPresent);
    CHECK_AND_RETURN_RET_LOG(itr != callDevices.end(), ERR_OPERATION_FAILED,
        "Requested device not available %{public}d ", deviceType);
    if (active) {
        if (deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
            (*itr)->isEnable_ = true;
            audioDeviceManager_.UpdateDevicesListInfo(std::make_shared<AudioDeviceDescriptor>(**itr), ENABLE_UPDATE);
            AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(address);
        }
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(**itr), uid, "SetCallDeviceActive");
#ifdef BLUETOOTH_ENABLE
        HandleActiveBt(deviceType, (*itr)->macAddress_);
#endif
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), uid, "SetCallDeviceActive");
#ifdef BLUETOOTH_ENABLE
        HandleNegtiveBt(deviceType);
#endif
    }
    return SUCCESS;
}

void AudioActiveDevice::UpdateActiveDeviceRoute(DeviceType deviceType, DeviceFlag deviceFlag,
    const std::string &deviceName, std::string networkId)
{
    Trace trace("KeyAction AudioActiveDevice::UpdateActiveDeviceRoute DeviceType:" + std::to_string(deviceType));
    CHECK_AND_RETURN_LOG(networkId == LOCAL_NETWORK_ID, "distributed device, do not update route");
    AUDIO_INFO_LOG("[PipeExecInfo] Active route with type[%{public}d]", deviceType);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back(make_pair(deviceType, deviceFlag));
    UpdateActiveDevicesRoute(activeDevices, deviceName);
}

void AudioActiveDevice::UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>>
    &activeDevices, const std::string &deviceName)
{
    CHECK_AND_RETURN_LOG(!activeDevices.empty(), "activeDevices is empty.");

    std::string deviceTypesInfo = "";
    for (size_t i = 0; i < activeDevices.size(); i++) {
        deviceTypesInfo = deviceTypesInfo + " " + std::to_string(activeDevices[i].first);
    }
    AUDIO_INFO_LOG("[PipeExecInfo] Active route with types[%{public}s]", deviceTypesInfo.c_str());

    Trace trace("AudioActiveDevice::UpdateActiveDevicesRoute DeviceTypes:" + deviceTypesInfo);
    auto ret = AudioServerProxy::GetInstance().UpdateActiveDevicesRouteProxy(activeDevices,
        audioA2dpOffloadFlag_.GetA2dpOffloadFlag(), deviceName);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the route for %{public}s", deviceTypesInfo.c_str());
}

bool AudioActiveDevice::IsDeviceInVector(std::shared_ptr<AudioDeviceDescriptor> desc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs)
{
    for (auto &it : descs) {
        CHECK_AND_RETURN_RET(!it->IsSameDeviceDesc(desc), true);
    }
    return false;
}

void AudioActiveDevice::UpdateStreamDeviceMap(std::string source)
{
    AUDIO_INFO_LOG("update for %{public}s", source.c_str());
    std::vector<std::shared_ptr<AudioStreamDescriptor>> descs =
        AudioPipeManager::GetPipeManager()->GetAllOutputStreamDescs();
    activeOutputDevices_.clear();
    for (auto &desc : descs) {
        CHECK_AND_CONTINUE(desc != nullptr);
        AUDIO_INFO_LOG("session: %{public}d, calleruid: %{public}d, appuid: %{public}d " \
            "usage:%{public}d devices:%{public}s",
            desc->sessionId_, desc->callerUid_, desc->appInfo_.appUid,
            desc->rendererInfo_.streamUsage, desc->GetNewDevicesInfo().c_str());
        AudioStreamType streamType = VolumeUtils::GetVolumeTypeFromStreamUsage(desc->rendererInfo_.streamUsage);
        streamTypeDeviceMap_[streamType] = desc->newDeviceDescs_.back();
        streamUsageDeviceMap_[desc->rendererInfo_.streamUsage] = desc->newDeviceDescs_.front();
        for (const auto &device : desc->newDeviceDescs_) {
            CHECK_AND_CONTINUE(!IsDeviceInVector(device, activeOutputDevices_));
            activeOutputDevices_.push_back(device);
        }
    }

    for (auto &pair : streamTypeDeviceMap_) {
        CHECK_AND_CONTINUE(!IsDeviceInVector(pair.second, activeOutputDevices_));
        streamTypeDeviceMap_[pair.first] = nullptr;
    }

    for (auto &pair : streamUsageDeviceMap_) {
        CHECK_AND_CONTINUE(!IsDeviceInVector(pair.second, activeOutputDevices_));
        streamUsageDeviceMap_[pair.first] = nullptr;
    }
}
}
}
