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
#undef LOG_TAG
#define LOG_TAG "AudioZone"

#include "audio_info.h"
#include "audio_zone.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
static int32_t GenerateZoneId()
{
    static int32_t genId = 1;
    static std::mutex genLock;
    std::unique_lock<std::mutex> lock(genLock);
    int32_t id = genId;
    genId++;
    if (genId <= 0) {
        genId = 1;
    }
    return id;
}

AudioZoneBindKey::AudioZoneBindKey(int32_t uid)
    : uid_(uid)
{
}

AudioZoneBindKey::AudioZoneBindKey(uint32_t streamId)
    : streamId_(streamId)
{
}

AudioZoneBindKey::AudioZoneBindKey(int32_t uid, StreamUsage streamUsage)
    : uid_(uid),
      usage_(streamUsage)
{
}

AudioZoneBindKey::AudioZoneBindKey(int32_t uid, const std::string &deviceTag)
    : uid_(uid),
      deviceTag_(deviceTag)
{
}

AudioZoneBindKey::AudioZoneBindKey(int32_t uid, const std::string &deviceTag, uint32_t streamId,
    const StreamUsage &usage)
    : uid_(uid),
      deviceTag_(deviceTag),
      streamId_(streamId),
      usage_(usage)
{
}

AudioZoneBindKey::AudioZoneBindKey(const AudioZoneBindKey &other)
{
    Assign(other);
}

AudioZoneBindKey::AudioZoneBindKey(AudioZoneBindKey &&other)
{
    Swap(std::move(other));
}

AudioZoneBindKey &AudioZoneBindKey::operator=(const AudioZoneBindKey &other)
{
    Assign(other);
    return *this;
}

AudioZoneBindKey &AudioZoneBindKey::operator=(AudioZoneBindKey &&other)
{
    Swap(std::move(other));
    return *this;
}

bool AudioZoneBindKey::operator==(const AudioZoneBindKey &other) const
{
    return this->uid_ == other.uid_ &&
        this->deviceTag_ == other.deviceTag_ &&
        this->streamId_ == other.streamId_ &&
        this->usage_ == other.usage_;
}

bool AudioZoneBindKey::operator!=(const AudioZoneBindKey &other) const
{
    return !(*this == other);
}

void AudioZoneBindKey::Assign(const AudioZoneBindKey &other)
{
    this->uid_ = other.uid_;
    this->deviceTag_ = other.deviceTag_;
    this->streamId_ = other.streamId_;
    this->usage_ = other.usage_;
}

void AudioZoneBindKey::Swap(AudioZoneBindKey &&other)
{
    this->uid_ = other.uid_;
    this->deviceTag_ = other.deviceTag_;
    this->streamId_ = other.streamId_;
    this->usage_ = other.usage_;
}

int32_t AudioZoneBindKey::GetUid() const
{
    return uid_;
}

uint32_t AudioZoneBindKey::GetStreamId() const
{
    return streamId_;
}

const std::string AudioZoneBindKey::GetString() const
{
    std::string str = "uid=";
    str += std::to_string(uid_);
    str += ",deviceTag=";
    str += deviceTag_;
    str += ",streamId=";
    str += std::to_string(streamId_);
    str += ",usage_=";
    str += std::to_string(usage_);
    return str;
}

bool AudioZoneBindKey::IsContain(const AudioZoneBindKey &other) const
{
    std::vector<AudioZoneBindKey> supportKeys = GetSupportKeys(other);
    int32_t index = -1;
    int32_t otherIndex = -1;
    for (int32_t i = 0; i < static_cast<int32_t>(supportKeys.size()); i++) {
        if (supportKeys[i] == *this) {
            index = i;
        }

        if (supportKeys[i] == other) {
            otherIndex = i;
        }
    }
    CHECK_AND_RETURN_RET(index != -1, false);
    return index >= otherIndex;
}

