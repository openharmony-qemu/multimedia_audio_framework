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

#include <gtest/gtest.h>
#include <cinttypes>

#include "audio_log.h"
#include "audio_system_manager.h"
#include "audio_errors.h"
#include "audio_device_descriptor.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioSystemManagerSupplementTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override {}
    void TearDown() override {}
};

// ==================== Microphone Mute Tests ====================

/**
 * @tc.name  : Test IsMicrophoneMute API
 * @tc.type  : FUNC
 * @tc.number: IsMicrophoneMute_001
 * @tc.desc  : Test IsMicrophoneMute interface.
 */
HWTEST(AudioSystemManagerSupplementTest, IsMicrophoneMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest IsMicrophoneMute_001 start");
    bool isMute = AudioSystemManager::GetInstance()->IsMicrophoneMute();
    AUDIO_INFO_LOG("IsMicrophoneMute_001 isMute:%{public}d", isMute);
    SUCCEED();
}

// ==================== AudioMonoState and AudioBalanceValue Tests ====================

/**
 * @tc.name  : Test SetAudioMonoState API
 * @tc.type  : FUNC
 * @tc.number: SetAudioMonoState_001
 * @tc.desc  : Test SetAudioMonoState interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SetAudioMonoState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetAudioMonoState_001 start");
    AudioSystemManager::GetInstance()->SetAudioMonoState(true);
    AudioSystemManager::GetInstance()->SetAudioMonoState(false);
    SUCCEED();
}

/**
 * @tc.name  : Test SetAudioBalanceValue API
 * @tc.type  : FUNC
 * @tc.number: SetAudioBalanceValue_001
 * @tc.desc  : Test SetAudioBalanceValue interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SetAudioBalanceValue_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetAudioBalanceValue_001 start");
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(0.5f);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(-1.0f);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(2.0f);
    SUCCEED();
}

// ==================== GetTransactionId Tests ====================

/**
 * @tc.name  : Test GetTransactionId API
 * @tc.type  : FUNC
 * @tc.number: GetTransactionId_001
 * @tc.desc  : Test GetTransactionId interface.
 */
HWTEST(AudioSystemManagerSupplementTest, GetTransactionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GetTransactionId_001 start");
    uint64_t transactionId = AudioSystemManager::GetInstance()->GetTransactionId(
        DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    AUDIO_INFO_LOG("GetTransactionId_001 transactionId:%{public}" PRIu64, transactionId);
    EXPECT_TRUE(transactionId >= 0);
}

// ==================== OnVoiceWakeupState Tests ====================

/**
 * @tc.name  : Test OnVoiceWakeupState API
 * @tc.type  : FUNC
 * @tc.number: OnVoiceWakeupState_001
 * @tc.desc  : Test OnVoiceWakeupState interface.
 */
HWTEST(AudioSystemManagerSupplementTest, OnVoiceWakeupState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest OnVoiceWakeupState_001 start");
    int32_t result = AudioSystemManager::GetInstance()->OnVoiceWakeupState(true);
    AUDIO_INFO_LOG("OnVoiceWakeupState_001 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);

    result = AudioSystemManager::GetInstance()->OnVoiceWakeupState(false);
    EXPECT_EQ(result, SUCCESS);
}

// ==================== GenerateSessionId Tests ====================

/**
 * @tc.name  : Test GenerateSessionId API
 * @tc.type  : FUNC
 * @tc.number: GenerateSessionId_001
 * @tc.desc  : Test GenerateSessionId interface.
 */
HWTEST(AudioSystemManagerSupplementTest, GenerateSessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GenerateSessionId_001 start");
    uint32_t sessionId = 0;
    int32_t result = AudioSystemManager::GetInstance()->GenerateSessionId(sessionId);
    AUDIO_INFO_LOG("GenerateSessionId_001 result:%{public}d, sessionId:%{public}u", result, sessionId);
    EXPECT_EQ(result, SUCCESS);
}

// ==================== GetEffectLatency Tests ====================

/**
 * @tc.name  : Test GetEffectLatency API
 * @tc.type  : FUNC
 * @tc.number: GetEffectLatency_001
 * @tc.desc  : Test GetEffectLatency interface.
 */
HWTEST(AudioSystemManagerSupplementTest, GetEffectLatency_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GetEffectLatency_001 start");
    uint32_t latency = AudioSystemManager::GetInstance()->GetEffectLatency("test_session");
    AUDIO_INFO_LOG("GetEffectLatency_001 latency:%{public}u", latency);
    EXPECT_TRUE(latency >= 0);
}

// ==================== CleanUpResource Tests ====================

/**
 * @tc.name  : Test CleanUpResource API
 * @tc.type  : FUNC
 * @tc.number: CleanUpResource_001
 * @tc.desc  : Test CleanUpResource interface.
 */
HWTEST(AudioSystemManagerSupplementTest, CleanUpResource_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest CleanUpResource_001 start");
    AudioSystemManager::GetInstance()->CleanUpResource();
    SUCCEED();
}

} // namespace AudioStandard
} // namespace OHOS
