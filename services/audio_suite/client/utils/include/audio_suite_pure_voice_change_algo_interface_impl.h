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

#ifndef AUDIO_SUITE_PURE_VOICE_CHANGE_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_PURE_VOICE_CHANGE_ALGO_INTERFACE_IMPL_H
#define ALGO_CHANNEL_PURE_NUM (1)    // 算法声道数
#define ALGO_BYTE_PURE_NUM (2)       // 算法每个采样点的字节数
#define AUDIO_PURE_DURATION (2)        // 输入音频的持续时间，以10ms为单位

#include "audio_suite_algo_interface.h"
#include "imedia_api.h"
#include <utility>
#include <dlfcn.h>
#include "audio_errors.h"
#include "audio_voicemorphing_api.h"
#include "audio_suite_algo_interface.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using FunVoiceMphGetSize = int (*)(AudioVoiceMphMemSize *memSize);
using FunVoiceMphInit = int (*)(char *handle, char *scratchBuf);
using FunVoiceMphApply = int (*)(char *handle, char *scratchBuf, AudioVoiceMphData *data);
using FunVoiceMphSetPara = int (*)(char *handle, SpeakerSex gender, AudioVoiceMphTradType type, float pitch);

static const std::unordered_map<std::string, AudioVoiceMphTradType> pureTypeMap = {
    {"1", AUDIO_VOICE_MPH_TRAD_CARTOON},
    {"2", AUDIO_VOICE_MPH_TRAD_CUTE},
    {"3", AUDIO_VOICE_MPH_TRAD_FEMALE},
    {"4", AUDIO_VOICE_MPH_TRAD_MALE},
    {"5", AUDIO_VOICE_MPH_TRAD_MONSTER},
    {"6", AUDIO_VOICE_MPH_TRAD_ROBOTS},
    {"7", AUDIO_VOICE_MPH_TRAD_SEASONED}
} ;

static const std::unordered_map<std::string, SpeakerSex> pureSexTypeMap = {
    {"1", VMP_TRAD_FEMALE},
    {"2", VMP_TRAD_MALE}
} ;

struct VoiceMphingAlgoApi {
    FunVoiceMphGetSize getSize{nullptr};
    FunVoiceMphInit initAlgo{nullptr};
    FunVoiceMphSetPara setPara{nullptr};
    FunVoiceMphApply applyAlgo{nullptr};
};

class AudioSuitePureVoiceChangeAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    AudioSuitePureVoiceChangeAlgoInterfaceImpl(NodeParameter &nc);
    ~AudioSuitePureVoiceChangeAlgoInterfaceImpl();

    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override;
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    int32_t LoadAlgorithmFunction(void);
    int32_t ApplyAndWaitReady(void);
    void UnApply(void);
    void Release();
    VoiceMphingAlgoApi vmAlgoApi_{0};
    std::vector<float> inBuf_;
    std::vector<float> outBuf_;
    char *handle_ = nullptr;
    char *scratchBuf_ = nullptr;
    void *libHandle_{nullptr};
    AudioSuiteLibraryManager algoLibrary_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_PURE_VOICE_CHANGE_ALGO_INTERFACE_IMPL_H