/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_background_manager_unit_test.h"
#include "audio_common_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioBackgroundManagerUnitTest::SetUp(void) {}

void AudioBackgroundManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test SubscribeBackgroundTask API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_001
 * @tc.desc  : Test SubscribeBackgroundTask
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_001, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    audioBackgroundManagerTest_->backgroundTaskListener_ = nullptr;
    audioBackgroundManagerTest_->SubscribeBackgroundTask();
    ASSERT_TRUE(audioBackgroundManagerTest_->backgroundTaskListener_ != nullptr);
}

/**
 * @tc.name  : Test SubscribeBackgroundTask API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_002
 * @tc.desc  : Test SubscribeBackgroundTask
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_002, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    audioBackgroundManagerTest_->backgroundTaskListener_ = std::make_shared<BackgroundTaskListener>();

    audioBackgroundManagerTest_->SubscribeBackgroundTask();
    ASSERT_TRUE(audioBackgroundManagerTest_->backgroundTaskListener_ != nullptr);
}


/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_003
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_003, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(0, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_004
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_004, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, 1);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_005
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_005, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isBack = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, 1);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_006
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_006, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isBack = true;
    appState.hasBackTask = true;
    appState.hasSession = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_007
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_007, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isBack = true;
    appState.hasBackTask = false;
    appState.isBinder = true;
    appState.hasSession = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_008
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_008, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isBack = true;
    appState.hasBackTask = false;
    appState.isBinder = false;
    appState.hasSession = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_009
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_009, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isBack = true;
    appState.hasBackTask = false;
    appState.isBinder = false;
    appState.hasSession = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsAllowedPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_010
 * @tc.desc  : Test IsAllowedPlayback
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_010, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isBack = true;
    appState.hasBackTask = false;
    appState.isBinder = true;
    appState.hasSession = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);

    bool ret = audioBackgroundManagerTest_->IsAllowedPlayback(1, pid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_011
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_011, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_END;
    AppState appState;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_012
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_012, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_FOREGROUND;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, false);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_013
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_013, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_BACKGROUND;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_014
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_014, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_BACKGROUND;

    AppState appState;
    appState.isBack = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_015
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_015, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_BACKGROUND;

    AppState appState;
    appState.isBack = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_016
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_016, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_FOREGROUND;

    AppState appState;
    appState.isBack = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, false);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_017
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_017, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_BACKGROUND;

    AppState appState;
    appState.isBack = false;
    appState.hasSession = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    VolumeUtils::SetPCVolumeEnable(false);
    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_018
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_018, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_BACKGROUND;

    AppState appState;
    appState.isBack = false;
    appState.hasSession = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    VolumeUtils::SetPCVolumeEnable(false);
    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_019
 * @tc.desc  : Test NotifyAppStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_019, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppIsBackState state = STATE_BACKGROUND;

    AppState appState;
    appState.isBack = false;
    appState.hasSession = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    VolumeUtils::SetPCVolumeEnable(true);
    audioBackgroundManagerTest_->NotifyAppStateChange(uid, pid, state);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test NotifyBackgroundTaskStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_020
 * @tc.desc  : Test NotifyBackgroundTaskStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_020, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasBackgroundTask = true;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);
    
    audioBackgroundManagerTest_->NotifyBackgroundTaskStateChange(uid, pid, hasBackgroundTask);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, true);
}

/**
 * @tc.name  : Test NotifyBackgroundTaskStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_021
 * @tc.desc  : Test NotifyBackgroundTaskStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_021, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasBackgroundTask = true;
    AppState appState;
    appState.hasBackTask = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    
    audioBackgroundManagerTest_->NotifyBackgroundTaskStateChange(uid, pid, hasBackgroundTask);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, true);
}

/**
 * @tc.name  : Test NotifyBackgroundTaskStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_022
 * @tc.desc  : Test NotifyBackgroundTaskStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_022, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasBackgroundTask = true;
    AppState appState;
    appState.hasBackTask = false;
    appState.isFreeze = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    
    audioBackgroundManagerTest_->NotifyBackgroundTaskStateChange(uid, pid, hasBackgroundTask);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, true);
}

/**
 * @tc.name  : Test NotifyBackgroundTaskStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_023
 * @tc.desc  : Test NotifyBackgroundTaskStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_023, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasBackgroundTask = true;
    AppState appState;
    appState.hasBackTask = false;
    appState.isFreeze = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    
    audioBackgroundManagerTest_->NotifyBackgroundTaskStateChange(uid, pid, hasBackgroundTask);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, true);
}

/**
 * @tc.name  : Test NotifyBackgroundTaskStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_024
 * @tc.desc  : Test NotifyBackgroundTaskStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_024, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasBackgroundTask = false;
    AppState appState;
    appState.hasBackTask = true;
    appState.isFreeze = false;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    
    audioBackgroundManagerTest_->NotifyBackgroundTaskStateChange(uid, pid, hasBackgroundTask);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, false);
}

/**
 * @tc.name  : Test NotifySessionStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_025
 * @tc.desc  : Test NotifySessionStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_025, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasSession = false;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);
    
    int32_t ret = audioBackgroundManagerTest_->NotifySessionStateChange(uid, pid, hasSession);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasSession, false);
}

/**
 * @tc.name  : Test NotifySessionStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_026
 * @tc.desc  : Test NotifySessionStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_026, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    bool hasSession = false;
    AppState appState;
    appState.hasSession = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    
    int32_t ret = audioBackgroundManagerTest_->NotifySessionStateChange(uid, pid, hasSession);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasSession, false);
}

/**
 * @tc.name  : Test HandleSessionStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_027
 * @tc.desc  : Test HandleSessionStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_027, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 1003;
    AppState appState;
    appState.hasSession = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    
    audioBackgroundManagerTest_->HandleSessionStateChange(uid, pid);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasSession, true);
}

/**
 * @tc.name  : Test HandleSessionStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_028
 * @tc.desc  : Test HandleSessionStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_028, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.hasSession = false;
    appState.isBack = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    VolumeUtils::SetPCVolumeEnable(true);

    audioBackgroundManagerTest_->HandleSessionStateChange(uid, pid);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBack, true);
}

/**
 * @tc.name  : Test HandleSessionStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_029
 * @tc.desc  : Test HandleSessionStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_029, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.hasSession = false;
    appState.isBack = true;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    VolumeUtils::SetPCVolumeEnable(false);

    audioBackgroundManagerTest_->HandleSessionStateChange(uid, pid);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasSession, false);
}

/**
 * @tc.name  : Test NotifyFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_030
 * @tc.desc  : Test NotifyFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_030, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    std::set<int32_t> pidList;
    bool isFreeze = false;

    int32_t ret = audioBackgroundManagerTest_->NotifyFreezeStateChange(pidList, isFreeze);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifyFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_031
 * @tc.desc  : Test NotifyFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_031, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    std::set<int32_t> pidList;
    bool isFreeze = false;
    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isFreeze = true;
    pidList.insert(pid);
    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    int32_t ret = audioBackgroundManagerTest_->NotifyFreezeStateChange(pidList, isFreeze);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isFreeze, false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBinder, true);
}

/**
 * @tc.name  : Test NotifyFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_032
 * @tc.desc  : Test NotifyFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_032, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    std::set<int32_t> pidList;
    bool isFreeze = false;
    int32_t pid = 0;
    pidList.insert(pid);
    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    int32_t ret = audioBackgroundManagerTest_->NotifyFreezeStateChange(pidList, isFreeze);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isFreeze, false);
}

/**
 * @tc.name  : Test ResetAllProxy API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_033
 * @tc.desc  : Test ResetAllProxy
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_033, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    int32_t ret = audioBackgroundManagerTest_->ResetAllProxy();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ResetAllProxy API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_034
 * @tc.desc  : Test ResetAllProxy
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_034, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.isFreeze = true;
    appState.isBinder = true;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    int32_t ret = audioBackgroundManagerTest_->ResetAllProxy();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isFreeze, false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].isBinder, false);
}

/**
 * @tc.name  : Test HandleFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_035
 * @tc.desc  : Test HandleFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_035, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.hasBackTask = true;
    bool isFreeze = true;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->HandleFreezeStateChange(pid, isFreeze);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, true);
}

/**
 * @tc.name  : Test HandleFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_036
 * @tc.desc  : Test HandleFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_036, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.hasBackTask = false;
    bool isFreeze = true;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->HandleFreezeStateChange(pid, isFreeze);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, false);
}

/**
 * @tc.name  : Test HandleFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_037
 * @tc.desc  : Test HandleFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_037, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.hasBackTask = false;
    bool isFreeze = false;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->HandleFreezeStateChange(pid, isFreeze);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, false);
}

/**
 * @tc.name  : Test HandleFreezeStateChange API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_038
 * @tc.desc  : Test HandleFreezeStateChange
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_038, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;
    appState.hasBackTask = true;
    bool isFreeze = false;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->HandleFreezeStateChange(pid, isFreeze);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_[pid].hasBackTask, true);
}

/**
 * @tc.name  : Test DeleteFromMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_039
 * @tc.desc  : Test DeleteFromMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_039, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->DeleteFromMap(1000);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
}

/**
 * @tc.name  : Test DeleteFromMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_040
 * @tc.desc  : Test DeleteFromMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_040, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    audioBackgroundManagerTest_->DeleteFromMap(pid);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);
}

/**
 * @tc.name  : Test DeleteFromMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_041
 * @tc.desc  : Test DeleteFromMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_041, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    audioBackgroundManagerTest_->DeleteFromMap(pid);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);
}

/**
 * @tc.name  : Test FindKeyInMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_042
 * @tc.desc  : Test FindKeyInMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_042, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    bool ret = audioBackgroundManagerTest_->FindKeyInMap(pid);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
}

/**
 * @tc.name  : Test FindKeyInMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_043
 * @tc.desc  : Test FindKeyInMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_043, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    bool ret = audioBackgroundManagerTest_->FindKeyInMap(pid);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test FindKeyInMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_044
 * @tc.desc  : Test FindKeyInMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_044, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    int32_t uid = 0;
    AppState appState;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMap(pid, uid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    bool ret = audioBackgroundManagerTest_->FindKeyInMap(1000);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
}

/**
 * @tc.name  : Test WriteAppStateChangeSysEvent API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_045
 * @tc.desc  : Test WriteAppStateChangeSysEvent
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_045, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    AppState appState;
    appState.isFreeze = 1;
    appState.isBack = 1;
    appState.hasSession = 1;
    appState.hasBackTask = 1;
    appState.isBinder = 1;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), true);

    audioBackgroundManagerTest_->WriteAppStateChangeSysEvent(pid, appState, 1);
    sleep(1000);

    audioBackgroundManagerTest_->RecoryAppState();
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.size(), 1);

    auto it = audioBackgroundManagerTest_->appStatesMap_.find(pid);
    EXPECT_EQ(it != audioBackgroundManagerTest_->appStatesMap_.end(), true);

    AppState recoryState = it->second;
    EXPECT_EQ(appState.isFreeze, recoryState.isFreeze);
    EXPECT_EQ(appState.isBack, recoryState.isBack);
    EXPECT_EQ(appState.hasSession, recoryState.hasSession);
    EXPECT_EQ(appState.hasBackTask, recoryState.hasBackTask);
    EXPECT_EQ(appState.isBinder, recoryState.isBinder);
}

/**
 * @tc.name  : Test FindKeyInMap API
 * @tc.type  : FUNC
 * @tc.number: AudioBackgroundManager_046
 * @tc.desc  : Test FindKeyInMap
 */
HWTEST(AudioBackgroundManagerUnitTest, AudioBackgroundManager_046, TestSize.Level1)
{
    AudioBackgroundManager* audioBackgroundManagerTest_ = nullptr;
    audioBackgroundManagerTest_ = &AudioBackgroundManager::GetInstance();
    ASSERT_TRUE(audioBackgroundManagerTest_ != nullptr);

    int32_t pid = 0;
    AppState appState;

    audioBackgroundManagerTest_->appStatesMap_.clear();
    audioBackgroundManagerTest_->InsertIntoAppStatesMapWithoutUid(pid, appState);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);

    bool ret = audioBackgroundManagerTest_->FindKeyInMap(pid);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(audioBackgroundManagerTest_->appStatesMap_.empty(), false);
}
} // namespace AudioStandard
} // namespace OHOS