const std::vector<AudioZoneBindKey> AudioZoneBindKey::GetSupportKeys(int32_t uid, const std::string &deviceTag,
    uint32_t streamId, const StreamUsage &usage)
{
    std::vector<AudioZoneBindKey> keys;
    keys.push_back(AudioZoneBindKey(uid, deviceTag, streamId, StreamUsage::STREAM_USAGE_INVALID));
    auto pushBack = [&keys](const AudioZoneBindKey &temp) {
        for (auto &key : keys) {
            CHECK_AND_RETURN(key != temp);
        }
        keys.push_back(temp);
    };
    if (streamId != INVALID_STREAM_ID) {
        pushBack(AudioZoneBindKey(streamId));
    }
    pushBack(AudioZoneBindKey(uid, ""));
    pushBack(AudioZoneBindKey(uid, usage));
    pushBack(AudioZoneBindKey(uid));
    pushBack(AudioZoneBindKey(uid, deviceTag));
    pushBack(AudioZoneBindKey(INVALID_UID, "", INVALID_STREAM_ID, usage));
    return keys;
}

const std::vector<AudioZoneBindKey> AudioZoneBindKey::GetSupportKeys(const AudioZoneBindKey &key)
{
    int32_t uid = key.uid_;
    std::string deviceTag = key.deviceTag_;
    uint32_t streamId = key.streamId_;
    StreamUsage usage = key.usage_;
    return GetSupportKeys(uid, deviceTag, streamId, usage);
}

AudioZone::AudioZone(std::shared_ptr<AudioZoneClientManager> manager,
    const std::string &name, const AudioZoneContext &context, pid_t clientPid)
    : zoneId_(GenerateZoneId()),
      name_(name),
      clientManager_(manager),
      zoneClientPid_(clientPid),
      context_(context)
{
}

int32_t AudioZone::GetId()
{
    return zoneId_;
}

void AudioZone::UpdateContext(const AudioZoneContext &context)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    context_ = context;
}

bool AudioZone::IsSupportDeviceRole(DeviceRole deviceRole)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    if (deviceRole == DEVICE_ROLE_NONE) {
        return true;
    }

    switch (context_.supportType_) {
        case AudioZoneSupportType::PLAY_ONLY:
            return deviceRole == OUTPUT_DEVICE;
        case AudioZoneSupportType::RECORD_ONLY:
            return deviceRole == INPUT_DEVICE;
        case AudioZoneSupportType::PLAY_AND_RECORD:
        default:
            return true;
    }
}

bool AudioZone::IsVolumeProxyEnable(DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return volumeProxyDeviceTypes_.count(deviceType);
}

const std::shared_ptr<AudioZoneDescriptor> AudioZone::GetDescriptor()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return GetDescriptorNoLock();
}

const std::string AudioZone::GetStringDescriptor()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    std::string str;
    str += "zone name is";
    str += name_;
    str += "\n";
    for (auto &key : keys_) {
        str += "bing key ";
        str += key.GetString();
        str += "\n";
    }
    return str;
}

const std::string AudioZone::GetName()
{
    return name_;
}

int32_t AudioZone::GetUserId()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return context_.userId_;
}

const std::shared_ptr<AudioZoneDescriptor> AudioZone::GetDescriptorNoLock()
{
    std::shared_ptr<AudioZoneDescriptor> descriptor = std::make_shared<AudioZoneDescriptor>();
    CHECK_AND_RETURN_RET_LOG(descriptor != nullptr, nullptr, "descriptor is nullptr");

    descriptor->zoneId_ = zoneId_;
    descriptor->name_ = name_;
    descriptor->initMode_ = static_cast<int32_t>(context_.initMode_);
    descriptor->supportType_ = static_cast<int32_t>(context_.supportType_);
    AUDIO_INFO_LOG("zone %{public}d init mode %{public}d", zoneId_, descriptor->initMode_);

    for (const auto &key : keys_) {
        descriptor->uids_.insert(key.GetUid());
        uint32_t streamId = key.GetStreamId();
        if (streamId != INVALID_STREAM_ID) {
            descriptor->streamIds_.insert(streamId);
        }
    }
    for (const auto &it : devices_) {
        descriptor->devices_.emplace_back(it.first);
    }
    return descriptor;
}

void AudioZone::BindByKey(const AudioZoneBindKey &key)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (auto itKey = keys_.begin(); itKey != keys_.end();) {
        if (key.IsContain(*itKey)) {
            AUDIO_INFO_LOG("erase high key %{public}s to %{public}s for zone %{public}d",
                itKey->GetString().c_str(), key.GetString().c_str(), zoneId_);
            keys_.erase(itKey++);
        } else {
            ++itKey;
        }
    }
    keys_.emplace_back(key);
    AUDIO_INFO_LOG("bind key %{public}s to zone %{public}d", key.GetString().c_str(), zoneId_);
    for (auto &temp : keys_) {
        AUDIO_DEBUG_LOG("zone %{public}d bind key %{public}s", zoneId_, temp.GetString().c_str());
    }
    SendZoneChangeEvent(AudioZoneChangeReason::BIND_NEW_APP);
}

void AudioZone::RemoveKey(const AudioZoneBindKey &key)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto itKey = std::find(keys_.begin(), keys_.end(), key);
    CHECK_AND_RETURN_LOG(itKey != keys_.end(), "key %{public}s not exist for zone %{public}d",
        key.GetString().c_str(), zoneId_);

    AUDIO_INFO_LOG("remove key %{public}s for zone %{public}d", key.GetString().c_str(), zoneId_);
    keys_.erase(itKey);
    for (auto &temp : keys_) {
        AUDIO_DEBUG_LOG("zone %{public}d bind key %{public}s", zoneId_, temp.GetString().c_str());
    }
    SendZoneChangeEvent(AudioZoneChangeReason::UNBIND_APP);
}

bool AudioZone::IsContainKey(const AudioZoneBindKey &key)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &it : keys_) {
        if (it == key) {
            return true;
        }
    }
    return false;
}

int32_t AudioZone::AddDeviceDescriptor(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &device : devices) {
        CHECK_AND_RETURN_RET_LOG(device != nullptr, ERR_INVALID_PARAM, "device is nullptr");
        auto findDevice = [&device] (const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item) {
            return device->IsSameDeviceDesc(*(item.first));
        };
        
        auto itDev = std::find_if(devices_.begin(), devices_.end(), findDevice);
        if (itDev != devices_.end()) {
            AUDIO_WARNING_LOG("add duplicate  device %{public}d,%{public}d,%{public}s to zone %{public}d",
                device->deviceType_, device->deviceId_, device->deviceName_.c_str(), zoneId_);
        } else {
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectDevices;
            AudioConnectedDevice::GetInstance().GetAllConnectedDeviceByType(device->networkId_,
                device->deviceType_, device->macAddress_, device->deviceRole_, connectDevices);
            std::shared_ptr<AudioDeviceDescriptor> target =
                connectDevices.size() != 0 ? connectDevices.front() : device;
            devices_.emplace_back(std::make_pair(target, connectDevices.size() != 0));
            AUDIO_INFO_LOG("add device %{public}d,%{public}d,%{public}s to zone %{public}d",
                target->deviceType_, target->deviceId_, target->deviceName_.c_str(), zoneId_);
        }
    }
    return SUCCESS;
}

int32_t AudioZone::RemoveDeviceDescriptor(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &device : devices) {
        CHECK_AND_RETURN_RET_LOG(device != nullptr, ERR_INVALID_PARAM, "device is nullptr");
        auto findDevice = [device] (const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item) {
            return device->IsSameDeviceDesc(*(item.first));
        };

        auto itDev = std::find_if(devices_.begin(), devices_.end(), findDevice);
        CHECK_AND_CONTINUE(itDev != devices_.end());

        devices_.erase(itDev);
        AUDIO_INFO_LOG("remove device %{public}d,%{public}d,%{public}s from zone %{public}d",
            device->deviceType_, device->deviceId_, device->deviceName_.c_str(), zoneId_);
    }
    return SUCCESS;
}

int32_t AudioZone::UpdateDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> device)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERR_INVALID_PARAM, "device is nullptr");
    auto findDevice = [&device] (const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item) {
        return device->IsSameDeviceDesc(*(item.first));
    };
    auto itDev = std::find_if(devices_.begin(), devices_.end(), findDevice);
    CHECK_AND_RETURN_RET_LOG(itDev != devices_.end(), ERROR,
        "update device %{public}d,%{public}d,%{public}s not exist for zone %{public}d",
        device->deviceType_, device->deviceId_, device->deviceName_.c_str(), zoneId_);

    devices_.erase(itDev);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectDevices;
        AudioConnectedDevice::GetInstance().GetAllConnectedDeviceByType(device->networkId_,
            device->deviceType_, device->macAddress_, device->deviceRole_, connectDevices);
    devices_.emplace_back(std::make_pair(device, connectDevices.size() != 0));
    AUDIO_INFO_LOG("add device %{public}d,%{public}d,%{public}s to zone %{public}d",
        device->deviceType_, device->deviceId_, device->deviceName_.c_str(), zoneId_);
    return SUCCESS;
}

int32_t AudioZone::EnableDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> device)
{
    return SetDeviceDescriptorState(device, true);
}

int32_t AudioZone::DisableDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> device)
{
    return SetDeviceDescriptorState(device, false);
}

int32_t AudioZone::SetDeviceDescriptorState(const std::shared_ptr<AudioDeviceDescriptor> device, const bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is nullptr");
    auto findDevice = [device] (const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item) {
        return device->IsSameDeviceDesc(*(item.first));
    };

    auto itDev = std::find_if(devices_.begin(), devices_.end(), findDevice);
    CHECK_AND_RETURN_RET_LOG(itDev != devices_.end(), ERROR,
        "device %{public}d,%{public}d,%{public}s not exist for zone %{public}d",
        device->deviceType_, device->deviceId_, device->deviceName_.c_str(), zoneId_);

    itDev->second = enable;
    AUDIO_INFO_LOG("%{public}s device %{public}d,%{public}d,%{public}s of zone %{public}d",
        enable ? "enable" : "disable", device->deviceType_, device->deviceId_, device->deviceName_.c_str(), zoneId_);
    return SUCCESS;
}

bool AudioZone::IsDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> device)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, false, "device is nullptr");
    auto findDevice = [device] (const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item) {
        return device->IsSameDeviceDesc(*(item.first));
    };
    auto itDev = std::find_if(devices_.begin(), devices_.end(), findDevice);
    CHECK_AND_RETURN_RET(itDev != devices_.end(), false);

    return itDev->second;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioZone::FetchOutputDevices(StreamUsage streamUsage,
    int32_t clientUid, const RouterType &bypassType)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &device : devices_) {
        if (device.second && device.first->deviceRole_ == OUTPUT_DEVICE) {
            descs.emplace_back(device.first);
            return descs;
        }
    }
    return descs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioZone::GetAllOutputDevices()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    for (const auto &device : devices_) {
        if (device.first->deviceRole_ == OUTPUT_DEVICE) {
            devices.emplace_back(device.first);
        }
    }
    return devices;
}

std::unordered_set<DeviceType> AudioZone::GetAllOutputDeviceTypes()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return GetAllOutputDeviceTypesNoLock();
}

std::unordered_set<DeviceType> AudioZone::GetAllOutputDeviceTypesNoLock()
{
    std::unordered_set<DeviceType> deviceTypes;
    for (const auto &device : devices_) {
        if (device.first->deviceRole_ == OUTPUT_DEVICE) {
            deviceTypes.insert(device.first->deviceType_);
        }
    }
    return deviceTypes;
}

std::shared_ptr<AudioDeviceDescriptor> AudioZone::FetchInputDevice(SourceType sourceType, int32_t clientUid)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &device : devices_) {
        if (device.second && device.first->deviceRole_ == INPUT_DEVICE) {
            return device.first;
        }
    }
    return nullptr;
}

int32_t AudioZone::EnableChangeReport(pid_t clientPid, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    if (enable) {
        changeReportClientList_.insert(clientPid);
    } else {
        changeReportClientList_.erase(clientPid);
    }
    AUDIO_INFO_LOG(" %{public}s zone %{public}d change report to client %{public}d",
        enable ? "enable" : "disable", zoneId_, clientPid);
    return SUCCESS;
}

void AudioZone::SendZoneChangeEvent(AudioZoneChangeReason reason)
{
    CHECK_AND_RETURN_LOG(clientManager_ != nullptr, "clientManager is nullptr");
    for (auto &pid : changeReportClientList_) {
        clientManager_->SendZoneChangeEvent(pid, this->GetDescriptorNoLock(), reason);
    }
}

int32_t AudioZone::EnableSystemVolumeProxy(pid_t clientPid, DeviceType deviceType, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    volumeProxyClientPid_ = clientPid;
    if (deviceType == DeviceType::DEVICE_TYPE_NONE || deviceType == DeviceType::DEVICE_TYPE_INVALID) {
        if (enable) {
            volumeProxyDeviceTypes_ = GetAllOutputDeviceTypesNoLock();
        } else {
            volumeProxyDeviceTypes_.clear();
        }
    } else {
        if (enable) {
            volumeProxyDeviceTypes_.insert(deviceType);
        } else {
            size_t count = volumeProxyDeviceTypes_.erase(deviceType);
            CHECK_AND_RETURN_RET_LOG(count > 0, ERROR,
                "zone %{public}d volume proxy device type %{public}d is not enabled", zoneId_, deviceType);
        }
    }
    AUDIO_INFO_LOG("device type %{public}d volume proxy is %{public}s by %{public}d",
        deviceType, enable ? "enable" : "disable", clientPid);
    return SUCCESS;
}

int32_t AudioZone::SetSystemVolumeLevel(DeviceType deviceType, AudioVolumeType volumeType,
    const VolumeScale &volume, int32_t volumeFlag)
{
    std::shared_ptr<AudioZoneClientManager> mgr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(clientManager_ != nullptr, ERROR, "clientManager is nullptr");
        CHECK_AND_RETURN_RET_LOG(volumeProxyDeviceTypes_.count(deviceType), ERROR, "volume proxy is not enable");

        mgr = clientManager_;
    }
    return mgr->SetSystemVolumeLevel(volumeProxyClientPid_, zoneId_, deviceType,
        volumeType, volume, volumeFlag);
}

int32_t AudioZone::GetSystemVolumeLevel(DeviceType deviceType, AudioVolumeType volumeType)
{
    std::shared_ptr<AudioZoneClientManager> mgr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(clientManager_ != nullptr, ERROR, "clientManager is nullptr");
        CHECK_AND_RETURN_RET_LOG(volumeProxyDeviceTypes_.count(deviceType), ERROR, "volume proxy is not enable");

        mgr = clientManager_;
    }
    return mgr->GetSystemVolumeLevel(volumeProxyClientPid_, zoneId_, deviceType, volumeType);
}

int32_t AudioZone::GetSystemVolumeDegree(DeviceType deviceType, AudioVolumeType volumeType)
{
    std::shared_ptr<AudioZoneClientManager> mgr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(clientManager_ != nullptr, ERROR, "clientManager is nullptr");
        CHECK_AND_RETURN_RET_LOG(volumeProxyDeviceTypes_.count(deviceType), ERROR, "volume proxy is not enable");

        mgr = clientManager_;
    }
    return mgr->GetSystemVolumeDegree(volumeProxyClientPid_, zoneId_, deviceType, volumeType);
}

pid_t AudioZone::GetClientPid()
{
    return zoneClientPid_;
}

bool AudioZone::CheckDeviceInZone(AudioDeviceDescriptor device)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &it : devices_) {
        if (it.second && it.first->deviceType_ == device.deviceType_ &&
            it.first->networkId_ == device.networkId_) {
            return true;
        }
    }
    return false;
}

bool AudioZone::HasVolumeControllableDeviceInZone()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &it : devices_) {
        CHECK_AND_RETURN_RET(it.first == nullptr || !it.second ||
            it.first->volumeBehavior_.isVolumeControlDisabled, true);
    }
    return false;
}

std::shared_ptr<AudioDeviceDescriptor> AudioZone::GetDeviceDescriptor(DeviceType type, std::string networkId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &it : devices_) {
        if (it.second && it.first->deviceType_ == type &&
            it.first->networkId_ == networkId) {
            return it.first;
        }
    }
    AUDIO_ERR_LOG("device not found, deviceType: %{public}d", type);
    return nullptr;
}
} // namespace AudioStandard
} // namespace OHOS