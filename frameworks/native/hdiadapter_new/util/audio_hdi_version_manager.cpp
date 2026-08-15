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

#include "util/audio_hdi_version_manager.h"

namespace OHOS {
namespace AudioStandard {

namespace {
class AudioHdiVersionManager {
public:
    static AudioHdiVersionManager& GetInstance()
    {
        static AudioHdiVersionManager instance;
        return instance;
    }

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
} // namespace

bool IsAudioHdiV61OrAbove()
{
    return AudioHdiVersionManager::GetInstance().IsV61OrAbove();
}

void SetAudioHdiVersion(uint32_t majorVersion, uint32_t minorVersion)
{
    AudioHdiVersionManager::GetInstance().SetVersion(majorVersion, minorVersion);
}

}
}
