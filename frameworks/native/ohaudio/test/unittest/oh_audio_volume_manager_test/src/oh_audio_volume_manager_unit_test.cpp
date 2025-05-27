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

#include "oh_audio_volume_manager_unit_test.h"
#include "OHAudioVolumeManager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioVolumeManagerUnitTest::SetUpTestCase(void) { }

void OHAudioVolumeManagerUnitTest::TearDownTestCase(void) { }

void OHAudioVolumeManagerUnitTest::SetUp(void) { }

void OHAudioVolumeManagerUnitTest::TearDown(void) { }

namespace {
constexpr int32_t INVALID_VALUE = -1;
}

void MyOnStreamVolumeChangeCallback(void *userData, OH_AudioStream_Usage usage,
    int32_t volumeLevel, bool updateUi)
{
    if (userData == nullptr) {
        return;
    }
    int32_t *myVolumeLevel = (int32_t *)userData;
    *myVolumeLevel = volumeLevel;
    (void)usage;
    (void)updateUi;
}

void MyOnRingerModeChangeCallback(void *userData, OH_AudioRingerMode ringerMode)
{
    if (userData == nullptr) {
        return;
    }
    AudioRingerMode *myRingerMode = (AudioRingerMode *)userData;
    *myRingerMode = static_cast<AudioRingerMode>(ringerMode);
}

/**
 * @tc.name  : Test OH_AudioManager_GetAudioVolumeManager.
 * @tc.number: OH_AudioManager_GetAudioVolumeManager_001
 * @tc.desc  : Test OH_AudioManager_GetAudioVolumeManager with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioManager_GetAudioVolumeManager_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioManager_GetAudioVolumeManager.
 * @tc.number: OH_AudioManager_GetAudioVolumeManager_002
 * @tc.desc  : Test OH_AudioManager_GetAudioVolumeManager with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioManager_GetAudioVolumeManager_002, TestSize.Level0)
{
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetMaxVolumeByUsage.
 * @tc.number: OH_AudioVolumeManager_GetMaxVolumeByUsage_001
 * @tc.desc  : Test OH_AudioVolumeManager_GetMaxVolumeByUsage with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetMaxVolumeByUsage_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    int32_t maxVolumeLevel = 0;
    result = OH_AudioVolumeManager_GetMaxVolumeByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, &maxVolumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetMaxVolumeByUsage.
 * @tc.number: OH_AudioVolumeManager_GetMaxVolumeByUsage_002
 * @tc.desc  : Test OH_AudioVolumeManager_GetMaxVolumeByUsage with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetMaxVolumeByUsage_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    int32_t maxVolumeLevel = 0;
    result = OH_AudioVolumeManager_GetMaxVolumeByUsage(nullptr, AUDIOSTREAM_USAGE_GAME, &maxVolumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetMaxVolumeByUsage(volumeManager, (OH_AudioStream_Usage)INVALID_VALUE,
        &maxVolumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetMaxVolumeByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetMinVolumeByUsage.
 * @tc.number: OH_AudioVolumeManager_GetMinVolumeByUsage_001
 * @tc.desc  : Test OH_AudioVolumeManager_GetMinVolumeByUsage with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetMinVolumeByUsage_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    int32_t minVolumeLevel = 0;
    result = OH_AudioVolumeManager_GetMinVolumeByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, &minVolumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetMinVolumeByUsage.
 * @tc.number: OH_AudioVolumeManager_GetMinVolumeByUsage_002
 * @tc.desc  : Test OH_AudioVolumeManager_GetMinVolumeByUsage with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetMinVolumeByUsage_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    int32_t minVolumeLevel = 0;
    result = OH_AudioVolumeManager_GetMinVolumeByUsage(nullptr, AUDIOSTREAM_USAGE_GAME, &minVolumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetMinVolumeByUsage(volumeManager, (OH_AudioStream_Usage)INVALID_VALUE,
        &minVolumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetMinVolumeByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetVolumeByUsage.
 * @tc.number: OH_AudioVolumeManager_GetVolumeByUsage_001
 * @tc.desc  : Test OH_AudioVolumeManager_GetVolumeByUsage with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetVolumeByUsage_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    int32_t volumeLevel = 0;
    result = OH_AudioVolumeManager_GetVolumeByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, &volumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetVolumeByUsage.
 * @tc.number: OH_AudioVolumeManager_GetVolumeByUsage_002
 * @tc.desc  : Test OH_AudioVolumeManager_GetVolumeByUsage with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetVolumeByUsage_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    int32_t volumeLevel = 0;
    result = OH_AudioVolumeManager_GetVolumeByUsage(nullptr, AUDIOSTREAM_USAGE_GAME, &volumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetVolumeByUsage(volumeManager, (OH_AudioStream_Usage)INVALID_VALUE,
        &volumeLevel);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetVolumeByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_IsMuteByUsage.
 * @tc.number: OH_AudioVolumeManager_IsMuteByUsage_001
 * @tc.desc  : Test OH_AudioVolumeManager_IsMuteByUsage with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_IsMuteByUsage_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    bool muted = false;
    result = OH_AudioVolumeManager_IsMuteByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, &muted);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_IsMuteByUsage.
 * @tc.number: OH_AudioVolumeManager_IsMuteByUsage_002
 * @tc.desc  : Test OH_AudioVolumeManager_IsMuteByUsage with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_IsMuteByUsage_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    bool muted = false;
    result = OH_AudioVolumeManager_IsMuteByUsage(nullptr, AUDIOSTREAM_USAGE_GAME, &muted);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_IsMuteByUsage(volumeManager, (OH_AudioStream_Usage)INVALID_VALUE,
        &muted);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_IsMuteByUsage(volumeManager, AUDIOSTREAM_USAGE_GAME, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback.
 * @tc.number: OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback_001
 * @tc.desc  : Test OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback(volumeManager, AUDIOSTREAM_USAGE_GAME,
        MyOnStreamVolumeChangeCallback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback(volumeManager, MyOnStreamVolumeChangeCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback.
 * @tc.number: OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback_002
 * @tc.desc  : Test OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback(nullptr, AUDIOSTREAM_USAGE_GAME,
        MyOnStreamVolumeChangeCallback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback(volumeManager,
        (OH_AudioStream_Usage)INVALID_VALUE, MyOnStreamVolumeChangeCallback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_RegisterStreamVolumeChangeCallback(volumeManager, AUDIOSTREAM_USAGE_GAME,
        nullptr, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback.
 * @tc.number: OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback_001
 * @tc.desc  : Test OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback(nullptr, MyOnStreamVolumeChangeCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_UnregisterStreamVolumeChangeCallback(volumeManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetRingerMode.
 * @tc.number: OH_AudioVolumeManager_GetRingerMode_001
 * @tc.desc  : Test OH_AudioVolumeManager_GetRingerMode with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetRingerMode_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    OH_AudioRingerMode ringerMode = AUDIO_RINGER_MODE_NORMAL;
    result = OH_AudioVolumeManager_GetRingerMode(volumeManager, &ringerMode);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_GetRingerMode.
 * @tc.number: OH_AudioVolumeManager_GetRingerMode_002
 * @tc.desc  : Test OH_AudioVolumeManager_GetRingerMode with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_GetRingerMode_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    OH_AudioRingerMode ringerMode = AUDIO_RINGER_MODE_NORMAL;
    result = OH_AudioVolumeManager_GetRingerMode(nullptr, &ringerMode);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_GetRingerMode(volumeManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_RegisterRingerModeChangeCallback.
 * @tc.number: OH_AudioVolumeManager_RegisterRingerModeChangeCallback_001
 * @tc.desc  : Test OH_AudioVolumeManager_RegisterRingerModeChangeCallback with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_RegisterRingerModeChangeCallback_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_RegisterRingerModeChangeCallback(volumeManager, MyOnRingerModeChangeCallback,
        nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_UnregisterRingerModeChangeCallback(volumeManager, MyOnRingerModeChangeCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_RegisterRingerModeChangeCallback.
 * @tc.number: OH_AudioVolumeManager_RegisterRingerModeChangeCallback_002
 * @tc.desc  : Test OH_AudioVolumeManager_RegisterRingerModeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_RegisterRingerModeChangeCallback_002, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_RegisterRingerModeChangeCallback(nullptr, MyOnRingerModeChangeCallback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_RegisterRingerModeChangeCallback(volumeManager, nullptr, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioVolumeManager_UnregisterRingerModeChangeCallback.
 * @tc.number: OH_AudioVolumeManager_UnregisterRingerModeChangeCallback_001
 * @tc.desc  : Test OH_AudioVolumeManager_UnregisterRingerModeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OH_AudioVolumeManager_UnregisterRingerModeChangeCallback_001, TestSize.Level0)
{
    OH_AudioVolumeManager *volumeManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioVolumeManager(&volumeManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioVolumeManager_UnregisterRingerModeChangeCallback(nullptr, MyOnRingerModeChangeCallback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioVolumeManager_UnregisterRingerModeChangeCallback(volumeManager, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioRingerModeChangeCallback.
 * @tc.number: SetAudioRingerModeChangeCallback_001
 * @tc.desc  : Test SetAudioRingerModeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, SetAudioRingerModeChangeCallback_001, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = volumeManager->SetAudioRingerModeChangeCallback(nullptr, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioRingerModeChangeCallback.
 * @tc.number: SetAudioRingerModeChangeCallback_002
 * @tc.desc  : Test SetAudioRingerModeChangeCallback with repeat registration.
 */
HWTEST(OHAudioVolumeManagerUnitTest, SetAudioRingerModeChangeCallback_002, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = volumeManager->SetAudioRingerModeChangeCallback(MyOnRingerModeChangeCallback, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);

    res = volumeManager->SetAudioRingerModeChangeCallback(MyOnRingerModeChangeCallback, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);

    res = volumeManager->UnsetAudioRingerModeChangeCallback(MyOnRingerModeChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test UnsetAudioRingerModeChangeCallback.
 * @tc.number: UnsetAudioRingerModeChangeCallback_001
 * @tc.desc  : Test UnsetAudioRingerModeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, UnsetAudioRingerModeChangeCallback_001, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = volumeManager->UnsetAudioRingerModeChangeCallback(nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    res = volumeManager->UnsetAudioRingerModeChangeCallback(MyOnRingerModeChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetStreamVolumeChangeCallback.
 * @tc.number: SetStreamVolumeChangeCallback_001
 * @tc.desc  : Test SetStreamVolumeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, SetStreamVolumeChangeCallback_001, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = volumeManager->SetStreamVolumeChangeCallback(nullptr, STREAM_USAGE_MUSIC, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetStreamVolumeChangeCallback.
 * @tc.number: SetStreamVolumeChangeCallback_002
 * @tc.desc  : Test SetStreamVolumeChangeCallback with repeat registration.
 */
HWTEST(OHAudioVolumeManagerUnitTest, SetStreamVolumeChangeCallback_002, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = INVALID_VALUE;
    res = volumeManager->SetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback, STREAM_USAGE_MUSIC, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);
    res = volumeManager->SetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback, STREAM_USAGE_MUSIC, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);

    res = volumeManager->UnsetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test SetStreamVolumeChangeCallback.
 * @tc.number: SetStreamVolumeChangeCallback_003
 * @tc.desc  : Test SetStreamVolumeChangeCallback with reuse callback.
 */
HWTEST(OHAudioVolumeManagerUnitTest, SetStreamVolumeChangeCallback_003, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = INVALID_VALUE;
    res = volumeManager->SetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback, STREAM_USAGE_MUSIC, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);

    res = volumeManager->SetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback, STREAM_USAGE_ALARM, nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    res = volumeManager->UnsetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test UnsetStreamVolumeChangeCallback.
 * @tc.number: UnsetStreamVolumeChangeCallback_001
 * @tc.desc  : Test UnsetStreamVolumeChangeCallback with invalid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, UnsetStreamVolumeChangeCallback_001, TestSize.Level0)
{
    OHAudioVolumeManager *volumeManager = OHAudioVolumeManager::GetInstance();
    EXPECT_NE(volumeManager, nullptr);

    int32_t res = volumeManager->UnsetStreamVolumeChangeCallback(nullptr);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    res = volumeManager->UnsetStreamVolumeChangeCallback(MyOnStreamVolumeChangeCallback);
    EXPECT_EQ(res, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OnVolumeKeyEvent.
 * @tc.number: OnVolumeKeyEvent_001
 * @tc.desc  : Test OnVolumeKeyEvent with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OnVolumeKeyEvent_001, TestSize.Level0)
{
    int32_t volume = INVALID_VALUE;
    auto ohStreamVolumeChangeCallback = std::make_shared<OHStreamVolumeChangeCallback>(MyOnStreamVolumeChangeCallback,
        StreamUsage::STREAM_USAGE_MUSIC, &volume);
    EXPECT_NE(ohStreamVolumeChangeCallback, nullptr);

    StreamVolumeEvent event {};
    event.volume = 1;
    event.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    ohStreamVolumeChangeCallback->OnStreamVolumeChange(event);
    EXPECT_EQ(volume, event.volume);
}

/**
 * @tc.name  : Test OnVolumeKeyEvent.
 * @tc.number: OnVolumeKeyEvent_002
 * @tc.desc  : Test OnVolumeKeyEvent listening streamUsage is inconsistent with the incoming streamUsage.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OnVolumeKeyEvent_002, TestSize.Level0)
{
    int32_t volume = INVALID_VALUE;
    auto ohStreamVolumeChangeCallback = std::make_shared<OHStreamVolumeChangeCallback>(MyOnStreamVolumeChangeCallback,
        StreamUsage::STREAM_USAGE_MUSIC, &volume);
    EXPECT_NE(ohStreamVolumeChangeCallback, nullptr);

    StreamVolumeEvent event {};
    event.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    ohStreamVolumeChangeCallback->OnStreamVolumeChange(event);
}

/**
 * @tc.name  : Test OnRingerModeUpdated.
 * @tc.number: OnRingerModeUpdated_001
 * @tc.desc  : Test OnRingerModeUpdated with valid params.
 */
HWTEST(OHAudioVolumeManagerUnitTest, OnRingerModeUpdated_001, TestSize.Level0)
{
    AudioRingerMode ringerMode = (AudioRingerMode)INVALID_VALUE;
    auto onAudioRingerModeCallback = std::make_shared<OHAudioRingerModeCallback>(MyOnRingerModeChangeCallback,
        &ringerMode);
    EXPECT_NE(onAudioRingerModeCallback, nullptr);

    onAudioRingerModeCallback->OnRingerModeUpdated(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_NORMAL);
}
} // namespace AudioStandard
} // namespace OHOS
