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
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_input_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"

#include "audio_suite_mixer_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_limiter.h"
#include "audio_suite_channel.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

// Test helper class to access private members
class AudioSuiteMixerNodeTestHelper : public AudioSuiteMixerNode {
public:
    explicit AudioSuiteMixerNodeTestHelper(uint32_t threadCount) : AudioSuiteMixerNode(threadCount) {}

    // Expose private methods for testing
    using AudioSuiteMixerNode::SubmitPullTasks;

    // Allow access to finishedPrenodeSet and inputStream_
    std::unordered_set<std::shared_ptr<AudioNode>>& GetFinishedPrenodeSet()
    {
        return finishedPrenodeSet;
    }

    InputPort<AudioSuitePcmBuffer*>& GetInputStream()
    {
        return inputStream_;
    }
};

std::string g_fileNameOne = "/data/mix1_48000_2_32f.pcm";
std::string g_fileNameTwo = "/data/mix2_48000_2_32f.pcm";
std::string g_outFilename = "/data/out.pcm";
std::string g_baseFilename = "/data/base_mix_48000_32_2.pcm";

AudioFormat audioFormat = {
    {
        CH_LAYOUT_STEREO,
        2,
    },
    SAMPLE_F32LE,
    SAMPLE_RATE_48000
};
const uint32_t CHANNEL_COUNT = 2;
const AudioChannelLayout LAY_OUT = CH_LAYOUT_STEREO;

size_t g_frameCount20Ms = (SAMPLE_RATE_48000 * STEREO * 20) / 1000; // 20ms of data
size_t g_frameCount20MsTwo = (SAMPLE_RATE_48000 * STEREO * 20) / 1000; // 20ms of data
static constexpr uint32_t NEED_DATA_LENGTH = 20;

class AudioSuiteMixerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteMixerTest::SetUp()
{
    if (!AllNodeTypesSupported()) {
        GTEST_SKIP() << "not support all node types, skip this test";
    }
}

void AudioSuiteMixerTest::TearDown()
{}

namespace {
HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNode, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> audioSuiteMixerNode =std::make_shared<AudioSuiteMixerNode>(5);
    EXPECT_EQ(audioSuiteMixerNode->GetSampleRate(), audioFormat.rate);
}

HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNodeReadFile, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    int32_t ret = node->InitCacheLength(NEED_DATA_LENGTH);
    EXPECT_EQ(ret, SUCCESS);

    std::ifstream file1(g_fileNameOne, std::ios::binary | std::ios::ate);
    std::ifstream file2(g_fileNameTwo, std::ios::binary | std::ios::ate);

    ASSERT_TRUE(file1.is_open()) << "Failed to open file1: " << g_fileNameOne;
    ASSERT_TRUE(file2.is_open()) << "Failed to open file2: " << g_fileNameTwo;

    file1.seekg(0, std::ios::beg);
    file2.seekg(0, std::ios::beg);

    
    std::ofstream outProcessedFile(g_outFilename, std::ios::binary);
    AudioSuitePcmBuffer buffer1(PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    AudioSuitePcmBuffer buffer2(PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    uint32_t dataSize = buffer1.GetDataSize();

    while (true) {

        file1.read(reinterpret_cast<char *>(buffer1.GetPcmData()), dataSize);
        file2.read(reinterpret_cast<char *>(buffer2.GetPcmData()), dataSize);

        if (file1.eof() || file2.eof()) {
            break;
        }
        std::vector<AudioSuitePcmBuffer *> inputs = {&buffer1, &buffer2};
        std::vector<AudioSuitePcmBuffer *> outPcmbuffer = node->SignalProcess(inputs);
        outProcessedFile.write(reinterpret_cast<const char *>(outPcmbuffer[0]->GetPcmData()),
            dataSize);
        inputs.clear();
    }
    file1.close();
    file2.close();
    outProcessedFile.close();
}

// ============================================================================
// Test CollectPullResults - Basic Functionality
// ============================================================================

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_SingleTask, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = true;
    mockResult.preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    
    AudioSuitePcmBuffer* mockBuffer = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    mockResult.data = {mockBuffer};

    futures.push_back(std::async([mockResult]() mutable {
        return mockResult;
    }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 1);
    EXPECT_NE(preOutputs[0], nullptr);

    delete mockBuffer;
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_EmptyData, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = nullptr;
    mockResult.data = {};  // Empty vector

    futures.push_back(std::async([&]() {
        return mockResult;
    }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);  // Should return true even with empty data
    EXPECT_EQ(preOutputs.size(), 0);
}

HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNodeCompar, TestSize.Level0)
{
    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_baseFilename, std::ios::binary);

    ASSERT_TRUE(outFile.is_open()) << "Failed to open out.pcm";
    ASSERT_TRUE(baseFile.is_open()) << "Failed to open base.pcm";

    std::vector<char> out_data;
    std::vector<char> base_data;

    outFile.seekg(0, std::ios::end);
    out_data.resize(outFile.tellg());
    outFile.seekg(0, std::ios::beg);
    outFile.read(out_data.data(), out_data.size());

    baseFile.seekg(0, std::ios::end);
    base_data.resize(baseFile.tellg());
    baseFile.seekg(0, std::ios::beg);
    baseFile.read(base_data.data(), base_data.size());

    outFile.close();
    baseFile.close();

    EXPECT_EQ(out_data, base_data) << "out.pcm and base.pcm are not identical";
    std::remove(g_outFilename.c_str());
}

// ============================================================================
// Test SubmitPullTasks - Functionality Coverage
// ============================================================================

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_EmptyMap, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::unordered_map<OutputPort<AudioSuitePcmBuffer*>*, std::shared_ptr<AudioNode>> emptyMap;

    auto futures = node->SubmitPullTasks(emptyMap);

    EXPECT_TRUE(futures.empty());
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_NullOutputPort, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::unordered_map<OutputPort<AudioSuitePcmBuffer*>*, std::shared_ptr<AudioNode>> preOutputMap;
    auto mockNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    preOutputMap[nullptr] = mockNode;

    auto futures = node->SubmitPullTasks(preOutputMap);

    EXPECT_EQ(futures.size(), 1);
    auto result = futures[0].get();
    EXPECT_FALSE(result.ok);
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_NullNode, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::unordered_map<OutputPort<AudioSuitePcmBuffer*>*, std::shared_ptr<AudioNode>> preOutputMap;
    // Create a real OutputPort on the stack
    OutputPort<AudioSuitePcmBuffer*> mockPort;
    preOutputMap[&mockPort] = nullptr;

    auto futures = node->SubmitPullTasks(preOutputMap);

    EXPECT_EQ(futures.size(), 1);
    auto result = futures[0].get();
    EXPECT_FALSE(result.ok);
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_WithConnectedNode, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    // Create an input node that will provide data
    auto inputNode = std::make_shared<AudioInputNode>(audioFormat);
    inputNode->Init();

    // Connect the input node to the mixer node
    node->Connect(inputNode);

    // Get the preOutputMap from the mixer node's input stream
    auto& preOutputMap = node->GetInputStream().GetPreOutputMap();

    auto futures = node->SubmitPullTasks(preOutputMap);

    EXPECT_EQ(futures.size(), 1);
    auto result = futures[0].get();
    // The input node may return empty data or valid data, depending on its state
    // The important thing is that the future completes without crash
    EXPECT_FALSE(result.ok); // Input node should return empty data initially
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_MultipleConnectedNodes, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    // Create multiple input nodes
    auto inputNode1 = std::make_shared<AudioInputNode>(audioFormat);
    inputNode1->Init();
    auto inputNode2 = std::make_shared<AudioInputNode>(audioFormat);
    inputNode2->Init();

    // Connect both nodes to the mixer
    node->Connect(inputNode1);
    node->Connect(inputNode2);

    // Get the preOutputMap
    auto& preOutputMap = node->GetInputStream().GetPreOutputMap();

    auto futures = node->SubmitPullTasks(preOutputMap);

    EXPECT_EQ(futures.size(), 2);

    // Get both results
    auto result1 = futures[0].get();
    auto result2 = futures[1].get();

    // Both should complete without crash
    EXPECT_FALSE(result1.ok); // Input nodes return empty data initially
    EXPECT_FALSE(result2.ok);
}

// ============================================================================
// Test ReadProcessNodePreOutputData - Integration Test
// ============================================================================

HWTEST_F(AudioSuiteMixerTest, ReadProcessNodePreOutputData_EmptyInputs, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    auto& preOutputs = node->ReadProcessNodePreOutputData();

    EXPECT_TRUE(preOutputs.empty());
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_NullPreNode, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = nullptr; // Null preNode
    mockResult.data = {nullptr};
    futures.push_back(std::async([&]() {
        return mockResult;
    }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0); // Should not add data when preNode is null
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_NotOk, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = false; // Not ok
    mockResult.isFinished = false;
    mockResult.preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    mockResult.data = {};
    futures.push_back(std::async([&]() {
        return mockResult;
    }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0);
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_FinishedPrenodeInSet, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    auto preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    auto helper = std::static_pointer_cast<AudioSuiteMixerNodeTestHelper>(node);
    if (helper) {
        // Add preNode to finished set
        helper->GetFinishedPrenodeSet().insert(preNode);
    }

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = preNode; // This node is in finished set
    mockResult.data = {nullptr};
    futures.push_back(std::async([&]() {
        return mockResult;
    }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0); // Should skip output from finished node
}

}  // namespace