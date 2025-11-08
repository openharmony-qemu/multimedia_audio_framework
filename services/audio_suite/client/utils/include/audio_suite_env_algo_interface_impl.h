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

#ifndef AUDIO_SUITE_ENV_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_ENV_ALGO_INTERFACE_IMPL_H
#define ALGO_CHANNEL_NUM (2)    // 算法声道数
#define ALGO_BYTE_NUM (2)       // 算法每个采样点的字节数
#define MASTERVOLUME (15)       // 算法音量设置
#define TWO_BYTES_WIDTH (16)
#define AUDIO_DURATION (2)        // 输入音频的持续时间，以10ms为单位

#include "audio_suite_algo_interface.h"
#include "imedia_api.h"
#include "audio_suite_log.h"
#include <utility>
#include <dlfcn.h>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using FuniMedia_Env_GetSize = int (*)(iMedia_SWS_MEM_SIZE *);
using FuniMedia_Env_Init = int (*)(void *, void *, IMEDIA_INT32 iScratchBufLen, const iMedia_Env_PARA);
using FuniMedia_Env_Apply = int (*)(void *, void *, IMEDIA_INT32 iScratchBufLen, iMedia_SWS_DATA *);
using FuniMedia_Env_SetParams = int (*)(void *, void *, IMEDIA_INT32 iScratchBufLen, const iMedia_Env_PARA);
using FuniMedia_Env_GetParams = int (*)(void *, iMedia_Env_PARA *);

struct EnvAlgoApi {
    FuniMedia_Env_GetSize getSize{nullptr};
    FuniMedia_Env_Init initAlgo{nullptr};
    FuniMedia_Env_Apply applyAlgo{nullptr};
    FuniMedia_Env_SetParams setPara{nullptr};
    FuniMedia_Env_GetParams getPara{nullptr};
};

class AudioSuiteEnvAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteEnvAlgoInterfaceImpl(NodeCapability &nc);
    ~AudioSuiteEnvAlgoInterfaceImpl();

    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override;
    int32_t Apply(std::vector<uint8_t *> &v1, std::vector<uint8_t *> &v2) override;

private:
    bool isEnvAlgoInit_ = false;
    std::vector<char> runBuf_;
    std::vector<char> scratchBuf_;

    void *libHandle_{nullptr};
    EnvAlgoApi algoApi_{0};
    iMedia_SWS_DATA stData_;
    std::vector<uint32_t> dataIn_;
    std::vector<uint32_t> dataOut_;
    iMedia_Env_PARA para_;
    iMedia_SWS_MEM_SIZE stSize_;
    size_t frameLen_;
    size_t inputSamples_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_ENV_ALGO_INTERFACE_IMPL_H