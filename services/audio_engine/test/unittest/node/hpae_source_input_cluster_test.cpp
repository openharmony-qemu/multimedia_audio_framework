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
#include "hpae_source_input_node.h"
#include "hpae_source_input_cluster.h"
#include "hpae_source_output_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;

class TestStatusCallback : public INodeCallback, public std::enable_shared_from_this<TestStatusCallback> {
public:
    std::weak_ptr<INodeCallback> GetWeakPtr();
    virtual ~TestStatusCallback() = default;
};

std::weak_ptr<INodeCallback> TestStatusCallback::GetWeakPtr()
{
    return weak_from_this();
}


class HpaeSourceInputClusterTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceInputClusterTest::SetUp()
{}

void HpaeSourceInputClusterTest::TearDown()
{}

TEST_F(HpaeSourceInputClusterTest, constructHpaeSourceInputClusterNode)
{
    std::shared_ptr<TestStatusCallback> g_testStatuscallback = std::make_shared<TestStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.statusCallback = g_testStatuscallback->GetWeakPtr();

    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceInputCluster->GetNodeId(), 0);
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
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS