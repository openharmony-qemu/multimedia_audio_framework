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

#include "oh_audio_session_manager_unit_test.h"
#include "native_audio_session_manager.h"
#include "OHAudioSessionManager.h"
#include "OHAudioRoutingManager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioSessionManagerUnitTest::SetUpTestCase(void) { }

void OHAudioSessionManagerUnitTest::TearDownTestCase(void) { }

void OHAudioSessionManagerUnitTest::SetUp(void) { }

void OHAudioSessionManagerUnitTest::TearDown(void) { }

void MyStateCallback(OH_AudioSession_StateChangedEvent event)
{
    return;
}

void MyDeviceCallback(OH_AudioDeviceDescriptorArray *devices,
    OH_AudioStream_DeviceChangeReason changeReason,
    OH_AudioSession_OutputDeviceChangeRecommendedAction recommendedAction)
{
    return;
}

/**
 * @tc.name  : Test OH_AudioManager_GetAudioSessionManager.
 * @tc.number: OH_AudioManager_GetAudioSessionManager_001
 * @tc.desc  : Test OH_AudioManager_GetAudioSessionManager with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioManager_GetAudioSessionManager_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_SetScene.
 * @tc.number: OH_AudioSessionManager_SetScene_001
 * @tc.desc  : Test OH_GetAudOH_AudioSessionManager_SetSceneioManager with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_SetScene_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    OH_AudioSession_Scene scene = AUDIO_SESSION_SCENE_MEDIA;
    result = OH_AudioSessionManager_SetScene(audioManager, scene);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    scene = AUDIO_SESSION_SCENE_GAME;
    result = OH_AudioSessionManager_SetScene(audioManager, scene);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    scene = AUDIO_SESSION_SCENE_VOICE_COMMUNICATION;
    result = OH_AudioSessionManager_SetScene(audioManager, scene);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_RegisterStateChangeCallback.
 * @tc.number: OH_AudioSessionManager_RegisterStateChangeCallback_001
 * @tc.desc  : Test OH_AudioSessionManager_RegisterStateChangeCallback with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_RegisterStateChangeCallback_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_RegisterStateChangeCallback(audioManager, MyStateCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_UnregisterStateChangeCallback(audioManager, MyStateCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_RegisterStateChangeCallback.
 * @tc.number: OH_AudioSessionManager_RegisterStateChangeCallback_002
 * @tc.desc  : Test OH_AudioSessionManager_RegisterStateChangeCallback with invalid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_RegisterStateChangeCallback_002, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_RegisterStateChangeCallback(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioSessionManager_UnregisterStateChangeCallback(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_RegisterStateChangeCallback.
 * @tc.number: OH_AudioSessionManager_RegisterStateChangeCallback_003
 * @tc.desc  : Test OH_AudioSessionManager_RegisterStateChangeCallback with double register.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_RegisterStateChangeCallback_003, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_RegisterStateChangeCallback(audioManager, MyStateCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_RegisterStateChangeCallback(audioManager, MyStateCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_UnregisterStateChangeCallback(audioManager, MyStateCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_UnregisterStateChangeCallback(audioManager, MyStateCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OnAudOnAudioSessionStateChangedioSceneChange.
 * @tc.number: OnAudioSessionStateChanged_001
 * @tc.desc  : Test OnAudioSessionStateChanged with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OnAudioSessionStateChanged_001, TestSize.Level0)
{
    std::shared_ptr<OHAudioSessionStateCallback> cb = std::make_shared<OHAudioSessionStateCallback>(MyStateCallback);
    EXPECT_NE(cb, nullptr);

    AudioSessionStateChangedEvent stateChangedEvent;
    stateChangedEvent.stateChangeHint = AudioSessionStateChangeHint::INVALID;
    cb->OnAudioSessionStateChanged(stateChangedEvent);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_SetDefaultOutputDevice.
 * @tc.number: OH_AudioSessionManager_deviceType_001
 * @tc.desc  : Test OH_AudioSessionManager_SetDefaultOutputDevice with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_deviceType_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    OH_AudioDevice_Type deviceTypeGet = AUDIO_DEVICE_TYPE_EARPIECE;
    result = OH_AudioSessionManager_GetDefaultOutputDevice(audioManager, &deviceTypeGet);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_EQ(deviceTypeGet, AUDIO_DEVICE_TYPE_INVALID);

    OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_EARPIECE;
    result = OH_AudioSessionManager_SetDefaultOutputDevice(audioManager, deviceType);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    deviceType = AUDIO_DEVICE_TYPE_DEFAULT;
    result = OH_AudioSessionManager_SetDefaultOutputDevice(audioManager, deviceType);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    deviceType = AUDIO_DEVICE_TYPE_SPEAKER;
    result = OH_AudioSessionManager_SetDefaultOutputDevice(audioManager, deviceType);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_GetDefaultOutputDevice(audioManager, &deviceTypeGet);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_EQ(deviceTypeGet, AUDIO_DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_SetDefaultOutputDevice.
 * @tc.number: OH_AudioSessionManager_deviceType_002
 * @tc.desc  : Test OH_AudioSessionManager_SetDefaultOutputDevice with invalid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_deviceType_002, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
    result = OH_AudioSessionManager_SetDefaultOutputDevice(audioManager, deviceType);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_GetDefaultOutputDevice.
 * @tc.number: OH_AudioSessionManager_GetDefaultOutputDevice_001
 * @tc.desc  : Test OH_AudioSessionManager_GetDefaultOutputDevice with invalid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_GetDefaultOutputDevice_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_GetDefaultOutputDevice(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback.
 * @tc.number: OH_AudioSessionManager_RegisterDeviceCallback_001
 * @tc.desc  : Test OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_RegisterDeviceCallback_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback(audioManager, MyDeviceCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_UnregisterCurrentOutputDeviceChangeCallback(audioManager, MyDeviceCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback.
 * @tc.number: OH_AudioSessionManager_RegisterDeviceCallback_002
 * @tc.desc  : Test OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback with invalid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_RegisterDeviceCallback_002, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioSessionManager_UnregisterCurrentOutputDeviceChangeCallback(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback.
 * @tc.number: OH_AudioSessionManager_RegisterDeviceCallback_003
 * @tc.desc  : Test OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback with double register.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_RegisterDeviceCallback_003, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback(audioManager, MyDeviceCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback(audioManager, MyDeviceCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_UnregisterCurrentOutputDeviceChangeCallback(audioManager, MyDeviceCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioSessionManager_UnregisterCurrentOutputDeviceChangeCallback(audioManager, MyDeviceCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_ReleaseDevices.
 * @tc.number: OH_AudioSessionManager_ReleaseDevices_001
 * @tc.desc  : Test OH_AudioSessionManager_ReleaseDevices with valid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_ReleaseDevices_001, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    result = OH_AudioSessionManager_ReleaseDevices(audioManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSessionManager_ReleaseDevices.
 * @tc.number: OH_AudioSessionManager_ReleaseDevices_002
 * @tc.desc  : Test OH_AudioSessionManager_ReleaseDevices with invalid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, OH_AudioSessionManager_ReleaseDevices_002, TestSize.Level0)
{
    OH_AudioSessionManager *audioManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioSessionManager(&audioManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_TRUE(audioManager != nullptr);

    result = OH_AudioSessionManager_ReleaseDevices(nullptr, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioSessionManager_ReleaseDevices(audioManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    OH_AudioDeviceDescriptorArray audioDeviceDescriptorArray;
    audioDeviceDescriptorArray.descriptors = nullptr;

    result = OH_AudioSessionManager_ReleaseDevices(audioManager, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioSessionCurrentDeviceChangeCallback
 * @tc.number: SetAudioSessionCurrentDeviceChangeCallback_001
 * @tc.desc  : Test SetAudioSessionCurrentDeviceChangeCallback with nullptr parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, SetAudioSessionCurrentDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioCommon_Result result = OHAudioSessionManager::GetInstance()->
        SetAudioSessionCurrentDeviceChangeCallback(nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioSessionStateChangeCallback
 * @tc.number: SetAudioSessionStateChangeCallback_001
 * @tc.desc  : Test SetAudioSessionStateChangeCallback with invalid parameter.
 */
HWTEST(OHAudioSessionManagerUnitTest, SetAudioSessionStateChangeCallback_001, TestSize.Level0)
{
    OH_AudioCommon_Result result = OHAudioSessionManager::GetInstance()->
        SetAudioSessionStateChangeCallback(nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

} // namespace AudioStandard
} // namespace OHOS
