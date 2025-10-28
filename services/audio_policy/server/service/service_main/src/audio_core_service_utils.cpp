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
#include "audio_volume.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t FIRST_STREAM_PRIORITY = 0;
static const int32_t SECOND_STREAM_PRIORITY = 1;
static const int32_t THIRD_STREAM_PRIORITY = 2;
}

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

bool AudioCoreServiceUtils::IsDualOnActive()
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");
    return pipeManager->IsStreamUsageActive(STREAM_USAGE_ALARM) ||
        pipeManager->IsStreamUsageActive(STREAM_USAGE_VOICE_RINGTONE) ||
        pipeManager->IsStreamUsageActive(STREAM_USAGE_RINGTONE);
}

void AudioCoreServiceUtils::SortOutputStreamDescsForUsage(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    std::unordered_map<uint32_t, uint32_t> streamDescsPriority;
    for (auto &streamDesc : streamDescs) {
        if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
            streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
            streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_CALL_ASSISTANT) {
            streamDescsPriority[streamDesc->sessionId_] = FIRST_STREAM_PRIORITY;
        } else if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_NOTIFICATION_RINGTONE ||
                streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
            streamDescsPriority[streamDesc->sessionId_] = SECOND_STREAM_PRIORITY;
        } else {
            streamDescsPriority[streamDesc->sessionId_] = THIRD_STREAM_PRIORITY;
        }
    }
    std::sort(streamDescs.begin(), streamDescs.end(),
        [&streamDescsPriority](const std::shared_ptr<AudioStreamDescriptor> &streamDescOne,
        const std::shared_ptr<AudioStreamDescriptor> &streamDescTwo) {
            return streamDescsPriority[streamDescOne->sessionId_] < streamDescsPriority[streamDescTwo->sessionId_];
        });
}
} // namespace AudioStandard
} // namespace OHOS
