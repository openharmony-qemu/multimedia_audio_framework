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

#ifndef AUDIO_SUITE_TEMPO_PITCH_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_TEMPO_PITCH_ALGO_INTERFACE_IMPL_H

#include "audio_suite_algo_interface.h"
#include "audio_suite_base.h"
#include "audio_suite_tempo_pitch_api.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using TEMPO_CREATE_FUNC = PVParam(*)(int);
using TEMPO_DESTROY_FUNC = void(*)(PVParam);
using TEMPO_SET_FUNC = int(*)(PVParam, float);
using TEMPO_APPLY_FUNC = int(*)(PVParam, const short *, short *, int, int);

struct TempoAlgoApi {
    TEMPO_CREATE_FUNC create{nullptr};
    TEMPO_DESTROY_FUNC destroy{nullptr};
    TEMPO_SET_FUNC setParam{nullptr};
    TEMPO_APPLY_FUNC apply{nullptr};
};

namespace {
const std::string PITCH_LIB = "PITCHLIB";
static constexpr int32_t RESIZE_EXPAND_SIZE = 256;
static constexpr int32_t INIT_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr int32_t TEMPO_PITCH_ALGO_FRAME_LEN = 960;      // 20ms data, 480 samples * 1 channel
}  // namespace

class AudioSuiteTempoPitchAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    AudioSuiteTempoPitchAlgoInterfaceImpl(NodeCapability &nc);
    ~AudioSuiteTempoPitchAlgoInterfaceImpl();

    int32_t TempoInit(std::string soName);
    int32_t PitchInit(std::string soName);
    int32_t Init() override;
    int32_t Deinit() override;
    int32_t SetParameter(const std::string &paramType_, const std::string &paramValue) override;
    int32_t GetParameter(const std::string &paramType, std::string &paramValue) override;
    int32_t Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs) override;

private:
    float speedRate_ = 1.0f;    // 变速
    float pitchRate_ = 1.0f;    // 变调
    int32_t expendSize_ = 0;
    // tempo
    TempoAlgoApi tempoAlgoApi_{0};
    PVParam tempoAlgoHandle_{nullptr};
    void *tempoSoHandle_{nullptr};
    // pitch
    void *pitchSoHandle_{nullptr};
    AudioEffectLibrary* pitchLibHandle_ = nullptr;
    AudioEffectHandle pitchAlgoHandle_ = nullptr;

    std::vector<int16_t> tempDataOut_;

    std::vector<float> ParseStringToFloatArray(const std::string &str, char delimiter)
    {
        std::vector<float> params;
        std::string token;
        std::istringstream iss(str);

        while (std::getline(iss, token, delimiter)) {
            if (!token.empty()) {
                params.push_back(std::stof(token));
            }
        }
        return params;
    }
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif