/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include "audio_suite_unittest_tools.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"
#include "audio_suite_input_node.h"
#include "audio_suite_hoa_render_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_hoa_render_algo_interface_impl.h"
#include "securec.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

static constexpr uint32_t NEED_DATA_LENGTH = 20;
static constexpr uint32_t HOA_CHANNELS = 4;

class TestableHoaRenderNode : public AudioSuiteHoaRenderNode {
public:
    TestableHoaRenderNode() = default;
    ~TestableHoaRenderNode() override = default;

    std::vector<AudioSuitePcmBuffer *> PullForTest(PcmBufferFormat outFormat, bool needConvert, uint32_t needLen)
    {
        return PullOutputData(outFormat, needConvert, needLen);
    }
};

class FakeHoaPreNode : public AudioNode {
public:
    FakeHoaPreNode() : AudioNode(NODE_TYPE_INPUT) {}
    ~FakeHoaPreNode() override = default;

    MOCK_METHOD(int32_t, DoProcess, (uint32_t needDataLength), (override));

    int32_t Flush() override { return SUCCESS; }

    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) { return ERR_INVALID_PARAM; }
        AddPreNode(preNode);
        preNode->AddNextNode(GetSharedInstance());
        return SUCCESS;
    }

    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) { return ERR_INVALID_PARAM; }
        RemovePreNode(preNode);
        preNode->RemoveNextNode(GetSharedInstance());
        return SUCCESS;
    }

    void SetOutBuffer(AudioSuitePcmBuffer *buf) { outBuf_ = buf; }

    int32_t PushCurrentFrame(uint32_t needLen)
    {
        (void)needLen;
        if (outBuf_ != nullptr) {
            WriteOutputData(outBuf_);
        }
        return SUCCESS;
    }

private:
    AudioSuitePcmBuffer *outBuf_ {nullptr};
};

class AudioSuiteHoaRenderNodeTest : public testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
        node_ = std::make_shared<TestableHoaRenderNode>();
    }

    void TearDown() override
    {
        if (node_ != nullptr) {
            node_->DeInit();
            node_.reset();
        }
    }

    std::shared_ptr<TestableHoaRenderNode> node_;
    PcmBufferFormat hoaFormat_ = {SAMPLE_RATE_48000, HOA_CHANNELS, CH_LAYOUT_HOA_ORDER1_ACN_N3D, SAMPLE_S32LE};
    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S32LE};
    std::unique_ptr<AudioSuitePcmBuffer> buffer_ = std::make_unique<AudioSuitePcmBuffer>(hoaFormat_, NEED_DATA_LENGTH);
};

HWTEST_F(AudioSuiteHoaRenderNodeTest, AudioSuiteHoaRenderNodeInitTest, TestSize.Level0)
{
    int32_t ret = node_->Init();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(node_->isInit_);

    ret = node_->Init();
    EXPECT_EQ(ret, ERROR);
    node_->isInit_ = false;
}

HWTEST_F(AudioSuiteHoaRenderNodeTest, AudioSuiteHoaRenderNodeDeInitTest, TestSize.Level0)
{
    int32_t ret = node_->Init();
    EXPECT_EQ(ret, SUCCESS);

    ret = node_->DeInit();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(node_->isInit_);

    ret = node_->DeInit();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteHoaRenderNodeTest, AudioSuiteHoaRenderNodeSetOptionsTest, TestSize.Level0)
{
    node_->Init();

    AudioChannelLayout channelLayout = CH_LAYOUT_HOA_ORDER1_ACN_N3D;
    std::string layoutStr = std::to_string(static_cast<uint64_t>(channelLayout));

    int32_t ret = node_->SetOptions("hoaLayout", layoutStr);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteHoaRenderNodeTest, AudioSuiteHoaRenderNodeDoProcessFullFlowTest, TestSize.Level0)
{
    node_->Init();

    auto preNode = std::make_shared<FakeHoaPreNode>();
    preNode->InitFormatConverters();
    preNode->SetOutBuffer(buffer_.get());
    node_->Connect(preNode);

    size_t frameSizeInput = buffer_->GetDataSize();
    int32_t frameIndex = 0;
    int32_t totalFrames = 5;

    EXPECT_CALL(*preNode, DoProcess(_))
    .WillRepeatedly(::testing::Invoke([&frameIndex, &totalFrames, frameSizeInput, preNode, this]() {
        std::vector<uint8_t> zeros(frameSizeInput, 0);
        memcpy_s(buffer_->GetPcmData(), frameSizeInput, zeros.data(), frameSizeInput);

        if (frameIndex == totalFrames - 1) {
            buffer_->SetIsFinished(true);
        }

        frameIndex++;
        return preNode->PushCurrentFrame(20);
    }));

    bool isFinished = false;
    while (!isFinished) {
        std::vector<AudioSuitePcmBuffer *> result = node_->PullForTest(outFormat_, false, 20);
        if (result.empty()) {
            break;
        }
        isFinished = result[0]->GetIsFinished();
    }

    EXPECT_TRUE(node_->GetAudioNodeDataFinishedFlag());

    std::vector<AudioSuitePcmBuffer *> result = node_->PullForTest(outFormat_, false, 20);
    EXPECT_TRUE(result.empty());

    node_->DisConnect(preNode);
    buffer_->SetIsFinished(false);
    node_->Flush();
    Mock::VerifyAndClearExpectations(preNode.get());
    preNode.reset();
}

}  // namespace