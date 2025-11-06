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
#include <gtest/gtest.h>
#include "audio_suite_input_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {
class AudioSuiteOutputPortTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(AudioSuiteOutputPortTest, OutputPort_001, TestSize.Level0)
{
    AudioFormat audioFormat;
    audioFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    audioFormat.audioChannelInfo.numChannels = 2;
    audioFormat.rate = AudioSamplingRate::SAMPLE_RATE_8000;
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);

    std::shared_ptr<OutputPort<AudioSuitePcmBuffer *>> outputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer *>>(inputNode);
    EXPECT_NE(outputPort, nullptr);

    std::vector<AudioSuitePcmBuffer *> data = outputPort->PullOutputData(
        PcmBufferFormat(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S32LE), true);
    EXPECT_EQ(data.size(), 0);

    AudioSuitePcmBuffer *pcm = nullptr;
    outputPort->WriteDataToOutput(pcm);
}

HWTEST_F(AudioSuiteOutputPortTest, OutputPortHandleInputPort_001, TestSize.Level0)
{
    AudioFormat audioFormat;
    audioFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    audioFormat.audioChannelInfo.numChannels = 2;
    audioFormat.rate = AudioSamplingRate::SAMPLE_RATE_8000;
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);

    std::shared_ptr<OutputPort<AudioSuitePcmBuffer *>> outputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer *>>(inputNode);
    EXPECT_NE(outputPort, nullptr);

    InputPort<AudioSuitePcmBuffer*>* inputPort = new InputPort<AudioSuitePcmBuffer*>();
    outputPort->AddInput(inputPort);
    outputPort->RemoveInput(inputPort);

    auto num = outputPort->GetInputNum();
    EXPECT_EQ(num, 0);

    outputPort->SetPortType(AudioNodePortType::AUDIO_NODE_DEFAULT_OUTPORT_TYPE);

    auto type = outputPort->GetPortType();
    EXPECT_EQ(type, AudioNodePortType::AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
}
}