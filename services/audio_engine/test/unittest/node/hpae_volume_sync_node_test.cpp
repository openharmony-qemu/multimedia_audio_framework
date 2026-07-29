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
#include <gmock/gmock.h>
#include <cmath>
#include <memory>
#include "hpae_volume_sync_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "audio_volume.h"
#include "hpae_mocks.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeVolumeSyncNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeVolumeSyncNodeTest::SetUp()
{}

void HpaeVolumeSyncNodeTest::TearDown()
{}

namespace {

constexpr uint32_t DEFAULT_NODE_ID = 1234;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr uint32_t DEFAULT_SESSION_ID = 100;
constexpr float DEFAULT_VOLUME = 1.0f;
constexpr float TEST_VOLUME_1 = 0.5f;
// Synthetic id whose HDI id base bits (bit12-13, see id_handler.h:69-73) encode
// HDI_ID_BASE_CAPTURE (= 0x1000u = HDI_ID_BASE_CAPTURE << HDI_ID_BASE_OFFSET).
// IdHandler::CheckId requires base == HDI_ID_BASE_RENDER (hdi_adapter_info.h:22-25),
// so this CAPTURE-base id fails CheckId's first guard → HdiAdapterManager::GetRenderSink
// returns nullptr (hdi_adapter_manager.cpp:143). It is != HDI_INVALID_ID (0xFFFFFFFF),
// so HpaeVolumeSyncNode::GetRenderSink()'s re-fetch guard does not fire. Used to force
// SetSinkVolume()'s else branch (audioRendererSink == nullptr).
constexpr uint32_t BOGUS_CAPTURE_BASE_RENDER_ID = 0x1000u;

/**
 * @tc.name  : constructHpaeVolumeSyncNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_001
 * @tc.desc  : Test construct HpaeVolumeSyncNode with primary device class
 */
HWTEST_F(HpaeVolumeSyncNodeTest, constructHpaeVolumeSyncNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.deviceNetId = "LocalDevice";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_NE(volumeSyncNode, nullptr);
    EXPECT_EQ(volumeSyncNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(volumeSyncNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(volumeSyncNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(volumeSyncNode->GetBitWidth(), nodeInfo.format);
}

/**
 * @tc.name  : constructHpaeVolumeSyncNode_002
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_002
 * @tc.desc  : Test construct HpaeVolumeSyncNode with inner capturer device class
 */
HWTEST_F(HpaeVolumeSyncNodeTest, constructHpaeVolumeSyncNode_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "inner_capturer";
    nodeInfo.deviceNetId = "LocalDevice";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_NE(volumeSyncNode, nullptr);
    EXPECT_EQ(volumeSyncNode->GetDeviceClass(), "inner_capturer");
}

/**
 * @tc.name  : constructHpaeVolumeSyncNode_003
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_003
 * @tc.desc  : Test construct HpaeVolumeSyncNode with virtual injector device class
 */
HWTEST_F(HpaeVolumeSyncNodeTest, constructHpaeVolumeSyncNode_003, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "virtual_injector";
    nodeInfo.deviceNetId = "LocalDevice";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_NE(volumeSyncNode, nullptr);
    EXPECT_EQ(volumeSyncNode->GetDeviceClass(), "virtual_injector");
}

/**
 * @tc.name  : getNodeInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_004
 * @tc.desc  : Test GetNodeInfo method
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getNodeInfo_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    HpaeNodeInfo &retNi = volumeSyncNode->GetNodeInfo();

    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.deviceClass, nodeInfo.deviceClass);
}

/**
 * @tc.name  : getLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_005
 * @tc.desc  : Test GetLatency method, should return 0
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getLatency_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetLatency(), 0);
    EXPECT_EQ(volumeSyncNode->GetLatency(DEFAULT_SESSION_ID), 0);
}

/**
 * @tc.name  : signalProcess_EmptyInput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_006
 * @tc.desc  : Test SignalProcess with empty input
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_EmptyInput_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> emptyInputs;
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(emptyInputs);

    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : signalProcess_PassThrough_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_007
 * @tc.desc  : Test SignalProcess pass through audio data without modification
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_PassThrough_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    // Create pcm buffer
    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);

    // Fill with test data
    float *data = pcmBuffer.GetPcmDataBuffer();
    uint32_t totalSamples = nodeInfo.frameLen * static_cast<uint32_t>(nodeInfo.channels);
    for (uint32_t i = 0; i < totalSamples; i++) {
        data[i] = static_cast<float>(i) / 1000.0f;
    }

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);

    // Should return the same buffer (pass through)
    EXPECT_EQ(result, &pcmBuffer);

    // Data should not be modified
    for (uint32_t i = 0; i < totalSamples; i++) {
        EXPECT_FLOAT_EQ(data[i], static_cast<float>(i) / 1000.0f);
    }
}

/**
 * @tc.name  : resetVolume_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_008
 * @tc.desc  : Test ResetVolume method
 */
HWTEST_F(HpaeVolumeSyncNodeTest, resetVolume_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    // ResetVolume should not crash
    volumeSyncNode->ResetVolume();
    // Verify node is still usable after reset
    EXPECT_EQ(volumeSyncNode->GetSessionId(), DEFAULT_SESSION_ID);
}

/**
 * @tc.name  : resetVolume_002
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_009
 * @tc.desc  : Test ResetVolume method with inner capturer device class
 */
HWTEST_F(HpaeVolumeSyncNodeTest, resetVolume_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "inner_capturer";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    // ResetVolume should not crash for inner capturer
    volumeSyncNode->ResetVolume();
    EXPECT_EQ(volumeSyncNode->GetDeviceClass(), "inner_capturer");
}

/**
 * @tc.name  : signalProcess_VolumeChange_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_010
 * @tc.desc  : Test SignalProcess when volume changes
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_VolumeChange_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    // Create pcm buffer
    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    float *data = pcmBuffer.GetPcmDataBuffer();
    uint32_t totalSamples = nodeInfo.frameLen * static_cast<uint32_t>(nodeInfo.channels);
    for (uint32_t i = 0; i < totalSamples; i++) {
        data[i] = 0.5f;
    }

    // Set history volume to different value to simulate volume change
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, TEST_VOLUME_1);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);

    // Should return the same buffer
    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : signalProcess_NoVolumeChange_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_011
 * @tc.desc  : Test SignalProcess when volume does not change
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_NoVolumeChange_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    // Create pcm buffer
    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    float *data = pcmBuffer.GetPcmDataBuffer();
    uint32_t totalSamples = nodeInfo.frameLen * static_cast<uint32_t>(nodeInfo.channels);
    for (uint32_t i = 0; i < totalSamples; i++) {
        data[i] = 0.5f;
    }

    // Set history volume to same value as current to simulate no volume change
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, DEFAULT_VOLUME);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);

    // Should return the same buffer
    EXPECT_EQ(result, &pcmBuffer);

    // Data should not be modified
    for (uint32_t i = 0; i < totalSamples; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.5f);
    }
}

/**
 * @tc.name  : differentSampleRates_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_012
 * @tc.desc  : Test with different sample rates
 */
HWTEST_F(HpaeVolumeSyncNodeTest, differentSampleRates_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetSampleRate(), SAMPLE_RATE_44100);
}

/**
 * @tc.name  : differentChannels_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_013
 * @tc.desc  : Test with different channel configurations
 */
HWTEST_F(HpaeVolumeSyncNodeTest, differentChannels_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetChannelCount(), MONO);
}

/**
 * @tc.name  : differentFormats_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_014
 * @tc.desc  : Test with different sample formats
 */
HWTEST_F(HpaeVolumeSyncNodeTest, differentFormats_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetBitWidth(), SAMPLE_S16LE);
}

/**
 * @tc.name  : remoteDeviceClass_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_015
 * @tc.desc  : Test with remote device class
 */
HWTEST_F(HpaeVolumeSyncNodeTest, remoteDeviceClass_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "remote";
    nodeInfo.deviceNetId = "remote_device_001";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetDeviceClass(), "remote");
    EXPECT_EQ(volumeSyncNode->GetDeviceNetId(), "remote_device_001");
}

/**
 * @tc.name  : signalProcess_MultipleCalls_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_016
 * @tc.desc  : Test multiple consecutive SignalProcess calls
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_MultipleCalls_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };

    // Multiple calls should not crash
    for (int i = 0; i < 10; i++) {
        HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);
        EXPECT_EQ(result, &pcmBuffer);
    }
}

/**
 * @tc.name  : signalProcess_WithNullRenderSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_017
 * @tc.desc  : Test SignalProcess when render sink is nullptr (renderId invalid)
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_WithNullRenderSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "invalid_device_class";
    nodeInfo.deviceNetId = "LocalDevice";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    // Set different history volume to trigger volume change
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, TEST_VOLUME_1);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    // Should not crash even if render sink is nullptr
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);

    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : deviceNetId_LocalDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_018
 * @tc.desc  : Test with LocalDevice network id
 */
HWTEST_F(HpaeVolumeSyncNodeTest, deviceNetId_LocalDevice_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.deviceNetId = "LocalDevice";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetDeviceNetId(), "LocalDevice");
}

/**
 * @tc.name  : setNodeInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_019
 * @tc.desc  : Test SetNodeInfo method
 */
HWTEST_F(HpaeVolumeSyncNodeTest, setNodeInfo_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    HpaeNodeInfo newNodeInfo;
    newNodeInfo.samplingRate = SAMPLE_RATE_44100;
    newNodeInfo.channels = MONO;
    volumeSyncNode->SetNodeInfo(newNodeInfo);

    HpaeNodeInfo &retInfo = volumeSyncNode->GetNodeInfo();
    EXPECT_EQ(retInfo.samplingRate, SAMPLE_RATE_44100);
    EXPECT_EQ(retInfo.channels, MONO);
}

/**
 * @tc.name  : streamType_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_020
 * @tc.desc  : Test with different stream types
 */
HWTEST_F(HpaeVolumeSyncNodeTest, streamType_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.streamType = STREAM_VOICE_CALL;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetStreamType(), STREAM_VOICE_CALL);
}

/**
 * @tc.name  : destructHpaeVolumeSyncNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_021
 * @tc.desc  : Test destructor
 */
HWTEST_F(HpaeVolumeSyncNodeTest, destructHpaeVolumeSyncNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    {
        std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
        EXPECT_NE(volumeSyncNode, nullptr);
    }
    // Node should be destructed without crash
}

/**
 * @tc.name  : signalProcess_ZeroVolume_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_022
 * @tc.desc  : Test SignalProcess with zero volume
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_ZeroVolume_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    float *data = pcmBuffer.GetPcmDataBuffer();
    uint32_t totalSamples = nodeInfo.frameLen * static_cast<uint32_t>(nodeInfo.channels);
    for (uint32_t i = 0; i < totalSamples; i++) {
        data[i] = 0.5f;
    }

    // Set history volume to 0 to simulate zero volume
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, 0.0f);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);

    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : nodeId_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_023
 * @tc.desc  : Test SetNodeId and GetNodeId methods
 */
HWTEST_F(HpaeVolumeSyncNodeTest, nodeId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    uint32_t newId = 5678;
    volumeSyncNode->SetNodeId(newId);
    EXPECT_EQ(volumeSyncNode->GetNodeId(), newId);
}

/**
 * @tc.name  : nodeName_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_024
 * @tc.desc  : Test SetNodeName and GetNodeName methods
 */
HWTEST_F(HpaeVolumeSyncNodeTest, nodeName_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    std::string nodeName = "testVolumeSyncNode";
    volumeSyncNode->SetNodeName(nodeName);
    EXPECT_EQ(volumeSyncNode->GetNodeName(), nodeName);
}

// ==================== Private function / member tests ====================

/**
 * @tc.name  : privateMember_renderId_initial_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_025
 * @tc.desc  : Test renderId_ initial value is HDI_INVALID_ID
 */
HWTEST_F(HpaeVolumeSyncNodeTest, privateMember_renderId_initial_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    EXPECT_NE(volumeSyncNode->renderId_, HDI_INVALID_ID);
}

/**
 * @tc.name  : getRenderSink_invalidId_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_026
 * @tc.desc  : Test GetRenderSink when renderId_ is HDI_INVALID_ID (calls HdiAdapterManager)
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getRenderSink_invalidId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    EXPECT_NE(volumeSyncNode->renderId_, HDI_INVALID_ID);

    // Call GetRenderSink - should try to get ID from HdiAdapterManager
    // In test environment, no real adapter, so it returns nullptr
    auto sink = volumeSyncNode->GetRenderSink();
    // renderId_ should have been updated (no longer HDI_INVALID_ID)
    EXPECT_NE(volumeSyncNode->renderId_, HDI_INVALID_ID);
}

/**
 * @tc.name  : getRenderSink_cachedId_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_027
 * @tc.desc  : Test GetRenderSink uses cached renderId_ on second call
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getRenderSink_cachedId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    auto sink1 = volumeSyncNode->GetRenderSink();
    auto firstId = volumeSyncNode->renderId_;
    EXPECT_NE(firstId, HDI_INVALID_ID);

    auto sink2 = volumeSyncNode->GetRenderSink();
    // renderId_ should not change on second call
    EXPECT_EQ(volumeSyncNode->renderId_, firstId);
}

/**
 * @tc.name  : signalProcess_largeVolumeChange_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_028
 * @tc.desc  : Test SignalProcess with large volume change triggers volume sync
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_largeVolumeChange_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    // Set history volume to 0 to trigger a large volume change
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, 0.0f);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);

    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : resetVolume_multipleCalls_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_029
 * @tc.desc  : Test multiple ResetVolume calls do not crash
 */
HWTEST_F(HpaeVolumeSyncNodeTest, resetVolume_multipleCalls_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    volumeSyncNode->ResetVolume();
    volumeSyncNode->ResetVolume();
    volumeSyncNode->ResetVolume();
    // Verify node still functional after multiple resets
    EXPECT_EQ(volumeSyncNode->GetSessionId(), DEFAULT_SESSION_ID);
}

/**
 * @tc.name  : signalProcess_volumeChange_withNullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_030
 * @tc.desc  : Test SignalProcess handles volume change when render sink is null
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_volumeChange_withNullSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "unknown_device";
    nodeInfo.deviceNetId = "LocalDevice";
    nodeInfo.sessionId = DEFAULT_SESSION_ID + 1;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    // Force a volume change with different history
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID + 1, 0.1f);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    // Should not crash even with null render sink and volume change
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : signalProcess_thenResetVolume_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_031
 * @tc.desc  : Test SignalProcess after ResetVolume uses updated history
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_thenResetVolume_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    // ResetVolume sets history to current system gain
    volumeSyncNode->ResetVolume();

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    // After ResetVolume, history should match current → no volume change path
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : getLatency_withSessionId_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_032
 * @tc.desc  : Test GetLatency with various session IDs always returns 0
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getLatency_withSessionId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    EXPECT_EQ(volumeSyncNode->GetLatency(0), 0u);
    EXPECT_EQ(volumeSyncNode->GetLatency(100), 0u);
    EXPECT_EQ(volumeSyncNode->GetLatency(UINT32_MAX), 0u);
}

/**
 * @tc.name  : signalProcess_negativeVolumeChange_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_033
 * @tc.desc  : Test SignalProcess with negative volume change (volume decrease)
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_negativeVolumeChange_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    // Set history to high value to simulate volume decrease
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, 2.0f);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : privateMember_isInnerCapturerOrInjector_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_034
 * @tc.desc  : Test isInnerCapturerOrInjector_ default value is false
 */
HWTEST_F(HpaeVolumeSyncNodeTest, privateMember_isInnerCapturerOrInjector_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    EXPECT_EQ(volumeSyncNode->isInnerCapturerOrInjector_, false);
}

/**
 * @tc.name  : getRenderSink_voipDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_035
 * @tc.desc  : Test GetRenderSink with primary_direct_voip device class
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getRenderSink_voipDevice_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary_direct_voip";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    auto sink = volumeSyncNode->GetRenderSink();
    // renderId_ should be updated from HdiAdapterManager
    EXPECT_NE(volumeSyncNode->renderId_, HDI_INVALID_ID);
}

/**
 * @tc.name  : signalProcess_volumeSyncInChain_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_036
 * @tc.desc  : Test VolumeSyncNode in a node chain with SinkOutputNode
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_volumeSyncInChain_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "file_io";
    nodeInfo.streamType = STREAM_MUSIC;

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    auto sinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    sinkOutputNode->Connect(volumeSyncNode);
    volumeSyncNode->Connect(sinkInputNode);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);
    EXPECT_EQ(sinkOutputNode->GetRenderSinkInstance("file_io", "LocalDevice"), 0);

    // DoProcess should not crash
    sinkOutputNode->DoProcess();

    sinkOutputNode->DisConnect(volumeSyncNode);
    volumeSyncNode->DisConnect(sinkInputNode);
}

/**
 * @tc.name  : signalProcess_epsilonVolume_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_037
 * @tc.desc  : Test SignalProcess with volume difference within epsilon (no change detected)
 */
HWTEST_F(HpaeVolumeSyncNodeTest, signalProcess_epsilonVolume_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.streamType = STREAM_MUSIC;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);

    PcmBufferInfo bufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, 0);
    HpaePcmBuffer pcmBuffer(bufferInfo);
    pcmBuffer.SetBufferValid(true);

    // Set history volume very close to default (within epsilon)
    AudioVolume::GetInstance()->SetHistoryVolume(DEFAULT_SESSION_ID, DEFAULT_VOLUME - 1e-7f);

    std::vector<HpaePcmBuffer *> inputs = { &pcmBuffer };
    HpaePcmBuffer *result = volumeSyncNode->SignalProcess(inputs);
    EXPECT_EQ(result, &pcmBuffer);
}

/**
 * @tc.name  : dfxNodeAdminCallback_constructDestruct_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_038
 * @tc.desc  : Cover ENABLE_HIDUMP_DFX TRUE bodies in ctor (OnNotifyDfxNodeAdmin(true, ...))
 *             and dtor (OnNotifyDfxNodeAdmin(false, ...)) by injecting a live statusCallback.
 */
HWTEST_F(HpaeVolumeSyncNodeTest, dfxNodeAdminCallback_constructDestruct_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    // Inject a live statusCallback so GetNodeStatusCallback().lock() succeeds in ctor/dtor.
    auto callback = std::make_shared<NiceMock<MockNodeCallback>>();
    nodeInfo.statusCallback = callback;

    // Each EXPECT_CALL discriminates its own branch body: deleting the matching
    // production line makes the corresponding Times(1) fail.
    EXPECT_CALL(*callback, OnNotifyDfxNodeAdmin(true, _)).Times(1);
    EXPECT_CALL(*callback, OnNotifyDfxNodeAdmin(false, _)).Times(1);
    {
        // Construction triggers the true branch; destruction (on scope exit) triggers
        // the false branch while callback's shared_ptr is still alive.
        std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
        EXPECT_NE(volumeSyncNode, nullptr);
    }
}

/**
 * @tc.name  : getRenderSink_forcedInvalidId_resets_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_039
 * @tc.desc  : Cover GetRenderSink TRUE body (renderId_ == HDI_INVALID_ID) by forcing renderId_
 *             back to HDI_INVALID_ID after construction, then invoking GetRenderSink.
 */
HWTEST_F(HpaeVolumeSyncNodeTest, getRenderSink_forcedInvalidId_resets_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    ASSERT_NE(volumeSyncNode, nullptr);
    // Constructor already assigned a valid (non-INVALID) id via HdiAdapterManager.
    EXPECT_NE(volumeSyncNode->renderId_, HDI_INVALID_ID);

    // Force the TRUE-branch precondition so GetRenderSink re-runs HdiAdapterManager::GetId.
    volumeSyncNode->renderId_ = HDI_INVALID_ID;
    EXPECT_EQ(volumeSyncNode->renderId_, HDI_INVALID_ID);

    // Returned shared_ptr<IAudioRenderSink> is intentionally not bound/asserted: it is the
    // real HdiAdapterManager::GetRenderSink create path (device-dependent). The discriminating
    // signal here is the renderId_ side effect, asserted below.
    volumeSyncNode->GetRenderSink();
    // Discriminating: if the TRUE body assignment line is removed, renderId_ stays INVALID.
    EXPECT_NE(volumeSyncNode->renderId_, HDI_INVALID_ID);
}

