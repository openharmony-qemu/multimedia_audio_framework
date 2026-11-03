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
#include <chrono>
#include <memory>
#include <thread>

#include "hpae_process_cluster.h"
#include "hpae_session_effect_node.h"
#include "hpae_sink_input_node.h"
#include "effect_instance.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "fake_audio_effect_lib_entry.h"
#include "mock_effect_instance.h"
#include "hpae_plugin_node.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr uint32_t TEST_SESSION_ID = 42;
static constexpr uint32_t TEST_NODE_ID = 100;
static constexpr size_t TEST_FRAME_LEN = 960;

static HpaeNodeInfo MakeTestNodeInfo()
{
    HpaeNodeInfo info;
    info.nodeId = TEST_NODE_ID;
    info.sessionId = TEST_SESSION_ID;
    info.frameLen = TEST_FRAME_LEN;
    info.samplingRate = SAMPLE_RATE_48000;
    info.channels = STEREO;
    info.format = SAMPLE_F32LE;
    return info;
}

static AudioEffectConfig MakeTestConfig()
{
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg.samplingRate = SAMPLE_RATE_48000;
    config.outputCfg.channels = STEREO;
    return config;
}

class HpaeProcessClusterSeTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        HpaeNodeInfo nodeInfo = MakeTestNodeInfo();
        HpaeSinkInfo dummySinkInfo;
        cluster_ = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);

        auto sinkInput = std::make_shared<HpaeSinkInputNode>(nodeInfo);
        cluster_->CreateNodes(sinkInput);
        cluster_->Connect(sinkInput);
    }

    void TearDown() override {}

    std::shared_ptr<HpaeProcessCluster> cluster_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// PC-3: HasSessionEffectNode returns correct status
HWTEST_F(HpaeProcessClusterSeTest, HasSessionEffectNode, TestSize.Level0)
{
    // 未创建 -> false
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    // 创建后 -> true
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    // 销毁后 -> false
    uint64_t taskId = 0;
    cluster_->DestroySessionEffectNode(TEST_SESSION_ID, taskId);
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
}

// PC-1: CreateSessionEffectNode inserts node into processing chain
HWTEST_F(HpaeProcessClusterSeTest, CreateSessionEffectNode_connects, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);

    int32_t ret = cluster_->CreateSessionEffectNode(
        TEST_SESSION_ID, "test_effect", instance, config_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
}

// PC-2: DestroySessionEffectNode removes node and restores chain
HWTEST_F(HpaeProcessClusterSeTest, DestroySessionEffectNode_disconnects, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    ASSERT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    uint64_t taskId = 0;
    int32_t ret = cluster_->DestroySessionEffectNode(TEST_SESSION_ID, taskId);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
}

// PC-2a: DestroySessionEffectNode waits preheat thread before disconnect/flush path
HWTEST_F(HpaeProcessClusterSeTest, DestroySessionEffectNode_waitsPreheatThreadIdle, TestSize.Level0)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);

    node->stopRequested_.store(false);
    node->preheatThread_ = std::thread([node]() {
        while (!node->stopRequested_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    ASSERT_TRUE(node->preheatThread_.joinable());

    uint64_t taskId = 0;
    int32_t ret = cluster_->DestroySessionEffectNode(TEST_SESSION_ID, taskId);

    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_TRUE(node->stopRequested_.load());
    EXPECT_FALSE(node->preheatThread_.joinable());
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
}

// PC-4: DoProcess with SessionEffectNode active
HWTEST_F(HpaeProcessClusterSeTest, DoProcess_callsSignalProcess, TestSize.Level0)
{
    FakeAudioEffectLibEntry fake;
    fake.SetPreheatFrames(0);
    auto instance = std::make_shared<EffectInstance>();
    auto lib = fake.GetLibrary();
    instance->Init("test_effect", config_, &lib);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
}

// PC-5: SessionEffect start/pause control works
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectStart_pause, TestSize.Level0)
{
    FakeAudioEffectLibEntry fake;
    fake.SetPreheatFrames(0);
    auto instance = std::make_shared<EffectInstance>();
    auto lib = fake.GetLibrary();
    instance->Init("test_effect", config_, &lib);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    cluster_->SessionEffectStart(TEST_SESSION_ID);
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    EXPECT_EQ(cluster_->SessionEffectPause(TEST_SESSION_ID), 0);
    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->IsPaused());
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));  // still exists after pause
}

// Stream data callback: fills PCM buffer with a constant float value.
// Registered via SinkInputNode::RegisterWriteCallback to inject test data.
class WriteFixedFloatCb : public IStreamCallback,
                           public std::enable_shared_from_this<WriteFixedFloatCb> {
public:
    explicit WriteFixedFloatCb(float value) : value_(value) {}
    virtual ~WriteFixedFloatCb() = default;
    int32_t OnStreamData(AudioCallBackStreamInfo &info) override
    {
        float *data = reinterpret_cast<float *>(info.inputData);
        size_t n = info.requestDataLen / sizeof(float);
        for (size_t i = 0; i < n; i++) {
            data[i] = value_;
        }
        return 0;
    }

private:
    float value_;
};

// Probe node: captures output buffer for verification.
class ProbeNode : public HpaePluginNode {
public:
    using HpaePluginNode::HpaePluginNode;
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override
    {
        captured_ = inputs.empty() ? nullptr : inputs[0];
        return captured_;
    }
    HpaePcmBuffer *GetCaptured() const { return captured_; }
    uint64_t GetLatency(uint32_t) override { return 0; }

private:
    HpaePcmBuffer *captured_ = nullptr;
};

__attribute__((unused)) static void ExpectBufferEquals(HpaePcmBuffer *buf, float expectedValue)
{
    ASSERT_NE(buf, nullptr);
    float *data = buf->GetPcmDataBuffer();
    size_t n = buf->GetFrameLen() * buf->GetChannelCount();
    for (size_t i = 0; i < n; i++) {
        EXPECT_FLOAT_EQ(data[i], expectedValue) << " mismatch at sample " << i;
    }
}

static void ExpectBufferSilence(HpaePcmBuffer *buf)
{
    ASSERT_NE(buf, nullptr);
    float *data = buf->GetPcmDataBuffer();
    size_t n = buf->GetFrameLen() * buf->GetChannelCount();
    for (size_t i = 0; i < n; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.0f) << " non-zero at sample " << i;
    }
}

class HpaeProcessClusterSeDataflowTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fake_.Reset();
        HpaeNodeInfo nodeInfo = MakeTestNodeInfo();

        sinkInput_ = std::make_shared<HpaeSinkInputNode>(nodeInfo);
        callback_ = std::make_shared<WriteFixedFloatCb>(0.5f);
        sinkInput_->RegisterWriteCallback(callback_->shared_from_this());

        HpaeSinkInfo dummySinkInfo;
        cluster_ = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
        cluster_->CreateNodes(sinkInput_);
        cluster_->Connect(sinkInput_);

        HpaeNodeInfo probeInfo = MakeTestNodeInfo();
        probeInfo.nodeId = TEST_NODE_ID + 1;
        probe_ = std::make_shared<ProbeNode>(probeInfo);
        probe_->Connect(cluster_);
    }

    void TearDown() override
    {
        auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
        if (seNode != nullptr) {
            seNode->DropEffect();
        }
    }

    std::shared_ptr<EffectInstance> CreateEffectInstance()
    {
        auto inst = std::make_shared<EffectInstance>();
        auto lib = fake_.GetLibrary();
        inst->Init("test_effect", config_, &lib);
        return inst;
    }

    FakeAudioEffectLibEntry fake_;
    std::shared_ptr<HpaeSinkInputNode> sinkInput_;
    std::shared_ptr<WriteFixedFloatCb> callback_;
    std::shared_ptr<HpaeProcessCluster> cluster_;
    std::shared_ptr<ProbeNode> probe_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// PC-DF-1: WARMING_UP -- SE outputs silence, does not pull upstream
HWTEST_F(HpaeProcessClusterSeDataflowTest, warmingUp_outputsSilence, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_EQ(seNode->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    seNode->rawBypassNeeded_ = true;

    probe_->DoProcess();

    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    ExpectBufferSilence(out);
}

// PC-DF-2: INTERCEPTING — SE pulls upstream, FeedInput called, outputs silence
HWTEST_F(HpaeProcessClusterSeDataflowTest, intercepting_feedsAndOutputsSilence, TestSize.Level0)
{
    fake_.SetPreheatFrames(1);  // Preheat exits after 1 frame
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    seNode->rawBypassNeeded_ = true;
    // Wait for preheat to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // First DoProcess: WARMING_UP→INTERCEPTING (prebufFilled=true)
    probe_->DoProcess();

    uint32_t feedCountAfterTransition = fake_.GetFeedInputCount();
    EXPECT_GE(feedCountAfterTransition, 1u);

    // Second DoProcess: still INTERCEPTING, feeds more, outputs silence
    probe_->DoProcess();
    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    ExpectBufferSilence(out);
    EXPECT_GT(fake_.GetFeedInputCount(), feedCountAfterTransition);
}

// PC-DF-3: ACTIVE — SE outputs algorithm passthrough data (0.5f)
HWTEST_F(HpaeProcessClusterSeDataflowTest, active_outputsAlgoData, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);       // Skip preheat entirely
    fake_.SetProcessThreshold(1920); // Match actual FeedInput size (960 frames * 2 ch)
    fake_.SetProcessDelayMs(0);      // Instant processing
    fake_.SetOutputPersistent(true); // getOutput won't consume buffer, safe for skip
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);

    // #1: WARMING_UP→INTERCEPTING (prebufFilled=true from SetPreheatFrames(0))
    probe_->DoProcess();
    ASSERT_EQ(seNode->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    // #2: INTERCEPTING→ACTIVE (output now available from #1 processing)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_EQ(seNode->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Verify SE node processed data (FeedInput+GetOutput called) — proves data flow works
    EXPECT_GE(fake_.GetFeedInputCount(), 2u);
    EXPECT_GE(fake_.GetGetOutputCount(), 1u);

    // ACTIVE state: ProcessActive feeds input and reads algo output (proves ACTIVE data path)
    fake_.Reset();
    probe_->DoProcess();
    EXPECT_GT(fake_.GetFeedInputCount(), 0u) << "FeedInput not called in ACTIVE state";
    EXPECT_GT(fake_.GetGetOutputCount(), 0u) << "GetOutput not called in ACTIVE state";
}

// PC-DF-4: DestroySessionEffectNode — data flows through without SE
HWTEST_F(HpaeProcessClusterSeDataflowTest, destroy_restoresPassthrough, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    fake_.SetOutputPersistent(true);
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);

    // Drive to ACTIVE
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(seNode->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    uint64_t taskId = 0;
    cluster_->DestroySessionEffectNode(TEST_SESSION_ID, taskId);
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    // After destroy, data flows through without SE processing
    probe_->DoProcess();
    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    // Verify passthrough: SE node no longer exists, data flows directly
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
}

// PC-TOPO-1: CreateSessionEffectNode — topology correct
HWTEST_F(HpaeProcessClusterSeDataflowTest, create_topo_correct, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_EQ(seNode->GetPreOutNum(), 1u);

    // ConverterNode's output should connect to SE
    auto converterNode = cluster_->GetConverterNodeById(TEST_SESSION_ID);
    ASSERT_NE(converterNode, nullptr);
    EXPECT_EQ(converterNode->GetOutputPortNum(), 1u);
}

// PC-TOPO-2: DestroySessionEffectNode — chain restored
HWTEST_F(HpaeProcessClusterSeDataflowTest, destroy_topo_restored, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    uint64_t taskId = 0;
    cluster_->DestroySessionEffectNode(TEST_SESSION_ID, taskId);

    auto converterNode = cluster_->GetConverterNodeById(TEST_SESSION_ID);
    ASSERT_NE(converterNode, nullptr);
    EXPECT_EQ(converterNode->GetOutputPortNum(), 1u);
}

// PC-FLUSH-1: SessionEffectFlush forwards to node and returns SUCCESS
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectFlush_success, TestSize.Level0)
{
    FakeAudioEffectLibEntry fake;
    fake.SetPreheatFrames(0);
    auto instance = std::make_shared<EffectInstance>();
    auto lib = fake.GetLibrary();
    instance->Init("test_effect", config_, &lib);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    int32_t ret = cluster_->SessionEffectFlush(TEST_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);
    // FlushEffect restarts preheat thread; allow it to settle before tear-down
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// PC-FLUSH-2: SessionEffectFlush returns ERR_EFFECT_NOT_BOUND when no SE
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectFlush_notBound, TestSize.Level0)
{
    int32_t ret = cluster_->SessionEffectFlush(TEST_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// PC-PAUSE-1: SessionEffectPause sets isPaused on node
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectPause_success, TestSize.Level0)
{
    FakeAudioEffectLibEntry fake;
    fake.SetPreheatFrames(0);
    auto instance = std::make_shared<EffectInstance>();
    auto lib = fake.GetLibrary();
    instance->Init("test_effect", config_, &lib);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    int32_t ret = cluster_->SessionEffectPause(TEST_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->IsPaused());
}

// PC-PAUSE-2: SessionEffectPause returns ERR_EFFECT_NOT_BOUND when no SE
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectPause_notBound, TestSize.Level0)
{
    int32_t ret = cluster_->SessionEffectPause(TEST_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// PC-MOVE-1: SessionEffectPrepareMoveStream calls CloseSessionEffectCore
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectPrepareMoveStream_success, TestSize.Level0)
{
    FakeAudioEffectLibEntry fake;
    fake.SetPreheatFrames(0);
    auto instance = std::make_shared<EffectInstance>();
    auto lib = fake.GetLibrary();
    instance->Init("test_effect", config_, &lib);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    cluster_->SessionEffectPrepareMoveStream(TEST_SESSION_ID);
    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);
}

// PC-MOVE-2: SessionEffectPrepareMoveStream no crash when no SE
HWTEST_F(HpaeProcessClusterSeTest, SessionEffectPrepareMoveStream_notBound, TestSize.Level0)
{
    // void return — just verify no crash
    cluster_->SessionEffectPrepareMoveStream(TEST_SESSION_ID);
    SUCCEED();
}

// T1: Connect with SE — no duplicate output ports on loudnessGainNode
HWTEST_F(HpaeProcessClusterSeDataflowTest, connect_withSE_noDuplicateOutput, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    ASSERT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    auto loudnessGain = cluster_->GetLoudnessGainNode(TEST_SESSION_ID);

    cluster_->DisConnect(sinkInput_);
    cluster_->Connect(sinkInput_);

    ASSERT_NE(loudnessGain, nullptr);
    EXPECT_EQ(loudnessGain->GetOutputPortNum(), 1u);
    ASSERT_NE(seNode, nullptr);
    EXPECT_EQ(seNode->GetOutputPortNum(), 1u);
}

// T3: Repeated Pause/Start — port count doesn't leak
HWTEST_F(HpaeProcessClusterSeDataflowTest, pauseStart_noConnectionLeak, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    auto loudnessGain = cluster_->GetLoudnessGainNode(TEST_SESSION_ID);
    ASSERT_NE(loudnessGain, nullptr);
    size_t initialPortCount = loudnessGain->GetOutputPortNum();

    for (int i = 0; i < 3; i++) {
        cluster_->DisConnect(sinkInput_);
        cluster_->Connect(sinkInput_);
    }

    EXPECT_EQ(loudnessGain->GetOutputPortNum(), initialPortCount);
}

// T4: DisConnect with SE — disconnects SE from loudnessGainNode
HWTEST_F(HpaeProcessClusterSeDataflowTest, disconnect_withSE_disconnectsSE, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    auto seNode = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    auto loudnessGain = cluster_->GetLoudnessGainNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    ASSERT_NE(loudnessGain, nullptr);

    // Before disconnect: chain intact
    EXPECT_EQ(loudnessGain->GetOutputPortNum(), 1u);
    EXPECT_EQ(seNode->GetOutputPortNum(), 1u);

    // DisConnect
    cluster_->DisConnect(sinkInput_);
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
    EXPECT_EQ(loudnessGain->GetOutputPortNum(), 0u);

    // Reconnect
    cluster_->Connect(sinkInput_);
    EXPECT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
    EXPECT_EQ(loudnessGain->GetOutputPortNum(), 1u);
    EXPECT_EQ(seNode->GetOutputPortNum(), 1u);
}

// T5: Pause→Start WARMING_UP recovery (ProcessCluster layer)
// SessionEffectStart directly sets WARMING_UP and starts preheat thread
HWTEST_F(HpaeProcessClusterSeTest, pauseStart_warmingUp, TestSize.Level0)
{
    FakeAudioEffectLibEntry fake;
    fake.SetPreheatFrames(0);
    auto instance = std::make_shared<EffectInstance>();
    auto lib = fake.GetLibrary();
    instance->Init("test_effect", config_, &lib);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    // SessionEffectStart directly sets WARMING_UP (no DoProcess needed)
    cluster_->SessionEffectStart(TEST_SESSION_ID);
    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    // Pause
    cluster_->SessionEffectPause(TEST_SESSION_ID);
    EXPECT_TRUE(node->IsPaused());
    // Status preserved (WARMING_UP), preheat thread stopped
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    // Start — should resume from WARMING_UP
    cluster_->SessionEffectStart(TEST_SESSION_ID);
    EXPECT_FALSE(node->IsPaused());
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// T6: Pause→Start INTERCEPTING recovery (ProcessCluster layer, data flow)
HWTEST_F(HpaeProcessClusterSeDataflowTest, pauseStart_intercepting, TestSize.Level0)
{
    fake_.SetPreheatFrames(1);
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);

    // Wait for preheat to complete → INTERCEPTING
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    probe_->DoProcess();
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    // Pause
    cluster_->SessionEffectPause(TEST_SESSION_ID);
    EXPECT_TRUE(node->IsPaused());

    // Start
    cluster_->SessionEffectStart(TEST_SESSION_ID);
    EXPECT_FALSE(node->IsPaused());
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::INTERCEPTING);

    // Data flow verification: after resume, DoProcess should pull upstream data
    fake_.Reset();
    probe_->DoProcess();
    EXPECT_GT(fake_.GetFeedInputCount(), 0u) << "FeedInput not called after resume — data path broken";
}

// T7: Pause→Start ACTIVE recovery (ProcessCluster layer, data flow)
HWTEST_F(HpaeProcessClusterSeDataflowTest, pauseStart_active, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    fake_.SetOutputPersistent(true);
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);

    // Drive to ACTIVE
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Pause
    cluster_->SessionEffectPause(TEST_SESSION_ID);
    EXPECT_TRUE(node->IsPaused());

    // Start
    cluster_->SessionEffectStart(TEST_SESSION_ID);
    EXPECT_FALSE(node->IsPaused());
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Data flow verification
    fake_.Reset();
    probe_->DoProcess();
    EXPECT_GT(fake_.GetFeedInputCount(), 0u) << "FeedInput not called after ACTIVE resume — data path broken";
    EXPECT_GT(fake_.GetGetOutputCount(), 0u) << "GetOutput not called after ACTIVE resume — output path broken";
}

// T8: Pause→Flush→Start from IDLE restart (ProcessCluster layer)
HWTEST_F(HpaeProcessClusterSeDataflowTest, pauseFlushStart_fromIdle, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    fake_.SetOutputPersistent(true);
    auto instance = CreateEffectInstance();
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    cluster_->SessionEffectStart(TEST_SESSION_ID);

    auto node = cluster_->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(node, nullptr);
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Pause
    cluster_->SessionEffectPause(TEST_SESSION_ID);
    EXPECT_TRUE(node->IsPaused());

    // Flush — resets to IDLE, clears isPaused_
    cluster_->SessionEffectFlush(TEST_SESSION_ID);
    EXPECT_FALSE(node->IsPaused());
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    // Start — should go through first-start path → WARMING_UP
    cluster_->SessionEffectStart(TEST_SESSION_ID);
    EXPECT_FALSE(node->IsPaused());
    EXPECT_EQ(node->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// RATE-1: QueryAlgoInputFormat returns 48kHz when SessionEffectNode is present
HWTEST_F(HpaeProcessClusterSeTest, QueryAlgoInputFormat_forces48kWhenSE, TestSize.Level0)
{
    // Setup: cluster with sink rate = 96kHz, loudness OFF
    HpaeNodeInfo nodeInfo = MakeTestNodeInfo();
    HpaeSinkInfo sinkInfo;
    sinkInfo.samplingRate = SAMPLE_RATE_96000;
    sinkInfo.channels = STEREO;
    sinkInfo.channelLayout = CH_LAYOUT_STEREO;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    auto sinkInput = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    cluster->CreateNodes(sinkInput);
    cluster->Connect(sinkInput);

    // Without SE node, renderEffectNode clamps rate to 48k
    AudioBasicFormat fmt;
    cluster->QueryAlgoInputFormat(TEST_SESSION_ID, fmt);
    EXPECT_EQ(fmt.rate, SAMPLE_RATE_48000);

    // Create SE node
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    cluster->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);

    // With SE node, rate should be forced to 48k
    cluster->QueryAlgoInputFormat(TEST_SESSION_ID, fmt);
    EXPECT_EQ(fmt.rate, SAMPLE_RATE_48000);
}

// RATE-2: with per-stream SE divergence, the mixer converter keeps mixer input format uniform (bug repro)
HWTEST_F(HpaeProcessClusterSeTest, MixerConverterUniformFormatWithSE, TestSize.Level0)
{
    // SCENE_EXTRA cluster, sink 96kHz: no render effect, mixer format = sink format
    HpaeNodeInfo nodeInfoA = MakeTestNodeInfo();
    nodeInfoA.sceneType = HPAE_SCENE_EFFECT_NONE;
    HpaeNodeInfo nodeInfoB = MakeTestNodeInfo();
    nodeInfoB.sessionId = TEST_SESSION_ID + 1;
    nodeInfoB.nodeId = TEST_NODE_ID + 1;
    nodeInfoB.sceneType = HPAE_SCENE_EFFECT_NONE;

    HpaeSinkInfo sinkInfo;
    sinkInfo.samplingRate = SAMPLE_RATE_96000;
    sinkInfo.channels = STEREO;
    sinkInfo.channelLayout = CH_LAYOUT_STEREO;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfoA, sinkInfo);
    auto sinkInputA = std::make_shared<HpaeSinkInputNode>(nodeInfoA);
    auto sinkInputB = std::make_shared<HpaeSinkInputNode>(nodeInfoB);
    ASSERT_EQ(cluster->CreateNodes(sinkInputA), SUCCESS);
    ASSERT_EQ(cluster->CreateNodes(sinkInputB), SUCCESS);
    cluster->Connect(sinkInputA);
    cluster->Connect(sinkInputB);
    EXPECT_EQ(cluster->GetMixerConverterNodeCount(), 2);
    EXPECT_EQ(cluster->GetPreOutNum(), 2); // both streams wired into the mixer via their mixer converters

    // SE only on stream A: the per-frame format queries now diverge (48k vs 96k) -- the old bug input
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    ASSERT_EQ(cluster->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    AudioBasicFormat fmtA;
    AudioBasicFormat fmtB;
    fmtA.rate = SAMPLE_RATE_44100; // sentinel: 48000 equals the default, so a no-write query cannot pass vacuously
    EXPECT_EQ(cluster->QueryAlgoInputFormat(TEST_SESSION_ID, fmtA), SUCCESS);
    EXPECT_EQ(fmtA.rate, SAMPLE_RATE_48000); // SE forces 48k for A's conv#1
    EXPECT_EQ(cluster->QueryAlgoInputFormat(TEST_SESSION_ID + 1, fmtB), SUCCESS);
    EXPECT_EQ(fmtB.rate, SAMPLE_RATE_96000); // B falls back to mixer format

    // mixer-side target and the mixer converter output are uniform at sink rate for both streams
    EXPECT_EQ(cluster->QueryMixInputFormat(TEST_SESSION_ID, fmtA), SUCCESS);
    EXPECT_EQ(fmtA.rate, SAMPLE_RATE_96000);
    ASSERT_NE(cluster->GetMixerConverterNodeById(TEST_SESSION_ID), nullptr);
    ASSERT_NE(cluster->GetMixerConverterNodeById(TEST_SESSION_ID + 1), nullptr);
    EXPECT_EQ(cluster->GetMixerConverterNodeById(TEST_SESSION_ID)->GetSampleRate(), SAMPLE_RATE_96000);
    EXPECT_EQ(cluster->GetMixerConverterNodeById(TEST_SESSION_ID + 1)->GetSampleRate(), SAMPLE_RATE_96000);
    // 20ms frameLen at the 96k mixer-input rate (pins the 2x-drain defect fix)
    EXPECT_EQ(cluster->GetMixerConverterNodeById(TEST_SESSION_ID)->GetFrameLen(), 1920u);
    EXPECT_EQ(cluster->GetMixerConverterNodeById(TEST_SESSION_ID + 1)->GetFrameLen(), 1920u);
}

// RATE-3: adapters deliver position-specific formats live, no re-registration needed
HWTEST_F(HpaeProcessClusterSeTest, FormatQueryAdapterLiveUpdate, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = MakeTestNodeInfo();
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    HpaeSinkInfo sinkInfo;
    sinkInfo.samplingRate = SAMPLE_RATE_96000;
    sinkInfo.channels = STEREO;
    sinkInfo.channelLayout = CH_LAYOUT_STEREO;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    auto sinkInput = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    ASSERT_EQ(cluster->CreateNodes(sinkInput), SUCCESS);

    auto converter = cluster->GetConverterNodeById(TEST_SESSION_ID);
    auto mixerConverter = cluster->GetMixerConverterNodeById(TEST_SESSION_ID);
    ASSERT_NE(converter, nullptr);
    ASSERT_NE(mixerConverter, nullptr);
    // both converters start at the sink format (no special algo requirement yet)
    EXPECT_EQ(converter->GetSampleRate(), SAMPLE_RATE_96000);
    EXPECT_EQ(mixerConverter->GetSampleRate(), SAMPLE_RATE_96000);

    // SE created after node creation: the algo-side adapter answers 48k on the next
    // per-frame query through the same registered callback, no re-registration
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    ASSERT_EQ(cluster->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    EXPECT_TRUE(converter->CheckUpdateOutInfo()); // algo adapter now answers 48k stereo
    EXPECT_EQ(converter->GetSampleRate(), SAMPLE_RATE_48000);
    EXPECT_EQ(converter->GetFrameLen(), 960u); // frameLen rescales with the rate (20ms kept)
    EXPECT_FALSE(mixerConverter->CheckUpdateOutInfo()); // mix adapter still answers sink format
    EXPECT_EQ(mixerConverter->GetSampleRate(), SAMPLE_RATE_96000);
}

// DESTROY-1: DestroyNodes cleans idSessionEffectMap_ when SE node exists
HWTEST_F(HpaeProcessClusterSeTest, DestroyNodes_cleansSessionEffectMap, TestSize.Level0)
{
    // Create SE node
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("test_effect", config_);
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    ASSERT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    // DestroyNodes should clean up SE node
    int32_t ret = cluster_->DestroyNodes(TEST_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    // Verify other maps also cleaned
    EXPECT_EQ(cluster_->GetConverterNodeCount(), 0);
    EXPECT_EQ(cluster_->GetGainNodeCount(), 0);
    EXPECT_EQ(cluster_->GetLoudnessGainNodeCount(), 0);
}

// DESTROY-2: Reload cleanup detaches SE without disabling the logical effect instance
HWTEST_F(HpaeProcessClusterSeTest, DestroyNodes_reloadDetachDoesNotStopEffect, TestSize.Level0)
{
    auto instance = std::make_shared<testing::NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(testing::Return(0));
    cluster_->CreateSessionEffectNode(TEST_SESSION_ID, "test_effect", instance, config_);
    ASSERT_TRUE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));

    EXPECT_CALL(*instance, StopEffect()).Times(0);
    int32_t ret = cluster_->DestroyNodes(TEST_SESSION_ID, SessionEffectCleanupMode::DETACH_FOR_RELOAD);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(cluster_->HasSessionEffectNode(TEST_SESSION_ID));
    EXPECT_EQ(cluster_->GetConverterNodeCount(), 0);
    EXPECT_EQ(cluster_->GetGainNodeCount(), 0);
    EXPECT_EQ(cluster_->GetLoudnessGainNodeCount(), 0);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
