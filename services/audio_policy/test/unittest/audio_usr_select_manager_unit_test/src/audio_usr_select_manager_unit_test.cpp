/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio_usr_select_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEVICE_ID1 = 1;
constexpr int32_t DEVICE_ID2 = 2;

void AudioUsrSelectManagerUnitTest::SetUpTestCase(void) {}
void AudioUsrSelectManagerUnitTest::TearDownTestCase(void) {}
void AudioUsrSelectManagerUnitTest::SetUp(void)
{
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    desc1->deviceId_ = DEVICE_ID1;
    desc1->macAddress_ = "00:11:22:33:44:55";
    desc1->connectState_ = VIRTUAL_CONNECTED;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc1);

    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, DeviceRole::INPUT_DEVICE);
    desc2->deviceId_ = DEVICE_ID2;
    desc2->macAddress_ = "00:11:22:33:44:55";
    desc2->connectState_ = CONNECTED;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc1);
}
void AudioUsrSelectManagerUnitTest::TearDown(void)
{
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_SelectInputDeviceByUid_001
* @tc.desc  : Test SelectInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_SelectInputDeviceByUid_001, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t uid = 123;
    EXPECT_EQ(audioUsrSelectManager.SelectInputDeviceByUid(desc, uid), false);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_SelectInputDeviceByUid_002
* @tc.desc  : Test SelectInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_SelectInputDeviceByUid_002, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = 1;
    int32_t uid = 123;
    EXPECT_EQ(audioUsrSelectManager.SelectInputDeviceByUid(desc, uid), false);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_IsSourceTypeSupportedByMC_001
* @tc.desc  : Test IsSourceTypeSupportedByMC interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_IsSourceTypeSupportedByMC_001, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();

    EXPECT_TRUE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_MIC));
    EXPECT_TRUE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_CAMCORDER));
    EXPECT_TRUE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_LIVE));

    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_INVALID));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_VOICE_RECOGNITION));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_WAKEUP));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_VOICE_CALL));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_VOICE_COMMUNICATION));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_ULTRASONIC));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_VIRTUAL_CAPTURE));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_VOICE_MESSAGE));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_REMOTE_CAST));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_UNPROCESSED));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_EC));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_MIC_REF));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_OFFLOAD_CAPTURE));
    EXPECT_FALSE(audioUsrSelectManager.IsSourceTypeSupportedByMC(SourceType::SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT));
}

} // namespace AudioStandard
} // namespace OHOS
