/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioInterruptUtils"
#endif

#include "audio_bundle_manager.h"
#include "audio_interrupt_utils.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "app_mgr_client.h"
#include "dfx_msg_manager.h"
#include "audio_adapter_manager.h"
#include "audio_anco_playback_info.h"
#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {

std::string AudioInterruptUtils::GetAudioInterruptBundleName(const AudioInterrupt &audioInterrupt)
{
    if (audioInterrupt.bundleName.empty()) {
        auto info = AudioBundleManager::GetBundleInfoFromUid(audioInterrupt.uid);
        audioInterrupt.bundleName = info.name;
        AUDIO_INFO_LOG("Get audio interrupt bundle name: %{public}s", audioInterrupt.bundleName.c_str());
    }
    return audioInterrupt.bundleName;
}

uint8_t AudioInterruptUtils::GetAppState(int32_t appPid)
{
    OHOS::AppExecFwk::AppMgrClient appManager;
    OHOS::AppExecFwk::RunningProcessInfo infos;
    uint8_t state = 0;
    appManager.GetRunningProcessInfoByPid(appPid, infos);
    state = static_cast<uint8_t>(infos.state_);
    if (state == 0) {
        AUDIO_WARNING_LOG("GetAppState failed, appPid=%{public}d", appPid);
    }
    return state;
}

bool AudioInterruptUtils::IsMediaStream(AudioStreamType audioStreamType)
{
    if (audioStreamType == STREAM_MUSIC || audioStreamType == STREAM_MOVIE || audioStreamType == STREAM_SPEECH) {
        return true;
    }
    return false;
}

bool AudioInterruptUtils::IsInterPhoneInterrupt(const AudioInterrupt &audioInterrupt)
{
    return (audioInterrupt.audioFocusType.streamType == STREAM_INTERPHONE ||
        audioInterrupt.audioFocusType.sourceType == SOURCE_TYPE_INTERPHONE);
}

bool AudioInterruptUtils::GetWindowState(const AudioInterrupt &audioInterrupt)
{
#ifdef HAS_WINDOW_MANAGER
    if (audioInterrupt.uid == AUDIO_ID) {
        std::vector<AncoPlaybackInfo> ancoPlaybackInfos;
        AudioPolicyService::GetAudioPolicyService().GetAncoPlaybackInfo(ancoPlaybackInfos);
        for (const auto &info : ancoPlaybackInfos) {
            if (!info.GetPackageName().empty() && info.GetPlaybackState() == ANCO_PLAYBACK_STATE_PLAYING &&
                WindowUtils::CheckAncoWindowState(info.GetPackageName())) {
                    AUDIO_INFO_LOG("anco foreground: %{public}s", info.GetPackageName().c_str());
                    return true;
            }
        }
        AUDIO_INFO_LOG("no anco in foreground");
        return false;
    } else {
        return WindowUtils::CheckWindowState(audioInterrupt.pid);
    }
#else
    return false;
#endif
}
} // namespace AudioStandard
} // namespace OHOS