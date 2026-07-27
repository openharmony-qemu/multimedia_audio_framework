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

#include "audio_effect_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_separation_effect_enabled_change_callback.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
// Local constant for default separation volume type
constexpr AudioSeparationVolumeType AUDIO_SEPARATION_VOLUME_TYPE_DEFAULT = VOLUME_TYPE_VOCAL;

// Mock implementation of abstract callback
class MockSeparationEffectCallback : public AudioSeparationEffectEnabledChangeCallback {
public:
    void OnAudioSeparationEffectEnabledChange(bool enabled) override {}
};

class AudioEffectManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioEffectManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetUpTestCase");
}

void AudioEffectManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest TearDownTestCase");
}

void AudioEffectManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetUp");
}

void AudioEffectManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest TearDown");
}

/**
 * @tc.name   : Test GetSupportedAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: GetSupportedAudioEffectProperty_001
 * @tc.desc  : Test GetSupportedAudioEffectProperty interface.
 */
HWTEST(AudioEffectManagerUnitTest, GetSupportedAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest GetSupportedAudioEffectProperty_001 start");
    AudioEffectPropertyArray propertyArray;
    int32_t result = AudioEffectManager::GetInstance()->GetSupportedAudioEffectProperty(propertyArray);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest GetSupportedAudioEffectProperty_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test SetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectProperty_001
 * @tc.desc  : Test SetAudioEffectProperty interface.
 */
HWTEST(AudioEffectManagerUnitTest, SetAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioEffectProperty_001 start");
    AudioEffectPropertyArray propertyArray;
    int32_t result = AudioEffectManager::GetInstance()->SetAudioEffectProperty(propertyArray);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioEffectProperty_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test GetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEffectProperty_001
 * @tc.desc  : Test GetAudioEffectProperty interface.
 */
HWTEST(AudioEffectManagerUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest GetAudioEffectProperty_001 start");
    AudioEffectPropertyArray propertyArray;
    int32_t result = AudioEffectManager::GetInstance()->GetAudioEffectProperty(propertyArray);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest GetAudioEffectProperty_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test IsAudioSeparationEffectSupported API
 * @tc.type  : FUNC
 * @tc.number: IsAudioSeparationEffectSupported_001
 * @tc.desc  : Test IsAudioSeparationEffectSupported interface.
 */
HWTEST(AudioEffectManagerUnitTest, IsAudioSeparationEffectSupported_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest IsAudioSeparationEffectSupported_001 start");
    bool result = AudioEffectManager::GetInstance()->IsAudioSeparationEffectSupported();
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest IsAudioSeparationEffectSupported_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetAudioSeparationEffectEnabled API
 * @tc.type  : FUNC
 * @tc.number: SetAudioSeparationEffectEnabled_001
 * @tc.desc  : Test SetAudioSeparationEffectEnabled interface.
 */
HWTEST(AudioEffectManagerUnitTest, SetAudioSeparationEffectEnabled_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectEnabled_001 start");
    int32_t result = AudioEffectManager::GetInstance()->SetAudioSeparationEffectEnabled(true, 1000, 0);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectEnabled_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test SetAudioSeparationEffectVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAudioSeparationEffectVolume_001
 * @tc.desc  : Test SetAudioSeparationEffectVolume interface.
 */
HWTEST(AudioEffectManagerUnitTest, SetAudioSeparationEffectVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectVolume_001 start");
    int32_t result = AudioEffectManager::GetInstance()
        ->SetAudioSeparationEffectVolume(AUDIO_SEPARATION_VOLUME_TYPE_DEFAULT, 1.0);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectVolume_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test OnAudioSeparationEffectEnabledChange API
 * @tc.type  : FUNC
 * @tc.number: OnAudioSeparationEffectEnabledChange_001
 * @tc.desc  : Test OnAudioSeparationEffectEnabledChange interface.
 */
HWTEST(AudioEffectManagerUnitTest, OnAudioSeparationEffectEnabledChange_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest OnAudioSeparationEffectEnabledChange_001 start");
    auto callback = std::make_shared<MockSeparationEffectCallback>();
    int32_t result = AudioEffectManager::GetInstance()->OnAudioSeparationEffectEnabledChange(callback);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest OnAudioSeparationEffectEnabledChange_001 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test OffAudioSeparationEffectEnabledChange API
 * @tc.type  : FUNC
 * @tc.number: OffAudioSeparationEffectEnabledChange_001
 * @tc.desc  : Test OffAudioSeparationEffectEnabledChange interface.
 */
HWTEST(AudioEffectManagerUnitTest, OffAudioSeparationEffectEnabledChange_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest OffAudioSeparationEffectEnabledChange_001 start");
    int32_t result = AudioEffectManager::GetInstance()->OffAudioSeparationEffectEnabledChange();
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest OffAudioSeparationEffectEnabledChange_001 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test SetAudioSeparationEffectVolume API with invalid volume
 * @tc.type  : FUNC
 * @tc.number: SetAudioSeparationEffectVolume_002
 * @tc.desc  : Test SetAudioSeparationEffectVolume with invalid volume value.
 */
HWTEST(AudioEffectManagerUnitTest, SetAudioSeparationEffectVolume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectVolume_002 start");
    // Test with invalid volume (negative value)
    int32_t result = AudioEffectManager::GetInstance()->SetAudioSeparationEffectVolume(
        AUDIO_SEPARATION_VOLUME_TYPE_DEFAULT, -1.0);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectVolume_002 result:%{public}d", result);
    // Should return error for invalid volume
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test SetAudioSeparationEffectVolume API with volume out of range
 * @tc.type  : FUNC
 * @tc.number: SetAudioSeparationEffectVolume_003
 * @tc.desc  : Test SetAudioSeparationEffectVolume with volume > 1.0.
 */
HWTEST(AudioEffectManagerUnitTest, SetAudioSeparationEffectVolume_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectVolume_003 start");
    // Test with invalid volume (greater than 1.0)
    int32_t result = AudioEffectManager::GetInstance()->SetAudioSeparationEffectVolume(
        AUDIO_SEPARATION_VOLUME_TYPE_DEFAULT, 2.0);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectVolume_003 result:%{public}d", result);
    // Should return error for invalid volume
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name   : Test SetAudioSeparationEffectEnabled API with invalid parameters
 * @tc.type  : FUNC
 * @tc.number: SetAudioSeparationEffectEnabled_002
 * @tc.desc  : Test SetAudioSeparationEffectEnabled with invalid uid.
 */
HWTEST(AudioEffectManagerUnitTest, SetAudioSeparationEffectEnabled_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectEnabled_002 start");
    // Test with invalid uid (negative)
    int32_t result = AudioEffectManager::GetInstance()->SetAudioSeparationEffectEnabled(true, -1, 0);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest SetAudioSeparationEffectEnabled_002 result:%{public}d", result);
    // Result depends on AudioPolicyManager
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test OnAudioSeparationEffectEnabledChange API with null callback
 * @tc.type  : FUNC
 * @tc.number: OnAudioSeparationEffectEnabledChange_002
 * @tc.desc  : Test OnAudioSeparationEffectEnabledChange with null callback.
 */
HWTEST(AudioEffectManagerUnitTest, OnAudioSeparationEffectEnabledChange_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest OnAudioSeparationEffectEnabledChange_002 start");
    // Test with null callback
    std::shared_ptr<AudioSeparationEffectEnabledChangeCallback> nullCallback = nullptr;
    int32_t result = AudioEffectManager::GetInstance()->OnAudioSeparationEffectEnabledChange(nullCallback);
    AUDIO_INFO_LOG("AudioEffectManagerUnitTest OnAudioSeparationEffectEnabledChange_002 result:%{public}d", result);
    // Should return error for null callback
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

} // namespace AudioStandard
} // namespace OHOS
