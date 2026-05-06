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

#include "sco_audio_scene_manager_unit_test.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void ScoAudioSceneManagerTest::SetUp()
{
    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
}

void ScoAudioSceneManagerTest::TearDown()
{
    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
}

/**
 * @tc.name  : ScoAudioSceneManager_AddScoStream_001
 * @tc.desc  : Test AddScoStream with cellular call scene
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, AddScoStream_001, TestSize.Level1)
{
    uint32_t streamId = 100001;
    AudioScene audioScene = AUDIO_SCENE_PHONE_CALL;
    int32_t uid = 1001;
    bool isRecognition = false;
    
    ScoAudioSceneManager::GetInstance().AddScoStream(streamId, audioScene, uid, isRecognition);
    
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_PHONE_CALL);
}

/**
 * @tc.name  : ScoAudioSceneManager_AddScoStream_002
 * @tc.desc  : Test AddScoStream with voip scene
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, AddScoStream_002, TestSize.Level1)
{
    uint32_t streamId = 100002;
    AudioScene audioScene = AUDIO_SCENE_PHONE_CHAT;
    int32_t uid = 1002;
    bool isRecognition = false;
    
    ScoAudioSceneManager::GetInstance().AddScoStream(streamId, audioScene, uid, isRecognition);
    
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_PHONE_CHAT);
}

/**
 * @tc.name  : ScoAudioSceneManager_AddScoStream_003
 * @tc.desc  : Test AddScoStream with recognition scene
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, AddScoStream_003, TestSize.Level1)
{
    uint32_t streamId = 100003;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    int32_t uid = 1003;
    bool isRecognition = true;
    
    ScoAudioSceneManager::GetInstance().AddScoStream(streamId, audioScene, uid, isRecognition);
    
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name  : ScoAudioSceneManager_AddScoStream_004
 * @tc.desc  : Test AddScoStream with ringing scene
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, AddScoStream_004, TestSize.Level1)
{
    uint32_t streamId = 100004;
    AudioScene audioScene = AUDIO_SCENE_RINGING;
    int32_t uid = 1004;
    bool isRecognition = false;
    
    ScoAudioSceneManager::GetInstance().AddScoStream(streamId, audioScene, uid, isRecognition);
    
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_RINGING);
}

/**
 * @tc.name  : ScoAudioSceneManager_RemoveScoStream_001
 * @tc.desc  : Test RemoveScoStream
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, RemoveScoStream_001, TestSize.Level1)
{
    uint32_t streamId = 100001;
    AudioScene audioScene = AUDIO_SCENE_PHONE_CALL;
    int32_t uid = 1001;
    bool isRecognition = false;
    
    ScoAudioSceneManager::GetInstance().AddScoStream(streamId, audioScene, uid, isRecognition);
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
    
    ScoAudioSceneManager::GetInstance().RemoveScoStream(streamId);
    EXPECT_FALSE(ScoAudioSceneManager::GetInstance().HasScoStream());
}

/**
 * @tc.name  : ScoAudioSceneManager_GetHighestPriorityAudioScene_001
 * @tc.desc  : Test GetHighestPriorityAudioScene with multiple streams
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetHighestPriorityAudioScene_001, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().AddScoStream(100001, AUDIO_SCENE_PHONE_CHAT, 1001, false);
    ScoAudioSceneManager::GetInstance().AddScoStream(100002, AUDIO_SCENE_PHONE_CALL, 1002, false);
    ScoAudioSceneManager::GetInstance().AddScoStream(100003, AUDIO_SCENE_DEFAULT, 1003, true);
    
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_PHONE_CALL);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetHighestPriorityAudioScene_002
 * @tc.desc  : Test GetHighestPriorityAudioScene with voip and recognition
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetHighestPriorityAudioScene_002, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().AddScoStream(100001, AUDIO_SCENE_PHONE_CHAT, 1001, false);
    ScoAudioSceneManager::GetInstance().AddScoStream(100002, AUDIO_SCENE_DEFAULT, 1002, true);
    
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_PHONE_CHAT);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetHighestPriorityAudioScene_003
 * @tc.desc  : Test GetHighestPriorityAudioScene with ringing and recognition
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetHighestPriorityAudioScene_003, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().AddScoStream(100001, AUDIO_SCENE_RINGING, 1001, false);
    ScoAudioSceneManager::GetInstance().AddScoStream(100002, AUDIO_SCENE_DEFAULT, 1002, true);
    
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_RINGING);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetHighestPriorityAudioScene_004
 * @tc.desc  : Test GetHighestPriorityAudioScene with empty manager
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetHighestPriorityAudioScene_004, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
    EXPECT_EQ(ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene(), AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetAudioScenePriority_001
 * @tc.desc  : Test GetAudioScenePriority with cellular call
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetAudioScenePriority_001, TestSize.Level1)
{
    ScoAudioScenePriority priority = ScoAudioSceneManager::GetInstance().GetAudioScenePriority(
        AUDIO_SCENE_PHONE_CALL, false);
    EXPECT_EQ(priority, SCO_PRIORITY_CELLULAR);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetAudioScenePriority_002
 * @tc.desc  : Test GetAudioScenePriority with voip
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetAudioScenePriority_002, TestSize.Level1)
{
    ScoAudioScenePriority priority = ScoAudioSceneManager::GetInstance().GetAudioScenePriority(
        AUDIO_SCENE_PHONE_CHAT, false);
    EXPECT_EQ(priority, SCO_PRIORITY_VOIP);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetAudioScenePriority_003
 * @tc.desc  : Test GetAudioScenePriority with recognition
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetAudioScenePriority_003, TestSize.Level1)
{
    ScoAudioScenePriority priority = ScoAudioSceneManager::GetInstance().GetAudioScenePriority(
        AUDIO_SCENE_DEFAULT, true);
    EXPECT_EQ(priority, SCO_PRIORITY_VOICE_RECOGNITION);
}

/**
 * @tc.name  : ScoAudioSceneManager_GetAudioScenePriority_004
 * @tc.desc  : Test GetAudioScenePriority with ringing
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, GetAudioScenePriority_004, TestSize.Level1)
{
    ScoAudioScenePriority priority = ScoAudioSceneManager::GetInstance().GetAudioScenePriority(
        AUDIO_SCENE_RINGING, false);
    EXPECT_EQ(priority, SCO_PRIORITY_VOIP);
}

/**
 * @tc.name  : ScoAudioSceneManager_HasScoStream_001
 * @tc.desc  : Test HasScoStream when empty
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, HasScoStream_001, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
    EXPECT_FALSE(ScoAudioSceneManager::GetInstance().HasScoStream());
}

/**
 * @tc.name  : ScoAudioSceneManager_HasScoStream_002
 * @tc.desc  : Test HasScoStream when has stream
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, HasScoStream_002, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().AddScoStream(100001, AUDIO_SCENE_PHONE_CALL, 1001, false);
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
}

/**
 * @tc.name  : ScoAudioSceneManager_ClearAllScoStreams_001
 * @tc.desc  : Test ClearAllScoStreams
 * @tc.type  : FUNC
 */
HWTEST_F(ScoAudioSceneManagerTest, ClearAllScoStreams_001, TestSize.Level1)
{
    ScoAudioSceneManager::GetInstance().AddScoStream(100001, AUDIO_SCENE_PHONE_CALL, 1001, false);
    ScoAudioSceneManager::GetInstance().AddScoStream(100002, AUDIO_SCENE_PHONE_CHAT, 1002, false);
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());
    
    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
    EXPECT_FALSE(ScoAudioSceneManager::GetInstance().HasScoStream());
}

} // namespace AudioStandard
} // namespace OHOS