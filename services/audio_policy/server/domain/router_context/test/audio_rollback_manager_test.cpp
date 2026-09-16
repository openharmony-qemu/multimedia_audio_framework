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
    bool HasContinueSelection(int32_t deviceId);
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
    rb.continueSelections_.clear();
    rb.rollbackInfo_.reset();
}

bool AudioRollbackManagerTest::HasContinueSelection(int32_t deviceId)
{
    auto &rb = AudioRollbackManager::GetInstance();
    std::lock_guard<std::mutex> lg(rb.mutex_);
    for (auto &item : rb.continueSelections_) {
        if (item.deviceId == deviceId) {
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
 * @tc.name  : Scenario1_ForceSelectUnconnected_ConnectIn2s_LostIn1s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(有强选)-2s内连接成功-1s内连接丢失-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario1_ForceSelectUnconnected_ConnectIn2s_LostIn1s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:01");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:01", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    ASSERT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
    EXPECT_TRUE(HasContinueSelection(newDev->deviceId_));

    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    rollback.ContinueSelection(newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    rollback.RollbackSelection(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario2_ForceSelectUnconnected_ConnectIn2s_LostAfter1s_NoRollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(有强选)-2s内连接成功-超过1s后连接丢失-不应回滚
 */
HWTEST_F(AudioRollbackManagerTest, Scenario2_ForceSelectUnconnected_ConnectIn2s_LostAfter1s_NoRollback,
    TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:02");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:02", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    rollback.ContinueSelection(newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    std::this_thread::sleep_for(std::chrono::milliseconds(1300));
    EXPECT_FALSE(HasRollbackInfo());

    rollback.RollbackSelection(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
}

/**
 * @tc.name  : Scenario3_ForceSelectUnconnected_NoConnectIn2s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(有强选)-2s内没有连接成功-应回滚(维持旧强选设备)
 */
HWTEST_F(AudioRollbackManagerTest, Scenario3_ForceSelectUnconnected_NoConnectIn2s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:03");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:03", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
    EXPECT_TRUE(HasContinueSelection(newDev->deviceId_));

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_FALSE(HasContinueSelection(newDev->deviceId_));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario4_ForceSelectUnconnected_ConnectFailedIn2s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(有强选)-2s内上报连接失败-应回滚(维持旧强选设备)
 */
HWTEST_F(AudioRollbackManagerTest, Scenario4_ForceSelectUnconnected_ConnectFailedIn2s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:04");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:04", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
    EXPECT_TRUE(HasContinueSelection(newDev->deviceId_));

    rollback.RollbackSelection(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario5_ForceSelectUnconnected_NoCurrentForceSelect_NoRollback
 * @tc.type  : FUNC
 * @tc.desc  : 强选未连接设备(当前无强选)-2s内没有连接成功-不应回滚(默认路由保持)
 */
HWTEST_F(AudioRollbackManagerTest, Scenario5_ForceSelectUnconnected_NoCurrentForceSelect_NoRollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:05");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:05", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_FALSE(HasContinueSelection(newDev->deviceId_));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario6_NewDeviceOnline_NoConnectIn2s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 新设备上线-2s内没有连接成功-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario6_NewDeviceOnline_NoConnectIn2s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:06");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:06", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetNewConnectRollback(newDev);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), newDev->deviceId_);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : Scenario7_NewDeviceOnline_ConnectIn2s_LostIn1s_Rollback
 * @tc.type  : FUNC
 * @tc.desc  : 新设备上线-2s内连接成功-1s内连接丢失-应回滚到旧设备
 */
HWTEST_F(AudioRollbackManagerTest, Scenario7_NewDeviceOnline_ConnectIn2s_LostIn1s_Rollback, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:07");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:07", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);

    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetNewConnectRollback(newDev);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);

    newDev->connectState_ = CONNECTED;
    rollback.SetConnectLostRollback(newDev);
    rollback.ContinueSelection(newDev);

    rollback.RollbackSelection(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : HandleNonConnected_InvalidDevice
 * @tc.type  : FUNC
 * @tc.desc  : HandleNonConnected returns false when device is null or deviceId <= 0.
 */
HWTEST_F(AudioRollbackManagerTest, HandleNonConnected_InvalidDevice, TestSize.Level1)
{
    auto &rollback = AudioRollbackManager::GetInstance();
    EXPECT_FALSE(rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, nullptr));
    auto badDev = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    badDev->deviceId_ = 0;
    EXPECT_FALSE(rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, badDev));
}

/**
 * @tc.name  : HandleNonConnected_ConnectedDevice_NotHandled
 * @tc.type  : FUNC
 * @tc.desc  : HandleNonConnected returns false for connected device and SetMediaOutputDevice applies it.
 */
HWTEST_F(AudioRollbackManagerTest, HandleNonConnected_ConnectedDevice_NotHandled, TestSize.Level1)
{
    auto connDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:10", CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();
    bool handled = rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, connDev);
    EXPECT_FALSE(handled);

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), connDev->deviceId_);
}

/**
 * @tc.name  : HandleNonConnected_LowUid_BumpedToSystem
 * @tc.type  : FUNC
 * @tc.desc  : HandleNonConnected bumps uid < SYSTEM_UID to SYSTEM_UID and defers.
 */
HWTEST_F(AudioRollbackManagerTest, HandleNonConnected_LowUid_BumpedToSystem, TestSize.Level1)
{
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:11", VIRTUAL_CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();
    EXPECT_TRUE(rollback.HandleNonConnected(0, SelectDeviceType::MEDIA_OUTPUT, INVALID_STREAM_ID, newDev));

    std::lock_guard<std::mutex> lg(rollback.mutex_);
    ASSERT_FALSE(rollback.continueSelections_.empty());
    EXPECT_EQ(rollback.continueSelections_.back().uid, SYSTEM_UID);
    EXPECT_EQ(rollback.continueSelections_.back().deviceId, newDev->deviceId_);
}

/**
 * @tc.name  : HandleNonConnected_ExistingSameDevice_NoDuplicate
 * @tc.type  : FUNC
 * @tc.desc  : HandleNonConnected returns true and adds no duplicate when same deviceId already pending.
 */
HWTEST_F(AudioRollbackManagerTest, HandleNonConnected_ExistingSameDevice_NoDuplicate, TestSize.Level1)
{
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:12", VIRTUAL_CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();
    EXPECT_TRUE(rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, newDev));
    size_t countBefore = 0;
    {
        std::lock_guard<std::mutex> lg(rollback.mutex_);
        countBefore = rollback.continueSelections_.size();
    }
    EXPECT_TRUE(rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, newDev));
    {
        std::lock_guard<std::mutex> lg(rollback.mutex_);
        EXPECT_EQ(rollback.continueSelections_.size(), countBefore);
    }
}

/**
 * @tc.name  : HandleNonConnected_ExistingDifferentDevice_Updated
 * @tc.type  : FUNC
 * @tc.desc  : HandleNonConnected updates existing pending selection to new deviceId.
 */
HWTEST_F(AudioRollbackManagerTest, HandleNonConnected_ExistingDifferentDevice_Updated, TestSize.Level1)
{
    auto devA = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:13", VIRTUAL_CONNECTED);
    auto devB = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:14", VIRTUAL_CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();
    EXPECT_TRUE(rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, devA));
    EXPECT_TRUE(rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_OUTPUT,
        INVALID_STREAM_ID, devB));
    {
        std::lock_guard<std::mutex> lg(rollback.mutex_);
        ASSERT_EQ(rollback.continueSelections_.size(), 1);
        EXPECT_EQ(rollback.continueSelections_.back().deviceId, devB->deviceId_);
    }
}

