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

#include "OHAudioVolumeManager.h"
#include <set>

#include "audio_common_log.h"
#include "audio_system_manager.h"
#include "audio_common_utils.h"

namespace {
const std::set<OH_AudioStream_Usage> VALID_OH_STREAM_USAGES = {
    AUDIOSTREAM_USAGE_UNKNOWN,
    AUDIOSTREAM_USAGE_MUSIC,
    AUDIOSTREAM_USAGE_VOICE_COMMUNICATION,
    AUDIOSTREAM_USAGE_VOICE_ASSISTANT,
    AUDIOSTREAM_USAGE_ALARM,
    AUDIOSTREAM_USAGE_VOICE_MESSAGE,
    AUDIOSTREAM_USAGE_RINGTONE,
    AUDIOSTREAM_USAGE_NOTIFICATION,
    AUDIOSTREAM_USAGE_ACCESSIBILITY,
    AUDIOSTREAM_USAGE_MOVIE,
    AUDIOSTREAM_USAGE_GAME,
    AUDIOSTREAM_USAGE_AUDIOBOOK,
    AUDIOSTREAM_USAGE_NAVIGATION,
    AUDIOSTREAM_USAGE_VIDEO_COMMUNICATION
};

const std::set<OH_AudioRingerMode> VALID_OH_RINGER_MODES = {
    AUDIO_RINGER_MODE_SILENT,
    AUDIO_RINGER_MODE_VIBRATE,
    AUDIO_RINGER_MODE_NORMAL
};
}

using OHOS::AudioStandard::OHAudioVolumeManager;
using OHOS::AudioStandard::StreamUsage;

OH_AudioCommon_Result OH_AudioManager_GetAudioVolumeManager(
    OH_AudioVolumeManager **volumeManager)
{
    if (volumeManager == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioVolumeManager");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *manager = OHAudioVolumeManager::GetInstance();
    *volumeManager = (OH_AudioVolumeManager *)manager;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioVolumeManager_GetMaxVolumeByUsage(OH_AudioVolumeManager *volumeManager,
    OH_AudioStream_Usage usage, int32_t *maxVolumeLevel)
{
    if (volumeManager == nullptr || !VALID_OH_STREAM_USAGES.count(usage) || maxVolumeLevel == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or usage or maxVolumeLevel");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    int32_t volumeLevel = ohAudioVolumeManager->GetMaxVolumeByUsage(streamUsage);
    if (volumeLevel < 0) {
        AUDIO_ERR_LOG("GetMaxVolumeByUsage failed");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    *maxVolumeLevel = volumeLevel;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioVolumeManager_GetMinVolumeByUsage(OH_AudioVolumeManager *volumeManager,
    OH_AudioStream_Usage usage, int32_t *minVolumeLevel)
{
    if (volumeManager == nullptr || !VALID_OH_STREAM_USAGES.count(usage) || minVolumeLevel == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or usage or maxVolumeLevel");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    int32_t volumeLevel = ohAudioVolumeManager->GetMinVolumeByUsage(streamUsage);
    if (volumeLevel < 0) {
        AUDIO_ERR_LOG("GetMinVolumeByUsage failed");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    *minVolumeLevel = volumeLevel;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioVolumeManager_GetVolumeByUsage(OH_AudioVolumeManager *volumeManager,
    OH_AudioStream_Usage usage, int32_t *volumeLevel)
{
    if (volumeManager == nullptr || !VALID_OH_STREAM_USAGES.count(usage) || volumeLevel == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or usage or maxVolumeLevel");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    int32_t volume = ohAudioVolumeManager->GetVolumeByUsage(streamUsage);
    if (volume < 0) {
        AUDIO_ERR_LOG("GetVolumeByUsage failed");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    *volumeLevel = volume;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioVolumeManager_IsMuteByUsage(OH_AudioVolumeManager *volumeManager,
    OH_AudioStream_Usage usage, bool *muted)
{
    if (volumeManager == nullptr || !VALID_OH_STREAM_USAGES.count(usage) || muted == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or usage or muted");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    int32_t result = ohAudioVolumeManager->IsMuteByUsage(streamUsage, *muted);
    if (result < 0) {
        AUDIO_ERR_LOG("IsMuteByUsage failed");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback(
    OH_AudioVolumeManager *volumeManager, OH_AudioStream_Usage usage,
    OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback, void *userData)
{
    if (volumeManager == nullptr || !VALID_OH_STREAM_USAGES.count(usage) || callback == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or usage or callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    int32_t result = ohAudioVolumeManager->SetStreamVolumeChangeCallback(callback, streamUsage, userData);
    return static_cast<OH_AudioCommon_Result>(result);
}

OH_AudioCommon_Result OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback(
    OH_AudioVolumeManager *volumeManager,
    OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback)
{
    if (volumeManager == nullptr || callback == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    int32_t result = ohAudioVolumeManager->UnsetStreamVolumeChangeCallback(callback);
    return static_cast<OH_AudioCommon_Result>(result);
}

OH_AudioCommon_Result OH_AudioVolumeManager_GetRingerMode(OH_AudioVolumeManager *volumeManager,
    OH_AudioRingerMode *ringerMode)
{
    if (volumeManager == nullptr || ringerMode == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or ringerMode");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    int32_t result = ohAudioVolumeManager->GetRingerMode();
    if (result < 0 || VALID_OH_RINGER_MODES.count(static_cast<OH_AudioRingerMode>(result)) == 0) {
        AUDIO_ERR_LOG("IsMuteByUsage failed");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    *ringerMode = static_cast<OH_AudioRingerMode>(result);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioVolumeManager_RegisterRingerModeChangeCallback(
    OH_AudioVolumeManager *volumeManager,
    OH_AudioVolumeManager_OnRingerModeChangeCallback callback, void *userData)
{
    if (volumeManager == nullptr || callback == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    int32_t result = ohAudioVolumeManager->SetAudioRingerModeChangeCallback(callback, userData);
    return static_cast<OH_AudioCommon_Result>(result);
}

OH_AudioCommon_Result OH_AudioVolumeManager_UnregisterRingerModeChangeCallback(
    OH_AudioVolumeManager *volumeManager,
    OH_AudioVolumeManager_OnRingerModeChangeCallback callback)
{
    if (volumeManager == nullptr || callback == nullptr) {
        AUDIO_ERR_LOG("invalid volumeManager or callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioVolumeManager *ohAudioVolumeManager = (OHAudioVolumeManager*)volumeManager;
    int32_t result = ohAudioVolumeManager->UnsetAudioRingerModeChangeCallback(callback);
    return static_cast<OH_AudioCommon_Result>(result);
}

namespace OHOS {
namespace AudioStandard {
void OHStreamVolumeChangeCallback::OnStreamVolumeChange(StreamVolumeEvent streamVolumeEvent)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the fuction is nullptr");
    if (usage_ != streamVolumeEvent.streamUsage) {
        AUDIO_ERR_LOG("usage is not equal");
        return;
    }
    callback_(userData_, static_cast<OH_AudioStream_Usage>(usage_), streamVolumeEvent.volume,
        streamVolumeEvent.updateUi);
}

void OHAudioRingerModeCallback::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the fuction is nullptr");
    callback_(userData_, static_cast<OH_AudioRingerMode>(ringerMode));
}

OHAudioVolumeManager *OHAudioVolumeManager::GetInstance()
{
    static OHAudioVolumeManager instance;
    return &instance;
}

OHAudioVolumeManager::OHAudioVolumeManager()
{
    audioSystemManager_ = AudioSystemManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioSystemManager_ != nullptr, "failed, audioSystemManager_ is nullptr");
    audioGroupManager_ = audioSystemManager_->GetGroupManager(DEFAULT_VOLUME_GROUP_ID);
    CHECK_AND_RETURN_LOG(audioGroupManager_ != nullptr, "failed, audioGroupManager_ is nullptr");
}

int32_t OHAudioVolumeManager::GetMaxVolumeByUsage(StreamUsage streamUsage)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioSystemManager_ is nullptr");
    return audioSystemManager_->GetMaxVolumeByUsage(streamUsage);
}

int32_t OHAudioVolumeManager::GetMinVolumeByUsage(StreamUsage streamUsage)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioSystemManager_ is nullptr");
    return audioSystemManager_->GetMinVolumeByUsage(streamUsage);
}

int32_t OHAudioVolumeManager::GetVolumeByUsage(StreamUsage streamUsage)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioSystemManager_ is nullptr");
    return audioSystemManager_->GetVolumeByUsage(streamUsage);
}

int32_t OHAudioVolumeManager::IsMuteByUsage(StreamUsage streamUsage, bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioSystemManager_ is nullptr");
    return audioSystemManager_->IsStreamMuteByUsage(streamUsage, isMute);
}

int32_t OHAudioVolumeManager::GetRingerMode()
{
    CHECK_AND_RETURN_RET_LOG(audioGroupManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioGroupManager_ is nullptr");
    return (int32_t)audioGroupManager_->GetRingerMode();
}

int32_t OHAudioVolumeManager::SetStreamVolumeChangeCallback(
    OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback, StreamUsage streamUsage, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioSystemManager_ is nullptr");
    if (callback == nullptr) {
        AUDIO_ERR_LOG("invalid callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(streamVolumeCbMutex_);
    if (streamVolumeCallbacks_.count(callback)) {
        if (streamVolumeCallbacks_[callback].first == streamUsage) {
            AUDIO_INFO_LOG("callback already registered");
            return AUDIOCOMMON_RESULT_SUCCESS;
        } else {
            AUDIO_ERR_LOG("callback already registered for different streamUsage:%{public}d",
                streamVolumeCallbacks_[callback].first);
            return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
        }
    }

    streamUsage = (streamUsage == StreamUsage::STREAM_USAGE_UNKNOWN) ? StreamUsage::STREAM_USAGE_MUSIC : streamUsage;
    auto ohAudioVolumeCallback = std::make_shared<OHStreamVolumeChangeCallback>(callback, streamUsage, userData);
    CHECK_AND_RETURN_RET_LOG(ohAudioVolumeCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed to create callback!");

    int32_t result = audioSystemManager_->RegisterStreamVolumeChangeCallback(getpid(), { streamUsage },
        ohAudioVolumeCallback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        streamVolumeCallbacks_[callback] = {streamUsage, ohAudioVolumeCallback};
    }
    return result == AUDIOCOMMON_RESULT_SUCCESS ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

int32_t OHAudioVolumeManager::UnsetStreamVolumeChangeCallback(
    OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioSystemManager_ is nullptr");

    std::lock_guard<std::mutex> lock(streamVolumeCbMutex_);
    if (callback == nullptr || !streamVolumeCallbacks_.count(callback)) {
        AUDIO_ERR_LOG("invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    int32_t result = audioSystemManager_->UnregisterStreamVolumeChangeCallback(getpid(),
        streamVolumeCallbacks_[callback].second);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        streamVolumeCallbacks_.erase(callback);
    }
    return result == AUDIOCOMMON_RESULT_SUCCESS ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

int32_t OHAudioVolumeManager::SetAudioRingerModeChangeCallback(
    OH_AudioVolumeManager_OnRingerModeChangeCallback callback, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(audioGroupManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioGroupManager_ is nullptr");
    if (callback == nullptr) {
        AUDIO_ERR_LOG("invalid callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(ringerModeCbMutex_);
    if (ringerModeCallbacks_.count(callback)) {
        AUDIO_INFO_LOG("callback already registered");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }

    auto ohAudioRingerModeCallback = std::make_shared<OHAudioRingerModeCallback>(callback, userData);
    CHECK_AND_RETURN_RET_LOG(ohAudioRingerModeCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed to create callback!");

    int32_t result = audioGroupManager_->SetRingerModeCallback(getpid(), ohAudioRingerModeCallback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        ringerModeCallbacks_.emplace(callback, ohAudioRingerModeCallback);
    }
    return result == AUDIOCOMMON_RESULT_SUCCESS ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

int32_t OHAudioVolumeManager::UnsetAudioRingerModeChangeCallback(
    OH_AudioVolumeManager_OnRingerModeChangeCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioGroupManager_ != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "failed, audioGroupManager_ is nullptr");
    std::lock_guard<std::mutex> lock(ringerModeCbMutex_);
    if (callback == nullptr || !ringerModeCallbacks_.count(callback)) {
        AUDIO_ERR_LOG("invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    int32_t result = audioGroupManager_->UnsetRingerModeCallback(getpid(), ringerModeCallbacks_[callback]);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        ringerModeCallbacks_.erase(callback);
    }
    return result == AUDIOCOMMON_RESULT_SUCCESS ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}
} // namespace AudioStandard
} // namespace OHOS