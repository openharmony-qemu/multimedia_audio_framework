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

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

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

}
}