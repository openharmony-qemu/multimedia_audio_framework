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

#ifndef AUDIO_SUITE_CHANNEL_H
#define AUDIO_SUITE_CHANNEL_H

#include <vector>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_format_conversion.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

template <typename T>
class InputPort;

template <typename T>
class OutputPort {
public:
    OutputPort() = default;
    void WriteDataToOutput(T data);
    OutputPort(const OutputPort &that) = delete;
    std::vector<T> PullOutputData(PcmBufferFormat outFormat, bool needConvert);
    int32_t PullOutputDataForDoubleFrame();
    void AddInput(InputPort<T>* input);
    void RemoveInput(InputPort<T>* input);
    size_t GetInputNum() const;
    void SetPortType(AudioNodePortType type) {portType_ = type;}
    AudioNodePortType GetPortType() {return portType_;}
    int32_t ResetResampleCfg();
    int32_t SetOutputPort(std::shared_ptr<AudioNode> node);
private:
    std::set<InputPort<T>*> inputPortSet_;
    std::vector<T> outputData_;
    std::shared_ptr<AudioNode> audioNode_;
    AudioNodePortType portType_ = AudioNodePortType::AUDIO_NODE_DEFAULT_OUTPORT_TYPE;

    std::vector<std::unique_ptr<AudioSuiteFormatConversion>> convert_;
    std::vector<AudioSuitePcmBuffer> tmpData_;
};

template <typename T>
class InputPort {
public:
    InputPort()
    {}
    ~InputPort();
    std::vector<T>& ReadPreOutputData(PcmBufferFormat outFormat, bool needConvert);

    void Connect(const std::shared_ptr<AudioNode>& node, OutputPort<T>* output);

    void DisConnect(const std::shared_ptr<AudioNode>& node);

    size_t GetPreOutputNum() const;

    const std::unordered_map<OutputPort<T>*, std::shared_ptr<AudioNode>>& GetPreOutputMap();

    bool CheckIfDisConnected(OutputPort<T>* output);

    InputPort(const InputPort &that) = delete;

    void deInit();

    void AddPreOutput(const std::shared_ptr<AudioNode>& node, OutputPort<T>* output);
    void RemovePreOutput(OutputPort<T>* output);
    std::vector<T> getInputData();
    std::vector<T>& getInputDataRef();
private:
    std::unordered_map<OutputPort<T>*, std::shared_ptr<AudioNode>> outputPorts_;
    std::vector<T> inputData_;
};

}
}
}
#endif