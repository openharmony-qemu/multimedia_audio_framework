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
#ifndef AUDIO_SUITE_PROCESS_NODE_H
#define AUDIO_SUITE_PROCESS_NODE_H

#include <unordered_set>
#include <memory>
#include <vector>
#include "audio_errors.h"
#include "audio_suite_channel.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_capabilities.h"
#include "audio_suite_perf.h"
#include "audio_suite_algo_interface.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteProcessNode : public AudioNode {
public:
    AudioSuiteProcessNode(AudioNodeType nodeType);
    AudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat);
    virtual ~AudioSuiteProcessNode() = default;
    int32_t DoProcess(uint32_t needDataLength) override;
    int32_t Connect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t Flush() override;
    int32_t SetOptions(std::string name, std::string value) override;
    int32_t GetOptions(std::string name, std::string &value) override;
    std::string paraName_ = "";
    std::string paraValue_ = "";
    AudioSuiteProcessNode(const AudioSuiteProcessNode& others) = delete;
    AudioSamplingRate GetSampleRate()
    {
        return AudioNode::GetAudioNodeInfo().audioFormat.rate;
    }
    AudioSampleFormat GetBitWidth()
    {
        return AudioNode::GetAudioNodeInfo().audioFormat.format;
    }
    uint32_t GetChannelCount()
    {
        return AudioNode::GetAudioNodeInfo().audioFormat.audioChannelInfo.numChannels;
    }

    virtual OutputPort<AudioSuitePcmBuffer*>* GetOutputPort() override
    {
        return &outputStream_;
    }

protected:
    virtual std::vector<AudioSuitePcmBuffer *> SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs);
    std::vector<AudioSuitePcmBuffer*>& ReadProcessNodePreOutputData();
    virtual uint32_t CalculationNeedBytes(uint32_t frameLengthMs);
    int32_t InitCacheLength(uint32_t needDataLength);
    int32_t ProcessBypassMode(uint32_t needDataLength);
    int32_t ProcessDirectly();
    int32_t ProcessWithCache();
    int32_t ObtainProcessedData();
    int32_t InitOutputStream();
    // for dfx
    void CheckEffectNodeProcessTime(uint32_t dataDurationMS, uint64_t processDurationUS);
    void CheckEffectNodeOvertimeCount();

    std::unordered_set<std::shared_ptr<AudioNode>> finishedPrenodeSet;
    OutputPort<AudioSuitePcmBuffer *> outputStream_;
    InputPort<AudioSuitePcmBuffer *> inputStream_;
    AudioSuiteFormatConversion convert_;
    
    uint32_t nodeNeedDataDuration_ = 0;
    uint32_t requestPreNodeDuration_ = 0;
    uint32_t frameOutBytes_ = 0;
    uint32_t resultNumber_ = 1;
    uint32_t nextNeedDataLength_ = 0;

    std::vector<AudioSuitePcmBuffer> algoOutPcmBuffer_;
    std::vector<AudioSuiteRingBuffer> cachedBuffer_;
    std::vector<AudioSuitePcmBuffer> downStreamData_;

    std::vector<AudioSuitePcmBuffer *> algoProcessedResult_;
    std::vector<AudioSuitePcmBuffer *> intermediateResult_;
    
    std::vector<uint8_t *> algoInput_{nullptr};
    std::vector<uint8_t *> algoOutput_;
    std::shared_ptr<AudioSuiteAlgoInterface> algoInterface_{ nullptr };
    NodeParameter nodeParameter_;
 
    bool secondCall_ = false;
    bool needCache_ = false;
    bool isOutputPortInit_ = false;

private:
    // for dfx
    int32_t signalProcessTotalCount_ = 0;
    std::array<int32_t, RTF_OVERTIME_LEVELS> rtfOvertimeCounters_{};
    int32_t rtfOver100Count_ = 0;
    uint32_t maxRequestLength = 100;
};

}
}
}
#endif