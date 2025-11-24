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

#ifndef AUDIO_SUITE_TEMPO_PITCH_NODE_H
#define AUDIO_SUITE_TEMPO_PITCH_NODE_H
 
#include "audio_suite_tempo_pitch_algo_interface_impl.h"
#include "audio_suite_process_node.h"
#include <queue>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteTempoPitchNode : public AudioSuiteProcessNode {
public:
    explicit AudioSuiteTempoPitchNode();
    ~AudioSuiteTempoPitchNode();
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t DoProcess() override;
    int32_t DoProcessPreOutputs(AudioSuitePcmBuffer** tempOut);
    int32_t SetOptions(std::string name, std::string value) override;
    int32_t GetOptions(std::string name, std::string &value) override;
    int32_t PadBufferToPcmBuffer(AudioSuitePcmBuffer &pcmBuffer);
    int32_t SplitDataToQueue(uint8_t* outBuffer, int32_t outFrameBytes);

protected:
    AudioSuitePcmBuffer *SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override;

private:
    bool isInit_ = false;
    std::shared_ptr<AudioSuiteTempoPitchAlgoInterfaceImpl> algoInterface_;
    AudioSuitePcmBuffer outPcmBuffer_;
    std::vector<uint8_t> outBuffer_;    // 每次apply输出的buffer
    std::vector<uint8_t> currentDataBuffer_;    // 当前缓存的buffer
    std::vector<uint8_t *> tmpin_;
    std::vector<uint8_t *> tmpout_;
 
    int32_t bufferRemainSize_ = 0;  // 当前缓存的buffer剩余容量
    std::queue<std::vector<uint8_t>> readyDataBuffer_;
    bool readFinishedFlag_ = false;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_TEMPO_PITCH_NODE_H