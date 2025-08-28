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
#include "hpae_remote_output_cluster.h"
#include "hpae_mixer_node.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class HpaeRemoteOutputClusterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeRemoteOutputClusterTest::SetUp()
{}

void HpaeRemoteOutputClusterTest::TearDown()
{}

HWTEST_F(HpaeRemoteOutputClusterTest, constructNode_01, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.needEmptyChunk = false;
    HpaeNodeInfo nodeInfo1;
    nodeInfo1.nodeId = 1001; // 1001: node id
    nodeInfo1.frameLen = 960; // 960: frameLen
    nodeInfo1.sessionId = 123456; // 123456: session id
    nodeInfo1.samplingRate = SAMPLE_RATE_48000;
    nodeInfo1.channels = STEREO;
    nodeInfo1.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRemoteOutputCluster> hpaeRemoteOutputCluster =
        std::make_shared<HpaeRemoteOutputCluster>(nodeInfo1, sinkInfo);
    EXPECT_NE(hpaeRemoteOutputCluster, nullptr);

    HpaeNodeInfo nodeInfo2;
    nodeInfo2.nodeId = 1002; // 1002: nodeId
    nodeInfo2.frameLen = 960; // 960: frame len
    nodeInfo2.samplingRate = SAMPLE_RATE_48000;
    nodeInfo2.channels = STEREO;
    nodeInfo2.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo2);
    EXPECT_NE(hpaeMixerNode, nullptr);

    hpaeRemoteOutputCluster->Connect(hpaeMixerNode);
    hpaeRemoteOutputCluster->SetTimeoutStopThd(100); // 100: time
    hpaeRemoteOutputCluster->DoProcess();
    hpaeRemoteOutputCluster->GetConverterNodeCount();
    hpaeRemoteOutputCluster->DeInit();
    hpaeRemoteOutputCluster->Flush();
    hpaeRemoteOutputCluster->Pause();
    hpaeRemoteOutputCluster->ResetRender();
    hpaeRemoteOutputCluster->Resume();
    hpaeRemoteOutputCluster->Start();
    hpaeRemoteOutputCluster->Stop();
    hpaeRemoteOutputCluster->DisConnect(hpaeMixerNode);
}

HWTEST_F(HpaeRemoteOutputClusterTest, SetTimeoutStopThd_01, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo1;
    nodeInfo1.nodeId = 1001; // 1001: node id
    nodeInfo1.frameLen = 0;
    nodeInfo1.sessionId = 123456; // 123456: session id
    nodeInfo1.samplingRate = SAMPLE_RATE_48000;
    nodeInfo1.channels = STEREO;
    nodeInfo1.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRemoteOutputCluster> hpaeRemoteOutputCluster =
        std::make_shared<HpaeRemoteOutputCluster>(nodeInfo1, sinkInfo);
    EXPECT_NE(hpaeRemoteOutputCluster, nullptr);

    HpaeNodeInfo nodeInfo2;
    nodeInfo2.nodeId = 1002; // 1002: node id
    nodeInfo2.frameLen = 960; // 960: frameLen
    nodeInfo2.samplingRate = SAMPLE_RATE_48000;
    nodeInfo2.channels = STEREO;
    nodeInfo2.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo2);
    EXPECT_NE(hpaeMixerNode, nullptr);
    
    hpaeRemoteOutputCluster->Connect(hpaeMixerNode);
    hpaeRemoteOutputCluster->SetTimeoutStopThd(100); // 100: time
    hpaeRemoteOutputCluster->DoProcess();
    hpaeRemoteOutputCluster->GetConverterNodeCount();
    hpaeRemoteOutputCluster->GetFrameData();
    hpaeRemoteOutputCluster->GetPreOutNum();
    hpaeRemoteOutputCluster->DeInit();
    hpaeRemoteOutputCluster->Flush();
    hpaeRemoteOutputCluster->Pause();
    hpaeRemoteOutputCluster->ResetRender();
    hpaeRemoteOutputCluster->Resume();
    hpaeRemoteOutputCluster->Start();
    hpaeRemoteOutputCluster->Stop();
    hpaeRemoteOutputCluster->DisConnect(hpaeMixerNode);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS