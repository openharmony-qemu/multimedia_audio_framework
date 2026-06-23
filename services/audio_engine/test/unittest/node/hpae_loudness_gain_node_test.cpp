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
#include <cmath>
#include <cstring>
#include <dlfcn.h>
#include <memory>
#include "hpae_loudness_gain_node.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeLoudnessGainNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeLoudnessGainNodeTest::SetUp()
{}

void HpaeLoudnessGainNodeTest::TearDown()
{}

namespace {

constexpr uint32_t TEST_ID = 1234;
constexpr uint32_t TEST_FRAMELEN = 960;
constexpr int TIMES = 5;
constexpr float LOUDNESS_GAIN_VALUE = 10.0f;
constexpr float EPSILON_TOLERANCE = 1e-4f;
constexpr uint32_t TEST_FRAMELEN_96K = 1920;
constexpr uint32_t TEST_FRAMELEN_192K = 3840;

static void FillBufferWithValue(HpaePcmBuffer& buffer, float value)
{
    float* data = buffer.GetPcmDataBuffer();
    uint32_t totalSamples = buffer.GetFrameLen() * buffer.GetChannelCount();
    for (uint32_t i = 0; i < totalSamples; i++) {
        data[i] = value;
    }
}

int32_t FakeLoudnessProcess(AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer)
{
    (void)self;
    if (inBuffer == nullptr || outBuffer == nullptr || inBuffer->raw == nullptr || outBuffer->raw == nullptr) {
        return ERROR;
    }
    // The production node passes frameLength only; tests only need a deterministic success path.
    if (inBuffer->raw != outBuffer->raw) {
        (void)memcpy_s(outBuffer->raw, inBuffer->frameLength * STEREO * sizeof(float), inBuffer->raw,
            inBuffer->frameLength * STEREO * sizeof(float));
    }
    return SUCCESS;
}

int32_t FakeLoudnessCommand(AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
    AudioEffectTransInfo *replyInfo)
{
    (void)self;
    (void)cmdCode;
    (void)cmdInfo;
    if (replyInfo != nullptr && replyInfo->data != nullptr && replyInfo->size >= sizeof(int32_t)) {
        *static_cast<int32_t *>(replyInfo->data) = SUCCESS;
    }
    return SUCCESS;
}

int32_t FakeLoudnessCreateEffect(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
{
    (void)descriptor;
    if (handle == nullptr) {
        return ERROR;
    }
    static AudioEffectInterface fakeInterface = {
        .process = FakeLoudnessProcess,
        .command = FakeLoudnessCommand,
        .feedInput = nullptr,
        .getOutput = nullptr,
    };
    auto fakeHandle = new AudioEffectInterface *;
    *fakeHandle = &fakeInterface;
    *handle = fakeHandle;
    return SUCCESS;
}

int32_t FakeLoudnessReleaseEffect(AudioEffectHandle handle)
{
    delete handle;
    return SUCCESS;
}

AudioEffectLibrary g_fakeLoudnessLibrary = {
    .version = 1,
    .name = "fake_loudness",
    .implementor = "unit_test",
    .checkEffect = nullptr,
    .createEffect = FakeLoudnessCreateEffect,
    .releaseEffect = FakeLoudnessReleaseEffect,
    .supportEffect = nullptr,
};

void InjectFakeDspLibrary(const std::shared_ptr<HpaeLoudnessGainNode> &node)
{
    if (node->dlHandle_ != nullptr) {
        dlclose(node->dlHandle_);
    }
    node->dlHandle_ = dlopen(nullptr, RTLD_NOW);
    ASSERT_NE(node->dlHandle_, nullptr);
    node->audioEffectLibHandle_ = &g_fakeLoudnessLibrary;
}

HWTEST_F(HpaeLoudnessGainNodeTest, testLoudnessGainNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeLoudnessGainNode> hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    std::vector<HpaePcmBuffer*> inputs;
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    inputs.emplace_back(&hpaePcmBuffer);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(0.0f), SUCCESS);
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), 0.0f);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE), SUCCESS);
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), LOUDNESS_GAIN_VALUE);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(0.0f), SUCCESS);
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), 0.0f);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    std::vector<HpaePcmBuffer*> inputs1;
    PcmBufferInfo pcmBufferInfo1(CHANNEL_6, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer1(pcmBufferInfo1);
    inputs1.emplace_back(&hpaePcmBuffer1);
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE), SUCCESS);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs1);
    }
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), LOUDNESS_GAIN_VALUE);
}

/**
 * @tc.name  : FaultCode_ReleaseHandle_NullHandle
 * @tc.type  : FUNC
 * @tc.desc  : Test ReleaseHandle returns ERROR when handle_ is nullptr,
 *             covering PLAY_CREATE_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_ReleaseHandle_NullHandle, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // handle_ is nullptr by default (no dl handle loaded in UT)
    int32_t result = hpaeLoudnessGainNode->ReleaseHandle(0.0f);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_SetLoudnessGain_SameGain
 * @tc.type  : FUNC
 * @tc.desc  : Test SetLoudnessGain returns SUCCESS when setting same gain value,
 *             covering PLAY_CREATE_INVALID_PARAM fault code path for same-value skip.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_SetLoudnessGain_SameGain, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Initial gain is 0.0f, setting same value should return SUCCESS
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(0.0f), SUCCESS);
}

/**
 * @tc.name  : FaultCode_SignalProcess_EmptyInputs
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess returns &silenceData_ when inputs is empty,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_SignalProcess_EmptyInputs, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Set gain to non-zero to avoid early return at gain==0 check
    hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE);

    // Empty inputs vector should return &silenceData_
    std::vector<HpaePcmBuffer*> emptyInputs;
    HpaePcmBuffer* result = hpaeLoudnessGainNode->SignalProcess(emptyInputs);
    EXPECT_EQ(result, &hpaeLoudnessGainNode->silenceData_);
}

/**
 * @tc.name  : FaultCode_SignalProcess_NullFirstInput
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess returns &silenceData_ when first input is nullptr,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_SignalProcess_NullFirstInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Set gain to non-zero
    hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE);

    // First input is nullptr
    std::vector<HpaePcmBuffer*> inputs = {nullptr};
    HpaePcmBuffer* result = hpaeLoudnessGainNode->SignalProcess(inputs);
    EXPECT_EQ(result, &hpaeLoudnessGainNode->silenceData_);
}

/**
* @tc.name  : dynamicRateSwitch48kTo96k_001
* @tc.type  : FUNC
* @tc.desc  : Test dynamic sample rate switch from 48kHz to 96kHz updates internal pcmBufferInfo_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, dynamicRateSwitch48kTo96k_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Process at 48kHz first
    PcmBufferInfo pcmBufferInfo48k(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer buffer48k(pcmBufferInfo48k);
    FillBufferWithValue(buffer48k, 1.0f);
    std::vector<HpaePcmBuffer*> inputs48k = {&buffer48k};
    node->SignalProcess(inputs48k);

    node->SetLoudnessGain(3.0f);

    // Process at 96kHz
    PcmBufferInfo pcmBufferInfo96k(STEREO, TEST_FRAMELEN_96K, SAMPLE_RATE_96000);
    HpaePcmBuffer buffer96k(pcmBufferInfo96k);
    FillBufferWithValue(buffer96k, 1.0f);
    std::vector<HpaePcmBuffer*> inputs96k = {&buffer96k};
    node->SignalProcess(inputs96k);

    EXPECT_EQ(node->pcmBufferInfo_.rate, SAMPLE_RATE_96000);
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 3.0f);
}

/**
* @tc.name  : gainZeroPassthrough_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess returns input directly when gain is 0.0 dB
*/
HWTEST_F(HpaeLoudnessGainNodeTest, gainZeroPassthrough_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 0.75f);

    // Default gain is 0.0f
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* result = node->SignalProcess(inputs);

    EXPECT_EQ(result, inputs[0]);
}

