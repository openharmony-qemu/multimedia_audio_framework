
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
#include "audio_router_center.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"
#include "sle_audio_device_manager.h"
#include "audio_pipe_manager.h"
#include "audio_zone_service.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t DEFAULT_PIPE_ID = 0;
}

static const int32_t MEDIA_SERVICE_UID = 1013;

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

DeviceType AudioActiveDevice::GetCurrentInputDeviceType(const int32_t uid)
{
    return AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice(uid).deviceType_;
}

std::string AudioActiveDevice::GetCurrentInputDeviceMacAddr(const int32_t uid)
{
    return AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice(uid).macAddress_;
}

DeviceType AudioActiveDevice::GetCurrentOutputDeviceType(const int32_t uid)
{
    return AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(uid).deviceType_;
}

DeviceCategory AudioActiveDevice::GetCurrentOutputDeviceCategory(const int32_t uid)
{
    return AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(uid).deviceCategory_;
}

std::string AudioActiveDevice::GetCurrentOutputDeviceNetworkId(const int32_t uid)
{
    return AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(uid).networkId_;
}

std::string AudioActiveDevice::GetCurrentOutputDeviceMacAddr(const int32_t uid)
{
    return AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(uid).macAddress_;
}

float AudioActiveDevice::GetMaxAmplitude(const int32_t deviceId, AudioInterrupt audioInterrupt)
{
    auto uid = IPCSkeleton::GetCallingUid();
    auto desc = audioDeviceManager_.FindConnectedDeviceById(deviceId);
    CHECK_AND_RETURN_RET(desc, 0);
    if (AudioRouterSelectStrategy::GetInstance().IsCurrentOutputDevice(deviceId, uid)) {
        uint32_t sessionId = audioInterrupt.streamId;
        std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkName(desc,
            static_cast<int32_t>(sessionId));
        std::string deviceClass = AudioPolicyUtils::GetInstance().GetOutputDeviceClassBySinkPortName(sinkName);
        return AudioServerProxy::GetInstance().GetMaxAmplitudeProxy(true, deviceClass);
    }
    if (AudioRouterSelectStrategy::GetInstance().IsCurrentInputDevice(deviceId, uid)) {
        std::string sourceName = AudioPolicyUtils::GetInstance().GetSourcePortName(
            desc->deviceType_);
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
    auto descs = AudioRouterSelectStrategy::GetInstance().FindCurrentOutputDevice(
        {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_NEARLINK, DEVICE_TYPE_BLUETOOTH_A2DP});
    for (auto &item : descs) {
        CHECK_AND_CONTINUE_LOG(item != nullptr, "item is nullptr");
        bool isSameDevice = item->IsSameDeviceDescPtr(audioDeviceDescriptor) ||
            item->IsSameBtDeviceDesc(audioDeviceDescriptor);
        NotifyUserDisSelectionEventToBt(item, isSameDevice, streamUsage);
    }

    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO ||
        audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        Bluetooth::SendUserSelectionEvent(audioDeviceDescriptor->deviceType_,
            audioDeviceDescriptor->macAddress_, USER_SELECT_SLE);
    }
    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(*audioDeviceDescriptor,
            streamUsage, USER_SELECT_SLE);
    }
#endif
}

void AudioActiveDevice::NotifyUserDisSelectionEventToBt(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
    bool isSameDevice, StreamUsage streamUsage)
{
    CHECK_AND_RETURN_LOG(!isSameDevice, "isSameDevice is true, do not notify");
    AUDIO_INFO_LOG("UserDisSelection start");
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "deviceDesc is nullptr");
#ifdef BLUETOOTH_ENABLE
    Bluetooth::SendUserSelectionEvent(
        audioDeviceDescriptor->deviceType_, audioDeviceDescriptor->macAddress_, USER_NOT_SELECT_SLE);
    auto descs = AudioRouterSelectStrategy::GetInstance().FindCurrentOutputDevice({DEVICE_TYPE_BLUETOOTH_SCO});
    if (!descs.empty() && audioDeviceDescriptor->IsSameDeviceDescPtr(*descs.begin())) {
        Bluetooth::AudioHfpManager::DisconnectSco();
    }
#endif
    SleAudioDeviceManager::GetInstance().SendUserSelection(*audioDeviceDescriptor, streamUsage, USER_NOT_SELECT_SLE);
}

void AudioActiveDevice::NotifyUserSelectionEventForInput(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
    SourceType sourceType)
{
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "audioDeviceDescriptor is nullptr");
#ifdef BLUETOOTH_ENABLE
    auto descs = AudioRouterSelectStrategy::GetInstance().FindCurrentInputDevice(
        {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_BLUETOOTH_A2DP_IN});
    for (auto &item : descs) {
        CHECK_AND_CONTINUE_LOG(item != nullptr, "item is nullptr");
        bool isSameDevice = item->IsSameDeviceDescPtr(audioDeviceDescriptor) ||
            item->IsSameBtDeviceDesc(audioDeviceDescriptor);
        NotifyUserDisSelectionEventToBt(item, isSameDevice);
    }

    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO ||
        audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        Bluetooth::SendUserSelectionEvent(audioDeviceDescriptor->deviceType_,
            audioDeviceDescriptor->macAddress_, USER_SELECT_SLE);
    }
    if (audioDeviceDescriptor->deviceType_ == DEVICE_TYPE_NEARLINK_IN) {
        SleAudioDeviceManager::GetInstance().SendUserSelection(*audioDeviceDescriptor,
            sourceType, USER_SELECT_SLE);
    }
#endif
}

void AudioActiveDevice::NotifyUserSelectionEventToRemote(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
    CHECK_AND_RETURN(desc->networkId_ != LOCAL_NETWORK_ID);
    UpdateActiveDeviceRoute(desc->deviceType_, DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG, DEFAULT_PIPE_ID,
        desc->networkId_);
}

void AudioActiveDevice::NotifyUserDisSelectionEventToRemote(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
    CHECK_AND_RETURN(desc->networkId_ != LOCAL_NETWORK_ID);
    ReleaseActiveDeviceRoute(desc->deviceType_, DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG, desc->networkId_);
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

bool AudioActiveDevice::IsDeviceActive(DeviceType deviceType)
{
    AUDIO_DEBUG_LOG("type [%{public}d]", deviceType);
    CHECK_AND_RETURN_RET(GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID, false);
    return GetCurrentOutputDeviceType() == deviceType;
}

void AudioActiveDevice::UpdateActiveDeviceRoute(DeviceType deviceType, DeviceFlag deviceFlag, const uint32_t pipeId,
    const std::string &networkId)
{
    Trace trace("KeyAction AudioActiveDevice::UpdateActiveDeviceRoute DeviceType:" + std::to_string(deviceType));
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back(make_pair(deviceType, deviceFlag));
    UpdateActiveDevicesRoute(activeDevices, pipeId, networkId);
}

void AudioActiveDevice::UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    const uint32_t pipeId, const std::string &networkId)
{
    CHECK_AND_RETURN_LOG(!activeDevices.empty(), "activeDevices is empty.");

    std::string deviceTypesInfo = "";
    for (size_t i = 0; i < activeDevices.size(); i++) {
        deviceTypesInfo = deviceTypesInfo + " " + std::to_string(activeDevices[i].first);
    }
    AUDIO_INFO_LOG("[PipeExecInfo] types[%{public}s]", deviceTypesInfo.c_str());

    Trace trace("AudioActiveDevice::UpdateActiveDevicesRoute DeviceTypes:" + deviceTypesInfo);
    auto ret = AudioServerProxy::GetInstance().UpdateActiveDevicesRouteProxy(activeDevices,
        audioA2dpOffloadFlag_.GetA2dpOffloadFlag(), pipeId, networkId);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the route for %{public}s", deviceTypesInfo.c_str());
}

void AudioActiveDevice::ReleaseActiveDeviceRoute(InternalDeviceType deviceType, DeviceFlag deviceFlag,
    const std::string &networkId)
{
    Trace trace("KeyAction AudioActiveDevice::ReleaseActiveDeviceRoute DeviceType:" + std::to_string(deviceType));
    auto ret = AudioServerProxy::GetInstance().ReleaseActiveDeviceRouteProxy(deviceType, deviceFlag, networkId);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to release the route for %{public}d", deviceType);
}

