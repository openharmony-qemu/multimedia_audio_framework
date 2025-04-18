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
#include "audio_errors.h"
#include "audio_zone.h"
#include "audio_zone_client_proxy.h"
#include "audio_zone_client_manager.h"
#include "audio_zone_interrupt_reporter.h"
#include "audio_device_lock.h"
#include "audio_connected_device.h"
#include "audio_core_service.h"

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

int32_t AudioZoneService::CreateAudioZone(const std::string &name, const AudioZoneContext &context)
{
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(zoneClientManager_, name, context);
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone is nullptr");
    int32_t zoneId = zone->GetId();
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        if (zoneMaps_.find(zoneId) != zoneMaps_.end()) {
            AUDIO_ERR_LOG("zone id %{public}d is duplicate", zoneId);
            return ERROR;
        }
        zoneMaps_[zoneId] = zone;

        if (zoneClientManager_ != nullptr) {
            for (auto &pid : zoneReportClientList_) {
                zoneClientManager_->SendZoneAddEvent(pid, zone->GetDescriptor());
            }
        }

        if (interruptService_ != nullptr) {
            interruptService_->CreateAudioInterruptZone(zoneId, context.focusStrategy_);
        }
    }
    AUDIO_INFO_LOG("create zone id %{public}d", zoneId);
    return zoneId;
}

void AudioZoneService::ReleaseAudioZone(int32_t zoneId)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        if (zoneMaps_.find(zoneId) == zoneMaps_.end()) {
            AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
            return;
        }
        zoneMaps_.erase(zoneId);
        tmp = interruptService_;
    }

    if (tmp != nullptr) {
        auto reporters = AudioZoneInterruptReporter::CreateReporter(tmp,
            zoneClientManager_, AudioZoneInterruptReason::RELEASE_AUDIO_ZONE);
        tmp->ReleaseAudioInterruptZone(zoneId,
            [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag)->int32_t {
                return this->FindAudioZoneByKey(uid, deviceTag, streamTag);
        });
        for (auto &report : reporters) {
            report->ReportInterrupt();
        }
    }

    {
        if (zoneClientManager_ != nullptr) {
            for (auto &pid : zoneReportClientList_) {
                zoneClientManager_->SendZoneRemoveEvent(pid, zoneId);
            }
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
    if (zone == nullptr) {
        AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
        return nullptr;
    }
    return zone->GetDescriptor();
}

int32_t AudioZoneService::BindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        if (zone == nullptr) {
            AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
            return ERROR;
        }
        int ret = zone->AddDeviceDescriptor(devices);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    for (auto device : devices) {
        RemoveDeviceFromGlobal(device);
    }
    return SUCCESS;
}

void AudioZoneService::RemoveDeviceFromGlobal(std::shared_ptr<AudioDeviceDescriptor> device)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectDevices;
    AudioConnectedDevice::GetInstance().GetAllConnectedDeviceByType(device->networkId_,
        device->deviceType_, device->macAddress_, device->deviceRole_, connectDevices);
    CHECK_AND_RETURN_LOG(connectDevices.size() != 0, "connectDevices is empty.");
    AudioDeviceLock::GetInstance().OnDeviceStatusUpdated(*device, false);
}

int32_t AudioZoneService::UnBindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> toGlobalDevices;
    {
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        if (zone == nullptr) {
            AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
            return ERROR;
        }
        for (auto it : devices) {
            if (zone->IsDeviceConnect(it)) {
                toGlobalDevices.push_back(it);
            }
        }
        zone->RemoveDeviceDescriptor(devices);
    }
    // maybe whether or not add unbind devices to global is specified by caller
    for (auto it : toGlobalDevices) {
        AudioDeviceLock::GetInstance().OnDeviceStatusUpdated(*it, true);
    }
    return SUCCESS;
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
    if (zone == nullptr) {
        AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
        return ERROR;
    }
    return zone->EnableChangeReport(clientPid, enable);
}

int32_t AudioZoneService::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    return AddKeyToAudioZone(zoneId, uid, "", "");
}

