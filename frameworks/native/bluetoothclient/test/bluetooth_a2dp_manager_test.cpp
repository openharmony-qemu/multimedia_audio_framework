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
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "bluetooth_a2dp_mock_interface.h"
#include "bluetooth_errorcode.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

using namespace testing::ext;
using namespace testing;

#define A2DP_DEVICE_MAC1 "28:FA:19:1E:41:0E"
#define A2DP_DEVICE_MAC2 "24:E9:CA:60:2F:CB"
#define MOCK_RETURN_VALUE 1
#define MOCK_RETURN_VALUE0 0
#define MOCK_INPUT_VALUE 1

class DeviceStatusObserverMock : public IDeviceStatusObserver {
public:
    void OnDeviceStatusUpdated(AudioStandard::DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false) override {};
    void OnMicrophoneBlockedUpdate(AudioStandard::DeviceType devType, DeviceBlockStatus status) override {};
    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {};
    void OnDeviceConfigurationChanged(AudioStandard::DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo) override {};
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false) override {};
    void OnServiceConnected(AudioServiceIndex serviceIndex) override {};
    void OnServiceDisconnected(AudioServiceIndex serviceIndex) override {};
    void OnForcedDeviceSelected(AudioStandard::DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr) override {};
    void OnPrivacyDeviceSelected(AudioStandard::DeviceType devType, const std::string &macAddress) override {};
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {};
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand) override {};
    void OnConnectFailed(AudioDeviceDescriptor &desc) override {};
};

class BluetoothA2dpManagerTest : public testing::Test {
public:
    void SetUp(void) override
    {
        BluetoothA2dpMockInterface::mockInterface_ = std::make_shared<BluetoothA2dpMockInterface>();
        AudioA2dpManager::a2dpListener_ = std::make_shared<AudioA2dpListener>();
        MediaBluetoothDeviceManager::a2dpBluetoothDeviceMap_[A2DP_DEVICE_MAC1] =
            BluetoothRemoteDevice(A2DP_DEVICE_MAC1);
        MediaBluetoothDeviceManager::a2dpBluetoothDeviceMap_[A2DP_DEVICE_MAC2] =
            BluetoothRemoteDevice(A2DP_DEVICE_MAC2);
    }

    void TearDown(void) override
    {
        BluetoothA2dpMockInterface::mockInterface_ = nullptr;

        AudioA2dpManager::a2dpListener_ = nullptr;
        AudioA2dpManager::connectionState_ = static_cast<int>(BTConnectState::DISCONNECTED);
        AudioA2dpManager::captureConnectionState_ = static_cast<int32_t>(BTHdapConnectState::DISCONNECTED);
        AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice();
    }
};

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_001
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_001, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), SetActiveSinkDevice(_))
        .Times(2)
        .WillRepeatedly(Return(SUCCESS));

    EXPECT_NE(AudioA2dpManager::SetActiveA2dpDevice("33:33:33"), SUCCESS);
    EXPECT_EQ(AudioA2dpManager::SetActiveA2dpDevice(A2DP_DEVICE_MAC1), SUCCESS);
    EXPECT_EQ(AudioA2dpManager::SetActiveA2dpDevice(A2DP_DEVICE_MAC2), SUCCESS);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_002
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_002, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetActiveA2dpDevice())
        .WillOnce(Return(A2DP_DEVICE_MAC2));

    EXPECT_EQ(AudioA2dpManager::GetActiveA2dpDevice(), A2DP_DEVICE_MAC2);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_003
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_003, TestSize.Level1)
{
    A2dpCodecStatus codecStatus;
    codecStatus.codecInfo.sampleRate = A2DP_L2HCV2_SAMPLE_RATE_48000_USER;
    codecStatus.codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_16_USER;
    codecStatus.codecInfo.channelMode = A2DP_SBC_CHANNEL_MODE_STEREO_USER;
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetCodecStatus(_))
        .WillOnce(Return(codecStatus));

    AudioStreamInfo streamInfo;
    EXPECT_EQ(AudioA2dpManager::GetA2dpDeviceStreamInfo(A2DP_DEVICE_MAC2, streamInfo), SUCCESS);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_005
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_005, TestSize.Level1)
{
    A2dpCodecStatus codecStatus;
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetCodecStatus(_))
        .WillOnce(Return(codecStatus));

    AudioStreamInfo streamInfo;
    EXPECT_EQ(AudioA2dpManager::GetA2dpDeviceStreamInfo(A2DP_DEVICE_MAC2, streamInfo), AudioStandard::ERROR);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_006
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_006, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetDevicesByStates(_, _))
        .WillOnce(Invoke([](const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) {
            devices.push_back(BluetoothRemoteDevice());
            return MOCK_RETURN_VALUE;
        }));

    std::vector<BluetoothRemoteDevice> devices;
    EXPECT_EQ(AudioA2dpManager::HasA2dpDeviceConnected(), true);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_007
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_007, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), A2dpOffloadSessionRequest(_, _))
        .WillOnce(Return(MOCK_RETURN_VALUE));

    std::vector<A2dpStreamInfo> info;
    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice("00:00:00:00:00:00");
    EXPECT_EQ(AudioA2dpManager::A2dpOffloadSessionRequest(info), A2DP_NOT_OFFLOAD);

    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice(A2DP_DEVICE_MAC2);
    EXPECT_NE(AudioA2dpManager::A2dpOffloadSessionRequest(info), AudioStandard::ERROR);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_008
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_008, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), OffloadStartPlaying(_, _))
        .WillOnce(Return(MOCK_RETURN_VALUE));

    std::vector<int32_t> sessionsID;
    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice("00:00:00:00:00:00");
    EXPECT_EQ(AudioA2dpManager::OffloadStartPlaying(sessionsID), AudioStandard::ERROR);

    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice(A2DP_DEVICE_MAC2);
    EXPECT_NE(AudioA2dpManager::OffloadStartPlaying(sessionsID), AudioStandard::ERROR);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_009
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_009, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), OffloadStopPlaying(_, _))
        .WillOnce(Return(MOCK_RETURN_VALUE));

    std::vector<int32_t> sessionsID;
    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice("00:00:00:00:00:00");
    EXPECT_EQ(AudioA2dpManager::OffloadStopPlaying(sessionsID), AudioStandard::ERROR);

    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice(A2DP_DEVICE_MAC2);
    EXPECT_NE(AudioA2dpManager::OffloadStopPlaying(sessionsID), AudioStandard::ERROR);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_010
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_010, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetRenderPosition(_, _, _, _))
        .WillOnce(Return(MOCK_RETURN_VALUE));

    uint32_t delayValue = MOCK_INPUT_VALUE;
    uint64_t sendDataSize = MOCK_INPUT_VALUE;
    uint32_t timeStamp = MOCK_INPUT_VALUE;
    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice("00:00:00:00:00:00");
    EXPECT_EQ(AudioA2dpManager::GetRenderPosition(delayValue, sendDataSize, timeStamp), AudioStandard::ERROR);

    AudioA2dpManager::activeA2dpDevice_ = BluetoothRemoteDevice(A2DP_DEVICE_MAC2);
    EXPECT_NE(AudioA2dpManager::GetRenderPosition(delayValue, sendDataSize, timeStamp), AudioStandard::ERROR);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_011
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_011, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetDevicesByStates(_, _))
        .WillOnce(Invoke([](const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) {
            devices.push_back(BluetoothRemoteDevice(A2DP_DEVICE_MAC1, 0)); // 0 means BTTransport::ADAPTER_BREDR
            return MOCK_RETURN_VALUE;
        }));
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetVirtualDeviceList(_))
        .WillOnce(Invoke([](std::vector<std::string> &devices) {
            devices.push_back(A2DP_DEVICE_MAC2);
        }));

    DeviceStatusObserverMock observer;
    RegisterDeviceObserver(observer);
    AudioA2dpManager::CheckA2dpDeviceReconnect();
    EXPECT_EQ(MediaBluetoothDeviceManager::virtualDevices_.size(), 1);
    EXPECT_EQ(MediaBluetoothDeviceManager::negativeDevices_.size(), 1);
}

/**
 * @tc.name  : Test BluetoothA2dpManagerTest.
 * @tc.number: BluetoothA2dpManagerTest_012
 * @tc.desc  : Test a2dp device manager.
 */
HWTEST_F(BluetoothA2dpManagerTest, BluetoothA2dpManagerTest_012, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), GetVirtualDeviceList(_))
        .WillOnce(Invoke([](std::vector<std::string> &devices) {
            devices.push_back(A2DP_DEVICE_MAC1);
        }));
    EXPECT_CALL(*(BluetoothA2dpMockInterface::mockInterface_.get()), Connect(_))
        .WillOnce(Return(MOCK_RETURN_VALUE0));

    EXPECT_EQ(AudioA2dpManager::Connect(A2DP_DEVICE_MAC1), SUCCESS);
}
} // namespace Bluetooth
} // namespace OHOS