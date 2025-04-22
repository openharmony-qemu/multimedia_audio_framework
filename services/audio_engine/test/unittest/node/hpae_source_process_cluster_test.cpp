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
#include "hpae_source_process_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_source_output_node.h"
#include "hpae_source_input_cluster.h"


namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;

class HpaeSourceProcessClusterTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceProcessClusterTest::SetUp()
{}

void HpaeSourceProcessClusterTest::TearDown()
{}

TEST_F(HpaeSourceProcessClusterTest, constructHpaeSourceProcessClusterNode)
{
    std::shared_ptr<NodeStatusCallback> g_testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = g_testStatuscallback->GetWeakPtr();
    std::shared_ptr<HpaeSourceProcessCluster> hpaeSourceProcessCluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceProcessCluster->GetNodeId(), 0);
    EXPECT_EQ(hpaeSourceProcessCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSourceProcessCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSourceProcessCluster->GetBitWidth(), nodeInfo.format);
    EXPECT_EQ(hpaeSourceProcessCluster->GetCapturerEffectNodeUseCount(), 1);

    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode1 = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode1->ConnectWithInfo(hpaeSourceProcessCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetCapturerEffectNodeUseCount(), 1 + 1);
    EXPECT_EQ(hpaeSourceProcessCluster->GetConverterNodeCount(), 1);

    nodeInfo.samplingRate = SAMPLE_RATE_16000;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode2 = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode2->ConnectWithInfo(hpaeSourceProcessCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetCapturerEffectNodeUseCount(), 1 + 1 + 1);
    EXPECT_EQ(hpaeSourceProcessCluster->GetConverterNodeCount(), 1 + 1);

    hpaeSourceOutputNode2->DisConnectWithInfo(hpaeSourceProcessCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetCapturerEffectNodeUseCount(), 1 + 1);

    HpaeNodeInfo sourceInputNodeInfo;
    sourceInputNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInputNodeInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInputNodeInfo.channels = STEREO;
    sourceInputNodeInfo.format = SAMPLE_F32LE;
    sourceInputNodeInfo.statusCallback = g_testStatuscallback->GetWeakPtr();
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(sourceInputNodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetPreOutNum(), 0);
    hpaeSourceProcessCluster->ConnectWithInfo(hpaeSourceInputCluster, hpaeSourceProcessCluster->GetNodeInfo());
    EXPECT_EQ(hpaeSourceProcessCluster->GetPreOutNum(), 1);
    hpaeSourceProcessCluster->DisConnectWithInfo(hpaeSourceInputCluster, hpaeSourceProcessCluster->GetNodeInfo());
    EXPECT_EQ(hpaeSourceProcessCluster->GetPreOutNum(), 0);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS