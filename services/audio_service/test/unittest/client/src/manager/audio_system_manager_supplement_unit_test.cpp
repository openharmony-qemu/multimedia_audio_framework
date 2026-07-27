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

// ==================== AudioParameter Tests ====================

/**
 * @tc.name  : Test GetAudioParameter API
 * @tc.type  : FUNC
 * @tc.number: GetAudioParameter_001
 * @tc.desc  : Test GetAudioParameter with empty key.
 */
HWTEST(AudioSystemManagerSupplementTest, GetAudioParameter_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GetAudioParameter_001 start");
    std::string result = AudioSystemManager::GetInstance()->GetAudioParameter("");
    AUDIO_INFO_LOG("GetAudioParameter_001 result:%{public}s", result.c_str());
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : Test SetAudioParameter API
 * @tc.type  : FUNC
 * @tc.number: SetAudioParameter_001
 * @tc.desc  : Test SetAudioParameter with valid key-value.
 */
HWTEST(AudioSystemManagerSupplementTest, SetAudioParameter_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetAudioParameter_001 start");
    AudioSystemManager::GetInstance()->SetAudioParameter("test_key", "test_value");
    std::string result = AudioSystemManager::GetInstance()->GetAudioParameter("test_key");
    AUDIO_INFO_LOG("SetAudioParameter_001 result:%{public}s", result.c_str());
    EXPECT_TRUE(result.empty());
}

// ==================== Microphone Mute Tests ====================

/**
 * @tc.name  : Test SetMicrophoneMute API
 * @tc.type  : FUNC
 * @tc.number: SetMicrophoneMute_001
 * @tc.desc  : Test SetMicrophoneMute interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SetMicrophoneMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetMicrophoneMute_001 start");
    int32_t result = AudioSystemManager::GetInstance()->SetMicrophoneMute(true);
    AUDIO_INFO_LOG("SetMicrophoneMute_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

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

/**
 * @tc.name  : Test SetVoiceRingtoneMute API
 * @tc.type  : FUNC
 * @tc.number: SetVoiceRingtoneMute_001
 * @tc.desc  : Test SetVoiceRingtoneMute interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SetVoiceRingtoneMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetVoiceRingtoneMute_001 start");
    int32_t result = AudioSystemManager::GetInstance()->SetVoiceRingtoneMute(true);
    AUDIO_INFO_LOG("SetVoiceRingtoneMute_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

// ==================== GetSelfBundleName Tests ====================

/**
 * @tc.name  : Test GetSelfBundleName with uid API
 * @tc.type  : FUNC
 * @tc.number: GetSelfBundleName_002
 * @tc.desc  : Test GetSelfBundleName(uid) interface.
 */
HWTEST(AudioSystemManagerSupplementTest, GetSelfBundleName_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GetSelfBundleName_002 start");
    std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName(-1);
    AUDIO_INFO_LOG("GetSelfBundleName_002 bundleName:%{public}s", bundleName.c_str());
    EXPECT_TRUE(bundleName.empty());
}

/**
 * @tc.name  : Test GetSelfBundleName without uid API
 * @tc.type  : FUNC
 * @tc.number: GetSelfBundleName_003
 * @tc.desc  : Test GetSelfBundleName() interface.
 */
HWTEST(AudioSystemManagerSupplementTest, GetSelfBundleName_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GetSelfBundleName_003 start");
    std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName();
    AUDIO_INFO_LOG("GetSelfBundleName_003 bundleName:%{public}s", bundleName.c_str());
    EXPECT_TRUE(bundleName.empty());
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

// ==================== SystemSoundUri Tests ====================

/**
 * @tc.name  : Test SetSystemSoundUri and GetSystemSoundUri API
 * @tc.type  : FUNC
 * @tc.number: SystemSoundUri_001
 * @tc.desc  : Test SetSystemSoundUri and GetSystemSoundUri interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SystemSoundUri_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SystemSoundUri_001 start");
    std::string key = "test_sound_key";
    std::string uri = "test_sound_uri";
    int32_t result = AudioSystemManager::GetInstance()->SetSystemSoundUri(key, uri);
    AUDIO_INFO_LOG("SystemSoundUri_001 Set result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);

    std::string getUri = AudioSystemManager::GetInstance()->GetSystemSoundUri(key);
    AUDIO_INFO_LOG("SystemSoundUri_001 Get result:%{public}s", getUri.c_str());
}

/**
 * @tc.name  : Test GetSystemSoundPath API
 * @tc.type  : FUNC
 * @tc.number: GetSystemSoundPath_001
 * @tc.desc  : Test GetSystemSoundPath interface.
 */
HWTEST(AudioSystemManagerSupplementTest, GetSystemSoundPath_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest GetSystemSoundPath_001 start");
    std::string path = AudioSystemManager::GetInstance()->GetSystemSoundPath(0);
    AUDIO_INFO_LOG("GetSystemSoundPath_001 path:%{public}s", path.c_str());
    EXPECT_FALSE(path.empty());
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

// ==================== VirtualCall Tests ====================

/**
 * @tc.name  : Test SetVirtualCall and GetVirtualCall API
 * @tc.type  : FUNC
 * @tc.number: VirtualCall_001
 * @tc.desc  : Test SetVirtualCall and GetVirtualCall interface.
 */
HWTEST(AudioSystemManagerSupplementTest, VirtualCall_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest VirtualCall_001 start");
    int32_t setResult = AudioSystemManager::GetInstance()->SetVirtualCall(true);
    AUDIO_INFO_LOG("VirtualCall_001 Set result:%{public}d", setResult);
    EXPECT_NE(setResult, SUCCESS);

    bool isVirtual = AudioSystemManager::GetInstance()->GetVirtualCall();
    AUDIO_INFO_LOG("VirtualCall_001 Get result:%{public}d", isVirtual);
    SUCCEED();

    setResult = AudioSystemManager::GetInstance()->SetVirtualCall(false);
    EXPECT_NE(setResult, SUCCESS);
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
    EXPECT_NE(result, SUCCESS);
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

// ==================== SetSplitModeReady and LoadSplitModule Tests ====================

/**
 * @tc.name  : Test SetSplitModeReady API
 * @tc.type  : FUNC
 * @tc.number: SetSplitModeReady_001
 * @tc.desc  : Test SetSplitModeReady interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SetSplitModeReady_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetSplitModeReady_001 start");
    int32_t result = AudioSystemManager::GetInstance()->SetSplitModeReady();
    AUDIO_INFO_LOG("SetSplitModeReady_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test LoadSplitModule API
 * @tc.type  : FUNC
 * @tc.number: LoadSplitModule_001
 * @tc.desc  : Test LoadSplitModule interface.
 */
HWTEST(AudioSystemManagerSupplementTest, LoadSplitModule_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest LoadSplitModule_001 start");
    int32_t result = AudioSystemManager::GetInstance()->LoadSplitModule("test_args", "test_network");
    AUDIO_INFO_LOG("LoadSplitModule_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
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

// ==================== SetRenderWhitelist Tests ====================

/**
 * @tc.name  : Test SetRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: SetRenderWhitelist_001
 * @tc.desc  : Test SetRenderWhitelist interface.
 */
HWTEST(AudioSystemManagerSupplementTest, SetRenderWhitelist_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerSupplementTest SetRenderWhitelist_001 start");
    std::vector<std::string> whitelist;
    whitelist.push_back("com.test.app1");
    whitelist.push_back("com.test.app2");
    int32_t result = AudioSystemManager::GetInstance()->SetRenderWhitelist(whitelist);
    AUDIO_INFO_LOG("SetRenderWhitelist_001 result:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
