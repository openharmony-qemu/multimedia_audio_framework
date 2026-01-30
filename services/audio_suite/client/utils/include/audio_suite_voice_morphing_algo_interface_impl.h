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

#ifndef AUDIO_SUITE_VOICE_MORPHING_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_VOICE_MORPHING_ALGO_INTERFACE_IMPL_H

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

static const std::unordered_map<std::string, AudioVoiceMorphingType> voiceBeautifierTypeMap = {
    {"1", AUDIO_VOICE_MORPH_CLEAR},
    {"2", AUDIO_VOICE_MORPH_THEATRE},
    {"3", AUDIO_VOICE_MORPH_CD},
    {"4", AUDIO_VOICE_MORPH_RECORDING_STUDIO}
};

static const std::unordered_map<std::string, AudioVoiceMorphingType> generalVoiceChangeTypeMap = {
    {"1", AUDIO_VOICE_MPH_CUTE},
    {"2", AUDIO_VOICE_MPH_CYBERPUNK},
    {"3", AUDIO_VOICE_MPH_FEMALE},
    {"4", AUDIO_VOICE_MPH_MALE},
    {"5", AUDIO_VOICE_MPH_MIX},
    {"6", AUDIO_VOICE_MPH_MONSTER},
    {"7", AUDIO_VOICE_MPH_SEASONED},
    {"8", AUDIO_VOICE_MPH_SYNTH},
    {"9", AUDIO_VOICE_MPH_TRILL},
    {"10", AUDIO_VOICE_MPH_WAR}
};

class AudioSuiteVoiceMorphingAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    AudioSuiteVoiceMorphingAlgoInterfaceImpl(NodeParameter &nc);
    ~AudioSuiteVoiceMorphingAlgoInterfaceImpl();

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
    VoiceMorphingAlgoApi vmAlgoApi_{0};
    std::vector<uint32_t> inBuf_;
    std::vector<uint32_t> outBuf_;
    std::vector<char> handle_;
    std::vector<char> scratchBuf_;
    void *libHandle_{nullptr};
    AudioSuiteLibraryManager algoLibrary_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif