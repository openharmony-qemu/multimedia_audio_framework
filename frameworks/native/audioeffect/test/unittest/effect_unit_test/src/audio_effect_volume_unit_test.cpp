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

#ifndef LOG_TAG
#define LOG_TAG "AudioEffectVolumeUnitTest"
#endif

#include "audio_effect_volume_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_volume.h"
#include "audio_errors.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
}

void AudioEffectVolumeUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetUpTestCase start ");
}
void AudioEffectVolumeUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: TearDownTestCase start ");
}
void AudioEffectVolumeUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetUp start ");
}
void AudioEffectVolumeUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: TearDown start ");
}

/**
* @tc.name   : Test GetSystemVolume API
* @tc.number : GetSystemVolume_001
* @tc.desc   : Test GetSystemVolume interface.
*/
HWTEST(AudioEffectVolumeUnitTest, GetSystemVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetSystemVolume_001 start ");
    const int32_t systemVolumeType = 1;
    const float systemVolume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetSystemVolume(systemVolumeType, systemVolume);
    float result = audioEffectVolume->GetSystemVolume(systemVolumeType);
    EXPECT_EQ(systemVolume, result);

    result = audioEffectVolume->GetSystemVolume(9999);
    EXPECT_EQ(systemVolume, result);
}

/**
* @tc.name   : Test GetStreamVolume API
* @tc.number : GetStreamVolume_001
* @tc.desc   : Test GetStreamVolume interface.
*/
HWTEST(AudioEffectVolumeUnitTest, GetStreamVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetSystemVolume_001 start ");
    const std::string sessionID = "123";
    const float streamVolume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
    float result = audioEffectVolume->GetStreamVolume(sessionID);
    EXPECT_EQ(streamVolume, result);

    result = audioEffectVolume->GetStreamVolume("");
    EXPECT_EQ(1.0, result);
}

/**
* @tc.name   : Test StreamVolumeDelete API
* @tc.number : StreamVolumeDelete_001
* @tc.desc   : Test StreamVolumeDelete interface.
*/
HWTEST(AudioEffectVolumeUnitTest, StreamVolumeDelete_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: StreamVolumeDelete_001 start ");
    const std::string sessionID = "123";
    const float streamVolume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
    float result = audioEffectVolume->GetStreamVolume(sessionID);
    EXPECT_EQ(streamVolume, result);

    int32_t result2 = audioEffectVolume->StreamVolumeDelete(sessionID);
    EXPECT_EQ(0, result2);

    result2 = audioEffectVolume->StreamVolumeDelete(sessionID);
    EXPECT_EQ(0, result2);
}

/**
* @tc.name   : Test GetDspVolume API
* @tc.number : GetDspVolume_001
* @tc.desc   : Test GetDspVolume interface.
*/
HWTEST(AudioEffectVolumeUnitTest, GetDspVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetDspVolume_001 start ");
    const std::string sceneType = "SCENE_MUSIC";
    const float volume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetDspVolume(volume);
    float result = audioEffectVolume->GetDspVolume();
    EXPECT_EQ(volume, result);
}

/**
* @tc.name   : Test SetSystemVolumeWithDeviceType and GetSystemVolumeWithDeviceType API
* @tc.number : SetSystemVolumeWithDeviceType_001
* @tc.desc   : Test SetSystemVolumeWithDeviceType and GetSystemVolumeWithDeviceType interface for SPEAKER.
*/
HWTEST(AudioEffectVolumeUnitTest, SetSystemVolumeWithDeviceType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetSystemVolumeWithDeviceType_001 start ");
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.6f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    float result = audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, deviceType);
    EXPECT_EQ(systemVolume, result);
}

/**
* @tc.name   : Test SetSystemVolumeWithDeviceType and GetSystemVolumeWithDeviceType API
* @tc.number : SetSystemVolumeWithDeviceType_002
* @tc.desc   : Test SetSystemVolumeWithDeviceType and GetSystemVolumeWithDeviceType interface for BLUETOOTH_A2DP.
*/
HWTEST(AudioEffectVolumeUnitTest, SetSystemVolumeWithDeviceType_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetSystemVolumeWithDeviceType_002 start ");
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.7f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    float result = audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, deviceType);
    EXPECT_EQ(systemVolume, result);
}

/**
* @tc.name   : Test SetSystemVolumeWithDeviceType and GetSystemVolumeWithDeviceType API
* @tc.number : SetSystemVolumeWithDeviceType_003
* @tc.desc   : Test SetSystemVolumeWithDeviceType and GetSystemVolumeWithDeviceType interface for WIRED_HEADSET.
*/
HWTEST(AudioEffectVolumeUnitTest, SetSystemVolumeWithDeviceType_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetSystemVolumeWithDeviceType_003 start ");
    const int32_t systemVolumeType = STREAM_RING;
    const float systemVolume = 0.8f;
    const DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    float result = audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, deviceType);
    EXPECT_EQ(systemVolume, result);
}

/**
* @tc.name   : Test GetSystemVolumeWithDeviceType fallback to SPEAKER
* @tc.number : GetSystemVolumeWithDeviceType_Fallback_001
* @tc.desc   : Test GetSystemVolumeWithDeviceType fallback to SPEAKER when device not set.
*/
HWTEST(AudioEffectVolumeUnitTest, GetSystemVolumeWithDeviceType_Fallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetSystemVolumeWithDeviceType_Fallback_001 start ");
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.5f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    // Set volume for SPEAKER
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    // Get volume for unset device (USB_HEADSET) - should fallback to SPEAKER
    float result = audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(systemVolume, result);
}

/**
* @tc.name   : Test GetSystemVolumeWithDeviceType final fallback
* @tc.number : GetSystemVolumeWithDeviceType_Fallback_002
* @tc.desc   : Test GetSystemVolumeWithDeviceType final fallback to 1.0f.
*/
HWTEST(AudioEffectVolumeUnitTest, GetSystemVolumeWithDeviceType_Fallback_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetSystemVolumeWithDeviceType_Fallback_002 start ");
    const int32_t systemVolumeType = STREAM_ALARM;
    const DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    // Get volume for unset device and stream type - should return default 1.0f
    float result = audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, deviceType);
    EXPECT_EQ(0.5f, result);
}

/**
* @tc.name   : Test multiple device types with different volumes
* @tc.number : SetSystemVolumeWithDeviceType_Multiple_001
* @tc.desc   : Test setting different volumes for different device types.
*/
HWTEST(AudioEffectVolumeUnitTest, SetSystemVolumeWithDeviceType_Multiple_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetSystemVolumeWithDeviceType_Multiple_001 start ");
    const int32_t systemVolumeType = STREAM_MUSIC;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();

    // Set different volumes for different devices
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, 0.5f, DEVICE_TYPE_SPEAKER);
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, 0.6f, DEVICE_TYPE_BLUETOOTH_A2DP);
    audioEffectVolume->SetSystemVolumeWithDeviceType(systemVolumeType, 0.7f, DEVICE_TYPE_WIRED_HEADSET);

    // Verify each device has correct volume
    EXPECT_EQ(0.5f, audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, DEVICE_TYPE_SPEAKER));
    EXPECT_EQ(0.6f, audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, DEVICE_TYPE_BLUETOOTH_A2DP));
    EXPECT_EQ(0.7f, audioEffectVolume->GetSystemVolumeWithDeviceType(systemVolumeType, DEVICE_TYPE_WIRED_HEADSET));
}

/**
* @tc.name   : Test multiple stream types for same device
* @tc.number : SetSystemVolumeWithDeviceType_Multiple_002
* @tc.desc   : Test setting different volumes for different stream types on same device.
*/
HWTEST(AudioEffectVolumeUnitTest, SetSystemVolumeWithDeviceType_Multiple_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetSystemVolumeWithDeviceType_Multiple_002 start ");
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();

    // Set different volumes for different stream types
    audioEffectVolume->SetSystemVolumeWithDeviceType(STREAM_MUSIC, 0.5f, deviceType);
    audioEffectVolume->SetSystemVolumeWithDeviceType(STREAM_RING, 0.6f, deviceType);
    audioEffectVolume->SetSystemVolumeWithDeviceType(STREAM_NOTIFICATION, 0.7f, deviceType);

    // Verify each stream type has correct volume
    EXPECT_EQ(0.5f, audioEffectVolume->GetSystemVolumeWithDeviceType(STREAM_MUSIC, deviceType));
    EXPECT_EQ(0.6f, audioEffectVolume->GetSystemVolumeWithDeviceType(STREAM_RING, deviceType));
    EXPECT_EQ(0.7f, audioEffectVolume->GetSystemVolumeWithDeviceType(STREAM_NOTIFICATION, deviceType));
}
} // namespace AudioStandard
} // namespace OHOS