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

#ifndef AUDIO_PLAYING_APP_MANAGER_H
#define AUDIO_PLAYING_APP_MANAGER_H

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
class AudioPlayingAppManager {
public:
    explicit AudioPlayingAppManager(std::function<bool(const std::string &)> setter);

    template<typename T>
    void UpdateAppsUid(const T &itBegin, const T &itEnd)
    {
        Trace trace("AudioPlayingAppManager::UpdateAppsUid");
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<int32_t> appsUid(itBegin, itEnd);
        if (currentAppsUid_ == appsUid) {
            return;
        }
        currentAppsUid_ = NormalizeAppsUid(appsUid);
    }

    void Start();
    void Stop();
    void UpdatePlayingAppNameToHdi();

private:
    static constexpr const char *PLAYING_APP_NAME_KEY = "PLAYING_APP_NAME";

    std::string BuildPlayingAppNameParam(const std::vector<int32_t> &appsUid) const;
    std::string BuildEmptyPlayingAppNameParam() const;
    std::vector<int32_t> NormalizeAppsUid(const std::vector<int32_t> &appsUid) const;

    std::mutex mutex_;
    std::function<bool(const std::string &)> setter_;
    bool isStarted_ = false;
    std::vector<int32_t> currentAppsUid_;
    std::vector<int32_t> lastAppsUid_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_PLAYING_APP_MANAGER_H
