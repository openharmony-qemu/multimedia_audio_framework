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
#include "hpae_process_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_sink_input_node.h"
#include "hpae_remote_sink_output_node.h"
#include "hpae_mixer_node.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class HpaeRemoteSinkOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeRemoteSinkOutputNodeTest::SetUp()
{}

void HpaeRemoteSinkOutputNodeTest::TearDown()
{}

HWTEST_F(HpaeRemoteSinkOutputNodeTest, constructNode_01, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo1;
    nodeInfo1.nodeId = 1001; // 1001: node id
    nodeInfo1.frameLen = 960; // 960: frameLen
    nodeInfo1.sessionId = 123456; // 123456: sessionId
    nodeInfo1.samplingRate = SAMPLE_RATE_48000;
    nodeInfo1.channels = STEREO;
    nodeInfo1.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRemoteSinkOutputNode> hpaeRemoteSinkOutputNode =
        std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo1, sinkInfo);

    HpaeNodeInfo nodeInfo2;
    nodeInfo2.nodeId = 1002; // 1002: nodeId
    nodeInfo2.frameLen = 960; // 960: frameLen
    nodeInfo2.samplingRate = SAMPLE_RATE_48000;
    nodeInfo2.channels = STEREO;
    nodeInfo2.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo2);

    hpaeRemoteSinkOutputNode->Connect(hpaeMixerNode);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->GetRenderSinkInstance("remote", "LocalDevice"), 0);
    hpaeRemoteSinkOutputNode->DoProcess();
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkDeInit(), SUCCESS);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkFlush(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkPause(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkReset(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkResume(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkStart(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkStop(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->GetPreOutNum(), 1);
    
    hpaeRemoteSinkOutputNode->DisConnect(hpaeMixerNode);
}

/**
 * @tc.name  : HandlePcmDumping_001
 * @tc.type  : FUNC
 * @tc.desc  : Test PCM dumping logic with correct Dumper construction.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, HandlePcmDumping_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    size_t dataSize = 1024;
    std::vector<char> mockData(dataSize, 0x5A);
    char* dataPtr = mockData.data();

    remoteNode->outputMediaPcmDumper_ = nullptr;
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_MEDIA, dataPtr, dataSize);

    remoteNode->outputMediaPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_media_dump.pcm");
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_MEDIA, dataPtr, dataSize);

    remoteNode->outputNavigationPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_nav_dump.pcm");
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_NAVIGATION, dataPtr, dataSize);

    remoteNode->outputCommunicationPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_comm_dump.pcm");
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_COMMUNICATION, dataPtr, dataSize);
}

/**
 * @tc.name  : ResetAll_001
 * @tc.type  : FUNC
 * @tc.number: ResetAll_001
 * @tc.desc  : Test ResetAll to cover node traversal and conditional disconnection.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, ResetAll_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    HpaeNodeInfo infoA;
    infoA.nodeId = 3001;
    auto nodeA = std::make_shared<HpaeMixerNode>(infoA);
    remoteNode->Connect(nodeA);

    HpaeNodeInfo infoB;
    infoB.nodeId = 3002;
    auto nodeB = std::make_shared<HpaeMixerNode>(infoB);
    remoteNode->Connect(nodeB);

    EXPECT_EQ(remoteNode->GetPreOutNum(), 2);

    bool result = remoteNode->ResetAll();

    EXPECT_TRUE(result);

    EXPECT_EQ(remoteNode->GetPreOutNum(), 0);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS