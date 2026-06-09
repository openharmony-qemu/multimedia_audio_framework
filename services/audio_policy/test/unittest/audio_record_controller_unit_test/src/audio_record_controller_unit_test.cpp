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

#include "audio_record_controller_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_background_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr int32_t TEST_UID = 1001;
constexpr int32_t TEST_UID_OFFSET_CAMCORDER = 1;
constexpr int32_t TEST_UID_OFFSET_LIVE = 2;
constexpr int32_t TEST_UID_OFFSET_DISABLE = 10;
constexpr int32_t TEST_UID_OFFSET_NON_MONITORED = 20;
constexpr int32_t TEST_UID_OFFSET_APP_STATE = 30;
constexpr int32_t TEST_UID_OFFSET_APP_STATE_NON_MONITORED = 40;
constexpr int32_t TEST_UID_OFFSET_APP_DIED = 50;
constexpr int32_t TEST_UID_OFFSET_APP_DIED_NON_MONITORED = 60;
constexpr int32_t TEST_UID_OFFSET_LIFECYCLE = 70;
constexpr int32_t TEST_UID_OFFSET_SESSION = 80;
constexpr int32_t TEST_UID_OFFSET_SESSION_CAMCORDER = 81;
constexpr int32_t TEST_UID_OFFSET_SESSION_LIVE = 82;
constexpr int32_t TEST_UID_OFFSET_FOREGROUND_BG = 90;
constexpr int32_t TEST_UID_OFFSET_FOREGROUND_FG = 91;
constexpr int32_t TEST_UID_OFFSET_DEDUP = 92;
constexpr int32_t TEST_UID_OFFSET_MULTI_APP = 100;
constexpr int32_t TEST_UID_OFFSET_BG_NO_SESSION = 110;
constexpr int32_t TEST_UID_OFFSET_ON_RECORD = 120;
constexpr int32_t TEST_UID_OFFSET_ON_RECORD_STOP = 130;
constexpr int32_t TEST_UID_OFFSET_PRIORITY = 140;
constexpr int32_t TEST_UID_OFFSET_MULTI_SESSION = 150;
constexpr int32_t TEST_UID_OFFSET_SESSION_FALLBACK = 160;
constexpr int32_t TEST_UID_OFFSET_RE_ENABLE_DIFF = 170;
constexpr int32_t TEST_UID_OFFSET_ON_RECORD_START_INVALID = 180;
constexpr int32_t TEST_PID = 2001;
constexpr uint32_t MOCK_SESSION_ID = 100;
constexpr uint32_t MOCK_SESSION_ID_2 = 200;
constexpr uint32_t MOCK_SESSION_ID_3 = 300;

static bool HasMonitoredApp(AudioRecordController &controller, int32_t uid, int32_t pid,
    uint32_t sessionId, SourceType sourceType)
{
    for (const auto &item : controller.monitoredApps_) {
        if (item.uid == uid && item.pid == pid && item.sessionId == sessionId && item.sourceType == sourceType) {
            return true;
        }
    }
    return false;
}

static size_t CountMonitoredAppsByUidPid(AudioRecordController &controller, int32_t uid, int32_t pid)
{
    size_t count = 0;
    for (const auto &item : controller.monitoredApps_) {
        if (item.uid == uid && item.pid == pid) {
            count++;
        }
    }
    return count;
}

void AudioRecordControllerUnitTest::SetUpTestCase(void) {}
void AudioRecordControllerUnitTest::TearDownTestCase(void) {}
void AudioRecordControllerUnitTest::SetUp(void) {}
void AudioRecordControllerUnitTest::TearDown(void) {}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_INVALID;
    int32_t ret = controller.EnableSystemRecordController(true, config, TEST_UID, TEST_PID);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t ret = controller.EnableSystemRecordController(true, config, TEST_UID, TEST_PID);
    EXPECT_EQ(ret, SUCCESS);
    controller.EnableSystemRecordController(false, config, TEST_UID, TEST_PID);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_003, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_CAMCORDER;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_CAMCORDER;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_CAMCORDER;
    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_004, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_LIVE;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_LIVE;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_LIVE;
    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_005, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_DISABLE;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_DISABLE;
    int32_t ret1 = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret1, SUCCESS);
    int32_t ret = controller.EnableSystemRecordController(false, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_006, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_NON_MONITORED;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_NON_MONITORED;
    int32_t ret = controller.EnableSystemRecordController(false, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioRecordControllerUnitTest, OnAppStateChanged_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_APP_STATE;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_APP_STATE;
    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    controller.OnAppStateChanged(uid, pid, false);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnAppStateChanged_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_APP_STATE_NON_MONITORED;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_APP_STATE_NON_MONITORED;
    controller.OnAppStateChanged(uid, pid, true);
}

HWTEST(AudioRecordControllerUnitTest, OnAppDied_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_CAMCORDER;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_APP_DIED;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_APP_DIED;
    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    controller.OnAppDied(uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnAppDied_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_APP_DIED_NON_MONITORED;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_APP_DIED_NON_MONITORED;
    controller.OnAppDied(uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStart_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD_START_INVALID;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD_START_INVALID;
    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_INVALID);
    EXPECT_FALSE(HasMonitoredApp(controller, uid, pid, MOCK_SESSION_ID, SOURCE_TYPE_INVALID));
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStop_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD_START_INVALID + 1;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD_START_INVALID + 1;
    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_INVALID);
    EXPECT_FALSE(HasMonitoredApp(controller, uid, pid, MOCK_SESSION_ID, SOURCE_TYPE_INVALID));
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_007, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_LIFECYCLE;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_LIFECYCLE;

    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);

    ret = controller.EnableSystemRecordController(false, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);

    ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnAppStateChanged_003, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_SESSION;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_SESSION;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    controller.OnAppStateChanged(uid, pid, true);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnAppStateChanged_004, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig configMic;
    configMic.sourceType = SOURCE_TYPE_MIC;
    SystemRecordControllerConfig configCamcorder;
    configCamcorder.sourceType = SOURCE_TYPE_CAMCORDER;
    int32_t uid1 = TEST_UID + TEST_UID_OFFSET_SESSION + 1;
    int32_t pid1 = TEST_PID + TEST_UID_OFFSET_SESSION + 1;
    int32_t uid2 = TEST_UID + TEST_UID_OFFSET_SESSION + 2;
    int32_t pid2 = TEST_PID + TEST_UID_OFFSET_SESSION + 2;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid1, pid1, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, configMic, uid1, pid1);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid1);

    controller.OnRecordStart(MOCK_SESSION_ID, uid2, pid2, SOURCE_TYPE_CAMCORDER);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid2);

    controller.OnAppStateChanged(uid1, pid1, true);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid2);

    controller.OnRecordStop(MOCK_SESSION_ID, uid2, pid2, SOURCE_TYPE_CAMCORDER);
    controller.EnableSystemRecordController(false, configMic, uid1, pid1);
}

HWTEST(AudioRecordControllerUnitTest, OnAppStateChanged_005, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig configLive;
    configLive.sourceType = SOURCE_TYPE_LIVE;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_SESSION_LIVE;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_SESSION_LIVE;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, configLive, uid, pid);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);

    controller.OnRecordStart(MOCK_SESSION_ID_2, uid, pid, SOURCE_TYPE_LIVE);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);

    SystemRecordControllerChangeInfo beforeChange = controller.lastChangeInfo_;
    controller.OnAppStateChanged(uid, pid, false);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, beforeChange.enabled);
    EXPECT_EQ(controller.lastChangeInfo_.uid, beforeChange.uid);

    controller.OnRecordStop(MOCK_SESSION_ID_2, uid, pid, SOURCE_TYPE_LIVE);
    controller.EnableSystemRecordController(false, configLive, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_008, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig configMic;
    configMic.sourceType = SOURCE_TYPE_MIC;
    SystemRecordControllerConfig configCamcorder;
    configCamcorder.sourceType = SOURCE_TYPE_CAMCORDER;
    int32_t uid1 = TEST_UID + TEST_UID_OFFSET_SESSION_CAMCORDER;
    int32_t pid1 = TEST_PID + TEST_UID_OFFSET_SESSION_CAMCORDER;
    int32_t uid2 = TEST_UID + TEST_UID_OFFSET_SESSION_CAMCORDER + 1;
    int32_t pid2 = TEST_PID + TEST_UID_OFFSET_SESSION_CAMCORDER + 1;

    controller.OnRecordStart(MOCK_SESSION_ID, uid1, pid1, SOURCE_TYPE_CAMCORDER);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid1);

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid2, pid2, STATE_FOREGROUND);
    int32_t ret = controller.EnableSystemRecordController(true, configMic, uid2, pid2);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid1);

    controller.OnRecordStop(MOCK_SESSION_ID, uid1, pid1, SOURCE_TYPE_CAMCORDER);
    controller.EnableSystemRecordController(false, configMic, uid2, pid2);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStart_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerChangeInfo beforeInfo = controller.lastChangeInfo_;
    size_t beforeSize = controller.monitoredApps_.size();
    controller.OnRecordStart(MOCK_SESSION_ID, TEST_UID, TEST_PID, SOURCE_TYPE_INVALID);
    EXPECT_EQ(controller.lastChangeInfo_, beforeInfo);
    EXPECT_EQ(controller.monitoredApps_.size(), beforeSize);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStop_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD_STOP;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD_STOP;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);

    SystemRecordControllerChangeInfo beforeInfo = controller.lastChangeInfo_;
    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_INVALID);
    EXPECT_EQ(controller.lastChangeInfo_, beforeInfo);

    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_009, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_FOREGROUND_BG;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_FOREGROUND_BG;
    controller.lastChangeInfo_.enabled = true;
    controller.lastChangeInfo_.uid = uid;
    controller.lastChangeInfo_.sourceType = SOURCE_TYPE_MIC;
    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));
    EXPECT_EQ(controller.lastChangeInfo_.enabled, false);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_010, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_FOREGROUND_FG;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_FOREGROUND_FG;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    int32_t ret = controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_MIC);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, NotifyEnabledChange_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_DEDUP;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_DEDUP;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    controller.OnAppStateChanged(uid, pid, true);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, NotifyEnabledChange_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_DEDUP + 1;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_DEDUP + 1;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    controller.OnAppStateChanged(uid, pid, false);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, false);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnAppStateChanged_006, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_BG_NO_SESSION;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_BG_NO_SESSION;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    controller.OnAppStateChanged(uid, pid, false);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, false);
    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_011, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig configMic;
    configMic.sourceType = SOURCE_TYPE_MIC;
    SystemRecordControllerConfig configCamcorder;
    configCamcorder.sourceType = SOURCE_TYPE_CAMCORDER;
    int32_t uid1 = TEST_UID + TEST_UID_OFFSET_MULTI_APP;
    int32_t uid2 = TEST_UID + TEST_UID_OFFSET_MULTI_APP + 1;
    int32_t pid1 = TEST_PID + TEST_UID_OFFSET_MULTI_APP;
    int32_t pid2 = TEST_PID + TEST_UID_OFFSET_MULTI_APP + 1;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid1, pid1, STATE_FOREGROUND);
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid2, pid2, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, configMic, uid1, pid1);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid1);
    controller.EnableSystemRecordController(true, configCamcorder, uid2, pid2);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid2);
    EXPECT_TRUE(HasMonitoredApp(controller, uid1, pid1, 0, SOURCE_TYPE_MIC));
    EXPECT_TRUE(HasMonitoredApp(controller, uid2, pid2, 0, SOURCE_TYPE_CAMCORDER));
    controller.EnableSystemRecordController(false, configMic, uid1, pid1);
    controller.EnableSystemRecordController(false, configCamcorder, uid2, pid2);
}

HWTEST(AudioRecordControllerUnitTest, EnableSystemRecordController_012, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig configMic;
    configMic.sourceType = SOURCE_TYPE_MIC;
    SystemRecordControllerConfig configLive;
    configLive.sourceType = SOURCE_TYPE_LIVE;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_RE_ENABLE_DIFF;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_RE_ENABLE_DIFF;
    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, configMic, uid, pid);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_MIC);
    controller.EnableSystemRecordController(true, configLive, uid, pid);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_LIVE));
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_LIVE);
    controller.EnableSystemRecordController(false, configLive, uid, pid);
    controller.EnableSystemRecordController(false, configMic, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStart_003, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD;
    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, MOCK_SESSION_ID, SOURCE_TYPE_MIC));
    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStart_004, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD + 1;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD + 1;
    controller.lastChangeInfo_ = {};
    controller.OnRecordStart(MOCK_SESSION_ID_2, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_MIC);
    controller.OnRecordStop(MOCK_SESSION_ID_2, uid, pid, SOURCE_TYPE_MIC);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStop_003, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD_STOP;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD_STOP;
    controller.lastChangeInfo_ = {};
    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_FALSE(HasMonitoredApp(controller, uid, pid, MOCK_SESSION_ID, SOURCE_TYPE_MIC));
    EXPECT_EQ(controller.lastChangeInfo_.enabled, false);
}

HWTEST(AudioRecordControllerUnitTest, PrioritySorting_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uidMic = TEST_UID + TEST_UID_OFFSET_PRIORITY;
    int32_t pidMic = TEST_PID + TEST_UID_OFFSET_PRIORITY;
    int32_t uidCam = TEST_UID + TEST_UID_OFFSET_PRIORITY + 1;
    int32_t pidCam = TEST_PID + TEST_UID_OFFSET_PRIORITY + 1;

    controller.OnRecordStart(MOCK_SESSION_ID, uidMic, pidMic, SOURCE_TYPE_MIC);
    controller.OnRecordStart(MOCK_SESSION_ID_2, uidCam, pidCam, SOURCE_TYPE_CAMCORDER);

    size_t micIdx = controller.monitoredApps_.size();
    size_t camIdx = controller.monitoredApps_.size();
    for (size_t i = 0; i < controller.monitoredApps_.size(); i++) {
        const auto &item = controller.monitoredApps_[i];
        if (item.uid == uidMic && item.sessionId == MOCK_SESSION_ID && item.sourceType == SOURCE_TYPE_MIC) {
            micIdx = i;
        }
        if (item.uid == uidCam && item.sessionId == MOCK_SESSION_ID_2 && item.sourceType == SOURCE_TYPE_CAMCORDER) {
            camIdx = i;
        }
    }
    EXPECT_LT(micIdx, camIdx);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uidCam);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_CAMCORDER);

    controller.OnRecordStop(MOCK_SESSION_ID, uidMic, pidMic, SOURCE_TYPE_MIC);
    controller.OnRecordStop(MOCK_SESSION_ID_2, uidCam, pidCam, SOURCE_TYPE_CAMCORDER);
}

HWTEST(AudioRecordControllerUnitTest, PrioritySorting_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uidMic = TEST_UID + TEST_UID_OFFSET_PRIORITY + 2;
    int32_t pidMic = TEST_PID + TEST_UID_OFFSET_PRIORITY + 2;
    int32_t uidLive = TEST_UID + TEST_UID_OFFSET_PRIORITY + 3;
    int32_t pidLive = TEST_PID + TEST_UID_OFFSET_PRIORITY + 3;

    controller.OnRecordStart(MOCK_SESSION_ID, uidMic, pidMic, SOURCE_TYPE_MIC);
    controller.OnRecordStart(MOCK_SESSION_ID_2, uidLive, pidLive, SOURCE_TYPE_LIVE);

    EXPECT_EQ(controller.lastChangeInfo_.uid, uidLive);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_LIVE);

    controller.OnRecordStop(MOCK_SESSION_ID, uidMic, pidMic, SOURCE_TYPE_MIC);
    controller.OnRecordStop(MOCK_SESSION_ID_2, uidLive, pidLive, SOURCE_TYPE_LIVE);
}

HWTEST(AudioRecordControllerUnitTest, MultiSession_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_MULTI_SESSION;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_MULTI_SESSION;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);

    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, MOCK_SESSION_ID, SOURCE_TYPE_MIC));
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);

    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_FALSE(HasMonitoredApp(controller, uid, pid, MOCK_SESSION_ID, SOURCE_TYPE_MIC));
    EXPECT_TRUE(HasMonitoredApp(controller, uid, pid, 0, SOURCE_TYPE_MIC));

    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, SessionFallback_001, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_SESSION_FALLBACK;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_SESSION_FALLBACK;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, config, uid, pid);
    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);

    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);

    controller.EnableSystemRecordController(false, config, uid, pid);
}

HWTEST(AudioRecordControllerUnitTest, SessionFallback_002, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_SESSION_FALLBACK + 1;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_SESSION_FALLBACK + 1;

    controller.lastChangeInfo_ = {};
    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, true);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);

    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(controller.lastChangeInfo_.enabled, false);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uid);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStart_005, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD + 2;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD + 2;

    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(CountMonitoredAppsByUidPid(controller, uid, pid), 1u);

    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(CountMonitoredAppsByUidPid(controller, uid, pid), 1u);

    controller.OnRecordStop(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
}

HWTEST(AudioRecordControllerUnitTest, OnAppDied_003, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID + TEST_UID_OFFSET_MULTI_SESSION + 1;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_MULTI_SESSION + 1;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, STATE_FOREGROUND);
    controller.EnableSystemRecordController(true, config, uid, pid);
    controller.OnRecordStart(MOCK_SESSION_ID, uid, pid, SOURCE_TYPE_MIC);
    EXPECT_EQ(CountMonitoredAppsByUidPid(controller, uid, pid), 2u);

    controller.OnAppDied(uid, pid);
    EXPECT_EQ(CountMonitoredAppsByUidPid(controller, uid, pid), 0u);
}

HWTEST(AudioRecordControllerUnitTest, OnRecordStart_006, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    int32_t uid = TEST_UID + TEST_UID_OFFSET_ON_RECORD + 3;
    int32_t pid = TEST_PID + TEST_UID_OFFSET_ON_RECORD + 3;
    int32_t uidCam = TEST_UID + TEST_UID_OFFSET_ON_RECORD + 4;
    int32_t pidCam = TEST_PID + TEST_UID_OFFSET_ON_RECORD + 4;

    controller.OnRecordStart(0, uid, pid, SOURCE_TYPE_MIC);
    controller.OnRecordStart(MOCK_SESSION_ID, uidCam, pidCam, SOURCE_TYPE_CAMCORDER);

    EXPECT_EQ(controller.lastChangeInfo_.uid, uidCam);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_CAMCORDER);

    controller.OnRecordStop(0, uid, pid, SOURCE_TYPE_MIC);
    controller.OnRecordStop(MOCK_SESSION_ID, uidCam, pidCam, SOURCE_TYPE_CAMCORDER);
}

HWTEST(AudioRecordControllerUnitTest, PrioritySorting_003, TestSize.Level1)
{
    auto &controller = AudioRecordController::GetInstance();
    SystemRecordControllerConfig configMic;
    configMic.sourceType = SOURCE_TYPE_MIC;
    int32_t uidMic = TEST_UID + TEST_UID_OFFSET_PRIORITY + 4;
    int32_t pidMic = TEST_PID + TEST_UID_OFFSET_PRIORITY + 4;
    int32_t uidCamSession = TEST_UID + TEST_UID_OFFSET_PRIORITY + 5;
    int32_t pidCamSession = TEST_PID + TEST_UID_OFFSET_PRIORITY + 5;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uidMic, pidMic, STATE_FOREGROUND);
    controller.lastChangeInfo_ = {};
    controller.EnableSystemRecordController(true, configMic, uidMic, pidMic);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uidMic);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_MIC);

    controller.OnRecordStart(MOCK_SESSION_ID_3, uidCamSession, pidCamSession, SOURCE_TYPE_CAMCORDER);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uidCamSession);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_CAMCORDER);

    controller.OnRecordStop(MOCK_SESSION_ID_3, uidCamSession, pidCamSession, SOURCE_TYPE_CAMCORDER);
    EXPECT_EQ(controller.lastChangeInfo_.uid, uidMic);
    EXPECT_EQ(controller.lastChangeInfo_.sourceType, SOURCE_TYPE_MIC);

    controller.EnableSystemRecordController(false, configMic, uidMic, pidMic);
}

} // namespace AudioStandard
} // namespace OHOS
