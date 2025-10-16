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

#ifndef AUDIO_SUITE_ENV_NODE_H
#define AUDIO_SUITE_ENV_NODE_H
#define RESAMPLE_QUALITY (5)

#include <vector>
#include <string>
#include <iostream>
#include "audio_suite_env_algo_interface_impl.h"
#include "audio_suite_process_node.h"
#include "audio_suite_log.h"
#include "audio_suite_info.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteEnvNode : public AudioSuiteProcessNode {
public:
    AudioSuiteEnvNode();
    ~AudioSuiteEnvNode();
    int32_t Init() override;
    int32_t DeInit() override;
    bool Reset() override;
    int32_t SetOptions(std::string name, std::string value) override;
    std::shared_ptr<AudioSuiteEnvAlgoInterfaceImpl> envAlgoInterfaceImpl_;
    AudioSuitePcmBuffer outPcmBuffer_;

protected:
    AudioSuitePcmBuffer *SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override;

private:
    bool isInit_ = false;
    int32_t preProcess(AudioSuitePcmBuffer *inputPcmBuffer);
    int32_t CopyBuffer(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer);
    int32_t DoChannelConvert(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer);
    int32_t DoResample(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer);
    std::vector<uint8_t> inputDataBuffer_;
    std::vector<uint8_t> outputDataBuffer_;
    std::vector<uint8_t *> tmpin_;
    std::vector<uint8_t *> tmpout_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_ENV_NODE_H