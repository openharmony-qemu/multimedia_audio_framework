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
#include "hpae_source_input_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
#include "hpae_format_convert.h"
#include "hpae_mocks.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";

class HpaeSourceInputClusterTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceInputClusterTest::SetUp()
{}

void HpaeSourceInputClusterTest::TearDown()
{}

HWTEST_F(HpaeSourceInputClusterTest, constructHpaeSourceInputClusterNode, TestSize.Level0)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.statusCallback = testStatuscallback;

    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceInputCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSourceInputCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSourceInputCluster->GetBitWidth(), nodeInfo.format);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->Connect(hpaeSourceInputCluster);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1 + 1);
    EXPECT_EQ(hpaeSourceInputCluster->GetConverterNodeCount(), 0);

    nodeInfo.samplingRate = SAMPLE_RATE_16000;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode1 = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode1->ConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1 + 1 + 1);
    EXPECT_EQ(hpaeSourceInputCluster->GetConverterNodeCount(), 1);

    hpaeSourceOutputNode1->DisConnectWithInfo(hpaeSourceInputCluster, hpaeSourceOutputNode1->GetNodeInfo());
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1 + 1);
    EXPECT_EQ(hpaeSourceInputCluster->GetConverterNodeCount(), 1); // no delete converter now
}

HWTEST_F(HpaeSourceInputClusterTest, testInterfaces, TestSize.Level0)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = testStatuscallback;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    hpaeSourceInputCluster->DoProcess();
    hpaeSourceInputCluster->ResetAll();
    EXPECT_NE(hpaeSourceInputCluster->GetSharedInstance(nodeInfo), nullptr);
    EXPECT_NE(hpaeSourceInputCluster->CapturerSourcePause(), 0);
    EXPECT_NE(hpaeSourceInputCluster->CapturerSourceResume(), 0);
    EXPECT_NE(hpaeSourceInputCluster->CapturerSourceReset(), 0);
    EXPECT_EQ(hpaeSourceInputCluster->GetOutputPortNum(nodeInfo), 0);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeType(), HPAE_SOURCE_DEFAULT);
    EXPECT_NE(hpaeSourceInputCluster->CapturerSourceFlush(), 0);

    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MICREF;
    hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
}

static std::shared_ptr<MockAudioCaptureSource> SetMockSourceInputNode(
    std::shared_ptr<HpaeSourceInputCluster> &sourceInputCluster, bool isInited = true)
{
    if (!sourceInputCluster) {
        return nullptr;
    }
    auto sourceInputNode = sourceInputCluster->sourceInputNode_;
    EXPECT_NE(sourceInputNode, nullptr);
    auto mockCaptureSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    sourceInputNode->audioCapturerSource_ = mockCaptureSource;
    sourceInputNode->captureId_ = 1;
    EXPECT_CALL(*mockCaptureSource, IsInited())
        .WillRepeatedly(Return(isInited));
    return mockCaptureSource;
}

/**
 * @tc.name  : Test HpaeSourceInputCluster init
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterInitTest
 * @tc.desc  : Test HpaeSourceInputCluster init interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterInitTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster, false);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Init(_))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceInit(attr), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Init(_))
        .WillOnce(Return(ERROR));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceInit(attr), ERROR);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster Flush
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterFlushTest
 * @tc.desc  : Test HpaeSourceInputCluster Flush interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterFlushTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Flush())
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceFlush(), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Flush())
        .WillOnce(Return(ERROR));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceFlush(), ERROR);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster Pause
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterPauseTest
 * @tc.desc  : Test HpaeSourceInputCluster Pause interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterPauseTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Pause())
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourcePause(), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Pause())
        .WillOnce(Return(ERROR));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourcePause(), ERROR);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster Reset
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterResetTest
 * @tc.desc  : Test HpaeSourceInputCluster Reset interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterResetTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Reset())
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceReset(), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Reset())
        .WillOnce(Return(ERROR));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceReset(), ERROR);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster Resume
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterResumeTest
 * @tc.desc  : Test HpaeSourceInputCluster Resume interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterResumeTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Resume())
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceResume(), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Resume())
        .WillOnce(Return(ERROR));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceResume(), ERROR);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster Start
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterStartTest
 * @tc.desc  : Test HpaeSourceInputCluster Start interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterStartTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Start())
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceStart(), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Start())
        .WillOnce(Return(ERROR));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceStart(), ERROR);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster Stop
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterStopTest
 * @tc.desc  : Test HpaeSourceInputCluster Stop interface return SUCCESS and ERROR
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterStopTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto mockCaptureSource = SetMockSourceInputNode(hpaeSourceInputCluster);
    IAudioSourceAttr attr;
    EXPECT_CALL(*mockCaptureSource, Stop())
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceStop(), SUCCESS);

    EXPECT_CALL(*mockCaptureSource, Stop())
        .WillOnce(Return(ERROR));
    // iAudioCapturerSource stop fail does not block sourceInputNode, so interface return is SUCCESS
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceStop(), SUCCESS);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster constructor
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterConstructorTest_001
 * @tc.desc  : Test HpaeSourceInputCluster constructor with vector parameter
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterConstructorTest_001, TestSize.Level1)
{
    std::vector<HpaeNodeInfo> vec;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sourceType = SOURCE_TYPE_MIC;
    nodeInfo.deviceName = "Begin";
    vec.emplace_back(nodeInfo);
    nodeInfo.sourceType = SOURCE_TYPE_INVALID;
    nodeInfo.deviceName = "End";
    vec.emplace_back(nodeInfo);
    EXPECT_EQ(vec.size(), 2); // 2 for size
    auto sourceInputCluster = std::make_shared<HpaeSourceInputCluster>(vec);
    EXPECT_NE(sourceInputCluster, nullptr);
    HpaeNodeInfo clusterInfo = sourceInputCluster->GetNodeInfo();
    EXPECT_EQ(clusterInfo.sourceType, vec.begin()->sourceType);
    EXPECT_EQ(clusterInfo.deviceName, vec.begin()->deviceName);
}

/**
 * @tc.name  : Test HpaeSourceInputCluster ResetAll
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceInputClusterResetAllTest_001
 * @tc.desc  : Test HpaeSourceInputCluster ResetAll with convertMap not empty
 */
HWTEST_F(HpaeSourceInputClusterTest, HpaeSourceInputClusterResetAllTest_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    auto &fmtConverMap = hpaeSourceInputCluster->fmtConverterNodeMap_;

    nodeInfo.channels = MONO;
    auto node1 = hpaeSourceInputCluster->GetSharedInstance(nodeInfo);
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(fmtConverMap.size(), 1);

    nodeInfo.format = SAMPLE_S16LE;
    auto node2 = hpaeSourceInputCluster->GetSharedInstance(nodeInfo);
    EXPECT_NE(node2, nullptr);
    EXPECT_EQ(fmtConverMap.size(), 2); // 2 for converterMap size
    EXPECT_EQ(hpaeSourceInputCluster->ResetAll(), true);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS