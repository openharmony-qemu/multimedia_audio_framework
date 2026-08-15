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

#ifndef AUDIO_HDI_VERSION_MANAGER_H
#define AUDIO_HDI_VERSION_MANAGER_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {

// Get and save HDI version, add for flex version. Use for new HDI define compatible.
// GetInstance() is defined in util/audio_hdi_version_manager.cpp so that the singleton
// exists only in libhdiadapter_new.z.so; an inline definition would emit a weak copy
// into every DSO including this header and the copies are not merged under
// -Bsymbolic/CFI cross-DSO, leaving each DSO with its own version state.
class AudioHdiVersionManager {
public:
    static AudioHdiVersionManager& GetInstance();

    void SetVersion(uint32_t major, uint32_t minor)
    {
        majorVersion_ = major;
        minorVersion_ = minor;
    }

    bool IsV61OrAbove() const { return (majorVersion_ == 6 && minorVersion_ >= 1) || majorVersion_ > 6; }

private:
    AudioHdiVersionManager() = default;
    ~AudioHdiVersionManager() = default;
    AudioHdiVersionManager(const AudioHdiVersionManager&) = delete;
    AudioHdiVersionManager &operator=(const AudioHdiVersionManager&) = delete;

    uint32_t majorVersion_ = 0;
    uint32_t minorVersion_ = 0;
};

}
}

#endif
