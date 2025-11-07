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
#include <cmath>
#include <memory>
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_mocks.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const char *ROOT_PATH = "/data/source_file_io_48000_2_s16le.pcm";
constexpr int64_t SILENCE_TIME_OUT_US = 5 * 1000000;
class HpaeSinkOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSinkOutputNodeTest::SetUp()
{}

void HpaeSinkOutputNodeTest::TearDown()
{}

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    size_t frameLen = 960;
    uint32_t nodeId = 1243;
    nodeInfo.nodeId = nodeId;
    nodeInfo.frameLen = frameLen;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
}

static void PreparePcmBufferInfo(PcmBufferInfo &bufferInfo)
{
    bufferInfo = {2, 960, 48000}; // 2channel 960framelen 48000samplerate
}

HWTEST_F(HpaeSinkOutputNodeTest, constructHpaeSinkOutputNode, TestSize.Level0)
{
    uint32_t sessionId = 10001;
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkOutputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSinkOutputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSinkOutputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSinkOutputNode->GetBitWidth(), nodeInfo.format);
    EXPECT_EQ(hpaeSinkOutputNode->GetSessionId(), nodeInfo.sessionId);

    HpaeNodeInfo &retNi = hpaeSinkOutputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.sessionId, nodeInfo.sessionId);
}

static int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    for (int32_t i = 0; i < len / SAMPLE_F32LE; i++) {
        float diff = *((float *)data + i) - i;
        EXPECT_EQ(diff, 0);
    }
    return 0;
}

HWTEST_F(HpaeSinkOutputNodeTest, testHpaeSinkOutConnectNode, TestSize.Level0)
{
    size_t usedCount = 2;
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    std::shared_ptr<WriteIncDataCb> writeIncDataCb = std::make_shared<WriteIncDataCb>(SAMPLE_F32LE);
    hpaeSinkInputNode->RegisterWriteCallback(writeIncDataCb);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_NEW, true);
    IAudioSinkAttr attr;
    attr.adapterName = "file_io";
    attr.openMicSpeaker = 0;
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.filePath = ROOT_PATH;
    attr.deviceNetworkId = deviceNetId.c_str();
    attr.deviceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;

    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_IDLE, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkStart(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_RUNNING, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkPause(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_SUSPENDED, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_SUSPENDED, true);
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    EXPECT_EQ(hpaeSinkInputNode.use_count(), usedCount);
    hpaeSinkOutputNode->DisConnect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
    std::function<void(bool)> callback = [](bool state) { EXPECT_FALSE(state); };
    hpaeSinkOutputNode->RegisterCurrentDeviceCallback(callback);
    hpaeSinkOutputNode->RenderSinkDeInit();
}

HWTEST_F(HpaeSinkOutputNodeTest, testHpaeSinkOutConnectNodeRemote, TestSize.Level0)
{
    size_t usedCount = 2;
    std::string deviceClass = "remote";
    std::string deviceNetId = "LocalDevice";
    HpaeNodeInfo nodeInfo;
    nodeInfo.deviceClass = deviceClass;
    PrepareNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    std::shared_ptr<WriteIncDataCb> writeIncDataCb = std::make_shared<WriteIncDataCb>(SAMPLE_F32LE);
    hpaeSinkInputNode->RegisterWriteCallback(writeIncDataCb);
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_NEW, true);
    IAudioSinkAttr attr;
    attr.adapterName = "file_io";
    attr.openMicSpeaker = 0;
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.filePath = ROOT_PATH;
    attr.deviceNetworkId = deviceNetId.c_str();
    attr.deviceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;

    hpaeSinkOutputNode->RenderSinkInit(attr);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_IDLE, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkStart(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_RUNNING, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkPause(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_SUSPENDED, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_SUSPENDED, true);
    hpaeSinkOutputNode->remoteTimePoint_ = std::chrono::high_resolution_clock::now();
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    EXPECT_EQ(hpaeSinkInputNode.use_count(), usedCount);
    hpaeSinkOutputNode->DisConnect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
    std::function<void(bool)> callback = [](bool state) { EXPECT_FALSE(state); };
    hpaeSinkOutputNode->RegisterCurrentDeviceCallback(callback);
    hpaeSinkOutputNode->RenderSinkDeInit();
}

#ifdef ENABLE_HOOK_PCM
HWTEST_F(HpaeSinkOutputNodeTest, testDoProcessAfterResetPcmDumper, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    std::string deviceClass = "remote";
    std::string deviceNetId = "LocalDevice";
    nodeInfo.deviceClass = deviceClass;
    PrepareNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    std::shared_ptr<WriteIncDataCb> writeIncDataCb = std::make_shared<WriteIncDataCb>(SAMPLE_F32LE);
    hpaeSinkInputNode->RegisterWriteCallback(writeIncDataCb);

    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_NEW, true);

    IAudioSinkAttr attr;
    attr.adapterName = "file_io";
    attr.openMicSpeaker = 0;
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.filePath = ROOT_PATH;
    attr.deviceNetworkId = deviceNetId.c_str();
    attr.deviceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
    hpaeSinkOutputNode->RenderSinkInit(attr);
    hpaeSinkOutputNode->RenderSinkStart();
    hpaeSinkOutputNode->DoProcess();
    std::function<void(bool)> callback = [](bool state) { EXPECT_FALSE(state); };
    hpaeSinkOutputNode->RegisterCurrentDeviceCallback(callback);
    hpaeSinkOutputNode->RenderSinkDeInit();
}
#endif

HWTEST_F(HpaeSinkOutputNodeTest, testHpaeSinkOutHandleHapticParam, TestSize.Level0)
{
    size_t usedCount = 2;
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    std::shared_ptr<WriteIncDataCb> writeIncDataCb = std::make_shared<WriteIncDataCb>(SAMPLE_F32LE);
    hpaeSinkInputNode->RegisterWriteCallback(writeIncDataCb);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_NEW, true);
    IAudioSinkAttr attr;
    attr.adapterName = "file_io";
    attr.openMicSpeaker = 0;
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.filePath = ROOT_PATH;
    attr.deviceNetworkId = deviceNetId.c_str();
    attr.deviceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
    int32_t syncId = 123;

    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_IDLE, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkStart(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_RUNNING, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkPause(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_SUSPENDED, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState() == STREAM_MANAGER_SUSPENDED, true);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkSetSyncId(syncId), SUCCESS);
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    EXPECT_EQ(hpaeSinkInputNode.use_count(), usedCount);
    hpaeSinkOutputNode->DisConnect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
    std::function<void(bool)> callback = [](bool state) { EXPECT_FALSE(state); };
    hpaeSinkOutputNode->RegisterCurrentDeviceCallback(callback);
    hpaeSinkOutputNode->RenderSinkDeInit();
}

// Test case: should skip when device class is not primary
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_NonPrimaryDevice_ShouldSkip, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);

    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "not_primary"; // Non-primary device

    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    // Ensure no mock methods are called
    EXPECT_CALL(*mockSink, SetPaPower(::testing::_)).Times(0);
    EXPECT_CALL(*mockSink, GetAudioScene()).Times(0);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
}

// Test case: should skip when PCM buffer is invalid
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_InvalidBuffer_ShouldSkip, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);
    pcmBuffer->pcmBufferInfo_.state = PCM_BUFFER_STATE_INVALID; // Set buffer invalid

    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "primary"; // primary device
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Ensure no mock methods are called
    EXPECT_CALL(*mockSink, SetPaPower(::testing::_)).Times(0);
    EXPECT_CALL(*mockSink, GetAudioScene()).Times(0);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
}

// Test case: should start timer when first entering silence
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_FirstSilence_ShouldStartTimer, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);
    pcmBuffer->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE; // Silence data

    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "primary"; // primary device
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Initial state: timer not started
    hpaeSinkOutputNode->isDisplayPaPowerState_ = false;
    std::vector<int32_t> appsUid{0};

    EXPECT_CALL(*mockSink, GetAudioScene()).Times(0);
    EXPECT_CALL(*mockSink, SetPaPower(::testing::_)).Times(0);
    EXPECT_CALL(*mockSink, UpdateAppsUid(appsUid)).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(true));

    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());

    // Verify timer start flag is set
    EXPECT_TRUE(hpaeSinkOutputNode->isDisplayPaPowerState_);
    // Verify silence time accumulation is correct
    // 960 framelen, 1000000 us to s, 48000 samplerate
    int64_t expectedTime = static_cast<int64_t>(960) * 1000000 / 48000;
    EXPECT_EQ(hpaeSinkOutputNode->silenceDataUs_, expectedTime);
}

// Test case: should close PA when silence timeout and scene condition met
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_SilenceTimeout_ShouldClosePa, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);
    pcmBuffer->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE;

    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Initial state: PA on and silence time near threshold
    hpaeSinkOutputNode->isOpenPaPower_ = true;
    hpaeSinkOutputNode->silenceDataUs_ = SILENCE_TIME_OUT_US; // 5 seconds
    std::vector<int32_t> appsUid{0};

    // Mock: normal audio scene
    EXPECT_CALL(*mockSink, GetAudioScene()).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, SetPaPower(false)).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, UpdateAppsUid(appsUid)).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(true));
    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
    // Verify PA is closed and timer reset
    EXPECT_FALSE(hpaeSinkOutputNode->isOpenPaPower_);
    EXPECT_EQ(hpaeSinkOutputNode->silenceDataUs_, 0);
}

// Test case: should monitor timeout after PA closed
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_PaClosedSilence_ShouldMonitorTimeout, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);
    pcmBuffer->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE;
    
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Initial state: PA closed and silence time exceeds 10 seconds
    hpaeSinkOutputNode->isOpenPaPower_ = false;
    hpaeSinkOutputNode->silenceDataUs_ = 5 * 60 * 1000000; // 5 * 60s
    std::vector<int32_t> appsUid{0};

    EXPECT_CALL(*mockSink, SetPaPower(::testing::_)).Times(0); // Should not close again
    EXPECT_CALL(*mockSink, UpdateAppsUid(appsUid)).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(true));
    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
    // Verify timer reset and log triggered
    EXPECT_EQ(hpaeSinkOutputNode->silenceDataUs_, 0);
}

// Test case: non-silence data should break closing process
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_NonSilence_ShouldBreakCloseProcess, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);

    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Timer already started
    hpaeSinkOutputNode->isDisplayPaPowerState_ = true;
    hpaeSinkOutputNode->silenceDataUs_ = 3 * 1000000; // 3 seconds

    EXPECT_CALL(*mockSink, SetPaPower(true)).Times(0); // PA not closed yet
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
    // Verify timer stopped and reset
    EXPECT_FALSE(hpaeSinkOutputNode->isDisplayPaPowerState_);
    EXPECT_EQ(hpaeSinkOutputNode->silenceDataUs_, 0);
}

// Test case: should open PA when receiving non-silence data
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_NonSilence_ShouldOpenPa, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);

    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // PA is currently closed
    hpaeSinkOutputNode->isOpenPaPower_ = false;
    EXPECT_CALL(*mockSink, SetPaPower(true)).WillOnce(Return(0));
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
    // Verify PA is opened
    EXPECT_TRUE(hpaeSinkOutputNode->isOpenPaPower_);
    EXPECT_EQ(hpaeSinkOutputNode->silenceDataUs_, 0);
}

// Test case: should not close PA when scene condition not met
HWTEST_F(HpaeSinkOutputNodeTest, HandlePaPower_SilenceTimeoutWrongScene_ShouldNotClosePa, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);
    pcmBuffer->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE;
    
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    
    // Initial state: PA on and silence time exceeded
    hpaeSinkOutputNode->isOpenPaPower_ = true;
    hpaeSinkOutputNode->silenceDataUs_ = SILENCE_TIME_OUT_US; // 5s
    std::vector<int32_t> appsUid{0};
    // Mock: non-normal audio scene
    EXPECT_CALL(*mockSink, GetAudioScene()).WillOnce(Return(1)); // Scene not 0
    EXPECT_CALL(*mockSink, SetPaPower(::testing::_)).Times(0); // Should not call close
    EXPECT_CALL(*mockSink, UpdateAppsUid(appsUid)).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(true));
    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
    // Verify PA still on and timer not reset
    EXPECT_TRUE(hpaeSinkOutputNode->isOpenPaPower_);
    EXPECT_GT(hpaeSinkOutputNode->silenceDataUs_, SILENCE_TIME_OUT_US);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS