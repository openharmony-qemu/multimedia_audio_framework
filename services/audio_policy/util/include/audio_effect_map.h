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

#ifndef AUDIO_EFFECT_MAP_H
#define AUDIO_EFFECT_MAP_H

#include <map>

#include "audio_effect.h"


namespace OHOS {
namespace AudioStandard {

const std::unordered_map<AudioEffectScene, std::string>& GetSupportedSceneType();

const std::unordered_map<AudioEnhanceScene, std::string>& GetEnhanceSupportedSceneType();

const std::unordered_map<AudioEffectMode, std::string>& GetAudioSupportedSceneModes();

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_EFFECT_MAP_H