/**
 * @tc.name  : ContinueSelection_InvalidAndNoMatch
 * @tc.type  : FUNC
 * @tc.desc  : ContinueSelection returns early for null, non-connected device and no matching pending.
 */
HWTEST_F(AudioRollbackManagerTest, ContinueSelection_InvalidAndNoMatch, TestSize.Level1)
{
    auto &rollback = AudioRollbackManager::GetInstance();
    EXPECT_NO_THROW(rollback.ContinueSelection(nullptr));
    auto nonConn = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:15", VIRTUAL_CONNECTED);
    EXPECT_NO_THROW(rollback.ContinueSelection(nonConn));
    auto noMatch = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:16", CONNECTED);
    EXPECT_NO_THROW(rollback.ContinueSelection(noMatch));
}

/**
 * @tc.name  : ContinueSelection_ApplyInputAndCallTypes
 * @tc.type  : FUNC
 * @tc.desc  : ContinueSelection applies pending selections for MEDIA_INPUT, CALL_INPUT and CALL_OUTPUT.
 */
HWTEST_F(AudioRollbackManagerTest, ContinueSelection_ApplyInputAndCallTypes, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:20");
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev, "ut");
    auto &rollback = AudioRollbackManager::GetInstance();

    auto d1 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:21", VIRTUAL_CONNECTED);
    rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::MEDIA_INPUT, INVALID_STREAM_ID, d1);
    d1->connectState_ = CONNECTED;
    rollback.ContinueSelection(d1);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_INPUT), d1->deviceId_);

    auto d2 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:22", VIRTUAL_CONNECTED);
    rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::CALL_INPUT, INVALID_STREAM_ID, d2);
    d2->connectState_ = CONNECTED;
    rollback.ContinueSelection(d2);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_INPUT), d2->deviceId_);

    auto d3 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:23", VIRTUAL_CONNECTED);
    rollback.HandleNonConnected(SYSTEM_UID, SelectDeviceType::CALL_OUTPUT, INVALID_STREAM_ID, d3);
    d3->connectState_ = CONNECTED;
    rollback.ContinueSelection(d3);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), d3->deviceId_);
}

/**
 * @tc.name  : SetDevice_Wrappers_DeferredForUnconnected
 * @tc.type  : FUNC
 * @tc.desc  : Set{Media,Call}{Input,Output}Device defer when device is not connected.
 */
HWTEST_F(AudioRollbackManagerTest, SetDevice_Wrappers_DeferredForUnconnected, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:30");
    SetOutputBaseline(oldDev);
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev);
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, oldDev, "ut");
    auto unDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:30", VIRTUAL_CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), oldDev->deviceId_);
    rollback.SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_INPUT), oldDev->deviceId_);
    rollback.SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_INPUT), oldDev->deviceId_);
    rollback.SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, unDev, "ut");
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), oldDev->deviceId_);
}

/**
 * @tc.name  : SetDevice_Wrappers_AppliedForConnected
 * @tc.type  : FUNC
 * @tc.desc  : Set{Media,Call}{Input,Output}Device apply when device is connected.
 */
HWTEST_F(AudioRollbackManagerTest, SetDevice_Wrappers_AppliedForConnected, TestSize.Level1)
{
    auto connDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:31", CONNECTED);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_INPUT), connDev->deviceId_);
    rollback.SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_INPUT), connDev->deviceId_);
    rollback.SetCallOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, connDev, "ut");
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::CALL_OUTPUT), connDev->deviceId_);
}

/**
 * @tc.name  : SetNewConnectRollback_InvalidAndReuse
 * @tc.type  : FUNC
 * @tc.desc  : SetNewConnectRollback ignores null/connected device and reuses existing rollback info.
 */
HWTEST_F(AudioRollbackManagerTest, SetNewConnectRollback_InvalidAndReuse, TestSize.Level1)
{
    auto &rollback = AudioRollbackManager::GetInstance();
    rollback.SetNewConnectRollback(nullptr);
    EXPECT_FALSE(HasRollbackInfo());
    auto connDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:40", CONNECTED);
    rollback.SetNewConnectRollback(connDev);
    EXPECT_FALSE(HasRollbackInfo());

    auto dev1 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:41", VIRTUAL_CONNECTED);
    rollback.SetNewConnectRollback(dev1);
    EXPECT_TRUE(HasRollbackInfo());
    EXPECT_EQ(RollbackDeviceId(), dev1->deviceId_);

    auto dev2 = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:42", VIRTUAL_CONNECTED);
    rollback.SetNewConnectRollback(dev2);
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
 * @tc.name  : RollbackSelection_MismatchAndSkipNotConnected
 * @tc.type  : FUNC
 * @tc.desc  : RollbackSelection returns on mismatch and skips snapshot items not connected.
 */
HWTEST_F(AudioRollbackManagerTest, RollbackSelection_MismatchAndSkipNotConnected, TestSize.Level1)
{
    auto oldDev = AddConnectedDevice(DEVICE_TYPE_SPEAKER, "00:00:00:00:0A:60");
    auto newDev = MakeLocalDevice(DEVICE_TYPE_BLUETOOTH_A2DP, "00:00:00:00:0B:60", VIRTUAL_CONNECTED);
    SetOutputBaseline(oldDev);
    auto &rollback = AudioRollbackManager::GetInstance();

    rollback.SetNewConnectRollback(newDev);
    EXPECT_EQ(RollbackDeviceId(), newDev->deviceId_);
    rollback.RollbackSelection(oldDev->deviceId_, NEW_CONNECT_ROLLBACK);
    EXPECT_TRUE(HasRollbackInfo());
    rollback.RollbackSelection(newDev->deviceId_, CONNECT_LOST_ROLLBACK);
    EXPECT_TRUE(HasRollbackInfo());

    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, newDev);
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
    oldDev->connectState_ = VIRTUAL_CONNECTED;
    std::this_thread::sleep_for(std::chrono::milliseconds(MS_OVER_2SEC));
    EXPECT_FALSE(HasRollbackInfo());
    EXPECT_EQ(GetSystemDeviceId(SelectDeviceType::MEDIA_OUTPUT), newDev->deviceId_);
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
        rb.continueSelections_.clear();
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

}
}
