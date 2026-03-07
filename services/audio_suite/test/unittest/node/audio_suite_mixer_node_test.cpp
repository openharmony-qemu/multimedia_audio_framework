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
#include <gmock/gmock.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <future>
#include <memory>
#include <unordered_set>
#include <vector>

#include "audio_errors.h"
#include "audio_suite_mixer_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_process_node.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

// Test helper class to access private/protected members (depending on new design)
class AudioSuiteMixerNodeTestHelper : public AudioSuiteMixerNode {
public:
    explicit AudioSuiteMixerNodeTestHelper(uint32_t threadCount) : AudioSuiteMixerNode(threadCount) {}

    using AudioSuiteMixerNode::SubmitPullTasks;

    std::unordered_set<std::shared_ptr<AudioNode>> &GetFinishedPrenodeSet()
    {
        return finishedPrenodeSet;
    }

    const std::vector<std::weak_ptr<AudioNode>> &GetPreNodesForTest() const
    {
        return preNodes_;
    }
};

namespace {

/**
 * Producer pre-node for mixer:
 * When mixer triggers pull on pre-node, pre-node's DoProcess() should be called
 * and it writes a pcm buffer into outputData_ via WriteOutputData().
 *
 * NOTE:
 * - This class assumes AudioNode has WriteOutputData() (per refactor design).
 * - We rely on mixer to pull via DoProcess chain.
 */
class FakeProducerPreNode : public AudioNode {
public:
    explicit FakeProducerPreNode(const PcmBufferFormat &fmt)
        : AudioNode(NODE_TYPE_EQUALIZER), fmt_(fmt)
    {
        buffer_ = std::make_unique<AudioSuitePcmBuffer>(fmt_);
    }

    ~FakeProducerPreNode() override = default;

    MOCK_METHOD(int32_t, DoProcess, (uint32_t needDataLength), (override));

    int32_t Flush() override
    {
        return SUCCESS;
    }

    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) {
            return ERR_INVALID_PARAM;
        }
        AddPreNode(preNode);
        preNode->AddNextNode(GetSharedInstance());
        return SUCCESS;
    }

    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) {
            return ERR_INVALID_PARAM;
        }
        RemovePreNode(preNode);
        preNode->RemoveNextNode(GetSharedInstance());
        return SUCCESS;
    }

    void SetPattern(uint8_t v)
    {
        pattern_ = v;
    }

    void SetFinished(bool finished)
    {
        finished_ = finished;
    }

    // called by gmock Invoke
    int32_t ProduceOnce(uint32_t needLen)
    {
        (void)needLen;
        uint8_t *p = buffer_->GetPcmData();
        if (p != nullptr) {
            (void)memset_s(p, buffer_->GetDataSize(), pattern_, buffer_->GetDataSize());
        }
        buffer_->SetIsFinished(finished_);
        WriteOutputData(buffer_.get());
        return SUCCESS;
    }

private:
    PcmBufferFormat fmt_;
    std::unique_ptr<AudioSuitePcmBuffer> buffer_;
    uint8_t pattern_ {0x11};
    bool finished_ {false};
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
static constexpr uint32_t NEED_DATA_LENGTH = 20;

class AudioSuiteMixerTest : public testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
    }
    void TearDown() override {}
};

HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNode, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> audioSuiteMixerNode = std::make_shared<AudioSuiteMixerNode>(5);
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
        ASSERT_FALSE(outPcmbuffer.empty());
        ASSERT_NE(outPcmbuffer[0], nullptr);

        outProcessedFile.write(reinterpret_cast<const char *>(outPcmbuffer[0]->GetPcmData()), dataSize);
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

    AudioSuitePcmBuffer *mockBuffer = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    mockResult.data = {mockBuffer};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

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
    mockResult.data = {};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
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
// Test SubmitPullTasks - Adapted for "delete port" scheme
// ============================================================================

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_EmptyPreNodes, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);
    auto futures = node->SubmitPullTasks();

    EXPECT_TRUE(futures.empty());
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_NullNode, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    auto futures = node->SubmitPullTasks();
    EXPECT_TRUE(futures.empty());
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_WithConnectedProducerNode_ShouldOk, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    // Create a producer preNode that always outputs one buffer
    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};
    auto preNode = std::make_shared<FakeProducerPreNode>(fmt);
    preNode->Init();
    preNode->InitFormatConverters();
    preNode->SetPattern(0x22);
    preNode->SetFinished(false);

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .WillRepeatedly(::testing::Invoke([&](uint32_t needLen) { return preNode->ProduceOnce(needLen); }));

    node->Connect(preNode);

    auto futures = node->SubmitPullTasks();
    ASSERT_EQ(futures.size(), 1);

    auto result = futures[0].get();
    EXPECT_TRUE(result.ok);
    EXPECT_FALSE(result.data.empty());
    EXPECT_NE(result.data[0], nullptr);

    // basic content check: first byte matches pattern
    EXPECT_EQ(result.data[0]->GetPcmData()[0], 0x22);
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_MultipleConnectedProducerNodes_ShouldOk, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};

    auto preNode1 = std::make_shared<FakeProducerPreNode>(fmt);
    preNode1->Init();
    preNode1->InitFormatConverters();
    preNode1->SetPattern(0x33);
    EXPECT_CALL(*preNode1, DoProcess(NEED_DATA_LENGTH))
        .WillRepeatedly(::testing::Invoke([&](uint32_t needLen) { return preNode1->ProduceOnce(needLen); }));

    auto preNode2 = std::make_shared<FakeProducerPreNode>(fmt);
    preNode2->Init();
    preNode2->InitFormatConverters();
    preNode2->SetPattern(0x44);
    EXPECT_CALL(*preNode2, DoProcess(NEED_DATA_LENGTH))
        .WillRepeatedly(::testing::Invoke([&](uint32_t needLen) { return preNode2->ProduceOnce(needLen); }));

    node->Connect(preNode1);
    node->Connect(preNode2);

    auto futures = node->SubmitPullTasks();
    ASSERT_EQ(futures.size(), 2);

    auto result1 = futures[0].get();
    auto result2 = futures[1].get();

    EXPECT_TRUE(result1.ok);
    EXPECT_TRUE(result2.ok);
    EXPECT_FALSE(result1.data.empty());
    EXPECT_FALSE(result2.data.empty());
}

// ============================================================================
// Test ReadProcessNodePreOutputData - Integration Test
// ============================================================================

HWTEST_F(AudioSuiteMixerTest, ReadProcessNodePreOutputData_EmptyInputs, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    node->ReadProcessNodePreOutputData();
    EXPECT_TRUE(node->preNodeResult_.empty());
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
    mockResult.preNode = nullptr;
    mockResult.data = {nullptr};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0);
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_NotOk, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = false;
    mockResult.isFinished = false;
    mockResult.preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    mockResult.data = {};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0);
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_FinishedPrenodeInSet, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    auto preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    node->GetFinishedPrenodeSet().insert(preNode);

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = preNode;
    mockResult.data = {nullptr};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0);
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_WeakPtrExpired, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};
    auto preNode = std::make_shared<FakeProducerPreNode>(fmt);
    node->Connect(preNode);
    preNode.reset();

    auto futures = node->SubmitPullTasks();
    EXPECT_TRUE(futures.empty());
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_MultipleDataWithNull, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    
    AudioSuitePcmBuffer *mockBuffer1 = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    AudioSuitePcmBuffer *mockBuffer2 = nullptr;
    mockResult.data = {mockBuffer1, mockBuffer2};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_FALSE(isFinished);
    EXPECT_EQ(preOutputs.size(), 1);

    delete mockBuffer1;
}

HWTEST_F(AudioSuiteMixerTest, ReadProcessNodePreOutputData_WithPreNodes, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};
    auto preNode = std::make_shared<FakeProducerPreNode>(fmt);
    preNode->Init();
    preNode->InitFormatConverters();
    preNode->SetPattern(0x55);
    preNode->SetFinished(false);

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .WillRepeatedly(::testing::Invoke([&](uint32_t needLen) { return preNode->ProduceOnce(needLen); }));

    node->Connect(preNode);
    node->ReadProcessNodePreOutputData();

    EXPECT_FALSE(node->preNodeResult_.empty());
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_EmptyDataInFutures, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = true;
    mockResult.preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    mockResult.data = {};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0);
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_MultipleBuffersWithNull, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    
    AudioSuitePcmBuffer *mockBuffer1 = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    AudioSuitePcmBuffer *mockBuffer2 = nullptr;
    AudioSuitePcmBuffer *mockBuffer3 = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    mockResult.data = {mockBuffer1, mockBuffer2, mockBuffer3};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_FALSE(isFinished);
    EXPECT_EQ(preOutputs.size(), 2);

    delete mockBuffer1;
    delete mockBuffer3;
}

HWTEST_F(AudioSuiteMixerTest, CollectPullResults_FinishedNodeInSet, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> preOutputs;
    std::vector<std::future<AudioSuiteMixerNode::PullResult>> futures;

    auto preNode = std::make_shared<AudioSuiteProcessNode>(AudioNodeType::NODE_TYPE_EQUALIZER);
    node->GetFinishedPrenodeSet().insert(preNode);

    AudioSuiteMixerNode::PullResult mockResult;
    mockResult.ok = true;
    mockResult.isFinished = false;
    mockResult.preNode = preNode;
    
    AudioSuitePcmBuffer *mockBuffer = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE));
    mockResult.data = {mockBuffer};

    futures.push_back(std::async([mockResult]() mutable { return mockResult; }));

    bool isFinished = node->CollectPullResults(preOutputs, futures);

    EXPECT_TRUE(isFinished);
    EXPECT_EQ(preOutputs.size(), 0);

    delete mockBuffer;
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_WithExpiredPreNodes, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};
    auto preNode1 = std::make_shared<FakeProducerPreNode>(fmt);
    auto preNode2 = std::make_shared<FakeProducerPreNode>(fmt);
    
    node->Connect(preNode1);
    node->Connect(preNode2);
    
    preNode1.reset();
    preNode2.reset();

    auto futures = node->SubmitPullTasks();
    EXPECT_TRUE(futures.empty());
}

HWTEST_F(AudioSuiteMixerTest, SubmitPullTasks_MixedValidAndExpired, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNodeTestHelper>(2);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};
    auto preNode1 = std::make_shared<FakeProducerPreNode>(fmt);
    auto preNode2 = std::make_shared<FakeProducerPreNode>(fmt);
    
    preNode1->Init();
    preNode1->InitFormatConverters();
    preNode1->SetPattern(0x66);
    EXPECT_CALL(*preNode1, DoProcess(NEED_DATA_LENGTH))
        .WillRepeatedly(::testing::Invoke([&](uint32_t needLen) { return preNode1->ProduceOnce(needLen); }));

    node->Connect(preNode1);
    node->Connect(preNode2);
    
    preNode2.reset();

    auto futures = node->SubmitPullTasks();
    EXPECT_EQ(futures.size(), 1);

    auto result = futures[0].get();
    EXPECT_TRUE(result.ok);
}

HWTEST_F(AudioSuiteMixerTest, SignalProcess_MultipleInputs, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    PcmBufferFormat fmt {SAMPLE_RATE_48000, CHANNEL_COUNT, LAY_OUT, SAMPLE_F32LE};
    
    AudioSuitePcmBuffer buffer1(fmt);
    AudioSuitePcmBuffer buffer2(fmt);
    AudioSuitePcmBuffer buffer3(fmt);
    
    std::vector<AudioSuitePcmBuffer *> inputs = {&buffer1, &buffer2, &buffer3};
    std::vector<AudioSuitePcmBuffer *> outPcmbuffer = node->SignalProcess(inputs);
    
    ASSERT_FALSE(outPcmbuffer.empty());
    ASSERT_NE(outPcmbuffer[0], nullptr);
}

HWTEST_F(AudioSuiteMixerTest, SignalProcess_NullInput, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> inputs = {nullptr};
    std::vector<AudioSuitePcmBuffer *> outPcmbuffer = node->SignalProcess(inputs);
    
    ASSERT_FALSE(outPcmbuffer.empty());
    EXPECT_EQ(outPcmbuffer[0], nullptr);
}

HWTEST_F(AudioSuiteMixerTest, SignalProcess_EmptyInputs, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteMixerNode>(5);
    node->Init();
    node->InitCacheLength(NEED_DATA_LENGTH);

    std::vector<AudioSuitePcmBuffer *> inputs = {};
    std::vector<AudioSuitePcmBuffer *> outPcmbuffer = node->SignalProcess(inputs);
    
    ASSERT_FALSE(outPcmbuffer.empty());
    EXPECT_EQ(outPcmbuffer[0], nullptr);
}

}  // namespace