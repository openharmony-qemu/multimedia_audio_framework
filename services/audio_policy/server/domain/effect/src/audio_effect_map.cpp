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

#ifndef LOG_TAG
#define LOG_TAG "AudioManagerUtil"
#endif

#include "audio_effect_map.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
const std::unordered_map<AudioEffectScene, std::string>& GetSupportedSceneType()
{
    static const std::unordered_map<AudioEffectScene, std::string> audioSupportedSceneTypes = {
        {SCENE_OTHERS, "SCENE_OTHERS"},
        {SCENE_MUSIC, "SCENE_MUSIC"},
        {SCENE_MOVIE, "SCENE_MOVIE"},
        {SCENE_GAME, "SCENE_GAME"},
        {SCENE_SPEECH, "SCENE_SPEECH"},
        {SCENE_RING, "SCENE_RING"},
        {SCENE_VOIP_DOWN, "SCENE_VOIP_DOWN"},
        {SCENE_COLLABORATIVE, "SCENE_COLLABORATIVE"},
    };
    return audioSupportedSceneTypes;
}

const std::unordered_map<AudioEnhanceScene, std::string>& GetEnhanceSupportedSceneType()
{
    static const std::unordered_map<AudioEnhanceScene, std::string> audioEnhanceSupportedSceneTypes = {
        {SCENE_VOIP_UP, "SCENE_VOIP_UP"},
        {SCENE_RECORD, "SCENE_RECORD"},
        {SCENE_ASR, "SCENE_ASR"},
        {SCENE_PRE_ENHANCE, "SCENE_PRE_ENHANCE"},
        {SCENE_VOICE_MESSAGE, "SCENE_VOICE_MESSAGE"},
        {SCENE_RECOGNITION, "SCENE_RECOGNITION"},
    };
    return audioEnhanceSupportedSceneTypes;
}

const std::unordered_map<AudioEffectMode, std::string>& GetAudioSupportedSceneModes()
{
    static const std::unordered_map<AudioEffectMode, std::string> audioSupportedSceneModes = {
        {EFFECT_NONE, "EFFECT_NONE"},
        {EFFECT_DEFAULT, "EFFECT_DEFAULT"},
    };
    return audioSupportedSceneModes;
}

} // namespace AudioStandard
} // namespace OHOS