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
#define LOG_TAG "SleAudioDeviceManager"
#endif

#include "sle_audio_device_manager.h"

#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_policy_utils.h"

#include "client_type_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const std::map<uint32_t, std::set<StreamUsage>> STREAM_USAGE_TO_SLE_STREAM_TYPE = {
    {SLE_AUDIO_STREAM_NONE, {}},
    {SLE_AUDIO_STREAM_UNDEFINED, {STREAM_USAGE_ULTRASONIC, STREAM_USAGE_ENFORCED_TONE, STREAM_USAGE_DTMF,
        STREAM_USAGE_ALARM, STREAM_USAGE_NOTIFICATION, STREAM_USAGE_SYSTEM,
        STREAM_USAGE_ACCESSIBILITY, STREAM_USAGE_VOICE_MESSAGE}},
    {SLE_AUDIO_STREAM_MUSIC, {STREAM_USAGE_UNKNOWN, STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC, STREAM_USAGE_AUDIOBOOK,
        STREAM_USAGE_VOICE_ASSISTANT}},
    {SLE_AUDIO_STREAM_VOICE_CALL, {STREAM_USAGE_VOICE_MODEM_COMMUNICATION}},
    {SLE_AUDIO_STREAM_VOICE_ASSISTANT, {}},
    {SLE_AUDIO_STREAM_RING, {STREAM_USAGE_NOTIFICATION_RINGTONE, STREAM_USAGE_RINGTONE, STREAM_USAGE_RANGING,
        STREAM_USAGE_VOICE_RINGTONE}},
    {SLE_AUDIO_STREAM_VOIP, {STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_VIDEO_COMMUNICATION,
        STREAM_USAGE_VOICE_CALL_ASSISTANT}},
    {SLE_AUDIO_STREAM_GAME, {STREAM_USAGE_GAME}},
    {SLE_AUDIO_STREAM_ALERT, {}},
    {SLE_AUDIO_STREAM_VIDEO, {STREAM_USAGE_MOVIE}},
    {SLE_AUDIO_STREAM_GUID, {STREAM_USAGE_NAVIGATION}}
};

const std::map<uint32_t, std::set<SourceType>> SOURCE_TYPE_TO_SLE_STREAM_TYPE = {
    {SLE_AUDIO_STREAM_NONE, {}},
    {SLE_AUDIO_STREAM_VOICE_CALL, {SOURCE_TYPE_VIRTUAL_CAPTURE, SOURCE_TYPE_VOICE_CALL}},
    {SLE_AUDIO_STREAM_VOIP, {SOURCE_TYPE_VOICE_COMMUNICATION}},
    {SLE_AUDIO_STREAM_VOICE_ASSISTANT, {SOURCE_TYPE_VOICE_RECOGNITION, SOURCE_TYPE_VOICE_TRANSCRIPTION}},
    {SLE_AUDIO_STREAM_RECORD, {SOURCE_TYPE_MIC, SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_VOICE_MESSAGE,
        SOURCE_TYPE_CAMCORDER, SOURCE_TYPE_UNPROCESSED, SOURCE_TYPE_LIVE}}
};

const std::vector<StreamUsage> GAME_FAST_SUPPORTED_USAGES = {
    STREAM_USAGE_UNKNOWN, STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC, STREAM_USAGE_GAME, STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_MOVIE
};
} // namespace
int32_t SleAudioDeviceManager::SetSleAudioOperationCallback(const sptr<IStandardSleAudioOperationCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    callback_ = callback;
    return SUCCESS;
}

void SleAudioDeviceManager::GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "callback is nullptr");
    callback_->GetSleAudioDeviceList(devices);
}

void SleAudioDeviceManager::GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "callback is nullptr");
    callback_->GetSleVirtualAudioDeviceList(devices);
}

bool SleAudioDeviceManager::IsInBandRingOpen(const std::string &device) const
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, false, "callback is nullptr");
    bool ret = false;
    callback_->IsInBandRingOpen(device, ret);
    return ret;
}

uint32_t SleAudioDeviceManager::GetSupportStreamType(const std::string &device) const
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    uint32_t retType = static_cast<uint32_t>(ERROR);
    callback_->GetSupportStreamType(device, retType);
    return retType;
}

int32_t SleAudioDeviceManager::SetActiveSinkDevice(const std::string &device, uint32_t streamType)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    int32_t ret = ERROR;
    callback_->SetActiveSinkDevice(device, streamType, ret);
    return ret;
}

int32_t SleAudioDeviceManager::StartPlaying(const std::string &device, uint32_t streamType, int32_t timeoutMs)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    AUDIO_INFO_LOG("device [%{public}s] sle streamType [%{public}u]",
        AudioPolicyUtils::GetInstance().GetEncryptAddr(device).c_str(), streamType);
    std::lock_guard<std::mutex> lock(startedSleStreamTypeMutex_);
    int32_t ret = ERROR;
    if (startedSleStreamType_[device][streamType].isStarted) {
        AUDIO_INFO_LOG("sle stream type %{public}u is already started", streamType);
        return SUCCESS;
    }
    callback_->StartPlaying(device, streamType, timeoutMs, ret);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "startplaying failed");
    startedSleStreamType_[device][streamType].isStarted = true;

    return ret;
}

int32_t SleAudioDeviceManager::StopPlaying(const std::string &device, uint32_t streamType)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t ret = ERROR;
    AUDIO_INFO_LOG("device [%{public}s] sle streamType [%{public}u]",
        AudioPolicyUtils::GetInstance().GetEncryptAddr(device).c_str(), streamType);
    callback_->StopPlaying(device, streamType, ret);
    return ret;
}

int32_t SleAudioDeviceManager::ConnectAllowedProfiles(const std::string &remoteAddr) const
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    int32_t ret = ERROR;
    callback_->ConnectAllowedProfiles(remoteAddr, ret);
    return ret;
}

int32_t SleAudioDeviceManager::SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    int32_t ret = ERROR;
    callback_->SetDeviceAbsVolume(remoteAddr, volume, streamType, ret);
    return ret;
}

int32_t SleAudioDeviceManager::SendUserSelection(const std::string &device,
    uint32_t streamType, int32_t eventType)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    int32_t ret = ERROR;
    callback_->SendUserSelection(device, streamType, eventType, ret);
    return ret;
}

int32_t SleAudioDeviceManager::GetRenderPosition(const std::string &device, uint32_t &delayValue)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return callback_->GetRenderPosition(device, delayValue);
}

bool IsSupportedByGameFastPath(bool isGameApp, StreamUsage usage)
{
    return isGameApp && (std::find(GAME_FAST_SUPPORTED_USAGES.begin(), GAME_FAST_SUPPORTED_USAGES.end(), usage) !=
        GAME_FAST_SUPPORTED_USAGES.end());
}

bool SleAudioDeviceManager::IsGameApp(int32_t uid)
{
    std::unique_lock<std::mutex> lock(clientTypeMapMutex_);
    auto it = clientTypeMap_.find(uid);
    if (it == clientTypeMap_.end()) {
        auto clientType = ClientTypeManager::GetInstance()->GetClientTypeByUidSync(uid);
        clientTypeMap_.insert_or_assign(uid, clientType == CLIENT_TYPE_GAME);
        AUDIO_INFO_LOG("uid [%{public}d] clientType is [%{public}d]", uid, clientType);
        return clientType == CLIENT_TYPE_GAME;
    }
    return it->second;
}

uint32_t SleAudioDeviceManager::GetSleStreamTypeByStreamUsage(StreamUsage streamUsage, int32_t uid)
{
    bool isGameApp = IsGameApp(uid);
    for (const auto &pair : STREAM_USAGE_TO_SLE_STREAM_TYPE) {
        if (IsSupportedByGameFastPath(isGameApp, streamUsage)) {
            return SLE_AUDIO_STREAM_GAME;
        }
        if (pair.second.find(streamUsage) != pair.second.end()) {
            return pair.first;
        }
    }
    return STREAM_USAGE_TO_SLE_STREAM_TYPE.begin()->first; // Default to NONE
}

