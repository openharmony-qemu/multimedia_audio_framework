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

#include "audio_general_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static int32_t NUM_1 = 1;
static int32_t NUM_2 = 2;
static int32_t NUM_64 = 64;

void AudioGeneralManagerUnitTest::SetUpTestCase(void) {}
void AudioGeneralManagerUnitTest::TearDownTestCase(void) {}
void AudioGeneralManagerUnitTest::SetUp(void) {}
void AudioGeneralManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_001.
* @tc.desc  : Test AudioGeneralManager::RegisterFocusInfoChangeCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_001, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    std::shared_ptr<AudioFocusInfoChangeCallback> callback = std::make_shared<AudioFocusInfoChangeCallbackTest>();
    EXPECT_NE(callback, nullptr);

    audioGeneralManagerPtr->audioFocusInfoCallback_ = nullptr;
    auto ret = audioGeneralManagerPtr->RegisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_002.
* @tc.desc  : Test AudioGeneralManager::RegisterFocusInfoChangeCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_002, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    std::shared_ptr<AudioFocusInfoChangeCallback> callback = std::make_shared<AudioFocusInfoChangeCallbackTest>();
    EXPECT_NE(callback, nullptr);

    audioGeneralManagerPtr->audioFocusInfoCallback_ = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(audioGeneralManagerPtr->audioFocusInfoCallback_, nullptr);

    auto ret = audioGeneralManagerPtr->RegisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_003
* @tc.desc  : Test AudioGeneralManager::GetVolume()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_003, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;

    auto ret = audioGeneralManagerPtr->GetVolume(volumeType);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_004
* @tc.desc  : Test AudioGeneralManager::GetVolume()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_004, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    AudioVolumeType volumeType = AudioVolumeType::STREAM_ULTRASONIC;

    auto ret = audioGeneralManagerPtr->GetVolume(volumeType);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_005
* @tc.desc  : Test AudioGeneralManager::GetVolume()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_005, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    AudioVolumeType volumeType = AudioVolumeType::STREAM_TTS;

    auto ret = audioGeneralManagerPtr->GetVolume(volumeType);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_006
* @tc.desc  : Test AudioGeneralManager::UnregisterVolumeKeyEventCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_006, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    int32_t clientPid = 0;
    std::shared_ptr<VolumeKeyEventCallback> callback = nullptr;

    auto ret = audioGeneralManagerPtr->UnregisterVolumeKeyEventCallback(clientPid, callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_007
* @tc.desc  : Test AudioGeneralManager::GetMaxVolume()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_007, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    AudioVolumeType volumeType = AudioVolumeType::STREAM_ALL;

    auto ret = audioGeneralManagerPtr->GetMaxVolume(volumeType);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_008
* @tc.desc  : Test AudioGeneralManager::GetMaxVolume()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_008, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    AudioVolumeType volumeType = AudioVolumeType::STREAM_ULTRASONIC;

    auto ret = audioGeneralManagerPtr->GetMaxVolume(volumeType);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_009
* @tc.desc  : Test AudioGeneralManager::GetMaxVolume()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_009, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    AudioVolumeType volumeType = AudioVolumeType::STREAM_TTS;

    auto ret = audioGeneralManagerPtr->GetMaxVolume(volumeType);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_010
* @tc.desc  : Test AudioGeneralManager::UnregisterFocusInfoChangeCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_010, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    std::shared_ptr<AudioFocusInfoChangeCallback> callback = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(callback, nullptr);

    auto ret = audioGeneralManagerPtr->UnregisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_011
* @tc.desc  : Test AudioGeneralManager::UnregisterFocusInfoChangeCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_011, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    std::shared_ptr<AudioFocusInfoChangeCallback> callback = nullptr;

    auto ret = audioGeneralManagerPtr->UnregisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_012
* @tc.desc  : Test AudioGeneralManager::SelectOutputDevice()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_012, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE,
        OUTPUT_DEVICE, 0, 0, REMOTE_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = {audioDeviceDescriptor};

    auto ret = audioGeneralManagerPtr->SelectOutputDevice(audioDeviceDescriptors);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_013
* @tc.desc  : Test AudioGeneralManager::SelectOutputDevice()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_013, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE,
        OUTPUT_DEVICE, 0, 0, REMOTE_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = {audioDeviceDescriptor};

    auto ret = audioGeneralManagerPtr->SelectOutputDevice(audioDeviceDescriptors);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_014
* @tc.desc  : Test AudioGeneralManager::SelectOutputDevice()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_014, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    std::string str(NUM_64, 'A');
    EXPECT_EQ(str.size(), NUM_64);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE,
        OUTPUT_DEVICE, 0, 0, str);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = {audioDeviceDescriptor};

    auto ret = audioGeneralManagerPtr->SelectOutputDevice(audioDeviceDescriptors);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_015
* @tc.desc  : Test AudioGeneralManager::SelectOutputDevice()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_015, TestSize.Level1)
{
    auto audioGeneralManagerPtr = AudioGeneralManager::GetInstance();
    EXPECT_NE(audioGeneralManagerPtr, nullptr);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE,
        OUTPUT_DEVICE, 0, 0, LOCAL_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = {audioDeviceDescriptor};

    auto ret = audioGeneralManagerPtr->SelectOutputDevice(audioDeviceDescriptors);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_016
* @tc.desc  : Test AudioFocusInfoChangeCallbackImpl::SaveCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_016, TestSize.Level1)
{
    auto audioFocusInfoChangeCallbackImpl = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(audioFocusInfoChangeCallbackImpl, nullptr);

    auto callback = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(callback, nullptr);

    audioFocusInfoChangeCallbackImpl->callbackList_.push_back(callback);

    audioFocusInfoChangeCallbackImpl->SaveCallback(callback);
    EXPECT_EQ(audioFocusInfoChangeCallbackImpl->callbackList_.size(), NUM_1);
}

/**
* @tc.name  : Test AudioGeneralManager.
* @tc.number: AudioGeneralManager_017
* @tc.desc  : Test AudioFocusInfoChangeCallbackImpl::SaveCallback()
*/
HWTEST(AudioGeneralManagerUnitTest, AudioGeneralManager_017, TestSize.Level1)
{
    auto audioFocusInfoChangeCallbackImpl = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(audioFocusInfoChangeCallbackImpl, nullptr);

    auto callback = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(callback, nullptr);

    auto callback2 = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
    EXPECT_NE(callback2, nullptr);

    EXPECT_NE(callback, callback2);

    audioFocusInfoChangeCallbackImpl->callbackList_.push_back(callback);

    audioFocusInfoChangeCallbackImpl->SaveCallback(callback2);
    EXPECT_EQ(audioFocusInfoChangeCallbackImpl->callbackList_.size(), NUM_2);
}
} // namespace AudioStandard
} // namespace OHOS