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

#include "audio_a2dp_offload_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const std::string TEST_DEVICE_ADDR = "00:11:22:33:44:55";
static const int32_t TEST_SESSION_ID_BASE = 100000;
static const int32_t TEST_STREAM_1_SESSION_ID = 100001;
static const int32_t TEST_STREAM_2_SESSION_ID = 100002;
static const int32_t TEST_STREAM_3_SESSION_ID = 100003;
static std::vector<int32_t> TEST_SESSION_ID_VECTOR = {
    TEST_STREAM_1_SESSION_ID,
    TEST_STREAM_2_SESSION_ID,
    TEST_STREAM_3_SESSION_ID
};

void AudioA2dpOffloadManagerUnitTest::SetUpTestCase(void) {}
void AudioA2dpOffloadManagerUnitTest::TearDownTestCase(void) {}

void AudioA2dpOffloadManagerUnitTest::SetUp(void)
{
    testManager_ = std::make_shared<AudioA2dpOffloadManager>();
}

void AudioA2dpOffloadManagerUnitTest::TearDown(void)
{
    testManager_ = nullptr;
}

void AudioA2dpOffloadManagerUnitTest::MakeStreamCollectorData(uint32_t runningStreamCnt, uint32_t stopStreamCnt)
{
    if (testManager_ == nullptr) {
        return;
    }

    for (uint32_t i = 0; i < runningStreamCnt; ++i) {
        AudioStreamChangeInfo streamInfo;
        streamInfo.audioRendererChangeInfo.sessionId = i + TEST_SESSION_ID_BASE;
        streamInfo.audioRendererChangeInfo.rendererState = RENDERER_RUNNING;
        testManager_->streamCollector_.AddRendererStream(streamInfo);
    }

    for (uint32_t i = 0; i < stopStreamCnt; ++i) {
        AudioStreamChangeInfo streamInfo;
        streamInfo.audioRendererChangeInfo.sessionId = i + runningStreamCnt + TEST_SESSION_ID_BASE;
        streamInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;
        testManager_->streamCollector_.AddRendererStream(streamInfo);
    }
}

