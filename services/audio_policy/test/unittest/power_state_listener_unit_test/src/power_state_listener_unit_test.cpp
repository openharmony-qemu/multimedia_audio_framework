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

#include "power_state_listener_unit_test.h"
#include "suspend/sync_sleep_callback_ipc_interface_code.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void PowerStateListenerUnitTest::SetUpTestCase(void) {}
void PowerStateListenerUnitTest::TearDownTestCase(void) {}
void PowerStateListenerUnitTest::SetUp(void) {}
void PowerStateListenerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test PowerStateListener.
* @tc.number: PowerStateListenerUnitTest_001.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<PowerStateListenerStub> powerStateListenerStub = std::make_shared<PowerStateListener>(
        audioPolicyServer);
    ASSERT_TRUE(powerStateListenerStub != nullptr);

    uint32_t code = static_cast<int32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(PowerStateListenerStub::GetDescriptor());
    auto result = powerStateListenerStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : Test PowerStateListener.
* @tc.number: PowerStateListenerUnitTest_002.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_002, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<PowerStateListenerStub> powerStateListenerStub = std::make_shared<PowerStateListener>(
        audioPolicyServer);
    ASSERT_TRUE(powerStateListenerStub != nullptr);

    uint32_t code = static_cast<int32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(PowerStateListenerStub::GetDescriptor());
    auto result = powerStateListenerStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : Test PowerStateListener.
* @tc.number: PowerStateListenerUnitTest_003.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_003, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<PowerStateListenerStub> powerStateListenerStub = std::make_shared<PowerStateListener>(
        audioPolicyServer);
    ASSERT_TRUE(powerStateListenerStub != nullptr);

    uint32_t code = -1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(PowerStateListenerStub::GetDescriptor());
    powerStateListenerStub->OnRemoteRequest(code, data, reply, option);
}

/**
* @tc.name  : Test PowerStateListener.
* @tc.number: PowerStateListenerUnitTest_004.
* @tc.desc  : Test ControlAudioFocus.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_004, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    auto powerStateListener = std::make_shared<PowerStateListener>(audioPolicyServer);
    ASSERT_TRUE(powerStateListener != nullptr);

    powerStateListener->audioPolicyServer_ = nullptr;
    powerStateListener->ControlAudioFocus(true);
}

/**
* @tc.name  : Test SyncHibernateListenerStub.
* @tc.number: PowerStateListenerUnitTest_005.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_005, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    ASSERT_TRUE(syncHibernateListenerStub != nullptr);

    uint32_t code = static_cast<int32_t>(PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SyncHibernateListenerStub::GetDescriptor());
    auto result = syncHibernateListenerStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : Test SyncHibernateListenerStub.
* @tc.number: PowerStateListenerUnitTest_006.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_006, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    ASSERT_TRUE(syncHibernateListenerStub != nullptr);

    uint32_t code = static_cast<int32_t>(PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SyncHibernateListenerStub::GetDescriptor());
    auto result = syncHibernateListenerStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : Test SyncHibernateListenerStub.
* @tc.number: PowerStateListenerUnitTest_007.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_007, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    ASSERT_TRUE(syncHibernateListenerStub != nullptr);

    uint32_t code = -1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SyncHibernateListenerStub::GetDescriptor());
    auto result = syncHibernateListenerStub->OnRemoteRequest(code, data, reply, option);
}

/**
* @tc.name  : Test SyncHibernateListenerStub.
* @tc.number: PowerStateListenerUnitTest_008.
* @tc.desc  : Test ControlAudioFocus.
*/
HWTEST_F(PowerStateListenerUnitTest, PowerStateListenerUnitTest_008, TestSize.Level1)
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    ASSERT_TRUE(syncHibernateListenerStub != nullptr);

    syncHibernateListenerStub->OnSyncWakeupCallbackStub();
}
}
}