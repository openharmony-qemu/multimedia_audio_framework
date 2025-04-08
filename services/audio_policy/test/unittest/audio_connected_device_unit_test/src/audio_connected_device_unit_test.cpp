/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "audio_device_descriptor.h"
#include "audio_connected_device_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioConnectedDeviceUnitTest::SetUpTestCase(void) {}
void AudioConnectedDeviceUnitTest::TearDownTestCase(void) {}
void AudioConnectedDeviceUnitTest::SetUp(void) {}
void AudioConnectedDeviceUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_001.
* @tc.desc  : Test IsConnectedOutputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_001, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_FILE_SINK,
        DeviceRole::OUTPUT_DEVICE);
    AudioConnectedDevice::GetInstance().connectedDevices_.push_back(desc);
    bool result = AudioConnectedDevice::GetInstance().IsConnectedOutputDevice(desc);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_002.
* @tc.desc  : Test IsConnectedOutputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_002, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_EARPIECE,
        DeviceRole::OUTPUT_DEVICE);
    AudioConnectedDevice::GetInstance().connectedDevices_.push_back(desc);
    bool result = AudioConnectedDevice::GetInstance().IsConnectedOutputDevice(desc);
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_003.
* @tc.desc  : Test CheckExistOutputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_003, TestSize.Level1)
{
    std::string macAddress = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_EARPIECE,
        DeviceRole::OUTPUT_DEVICE);
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->CheckExistOutputDevice(DeviceType::DEVICE_TYPE_EARPIECE, macAddress);
    EXPECT_NE(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_004.
* @tc.desc  : Test CheckExistOutputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_004, TestSize.Level1)
{
    std::string macAddress = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = macAddress;
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->CheckExistOutputDevice(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, macAddress);
    EXPECT_NE(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_005.
* @tc.desc  : Test CheckExistOutputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_005, TestSize.Level1)
{
    std::string macAddress = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::INPUT_DEVICE);
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->CheckExistOutputDevice(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, macAddress);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->CheckExistOutputDevice(DeviceType::DEVICE_TYPE_DP, macAddress);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_006.
* @tc.desc  : Test CheckExistOutputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_006, TestSize.Level1)
{
    std::string macAddress = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::OUTPUT_DEVICE);
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->CheckExistOutputDevice(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, macAddress);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_007.
* @tc.desc  : Test CheckExistInputDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_007, TestSize.Level1)
{
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
        DeviceRole::INPUT_DEVICE);
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->CheckExistInputDevice(DeviceType::DEVICE_TYPE_DP);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->CheckExistInputDevice(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_NE(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_008.
* @tc.desc  : Test GetConnectedDeviceByType.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_008, TestSize.Level1)
{
    std::string networkId = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = networkId;
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_DP);
    EXPECT_NE(result, nullptr);

    result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_009.
* @tc.desc  : Test GetConnectedDeviceByType.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_009, TestSize.Level1)
{
    std::string networkId = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "networkId";
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_DP);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_010.
* @tc.desc  : Test GetConnectedDeviceByType.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_010, TestSize.Level1)
{
    std::string networkId = "test";
    std::string macAddress = "macAddress";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "networkId";
    desc->macAddress_ = macAddress;
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_DP, macAddress);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_EARPIECE, macAddress);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_DP, macAddress);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_011.
* @tc.desc  : Test GetConnectedDeviceByType.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_011, TestSize.Level1)
{
    std::string networkId = "test";
    std::string macAddress = "macAddress";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = networkId;
    desc->macAddress_ = "test1";
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_DP, macAddress);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_EARPIECE, macAddress);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_012.
* @tc.desc  : Test GetConnectedDeviceByType.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_012, TestSize.Level1)
{
    std::string networkId = "test";
    std::string macAddress = "macAddress";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "networkId";
    desc->macAddress_ = "test1";
    audioConnectedDevice->connectedDevices_.push_back(desc);
    auto result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_DP, macAddress);
    EXPECT_EQ(result, nullptr);

    result = audioConnectedDevice->GetConnectedDeviceByType(networkId, DeviceType::DEVICE_TYPE_EARPIECE,
        macAddress);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_013.