uint32_t SleAudioDeviceManager::GetSleStreamTypeBySourceType(SourceType sourceType) const
{
    for (const auto &pair : SOURCE_TYPE_TO_SLE_STREAM_TYPE) {
        if (pair.second.find(sourceType) != pair.second.end()) {
            return pair.first;
        }
    }
    return SOURCE_TYPE_TO_SLE_STREAM_TYPE.begin()->first; // Default to NONE
}

std::set<StreamUsage> SleAudioDeviceManager::GetStreamUsagesBySleStreamType(uint32_t streamType) const
{
    if (STREAM_USAGE_TO_SLE_STREAM_TYPE.contains(streamType)) {
        return STREAM_USAGE_TO_SLE_STREAM_TYPE.at(streamType);
    }

    return std::set<StreamUsage>();
}

std::set<SourceType> SleAudioDeviceManager::GetSourceTypesBySleStreamType(uint32_t streamType) const
{
    if (SOURCE_TYPE_TO_SLE_STREAM_TYPE.contains(streamType)) {
        return SOURCE_TYPE_TO_SLE_STREAM_TYPE.at(streamType);
    }

    return std::set<SourceType>();
}

int32_t SleAudioDeviceManager::SetActiveDevice(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage,
    int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(IsNearlinkDevice(deviceDesc.deviceType_), ERROR, "device type is not nearlink");
    return SetActiveSinkDevice(deviceDesc.macAddress_, GetSleStreamTypeByStreamUsage(streamUsage, uid));
}

int32_t SleAudioDeviceManager::SetActiveDevice(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType,
    int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(IsNearlinkDevice(deviceDesc.deviceType_), ERROR, "device type is not nearlink");
    return SetActiveSinkDevice(deviceDesc.macAddress_, GetSleStreamTypeBySourceType(sourceType));
}

int32_t SleAudioDeviceManager::StartPlaying(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage,
    int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK, ERROR, "device type is not nearlink");
    static const unordered_set<StreamUsage> EXTENDED_WAITING_USAGES = {
        STREAM_USAGE_VOICE_MESSAGE, STREAM_USAGE_VOICE_ASSISTANT, STREAM_USAGE_NAVIGATION
    };
    return StartPlaying(deviceDesc.macAddress_, GetSleStreamTypeByStreamUsage(streamUsage, uid),
        EXTENDED_WAITING_USAGES.contains(streamUsage) ?
            SLE_EXTENDED_TIMEOUT_MS : SLE_STANDARD_TIMEOUT_MS);
}

int32_t SleAudioDeviceManager::StartPlaying(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType,
    int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK_IN, ERROR, "device type is not nearlink");
    static const unordered_set<SourceType> EXTENDED_WAITING_TYPES = {
        SOURCE_TYPE_VOICE_RECOGNITION
    };
    return StartPlaying(deviceDesc.macAddress_, GetSleStreamTypeBySourceType(sourceType),
        EXTENDED_WAITING_TYPES.contains(sourceType) ?
            SLE_EXTENDED_TIMEOUT_MS : SLE_STANDARD_TIMEOUT_MS);
}

int32_t SleAudioDeviceManager::StopPlaying(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage,
    int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK, ERROR, "device type is not nearlink");
    return StopPlaying(deviceDesc.macAddress_, GetSleStreamTypeByStreamUsage(streamUsage, uid));
}

int32_t SleAudioDeviceManager::StopPlaying(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType,
    int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK_IN, ERROR, "device type is not nearlink");
    return StopPlaying(deviceDesc.macAddress_, GetSleStreamTypeBySourceType(sourceType));
}

int32_t SleAudioDeviceManager::SetDeviceAbsVolume(const std::string &device, AudioStreamType streamType, int32_t volume)
{
    CHECK_AND_RETURN_RET_LOG(volume >= 0, ERR_INVALID_PARAM, "volume is invalid");

    auto it = deviceVolumeConfigInfo_.find(device);
    CHECK_AND_RETURN_RET_LOG(it != deviceVolumeConfigInfo_.end(), ERR_INVALID_PARAM, "device not found");

    int32_t ret = SUCCESS;
    if (streamType == STREAM_MUSIC) {
        ret = SetDeviceAbsVolume(device, static_cast<uint32_t>(volume), SLE_AUDIO_STREAM_MUSIC);
    } else {
        ret = SetDeviceAbsVolume(device, static_cast<uint32_t>(volume), SLE_AUDIO_STREAM_VOICE_CALL);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "set device to nearlink failed");

    return ret;
}

int32_t SleAudioDeviceManager::SendUserSelection(const AudioDeviceDescriptor &deviceDesc,
    StreamUsage streamUsage, SleSelectState eventType)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK, ERROR, "device type is not nearlink");
    SleAudioStreamType sleStreamType = AudioPolicyUtils::GetInstance().IsVoiceStreamType(streamUsage) ?
        SLE_AUDIO_STREAM_VOICE_CALL : SLE_AUDIO_STREAM_MUSIC;
    return SendUserSelection(deviceDesc.macAddress_, sleStreamType, static_cast<int32_t>(eventType));
}

int32_t SleAudioDeviceManager::SendUserSelection(const AudioDeviceDescriptor &deviceDesc,
    SourceType sourceType, SleSelectState eventType)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK_IN, ERROR, "device type is not nearlink");
    SleAudioStreamType sleStreamType = AudioPolicyUtils::GetInstance().IsVoiceSourceType(sourceType) ?
        SLE_AUDIO_STREAM_VOICE_CALL : SLE_AUDIO_STREAM_MUSIC;
    return SendUserSelection(deviceDesc.macAddress_, sleStreamType, static_cast<int32_t>(eventType));
}

int32_t SleAudioDeviceManager::AddNearlinkDevice(const AudioDeviceDescriptor &deviceDesc)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK &&
        deviceDesc.connectState_ == CONNECTED, ERROR, "device type is not nearlink");
    std::lock_guard<std::mutex> lock(deviceVolumeConfigMutex_);
    deviceVolumeConfigInfo_[deviceDesc.macAddress_] =
        std::make_pair(SleVolumeConfigInfo{STREAM_MUSIC, deviceDesc.mediaVolume_},
        SleVolumeConfigInfo{STREAM_VOICE_CALL, deviceDesc.callVolume_});
    return SUCCESS;
}

int32_t SleAudioDeviceManager::RemoveNearlinkDevice(const AudioDeviceDescriptor &deviceDesc)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK &&
        deviceDesc.connectState_ == CONNECTED, ERROR, "device type is not nearlink");
    std::lock_guard<std::mutex> lock(deviceVolumeConfigMutex_);
    deviceVolumeConfigInfo_.erase(deviceDesc.macAddress_);
    startedSleStreamType_.erase(deviceDesc.macAddress_);
    return SUCCESS;
}

bool SleAudioDeviceManager::IsNearlinkDevice(DeviceType deviceType)
{
    return deviceType == DEVICE_TYPE_NEARLINK || deviceType == DEVICE_TYPE_NEARLINK_IN;
}

bool SleAudioDeviceManager::IsMoveToNearlinkDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() != 0 && streamDesc->newDeviceDescs_[0] != nullptr,
        false, "newDeviceDescs is invaild");
    return IsNearlinkDevice(streamDesc->newDeviceDescs_[0]->deviceType_);
}

bool SleAudioDeviceManager::IsNearlinkMoveToOtherDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() != 0 && streamDesc->newDeviceDescs_[0] != nullptr,
        false, "newDeviceDescs is invaild");
    if (streamDesc->oldDeviceDescs_.size() != 0 && streamDesc->oldDeviceDescs_[0] != nullptr &&
        IsNearlinkDevice(streamDesc->oldDeviceDescs_[0]->deviceType_)) {
            return !streamDesc->oldDeviceDescs_[0]->IsSameDeviceDescPtr(streamDesc->newDeviceDescs_[0]);
    }
    return false;
}

void SleAudioDeviceManager::UpdateStreamTypeMap(const std::string &deviceAddr, uint32_t streamType,
    uint32_t sessionId, bool isAdd)
{
    std::lock_guard<std::mutex> lock(startedSleStreamTypeMutex_);
    auto &sessionSet = startedSleStreamType_[deviceAddr][streamType].sessionIds;
    AUDIO_INFO_LOG("sle device %{public}s, add [%{public}d] streamType %{public}u sessionId %{public}d",
        AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceAddr).c_str(), isAdd, streamType, sessionId);
    if (isAdd) {
        sessionSet.insert(sessionId);
    } else {
        bool isErased = sessionSet.erase(sessionId) > 0;
        if (isErased && sessionSet.empty()) {
            StopPlaying(deviceAddr, streamType);
            startedSleStreamType_[deviceAddr][streamType].isStarted = false;
        }
    }
}

void SleAudioDeviceManager::UpdateSleStreamTypeCount(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    bool isRemoved)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");

    uint32_t sessionId = streamDesc->sessionId_;
    uint32_t streamType = 0;
    std::string newDeviceAddr = "";
    std::string oldDeviceAddr = "";
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;

        if (IsNearlinkMoveToOtherDevice(streamDesc)) {
            streamType = GetSleStreamTypeByStreamUsage(streamUsage, streamDesc->GetRealUid());
            oldDeviceAddr = streamDesc->oldDeviceDescs_[0]->macAddress_;
            UpdateStreamTypeMap(oldDeviceAddr, streamType, sessionId, false);
        }
        if (IsMoveToNearlinkDevice(streamDesc)) {
            streamType = GetSleStreamTypeByStreamUsage(streamUsage, streamDesc->GetRealUid());
            newDeviceAddr = streamDesc->newDeviceDescs_[0]->macAddress_;
            if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
                UpdateStreamTypeMap(newDeviceAddr, streamType, sessionId, true);
            } else {
                AUDIO_INFO_LOG("session %{public}d is not running", sessionId);
                UpdateStreamTypeMap(newDeviceAddr, streamType, sessionId, false);
            }
            if (isRemoved) {
                UpdateStreamTypeMap(oldDeviceAddr, streamType, sessionId, false);
            }
        }
    } else {
        SourceType sourceType = streamDesc->capturerInfo_.sourceType;
        streamType = GetSleStreamTypeBySourceType(sourceType);

        if (IsNearlinkMoveToOtherDevice(streamDesc)) {
            oldDeviceAddr = streamDesc->oldDeviceDescs_[0]->macAddress_;
            UpdateStreamTypeMap(oldDeviceAddr, streamType, sessionId, false);
        }
        if (IsMoveToNearlinkDevice(streamDesc)) {
            newDeviceAddr = streamDesc->newDeviceDescs_[0]->macAddress_;
            if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
                UpdateStreamTypeMap(newDeviceAddr, streamType, sessionId, true);
            } else {
                AUDIO_INFO_LOG("session %{public}d is not running", sessionId);
                UpdateStreamTypeMap(newDeviceAddr, streamType, sessionId, false);
            }
            if (isRemoved) {
                UpdateStreamTypeMap(oldDeviceAddr, streamType, sessionId, false);
            }
        }
    }
}

