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
const int32_t OUT_OF_RANGE_SESSION = 999;
const int32_t NO_EXITS_SESSIONID_ID = 99999;
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
    EXPECT_CALL(*mockSink, GetAudioScene()).WillOnce(Return(1)).WillRepeatedly(Return(1)); // Scene not 0
    EXPECT_CALL(*mockSink, SetPaPower(::testing::_)).Times(0); // Should not call close
    EXPECT_CALL(*mockSink, UpdateAppsUid(appsUid)).WillOnce(Return(0));
    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(true));
    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    hpaeSinkOutputNode->HandlePaPower(pcmBuffer.get());
    // Verify PA still on and timer not reset
    EXPECT_TRUE(hpaeSinkOutputNode->isOpenPaPower_);
    EXPECT_GT(hpaeSinkOutputNode->silenceDataUs_, SILENCE_TIME_OUT_US);
}

// Test case: CheckAndSetCollDelayForRenderFrameFailed
HWTEST_F(HpaeSinkOutputNodeTest, CheckAndSetCollDelayForRenderFrameFailed, TestSize.Level0)
{
    PcmBufferInfo bufferInfo;
    PreparePcmBufferInfo(bufferInfo);
    std::shared_ptr<HpaePcmBuffer> pcmBuffer = std::make_shared<HpaePcmBuffer>(bufferInfo);
    pcmBuffer->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE;
    
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->collRenderFrameFailedCount_.store(0);
    hpaeSinkOutputNode->CheckAndSetCollDelayForRenderFrameFailed();

    EXPECT_EQ(hpaeSinkOutputNode->collRenderFrameFailedCount_.load(), 0);
    hpaeSinkOutputNode->collRenderFrameFailedCount_.fetch_add(1);
    EXPECT_EQ(hpaeSinkOutputNode->collRenderFrameFailedCount_.load(), 1);
    hpaeSinkOutputNode->collRenderFrameFailedCount_.fetch_add(1);
    hpaeSinkOutputNode->collRenderFrameFailedCount_.fetch_add(1);
    hpaeSinkOutputNode->CheckAndSetCollDelayForRenderFrameFailed();
    EXPECT_EQ(hpaeSinkOutputNode->collRenderFrameFailedCount_.load(), 0);
}

/**
 * @tc.name  : SetAuxiliarySinkEnable_001
 * @tc.type  : FUNC
 * @tc.desc  : Test enabling auxiliary sink when it's not yet created.
 */
HWTEST_F(HpaeSinkOutputNodeTest, SetAuxiliarySinkEnable_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxiliarySink_ = nullptr;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "Auxiliary_Speaker";

    int32_t result = hpaeSinkOutputNode->SetAuxiliarySinkEnable(true);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(hpaeSinkOutputNode->auxSinkEnable_);
}

/**
 * @tc.name  : SetAuxiliarySinkEnable_002
 * @tc.type  : FUNC
 * @tc.desc  : Test disabling auxiliary sink.
 */
HWTEST_F(HpaeSinkOutputNodeTest, SetAuxiliarySinkEnable_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxiliarySink_ = std::make_shared<MockAudioRenderSink>();

    int32_t result = hpaeSinkOutputNode->SetAuxiliarySinkEnable(false);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_FALSE(hpaeSinkOutputNode->auxSinkEnable_);
}

/**
 * @tc.name  : GetLatency_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when audioRendererSink_ is valid.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    uint32_t mockLatency = 50;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(0)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, mockLatency);
}

/**
 * @tc.name  : GetLatency_002
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when audioRendererSink_ is nullptr (Defensive path).
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, static_cast<uint32_t>(ERROR));
}

/**
 * @tc.name  : RenderSinkReset_Resume_001
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset and Resume with valid and null sink pointers.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkReset_Resume_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkReset(), ERROR);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkResume(), ERROR);

    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    EXPECT_CALL(*mockSink, Reset()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkReset(), SUCCESS);

    hpaeSinkOutputNode->SetSinkState(STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkResume(), SUCCESS);

    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState(), STREAM_MANAGER_RUNNING);

    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(ERR_OPERATION_FAILED));
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkResume(), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Reset_001
 * @tc.type  : FUNC
 * @tc.desc  : Test the topology disconnection logic in Reset.
 */
HWTEST_F(HpaeSinkOutputNodeTest, Reset_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    HpaeNodeInfo inputInfo;
    inputInfo.sessionId = 80001;
    auto inputNode = std::make_shared<HpaeSinkInputNode>(inputInfo);

    hpaeSinkOutputNode->Connect(inputNode);

    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 1);

    bool result = hpaeSinkOutputNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink when auxSinkEnable is false.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = false;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, RenderFrame(_, _, _)).Times(0);

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink when auxSinkState is not RUNNING.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, RenderFrame(_, _, _)).Times(0);

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_003
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink when sinkName is not in AUXILIARY_SPEAKER_LIST.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "speaker"; // Not in AUXILIARY_SPEAKER_LIST

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, RenderFrame(_, _, _)).Times(0);

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_004
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink when auxiliarySink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxiliarySink_ = nullptr;

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_005
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink with all conditions met (success path).
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, RenderFrame(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format)),
            Return(0)));

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_006
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink for usb sink.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb";

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, RenderFrame(_, _, _)).WillOnce(Return(0));

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : Reset_002
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset with multiple connected nodes.
 */
HWTEST_F(HpaeSinkOutputNodeTest, Reset_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    HpaeNodeInfo inputInfo1;
    inputInfo1.sessionId = 80001;
    auto inputNode1 = std::make_shared<HpaeSinkInputNode>(inputInfo1);

    HpaeNodeInfo inputInfo2;
    inputInfo2.sessionId = 80002;
    auto inputNode2 = std::make_shared<HpaeSinkInputNode>(inputInfo2);

    HpaeNodeInfo inputInfo3;
    inputInfo3.sessionId = 80003;
    auto inputNode3 = std::make_shared<HpaeSinkInputNode>(inputInfo3);

    hpaeSinkOutputNode->Connect(inputNode1);
    hpaeSinkOutputNode->Connect(inputNode2);
    hpaeSinkOutputNode->Connect(inputNode3);

    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 3);

    bool result = hpaeSinkOutputNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
}

/**
 * @tc.name  : Reset_003
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset with no connected nodes.
 */
HWTEST_F(HpaeSinkOutputNodeTest, Reset_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);

    bool result = hpaeSinkOutputNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
}

/**
 * @tc.name  : AuxiliarySinkInit_001
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when sinkName is not in AUXILIARY_SPEAKER_LIST.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "speaker"; // Not in AUXILIARY_SPEAKER_LIST
    hpaeSinkOutputNode->auxSinkEnable_ = true;

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkInit_002
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when auxSinkEnable is false.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = false;

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkInit_003
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when auxiliarySink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxiliarySink_ = nullptr;

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkInit_004
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when auxiliarySink is already initialized.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, IsInited()).WillOnce(Return(true));

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : AuxiliarySinkInit_005
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when Init succeeds.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockAuxSink, Init(_)).WillOnce(Return(SUCCESS));

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : AuxiliarySinkInit_006
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when Init fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockAuxSink, Init(_)).WillOnce(Return(ERR_OPERATION_FAILED));

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkInit_007
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit for usb sink.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb";
    hpaeSinkOutputNode->auxSinkEnable_ = true;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockAuxSink, Init(_)).WillOnce(Return(SUCCESS));

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : AuxiliarySinkDeInit_001
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkDeInit when sinkName is not in AUXILIARY_SPEAKER_LIST.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkDeInit_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "speaker"; // Not in AUXILIARY_SPEAKER_LIST

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkDeInit();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkDeInit_002
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkDeInit when auxiliarySink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkDeInit_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxiliarySink_ = nullptr;

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkDeInit();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkDeInit_003
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkDeInit when auxiliarySink is already deinitialized.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkDeInit_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, IsInited()).WillOnce(Return(false));

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkDeInit();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->auxiliarySink_, nullptr);
}

/**
 * @tc.name  : RenderSinkFlush_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkFlush when audioRendererSink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkFlush_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;

    int32_t result = hpaeSinkOutputNode->RenderSinkFlush();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderSinkFlush_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkFlush when Flush succeeds.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkFlush_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Flush()).WillOnce(Return(SUCCESS));

    int32_t result = hpaeSinkOutputNode->RenderSinkFlush();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : RenderSinkFlush_003
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkFlush when Flush fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkFlush_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Flush()).WillOnce(Return(ERR_OPERATION_FAILED));

    int32_t result = hpaeSinkOutputNode->RenderSinkFlush();

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : RenderSinkReset_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkReset when audioRendererSink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkReset_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;

    int32_t result = hpaeSinkOutputNode->RenderSinkReset();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderSinkReset_003
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkReset when Reset succeeds.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkReset_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Reset()).WillOnce(Return(SUCCESS));

    int32_t result = hpaeSinkOutputNode->RenderSinkReset();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : RenderSinkReset_004
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkReset when Reset fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkReset_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Reset()).WillOnce(Return(ERR_OPERATION_FAILED));

    int32_t result = hpaeSinkOutputNode->RenderSinkReset();

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : RenderSinkResume_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume when audioRendererSink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;

    int32_t result = hpaeSinkOutputNode->RenderSinkResume();

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderSinkResume_003
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume when Resume succeeds.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    hpaeSinkOutputNode->SetSinkState(STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(SUCCESS));

    int32_t result = hpaeSinkOutputNode->RenderSinkResume();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : RenderSinkResume_004
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume when Resume fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    hpaeSinkOutputNode->SetSinkState(STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(ERR_OPERATION_FAILED));

    int32_t result = hpaeSinkOutputNode->RenderSinkResume();

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_001
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState when sinkName is not in AUXILIARY_SPEAKER_LIST.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "speaker"; // Not in AUXILIARY_SPEAKER_LIST
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_002
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState when usage is not in filter sets.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_RINGTONE, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_003
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState adding valid usage with RUNNING state.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
    EXPECT_NE(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_004
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState removing valid usage.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    // First add a valid session
    hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_[10001] = STREAM_USAGE_MUSIC;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    // Now remove it
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10001,
        STREAM_USAGE_MUSIC, RENDERER_STOPPED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_005
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState adding invalid usage with RUNNING state.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_NE(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_006
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState removing invalid usage when valid sessions exist.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    // Add invalid session first (causes auxSinkState to be IDLE)
    hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_[10001] = STREAM_USAGE_VOICE_COMMUNICATION;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add valid session
    hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_[10002] = STREAM_USAGE_MUSIC;

    // Remove invalid session
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_STOPPED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_007
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with STATE_CHANGE to RUNNING.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_STATE_CHANGE, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_008
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with multiple valid sessions.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    // Add multiple valid sessions
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10002,
        STREAM_USAGE_MOVIE, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    // Remove one session
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10001,
        STREAM_USAGE_MUSIC, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    // Remove last session
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10002,
        STREAM_USAGE_MOVIE, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_009
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with invalid usage and state not RUNNING.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_009, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add invalid session with stopped state - should not add to filter
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_STOPPED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_010
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState for usb sink.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_010, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb"; // In AUXILIARY_SPEAKER_LIST
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001, STREAM_USAGE_MUSIC, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_011
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with game usage.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_011, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_GAME, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_012
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with audiobook usage.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_012, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_AUDIOBOOK, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_013
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with video_communication usage.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_013, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_VIDEO_COMMUNICATION, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : GetLatency_003
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency with different latency values.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    uint32_t mockLatency = 100;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(0)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, mockLatency);
}

/**
 * @tc.name  : GetLatency_004
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency with zero latency.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    uint32_t mockLatency = 0;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(0)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, mockLatency);
}

/**
 * @tc.name  : GetLatency_005
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when GetLatency on sink fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    uint32_t mockLatency = 50;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(ERR_OPERATION_FAILED)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, mockLatency); // Still returns latency value even if GetLatency fails
}

/**
 * @tc.name  : GetLatency_006
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency with maximum latency.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    uint32_t mockLatency = UINT32_MAX;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(0)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, UINT32_MAX);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_014
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState removing valid session that doesn't exist (error path).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_014, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    // Try to remove session that doesn't exist - should fail CHECK_AND_RETURN_LOG
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE,
        NO_EXITS_SESSIONID_ID, STREAM_USAGE_MUSIC, RENDERER_STOPPED);

    // State should remain RUNNING
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_015
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState removing invalid session that doesn't exist (error path).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_015, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Try to remove invalid session that doesn't exist - should fail CHECK_AND_RETURN_LOG
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, NO_EXITS_SESSIONID_ID,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_STOPPED);

    // State should remain IDLE
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_016
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState adding valid session with non-running state (no add).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_016, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add valid session with PAUSED state - should not add to filter
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001, STREAM_USAGE_MUSIC, RENDERER_PAUSED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_017
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState STATE_CHANGE with non-running state (remove).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_017, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    // Add valid session first
    hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_[10001] = STREAM_USAGE_MUSIC;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    // State change to PAUSED - should remove from filter
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_STATE_CHANGE, 10001,
        STREAM_USAGE_MUSIC, RENDERER_PAUSED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_018
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with all four valid usages combined.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_018, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add all four valid usages
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10002,
        STREAM_USAGE_MOVIE, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10003,
        STREAM_USAGE_GAME, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10004,
        STREAM_USAGE_AUDIOBOOK, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    // Remove all sessions one by one
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10001,
        STREAM_USAGE_MUSIC, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10002,
        STREAM_USAGE_MOVIE, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10003,
        STREAM_USAGE_GAME, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10004,
        STREAM_USAGE_AUDIOBOOK, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_019
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with both invalid usages combined.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_019, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    // Add both invalid usages - should make state IDLE
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);

    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10002,
        STREAM_USAGE_VIDEO_COMMUNICATION, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);

    // Remove one invalid session
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);

    // Remove last invalid session
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10002,
        STREAM_USAGE_VIDEO_COMMUNICATION, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_020
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState mixing valid and invalid usages.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_020, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add valid usage - state becomes RUNNING
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    // Add invalid usage - state becomes IDLE (invalid blocks valid)
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10002,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);

    // Remove invalid usage - state becomes RUNNING (only valid left)
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10002,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);

    // Remove valid usage - state becomes IDLE
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_REMOVE, 10001,
        STREAM_USAGE_MUSIC, RENDERER_STOPPED);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_021
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with invalid sink name.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_021, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // Set sinkName to something not in AUXILIARY_SPEAKER_LIST
    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "invalid_sink";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Update state - should not change anything due to CHECK_AND_RETURN at start
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MUSIC, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.empty());
}

/**
 * @tc.name  : RenderSinkFlush_004
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkFlush with Flush returning different error codes.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkFlush_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Test with various error codes
    EXPECT_CALL(*mockSink, Flush()).WillOnce(Return(ERR_ILLEGAL_STATE));
    int32_t result = hpaeSinkOutputNode->RenderSinkFlush();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    EXPECT_CALL(*mockSink, Flush()).WillOnce(Return(ERR_INVALID_PARAM));
    result = hpaeSinkOutputNode->RenderSinkFlush();
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : RenderSinkFlush_005
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkFlush multiple calls in sequence.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkFlush_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Flush())
        .Times(3)
        .WillRepeatedly(Return(SUCCESS));

    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkFlush(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkFlush(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkFlush(), SUCCESS);
}

/**
 * @tc.name  : RenderSinkReset_005
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkReset with Reset returning different error codes.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkReset_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Test with various error codes
    EXPECT_CALL(*mockSink, Reset()).WillOnce(Return(ERR_ILLEGAL_STATE));
    int32_t result = hpaeSinkOutputNode->RenderSinkReset();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    EXPECT_CALL(*mockSink, Reset()).WillOnce(Return(ERR_INVALID_PARAM));
    result = hpaeSinkOutputNode->RenderSinkReset();
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : RenderSinkResume_005
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume with Resume returning different error codes.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Test with various error codes - SetSinkState should NOT be called on error
    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(ERR_ILLEGAL_STATE));
    int32_t result = hpaeSinkOutputNode->RenderSinkResume();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
    // State should remain unchanged since Resume failed

    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(ERR_INVALID_PARAM));
    result = hpaeSinkOutputNode->RenderSinkResume();
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : RenderSinkResume_006
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume multiple calls in sequence.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Resume())
        .Times(3)
        .WillRepeatedly(Return(SUCCESS));

    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkResume(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState(), STREAM_MANAGER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkResume(), SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->RenderSinkResume(), SUCCESS);
}

/**
 * @tc.name  : AuxiliarySinkInit_009
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkInit when auxSinkEnable is true but sinkName is invalid.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkInit_009, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "invalid_sink";
    hpaeSinkOutputNode->auxSinkEnable_ = true;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkInit();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : AuxiliarySinkDeInit_007
 * @tc.type  : FUNC
 * @tc.desc  : Test AuxiliarySinkDeInit when sinkName is not in AUXILIARY_SPEAKER_LIST.
 */
HWTEST_F(HpaeSinkOutputNodeTest, AuxiliarySinkDeInit_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "invalid_sink";

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    int32_t result = hpaeSinkOutputNode->AuxiliarySinkDeInit();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_007
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink with all conditions met but RenderFrame fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    EXPECT_CALL(*mockAuxSink, RenderFrame(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(DoAll(SetArgReferee<2>(0), Return(ERR_OPERATION_FAILED)));

    // Should not crash even if RenderFrame fails
    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_008
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink with partial writeLen.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    // Simulate partial write - write half the data
    uint64_t partialWriteLen = hpaeSinkOutputNode->renderSize_ / 2;
    EXPECT_CALL(*mockAuxSink, RenderFrame(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(DoAll(SetArgReferee<2>(partialWriteLen), Return(SUCCESS)));

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : Reset_004
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset with single connected node.
 */
HWTEST_F(HpaeSinkOutputNodeTest, Reset_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    HpaeNodeInfo inputInfo;
    inputInfo.sessionId = 80001;
    auto inputNode = std::make_shared<HpaeSinkInputNode>(inputInfo);

    hpaeSinkOutputNode->Connect(inputNode);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 1);

    bool result = hpaeSinkOutputNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);

    // Verify Reset can be called again after disconnect
    result = hpaeSinkOutputNode->Reset();
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : GetLatency_007
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency with various latency boundary values.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Test with latency = 1 (minimum non-zero)
    uint32_t mockLatency = 1;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(0)));
    uint32_t result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, 1);

    // Test with latency = 1000
    mockLatency = 1000;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(mockLatency), Return(0)));
    result = hpaeSinkOutputNode->GetLatency();
    EXPECT_EQ(result, 1000);
}

/**
 * @tc.name  : GetLatency_008
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when latency member is updated but GetLatency call fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Initial latency value
    hpaeSinkOutputNode->latency_ = 100;

    // GetLatency fails but still updates the member variable
    uint32_t newLatency = 200;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(newLatency), Return(ERR_OPERATION_FAILED)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();
    // Result should be the new latency value even though GetLatency failed
    EXPECT_EQ(result, newLatency);
}

/**
 * @tc.name  : RenderSinkResume_007
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume state transition from SUSPENDED to RUNNING.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Set initial state to SUSPENDED
    hpaeSinkOutputNode->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    // Resume should change state to RUNNING
    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(SUCCESS));
    int32_t result = hpaeSinkOutputNode->RenderSinkResume();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : RenderSinkResume_008
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume state unchanged on error.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkResume_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Set initial state to SUSPENDED
    hpaeSinkOutputNode->SetSinkState(STREAM_MANAGER_SUSPENDED);

    // Resume fails, state should remain SUSPENDED
    EXPECT_CALL(*mockSink, Resume()).WillOnce(Return(ERR_OPERATION_FAILED));
    int32_t result = hpaeSinkOutputNode->RenderSinkResume();
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
    EXPECT_EQ(hpaeSinkOutputNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_009
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink when RenderFrame fails (critical path coverage).
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_009, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    // Mock RenderFrame to fail with error
    uint64_t writeLen = 0;
    EXPECT_CALL(*mockAuxSink, RenderFrame(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(DoAll(SetArgReferee<2>(writeLen), Return(ERR_OPERATION_FAILED)));

    // Should handle failure gracefully without crash
    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : RenderFrameForAuxiliarySink_010
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderFrameForAuxiliarySink when RenderFrame returns SUCCESS but writeLen is zero.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderFrameForAuxiliarySink_010, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb";
    hpaeSinkOutputNode->auxSinkEnable_ = true;
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_RUNNING;

    auto mockAuxSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->auxiliarySink_ = mockAuxSink;

    // Mock RenderFrame to return success but with zero bytes written
    uint64_t writeLen = 0;
    EXPECT_CALL(*mockAuxSink, RenderFrame(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(DoAll(SetArgReferee<2>(writeLen), Return(SUCCESS)));

    hpaeSinkOutputNode->RenderFrameForAuxiliarySink();
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_022
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with RENDERER_NEW state (state coverage gap).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_022, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add with RENDERER_NEW state - should NOT add to filter (only RUNNING adds)
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001, STREAM_USAGE_MUSIC, RENDERER_NEW);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_023
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with RENDERER_PREPARED state (state coverage gap).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_023, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Add with RENDERER_PREPARED state - should NOT add to filter
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_MOVIE, RENDERER_PREPARED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_024
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with RENDERER_PAUSED state (state coverage gap).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_024, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";

    // First add valid session with RUNNING state
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001, STREAM_USAGE_GAME, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_RUNNING);
    EXPECT_NE(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());

    // Now change state to PAUSED - should remove from filter
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_STATE_CHANGE, 10001,
        STREAM_USAGE_GAME, RENDERER_PAUSED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_025
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with invalid usage RENDERER_PAUSED to PAUSED transition.
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_025, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb";

    // First add invalid session with RUNNING state
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_RUNNING);
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_NE(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.end());

    // Change state to PAUSED - should remove from filter
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_STATE_CHANGE, 10001,
        STREAM_USAGE_VOICE_COMMUNICATION, RENDERER_PAUSED);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.find(10001),
        hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.end());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_026
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with empty sinkName string (edge case).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_026, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = ""; // Empty string
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Update with empty sinkName - should not modify state due to CHECK_AND_RETURN at start
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001, STREAM_USAGE_MUSIC, RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.empty());
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.empty());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_027
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with out-of-range StreamUsage value (edge case).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_027, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "a2dp";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Use an out-of-range StreamUsage value (999)
    // Should not add to filter due to CHECK_AND_RETURN_LOG for invalid usage
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001,
        static_cast<StreamUsage>(OUT_OF_RANGE_SESSION), RENDERER_RUNNING);

    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.empty());
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.empty());
}

/**
 * @tc.name  : UpdateAuxiliarySinkState_028
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAuxiliarySinkState with maximum StreamUsage value (boundary test).
 */
HWTEST_F(HpaeSinkOutputNodeTest, UpdateAuxiliarySinkState_028, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->sinkOutAttr_.sinkName = "usb";
    hpaeSinkOutputNode->auxSinkState_ = STREAM_MANAGER_IDLE;

    // Use maximum possible StreamUsage value
    StreamUsage maxUsage = static_cast<StreamUsage>(INT32_MAX);
    hpaeSinkOutputNode->UpdateAuxiliarySinkState(STREAM_CHANGE_TYPE_ADD, 10001, maxUsage, RENDERER_RUNNING);

    // Should not add to filter for invalid usage
    EXPECT_EQ(hpaeSinkOutputNode->auxSinkState_, STREAM_MANAGER_IDLE);
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkValidFilter_.empty());
    EXPECT_TRUE(hpaeSinkOutputNode->sessionsWithAuxSinkInvalidFilter_.empty());
}

/**
 * @tc.name  : GetLatency_009
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency verifies latency_ member after GetLatency call fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetLatency_009, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    // Set initial latency value
    hpaeSinkOutputNode->latency_ = 100;

    // Mock GetLatency to fail but still update the reference parameter
    uint32_t newLatency = 250;
    EXPECT_CALL(*mockSink, GetLatency(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(newLatency), Return(ERR_OPERATION_FAILED)));

    uint32_t result = hpaeSinkOutputNode->GetLatency();

    // Verify that latency_ member was updated even though GetLatency failed
    EXPECT_EQ(result, newLatency);
    EXPECT_EQ(hpaeSinkOutputNode->latency_, newLatency);
}

/**
 * @tc.name  : ResetAll_SingleConnectedNode_001
 * @tc.type  : FUNC
 * @tc.desc  : Test ResetAll disconnects all connected nodes (single node case).
 */
HWTEST_F(HpaeSinkOutputNodeTest, ResetAll_SingleConnectedNode_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    // Connect the nodes
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 1U);

    // ResetAll should disconnect the node
    bool result = hpaeSinkOutputNode->ResetAll();
    EXPECT_EQ(result, true);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0U);
}

/**
 * @tc.name  : ResetAll_MultipleConnectedNodes_002
 * @tc.type  : FUNC
 * @tc.desc  : Test ResetAll disconnects all connected nodes (multiple nodes case).
 */
HWTEST_F(HpaeSinkOutputNodeTest, ResetAll_MultipleConnectedNodes_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto hpaeSinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    // Connect multiple nodes
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode1);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode2);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 2U);

    // ResetAll should disconnect all nodes
    bool result = hpaeSinkOutputNode->ResetAll();
    EXPECT_EQ(result, true);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0U);
}

/**
 * @tc.name  : RenderSinkSetPriPaPower_Success_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkSetPriPaPower succeeds with valid sink.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkSetPriPaPower_Success_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, SetPriPaPower())
        .WillOnce(Return(SUCCESS));

    int32_t result = hpaeSinkOutputNode->RenderSinkSetPriPaPower();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : RenderSinkSetPriPaPower_NullSink_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkSetPriPaPower returns ERROR when sink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkSetPriPaPower_NullSink_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = hpaeSinkOutputNode->RenderSinkSetPriPaPower();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : NotifyStreamChangeToSink_Success_001
 * @tc.type  : FUNC
 * @tc.desc  : Test NotifyStreamChangeToSink succeeds with valid sink.
 */
HWTEST_F(HpaeSinkOutputNodeTest, NotifyStreamChangeToSink_Success_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    hpaeSinkOutputNode->state_ = STREAM_MANAGER_RUNNING;

    EXPECT_CALL(*mockSink, IsInited())
        .WillOnce(Return(true));

    // NotifyStreamChangeToSink doesn't return a value, just verify no crash
    hpaeSinkOutputNode->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, 1001,
        STREAM_USAGE_MEDIA, RENDERER_RUNNING, 10001);
}

/**
 * @tc.name  : NotifyStreamChangeToSink_NullSink_002
 * @tc.type  : FUNC
 * @tc.desc  : Test NotifyStreamChangeToSink handles null sink gracefully.
 */
HWTEST_F(HpaeSinkOutputNodeTest, NotifyStreamChangeToSink_NullSink_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    // NotifyStreamChangeToSink should handle null sink gracefully without crash
    hpaeSinkOutputNode->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, 1001,
        STREAM_USAGE_MEDIA, RENDERER_RUNNING, 10001);
}

/**
 * @tc.name  : SetCollaborationState_True_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetCollaborationState sets collaborationState to true.
 */
HWTEST_F(HpaeSinkOutputNodeTest, SetCollaborationState_True_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->SetCollaborationState(true);
    EXPECT_EQ(hpaeSinkOutputNode->collaborationState_.load(), true);
}

/**
 * @tc.name  : SetCollaborationState_False_002
 * @tc.type  : FUNC
 * @tc.desc  : Test SetCollaborationState sets collaborationState to false.
 */
HWTEST_F(HpaeSinkOutputNodeTest, SetCollaborationState_False_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // Set initial state to true
    hpaeSinkOutputNode->SetCollaborationState(true);
    EXPECT_EQ(hpaeSinkOutputNode->collaborationState_.load(), true);

    // Set state to false
    hpaeSinkOutputNode->SetCollaborationState(false);
    EXPECT_EQ(hpaeSinkOutputNode->collaborationState_.load(), false);
}

/**
 * @tc.name  : RenderSinkStart_NullSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStart returns ERROR when sink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkStart_NullSink_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = hpaeSinkOutputNode->RenderSinkStart();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderSinkStart_ErrorReturn_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStart returns error when sink Start fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkStart_ErrorReturn_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Start())
        .WillOnce(Return(ERR_OPERATION_FAILED));

    int32_t result = hpaeSinkOutputNode->RenderSinkStart();
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : RenderSinkStop_NullSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStop returns ERROR when sink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkStop_NullSink_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = hpaeSinkOutputNode->RenderSinkStop();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderSinkStop_ErrorReturn_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStop returns error when sink Stop fails.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkStop_ErrorReturn_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, Stop())
        .WillOnce(Return(ERR_OPERATION_FAILED));

    int32_t result = hpaeSinkOutputNode->RenderSinkStop();
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : RenderSinkPause_NullSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkPause returns ERROR when sink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, RenderSinkPause_NullSink_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = hpaeSinkOutputNode->RenderSinkPause();
    EXPECT_EQ(result, ERROR);
}

// FaultCode test constants
static constexpr int32_t TEST_APP_UID = 1001;
static constexpr int32_t TEST_APP_UID_SECOND = 1002;

/**
 * @tc.name  : FaultCode_RenderSinkInit_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkInit returns ERROR when audioRendererSink_ is nullptr,
 *             covering PLAY_CREATE_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeSinkOutputNodeTest, FaultCode_RenderSinkInit_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;
    IAudioSinkAttr attr;
    attr.adapterName = "primary";

    int32_t result = hpaeSinkOutputNode->RenderSinkInit(attr);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_RenderSinkInit_SinkInitFail
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkInit returns error when sink Init fails,
 *             covering PLAY_CREATE_STREAM_FAIL fault code path.
 */
HWTEST_F(HpaeSinkOutputNodeTest, FaultCode_RenderSinkInit_SinkInitFail, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink, Init(_)).WillOnce(Return(ERR_OPERATION_FAILED));

    IAudioSinkAttr attr;
    attr.adapterName = "primary";
    int32_t result = hpaeSinkOutputNode->RenderSinkInit(attr);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : FaultCode_UpdateAppsUid_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAppsUid returns ERROR when audioRendererSink_ is nullptr,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeSinkOutputNodeTest, FaultCode_UpdateAppsUid_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;
    std::vector<int32_t> appsUid = {TEST_APP_UID, TEST_APP_UID_SECOND};

    int32_t result = hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_UpdateAppsUid_SinkNotInited
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAppsUid returns ERR_ILLEGAL_STATE when sink is not initialized,
 *             covering PLAY_START_STATE_ILLEGAL fault code path.
 */
HWTEST_F(HpaeSinkOutputNodeTest, FaultCode_UpdateAppsUid_SinkNotInited, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(false));

    std::vector<int32_t> appsUid = {TEST_APP_UID};
    int32_t result = hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : FaultCode_RenderSinkDeInit_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkDeInit returns ERROR when audioRendererSink_ is nullptr.
 */
HWTEST_F(HpaeSinkOutputNodeTest, FaultCode_RenderSinkDeInit_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);

    hpaeSinkOutputNode->audioRendererSink_ = nullptr;

    int32_t result = hpaeSinkOutputNode->RenderSinkDeInit();
    EXPECT_EQ(result, ERROR);
}
// ==================== New UT for GetRenderId and GetCurrentActiveDevice ====================

/**
 * @tc.name  : GetRenderId_Default_001
 * @tc.type  : FUNC
 * @tc.number: GetRenderId_Default_001
 * @tc.desc  : Test GetRenderId returns HDI_INVALID_ID by default.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetRenderId_Default_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderId(), HDI_INVALID_ID);
}

/**
 * @tc.name  : GetRenderId_SetValue_001
 * @tc.type  : FUNC
 * @tc.number: GetRenderId_SetValue_001
 * @tc.desc  : Test GetRenderId returns correct value after setting private member.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetRenderId_SetValue_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->renderId_ = 100;
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderId(), 100u);
}

/**
 * @tc.name  : GetCurrentActiveDevice_NullSink_001
 * @tc.type  : FUNC
 * @tc.number: GetCurrentActiveDevice_NullSink_001
 * @tc.desc  : Test GetCurrentActiveDevice returns DEVICE_TYPE_NONE when sink is null.
 */
HWTEST_F(HpaeSinkOutputNodeTest, GetCurrentActiveDevice_NullSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    // audioRendererSink_ is nullptr by default
    EXPECT_EQ(hpaeSinkOutputNode->GetCurrentActiveDevice(), DEVICE_TYPE_NONE);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS