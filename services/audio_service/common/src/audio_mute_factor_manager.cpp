/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_mute_factor_manager.h"
#include <sstream>
#include <iterator>
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {

AudioMuteFactorManager& AudioMuteFactorManager::GetInstance()
{
    static AudioMuteFactorManager instance_;
    return instance_;
}

AudioMuteFactorManager::AudioMuteFactorManager()
{
    AUDIO_INFO_LOG("Construct");
}

AudioMuteFactorManager::~AudioMuteFactorManager()
{
    AUDIO_INFO_LOG("Destruct");
}

bool AudioMuteFactorManager::IsMdmMuted() const
{
    return isMdmMute_;
}

float AudioMuteFactorManager::GetMdmMuteStatus() const
{
    return isMdmMute_ ? 0.0f : 1.0f;
}

void AudioMuteFactorManager::SetMdmMuteStatus(const bool mdmMute)
{
    isMdmMute_ = mdmMute;
}

void AudioMuteFactorManager::SetUnmuteWhitelist(const std::vector<int32_t> &uids)
{
    auto newWhitelist = std::make_shared<std::unordered_set<int32_t>>(
        uids.begin(), uids.end());
    std::atomic_store(&unmuteWhitelist_, newWhitelist);
    std::ostringstream oss;
    std::copy(uids.begin(), uids.end(), std::ostream_iterator<int32_t>(oss, " "));
    AUDIO_INFO_LOG("set unmuteUids:%{public}s", oss.str().c_str());
}

void AudioMuteFactorManager::ClearUnmuteWhitelist()
{
    auto emptyWhitelist = std::make_shared<std::unordered_set<int32_t>>();
    std::atomic_store(&unmuteWhitelist_, emptyWhitelist);
    AUDIO_INFO_LOG("ClearUnmuteWhitelist");
}

float AudioMuteFactorManager::GetAacMuteFactor(int32_t appUid) const
{
    auto whitelist = std::atomic_load(&unmuteWhitelist_);
    if (whitelist == nullptr || whitelist->empty()) {
        return 1.0f;
    }
    return whitelist->count(appUid) > 0 ? 1.0f : 0.0f;
}
} // namespace AudioStandard
} // namespace OHOS