/**
 * @tc.name: ConnectA2dpOffload_001
 * @tc.desc: Test ConnectA2dpOffload with state already connected.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, ConnectA2dpOffload_001, TestSize.Level1)
{
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    testManager_->ConnectA2dpOffload(TEST_DEVICE_ADDR, TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(testManager_->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTED);
}

/**
 * @tc.name: ConnectA2dpOffload_002
 * @tc.desc: Test ConnectA2dpOffload with state already connecting.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, ConnectA2dpOffload_002, TestSize.Level1)
{
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    testManager_->ConnectA2dpOffload(TEST_DEVICE_ADDR, TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(testManager_->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTING);
}

/**
 * @tc.name: ConnectA2dpOffload_003
 * @tc.desc: Test ConnectA2dpOffload with state disconnected.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, ConnectA2dpOffload_003, TestSize.Level1)
{
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
    testManager_->ConnectA2dpOffload(TEST_DEVICE_ADDR, TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(testManager_->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTING);
}

/**
 * @tc.name: OffloadStartPlaying_001
 * @tc.desc: Test OffloadStartPlaying with entering the second if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_001, TestSize.Level1)
{
    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
    int32_t ret = testManager_->OffloadStartPlaying(TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
    EXPECT_EQ(testManager_->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_DISCONNECTED);
}

/**
 * @tc.name: OffloadStartPlaying_002
 * @tc.desc: Test OffloadStartPlaying without entering the second if branch due to ret != SUCCESS.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_002, TestSize.Level1)
{
    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    int32_t ret = testManager_->OffloadStartPlaying(TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
    EXPECT_EQ(testManager_->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_DISCONNECTED);
}

/**
 * @tc.name: OffloadStartPlaying_003
 * @tc.desc: Test OffloadStartPlaying without entering the second if branch due to
 *           state == CONNECTION_STATUS_CONNECTED.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_003, TestSize.Level1)
{
    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t ret = testManager_->OffloadStartPlaying(TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
    EXPECT_EQ(testManager_->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTED);
}

/**
 * @tc.name: OffloadStartPlaying_004
 * @tc.desc: Test OffloadStartPlaying
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_004, TestSize.Level1)
{
    testManager_->SetA2dpOffloadFlag(A2DP_NOT_OFFLOAD);
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t ret = testManager_->OffloadStartPlaying(TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: OffloadStartPlaying_005
 * @tc.desc: Test OffloadStartPlaying
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_005, TestSize.Level1)
{
    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    std::vector<int32_t> sessionIds = {};
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t ret = testManager_->OffloadStartPlaying(sessionIds);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: OffloadStopPlaying_001
 * @tc.desc: Test OffloadStopPlaying without entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStopPlaying_001, TestSize.Level1)
{
    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    int32_t ret = testManager_->OffloadStopPlaying(TEST_SESSION_ID_VECTOR);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
}

/**
 * @tc.name: GetA2dpOffloadCodecAndSendToDsp_001
 * @tc.desc: Test GetA2dpOffloadCodecAndSendToDsp without entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetA2dpOffloadCodecAndSendToDsp_001, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    testManager_->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    testManager_->GetA2dpOffloadCodecAndSendToDsp();
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: GetA2dpOffloadCodecAndSendToDsp_002
 * @tc.desc: Test GetA2dpOffloadCodecAndSendToDsp when entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetA2dpOffloadCodecAndSendToDsp_002, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    testManager_->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    testManager_->GetA2dpOffloadCodecAndSendToDsp();
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_001
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_001, TestSize.Level1)
{
    int32_t playingState = 1;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    testManager_->OnA2dpPlayingStateChanged(TEST_DEVICE_ADDR, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_002
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_002, TestSize.Level1)
{
    int32_t playingState = 2;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    testManager_->OnA2dpPlayingStateChanged(TEST_DEVICE_ADDR, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_003
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_003, TestSize.Level1)
{
    int32_t playingState = 1;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    testManager_->OnA2dpPlayingStateChanged(TEST_DEVICE_ADDR, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_004
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_004, TestSize.Level1)
{
    const std::string deviceAddress = testManager_->a2dpOffloadDeviceAddress_;
    int32_t playingState = 2;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    testManager_->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_005
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_005, TestSize.Level1)
{
    const std::string deviceAddress = testManager_->a2dpOffloadDeviceAddress_;
    int32_t playingState = 2;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    testManager_->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_006
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_006, TestSize.Level1)
{
    const std::string deviceAddress = testManager_->a2dpOffloadDeviceAddress_;
    int32_t playingState = 1;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    testManager_->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_007
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_007, TestSize.Level1)
{
    const std::string deviceAddress = testManager_->a2dpOffloadDeviceAddress_;
    int32_t playingState = 3;
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    testManager_->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(testManager_, nullptr);
}

/**
 * @tc.name: IsA2dpOffloadConnecting_001
 * @tc.desc: Test IsA2dpOffloadConnecting when entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, IsA2dpOffloadConnecting_001, TestSize.Level1)
{
    testManager_->connectionTriggerSessionIds_ = {TEST_STREAM_1_SESSION_ID};
    testManager_->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    bool result = testManager_->IsA2dpOffloadConnecting(TEST_STREAM_1_SESSION_ID);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_GetA2dpOffloadFlag_001
 * @tc.number : GetA2dpOffloadFlag_001
 * @tc.desc   : Test GetA2dpOffloadFlag()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetA2dpOffloadFlag_001, TestSize.Level3)
{
    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    EXPECT_EQ(A2DP_OFFLOAD, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForSpatializationChanged_001
 * @tc.number : UpdateA2dpOffloadFlagForSpatializationChanged_001
 * @tc.desc   : Test UpdateA2dpOffloadFlagForSpatializationChanged()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForSpatializationChanged_001, TestSize.Level4)
{
    std::unordered_map<uint32_t, bool> testSpatializationEnabledMap;
    testManager_->UpdateA2dpOffloadFlagForSpatializationChanged(testSpatializationEnabledMap, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(A2DP_OFFLOAD, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForStartStream_001
 * @tc.number : UpdateA2dpOffloadFlagForStartStream_001
 * @tc.desc   : Test UpdateA2dpOffloadFlagForStartStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForStartStream_001, TestSize.Level3)
{
    testManager_->UpdateA2dpOffloadFlagForStartStream(TEST_STREAM_1_SESSION_ID);
    EXPECT_EQ(A2DP_OFFLOAD, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForStartStream_002
 * @tc.number : UpdateA2dpOffloadFlagForStartStream_002
 * @tc.desc   : Test UpdateA2dpOffloadFlagForStartStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForStartStream_002, TestSize.Level3)
{
    uint32_t runningStreamCnt = 2;
    uint32_t stopStreamCnt = 0;
    MakeStreamCollectorData(runningStreamCnt, stopStreamCnt);

    testManager_->UpdateA2dpOffloadFlagForStartStream(TEST_SESSION_ID_BASE + runningStreamCnt);
    EXPECT_EQ(NO_A2DP_DEVICE, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForStartStream_003
 * @tc.number : UpdateA2dpOffloadFlagForStartStream_003
 * @tc.desc   : Test UpdateA2dpOffloadFlagForStartStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForStartStream_003, TestSize.Level3)
{
    uint32_t runningStreamCnt = 2;
    uint32_t stopStreamCnt = 2;
    MakeStreamCollectorData(runningStreamCnt, stopStreamCnt);

    testManager_->UpdateA2dpOffloadFlagForStartStream(TEST_SESSION_ID_BASE + runningStreamCnt);
    EXPECT_EQ(NO_A2DP_DEVICE, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForStartStream_004
 * @tc.number : UpdateA2dpOffloadFlagForStartStream_004
 * @tc.desc   : Test UpdateA2dpOffloadFlagForStartStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForStartStream_004, TestSize.Level3)
{
    uint32_t runningStreamCnt = 0;
    uint32_t stopStreamCnt = 2;
    MakeStreamCollectorData(runningStreamCnt, stopStreamCnt);

    testManager_->UpdateA2dpOffloadFlagForStartStream(TEST_SESSION_ID_BASE + runningStreamCnt);
    EXPECT_EQ(NO_A2DP_DEVICE, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForAllStream_001
 * @tc.number : UpdateA2dpOffloadFlagForAllStream_001
 * @tc.desc   : Test UpdateA2dpOffloadFlagForAllStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForAllStream_001, TestSize.Level4)
{
    testManager_->UpdateA2dpOffloadFlagForAllStream(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(NO_A2DP_DEVICE, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForAllStream_002
 * @tc.number : UpdateA2dpOffloadFlagForAllStream_002
 * @tc.desc   : Test UpdateA2dpOffloadFlagForAllStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForAllStream_002, TestSize.Level4)
{
    uint32_t runningStreamCnt = 2;
    uint32_t stopStreamCnt = 2;
    MakeStreamCollectorData(runningStreamCnt, stopStreamCnt);

    testManager_->UpdateA2dpOffloadFlagForAllStream(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_NE(A2DP_OFFLOAD, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForAllStream_003
 * @tc.number : UpdateA2dpOffloadFlagForAllStream_003
 * @tc.desc   : Test UpdateA2dpOffloadFlagForAllStream()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForAllStream_003, TestSize.Level4)
{
    uint32_t runningStreamCnt = 2;
    uint32_t stopStreamCnt = 2;
    MakeStreamCollectorData(runningStreamCnt, stopStreamCnt);

    testManager_->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    testManager_->UpdateA2dpOffloadFlagForAllStream(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_NE(A2DP_OFFLOAD, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_GetSpatialAudio_001
 * @tc.number : GetSpatialAudio_001
 * @tc.desc   : Test GetSpatialAudio() for different cases
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetSpatialAudio_001, TestSize.Level4)
{
    std::unordered_map<uint32_t, bool> testSpatializationEnabledMap;
    bool isSpatial = testManager_->GetSpatialAudio(true, TEST_STREAM_1_SESSION_ID,
        STREAM_USAGE_MUSIC, testSpatializationEnabledMap);
    EXPECT_EQ(false, isSpatial);

    isSpatial = testManager_->GetSpatialAudio(false, TEST_STREAM_1_SESSION_ID,
        STREAM_USAGE_MUSIC, testSpatializationEnabledMap);
    EXPECT_EQ(false, isSpatial);

    testSpatializationEnabledMap[TEST_STREAM_1_SESSION_ID] = true;
    isSpatial = testManager_->GetSpatialAudio(false, TEST_STREAM_1_SESSION_ID,
        STREAM_USAGE_MUSIC, testSpatializationEnabledMap);
    EXPECT_EQ(true, isSpatial);
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_UpdateA2dpOffloadFlagForA2dpDeviceOut_001
 * @tc.number : UpdateA2dpOffloadFlagForA2dpDeviceOut_001
 * @tc.desc   : Test UpdateA2dpOffloadFlagForA2dpDeviceOut()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForA2dpDeviceOut_001, TestSize.Level4)
{
    uint32_t runningStreamCnt = 1;
    uint32_t stopStreamCnt = 1;
    MakeStreamCollectorData(runningStreamCnt, stopStreamCnt);

    testManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
    EXPECT_EQ(NO_A2DP_DEVICE, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_HandleA2dpDeviceOutOffload_001
 * @tc.number : HandleA2dpDeviceOutOffload_001
 * @tc.desc   : Test HandleA2dpDeviceOutOffload()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleA2dpDeviceOutOffload_001, TestSize.Level4)
{
    std::vector<int32_t> testRunningSessions = {
        TEST_STREAM_1_SESSION_ID,
        TEST_STREAM_2_SESSION_ID
    };
    testManager_->HandleA2dpDeviceOutOffload(NO_A2DP_DEVICE, testRunningSessions);
    EXPECT_EQ(NO_A2DP_DEVICE, testManager_->GetA2dpOffloadFlag());
}

/**
 * @tc.name   : AudioA2dpOffloadManagerUnitTest_HandleA2dpDeviceInOffload_001
 * @tc.number : HandleA2dpDeviceInOffload_001
 * @tc.desc   : Test HandleA2dpDeviceInOffload()
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleA2dpDeviceInOffload_001, TestSize.Level4)
{
    std::vector<int32_t> testRunningSessions = {
        TEST_STREAM_1_SESSION_ID,
        TEST_STREAM_2_SESSION_ID
    };
    testManager_->HandleA2dpDeviceInOffload(A2DP_OFFLOAD, testRunningSessions);
    EXPECT_EQ(A2DP_OFFLOAD, testManager_->GetA2dpOffloadFlag());
}

} // namespace AudioStandard
} // namespace OHOS
