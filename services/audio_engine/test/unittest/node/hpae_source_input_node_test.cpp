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
#include <memory>
#include <vector>
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_format_convert.h"
#include "hpae_mocks.h"
#include "manager/hdi_adapter_manager.h"

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
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";

class HpaeSourceInputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceInputNodeTest::SetUp()
{}

void HpaeSourceInputNodeTest::TearDown()
{}

HWTEST_F(HpaeSourceInputNodeTest, constructHpaeSourceInputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSourceInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSourceInputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSourceInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

HWTEST_F(HpaeSourceInputNodeTest, testSourceInputOutputCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 1);
    {
        std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = hpaeSourceInputNode;
        EXPECT_EQ(hpaeSourceInputNode.use_count(), 2);  // 2 for test
        std::shared_ptr<HpaeNode> hpaeNode = outputNode->GetSharedInstance();
        EXPECT_EQ(hpaeSourceInputNode.use_count(), 3);  // 3 for test
        EXPECT_EQ(hpaeNode->GetSampleRate(), nodeInfo.samplingRate);
        EXPECT_EQ(hpaeNode->GetFrameLen(), nodeInfo.frameLen);
        EXPECT_EQ(hpaeNode->GetChannelCount(), nodeInfo.channels);
        EXPECT_EQ(hpaeNode->GetBitWidth(), nodeInfo.format);
    }
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 1);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    hpaeSourceOutputNode->Connect(hpaeSourceInputNode);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 2);  // 2 for test
    OutputPort<HpaePcmBuffer *> *outputPort = hpaeSourceInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 1);
    hpaeSourceOutputNode->DisConnect(hpaeSourceInputNode);
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 1);
    outputPort = hpaeSourceInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 0);
}

static void InitAudioSourceAttr(IAudioSourceAttr &attr, const HpaeNodeInfo &nodeInfo)
{
    attr.adapterName = "";
    attr.openMicSpeaker = 0;
    attr.format = AudioSampleFormat::INVALID_WIDTH;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.bufferSize = 0;
    attr.isBigEndian = false;
    attr.filePath = g_rootCapturerPath;
    attr.deviceNetworkId = "";
    attr.deviceType = 0;
    attr.sourceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
}

HWTEST_F(HpaeSourceInputNodeTest, testWriteDataToSourceInputDataCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    InitAudioSourceAttr(attr, nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState() == STREAM_MANAGER_RUNNING, true);
    hpaeSourceInputNode->DoProcess();
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState() == STREAM_MANAGER_SUSPENDED, true);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);
}

HWTEST_F(HpaeSourceInputNodeTest, testInterfaces_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_MICREF;
    std::vector<HpaeNodeInfo> nodeVector;
    nodeVector.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeVector.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_EC;
    nodeVector.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeVector);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    hpaeSourceOutputNode->Connect(hpaeSourceInputNode);

    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance("file_io", "LocalDevice", SOURCE_TYPE_WAKEUP, ""), 0);
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance("file_io", "LocalDevice", SOURCE_TYPE_MIC, "mic"), 0);
    hpaeSourceInputNode->SetSourceInputNodeType(HpaeSourceInputNodeType::HPAE_SOURCE_MIC_EC);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo), HPAE_SOURCE_BUFFER_TYPE_EC);
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo), HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_NE(hpaeSourceInputNode->GetOutputPort(), nullptr);
    EXPECT_NE(hpaeSourceInputNode->GetOutputPort(nodeInfo, false), nullptr);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortNum(), 0);
    EXPECT_NE(hpaeSourceInputNode->GetCaptureId(), 0);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortNum(nodeInfo), 0);
    hpaeSourceInputNode->GetNodeInfoWithInfo(nodeInfo.sourceBufferType);
    hpaeSourceInputNode->DoProcess();
    std::vector<int32_t> appsUid;
    std::vector<int32_t> sessionsId;
    constexpr int32_t testUid = 55;
    constexpr int32_t testSessionId = 66;
    appsUid.push_back(testUid);
    sessionsId.push_back(testSessionId);
    hpaeSourceInputNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceInputNodeType(), HpaeSourceInputNodeType::HPAE_SOURCE_MIC_EC);
    hpaeSourceInputNode->SetSourceInputNodeType(HpaeSourceInputNodeType::HPAE_SOURCE_MIC);
    EXPECT_NE(hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo), HPAE_SOURCE_BUFFER_TYPE_EC);
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance("file_io", "LocalDevice", SOURCE_TYPE_MIC, "mic"), 0);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortNum(), 1);
    EXPECT_EQ(hpaeSourceInputNode->Reset(), true);
    EXPECT_EQ(hpaeSourceInputNode->ResetAll(), true);
}

HWTEST_F(HpaeSourceInputNodeTest, testInterfaces_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint64_t requestBytes = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    std::vector<char> testData(requestBytes);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);

    EXPECT_NE(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);

    attr.filePath = "";
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);
}

HWTEST_F(HpaeSourceInputNodeTest, testDoprocess_001, TestSize.Level0)
{
    std::vector<HpaeNodeInfo> vec;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC_EC;
    vec.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    vec.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(vec);
        std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    hpaeSourceInputNode->DoProcess();
    EXPECT_NE(hpaeSourceInputNode, nullptr);
}

HWTEST_F(HpaeSourceInputNodeTest, testDoprocess_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_EC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
        std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    hpaeSourceInputNode->DoProcess();
    EXPECT_NE(hpaeSourceInputNode, nullptr);
}

HWTEST_F(HpaeSourceInputNodeTest, testDoprocess_003, TestSize.Level0)
{
    std::vector<HpaeNodeInfo> vec;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC_EC;
    vec.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    vec.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(vec);
        std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    hpaeSourceInputNode->CapturerSourceInit(attr);

    hpaeSourceInputNode->SetSourceInputNodeType(HpaeSourceInputNodeType::HPAE_SOURCE_OFFLOAD);
    hpaeSourceInputNode->DoProcess();
    EXPECT_NE(hpaeSourceInputNode, nullptr);
}

/**
 * @tc.name  : Test DoProcessMicInner
 * @tc.type  : FUNC
 * @tc.number: DoProcessInjectSleepTest_001
 * @tc.desc  : Test InitCapturerManager while inject=true, reply!=0, not early return
 */
HWTEST_F(HpaeSourceInputNodeTest, DoProcessInjectSleepTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->isInjecting_ = true;

    auto historyRemain = hpaeSourceInputNode->historyRemainSizeMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_EQ(historyRemain != hpaeSourceInputNode->historyRemainSizeMap_.end(), true);
    EXPECT_EQ(historyRemain->second, 0); // no data, so historyremian = 0

    EXPECT_CALL(*mockCaptureSource, CaptureFrame(_, _, _))
        .WillOnce([](char *frame, uint64_t requestBytes, uint64_t &replyBytes) {
            replyBytes = requestBytes;
            return SUCCESS;
        });
    hpaeSourceInputNode->DoProcess();
    EXPECT_EQ(historyRemain->second, 0); // not early return, data write to outputStream, so historyremain = 0
}

/**
 * @tc.name  : Test DoProcessMicInner
 * @tc.type  : FUNC
 * @tc.number: DoProcessInjectSleepTest_002
 * @tc.desc  : Test InitCapturerManager while inject=true, reply=0 and usbState=1, not early return
 */
HWTEST_F(HpaeSourceInputNodeTest, DoProcessInjectSleepTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->isInjecting_ = true;

    auto historyRemain = hpaeSourceInputNode->historyRemainSizeMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_EQ(historyRemain != hpaeSourceInputNode->historyRemainSizeMap_.end(), true);
    EXPECT_EQ(historyRemain->second, 0); // no data, so historyremian = 0
    auto historyData = hpaeSourceInputNode->historyDataMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_EQ(historyData->second.size(), 0);
    uint64_t fillSize = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    historyRemain->second += fillSize - 1; // fill history data
    historyData->second.resize(fillSize - 1); // fill history data
    EXPECT_CALL(*mockCaptureSource, CaptureFrame(_, _, _))
        .WillOnce([](char *frame, uint64_t requestBytes, uint64_t &replyBytes) {
            replyBytes = 0;
            return SUCCESS;
        });
    EXPECT_CALL(*mockCaptureSource, GetArmUsbDeviceStatus())
        .WillOnce([]() {
            return 1;
        });
    hpaeSourceInputNode->DoProcess();
    // not early return, reply=0 and history data write to outputStream, so historyremain = 0
    EXPECT_EQ(historyRemain->second, 0);
    EXPECT_EQ(historyData->second.size(), 0);
}

/**
 * @tc.name  : Test DoProcessMicInner
 * @tc.type  : FUNC
 * @tc.number: DoProcessInjectSleepTest_003
 * @tc.desc  : Test InitCapturerManager while inject=true, reply=0 and usbState=1, early return
 */
HWTEST_F(HpaeSourceInputNodeTest, DoProcessInjectSleepTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->isInjecting_ = true;

    auto historyRemain = hpaeSourceInputNode->historyRemainSizeMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_EQ(historyRemain != hpaeSourceInputNode->historyRemainSizeMap_.end(), true);
    EXPECT_EQ(historyRemain->second, 0); // no data, so historyremian = 0
    auto historyData = hpaeSourceInputNode->historyDataMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_EQ(historyData->second.size(), 0);
    uint64_t fillSize = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    historyRemain->second += fillSize - 1; // fill history data
    historyData->second.resize(fillSize - 1); // fill history data
    EXPECT_CALL(*mockCaptureSource, CaptureFrame(_, _, _))
        .WillOnce([](char *frame, uint64_t requestBytes, uint64_t &replyBytes) {
            replyBytes = 0;
            return SUCCESS;
        });
    EXPECT_CALL(*mockCaptureSource, GetArmUsbDeviceStatus())
        .WillOnce([]() {
            return 0;
        });
    hpaeSourceInputNode->DoProcess();
    // early return, reply=0 and history data not write to outputStream, so historyremain!=0
    EXPECT_NE(historyRemain->second, 0);
    EXPECT_NE(historyData->second.size(), 0);
}

/**
 * @tc.name  : Test CapturerSourceStop
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStopTest_001
 * @tc.desc  : Test CapturerSourceStop while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStopTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourceStop
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStopTest_002
 * @tc.desc  : Test CapturerSourceStop while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStopTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourceStop
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStopTest_003
 * @tc.desc  : Test CapturerSourceStop while audioCapturerSource is not inited
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStopTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(false)); // hdi not init
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourceStop
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStopTest_004
 * @tc.desc  : Test CapturerSourceStop while audioCapturerSource stop fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStopTest_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(true));
    EXPECT_CALL(*mockCaptureSource, Stop())
        .WillOnce(Return(ERROR));
    // iAudioCapturerSource stop fail does not block sourceInputNode, so interface return is SUCCESS
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourceStart
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStartTest_001
 * @tc.desc  : Test CapturerSourceStart while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStartTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceStart
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStartTest_002
 * @tc.desc  : Test CapturerSourceStart while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStartTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceStart
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStartTest_003
 * @tc.desc  : Test CapturerSourceStart while audioCapturerSource is not inited
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStartTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(false)); // hdi not init
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceStart
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceStartTest_004
 * @tc.desc  : Test CapturerSourceStart while audioCapturerSource start fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceStartTest_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(true));
    EXPECT_CALL(*mockCaptureSource, Start())
        .WillOnce(Return(ERROR));
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceFlush
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceFlushTest_001
 * @tc.desc  : Test CapturerSourceFlush while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceFlushTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourceFlush
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceFlushTest_002
 * @tc.desc  : Test CapturerSourceFlush while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceFlushTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = HDI_INVALID_ID;
    
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourceFlush
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceFlushTest_003
 * @tc.desc  : Test CapturerSourceFlush while audioCapturerSource is not inited
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceFlushTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(false)); // hdi not init
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourceFlush
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceFlushTest_004
 * @tc.desc  : Test CapturerSourceFlush while audioCapturerSource flush fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceFlushTest_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(true));
    EXPECT_CALL(*mockCaptureSource, Flush())
        .WillOnce(Return(ERROR)); // flush fail
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourcePause
 * @tc.type  : FUNC
 * @tc.number: CapturerSourcePauseTest_001
 * @tc.desc  : Test CapturerSourcePause while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourcePauseTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourcePause
 * @tc.type  : FUNC
 * @tc.number: CapturerSourcePauseTest_002
 * @tc.desc  : Test CapturerSourcePause while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourcePauseTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = HDI_INVALID_ID;
    
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourcePause
 * @tc.type  : FUNC
 * @tc.number: CapturerSourcePauseTest_003
 * @tc.desc  : Test CapturerSourcePause while audioCapturerSource is not inited
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourcePauseTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(false)); // hdi not init
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourcePause
 * @tc.type  : FUNC
 * @tc.number: CapturerSourcePauseTest_004
 * @tc.desc  : Test CapturerSourcePause while audioCapturerSource pause fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourcePauseTest_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(true));
    EXPECT_CALL(*mockCaptureSource, Pause())
        .WillOnce(Return(ERROR)); // pause fail
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : Test CapturerSourceReset
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceResetTest_001
 * @tc.desc  : Test CapturerSourceReset while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceResetTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourceReset
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceResetTest_002
 * @tc.desc  : Test CapturerSourceReset while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceResetTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = HDI_INVALID_ID;
    
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourceReset
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceResetTest_003
 * @tc.desc  : Test CapturerSourceReset while audioCapturerSource reset fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceResetTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, Reset())
        .WillOnce(Return(ERROR)); // reset fail
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
}

/**
 * @tc.name  : Test CapturerSourceResume
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceResumeTest_001
 * @tc.desc  : Test CapturerSourceResume while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceResumeTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceResume
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceResumeTest_002
 * @tc.desc  : Test CapturerSourceResume while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceResumeTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = HDI_INVALID_ID;
    
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceResume
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceResumeTest_003
 * @tc.desc  : Test CapturerSourceResume while audioCapturerSource resume fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceResumeTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, Resume())
        .WillOnce(Return(ERROR)); // resume fail
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : Test CapturerSourceInit
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceInitTest_001
 * @tc.desc  : Test CapturerSourceInit while audioCapturerSource is null
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceInitTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_NEW);
    IAudioSourceAttr attr;

    EXPECT_EQ(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_NEW);
}

/**
 * @tc.name  : Test CapturerSourceInit
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceInitTest_002
 * @tc.desc  : Test CapturerSourceInit while captureId is invalid
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceInitTest_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    IAudioSourceAttr attr;
    
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_NEW);
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = HDI_INVALID_ID;
    
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(hpaeSourceInputNode->captureId_, HDI_INVALID_ID);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_NEW);
}

/**
 * @tc.name  : Test CapturerSourceInit
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceInitTest_003
 * @tc.desc  : Test CapturerSourceInit while audioCapturerSource is already inited
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceInitTest_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    IAudioSourceAttr attr;

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_NEW);
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(true)); // already inited
        
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : Test CapturerSourceInit
 * @tc.type  : FUNC
 * @tc.number: CapturerSourceInitTest_004
 * @tc.desc  : Test CapturerSourceInit while audioCapturerSource init fail
 */
HWTEST_F(HpaeSourceInputNodeTest, CapturerSourceInitTest_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    IAudioSourceAttr attr;

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    hpaeSourceInputNode->captureId_ = 1;
    
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillOnce(Return(false)); // not inited
    EXPECT_CALL(*mockCaptureSource, Init(_))
        .WillOnce(Return(ERROR)); // init fail
        
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_IDLE);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState(), STREAM_MANAGER_NEW);
}

/**
 * @tc.name  : Test CaptureFrame fail
 * @tc.type  : FUNC
 * @tc.number: CaptureFrameFailTest_001
 * @tc.desc  : Test HpaeSourceInputNode CaptureFrame fail and sleep
 */
HWTEST_F(HpaeSourceInputNodeTest, CaptureFrameFailTest_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);

    EXPECT_CALL(*mockCaptureSource, CaptureFrame(_, _, _))
        .WillRepeatedly([](char *frame, uint64_t requestBytes, uint64_t &replyBytes) {
            replyBytes = 0;
            return ERROR;
        });
    uint64_t reply = 0;
    for (int32_t i = 0; i < 21; i++) { // 21 for test times
        hpaeSourceInputNode->ReadDataFromSource(nodeInfo.sourceBufferType, reply);
        EXPECT_EQ(hpaeSourceInputNode->backoffController_.delay_, std::min(i+1, 20));
    }
}

/**
 * @tc.name  : Test CaptureFrame fail
 * @tc.type  : FUNC
 * @tc.number: CaptureFrameFailTest_002
 * @tc.desc  : Test HpaeSourceInputNode micref CaptureFrame fail and sleep
 */
HWTEST_F(HpaeSourceInputNodeTest, CaptureFrameFailTest_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MICREF;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MICREF;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    hpaeSourceInputNode->audioCapturerSource_ = mockCaptureSource;
    EXPECT_NE(hpaeSourceInputNode->audioCapturerSource_, nullptr);

    EXPECT_CALL(*mockCaptureSource, CaptureFrame(_, _, _))
        .WillRepeatedly([](char *frame, uint64_t requestBytes, uint64_t &replyBytes) {
            replyBytes = 0;
            return ERROR;
        });
    uint64_t reply = 0;
    for (int32_t i = 0; i < 5; i++) { // 5 for test times
        hpaeSourceInputNode->ReadDataFromSource(nodeInfo.sourceBufferType, reply);
        EXPECT_EQ(hpaeSourceInputNode->backoffController_.delay_, 0); // micref not sleep
    }
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS