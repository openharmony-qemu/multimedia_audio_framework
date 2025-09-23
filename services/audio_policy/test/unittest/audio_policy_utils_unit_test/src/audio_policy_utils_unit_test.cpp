/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_policy_utils_unit_test.h"
#include "audio_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyUtilsUnitTest::SetUp(void) {}

void AudioPolicyUtilsUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test ErasePreferredDeviceByType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_001
 * @tc.desc  : Test ErasePreferredDeviceByType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_001, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    PreferredType preferredType = AUDIO_MEDIA_RENDER;
    audioPolicyUtilsTest_->isBTReconnecting_ = false;

    int32_t ret = audioPolicyUtilsTest_->ErasePreferredDeviceByType(preferredType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetNewSinkPortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_002
 * @tc.desc  : Test GetNewSinkPortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_002, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_ACCESSORY;

    std::string ret = audioPolicyUtilsTest_->GetNewSinkPortName(deviceType);
    EXPECT_EQ(ret, ACCESSORY_SOURCE);
}

/**
 * @tc.name  : Test GetNewSinkPortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_003
 * @tc.desc  : Test GetNewSinkPortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_003, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_HEARING_AID;

    std::string ret = audioPolicyUtilsTest_->GetNewSinkPortName(deviceType);
    EXPECT_EQ(ret, HEARING_AID_SPEAKER);
}

/**
 * @tc.name  : Test GetSinkName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_004
 * @tc.desc  : Test GetSinkName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_004, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    int32_t descriptorType = 0;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(descriptorType);;
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    int32_t sessionId = 0;

    std::string ret = audioPolicyUtilsTest_->GetSinkName(desc, sessionId);
    EXPECT_EQ(ret, PRIMARY_SPEAKER);
}

/**
 * @tc.name  : Test GetSinkName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_005
 * @tc.desc  : Test GetSinkName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_005, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    int32_t descriptorType = 0;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(descriptorType);;
    desc->networkId_ = "";
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t sessionId = 0;
    std::string test_result = "_out";

    std::string ret = audioPolicyUtilsTest_->GetSinkName(desc, sessionId);
    EXPECT_EQ(ret, test_result);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_006
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_006, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType);
    EXPECT_EQ(ret, BLUETOOTH_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_ACCESSORY;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType);
    EXPECT_EQ(ret, ACCESSORY_SOURCE);
}

/**
 * @tc.name  : Test GetOutputDeviceClassBySinkPortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_008
 * @tc.desc  : Test GetOutputDeviceClassBySinkPortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_008, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sinkPortName = BLUETOOTH_SPEAKER;

    std::string ret = audioPolicyUtilsTest_->GetOutputDeviceClassBySinkPortName(sinkPortName);
    EXPECT_EQ(ret, A2DP_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_009
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_009, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = BLUETOOTH_MIC;

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, A2DP_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = "test";

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, INVALID_CLASS);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_011
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_011, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "Built_in_wakeup";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_WAKEUP);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_012
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_012, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "fifo_output";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_013
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_013, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "fifo_input";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_014
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_014, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "file_sink";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_FILE_SINK);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_015
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_015, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "test";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test UnexcludeOutputDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_016
 * @tc.desc  : Test UnexcludeOutputDevices
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_016, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioPolicyUtilsTest_->isBTReconnecting_ = true;

    int32_t ret = audioPolicyUtilsTest_->UnexcludeOutputDevices(descs);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsSupportedNearlink API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_017
 * @tc.desc  : Test IsSupportedNearlink with system permission
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_017, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string bundleName = "com.ohos.test";
    bool hasSystemPermission = true;
    int32_t apiVersion = 0;

    int32_t ret = audioPolicyUtilsTest_->IsSupportedNearlink(bundleName, apiVersion, hasSystemPermission);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsSupportedNearlink API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_018
 * @tc.desc  : Test IsSupportedNearlink without system permission.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_018, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string bundleName = "com.ohos.test";
    bool hasSystemPermission = false;
    int32_t apiVersion = 0;

    int32_t ret = audioPolicyUtilsTest_->IsSupportedNearlink(bundleName, apiVersion, hasSystemPermission);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsSupportedNearlink API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_019
 * @tc.desc  : Test IsSupportedNearlink without system permission with 20 api.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_019, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string bundleName = "com.ohos.test";
    bool hasSystemPermission = false;
    int32_t apiVersion = 20;

    int32_t ret = audioPolicyUtilsTest_->IsSupportedNearlink(bundleName, apiVersion, hasSystemPermission);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsWirelessDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_020
 * @tc.desc  : Test IsWirelessDevice.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_020, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType type = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_NEARLINK;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_NEARLINK_IN;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_SPEAKER;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioPolicyUtils
 * @tc.number: ClearScoDeviceSuspendState_001
 * @tc.desc  : Test ClearScoDeviceSuspendState
 */
HWTEST(AudioPolicyUtilsUnitTest, ClearScoDeviceSuspendState_001, TestSize.Level1)
{
    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    string macAddress1 = "sdfs1";
    string macAddress2 = "sdfs2";
    AudioDeviceDescriptor desc;
    desc.deviceId_ = 114514;
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.macAddress_ = macAddress1;
    desc.networkId_ = LOCAL_NETWORK_ID;
    devMan.AddNewDevice(make_shared<AudioDeviceDescriptor>(desc));
    desc.deviceId_ = 114515;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = macAddress2;
    devMan.AddNewDevice(make_shared<AudioDeviceDescriptor>(desc));
    auto &utils = AudioPolicyUtils::GetInstance();
    utils.ClearScoDeviceSuspendState(macAddress1);
    utils.ClearScoDeviceSuspendState(macAddress2);
    EXPECT_EQ(devMan.ExistsByType(DEVICE_TYPE_NEARLINK), true);
}

} // namespace AudioStandard
} // namespace OHOS
