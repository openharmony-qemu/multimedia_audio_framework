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
#ifndef AUDIO_EFFECT_CONFIG_PARSER_H
#define AUDIO_EFFECT_CONFIG_PARSER_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include "audio_policy_log.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
typedef enum {
    INDEX_PRE_DEFAULT_SCENE = 0,
    INDEX_PRE_PRIOR_SCENE = 1,
    INDEX_PRE_NORMAL_SCENE = 2,
    INDEX_PRE_EXCEPTION = 3,
    NODE_SIZE_PRE = 4,
} XmlPreNodeIndex;
 
typedef enum {
    INDEX_POST_DEFAULT_SCENE = 0,
    INDEX_POST_PRIOR_SCENE = 1,
    INDEX_POST_NORMAL_SCENE = 2,
    INDEX_POST_MAPPING = 3,
    INDEX_POST_EXCEPTION = 4,
    NODE_SIZE_POST = 5,
} XmlPostNodeIndex;

class AudioEffectConfigParser {
public:
    explicit AudioEffectConfigParser();
    ~AudioEffectConfigParser();
    int32_t LoadEffectConfig(OriginalEffectConfig &result);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_EFFECT_CONFIG_PARSER_H