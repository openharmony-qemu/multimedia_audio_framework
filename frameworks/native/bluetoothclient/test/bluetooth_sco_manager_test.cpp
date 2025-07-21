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

#include <unistd.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "bluetooth_sco_manager.h"
#include "bluetooth_hfp_mock_interface.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

using namespace testing::ext;
using namespace testing;

#define HFP_TEST_DEVICE_MAC "11:22:33:44:55:66"

class BluetoothScoManagerTest : public testing::Test {
public:
    void SetUp(void) override
    {
        BluetoothHfpMockInterface::mockInterface_ = std::make_shared<BluetoothHfpMockInterface>();
    }

    void TearDown(void) override
    {
        BluetoothHfpMockInterface::mockInterface_ = nullptr;
    }

private:
    void SetDefaultExpectCall()
    {
        EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetScoState(_))
            .Times(1)
            .WillOnce(Return(AudioScoState::DISCONNECTED));
        EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetActiveDevice())
            .Times(AnyNumber());
    }
};

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_001
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_001, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_VIRTUAL, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_VIRTUAL), true);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTED);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_VIRTUAL), true);

    ret = scoManager.HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_VIRTUAL), false);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTED);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_VIRTUAL), false);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_002
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_002, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_VIRTUAL;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_VIRTUAL, device);
    EXPECT_EQ(ret, SUCCESS);
    ret = scoManager.HandleScoConnect(ScoCategory::SCO_DEFAULT, device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_003
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_003, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), CloseVoiceRecognition(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_RECOGNITION;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_004
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_004, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_CALLULAR;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_RECOGNITION, device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_005
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_005, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTING;
    scoManager.currentScoCategory_ = ScoCategory::SCO_CALLULAR;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    ret = scoManager.HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);
    scoManager.UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTED);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_006
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_006, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), OpenVoiceRecognition(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), CloseVoiceRecognition(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_RECOGNITION, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_DEFAULT), false);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_RECOGNITION), true);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTED);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_RECOGNITION), true);

    ret = scoManager.HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);
    scoManager.UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTED);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_007
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_007, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(0);

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTING;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_008
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_008, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(0);

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTING;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_009
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_009, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device1(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device1;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_CALLULAR;

    BluetoothRemoteDevice device2("01:02:03:04:05:06");
    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device2);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);

    scoManager.UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device1);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_010
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_010, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device1(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device1;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_CALLULAR;

    BluetoothRemoteDevice device2("01:02:03:04:05:06");
    int ret = scoManager.HandleScoDisconnect(device2);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_011
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_011, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(BT_ERR_SCO_HAS_BEEN_CONNECTED));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTED);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_012
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_012, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(2)
        .WillOnce(Return(BT_ERR_SCO_HAS_BEEN_CONNECTED + 1))
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), SetActiveDevice(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_CALLULAR), true);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_013
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_013, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(BT_ERR_VIRTUAL_CALL_NOT_STARTED + 1));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTED);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_014
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_014, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(0);

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;
    scoManager.scoStateDuration_ = 1;

    int ret = scoManager.HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
    for (int32_t i = 0; i < 15; i++) { /* 15: 1.5s */
        EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
        usleep(100000); /* 100000: 100ms */
    }
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_020
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_020, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetCurrentCategory(_))
        .Times(1)
        .WillOnce(Invoke([](ScoCategory &category) ->int32_t {
            category = ScoCategory::SCO_CALLULAR;
            return SUCCESS;
        }));

    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), ConnectSco(_))
        .Times(0);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), DisconnectSco(_))
        .Times(2)
        .WillOnce(Return(BT_ERR_VIRTUAL_CALL_NOT_STARTED))
        .WillOnce(Return(SUCCESS));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = scoManager.HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTING);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_021
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_021, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetCurrentCategory(_))
        .Times(1)
        .WillOnce(Invoke([](ScoCategory &category) ->int32_t {
            category = ScoCategory::SCO_CALLULAR;
            return SUCCESS;
        }));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;
    scoManager.UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_022
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_022, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetCurrentCategory(_))
        .Times(1)
        .WillOnce(Invoke([](ScoCategory &category) ->int32_t {
            category = ScoCategory::SCO_CALLULAR;
            return SUCCESS;
        }));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTED;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;
    scoManager.UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_CALLULAR), true);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_023
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_023, TestSize.Level1)
{
    SetDefaultExpectCall();
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetCurrentCategory(_))
        .Times(1)
        .WillOnce(Invoke([](ScoCategory &category) ->int32_t {
            category = ScoCategory::SCO_CALLULAR;
            return SUCCESS;
        }));

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::DISCONNECTING;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;
    scoManager.UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_CALLULAR), true);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_024
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_024, TestSize.Level1)
{
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetScoState(_))
        .Times(1)
        .WillOnce(Return(AudioScoState::CONNECTED));
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetActiveDevice())
        .Times(1);
    EXPECT_CALL(*(BluetoothHfpMockInterface::mockInterface_.get()), GetCurrentCategory(_))
        .Times(1)
        .WillOnce(Invoke([](ScoCategory &category) ->int32_t {
            category = ScoCategory::SCO_CALLULAR;
            return SUCCESS;
        }));

    BluetoothScoManager scoManager;
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTED);
    EXPECT_EQ(scoManager.IsInScoCategory(ScoCategory::SCO_CALLULAR), true);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_025
 * @tc.desc  : Test reset sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_025, TestSize.Level1)
{
    SetDefaultExpectCall();

    BluetoothScoManager scoManager;
    BluetoothRemoteDevice device(HFP_TEST_DEVICE_MAC);
    scoManager.currentScoDevice_ = device;
    scoManager.currentScoState_ = AudioScoState::CONNECTING;
    scoManager.currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    scoManager.ResetScoState(BluetoothRemoteDevice("1234"));
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::CONNECTING);
    scoManager.ResetScoState(device);
    EXPECT_EQ(scoManager.GetAudioScoState(), AudioScoState::DISCONNECTED);
}
} // namespace Bluetooth
} // namespace OHOS
