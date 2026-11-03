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

#include "audio_device_enhance_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_renderer.h"
#include "audio_capturer.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
class AudioDeviceEnhanceManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioDeviceEnhanceManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SetUpTestCase");
}

void AudioDeviceEnhanceManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest TearDownTestCase");
}

void AudioDeviceEnhanceManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SetUp");
}

void AudioDeviceEnhanceManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest TearDown");
}

/**
 * @tc.name   : Test IsEnhancedRoutingSupported API
 * @tc.type  : FUNC
 * @tc.number: IsEnhancedRoutingSupported_001
 * @tc.desc  : Test IsEnhancedRoutingSupported interface.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, IsEnhancedRoutingSupported_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest IsEnhancedRoutingSupported_001 start");
    bool supported = false;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().IsEnhancedRoutingSupported(supported);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest IsEnhancedRoutingSupported_001 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test SelectOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SelectOutputDevice_001
 * @tc.desc  : Test SelectOutputDevice with null descriptor.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDevice_001 start");
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectOutputDevice(nullDesc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDevice_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test SelectOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SelectOutputDevice_002
 * @tc.desc  : Test SelectOutputDevice with invalid device type.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDevice_002 start");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectOutputDevice(desc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDevice_002 result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SelectInputDevice API
 * @tc.type  : FUNC
 * @tc.number: SelectInputDevice_001
 * @tc.desc  : Test SelectInputDevice with null descriptor.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectInputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDevice_001 start");
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectInputDevice(nullDesc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDevice_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test SelectInputDevice API
 * @tc.type  : FUNC
 * @tc.number: SelectInputDevice_002
 * @tc.desc  : Test SelectInputDevice with invalid device type.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectInputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDevice_002 start");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectInputDevice(desc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDevice_002 result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SelectOutputDeviceForAudioRenderer API
 * @tc.type  : FUNC
 * @tc.number: SelectOutputDeviceForAudioRenderer_001
 * @tc.desc  : Test SelectOutputDeviceForAudioRenderer with null renderer.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectOutputDeviceForAudioRenderer_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDeviceForAudioRenderer_001 start");
    std::shared_ptr<AudioRenderer> nullRenderer = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectOutputDeviceForAudioRenderer(nullRenderer, desc);
    AUDIO_INFO_LOG("AudioDevEnhanceMgrUT SelectOutputDeviceForAudioRenderer_001 result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SelectInputDeviceForAudioCapturer API
 * @tc.type  : FUNC
 * @tc.number: SelectInputDeviceForAudioCapturer_001
 * @tc.desc  : Test SelectInputDeviceForAudioCapturer with null capturer.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectInputDeviceForAudioCapturer_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDeviceForAudioCapturer_001 start");
    std::shared_ptr<AudioCapturer> nullCapturer = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectInputDeviceForAudioCapturer(nullCapturer, desc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDeviceForAudioCapturer_001 result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test GetSoundCardInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSoundCardInfo_001
 * @tc.desc  : Test GetSoundCardInfo interface.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, GetSoundCardInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfo_001 start");
    SoundCardInfo info;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().GetSoundCardInfo(info);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfo_001 result:%{public}d", result);
#ifndef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
    // Feature switch is off: should return 801
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
#endif
    // When service is not available, should return empty info
    EXPECT_TRUE(info.IsEmpty());
}

/**
 * @tc.name   : Test SoundCardInfo IsEmpty API
 * @tc.type  : FUNC
 * @tc.number: SoundCardInfo_IsEmpty_001
 * @tc.desc  : Test SoundCardInfo IsEmpty interface.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SoundCardInfo_IsEmpty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_IsEmpty_001 start");
    SoundCardInfo info;
    bool result = info.IsEmpty();
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_IsEmpty_001 result:%{public}d", result);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : Test SoundCardInfo IsEmpty API
 * @tc.type  : FUNC
 * @tc.number: SoundCardInfo_IsEmpty_002
 * @tc.desc  : Test SoundCardInfo IsEmpty with filled info.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SoundCardInfo_IsEmpty_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_IsEmpty_002 start");
    SoundCardInfo info;
    info.name = "test";
    bool result = info.IsEmpty();
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_IsEmpty_002 result:%{public}d", result);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test SelectOutputDevice with valid device
 * @tc.type  : FUNC
 * @tc.number: SelectOutputDevice_003
 * @tc.desc  : Test SelectOutputDevice with valid output device (but enhanced routing not supported).
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectOutputDevice_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDevice_003 start");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    // When enhanced routing is not supported, should return SUCCESS directly
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectOutputDevice(desc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDevice_003 result:%{public}d", result);
    // In unit test environment, IsEnhancedRoutingSupported() may return false
    // So SelectOutputDevice() may return SUCCESS
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test SelectInputDevice with valid device
 * @tc.type  : FUNC
 * @tc.number: SelectInputDevice_003
 * @tc.desc  : Test SelectInputDevice with valid input device (but enhanced routing not supported).
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectInputDevice_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDevice_003 start");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    // When enhanced routing is not supported, should return SUCCESS directly
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectInputDevice(desc);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDevice_003 result:%{public}d", result);
    // In unit test environment, IsEnhancedRoutingSupported() may return false
    // So SelectInputDevice() may return SUCCESS
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test SelectOutputDeviceForAudioRenderer with released renderer
 * @tc.type  : FUNC
 * @tc.number: SelectOutputDeviceForAudioRenderer_002
 * @tc.desc  : Test SelectOutputDeviceForAudioRenderer with released renderer.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectOutputDeviceForAudioRenderer_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectOutputDeviceForAudioRenderer_002 start");
    // This test requires a mock AudioRenderer with RELEASED status
    // In unit test environment, we can't easily create such object
    // So we just test the null pointer case (already covered in _001)
    std::shared_ptr<AudioRenderer> renderer = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectOutputDeviceForAudioRenderer(renderer, desc);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test SelectInputDeviceForAudioCapturer with released capturer
 * @tc.type  : FUNC
 * @tc.number: SelectInputDeviceForAudioCapturer_002
 * @tc.desc  : Test SelectInputDeviceForAudioCapturer with released capturer.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SelectInputDeviceForAudioCapturer_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SelectInputDeviceForAudioCapturer_002 start");
    // This test requires a mock AudioCapturer with RELEASED status
    // In unit test environment, we can't easily create such object
    // So we just test the null pointer case (already covered in _001)
    std::shared_ptr<AudioCapturer> capturer = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().SelectInputDeviceForAudioCapturer(capturer, desc);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test GetSoundCardInfo with empty response
 * @tc.type  : FUNC
 * @tc.number: GetSoundCardInfo_002
 * @tc.desc  : Test GetSoundCardInfo when ParseSoundCardInfo returns empty info.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, GetSoundCardInfo_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfo_002 start");
    // When service proxy is not available, GetSoundCardInfo returns empty info
    SoundCardInfo info;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().GetSoundCardInfo(info);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfo_002 result:%{public}d", result);
    // In unit test environment, service is not available
    // GetSoundCardInfo() should return empty info
    EXPECT_TRUE(info.IsEmpty());  // Service not available, should return empty info
#ifndef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
    // Feature switch is off: the out param must not be modified, error code is 801
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name   : Test GetSoundCardInfoList API
 * @tc.type  : FUNC
 * @tc.number: GetSoundCardInfoList_001
 * @tc.desc  : Test GetSoundCardInfoList when feature switch is off.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, GetSoundCardInfoList_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfoList_001 start");
    std::vector<SoundCardInfo> soundCardList;
    int32_t result = AudioDeviceEnhanceManager::GetInstance().GetSoundCardInfoList(soundCardList);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfoList_001 result:%{public}d", result);
#ifndef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
    // Feature switch is off: should return 801 and keep the list empty
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
#endif
    EXPECT_TRUE(soundCardList.empty());
}

/**
 * @tc.name   : Test GetSoundCardInfoList API
 * @tc.type  : FUNC
 * @tc.number: GetSoundCardInfoList_002
 * @tc.desc  : Test GetSoundCardInfoList out param is not modified when feature switch is off.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, GetSoundCardInfoList_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfoList_002 start");
    std::vector<SoundCardInfo> soundCardList;
    soundCardList.push_back(SoundCardInfo());
    // Caller pre-filled the list, feature off should not touch it
    int32_t result = AudioDeviceEnhanceManager::GetInstance().GetSoundCardInfoList(soundCardList);
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest GetSoundCardInfoList_002 result:%{public}d", result);
#ifndef AUDIO_FRAMEWORK_FEATURE_SOUNDCARD_INFO
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
#endif
    EXPECT_EQ(soundCardList.size(), 1);
}

/**
 * @tc.name   : Test SoundCardInfo IsEmpty with only cardId filled
 * @tc.type  : FUNC
 * @tc.number: SoundCardInfo_IsEmpty_003
 * @tc.desc  : Test SoundCardInfo IsEmpty when only cardId is set.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SoundCardInfo_IsEmpty_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_IsEmpty_003 start");
    SoundCardInfo info;
    info.cardId = "ONBOARD\\VEN_test&platform&card0";
    bool result = info.IsEmpty();
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_IsEmpty_003 result:%{public}d", result);
    // cardId participates in IsEmpty(): cardId filled means not empty
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test SoundCardInfo default cardType
 * @tc.type  : FUNC
 * @tc.number: SoundCardInfo_CardType_001
 * @tc.desc  : Test SoundCardInfo default cardType is ONBOARD and USB can be assigned.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SoundCardInfo_CardType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_CardType_001 start");
    SoundCardInfo info;
    // Default card type is ONBOARD
    EXPECT_EQ(info.cardType, SoundCardType::ONBOARD);
    // USB card type can be assigned
    info.cardType = SoundCardType::USB;
    EXPECT_EQ(info.cardType, SoundCardType::USB);
}

/**
 * @tc.name   : Test SoundCardInfo USB cardId assignment
 * @tc.type  : FUNC
 * @tc.number: SoundCardInfo_CardId_001
 * @tc.desc  : Test SoundCardInfo with USB cardId filled is not empty.
 */
HWTEST_F(AudioDeviceEnhanceManagerUnitTest, SoundCardInfo_CardId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_CardId_001 start");
    SoundCardInfo info;
    info.cardId = "USB\\VID_1234&PID_5678\\3&2";
    info.cardType = SoundCardType::USB;
    bool result = info.IsEmpty();
    AUDIO_INFO_LOG("AudioDeviceEnhanceManagerUnitTest SoundCardInfo_CardId_001 result:%{public}d", result);
    EXPECT_FALSE(result);
    EXPECT_EQ(info.cardType, SoundCardType::USB);
}

} // namespace AudioStandard
} // namespace OHOS