bool AudioActiveDevice::IsDeviceInVector(std::shared_ptr<AudioDeviceDescriptor> desc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "IsDeviceInVector: desc is null");
    for (auto &it : descs) {
        CHECK_AND_RETURN_RET(!it->IsSameDeviceDesc(desc), true);
    }
    return false;
}

void AudioActiveDevice::UpdateDeviceListWithStreamState(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "Device dsc is null");
    for (auto &it : descs) {
        CHECK_AND_CONTINUE(it != nullptr);
        CHECK_AND_CONTINUE(it->IsSameDeviceDesc(desc));
        CHECK_AND_RETURN(!it->hasStreamRunning_);
        it->hasStreamRunning_ |= desc->hasStreamRunning_;
        return;
    }
    descs.push_back(desc);
}

void AudioActiveDevice::UpdateVolumeTypeDeviceMap(std::shared_ptr<AudioStreamDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "streamDesc is null");
    CHECK_AND_RETURN_LOG(desc->newDeviceDescs_.front() != nullptr, "Devicedesc is null");

    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamUsage(desc->rendererInfo_.streamUsage);
    UpdateDeviceListWithStreamState(desc->newDeviceDescs_.front(), volumeTypeDeviceMap_[volumeType]);
}

void AudioActiveDevice::UpdateStreamUsageDeviceMap(std::shared_ptr<AudioStreamDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    CHECK_AND_RETURN_LOG(desc->newDeviceDescs_.front() != nullptr, "Devicedesc is null");
    CHECK_AND_RETURN(!IsDeviceInVector(desc->newDeviceDescs_.front(),
        streamUsageDeviceMap_[desc->rendererInfo_.streamUsage]));

    UpdateDeviceListWithStreamState(desc->newDeviceDescs_.front(),
        streamUsageDeviceMap_[desc->rendererInfo_.streamUsage]);
}

void AudioActiveDevice::UpdateStreamDeviceMap(std::string source)
{
    std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> descs =
        AudioPipeManager::GetPipeManager()->GetAllOutputStreamDescsInfo();
    descs.push_back(AudioPipeManager::GetPipeManager()->GetModemCommunicationStreamDescCopy());
    activeOutputDevices_.clear();
    volumeTypeDeviceMap_.clear();
    streamUsageDeviceMap_.clear();
    for (auto &desc : descs) {
        CHECK_AND_CONTINUE(desc != nullptr);
        CHECK_AND_CONTINUE(!desc->newDeviceDescs_.empty() && desc->newDeviceDescs_.front() != nullptr);
        desc->newDeviceDescs_.front()->hasStreamRunning_ = desc->IsRunning() ? true :
            desc->newDeviceDescs_.front()->hasStreamRunning_;
        AUDIO_INFO_LOG("session: %{public}d, calleruid: %{public}d, appuid: %{public}d " \
            "usage:%{public}d devices:%{public}s, status:%{public}d",
            desc->sessionId_, desc->callerUid_, desc->appInfo_.appUid,
            desc->rendererInfo_.streamUsage, desc->GetNewDevicesInfo().c_str(),
            desc->newDeviceDescs_.front()->hasStreamRunning_);
        UpdateVolumeTypeDeviceMap(desc);
        UpdateStreamUsageDeviceMap(desc);

        for (const auto &device : desc->newDeviceDescs_) {
            CHECK_AND_CONTINUE(!IsDeviceInVector(device, activeOutputDevices_));
            activeOutputDevices_.push_back(device);
        }
    }

    for (auto& [volumeType, deviceList] : volumeTypeDeviceMap_) {
        SortDevicesByPriority(deviceList);
    }

    for (auto& [volumeType, deviceList] : streamUsageDeviceMap_) {
        SortDevicesByPriority(deviceList);
    }
}

bool AudioActiveDevice::IsDeviceInActiveOutputDevices(DeviceType type, bool isRemote)
{
    std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
    auto isExist = std::find_if(activeOutputDevices_.begin(), activeOutputDevices_.end(),
        [type, isRemote](std::shared_ptr<AudioDeviceDescriptor> &device) {
            return device->deviceType_ == type &&
                isRemote == (device->networkId_ != LOCAL_NETWORK_ID);
        });
    return isExist != activeOutputDevices_.end();
}