int32_t AudioZoneService::AddKeyToAudioZone(int32_t zoneId, int32_t uid,
    const std::string &deviceTag, const std::string &streamTag)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    int32_t srcZoneId;
    {
        AUDIO_DEBUG_LOG("add key %{public}d,%{public}s,%{public}s to zone %{public}d",
            uid, deviceTag.c_str(), streamTag.c_str(), zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        srcZoneId = FindAudioZoneByKey(uid, deviceTag, streamTag);
        auto zone = FindZone(zoneId);
        if (zone == nullptr) {
            AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
            return ERROR;
        }
        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE_LOG(it.first != zoneId && it.second != nullptr,
                "zoneId is duplicate or zone is nullptr");
            it.second->RemoveKey(AudioZoneBindKey(uid, deviceTag, streamTag));
        }
        zone->BindByKey(AudioZoneBindKey(uid, deviceTag, streamTag));
        tmp = interruptService_;
    }

    if (tmp != nullptr) {
        auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
            zoneClientManager_, AudioZoneInterruptReason::BIND_APP_TO_ZONE);
        tmp->MigrateAudioInterruptZone(srcZoneId,
            [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag)->int32_t {
                return this->FindAudioZoneByKey(uid, deviceTag, streamTag);
        });
        for (auto &report : reporter) {
            report->ReportInterrupt();
        }
    }
    return SUCCESS;
}

int32_t AudioZoneService::FindAudioZoneByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    return FindAudioZoneByKey(uid, "", "");
}

int32_t AudioZoneService::FindAudioZoneByKey(int32_t uid, const std::string &deviceTag,
    const std::string &streamTag)
{
    auto keyList = AudioZoneBindKey::GetSupportKeys(uid, deviceTag, streamTag);
    for (const auto &key : keyList) {
        for (const auto &it : zoneMaps_) {
            CHECK_AND_CONTINUE_LOG(it.second != nullptr, "zone is nullptr");
            if (it.second->IsContainKey(key)) {
                return it.first;
            }
        }
    }
    return 0;
}

int32_t AudioZoneService::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    return RemoveKeyFromAudioZone(zoneId, uid, "", "");
}

int32_t AudioZoneService::RemoveKeyFromAudioZone(int32_t zoneId, int32_t uid,
    const std::string &deviceTag, const std::string &streamTag)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        AUDIO_DEBUG_LOG("remove key %{public}d,%{public}d,%{public}s from zone %{public}d",
            uid, deviceTag, streamTag.c_str(), zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        auto zone = FindZone(zoneId);
        if (zone == nullptr) {
            AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
            return ERROR;
        }
        zone->RemoveKey(AudioZoneBindKey(uid, deviceTag, streamTag));
        tmp = interruptService_;
    }

    if (tmp != nullptr) {
        auto reporter = AudioZoneInterruptReporter::CreateReporter(tmp,
            zoneClientManager_, AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE);
        tmp->MigrateAudioInterruptZone(zoneId,
            [this](int32_t uid, const std::string &deviceTag, const std::string &streamTag)->int32_t {
                return this->FindAudioZoneByKey(uid, deviceTag, streamTag);
        });
        for (auto &report : reporter) {
            report->ReportInterrupt();
        }
    }
    return SUCCESS;
}

int32_t AudioZoneService::EnableSystemVolumeProxy(pid_t clientPid, int32_t zoneId, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    if (zoneClientManager_ == nullptr || !zoneClientManager_->IsRegisterAudioZoneClient(clientPid)) {
        AUDIO_ERR_LOG("client %{public}d for zone id %{public}d is not found", clientPid, zoneId);
        return ERROR;
    }
    auto zone = FindZone(zoneId);
    if (zone == nullptr) {
        AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
        return ERROR;
    }
    return zone->EnableSystemVolumeProxy(clientPid, enable);
}

AudioZoneFocusList AudioZoneService::GetAudioInterruptForZone(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    AudioZoneFocusList interrupts;
    if (!CheckIsZoneValid(zoneId)) {
        AUDIO_ERR_LOG("zone id %{public}d is not valid", zoneId);
        return interrupts;
    }

    if (interruptService_ != nullptr) {
        interruptService_->GetAudioFocusInfoList(zoneId, interrupts);
    }
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

AudioZoneFocusList AudioZoneService::GetAudioInterruptForZone(int32_t zoneId, const std::string &deviceTag)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    AudioZoneFocusList interrupts;
    if (!CheckIsZoneValid(zoneId)) {
        AUDIO_ERR_LOG("zone id %{public}d is not valid", zoneId);
        return interrupts;
    }

    if (interruptService_ != nullptr) {
        interruptService_->GetAudioFocusInfoList(zoneId, deviceTag, interrupts);
    }
    return interrupts;
}

