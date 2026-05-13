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
#define LOG_TAG "AudioZoneService"

#include "audio_zone_service.h"
#include "audio_log.h"
#include "audio_info.h"
#include "audio_errors.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_zone_interrupt_reporter.h"
#include "audio_device_lock.h"
#include "audio_connected_device.h"
#include "audio_core_service.h"
#include "audio_device_manager.h"
#include "audio_connected_device.h"
#include "audio_volume_manager.h"
#include "stream_dfx_manager.h"
#include <unistd.h>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {

AudioZoneService& AudioZoneService::GetInstance()
{
    static AudioZoneService service;
    return service;
}

void AudioZoneService::Init(std::shared_ptr<AudioPolicyServerHandler> handler,
    std::shared_ptr<AudioInterruptService> interruptService)
{
    CHECK_AND_RETURN_LOG(handler != nullptr && interruptService != nullptr,
        "handler or interruptService is nullptr");
    interruptService_ = interruptService;
    zoneClientManager_ = std::make_shared<AudioZoneClientManager>(handler);
    CHECK_AND_RETURN_LOG(zoneClientManager_ != nullptr, "create audio zone client manager failed");
    handler->SetAudioZoneEventDispatcher(zoneClientManager_);
    sessionTimeoutCallbackPtr_ = std::shared_ptr<SessionTimeOutCallback>(this, [](SessionTimeOutCallback*) {});
    OHOS::Singleton<AudioSessionService>::GetInstance().SetSessionTimeOutCallback(sessionTimeoutCallbackPtr_);
}

void AudioZoneService::DeInit()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    zoneMaps_.clear();
    zoneReportClientList_.clear();
    mainZoneContext_ = {};
    AudioZoneInterruptReporter::DisableAllInterruptReport();
    zoneClientManager_ = nullptr;
    interruptService_ = nullptr;
    sessionTimeoutCallbackPtr_ = nullptr;
}

int32_t AudioZoneService::CreateAudioZone(const std::string &name, const AudioZoneContext &context, pid_t clientPid)
{
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(zoneClientManager_, name, context, clientPid);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone is nullptr");
    int32_t zoneId = zone->GetId();
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(zoneMaps_.find(zoneId) == zoneMaps_.end(),
            ERROR, "zone id %{public}d is duplicate", zoneId);

        zoneMaps_[zoneId] = zone;

        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "zoneClientManager_ is nullptr");
        for (auto &pid : zoneReportClientList_) {
            zoneClientManager_->SendZoneAddEvent(pid, zone->GetDescriptor());
        }
        tmp = interruptService_;
    }
    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, ERROR,
        HILOG_COMM_ERROR("[CreateAudioZone]interruptService_ tmp is nullptr"));
    tmp->CreateAudioInterruptZone(zoneId, context);
    AUDIO_INFO_LOG("create zone id %{public}d, name %{public}s, supportType: %{public}d, userId: %{public}d", zoneId,
        name.c_str(), context.supportType_, context.userId_);

    return zoneId;
}

int32_t AudioZoneService::GetAudioZoneForApp(const std::vector<int32_t> &userIds, std::vector<int32_t> &retUserIds)
{
    retUserIds.clear();
    retUserIds.reserve(userIds.size());

    std::lock_guard<std::mutex> lock(zoneMutex_);

    std::string userIdsStr = "";
    std::string retUserIdsStr = "";
    for (const auto &userId : userIds) {
        userIdsStr += std::to_string(userId) + ",";
        auto findZone = [userId] (const std::pair<int32_t, std::shared_ptr<AudioZone>> &item) {
            return item.second != nullptr && item.second->GetUserId() == userId;
        };
        auto itZone = std::find_if(zoneMaps_.begin(), zoneMaps_.end(), findZone);
        retUserIds.emplace_back(itZone != zoneMaps_.end() ? userId : mainZoneContext_.userId_);
        retUserIdsStr += std::to_string(retUserIds.back()) + ",";
    }
    AUDIO_INFO_LOG("userIds: %{public}s, retUserIds: %{public}s", userIdsStr.c_str(), retUserIdsStr.c_str());
    return SUCCESS;
}

void AudioZoneService::ReleaseAudioZone(int32_t zoneId)
{
    if (interruptService_ != nullptr) {
        std::vector<int32_t> sessionUidList = interruptService_->GetAudioSessionUidList(zoneId);
        for (auto uid : sessionUidList) {
            RemoveUidFromAudioZone(zoneId, uid);
        }
    }
    AudioVolumeManager &volumeManager = AudioVolumeManager::GetInstance();
    volumeManager.SetAdjustVolumeForZone(0);
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_LOG(zoneMaps_.find(zoneId) != zoneMaps_.end(),
            "zone id %{public}d is not found", zoneId);

        zoneMaps_.erase(zoneId);
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_LOG(tmp != nullptr, "interruptService tmp is nullptr");

    auto reporters = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::RELEASE_AUDIO_ZONE);
    tmp->ReleaseAudioInterruptZone(zoneId,
        [this](int32_t uid, const std::string &deviceTag, uint32_t streamId,
            const StreamUsage &usage, DeviceRole deviceRole)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamId, usage, deviceRole);
    });
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }

    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_LOG(zoneClientManager_ != nullptr, "zoneClientManager_ is nullptr");
        for (auto &pid : zoneReportClientList_) {
            zoneClientManager_->SendZoneRemoveEvent(pid, zoneId);
        }
        AUDIO_INFO_LOG("release zone id %{public}d", zoneId);
    }
}

const std::vector<std::shared_ptr<AudioZoneDescriptor>> AudioZoneService::GetAllAudioZone()
{
    std::vector<std::shared_ptr<AudioZoneDescriptor>> zoneDescriptor;
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &it : zoneMaps_) {
        CHECK_AND_CONTINUE_LOG(it.second != nullptr, "zone is nullptr");
        zoneDescriptor.emplace_back(it.second->GetDescriptor());
    }
    return zoneDescriptor;
}

const std::shared_ptr<AudioZoneDescriptor> AudioZoneService::GetAudioZone(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, nullptr, "zone id %{public}d is not found", zoneId);
    return zone->GetDescriptor();
}

int32_t AudioZoneService::GetAudioZoneByName(std::string name)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (const auto &it : zoneMaps_) {
        CHECK_AND_CONTINUE_LOG(it.second != nullptr, "zone is nullptr");
        CHECK_AND_CONTINUE(it.second->GetName() == name);
        AUDIO_INFO_LOG("find zone %{public}d by name: %{public}s", it.first, name.c_str());
        return it.first;
    }
    return ERROR;
}

std::string AudioZoneService::GetZoneNameById(int32_t zoneId)
{
    if (zoneId == 0) {
        return std::string(PRIMARY_ZONE_NAME);
    }
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, "", "zone id %{public}d is not found", zoneId);
    return zone->GetName();
}

int32_t AudioZoneService::BindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);

        int ret = zone->AddDeviceDescriptor(devices);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "bind device to zone failed");
    }

    for (auto device : devices) {
        RemoveDeviceFromGlobal(device);
    }
    return SUCCESS;
}

void AudioZoneService::RemoveDeviceFromGlobal(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is nullptr");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectDevices;
    AudioConnectedDevice::GetInstance().GetAllConnectedDeviceByType(device->networkId_,
        device->deviceType_, device->macAddress_, device->deviceRole_, connectDevices);
    CHECK_AND_RETURN_LOG(connectDevices.size() != 0, "connectDevices is empty.");
    AudioDeviceStatus::GetInstance().RemoveDeviceFromGlobalOnly(device);
}

int32_t AudioZoneService::UnBindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> toGlobalDevices;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);

        for (auto it : devices) {
            CHECK_AND_CONTINUE_LOG(it != nullptr, "device is nullptr");
            CHECK_AND_CONTINUE(zone->IsDeviceConnect(it));
            auto temp = zone->GetDeviceDescriptor(it->deviceType_, it->networkId_);
            CHECK_AND_CONTINUE_LOG(temp != nullptr, "temp is not found");
            toGlobalDevices.push_back(it);
        }
        zone->RemoveDeviceDescriptor(devices);
    }
    // maybe whether or not add unbind devices to global is specified by caller
    for (auto it : toGlobalDevices) {
        AudioDeviceStatus::GetInstance().AddDeviceBackToGlobalOnly(it);
    }
    return SUCCESS;
}

void AudioZoneService::MoveDeviceToGlobalFromZones(std::shared_ptr<AudioDeviceDescriptor> device)
{
    bool findDeviceInZone = false;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        for (auto &zoneMap : zoneMaps_) {
            CHECK_AND_CONTINUE_LOG(zoneMap.second != nullptr, "zone is nullptr");
            CHECK_AND_CONTINUE(zoneMap.second->IsDeviceConnect(device));

            vector<std::shared_ptr<AudioDeviceDescriptor>> devices = {device};
            zoneMap.second->RemoveDeviceDescriptor(devices);
            findDeviceInZone = true;
        }
    }
    CHECK_AND_RETURN(findDeviceInZone);
    AudioDeviceManager::GetAudioDeviceManager().AddNewDevice(device);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device);
}

int32_t AudioZoneService::RegisterAudioZoneClient(pid_t clientPid, sptr<IStandardAudioZoneClient> client)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(client != nullptr && zoneClientManager_ != nullptr, ERROR,
        "client or zoneClientManager is nullptr");
    zoneClientManager_->RegisterAudioZoneClient(clientPid, client);
    return SUCCESS;
}

void AudioZoneService::UnRegisterAudioZoneClient(pid_t clientPid)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    zoneReportClientList_.erase(clientPid);
    AudioZoneInterruptReporter::DisableInterruptReport(clientPid);
    for (const auto &it : zoneMaps_) {
        it.second->EnableChangeReport(clientPid, false);
    }
    CHECK_AND_RETURN_LOG(zoneClientManager_ != nullptr, "zoneClientManager is nullptr");
    zoneClientManager_->UnRegisterAudioZoneClient(clientPid);
}

int32_t AudioZoneService::EnableAudioZoneReport(pid_t clientPid, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "zoneClientManager is nullptr");
    if (enable) {
        zoneReportClientList_.insert(clientPid);
    } else {
        zoneReportClientList_.erase(clientPid);
    }
    AUDIO_INFO_LOG("%{public}s zone event report to client %{public}d",
        enable ? "enable" : "disable", clientPid);
    return SUCCESS;
}

int32_t AudioZoneService::EnableAudioZoneChangeReport(pid_t clientPid,
    int32_t zoneId, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);

    return zone->EnableChangeReport(clientPid, enable);
}

int32_t AudioZoneService::AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream)
{
    return AddKeyToAudioZone(zoneId, INVALID_UID, "", INVALID_STREAM_ID, stream.streamUsage);
}

int32_t AudioZoneService::AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    for (auto stream : streams) {
        AddStreamToAudioZone(zoneId, stream);
    }
    return SUCCESS;
}

int32_t AudioZoneService::RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream)
{
    return RemoveKeysFromAudioZone(zoneId, {AudioZoneBindKey(INVALID_UID, "", INVALID_STREAM_ID, stream.streamUsage)});
}

int32_t AudioZoneService::RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
        for (auto stream : streams) {
            zone->RemoveKey(AudioZoneBindKey(INVALID_UID, "", INVALID_STREAM_ID, stream.streamUsage));
        }
        tmp = interruptService_;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, ERROR,
        HILOG_COMM_ERROR("[RemoveStreamsFromAudioZone]interruptService_ tmp is nullptr"));

    auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE);
    tmp->MigrateAudioInterruptZone(zoneId,
        [this](int32_t uid, const std::string &deviceTag, uint32_t streamId,
            const StreamUsage &usage, DeviceRole deviceRole)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamId, usage, deviceRole);
    });
    for (auto &report : reporter) {
        report->ReportInterrupt();
    }
    return SUCCESS;
}

int32_t AudioZoneService::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    return AddKeyToAudioZone(zoneId, uid, "", INVALID_STREAM_ID, StreamUsage::STREAM_USAGE_INVALID);
}

int32_t AudioZoneService::GetActiveAudioInterruptZone(int32_t &zoneId, AudioStreamType &streamType)
{
    CHECK_AND_RETURN_RET_LOG(interruptService_ != nullptr, ERROR, "interruptService_ is nullptr");
    return interruptService_->GetActiveAudioInterruptZone(zoneId, streamType);
}

void AudioZoneService::SetZoneDeviceVisible(bool visible)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    zoneDeviceVisible_ = visible;
}

bool AudioZoneService::IsZoneDeviceVisible()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return zoneDeviceVisible_;
}

int32_t AudioZoneService::AddKeyToAudioZone(int32_t zoneId, int32_t uid, const std::string &deviceTag,
    uint32_t streamId, const StreamUsage &usage)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    int32_t srcZoneId;
    {
        AUDIO_DEBUG_LOG("add key %{public}d,%{public}s,%{public}u,%{public}d to zone %{public}d",
            uid, deviceTag.c_str(), streamId, usage, zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        srcZoneId = FindAudioZoneByKey(uid, deviceTag, streamId, usage, DEVICE_ROLE_NONE);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);

        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE_LOG(it.first != zoneId && it.second != nullptr,
                "zoneId is duplicate or zone is nullptr");
            it.second->RemoveKey(AudioZoneBindKey(uid, deviceTag, streamId, usage));
        }
        zone->BindByKey(AudioZoneBindKey(uid, deviceTag, streamId, usage));
        tmp = interruptService_;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, ERROR,
        HILOG_COMM_ERROR("[AddKeyToAudioZone]interruptService_ tmp is nullptr"));

    auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::BIND_APP_TO_ZONE);
    tmp->MigrateAudioInterruptZone(srcZoneId,
        [this](int32_t uid, const std::string &deviceTag, uint32_t streamId,
            const StreamUsage &usage, DeviceRole deviceRole)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamId, usage, deviceRole);
    });
    for (auto &report : reporter) {
        report->ReportInterrupt();
    }
    return SUCCESS;
}

int32_t AudioZoneService::FindAudioZoneByUid(int32_t uid, DeviceRole deviceRole)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return FindAudioZoneByKey(uid, "", INVALID_STREAM_ID, StreamUsage::STREAM_USAGE_INVALID, deviceRole);
}

std::string AudioZoneService::FindAudioZoneNameByUid(int32_t uid, DeviceRole deviceRole, uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto keyList = AudioZoneBindKey::GetSupportKeys(uid, "", streamId, StreamUsage::STREAM_USAGE_INVALID);
    for (const auto &key : keyList) {
        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE(it.second != nullptr && it.second->IsContainKey(key));
            CHECK_AND_CONTINUE(it.second->IsSupportDeviceRole(deviceRole));
            return it.second->GetName();
        }
    }
    return std::string(PRIMARY_ZONE_NAME);
}

int32_t AudioZoneService::FindAudioZone(int32_t uid, StreamUsage usage, DeviceRole deviceRole, uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return FindAudioZoneByKey(uid, "", streamId, usage, deviceRole);
}

int32_t AudioZoneService::FindAudioZoneByKey(int32_t uid, const std::string &deviceTag,
    uint32_t streamId, const StreamUsage &usage, DeviceRole deviceRole)
{
    auto keyList = AudioZoneBindKey::GetSupportKeys(uid, deviceTag, streamId, usage);
    for (const auto &key : keyList) {
        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE(it.second != nullptr && it.second->IsContainKey(key));
            CHECK_AND_CONTINUE(it.second->IsSupportDeviceRole(deviceRole));
            return it.first;
        }
    }
    return 0;
}

int32_t AudioZoneService::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    return RemoveKeysFromAudioZone(zoneId,
        {AudioZoneBindKey(uid, "", INVALID_STREAM_ID, StreamUsage::STREAM_USAGE_INVALID)});
}

int32_t AudioZoneService::AddUidUsagesToAudioZone(int32_t zoneId, int32_t uid, const std::set<StreamUsage> &usages)
{
    int32_t ret = SUCCESS;
    for (const auto usage : usages) {
        const int32_t r = AddKeyToAudioZone(zoneId, uid, "", INVALID_STREAM_ID, usage);
        if (r != SUCCESS) {
            AUDIO_WARNING_LOG("AddKeyToAudioZone failed, uid %{public}d, usage %{public}d: %{public}d", uid, usage, r);
            ret = r;
        }
    }
    return ret;
}

int32_t AudioZoneService::RemoveUidUsagesFromAudioZone(int32_t zoneId, int32_t uid, const std::set<StreamUsage> &usages)
{
    std::vector<AudioZoneBindKey> bindKeys;
    for (const auto &usage : usages) {
        bindKeys.push_back(AudioZoneBindKey(uid, usage));
    }
    return RemoveKeysFromAudioZone(zoneId, bindKeys);
}

int32_t AudioZoneService::RemoveKeysFromAudioZone(int32_t zoneId, const std::vector<AudioZoneBindKey> &bindKeys)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
        for (const auto &bindKey : bindKeys) {
            AUDIO_DEBUG_LOG("remove key '%{public}s' from zone %{public}d", bindKey.GetString().c_str(), zoneId);
            zone->RemoveKey(bindKey);
            tmp = interruptService_;
        }
    }

    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, ERROR,
        HILOG_COMM_ERROR("[RemoveKeysFromAudioZone]interruptService_ tmp is nullptr"));

    auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE);
    tmp->MigrateAudioInterruptZone(zoneId,
        [this](int32_t uid, const std::string &deviceTag, uint32_t streamId,
            const StreamUsage &usage, DeviceRole deviceRole)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamId, usage, deviceRole);
    });
    for (auto &report : reporter) {
        report->ReportInterrupt();
    }
    return SUCCESS;
}

int32_t AudioZoneService::AddStreamIdToAudioZone(int32_t zoneId, uint32_t streamId)
{
    return AddKeyToAudioZone(zoneId, INVALID_UID, "", streamId, StreamUsage::STREAM_USAGE_INVALID);
}

int32_t AudioZoneService::RemoveStreamIdFromAudioZone(int32_t zoneId, uint32_t streamId)
{
    return RemoveKeysFromAudioZone(zoneId, {AudioZoneBindKey(streamId)});
}

int32_t AudioZoneService::EnableSystemVolumeProxy(pid_t clientPid, int32_t zoneId, DeviceType deviceType, bool enable)
{
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "zoneClientManager is nullptr");
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_->IsRegisterAudioZoneClient(clientPid), ERROR,
            "client %{public}d for zone id %{public}d is not found", clientPid, zoneId);

        if (zoneId == DEFAULT_ZONEID) {
            mainZoneVolumeProxyClientPid_ = clientPid;
            if (deviceType == DeviceType::DEVICE_TYPE_NONE || deviceType == DeviceType::DEVICE_TYPE_INVALID) {
                if (enable) {
                    mainZoneVolumeProxyDeviceTypes_ = audioConnectedDevice_.GetAllOutputDeviceTypes();
                } else {
                    mainZoneVolumeProxyDeviceTypes_.clear();
                }
            } else {
                if (enable) {
                    mainZoneVolumeProxyDeviceTypes_.insert(deviceType);
                } else {
                    size_t count = mainZoneVolumeProxyDeviceTypes_.erase(deviceType);
                    CHECK_AND_RETURN_RET_LOG(count > 0, ERROR,
                        "main zone volume proxy device type %{public}d is not enabled", deviceType);
                }
            }
            AUDIO_INFO_LOG("main zone device type %{public}d volume proxy is %{public}s by %{public}d",
                deviceType, enable ? "enable" : "disable", clientPid);
        } else {
            auto zone = FindZone(zoneId);
            CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
            zone->EnableSystemVolumeProxy(clientPid, deviceType, enable);
        }
    }
    
    if (enable) {
        if (deviceType == DeviceType::DEVICE_TYPE_NONE || deviceType == DeviceType::DEVICE_TYPE_INVALID) {
            auto outputDevices = audioConnectedDevice_.GetAllOutputDevices();
            for (const auto &desc : outputDevices) {
                audioVolumeManager_.InitAudioZoneVolume(zoneId, desc);
            }
        } else {
            auto desc = GetDeviceDescriptor(zoneId, deviceType);
            CHECK_AND_RETURN_RET_LOG(desc != nullptr, SUCCESS, "desc is not null");
            audioVolumeManager_.InitAudioZoneVolume(zoneId, desc);
        }
    }
    return SUCCESS;
}

bool AudioZoneService::IsSystemVolumeProxyEnable(int32_t zoneId, DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    if (zoneId == DEFAULT_ZONEID) {
        return mainZoneVolumeProxyDeviceTypes_.count(deviceType);
    }
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, false, "zone id %{public}d is not found", zoneId);
    return zone->IsVolumeProxyEnable(deviceType);
}

int32_t AudioZoneService::SetSystemVolumeLevel(int32_t zoneId, DeviceType deviceType, AudioVolumeType volumeType,
    const VolumeScale &volume, int32_t volumeFlag)
{
    AUDIO_INFO_LOG("zoneId: %{public}d, deviceType: %{public}d, volumeType: %{public}d, volumeLevel: %{public}d,"
        "volumeDegree: %{public}d, volumeFlag: %{public}d", zoneId, deviceType, volumeType, volume.VolumeLevel(),
        volume.VolumeDegree(), volumeFlag);
    if (zoneId == 0) {
        std::shared_ptr<AudioZoneClientManager> mgr;
        {
            std::lock_guard<std::mutex> lock(zoneMutex_);
            CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "clientManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(mainZoneVolumeProxyDeviceTypes_.count(deviceType),
                ERROR, "volume proxy is not enable");

            mgr = zoneClientManager_;
        }
        return mgr->SetSystemVolumeLevel(mainZoneVolumeProxyClientPid_, zoneId, deviceType,
            volumeType, volume, volumeFlag);
    }
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(deviceType), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->SetSystemVolumeLevel(deviceType, volumeType, volume, volumeFlag);
}

int32_t AudioZoneService::GetSystemVolumeLevel(int32_t zoneId, DeviceType deviceType, AudioVolumeType volumeType)
{
    if (zoneId == 0) {
        std::shared_ptr<AudioZoneClientManager> mgr;
        {
            std::lock_guard<std::mutex> lock(zoneMutex_);
            CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "clientManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(mainZoneVolumeProxyDeviceTypes_.count(deviceType),
                ERROR, "volume proxy is not enable");

            mgr = zoneClientManager_;
        }
        return mgr->GetSystemVolumeLevel(mainZoneVolumeProxyClientPid_, zoneId, deviceType, volumeType);
    }
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(deviceType), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->GetSystemVolumeLevel(deviceType, volumeType);
}

int32_t AudioZoneService::GetSystemVolumeDegree(int32_t zoneId, DeviceType deviceType, AudioVolumeType volumeType)
{
    if (zoneId == 0) {
        std::shared_ptr<AudioZoneClientManager> mgr;
        {
            std::lock_guard<std::mutex> lock(zoneMutex_);
            CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "clientManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(mainZoneVolumeProxyDeviceTypes_.count(deviceType),
                ERROR, "volume proxy is not enable");

            mgr = zoneClientManager_;
        }
        return mgr->GetSystemVolumeDegree(mainZoneVolumeProxyClientPid_, zoneId, deviceType, volumeType);
    }
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(deviceType), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->GetSystemVolumeDegree(deviceType, volumeType);
}

AudioZoneFocusList AudioZoneService::GetAudioInterruptForZone(int32_t zoneId)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    AudioZoneFocusList interrupts;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET(CheckIsZoneValid(zoneId), interrupts,
            HILOG_COMM_ERROR("[GetAudioInterruptForZone]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }
    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, interrupts,
        HILOG_COMM_ERROR("[GetAudioInterruptForZone]interruptService_ tmp is nullptr"));
    tmp->GetAudioFocusInfoList(zoneId, interrupts);
    return interrupts;
}

bool AudioZoneService::CheckIsZoneValid(int32_t zoneId)
{
    if (zoneId < 0) {
        return false;
    }
    if (zoneId == 0) {
        return true;
    }
    return FindZone(zoneId) != nullptr;
}

bool AudioZoneService::CheckZoneExist(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return CheckIsZoneValid(zoneId);
}

AudioZoneFocusList AudioZoneService::GetAudioInterruptForZone(int32_t zoneId, const std::string &deviceTag)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    AudioZoneFocusList interrupts;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET(CheckIsZoneValid(zoneId), interrupts,
            HILOG_COMM_ERROR("[GetAudioInterruptForZone]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }
    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, interrupts,
        HILOG_COMM_ERROR("[GetAudioInterruptForZone]interruptService_ tmp is nullptr"));
    tmp->GetAudioFocusInfoList(zoneId, deviceTag, interrupts);
    return interrupts;
}

int32_t AudioZoneService::EnableAudioZoneInterruptReport(pid_t clientPid, int32_t zoneId,
    const std::string &deviceTag, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR,
        "zoneClientManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(zoneClientManager_->IsRegisterAudioZoneClient(clientPid), ERROR,
        "no register client %{public}d", clientPid);

    return AudioZoneInterruptReporter::EnableInterruptReport(clientPid, zoneId, deviceTag, enable);
}

AudioInterruptResult AudioZoneService::ActivateAudioInterrupt(int32_t zoneId,
    const AudioInterrupt &audioInterrupt, bool isUpdatedAudioStrategy)
    
{
    AudioInterruptResult result;
    result.retCode = ERROR;
    bool isRecord = audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID;
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        JUDGE_AND_INFO_LOG(zoneId != 0, "active interrupt of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(zoneClientManager_ != nullptr && interruptService_ != nullptr, result,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                isRecord ? RECORD_INTERRUPT_START_NULL_POINTER : INTERRUPT_START_NULL_POINTER,
                "zoneClientManager or interruptService is nullptr", false),
            HILOG_COMM_ERROR("[ActivateAudioInterrupt]zoneClientManager or interruptService is nullptr"));
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(CheckIsZoneValid(zoneId), result,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                isRecord ? RECORD_INTERRUPT_START_INVALID_PARAM : INTERRUPT_START_INVALID_PARAM,
                "zone id is not valid", false),
            HILOG_COMM_ERROR("[ActivateAudioInterrupt]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(tmp != nullptr, result,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            isRecord ? RECORD_INTERRUPT_START_NULL_POINTER : INTERRUPT_START_NULL_POINTER,
            "interruptService_ tmp is nullptr", false),
        HILOG_COMM_ERROR("[ActivateAudioInterrupt]interruptService_ tmp is nullptr"));
    
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    result = tmp->ActivateAudioInterrupt(zoneId, audioInterrupt,
        isUpdatedAudioStrategy);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return result;
}

AudioInterruptResult AudioZoneService::DeactivateAudioInterrupt(int32_t zoneId,
    const AudioInterrupt &audioInterrupt, bool isRemoveFocusHis)
{
    AudioInterruptResult result;
    result.retCode = ERROR;
    bool isRecord = audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID;
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        JUDGE_AND_INFO_LOG(zoneId != 0, "deactive interrupt of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(zoneClientManager_ != nullptr && interruptService_ != nullptr, result,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                isRecord ? RECORD_INTERRUPT_STOP_NULL_POINTER : INTERRUPT_STOP_NULL_POINTER,
                "zoneClientManager or interruptService is nullptr", false),
            HILOG_COMM_ERROR("[DeactivateAudioInterrupt]zoneClientManager or interruptService is nullptr"));
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(CheckIsZoneValid(zoneId), result,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                isRecord ? RECORD_INTERRUPT_STOP_INVALID_PARAM : INTERRUPT_STOP_INVALID_PARAM,
                "zone id is not valid", false),
            HILOG_COMM_ERROR("[DeactivateAudioInterrupt]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(tmp != nullptr, result,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            isRecord ? RECORD_INTERRUPT_STOP_NULL_POINTER : INTERRUPT_STOP_NULL_POINTER,
            "interruptService_ tmp is nullptr", false),
        HILOG_COMM_ERROR("[DeactivateAudioInterrupt]interruptService_ tmp is nullptr"));
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    result = tmp->DeactivateAudioInterrupt(zoneId, audioInterrupt, isRemoveFocusHis);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return result;
}

int32_t AudioZoneService::InjectInterruptToAudioZone(int32_t zoneId,
    const AudioZoneFocusList &interrupts)
{
    return InjectInterruptToAudioZone(zoneId, "", interrupts);
}

int32_t AudioZoneService::InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
    const AudioZoneFocusList &interrupts)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        AUDIO_INFO_LOG("inject interrupt to zone %{public}d, device tag %{public}s",
            zoneId, deviceTag.c_str());
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            HILOG_COMM_ERROR("[InjectInterruptToAudioZone]zoneClientManager or interruptService is nullptr"));
        CHECK_AND_CALL_FUNC_RETURN_RET(CheckIsZoneValid(zoneId), ERROR,
            HILOG_COMM_ERROR("[InjectInterruptToAudioZone]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }
    
    CHECK_AND_CALL_FUNC_RETURN_RET(tmp != nullptr, ERROR,
        HILOG_COMM_ERROR("[InjectInterruptToAudioZone]interruptService_ tmp is nullptr"));
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId, tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT, interrupts);
    int32_t ret;
    if (deviceTag.empty()) {
        ret = tmp->InjectInterruptToAudioZone(zoneId, interrupts);
    } else {
        ret = tmp->InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
    }
    for (auto &report : reporters) {
        report->ReportInterrupt(deviceTag);
    }
    return ret;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioZoneService::FetchOutputDevices(int32_t zoneId,
    StreamUsage streamUsage, int32_t clientUid, RouterType bypassType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, devices, "zone id %{public}d is not found", zoneId);

    return zone->FetchOutputDevices(streamUsage, clientUid, bypassType);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioZoneService::GetAllOutputDevices(int32_t zoneId)
{
    if (zoneId == 0) {
        return audioConnectedDevice_.GetAllOutputDevices();
    }
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, {}, "zone id %{public}d is not found", zoneId);

    return zone->GetAllOutputDevices();
}

std::shared_ptr<AudioDeviceDescriptor> AudioZoneService::FetchInputDevice(int32_t zoneId,
    SourceType sourceType, int32_t clientUid)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, nullptr, "zone id %{public}d is not found", zoneId);

    return zone->FetchInputDevice(sourceType, clientUid);
}

std::shared_ptr<AudioZone> AudioZoneService::FindZone(int32_t zoneId)
{
    auto it = zoneMaps_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG(it != zoneMaps_.end(), nullptr, "zone id %{public}d is not found", zoneId);

    return zoneMaps_[zoneId];
}

const std::string AudioZoneService::GetZoneStringDescriptor(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, "", "zone id %{public}d is not found", zoneId);

    return zone->GetStringDescriptor();
}

int32_t AudioZoneService::UpdateDeviceFromGlobalForAllZone(std::shared_ptr<AudioDeviceDescriptor> device)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is nullptr!");
    for (const auto &it : zoneMaps_) {
        CHECK_AND_CONTINUE_LOG(it.second != nullptr, "zone id %{public}d is nullptr", it.first);
        int32_t res = it.second->UpdateDeviceDescriptor(device);
        if (res == SUCCESS) {
            AUDIO_INFO_LOG("zone id %{public}d enable device %{public}d success", it.first, device->deviceType_);
            return res;
        }
    }
    return ERROR;
}

int32_t AudioZoneService::ClearAudioFocusBySessionID(const int32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG(interruptService_ != nullptr, ERROR, "interruptService_ is nullptr");
    return interruptService_->ClearAudioFocusBySessionID(sessionID);
}

void AudioZoneService::ReleaseAudioZoneByClientPid(pid_t clientPid)
{
    int32_t zoneId;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto findZone = [&clientPid] (const std::pair<int32_t, std::shared_ptr<AudioZone>> &item) {
            CHECK_AND_RETURN_RET(item.second != nullptr, false);
            return item.second->GetClientPid() == clientPid;
        };

        auto itZone = std::find_if(zoneMaps_.begin(), zoneMaps_.end(), findZone);
        CHECK_AND_RETURN(itZone != zoneMaps_.end());
        zoneId = itZone->first;
    }

    AUDIO_INFO_LOG("client %{public}d died, release zone %{public}d", clientPid, zoneId);
    ReleaseAudioZone(zoneId);
}

bool AudioZoneService::CheckDeviceInAudioZone(AudioDeviceDescriptor device)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (auto &it : zoneMaps_) {
        if (it.second->CheckDeviceInZone(device)) {
            return true;
        }
    }
    return false;
}

bool AudioZoneService::HasVolumeControllableDeviceInAudioZone()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (auto &it : zoneMaps_) {
        if (it.second->HasVolumeControllableDeviceInZone()) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<AudioDeviceDescriptor> AudioZoneService::GetDeviceDescriptor(DeviceType type, std::string networkId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (auto &it : zoneMaps_) {
        auto desc = it.second->GetDeviceDescriptor(type, networkId);
        CHECK_AND_CONTINUE(desc != nullptr);
        return desc;
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioZoneService::GetDeviceDescriptor(int32_t zoneId, DeviceType type)
{
    if (zoneId == 0) {
        return audioConnectedDevice_.GetConnectedDeviceByType(type);
    }
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, nullptr, "zone id %{public}d is not found", zoneId);

    return zone->GetDeviceDescriptor(type);
}

AudioScene AudioZoneService::GetAudioSceneFromAllZones()
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        tmp = interruptService_;
    }
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, AUDIO_SCENE_DEFAULT, "interruptService_ tmp is nullptr");
    return tmp->GetHighestPriorityAudioSceneFromAllZones();
}

void AudioZoneService::UpdateContextForAudioZone(int32_t zoneId, const AudioZoneContext &context)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        if (zoneId == DEFAULT_ZONEID) {
            mainZoneContext_ = context;
            AUDIO_INFO_LOG("zoneId: %{public}d, userId: %{public}d", zoneId, context.userId_);
        } else {
            auto zone = FindZone(zoneId);
            if (zone != nullptr) {
                zone->UpdateContext(context);
                AUDIO_INFO_LOG("zoneId: %{public}d, supportType: %{public}d, userId: %{public}d", zoneId,
                    context.supportType_, context.userId_);
            } else {
                AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
            }
        }
        tmp = interruptService_;
    }
    CHECK_AND_RETURN_LOG(tmp != nullptr, "interruptService_ tmp is nullptr");
    CHECK_AND_RETURN(zoneId != DEFAULT_ZONEID);
    tmp->UpdateContextForAudioZone(zoneId, context);
}

void AudioZoneService::NotifyStreamSilentChange(uint32_t streamId)
{
    CHECK_AND_CALL_FUNC_RETURN(zoneClientManager_ != nullptr && interruptService_ != nullptr,
        AUDIO_INFO_LOG("zoneClientManager or interruptService is nullptr"));

    std::shared_ptr<AudioInterruptService> tmp = interruptService_;
    CHECK_AND_CALL_FUNC_RETURN(tmp != nullptr,
        AUDIO_INFO_LOG("interruptService_ tmp is nullptr"));

    int32_t zoneId = tmp->FindAudioZoneByStreamId(streamId);
    JUDGE_AND_INFO_LOG(zoneId != 0, "NotifyStreamSilentChange of zone %{public}d", zoneId);
    CHECK_AND_CALL_FUNC_RETURN(CheckIsZoneValid(zoneId),
        AUDIO_INFO_LOG("zone id %{public}d is not valid", zoneId));

    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_, AudioZoneInterruptReason::REMOTE_INJECT);
    if (tmp->NotifyStreamSilentChange(streamId)) {
        for (auto &report : reporters) {
            report->ReportInterrupt();
        }
    }
}

AudioInterruptResult AudioZoneService::ActivateAudioSession(const int32_t zoneId, const int32_t callerPid,
    const AudioSessionStrategy &strategy, const bool isStandalone, const bool stateChangeCallbackFlag,
    int32_t callerUid)
{
    AudioInterruptResult result;
    result.retCode = ERROR;
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        JUDGE_AND_INFO_LOG(zoneId != 0, "active audio session of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(zoneClientManager_ != nullptr && interruptService_ != nullptr, result,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                SESSION_START_NULL_POINTER, "zoneClientManager or interruptService is nullptr", false),
            HILOG_COMM_ERROR("[ActivateAudioSession]zoneClientManager or interruptService is nullptr"));
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(CheckIsZoneValid(zoneId), result,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                SESSION_START_INVALID_PARAM, "zone id is not valid", false),
            HILOG_COMM_ERROR("[ActivateAudioSession]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(tmp != nullptr, result,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_START_NULL_POINTER, "interruptService_ tmp is nullptr", false),
        HILOG_COMM_ERROR("[ActivateAudioSession]interruptService_ tmp is nullptr"));
    
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    result = tmp->ActivateAudioSession(zoneId, callerPid, strategy, isStandalone, stateChangeCallbackFlag, callerUid);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return result;
}

int32_t AudioZoneService::DeactivateAudioSession(int32_t zoneId, const int32_t callerPid)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        JUDGE_AND_INFO_LOG(zoneId != 0, "deactive audio session of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                SESSION_STOP_NULL_POINTER, "zoneClientManager or interruptService is nullptr", false),
            HILOG_COMM_ERROR("[DeactivateAudioSession]zoneClientManager or interruptService is nullptr"));
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(CheckIsZoneValid(zoneId), ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                SESSION_STOP_INVALID_PARAM, "zone id is not valid", false),
            HILOG_COMM_ERROR("[DeactivateAudioSession]zone id %{public}d is not valid", zoneId));
        tmp = interruptService_;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(tmp != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_STOP_NULL_POINTER, "interruptService_ tmp is nullptr", false),
        HILOG_COMM_ERROR("[DeactivateAudioSession]interruptService_ tmp is nullptr"));
    
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    auto onFakeInterruptDeactivate = [reporters]() {
        for (auto &report : reporters) {
            report->ReportInterrupt();
        }
    };
    int32_t result = tmp->DeactivateAudioSession(zoneId, callerPid, onFakeInterruptDeactivate);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return result;
}

void AudioZoneService::OnSessionTimeout(const int32_t pid)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_LOG(zoneClientManager_ != nullptr && interruptService_ != nullptr,
            "zoneClientManager or interruptService is nullptr");
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_LOG(tmp != nullptr, "interruptService_ tmp is nullptr");

    auto zoneIds = tmp->FindAudioZonesByPid(pid);
    AudioZoneInterruptReporter::ReporterVector reporters;
    for (auto zoneId : zoneIds) {
        CHECK_AND_CONTINUE(CheckIsZoneValid(zoneId));
        auto tempReporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
            tmp, zoneClientManager_, AudioZoneInterruptReason::REMOTE_INJECT);
        reporters.insert(reporters.end(), tempReporters.begin(), tempReporters.end());
    }
    tmp->OnSessionTimeout(pid);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
}
} // namespace AudioStandard
} // namespace OHOS