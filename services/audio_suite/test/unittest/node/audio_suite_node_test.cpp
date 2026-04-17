/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <vector>

#include "audio_errors.h"
#include "audio_suite_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_format_conversion.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

static constexpr uint32_t NEED_DATA_LENGTH = 20;

class TestAudioNode : public AudioNode {
public:
    TestAudioNode() : AudioNode(NODE_TYPE_EQUALIZER) {}
    explicit TestAudioNode(AudioFormat audioFormat) : AudioNode(NODE_TYPE_EQUALIZER, audioFormat) {}
    ~TestAudioNode() override = default;

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

    void SetDoProcessReturn(int32_t ret)
    {
        doProcessReturn_ = ret;
    }

    int32_t DoProcessImpl(uint32_t needDataLength)
    {
        (void)needDataLength;
        return doProcessReturn_;
    }

private:
    int32_t doProcessReturn_ = SUCCESS;
};

class AudioSuiteNodeTest : public ::testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }

        node_ = std::make_shared<TestAudioNode>();
        node_->Init();
    }

    void TearDown() override
    {
        if (node_) {
            node_->DeInit();
        }
    }

    std::shared_ptr<TestAudioNode> node_;
    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE};
};

HWTEST_F(AudioSuiteNodeTest, AddNextNode_NullNode, TestSize.Level0)
{
    node_->AddNextNode(nullptr);
    EXPECT_TRUE(node_->nextNodes_.empty());
}

HWTEST_F(AudioSuiteNodeTest, AddNextNode_SingleNode, TestSize.Level0)
{
    auto nextNode = std::make_shared<TestAudioNode>();
    node_->AddNextNode(nextNode);
    EXPECT_EQ(node_->nextNodes_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, AddNextNode_DuplicateNode, TestSize.Level0)
{
    auto nextNode = std::make_shared<TestAudioNode>();
    node_->AddNextNode(nextNode);
    node_->AddNextNode(nextNode);
    EXPECT_EQ(node_->nextNodes_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, AddNextNode_MultipleNodes, TestSize.Level0)
{
    auto nextNode1 = std::make_shared<TestAudioNode>();
    auto nextNode2 = std::make_shared<TestAudioNode>();
    auto nextNode3 = std::make_shared<TestAudioNode>();
    node_->AddNextNode(nextNode1);
    node_->AddNextNode(nextNode2);
    node_->AddNextNode(nextNode3);
    EXPECT_EQ(node_->nextNodes_.size(), 3);
}

HWTEST_F(AudioSuiteNodeTest, RemoveNextNode_NullNode, TestSize.Level0)
{
    auto nextNode = std::make_shared<TestAudioNode>();
    node_->AddNextNode(nextNode);
    node_->RemoveNextNode(nullptr);
    EXPECT_EQ(node_->nextNodes_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, RemoveNextNode_ExistingNode, TestSize.Level0)
{
    auto nextNode = std::make_shared<TestAudioNode>();
    node_->AddNextNode(nextNode);
    node_->RemoveNextNode(nextNode);
    EXPECT_TRUE(node_->nextNodes_.empty());
}

HWTEST_F(AudioSuiteNodeTest, AddPreNode_NullNode, TestSize.Level0)
{
    node_->AddPreNode(nullptr);
    EXPECT_TRUE(node_->preNodes_.empty());
}

HWTEST_F(AudioSuiteNodeTest, AddPreNode_SingleNode, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    node_->AddPreNode(preNode);
    EXPECT_EQ(node_->preNodes_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, AddPreNode_DuplicateNode, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    node_->AddPreNode(preNode);
    node_->AddPreNode(preNode);
    EXPECT_EQ(node_->preNodes_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, AddPreNode_MultipleNodes, TestSize.Level0)
{
    auto preNode1 = std::make_shared<TestAudioNode>();
    auto preNode2 = std::make_shared<TestAudioNode>();
    auto preNode3 = std::make_shared<TestAudioNode>();
    node_->AddPreNode(preNode1);
    node_->AddPreNode(preNode2);
    node_->AddPreNode(preNode3);
    EXPECT_EQ(node_->preNodes_.size(), 3);
}

HWTEST_F(AudioSuiteNodeTest, RemovePreNode_NullNode, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    node_->AddPreNode(preNode);
    node_->RemovePreNode(nullptr);
    EXPECT_EQ(node_->preNodes_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, RemovePreNode_ExistingNode, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    node_->AddPreNode(preNode);
    node_->RemovePreNode(preNode);
    EXPECT_TRUE(node_->preNodes_.empty());
}

HWTEST_F(AudioSuiteNodeTest, WriteOutputData_NullData, TestSize.Level0)
{
    auto ret = node_->WriteOutputData(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteNodeTest, WriteOutputData_ValidData, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto ret = node_->WriteOutputData(buffer.get());
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node_->outputData_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, WriteOutputData_MultipleData, TestSize.Level0)
{
    auto buffer1 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto buffer2 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto buffer3 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer1.get());
    node_->WriteOutputData(buffer2.get());
    node_->WriteOutputData(buffer3.get());
    EXPECT_EQ(node_->outputData_.size(), 3);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_EmptyOutputData, TestSize.Level0)
{
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_WithValidData, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_NoFormatConversion, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_WithFormatConversion, TestSize.Level0)
{
    PcmBufferFormat outFormat = {SAMPLE_RATE_44100, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE};
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat, true, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_MultipleOutputs, TestSize.Level0)
{
    auto buffer1 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto buffer2 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer1.get());
    node_->WriteOutputData(buffer2.get());
    node_->InitFormatConverters();
    node_->formatConverters_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 2);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_OutputDataClearedAfterPull, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(node_->outputData_.empty());
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_NoPreNodes, TestSize.Level0)
{
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(node_->preNodeResult_.empty());
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_WithSinglePreNode, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    preNode->WriteOutputData(buffer.get());
    preNode->InitFormatConverters();
    node_->AddPreNode(preNode);
    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH)).Times(1);
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(node_->preNodeResult_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_WithMultiplePreNodes, TestSize.Level0)
{
    auto preNode1 = std::make_shared<TestAudioNode>();
    auto preNode2 = std::make_shared<TestAudioNode>();
    auto buffer1 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto buffer2 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    preNode1->WriteOutputData(buffer1.get());
    preNode2->WriteOutputData(buffer2.get());
    preNode1->InitFormatConverters();
    preNode2->InitFormatConverters();
    node_->AddPreNode(preNode1);
    node_->AddPreNode(preNode2);
    EXPECT_CALL(*preNode1, DoProcess(NEED_DATA_LENGTH)).Times(1);
    EXPECT_CALL(*preNode2, DoProcess(NEED_DATA_LENGTH)).Times(1);
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(node_->preNodeResult_.size(), 2);
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_PreNodeResultClearedBeforeRead, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    preNode->WriteOutputData(buffer.get());
    preNode->InitFormatConverters();
    node_->AddPreNode(preNode);
    node_->preNodeResult_.push_back(nullptr);
    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH)).Times(1);
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(node_->preNodeResult_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, InitFormatConverters_Default, TestSize.Level0)
{
    auto ret = node_->InitFormatConverters();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node_->formatConverters_.size(), 1);
    EXPECT_EQ(node_->formatConversionBuffers_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, InitFormatConverters_MultipleInit, TestSize.Level0)
{
    node_->InitFormatConverters();
    node_->formatConverters_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
    node_->formatConversionBuffers_.resize(2);
    auto ret = node_->InitFormatConverters();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node_->formatConverters_.size(), 1);
    EXPECT_EQ(node_->formatConversionBuffers_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_EmptyOutputDataWithFormatConverters, TestSize.Level0)
{
    node_->InitFormatConverters();
    node_->formatConverters_.clear();
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_FormatConverterSizeMismatch, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    node_->formatConverters_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_NullDataInOutputData, TestSize.Level0)
{
    node_->outputData_.push_back(nullptr);
    node_->InitFormatConverters();
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_NullConverter, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    node_->formatConverters_[0] = nullptr;
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_FormatConversionNeeded, TestSize.Level0)
{
    PcmBufferFormat inFormat = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_F32LE};
    PcmBufferFormat outFormat = {SAMPLE_RATE_44100, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE};
    
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(inFormat);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat, true, NEED_DATA_LENGTH);
    
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 1);
    EXPECT_NE(result[0], nullptr);
}

HWTEST_F(AudioSuiteNodeTest, PullOutputData_NoFormatConversionNeeded, TestSize.Level0)
{
    PcmBufferFormat outFormat = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_F32LE};
    
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat);
    node_->WriteOutputData(buffer.get());
    node_->InitFormatConverters();
    
    EXPECT_CALL(*node_, DoProcess(NEED_DATA_LENGTH)).Times(1);
    auto result = node_->PullOutputData(outFormat, false, NEED_DATA_LENGTH);
    
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 1);
    EXPECT_NE(result[0], nullptr);
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_ExpiredWeakPtr, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    node_->AddPreNode(preNode);
    preNode.reset();
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(node_->preNodeResult_.empty());
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_PreNodeReturnsEmpty, TestSize.Level0)
{
    auto preNode = std::make_shared<TestAudioNode>();
    preNode->InitFormatConverters();
    node_->AddPreNode(preNode);
    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH)).Times(1);
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_TRUE(node_->preNodeResult_.empty());
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_MultiplePreNodesWithExpired, TestSize.Level0)
{
    auto preNode1 = std::make_shared<TestAudioNode>();
    auto preNode2 = std::make_shared<TestAudioNode>();
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    preNode1->WriteOutputData(buffer.get());
    preNode1->InitFormatConverters();
    node_->AddPreNode(preNode1);
    node_->AddPreNode(preNode2);
    preNode2.reset();
    EXPECT_CALL(*preNode1, DoProcess(NEED_DATA_LENGTH)).Times(1);
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(node_->preNodeResult_.size(), 1);
}

HWTEST_F(AudioSuiteNodeTest, ReadPreNodeData_PreNodeResultClearedBeforeRead_MultiplePreNodes, TestSize.Level0)
{
    auto preNode1 = std::make_shared<TestAudioNode>();
    auto preNode2 = std::make_shared<TestAudioNode>();
    auto buffer1 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto buffer2 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    preNode1->WriteOutputData(buffer1.get());
    preNode2->WriteOutputData(buffer2.get());
    preNode1->InitFormatConverters();
    preNode2->InitFormatConverters();
    node_->AddPreNode(preNode1);
    node_->AddPreNode(preNode2);
    node_->preNodeResult_.push_back(nullptr);
    node_->preNodeResult_.push_back(nullptr);
    EXPECT_CALL(*preNode1, DoProcess(NEED_DATA_LENGTH)).Times(1);
    EXPECT_CALL(*preNode2, DoProcess(NEED_DATA_LENGTH)).Times(1);
    node_->ReadPreNodeData(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(node_->preNodeResult_.size(), 2);
}

}  // namespace
