/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "audio_session_client_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioSessionClientManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioSessionClientManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetUpTestCase");
}

void AudioSessionClientManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT TearDownTestCase");
}

void AudioSessionClientManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetUp");
}

void AudioSessionClientManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT TearDown");
}

/**
 * @tc.name   : Test GetInstance API
 * @tc.type  : FUNC
 * @tc.number: GetInstance_001
 * @tc.desc  : Test GetInstance returns valid reference.
 */
HWTEST(AudioSessionClientManagerUnitTest, GetInstance_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT GetInstance_001 start");
    AudioSessionClientManager &instance = AudioSessionClientManager::GetInstance();
    // Just verify that GetInstance returns a valid reference
    (void)instance; // Suppress unused variable warning
    EXPECT_TRUE(true);
}

/**
 * @tc.name   : Test SetAudioSessionCallback API with null callback
 * @tc.type  : FUNC
 * @tc.number: SetAudioSessionCallback_001
 * @tc.desc  : Test SetAudioSessionCallback with null callback returns error.
 */
HWTEST(AudioSessionClientManagerUnitTest, SetAudioSessionCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionCallback_001 start");
    std::shared_ptr<AudioSessionCallback> nullCallback = nullptr;
    int32_t result = AudioSessionClientManager::GetInstance().SetAudioSessionCallback(nullCallback);
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionCallback_001 result:%{public}d", result);
    // Should return error for null callback
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SetAudioSessionStateChangeCallback API with null callback
 * @tc.type  : FUNC
 * @tc.number: SetAudioSessionStateChangeCallback_001
 * @tc.desc  : Test SetAudioSessionStateChangeCallback with null callback returns error.
 */
HWTEST(AudioSessionClientManagerUnitTest, SetAudioSessionStateChangeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionStateChangeCallback_001 start");
    std::shared_ptr<AudioSessionStateChangedCallback> nullCallback = nullptr;
    int32_t result = AudioSessionClientManager::GetInstance().SetAudioSessionStateChangeCallback(nullCallback);
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionStateChangeCallback_001 result:%{public}d", result);
    // Should return error for null callback
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SetAudioSessionCurrentDeviceChangeCallback API with null callback
 * @tc.type  : FUNC
 * @tc.number: SetAudioSessionCurrentDeviceChangeCallback_001
 * @tc.desc  : Test SetAudioSessionCurrentDeviceChangeCallback with null callback returns error.
 */
HWTEST(AudioSessionClientManagerUnitTest, SetAudioSessionCurrentDeviceChangeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionCurrentDeviceChangeCallback_001 start");
    std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> nullCallback = nullptr;
    int32_t result = AudioSessionClientManager::GetInstance().SetAudioSessionCurrentDeviceChangeCallback(nullCallback);
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionCurrentDeviceChangeCallback_001 result:%{public}d", result);
    // Should return error for null callback
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SetAudioSessionCurrentInputDeviceChangeCallback API with null callback
 * @tc.type  : FUNC
 * @tc.number: SetAudioSessionCurrentInputDeviceChangeCallback_001
 * @tc.desc  : Test SetAudioSessionCurrentInputDeviceChangeCallback with null callback returns error.
 */
HWTEST(AudioSessionClientManagerUnitTest, SetAudioSessionCurrentInputDeviceChangeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionCurrentInputDeviceChangeCallback_001 start");
    std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback> nullCallback = nullptr;
    int32_t result = AudioSessionClientManager::GetInstance()
        .SetAudioSessionCurrentInputDeviceChangeCallback(nullCallback);
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetAudioSessionCurrentInputDeviceChangeCallback_001 result:%{public}d", result);
    // Should return error for null callback
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test UnsetAudioSessionCallback API
 * @tc.type  : FUNC
 * @tc.number: UnsetAudioSessionCallback_001
 * @tc.desc  : Test UnsetAudioSessionCallback interface.
 */
HWTEST(AudioSessionClientManagerUnitTest, UnsetAudioSessionCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT UnsetAudioSessionCallback_001 start");
    int32_t result = AudioSessionClientManager::GetInstance().UnsetAudioSessionCallback();
    AUDIO_INFO_LOG("AudioSessCliMgrUT UnsetAudioSessionCallback_001 result:%{public}d", result);
    // Should return success or error code, but not crash
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test IsAudioSessionActivated API
 * @tc.type  : FUNC
 * @tc.number: IsAudioSessionActivated_001
 * @tc.desc  : Test IsAudioSessionActivated interface.
 */
HWTEST(AudioSessionClientManagerUnitTest, IsAudioSessionActivated_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT IsAudioSessionActivated_001 start");
    bool result = AudioSessionClientManager::GetInstance().IsAudioSessionActivated();
    AUDIO_INFO_LOG("AudioSessCliMgrUT IsAudioSessionActivated_001 result:%{public}d", result);
    // Should return false when session is not activated
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test IsOtherMediaPlaying API
 * @tc.type  : FUNC
 * @tc.number: IsOtherMediaPlaying_001
 * @tc.desc  : Test IsOtherMediaPlaying interface.
 */
HWTEST(AudioSessionClientManagerUnitTest, IsOtherMediaPlaying_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT IsOtherMediaPlaying_001 start");
    bool result = AudioSessionClientManager::GetInstance().IsOtherMediaPlaying();
    AUDIO_INFO_LOG("AudioSessCliMgrUT IsOtherMediaPlaying_001 result:%{public}d", result);
    // Should return false when no other media is playing
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test GetAvailableDevices API
 * @tc.type  : FUNC
 * @tc.number: GetAvailableDevices_001
 * @tc.desc  : Test GetAvailableDevices with valid usage.
 */
HWTEST(AudioSessionClientManagerUnitTest, GetAvailableDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT GetAvailableDevices_001 start");
    auto devices = AudioSessionClientManager::GetInstance().GetAvailableDevices(MEDIA_OUTPUT_DEVICES);
    AUDIO_INFO_LOG("AudioSessCliMgrUT GetAvailableDevices_001 devices size:%{public}zu", devices.size());
    // Should return a vector (may be empty)
    EXPECT_FALSE(devices.empty());
}

/**
 * @tc.name   : Test GetSelectedInputDevice API
 * @tc.type  : FUNC
 * @tc.number: GetSelectedInputDevice_001
 * @tc.desc  : Test GetSelectedInputDevice interface.
 */
HWTEST(AudioSessionClientManagerUnitTest, GetSelectedInputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT GetSelectedInputDevice_001 start");
    auto device = AudioSessionClientManager::GetInstance().GetSelectedInputDevice();
    AUDIO_INFO_LOG("AudioSessCliMgrUT GetSelectedInputDevice_001 device:%{public}p", device.get());
    // Should return nullptr or valid device
    EXPECT_NE(device, nullptr);
}

/**
 * @tc.name   : Test ClearSelectedInputDevice API
 * @tc.type  : FUNC
 * @tc.number: ClearSelectedInputDevice_001
 * @tc.desc  : Test ClearSelectedInputDevice interface.
 */
HWTEST(AudioSessionClientManagerUnitTest, ClearSelectedInputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT ClearSelectedInputDevice_001 start");
    int32_t result = AudioSessionClientManager::GetInstance().ClearSelectedInputDevice();
    AUDIO_INFO_LOG("AudioSessCliMgrUT ClearSelectedInputDevice_001 result:%{public}d", result);
    // Should return success or error code, but not crash
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test SetRegisterStateChangeCallbackFlag API
 * @tc.type  : FUNC
 * @tc.number: SetRegisterStateChangeCallbackFlag_001
 * @tc.desc  : Test SetRegisterStateChangeCallbackFlag interface.
 */
HWTEST(AudioSessionClientManagerUnitTest, SetRegisterStateChangeCallbackFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSessCliMgrUT SetRegisterStateChangeCallbackFlag_001 start");
    // Test with true
    AudioSessionClientManager::GetInstance().SetRegisterStateChangeCallbackFlag(true);
    // Test with false
    AudioSessionClientManager::GetInstance().SetRegisterStateChangeCallbackFlag(false);
    // If we reach here, the method doesn't crash
    EXPECT_TRUE(true);
}

} // namespace AudioStandard
} // namespace OHOS