bool AudioActiveDevice::IsDeviceInActiveOutputDevices(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
    auto isExist = std::find_if(activeOutputDevices_.begin(), activeOutputDevices_.end(),
        [desc](std::shared_ptr<AudioDeviceDescriptor> &device) {
            return desc->IsSameDeviceDesc(device);
        });
    return isExist != activeOutputDevices_.end();
}

void AudioActiveDevice::SortDevicesByPriority(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    bool hasAdjustVolumeZone = GetAdjustVolumeZoneId() != 0;
    std::sort(descs.begin(), descs.end(), [this, hasAdjustVolumeZone] (
        const std::shared_ptr<AudioDeviceDescriptor> &a, const std::shared_ptr<AudioDeviceDescriptor> &b) {
        if (!a || !b) {
            return a < b;
        }
        if (hasAdjustVolumeZone) {
            bool isDeviceInAudioZoneA = AudioZoneService::GetInstance().CheckDeviceInAudioZone(*a);
            bool isDeviceInAudioZoneB = AudioZoneService::GetInstance().CheckDeviceInAudioZone(*b);
            CHECK_AND_RETURN_RET(isDeviceInAudioZoneA == isDeviceInAudioZoneB, isDeviceInAudioZoneA);
        }
        CHECK_AND_RETURN_RET(a->hasStreamRunning_ == b->hasStreamRunning_, a->hasStreamRunning_);
        return audioDeviceManager_.GetDevicePriority(a) < audioDeviceManager_.GetDevicePriority(b);
    });
}

bool AudioActiveDevice::IsAvailableFrontDeviceInVector(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs)
{
    if (descs.empty()) {
        return false;
    }
    if (descs.front() == nullptr) {
        return false;
    }
    return true;
}

void AudioActiveDevice::SetAdjustVolumeForZone(int32_t zoneId)
{
    volumeAdjustZoneId_ = zoneId;
    UpdateStreamDeviceMap("SetAdjustVolumeForZone");
}

int32_t AudioActiveDevice::GetAdjustVolumeZoneId()
{
    return volumeAdjustZoneId_;
}

std::shared_ptr<AudioDeviceDescriptor> AudioActiveDevice::GetDeviceForVolume(AudioVolumeType volumeType)
{
    CHECK_AND_RETURN_RET_LOG(!audioConnectedDevice_.IsEmpty(), defaultOutputDevice_, "no device connected");
    AudioVolumeType type = VolumeUtils::GetVolumeTypeFromStreamType(volumeType);
    if (type == STREAM_ALL) {
        type = STREAM_MUSIC;
    }
    {
        std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
        if (volumeTypeDeviceMap_.contains(type)
            && IsAvailableFrontDeviceInVector(volumeTypeDeviceMap_[type])) {
            return volumeTypeDeviceMap_[type].front();
        }
    }

    std::vector<StreamUsage> usages = VolumeUtils::GetStreamUsageByVolumeTypeForFetchDevice(type);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    for (auto usage : usages) {
        FetchDeviceInfo info = {usage, "GetDeviceForVolumeByStreamType"};
        devices.push_back(AudioRouterCenter::GetAudioRouterCenter().FetchOutputDevices(info).front());
    }
    SortDevicesByPriority(devices);
    AUDIO_INFO_LOG("Get Device %{public}s for stream %{public}d from router and add to map",
        devices.front()->GetName().c_str(), type);
    {
        std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
        volumeTypeDeviceMap_[type] = devices;
    }
    return devices.front();
}

std::shared_ptr<AudioDeviceDescriptor> AudioActiveDevice::GetDeviceForVolume(StreamUsage usage)
{
    CHECK_AND_RETURN_RET_LOG(!audioConnectedDevice_.IsEmpty(), defaultOutputDevice_, "no device connected");
    {
        std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
        if (streamUsageDeviceMap_.contains(usage)
            && IsAvailableFrontDeviceInVector(streamUsageDeviceMap_[usage])) {
            AUDIO_INFO_LOG("Get Device %{public}s for stream %{public}d from map",
                streamUsageDeviceMap_[usage].front()->GetName().c_str(), usage);
            return streamUsageDeviceMap_[usage].front();
        }
    }
    FetchDeviceInfo info = {usage, "GetDeviceForVolumeByStreamUsage"};
    return AudioRouterCenter::GetAudioRouterCenter().FetchOutputDevices(info).front();
}

