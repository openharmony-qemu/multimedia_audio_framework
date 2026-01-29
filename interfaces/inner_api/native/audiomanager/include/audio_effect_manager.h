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

#ifndef ST_AUDIO_EFFECT_MANAGER_H
#define ST_AUDIO_EFFECT_MANAGER_H

#include <iostream>
#include <map>
#include "audio_effect.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioEffectManager {
public:
    AudioEffectManager() = default;
    virtual ~AudioEffectManager() = default;

    static AudioEffectManager *GetInstance();

    /**
     * @brief Get Audio render Effect param.
     *
     * @param AudioSceneEffectInfo  AudioSceneEffectInfo
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);

    /**
     * @brief Sets the audio effect Param.
     *
     * * @param effectParam The audio effect Param at which the effect needs to be rendered.
     * @return  Returns {@link SUCCESS} if audio effect Param is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray);

    /**
     * @brief Gets the audio effect Param.
     *
     * * @param effectParam The audio effect moParamde at which the effect needs to be rendered.
     * @return  Returns {@link SUCCESS} if audio effect Param is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_EFFECT_MANAGER_H
