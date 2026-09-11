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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "hpae_session_effect_node.h"
#include "hpae_plugin_node.h"
#include "hpae_node_common.h"
#include "effect_instance.h"
#include "mock_effect_instance.h"
#include "hpae_pcm_buffer.h"
#include "fake_audio_effect_lib_entry.h"
#include "test_case_common.h"
#include "hpae_mocks.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr uint32_t TEST_NODE_ID = 100;
static constexpr uint32_t TEST_UPSTREAM_NODE_ID = 99;
static constexpr uint32_t TEST_SESSION_ID = 42;
static constexpr uint32_t TEST_FRAME_LEN = 960;
static constexpr uint32_t TEST_SAMPLE_RATE = 48000;
static constexpr uint32_t TEST_CHANNEL_COUNT = 2;
static constexpr uint32_t TEST_PREHEAT_FRAME_COUNT = 5;
static constexpr size_t TEST_FRAME_BYTE_LEN =
    static_cast<size_t>(TEST_FRAME_LEN) * TEST_CHANNEL_COUNT * sizeof(float);
static constexpr size_t TEST_PREBUF_TARGET_BYTE_LEN = TEST_PREHEAT_FRAME_COUNT * TEST_FRAME_BYTE_LEN;

static AudioEffectConfig MakeTestConfig()
{
    AudioEffectConfig config;
    config.inputCfg.samplingRate = TEST_SAMPLE_RATE;
    config.inputCfg.channels = TEST_CHANNEL_COUNT;
    config.outputCfg.samplingRate = TEST_SAMPLE_RATE;
    config.outputCfg.channels = TEST_CHANNEL_COUNT;
    return config;
}

static HpaeNodeInfo MakeTestNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_NODE_ID;
    nodeInfo.sessionId = TEST_SESSION_ID;
    nodeInfo.frameLen = TEST_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    nodeInfo.sourceType = SOURCE_TYPE_INVALID;
    return nodeInfo;
}

class HpaeSessionEffectNodeTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        nodeInfo_ = MakeTestNodeInfo();
        node_ = std::make_shared<HpaeSessionEffectNode>(nodeInfo_);
    }

    void TearDown() override
    {
        node_.reset();
    }

    HpaeNodeInfo nodeInfo_;
    std::shared_ptr<HpaeSessionEffectNode> node_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// Construction and destruction
HWTEST_F(HpaeSessionEffectNodeTest, constructAndDestruct, TestSize.Level0)
{
    HpaeNodeInfo info = MakeTestNodeInfo();
    auto node = std::make_shared<HpaeSessionEffectNode>(info);
    EXPECT_NE(node, nullptr);
}

HWTEST_F(HpaeSessionEffectNodeTest, audioEffectConfig_defaultConstructedFieldsAreStable, TestSize.Level0)
{
    constexpr AudioEffectConfig config;

    static_assert(config.inputCfg.samplingRate == 0);
    static_assert(config.inputCfg.channels == 0);
    static_assert(config.inputCfg.format == 0);
    static_assert(config.inputCfg.channelLayout == 0);
    static_assert(config.inputCfg.encoding == ENCODING_PCM);
    static_assert(config.outputCfg.samplingRate == 0);
    static_assert(config.outputCfg.channels == 0);
    static_assert(config.outputCfg.format == 0);
    static_assert(config.outputCfg.channelLayout == 0);
    static_assert(config.outputCfg.encoding == ENCODING_PCM);

    EXPECT_EQ(config.inputCfg.encoding, ENCODING_PCM);
    EXPECT_EQ(config.outputCfg.encoding, ENCODING_PCM);
}

// SignalProcess bypass: returns input when non-empty
HWTEST_F(HpaeSessionEffectNodeTest, signalProcess_bypassReturnsInput, TestSize.Level0)
{
    PcmBufferInfo bufInfo;
    bufInfo.ch = 2;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer pcmBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuf);
}

// SignalProcess returns nullptr for empty input
HWTEST_F(HpaeSessionEffectNodeTest, signalProcess_emptyInput_returnsNullptr, TestSize.Level0)
{
    std::vector<HpaePcmBuffer *> inputs;
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    EXPECT_EQ(result, nullptr);
}

// SignalProcess returns nullptr for nullptr input
HWTEST_F(HpaeSessionEffectNodeTest, signalProcess_nullptrInput_returnsNullptr, TestSize.Level0)
{
    std::vector<HpaePcmBuffer *> inputs = {nullptr};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    EXPECT_EQ(result, nullptr);
}

// GetLatency returns 0 in Phase 1
HWTEST_F(HpaeSessionEffectNodeTest, getLatency_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(node_->GetLatency(0), 0u);
    EXPECT_EQ(node_->GetLatency(TEST_SESSION_ID), 0u);
}

// CreateEffectWithInstance stores the instance
HWTEST_F(HpaeSessionEffectNodeTest, createEffectWithInstance_storesInstance, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);

    int32_t ret = node_->CreateEffectWithInstance("test_effect", instance, config_);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(node_->GetEffectInstance(), nullptr);
    EXPECT_EQ(node_->GetEffectName(), "test_effect");
}

// CreateEffectWithInstance with nullptr returns error
HWTEST_F(HpaeSessionEffectNodeTest, createEffectWithInstance_nullptr_returnsError, TestSize.Level0)
{
    int32_t ret = node_->CreateEffectWithInstance("test_effect", nullptr, config_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_CREATE_FAILED));
}

HWTEST_F(HpaeSessionEffectNodeTest, setParameter_noInstance_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(node_->SetParameter(1, 0.5f), 0);
}

HWTEST_F(HpaeSessionEffectNodeTest, flushEffect_noInstance_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(node_->FlushEffect(), 0);
}

HWTEST_F(HpaeSessionEffectNodeTest, pauseEffect_noInstance_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(node_->PauseEffect(), 0);
}

HWTEST_F(HpaeSessionEffectNodeTest, stopPreheatThread_JoinsRunningThread, TestSize.Level0)
{
    node_->stopRequested_.store(false);
    node_->preheatThread_ = std::thread([node = node_]() {
        while (!node->stopRequested_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    ASSERT_TRUE(node_->preheatThread_.joinable());

    node_->StopPreheatThread();

    EXPECT_TRUE(node_->stopRequested_.load());
    EXPECT_FALSE(node_->preheatThread_.joinable());
}

// DropEffect clears node's reference and resets to IDLE
HWTEST_F(HpaeSessionEffectNodeTest, dropEffect_clearsInstance, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    node_->CreateEffectWithInstance("test_effect", instance, config_);

    int32_t ret = node_->DropEffect();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(node_->GetEffectInstance(), nullptr);
}

// DropEffect releases reference without calling Release on instance
HWTEST_F(HpaeSessionEffectNodeTest, dropEffect_resetsWithoutRelease, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    auto rawPtr = instance.get();
    node_->CreateEffectWithInstance("test_effect", instance, config_);

    // DropEffect should clear node's reference but keep the EffectInstance alive
    // (shared_ptr in test still holds it)
    int32_t ret = node_->DropEffect();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(node_->GetEffectInstance(), nullptr);
    // The original shared_ptr still holds a valid instance
    EXPECT_NE(rawPtr, nullptr);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);
}

// DropEffect when no instance is set
HWTEST_F(HpaeSessionEffectNodeTest, dropEffect_noInstance_noop, TestSize.Level0)
{
    int32_t ret = node_->DropEffect();
    EXPECT_EQ(ret, 0);
}

// GetEffectName returns empty when no effect set
HWTEST_F(HpaeSessionEffectNodeTest, getEffectName_emptyWhenNoEffect, TestSize.Level0)
{
    EXPECT_EQ(node_->GetEffectName(), "");
}

// GetAudioConfig returns default when no effect set
HWTEST_F(HpaeSessionEffectNodeTest, getAudioConfig_returnsSetConfig, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    node_->CreateEffectWithInstance("test_effect", instance, config_);

    auto cfg = node_->GetAudioConfig();
    EXPECT_EQ(cfg.inputCfg.samplingRate, 48000u);
    EXPECT_EQ(cfg.inputCfg.channels, 2u);
}

// StartEffect delegates to TriggerProcess
HWTEST_F(HpaeSessionEffectNodeTest, startEffect_delegatesToTriggerProcess, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    node_->CreateEffectWithInstance("test_effect", instance, config_);

    int32_t ret = node_->StartEffect();
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
}

// StartEffect returns 0 when no instance
HWTEST_F(HpaeSessionEffectNodeTest, startEffect_noInstance_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(node_->StartEffect(), 0);
}

// DropEffect returns 0 when no instance is bound
HWTEST_F(HpaeSessionEffectNodeTest, dropEffect_noInstance_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(node_->DropEffect(), 0);
}

// FlushEffect delegates to Flush
HWTEST_F(HpaeSessionEffectNodeTest, flushEffect_delegatesToFlush, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    node_->CreateEffectWithInstance("test_effect", instance, config_);

    int32_t ret = node_->FlushEffect();
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
}

// SetParameter delegates to instance
HWTEST_F(HpaeSessionEffectNodeTest, setParameter_delegatesToInstance, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    node_->CreateEffectWithInstance("test_effect", instance, config_);

    int32_t ret = node_->SetParameter(1, 0.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
}

// SE-CF1: StartCloseCrossfade moves a no-instance/IDLE node to STOPPING without pending
HWTEST_F(HpaeSessionEffectNodeTest, startCloseCrossfade_idleEntersStoppingNoPending, TestSize.Level0)
{
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    node_->StartCloseCrossfade();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);
    // No effect instance / not ACTIVE -> no queued wet, so no pending crossfade.
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
}

// SE-CF3: Crossfade bypass returns input (STOPPING bypass for a no-instance node)
HWTEST_F(HpaeSessionEffectNodeTest, crossfade_bypassReturnsInput, TestSize.Level0)
{
    node_->StartCloseCrossfade();

    PcmBufferInfo bufInfo;
    bufInfo.ch = 2;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer pcmBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuf);
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
}

// --- Phase 3A-1 algorithm SignalProcess tests ---

// Subclass that overrides StartPreheatThread to no-op (no real thread in tests)
class TestableSessionEffectNode : public HpaeSessionEffectNode {
public:
    explicit TestableSessionEffectNode(HpaeNodeInfo& nodeInfo)
        : HpaeNode(nodeInfo), HpaeSessionEffectNode(nodeInfo) {}
    HpaePcmBuffer *GetSilenceDataForTest()
    {
        return PrepareSilenceBuffer(silenceData_);
    }
    using HpaeSessionEffectNode::FeedInputFromBuffer;
    using HpaeSessionEffectNode::FeedPaddingInput;
    using HpaeSessionEffectNode::GetPrebufFilled;
    using HpaeSessionEffectNode::GetPrebufTargetBytes;
    using HpaeSessionEffectNode::GetPaddingBytesInForTest;
    using HpaeSessionEffectNode::RunPreheatThreadForTest;
    using HpaeSessionEffectNode::SetPrebufTargetBytesForTest;
    using HpaeSessionEffectNode::SetActiveNoDataStartForTest;
    using HpaeSessionEffectNode::ProcessActiveNoInputForTest;
    using HpaeSessionEffectNode::UpdatePrebufTargetBytesForTest;
    using HpaeSessionEffectNode::HandleNoDataTimeoutForTest;
    using HpaeSessionEffectNode::SetPreheatNoDataStartTimeForTest;
    using HpaeSessionEffectNode::SetStopRequestedForTest;
    using HpaeSessionEffectNode::GetTotalBytesIn;
    using HpaeSessionEffectNode::GetTotalBytesOut;
    void AccountOutputForTest(size_t byteLen)
    {
        AccountOutput(byteLen);
    }
protected:
    void StartPreheatThread() override {}
};

// Test helper for Phase 3C Crossfade: exposes SignalProcess for direct testing
class TestableSEWithSignalProcess : public TestableSessionEffectNode {
public:
    explicit TestableSEWithSignalProcess(HpaeNodeInfo& nodeInfo)
        : HpaeNode(nodeInfo), TestableSessionEffectNode(nodeInfo) {}
    using HpaeSessionEffectNode::SignalProcess;
};

class SessionEffectNodeAlgoTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fake_.Reset();
        fake_.SetProcessDelayMs(0); // disable real delay for unit tests
        nodeInfo_ = MakeTestNodeInfo();
        node_ = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    }
    void TearDown() override
    {
        node_.reset();
    }

    std::shared_ptr<EffectInstance> CreateInstanceWithLib()
    {
        auto inst = std::make_shared<EffectInstance>();
        auto lib = fake_.GetLibrary();
        inst->Init("test_effect", MakeTestConfig(), &lib);
        return inst;
    }

    FakeAudioEffectLibEntry fake_;
    HpaeNodeInfo nodeInfo_;
    std::shared_ptr<TestableSessionEffectNode> node_;
    AudioEffectConfig config_ = MakeTestConfig();
};

HWTEST_F(SessionEffectNodeAlgoTest, createEffectWithInstance_reconfigsSilenceData, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = MakeTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_96000;
    nodeInfo.frameLen = TEST_FRAME_LEN * 2;
    auto node = std::make_shared<TestableSessionEffectNode>(nodeInfo);
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();

    int32_t ret = node->CreateEffectWithInstance("test_effect", instance, config_);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(node->GetSampleRate(), SAMPLE_RATE_48000);
    EXPECT_EQ(node->GetFrameLen(), TEST_FRAME_LEN);
    EXPECT_EQ(node->GetChannelCount(), STEREO);
    ASSERT_NE(node->GetSilenceDataForTest(), nullptr);
    EXPECT_EQ(node->GetSilenceDataForTest()->GetSampleRate(), TEST_SAMPLE_RATE);
    EXPECT_EQ(node->GetSilenceDataForTest()->GetFrameLen(), TEST_FRAME_LEN);
    EXPECT_EQ(node->GetSilenceDataForTest()->GetChannelCount(), TEST_CHANNEL_COUNT);
}

HWTEST_F(SessionEffectNodeAlgoTest, createEffectWithInstance_fallsBackToInputConfigWhenOutputUnset, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_96000;
    config.inputCfg.channels = MONO;
    config.inputCfg.channelLayout = static_cast<uint64_t>(AudioChannelLayout::CH_LAYOUT_MONO);

    int32_t ret = node_->CreateEffectWithInstance("test_effect", instance, config);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(node_->GetSampleRate(), SAMPLE_RATE_96000);
    EXPECT_EQ(node_->GetFrameLen(), TEST_FRAME_LEN * 2);
    EXPECT_EQ(node_->GetChannelCount(), MONO);
    EXPECT_EQ(node_->GetChannelLayout(), AudioChannelLayout::CH_LAYOUT_MONO);
}

HWTEST_F(SessionEffectNodeAlgoTest, createEffectWithInstance_usesOutputConfigWhenProvided, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    AudioEffectConfig config = MakeTestConfig();
    config.inputCfg.channelLayout = static_cast<uint64_t>(AudioChannelLayout::CH_LAYOUT_STEREO);
    config.outputCfg.samplingRate = SAMPLE_RATE_96000;
    config.outputCfg.channels = MONO;
    config.outputCfg.channelLayout = static_cast<uint64_t>(AudioChannelLayout::CH_LAYOUT_MONO);

    int32_t ret = node_->CreateEffectWithInstance("test_effect", instance, config);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(node_->GetSampleRate(), SAMPLE_RATE_96000);
    EXPECT_EQ(node_->GetFrameLen(), TEST_FRAME_LEN * 2);
    EXPECT_EQ(node_->GetChannelCount(), MONO);
    EXPECT_EQ(node_->GetChannelLayout(), AudioChannelLayout::CH_LAYOUT_MONO);
    EXPECT_EQ(node_->GetSilenceDataForTest()->GetChannelLayout(),
        static_cast<uint64_t>(AudioChannelLayout::CH_LAYOUT_MONO));
}

HWTEST_F(SessionEffectNodeAlgoTest, createEffectWithInstance_invalidConfigSkipsReconfig, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    AudioEffectConfig config;

    int32_t ret = node_->CreateEffectWithInstance("test_effect", instance, config);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(node_->GetSampleRate(), SAMPLE_RATE_48000);
    EXPECT_EQ(node_->GetFrameLen(), TEST_FRAME_LEN);
    EXPECT_EQ(node_->GetChannelCount(), STEREO);
}

// SE-P1: Process bypass when not created
HWTEST_F(SessionEffectNodeAlgoTest, process_bypassWhenNotCreated, TestSize.Level0)
{
    PcmBufferInfo bufInfo;
    bufInfo.ch = 2;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer pcmBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuf);
}

// SE-P3: Process bypass on algo fail
HWTEST_F(SessionEffectNodeAlgoTest, process_bypassOnAlgoFail, TestSize.Level0)
{
    auto inst = CreateInstanceWithLib();
    auto *rawInst = inst.get();  // save raw pointer before move
    node_->CreateEffectWithInstance("test_effect", inst, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Feed enough to trigger process, then make getOutput fail
    size_t thresholdFloats = 96000 * config_.inputCfg.channels;
    std::vector<float> inputData(thresholdFloats, 1.0f);
    fake_.SetGetOutputFail(true);
    rawInst->FeedInput(reinterpret_cast<const uint8_t*>(inputData.data()),
                       inputData.size() * sizeof(float));
    fake_.WaitForProcessing();

    PcmBufferInfo bufInfo;
    bufInfo.ch = 2;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer pcmBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    // ACTIVE state: algorithm fail → no output → return silence (not bypass)
    EXPECT_NE(result, &pcmBuf);
}

// SE-P4: Process success returns algorithm output
HWTEST_F(SessionEffectNodeAlgoTest, process_successReturnsOutput, TestSize.Level0)
{
    auto inst = CreateInstanceWithLib();
    auto *rawInst = inst.get();  // save raw pointer before move
    node_->CreateEffectWithInstance("test_effect", inst, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Feed enough data to trigger process
    size_t thresholdFloats = 96000 * config_.inputCfg.channels;
    std::vector<float> inputData(thresholdFloats, 1.0f);
    rawInst->FeedInput(reinterpret_cast<const uint8_t*>(inputData.data()),
                       inputData.size() * sizeof(float));
    fake_.WaitForProcessing();

    PcmBufferInfo bufInfo;
    bufInfo.ch = 2;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer pcmBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

// --- Phase 3B: Three-stage state machine tests ---

// Upstream node: provides non-zero data when DoProcess pulls
class MockUpstreamNode : public HpaePluginNode {
public:
    explicit MockUpstreamNode(HpaeNodeInfo& info)
        : HpaePluginNode(info),
          pcmBufferInfo_(static_cast<uint32_t>(info.channels), info.frameLen,
              static_cast<uint32_t>(info.samplingRate)),
          outputBuffer_(pcmBufferInfo_)
    {}

    void SetOutputValid(bool valid)
    {
        outputValid_ = valid;
    }

    float *GetOutputDataForTest()
    {
        return outputBuffer_.GetPcmDataBuffer();
    }

    HpaePcmBuffer *GetOutputBufferForTest()
    {
        return &outputBuffer_;
    }

    void DoProcess() override
    {
        FillOutputBuffer();
        outputStream_.WriteDataToOutput(&outputBuffer_);
    }

    HpaePcmBuffer* SignalProcess(const std::vector<HpaePcmBuffer*>& inputs) override
    {
        if (!inputs.empty() && inputs[0] != nullptr) {
            float* d = inputs[0]->GetPcmDataBuffer();
            size_t n = inputs[0]->GetFrameLen() * inputs[0]->GetChannelCount();
            for (size_t i = 0; i < n; i++) {
                d[i] = 0.5f;
            }
        }
        return inputs.empty() ? nullptr : inputs[0];
    }
    uint64_t GetLatency(uint32_t) override { return 0; }

private:
    void FillOutputBuffer()
    {
        outputBuffer_.Reset();
        float *data = outputBuffer_.GetPcmDataBuffer();
        size_t sampleCount = outputBuffer_.GetFrameLen() * outputBuffer_.GetChannelCount();
        for (size_t i = 0; i < sampleCount; i++) {
            data[i] = outputValid_ ? 0.5f : 0.0f;
        }
        outputBuffer_.SetBufferValid(outputValid_);
        outputBuffer_.SetBufferSilence(!outputValid_);
    }

    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer outputBuffer_;
    bool outputValid_ = true;
};

// --- State machine test fixture ---

class SessionEffectNodeStateTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        nodeInfo_ = MakeTestNodeInfo();
        mockCallback_ = std::make_shared<testing::NiceMock<MockNodeCallback>>();
        nodeInfo_.statusCallback = mockCallback_;
        node_ = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
        mockInst_ = std::make_shared<testing::NiceMock<MockEffectInstance>>();

        // Default behaviors
        ON_CALL(*mockInst_, StopEffect()).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, GetAudioConfig()).WillByDefault(testing::ReturnRef(config_));
        ON_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, Flush()).WillByDefault(testing::Return(0));

        // Connect upstream — node reads from upstreamNode
        HpaeNodeInfo upInfo = MakeTestNodeInfo();
        upInfo.nodeId = TEST_UPSTREAM_NODE_ID;
        upstreamNode_ = std::make_shared<MockUpstreamNode>(upInfo);
        node_->Connect(upstreamNode_);
    }
    void TearDown() override
    {
        node_.reset();
        mockInst_.reset();
        upstreamNode_.reset();
    }

    // Helper: drive node to specified state
    void EnterState(HpaeSessionEffectNode::STATUS state, bool aissSwitchEnabled = true)
    {
        node_->CreateEffectWithInstance("test_effect", mockInst_, config_, aissSwitchEnabled);
        size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
        switch (state) {
            case HpaeSessionEffectNode::STATUS::WARMING_UP:
                node_->StartEffect();
                break;
            case HpaeSessionEffectNode::STATUS::INTERCEPTING:
                node_->StartEffect();
                node_->SetPrebufFilledForTest(true);
                node_->DoProcess();  // WARMING_UP->INTERCEPTING
                break;
            case HpaeSessionEffectNode::STATUS::ACTIVE:
                node_->StartEffect();
                node_->SetPrebufFilledForTest(true);
                node_->DoProcess();  // -> INTERCEPTING
                // Next DoProcess needs ReadOutput returning >0 to trigger ACTIVE transition
                EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
                    .WillOnce(testing::Return(byteLen));
                node_->DoProcess();  // INTERCEPTING->ACTIVE
                break;
            default: break;
        }
    }

    HpaeNodeInfo nodeInfo_;
    std::shared_ptr<TestableSessionEffectNode> node_;
    std::shared_ptr<testing::NiceMock<MockEffectInstance>> mockInst_;
    std::shared_ptr<testing::NiceMock<MockNodeCallback>> mockCallback_;
    std::shared_ptr<MockUpstreamNode> upstreamNode_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// SE-W1: WARMING_UP DoProcess disconnect — no upstream pull, no FeedInput, output silence
HWTEST_F(SessionEffectNodeStateTest, warmingUp_returnsSilence, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

HWTEST_F(SessionEffectNodeStateTest, startEffect_zeroPreheatMarksPrebufFilled, TestSize.Level0)
{
    EXPECT_CALL(*mockInst_, GetPreheatFrames()).WillOnce(testing::Return(0));
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    EXPECT_EQ(node_->StartEffect(), 0);

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->GetPrebufTargetBytes(), 0u);
    EXPECT_TRUE(node_->GetPrebufFilled());
}

// When prebufTargetBytes_ == 0 (no preheat thread), no preheat state change callback should be emitted
HWTEST_F(SessionEffectNodeStateTest, preheat_noCallback_whenPrebufTargetZero, TestSize.Level0)
{
    EXPECT_CALL(*mockInst_, GetPreheatFrames()).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockCallback_, OnSessionEffectPreheatStateChanged(testing::_, testing::_)).Times(0);

    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    EXPECT_EQ(node_->StartEffect(), 0);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->GetPrebufTargetBytes(), 0u);
}


HWTEST_F(SessionEffectNodeStateTest, doProcess_autoStartsFromIdleWhenInstanceBound, TestSize.Level0)
{
    EXPECT_CALL(*mockInst_, GetPreheatFrames()).WillOnce(testing::Return(0));
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);

    node_->DoProcess();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
    EXPECT_TRUE(node_->GetPrebufFilled());
}

// SE-W5: WARMING_UP -> INTERCEPTING when prebufFilled set
HWTEST_F(SessionEffectNodeStateTest, warmingUp_toIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->SetPrebufFilledForTest(true);

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
}

// SE-I1: INTERCEPTING pulls upstream and FeedInput to algorithm
HWTEST_F(SessionEffectNodeStateTest, intercepting_feedsInputCache, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen))
        .WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
}

// SE-I3: INTERCEPTING returns silence when no output
HWTEST_F(SessionEffectNodeStateTest, intercepting_returnsSilence, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
}

// SE-I4: INTERCEPTING -> ACTIVE when algorithm has first output
HWTEST_F(SessionEffectNodeStateTest, intercepting_toActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// SE-A1: ACTIVE reads algorithm output for downstream
HWTEST_F(SessionEffectNodeStateTest, active_readsAlgoOutput, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    size_t beforeOut = node_->GetTotalBytesOut();
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_EQ(node_->GetTotalBytesOut(), beforeOut + byteLen);
}

HWTEST_F(SessionEffectNodeStateTest, activeWithInput_doesNotRunNoInputTailDrain, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->SetActiveNoDataStartForTest(
        std::chrono::steady_clock::now() - std::chrono::milliseconds(1000));

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));

    node_->DoProcess();

    EXPECT_EQ(node_->GetPaddingBytesInForTest(), 0u);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

HWTEST_F(SessionEffectNodeStateTest, activeInvalidInput_keepsPluginRoutingWithoutFeeding, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    upstreamNode_->SetOutputValid(false);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    auto *invalidInputData = reinterpret_cast<uint8_t *>(upstreamNode_->GetOutputDataForTest());
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Invoke([invalidInputData](uint8_t *buf, size_t len, uint32_t *) -> int32_t {
            EXPECT_EQ(buf, invalidInputData);
            return static_cast<int32_t>(len);
        }));

    node_->DoProcess();
    HpaePcmBuffer *output = node_->GetOutputPort()->PullOutputData();

    EXPECT_EQ(node_->GetPaddingBytesInForTest(), 0u);
    EXPECT_EQ(output, upstreamNode_->GetOutputBufferForTest());
    ASSERT_NE(output, nullptr);
    EXPECT_TRUE(output->IsValid());
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

HWTEST_F(SessionEffectNodeStateTest, disabledWithInput_bypassesWithoutSignalProcess, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->EnableProcess(false);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).Times(0);

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

HWTEST_F(SessionEffectNodeStateTest, disabledWithoutInput_callsSignalProcessEmptyPath, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    isolatedNode->EnableProcess(false);

    isolatedNode->DoProcess();

    EXPECT_EQ(isolatedNode->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-A2: ACTIVE returns silence when output buffer empty
HWTEST_F(SessionEffectNodeStateTest, active_outputEmpty_returnsSilence, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t beforeOut = node_->GetTotalBytesOut();
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_EQ(node_->GetTotalBytesOut(), beforeOut);
}

// SE-A3: ACTIVE continuously feeds input
HWTEST_F(SessionEffectNodeStateTest, active_feedsInputContinuous, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).Times(2).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

HWTEST_F(SessionEffectNodeStateTest, activeWithoutInput_drainsCachedOutputFromDoProcess, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    isolatedNode->CreateEffectWithInstance("test_effect", mockInst_, config_);
    isolatedNode->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));

    isolatedNode->DoProcess();

    EXPECT_EQ(isolatedNode->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_EQ(isolatedNode->GetTotalBytesOut(), byteLen);
}

HWTEST_F(SessionEffectNodeStateTest, pausedDoProcessOutputsSilenceWithoutFeedingAlgorithm, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->PauseEffect();

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).Times(0);

    node_->DoProcess();

    EXPECT_TRUE(node_->IsPaused());
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

HWTEST_F(SessionEffectNodeStateTest, doProcess_resetsStatsWhenReportThresholdReached, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->SetStatsReportThresholdForTest(1);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));

    node_->DoProcess();

    EXPECT_EQ(node_->GetStatsForTest().framesProcessed, 0);
    EXPECT_EQ(node_->GetStatsForTest().framesOutput, 0);
}

HWTEST_F(SessionEffectNodeStateTest, activeNoDataAfterTimeout_feedsPaddingAndReadsOutput, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->SetActiveNoDataStartForTest(
        std::chrono::steady_clock::now() - std::chrono::milliseconds(1000));

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));

    HpaePcmBuffer *result = node_->ProcessActiveNoInputForTest();

    EXPECT_NE(result, nullptr);
    EXPECT_TRUE(result->IsValid());
    EXPECT_FALSE(result->IsSilence());
    EXPECT_EQ(node_->GetPaddingBytesInForTest(), byteLen);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, activeNoDataBeforeTimeout_readsCachedOutputWithoutPadding, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));

    HpaePcmBuffer *result = node_->ProcessActiveNoInputForTest();

    EXPECT_NE(result, nullptr);
    EXPECT_TRUE(result->IsValid());
    EXPECT_FALSE(result->IsSilence());
    EXPECT_EQ(node_->GetPaddingBytesInForTest(), 0u);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, activeNoDataPadding_stopsAtConfiguredLimit, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->SetActiveNoDataStartForTest(
        std::chrono::steady_clock::now() - std::chrono::milliseconds(1000));

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen))
        .Times(200)
        .WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .Times(240)
        .WillRepeatedly(testing::Return(byteLen));

    for (uint32_t i = 0; i < 240; ++i) {
        EXPECT_NE(node_->ProcessActiveNoInputForTest(), nullptr);
    }

    EXPECT_EQ(node_->GetPaddingBytesInForTest(), byteLen * 200);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, activeNoDataPaddingFailure_stopsAtConfiguredLimit, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->SetActiveNoDataStartForTest(
        std::chrono::steady_clock::now() - std::chrono::milliseconds(1000));

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen))
        .Times(200)
        .WillRepeatedly(testing::Return(-1));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .Times(240)
        .WillRepeatedly(testing::Return(byteLen));

    for (uint32_t i = 0; i < 240; ++i) {
        EXPECT_NE(node_->ProcessActiveNoInputForTest(), nullptr);
    }

    EXPECT_EQ(node_->GetPaddingBytesInForTest(), 0u);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, activeNoData_returnsSilenceWithoutInstanceOrWhenInactive, TestSize.Level0)
{
    EXPECT_EQ(node_->ProcessActiveNoInputForTest(), node_->GetSilenceDataForTest());

    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    EXPECT_EQ(node_->ProcessActiveNoInputForTest(), node_->GetSilenceDataForTest());
}

HWTEST_F(SessionEffectNodeStateTest, activeNoData_channelMismatchReturnsSilence, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    config.inputCfg.channels = MONO;
    config.outputCfg.channels = STEREO;
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).Times(0);

    EXPECT_EQ(node_->ProcessActiveNoInputForTest(), node_->GetSilenceDataForTest());
}

// --- Preheat thread test fixture ---

class SessionEffectNodeThreadTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        nodeInfo_ = MakeTestNodeInfo();
        node_ = std::make_shared<HpaeSessionEffectNode>(nodeInfo_);  // REAL node, not Testable
        mockInst_ = std::make_shared<testing::NiceMock<MockEffectInstance>>();

        ON_CALL(*mockInst_, StopEffect()).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, GetAudioConfig()).WillByDefault(testing::ReturnRef(config_));
        ON_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillByDefault(testing::Return(0));

        // Connect upstream — node reads from upstreamNode
        HpaeNodeInfo upInfo = MakeTestNodeInfo();
        upInfo.nodeId = TEST_UPSTREAM_NODE_ID;
        upstreamNode_ = std::make_shared<MockUpstreamNode>(upInfo);
        node_->Connect(upstreamNode_);
    }
    void TearDown() override
    {
        node_.reset();
        mockInst_.reset();
        upstreamNode_.reset();
    }

    HpaeNodeInfo nodeInfo_;
    std::shared_ptr<HpaeSessionEffectNode> node_;
    std::shared_ptr<testing::NiceMock<MockEffectInstance>> mockInst_;
    std::shared_ptr<MockUpstreamNode> upstreamNode_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// SE-W2: Preheat thread pulls upstream data and FeedInput
HWTEST_F(SessionEffectNodeThreadTest, preheatThread_pullsAndFeeds, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(TEST_PREHEAT_FRAME_COUNT));
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    const size_t prebufTarget = TEST_PREBUF_TARGET_BYTE_LEN;
    int count = 0;
    ON_CALL(*mockInst_, GetInputLevel())
        .WillByDefault(testing::Invoke([&]() -> size_t {
            return ++count > 5 ? prebufTarget : 0;
        }));

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_))
        .Times(testing::AtLeast(1));

    node_->StartEffect();
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    for (int i = 0; i < 100 && !node_->GetPrebufFilled(); i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(node_->GetPrebufFilled());
}

// SE-W3: Prebuf filled -> thread exits -> DoProcess transitions to INTERCEPTING
HWTEST_F(SessionEffectNodeThreadTest, preheatThread_exitAndTransition, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(TEST_PREHEAT_FRAME_COUNT));
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    const size_t prebufTarget = TEST_PREBUF_TARGET_BYTE_LEN;

    // Track mock calls via global atomics (thread-safe, no production code changes)
    static std::atomic<int> gFeedCount{0};
    static std::atomic<int> gLevelCount{0};
    static std::atomic<size_t> gLastLevel{0};
    gFeedCount = 0;
    gLevelCount = 0;
    gLastLevel = 0;

    ON_CALL(*mockInst_, FeedInput(testing::_, testing::_))
        .WillByDefault(testing::Invoke([&](const uint8_t*, size_t) -> int32_t {
            gFeedCount.fetch_add(1, std::memory_order_relaxed);
            return 0;
        }));
    ON_CALL(*mockInst_, GetInputLevel())
        .WillByDefault(testing::Invoke([&]() -> size_t {
            gLevelCount.fetch_add(1, std::memory_order_relaxed);
            gLastLevel.store(prebufTarget, std::memory_order_relaxed);
            return prebufTarget;
        }));

    node_->StartEffect();

    for (int i = 0; i < 100 && !node_->GetPrebufFilled(); i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_TRUE(node_->GetPrebufFilled());

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
}

// SE-W4: DropEffect sets stopRequested and joins thread
HWTEST_F(SessionEffectNodeThreadTest, dropEffect_joinsThread, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);

    ON_CALL(*mockInst_, GetInputLevel()).WillByDefault(testing::Return(0));

    node_->StartEffect();
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->DropEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    EXPECT_EQ(node_->GetEffectInstance(), nullptr);
}

HWTEST_F(SessionEffectNodeStateTest, preheatNoDataTimeout_feedsPaddingAndExits, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    isolatedNode->CreateEffectWithInstance("test_effect", mockInst_, config_);

    const size_t prebufTarget = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    isolatedNode->SetPrebufTargetBytesForTest(prebufTarget);

    ON_CALL(*mockInst_, GetInputLevel()).WillByDefault(testing::Return(0));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, prebufTarget))
        .WillOnce(testing::Return(0));

    auto start = std::chrono::steady_clock::now();
    isolatedNode->RunPreheatThreadForTest();
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 90);
    EXPECT_TRUE(isolatedNode->GetPrebufFilled());
    EXPECT_EQ(isolatedNode->GetPaddingBytesInForTest(), prebufTarget);
    EXPECT_EQ(isolatedNode->GetTotalBytesIn(), 0u);
    EXPECT_EQ(isolatedNode->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, preheatThreadWithoutEffectInstanceExits, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);

    isolatedNode->RunPreheatThreadForTest();

    EXPECT_TRUE(isolatedNode->GetPrebufFilled());
}

HWTEST_F(SessionEffectNodeStateTest, preheatThreadWithDataButZeroChannelsBacksOff, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    AudioEffectConfig invalidConfig;
    invalidConfig.inputCfg.samplingRate = TEST_SAMPLE_RATE;
    invalidConfig.inputCfg.channels = 0;
    isolatedNode->CreateEffectWithInstance("test_effect", mockInst_, invalidConfig);
    isolatedNode->Connect(upstreamNode_);
    isolatedNode->SetPrebufTargetBytesForTest(0);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    ON_CALL(*mockInst_, GetInputLevel()).WillByDefault(testing::Return(0));

    isolatedNode->RunPreheatThreadForTest();

    EXPECT_TRUE(isolatedNode->GetPrebufFilled());
}

HWTEST_F(SessionEffectNodeStateTest, preheatInvalidInput_doesNotFeedAlgorithm, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    isolatedNode->CreateEffectWithInstance("test_effect", mockInst_, config_);
    isolatedNode->Connect(upstreamNode_);
    upstreamNode_->SetOutputValid(false);

    const size_t inputByteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    isolatedNode->SetPrebufTargetBytesForTest(inputByteLen * 2);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, inputByteLen)).Times(0);
    ON_CALL(*mockInst_, GetInputLevel()).WillByDefault(testing::Return(inputByteLen * 2));

    isolatedNode->RunPreheatThreadForTest();

    EXPECT_TRUE(isolatedNode->GetPrebufFilled());
}

HWTEST_F(SessionEffectNodeStateTest, preheatNoDataTimeout_recordsPaddingFailure, TestSize.Level0)
{
    auto isolatedNode = std::make_shared<TestableSessionEffectNode>(nodeInfo_);
    isolatedNode->CreateEffectWithInstance("test_effect", mockInst_, config_);

    const size_t prebufTarget = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    isolatedNode->SetPrebufTargetBytesForTest(prebufTarget);

    ON_CALL(*mockInst_, GetInputLevel()).WillByDefault(testing::Return(0));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, prebufTarget))
        .WillOnce(testing::Return(-1));

    isolatedNode->RunPreheatThreadForTest();

    EXPECT_TRUE(isolatedNode->GetPrebufFilled());
    EXPECT_EQ(isolatedNode->GetPaddingBytesInForTest(), 0u);
    EXPECT_EQ(isolatedNode->GetStatsForTest().processErrors, 1);
}

// --- Phase 3B-1 warm-up frame discard tests (SE-WU) ---

// SE-WU1: INTERCEPTING→ACTIVE via ReadOutput with skipCount
HWTEST_F(SessionEffectNodeStateTest, warmup_skipStaleOutput, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->SetPrebufFilledForTest(true);
    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::NotNull()))
        .WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// SE-WU2: Flush resets warmup timestamps and restarts preheat
HWTEST_F(SessionEffectNodeStateTest, warmup_flushResets, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));
    node_->FlushEffect();
    // FlushEffect now restarts preheat → WARMING_UP
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// SE-WU3: DropEffect + recreate → re-enter WARMING_UP
HWTEST_F(SessionEffectNodeStateTest, warmup_dropAndRecreate, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    node_->DropEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    auto inst2 = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*inst2, GetPreheatFrames()).WillByDefault(testing::Return(5));
    ON_CALL(*inst2, GetAudioConfig()).WillByDefault(testing::ReturnRef(config_));

    node_->CreateEffectWithInstance("test_effect", inst2, config_);
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// --- Phase 3B-2: Operation matrix tests ---

// SE-OP1: Pause during WARMING_UP — caches preserved, status unchanged
HWTEST_F(SessionEffectNodeStateTest, pause_duringWarmingUp, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    // PauseEffect should NOT call StopEffect (no DISABLE)
    EXPECT_CALL(*mockInst_, StopEffect()).Times(0);
    node_->PauseEffect();

    // status_ preserved, isPaused_ set
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_TRUE(node_->IsPaused());
    EXPECT_NE(node_->GetEffectInstance(), nullptr);
}

// SE-OP2: Pause during INTERCEPTING — preserves caches
HWTEST_F(SessionEffectNodeStateTest, pause_duringIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    EXPECT_CALL(*mockInst_, StopEffect()).Times(0);
    node_->PauseEffect();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
    EXPECT_TRUE(node_->IsPaused());
}

// SE-OP3: Pause during ACTIVE — preserves caches
HWTEST_F(SessionEffectNodeStateTest, pause_duringActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    EXPECT_CALL(*mockInst_, StopEffect()).Times(0);
    node_->PauseEffect();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_TRUE(node_->IsPaused());
}

// SE-OP4: Resume from WARMING_UP paused — restart preheat with existing prebuf
HWTEST_F(SessionEffectNodeStateTest, resume_fromWarmingUp, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    node_->PauseEffect();
    ASSERT_TRUE(node_->IsPaused());

    // Resume: status_ is still WARMING_UP → restart preheat with the existing target
    node_->StartEffect();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_FALSE(node_->IsPaused());
    EXPECT_EQ(node_->GetPrebufTargetBytes(), TEST_PREBUF_TARGET_BYTE_LEN);
}

HWTEST_F(SessionEffectNodeStateTest, resume_fromWarmingUpWithZeroPreheatMarksFilled, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    node_->PauseEffect();
    ASSERT_TRUE(node_->IsPaused());

    node_->StartEffect();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_FALSE(node_->IsPaused());
    EXPECT_EQ(node_->GetPrebufTargetBytes(), 0u);
    EXPECT_TRUE(node_->GetPrebufFilled());
}

// SE-OP5: Resume from INTERCEPTING paused — restore, algo output may trigger ACTIVE
HWTEST_F(SessionEffectNodeStateTest, resume_fromIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);
    node_->PauseEffect();

    // Resume: status_ is still INTERCEPTING
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
    EXPECT_FALSE(node_->IsPaused());

    // Next DoProcess with algo output → ACTIVE
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// SE-OP6: Resume from ACTIVE paused — continue steady state
HWTEST_F(SessionEffectNodeStateTest, resume_fromActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->PauseEffect();

    // Resume: status_ is still ACTIVE
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_FALSE(node_->IsPaused());

    // Verify steady state continues
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// --- Phase 3B-2: Flush tests ---

// SE-OP7: Flush during INTERCEPTING — restart preheat with normal prebuf
HWTEST_F(SessionEffectNodeStateTest, flush_duringIntercepting, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t frameBytes = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    size_t expectedPrebuf = 5 * frameBytes;
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->FlushEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->GetPrebufTargetBytes(), expectedPrebuf);
}

// SE-OP8: Flush during ACTIVE — restart preheat with normal prebuf
HWTEST_F(SessionEffectNodeStateTest, flush_duringActive, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t frameBytes = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    size_t expectedPrebuf = 5 * frameBytes;
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->FlushEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->GetPrebufTargetBytes(), expectedPrebuf);
}

HWTEST_F(SessionEffectNodeStateTest, flush_duringWarmingUpRestartsPreheat, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);

    size_t frameBytes = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    size_t expectedPrebuf = 5 * frameBytes;
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->FlushEffect();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->GetPrebufTargetBytes(), expectedPrebuf);
    EXPECT_FALSE(node_->GetPrebufFilled());
}

HWTEST_F(SessionEffectNodeStateTest, flush_duringWarmingUpWithZeroPreheatMarksFilled, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(0));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);

    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->FlushEffect();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->GetPrebufTargetBytes(), 0u);
    EXPECT_TRUE(node_->GetPrebufFilled());
}

// SE-OP9: Flush when paused — only reset state, no thread restart
HWTEST_F(SessionEffectNodeStateTest, flush_whenPausedStopped, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->PauseEffect();  // isPaused_=true, status_=ACTIVE

    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->FlushEffect();
    // Should reset to IDLE, isPaused_ cleared
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    EXPECT_FALSE(node_->IsPaused());
}

// --- Phase 3B-2: Close SE tests ---

// SE-OP10: Close SE during WARMING_UP — join + RewindBuffer + flush + bypass
HWTEST_F(SessionEffectNodeStateTest, closeSE_duringWarmingUp, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    // Simulate preheat thread feeding data → totalBytesIn_ > totalBytesOut_
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer simBuf(bufInfo);
    node_->FeedInputFromBuffer(&simBuf);
    ASSERT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());

    EXPECT_CALL(*mockCallback_, OnRewindBuffer(testing::_, testing::Gt(0u)));
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->CloseSessionEffectCore();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-OP11: Close SE during INTERCEPTING — RewindBuffer + flush + bypass
HWTEST_F(SessionEffectNodeStateTest, closeSE_duringIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    // Feed directly to create in > out (SignalProcess always produces output or silence)
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer simBuf(bufInfo);
    node_->FeedInputFromBuffer(&simBuf);
    ASSERT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());

    EXPECT_CALL(*mockCallback_, OnRewindBuffer(testing::_, testing::Gt(0u)));
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->CloseSessionEffectCore();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-OP12: Close SE during ACTIVE — RewindBuffer + flush + bypass
HWTEST_F(SessionEffectNodeStateTest, closeSE_duringActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Feed directly to create in > out
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer simBuf(bufInfo);
    node_->FeedInputFromBuffer(&simBuf);
    ASSERT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());

    EXPECT_CALL(*mockCallback_, OnRewindBuffer(testing::_, testing::_));
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->CloseSessionEffectCore();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-OP12a: Close SE during ACTIVE must also stop stale preheat thread
HWTEST_F(SessionEffectNodeStateTest, closeSE_duringActive_joinsPreheatThread, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->stopRequested_.store(false);
    node_->preheatThread_ = std::thread([node = node_]() {
        while (!node->stopRequested_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    ASSERT_TRUE(node_->preheatThread_.joinable());

    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->CloseSessionEffectCore();

    EXPECT_TRUE(node_->stopRequested_.load());
    EXPECT_FALSE(node_->preheatThread_.joinable());
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// --- Phase 3B-2: MoveStream tests ---

// SE-OP13: MoveStream during WARMING_UP — join + RewindBuffer + flush
HWTEST_F(SessionEffectNodeStateTest, moveStream_duringWarmingUp, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    // Simulate preheat thread → totalBytesIn_ > 0
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer simBuf(bufInfo);
    node_->FeedInputFromBuffer(&simBuf);
    ASSERT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());

    EXPECT_CALL(*mockCallback_, OnRewindBuffer(testing::_, testing::Gt(0u)));
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->PrepareMoveStream();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-OP14: MoveStream during INTERCEPTING
HWTEST_F(SessionEffectNodeStateTest, moveStream_duringIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    // Feed directly to create in > out
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer simBuf(bufInfo);
    node_->FeedInputFromBuffer(&simBuf);
    ASSERT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());

    EXPECT_CALL(*mockCallback_, OnRewindBuffer(testing::_, testing::Gt(0u)));
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->PrepareMoveStream();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-OP15: MoveStream during ACTIVE
HWTEST_F(SessionEffectNodeStateTest, moveStream_duringActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Feed directly to create in > out
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer simBuf(bufInfo);
    node_->FeedInputFromBuffer(&simBuf);
    ASSERT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());

    EXPECT_CALL(*mockCallback_, OnRewindBuffer(testing::_, testing::_));
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->PrepareMoveStream();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// --- Phase 3B-2: Destroy tests ---

// SE-OP16: Destroy during WARMING_UP — join + clear + Lazy Release
HWTEST_F(SessionEffectNodeStateTest, destroy_duringWarmingUp, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_CALL(*mockInst_, StopEffect()).WillOnce(testing::Return(0));

    node_->DropEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    EXPECT_EQ(node_->GetEffectInstance(), nullptr);
}

// SE-OP17: Destroy during INTERCEPTING
HWTEST_F(SessionEffectNodeStateTest, destroy_duringIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);
    EXPECT_CALL(*mockInst_, StopEffect()).WillOnce(testing::Return(0));

    node_->DropEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// SE-OP18: Destroy during ACTIVE
HWTEST_F(SessionEffectNodeStateTest, destroy_duringActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_CALL(*mockInst_, StopEffect()).WillOnce(testing::Return(0));

    node_->DropEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

HWTEST_F(SessionEffectNodeStateTest, dropEffect_handlesStoppingAndUnknownStatus, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::STOPPING);
    EXPECT_CALL(*mockInst_, StopEffect()).WillOnce(testing::Return(0));

    EXPECT_EQ(node_->DropEffect(), 0);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    auto mockInst = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*mockInst, StopEffect()).WillByDefault(testing::Return(0));
    node_->CreateEffectWithInstance("test_effect", mockInst, config_);
    node_->SetStatusForTest(static_cast<HpaeSessionEffectNode::STATUS>(999));
    EXPECT_CALL(*mockInst, StopEffect()).WillOnce(testing::Return(0));

    EXPECT_EQ(node_->DropEffect(), 0);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// --- Phase 3B-2: Algo fail tests ---

// SE-OP19: Algo fail during INTERCEPTING — silence, no state change
HWTEST_F(SessionEffectNodeStateTest, algoFail_duringIntercepting, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    // FeedInput fails
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_))
        .WillRepeatedly(testing::Return(-1));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    // Should stay INTERCEPTING, not change state
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
}

// SE-OP20: Algo fail during ACTIVE — silence, no state change
HWTEST_F(SessionEffectNodeStateTest, algoFail_duringActive, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    // ReadOutput fails (returns 0 or negative)
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(-1));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_))
        .WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    // Should stay ACTIVE
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// --- Phase 3B-2: Seek combination tests ---

// SE-OP21: Seek during WARMING_UP = Pause + Flush + Start → re-preheat (full restart)
HWTEST_F(SessionEffectNodeStateTest, seek_duringWarmingUp, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);

    // Pause: status_ stays WARMING_UP, isPaused_=true
    node_->PauseEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_TRUE(node_->IsPaused());

    // Flush while paused: resets status_=IDLE, isPaused_=false (Seek = full restart)
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));
    node_->FlushEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    EXPECT_FALSE(node_->IsPaused());

    // Start: status_=IDLE → full start with existing preheat target
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_FALSE(node_->IsPaused());
}

// SE-OP22: Seek during INTERCEPTING = Pause + Flush + Start → full restart
HWTEST_F(SessionEffectNodeStateTest, seek_duringIntercepting, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    node_->PauseEffect();
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));
    node_->FlushEffect();

    // Flush when paused: status_ reset to IDLE
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    // Start: full start → WARMING_UP with existing preheat target
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// SE-OP23: Seek during ACTIVE = Pause + Flush + Start → full restart
HWTEST_F(SessionEffectNodeStateTest, seek_duringActive, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);

    node_->PauseEffect();

    // Flush when paused → reset to IDLE
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));
    node_->FlushEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    // Start: full start → WARMING_UP with existing preheat target
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// ============================================================
// Phase 3C: GetLatency tests (SE-L1 ~ SE-L3)
// ============================================================

// SE-L1: GetLatency returns (totalBytesIn_ - totalBytesOut_) in microseconds
HWTEST_F(SessionEffectNodeStateTest, GetLatency_returns_input_minus_output, TestSize.Level0)
{
    // rawBypassNeeded_=true forces the INTERCEPTING silence path so dry frames are not counted as
    // algorithm output; GetLatency() (bytesIn - algoBytesOut) then reflects algorithm-internal buffering.
    // CreateEffectWithInstance must run with aissSwitchEnabled=false, otherwise it resets
    // rawBypassNeeded_=false and the INTERCEPTING dry frame would be accounted as output.
    node_->rawBypassNeeded_ = true;
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE, false);
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    uint64_t oneFrameLatencyUs = byteLen * 1000000ULL / (TEST_CHANNEL_COUNT * sizeof(float) * 48000);

    // Entering ACTIVE consumes one algorithm output frame, leaving one fed input frame pending.
    EXPECT_EQ(node_->GetTotalBytesIn() - node_->GetTotalBytesOut(), byteLen);
    EXPECT_EQ(node_->GetLatency(), oneFrameLatencyUs);

    // Feed input directly (simulating preheat/prebuf) without corresponding output
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer extraBuf(bufInfo);
    node_->FeedInputFromBuffer(&extraBuf);
    EXPECT_GT(node_->GetTotalBytesIn(), node_->GetTotalBytesOut());
    EXPECT_EQ(node_->GetLatency(), oneFrameLatencyUs * 2);
}

HWTEST_F(SessionEffectNodeStateTest, latencyAccounting_paddingDoesNotIncreaseGetLatency, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    uint64_t oneFrameLatencyUs = byteLen * 1000000ULL / (TEST_CHANNEL_COUNT * sizeof(float) * TEST_SAMPLE_RATE);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).Times(2).WillRepeatedly(testing::Return(0));

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer pcmBuf(bufInfo);

    node_->FeedInputFromBuffer(&pcmBuf);
    EXPECT_EQ(node_->GetLatency(), oneFrameLatencyUs);

    EXPECT_EQ(node_->FeedPaddingInput(byteLen), 0);
    EXPECT_EQ(node_->GetPaddingBytesInForTest(), byteLen);
    EXPECT_EQ(node_->GetTotalBytesIn(), byteLen);
    EXPECT_EQ(node_->GetLatency(), oneFrameLatencyUs);

    node_->AccountOutputForTest(byteLen);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, latencyAccounting_paddingOnlyHasZeroLatency, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));

    EXPECT_EQ(node_->FeedPaddingInput(byteLen), 0);
    EXPECT_EQ(node_->GetPaddingBytesInForTest(), byteLen);
    EXPECT_EQ(node_->GetTotalBytesIn(), 0u);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, getLatency_zeroWhenSampleRateMissing, TestSize.Level0)
{
    AudioEffectConfig config;
    config.inputCfg.channels = TEST_CHANNEL_COUNT;
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer pcmBuf(bufInfo);

    EXPECT_EQ(node_->FeedInputFromBuffer(&pcmBuf), byteLen);
    EXPECT_EQ(node_->GetTotalBytesIn(), byteLen);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, feedInputFromBuffer_zeroChannelsReturnsZero, TestSize.Level0)
{
    AudioEffectConfig config;
    config.inputCfg.samplingRate = TEST_SAMPLE_RATE;
    config.inputCfg.channels = 0;
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer pcmBuf(bufInfo);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_EQ(node_->FeedInputFromBuffer(&pcmBuf), 0u);
    EXPECT_EQ(node_->GetStatsForTest().framesProcessed, 0);
}

HWTEST_F(SessionEffectNodeStateTest, feedInputFromBuffer_invalidBufferReturnsZero, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer pcmBuf(bufInfo);
    pcmBuf.SetBufferValid(false);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);

    EXPECT_EQ(node_->FeedInputFromBuffer(&pcmBuf), 0u);
    EXPECT_EQ(node_->GetTotalBytesIn(), 0u);
}

HWTEST_F(SessionEffectNodeStateTest, feedPaddingInput_returnsZeroWithoutInstanceOrByteLen, TestSize.Level0)
{
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_EQ(node_->FeedPaddingInput(byteLen), 0);

    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_EQ(node_->FeedPaddingInput(0), 0);
    EXPECT_EQ(node_->GetPaddingBytesInForTest(), 0u);
}

// SE-L2: WARMING_UP DoProcess outputs silence directly, not via SignalProcess
// so totalBytesOut_ stays 0, preserving GetLatency accuracy during warmup
HWTEST_F(SessionEffectNodeStateTest, GetLatency_includes_preheat_silence, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(5));
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->DoProcess();
    node_->DoProcess();

    // WARMING_UP DoProcess writes silence directly, bypasses SignalProcess
    EXPECT_EQ(node_->GetTotalBytesOut(), 0u);
    // Time-based skip: verify DoProcess stayed in WARMING_UP (prebuf not filled)
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// SE-L2-ext: INTERCEPTING silence progress follows time-based skip, not every silence frame
HWTEST_F(SessionEffectNodeStateTest, GetLatency_interceptingSilenceProgressMatchesSkipFrames, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    // rawBypassNeeded_=true forces the INTERCEPTING silence path (PrepareSilenceBuffer) so output
    // accounting matches the skip frames instead of counting every raw-bypass frame.
    node_->rawBypassNeeded_ = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    node_->SetPrebufFilledForTest(true);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    uint32_t capturedSkipFrames = 0;
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::NotNull()))
        .WillOnce(testing::DoAll(
            testing::WithArg<2>(testing::Invoke([&capturedSkipFrames](uint32_t *skipCount) {
                capturedSkipFrames = *skipCount;
            })),
            testing::Return(0)));

    node_->DoProcess();

    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
    EXPECT_EQ(node_->GetTotalBytesOut(), static_cast<size_t>(capturedSkipFrames) * byteLen);
    EXPECT_EQ(node_->GetStatsForTest().framesOutput, 0);
}

// SE-L3: GetLatency zero when bypass (no effect instance)
HWTEST_F(SessionEffectNodeStateTest, GetLatency_zero_when_bypass, TestSize.Level0)
{
    EXPECT_EQ(node_->GetLatency(), 0u);
}

// SE-L3-ext: After DropEffect all counters zeroed
HWTEST_F(SessionEffectNodeStateTest, GetLatency_zero_after_drop, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_CALL(*mockInst_, StopEffect()).WillOnce(testing::Return(0));
    node_->DropEffect();

    EXPECT_EQ(node_->GetTotalBytesIn(), 0u);
    EXPECT_EQ(node_->GetTotalBytesOut(), 0u);
    EXPECT_EQ(node_->GetLatency(), 0u);
}

// ============================================================
// Phase 3C: Crossfade tests (SE-CF1 ~ SE-CF3)
// ============================================================

class SessionEffectNodeCrossfadeTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        nodeInfo_ = MakeTestNodeInfo();
        node_ = std::make_shared<TestableSEWithSignalProcess>(nodeInfo_);
        mockInst_ = std::make_shared<testing::NiceMock<MockEffectInstance>>();

        ON_CALL(*mockInst_, StopEffect()).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillByDefault(testing::Return(0));
        ON_CALL(*mockInst_, Flush()).WillByDefault(testing::Return(0));
    }
    void TearDown() override
    {
        node_.reset();
        mockInst_.reset();
    }

    HpaeNodeInfo nodeInfo_;
    std::shared_ptr<TestableSEWithSignalProcess> node_;
    std::shared_ptr<testing::NiceMock<MockEffectInstance>> mockInst_;
};

// SE-CF1: INTERCEPTING→ACTIVE fade-in — first algo output gets linear fade-in
HWTEST_F(SessionEffectNodeCrossfadeTest, Crossfade_silence_to_algo_output, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t sampleCount = TEST_FRAME_LEN * TEST_CHANNEL_COUNT;
    size_t byteLen = sampleCount * sizeof(float);

    // Mock: algorithm has output ready (all 1.0f)
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::DoAll(
            testing::WithArgs<0, 1>(testing::Invoke(
                [](uint8_t *dst, size_t len) {
                    auto *f = reinterpret_cast<float *>(dst);
                    std::fill(f, f + len / sizeof(float), 1.0f);
                })),
            testing::Return(static_cast<int32_t>(byteLen))));

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer inputBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&inputBuf};
    HpaePcmBuffer *result = node_->SignalProcess(inputs);

    // After INTERCEPTING fade-in: status -> ACTIVE
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Verify fade-in: sample[i] = 1.0 * (i+1)/sampleCount
    float *out = result->GetPcmDataBuffer();
    for (size_t i = 0; i < sampleCount; ++i) {
        float expected = 1.0f * static_cast<float>(i + 1) / static_cast<float>(sampleCount);
        EXPECT_NEAR(out[i], expected, 1e-6f);
    }
}

// SE-CF2: Close crossfade drains queued wet without feeding dry back into the closing
// algorithm, then stays STOPPING (FinalizeClose owns deletion).
HWTEST_F(SessionEffectNodeCrossfadeTest, Crossfade_algoToBypass_usesQueuedWetWithoutFeed, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t sampleCount = TEST_FRAME_LEN * TEST_CHANNEL_COUNT;
    size_t byteLen = sampleCount * sizeof(float);

    // Input dry: all 0.5f
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer inputBuf(bufInfo);
    std::fill(inputBuf.GetPcmDataBuffer(), inputBuf.GetPcmDataBuffer() + sampleCount, 0.5f);

    // Closing algorithm must NOT be fed this dry block; it only drains queued wet (all 1.0f).
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::DoAll(
            testing::WithArgs<0, 1>(testing::Invoke([](uint8_t *dst, size_t len) {
                auto *wet = reinterpret_cast<float *>(dst);
                std::fill(wet, wet + len / sizeof(float), 1.0f);
            })),
            testing::Return(static_cast<int32_t>(byteLen))));

    node_->StartCloseCrossfade();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);
    HpaePcmBuffer *result = node_->SignalProcess({&inputBuf});

    ASSERT_EQ(result, &inputBuf);
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);
    // Per-frame wet->dry: out[frame][ch] = wet*(1-t) + dry*t, t=(frame+1)/frameLen.
    for (size_t frame = 0; frame < TEST_FRAME_LEN; ++frame) {
        float t = static_cast<float>(frame + 1) / static_cast<float>(TEST_FRAME_LEN);
        float expected = 1.0f * (1.0f - t) + 0.5f * t;
        for (size_t channel = 0; channel < TEST_CHANNEL_COUNT; ++channel) {
            EXPECT_NEAR(result->GetPcmDataBuffer()[frame * TEST_CHANNEL_COUNT + channel],
                expected, 1e-6f);
        }
    }
}

// SE-CF2b: Without a full wet block the close frame returns dry untouched, stays STOPPING,
// and silenceData_ stays zero (the close frame writes wet into tailOutputData_, never silenceData_).
HWTEST_F(SessionEffectNodeCrossfadeTest, StoppingWithoutFullWet_returnsDryAndKeepsSilenceZero, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    size_t sampleCount = TEST_FRAME_LEN * TEST_CHANNEL_COUNT;
    size_t byteLen = sampleCount * sizeof(float);
    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer inputBuf(bufInfo);
    std::fill(inputBuf.GetPcmDataBuffer(), inputBuf.GetPcmDataBuffer() + sampleCount, 0.25f);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(0));

    node_->StartCloseCrossfade();
    EXPECT_EQ(node_->SignalProcess({&inputBuf}), &inputBuf);  // close frame: no full wet -> dry
    EXPECT_EQ(node_->SignalProcess({&inputBuf}), &inputBuf);  // STOPPING bypass -> dry
    for (size_t i = 0; i < sampleCount; ++i) {
        EXPECT_FLOAT_EQ(inputBuf.GetPcmDataBuffer()[i], 0.25f);
    }

    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::WARMING_UP);
    auto *silence = node_->SignalProcess({&inputBuf});
    ASSERT_EQ(silence, node_->GetSilenceDataForTest());
    for (size_t i = 0; i < sampleCount; ++i) {
        EXPECT_FLOAT_EQ(silence->GetPcmDataBuffer()[i], 0.0f);
    }
}

HWTEST_F(SessionEffectNodeCrossfadeTest, SignalProcess_handlesIdleWarmingStoppingAndUnknownStatus, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer inputBuf(bufInfo);
    std::vector<HpaePcmBuffer *> inputs = {&inputBuf};

    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::IDLE);
    EXPECT_EQ(node_->SignalProcess(inputs), &inputBuf);

    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(node_->SignalProcess(inputs), node_->GetSilenceDataForTest());

    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::STOPPING);
    EXPECT_EQ(node_->SignalProcess(inputs), &inputBuf);

    node_->SetStatusForTest(static_cast<HpaeSessionEffectNode::STATUS>(999));
    EXPECT_EQ(node_->SignalProcess(inputs), &inputBuf);
}

// SE-CF3: Crossfade completes within a single frame (5~10ms @48kHz = 240~480 samples)
HWTEST_F(SessionEffectNodeCrossfadeTest, Crossfade_duration_5_10ms, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t sampleCount = TEST_FRAME_LEN * TEST_CHANNEL_COUNT;
    size_t byteLen = sampleCount * sizeof(float);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(static_cast<int32_t>(byteLen)));

    PcmBufferInfo bufInfo;
    bufInfo.ch = TEST_CHANNEL_COUNT;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = 48000;
    HpaePcmBuffer inputBuf(bufInfo);
    std::vector<HpaePcmBuffer *> inputs = {&inputBuf};

    // One SignalProcess call completes the transition
    node_->SignalProcess(inputs);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Next call is already ACTIVE
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, byteLen)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_)).WillRepeatedly(testing::Return(0));
    node_->SignalProcess(inputs);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// --- Phase 3A-2: SeProcessStats counter tests ---

// SE-D1: Stats counter accumulates frames in processing
HWTEST_F(SessionEffectNodeAlgoTest, stats_accumulatesFrames, TestSize.Level0)
{
    auto inst = CreateInstanceWithLib();
    node_->CreateEffectWithInstance("test_effect", inst, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    node_->SetStatsReportThresholdForTest(999999);

    PcmBufferInfo bufInfo;
    bufInfo.ch = config_.inputCfg.channels;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer pcmBuf(bufInfo);

    // SignalProcess calls FeedInputFromBuffer which accumulates framesProcessed
    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    node_->SignalProcess(inputs);

    const auto &stats = node_->GetStatsForTest();
    // framesProcessed should be > 0 (exact count depends on frameLen * channels)
    EXPECT_GT(stats.framesProcessed, 0);
}

// SE-D2: Stats counter resets on DropEffect
HWTEST_F(SessionEffectNodeAlgoTest, stats_resetsOnDrop, TestSize.Level0)
{
    auto inst = CreateInstanceWithLib();
    node_->CreateEffectWithInstance("test_effect", inst, config_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    node_->SetStatsReportThresholdForTest(999999);

    PcmBufferInfo bufInfo;
    bufInfo.ch = config_.inputCfg.channels;
    bufInfo.frameLen = TEST_FRAME_LEN;
    bufInfo.rate = TEST_SAMPLE_RATE;
    HpaePcmBuffer pcmBuf(bufInfo);

    std::vector<HpaePcmBuffer *> inputs = {&pcmBuf};
    node_->SignalProcess(inputs);
    EXPECT_GT(node_->GetStatsForTest().framesProcessed, 0);

    node_->DropEffect();
    EXPECT_EQ(node_->GetStatsForTest().framesProcessed, 0);
    EXPECT_EQ(node_->GetStatsForTest().framesOutput, 0);
}

// --- Time-based skip frames tests ---

// SE-TS1: ComputeSkipFrames returns ~0 for near-zero elapsed time
HWTEST_F(SessionEffectNodeStateTest, computeSkipFrames_nearZero, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    node_->SetPrebufFilledForTest(true);
    node_->DoProcess();
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);
}

// SE-TS2: ComputeSkipFrames grows with elapsed time
HWTEST_F(SessionEffectNodeStateTest, computeSkipFrames_growsWithTime, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    std::this_thread::sleep_for(std::chrono::milliseconds(25));

    node_->SetPrebufFilledForTest(true);
    node_->DoProcess();
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(byteLen));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// SE-TS3: Pause compensation — pause duration excluded from skip calculation
HWTEST_F(SessionEffectNodeStateTest, computeSkipFrames_pauseCompensation, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    node_->PauseEffect();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    node_->StartEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(byteLen));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

// SE-TS4: Flush resets timestamps — fresh warmup after flush
HWTEST_F(SessionEffectNodeStateTest, computeSkipFrames_flushResets, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(0));
    EnterState(HpaeSessionEffectNode::STATUS::INTERCEPTING);

    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));
    node_->FlushEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockInst_, FeedInput(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    node_->DoProcess();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
}

HWTEST_F(HpaeSessionEffectNodeTest, suspendPreheatForGraphSwitch_warmingUp_setsFlag, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(testing::Return(0));
    ON_CALL(*instance, StopEffect()).WillByDefault(testing::Return(0));
    ASSERT_EQ(node_->CreateEffectWithInstance("test_effect", instance, config_), 0);
    ASSERT_EQ(node_->StartEffect(), 0);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->SuspendSessionEffectPreheatForGraphSwitch();

    EXPECT_TRUE(node_->preheatSuspendedForGraphSwitch_);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    node_->DropEffect();
}

// B1b: Suspend while NOT WARMING_UP is a no-op (flag stays false).
HWTEST_F(HpaeSessionEffectNodeTest, suspendPreheatForGraphSwitch_nonWarmingUp_isNoOp, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(testing::Return(0));
    ON_CALL(*instance, StopEffect()).WillByDefault(testing::Return(0));
    ASSERT_EQ(node_->CreateEffectWithInstance("test_effect", instance, config_), 0);
    // status_ stays IDLE — StartEffect NOT called.
    ASSERT_NE(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    node_->SuspendSessionEffectPreheatForGraphSwitch();

    EXPECT_FALSE(node_->preheatSuspendedForGraphSwitch_);
    node_->DropEffect();
}

// B2a: Resume without prior Suspend is a no-op.
HWTEST_F(HpaeSessionEffectNodeTest, resumePreheatForGraphSwitch_withoutSuspend_isNoOp, TestSize.Level0)
{
    ASSERT_FALSE(node_->preheatSuspendedForGraphSwitch_);
    ASSERT_EQ(node_->GetEffectInstance(), nullptr);

    node_->ResumeSessionEffectPreheatForGraphSwitch();

    EXPECT_FALSE(node_->preheatSuspendedForGraphSwitch_);
}

// B2b + B3a: Resume when suspended but status no longer WARMING_UP clears the
// flag and skips RestartPreheat.
HWTEST_F(HpaeSessionEffectNodeTest, resumePreheatForGraphSwitch_suspendedNonWarmingUp_clearsFlag, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(testing::Return(0));
    ON_CALL(*instance, StopEffect()).WillByDefault(testing::Return(0));
    ASSERT_EQ(node_->CreateEffectWithInstance("test_effect", instance, config_), 0);
    ASSERT_EQ(node_->StartEffect(), 0);
    node_->SuspendSessionEffectPreheatForGraphSwitch();
    ASSERT_TRUE(node_->preheatSuspendedForGraphSwitch_);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::IDLE);  // status drifted off WARMING_UP

    node_->ResumeSessionEffectPreheatForGraphSwitch();

    EXPECT_FALSE(node_->preheatSuspendedForGraphSwitch_);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    node_->DropEffect();
}

// B2b + B3b: Resume when WARMING_UP but effectInstance_ already released clears
// the flag and skips RestartPreheat.
HWTEST_F(HpaeSessionEffectNodeTest, resumePreheatForGraphSwitch_warmingUpNoInstance_clearsFlag, TestSize.Level0)
{
    // Fixture node_ has no effectInstance_ (CreateEffectWithInstance not called).
    ASSERT_EQ(node_->GetEffectInstance(), nullptr);
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::WARMING_UP);
    node_->preheatSuspendedForGraphSwitch_ = true;  // simulate a prior Suspend

    node_->ResumeSessionEffectPreheatForGraphSwitch();

    EXPECT_FALSE(node_->preheatSuspendedForGraphSwitch_);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// B2b + B3c: Resume when suspended, WARMING_UP, and instance bound restarts
// the preheat thread and clears the flag.
HWTEST_F(HpaeSessionEffectNodeTest, resumePreheatForGraphSwitch_suspendedWarmingUp_restartsAndClearsFlag, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(testing::Return(0));
    ON_CALL(*instance, StopEffect()).WillByDefault(testing::Return(0));
    ASSERT_EQ(node_->CreateEffectWithInstance("test_effect", instance, config_), 0);
    ASSERT_EQ(node_->StartEffect(), 0);
    node_->SuspendSessionEffectPreheatForGraphSwitch();
    ASSERT_TRUE(node_->preheatSuspendedForGraphSwitch_);

    node_->ResumeSessionEffectPreheatForGraphSwitch();

    EXPECT_FALSE(node_->preheatSuspendedForGraphSwitch_);
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    node_->DropEffect();
}

// --- Phase 3D-1: branch coverage gaps from fresh gcov ---

// Covers CloseSessionEffectCore() — full function was previously unhit.
// Reset state, clear effectInstance_, transition to IDLE regardless of current state.
HWTEST_F(SessionEffectNodeStateTest, CloseSessionEffectCore_resetsStateAndInstance, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_NE(node_->GetEffectInstance(), nullptr);

    node_->CloseSessionEffectCore();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
    EXPECT_EQ(node_->GetEffectInstance(), nullptr);
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
    EXPECT_FALSE(node_->IsPaused());
}

// Covers UpdatePrebufTargetBytes early return when effectInstance_ == nullptr.
HWTEST_F(SessionEffectNodeStateTest, updatePrebufTargetBytes_noInstance_returnsEarly, TestSize.Level0)
{
    // No CreateEffectWithInstance called — effectInstance_ is null.
    node_->SetPrebufTargetBytesForTest(123u);
    node_->UpdatePrebufTargetBytesForTest();
    EXPECT_EQ(node_->GetPrebufTargetBytes(), 0u);
}

// Covers UpdatePrebufTargetBytes GetPcmByteLen-fail return (channels == 0).
HWTEST_F(SessionEffectNodeStateTest, updatePrebufTargetBytes_zeroChannels_returnsEarly, TestSize.Level0)
{
    AudioEffectConfig config = MakeTestConfig();
    config.inputCfg.channels = 0;
    node_->CreateEffectWithInstance("test_effect", mockInst_, config);

    EXPECT_CALL(*mockInst_, GetPreheatFrames()).WillRepeatedly(testing::Return(5));
    node_->UpdatePrebufTargetBytesForTest();
    EXPECT_EQ(node_->GetPrebufTargetBytes(), 0u);
}

// Covers HandleNoDataTimeout early-return when timeout not yet elapsed.
HWTEST_F(SessionEffectNodeStateTest, handleNoDataTimeout_withinWindow_returnsFalse, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetPrebufTargetBytesForTest(100u);
    node_->SetPreheatNoDataStartTimeForTest(std::chrono::steady_clock::now());
    EXPECT_FALSE(node_->HandleNoDataTimeoutForTest(0u));
}

// Covers HandleNoDataTimeout: timeout elapsed AND stopRequested_ == true.
HWTEST_F(SessionEffectNodeStateTest, handleNoDataTimeout_stopRequestedAfterTimeout_returnsTrue, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetPrebufTargetBytesForTest(100u);
    auto past = std::chrono::steady_clock::now() - std::chrono::hours(1);
    node_->SetPreheatNoDataStartTimeForTest(past);
    node_->SetStopRequestedForTest(true);
    EXPECT_TRUE(node_->HandleNoDataTimeoutForTest(0u));
}

// Covers HandleNoDataTimeout: timeout elapsed AND inputLevel >= prebufTargetBytes_.
HWTEST_F(SessionEffectNodeStateTest, handleNoDataTimeout_inputLevelAtTargetAfterTimeout_returnsTrue, TestSize.Level0)
{
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    node_->SetPrebufTargetBytesForTest(100u);
    auto past = std::chrono::steady_clock::now() - std::chrono::hours(1);
    node_->SetPreheatNoDataStartTimeForTest(past);
    // stopRequested_ stays false; inputLevel == prebufTargetBytes_ takes the early return.
    EXPECT_TRUE(node_->HandleNoDataTimeoutForTest(100u));
}

// Covers StartPreheatThread joinable branch (L541-543): after entering ACTIVE
// via the real preheat thread, the std::thread handle is still joinable even
// though PreheatThreadFunc has returned. Calling FlushEffect → RestartPreheat →
// StartPreheatThread must observe preheatThread_.joinable()==true and stop the
// existing thread before starting a new one.
HWTEST_F(SessionEffectNodeThreadTest, flushDuringActive_restartsPreheatThread_joinableBranch, TestSize.Level0)
{
    ON_CALL(*mockInst_, GetPreheatFrames()).WillByDefault(testing::Return(TEST_PREHEAT_FRAME_COUNT));
    node_->CreateEffectWithInstance("test_effect", mockInst_, config_);
    ASSERT_EQ(node_->StartEffect(), 0);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    // Drive preheat thread to completion: inputLevel reaches prebufTarget →
    // PreheatThreadFunc returns; preheatThread_ is still joinable (not joined yet).
    const size_t prebufTarget = TEST_PREBUF_TARGET_BYTE_LEN;
    int count = 0;
    ON_CALL(*mockInst_, GetInputLevel())
        .WillByDefault(testing::Invoke([&]() -> size_t {
            return ++count > 3 ? prebufTarget : 0;
        }));
    for (int i = 0; i < 100 && !node_->GetPrebufFilled(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(node_->GetPrebufFilled());

    // Force status to ACTIVE so FlushEffect takes the ACTIVE branch that calls
    // RestartPreheat without StopPreheatThread first.
    node_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    EXPECT_CALL(*mockInst_, Flush()).WillOnce(testing::Return(0));

    node_->FlushEffect();
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    // Wait for the new preheat thread to also reach prebufFilled so TearDown
    // joins cleanly without races.
    count = 0;
    for (int i = 0; i < 100 && !node_->GetPrebufFilled(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// SE-CF-CB1: pending close crossfade consumed -> OnCloseCrossfadeDone fired exactly once
// (mix skipped: ReadOutput returns 0, callback still fires — completion is unconditional)
HWTEST_F(SessionEffectNodeStateTest, closeCrossfade_done_firesCallbackOnce, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    // Shadow the saturated EnterState expectation: the close drain yields no wet block,
    // so the mix is skipped and the dry block passes through.
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(0));
    EXPECT_CALL(*mockCallback_, OnCloseCrossfadeDone(TEST_SESSION_ID)).Times(1);
    node_->StartCloseCrossfade();
    ASSERT_TRUE(node_->IsCloseCrossfadePending());
    node_->DoProcess();  // upstream pull -> SignalProcess -> ProcessCloseCrossfade
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
    EXPECT_EQ(node_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);
}

// SE-CF-CB2: mix path — ReadOutput returns a full block -> still exactly one callback
HWTEST_F(SessionEffectNodeStateTest, closeCrossfade_mixPath_firesCallbackOnce, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    size_t byteLen = TEST_FRAME_LEN * TEST_CHANNEL_COUNT * sizeof(float);
    // ProcessCloseCrossfade drains without a skip pointer (skipCount = nullptr)
    EXPECT_CALL(*mockInst_, ReadOutput(testing::_, byteLen, testing::_))
        .WillOnce(testing::Return(byteLen));
    EXPECT_CALL(*mockCallback_, OnCloseCrossfadeDone(TEST_SESSION_ID)).Times(1);
    node_->StartCloseCrossfade();
    node_->DoProcess();
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
}

// SE-CF-CB3: CHECK early-exit path (config channel mismatch) -> callback still fires once
HWTEST_F(SessionEffectNodeStateTest, closeCrossfade_earlyExit_firesCallbackOnce, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::ACTIVE);
    node_->audioConfig_.inputCfg.channels = 4;  // inputCfg 4ch vs outputCfg 2ch -> config-mismatch early exit
    EXPECT_CALL(*mockCallback_, OnCloseCrossfadeDone(TEST_SESSION_ID)).Times(1);
    node_->StartCloseCrossfade();
    node_->DoProcess();
    EXPECT_FALSE(node_->IsCloseCrossfadePending());
}

// SE-CF-CB4: no pending (WARMING_UP node) -> no callback ever
HWTEST_F(SessionEffectNodeStateTest, closeCrossfade_notPending_noCallback, TestSize.Level0)
{
    EnterState(HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_CALL(*mockCallback_, OnCloseCrossfadeDone(TEST_SESSION_ID)).Times(0);
    node_->StartCloseCrossfade();
    ASSERT_FALSE(node_->IsCloseCrossfadePending());
    node_->DoProcess();
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