void SleAudioDeviceManager::ResetSleStreamTypeCount(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");

    std::lock_guard<std::mutex> lock(startedSleStreamTypeMutex_);
    auto it = startedSleStreamType_.find(deviceDesc->macAddress_);
    CHECK_AND_RETURN_LOG(it != startedSleStreamType_.end(), "device %{public}s not found",
        AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->macAddress_).c_str());

    for (const auto &pair : it->second) {
        uint32_t streamType = pair.first;
        CHECK_AND_CONTINUE_LOG(!pair.second.sessionIds.empty(), "streamType %{public}u has no session", streamType);
        StopPlaying(deviceDesc->macAddress_, streamType);
    }

    SetActiveSinkDevice(deviceDesc->macAddress_, SLE_AUDIO_STREAM_NONE);

    startedSleStreamType_.erase(it);
}

std::unordered_map<uint32_t, std::unordered_set<uint32_t>> SleAudioDeviceManager::GetNearlinkStreamTypeMapByDevice(
    const std::string &deviceAddr)
{
    std::lock_guard<std::mutex> lock(startedSleStreamTypeMutex_);
    auto it = startedSleStreamType_.find(deviceAddr);
    auto ret = std::unordered_map<uint32_t, std::unordered_set<uint32_t>>();
    if (it != startedSleStreamType_.end()) {
        for (const auto &pair : it->second) {
            uint32_t streamType = pair.first;
            std::unordered_set<uint32_t> sessionSet = pair.second.sessionIds;
            if (!sessionSet.empty()) {
                ret[streamType] = sessionSet;
            }
        }
    }
    return ret;
}

int32_t SleAudioDeviceManager::SetNearlinkDeviceMute(const std::string &device, AudioStreamType streamType, bool isMute)
{
    std::lock_guard<std::mutex> lock(deviceVolumeConfigMutex_);
    CHECK_AND_RETURN_RET_LOG(deviceVolumeConfigInfo_.find(device) != deviceVolumeConfigInfo_.end(),
        ERR_INVALID_PARAM, "device not found");
    if (streamType == STREAM_MUSIC) {
        deviceVolumeConfigInfo_[device].first.isMute = isMute;
    } else {
        AUDIO_ERR_LOG("voice call cannot set mute");
    }
    return SUCCESS;
}

int32_t SleAudioDeviceManager::SetNearlinkDeviceVolumeLevel(const std::string &device, AudioStreamType streamType,
    const int32_t volumeLevel)
{
    std::lock_guard<std::mutex> lock(deviceVolumeConfigMutex_);
    CHECK_AND_RETURN_RET_LOG(deviceVolumeConfigInfo_.find(device) != deviceVolumeConfigInfo_.end(),
        ERR_INVALID_PARAM, "device not found");
    if (streamType == STREAM_MUSIC) {
        deviceVolumeConfigInfo_[device].first.volumeLevel = volumeLevel;
    } else if (streamType == STREAM_VOICE_CALL && volumeLevel > 0) {
        deviceVolumeConfigInfo_[device].second.volumeLevel = volumeLevel;
    }
    return SUCCESS;
}

int32_t SleAudioDeviceManager::GetVolumeLevelByVolumeType(AudioVolumeType volumeType,
    const AudioDeviceDescriptor &deviceDesc)
{
    std::lock_guard<std::mutex> lock(deviceVolumeConfigMutex_);
    CHECK_AND_RETURN_RET_LOG(deviceVolumeConfigInfo_.find(deviceDesc.macAddress_) != deviceVolumeConfigInfo_.end(),
        ERR_INVALID_PARAM, "device not found");
    if (volumeType == STREAM_MUSIC) {
        return deviceVolumeConfigInfo_[deviceDesc.macAddress_].first.isMute ? 0 :
            deviceVolumeConfigInfo_[deviceDesc.macAddress_].first.volumeLevel;
    } else if (volumeType == STREAM_VOICE_CALL) {
        return deviceVolumeConfigInfo_[deviceDesc.macAddress_].second.volumeLevel;
    }
    return 0;
}
} // namespace AudioStandard
} // namespace OHOS