int32_t AudioZoneService::EnableAudioZoneInterruptReport(pid_t clientPid, int32_t zoneId,
    const std::string &deviceTag, bool enable)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr, ERROR,
        "zoneClientManager is nullptr");
    if (!zoneClientManager_->IsRegisterAudioZoneClient(clientPid)) {
        AUDIO_ERR_LOG("no register client %{public}d", clientPid);
        return ERROR;
    }

    return AudioZoneInterruptReporter::EnableInterruptReport(clientPid, zoneId, deviceTag, enable);
}

int32_t AudioZoneService::ActivateAudioInterrupt(int32_t zoneId,
    const AudioInterrupt &audioInterrupt, bool isUpdatedAudioStrategy)
{
    std::shared_ptr<AudioInterruptService> tmp = nullptr;
    {
        AUDIO_INFO_LOG("active interrupt of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            "zoneClientManager or interruptService is nullptr");
        if (!CheckIsZoneValid(zoneId)) {
            AUDIO_ERR_LOG("zone id %{public}d is not valid", zoneId);
            return ERROR;
        }
        tmp = interruptService_;
    }

    CHECK_AND_RETURN_RET_LOG(tmp!= nullptr, ERROR, "interruptService_ tmp is nullptr");
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
        AUDIO_INFO_LOG("deactive interrupt of zone %{public}d", zoneId);
        std::lock_guard<std::mutex> lock(zoneMutex_);
        CHECK_AND_RETURN_RET_LOG(zoneClientManager_ != nullptr && interruptService_ != nullptr, ERROR,
            "zoneClientManager or interruptService is nullptr");
        if (!CheckIsZoneValid(zoneId)) {
            AUDIO_ERR_LOG("zone id %{public}d is not valid", zoneId);
            return ERROR;
        }
        tmp = interruptService_;
    }
    
    CHECK_AND_RETURN_RET_LOG(tmp!= nullptr, ERROR, "interruptService_ tmp is nullptr");
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
        if (!CheckIsZoneValid(zoneId)) {
            AUDIO_ERR_LOG("zone id %{public}d is not valid", zoneId);
            return ERROR;
        }
        tmp = interruptService_;
    }
    
    CHECK_AND_RETURN_RET_LOG(tmp!= nullptr, ERROR, "interruptService_ tmp is nullptr");
    auto reporters = AudioZoneInterruptReporter::CreateReporter(zoneId,
        tmp, zoneClientManager_,
        AudioZoneInterruptReason::REMOTE_INJECT);
    int32_t ret;
    if (deviceTag.empty()) {
        ret = tmp->InjectInterruptToAudioZone(zoneId, interrupts);
    } else {
        ret = tmp->InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
    }
    for (auto &report : reporters) {
        report->ReportInterrupt();
    }
    return ret;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioZoneService::FetchOutputDevices(int32_t zoneId,
    StreamUsage streamUsage, int32_t clientUid, const RouterType &bypassType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    if (zone == nullptr) {
        AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
        return devices;
    }
    return zone->FetchOutputDevices(streamUsage, clientUid, bypassType);
}

std::shared_ptr<AudioDeviceDescriptor> AudioZoneService::FetchInputDevice(int32_t zoneId,
    SourceType sourceType, int32_t clientUid)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    if (zone == nullptr) {
        AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
        return nullptr;
    }
    return zone->FetchInputDevice(sourceType, clientUid);
}

std::shared_ptr<AudioZone> AudioZoneService::FindZone(int32_t zoneId)
{
    auto it = zoneMaps_.find(zoneId);
    if (it == zoneMaps_.end()) {
        return nullptr;
    }
    return zoneMaps_[zoneId];
}

const std::string AudioZoneService::GetZoneStringDescriptor(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    auto zone = FindZone(zoneId);
    if (zone == nullptr) {
        AUDIO_ERR_LOG("zone id %{public}d is not found", zoneId);
        return "";
    }
    return zone->GetStringDescriptor();
}

int32_t AudioZoneService::UpdateDeviceFromGlobalForAllZone(std::shared_ptr<AudioDeviceDescriptor> device)
{
    std::lock_guard<std::mutex> lock(zoneMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is nullptr!");
    for (const auto &it : zoneMaps_) {
        CHECK_AND_CONTINUE_LOG(it.second!= nullptr, "zone id %{public}d is nullptr", it.first);
        int32_t res = it.second->UpdateDeviceDescriptor(device);
        if (res == SUCCESS) {
            AUDIO_INFO_LOG("zone id %{public}d enable device %{public}d success", it.first, device->deviceType_);
            return res;
        }
    }
    return ERROR;
}
} // namespace AudioStandard
} // namespace OHOS