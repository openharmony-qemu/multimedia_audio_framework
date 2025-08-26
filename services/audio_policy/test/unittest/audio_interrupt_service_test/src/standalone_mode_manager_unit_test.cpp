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

#include "audio_errors.h"
#include "standalone_mode_manager_unit_test.h"
#include "audio_session_info.h"
#include "audio_bundle_manager.h"
#include "audio_volume.h"
#include "audio_interrupt_service.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void StandaloneModeManagerUnitTest::SetUpTestCase(void) {}
void StandaloneModeManagerUnitTest::TearDownTestCase(void) {}
void StandaloneModeManagerUnitTest::SetUp(void) {}
void StandaloneModeManagerUnitTest::TearDown(void) {}

static int32_t STANDALONE_MODE = 5;

/**
* @tc.name  : Test CheckAndRecordStandaloneApp.
* @tc.number: StandaloneModeManagerUnitTest_001.
* @tc.desc  : Test CheckAndRecordStandaloneApp.
*/
HWTEST_F(StandaloneModeManagerUnitTest, StandaloneModeManagerUnitTest_001, TestSize.Level1)
{
    int32_t appUid = 1;
    bool isOnlyRecordUid = false;
    int32_t sessionId = 1;
    int32_t ownerPid = 1;
    int32_t displayId = 1;
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    StandaloneModeManager::GetInstance().EraseDeactivateAudioStream(
        appUid, sessionId);
    bool ret = StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(
        appUid, isOnlyRecordUid, sessionId);
    EXPECT_EQ(ret, false);
    StandaloneModeManager::GetInstance().SetAppSilentOnDisplay(ownerPid, displayId);
    ret = StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(
        appUid, isOnlyRecordUid, sessionId);
    EXPECT_EQ(ret, false);
    ownerPid = 1;
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(ownerPid);
}

/**
* @tc.name  : Test SetAppSilentOnDisplay.
* @tc.number: StandaloneModeManagerUnitTest_002.
* @tc.desc  : Test SetAppSilentOnDisplay.
*/
HWTEST_F(StandaloneModeManagerUnitTest, StandaloneModeManagerUnitTest_002, TestSize.Level1)
{
    int32_t ownerPid = 1;
    int32_t displayId = 1;
    int32_t appUid = 1;
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    auto ret = StandaloneModeManager::GetInstance().SetAppSilentOnDisplay(ownerPid, displayId);
    EXPECT_EQ(ret, 0);
    displayId = -1;
    ret = StandaloneModeManager::GetInstance().SetAppSilentOnDisplay(ownerPid, displayId);
    EXPECT_EQ(ret, 0);
    ownerPid = 0;
    ret = StandaloneModeManager::GetInstance().SetAppSilentOnDisplay(ownerPid, displayId);
    EXPECT_EQ(ret, 0);
    ownerPid = 1;
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(ownerPid);
}

/**
* @tc.name  : Test SetAppConcurrencyMode.
* @tc.number: StandaloneModeManagerUnitTest_003
* @tc.desc  : Test SetAppConcurrencyMode.
*/
HWTEST_F(StandaloneModeManagerUnitTest, StandaloneModeManagerUnitTest_003, TestSize.Level1)
{
    int32_t ownerPid = 1;
    int32_t appUid = 1;
    int32_t mode = STANDALONE_MODE;
    int32_t sessionId = 1;
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    auto ret = StandaloneModeManager::GetInstance().SetAppConcurrencyMode(ownerPid, appUid, mode);
    EXPECT_EQ(ret, 0);
    StandaloneModeManager::GetInstance().EraseDeactivateAudioStream(appUid, sessionId);
    StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(appUid, false, sessionId);
    mode = 0;
    ret = StandaloneModeManager::GetInstance().SetAppConcurrencyMode(ownerPid, appUid, mode);
    EXPECT_EQ(ret, 0);
    StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(appUid, false, sessionId);
    StandaloneModeManager::GetInstance().EraseDeactivateAudioStream(appUid, sessionId);
    mode = 1;
    ret = StandaloneModeManager::GetInstance().SetAppConcurrencyMode(ownerPid, appUid, mode);
    EXPECT_EQ(ret, 0);
    ownerPid = 0;
    ret = StandaloneModeManager::GetInstance().SetAppConcurrencyMode(ownerPid, appUid, mode);
    EXPECT_EQ(ret, -1);
    ownerPid = 1;
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(ownerPid);
}

/**
* @tc.name  : Test ResumeAllStandaloneApp.
* @tc.number: StandaloneModeManagerUnitTest_004
* @tc.desc  : Test ResumeAllStandaloneApp.
*/
HWTEST_F(StandaloneModeManagerUnitTest, StandaloneModeManagerUnitTest_004, TestSize.Level1)
{
    int32_t ownerPid = 1;
    int32_t appUid = 1;
    int32_t displayId = 1;
    int32_t sessionId = 1;
    bool isOnlyRecordUid = false;
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    auto ret = StandaloneModeManager::GetInstance().SetAppSilentOnDisplay(ownerPid, displayId);
    EXPECT_EQ(ret, 0);
    StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(
        appUid, isOnlyRecordUid, sessionId);
    ownerPid = 0;
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(ownerPid);
    ownerPid = 1;
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(ownerPid);
}

/**
* @tc.name  : Test ResumeAllStandaloneApp.
* @tc.number: StandaloneModeManagerUnitTest_005
* @tc.desc  : Test ResumeAllStandaloneApp.
*/
HWTEST_F(StandaloneModeManagerUnitTest, StandaloneModeManagerUnitTest_005, TestSize.Level1)
{
    auto standaloneModeManager = new StandaloneModeManager();
    EXPECT_NE(standaloneModeManager, nullptr);
    delete standaloneModeManager;
}

/**
* @tc.name  : Test ResumeAllStandaloneApp.
* @tc.number: StandaloneModeManagerUnitTest_006
* @tc.desc  : Test ResumeAllStandaloneApp.
*/
HWTEST_F(StandaloneModeManagerUnitTest, StandaloneModeManagerUnitTest_006, TestSize.Level1)
{
    int32_t appUid = 1;
    StandaloneModeManager::GetInstance().interruptService_ = nullptr;
    EXPECT_EQ(StandaloneModeManager::GetInstance().interruptService_, nullptr);
    StandaloneModeManager::GetInstance().RemoveExistingFocus(appUid);
    StandaloneModeManager::GetInstance().ExitStandaloneAndResumeFocus(appUid);
}

} // namespace AudioStandard
} // namespace OHOS