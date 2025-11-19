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
#include "bluetooth_sco_manager.h"
#include "bluetooth_hfp_mock_interface.h"
#include "bluetooth_errorcode.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

using namespace testing::ext;
using namespace testing;

#define HFP_DEVICE_MAC1 "28:FA:19:1E:41:0E"
#define HFP_DEVICE_MAC2 "24:E9:CA:60:2F:CB"
#define TEST_VIRTUAL_CALL_BUNDLE_NAME "test.service"

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
};

class BluetoothHfpManagerTest : public testing::Test {
public:
    void SetUp(void) override
    {
        BluetoothHfpMockInterface::mockInterface_ = std::make_shared<BluetoothHfpMockInterface>();

        HfpBluetoothDeviceManager::hfpBluetoothDeviceMap_[HFP_DEVICE_MAC1] =
            BluetoothRemoteDevice(HFP_DEVICE_MAC1);
        HfpBluetoothDeviceManager::hfpBluetoothDeviceMap_[HFP_DEVICE_MAC2] =
            BluetoothRemoteDevice(HFP_DEVICE_MAC2);
    }

    void TearDown(void) override
    {
        BluetoothScoManager::GetInstance().scoTimer_ = nullptr;
        BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::DISCONNECTED;
        BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_DEFAULT;
        BluetoothScoManager::GetInstance().cacheReq_ = nullptr;
        BluetoothScoManager::GetInstance().currentScoDevice_ = BluetoothRemoteDevice();
        BluetoothHfpMockInterface::mockInterface_ = nullptr;

        AudioHfpManager::hfpListener_ = nullptr;
        AudioHfpManager::scene_ = AUDIO_SCENE_DEFAULT;
        AudioHfpManager::isRecognitionScene_.store(false);
        AudioHfpManager::isRecordScene_.store(false);
        AudioHfpManager::virtualCalls_.clear();
        AudioHfpManager::activeHfpDevice_ = BluetoothRemoteDevice();
    }
};

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_001
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_001, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetScoState(_))
        .Times(1)
        .WillOnce(Return(AudioScoState::DISCONNECTED));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetActiveDevice())
        .Times(AnyNumber());
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), SetActiveDevice(_))
        .Times(2)
        .WillOnce(Return(SUCCESS))
        .WillOnce(Return(SUCCESS));

    EXPECT_NE(AudioHfpManager::SetActiveHfpDevice("33:33:33"), SUCCESS);
    EXPECT_EQ(AudioHfpManager::SetActiveHfpDevice(HFP_DEVICE_MAC1), SUCCESS);
    EXPECT_EQ(AudioHfpManager::SetActiveHfpDevice(HFP_DEVICE_MAC2), SUCCESS);
    EXPECT_EQ(AudioHfpManager::SetActiveHfpDevice(""), SUCCESS);
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_002
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_002, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), SetActiveDevice(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), IsInbandRingingEnabled(_))
        .Times(1)
        .WillOnce(Invoke([](bool &enable) ->int32_t {
            enable = true;
            return SUCCESS;
        }));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    
    EXPECT_EQ(AudioHfpManager::SetActiveHfpDevice(HFP_DEVICE_MAC1), SUCCESS);
    AudioHfpManager::UpdateAudioScene(AUDIO_SCENE_VOICE_RINGING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_CALLULAR), true);
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_003
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_003, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), SetActiveDevice(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), IsInbandRingingEnabled(_))
        .WillRepeatedly(Invoke([](bool &enable) ->int32_t {
            enable = true;
            return SUCCESS;
        }));
    
    EXPECT_EQ(AudioHfpManager::SetActiveHfpDevice(HFP_DEVICE_MAC2), SUCCESS);
    AudioHfpManager::UpdateAudioScene(AUDIO_SCENE_PHONE_CHAT);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_VIRTUAL), true);
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_004
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_004, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), IsInbandRingingEnabled(_))
        .WillRepeatedly(Invoke([](bool &enable) ->int32_t {
            enable = true;
            return SUCCESS;
        }));
    AudioHfpManager::activeHfpDevice_ = BluetoothRemoteDevice(HFP_DEVICE_MAC2);

    AudioHfpManager::UpdateAudioScene(AUDIO_SCENE_DEFAULT, true);
    EXPECT_EQ(AudioHfpManager::IsAudioScoStateConnect(), true);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_DEFAULT), true);
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_005
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_005, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), OpenVoiceRecognition(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), IsInbandRingingEnabled(_))
        .WillRepeatedly(Invoke([](bool &enable) ->int32_t {
            enable = true;
            return SUCCESS;
        }));
    AudioHfpManager::activeHfpDevice_ = BluetoothRemoteDevice(HFP_DEVICE_MAC2);

    AudioHfpManager::HandleScoWithRecongnition(true);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_RECOGNITION), true);
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_006
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_006, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), IsInbandRingingEnabled(_))
        .WillRepeatedly(Invoke([](bool &enable) ->int32_t {
            enable = true;
            return SUCCESS;
        }));
    AudioHfpManager::scene_ = AUDIO_SCENE_PHONE_CHAT;
    AudioHfpManager::activeHfpDevice_ = BluetoothRemoteDevice(HFP_DEVICE_MAC2);

    AudioHfpManager::SetVirtualCall(0, false);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_CALLULAR), true);
    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_CONNECTED,
        AudioHfpManager::activeHfpDevice_);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTED);

    AudioHfpManager::SetVirtualCall(0, true);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::DISCONNECTING);
    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED,
        AudioHfpManager::activeHfpDevice_);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_VIRTUAL), true);
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: HandleUserSelection_001
 * @tc.desc  : Test HandleUserSelection
 */
HWTEST_F(BluetoothHfpManagerTest, HandleUserSelection_001, TestSize.Level1)
{
    DeviceStatusObserverMock observer;
    RegisterDeviceObserver(observer);
    BluetoothRemoteDevice device1(HFP_DEVICE_MAC1);
    HfpBluetoothDeviceManager::HandleUserSelection(device1);
    BluetoothRemoteDevice device2(HFP_DEVICE_MAC2);
    HfpBluetoothDeviceManager::HandleUserSelection(device2);
    EXPECT_NE(device1.GetDeviceAddr(), device2.GetDeviceAddr());
}

/**
 * @tc.name  : Test BluetoothHfpManagerTest.
 * @tc.number: BluetoothHfpManagerTest_007
 * @tc.desc  : Test hfp device manager.
 */
HWTEST_F(BluetoothHfpManagerTest, BluetoothHfpManagerTest_007, TestSize.Level1)
{
    EXPECT_NE(AudioHfpManager::ClearActiveHfpDevice("33:33:33"), SUCCESS);
    EXPECT_EQ(AudioHfpManager::ClearActiveHfpDevice(""), SUCCESS);
}
} // namespace Bluetooth
} // namespace OHOS