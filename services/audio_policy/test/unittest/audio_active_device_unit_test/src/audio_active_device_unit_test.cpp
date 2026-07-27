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

#include "audio_policy_utils.h"
#include "audio_active_device_unit_test.h"
#include "audio_router_select_strategy.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioActiveDeviceUnitTest::SetUpTestCase(void) {}
void AudioActiveDeviceUnitTest::TearDownTestCase(void) {}
void AudioActiveDeviceUnitTest::SetUp(void) {}
void AudioActiveDeviceUnitTest::TearDown(void) {}

constexpr int32_t TEST_DEVICE_ID = 1001;

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_001.
* @tc.desc  : Test GetActiveA2dpDeviceStreamInfo.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    bool result = audioActiveDevice->GetActiveA2dpDeviceStreamInfo(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN,
        streamInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_002.
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});

    int32_t deviceId = AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice().deviceId_;
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(deviceId, audioInterrupt);
    EXPECT_FLOAT_EQ(result, 0.0f);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_011.
* @tc.desc  : Test IsDirectSupportedDevice.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_011, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    bool result = audioActiveDevice->IsDirectSupportedDevice();
    EXPECT_EQ(result, true);

    auto desc1 = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc1->deviceId_ = 1002;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc1);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc1});
    result = audioActiveDevice->IsDirectSupportedDevice();
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_012.
* @tc.desc  : Test IsDeviceActive.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_012, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    bool result = audioActiveDevice->IsDeviceActive(DeviceType::DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(result, true);

    result = audioActiveDevice->IsDeviceActive(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: GetCurrentInputDeviceMacAddr_001.
* @tc.desc  : Test GetCurrentInputDeviceMacAddr.
*/
HWTEST_F(AudioActiveDeviceUnitTest, GetCurrentInputDeviceMacAddr_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->deviceId_ = TEST_DEVICE_ID;
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    string ret = audioActiveDevice->GetCurrentInputDeviceMacAddr();
    EXPECT_EQ(ret, "00:11:22:33:44:55");
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: GetCurrentOutputDeviceCategory_001.
* @tc.desc  : Test GetCurrentOutputDeviceCategory.
*/
HWTEST_F(AudioActiveDeviceUnitTest, GetCurrentOutputDeviceCategory_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();

    DeviceCategory ret = audioActiveDevice->GetCurrentOutputDeviceCategory();
    EXPECT_EQ(ret, CATEGORY_DEFAULT);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: WriteOutputRouteChangeEvent_001.
* @tc.desc  : Test WriteOutputRouteChangeEvent.
*/
HWTEST_F(AudioActiveDeviceUnitTest, WriteOutputRouteChangeEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();

    audioActiveDevice->WriteOutputRouteChangeEvent(deviceDescriptor, reason);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}
/**
 * @tc.name  : GetDeviceForVolumeEnhanced_InvalidUid
 * @tc.number: GetDeviceForVolumeEnhanced_001
 * @tc.desc  : Test if(appUid==INVALID_UID) true branch, appUid replaced by callingUid
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetDeviceForVolumeEnhanced_InvalidUid, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    auto result = audioActiveDevice.GetDeviceForVolumeEnhanced(INVALID_UID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : GetDeviceForVolumeEnhanced_ValidUid
 * @tc.number: GetDeviceForVolumeEnhanced_002
 * @tc.desc  : Test if(appUid==INVALID_UID) false branch, appUid stays as passed value
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetDeviceForVolumeEnhanced_ValidUid, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    auto result = audioActiveDevice.GetDeviceForVolumeEnhanced(SYSTEM_UID);
    EXPECT_NE(result, nullptr);
}
/**
 * @tc.name  : GetUidVolumeDevicesForEnhancedRouting_UidNotFound
 * @tc.number: GetUidVolumeDevicesForEnhancedRouting_001
 * @tc.desc  : Test if condition true branch, uid not in uidVolumeDeviceMap_ returns empty vector
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetUidVolumeDevicesForEnhancedRouting_UidNotFound, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.uidVolumeDeviceMap_.clear();

    auto result = audioActiveDevice.GetUidVolumeDevicesForEnhancedRouting(9999);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : GetUidVolumeDevicesForEnhancedRouting_UidFound
 * @tc.number: GetUidVolumeDevicesForEnhancedRouting_002
 * @tc.desc  : Test if condition false branch, uid found in uidVolumeDeviceMap_ returns devices
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetUidVolumeDevicesForEnhancedRouting_UidFound, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.uidVolumeDeviceMap_.clear();
    int32_t uid = 1000;
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    audioActiveDevice.uidVolumeDeviceMap_[uid] = {desc};

    auto result = audioActiveDevice.GetUidVolumeDevicesForEnhancedRouting(uid);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DeviceType::DEVICE_TYPE_USB_HEADSET);

    audioActiveDevice.uidVolumeDeviceMap_.clear();
}
} // namespace AudioStandard
} // namespace OHOS
