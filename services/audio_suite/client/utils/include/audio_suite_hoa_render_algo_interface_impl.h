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

#ifndef AUDIO_SUITE_HOA_RENDER_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_HOA_RENDER_ALGO_INTERFACE_IMPL_H

#include "audio_suite_algo_interface.h"
#include "audio_hvs_api.h"
#include <vector>
#include <dlfcn.h>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using FunAudioHvsGetSize = int (*)(AudioHvsMemSize *);
using FunAudioHvsInit = int (*)(char *, char *);
using FunAudioHvsSetPara = int (*)(char *, char *);
using FunAudioHvsApply = int (*)(char *, char *, AudioHvsDynamicData *);

struct AudioHvsAlgoApi {
    FunAudioHvsGetSize getSize{nullptr};
    FunAudioHvsInit initAlgo{nullptr};
    FunAudioHvsSetPara setParaAlgo{nullptr};
    FunAudioHvsApply applyAlgo{nullptr};
};

class AudioSuiteHoaRenderAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteHoaRenderAlgoInterfaceImpl(NodeParameter &nc);
    ~AudioSuiteHoaRenderAlgoInterfaceImpl();

    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override;
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    bool isHoaAlgoInit_ = false;
    AudioHvsAlgoApi algoApi_{nullptr};
    void *libHandle_{nullptr};
    std::vector<char> algoHandle_;
    std::vector<char> scratchBuf_;
    AudioSuiteLibraryManager algoLibrary_;
    
    AudioChannelLayout channelLayout_ = CH_LAYOUT_HOA_ORDER1_ACN_N3D;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_HOA_RENDER_ALGO_INTERFACE_IMPL_H