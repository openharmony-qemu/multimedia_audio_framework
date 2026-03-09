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

#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include <cstring>
#include <iostream>
#include <functional>
#include <dlfcn.h>
#include "hpae_source_output_node.h"
#include "hpae_pcm_buffer.h"
#include "hpae_node_common.h"
#include "audio_errors.h"
#include "i_highpass_filter.h"
#include "audio_highpass_filter.h"

using namespace std;
using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_NODE_ID = 1243;

const std::string HIGH_PASS_FILTER_SO = "/system/lib64/libhighpass_filter.z.so";

static bool IsPcDevice()
{
    void *handle = dlopen(HIGH_PASS_FILTER_SO.c_str(), RTLD_LAZY);
    if (handle != nullptr) {
        dlclose(handle);
        return true;
    }
    return false;
}

class TestHighPassFilter : public IHighPassFilter {
public:
    TestHighPassFilter() : initSuccess_(true), applySuccess_(true), initialized_(false) {}
    ~TestHighPassFilter() override = default;

    void SetInitSuccess(bool success) { initSuccess_ = success; }
    void SetApplySuccess(bool success) { applySuccess_ = success; }
    bool IsInitialized() const { return initialized_; }

    int32_t Init(const int32_t channels) override
    {
        initialized_ = true;
        initChannels_ = channels;
        return initSuccess_ ? 0 : -1;
    }

    int32_t Apply(const std::vector<float> &samples, std::vector<float> &result) override
    {
        if (!initialized_ || !applySuccess_) {
            return -1;
        }
        applyCalledCount_++;
        lastSamplesSize_ = samples.size();
        for (size_t i = 0; i < samples.size(); ++i) {
            result[i] = samples[i] * 0.98f;
        }
        return 0;
    }

    static int32_t CreateInstance(IHighPassFilter **filter)
    {
        if (filter == nullptr) {
            return -1;
        }
        *filter = new TestHighPassFilter();
        return 0;
    }

    static void ResetCounters()
    {
        applyCalledCount_ = 0;
        lastSamplesSize_ = 0;
    }

    static uint32_t GetApplyCalledCount() { return applyCalledCount_; }
    static size_t GetLastSamplesSize() { return lastSamplesSize_; }
    static int32_t GetInitChannels() { return initChannels_; }

private:
    bool initSuccess_;
    bool applySuccess_;
    bool initialized_;
    static uint32_t applyCalledCount_;
    static size_t lastSamplesSize_;
    static int32_t initChannels_;
};

uint32_t TestHighPassFilter::applyCalledCount_ = 0;
size_t TestHighPassFilter::lastSamplesSize_ = 0;
int32_t TestHighPassFilter::initChannels_ = 0;

class HpaeSourceOutputNodeHighpassTest : public testing::Test {
public:
    void SetUp() override
    {
        TestHighPassFilter::ResetCounters();
        HpaeSourceOutputNode::SetPcEnableState(0);
    }

    void TearDown() override
    {
        TestHighPassFilter::ResetCounters();
        HpaeSourceOutputNode::SetPcEnableState(0);
    }

protected:
    static HpaeNodeInfo GetTestNodeInfo(SourceType sourceType = SOURCE_TYPE_MIC);

    static void EnablePcFilterState(bool enable)
    {
        HpaeSourceOutputNode::SetPcEnableState(enable ? 1 : 0);
    }

    static std::unique_ptr<HpaeSourceOutputNode> CreateTestNode(SourceType sourceType)
    {
        auto nodeInfo = GetTestNodeInfo(sourceType);
        return std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    }

    static void SetPcEnableState(int32_t state)
    {
        HpaeSourceOutputNode::SetPcEnableState(state);
    }

    static std::shared_ptr<HpaePcmBuffer> CreateTestPcmBuffer(uint32_t channels = STEREO,
                                                            uint32_t frameLen = DEFAULT_FRAME_LENGTH,
                                                            uint32_t sampleRate = 48000)
    {
        PcmBufferInfo pcmInfo(channels, frameLen, sampleRate);
        return std::make_shared<HpaePcmBuffer>(pcmInfo);
    }

    static std::shared_ptr<HpaePcmBuffer> CreateTestPcmBufferWithData(const std::vector<float>& data,
                                                                      uint32_t channels = STEREO,
                                                                      uint32_t frameLen = DEFAULT_FRAME_LENGTH,
                                                                      uint32_t sampleRate = 48000)
    {
        PcmBufferInfo pcmInfo(channels, frameLen, sampleRate);
        auto pcmBuffer = std::make_shared<HpaePcmBuffer>(pcmInfo);

        if (!data.empty()) {
            std::vector<float> frameData = data;
            pcmBuffer->PushFrameData(frameData);
        }

        return pcmBuffer;
    }
};

HpaeNodeInfo HpaeSourceOutputNodeHighpassTest::GetTestNodeInfo(SourceType sourceType)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sourceType = sourceType;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    return nodeInfo;
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Constructor_PcDevice_Ultrasonic_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    bool isPc = IsPcDevice();
    if (isPc) {
        EXPECT_EQ(node->isSupportHighpassFilter_, true);
        EXPECT_NE(node->highpassFilter_, nullptr);
        EXPECT_EQ(node->filterChannelCount_, STEREO);
    } else {
        EXPECT_EQ(node->isSupportHighpassFilter_, false);
        EXPECT_EQ(node->highpassFilter_, nullptr);
        EXPECT_EQ(node->filterChannelCount_, 0);
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Constructor_NonPcDevice_Mic_001, TestSize.Level1)
{
    SetPcEnableState(0);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_MIC);

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    EXPECT_EQ(node->isSupportHighpassFilter_, false);
    EXPECT_EQ(node->highpassFilter_, nullptr);
    EXPECT_EQ(node->filterChannelCount_, 0);
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Constructor_NonPcDevice_Ultrasonic_001, TestSize.Level1)
{
    SetPcEnableState(0);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    EXPECT_EQ(node->isSupportHighpassFilter_, false);
    EXPECT_EQ(node->highpassFilter_, nullptr);
    EXPECT_EQ(node->filterChannelCount_, 0);
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Constructor_PcDevice_VoiceComm_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_VOICE_COMMUNICATION);

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    EXPECT_EQ(node->isSupportHighpassFilter_, false);
    EXPECT_EQ(node->highpassFilter_, nullptr);
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Constructor_PcDevice_DifferentChannels_001, TestSize.Level1)
{
    SetPcEnableState(1);

    struct TestCase {
        AudioChannel channels;
        const char* name;
    } testCases[] = {
        {MONO, "MONO"},
        {STEREO, "STEREO"},
        {CHANNEL_4, "CHANNEL_4"},
        {static_cast<AudioChannel>(7), "7_CHANNELS"},
        {CHANNEL_8, "8_CHANNELS"}
    };

    bool isPc = IsPcDevice();
    for (const auto& testCase : testCases) {
        auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
        nodeInfo.channels = testCase.channels;

        auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);

        if (isPc) {
            EXPECT_EQ(node->isSupportHighpassFilter_, true);
            EXPECT_NE(node->highpassFilter_, nullptr);
            EXPECT_EQ(node->filterChannelCount_, static_cast<int32_t>(testCase.channels));
        } else {
            EXPECT_EQ(node->isSupportHighpassFilter_, false);
            EXPECT_EQ(node->highpassFilter_, nullptr);
            EXPECT_EQ(node->filterChannelCount_, 0);
        }
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Constructor_PcDevice_DifferentFrameLengths_001, TestSize.Level1)
{
    SetPcEnableState(1);

    std::vector<uint32_t> frameLengths = {512, 960, 1024, 2048, 4096};

    bool isPc = IsPcDevice();
    for (auto frameLen : frameLengths) {
        auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
        nodeInfo.frameLen = frameLen;

        auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);

        if (isPc) {
            EXPECT_EQ(node->isSupportHighpassFilter_, true);
            EXPECT_NE(node->highpassFilter_, nullptr);
            EXPECT_EQ(node->filterChannelCount_, STEREO);
        } else {
            EXPECT_EQ(node->isSupportHighpassFilter_, false);
            EXPECT_EQ(node->highpassFilter_, nullptr);
            EXPECT_EQ(node->filterChannelCount_, 0);
        }
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, ApplyHighpassFilter_PcDevice_NullBuffer_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    node->ApplyHighpassFilter(nullptr);
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, ApplyHighpassFilter_PcDevice_NullPcmData_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    if (node->isSupportHighpassFilter_) {
        auto pcmBuffer = CreateTestPcmBuffer();

        node->ApplyHighpassFilter(pcmBuffer.get());
        SUCCEED();
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, ApplyHighpassFilter_PcDevice_ValidData_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
    nodeInfo.frameLen = 480;

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    if (node->isSupportHighpassFilter_) {
        size_t sampleCount = node->filterChannelCount_ * 480;
        std::vector<float> pcmData(sampleCount, 0.5f);
        auto pcmBuffer = CreateTestPcmBufferWithData(pcmData,
                                                    static_cast<uint32_t>(node->filterChannelCount_),
                                                    480);

        node->ApplyHighpassFilter(pcmBuffer.get());
        SUCCEED();
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, ApplyHighpassFilter_PcDevice_LargeData_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
    nodeInfo.frameLen = 960;

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    if (node->isSupportHighpassFilter_) {
        size_t sampleCount = 960 * STEREO;
        std::vector<float> pcmData(sampleCount, 0.3f);
        auto pcmBuffer = CreateTestPcmBufferWithData(pcmData, STEREO, 960);

        node->ApplyHighpassFilter(pcmBuffer.get());
        SUCCEED();
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, DoProcess_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, DoProcess_NonPcDevice_NoFilter_001, TestSize.Level1)
{
    SetPcEnableState(0);
    auto node = CreateTestNode(SOURCE_TYPE_MIC);
    ASSERT_NE(node, nullptr);

    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, DoProcess_PcDevice_WithMute_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    node->SetMute(true);
    EXPECT_EQ(node->isMute_, true);

    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, SetState_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    node->SetState(HPAE_SESSION_PREPARED);
    EXPECT_EQ(node->GetState(), HPAE_SESSION_PREPARED);

    node->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(node->GetState(), HPAE_SESSION_RUNNING);
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, SetAppUid_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    int32_t testUid = 1001;
    node->SetAppUid(testUid);
    EXPECT_EQ(node->GetAppUid(), testUid);
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Destructor_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    bool isPc = IsPcDevice();
    {
        auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
        ASSERT_NE(node, nullptr);
        if (isPc) {
            ASSERT_NE(node->highpassFilter_, nullptr);
        } else {
            ASSERT_EQ(node->highpassFilter_, nullptr);
        }
    }
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, MultipleNodes_PcDevice_WithFilters_001, TestSize.Level1)
{
    SetPcEnableState(1);
    bool isPc = IsPcDevice();

    std::vector<std::unique_ptr<HpaeSourceOutputNode>> nodes;

    for (int i = 0; i < 3; ++i) {
        auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
        nodeInfo.nodeId = DEFAULT_NODE_ID + i;

        auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);
        if (isPc) {
            EXPECT_EQ(node->isSupportHighpassFilter_, true);
            EXPECT_NE(node->highpassFilter_, nullptr);
        } else {
            EXPECT_EQ(node->isSupportHighpassFilter_, false);
            EXPECT_EQ(node->highpassFilter_, nullptr);
        }

        nodes.push_back(std::move(node));
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, Reset_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    bool result = node->Reset();
    EXPECT_TRUE(result);

    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, ResetAll_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    bool result = node->ResetAll();
    EXPECT_TRUE(result);

    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, StateTransitions_PcDevice_WithFilter_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
    ASSERT_NE(node, nullptr);

    HpaeSessionState states[] = {
        HPAE_SESSION_NEW,
        HPAE_SESSION_PREPARED,
        HPAE_SESSION_RUNNING,
        HPAE_SESSION_PAUSED,
        HPAE_SESSION_STOPPED
    };

    for (auto state : states) {
        node->SetState(state);
        EXPECT_EQ(node->GetState(), state);
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, EdgeCase_PcDevice_ZeroFrameLength_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
    nodeInfo.frameLen = 0;

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    if (node->isSupportHighpassFilter_) {
        auto pcmBuffer = CreateTestPcmBuffer(STEREO, 0);

        node->ApplyHighpassFilter(pcmBuffer.get());
        SUCCEED();
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, EdgeCase_PcDevice_OneFrame_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
    nodeInfo.frameLen = 1;

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    if (node->isSupportHighpassFilter_) {
        std::vector<float> testData = {0.5f};
        auto pcmBuffer = CreateTestPcmBufferWithData(testData, STEREO, 1);

        node->ApplyHighpassFilter(pcmBuffer.get());
        SUCCEED();
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, EdgeCase_PcDevice_MonoChannel_001, TestSize.Level1)
{
    SetPcEnableState(1);
    auto nodeInfo = GetTestNodeInfo(SOURCE_TYPE_ULTRASONIC);
    nodeInfo.channels = MONO;

    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    bool isPc = IsPcDevice();
    if (isPc && node->isSupportHighpassFilter_) {
        EXPECT_EQ(node->filterChannelCount_, static_cast<int32_t>(MONO));
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, SourceTypeCombination_001, TestSize.Level1)
{
    SourceType testTypes[] = {
        SOURCE_TYPE_MIC,
        SOURCE_TYPE_ULTRASONIC,
        SOURCE_TYPE_VOICE_COMMUNICATION,
        SOURCE_TYPE_VOICE_RECOGNITION,
        SOURCE_TYPE_PLAYBACK_CAPTURE
    };

    bool isPc = IsPcDevice();
    for (auto sourceType : testTypes) {
        if (sourceType == SOURCE_TYPE_ULTRASONIC) {
            SetPcEnableState(1);
        } else {
            SetPcEnableState(0);
        }

        auto nodeInfo = GetTestNodeInfo(sourceType);
        auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);

        bool expectedSupport = (isPc && sourceType == SOURCE_TYPE_ULTRASONIC && PC_ENABLE_STATE != 0);
        EXPECT_EQ(node->isSupportHighpassFilter_, expectedSupport);

        if (expectedSupport) {
            EXPECT_NE(node->highpassFilter_, nullptr);
        } else {
            EXPECT_EQ(node->highpassFilter_, nullptr);
        }
    }
}

HWTEST_F(HpaeSourceOutputNodeHighpassTest, MemoryAllocation_PcDevice_001, TestSize.Level1)
{
    SetPcEnableState(1);
    bool isPc = IsPcDevice();

    for (int i = 0; i < 100; ++i) {
        auto node = CreateTestNode(SOURCE_TYPE_ULTRASONIC);
        ASSERT_NE(node, nullptr);
        if (isPc) {
            ASSERT_EQ(node->isSupportHighpassFilter_, true);
            ASSERT_NE(node->highpassFilter_, nullptr);
        } else {
            ASSERT_EQ(node->isSupportHighpassFilter_, false);
            ASSERT_EQ(node->highpassFilter_, nullptr);
        }
    }
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS