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

#include <cmath>
#include <gtest/gtest.h>
#include "hpae_audio_format_converter_cluster.h"
#include "hpae_mic_in_front_extract_node.h"
#include "hpae_mic_in_zero_pad_node.h"
#include "test_case_common.h"
#include "hpae_node_common.h"
#include "audio_stream_info.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeAudioFormatConverterClusterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeAudioFormatConverterClusterTest::SetUp()
{}

void HpaeAudioFormatConverterClusterTest::TearDown()
{}

namespace {
const size_t DEFAULT_FRAMELEN_SECOND = 960;

// scenario B fixture: HAL partitioned 6ch = main(4) + micIn(2), app wants main only (4ch)
// (consumed by the MicInCluster Compose/Latency tests; Task 4 chain e2e tests consume it further)
static void ConfigScenarioB(HpaeNodeInfo &pre, HpaeNodeInfo &out)
{
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(6);    // HAL total = main4 + micIn2
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = static_cast<AudioChannel>(4);    // app wants main only
    out.sourceType = SOURCE_TYPE_CAMCORDER;
    out.micInChannels = static_cast<AudioChannel>(2);
    out.appMicInChannelCount = 0;
}

// scenario C fixture: HAL main only 4ch, app wants total 6ch = main(4) + micIn(2)
// (consumed by the MicInCluster Compose/Latency tests; Task 4 chain e2e tests consume it further)
static void ConfigScenarioC(HpaeNodeInfo &pre, HpaeNodeInfo &out)
{
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(4);    // HAL main = 4
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = static_cast<AudioChannel>(6);    // app total = main4 + micIn2
    out.sourceType = SOURCE_TYPE_CAMCORDER;
    out.micInChannels = CHANNEL_UNKNOW;             // HAL reports no micIn
    out.appMicInChannelCount = 2;
}

// Passthrough stub used as chain upstream in e2e tests: pre-written port data is
// returned by PullOutputData without running DoProcess.
class FakeUpstreamNode : public HpaePluginNode {
public:
    explicit FakeUpstreamNode(HpaeNodeInfo &nodeInfo) : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo) {}
    uint64_t GetLatency(uint32_t sessionId = 0) override
    {
        return 0;
    }
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override
    {
        if (inputs.empty() || inputs[0] == nullptr) {
            return PrepareSilenceBuffer(silenceData_);
        }
        return inputs[0];
    }
};
} // namespace

