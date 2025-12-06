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
}

void AudioZoneService::DeInit()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    zoneMaps_.clear();
    zoneReportClientList_.clear();
    AudioZoneInterruptReporter::DisableAllInterruptReport();
    zoneClientManager_ = nullptr;
    interruptService_ = nullptr;
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
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");
    tmp->CreateAudioInterruptZone(zoneId, context);
    AUDIO_INFO_LOG("create zone id %{public}d, name %{public}s", zoneId, name.c_str());
    return zoneId;
}

void AudioZoneService::ReleaseAudioZone(int32_t zoneId)
{
    if (interruptService_ != nullptr) {
        std::vector<int32_t> sessionUidList = interruptService_->GetAudioSessionUidList(zoneId);
        for (auto uid : sessionUidList) {
            RemoveUidFromAudioZone(zoneId, uid);
        }
    }

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
        [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag,
            const StreamUsage &usage)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamTag, usage);
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
            CHECK_AND_CONTINUE(zone->IsDeviceConnect(it));
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
    return AddKeyToAudioZone(zoneId, INVALID_UID, "", "", stream.streamUsage);
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
    return RemoveKeyFromAudioZone(zoneId, INVALID_UID, "", "", stream.streamUsage);
}

int32_t AudioZoneService::RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
        for (auto stream : streams) {
            zone->RemoveKey(AudioZoneBindKey(INVALID_UID, "", "", stream.streamUsage));
        }
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");

    auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE);
    tmp->MigrateAudioInterruptZone(zoneId,
        [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag,
            const StreamUsage &usage)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamTag, usage);
    });
    for (auto &report : reporter) {
        report->ReportInterrupt();
    }
    return SUCCESS;
}

int32_t AudioZoneService::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    return AddKeyToAudioZone(zoneId, uid, "", "", StreamUsage::STREAM_USAGE_INVALID);
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

int32_t AudioZoneService::AddKeyToAudioZone(int32_t zoneId, int32_t uid,
    const std::string &deviceTag, const std::string &streamTag, const StreamUsage &usage)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    int32_t srcZoneId;
    {
        AUDIO_DEBUG_LOG("add key %{public}d,%{public}s,%{public}s to zone %{public}d",
            uid, deviceTag.c_str(), streamTag.c_str(), zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        srcZoneId = FindAudioZoneByKey(uid, deviceTag, streamTag, usage);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);

        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE_LOG(it.first != zoneId && it.second != nullptr,
                "zoneId is duplicate or zone is nullptr");
            it.second->RemoveKey(AudioZoneBindKey(uid, deviceTag, streamTag, usage));
        }
        zone->BindByKey(AudioZoneBindKey(uid, deviceTag, streamTag, usage));
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");
    
    auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::BIND_APP_TO_ZONE);
    tmp->MigrateAudioInterruptZone(srcZoneId,
        [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag,
            const StreamUsage &usage)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamTag, usage);
    });
    for (auto &report : reporter) {
        report->ReportInterrupt();
    }
    return SUCCESS;
}

int32_t AudioZoneService::FindAudioZoneByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return FindAudioZoneByKey(uid, "", "", StreamUsage::STREAM_USAGE_INVALID);
}

int32_t AudioZoneService::FindAudioSessionZoneid(int32_t callerUid, int32_t callerPid, bool isActivate)
{
    int32_t zoneId;
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        zoneId = FindAudioZoneByKey(callerUid, "", "", StreamUsage::STREAM_USAGE_INVALID);
        tmp = interruptService_;
    }
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, zoneId, "interruptService_ is nullptr");
    StreamUsage streamUsage = tmp->GetAudioSessionStreamUsage(callerPid);
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        if (streamUsage == StreamUsage::STREAM_USAGE_INVALID) {
            return zoneId;
        }
        zoneId = FindAudioZoneByKey(INVALID_UID, "", "", streamUsage);
        AUDIO_INFO_LOG("get audio session zoneId:%{public}d streamUsage:%{public}d isActivate:%{public}d",
            zoneId, streamUsage, isActivate);
    }
    isActivate ? AddUidToAudioZone(zoneId, callerUid) : RemoveUidFromAudioZone(zoneId, callerUid);
    return zoneId;
}

int32_t AudioZoneService::FindAudioZone(int32_t uid, StreamUsage usage)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    int32_t zoneId = FindAudioZoneByKey(uid, "", "", StreamUsage::STREAM_USAGE_INVALID);
    return zoneId != 0 ? zoneId : FindAudioZoneByKey(INVALID_UID, "", "", usage);
}

int32_t AudioZoneService::FindAudioZoneByKey(int32_t uid, const std::string &deviceTag,
    const std::string &streamTag, const StreamUsage &usage)
{
    auto keyList = AudioZoneBindKey::GetSupportKeys(uid, deviceTag, streamTag, usage);
    for (const auto &key : keyList) {
        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE(it.second != nullptr && it.second->IsContainKey(key));
            return it.first;
        }
    }
    return 0;
}

int32_t AudioZoneService::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    return RemoveKeyFromAudioZone(zoneId, uid, "", "", StreamUsage::STREAM_USAGE_INVALID);
}

int32_t AudioZoneService::RemoveKeyFromAudioZone(int32_t zoneId, int32_t uid,
    const std::string &deviceTag, const std::string &streamTag, const StreamUsage &usage)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        AUDIO_DEBUG_LOG("remove key %{public}d,%{public}s,%{public}s from zone %{public}d",
            uid, deviceTag.c_str(), streamTag.c_str(), zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);

        zone->RemoveKey(AudioZoneBindKey(uid, deviceTag, streamTag, usage));
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");

    auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
        zoneClientManager_, AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE);
    tmp->MigrateAudioInterruptZone(zoneId,
        [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag,
            const StreamUsage &usage)->int32_t {
            return this->FindAudioZoneByKey(uid, deviceTag, streamTag, usage);
    });
    for (auto &report : reporter) {
        report->ReportInterrupt();
    }
    return SUCCESS;
}

int32_t AudioZoneService::EnableSystemVolumeProxy(pid_t clientPid, int32_t zoneId, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR, "zoneClientManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(zoneClientManager_->IsRegisterAudioZoneClient(clientPid), ERROR,
        "client %{public}d for zone id %{public}d is not found", clientPid, zoneId);

    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    return zone->EnableSystemVolumeProxy(clientPid, enable);
}

bool AudioZoneService::IsSystemVolumeProxyEnable(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    return zone->IsVolumeProxyEnable();
}

int32_t AudioZoneService::SetSystemVolumeLevel(int32_t zoneId, AudioVolumeType volumeType,
    int32_t volumeLevel, int32_t volumeFlag)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->SetSystemVolumeLevel(volumeType, volumeLevel, volumeFlag);
}

int32_t AudioZoneService::GetSystemVolumeLevel(int32_t zoneId, AudioVolumeType volumeType)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->GetSystemVolumeLevel(volumeType);
}

int32_t AudioZoneService::SetSystemVolumeDegree(int32_t zoneId, AudioVolumeType volumeType,
    int32_t volumeDegree, int32_t volumeFlag)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->SetSystemVolumeDegree(volumeType, volumeDegree, volumeFlag);
}

int32_t AudioZoneService::GetSystemVolumeDegree(int32_t zoneId, AudioVolumeType volumeType)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone id %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(zone->IsVolumeProxyEnable(), ERROR,
        "zone id %{public}d IsVolumeProxyEnable is false", zoneId);
    return zone->GetSystemVolumeDegree(volumeType);
}

AudioZoneFocusList AudioZoneService::GetAudioInterruptForZone(int32_t zoneId)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    AudioZoneFocusList interrupts;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(CheckIsZoneValid(zoneId), interrupts, "zone id %{public}d is not valid", zoneId);
        tmp = interruptService_;
    }
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, interrupts, "interruptService_ tmp is nullptr");
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
        CHECK_AND_RETURN_RET_LOG(CheckIsZoneValid(zoneId), interrupts, "zone id %{public}d is not valid", zoneId);
        tmp = interruptService_;
    }
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, interrupts, "interruptService_ tmp is nullptr");
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

int32_t AudioZoneService::ActivateAudioInterrupt(int32_t zoneId,
    const AudioInterrupt &audioInterrupt, bool isUpdatedAudioStrategy)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        JUDGE_AND_INFO_LOG(zoneId != 0, "active interrupt of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            "zoneClientManager or interruptService is nullptr");
        CHECK_AND_RETURN_RET_LOG(CheckIsZoneValid(zoneId), ERROR,
            "zone id %{public}d is not valid", zoneId);
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    int ret = tmp->ActivateAudioInterrupt(zoneId, audioInterrupt,
        isUpdatedAudioStrategy);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return ret;
}

int32_t AudioZoneService::DeactivateAudioInterrupt(int32_t zoneId,
    const AudioInterrupt &audioInterrupt)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        JUDGE_AND_INFO_LOG(zoneId != 0, "deactive interrupt of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            "zoneClientManager or interruptService is nullptr");
        CHECK_AND_RETURN_RET_LOG(CheckIsZoneValid(zoneId), ERROR,
            "zone id %{public}d is not valid", zoneId);
        tmp = interruptService_;
    }
    
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    int ret = tmp->DeactivateAudioInterrupt(zoneId, audioInterrupt);
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return ret;
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
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            "zoneClientManager or interruptService is nullptr");
        CHECK_AND_RETURN_RET_LOG(CheckIsZoneValid(zoneId), ERROR,
            "zone id %{public}d is not valid", zoneId);
        tmp = interruptService_;
    }
    
    CHECK_AND_RETURN_RET_LOG(tmp != nullptr, ERROR, "interruptService_ tmp is nullptr");
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
    StreamUsage streamUsage, int32_t clientUid, const RouterType &bypassType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, devices, "zone id %{public}d is not found", zoneId);

    return zone->FetchOutputDevices(streamUsage, clientUid, bypassType);
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

bool AudioZoneService::CheckExistUidInAudioZone()
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    for (auto &it : zoneMaps_) {
        if (it.second->CheckExistUidInZone()) {
            return true;
        }
    }
    return false;
}
} // namespace AudioStandard
} // namespace OHOS