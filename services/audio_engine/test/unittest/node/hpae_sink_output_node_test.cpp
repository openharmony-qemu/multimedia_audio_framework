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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const char *ROOT_PATH = "/data/source_file_io_48000_2_s16le.pcm";
class HpaeSinkOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSinkOutputNodeTest::SetUp()
{}

void HpaeSinkOutputNodeTest::TearDown()
{}

class MockAudioRenderSink : public IAudioRenderSink {
public:
    MOCK_METHOD(int32_t, Init, (const IAudioSinkAttr &attr), (override));
    MOCK_METHOD(void, DeInit, (), (override));
    MOCK_METHOD(bool, IsInited, (), (override));

    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Stop, (), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));
    MOCK_METHOD(int32_t, Pause, (), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
    MOCK_METHOD(int32_t, Reset, (), (override));
    MOCK_METHOD(int32_t, RenderFrame, (char &data, uint64_t len, uint64_t &writeLen), (override));
    MOCK_METHOD(int64_t, GetVolumeDataCount, (), (override));

    MOCK_METHOD(int32_t, SuspendRenderSink, (), (override));
    MOCK_METHOD(int32_t, RestoreRenderSink, (), (override));

    MOCK_METHOD(void, SetAudioParameter,
        (const AudioParamKey key, const std::string &condition, const std::string &value), (override));
    MOCK_METHOD(std::string, GetAudioParameter,
        (const AudioParamKey key, const std::string &condition), (override));

    MOCK_METHOD(int32_t, SetVolume, (float left, float right), (override));
    MOCK_METHOD(int32_t, GetVolume, (float &left, float &right), (override));

    MOCK_METHOD(int32_t, GetLatency, (uint32_t &latency), (override));
    MOCK_METHOD(int32_t, GetTransactionId, (uint64_t &transactionId), (override));
    MOCK_METHOD(int32_t, GetPresentationPosition,
        (uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec), (override));
    MOCK_METHOD(float, GetMaxAmplitude, (), (override));
    MOCK_METHOD(void, SetAudioMonoState, (bool audioMono), (override));
    MOCK_METHOD(void, SetAudioBalanceValue, (float audioBalance), (override));

    int32_t SetSinkMuteForSwitchDevice(bool /* mute */) override { return 0; }
    int32_t SetDeviceConnectedFlag(bool /* flag */) override { return -1; }
    void SetSpeed(float /* speed */) override {}

    MOCK_METHOD(int32_t, SetAudioScene, (AudioScene audioScene, bool scoExcludeFlag), (override));
    MOCK_METHOD(int32_t, GetAudioScene, (), (override));

    MOCK_METHOD(int32_t, UpdateActiveDevice, (std::vector<DeviceType> &outputDevices), (override));

    void RegistCallback(uint32_t /* type */, IAudioSinkCallback * /* callback */) override {}
    void RegistCallback(uint32_t /* type */, std::shared_ptr<IAudioSinkCallback> /* callback */) override {}
    MOCK_METHOD(void, ResetActiveDeviceForDisconnect, (DeviceType device), (override));

    MOCK_METHOD(int32_t, SetPaPower, (int32_t flag), (override));
    MOCK_METHOD(int32_t, SetPriPaPower, (), (override));

    MOCK_METHOD(int32_t, UpdateAppsUid, (const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size), (override));
    MOCK_METHOD(int32_t, UpdateAppsUid, (const std::vector<int32_t> &appsUid), (override));

    int32_t SetRenderEmpty(int32_t /* durationUs */) override { return 0; }
    void SetAddress(const std::string & /* address */) override {}
    void SetInvalidState() override {}

    MOCK_METHOD(void, DumpInfo, (std::string &dumpString), (override));

    bool IsSinkInited() override { return false; }

    int32_t GetMmapBufferInfo(int & /* fd */, uint32_t & /* totalSizeInframe */,
        uint32_t & /* spanSizeInframe */, uint32_t & /* byteSizePerFrame */,
        uint32_t & /* syncInfoSize */) override { return -1; }
    int32_t GetMmapHandlePosition(uint64_t & /* frames */, int64_t & /* timeSec */,
        int64_t & /* timeNanoSec */) override { return -1; }

    int32_t Drain(AudioDrainType /* type */) override { return -1; }
    void RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> /* callback */) override {}
    int32_t RegistDirectHdiCallback(std::function<void(const RenderCallbackType type)> /* callback */) override
    {
        return 0;
    }
    int32_t SetBufferSize(uint32_t /* sizeMs */) override { return -1; }
    int32_t SetOffloadRenderCallbackType(RenderCallbackType /* type */) override { return -1; }
    int32_t LockOffloadRunningLock() override { return -1; }
    int32_t UnLockOffloadRunningLock() override { return -1; }

    int32_t SplitRenderFrame(char & /* data */, uint64_t /* len */, uint64_t & /* writeLen */,
        const char * /* streamType */) override { return -1; }

    int32_t UpdatePrimaryConnectionState(uint32_t /* operation */) override { return -1; }

    void SetDmDeviceType(uint16_t /* dmDeviceType */, DeviceType /* deviceType */) override {}
};

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    size_t frameLen = 960;
    uint32_t nodeId = 1243;
    nodeInfo.nodeId = nodeId;
    nodeInfo.frameLen = frameLen;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
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
    hpaeSinkOutputNode->RenderSinkDeInit();
}

HWTEST_F(HpaeSinkOutputNodeTest, testHpaeSinkOutHandlePaPower, TestSize.Level0)
{
    std::string deviceClass = "primary";
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
    attr.adapterName = "primary";
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
    std::vector<HpaePcmBuffer *> &outputVec = hpaeSinkOutputNode->inputStream_.ReadPreOutputData();
    EXPECT_FALSE(outputVec.empty());
    HpaePcmBuffer *outputData = outputVec.front();
    outputData->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE;
    hpaeSinkOutputNode->isOpenPaPower_ = false;
    hpaeSinkOutputNode->silenceDataUs_ = 500000000; // 500000000 us, long silence time
    hpaeSinkOutputNode->HandlePaPower(outputData);
    hpaeSinkOutputNode->RenderSinkDeInit();
}

HWTEST_F(HpaeSinkOutputNodeTest, testHpaeSinkOutHandlePaPower2, TestSize.Level0)
{
    PcmBufferInfo bufferInfo = { 2, 960, 48000 }; // 2 channel, 960 framelen, 48000 sampleRate
    std::shared_ptr<HpaePcmBuffer> outputData = std::make_shared<HpaePcmBuffer>(bufferInfo);
    outputData->pcmBufferInfo_.state = PCM_BUFFER_STATE_SILENCE;

    uint32_t sessionId = 10001; // default sessionID
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.sessionId = sessionId;
    nodeInfo.deviceClass = "primary"; // primary set pa power
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    auto mockSink = std::make_shared<MockAudioRenderSink>();
    hpaeSinkOutputNode->audioRendererSink_ = mockSink;
    hpaeSinkOutputNode->isOpenPaPower_ = true;
    hpaeSinkOutputNode->silenceDataUs_ = 500000000; // 500000000 us, long silence time

    EXPECT_CALL(*mockSink, GetAudioScene())
        .WillOnce(Return(0))
        .WillOnce(Return(1));
    EXPECT_CALL(*mockSink, SetPaPower(false))
        .WillOnce(Return(0));
    hpaeSinkOutputNode->HandlePaPower(outputData.get());

    hpaeSinkOutputNode->isOpenPaPower_ = true;
    hpaeSinkOutputNode->silenceDataUs_ = 500000000; // 500000000 us, long silence time
    hpaeSinkOutputNode->HandlePaPower(outputData.get());
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
    hpaeSinkOutputNode->RenderSinkDeInit();
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS