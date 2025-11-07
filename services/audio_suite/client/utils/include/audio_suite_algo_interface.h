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
#ifndef AUDIO_SUITE_ALGO_INTERFACE_H
#define AUDIO_SUITE_ALGO_INTERFACE_H

#include <memory>
#include <string>
#include "audio_suite_capabilities.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

enum class AlgoType {
    AUDIO_NODE_TYPE_EQUALIZER = 1,

    AUDIO_NODE_TYPE_NOISE_REDUCTION = 2,

    AUDIO_NODE_TYPE_SOUND_FIELD = 3,

    AUDIO_NODE_TYPE_AUDIO_SEPARATION = 4,

    AUDIO_NODE_TYPE_TEMPO_PITCH = 5,

    AUDIO_NODE_TYPE_SPACE_RENDER = 6,

    AUDIO_NODE_TYPE_VOICE_BEAUTIFIER = 7,

    AUDIO_NODE_TYPE_ENVIRONMENT_EFFECT = 8,
};

class AudioSuiteAlgoInterface {
public:
    virtual ~AudioSuiteAlgoInterface() = default;
    virtual int32_t Init() = 0;
    virtual int32_t Deinit() = 0;
    virtual int32_t SetParameter(const std::string& paramType, const std::string& paramValue) = 0;
    virtual int32_t GetParameter(const std::string& paramType, std::string& paramValue) = 0;
    virtual int32_t Apply(std::vector<uint8_t*>& v1, std::vector<uint8_t*>& v2) = 0;

    static std::shared_ptr<AudioSuiteAlgoInterface> CreateAlgoInterface(AlgoType algoType, NodeCapability &nc);

protected:
    NodeCapability nodeCapability;
};

}
}
}
#endif