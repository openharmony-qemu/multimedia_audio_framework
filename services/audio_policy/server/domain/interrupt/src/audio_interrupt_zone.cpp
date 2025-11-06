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
#ifndef LOG_TAG
#define LOG_TAG "AudioInterruptZone"
#endif

#include "audio_interrupt_zone.h"
#include "audio_interrupt_service.h"

namespace OHOS {
namespace AudioStandard {
static constexpr uid_t UID_AUDIO = 1041;

AudioInterruptZoneManager::AudioInterruptZoneManager()
{}

AudioInterruptZoneManager::~AudioInterruptZoneManager()
{
    service_ = nullptr;
}

void AudioInterruptZoneManager::InitService(AudioInterruptService *service)
{
    service_ = service;
}

int32_t AudioInterruptZoneManager::GetAudioFocusInfoList(const int32_t zoneId,
    AudioFocusList &focusInfoList)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    auto itZone = service_->zonesMap_.find(zoneId);
    focusInfoList = {};

    CHECK_AND_RETURN_RET_LOG(itZone != service_->zonesMap_.end(), SUCCESS, "zone %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(itZone->second != nullptr, SUCCESS, "zone %{public}d focusInfoList is null", zoneId);

    focusInfoList = itZone->second->audioFocusInfoList;
    return SUCCESS;
}

int32_t AudioInterruptZoneManager::GetAudioFocusInfoList(const int32_t zoneId,
    const std::string &deviceTag, AudioFocusList &focusInfoList)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    auto itZone = service_->zonesMap_.find(zoneId);
    focusInfoList = {};

    CHECK_AND_RETURN_RET_LOG(itZone != service_->zonesMap_.end(), SUCCESS, "zone %{public}d is not found", zoneId);
    CHECK_AND_RETURN_RET_LOG(itZone->second != nullptr, SUCCESS, "zone %{public}d focusInfoList is null", zoneId);

    for (const auto &focus : itZone->second->audioFocusInfoList) {
        CHECK_AND_CONTINUE(focus.first.deviceTag == deviceTag);
        focusInfoList.emplace_back(focus);
    }
    return SUCCESS;
}

int32_t AudioInterruptZoneManager::CreateAudioInterruptZone(const int32_t zoneId,
    const AudioZoneContext &context, bool checkPermission)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(zoneId >= 0, ERR_INVALID_PARAM, "zone id is invalid");
    if (checkPermission) {
        CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
            "audio zone permission deny");
    }

    auto &tempMap = service_->zonesMap_;
    CHECK_AND_RETURN_RET_LOG(tempMap.find(zoneId) == tempMap.end() || tempMap[zoneId] == nullptr,
        ERR_INVALID_PARAM, "zone %{public}d already exist", zoneId);

    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    CHECK_AND_RETURN_RET_LOG(zone != nullptr, ERROR, "zone %{public}d create interrupt failed", zoneId);

    zone->zoneId = zoneId;
    zone->context = context;
    tempMap[zoneId] = zone;
    return SUCCESS;
}

int32_t AudioInterruptZoneManager::ReleaseAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(func != nullptr, ERR_INVALID_PARAM, "zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "audio zone permission deny");

    auto &tempMap = service_->zonesMap_;
    CHECK_AND_RETURN_RET_LOG(tempMap.find(zoneId) != tempMap.end() && tempMap[zoneId] != nullptr,
        ERR_INVALID_PARAM, "zone %{public}d not exist", zoneId);
    CHECK_AND_RETURN_RET_LOG(zoneId != AudioInterruptService::ZONEID_DEFAULT,
        ERR_INVALID_PARAM, "zone %{public}d is default zone", zoneId);

    bool updateScene = false;
    auto &releaseZone = tempMap[zoneId];
    for (auto it = releaseZone->audioFocusInfoList.begin(); it != releaseZone->audioFocusInfoList.end(); it++) {
        if (((it->second != ACTIVE && it->second != DUCK) ||
            (it->first.streamUsage == STREAM_USAGE_UNKNOWN ||
            it->first.streamUsage == STREAM_USAGE_MEDIA ||
            it->first.streamUsage == STREAM_USAGE_MOVIE)) &&
            releaseZone->context.backStrategy_ == MediaBackStrategy::STOP) {
            ForceStopAudioFocusInZone(zoneId, it->first);
        } else {
            int32_t destZoneId = func(it->first.uid, it->first.deviceTag, "", it->first.streamUsage);
            service_->ActivateAudioInterruptInternal(destZoneId, it->first, false, updateScene);
        }
    }

    tempMap.erase(zoneId);
    return SUCCESS;
}

void AudioInterruptZoneManager::ForceStopAudioFocusInZone(int32_t zoneId, const AudioInterrupt &interrupt)
{
    AUDIO_DEBUG_LOG("force stop interrupt %{public}d,%{public}d,%{public}d of zone %{public}d",
        interrupt.uid, interrupt.pid, interrupt.streamId, zoneId);
    CHECK_AND_RETURN_LOG(service_ != nullptr, "interrupt service is nullptr");

    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    if (service_->handler_ != nullptr) {
        service_->handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, interrupt.streamId);
    }

    service_->sessionService_.RemoveStreamInfo(interrupt.pid, interrupt.streamId);
}

void AudioInterruptZoneManager::SendInterruptEventForMigration(
    const std::pair<AudioInterrupt, AudioFocuState> &audioInterrupt, const int32_t toZoneId)
{
    CHECK_AND_RETURN_LOG(service_ != nullptr, "interrupt service is nullptr");
    auto &tempMap = service_->zonesMap_;
    CHECK_AND_RETURN_LOG(tempMap.find(toZoneId) != tempMap.end() && tempMap[toZoneId] != nullptr,
        "zone %{public}d not exist", toZoneId);

    auto &toZoneFocusInfoList = tempMap[toZoneId]->audioFocusInfoList;
    auto isPresent = [audioInterrupt](const std::pair<AudioInterrupt, AudioFocuState> &item) {
        return item.first.streamId == audioInterrupt.first.streamId &&
            item.first.deviceTag == audioInterrupt.first.deviceTag;
    };
    auto itNewFocus = std::find_if(toZoneFocusInfoList.begin(), toZoneFocusInfoList.end(), isPresent);
    if (itNewFocus != toZoneFocusInfoList.end() && audioInterrupt.second != itNewFocus->second) {
        bool removeFocusInfo = false;
        service_->SendInterruptEvent(audioInterrupt.second, itNewFocus->second, itNewFocus, removeFocusInfo);
    }
}

int32_t AudioInterruptZoneManager::MigrateAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(func != nullptr, ERR_INVALID_PARAM, "zone id is invalid");
    auto &tempMap = service_->zonesMap_;
    CHECK_AND_RETURN_RET_LOG(tempMap.find(zoneId) != tempMap.end() && tempMap[zoneId] != nullptr,
        ERR_INVALID_PARAM, "zone %{public}d not exist", zoneId);

    auto &focusInfoList = tempMap[zoneId]->audioFocusInfoList;
    AUDIO_INFO_LOG("migrate interrupt size %{public}zu from zone %{public}d", focusInfoList.size(), zoneId);
    bool isMigrate = false;
    bool updateScene = false;
    for (auto itFocus = focusInfoList.begin(); itFocus != focusInfoList.end();) {
        int32_t toZoneId = func(itFocus->first.uid, itFocus->first.deviceTag, "", itFocus->first.streamUsage);
        if (toZoneId == zoneId) {
            ++itFocus;
            continue;
        }
        if (itFocus->second == ACTIVE || itFocus->second == DUCK) {
            service_->ActivateAudioInterruptInternal(toZoneId, itFocus->first, false, updateScene);
            SendInterruptEventForMigration(*itFocus, toZoneId);
        } else {
            ForceStopAudioFocusInZone(zoneId, itFocus->first);
        }
        focusInfoList.erase(itFocus++);
        isMigrate = true;
    }
    CHECK_AND_RETURN_RET_LOG(isMigrate, SUCCESS, "no interrupt need migrate");

    ForceStopAllAudioFocusInZone(tempMap[zoneId]);
    if (tempMap[zoneId]->audioFocusInfoList.size() > 0) {
        service_->ResumeAudioFocusList(zoneId, false);
    }
    return SUCCESS;
}

void AudioInterruptZoneManager::ForceStopAllAudioFocusInZone(std::shared_ptr<AudioInterruptZone> &zone)
{
    for (auto it = zone->audioFocusInfoList.begin(); it != zone->audioFocusInfoList.end();) {
        if (it->second == ACTIVE || it->second == DUCK) {
            it++;
            continue;
        }
        ForceStopAudioFocusInZone(zone->zoneId, it->first);
        zone->audioFocusInfoList.erase(it++);
    }
}

int32_t AudioInterruptZoneManager::InjectInterruptToAudioZone(const int32_t zoneId,
    const AudioFocusList &interrupts)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "audio zone permission deny");

    auto &tempMap = service_->zonesMap_;
    CHECK_AND_RETURN_RET_LOG(tempMap.find(zoneId) != tempMap.end() && tempMap[zoneId] != nullptr,
        ERR_INVALID_PARAM, "zone %{public}d not exist", zoneId);

    AUDIO_INFO_LOG("inject interrupt size %{public}zu to zone %{public}d", interrupts.size(), zoneId);
    auto oldFocusList = tempMap[zoneId]->audioFocusInfoList;
    AudioFocusList newFocusList = interrupts;
    for (auto itOld = oldFocusList.begin(); itOld != oldFocusList.end(); itOld++) {
        auto isPresent = [itOld](const std::pair<AudioInterrupt, AudioFocuState> &item) {
            return item.first.streamId == itOld->first.streamId;
        };
        auto itNew = std::find_if(newFocusList.begin(), newFocusList.end(), isPresent);
        if (itNew == newFocusList.end()) {
            ForceStopAudioFocusInZone(zoneId, itOld->first);
        } else {
            AUDIO_INFO_LOG("try to refresh interrupt %{public}d,%{public}d,%{public}d"
                "state from %{public}d to %{public}d",
                itOld->first.uid, itOld->first.pid, itOld->first.streamId,
                itOld->second, itNew->second);
            if (itNew->second != itOld->second) {
                bool removeFocusInfo = false;
                service_->SendInterruptEvent(itOld->second, itNew->second, itOld, removeFocusInfo);
            }
            newFocusList.erase(itNew);
        }
    }

    if (newFocusList.size() > 0) {
        AUDIO_WARNING_LOG("has unexplained new focus for zone %{public}d", zoneId);
    }

    tempMap[zoneId]->audioFocusInfoList = interrupts;
    return SUCCESS;
}

int32_t AudioInterruptZoneManager::InjectInterruptToAudioZone(const int32_t zoneId,
    const std::string &deviceTag, const AudioFocusList &interrupts)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "audio zone permission deny");

    auto &tempMap = service_->zonesMap_;
    CHECK_AND_RETURN_RET_LOG(tempMap.find(zoneId) != tempMap.end() && tempMap[zoneId] != nullptr,
        ERR_INVALID_PARAM, "zone %{public}d not exist", zoneId);
    CHECK_AND_RETURN_RET_LOG(!deviceTag.empty(), ERR_INVALID_PARAM,
        "device tag is invalid for zone %{public}d", zoneId);

    AUDIO_INFO_LOG("inject interrupt size %{public}zu with device tag %{public}s to zone %{public}d",
        interrupts.size(), deviceTag.c_str(), zoneId);
    AudioFocusList newFocusList = interrupts;
    AudioFocusList activeFocusList;
    AudioFocusIterator oldDeviceList = QueryAudioFocusFromZone(zoneId, deviceTag);

    for (auto &itNew : newFocusList) {
        auto isPresent = [itNew, deviceTag](const std::list<std::pair<AudioInterrupt,
            AudioFocuState>>::iterator &iter) {
            return iter->first.streamId == itNew.first.streamId && iter->first.deviceTag == deviceTag;
        };
        auto itOld = std::find_if(oldDeviceList.begin(), oldDeviceList.end(), isPresent);
        if (itOld == oldDeviceList.end()) {
            AUDIO_DEBUG_LOG("record new interrupt %{public}d", itNew.first.streamId);
            itNew.first.deviceTag = deviceTag;
            activeFocusList.emplace_back(itNew);
        } else {
            if (((*itOld)->second != ACTIVE && (*itOld)->second != DUCK) &&
                (itNew.second == ACTIVE || itNew.second == DUCK)) {
                activeFocusList.emplace_back(itNew);
                continue;
            }
            if ((*itOld)->second != itNew.second) {
                AUDIO_DEBUG_LOG("ref interrupt %{public}d state from %{public}d to %{public}d",
                    itNew.first.streamId, (*itOld)->second, itNew.second);
                (*itOld)->second = itNew.second;
            }
            oldDeviceList.erase(itOld);
        }
    }

    AUDIO_DEBUG_LOG("focus list size is %{public}zu for zone %{public}d before remove",
        tempMap[zoneId]->audioFocusInfoList.size(), zoneId);
    RemoveAudioZoneInterrupts(zoneId, oldDeviceList);
    TryActiveAudioFocusForZone(zoneId, activeFocusList);
    return SUCCESS;
}

AudioFocusIterator AudioInterruptZoneManager::QueryAudioFocusFromZone(int32_t zoneId,
    const std::string &deviceTag)
{
    AudioFocusIterator deviceList;
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, deviceList, "service is nullptr");
    CHECK_AND_RETURN_RET_LOG(service_->zonesMap_.find(zoneId) != service_->zonesMap_.end() &&
        service_->zonesMap_[zoneId] != nullptr, deviceList, "zone %{public}d not exist", zoneId);
    auto &focusInfoList = service_->zonesMap_[zoneId]->audioFocusInfoList;
    for (auto it = focusInfoList.begin(); it != focusInfoList.end(); it++) {
        CHECK_AND_CONTINUE(it->first.deviceTag == deviceTag);
        deviceList.emplace_back(it);
    }
    return deviceList;
}

void AudioInterruptZoneManager::RemoveAudioZoneInterrupts(int32_t zoneId, const AudioFocusIterator &focus)
{
    CHECK_AND_RETURN_LOG(service_ != nullptr, "service is nullptr");
    for (auto &it : focus) {
        service_->sessionService_.RemoveStreamInfo(it->first.pid, it->first.streamId);
        AUDIO_DEBUG_LOG("remove interrupt %{public}d from zone %{public}d",
            it->first.streamId, zoneId);
        CHECK_AND_CONTINUE(service_->zonesMap_.find(zoneId) != service_->zonesMap_.end() &&
            service_->zonesMap_[zoneId] != nullptr);
        service_->zonesMap_[zoneId]->audioFocusInfoList.erase(it);
    }
}

void AudioInterruptZoneManager::TryActiveAudioFocusForZone(int32_t zoneId, AudioFocusList &activeFocusList)
{
    CHECK_AND_RETURN_LOG(service_ != nullptr, "service is nullptr");
    CHECK_AND_RETURN_LOG(service_->zonesMap_.find(zoneId) != service_->zonesMap_.end() &&
        service_->zonesMap_[zoneId] != nullptr, "zone %{public}d not exist", zoneId);
    AUDIO_DEBUG_LOG("focus list size is %{public}zu for zone %{public}d before active",
        service_->zonesMap_[zoneId]->audioFocusInfoList.size(), zoneId);
    if (activeFocusList.size() > 0) {
        for (auto itActive : activeFocusList) {
            AUDIO_DEBUG_LOG("active new interrupt %{public}d", itActive.first.streamId);
            bool updateScene = false;
            service_->ActivateAudioInterruptInternal(zoneId, itActive.first, false, updateScene);
        }
    } else {
        TryResumeAudioFocusForZone(zoneId);
    }
    AUDIO_DEBUG_LOG("focus list size is %{public}zu for zone %{public}d after active",
        service_->zonesMap_[zoneId]->audioFocusInfoList.size(), zoneId);
}

void AudioInterruptZoneManager::TryResumeAudioFocusForZone(int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(service_ != nullptr, "service is nullptr");
    CHECK_AND_RETURN_LOG(service_->zonesMap_.find(zoneId) != service_->zonesMap_.end() &&
        service_->zonesMap_[zoneId] != nullptr, "zone %{public}d not exist", zoneId);
    AUDIO_DEBUG_LOG("try resume audio focus list for zone %{public}d", zoneId);
    auto &focusList = service_->zonesMap_[zoneId]->audioFocusInfoList;
    CHECK_AND_RETURN_LOG(focusList.size() > 0, "focus list is empty");

    for (auto it = focusList.begin(); it != focusList.end(); ++it) {
        CHECK_AND_RETURN(it->second != ACTIVE);
    }
    service_->ResumeAudioFocusList(zoneId, false);
}

int32_t AudioInterruptZoneManager::FindZoneByPid(int32_t pid)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, AudioInterruptService::ZONEID_DEFAULT, "service is nullptr");
    for (const auto &zone : service_->zonesMap_) {
        CHECK_AND_CONTINUE(zone.second != nullptr);

        for (const auto &it : zone.second->audioFocusInfoList) {
            CHECK_AND_CONTINUE(it.first.pid == pid);
            return zone.first;
        }
    }
    AUDIO_WARNING_LOG("pid %{public}d not in audio zone, use default", pid);
    return AudioInterruptService::ZONEID_DEFAULT;
}

bool AudioInterruptZoneManager::CheckAudioInterruptZonePermission()
{
    AUDIO_INFO_LOG("audiointerruptzone permission: %{public}d", PermissionUtil::VerifySystemPermission());
    return PermissionUtil::VerifySystemPermission();
}
} // namespace AudioStandard
} // namespace OHOS
