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

#ifndef AUDIO_SUITE_HOA_RENDER_NODE_H
#define AUDIO_SUITE_HOA_RENDER_NODE_H

#include "audio_suite_hoa_render_algo_interface_impl.h"
#include "audio_suite_process_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteHoaRenderNode : public AudioSuiteProcessNode {
public:
    AudioSuiteHoaRenderNode();
    ~AudioSuiteHoaRenderNode();

    int32_t Init() override;
    int32_t DeInit() override;
    int32_t DoProcess(uint32_t needDataLength) override;
    int32_t PrintNodeInfo(std::string &nodeInfo) override;
private:
    int32_t ObtainProcessedData();
    int32_t InitHoaCacheLength(uint32_t needDataLength);
    int32_t GetProcessedToBuffer();
    int32_t ProcessAlgoChunks();
private:
    bool isInit_ = false;
    AudioSuiteRingBuffer inputBuffer_;
    AudioSuiteRingBuffer outputBuffer_;
    std::vector<uint8_t> algoInputBuffer_;
    std::vector<uint8_t> algoOutputBuffer_;
    bool upstreamFinished_ = false;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_HOA_RENDER_NODE_H