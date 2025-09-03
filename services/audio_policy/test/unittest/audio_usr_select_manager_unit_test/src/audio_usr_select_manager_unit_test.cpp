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

void AudioUsrSelectManagerUnitTest::SetUpTestCase(void) {}
void AudioUsrSelectManagerUnitTest::TearDownTestCase(void) {}
void AudioUsrSelectManagerUnitTest::SetUp(void) {
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    desc1->deviceId_ = 1;
    desc1->macAddress_ = "00:11:22:33:44:55";
    desc1->connectState_ = VIRTUAL_CONNECTED;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc1);

    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, DeviceRole::INPUT_DEVICE);
    desc2->deviceId_ = 2;
    desc2->macAddress_ = "00:11:22:33:44:55";
    desc2->connectState_ = CONNECTED;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc1);
}
void AudioUsrSelectManagerUnitTest::TearDown(void) {
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
* @tc.number: AudioUsrSelectManager_SelectInputDeviceByUid_003
* @tc.desc  : Test SelectInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_SelectInputDeviceByUid_003, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = 2;
    int32_t uid = 123;
    EXPECT_EQ(audioUsrSelectManager.SelectInputDeviceByUid(desc, uid), true);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_GetSelectedInputDeviceByUid_001
* @tc.desc  : Test GetSelectedInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_GetSelectedInputDeviceByUid_001, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    int32_t uid = 321;
    auto desc = audioUsrSelectManager.GetSelectedInputDeviceByUid(uid);
    EXPECT_EQ(desc->deviceType_, DeviceType(0));
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_GetSelectedInputDeviceByUid_002
* @tc.desc  : Test GetSelectedInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_GetSelectedInputDeviceByUid_002, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    int32_t uid = 123;
    auto desc = audioUsrSelectManager.GetSelectedInputDeviceByUid(uid);
    EXPECT_EQ(desc->deviceId_, 2);

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
    desc = audioUsrSelectManager.GetSelectedInputDeviceByUid(uid);
    EXPECT_EQ(desc->deviceType_, DeviceType(0));
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_ClearSelectedInputDeviceByUid_001
* @tc.desc  : Test ClearSelectedInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_ClearSelectedInputDeviceByUid_001, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    int32_t uid = 321;
    audioUsrSelectManager.ClearSelectedInputDeviceByUid(uid);
    EXPECT_EQ(audioUsrSelectManager.selectedDevices_.size(), 1);

    uid = 123;
    audioUsrSelectManager.ClearSelectedInputDeviceByUid(uid);
    EXPECT_EQ(audioUsrSelectManager.selectedDevices_.size(), 0);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_PreferBluetoothAndNearlinkRecordByUid_001
* @tc.desc  : Test PreferBluetoothAndNearlinkRecordByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_PreferBluetoothAndNearlinkRecordByUid_001,
    TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    audioUsrSelectManager.PreferBluetoothAndNearlinkRecordByUid(123, true);
    EXPECT_EQ(audioUsrSelectManager.isPreferredBluetoothAndNearlinkRecord_.size(), 1);

    audioUsrSelectManager.PreferBluetoothAndNearlinkRecordByUid(123, false);
    EXPECT_EQ(audioUsrSelectManager.isPreferredBluetoothAndNearlinkRecord_.size(), 0);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_GetPreferBluetoothAndNearlinkRecordByUid_001
* @tc.desc  : Test GetPreferBluetoothAndNearlinkRecordByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_GetPreferBluetoothAndNearlinkRecordByUid_001,
    TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    EXPECT_EQ(audioUsrSelectManager.GetPreferBluetoothAndNearlinkRecordByUid(123), true);
    EXPECT_EQ(audioUsrSelectManager.GetPreferBluetoothAndNearlinkRecordByUid(321), false);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_GetRealUid_001
* @tc.desc  : Test GetRealUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_GetRealUid_001, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();

    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->callerUid_ = 123;
    EXPECT_EQ(audioUsrSelectManager.GetRealUid(stream), 123);

    stream->callerUid_ = 1013;
    stream->appInfo_.appUid = 321;
    EXPECT_EQ(audioUsrSelectManager.GetRealUid(stream), 321);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_JudgeFinalSelectDevice_001
* @tc.desc  : Test JudgeFinalSelectDevice interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_JudgeFinalSelectDevice_001, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    desc->deviceId_ = 1;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = VIRTUAL_CONNECTED;
    auto judge = audioUsrSelectManager.JudgeFinalSelectDevice(desc, SOURCE_TYPE_CAMCORDER);
    EXPECT_NE(judge, nullptr);
    EXPECT_EQ(judge->deviceId_, 2);

    judge = audioUsrSelectManager.JudgeFinalSelectDevice(desc, SOURCE_TYPE_LIVE);
    EXPECT_NE(judge, nullptr);
    EXPECT_EQ(judge->deviceId_, 2);
}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManager_JudgeFinalSelectDevice_002
* @tc.desc  : Test JudgeFinalSelectDevice interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManager_JudgeFinalSelectDevice_002, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, DeviceRole::INPUT_DEVICE);
    desc->deviceId_ = 2;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;

    auto judge = audioUsrSelectManager.JudgeFinalSelectDevice(desc, SOURCE_TYPE_CAMCORDER);
    EXPECT_NE(judge, nullptr);

    judge = audioUsrSelectManager.JudgeFinalSelectDevice(desc, SOURCE_TYPE_LIVE);
    EXPECT_NE(judge, nullptr);
    
    judge = audioUsrSelectManager.JudgeFinalSelectDevice(desc, SOURCE_TYPE_VOICE_CALL);
    EXPECT_NE(judge, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
