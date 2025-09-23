
/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef TAIHE_AUDIO_EFFECT_MANAGER_H
#define TAIHE_AUDIO_EFFECT_MANAGER_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_system_manager.h"
#include "audio_effect_manager.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string AUDIO_EFFECT_MGR_TAIHE_CLASS_NAME = "AudioEffectManager";

class AudioEffectManagerImpl {
public:
    AudioEffectManagerImpl();
    explicit AudioEffectManagerImpl(OHOS::AudioStandard::AudioEffectManager *audioEffectMngr);
    ~AudioEffectManagerImpl();

    static AudioEffectManager CreateEffectManagerWrapper();

    array<AudioEffectProperty> GetSupportedAudioEffectProperty();
    void SetAudioEffectProperty(array_view<AudioEffectProperty> propertyArray);
    array<AudioEffectProperty> GetAudioEffectProperty();

private:
    OHOS::AudioStandard::AudioEffectManager *audioEffectMngr_;
    int32_t cachedClientId_ = -1;
};
}  // namespace ANI::Audio
#endif // TAIHE_AUDIO_EFFECT_MANAGER_H
