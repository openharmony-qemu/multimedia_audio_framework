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

#ifndef AUDIO_SUITE_SPACE_RENDER_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_SPACE_RENDER_ALGO_INTERFACE_IMPL_H

#include "audio_suite_algo_interface.h"
#include "audio_hms_space_render_api.h"
#include "audio_suite_log.h"
#include <utility>
#include <dlfcn.h>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using FunSpaceRenderGetSpeces = SpaceRenderSpeces (*)(void);
using FunSpaceRenderGetSize = int (*)(const SpaceRenderParam *params);
using FunSpaceRenderGetLateSamples = int (*)(const char* params);
using FunSpaceRenderInit = int (*)(char *phandle, const SpaceRenderParam *params);
using FunSpaceRenderApply = int (*)(char* phandle, const short *pcmIn, const int inSampleCnt, short* pcmOut);
using FunSpaceRenderRelease = int (*)(char *phandle);

struct SpaceRenderAlgoApi {
    FunSpaceRenderGetSpeces getSpeces{nullptr};
    FunSpaceRenderGetSize getSize{nullptr};
    FunSpaceRenderGetLateSamples getLateSamples{nullptr};
    FunSpaceRenderInit initAlgo{nullptr};
    FunSpaceRenderApply applyAlgo{nullptr};
    FunSpaceRenderRelease releaseAlgo{nullptr};
};

class AudioSuiteSpaceRenderAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteSpaceRenderAlgoInterfaceImpl(NodeCapability &nc);
    ~AudioSuiteSpaceRenderAlgoInterfaceImpl();
 
    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override;
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    bool isSpaceRenderAlgoInit_ = false;
    SpaceRenderAlgoApi algoApi_{0};
    void *libHandle_{nullptr};
    SpaceRenderParam spaceRenderParam_ = {SPACE_RENDER_MODE_STATIC,
        {0.0f, 0.0f, 0.0f}, 0.0f, SPACE_RENDER_ROTATION_MODE_CW, 0.0f, 0};
    SpaceRenderSpeces applySpeces_ = {0};
    std::vector<char> spaceRenderHandle_;

    int SetPositionParameter(const std::string &paramValue);
    int SetExtensionParameter(const std::string &paramValue);
    int SetRotationParameter(const std::string &paramValue);
    int GetPositionParameter(std::string &paramValue);
    int GetExtensionParameter(std::string &paramValue);
    int GetRotationParameter(std::string &paramValue);
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_SPACE_RENDER_ALGO_INTERFACE_IMPL_H