* @tc.desc  : Test UpdateConnectDevice.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_013, TestSize.Level1)
{
    std::string macAddress = "macAddress";
    std::string deviceName = "deviceName";
    AudioStreamInfo streamInfo;
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    audioConnectedDevice->UpdateConnectDevice(DeviceType::DEVICE_TYPE_DP, macAddress, deviceName, streamInfo);
    EXPECT_NE(audioConnectedDevice, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = macAddress;
    audioConnectedDevice->connectedDevices_.push_back(desc);
    audioConnectedDevice->UpdateConnectDevice(DeviceType::DEVICE_TYPE_DP, macAddress, deviceName, streamInfo);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_014.
* @tc.desc  : Test GetUsbDeviceDescriptor.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_014, TestSize.Level1)
{
    std::string address = "test";
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    auto result = audioConnectedDevice->GetUsbDeviceDescriptor(address, DeviceRole::INPUT_DEVICE);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_015.
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_015, TestSize.Level1)
{
    std::string macAddress = "test";
    bool spatializationSupported = true;
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    std::string encryAddress =
        AudioSpatializationService::GetAudioSpatializationService().GetSha256EncryptAddress(macAddress);

    auto desc1 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::OUTPUT_DEVICE);
    desc1->macAddress_ = macAddress;
    desc1->spatializationSupported_ = spatializationSupported;
    audioConnectedDevice->connectedDevices_.push_back(desc1);

    auto desc2 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::OUTPUT_DEVICE);
    desc2->macAddress_ = macAddress;
    audioConnectedDevice->connectedDevices_.push_back(desc2);

    auto desc3 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
        DeviceRole::OUTPUT_DEVICE);
    desc3->macAddress_ = macAddress;
    desc3->spatializationSupported_ = spatializationSupported;
    audioConnectedDevice->connectedDevices_.push_back(desc3);

    auto desc4 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
        DeviceRole::OUTPUT_DEVICE);
    desc4->macAddress_ = macAddress;
    audioConnectedDevice->connectedDevices_.push_back(desc4);

    audioConnectedDevice->UpdateSpatializationSupported(encryAddress, spatializationSupported);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_016.
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_016, TestSize.Level1)
{
    std::string macAddress = "test";
    bool spatializationSupported = true;
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    std::string encryAddress =
        AudioSpatializationService::GetAudioSpatializationService().GetSha256EncryptAddress(macAddress);

    auto desc1 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::OUTPUT_DEVICE);
    desc1->macAddress_ = "test1";
    desc1->spatializationSupported_ = spatializationSupported;
    audioConnectedDevice->connectedDevices_.push_back(desc1);

    auto desc2 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
        DeviceRole::OUTPUT_DEVICE);
    desc2->macAddress_ = "test1";
    audioConnectedDevice->connectedDevices_.push_back(desc2);

    auto desc3 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
        DeviceRole::OUTPUT_DEVICE);
    desc3->macAddress_ = "test1";
    desc3->spatializationSupported_ = spatializationSupported;
    audioConnectedDevice->connectedDevices_.push_back(desc3);

    auto desc4 = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
        DeviceRole::OUTPUT_DEVICE);
    desc4->macAddress_ = "test1";
    audioConnectedDevice->connectedDevices_.push_back(desc4);

    audioConnectedDevice->UpdateSpatializationSupported(encryAddress, spatializationSupported);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_017
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_017, TestSize.Level1)
{
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    EXPECT_NE(audioConnectedDevice, nullptr);

    std::string networkId = "abc";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string macAddress = "test";
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->networkId_ = "abc";
    desc->macAddress_ = "test";
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioConnectedDevice->connectedDevices_.push_back(desc);

    audioConnectedDevice->GetAllConnectedDeviceByType(networkId, deviceType, macAddress, deviceRole, descForCb);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_018
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_018, TestSize.Level1)
{
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    EXPECT_NE(audioConnectedDevice, nullptr);

    std::string networkId = "abc";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "test";
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceType::DEVICE_TYPE_USB_HEADSET;
    desc->networkId_ = "abc";
    desc->macAddress_ = "test";
    desc->deviceRole_ = DeviceRole::DEVICE_ROLE_MAX;
    audioConnectedDevice->connectedDevices_.push_back(desc);

    audioConnectedDevice->GetAllConnectedDeviceByType(networkId, deviceType, macAddress, deviceRole, descForCb);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_019
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_019, TestSize.Level1)
{
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    EXPECT_NE(audioConnectedDevice, nullptr);

    std::string networkId = "abc";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "test";
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceType::DEVICE_TYPE_USB_HEADSET;
    desc->networkId_ = "abc";
    desc->macAddress_ = "123";
    desc->deviceRole_ = DeviceRole::DEVICE_ROLE_MAX;
    audioConnectedDevice->connectedDevices_.push_back(desc);

    audioConnectedDevice->GetAllConnectedDeviceByType(networkId, deviceType, macAddress, deviceRole, descForCb);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_020
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_020, TestSize.Level1)
{
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    EXPECT_NE(audioConnectedDevice, nullptr);

    std::string networkId = "abc";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "test";
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceType::DEVICE_TYPE_USB_HEADSET;
    desc->networkId_ = "456";
    desc->macAddress_ = "123";
    desc->deviceRole_ = DeviceRole::DEVICE_ROLE_MAX;
    audioConnectedDevice->connectedDevices_.push_back(desc);

    audioConnectedDevice->GetAllConnectedDeviceByType(networkId, deviceType, macAddress, deviceRole, descForCb);
    EXPECT_NE(audioConnectedDevice, nullptr);
}

/**
* @tc.name  : Test AudioConnectedDevice.
* @tc.number: AudioConnectedDeviceUnitTest_021
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioConnectedDeviceUnitTest, AudioConnectedDeviceUnitTest_021, TestSize.Level1)
{
    auto audioConnectedDevice = std::make_shared<AudioConnectedDevice>();
    EXPECT_NE(audioConnectedDevice, nullptr);

    std::string networkId = "abc";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string macAddress = "test";
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceType::DEVICE_TYPE_USB_HEADSET;
    desc->networkId_ = "456";
    desc->macAddress_ = "123";
    desc->deviceRole_ = DeviceRole::DEVICE_ROLE_MAX;
    audioConnectedDevice->connectedDevices_.push_back(desc);

    audioConnectedDevice->GetAllConnectedDeviceByType(networkId, deviceType, macAddress, deviceRole, descForCb);
    EXPECT_NE(audioConnectedDevice, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS