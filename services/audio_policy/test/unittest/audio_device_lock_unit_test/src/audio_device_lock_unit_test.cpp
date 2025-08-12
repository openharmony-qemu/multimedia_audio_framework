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

#include "audio_device_lock_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static int32_t NUM_5 = 5;
static int32_t NUM_20 = 20;

void AudioDeviceLockUnitTest::SetUpTestCase(void) {}
void AudioDeviceLockUnitTest::TearDownTestCase(void) {}
void AudioDeviceLockUnitTest::SetUp(void) {}
void AudioDeviceLockUnitTest::TearDown(void) {}

/**
* @tc.name  :  AudioDeviceLock_020
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_020, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = true;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_021
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_021, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_022
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_022, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_023
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_023, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_024
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_024, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = true;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_025
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_025, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_026
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_026, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_027
* @tc.desc  : Test AudioDeviceLock interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_027, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    DeviceInfoUpdateCommand command = DeviceInfoUpdateCommand::ENABLE_UPDATE;

    EXPECT_NO_THROW(audioDeviceLock->OnDeviceInfoUpdated(updatedDesc, command));
}
} // namespace AudioStandard
} // namespace OHOS