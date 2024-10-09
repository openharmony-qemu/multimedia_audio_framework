/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_EFFECT_SERVER_H
#define ST_AUDIO_EFFECT_SERVER_H

#include <cassert>
#include <cstdint>
#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

class AudioEffectServer {
public:
    explicit AudioEffectServer();
    ~AudioEffectServer();

    bool LoadAudioEffects(const std::vector<Library> &libraries, const std::vector<Effect> &effects,
                          std::vector<Effect> &successEffectList);

    std::vector<std::shared_ptr<AudioEffectLibEntry>> &GetEffectEntries();

private:
    std::vector<std::shared_ptr<AudioEffectLibEntry>> effectLibEntries_;
};

} // namespce AudioStandard
} // namespace OHOS
#endif
