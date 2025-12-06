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
 
#ifndef AUDIO_SUITE_AISS_NODE_H
#define AUDIO_SUITE_AISS_NODE_H

#include "audio_log.h"
#include "audio_suite_process_node.h"
#include "audio_suite_aiss_algo_interface_impl.h"
#include "audio_suite_pcm_buffer.h"
#include "channel_converter.h"
#include "hpae_format_convert.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
 
class AudioSuiteAissNode : public AudioSuiteProcessNode {
public:
    explicit AudioSuiteAissNode();
    ~AudioSuiteAissNode();
    int32_t DoProcess() override;
    int32_t Init() override;
    int32_t DeInit() override;

protected:
    AudioSuitePcmBuffer* SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs) override;
private:
    std::shared_ptr<AudioSuiteAlgoInterface> aissAlgo_{ nullptr };
    bool isInit_ = false;
    AudioSuitePcmBuffer tmpOutput_;
    AudioSuitePcmBuffer tmpHumanSoundOutput_;
    AudioSuitePcmBuffer tmpBkgSoundOutput_;
    std::vector<uint8_t *> tmpin_{nullptr};
    std::vector<uint8_t *> tmpout_{nullptr, nullptr, nullptr};
};
 
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif