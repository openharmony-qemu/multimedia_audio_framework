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
#ifndef ST_AUDIO_TONE_MANAGER_H
#define ST_AUDIO_TONE_MANAGER_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_ec_info.h"
#include "datashare_helper.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

class AudioToneManager {
public:
    static AudioToneManager& GetInstance()
    {
        static AudioToneManager instance;
        return instance;
    }
#ifdef FEATURE_DTMF_TONE
    bool LoadToneDtmfConfig();
    std::vector<int32_t> GetSupportedTones(const std::string &countryCode);
    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype, const std::string &countryCode);
#endif
private:
    AudioToneManager() {}
    ~AudioToneManager() {}
private:
#ifdef FEATURE_DTMF_TONE
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> toneDescriptorMap_;
    std::unordered_map<std::string, std::unordered_map<int32_t, std::shared_ptr<ToneInfo>>> customToneDescriptorMap_;
#endif
};

}
}

#endif