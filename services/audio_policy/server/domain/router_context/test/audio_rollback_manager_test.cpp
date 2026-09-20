/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <chrono>
#include <mutex>
#include <thread>
#include "audio_router_context_test_base.h"
#include "audio_rollback_manager.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_router_center.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t MS_OVER_2SEC = 2300;

class AudioRollbackManagerTest : public AudioRouterContextTestBase {
public:
    static void TearDownTestCase(void)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    }
    void TearDown() override;

    std::shared_ptr<AudioDeviceDescriptor> AddConnectedDevice(DeviceType type, const std::string &mac);
    std::shared_ptr<AudioDeviceDescriptor> MakeLocalDevice(DeviceType type, const std::string &mac,
        ConnectState state);
    int32_t GetSystemDeviceId(SelectDeviceType type);
    void SetOutputBaseline(const std::shared_ptr<AudioDeviceDescriptor> &device);
    void ClearRollbackState();
    bool HasSelectionRollback(int32_t targetId);
    bool HasRollbackInfo();
    int32_t RollbackDeviceId();
    std::shared_ptr<AudioDeviceSimpleDescriptor> ToSimple(
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    bool ContainsSelection(const std::vector<Selection> &sels, int32_t deviceId);
};

void AudioRollbackManagerTest::TearDown()
{
    AudioRouterContextTestBase::TearDown();
    ClearRollbackState();
}

void AudioRollbackManagerTest::ClearRollbackState()
{
    auto &rb = AudioRollbackManager::GetInstance();
    std::lock_guard<std::mutex> lg(rb.mutex_);
    rb.selectionRollbacks_.clear();
    rb.rollbackInfo_.reset();
}

bool AudioRollbackManagerTest::HasSelectionRollback(int32_t targetId)
{
    auto &rb = AudioRollbackManager::GetInstance();
    std::lock_guard<std::mutex> lg(rb.mutex_);
    for (auto &item : rb.selectionRollbacks_) {
        if (item.targetId == targetId) {
            return true;
        }
    }
    return false;
}

bool AudioRollbackManagerTest::HasRollbackInfo()
{
    auto &rb = AudioRollbackManager::GetInstance();
    std::lock_guard<std::mutex> lg(rb.mutex_);
    return rb.rollbackInfo_ != nullptr;
}

int32_t AudioRollbackManagerTest::RollbackDeviceId()
{
    auto &rb = AudioRollbackManager::GetInstance();
    std::lock_guard<std::mutex> lg(rb.mutex_);
    return rb.rollbackInfo_ ? rb.rollbackInfo_->deviceId : -1;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRollbackManagerTest::AddConnectedDevice(DeviceType type,
    const std::string &mac)
{
    auto dev = AddDeviceDescriptor(type, OUTPUT_DEVICE, mac);
    auto stored = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(dev->deviceId_);
    if (stored) {
        stored->connectState_ = CONNECTED;
        return stored;
    }
    return dev;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRollbackManagerTest::MakeLocalDevice(DeviceType type,
    const std::string &mac, ConnectState state)
{
    auto dev = CreateDeviceDescriptor(type, OUTPUT_DEVICE, mac);
    dev->connectState_ = state;
    return dev;
}

int32_t AudioRollbackManagerTest::GetSystemDeviceId(SelectDeviceType type)
{
    auto info = AudioRouterInfra::GetInstance().GetSystemSelectDevice(type);
    return info.device_ ? info.device_->deviceId_ : -1;
}

void AudioRollbackManagerTest::SetOutputBaseline(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, device);
}

/**
 * @tc.name  : Scenario1_ForceSelectUnconnected_NoConnectIn2s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(立即生效)-2s内未连接成功-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario1_ForceSelectUnconnected_NoConnectIn2s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:01");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:01", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    ASSERT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_FALSE(HasSelectionRollback(newDev->deviceId_));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario2_ForceSelectUnconnected_ConnectIn2s_NoRollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(立即生效)-2s内连接成功-应保持新设备(不回滚)
 */
HWTEST_F(AudioRollbackManagerTest, Scenario2_ForceSelectUnconnected_ConnectIn2s_NoRollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:02");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:02", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    EXPECT_FALSE(HasSelectionRollback(newDev->deviceId_));

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
}

/**
 * @tc.name  : Scenario3_ForceSelectUnconnected_NoCurrentSelect_NoRollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(当前无强选)-无旧选择可回滚-应保持新设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario3_ForceSelectUnconnected_NoCurrentSelect_NoRollback, TestSize.Level1)
{
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:03", VIRTUAL_CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    EXPECT_FALSE(HasSelectionRollback(newDev->deviceId_));

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
}

/**
 * @tc.name  : Scenario4_ForceSelectUnconnected_CallOutput_NoConnectIn2s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接CALL_OUTPUT设备-2s内未连接成功-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario4_ForceSelectUnconnected_CallOutput_NoConnectIn2s_Rollback,
    TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:04");
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev, "ut");
    ASSERT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), oldDev->deviceId_);
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:04", VIRTUAL_CONNECTED);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev, "ut");
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), newDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_FALSE(HasSelectionRollback(newDev->deviceId_));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario5_NewDeviceOnline_NoConnectIn2s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 新设备上线-2s内没有连接成功-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario5_NewDeviceOnline_NoConnectIn2s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:06");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:06", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.OnDeviceNewConnect(*newDev);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), newDev->deviceId_);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario6_NewDeviceOnline_ConnectIn2s_LostIn1s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 新设备上线-2s内连接成功-1s内连接丢失-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario6_NewDeviceOnline_ConnectIn2s_LostIn1s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:07");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:07", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.OnDeviceNewConnect(*newDev);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    rollback.RollbackSelections(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : OnDeviceNewConnect_InvalidAndReuse
 * @tc.type  : FUNC
 * @tc.desc  : OnDeviceNewConnect rejects connected/input-role device, accepts not-connected states,
 *             and reuses existing rollback info.
 */
HWTEST_F(AudioRollbackManagerTest, OnDeviceNewConnect_InvalidAndReuse, TestSize.Level1)
{
    auto &rollback = AudioRollbackManager::GetInstance();
    auto connDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:40", CONNECTED);
    rollback.OnDeviceNewConnect(*connDev);
    EXPECT_FALSE(HasRollbackInfo());

    auto inputDev = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, INPUT_DEVICE, "00:00:00:00:0B:43");
    inputDev->connectState_ = VIRTUAL_CONNECTED;
    rollback.OnDeviceNewConnect(*inputDev);
    EXPECT_FALSE(HasRollbackInfo());

    auto suspendDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:44", SUSPEND_CONNECTED);
    rollback.OnDeviceNewConnect(*suspendDev);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), suspendDev->deviceId_);

    auto deactiveDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:45", DEACTIVE_CONNECTED);
    rollback.OnDeviceNewConnect(*deactiveDev);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), deactiveDev->deviceId_);

    auto dev1 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:41", VIRTUAL_CONNECTED);
    rollback.OnDeviceNewConnect(*dev1);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), dev1->deviceId_);

    auto dev2 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:42", VIRTUAL_CONNECTED);
    rollback.OnDeviceNewConnect(*dev2);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), dev2->deviceId_);
}

/**
 * @tc.name  : SetConnectLostRollback_InvalidAndAlreadyLost
 * @tc.type  : FUNC
 * @tc.desc  : SetConnectLostRollback ignores null/non-connected device and duplicate CONNECT_LOST.
 */
HWTEST_F(AudioRollbackManagerTest, SetConnectLostRollback_InvalidAndAlreadyLost, TestSize.Level1)
{
    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetConnectLostRollback(nullptr);
    EXPECT_FALSE(HasRollbackInfo());
    auto nonConn = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:50", VIRTUAL_CONNECTED);
    rollback.SetConnectLostRollback(nonConn);
    EXPECT_FALSE(HasRollbackInfo());

    auto dev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:51", CONNECTED);
    rollback.SetConnectLostRollback(dev);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), dev->deviceId_);
    rollback.SetConnectLostRollback(dev);
    EXPECT_EQ(RollbackDeviceId(), dev->deviceId_);
}

/**
 * @tc.name  : SetConnectLostRollback_ClearsSelectionRollbacks
 * @tc.type  : FUNC
 * @tc.desc  : SetConnectLostRollback clears pending selection rollbacks on connect.
 */
HWTEST_F(AudioRollbackManagerTest, SetConnectLostRollback_ClearsSelectionRollbacks, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:70");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:70", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    EXPECT_FALSE(HasSelectionRollback(newDev->deviceId_));
}

/**
 * @tc.name  : RollbackSelections_MismatchAndSkipNotConnected
 * @tc.type  : FUNC
 * @tc.desc  : RollbackSelections returns on mismatch and skips snapshot items not connected.
 */
HWTEST_F(AudioRollbackManagerTest, RollbackSelections_MismatchAndSkipNotConnected, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:60");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:60", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.OnDeviceNewConnect(*newDev);
    EXPECT_EQ(RollbackDeviceId(), newDev->deviceId_);
    rollback.RollbackSelections(oldDev->deviceId_, NEW_CONNECT_ROLLBACK);
    EXPECT_TRUE(HasRollbackInfo());
    rollback.RollbackSelections(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_TRUE(HasRollbackInfo());

    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    oldDev->connectState_ = VIRTUAL_CONNECTED;
    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_FALSE(HasRollbackInfo());
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
}

/**
 * @tc.name  : SetDevice_Wrappers_AppliedForUnconnected
 * @tc.type  : FUNC
 * @tc.desc  : Set{Media,Call}{Input,Output}Device apply immediately for unconnected devices.
 */
HWTEST_F(AudioRollbackManagerTest, SetDevice_Wrappers_AppliedForUnconnected, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:30");
    SetOutputBaseline(oldDev);
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev, "ut");
    auto unDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:30", VIRTUAL_CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), unDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(unDev->deviceId_));
    rollback.SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_INPUT), unDev->deviceId_);
    rollback.SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_INPUT), unDev->deviceId_);
    rollback.SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev, "ut");
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), unDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(unDev->deviceId_));
}

/**
 * @tc.name  : SetDevice_Wrappers_AppliedForConnected
 * @tc.type  : FUNC
 * @tc.desc  : Set{Media,Call}{Input,Output}Device apply for connected devices.
 */
HWTEST_F(AudioRollbackManagerTest, SetDevice_Wrappers_AppliedForConnected, TestSize.Level1)
{
    auto connDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:31", CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), connDev->deviceId_);
    EXPECT_FALSE(HasSelectionRollback(connDev->deviceId_));
    rollback.SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_INPUT), connDev->deviceId_);
    rollback.SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_INPUT), connDev->deviceId_);
    rollback.SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev, "ut");
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), connDev->deviceId_);
    EXPECT_FALSE(HasSelectionRollback(connDev->deviceId_));
}

/**
 * @tc.name  : OnDeviceDisconnect_RollsBackSelectionRollbacks
 * @tc.type  : FUNC
 * @tc.desc  : OnDeviceDisconnect rolls back pending selection rollbacks for the disconnected device.
 */
HWTEST_F(AudioRollbackManagerTest, OnDeviceDisconnect_RollsBackSelectionRollbacks, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:80");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:80", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    ASSERT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    rollback.OnDeviceDisconnect(*newDev);
    EXPECT_FALSE(HasSelectionRollback(newDev->deviceId_));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : OnDeviceDisconnect_RollsBackConnectLost
 * @tc.type  : FUNC
 * @tc.desc  : OnDeviceDisconnect rolls back connect-lost rollback info for the disconnected device.
 */
HWTEST_F(AudioRollbackManagerTest, OnDeviceDisconnect_RollsBackConnectLost, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:81");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:81", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.OnDeviceNewConnect(*newDev);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    rollback.OnDeviceDisconnect(*newDev);
    EXPECT_FALSE(HasRollbackInfo());
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : OnDeviceDisconnect_NoMatch
 * @tc.type  : FUNC
 * @tc.desc  : OnDeviceDisconnect does nothing when no matching selection or rollback info exists.
 */
HWTEST_F(AudioRollbackManagerTest, OnDeviceDisconnect_NoMatch, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:82");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:82", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    auto otherDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:83", VIRTUAL_CONNECTED);
    rollback.OnDeviceDisconnect(*otherDev);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
}

/**
 * @tc.name  : SetSelectionRollback_SkipsSameDeviceId
 * @tc.type  : FUNC
 * @tc.desc  : SetSelectionRollback skips recording when the existing selection already points to
 *             the target device (item.deviceId == deviceId).
 */
HWTEST_F(AudioRollbackManagerTest, SetSelectionRollback_SkipsSameDeviceId, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:90");
    SetOutputBaseline(oldDev);
    ASSERT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);

    oldDev->connectState_ = VIRTUAL_CONNECTED;
    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    EXPECT_FALSE(HasSelectionRollback(oldDev->deviceId_));
}

/**
 * @tc.name  : RollbackSelection_DeactiveConnected
 * @tc.type  : FUNC
 * @tc.desc  : RollbackSelection restores a DEACTIVE_CONNECTED device and notifies BT.
 */
HWTEST_F(AudioRollbackManagerTest, RollbackSelection_DeactiveConnected, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:91");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:91", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    EXPECT_TRUE(HasSelectionRollback(newDev->deviceId_));

    oldDev->connectState_ = DEACTIVE_CONNECTED;
    rollback.OnDeviceDisconnect(*newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Test OnConnectStateUpdate.
 * @tc.number: OnConnectStateUpdate_001
 * @tc.desc  : OnConnectStateUpdate early-returns on wrong command/missing device
 *             and drives rollback on connect/disconnect.
 */
HWTEST_F(AudioRollbackManagerTest, OnConnectStateUpdate_001, TestSize.Level4)
{
    auto &rb = AudioRollbackManager::GetInstance();
    {
        std::lock_guard<std::mutex> lg(rb.mutex_);
        rb.selectionRollbacks_.clear();
        rb.rollbackInfo_.reset();
    }

    AudioDeviceDescriptor nonConnCmd(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    nonConnCmd.macAddress_ = "AA:BB:CC:DD:EE:F1";
    EXPECT_NO_THROW(rb.OnConnectStateUpdate(nonConnCmd, ENABLE_UPDATE));

    AudioDeviceDescriptor notFound(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    notFound.macAddress_ = "00:11:22:33:44:99";
    EXPECT_NO_THROW(rb.OnConnectStateUpdate(notFound, CONNECTSTATE_UPDATE));

    auto btDev = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    btDev->macAddress_ = "AA:BB:CC:DD:EE:F1";
    btDev->networkId_ = LOCAL_NETWORK_ID;
    btDev->deviceId_ = AudioPolicyUtils::startDeviceId++;
    btDev->connectState_ = CONNECTED;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(btDev);

    AudioDeviceDescriptor connDesc(*btDev);
    EXPECT_NO_THROW(rb.OnConnectStateUpdate(connDesc, CONNECTSTATE_UPDATE));
    {
        std::lock_guard<std::mutex> lg(rb.mutex_);
        EXPECT_TRUE(rb.rollbackInfo_ != nullptr);
    }

    btDev->connectState_ = VIRTUAL_CONNECTED;
    AudioDeviceDescriptor disconnDesc(*btDev);
    EXPECT_NO_THROW(rb.OnConnectStateUpdate(disconnDesc, CONNECTSTATE_UPDATE));
    {
        std::lock_guard<std::mutex> lg(rb.mutex_);
        EXPECT_FALSE(rb.rollbackInfo_ != nullptr);
    }

    btDev->connectState_ = VIRTUAL_CONNECTED;
    AudioDeviceManager::GetAudioDeviceManager().RemoveConnectedDevices(btDev);
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRollbackManagerTest::ToSimple(
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    if (!device) {
        return nullptr;
    }
    return std::make_shared<AudioDeviceSimpleDescriptor>(device->deviceType_, device->deviceRole_,
        device->deviceId_, device->networkId_, device->macAddress_);
}

bool AudioRollbackManagerTest::ContainsSelection(const std::vector<Selection> &sels, int32_t deviceId)
{
    for (auto &s : sels) {
        if (s.deviceId == deviceId) {
            return true;
        }
    }
    return false;
}

/**
 * @tc.name  : GetSelections_SystemAppStream
 * @tc.type  : FUNC
 * @tc.desc  : GetSelections returns system, app and stream selections and skips invalid/missing.
 */
HWTEST_F(AudioRollbackManagerTest, GetSelections_SystemAppStream, TestSize.Level1)
{
    auto &infra = AudioRouterInfra::GetInstance();
    auto sysDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:70");
    auto appDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:71", CONNECTED);
    auto streamDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:72", CONNECTED);
    auto inputDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:73", CONNECTED);

    infra.RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, SYSTEM_UID, ToSimple(sysDev));
    infra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, SYSTEM_UID, ToSimple(sysDev));
    infra.RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, TEST_APP_UID_10001, ToSimple(appDev));
    infra.RefreshSelectDevice(SelectDeviceType::CALL_INPUT, TEST_APP_UID_10001, nullptr);
    infra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    infra.UpdateStreamSelectDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, ToSimple(streamDev));
    infra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    infra.UpdateOutputStreamState(TEST_APP_UID_10001, 4, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    infra.UpdateStreamSelectDevice(TEST_APP_UID_10001, 4, nullptr);
    infra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_3, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);
    infra.UpdateStreamSelectDevice(TEST_APP_UID_10001, TEST_STREAM_ID_3, ToSimple(inputDev));

    auto outputSels = infra.GetSelections({SelectDeviceType::MEDIA_OUTPUT, SelectDeviceType::CALL_OUTPUT});
    EXPECT_TRUE(ContainsSelection(outputSels, sysDev->deviceId_));
    EXPECT_TRUE(ContainsSelection(outputSels, appDev->deviceId_));
    EXPECT_TRUE(ContainsSelection(outputSels, streamDev->deviceId_));
    EXPECT_FALSE(ContainsSelection(outputSels, inputDev->deviceId_));

    auto inputSels = infra.GetSelections({SelectDeviceType::MEDIA_INPUT, SelectDeviceType::CALL_INPUT});
    EXPECT_TRUE(ContainsSelection(inputSels, inputDev->deviceId_));
    EXPECT_FALSE(ContainsSelection(inputSels, sysDev->deviceId_));
}

/**
 * @tc.name  : GetSelections_CallTypeViaConfig
 * @tc.type  : FUNC
 * @tc.desc  : GetSelections classifies call streams via router center config.
 */
HWTEST_F(AudioRollbackManagerTest, GetSelections_CallTypeViaConfig, TestSize.Level1)
{
    auto &center = AudioRouterCenter::GetAudioRouterCenter();
    center.renderConfigMap_[STREAM_USAGE_VOICE_COMMUNICATION] = "CallRenderRouters";
    center.capturerConfigMap_[SOURCE_TYPE_VOICE_COMMUNICATION] = "CallCaptureRouters";
    auto &infra = AudioRouterInfra::GetInstance();
    auto callOutDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:74", CONNECTED);
    auto callInDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:75", CONNECTED);

    infra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    infra.UpdateStreamSelectDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, ToSimple(callOutDev));
    infra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_2, SOURCE_TYPE_VOICE_COMMUNICATION,
        CapturerState::CAPTURER_RUNNING);
    infra.UpdateStreamSelectDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2, ToSimple(callInDev));

    auto outSels = infra.GetSelections({SelectDeviceType::CALL_OUTPUT});
    EXPECT_TRUE(ContainsSelection(outSels, callOutDev->deviceId_));
    auto inSels = infra.GetSelections({SelectDeviceType::CALL_INPUT});
    EXPECT_TRUE(ContainsSelection(inSels, callInDev->deviceId_));

    center.renderConfigMap_.erase(STREAM_USAGE_VOICE_COMMUNICATION);
    center.capturerConfigMap_.erase(SOURCE_TYPE_VOICE_COMMUNICATION);
}

/**
 * @tc.name  : HasValidSelectDevice_Branches
 * @tc.type  : FUNC
 * @tc.desc  : HasValidSelectDevice returns false for null/NONE and true for valid device.
 */
HWTEST_F(AudioRollbackManagerTest, HasValidSelectDevice_Branches, TestSize.Level1)
{
    auto &infra = AudioRouterInfra::GetInstance();
    AudioSelectDeviceInfo nullInfo;
    EXPECT_FALSE(infra.HasValidSelectDevice(nullInfo));
    auto noneDev = std::make_shared<AudioDeviceSimpleDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE, 0, "", "");
    AudioSelectDeviceInfo noneInfo{noneDev, 0};
    EXPECT_FALSE(infra.HasValidSelectDevice(noneInfo));
    auto validDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:76", CONNECTED);
    AudioSelectDeviceInfo validInfo{ToSimple(validDev), 0};
    EXPECT_TRUE(infra.HasValidSelectDevice(validInfo));
}

}
}