/**
* @tc.name  : getLatency_001
* @tc.type  : FUNC
* @tc.desc  : Test GetLatency returns 0 when handle_ is null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, getLatency_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    EXPECT_EQ(node->GetLatency(), 0);
}

/**
* @tc.name  : getLatencyWithSessionId_001
* @tc.type  : FUNC
* @tc.desc  : Test GetLatency with sessionId returns 0 when handle_ is null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, getLatencyWithSessionId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    EXPECT_EQ(node->GetLatency(12345), 0);
}

/**
* @tc.name  : setLoudnessGainUpdatesLinearGain_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain correctly updates internal linearGain_ member
*/
HWTEST_F(HpaeLoudnessGainNodeTest, setLoudnessGainUpdatesLinearGain_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    node->SetLoudnessGain(0.0f);
    EXPECT_LT(fabs(node->linearGain_ - 1.0f), EPSILON_TOLERANCE);
}

/**
* @tc.name  : isLoudnessAlgoOn_ReturnsFalse_001
* @tc.type  : FUNC
* @tc.desc  : Test IsLoudnessAlgoOn returns false when handle_ is nullptr (linear fallback mode)
*/
HWTEST_F(HpaeLoudnessGainNodeTest, isLoudnessAlgoOn_ReturnsFalse_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    EXPECT_FALSE(node->IsLoudnessAlgoOn());

    node->SetLoudnessGain(6.0f);
    EXPECT_FALSE(node->IsLoudnessAlgoOn());

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SetLoudnessGain_PositiveGain_UpdatesLinearGain_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain with positive non-zero gain correctly updates linearGain_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SetLoudnessGain_PositiveGain_UpdatesLinearGain_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    node->SetLoudnessGain(6.0f);
    EXPECT_LT(fabs(node->linearGain_ - pow(10.0f, 6.0f / 20.0f)), EPSILON_TOLERANCE);
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 6.0f);

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SetLoudnessGain_NegativeGain_UpdatesLinearGain_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain with negative gain correctly updates linearGain_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SetLoudnessGain_NegativeGain_UpdatesLinearGain_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    node->SetLoudnessGain(-3.0f);
    EXPECT_LT(fabs(node->linearGain_ - pow(10.0f, -3.0f / 20.0f)), EPSILON_TOLERANCE);
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), -3.0f);

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SignalProcess_LinearGainOutput_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess linear gain multiplication produces correct output buffer values with +20dB
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SignalProcess_LinearGainOutput_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(20.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples; i++) {
        EXPECT_LT(fabs(outData[i] - 10.0f), EPSILON_TOLERANCE);
    }

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SignalProcess_OutputBufferPointer_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess linear gain path returns address of loudnessGainOutput_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SignalProcess_OutputBufferPointer_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 0.5f);

    node->SetLoudnessGain(6.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* result = node->SignalProcess(inputs);

    EXPECT_EQ(result, &node->loudnessGainOutput_);

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SignalProcess_96kHzLinearGain_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess at 96kHz with linear gain fallback produces correct output buffer values
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SignalProcess_96kHzLinearGain_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN_96K;
    nodeInfo.samplingRate = SAMPLE_RATE_96000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN_96K, SAMPLE_RATE_96000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(6.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float expectedGain = pow(10, 6.0f / 20);
    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples && i < 16; i++) {
        EXPECT_LT(fabs(outData[i] - expectedGain), EPSILON_TOLERANCE);
    }

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SignalProcess_192kHzLinearGain_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess at 192kHz with linear gain fallback produces correct output buffer values
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SignalProcess_192kHzLinearGain_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN_192K;
    nodeInfo.samplingRate = SAMPLE_RATE_192000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN_192K, SAMPLE_RATE_192000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(6.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float expectedGain = pow(10, 6.0f / 20);
    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples && i < 16; i++) {
        EXPECT_LT(fabs(outData[i] - expectedGain), EPSILON_TOLERANCE);
    }

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : CheckUpdateInfo_ChannelChange_001
* @tc.type  : FUNC
* @tc.desc  : Test CheckUpdateInfo triggers ReConfig when channel count changes from STEREO to CHANNEL_6
*/
HWTEST_F(HpaeLoudnessGainNodeTest, CheckUpdateInfo_ChannelChange_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    node->SetLoudnessGain(6.0f);

    PcmBufferInfo pcmBufferInfo(CHANNEL_6, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    node->SignalProcess(inputs);

    EXPECT_EQ(node->pcmBufferInfo_.ch, static_cast<uint32_t>(CHANNEL_6));

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : CheckUpdateInfo_FrameLenChange_001
* @tc.type  : FUNC
* @tc.desc  : Test CheckUpdateInfo triggers ReConfig when frameLen changes from 960 to 480
*/
HWTEST_F(HpaeLoudnessGainNodeTest, CheckUpdateInfo_FrameLenChange_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    node->SetLoudnessGain(6.0f);

    PcmBufferInfo pcmBufferInfo(STEREO, 480, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    node->SignalProcess(inputs);

    EXPECT_EQ(node->pcmBufferInfo_.frameLen, static_cast<uint32_t>(480));

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : CheckUpdateInfo_ChannelLayoutChange_001
* @tc.type  : FUNC
* @tc.desc  : Test CheckUpdateInfo triggers ReConfig when channelLayout changes
*/
HWTEST_F(HpaeLoudnessGainNodeTest, CheckUpdateInfo_ChannelLayoutChange_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    node->SetLoudnessGain(6.0f);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000, 3);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    node->SignalProcess(inputs);

    EXPECT_EQ(node->pcmBufferInfo_.channelLayout, static_cast<uint64_t>(3));

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : Constructor_InitialState_001
* @tc.type  : FUNC
* @tc.desc  : Test constructor initializes all key members to expected default values
*/
HWTEST_F(HpaeLoudnessGainNodeTest, Constructor_InitialState_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    EXPECT_EQ(node->handle_, nullptr);
    EXPECT_EQ(node->audioEffectLibHandle_ != nullptr, node->dlHandle_ != nullptr);
    EXPECT_FLOAT_EQ(node->loudnessGain_, 0.0f);
    EXPECT_LT(fabs(node->linearGain_ - 1.0f), EPSILON_TOLERANCE);
    EXPECT_EQ(node->pcmBufferInfo_.ch, static_cast<uint32_t>(STEREO));
    EXPECT_EQ(node->pcmBufferInfo_.frameLen, static_cast<uint32_t>(TEST_FRAMELEN));
    EXPECT_EQ(node->pcmBufferInfo_.rate, static_cast<uint32_t>(SAMPLE_RATE_48000));
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 0.0f);
}

/**
* @tc.name  : SetLoudnessGain_MultipleUpdates_001
* @tc.type  : FUNC
* @tc.desc  : Test multiple SetLoudnessGain calls each correctly update linearGain_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SetLoudnessGain_MultipleUpdates_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    void* savedDlHandle = node->dlHandle_;
    node->dlHandle_ = nullptr;

    node->SetLoudnessGain(0.0f);
    EXPECT_LT(fabs(node->linearGain_ - 1.0f), EPSILON_TOLERANCE);

    node->SetLoudnessGain(6.0f);
    EXPECT_LT(fabs(node->linearGain_ - pow(10.0f, 6.0f / 20.0f)), EPSILON_TOLERANCE);

    node->SetLoudnessGain(-3.0f);
    EXPECT_LT(fabs(node->linearGain_ - pow(10.0f, -3.0f / 20.0f)), EPSILON_TOLERANCE);

    node->SetLoudnessGain(20.0f);
    EXPECT_LT(fabs(node->linearGain_ - 10.0f), EPSILON_TOLERANCE);

    node->SetLoudnessGain(0.0f);
    EXPECT_LT(fabs(node->linearGain_ - 1.0f), EPSILON_TOLERANCE);

    node->dlHandle_ = savedDlHandle;
}

/**
* @tc.name  : SignalProcess_DspPath_NullHandle_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess DSP path returns inputs[0] when handle_ is null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SignalProcess_DspPath_NullHandle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);
    // dlHandle_ is non-null (DSP loaded), handle_ is null

    // Directly set loudnessGain_ to bypass SetLoudnessGain, keeping handle_ null
    node->loudnessGain_ = 6.0f;

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};

    // DSP path: dlHandle_ non-null -> else branch -> CHECK_AND_RETURN_RET(handle_, inputs[0])
    HpaePcmBuffer* result = node->SignalProcess(inputs);
    EXPECT_EQ(result, inputs[0]);
}

/**
* @tc.name  : SetLoudnessGain_DspPath_CreateHandle_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain DSP path creates handle_ and updates loudnessGain_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SetLoudnessGain_DspPath_CreateHandle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);
    // dlHandle_ is non-null, takes DSP path

    int32_t ret = node->SetLoudnessGain(6.0f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(node->handle_, nullptr);
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 6.0f);
    EXPECT_TRUE(node->IsLoudnessAlgoOn());
}

/**
* @tc.name  : IsLoudnessAlgoOn_DspPath_WithHandle_001
* @tc.type  : FUNC
* @tc.desc  : Test IsLoudnessAlgoOn returns true when handle_ is created, false after release
*/
HWTEST_F(HpaeLoudnessGainNodeTest, IsLoudnessAlgoOn_DspPath_WithHandle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);

    EXPECT_FALSE(node->IsLoudnessAlgoOn());

    node->SetLoudnessGain(6.0f);
    EXPECT_TRUE(node->IsLoudnessAlgoOn());

    node->SetLoudnessGain(0.0f);
    EXPECT_FALSE(node->IsLoudnessAlgoOn());
}

/**
* @tc.name  : GetLatency_DspPath_WithHandle_001
* @tc.type  : FUNC
* @tc.desc  : Test GetLatency returns DEFAULT_LATENCY_IN_US when handle_ is non-null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, GetLatency_DspPath_WithHandle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);

    node->SetLoudnessGain(6.0f);
    EXPECT_NE(node->handle_, nullptr);

    EXPECT_EQ(node->GetLatency(), 5000);
}

/**
* @tc.name  : SignalProcess_DspPath_WithHandle_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess DSP path calls process() when handle_ is non-null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SignalProcess_DspPath_WithHandle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);

    node->SetLoudnessGain(6.0f);
    ASSERT_NE(node->handle_, nullptr);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};

    // DSP path with handle_ non-null -> process() called -> returns &loudnessGainOutput_
    HpaePcmBuffer* result = node->SignalProcess(inputs);
    EXPECT_EQ(result, &node->loudnessGainOutput_);
}

/**
* @tc.name  : SetLoudnessGain_DspPath_GainChange_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain DSP path with existing handle_ updates gain via SET_PARAM
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SetLoudnessGain_DspPath_GainChange_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);

    // First create handle_ via SetLoudnessGain(6.0f)
    ASSERT_EQ(node->SetLoudnessGain(6.0f), SUCCESS);
    ASSERT_NE(node->handle_, nullptr);

    // Change gain to 10.0f -> SET_PARAM only (skips createEffect since handle_ exists)
    int32_t ret = node->SetLoudnessGain(10.0f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 10.0f);
    EXPECT_TRUE(node->IsLoudnessAlgoOn());
}

/**
* @tc.name  : SetLoudnessGain_DspPath_ZeroGain_ReleaseHandle_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain(0) through DSP path releases handle_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, SetLoudnessGain_DspPath_ZeroGain_ReleaseHandle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);

    ASSERT_EQ(node->SetLoudnessGain(6.0f), SUCCESS);
    ASSERT_NE(node->handle_, nullptr);

    // Set gain to 0 -> ReleaseHandle path
    int32_t ret = node->SetLoudnessGain(0.0f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node->handle_, nullptr);
    EXPECT_FALSE(node->IsLoudnessAlgoOn());
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 0.0f);
}

/**
* @tc.name  : ReleaseHandle_DspPath_Success_001
* @tc.type  : FUNC
* @tc.desc  : Test ReleaseHandle succeeds when handle_ is non-null through DSP path
*/
HWTEST_F(HpaeLoudnessGainNodeTest, ReleaseHandle_DspPath_Success_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    InjectFakeDspLibrary(node);

    ASSERT_EQ(node->SetLoudnessGain(6.0f), SUCCESS);
    ASSERT_NE(node->handle_, nullptr);

    int32_t ret = node->ReleaseHandle(0.0f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node->handle_, nullptr);
}
}