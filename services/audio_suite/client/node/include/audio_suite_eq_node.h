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

#ifndef AUDIO_SUITE_EQ_NODE_H
#define AUDIO_SUITE_EQ_NODE_H

#include "audio_suite_eq_algo_interface_impl.h"
#include "audio_suite_process_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteEqNode : public AudioSuiteProcessNode {
public:
    AudioSuiteEqNode();
    ~AudioSuiteEqNode();
    int32_t Init() override;
    int32_t DeInit() override;
    bool Reset() override;
    bool IsEqNodeInit();
    bool SetEqMode(EqualizerMode type);
    int32_t SetOptions(std::string name, std::string value) override;
    int32_t GetOptions(std::string name, std::string &value) override;
    AudioSuitePcmBuffer outPcmBuffer_;
    AudioSuitePcmBuffer tmpPcmBuffer_;

protected:
    AudioSuitePcmBuffer *SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override;

private:
    bool isEqNodeInit_ = false;
    std::vector<uint8_t> eqInputDataBuffer_;
    std::vector<uint8_t> eqOutputDataBuffer_;
    std::vector<uint8_t *> tmpin_;
    std::vector<uint8_t *> tmpout_;
    std::shared_ptr<AudioSuiteEqAlgoInterfaceImpl> eqAlgoInterfaceImpl_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_EQ_NODE_H