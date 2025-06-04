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

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

using namespace testing::ext;
using namespace testing;

class BluetoothScoManagerTest : public testing::Test {
public:
    void SetUp(void) override {}

    void TearDown(void) override
    {
        BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::DISCONNECTED;
        BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_DEFAULT;
        BluetoothScoManager::GetInstance().cacheReq_ = nullptr;
    }
};

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_001
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_001, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_VIRTUAL, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_VIRTUAL), true);

    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTED);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_VIRTUAL), true);

    ret = BluetoothScoManager::GetInstance().HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::DISCONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_VIRTUAL), false);

    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::DISCONNECTED);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_VIRTUAL), false);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_002
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_002, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(0);
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::CONNECTED;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_VIRTUAL;

    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_VIRTUAL, device);
    EXPECT_EQ(ret, SUCCESS);
    ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_DEFAULT, device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_003
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_003, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(0);
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::CONNECTED;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_RECOGNITION;

    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_VIRTUAL, device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_004
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_004, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(0);
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::CONNECTED;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_CALLULAR;

    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_RECOGNITION, device);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_005
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_005, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(0);
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::CONNECTING;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_CALLULAR;

    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);
    ret = BluetoothScoManager::GetInstance().HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);


    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::DISCONNECTING);
    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::DISCONNECTED);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_006
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_006, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, DisconnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::CONNECTED;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_RECOGNITION, device);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::DISCONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_DEFAULT), false);

    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_RECOGNITION), true);

    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_CONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTED);
    EXPECT_EQ(BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_RECOGNITION), true);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_007
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_007, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, DisconnectSco(_))
        .Times(0);
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::DISCONNECTING;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = BluetoothScoManager::GetInstance().HandleScoConnect(ScoCategory::SCO_CALLULAR, device);
    EXPECT_EQ(ret, SUCCESS);

    BluetoothScoManager::GetInstance().UpdateScoState(HfpScoConnectState::SCO_DISCONNECTED, device);
    EXPECT_EQ(BluetoothScoManager::GetInstance().GetAudioScoState(), AudioScoState::CONNECTING);
}

/**
 * @tc.name  : Test BluetoothScoManager.
 * @tc.number: BluetoothScoManagerTest_008
 * @tc.desc  : Test connect sco.
 */
HWTEST_F(BluetoothScoManagerTest, BluetoothScoManagerTest_008, TestSize.Level1)
{
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, ConnectSco(_))
        .Times(0);
    EXPECT_CALL(BluetoothHfpMockInterface::mockInterface_, DisconnectSco(_))
        .Times(0);
    
    BluetoothRemoteDevice device("11::22::33::44::55::66");
    BluetoothScoManager::GetInstance().activeHfpDevice_ = device;
    BluetoothScoManager::GetInstance().currentScoState_ = AudioScoState::DISCONNECTING;
    BluetoothScoManager::GetInstance().currentScoCategory_ = ScoCategory::SCO_DEFAULT;

    int ret = BluetoothScoManager::GetInstance().HandleScoDisconnect(device);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace Bluetooth
} // namespace OHOS
