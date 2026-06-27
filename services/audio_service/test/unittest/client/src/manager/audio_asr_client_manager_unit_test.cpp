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

#include "audio_asr_client_manager.h"
#include "audio_asr.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioAsrClientManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioAsrClientManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetUpTestCase");
}

void AudioAsrClientManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT TearDownTestCase");
}

void AudioAsrClientManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetUp");
}

void AudioAsrClientManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT TearDown");
}

/**
 * @tc.name   : Test SetAsrAecMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrAecMode_001
 * @tc.desc  : Test SetAsrAecMode with BYPASS mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrAecMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrAecMode_001 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrAecMode(AsrAecMode::BYPASS);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrAecMode_001 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetAsrAecMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrAecMode_002
 * @tc.desc  : Test SetAsrAecMode with STANDARD mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrAecMode_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrAecMode_002 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrAecMode(AsrAecMode::STANDARD);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrAecMode_002 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test GetAsrAecMode API
 * @tc.type  : FUNC
 * @tc.number: GetAsrAecMode_001
 * @tc.desc  : Test GetAsrAecMode interface.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, GetAsrAecMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT GetAsrAecMode_001 start");
    AsrAecMode mode = AsrAecMode::BYPASS;
    int32_t result = AudioAsrClientManager::GetInstance().GetAsrAecMode(mode);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT GetAsrAecMode_001 result:%{public}d, mode:%{public}d",
        result, static_cast<int32_t>(mode));
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
    (void)mode;
}

/**
 * @tc.name   : Test SetAsrNoiseSuppressionMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrNoiseSuppressionMode_001
 * @tc.desc  : Test SetAsrNoiseSuppressionMode with BYPASS mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrNoiseSuppressionMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrNoiseSuppressionMode_001 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode::BYPASS);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrNoiseSuppressionMode_001 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetAsrNoiseSuppressionMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrNoiseSuppressionMode_002
 * @tc.desc  : Test SetAsrNoiseSuppressionMode with STANDARD mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrNoiseSuppressionMode_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrNoiseSuppressionMode_002 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode::STANDARD);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrNoiseSuppressionMode_002 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test GetAsrNoiseSuppressionMode API
 * @tc.type  : FUNC
 * @tc.number: GetAsrNoiseSuppressionMode_001
 * @tc.desc  : Test GetAsrNoiseSuppressionMode interface.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, GetAsrNoiseSuppressionMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT GetAsrNoiseSuppressionMode_001 start");
    AsrNoiseSuppressionMode mode = AsrNoiseSuppressionMode::BYPASS;
    int32_t result = AudioAsrClientManager::GetInstance().GetAsrNoiseSuppressionMode(mode);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT GetAsrNoiseSuppressionMode_001 result:%{public}d, mode:%{public}d",
        result, static_cast<int32_t>(mode));
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
    (void)mode;
}

/**
 * @tc.name   : Test SetAsrWhisperDetectionMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrWhisperDetectionMode_001
 * @tc.desc  : Test SetAsrWhisperDetectionMode with BYPASS mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrWhisperDetectionMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrWhisperDetectionMode_001 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrWhisperDetectionMode(AsrWhisperDetectionMode::BYPASS);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrWhisperDetectionMode_001 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetAsrWhisperDetectionMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrWhisperDetectionMode_002
 * @tc.desc  : Test SetAsrWhisperDetectionMode with STANDARD mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrWhisperDetectionMode_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrWhisperDetectionMode_002 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrWhisperDetectionMode(AsrWhisperDetectionMode::STANDARD);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrWhisperDetectionMode_002 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test GetAsrWhisperDetectionMode API
 * @tc.type  : FUNC
 * @tc.number: GetAsrWhisperDetectionMode_001
 * @tc.desc  : Test GetAsrWhisperDetectionMode interface.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, GetAsrWhisperDetectionMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT GetAsrWhisperDetectionMode_001 start");
    AsrWhisperDetectionMode mode = AsrWhisperDetectionMode::BYPASS;
    int32_t result = AudioAsrClientManager::GetInstance().GetAsrWhisperDetectionMode(mode);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT GetAsrWhisperDetectionMode_001 result:%{public}d, mode:%{public}d",
        result, static_cast<int32_t>(mode));
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
    (void)mode;
}

/**
 * @tc.name   : Test SetAsrVoiceControlMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrVoiceControlMode_001
 * @tc.desc  : Test SetAsrVoiceControlMode with AUDIO_2_VOICETX mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrVoiceControlMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrVoiceControlMode_001 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrVoiceControlMode(AsrVoiceControlMode::AUDIO_2_VOICETX,
        true);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrVoiceControlMode_001 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetAsrVoiceMuteMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrVoiceMuteMode_001
 * @tc.desc  : Test SetAsrVoiceMuteMode with AUDIO_2_VOICETX mode.
 */
HWTEST_F(AudioAsrClientManagerUnitTest, SetAsrVoiceMuteMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrVoiceMuteMode_001 start");
    int32_t result = AudioAsrClientManager::GetInstance().SetAsrVoiceMuteMode(AsrVoiceMuteMode::OUTPUT_MUTE,
        true);
    AUDIO_INFO_LOG("AudioAsrCliMgrUT SetAsrVoiceMuteMode_001 result:%{public}d", result);
    // Result depends on AudioServiceProxy availability
    // Just check function doesn't crash
    (void)result;
}

} // namespace AudioStandard
} // namespace OHOS
