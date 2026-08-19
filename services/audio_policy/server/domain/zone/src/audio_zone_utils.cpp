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
#undef LOG_TAG
#define LOG_TAG "AudioZoneUtils"

#include "audio_zone_utils.h"
#include "audio_pipe_manager.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {
void AudioZoneUtils::FilterRemoteAudioInterrupt(const std::shared_ptr<AudioInterruptService> &interruptService,
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusList)
{
    CHECK_AND_RETURN_LOG(interruptService != nullptr, "interruptService is nullptr");
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_LOG(pipeManager != nullptr, "pipeManager is nullptr");
    for (auto focus = focusList.begin(); focus != focusList.end();) {
        uint32_t realSessionId = interruptService->GetRealSessionIdByAudioInterrupt(focus->first);
        auto streamDesc = pipeManager->GetStreamDescById(realSessionId);
        if (streamDesc != nullptr && streamDesc->IsDeviceRemote()) {
            AUDIO_INFO_LOG("[Remotefocus]sessionId: %{public}d is remote", focus->first.streamId);
            focus++;
        } else {
            AUDIO_INFO_LOG("[Remotefocus]sessionId: %{public}d is not remote", focus->first.streamId);
            focus = focusList.erase(focus);
        }
    }
}

} // namespace AudioStandard
} // namespace OHOS