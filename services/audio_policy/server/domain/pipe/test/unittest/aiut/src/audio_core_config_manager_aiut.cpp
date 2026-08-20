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

#include "audio_core_config_manager_aiut.h"
#include "aiut_log.h"
#include "audio_core_config_manager.h"
#include "audio_errors.h"

#include <unistd.h>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
namespace {
}  // namespace

void AudioCoreConfigManagerAiut::SetUpTestCase(void) {}
void AudioCoreConfigManagerAiut::TearDownTestCase(void) {}
void AudioCoreConfigManagerAiut::SetUp(void) {}
void AudioCoreConfigManagerAiut::TearDown(void) {}

// ==================== OnHasEarpieceParsed Tests ====================

/**
 * @tc.name  : Test OnHasEarpieceParsed with true value
 * @tc.number: AudioCoreConfigManager_OnHasEarpieceParsed_True
 * @tc.desc  : Test OnHasEarpieceParsed interface.
 *             Sets hasEarpiece_ to true and verifies GetHasEarpiece returns true.
 * @tc.owner : audio_framework_route
 */
HWTEST_F(AudioCoreConfigManagerAiut,
    AudioCoreConfigManager_OnHasEarpieceParsed_True, TestSize.Level1)
{
    AIUT_INFO_LOG("start: OnHasEarpieceParsed(true) and verify GetHasEarpiece");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnHasEarpieceParsed(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetHasEarpiece(),
        "expected hasEarpiece_=true, got false");
}

/**
 * @tc.name  : Test OnHasEarpieceParsed with false value
 * @tc.number: AudioCoreConfigManager_OnHasEarpieceParsed_False
 * @tc.desc  : Test OnHasEarpieceParsed interface.
 *             Sets hasEarpiece_ to false and verifies GetHasEarpiece returns false.
 * @tc.owner : audio_framework_route
 */
HWTEST_F(AudioCoreConfigManagerAiut,
    AudioCoreConfigManager_OnHasEarpieceParsed_False, TestSize.Level1)
{
    AIUT_INFO_LOG("start: OnHasEarpieceParsed(false) and verify GetHasEarpiece");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnHasEarpieceParsed(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetHasEarpiece(),
        "expected hasEarpiece_=false, got true");
}

// ==================== GetHasEarpiece Tests ====================

/**
 * @tc.name  : Test GetHasEarpiece after setting true
 * @tc.number: AudioCoreConfigManager_GetHasEarpiece_True
 * @tc.desc  : Test GetHasEarpiece interface.
 *             After OnHasEarpieceParsed(true), GetHasEarpiece should return true.
 * @tc.owner : audio_framework_route
 */
HWTEST_F(AudioCoreConfigManagerAiut,
    AudioCoreConfigManager_GetHasEarpiece_True, TestSize.Level1)
{
    AIUT_INFO_LOG("start: GetHasEarpiece after OnHasEarpieceParsed(true)");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnHasEarpieceParsed(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetHasEarpiece(),
        "expected GetHasEarpiece()=true after OnHasEarpieceParsed(true)");
}

/**
 * @tc.name  : Test GetHasEarpiece after setting false
 * @tc.number: AudioCoreConfigManager_GetHasEarpiece_False
 * @tc.desc  : Test GetHasEarpiece interface.
 *             After OnHasEarpieceParsed(false), GetHasEarpiece should return false.
 * @tc.owner : audio_framework_route
 */
HWTEST_F(AudioCoreConfigManagerAiut,
    AudioCoreConfigManager_GetHasEarpiece_False, TestSize.Level1)
{
    AIUT_INFO_LOG("start: GetHasEarpiece after OnHasEarpieceParsed(false)");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnHasEarpieceParsed(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetHasEarpiece(),
        "expected GetHasEarpiece()=false after OnHasEarpieceParsed(false)");
}

// ==================== OnHasEarpieceParsed Toggle Tests ====================

/**
 * @tc.name  : Test OnHasEarpieceParsed toggle from true to false
 * @tc.number: AudioCoreConfigManager_OnHasEarpieceParsed_Toggle
 * @tc.desc  : Test OnHasEarpieceParsed interface.
 *             Toggle from true to false and verify value changes correctly.
 * @tc.owner : audio_framework_route
 */
HWTEST_F(AudioCoreConfigManagerAiut,
    AudioCoreConfigManager_OnHasEarpieceParsed_Toggle, TestSize.Level1)
{
    AIUT_INFO_LOG("start: toggle OnHasEarpieceParsed true then false");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnHasEarpieceParsed(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetHasEarpiece(),
        "expected true after first OnHasEarpieceParsed(true)");

    configManager.OnHasEarpieceParsed(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetHasEarpiece(),
        "expected false after second OnHasEarpieceParsed(false)");
}
}  // namespace AudioStandard
}  // namespace OHOS