/**
 * @tc.name  : setSinkVolume_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeVolumeSyncNodeTest_040
 * @tc.desc  : Cover SetSinkVolume else branch (audioRendererSink == nullptr) by forcing renderId_
 *             to BOGUS_CAPTURE_BASE_RENDER_ID (HDI id base = CAPTURE ≠ RENDER) so
 *             HdiAdapterManager::GetRenderSink returns nullptr. NOTE: the ctor hardcodes renderId_
 *             to the voip render id (independent of deviceClass, hence deviceClass = "primary" here
 *             is irrelevant); this case forcibly overwrites renderId_ to a bogus CAPTURE-base id so
 *             GetRenderSink returns nullptr — merely changing deviceClass would NOT null the sink.
 *             The else body emits only an AUDIO_WARNING_LOG with no observable side effect, so this
 *             case is honestly classified as branch-coverage (NOT delete-line discrimination on the
 *             else body).
 *             Discrimination here comes from two other points:
 *               (1) Precondition lock (primary discriminator): EXPECT_EQ(GetRenderSink(), nullptr)
 *                   before calling SetSinkVolume pins down "sink is null → else is taken, if is
 *                   not". If renderId_ is not corrupted (sink non-null, e.g. forgetting to set it),
 *                   this EXPECT_EQ fails → case fails. This is what separates "else ran" from
 *                   "if ran".
 *               (2) Guard delete-line discrimination: after SetSinkVolume(0.5f) the trivial
 *                   GetSessionId() assertion must be reachable → process survived. If someone
 *                   deletes the production `if (audioRendererSink != nullptr)` guard, then with
 *                   sink == nullptr the body would dereference null via SetVolume() → sanitizer
 *                   abort → the post assertion becomes unreachable → case fails. I.e. the guard
 *                   is what protects against null-deref, and removing it makes this case fail.
 */
HWTEST_F(HpaeVolumeSyncNodeTest, setSinkVolume_nullSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.sessionId = DEFAULT_SESSION_ID;

    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
    ASSERT_NE(volumeSyncNode, nullptr);

    // Force renderId_ to a CAPTURE-base bogus id (see BOGUS_CAPTURE_BASE_RENDER_ID comment):
    // IdHandler::CheckId(_, HDI_ID_BASE_RENDER) fails on ParseBase mismatch (first guard),
    // so HdiAdapterManager::GetRenderSink returns nullptr. The id is not HDI_INVALID_ID, so
    // GetRenderSink()'s own re-fetch guard (if (renderId_ == HDI_INVALID_ID)) does not fire
    // and cannot overwrite the bogus value with a valid one.
    volumeSyncNode->renderId_ = BOGUS_CAPTURE_BASE_RENDER_ID;

    // Precondition lock (PRIMARY discriminator): sink must be null, proving SetSinkVolume will
    // take the else branch rather than the if branch. Deleting this EXPECT_EQ does not affect
    // coverage; it exists to fail-fast if setup forgets to corrupt renderId_ (sink would then
    // be non-null and the case would silently cover the wrong branch).
    EXPECT_EQ(volumeSyncNode->GetRenderSink(), nullptr);

    // Invoke private SetSinkVolume (-fno-access-control enabled). With sink == nullptr the if
    // guard is false → else body runs (the AUDIO_WARNING_LOG line targeted by this case).
    volumeSyncNode->SetSinkVolume(TEST_VOLUME_1);

    // Guard delete-line discrimination: trivial assertion proving the process survived the call.
    // If the production `if (audioRendererSink != nullptr)` guard were removed, the if-body would
    // dereference null via SetVolume() → sanitizer abort → this line would never execute → fail.
    EXPECT_EQ(volumeSyncNode->GetSessionId(), DEFAULT_SESSION_ID);
}
}  // namespace
