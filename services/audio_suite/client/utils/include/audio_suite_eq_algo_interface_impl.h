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

#ifndef AUDIO_SUITE_EQ_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_EQ_ALGO_INTERFACE_IMPL_H

#include "audio_suite_algo_interface.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteEqAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    AudioSuiteEqAlgoInterfaceImpl() = default;
    ~AudioSuiteEqAlgoInterfaceImpl() = default;

    int32_t Init() override
    {
        return SUCCESS;
    }

    int32_t Deinit() override
    {
        return SUCCESS;
    }

    int32_t SetParameter(const std::string& paramType, const std::string& paramValue) override
    {
        return SUCCESS;
    }

    int32_t GetParameter(const std::string& paramType, std::string& paramValue) override
    {
        return SUCCESS;
    }

    int32_t Apply(std::vector<uint8_t*>& v1, std::vector<uint8_t*>& v2) override
    {
        return SUCCESS;
    }
};

}
}
}

#endif