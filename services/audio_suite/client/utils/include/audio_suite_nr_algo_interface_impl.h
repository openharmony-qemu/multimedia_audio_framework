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

#ifndef AUDIO_SUITE_NR_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_NR_ALGO_INTERFACE_IMPL_H

#include "audio_errors.h"
#include "audio_hms_ainr_api.h"
#include "audio_suite_algo_interface.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using FunAudioAinrGetVersion = signed int (*)(unsigned int *version, unsigned int *releaseTime);
using FunAudioAinrGetSize = signed int (*)(signed int *chanSize);
using FunAudioAinrInit = signed int (*)(signed char *handle, AudioAinrPstSysConfig config, unsigned int bufSize);
using FunAudioAinrApply = signed int (*)(signed char *handle, AudioAinrDataTransferPointer pAhaData);

struct AinrAlgoApi {
    FunAudioAinrGetVersion getVersion{nullptr};
    FunAudioAinrGetSize getSize{nullptr};
    FunAudioAinrInit initAlgo{nullptr};
    FunAudioAinrApply applyAlgo{nullptr};
};

class AudioSuiteNrAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteNrAlgoInterfaceImpl(NodeCapability &nc);
    ~AudioSuiteNrAlgoInterfaceImpl();

    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType, const std::string &paramValue) override { return SUCCESS; }
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override { return SUCCESS; }
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    AinrAlgoApi algoApi_{0};
    std::unique_ptr<signed char[]> algoHandle_{nullptr};
    AudioAinrStruSysConfig algoDefaultConfig_{0};
    void *libHandle_{nullptr};
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif