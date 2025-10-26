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

#ifndef AUDIO_SUITE_AISS_ALGO_INTERFACE_IMPL_H
#define AUDIO_SUITE_AISS_ALGO_INTERFACE_IMPL_H

#include "audio_log.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_suite_algo_interface.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteAissAlgoInterfaceImpl : public AudioSuiteAlgoInterface {
public:
    explicit AudioSuiteAissAlgoInterfaceImpl(NodeCapability &nc);

    ~AudioSuiteAissAlgoInterfaceImpl()
    {
        int32_t ret = Deinit();
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "AudioSuiteAissAlgoInterfaceImpl Deinit failed");
    }

    int32_t Init() override;

    int32_t Deinit() override;

    int32_t SetParameter(const std::string& paramType, const std::string& paramValue) override;

    int32_t GetParameter(const std::string& paramType, std::string& paramValue) override;

    int32_t Apply(std::vector<uint8_t*>& v1, std::vector<uint8_t*>& v2) override;
private:
    int32_t CheckFilePath(std::string &filePath);

    int32_t InitIOBufferConfig();

    int32_t InitConfig();

    int32_t InitAudioEffectParam();

    int32_t InitAudioEffectProperty();

    void SeparateChannels(const int32_t &frameLength, float *input, float *humanOutput, float *bkgOutput);

    void* soHandle_{ nullptr };

    AudioEffectLibrary* audioEffectLibHandle_{ nullptr };

    AudioEffectHandle algoHandle_{ nullptr };

    AudioBuffer inAudioBuffer_;

    AudioBuffer outAudioBuffer_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
 
#endif // AUDIO_SUITE_AISS_ALGO_INTERFACE_IMPL_H