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
    if (itZone != service_->zonesMap_.end() && itZone->second != nullptr) {
        focusInfoList = itZone->second->audioFocusInfoList;
    } else {
        focusInfoList = {};
    }

    return SUCCESS;
}

int32_t AudioInterruptZoneManager::GetAudioFocusInfoList(const int32_t zoneId,
    const std::string &deviceTag, AudioFocusList &focusInfoList)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    auto itZone = service_->zonesMap_.find(zoneId);
    if (itZone != service_->zonesMap_.end() && itZone->second != nullptr) {
        for (const auto &focus : itZone->second->audioFocusInfoList) {
            if (focus.first.deviceTag != deviceTag) {
                continue;
            }
            focusInfoList.emplace_back(focus);
        }
    } else {
        focusInfoList = {};
    }

    return SUCCESS;
}

int32_t AudioInterruptZoneManager::CreateAudioInterruptZone(const int32_t zoneId,
    AudioZoneFocusStrategy focusStrategy, bool checkPermission)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(zoneId >= 0, ERR_INVALID_PARAM, "zone id is invalid");
    if (checkPermission) {
        CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
            "audio zone permission deny");
    }

    auto &tempMap = service_->zonesMap_;
    if (tempMap.find(zoneId) != tempMap.end() && tempMap[zoneId] != nullptr) {
        AUDIO_INFO_LOG("zone %{public}d already exist", zoneId);
        return ERR_INVALID_PARAM;
    }

    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (zone == nullptr) {
        return ERROR;
    }
    zone->zoneId = zoneId;
    zone->focusStrategy = focusStrategy;
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
    if (tempMap.find(zoneId) == tempMap.end() || tempMap[zoneId] == nullptr) {
        AUDIO_WARNING_LOG("zone %{public}d not exist", zoneId);
        return ERR_INVALID_PARAM;
    }
    if (zoneId == AudioInterruptService::ZONEID_DEFAULT) {
        return ERR_INVALID_PARAM;
    }

    bool updateScene = false;
    auto &releaseZone = tempMap[zoneId];
    for (auto it = releaseZone->audioFocusInfoList.begin(); it != releaseZone->audioFocusInfoList.end(); it++) {
        if ((it->second != ACTIVE && it->second != DUCK) ||
            (it->first.streamUsage == STREAM_USAGE_UNKNOWN ||
            it->first.streamUsage == STREAM_USAGE_MEDIA ||
            it->first.streamUsage == STREAM_USAGE_MOVIE)) {
            ForceStopAudioFocusInZone(zoneId, it->first);
        } else {
            int32_t destZoneId = func(it->first.uid, it->first.deviceTag, "");
            service_->ActivateAudioInterruptInternal(zoneId, it->first, false, updateScene);
        }
    }

    tempMap.erase(zoneId);
    return SUCCESS;
}

void AudioInterruptZoneManager::ForceStopAudioFocusInZone(int32_t zoneId, const AudioInterrupt &interrupt)
{
    AUDIO_DEBUG_LOG("force stop interrupt %{public}d,%{public}d,%{public}d of zone %{public}d",
        interrupt.uid, interrupt.pid, interrupt.streamId, zoneId);

    CHECK_AND_RETURN_LOG(service_->sessionService_ != nullptr, "session service is nullptr");
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    if (service_->handler_ != nullptr) {
        service_->handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, interrupt.streamId);
    }

    auto audioSession = service_->sessionService_->GetAudioSessionByPid(interrupt.pid);
    if (audioSession != nullptr) {
        audioSession->RemoveAudioInterrptByStreamId(interrupt.streamId);
    }
}

int32_t AudioInterruptZoneManager::MigrateAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(func != nullptr, ERR_INVALID_PARAM, "zone id is invalid");
    auto &tempMap = service_->zonesMap_;
    if (tempMap.find(zoneId) == tempMap.end() || tempMap[zoneId] == nullptr) {
        AUDIO_WARNING_LOG("zone %{public}d not exist", zoneId);
        return ERR_INVALID_PARAM;
    }

    auto &focusInfoList = tempMap[zoneId]->audioFocusInfoList;
    AUDIO_INFO_LOG("migrate interrupt size %{public}zu from zone %{public}d", focusInfoList.size(), zoneId);
    bool isMigrate = false;
    bool updateScene = false;
    for (auto itFocus = focusInfoList.begin(); itFocus != focusInfoList.end();) {
        int32_t toZoneId = func(itFocus->first.uid, itFocus->first.deviceTag, "");
        if (toZoneId == zoneId) {
            ++itFocus;
            continue;
        }
        if (itFocus->second == ACTIVE) {
            service_->ActivateAudioInterruptInternal(toZoneId, itFocus->first, false, updateScene);
        } else {
            ForceStopAudioFocusInZone(zoneId, itFocus->first);
        }
        focusInfoList.erase(itFocus++);
        isMigrate = true;
    }
    if (!isMigrate) {
        return SUCCESS;
    }

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

int32_t AudioInterruptZoneManager::InjectInterruptToAudiotZone(const int32_t zoneId,
    const AudioFocusList &interrupts)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "audio zone permission deny");

    auto &tempMap = service_->zonesMap_;
    if (tempMap.find(zoneId) == tempMap.end() || tempMap[zoneId] == nullptr) {
        AUDIO_WARNING_LOG("zone %{public}d not exist", zoneId);
        return ERR_INVALID_PARAM;
    }

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

int32_t AudioInterruptZoneManager::InjectInterruptToAudiotZone(const int32_t zoneId,
    const std::string &deviceTag, const AudioFocusList &interrupts)
{
    CHECK_AND_RETURN_RET_LOG(service_ != nullptr, ERR_INVALID_PARAM, "interrupt service is nullptr");
    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "audio zone permission deny");

    auto &tempMap = service_->zonesMap_;
    if (tempMap.find(zoneId) == tempMap.end() || tempMap[zoneId] == nullptr) {
        AUDIO_WARNING_LOG("zone %{public}d not exist", zoneId);
        return ERR_INVALID_PARAM;
    }
    if (deviceTag.empty()) {
        AUDIO_WARNING_LOG("device tag is invalid for zone %{public}d", zoneId);
        return ERR_INVALID_PARAM;
    }

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
    auto &focusInfoList = service_->zonesMap_[zoneId]->audioFocusInfoList;
    AudioFocusIterator deviceList;
    for (auto it = focusInfoList.begin(); it != focusInfoList.end(); it++) {
        if (it->first.deviceTag != deviceTag) {
            continue;
        }
        deviceList.emplace_back(it);
    }
    return deviceList;
}

void AudioInterruptZoneManager::RemoveAudioZoneInterrupts(int32_t zoneId, const AudioFocusIterator &focus)
{
    CHECK_AND_RETURN_LOG(service_->sessionService_ != nullptr, "session service is nullptr");
    for (auto &it : focus) {
        auto audioSession = service_->sessionService_->GetAudioSessionByPid(it->first.pid);
        if (audioSession != nullptr) {
            audioSession->RemoveAudioInterrptByStreamId(it->first.streamId);
        }
        AUDIO_DEBUG_LOG("remove interrupt %{public}d from zone %{public}d",
            it->first.streamId, zoneId);
        service_->zonesMap_[zoneId]->audioFocusInfoList.erase(it);
    }
}

void AudioInterruptZoneManager::TryActiveAudioFocusForZone(int32_t zoneId, AudioFocusList &activeFocusList)
{
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
    AUDIO_DEBUG_LOG("try resume audio focus list for zone %{public}d", zoneId);
    auto &focusList = service_->zonesMap_[zoneId]->audioFocusInfoList;
    if (focusList.size() == 0) {
        return;
    }
    for (auto it = focusList.begin(); it != focusList.end(); ++it) {
        if (it->second == ACTIVE) {
            return;
        }
    }
    service_->ResumeAudioFocusList(zoneId, false);
}

int32_t AudioInterruptZoneManager::FindZoneByPid(int32_t pid)
{
    for (const auto &zone : service_->zonesMap_) {
        if (zone.second == nullptr) {
            continue;
        }

        for (const auto &it : zone.second->audioFocusInfoList) {
            if (it.first.pid == pid) {
                return zone.first;
            }
        }
    }
    AUDIO_WARNING_LOG("pid %{public}d not in audio zone, use default", pid);
    return AudioInterruptService::ZONEID_DEFAULT;
}

bool AudioInterruptZoneManager::CheckAudioInterruptZonePermission()
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid == UID_AUDIO) {
        return true;
    }
    return false;
}
} // namespace AudioStandard
} // namespace OHOS
