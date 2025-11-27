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

#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_session_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioSessionManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test ActivateAudioSession API
 * @tc.type  : FUNC
 * @tc.number: ActivateAudioSession_001
 * @tc.desc  : Test ActivateAudioSession interface.
 */
HWTEST(AudioSessionManagerUnitTest, ActivateAudioSession_001, TestSize.Level1)
{
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;

    int32_t result = AudioSessionManager::GetInstance()->ActivateAudioSession(strategy);
    EXPECT_EQ(result, SUCCESS);

    result = AudioSessionManager::GetInstance()->SetDefaultOutputDevice(DEVICE_TYPE_INVALID);
    EXPECT_EQ(result, ERROR_INVALID_PARAM);

    result = AudioSessionManager::GetInstance()->ActivateAudioSession(strategy);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test UnsetAudioSessionStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: UnsetAudioSessionStateChangeCallback_001
 * @tc.desc  : Test UnsetAudioSessionStateChangeCallback interface.
 */
HWTEST(AudioSessionManagerUnitTest, UnsetAudioSessionStateChangeCallback_001, TestSize.Level1)
{
    int32_t result = AudioSessionManager::GetInstance()->UnsetAudioSessionStateChangeCallback();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test UnsetAudioSessionCurrentDeviceChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: UnsetAudioSessionCurrentDeviceChangeCallback_001
 * @tc.desc  : Test UnsetAudioSessionCurrentDeviceChangeCallback interface.
 */
HWTEST(AudioSessionManagerUnitTest, UnsetAudioSessionCurrentDeviceChangeCallback_001, TestSize.Level1)
{
    int32_t result = AudioSessionManager::GetInstance()->UnsetAudioSessionCurrentDeviceChangeCallback();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioSessionRestoreParams class
 * @tc.type  : FUNC
 * @tc.number: AudioSessionRestoreParams_001
 * @tc.desc  : Test AudioSessionRestoreParams class interface.
 */
HWTEST(AudioSessionManagerUnitTest, AudioSessionRestoreParams_001, TestSize.Level1)
{
    AudioSessionRestoreParams restoreParams_;
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 0);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 1);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_SET_SCENE, 1);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_SET_SCENE, 0);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 1);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 1);
    EXPECT_EQ(restoreParams_.actions_.size(), 2);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_SET_SCENE, 1);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_SET_SCENE, 1);
    EXPECT_EQ(restoreParams_.actions_.size(), 3);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 0);
    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 0);
    EXPECT_EQ(restoreParams_.actions_.size(), 2);

    restoreParams_.OnAudioSessionStateChanged(AudioSessionStateChangeHint::INVALID);
    restoreParams_.OnAudioSessionStateChanged(AudioSessionStateChangeHint::STOP);
    restoreParams_.OnAudioSessionStateChanged(AudioSessionStateChangeHint::TIME_OUT_STOP);
    restoreParams_.OnAudioSessionStateChanged(AudioSessionStateChangeHint::PAUSE);
    EXPECT_EQ(restoreParams_.actions_.size(), 0);

    restoreParams_.RecordAudioSessionOpt(AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 0);
    restoreParams_.OnAudioSessionDeactive();
    EXPECT_EQ(restoreParams_.actions_.size(), 0);
}

/**
 * @tc.name  : Test OnAudioPolicyServiceDied api
 * @tc.type  : FUNC
 * @tc.number: AudioSessionManagerServiceDiedRestore_001
 * @tc.desc  : Test OnAudioPolicyServiceDied interface.
 */
HWTEST(AudioSessionManagerUnitTest, AudioSessionManagerServiceDiedRestore_001, TestSize.Level1)
{
    AudioSessionManagerServiceDiedRestore restore;

    AudioSessionManager::GetInstance()->restoreParams_.actions_.clear();
    AudioSessionManager::GetInstance()->setDefaultOutputDevice_ = false;
    restore.OnAudioPolicyServiceDied();

    AudioSessionManager::GetInstance()->setDefaultOutputDevice_ = true;
    AudioSessionManager::GetInstance()->setDeviceType_ = DEVICE_TYPE_DEFAULT;
    AudioSessionManager::GetInstance()->restoreParams_.RecordAudioSessionOpt(
        AudioSessionRestoreParams::OperationType::AUDIO_SESSION_ACTIVATE, 0);
    restore.OnAudioPolicyServiceDied();
    EXPECT_EQ(AudioSessionManager::GetInstance()->restoreParams_.actions_.size(), 1);
}

} // namespace AudioStandard
} // namespace OHOS