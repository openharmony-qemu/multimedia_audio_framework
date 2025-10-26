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

#ifndef AUDIO_SUITE_SOUNDFIELD_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_SOUNDFIELD_ALGO_INTERFACE_IMPL_H

#include "imedia_api.h"
#include "audio_suite_algo_interface.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr size_t SOUNDFIELD_ALGO_FRAME_LEN = 960;      // 10ms data, 480 samples * 2 channel
static constexpr size_t SOUNDFIELD_ALGO_FRAME_SIZE = SOUNDFIELD_ALGO_FRAME_LEN * sizeof(int16_t);
}  // namespace

using Fun_iMedia_Surround_GetSize = IMEDIA_INT32 (*)(iMedia_SWS_MEM_SIZE *pMemSize);
using Fun_iMedia_Surround_Init = IMEDIA_INT32 (*)(IMEDIA_VOID *pHandle, IMEDIA_VOID *pScratchBuf,
    IMEDIA_INT32 iScratchBufLen, const iMedia_Surround_PARA surroundType);
using Fun_iMedia_Surround_Apply = IMEDIA_INT32 (*)(
    IMEDIA_VOID *pHandle, IMEDIA_VOID *pScratchBuf, IMEDIA_INT32 iScratchBufLen, iMedia_SWS_DATA *pData);
using Fun_iMedia_Surround_SetParams = IMEDIA_INT32 (*)(IMEDIA_VOID *pHandle, IMEDIA_VOID *pScratchBuf,
    IMEDIA_INT32 iScratchBufLen, const iMedia_Surround_PARA surroundType);
using Fun_iMedia_Surround_GetParams = IMEDIA_INT32 (*)(IMEDIA_VOID *pHandle, iMedia_Surround_PARA *pSurroundType);

struct SoundFieldAlgoApi {
    Fun_iMedia_Surround_GetSize getSize{nullptr};
    Fun_iMedia_Surround_Init initAlgo{nullptr};
    Fun_iMedia_Surround_Apply applyAlgo{nullptr};
    Fun_iMedia_Surround_SetParams setPara{nullptr};
    Fun_iMedia_Surround_GetParams getPara{nullptr};
};

class AudioSuiteSoundFieldAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteSoundFieldAlgoInterfaceImpl(NodeCapability &nc);
    ~AudioSuiteSoundFieldAlgoInterfaceImpl();

    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override;
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    void *libHandle_{nullptr};
    SoundFieldAlgoApi algoApi_{0};
    std::unique_ptr<uint8_t[]> algoRunBuf_{nullptr};
    std::unique_ptr<uint8_t[]> algoScratchBuf_{nullptr};
    iMedia_SWS_MEM_SIZE stSize_{0};
    iMedia_SWS_DATA stData_{0};
    std::array<IMEDIA_INT32, SOUNDFIELD_ALGO_FRAME_LEN> dataIn_;
    std::array<IMEDIA_INT32, SOUNDFIELD_ALGO_FRAME_LEN> dataOut_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif