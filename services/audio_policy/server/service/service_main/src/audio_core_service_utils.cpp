/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioCoreServiceUtils"
#endif

#include "audio_core_service_utils.h"
#include "audio_policy_manager_factory.h"
#include "audio_scene_manager.h"

namespace OHOS {
namespace AudioStandard {

bool AudioCoreServiceUtils::IsDualStreamWhenRingDual(AudioStreamType streamType)
{
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType == STREAM_RING || volumeType == STREAM_ALARM || volumeType == STREAM_ACCESSIBILITY) {
        return true;
    }
    return false;
}

bool AudioCoreServiceUtils::IsOverRunPlayback(AudioMode &mode, RendererState rendererState)
{
    if (mode != AUDIO_MODE_PLAYBACK) {
        return false;
    }
    if (rendererState == RENDERER_STOPPED || rendererState == RENDERER_RELEASED) {
        return true;
    }
    if (rendererState == RENDERER_PAUSED &&
        AudioSceneManager::GetInstance().GetAudioScene(true) != AUDIO_SCENE_RINGING) {
        return true;
    }
    return false;
}

bool AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    const int32_t sessionId)
{
    if (descs.size() !=  AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT) {
        return false;
    }
    if (AudioPolicyUtils::GetInstance().GetSinkName(*descs.front(), sessionId) != PRIMARY_SPEAKER) {
        return false;
    }
    if (AudioPolicyUtils::GetInstance().GetSinkName(*descs.back(), sessionId) != PRIMARY_SPEAKER) {
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(descs.back() != nullptr, false, "back is nullptr");
    if (descs.back()->deviceType_ != DEVICE_TYPE_SPEAKER) {
        return false;
    }
    AUDIO_INFO_LOG("ring dual tone on primary speaker and mute music.");
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetInnerStreamMute(STREAM_MUSIC, true, STREAM_USAGE_MUSIC);
    return true;
}


bool AudioCoreServiceUtils::NeedDualHalToneInStatus(AudioRingerMode mode, StreamUsage usage,
    bool isPcVolumeEnable, bool isMusicMute)
{
    if (mode != RINGER_MODE_NORMAL && usage != STREAM_USAGE_ALARM) {
        return false;
    }
    if (isPcVolumeEnable && isMusicMute) {
        return false;
    }
    return true;
}

bool AudioCoreServiceUtils::IsAlarmOnActive(StreamUsage usage, bool isAlarmActive)
{
    if (usage != STREAM_USAGE_ALARM) {
        return false;
    }
    return isAlarmActive;
}
} // namespace AudioStandard
} // namespace OHOS