std::shared_ptr<AudioDeviceDescriptor> AudioActiveDevice::GetDeviceForVolume()
{
    CHECK_AND_RETURN_RET_LOG(!audioConnectedDevice_.IsEmpty(), defaultOutputDevice_, "no device connected");
    std::vector<StreamUsage> typeList = {
        STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
        STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_VIDEO_COMMUNICATION,
        STREAM_USAGE_INTERPHONE,
        STREAM_USAGE_VOICE_MESSAGE,
        STREAM_USAGE_NOTIFICATION,
        STREAM_USAGE_VOICE_ASSISTANT,
        STREAM_USAGE_RINGTONE,
        STREAM_USAGE_ALARM,
        STREAM_USAGE_NAVIGATION,
        STREAM_USAGE_MUSIC,
        STREAM_USAGE_MOVIE,
        STREAM_USAGE_AUDIOBOOK,
        STREAM_USAGE_GAME,
        STREAM_USAGE_DTMF,
        STREAM_USAGE_SYSTEM,
        STREAM_USAGE_ENFORCED_TONE,
#ifdef MULTI_ALARM_LEVEL
        STREAM_USAGE_ANNOUNCEMENT,
        STREAM_USAGE_EMERGENCY
#endif
    };
    {
        std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
        for (StreamUsage usage : typeList) {
            CHECK_AND_CONTINUE(streamUsageDeviceMap_.contains(usage)
                && IsAvailableFrontDeviceInVector(streamUsageDeviceMap_[usage]));
            return streamUsageDeviceMap_[usage].front();
        }
    }
    FetchDeviceInfo info = {STREAM_USAGE_MUSIC, "GetDeviceForVolumeByTopPriority"};
    return AudioRouterCenter::GetAudioRouterCenter().FetchOutputDevices(info).front();
}

std::shared_ptr<AudioDeviceDescriptor> AudioActiveDevice::GetDeviceForVolume(int32_t appUid)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> descs =
        AudioPipeManager::GetPipeManager()->GetAllOutputStreamDescsInfo();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> tmp;
    for (auto desc : descs) {
        CHECK_AND_CONTINUE(desc != nullptr);
        CHECK_AND_CONTINUE(GetRealUid(desc) == appUid);
        tmp.push_back(desc->newDeviceDescs_.front());
    }
    CHECK_AND_RETURN_RET_LOG(!tmp.empty(), GetDeviceForVolume(),
        "no uid %{public}d in active", appUid);
    SortDevicesByPriority(tmp);
    return tmp.front();
}

std::shared_ptr<AudioDeviceDescriptor> AudioActiveDevice::GetActiveDeviceForVolume(int32_t appUid)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> descs =
        AudioPipeManager::GetPipeManager()->GetAllOutputStreamDescsInfo();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> tmp;
    for (auto desc : descs) {
        CHECK_AND_CONTINUE(desc != nullptr && GetRealUid(desc) == appUid);
        tmp.push_back(desc->newDeviceDescs_.front());
    }
    CHECK_AND_RETURN_RET_LOG(!tmp.empty(), nullptr, "no uid %{public}d in active", appUid);
    SortDevicesByPriority(tmp);
    return tmp.front();
}

int32_t AudioActiveDevice::GetRealUid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, -1, "Stream desc is nullptr");
    if (streamDesc->callerUid_ == MEDIA_SERVICE_UID) {
        return streamDesc->appInfo_.appUid;
    }
    return streamDesc->callerUid_;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioActiveDevice::GetActiveOutputDevices()
{
    std::lock_guard<std::mutex> lock(deviceForVolumeMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> copy;
    copy.reserve(activeOutputDevices_.size()); // 预留空间，提高效率

    for (const auto& devicePtr : activeOutputDevices_) {
        CHECK_AND_CONTINUE(devicePtr != nullptr);
        auto newDevice = std::make_shared<AudioDeviceDescriptor>(*devicePtr);
        copy.push_back(newDevice);
    }
    return copy;
}
}
}
