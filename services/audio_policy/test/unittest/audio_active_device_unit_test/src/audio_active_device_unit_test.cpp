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

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioActiveDeviceUnitTest::SetUpTestCase(void) {}
void AudioActiveDeviceUnitTest::TearDownTestCase(void) {}
void AudioActiveDeviceUnitTest::SetUp(void) {}
void AudioActiveDeviceUnitTest::TearDown(void) {}

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
    int32_t deviceId = AudioActiveDevice::GetInstance().GetCurrentInputDevice().deviceId_;
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(deviceId, audioInterrupt);
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_003.
* @tc.desc  : Test UpdateDevice.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_003, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NONE);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;

    audioActiveDevice->UpdateDevice(desc, reason, rendererChangeInfo);
    EXPECT_NE(rendererChangeInfo, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_004.
* @tc.desc  : Test UpdateDevice.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_004, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NONE);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;

    audioActiveDevice->UpdateDevice(desc, reason, rendererChangeInfo);
    EXPECT_NE(rendererChangeInfo, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_005.
* @tc.desc  : Test HandleActiveBt.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_005, TestSize.Level1)
{
    std::string macAddress = "test";
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleActiveBt(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, macAddress);
    EXPECT_NE(audioActiveDevice, nullptr);

    audioActiveDevice->HandleActiveBt(DeviceType::DEVICE_TYPE_EARPIECE, macAddress);
    EXPECT_NE(audioActiveDevice, nullptr);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_EARPIECE;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleActiveBt(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, macAddress);
    EXPECT_NE(audioActiveDevice, nullptr);

    audioActiveDevice->HandleActiveBt(DeviceType::DEVICE_TYPE_EARPIECE, macAddress);
    EXPECT_NE(audioActiveDevice, nullptr);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleActiveBt(DeviceType::DEVICE_TYPE_EARPIECE, macAddress);
    EXPECT_NE(audioActiveDevice, nullptr);

    audioActiveDevice->HandleActiveBt(DeviceType::DEVICE_TYPE_NEARLINK, macAddress);
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_006.
* @tc.desc  : Test HandleNegtiveBt.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_006, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_NE(audioActiveDevice, nullptr);

    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_EARPIECE);
    EXPECT_NE(audioActiveDevice, nullptr);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_EARPIECE;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_NE(audioActiveDevice, nullptr);

    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_EARPIECE);
    EXPECT_NE(audioActiveDevice, nullptr);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_NEARLINK);
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_007.
* @tc.desc  : Test HandleNegtiveBt.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_007, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    for (const auto &desc : callDevices) {
        int32_t result = audioActiveDevice->SetDeviceActive(DeviceType::DEVICE_TYPE_FILE_SINK, true);
        EXPECT_EQ(result, SUCCESS);
    }

    for (const auto &desc : callDevices) {
        int32_t result = audioActiveDevice->SetDeviceActive(DeviceType::DEVICE_TYPE_FILE_SINK, false);
        EXPECT_EQ(result, SUCCESS);
    }
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_008.
* @tc.desc  : Test SetCallDeviceActive.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_008, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    for (const auto &desc : callDevices) {
        int32_t result = audioActiveDevice->SetCallDeviceActive(desc->deviceType_, true, desc->macAddress_);
    }
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_009.
* @tc.desc  : Test SetCallDeviceActive.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_009, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    for (const auto &desc : callDevices) {
        audioActiveDevice->SetCallDeviceActive(desc->deviceType_, false, desc->macAddress_);
    }
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_011.
* @tc.desc  : Test IsDirectSupportedDevice.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_011, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    AudioDeviceDescriptor audioDeviceDescriptor(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    audioActiveDevice->SetCurrentOutputDevice(audioDeviceDescriptor);
    bool result = audioActiveDevice->IsDirectSupportedDevice();
    EXPECT_EQ(result, true);

    AudioDeviceDescriptor audioDeviceDescriptor1(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    audioActiveDevice->SetCurrentOutputDevice(audioDeviceDescriptor1);
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
    AudioDeviceDescriptor audioDeviceDescriptor(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    audioActiveDevice->SetCurrentOutputDevice(audioDeviceDescriptor);
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

    audioActiveDevice->currentActiveDevice_.macAddress_ = "00:11:22:33:44:55";
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
} // namespace AudioStandard
} // namespace OHOS
