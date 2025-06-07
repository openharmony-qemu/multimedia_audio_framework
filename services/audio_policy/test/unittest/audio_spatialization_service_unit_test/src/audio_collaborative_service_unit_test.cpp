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
#include <string.h>
#include "audio_collaborative_service_unit_test.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
void AudioCollaborativeServiceUnitTest::SetUpTestCase(void){}
void AudioCollaborativeServiceUnitTest::TearDownTestCase(void) {}
void AudioCollaborativeServiceUnitTest::SetUp(void) {}
void AudioCollaborativeServiceUnitTest::TearDown(void) {}
static std::string testAddr1 = "address1";
static std::string testAddr2 = "address2";
static std::string testAddr3 = "address3";
/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_001
* @tc.desc  : Test isCollaborativePlaybackSupported.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_001, TestSize.Level0)
{
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupported();
    EXPECT_EQ(isSupported, true);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_002
* @tc.desc  : Test SetCollaborativePlaybackEnabledForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_002, TestSize.Level0)
{
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    audioDevice1->macAddress_ = testAddr1;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, false);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, false);
    EXPECT_EQ(ret, SUCCESS);
}


/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_003
* @tc.desc  : Test IsCollaborativePlaybackEnabledForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_003, TestSize.Level0)
{
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice2 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice3 = std::make_shared<AudioDeviceDescriptor>();
    AudioDevice1->macAddress_ = testAddr1;
    AudioDevice2->macAddress_ = testAddr2;
    AudioDevice3->macAddress_ = testAddr3;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice2, false);
    EXPECT_EQ(ret, SUCCESS);
    bool isEnabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(AudioDevice1);
    EXPECT_EQ(isEnabled, true);
    isEnabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(AudioDevice2);
    EXPECT_EQ(isEnabled, false);
    isEnabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(AudioDevice3);
    EXPECT_EQ(isEnabled, false);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_004
* @tc.desc  : Test UpdateCurrentDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_004, TestSize.Level0)
{
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice2 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice3 = std::make_shared<AudioDeviceDescriptor>();
    AudioDevice1->macAddress_ = testAddr1;
    AudioDevice1->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDevice2->macAddress_ = testAddr2;
    AudioDevice2->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDevice3->macAddress_ = testAddr3;
    AudioDevice3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice2, false);
    EXPECT_EQ(ret, SUCCESS);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice1);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice2);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice3);
}

} // AudioStandard
} // OHOS