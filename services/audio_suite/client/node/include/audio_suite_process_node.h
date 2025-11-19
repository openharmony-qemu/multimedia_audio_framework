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

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteProcessNode : public AudioNode {
public:
    AudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat);
    virtual ~AudioSuiteProcessNode() = default;
    int32_t DoProcess() override;
    int32_t Connect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t Flush() override;
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

    virtual std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> GetOutputPort() override
    {
        if (!outputStream_) {
            outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
        }
        return outputStream_;
    }

protected:
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> outputStream_;
    std::shared_ptr<InputPort<AudioSuitePcmBuffer*>> inputStream_;
    virtual AudioSuitePcmBuffer* SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs) = 0;
    std::vector<AudioSuitePcmBuffer*>& ReadProcessNodePreOutputData();
    std::unordered_set<std::shared_ptr<AudioNode>> finishedPrenodeSet;
    NodeCapability nodeCapability;
};

}
}
}
#endif