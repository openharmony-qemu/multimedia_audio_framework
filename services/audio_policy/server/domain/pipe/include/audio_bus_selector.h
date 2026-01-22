/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_BUS_SELECTOR_H
#define AUDIO_BUS_SELECTOR_H

#include <vector>
#include <map>
#include <mutex>
#include "audio_stream_info.h"
#include "audio_zone_info.h"
#include "audio_zone_service.h"
#include "audio_pipe_manager.h"
#include "audio_policy_config_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioBusSelector {
public:
    AudioBusSelector(const AudioBusSelector &) = delete;
    AudioBusSelector &operator=(const AudioBusSelector &) = delete;

    static AudioBusSelector &GetBusSelector() noexcept;
    int32_t SetCustomAudioMix(const std::string &zoneName, const std::vector<AudioZoneMix> &audioMixes);
    std::vector<std::string> GetBusAddressesByStreamDesc(const std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    std::string GetSinkNameByStreamId(int32_t streamId);

private:
    AudioBusSelector(AudioZoneService &audioZoneService, const std::shared_ptr<AudioPipeManager> &audioPipeManager,
                    AudioPolicyConfigManager &audioConfigManager)
        : audioZoneService_(audioZoneService),
          audioPipeManager_(audioPipeManager),
          audioConfigManager_(audioConfigManager)
    {
    }
    ~AudioBusSelector() = default;

    std::string GetDefaultBusByConfig();

    AudioZoneService &audioZoneService_;
    const std::shared_ptr<AudioPipeManager> audioPipeManager_{nullptr};
    AudioPolicyConfigManager &audioConfigManager_;
    std::map<std::string, std::vector<AudioZoneMix>> audioMixMap_;
    std::mutex audioMixMutex_;
};

}  // namespace AudioStandard
}  // namespace OHOS

#endif  // AUDIO_BUS_SELECTOR_H