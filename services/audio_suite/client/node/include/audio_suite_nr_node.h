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

#ifndef AUDIO_SUITE_NR_NODE_H
#define AUDIO_SUITE_NR_NODE_H

#include "audio_suite_algo_interface.h"
#include "audio_suite_process_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteNrNode : public AudioSuiteProcessNode {
public:
    explicit AudioSuiteNrNode();
    ~AudioSuiteNrNode();
    
    int32_t Init() override;
    int32_t DeInit() override;

protected:
    AudioSuitePcmBuffer *SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override;

private:
    AudioSuitePcmBuffer outPcmBuffer_;
    std::shared_ptr<AudioSuiteAlgoInterface> algoInterface_;
    std::vector<uint8_t *> algoInputs_{nullptr};
    std::vector<uint8_t *> algoOutputs_{nullptr};
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_NR_NODE_H