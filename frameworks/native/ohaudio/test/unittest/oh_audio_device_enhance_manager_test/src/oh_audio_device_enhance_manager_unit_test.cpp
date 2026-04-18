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

#include "oh_audio_device_enhance_manager_unit_test.h"

#include <cstdlib>

#include "OHAudioDeviceEnhanceManager.h"
#include "audio_errors.h"
#include "native_audio_device_enhance_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioDeviceEnhanceManagerUnitTest::SetUpTestCase(void) {}

void OHAudioDeviceEnhanceManagerUnitTest::TearDownTestCase(void) {}

void OHAudioDeviceEnhanceManagerUnitTest::SetUp(void) {}

void OHAudioDeviceEnhanceManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test OH_AudioManager_GetAudioDeviceEnhanceManager.
 * @tc.number: OH_AudioManager_GetAudioDeviceEnhanceManager_001
 * @tc.desc  : Test null output parameter.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioManager_GetAudioDeviceEnhanceManager_001, TestSize.Level0)
{
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioManager_GetAudioDeviceEnhanceManager.
 * @tc.number: OH_AudioManager_GetAudioDeviceEnhanceManager_002
 * @tc.desc  : Test valid parameter.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioManager_GetAudioDeviceEnhanceManager_002, TestSize.Level0)
{
    OH_AudioDeviceEnhanceManager *manager = nullptr;
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(&manager);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(manager, nullptr);
}

/**
 * @tc.name  : Test OH_AudioManager_GetAudioDeviceEnhanceManager.
 * @tc.number: OH_AudioManager_GetAudioDeviceEnhanceManager_003
 * @tc.desc  : Test returned manager matches singleton wrapper instance.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioManager_GetAudioDeviceEnhanceManager_003, TestSize.Level0)
{
    OH_AudioDeviceEnhanceManager *manager = nullptr;
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(&manager);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(manager, nullptr);

    auto &instance = OHAudioDeviceEnhanceManager::GetInstance();
    auto *expected = reinterpret_cast<OH_AudioDeviceEnhanceManager *>(&instance);
    EXPECT_EQ(manager, expected);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported.
 * @tc.number: OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported_001
 * @tc.desc  : Test invalid parameter.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported_001,
    TestSize.Level0)
{
    bool supported = false;
    auto ret = OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported(nullptr, &supported);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported.
 * @tc.number: OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported_002
 * @tc.desc  : Test supported output parameter is null.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported_002,
    TestSize.Level0)
{
    OH_AudioDeviceEnhanceManager *manager = nullptr;
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(&manager);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(manager, nullptr);

    ret = OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported(manager, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported.
* @tc.number: OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported_004
 * @tc.desc  : Test C API result is consistent with native wrapper result.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported_004,
    TestSize.Level0)
{
    OH_AudioDeviceEnhanceManager *manager = nullptr;
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(&manager);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(manager, nullptr);

    bool cApiSupported = false;
    ret = OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported(manager, &cApiSupported);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);

    bool nativeSupported = false;
    auto nativeRet = OHAudioDeviceEnhanceManager::GetInstance().IsEnhancedRoutingSupported(nativeSupported);
    EXPECT_EQ(nativeRet, SUCCESS);
    EXPECT_EQ(cApiSupported, nativeSupported);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectOutputDevice.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectOutputDevice_001
 * @tc.desc  : Test null manager.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectOutputDevice_001, TestSize.Level0)
{
    auto ret = OH_AudioDeviceEnhanceManager_SelectOutputDevice(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectInputDevice.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectInputDevice_001
 * @tc.desc  : Test null manager.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectInputDevice_001, TestSize.Level0)
{
    auto ret = OH_AudioDeviceEnhanceManager_SelectInputDevice(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer_001
 * @tc.desc  : Test null renderer.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer_001,
    TestSize.Level0)
{
    OH_AudioDeviceEnhanceManager *manager = nullptr;
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(&manager);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(manager, nullptr);

    ret = OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer(manager, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer_002
 * @tc.desc  : Test null manager and valid renderer.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer_002,
    TestSize.Level0)
{
    auto *renderer = reinterpret_cast<OH_AudioRenderer *>(0x1);
    auto ret = OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer(nullptr, renderer, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer_001
 * @tc.desc  : Test null capturer.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer_001,
    TestSize.Level0)
{
    OH_AudioDeviceEnhanceManager *manager = nullptr;
    auto ret = OH_AudioManager_GetAudioDeviceEnhanceManager(&manager);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(manager, nullptr);

    ret = OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer(manager, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer_002
 * @tc.desc  : Test null manager and valid capturer.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer_002,
    TestSize.Level0)
{
    auto *capturer = reinterpret_cast<OH_AudioCapturer *>(0x1);
    auto ret = OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer(nullptr, capturer, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer_003
 * @tc.desc  : Test null manager and null renderer.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer_003,
    TestSize.Level0)
{
    auto ret = OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer.
 * @tc.number: OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer_003
 * @tc.desc  : Test null manager and null capturer.
 */
HWTEST(OHAudioDeviceEnhanceManagerUnitTest, OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer_003,
    TestSize.Level0)
{
    auto ret = OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}
} // namespace AudioStandard
} // namespace OHOS
