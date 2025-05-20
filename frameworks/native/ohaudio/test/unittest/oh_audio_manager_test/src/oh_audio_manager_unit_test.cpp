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

#include "oh_audio_manager_unit_test.h"
#include "OHAudioManager.h"

using namespace testing::ext;

namespace {
constexpr int32_t INVALID_VALUE = -1;
}

namespace OHOS {
namespace AudioStandard {
void OHAudioManagerUnitTest::SetUpTestCase(void) { }

void OHAudioManagerUnitTest::TearDownTestCase(void) { }

void OHAudioManagerUnitTest::SetUp(void) { }

void OHAudioManagerUnitTest::TearDown(void) { }

void MyOnAudioSceneChangeCallback(void *userData, OH_AudioScene scene)
{
    if (userData == nullptr) {
        return;
    }
    AudioScene *myScene = (AudioScene *)userData;
    *myScene = static_cast<AudioScene>(scene);
}

/**
 * @tc.name  : Test OH_GetAudioManager.
 * @tc.number: OH_GetAudioManager_001
 * @tc.desc  : Test OH_GetAudioManager with valid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_GetAudioManager_001, TestSize.Level0)
{
    OH_AudioManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_GetAudioManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_GetAudioManager.
 * @tc.number: OH_GetAudioManager_002
 * @tc.desc  : Test OH_GetAudioManager with invalid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_GetAudioManager_002, TestSize.Level0)
{
    OH_AudioCommon_Result result = OH_GetAudioManager(nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_GetAudioScene.
 * @tc.number: OH_GetAudioScene_001
 * @tc.desc  : Test OH_GetAudioScene  with valid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_GetAudioScene_001, TestSize.Level0)
{
    OH_AudioManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_GetAudioManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    OH_AudioScene scene;
    result = OH_GetAudioScene(audioManager, &scene);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_GetAudioScene.
 * @tc.number: OH_GetAudioScene_002
 * @tc.desc  : Test OH_GetAudioScene with invalid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_GetAudioScene_002, TestSize.Level0)
{
    OH_AudioManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_GetAudioManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    OH_AudioScene scene;
    result = OH_GetAudioScene(nullptr, &scene);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_GetAudioScene(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioManager_RegisterAudioSceneChangeCallback.
 * @tc.number: OH_AudioManager_RegisterAudioSceneChangeCallback_001
 * @tc.desc  : Test OH_AudioManager_RegisterAudioSceneChangeCallback with valid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_AudioManager_RegisterAudioSceneChangeCallback_001, TestSize.Level0)
{
    OH_AudioManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_GetAudioManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioManager_RegisterAudioSceneChangeCallback(audioManager, MyOnAudioSceneChangeCallback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioManager_UnregisterAudioSceneChangeCallback(audioManager, MyOnAudioSceneChangeCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioManager_RegisterAudioSceneChangeCallback.
 * @tc.number: OH_AudioManager_RegisterAudioSceneChangeCallback_002
 * @tc.desc  : Test OH_AudioManager_RegisterAudioSceneChangeCallback with invalid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_AudioManager_RegisterAudioSceneChangeCallback_002, TestSize.Level0)
{
    OH_AudioManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_GetAudioManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioManager_RegisterAudioSceneChangeCallback(nullptr, MyOnAudioSceneChangeCallback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioManager_RegisterAudioSceneChangeCallback(audioManager, nullptr, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioManager_UnregisterAudioSceneChangeCallback.
 * @tc.number: OH_AudioManager_UnregisterAudioSceneChangeCallback_001
 * @tc.desc  : Test OH_AudioManager_UnregisterAudioSceneChangeCallback with invalid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OH_AudioManager_UnregisterAudioSceneChangeCallback_001, TestSize.Level0)
{
    OH_AudioManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_GetAudioManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioManager_UnregisterAudioSceneChangeCallback(nullptr, MyOnAudioSceneChangeCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioManager_UnregisterAudioSceneChangeCallback(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioSceneChangeCallback.
 * @tc.number: SetAudioSceneChangeCallback_001
 * @tc.desc  : Test SetAudioSceneChangeCallback with repeat registration.
 */
HWTEST(OHAudioManagerUnitTest, SetAudioSceneChangeCallback_001, TestSize.Level0)
{
    OHAudioManager *ohAudioManager = OHAudioManager::GetInstance();
    EXPECT_NE(ohAudioManager, nullptr);

    int32_t res = ohAudioManager->SetAudioSceneChangeCallback(MyOnAudioSceneChangeCallback, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);

    res = ohAudioManager->SetAudioSceneChangeCallback(MyOnAudioSceneChangeCallback, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);

    res =  ohAudioManager->UnsetAudioSceneChangeCallback(MyOnAudioSceneChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test SetAudioSceneChangeCallback.
 * @tc.number: SetAudioSceneChangeCallback_002
 * @tc.desc  : Test SetAudioSceneChangeCallback with invalid parameter.
 */
HWTEST(OHAudioManagerUnitTest, SetAudioSceneChangeCallback_002, TestSize.Level0)
{
    OHAudioManager *ohAudioManager = OHAudioManager::GetInstance();
    EXPECT_NE(ohAudioManager, nullptr);

    int32_t res = ohAudioManager->SetAudioSceneChangeCallback(nullptr, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test UnsetAudioSceneChangeCallback.
 * @tc.number: UnsetAudioSceneChangeCallback_001
 * @tc.desc  : Test UnsetAudioSceneChangeCallback with invalid parameter.
 */
HWTEST(OHAudioManagerUnitTest, UnsetAudioSceneChangeCallback_001, TestSize.Level0)
{
    OHAudioManager *ohAudioManager = OHAudioManager::GetInstance();
    EXPECT_NE(ohAudioManager, nullptr);

    int32_t res = ohAudioManager->UnsetAudioSceneChangeCallback(nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    res = ohAudioManager->UnsetAudioSceneChangeCallback(MyOnAudioSceneChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OnAudioSceneChange.
 * @tc.number: OnAudioSceneChange_001
 * @tc.desc  : Test OnAudioSceneChange with valid parameter.
 */
HWTEST(OHAudioManagerUnitTest, OnAudioSceneChange_001, TestSize.Level0)
{
    AudioScene scene = (AudioScene)INVALID_VALUE;
    auto cb = std::make_shared<OHAudioManagerAudioSceneChangedCallback>(MyOnAudioSceneChangeCallback, &scene);
    EXPECT_NE(cb, nullptr);

    cb->OnAudioSceneChange(AudioScene::AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(scene, AudioScene::AUDIO_SCENE_DEFAULT);

    cb->OnAudioSceneChange(AudioScene::AUDIO_SCENE_VOICE_RINGING);
    EXPECT_EQ(scene, AudioScene::AUDIO_SCENE_RINGING);

    cb->OnAudioSceneChange(AudioScene::AUDIO_SCENE_MAX);
    EXPECT_EQ(scene, AudioScene::AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name  : Test OnAudioSceneChange.
 * @tc.number: OnAudioSceneChange_002
 * @tc.desc  : Test OnAudioSceneChange with cb is nullptr.
 */
HWTEST(OHAudioManagerUnitTest, OnAudioSceneChange_002, TestSize.Level0)
{
    AudioScene scene = (AudioScene)INVALID_VALUE;
    auto cb = std::make_shared<OHAudioManagerAudioSceneChangedCallback>(nullptr, &scene);
    EXPECT_NE(cb, nullptr);

    cb->OnAudioSceneChange(AudioScene::AUDIO_SCENE_DEFAULT);
    EXPECT_NE(scene, AudioScene::AUDIO_SCENE_DEFAULT);
}
} // namespace AudioStandard
} // namespace OHOS
