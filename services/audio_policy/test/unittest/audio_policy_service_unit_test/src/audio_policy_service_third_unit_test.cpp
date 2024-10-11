/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_policy_service_third_unit_test.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_isInit = false;
static const std::string PIPE_PRIMARY_OUTPUT_UNITTEST = "primary_output";
static const std::string PIPE_PRIMARY_INPUT_UNITTEST = "primary_input";
static const std::string PIPE_USB_ARM_OUTPUT_UNITTEST = "usb_arm_output";
static const std::string PIPE_DP_OUTPUT_UNITTEST = "dp_output";
static const std::string PIPE_USB_ARM_INPUT_UNITTEST = "usb_arm_input";

static AudioPolicyServer g_server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
void AudioPolicyServiceThirdUnitTest::SetUpTestCase(void) {}
void AudioPolicyServiceThirdUnitTest::TearDownTestCase(void) {}
void AudioPolicyServiceThirdUnitTest::SetUp(void) {}
void AudioPolicyServiceThirdUnitTest::TearDown(void)
{
    g_server.OnStop();
    g_isInit = false;
}

AudioPolicyServer *GetServerPtr()
{
    if (!g_isInit) {
        g_server.OnStart();
        g_server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        g_server.OnAddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "");
        g_server.OnAddSystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID, "");
        g_server.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
        g_server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        g_server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        g_server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        g_server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_isInit = true;
    }
    return &g_server;
}

/**
* @tc.name  : Test OnDeviceInfoUpdated.
* @tc.number: OnDeviceInfoUpdated_001
* @tc.desc  : Test OnDeviceInfoUpdated interfaces.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnDeviceInfoUpdated_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->isEnable_ = true;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;

    GetServerPtr()->audioPolicyService_.OnDeviceInfoUpdated(*desc, command);
}

/**
* @tc.name  : Test OnDeviceInfoUpdated.
* @tc.number: OnDeviceInfoUpdated_002
* @tc.desc  : Test OnDeviceInfoUpdated interfaces.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnDeviceInfoUpdated_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->isEnable_ = true;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;

    GetServerPtr()->audioPolicyService_.OnDeviceInfoUpdated(*desc, command);
}

/**
* @tc.name  : Test OnDeviceInfoUpdated.
* @tc.number: OnDeviceInfoUpdated_003
* @tc.desc  : Test OnDeviceInfoUpdated interfaces.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnDeviceInfoUpdated_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->isEnable_ = false;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;

    GetServerPtr()->audioPolicyService_.OnDeviceInfoUpdated(*desc, command);
}
/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_001
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    desc->isEnable_ = true;

    GetServerPtr()->audioPolicyService_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_002
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;

    desc->connectState_ = DEACTIVE_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = true;

    GetServerPtr()->audioPolicyService_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_003
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;

    desc->connectState_ = VIRTUAL_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = false;

    GetServerPtr()->audioPolicyService_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_004
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_004, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    desc->connectState_ = VIRTUAL_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = false;

    GetServerPtr()->audioPolicyService_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_005
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_005, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.connectedDevices_.clear();
    // dummy data
    sptr<AudioDeviceDescriptor> desc = new(std::nothrow) AudioDeviceDescriptor();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    desc->connectState_ = VIRTUAL_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = true;

    GetServerPtr()->audioPolicyService_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_001
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();

    std::string dumpString = "666";
    server->audioPolicyService_.GetOffloadStatusDump(dumpString);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_002
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_002, TestSize.Level1)
{
    auto server = GetServerPtr();

    std::string dumpString = "666";
    server->audioPolicyService_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.GetOffloadStatusDump(dumpString);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_003
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_003, TestSize.Level1)
{
    auto server = GetServerPtr();

    std::string dumpString = "666";
    server->audioPolicyService_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    server->audioPolicyService_.GetOffloadStatusDump(dumpString);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_004
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_004, TestSize.Level1)
{
    auto server = GetServerPtr();

    std::string dumpString = "666";
    server->audioPolicyService_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.GetOffloadStatusDump(dumpString);
}
} // namespace AudioStandard
} // namespace OHOS
