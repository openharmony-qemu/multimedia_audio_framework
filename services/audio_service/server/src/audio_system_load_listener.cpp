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
#define LOG_TAG "AudioSystemLoadListener"
#endif

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_info.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "async_action_handler.h"
#include "audio_asr.h"
#include "audio_service.h"

#include "res_sched_client.h"
#include "audio_effect_chain_manager.h"
#include "audio_system_load_listener.h"
#include "parameter.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr int32_t ONE_MINUTE = 60000;
    constexpr int32_t THREE_MINUTES = 3 * ONE_MINUTE;
    constexpr int32_t SYSTEM_LOAD_LEVEL_EMERGENCY = 6;
    constexpr int32_t SYSTEM_LOAD_LEVEL_ESCAPE = 7;
    constexpr int32_t SYSTEM_LOAD_LEVEL_OFFSET1 = 1;
    constexpr int32_t SYSTEM_LOAD_LEVEL_OFFSET2 = 2;
    constexpr const char *CONTROL_SPATIAL_AUDIO = "ControlSpatialAudio";
    sptr<AudioSystemloadListener> systemLoadListener_ = new (std::nothrow) AudioSystemloadListener();
    auto listenerHandler = std::make_shared<AudioSystemloadListenerHandler>();
    static int32_t controlSpatialLevel = GetIntParameter(
        "const.multimedia.audio.control_spatial_sysload_level", SYSTEM_LOAD_LEVEL_EMERGENCY);
}

AudioSystemloadListenerHandler::AudioSystemloadListenerHandler()
    : AppExecFwk::EventHandler(
        AppExecFwk::EventRunner::Create("AudioSysLoadRunner", AppExecFwk::ThreadMode::FFRT))
{
    AUDIO_DEBUG_LOG("AudioSystemloadListenerHandler");
}

AudioSystemloadListenerHandler::~AudioSystemloadListenerHandler()
{
    AUDIO_DEBUG_LOG("~AudioSystemloadListenerHandler");
}

void AudioSystemloadListener::RegisterResSchedSys()
{
    CHECK_AND_RETURN_LOG(systemLoadListener_ != nullptr, "systemLoadListener_ is nullptr");
    ResourceSchedule::ResSchedClient::GetInstance().RegisterSystemloadNotifier(systemLoadListener_);
    AUDIO_INFO_LOG("RegisterResSchedSys");
}

void AudioSystemloadListener::UnregisterResSchedSys()
{
    CHECK_AND_RETURN_LOG(systemLoadListener_ != nullptr && IsAudioStreamEmpty(),
        "systemLoadListener_ is nullptr or AudioStream is not Empty");
    ResourceSchedule::ResSchedClient::GetInstance().UnRegisterSystemloadNotifier(systemLoadListener_);
    AUDIO_INFO_LOG("UnregisterResSchedSys");
}

bool AudioSystemloadListener::IsAudioStreamEmpty()
{
    int32_t streamCnt = AudioService::GetInstance()->GetCurrentRendererStreamCnt();
    return streamCnt > 0 ? false : true;
}

void AudioSystemloadListener::PostControlSpatialAudioTask(int32_t delayMs, const std::string &disableSpatialAudio)
{
    auto task = [delayMs, disableSpatialAudio]() {
        AUDIO_INFO_LOG("PostControlSpatialAudioTask delayMs: %{public}d, disableSpatialAudio: %{public}s",
            delayMs, disableSpatialAudio.c_str());
        AudioEffectChainManager::GetInstance()->UpdateParamExtra("audio_effect", SYSTEM_LOAD_SUBKEY,
            disableSpatialAudio);
    };
    listenerHandler->PostTask(task, CONTROL_SPATIAL_AUDIO, delayMs);
}

void AudioSystemloadListener::OnSystemloadLevel(int32_t level)
{
    const bool isEmpty = IsAudioStreamEmpty();
    std::string disableSpatialAudio = "0";

    listenerHandler->RemoveTask(CONTROL_SPATIAL_AUDIO);
    AUDIO_INFO_LOG("OnSystemloadLevel level: %{public}d, IsAudioStreamEmpty: %{public}d", level, isEmpty);
    CHECK_AND_RETURN_LOG(!isEmpty, "audio stream empty, ignore");
    if (level >= controlSpatialLevel) {
        disableSpatialAudio = "1";
        int32_t delayMs = (level >= SYSTEM_LOAD_LEVEL_ESCAPE) ? ONE_MINUTE : THREE_MINUTES;
        PostControlSpatialAudioTask(delayMs, disableSpatialAudio);
    } else if (level <= controlSpatialLevel - SYSTEM_LOAD_LEVEL_OFFSET2) {
        disableSpatialAudio = "0";
        AUDIO_INFO_LOG("Open SpatialAudio Immediately, disableSpatialAudio: %{public}s",
            disableSpatialAudio.c_str());
        AudioEffectChainManager::GetInstance()->UpdateParamExtra("audio_effect", SYSTEM_LOAD_SUBKEY,
            disableSpatialAudio);
    } else if (level == controlSpatialLevel - SYSTEM_LOAD_LEVEL_OFFSET1) {
        disableSpatialAudio = "0";
        PostControlSpatialAudioTask(THREE_MINUTES, disableSpatialAudio);
    }
}

} // namespace AudioStandard
} // namespace OHOS