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

namespace OHOS {
namespace AudioStandard {

std::string AudioInterruptUtils::GetAudioInterruptBundleName(AudioInterrupt &audioInterrupt)
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

} // namespace AudioStandard
} // namespace OHOS