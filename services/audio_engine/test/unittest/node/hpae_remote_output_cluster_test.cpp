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
#include "hpae_audio_format_converter_node.h"

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

/**
 * @tc.name  : TransStreamUsageToSplitSceneType_01
 * @tc.type  : FUNC
 * @tc.number: TransStreamUsageToSplitSceneType_01
 * @tc.desc  : Test the conversion from StreamUsage and SplitMode to HpaeProcessorType.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, TransStreamUsageToSplitSceneType_01, TestSize.Level0)
{
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, ""), HPAE_SCENE_DEFAULT);

    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, "1"), HPAE_SCENE_SPLIT_MEDIA);

    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NAVIGATION, "1:2"), HPAE_SCENE_SPLIT_NAVIGATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, "1:2"), HPAE_SCENE_SPLIT_MEDIA);

    std::string mode3 = "part1:part2:part3";
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NAVIGATION, mode3), HPAE_SCENE_SPLIT_NAVIGATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_COMMUNICATION, mode3),
        HPAE_SCENE_SPLIT_COMMUNICATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VIDEO_COMMUNICATION, mode3),
        HPAE_SCENE_SPLIT_COMMUNICATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, "1:2:3:4"), HPAE_SCENE_SPLIT_MEDIA);
}

/**
 * @tc.name  : UpdateStreamInfo_003
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamInfo_003
 * @tc.desc  : Test UpdateStreamInfo with the correct constructor (two NodeInfo params).
 */
HWTEST_F(HpaeRemoteOutputClusterTest, UpdateStreamInfo_003, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo clusterBaseInfo;
    clusterBaseInfo.nodeName = "RemoteOutputCluster";
    clusterBaseInfo.samplingRate = SAMPLE_RATE_48000;
    auto cluster = std::make_shared<HpaeRemoteOutputCluster>(clusterBaseInfo, sinkInfo);

    HpaeNodeInfo preInfo;
    preInfo.nodeName = "SourceNode";
    preInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    preInfo.streamType = AudioStreamType::STREAM_MUSIC;
    preInfo.effectInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    auto preNode = std::make_shared<HpaeMixerNode>(preInfo);

    HpaeNodeInfo curInfo = clusterBaseInfo;
    curInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    cluster->sceneConverterMap_[HPAE_SCENE_SPLIT_MEDIA] =
        std::make_shared<HpaeAudioFormatConverterNode>(preInfo, curInfo);

    cluster->sceneMixerMap_[HPAE_SCENE_SPLIT_MEDIA] = std::make_shared<HpaeMixerNode>(preInfo);
    cluster->UpdateStreamInfo(preNode);
    auto targetConverter = cluster->sceneConverterMap_[HPAE_SCENE_SPLIT_MEDIA];

    ASSERT_NE(targetConverter, nullptr);
    EXPECT_EQ(targetConverter->GetNodeInfo().streamType, AudioStreamType::STREAM_MUSIC);
}

/**
 * @tc.name  : UpdateStreamInfo_001
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamInfo_001
 * @tc.desc  : Test UpdateStreamInfo with preNode == nullptr - should return gracefully.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, UpdateStreamInfo_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo clusterBaseInfo;
    clusterBaseInfo.nodeName = "RemoteOutputCluster";
    clusterBaseInfo.samplingRate = SAMPLE_RATE_48000;
    auto cluster = std::make_shared<HpaeRemoteOutputCluster>(clusterBaseInfo, sinkInfo);
    EXPECT_NE(cluster, nullptr);

    std::shared_ptr<OutputNode<HpaePcmBuffer *>> preNode = nullptr;
    cluster->UpdateStreamInfo(preNode);
    SUCCEED();  // If no crash, test passes
}

/**
 * @tc.name  : UpdateStreamInfo_002
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamInfo_002
 * @tc.desc  : Test UpdateStreamInfo with mixerNode == nullptr - should handle gracefully.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, UpdateStreamInfo_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo clusterBaseInfo;
    clusterBaseInfo.nodeName = "RemoteOutputCluster";
    clusterBaseInfo.samplingRate = SAMPLE_RATE_48000;
    auto cluster = std::make_shared<HpaeRemoteOutputCluster>(clusterBaseInfo, sinkInfo);
    EXPECT_NE(cluster, nullptr);

    HpaeNodeInfo preInfo;
    preInfo.nodeName = "SourceNode";
    preInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    preInfo.streamType = AudioStreamType::STREAM_MUSIC;
    preInfo.effectInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    auto preNode = std::make_shared<HpaeMixerNode>(preInfo);
    EXPECT_NE(preNode, nullptr);

    cluster->UpdateStreamInfo(preNode);
    SUCCEED();  // If no crash, test passes
}

/**
 * @tc.name  : UpdateStreamInfo_004
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamInfo_004
 * @tc.desc  : Test UpdateStreamInfo with convertNode == nullptr - should handle gracefully.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, UpdateStreamInfo_004, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo clusterBaseInfo;
    clusterBaseInfo.nodeName = "RemoteOutputCluster";
    clusterBaseInfo.samplingRate = SAMPLE_RATE_48000;
    auto cluster = std::make_shared<HpaeRemoteOutputCluster>(clusterBaseInfo, sinkInfo);
    EXPECT_NE(cluster, nullptr);

    HpaeNodeInfo preInfo;
    preInfo.nodeName = "SourceNode";
    preInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    preInfo.streamType = AudioStreamType::STREAM_MUSIC;
    preInfo.effectInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    auto preNode = std::make_shared<HpaeMixerNode>(preInfo);
    EXPECT_NE(preNode, nullptr);

    cluster->sceneMixerMap_[HPAE_SCENE_SPLIT_MEDIA] = std::make_shared<HpaeMixerNode>(preInfo);
    cluster->UpdateStreamInfo(preNode);
    SUCCEED();  // If no crash, test passes
}

/**
 * @tc.name  : TransStreamUsageToSplitSceneType_03
 * @tc.type  : FUNC
 * @tc.number: TransStreamUsageToSplitSceneType_03
 * @tc.desc  : Test TransStreamUsageToSplitSceneType with trailing empty (1::) - splitNums=2.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, TransStreamUsageToSplitSceneType_03, TestSize.Level1)
{
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NAVIGATION, "1::"),
        HPAE_SCENE_SPLIT_NAVIGATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, "1::"),
        HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_COMMUNICATION, "1::"),
        HPAE_SCENE_SPLIT_MEDIA);
}

/**
 * @tc.name  : TransStreamUsageToSplitSceneType_05
 * @tc.type  : FUNC
 * @tc.number: TransStreamUsageToSplitSceneType_05
 * @tc.desc  : Test TransStreamUsageToSplitSceneType with all StreamUsage enum values.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, TransStreamUsageToSplitSceneType_05, TestSize.Level1)
{
    std::string mode2 = "1:2";
    std::string mode3 = "1:2:3";

    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_UNKNOWN, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_COMMUNICATION, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_ASSISTANT, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NAVIGATION, mode2), HPAE_SCENE_SPLIT_NAVIGATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_RINGTONE, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_MESSAGE, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_ALARM, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NOTIFICATION, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_DTMF, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VIDEO_COMMUNICATION, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_AUDIOBOOK, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_GAME, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_SYSTEM, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_ENFORCED_TONE, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_ULTRASONIC, mode2), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MOVIE, mode2), HPAE_SCENE_SPLIT_MEDIA);

    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_UNKNOWN, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MEDIA, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_COMMUNICATION, mode3),
        HPAE_SCENE_SPLIT_COMMUNICATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_ASSISTANT, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NAVIGATION, mode3), HPAE_SCENE_SPLIT_NAVIGATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_RINGTONE, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VOICE_MESSAGE, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_ALARM, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_NOTIFICATION, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_DTMF, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_VIDEO_COMMUNICATION, mode3),
        HPAE_SCENE_SPLIT_COMMUNICATION);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_AUDIOBOOK, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_GAME, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_SYSTEM, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_ENFORCED_TONE, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_ULTRASONIC, mode3), HPAE_SCENE_SPLIT_MEDIA);
    EXPECT_EQ(TransStreamUsageToSplitSceneType(STREAM_USAGE_MOVIE, mode3), HPAE_SCENE_SPLIT_MEDIA);
}
// ==================== New UT for GetRenderId and GetCurrentOutputDevice ====================

/**
 * @tc.name  : GetRenderId_Default_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderId returns HDI_INVALID_ID by default.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, GetRenderId_Default_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = 960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote";
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRemoteOutputCluster->GetRenderId(), HDI_INVALID_ID);
}

/**
 * @tc.name  : GetCurrentOutputDevice_NullSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetCurrentOutputDevice returns DEVICE_TYPE_NONE when sink is null.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, GetCurrentOutputDevice_NullSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = 960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote";
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRemoteOutputCluster->GetCurrentOutputDevice(), DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : GetRenderId_SetValue_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderId returns correct value after setting via sink output node.
 */
HWTEST_F(HpaeRemoteOutputClusterTest, GetRenderId_SetValue_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = 960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote";
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->hpaeSinkOutputNode_->renderId_ = 300;
    EXPECT_EQ(hpaeRemoteOutputCluster->GetRenderId(), 300u);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS