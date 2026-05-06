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
#define LOG_TAG "AudioPlayingAppManager"
#endif

#include "util/audio_playing_app_manager.h"

#include <algorithm>
#include <utility>

#include "audio_bundle_manager.h"
#include "audio_hdi_log.h"

namespace OHOS {
namespace AudioStandard {
AudioPlayingAppManager::AudioPlayingAppManager(std::function<bool(const std::string &)> setter)
    : setter_(std::move(setter))
{
}

void AudioPlayingAppManager::Start()
{
    Trace trace("AudioPlayingAppManager::Start");
    std::lock_guard<std::mutex> lock(mutex_);
    lastAppsUid_ = {};
    isStarted_ = true;
}

void AudioPlayingAppManager::Stop()
{
    Trace trace("AudioPlayingAppManager::Stop");
    std::lock_guard<std::mutex> lock(mutex_);
    isStarted_ = false;
    lastAppsUid_ = {};
    if (setter_) {
        setter_(BuildEmptyPlayingAppNameParam());
    } else {
        AUDIO_ERR_LOG("setter is null, skip updating playing app name");
    }
}

void AudioPlayingAppManager::UpdatePlayingAppNameToHdi()
{
    Trace trace("AudioPlayingAppManager::UpdatePlayingAppNameToHdi");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((!isStarted_) || currentAppsUid_ == lastAppsUid_) {
        return;
    }
    if (!setter_) {
        AUDIO_ERR_LOG("setter is null, skip updating playing app name");
        return;
    }
    if (setter_(BuildPlayingAppNameParam(currentAppsUid_))) {
        lastAppsUid_ = currentAppsUid_;
    }
}

std::string AudioPlayingAppManager::BuildPlayingAppNameParam(const std::vector<int32_t> &appsUid) const
{
    std::string param = std::string(PLAYING_APP_NAME_KEY) + "=[";
    bool first = true;
    for (int32_t appUid : appsUid) {
        std::string bundleName = AudioBundleManager::GetBundleNameFromUidCached(appUid);
        if (bundleName.empty()) {
            continue;
        }
        if (!first) {
            param += ",";
        }
        param += bundleName;
        first = false;
    }
    param += "]";
    return param;
}

std::string AudioPlayingAppManager::BuildEmptyPlayingAppNameParam() const
{
    return std::string(PLAYING_APP_NAME_KEY) + "=[]";
}

std::vector<int32_t> AudioPlayingAppManager::NormalizeAppsUid(const std::vector<int32_t> &appsUid) const
{
    std::vector<int32_t> result;
    for (int32_t appUid : appsUid) {
        if (std::find(result.begin(), result.end(), appUid) != result.end()) {
            continue;
        }
        result.push_back(appUid);
    }
    return result;
}
} // namespace AudioStandard
} // namespace OHOS
