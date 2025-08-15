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
#include <gtest/gtest.h>
#include "audio_stream_checker.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const uint64_t DEFAULT_TIME = 3711509424L;

class AudioStreamCheckerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioStreamCheckerTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioStreamCheckerTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioStreamCheckerTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioStreamCheckerTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test InitChecker API
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_001
 */
HWTEST(AudioStreamCheckerTest, InitCheckerTest_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->InitChecker(para, 100000, 100000);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test RecordFrame API
 * @tc.type  : FUNC
 * @tc.number: RecordFrame_001
 */
HWTEST(AudioStreamCheckerTest, RecordFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->RecordMuteFrame();
    int32_t num = checker->checkParaVector_[0].muteFrameNum;
    EXPECT_GT(num, 0);
    checker->RecordNodataFrame();
    num = checker->checkParaVector_[0].noDataFrameNum;
    EXPECT_GT(num, 0);
    checker->RecordNormalFrame();
    num = checker->checkParaVector_[0].normalFrameCount;
    EXPECT_GT(num, 0);
}

/**
 * @tc.name  : Test GetAppUid API
 * @tc.type  : FUNC
 * @tc.number: GetAppUid_001
 */
HWTEST(AudioStreamCheckerTest, GetAppUid_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    cfg.appInfo.appUid = 20002000;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    int32_t uid = checker->GetAppUid();
    EXPECT_EQ(uid, 20002000);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerPara_001
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerPara_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->DeleteCheckerPara(100000, 100000);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_001
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 0;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->RecordMuteFrame();
    checker->RecordNormalFrame();
    checker->MonitorCheckFrame();
    DataTransferStateChangeType status = checker->checkParaVector_[0].lastStatus;
    EXPECT_EQ(status, DATA_TRANS_STOP);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_002
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 0;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    for (int i = 0; i < 4; i++) {
        checker->RecordNormalFrame();
    }
    checker->MonitorCheckFrame();
    DataTransferStateChangeType status = checker->checkParaVector_[0].lastStatus;
    EXPECT_EQ(status, DATA_TRANS_RESUME);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_003
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 0;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->RecordMuteFrame();
    checker->RecordNormalFrame();
    checker->MonitorCheckFrame();
    for (int i = 0; i < 4; i++) {
        checker->RecordNormalFrame();
    }
    checker->MonitorCheckFrame();
    DataTransferStateChangeType status = checker->checkParaVector_[0].lastStatus;
    EXPECT_EQ(status, DATA_TRANS_RESUME);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_001
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->MonitorOnAllCallback(AUDIO_STREAM_START, false);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_002
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 2;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->MonitorOnAllCallback(DATA_TRANS_RESUME, true);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_003
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 2;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->monitorSwitch_ = false;
    checker->MonitorOnAllCallback(DATA_TRANS_RESUME, true);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_001
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->OnRemoteAppDied(100000);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_001
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = false;
    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_002
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = true;
    checkerPara.isMonitorMuteFrame = true;
    checkerPara.isMonitorNoDataFrame = true;
    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_003
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = true;
    checkerPara.para.timeInterval = 2000000000;
    checkerPara.lastUpdateTime = ClockTime::GetCurNano();
    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_001
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = DATA_TRANS_STOP;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 60;
    float badFrameRatio = 0.5f;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_002
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = AUDIO_STREAM_PAUSE;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 60;
    float badFrameRatio = 0.5f;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_003
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = AUDIO_STREAM_START;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 60;
    float badFrameRatio = 0.5f;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_004
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = DATA_TRANS_RESUME;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}
/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_005
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = AUDIO_STREAM_PAUSE;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}
/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_006
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = DATA_TRANS_STOP;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnCallback_001
 */
HWTEST(AudioStreamCheckerTest, MonitorOnCallback_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->monitorSwitch_ = false;
    CheckerParam checkerPara;
    checker->MonitorOnCallback(AUDIO_STREAM_START, true, checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnCallback_002
 */
HWTEST(AudioStreamCheckerTest, MonitorOnCallback_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.sumFrameCount = 0;
    checkerPara.hasInitCheck = true;
    checker->MonitorOnCallback(AUDIO_STREAM_START, true, checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnCallback_003
 */
HWTEST(AudioStreamCheckerTest, MonitorOnCallback_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.sumFrameCount = 100;
    checkerPara.hasInitCheck = true;
    checker->MonitorOnCallback(AUDIO_STREAM_START, true, checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test RecordStandbyTime API
 * @tc.type  : FUNC
 * @tc.number: RecordStandbyTime_001
 */
HWTEST(AudioStreamCheckerTest, RecordStandbyTime_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->RecordStandbyTime(true);
    int64_t time = checker->checkParaVector_[0].standbyStartTime;
    EXPECT_GT(time, 0);
}

/**
 * @tc.name  : Test RecordStandbyTime API
 * @tc.type  : FUNC
 * @tc.number: RecordStandbyTime_002
 */
HWTEST(AudioStreamCheckerTest, RecordStandbyTime_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->RecordStandbyTime(false);
    int64_t time = checker->checkParaVector_[0].standbyStopTime;
    EXPECT_GT(time, 0);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_001
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.standbyStartTime = ClockTime::GetCurNano();
    checkerPara.standbyStopTime = checkerPara.standbyStartTime + 1000000000;
    int64_t abnormalFrameNum = 0;
    checker->CalculateFrameAfterStandby(checkerPara, abnormalFrameNum);
    EXPECT_GT(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_002
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.standbyStartTime = ClockTime::GetCurNano() - 1000000000;
    checkerPara.standbyStopTime = 0;
    int64_t abnormalFrameNum = 0;
    checker->CalculateFrameAfterStandby(checkerPara, abnormalFrameNum);
    EXPECT_GT(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_003
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.standbyStartTime = 0;
    checkerPara.lastUpdateTime = ClockTime::GetCurNano();
    checkerPara.standbyStopTime = checkerPara.lastUpdateTime + 1000000000;
    int64_t abnormalFrameNum = 0;
    checker->CalculateFrameAfterStandby(checkerPara, abnormalFrameNum);
    EXPECT_GT(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_004
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.standbyStartTime = 0;
    checkerPara.standbyStopTime = 0;
    int64_t abnormalFrameNum = 0;
    checker->CalculateFrameAfterStandby(checkerPara, abnormalFrameNum);
    EXPECT_EQ(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test UpdateAppState API
 * @tc.type  : FUNC
 * @tc.number: UpdateAppState_001
 */
HWTEST(AudioStreamCheckerTest, UpdateAppState_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->UpdateAppState(true);
    EXPECT_EQ(checker->isBackground_, true);
}

/**
 * @tc.name  : Test UpdateAppState API
 * @tc.type  : FUNC
 * @tc.number: UpdateAppState_002
 */
HWTEST(AudioStreamCheckerTest, UpdateAppState_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->UpdateAppState(false);
    EXPECT_EQ(checker->isBackground_, false);
}

/**
 * @tc.name  : Test InitChecker API
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_002
 */
HWTEST(AudioStreamCheckerTest, InitCheckerTest_002, TestSize.Level1)
{
    DataTransferMonitorParam para;
    int32_t pid = 100000;
    int32_t callbackId = 100000;
    CheckerParam checkerParamTest;

    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checkerParamTest.pid = pid;
    checkerParamTest.callbackId = callbackId;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);

    checker->InitChecker(para, pid, callbackId);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name  : Test InitChecker API
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_003
 */
HWTEST(AudioStreamCheckerTest, InitCheckerTest_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    DataTransferMonitorParam para;
    int32_t pid = 100000;
    int32_t callbackId = 100000;
    CheckerParam checkerParamTest;

    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = callbackId;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->isNeedCreateThread_.store(false);

    checker->InitChecker(para, pid, callbackId);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 2);
}

/**
 * @tc.name  : Test InitChecker API
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_004
 */
HWTEST(AudioStreamCheckerTest, InitCheckerTest_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    DataTransferMonitorParam para;
    int32_t pid = 100000;
    int32_t callbackId = 100000;
    CheckerParam checkerParamTest;
    
    checkerParamTest.pid = pid;
    checkerParamTest.callbackId = 0;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->isNeedCreateThread_.store(true);

    checker->InitChecker(para, pid, callbackId);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), false);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 2);
}

/**
 * @tc.name  : Test InitChecker API
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_005
 */
HWTEST(AudioStreamCheckerTest, InitCheckerTest_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    DataTransferMonitorParam para;
    int32_t pid = 100000;
    int32_t callbackId = 100000;
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = 0;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->isNeedCreateThread_.store(true);

    checker->InitChecker(para, pid, callbackId);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), false);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 2);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerParaTest_002
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerParaTest_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    DataTransferMonitorParam para;

    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = 0;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->isNeedCreateThread_.store(false);

    checker->InitChecker(para, 100000, 100000);
    checker->DeleteCheckerPara(100000, 100000);

    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), false);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerParaTest_003
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerParaTest_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    DataTransferMonitorParam para;

    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = 0;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->isNeedCreateThread_.store(false);

    checker->InitChecker(para, 100000, 100000);
    checker->DeleteCheckerPara(100000, 0);

    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 2);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), false);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerParaTest_004
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerParaTest_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    DataTransferMonitorParam para;

    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = 0;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->isNeedCreateThread_.store(false);

    checker->InitChecker(para, 100000, 100000);
    checker->DeleteCheckerPara(0, 100000);

    int32_t size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 2);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), false);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_005
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    int64_t abnormalFrameNum = 0;

    para.standbyStartTime = 1;
    para.standbyStopTime = 20000001;
    para.isMonitorNoDataFrame = true;
    checker->streamConfig_.rendererInfo.rendererFlags = 0;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_EQ(abnormalFrameNum, 1);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_006
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    int64_t abnormalFrameNum = 0;

    para.standbyStartTime = 0;
    para.standbyStopTime = 20000000;
    para.lastUpdateTime = 0;
    para.isMonitorNoDataFrame = true;
    checker->streamConfig_.rendererInfo.rendererFlags = 0;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_EQ(abnormalFrameNum, 1);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_002
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 200000);
    checker->InitChecker(para, 100001, 200001);

    checker->OnRemoteAppDied(100000);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_003
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 200000);
    checker->InitChecker(para, 100001, 200001);
    checker->InitChecker(para, 100002, 200002);

    checker->OnRemoteAppDied(100000);
    checker->OnRemoteAppDied(100001);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_004
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 200000);
    checker->InitChecker(para, 100001, 200001);

    checker->OnRemoteAppDied(100003);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 2);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_005
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->OnRemoteAppDied(100000);

    EXPECT_EQ(checker->isNeedCreateThread_.load(), true);
    EXPECT_EQ(checker->isKeepCheck_.load(), false);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_004
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->monitorSwitch_ = false;

    checker->MonitorCheckFrame();
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_007
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_007, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = AUDIO_STREAM_PAUSE;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;

    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_008
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_008, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = DATA_TRANS_STOP;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;

    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_004
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = DATA_TRANS_STOP;
    checkerPara.sumFrameCount = 100;
    checkerPara.hasInitCheck = false;

    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_005
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.lastStatus = DATA_TRANS_STOP;
    checkerPara.sumFrameCount = 100;
    checkerPara.hasInitCheck = true;
    checkerPara.isMonitorMuteFrame = true;

    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_004
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->monitorSwitch_ = false;
    DataTransferStateChangeType type = AUDIO_STREAM_START;

    checker->MonitorOnAllCallback(type, true);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_005
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = 0;
    checkerParamTest.isMonitorNoDataFrame = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);

    checker->monitorSwitch_ = true;
    DataTransferStateChangeType type = DATA_TRANS_RESUME;

    checker->MonitorOnAllCallback(type, true);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name  : Test RecordStandbyTime API
 * @tc.type  : FUNC
 * @tc.number: RecordStandbyTime_003
 */
HWTEST(AudioStreamCheckerTest, RecordStandbyTime_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.standbyStartTime = 0;
    checkerParamTest.isMonitorNoDataFrame = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);

    checker->RecordStandbyTime(false);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
    EXPECT_TRUE(checker->checkParaVector_[0].standbyStopTime > 0);
    EXPECT_EQ(checker->checkParaVector_[0].isInStandby, false);
}

/**
 * @tc.name  : Test MonitorOnCallback API
 * @tc.type  : FUNC
 * @tc.number: MMonitorOnCallback_004
 */
HWTEST(AudioStreamCheckerTest, MonitorOnCallback_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.lastUpdateTime = 0;
    checkerParamTest.hasInitCheck = true;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);

    DataTransferStateChangeType type = AUDIO_STREAM_START;
    checker->monitorSwitch_ = false;

    checker->MonitorOnCallback(type, true, checkerParamTest);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(size, 1);
    EXPECT_TRUE(checkerParamTest.lastUpdateTime > 0);
}

/**
 * @tc.name  : Test MonitorOnCallback API
 * @tc.type  : FUNC
 * @tc.number: MMonitorOnCallback_005
 */
HWTEST(AudioStreamCheckerTest, MonitorOnCallback_005, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.sumFrameCount = 1;
    checkerParamTest.lastUpdateTime = 0;
    checkerParamTest.hasInitCheck = true;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);

    DataTransferStateChangeType type = AUDIO_STREAM_START;
    checker->monitorSwitch_ = true;

    checker->MonitorOnCallback(type, true, checkerParamTest);
    EXPECT_EQ(checkerParamTest.lastStatus, AUDIO_STREAM_START);
    EXPECT_TRUE(checkerParamTest.lastUpdateTime > 0);
}

/**
 * @tc.name  : Test IsMonitorMuteFrame API
 * @tc.type  : FUNC
 * @tc.number: IsMonitorMuteFrame_001
 */
HWTEST(AudioStreamCheckerTest, IsMonitorMuteFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.hasInitCheck = false;

    bool ret = checker->IsMonitorMuteFrame(checkerParamTest);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsMonitorNoDataFrame API
 * @tc.type  : FUNC
 * @tc.number: IsMonitorNoDataFrame_001
 */
HWTEST(AudioStreamCheckerTest, IsMonitorNoDataFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.hasInitCheck = false;

    bool ret = checker->IsMonitorNoDataFrame(checkerParamTest);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerPara_002
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerPara_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->checkParaVector_.push_back({1, 1});
    checker->DeleteCheckerPara(1, 1);
    EXPECT_EQ(checker->checkParaVector_.size(), 0);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerPara_003
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerPara_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->checkParaVector_.push_back({1, 1});
    checker->DeleteCheckerPara(2, 2);
    EXPECT_EQ(checker->checkParaVector_.size(), 1);
}

/**
 * @tc.name  : Test DeleteCheckerPara API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckerPara_004
 */
HWTEST(AudioStreamCheckerTest, DeleteCheckerPara_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->DeleteCheckerPara(1, 1);
    EXPECT_EQ(checker->checkParaVector_.size(), 0);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_006
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    int32_t pid = 1234;
    checker->checkParaVector_.push_back({pid, 0});
    checker->OnRemoteAppDied(pid);
    EXPECT_EQ(checker->checkParaVector_.size(), 0);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), true);
    EXPECT_EQ(checker->isKeepCheck_.load(), false);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_007
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_007, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    int32_t pid = 1234;
    checker->checkParaVector_.push_back({5678, 0});
    checker->OnRemoteAppDied(pid);
    EXPECT_EQ(checker->checkParaVector_.size(), 1);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), true);
    EXPECT_NE(checker->isKeepCheck_.load(), true);
}

/**
 * @tc.name  : Test OnRemoteAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnRemoteAppDied_008
 */
HWTEST(AudioStreamCheckerTest, OnRemoteAppDied_008, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    int32_t pid = 1234;
    checker->OnRemoteAppDied(pid);
    EXPECT_EQ(checker->checkParaVector_.size(), 0);
    EXPECT_EQ(checker->isNeedCreateThread_.load(), true);
    EXPECT_EQ(checker->isKeepCheck_.load(), false);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_005
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_005, TestSize.Level0)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->monitorSwitch_ = false;
    checker->MonitorCheckFrame();
    EXPECT_FALSE(checker->monitorSwitch_);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_006
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_006, TestSize.Level0)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->monitorSwitch_ = true;
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.lastUpdateTime = 0;
    checkerParamTest.hasInitCheck = true;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest); // Assuming CheckPara is a valid struct
    checker->MonitorCheckFrame();
    EXPECT_TRUE(checker->monitorSwitch_);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_007
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_007, TestSize.Level0)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    para.standbyStartTime = 0;
    para.standbyStopTime = 0;
    int64_t abnormalFrameNum = 0;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_EQ(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_008
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_008, TestSize.Level0)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    para.standbyStartTime = 0;
    para.standbyStopTime = 200;
    int64_t abnormalFrameNum = 0;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_NE(abnormalFrameNum, 10);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_006
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_006, TestSize.Level0)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    para.hasInitCheck = false;
    para.para.clientUID = 12345;
    checker->MonitorCheckFrameSub(para);
    EXPECT_NE(para.lastStatus, -1); // Assuming lastStatus is updated in the function
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_007
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_007, TestSize.Level0)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    para.hasInitCheck = true;
    para.isMonitorMuteFrame = true;
    para.muteFrameNum = 10;
    checker->MonitorCheckFrameSub(para);
    EXPECT_NE(para.lastStatus, -1); // Assuming lastStatus is updated in the function
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_008
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_008, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->monitorSwitch_ = false;
    checker->MonitorOnAllCallback(DATA_TRANS_RESUME, false);
    EXPECT_FALSE(checker->monitorSwitch_);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_009
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_009, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->monitorSwitch_ = true;
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.lastUpdateTime = 0;
    checkerParamTest.hasInitCheck = true;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->MonitorOnAllCallback(DATA_TRANS_RESUME, true);
    EXPECT_TRUE(checker->monitorSwitch_);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_0010
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_010, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->monitorSwitch_ = true;
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.lastUpdateTime = 0;
    checkerParamTest.hasInitCheck = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);
    checker->MonitorOnAllCallback(DATA_TRANS_RESUME, false);
    EXPECT_TRUE(checker->monitorSwitch_);
}

/**
 * @tc.name  : Test CheckVolume API
 * @tc.type  : FUNC
 * @tc.number: CheckVolume_001
 */
HWTEST(AudioStreamCheckerTest, CheckVolume_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);

    checker->curVolume_ = 0.0f;
    checker->preVolume_ = 0.0f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, checker->preVolume_);

    checker->curVolume_ = 0.0f;
    checker->preVolume_ = 1.0f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, checker->preVolume_);

    checker->curVolume_ = 1.0f;
    checker->preVolume_ = 0.0f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, checker->preVolume_);

    checker->curVolume_ = 1.0f;
    checker->preVolume_ = 1.0f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, checker->preVolume_);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_008
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameSub_008, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = true;
    checkerPara.isMonitorMuteFrame = false;
    checkerPara.isMonitorNoDataFrame = true;
    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_NE(0, size);
}
 
/**
 * @tc.name  : Test MonitorOnCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnCallback_006
 */
HWTEST(AudioStreamCheckerTest, MonitorOnCallback_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checker->monitorSwitch_ = false;

    checker->MonitorOnCallback(AUDIO_STREAM_START, true, checkerPara);
    EXPECT_EQ(DATA_TRANS_RESUME, checkerPara.lastStatus);

    checkerPara.pid = 100;
    checker->MonitorOnCallback(AUDIO_STREAM_PAUSE, true, checkerPara);
    EXPECT_EQ(0, checkerPara.sumFrameCount);

    checker->monitorSwitch_ = true;
    checkerPara.sumFrameCount = 100;
    checker->MonitorOnCallback(AUDIO_STREAM_START, false, checkerPara);
    EXPECT_EQ(0, checkerPara.callbackId);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_009
 */
HWTEST(AudioStreamCheckerTest, CalculateFrameAfterStandby_009, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam para;
    int64_t abnormalFrameNum = 0;
    checker->streamConfig_.rendererInfo.rendererFlags = 0;

    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_EQ(false, para.isMonitorNoDataFrame);

    para.standbyStopTime = DEFAULT_TIME;
    para.isMonitorNoDataFrame = true;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_EQ(0, para.sumFrameCount);

    para.isInStandby = 1;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
    EXPECT_EQ(DATA_TRANS_RESUME, para.lastStatus);
}

/**
 * @tc.name  : Test MonitorCheckFrame API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrame_007
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrame_007, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    checker->monitorSwitch_ = true;
 
    checker->MonitorCheckFrame();
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(1, size);
}
 
/**
 * @tc.name  : Test MonitorCheckFrameAction API
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_009
 */
HWTEST(AudioStreamCheckerTest, MonitorCheckFrameAction_009, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    CheckerParam checkerPara;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;

    checkerPara.lastStatus = DATA_TRANS_STOP;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    EXPECT_EQ(0, checkerPara.noDataFrameNum);

    checkerPara.lastStatus = AUDIO_STREAM_STOP;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    EXPECT_EQ(0, checker->streamConfig_.originalSessionId);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_006
 */
HWTEST(AudioStreamCheckerTest, MonitorOnAllCallback_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.pid = 0;
    checkerParamTest.callbackId = 0;
    checkerParamTest.isMonitorNoDataFrame = false;
    checker->checkParaVector_.clear();
    checker->checkParaVector_.push_back(checkerParamTest);

    checker->monitorSwitch_ = true;
    DataTransferStateChangeType type = DATA_TRANS_RESUME;
 
    checker->MonitorOnAllCallback(type, false);
    int size = checker->checkParaVector_.size();
    EXPECT_EQ(1, size);
}

/**
 * @tc.name  : Test IsMonitorNoDataFrame API
 * @tc.type  : FUNC
 * @tc.number: IsMonitorNoDataFrame_002
 */
HWTEST(AudioStreamCheckerTest, IsMonitorNoDataFrame_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.hasInitCheck = true;
    bool ret = checker->IsMonitorNoDataFrame(checkerParamTest);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test CheckStreamThread API
 * @tc.type  : FUNC
 * @tc.number: CheckStreamThread_001
 */
HWTEST(AudioStreamCheckerTest, CheckStreamThread_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->CheckStreamThread();
    EXPECT_EQ(false, checker->isKeepCheck_);
}
 
/**
 * @tc.name  : Test IsMonitorMuteFrame API
 * @tc.type  : FUNC
 * @tc.number: IsMonitorMuteFrame_002
 */
HWTEST(AudioStreamCheckerTest, IsMonitorMuteFrame_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CheckerParam checkerParamTest;
    checkerParamTest.hasInitCheck = true;
 
    bool ret = checker->IsMonitorMuteFrame(checkerParamTest);
    EXPECT_EQ(false, ret);
}
}
}