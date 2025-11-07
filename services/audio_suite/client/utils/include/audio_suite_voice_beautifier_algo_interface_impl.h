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

#ifndef AUDIO_SUITE_VOICE_BEAUTIFIER_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_VOICE_BEAUTIFIER_ALGO_INTERFACE_IMPL_H

#include "audio_voicemorphing_api.h"
#include "audio_suite_algo_interface.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using FunAudioVoiceMorphingGetsize = int (*)(AudioVoiceMorphingMemSize *memSize);
using FunAudioVoiceMorphingInit = int (*)(char *handle, char *scratchBuf);
using FunAudioVoiceMorphingSetParam = int (*)(char *handle, AudioVoiceMorphingType type);
using FunAudioVoiceMorphingApply = int (*)(AudioVoiceMorphingData *data, char *handle, char *scratchBuf);

struct VoiceMorphingAlgoApi {
    FunAudioVoiceMorphingGetsize getSize{nullptr};
    FunAudioVoiceMorphingInit init{nullptr};
    FunAudioVoiceMorphingSetParam setParam{nullptr};
    FunAudioVoiceMorphingApply apply{nullptr};
};

static const std::string VOICE_BEAUTIFIER_CLEAR =
    std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
static const std::string VOICE_BEAUTIFIER_THEATRE =
    std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_THEATRE));
static const std::string VOICE_BEAUTIFIER_CD =
    std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
static const std::string VOICE_BEAUTIFIER_RECORDING_STUDIO =
    std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO));
static const std::map<std::string, AudioVoiceMorphingType> OPTIONS_MAP = {
    {VOICE_BEAUTIFIER_CLEAR, AUDIO_VOICE_MORPH_CLEAR},
    {VOICE_BEAUTIFIER_THEATRE, AUDIO_VOICE_MORPH_THEATRE},
    {VOICE_BEAUTIFIER_CD, AUDIO_VOICE_MORPH_CD},
    {VOICE_BEAUTIFIER_RECORDING_STUDIO, AUDIO_VOICE_MORPH_RECORDING_STUDIO}};

class AudioSuiteVoiceBeautifierAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteVoiceBeautifierAlgoInterfaceImpl(NodeCapability &nc);
    ~AudioSuiteVoiceBeautifierAlgoInterfaceImpl();

    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType_, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override
    {
        return SUCCESS;
    }
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    int32_t LoadAlgorithmFunction(void);
    int32_t ApplyAndWaitReady(void);
    void UnApply(void);
    void Release();
    VoiceMorphingAlgoApi vmAlgoApi_{0};
    uint32_t *inBuf_ = nullptr;
    uint32_t *outBuf_ = nullptr;
    char *handle_ = nullptr;
    char *scratchBuf_ = nullptr;
    void *libHandle_{nullptr};
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif