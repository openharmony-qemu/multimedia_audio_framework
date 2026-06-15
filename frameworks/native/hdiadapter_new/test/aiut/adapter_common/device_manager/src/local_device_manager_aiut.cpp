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

#include "local_device_manager_aiut.h"
#include "aiut_log.h"
#include "adapter/local_device_manager.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void LocalDeviceManagerAiut::SetUpTestCase(void) {}
void LocalDeviceManagerAiut::TearDownTestCase(void) {}
void LocalDeviceManagerAiut::SetUp(void) {}
void LocalDeviceManagerAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test LocalDeviceManager construction
 * @tc.number: LocalDeviceManager_Constructor_001
 * @tc.desc  : Test LocalDeviceManager default constructor creates valid object.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    AIUT_ASSERT_NE_INFO_LOG(mgr, nullptr, "mgr is null");
}

/**
 * @tc.name  : Test LocalDeviceManager destruction
 * @tc.number: LocalDeviceManager_Destructor_001
 * @tc.desc  : Test LocalDeviceManager destructor does not crash on default-constructed object.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    mgr.reset();
}

// ==================== SetAudioParameter / GetAudioParameter Tests ====================

/**
 * @tc.name  : Test SetAudioParameter when adapter is nullptr
 * @tc.number: LocalDeviceManager_SetAudioParameter_001
 * @tc.desc  : Test SetAudioParameter returns error when adapter not loaded.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_SetAudioParameter_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    int32_t ret = mgr->SetAudioParameter("primary", AudioParamKey::NONE, "", "k1=v1");
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "SetAudioParameter should fail when adapter not loaded");
}

/**
 * @tc.name  : Test GetAudioParameter when adapter is nullptr
 * @tc.number: LocalDeviceManager_GetAudioParameter_001
 * @tc.desc  : Test GetAudioParameter returns empty string when adapter not loaded.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_GetAudioParameter_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    std::string result = mgr->GetAudioParameter("primary", AudioParamKey::NONE, "");
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(), "GetAudioParameter should return empty when adapter not loaded");
}

// ==================== SetVoiceVolume Tests ====================

/**
 * @tc.name  : Test SetVoiceVolume when adapter is nullptr
 * @tc.number: LocalDeviceManager_SetVoiceVolume_001
 * @tc.desc  : Test SetVoiceVolume returns error when adapter not loaded.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_SetVoiceVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    int32_t ret = mgr->SetVoiceVolume("primary", 0.5f);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "SetVoiceVolume should fail when adapter not loaded");
}

// ==================== SetAudioScene Tests ====================

/**
 * @tc.name  : Test SetAudioScene with default scene
 * @tc.number: LocalDeviceManager_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene does not crash with default scene.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_SetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    mgr->SetAudioScene(AUDIO_SCENE_DEFAULT);
}

// ==================== SetDmDeviceType Tests ====================

/**
 * @tc.name  : Test SetDmDeviceType
 * @tc.number: LocalDeviceManager_SetDmDeviceType_001
 * @tc.desc  : Test SetDmDeviceType does not crash.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_SetDmDeviceType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    mgr->SetDmDeviceType(1, DEVICE_TYPE_SPEAKER);
}

// ==================== SetPhoneCallSceneType Tests ====================

/**
 * @tc.name  : Test SetPhoneCallSceneType with invalid type
 * @tc.number: LocalDeviceManager_SetPhoneCallSceneType_001
 * @tc.desc  : Test SetPhoneCallSceneType returns error with invalid type.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_SetPhoneCallSceneType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    int32_t ret = mgr->SetPhoneCallSceneType("primary", 99);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "SetPhoneCallSceneType should fail with invalid type");
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo
 * @tc.number: LocalDeviceManager_DumpInfo_001
 * @tc.desc  : Test DumpInfo does not crash on empty adapter map.
 */
HWTEST_F(LocalDeviceManagerAiut, LocalDeviceManager_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    std::string dumpString;
    mgr->DumpInfo(dumpString);
    AIUT_EXPECT_TRUE_INFO_LOG(dumpString.empty(), "DumpInfo should produce empty string for empty adapter map");
}

} // namespace AudioStandard
} // namespace OHOS
