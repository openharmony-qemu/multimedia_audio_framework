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

#include "audio_policy_utils.h"
#include "audio_a2dp_offload_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioA2dpOffloadManagerUnitTest::SetUpTestCase(void) {}
void AudioA2dpOffloadManagerUnitTest::TearDownTestCase(void) {}
void AudioA2dpOffloadManagerUnitTest::SetUp(void) {}
void AudioA2dpOffloadManagerUnitTest::TearDown(void) {}

/**
 * @tc.name: ConnectA2dpOffload_001
 * @tc.desc: Test ConnectA2dpOffload with state already connected.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, ConnectA2dpOffload_001, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    std::vector<int32_t> sessionIds = {1, 2, 3};
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    manager->ConnectA2dpOffload(deviceAddress, sessionIds);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTED);
}

/**
 * @tc.name: ConnectA2dpOffload_002
 * @tc.desc: Test ConnectA2dpOffload with state already connecting.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, ConnectA2dpOffload_002, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    std::vector<int32_t> sessionIds = {1, 2, 3};
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    manager->ConnectA2dpOffload(deviceAddress, sessionIds);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTING);
}

/**
 * @tc.name: ConnectA2dpOffload_003
 * @tc.desc: Test ConnectA2dpOffload with state disconnected.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, ConnectA2dpOffload_003, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    std::vector<int32_t> sessionIds = {1, 2, 3};
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
    manager->ConnectA2dpOffload(deviceAddress, sessionIds);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTING);
}

/**
 * @tc.name: WaitForConnectionCompleted_001
 * @tc.desc: Test WaitForConnectionCompleted with connection timeout.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, WaitForConnectionCompleted_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const int32_t shortTimeout = 100;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
    std::thread testThread([manager]() {
        manager->WaitForConnectionCompleted();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(shortTimeout + 100));
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_DISCONNECTED);
    testThread.join();
}


/**
 * @tc.name: OffloadStartPlaying_001
 * @tc.desc: Test OffloadStartPlaying with entering the second if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
    int32_t ret = manager->OffloadStartPlaying(sessionIds);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_DISCONNECTED);
}

/**
 * @tc.name: OffloadStartPlaying_002
 * @tc.desc: Test OffloadStartPlaying without entering the second if branch due to ret != SUCCESS.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    int32_t ret = manager->OffloadStartPlaying(sessionIds);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_DISCONNECTED);
}

/**
 * @tc.name: OffloadStartPlaying_003
 * @tc.desc: Test OffloadStartPlaying without entering the second if branch due to state == CONNECTION_STATUS_CONNECTED.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t ret = manager->OffloadStartPlaying(sessionIds);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_CONNECTED);
}

/**
 * @tc.name: OffloadStartPlaying_004
 * @tc.desc: Test OffloadStartPlaying
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OffloadStartPlaying_004, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_NOT_OFFLOAD);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t ret = manager->OffloadStartPlaying(sessionIds);
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
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    std::vector<int32_t> sessionIds = {};
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t ret = manager->OffloadStartPlaying(sessionIds);
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
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    int32_t ret = manager->OffloadStopPlaying(sessionIds);
    EXPECT_EQ(ret, BASE_AUDIO_ERR_OFFSET);
}

/**
 * @tc.name: UpdateA2dpOffloadFlagForAllStream_001
 * @tc.desc: Test UpdateA2dpOffloadFlagForAllStream with entering all if branches.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForAllStream_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnableMap = {
        {1, true},
        {2, false}
    };
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto changeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    changeInfo1->sessionId = 1;
    changeInfo1->rendererState = RENDERER_RUNNING;
    audioRendererChangeInfos.push_back(changeInfo1);

    auto changeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    changeInfo2->sessionId = 2;
    changeInfo2->rendererState = RENDERER_STOPPED;
    audioRendererChangeInfos.push_back(changeInfo2);
    manager->streamCollector_.audioRendererChangeInfos_ = audioRendererChangeInfos;
    manager->UpdateA2dpOffloadFlagForAllStream(sessionIDToSpatializationEnableMap, deviceType);
    EXPECT_EQ(manager->streamCollector_.audioRendererChangeInfos_, audioRendererChangeInfos);
}


/**
 * @tc.name: UpdateA2dpOffloadFlagForAllStream_002
 * @tc.desc: Test UpdateA2dpOffloadFlagForAllStream without entering any if branches.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlagForAllStream_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnableMap;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto changeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    changeInfo1->sessionId = 1;
    changeInfo1->rendererState = RENDERER_RUNNING;
    audioRendererChangeInfos.push_back(changeInfo1);

    auto changeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    changeInfo2->sessionId = 2;
    changeInfo2->rendererState = RENDERER_RUNNING;
    audioRendererChangeInfos.push_back(changeInfo2);
    manager->streamCollector_.audioRendererChangeInfos_ = audioRendererChangeInfos;
    manager->UpdateA2dpOffloadFlagForAllStream(sessionIDToSpatializationEnableMap, deviceType);
    EXPECT_EQ(manager->streamCollector_.audioRendererChangeInfos_, audioRendererChangeInfos);
}

/**
 * @tc.name: UpdateA2dpOffloadFlag_001
 * @tc.desc: Test UpdateA2dpOffloadFlag with deviceType DEVICE_TYPE_BLUETOOTH_A2DP
 *  and receiveOffloadFlag != GetA2dpOffloadFlag.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlag_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<Bluetooth::A2dpStreamInfo> allActiveSessions;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    a2dpStreamInfo.sessionId = 1;
    a2dpStreamInfo.streamType = 1;
    a2dpStreamInfo.isSpatialAudio = false;
    allActiveSessions.push_back(a2dpStreamInfo);
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    manager->UpdateA2dpOffloadFlag(allActiveSessions, deviceType);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: UpdateA2dpOffloadFlag_002
 * @tc.desc: Test UpdateA2dpOffloadFlag with deviceType DEVICE_TYPE_NONE and current output device is BLUETOOTH_A2DP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlag_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    std::vector<Bluetooth::A2dpStreamInfo> allActiveSessions;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    a2dpStreamInfo.sessionId = 1;
    a2dpStreamInfo.streamType = 1;
    a2dpStreamInfo.isSpatialAudio = false;
    allActiveSessions.push_back(a2dpStreamInfo);
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    manager->UpdateA2dpOffloadFlag(allActiveSessions, deviceType);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: UpdateA2dpOffloadFlag_003
 * @tc.desc: Test UpdateA2dpOffloadFlag with receiveOffloadFlag == NO_A2DP_DEVICE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlag_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<Bluetooth::A2dpStreamInfo> allActiveSessions;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    a2dpStreamInfo.sessionId = 1;
    a2dpStreamInfo.streamType = 1;
    a2dpStreamInfo.isSpatialAudio = false;
    allActiveSessions.push_back(a2dpStreamInfo);
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    manager->UpdateA2dpOffloadFlag(allActiveSessions, deviceType);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: UpdateA2dpOffloadFlag_004
 * @tc.desc: Test UpdateA2dpOffloadFlag with receiveOffloadFlag == GetA2dpOffloadFlag and both are A2DP_OFFLOAD.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, UpdateA2dpOffloadFlag_004, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<Bluetooth::A2dpStreamInfo> allActiveSessions;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    a2dpStreamInfo.sessionId = 1;
    a2dpStreamInfo.streamType = 1;
    a2dpStreamInfo.isSpatialAudio = false;
    allActiveSessions.push_back(a2dpStreamInfo);
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    manager->UpdateA2dpOffloadFlag(allActiveSessions, deviceType);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: HandleA2dpDeviceOutOffload_001
 * @tc.desc: Test HandleA2dpDeviceOutOffload with current output device type being BLUETOOTH_A2DP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleA2dpDeviceOutOffload_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor descriptor;
    descriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    manager->audioActiveDevice_.SetCurrentOutputDevice(descriptor);
    BluetoothOffloadState a2dpOffloadFlag = NO_A2DP_DEVICE;
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t result = manager->HandleA2dpDeviceOutOffload(a2dpOffloadFlag);
    EXPECT_EQ(result, manager->HandleActiveDevice(deviceDescriptor));
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetA2dpOffloadFlag(), a2dpOffloadFlag);
}

/**
 * @tc.name: HandleA2dpDeviceOutOffload_002
 * @tc.desc: Test HandleA2dpDeviceOutOffload with current output device type not being BLUETOOTH_A2DP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleA2dpDeviceOutOffload_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    BluetoothOffloadState a2dpOffloadFlag = NO_A2DP_DEVICE;
    int32_t result = manager->HandleA2dpDeviceOutOffload(a2dpOffloadFlag);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager->audioA2dpOffloadFlag_.GetA2dpOffloadFlag(), a2dpOffloadFlag);
}

/**
 * @tc.name: HandleA2dpDeviceInOffload_001
 * @tc.desc: Test HandleA2dpDeviceInOffload with A2DP offload connected.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleA2dpDeviceInOffload_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_OFFLOAD);
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    int32_t result = manager->HandleA2dpDeviceInOffload(A2DP_OFFLOAD);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: HandleA2dpDeviceInOffload_002
 * @tc.desc: Test HandleA2dpDeviceInOffload when the if condition is not met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleA2dpDeviceInOffload_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->SetA2dpOffloadFlag(A2DP_NOT_OFFLOAD);
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    int32_t result = manager->HandleA2dpDeviceInOffload(A2DP_NOT_OFFLOAD);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * @tc.name: GetA2dpOffloadCodecAndSendToDsp_001
 * @tc.desc: Test GetA2dpOffloadCodecAndSendToDsp without entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetA2dpOffloadCodecAndSendToDsp_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    manager->GetA2dpOffloadCodecAndSendToDsp();
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: GetA2dpOffloadCodecAndSendToDsp_002
 * @tc.desc: Test GetA2dpOffloadCodecAndSendToDsp when entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetA2dpOffloadCodecAndSendToDsp_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    manager->GetA2dpOffloadCodecAndSendToDsp();
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: FetchStreamForA2dpOffload_001
 * @tc.desc: Test FetchStreamForA2dpOffload when IsRendererStreamRunning returns false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, FetchStreamForA2dpOffload_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    changeInfo->clientUID = 123;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    manager->FetchStreamForA2dpOffload(false);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: FetchStreamForA2dpOffload_002
 * @tc.desc: Test FetchStreamForA2dpOffload when deviceType is not BLUETOOTH_A2DP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, FetchStreamForA2dpOffload_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    changeInfo->clientUID = 123;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    manager->FetchStreamForA2dpOffload(false);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: FetchStreamForA2dpOffload_003
 * @tc.desc: Test FetchStreamForA2dpOffload when requireReset is false and rendererFlags is not AUDIO_FLAG_MMAP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, FetchStreamForA2dpOffload_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    changeInfo->clientUID = 123;
    changeInfo->rendererInfo.rendererFlags = STREAM_USAGE_UNKNOWN;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    manager->FetchStreamForA2dpOffload(false);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: FetchStreamForA2dpOffload_004
 * @tc.desc: Test FetchStreamForA2dpOffload when requireReset is true and rendererFlags is not AUDIO_FLAG_MMAP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, FetchStreamForA2dpOffload_004, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();

    // 创建 rendererChangeInfos
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    changeInfo->clientUID = 123;
    changeInfo->rendererInfo.rendererFlags = STREAM_USAGE_UNKNOWN;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    manager->FetchStreamForA2dpOffload(true);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: FetchStreamForA2dpOffload_005
 * @tc.desc: Test FetchStreamForA2dpOffload when requireReset is true and rendererFlags is AUDIO_FLAG_MMAP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, FetchStreamForA2dpOffload_005, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    changeInfo->clientUID = 123;
    changeInfo->rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    manager->FetchStreamForA2dpOffload(true);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: GetAllRunningStreamSession_001
 * @tc.desc: Test GetAllRunningStreamSession when rendererState is not RUNNING and doStop is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetAllRunningStreamSession_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererState = RENDERER_PAUSED;
    changeInfo->sessionId = 123;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    std::vector<int32_t> allSessions;
    manager->GetAllRunningStreamSession(allSessions, true);
    EXPECT_TRUE(allSessions.empty());
}

/**
 * @tc.name: GetAllRunningStreamSession_002
 * @tc.desc: Test GetAllRunningStreamSession when rendererState is not RUNNING and doStop is false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetAllRunningStreamSession_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererState = RENDERER_PAUSED; // 不是 RUNNING
    changeInfo->sessionId = 123;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    std::vector<int32_t> allSessions;
    manager->GetAllRunningStreamSession(allSessions, false);
    EXPECT_TRUE(allSessions.empty());
}

/**
 * @tc.name: GetAllRunningStreamSession_003
 * @tc.desc: Test GetAllRunningStreamSession when rendererState is RUNNING and doStop is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetAllRunningStreamSession_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererState = RENDERER_RUNNING;
    changeInfo->sessionId = 123;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    std::vector<int32_t> allSessions;
    manager->GetAllRunningStreamSession(allSessions, true);
    EXPECT_EQ(allSessions.size(), 1);
    EXPECT_EQ(allSessions[0], 123);
}

/**
 * @tc.name: GetAllRunningStreamSession_004
 * @tc.desc: Test GetAllRunningStreamSession when rendererState is RUNNING and doStop is false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetAllRunningStreamSession_004, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererState = RENDERER_RUNNING;
    changeInfo->sessionId = 123;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    std::vector<int32_t> allSessions;
    manager->GetAllRunningStreamSession(allSessions, false);
    EXPECT_EQ(allSessions.size(), 1);
    EXPECT_EQ(allSessions[0], 123);
}

/**
 * @tc.name: GetVolumeGroupType_001
 * @tc.desc: Test GetVolumeGroupType when deviceType is DEVICE_TYPE_HDMI.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetVolumeGroupType_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::string volumeGroupType = manager->GetVolumeGroupType(DEVICE_TYPE_HDMI);
    EXPECT_EQ(volumeGroupType, "build-in");
}

/**
 * @tc.name: GetVolumeGroupType_002
 * @tc.desc: Test GetVolumeGroupType when deviceType is DEVICE_TYPE_BLUETOOTH_SCO.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetVolumeGroupType_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::string volumeGroupType = manager->GetVolumeGroupType(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(volumeGroupType, "wireless");
}

/**
 * @tc.name: GetVolumeGroupType_003
 * @tc.desc: Test GetVolumeGroupType when deviceType is DEVICE_TYPE_USB_ARM_HEADSET.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetVolumeGroupType_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::string volumeGroupType = manager->GetVolumeGroupType(DEVICE_TYPE_USB_ARM_HEADSET);
    EXPECT_EQ(volumeGroupType, "wired");
}

/**
 * @tc.name: GetVolumeGroupType_004
 * @tc.desc: Test GetVolumeGroupType when deviceType is not supported.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, GetVolumeGroupType_004, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::string volumeGroupType = manager->GetVolumeGroupType(DEVICE_TYPE_MAX);
    EXPECT_EQ(volumeGroupType, "");
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_001
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = "123";
    int32_t playingState = 1;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_002
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = "123";
    int32_t playingState = 2;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_003
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = "123";
    int32_t playingState = 1;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_004
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_004, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = manager->a2dpOffloadDeviceAddress_;
    int32_t playingState = 2;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_005
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_005, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = manager->a2dpOffloadDeviceAddress_;
    int32_t playingState = 2;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_006
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_006, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = manager->a2dpOffloadDeviceAddress_;
    int32_t playingState = 1;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: OnA2dpPlayingStateChanged_007
 * @tc.desc: Test OnA2dpPlayingStateChanged.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, OnA2dpPlayingStateChanged_007, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = manager->a2dpOffloadDeviceAddress_;
    int32_t playingState = 3;
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name: IsA2dpOffloadConnecting_001
 * @tc.desc: Test IsA2dpOffloadConnecting when entering the if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, IsA2dpOffloadConnecting_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->connectionTriggerSessionIds_ = {123};
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    bool result = manager->IsA2dpOffloadConnecting(123);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: HandleActiveDevice_001
 * @tc.desc: Test HandleActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleActiveDevice_001, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_NONE;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    manager->audioConfigManager_.isUpdateRouteSupported_ = true;
    int32_t result = manager->HandleActiveDevice(deviceDescriptor);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name: HandleActiveDevice_002
 * @tc.desc: Test HandleActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleActiveDevice_002, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceDescriptor preDeviceDescriptor;
    preDeviceDescriptor.deviceType_ = DEVICE_TYPE_EARPIECE;
    manager->audioActiveDevice_.SetCurrentOutputDevice(preDeviceDescriptor);
    manager->audioConfigManager_.isUpdateRouteSupported_ = false;
    int32_t result = manager->HandleActiveDevice(deviceDescriptor);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: HandleActiveDevice_003
 * @tc.desc: Test HandleActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpOffloadManagerUnitTest, HandleActiveDevice_003, TestSize.Level1)
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor preDeviceDescriptor;
    preDeviceDescriptor.deviceType_ = DEVICE_TYPE_EARPIECE;
    manager->audioActiveDevice_.SetCurrentOutputDevice(preDeviceDescriptor);
    manager->audioConfigManager_.isUpdateRouteSupported_ = false;
    int32_t result = manager->HandleActiveDevice(deviceDescriptor);
    EXPECT_EQ(result, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