/*
 * @tc.name  : FrontExtract_SignalProcess_001
 * @tc.type  : FUNC
 * @tc.number: FrontExtract_SignalProcess_001
 * @tc.desc  : Partitioned 6ch input -> 4ch output equals the leading main block; micIn tail dropped.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, FrontExtract_SignalProcess_001, TestSize.Level0)
{
    HpaeNodeInfo pre;   // raw upstream: partitioned total
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(6);
    HpaeNodeInfo main;  // cluster-computed main info
    main.samplingRate = SAMPLE_RATE_48000;
    main.frameLen = DEFAULT_FRAMELEN_SECOND;
    main.channels = static_cast<AudioChannel>(4);

    auto node = std::make_shared<HpaeMicInFrontExtractNode>(pre, main);
    PcmBufferInfo pcmInfo(6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    // non-interleaved partition: [main block 4x960 = 0.5f][micIn block 2x960 = 0.9f]
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }
    for (size_t i = DEFAULT_FRAMELEN_SECOND * 4; i < DEFAULT_FRAMELEN_SECOND * 6; i++) {
        input.GetPcmDataBuffer()[i] = 0.9f;
    }

    HpaePcmBuffer *out = node->SignalProcess({&input});
    ASSERT_NE(out, nullptr);
    ASSERT_EQ(out->GetChannelCount(), 4u);
    ASSERT_EQ(out->GetFrameLen(), DEFAULT_FRAMELEN_SECOND);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        EXPECT_FLOAT_EQ(out->GetPcmDataBuffer()[i], 0.5f);  // main block preserved
    }
}

/*
 * @tc.name  : FrontExtract_SignalProcess_002
 * @tc.type  : FUNC
 * @tc.number: FrontExtract_SignalProcess_002
 * @tc.desc  : Invalid input yields silence marked invalid+silent (4ch width).
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, FrontExtract_SignalProcess_002, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(6);
    HpaeNodeInfo main;
    main.samplingRate = SAMPLE_RATE_48000;
    main.frameLen = DEFAULT_FRAMELEN_SECOND;
    main.channels = static_cast<AudioChannel>(4);

    auto node = std::make_shared<HpaeMicInFrontExtractNode>(pre, main);
    PcmBufferInfo pcmInfo(6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(false);

    HpaePcmBuffer *out = node->SignalProcess({&input});
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(out->IsValid());
    EXPECT_TRUE(out->IsSilence());
}

/*
 * @tc.name  : FrontExtract_UpstreamChange_001
 * @tc.type  : FUNC
 * @tc.number: FrontExtract_UpstreamChange_001
 * @tc.desc  : Upstream channel change mid-stream exercises the frozen-width warning branch (no crash).
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, FrontExtract_UpstreamChange_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(6);
    HpaeNodeInfo main;
    main.samplingRate = SAMPLE_RATE_48000;
    main.frameLen = DEFAULT_FRAMELEN_SECOND;
    main.channels = static_cast<AudioChannel>(4);

    auto node = std::make_shared<HpaeMicInFrontExtractNode>(pre, main);
    PcmBufferInfo pcmInfo6(6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input6(pcmInfo6);
    input6.SetBufferValid(true);
    ASSERT_NE(node->SignalProcess({&input6}), nullptr);

    PcmBufferInfo pcmInfo8(8, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input8(pcmInfo8);
    input8.SetBufferValid(true);
    // frozen width vs 8ch upstream -> warning branch, still extracts leading 4ch
    HpaePcmBuffer *out = node->SignalProcess({&input8});
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(out->GetChannelCount(), 4u);
    EXPECT_TRUE(out->IsValid());
}

/*
 * @tc.name  : ZeroPad_SignalProcess_001
 * @tc.type  : FUNC
 * @tc.number: ZeroPad_SignalProcess_001
 * @tc.desc  : main 4ch input -> total 6ch output partitioned [main block][all-zero micIn block].
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, ZeroPad_SignalProcess_001, TestSize.Level0)
{
    HpaeNodeInfo main;  // upstream: main only
    main.samplingRate = SAMPLE_RATE_48000;
    main.frameLen = DEFAULT_FRAMELEN_SECOND;
    main.channels = static_cast<AudioChannel>(4);
    HpaeNodeInfo total; // app total
    total.samplingRate = SAMPLE_RATE_48000;
    total.frameLen = DEFAULT_FRAMELEN_SECOND;
    total.channels = static_cast<AudioChannel>(6);

    auto node = std::make_shared<HpaeMicInZeroPadNode>(main, total);
    PcmBufferInfo pcmInfo(4, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }

    HpaePcmBuffer *out = node->SignalProcess({&input});
    ASSERT_NE(out, nullptr);
    ASSERT_EQ(out->GetChannelCount(), 6u);
    ASSERT_EQ(out->GetFrameLen(), DEFAULT_FRAMELEN_SECOND);
    EXPECT_TRUE(out->IsValid());
    size_t mainRegion = DEFAULT_FRAMELEN_SECOND * 4;
    size_t totalRegion = DEFAULT_FRAMELEN_SECOND * 6;
    for (size_t i = 0; i < mainRegion; i++) {
        EXPECT_FLOAT_EQ(out->GetPcmDataBuffer()[i], 0.5f);
    }
    for (size_t i = mainRegion; i < totalRegion; i++) {
        EXPECT_FLOAT_EQ(out->GetPcmDataBuffer()[i], 0.0f);  // single contiguous zero fill
    }

    // poison the micIn tail through the returned buffer, then re-process the same input:
    // the tail must be re-zeroed every cycle, not merely zero-initialized
    for (size_t i = mainRegion; i < totalRegion; i++) {
        out->GetPcmDataBuffer()[i] = 0.9f;
    }
    HpaePcmBuffer *outAgain = node->SignalProcess({&input});
    ASSERT_NE(outAgain, nullptr);
    for (size_t i = 0; i < mainRegion; i++) {
        EXPECT_FLOAT_EQ(outAgain->GetPcmDataBuffer()[i], 0.5f);
    }
    for (size_t i = mainRegion; i < totalRegion; i++) {
        EXPECT_FLOAT_EQ(outAgain->GetPcmDataBuffer()[i], 0.0f);
    }

    // state propagation: a valid-but-silent input is reflected on the output buffer
    input.SetBufferSilence(true);
    HpaePcmBuffer *outSilent = node->SignalProcess({&input});
    ASSERT_NE(outSilent, nullptr);
    EXPECT_TRUE(outSilent->IsValid());
    EXPECT_TRUE(outSilent->IsSilence());
}

/*
 * @tc.name  : ZeroPad_SignalProcess_002
 * @tc.type  : FUNC
 * @tc.number: ZeroPad_SignalProcess_002
 * @tc.desc  : Invalid input yields silence at total(6) width, marked invalid+silent.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, ZeroPad_SignalProcess_002, TestSize.Level0)
{
    HpaeNodeInfo main;
    main.samplingRate = SAMPLE_RATE_48000;
    main.frameLen = DEFAULT_FRAMELEN_SECOND;
    main.channels = static_cast<AudioChannel>(4);
    HpaeNodeInfo total;
    total.samplingRate = SAMPLE_RATE_48000;
    total.frameLen = DEFAULT_FRAMELEN_SECOND;
    total.channels = static_cast<AudioChannel>(6);

    auto node = std::make_shared<HpaeMicInZeroPadNode>(main, total);
    PcmBufferInfo pcmInfo(4, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(false);

    HpaePcmBuffer *out = node->SignalProcess({&input});
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(out->IsValid());
    EXPECT_TRUE(out->IsSilence());
    EXPECT_EQ(out->GetChannelCount(), 6u);
}

/*
 * @tc.name  : ZeroPad_UpstreamChange_001
 * @tc.type  : FUNC
 * @tc.number: ZeroPad_UpstreamChange_001
 * @tc.desc  : Upstream channel change mid-stream exercises the frozen-width warning branch (no crash).
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, ZeroPad_UpstreamChange_001, TestSize.Level0)
{
    HpaeNodeInfo main;
    main.samplingRate = SAMPLE_RATE_48000;
    main.frameLen = DEFAULT_FRAMELEN_SECOND;
    main.channels = static_cast<AudioChannel>(4);
    HpaeNodeInfo total;
    total.samplingRate = SAMPLE_RATE_48000;
    total.frameLen = DEFAULT_FRAMELEN_SECOND;
    total.channels = static_cast<AudioChannel>(6);

    auto node = std::make_shared<HpaeMicInZeroPadNode>(main, total);
    PcmBufferInfo pcmInfo4(4, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input4(pcmInfo4);
    input4.SetBufferValid(true);
    ASSERT_NE(node->SignalProcess({&input4}), nullptr);

    PcmBufferInfo pcmInfo5(5, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input5(pcmInfo5);
    input5.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 5; i++) {
        input5.GetPcmDataBuffer()[i] = 0.5f;
    }
    // 5 <= total 6 -> frozen-width warning fires but extraction continues:
    // 5ch main copied + 1ch zero tail
    HpaePcmBuffer *out5 = node->SignalProcess({&input5});
    ASSERT_NE(out5, nullptr);
    EXPECT_TRUE(out5->IsValid());
    EXPECT_EQ(out5->GetChannelCount(), 6u);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 5; i++) {
        EXPECT_FLOAT_EQ(out5->GetPcmDataBuffer()[i], 0.5f);
    }
    for (size_t i = DEFAULT_FRAMELEN_SECOND * 5; i < DEFAULT_FRAMELEN_SECOND * 6; i++) {
        EXPECT_FLOAT_EQ(out5->GetPcmDataBuffer()[i], 0.0f);
    }

    PcmBufferInfo pcmInfo8(8, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input8(pcmInfo8);
    input8.SetBufferValid(true);
    // frozen total width vs 8ch upstream (> total) -> silence fallback path, no crash
    HpaePcmBuffer *out = node->SignalProcess({&input8});
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(out->IsValid());  // 8 > total 6 -> hard error -> silence
}

/*
 * @tc.name  : MicInCluster_Compose_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_Compose_001
 * @tc.desc  : Scenario B: head = extract node, tail = converter node, zero-pad node absent;
 *             inner converter sees sanitized info (in=main 4ch, out=4ch, no micIn fields).
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_Compose_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    HpaeNodeInfo out;
    ConfigScenarioB(pre, out);

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    ASSERT_NE(cluster->extractNode_, nullptr);
    ASSERT_NE(cluster->converterNode_, nullptr);
    EXPECT_EQ(cluster->zeroPadNode_, nullptr);
    EXPECT_EQ(cluster->head_, cluster->extractNode_);
    EXPECT_EQ(cluster->tail_, cluster->converterNode_);
    EXPECT_EQ(cluster->extractNode_->GetChannelCount(), 4u);
    EXPECT_EQ(cluster->converterNode_->channelConverter_.GetInChannelInfo().numChannels, 4u);
    EXPECT_EQ(cluster->converterNode_->channelConverter_.GetOutChannelInfo().numChannels, 4u);
    EXPECT_EQ(cluster->converterNode_->GetPreOutNum(), 1u);   // internal link wired
}

/*
 * @tc.name  : MicInCluster_Compose_002
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_Compose_002
 * @tc.desc  : Scenario C: head = converter node, tail = zero-pad node, extract node absent;
 *             inner converter sanitized (4->4), zero-pad node carries total(6).
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_Compose_002, TestSize.Level0)
{
    HpaeNodeInfo pre;
    HpaeNodeInfo out;
    ConfigScenarioC(pre, out);

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    ASSERT_NE(cluster->converterNode_, nullptr);
    ASSERT_NE(cluster->zeroPadNode_, nullptr);
    EXPECT_EQ(cluster->extractNode_, nullptr);
    EXPECT_EQ(cluster->head_, cluster->converterNode_);
    EXPECT_EQ(cluster->tail_, cluster->zeroPadNode_);
    EXPECT_EQ(cluster->converterNode_->channelConverter_.GetOutChannelInfo().numChannels, 4u);
    EXPECT_EQ(cluster->zeroPadNode_->GetChannelCount(), 6u);
    EXPECT_EQ(cluster->zeroPadNode_->GetPreOutNum(), 1u);     // internal link wired
    // cluster-level node info stays the app total (parity with the old converter behavior)
    EXPECT_EQ(cluster->GetChannelCount(), 6u);
}

/*
 * @tc.name  : MicInCluster_Compose_003
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_Compose_003
 * @tc.desc  : Scenario A (app wants micIn AND HAL has it): no mode nodes, head == tail == converter.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_Compose_003, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(6);
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = static_cast<AudioChannel>(6);
    out.sourceType = SOURCE_TYPE_CAMCORDER;
    out.micInChannels = static_cast<AudioChannel>(2);
    out.appMicInChannelCount = 2;

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    EXPECT_EQ(cluster->extractNode_, nullptr);
    EXPECT_EQ(cluster->zeroPadNode_, nullptr);
    ASSERT_NE(cluster->converterNode_, nullptr);
    EXPECT_EQ(cluster->head_, cluster->converterNode_);
    EXPECT_EQ(cluster->tail_, cluster->converterNode_);
}

/*
 * @tc.name  : MicInCluster_Compose_004
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_Compose_004
 * @tc.desc  : Scenario D (non-camcorder plain stream): single converter, identical to bare node.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_Compose_004, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = STEREO;
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = STEREO;

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    EXPECT_EQ(cluster->extractNode_, nullptr);
    EXPECT_EQ(cluster->zeroPadNode_, nullptr);
    ASSERT_NE(cluster->converterNode_, nullptr);
    EXPECT_EQ(cluster->head_, cluster->converterNode_);
    EXPECT_EQ(cluster->tail_, cluster->converterNode_);
}

/*
 * @tc.name  : MicInCluster_Latency_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_Latency_001
 * @tc.desc  : GetLatency forwards the inner converter latency: 0 at rest; with the inner
 *             converter buffering input (48k->44.1k scenario C chain) the cluster reports
 *             exactly the inner converter's latency (extract/zero-pad add none).
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_Latency_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    HpaeNodeInfo out;
    ConfigScenarioC(pre, out);

    auto rest = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    EXPECT_EQ(rest->GetLatency(), 0u);

    // 48k in -> 44.1k out scenario C: 20ms out frame = 882
    out.samplingRate = SAMPLE_RATE_44100;
    out.frameLen = FRAME_LEN_20MS * SAMPLE_RATE_44100 / MILLISECOND_PER_SECOND;
    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    std::vector<float> warm(480, 0.5f); // 120 frames @ 4ch
    ASSERT_TRUE(cluster->converterNode_->AppendToUnprocessedBuffer(warm.data(), 120, 4));
    EXPECT_GT(cluster->GetLatency(), 0u);
    EXPECT_EQ(cluster->GetLatency(), cluster->converterNode_->GetLatency());
}

/*
 * @tc.name  : MicInCluster_E2E_ScenarioB_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_E2E_ScenarioB_001
 * @tc.desc  : Chain e2e scenario B (same rate): partitioned 6ch in -> 4ch main out.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_E2E_ScenarioB_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    HpaeNodeInfo out;
    ConfigScenarioB(pre, out);

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    HpaeNodeInfo upstreamInfo = pre;
    auto upstream = std::make_shared<FakeUpstreamNode>(upstreamInfo);
    cluster->Connect(upstream);

    PcmBufferInfo pcmInfo(6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;   // main block
    }
    for (size_t i = DEFAULT_FRAMELEN_SECOND * 4; i < DEFAULT_FRAMELEN_SECOND * 6; i++) {
        input.GetPcmDataBuffer()[i] = 0.9f;   // micIn block (must be dropped)
    }
    upstream->outputStream_.WriteDataToOutput(&input);

    HpaePcmBuffer *result = cluster->GetOutputPort()->PullOutputData();
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetChannelCount(), 4u);
    ASSERT_EQ(result->GetFrameLen(), DEFAULT_FRAMELEN_SECOND);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        EXPECT_FLOAT_EQ(result->GetPcmDataBuffer()[i], 0.5f);
    }
}

/*
 * @tc.name  : MicInCluster_E2E_ScenarioC_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_E2E_ScenarioC_001
 * @tc.desc  : Chain e2e scenario C (same rate): main 4ch in -> total 6ch [main block][zero micIn].
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_E2E_ScenarioC_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    HpaeNodeInfo out;
    ConfigScenarioC(pre, out);

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    HpaeNodeInfo upstreamInfo = pre;
    auto upstream = std::make_shared<FakeUpstreamNode>(upstreamInfo);
    cluster->Connect(upstream);

    PcmBufferInfo pcmInfo(4, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }
    upstream->outputStream_.WriteDataToOutput(&input);

    HpaePcmBuffer *result = cluster->GetOutputPort()->PullOutputData();
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetChannelCount(), 6u);
    ASSERT_EQ(result->GetFrameLen(), DEFAULT_FRAMELEN_SECOND);
    size_t mainRegion = DEFAULT_FRAMELEN_SECOND * 4;
    size_t totalRegion = DEFAULT_FRAMELEN_SECOND * 6;
    for (size_t i = 0; i < mainRegion; i++) {
        EXPECT_FLOAT_EQ(result->GetPcmDataBuffer()[i], 0.5f);
    }
    for (size_t i = mainRegion; i < totalRegion; i++) {
        EXPECT_FLOAT_EQ(result->GetPcmDataBuffer()[i], 0.0f);
    }
}

/*
 * @tc.name  : MicInCluster_E2E_ScenarioC_RateConv_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_E2E_ScenarioC_RateConv_001
 * @tc.desc  : Chain e2e scenario C with 48k->44.1k: main(4) resampled to 882 frames,
 *             micIn(2) zero-filled; output partitioned [main 4x882][zero 2x882].
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_E2E_ScenarioC_RateConv_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(4);
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_44100;
    out.frameLen = 882;
    out.channels = static_cast<AudioChannel>(6);
    out.sourceType = SOURCE_TYPE_CAMCORDER;
    out.micInChannels = CHANNEL_UNKNOW;
    out.appMicInChannelCount = 2;

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    HpaeNodeInfo upstreamInfo = pre;
    auto upstream = std::make_shared<FakeUpstreamNode>(upstreamInfo);
    cluster->Connect(upstream);

    PcmBufferInfo pcmInfo(4, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }

    // Warm-up: ProResampler skips half taps on first use. Discard the first output chunk.
    upstream->outputStream_.WriteDataToOutput(&input);
    ASSERT_NE(cluster->GetOutputPort()->PullOutputData(), nullptr);
    upstream->outputStream_.WriteDataToOutput(&input);
    HpaePcmBuffer *result = cluster->GetOutputPort()->PullOutputData();

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetChannelCount(), 6u);
    ASSERT_EQ(result->GetFrameLen(), 882u);
    size_t mainRegion = static_cast<size_t>(882) * 4;
    size_t totalRegion = static_cast<size_t>(882) * 6;
    for (size_t i = 0; i < mainRegion; i++) {
        EXPECT_NEAR(result->GetPcmDataBuffer()[i], 0.5f, 0.1f);
    }
    for (size_t i = mainRegion; i < totalRegion; i++) {
        EXPECT_FLOAT_EQ(result->GetPcmDataBuffer()[i], 0.0f);
    }
}

/*
 * @tc.name  : MicInCluster_E2E_ScenarioC_Upmix_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_E2E_ScenarioC_Upmix_001
 * @tc.desc  : Scenario C edge: preNodeInfo.channels(2) < main(4) -> converter upmixes, micIn stays zero.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_E2E_ScenarioC_Upmix_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(2);   // HAL main = 2 < required main 4
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = static_cast<AudioChannel>(6);   // total = main4 + micIn2
    out.sourceType = SOURCE_TYPE_CAMCORDER;
    out.micInChannels = CHANNEL_UNKNOW;
    out.appMicInChannelCount = 2;

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    ASSERT_EQ(cluster->converterNode_->channelConverter_.GetInChannelInfo().numChannels, 2u);
    ASSERT_EQ(cluster->converterNode_->channelConverter_.GetOutChannelInfo().numChannels, 4u);
    HpaeNodeInfo upstreamInfo = pre;
    auto upstream = std::make_shared<FakeUpstreamNode>(upstreamInfo);
    cluster->Connect(upstream);

    PcmBufferInfo pcmInfo(2, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 2; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }
    upstream->outputStream_.WriteDataToOutput(&input);

    HpaePcmBuffer *result = cluster->GetOutputPort()->PullOutputData();
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetChannelCount(), 6u);
    ASSERT_EQ(result->GetFrameLen(), DEFAULT_FRAMELEN_SECOND);
    // micIn block fully zero regardless of the upmix result on the main block
    size_t mainRegion = DEFAULT_FRAMELEN_SECOND * 4;
    size_t totalRegion = DEFAULT_FRAMELEN_SECOND * 6;
    for (size_t i = mainRegion; i < totalRegion; i++) {
        EXPECT_FLOAT_EQ(result->GetPcmDataBuffer()[i], 0.0f);
    }
    float mainSum = 0.0f;
    for (size_t i = 0; i < mainRegion; i++) {
        mainSum += std::fabs(result->GetPcmDataBuffer()[i]);
    }
    EXPECT_GT(mainSum, 0.0f);   // upmixed main block is live (not silenced/garbled to zero)
}

/*
 * @tc.name  : MicInCluster_E2E_ScenarioA_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_E2E_ScenarioA_001
 * @tc.desc  : Scenario A regression: full-channel path, no partition in output.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_E2E_ScenarioA_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = static_cast<AudioChannel>(6);
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = static_cast<AudioChannel>(6);
    out.sourceType = SOURCE_TYPE_CAMCORDER;
    out.micInChannels = static_cast<AudioChannel>(2);
    out.appMicInChannelCount = 2;

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    ASSERT_EQ(cluster->head_, cluster->converterNode_);
    HpaeNodeInfo upstreamInfo = pre;
    auto upstream = std::make_shared<FakeUpstreamNode>(upstreamInfo);
    cluster->Connect(upstream);

    PcmBufferInfo pcmInfo(6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 6; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }
    upstream->outputStream_.WriteDataToOutput(&input);

    HpaePcmBuffer *result = cluster->GetOutputPort()->PullOutputData();
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetChannelCount(), 6u);
    ASSERT_EQ(result->GetFrameLen(), DEFAULT_FRAMELEN_SECOND);
    // full-channel passthrough: every interleaved sample preserved (no zero tail)
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 6; i++) {
        EXPECT_FLOAT_EQ(result->GetPcmDataBuffer()[i], 0.5f);
    }
}

/*
 * @tc.name  : MicInCluster_E2E_InjectorTag_001
 * @tc.type  : FUNC
 * @tc.number: MicInCluster_E2E_InjectorTag_001
 * @tc.desc  : Chain e2e scenario C with INJECTOR tag: cluster->SetSourceBufferType rides the
 *             final output buffer pulled from the tail.
 */
HWTEST_F(HpaeAudioFormatConverterClusterTest, MicInCluster_E2E_InjectorTag_001, TestSize.Level0)
{
    HpaeNodeInfo pre;
    HpaeNodeInfo out;
    ConfigScenarioC(pre, out);

    auto cluster = std::make_shared<HpaeAudioFormatConverterCluster>(pre, out);
    cluster->SetSourceBufferType(HPAE_SOURCE_BUFFER_TYPE_INJECTOR);
    HpaeNodeInfo upstreamInfo = pre;
    auto upstream = std::make_shared<FakeUpstreamNode>(upstreamInfo);
    cluster->Connect(upstream);

    PcmBufferInfo pcmInfo(4, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.SetBufferValid(true);
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * 4; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }
    upstream->outputStream_.WriteDataToOutput(&input);

    HpaePcmBuffer *result = cluster->GetOutputPort()->PullOutputData();
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetChannelCount(), 6u);
    // the injector tag must ride the tail (zero-pad) output buffer, not be lost on the shell
    EXPECT_EQ(result->GetSourceBufferType(), HPAE_SOURCE_BUFFER_TYPE_INJECTOR);
}
