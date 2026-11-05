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
#define LOG_TAG "AudioZoneFocusFilter"

#include "audio_zone_utils.h"
#include "audio_log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {
void AudioZoneFocusFilter::UpdateLocalSessionIds(const std::vector<uint32_t> &sessionIds)
{
    std::lock_guard<std::mutex> lock(localSessionIdsMutex_);
    localSessionIds_ = sessionIds;
}

std::vector<uint32_t> AudioZoneFocusFilter::GetLocalSessionIds()
{
    std::lock_guard<std::mutex> lock(localSessionIdsMutex_);
    return localSessionIds_;
}

void AudioZoneFocusFilter::FilterAudioInterrupt(AudioFocusFilterType filterType,
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusList,
    const std::shared_ptr<AudioInterruptService> &interruptService)
{
    CHECK_AND_RETURN(interruptService != nullptr);
    auto localSessionIds = GetLocalSessionIds();
    for (auto focus = focusList.begin(); focus != focusList.end();) {
        uint32_t realSessionId = focus->first.streamId;
        realSessionId = interruptService->GetRealSessionIdByAudioInterrupt(focus->first);
        bool isLocal = realSessionId != INVALID_STREAM_ID ? std::find(localSessionIds.begin(), localSessionIds.end(),
            realSessionId) != localSessionIds.end() : true;
        bool shouldErase = (filterType == AudioFocusFilterType::FILTER_REMOTE_FOCUS) ? isLocal : !isLocal;
        if (shouldErase) {
            focus = focusList.erase(focus);
        } else {
            focus++;
        }
    }
}

} // namespace AudioStandard
} // namespace OHOS