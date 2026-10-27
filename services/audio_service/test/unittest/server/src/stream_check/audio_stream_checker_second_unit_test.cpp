/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "audio_stream_checker_thread.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const uint64_t DEFAULT_TIME = 3711509424L;

class AudioStreamCheckerSecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioStreamCheckerSecondTest::SetUpTestCase(void)
{
}

void AudioStreamCheckerSecondTest::TearDownTestCase(void)
{
}

void AudioStreamCheckerSecondTest::SetUp(void)
{
}

void AudioStreamCheckerSecondTest::TearDown(void)
{
}

/**
 * @tc.name  : Test InitChecker API - with null pid and callbackId
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_006
 */
HWTEST(AudioStreamCheckerSecondTest, InitCheckerTest_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    // Test with pid=0 and callbackId=0, should add new item
    checker->InitChecker(para, 0, 0);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test InitChecker API - with different pid but same callbackId
 * @tc.type  : FUNC
 * @tc.number: InitCheckerTest_007
 */
HWTEST(AudioStreamCheckerSecondTest, InitCheckerTest_007, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    // First init
    checker->InitChecker(para, 100000, 100000);
    // Second init with different pid but same callbackId, should not add
    checker->InitChecker(para, 100001, 100000);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_NE(size, 1);
}

/**
 * @tc.name  : Test RecordMuteFrame API - with empty vector
 * @tc.type  : FUNC
 * @tc.number: RecordMuteFrame_001
 */
HWTEST(AudioStreamCheckerSecondTest, RecordMuteFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    // No checker para vector, should do nothing
    checker->RecordMuteFrame();
    EXPECT_TRUE(checker->checkParaVector_.empty());
}

/**
 * @tc.name  : Test RecordNodataFrame API - with empty vector
 * @tc.type  : FUNC
 * @tc.number: RecordNodataFrame_001
 */
HWTEST(AudioStreamCheckerSecondTest, RecordNodataFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    // No checker para vector, should do nothing
    checker->RecordNodataFrame();
    EXPECT_TRUE(checker->checkParaVector_.empty());
}

/**
 * @tc.name  : Test RecordNormalFrame API - with empty vector
 * @tc.type  : FUNC
 * @tc.number: RecordNormalFrame_001
 */
HWTEST(AudioStreamCheckerSecondTest, RecordNormalFrame_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    // No checker para vector, should do nothing
    checker->RecordNormalFrame();
    EXPECT_TRUE(checker->checkParaVector_.empty());
}

/**
 * @tc.name  : Test CheckVolume API - volume not changed
 * @tc.type  : FUNC
 * @tc.number: CheckVolume_002
 */
HWTEST(AudioStreamCheckerSecondTest, CheckVolume_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    checker->curVolume_ = 0.5f;
    checker->preVolume_ = 0.5f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, checker->preVolume_);
}

/**
 * @tc.name  : Test CheckVolume API - change to mute
 * @tc.type  : FUNC
 * @tc.number: CheckVolume_003
 */
HWTEST(AudioStreamCheckerSecondTest, CheckVolume_003, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->curVolume_ = 0.0f;
    checker->preVolume_ = 0.5f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, 0.0f);
    EXPECT_EQ(checker->preVolume_, 0.0f);
}

/**
 * @tc.name  : Test CheckVolume API - change to unmute
 * @tc.type  : FUNC
 * @tc.number: CheckVolume_004
 */
HWTEST(AudioStreamCheckerSecondTest, CheckVolume_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->curVolume_ = 0.5f;
    checker->preVolume_ = 0.0f;
    checker->CheckVolume();
    EXPECT_EQ(checker->curVolume_, 0.5f);
    EXPECT_EQ(checker->preVolume_, 0.5f);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API - with isMonitorNoDataFrame
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_008
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorCheckFrameSub_008, TestSize.Level1)
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
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.isMonitorMuteFrame = false;
    checker->MonitorCheckFrameSub(checkerPara);
    int size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameSub API - with timeCost < timeInterval
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameSub_009
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorCheckFrameSub_009, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;  // 2 seconds
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = true;
    checkerPara.isMonitorMuteFrame = true;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.lastUpdateTime = ClockTime::GetCurNano();  // Just now
    checkerPara.muteFrameNum = 10;
    checkerPara.noDataFrameNum = 10;
    
    // timeCost will be very small, less than timeInterval
    checker->MonitorCheckFrameSub(checkerPara);
    EXPECT_EQ(checkerPara.muteFrameNum, 0);
}

/**
 * @tc.name  : Test MonitorOnCallback API - with sumFrameCount > 0 and isNeedCallback
 * @tc.type  : FUNC
 * @tc.number: MonitorOnCallback_006
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorOnCallback_006, TestSize.Level1)
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
    checkerPara.sumFrameCount = 100;
    checkerPara.lastStatus = DATA_TRANS_STOP;
    
    checker->MonitorOnCallback(DATA_TRANS_STOP, true, checkerPara);
    EXPECT_EQ(checkerPara.lastStatus, DATA_TRANS_STOP);
}

/**
 * @tc.name  : Test MonitorOnCallback API - with sumFrameCount == 0
 * @tc.type  : FUNC
 * @tc.number: MonitorOnCallback_007
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorOnCallback_007, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = true;
    checkerPara.sumFrameCount = 0;
    checkerPara.lastStatus = DATA_TRANS_RESUME;
    
    checker->MonitorOnCallback(DATA_TRANS_RESUME, true, checkerPara);
    EXPECT_NE(checkerPara.lastStatus, -1);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API - playback mode with AUDIOSTREAM_LATENCY_MODE_NORMAL
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_009
 */
HWTEST(AudioStreamCheckerSecondTest, CalculateFrameAfterStandby_009, TestSize.Level1)
{
    AudioProcessConfig cfg;
    cfg.audioMode = AUDIO_MODE_PLAYBACK;
    cfg.rendererInfo.rendererFlags = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.abnormalStartTime = ClockTime::GetCurNano() - 200000000;  // 200ms ago
    checkerPara.abnormalStopTime = 0;
    int64_t abnormalFrameNum = 0;
    
    checker->CalculateFrameAfterStandby(checkerPara, abnormalFrameNum);
    // abnormalStartTime = now - 200ms, stopTime = 0 (DEFAULT_TIME).
    // CalculateFrameCommon: calculateFrameNum = (now - startVal) / NORMAL_FRAME_PER_TIME
    // = 200000000ns / 20000000ns(20ms) = 10. isMonitorNoDataFrame=true so abnormalFrameNum += 10.
    EXPECT_EQ(abnormalFrameNum, 10);
}

/**
 * @tc.name  : Test CalculateFrameAfterStandby API - playback mode with fast latency
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterStandby_010
 */
HWTEST(AudioStreamCheckerSecondTest, CalculateFrameAfterStandby_010, TestSize.Level1)
{
    AudioProcessConfig cfg;
    cfg.audioMode = AUDIO_MODE_PLAYBACK;
    cfg.rendererInfo.rendererFlags = 1;  // Not AUDIOSTREAM_LATENCY_MODE_NORMAL
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);

    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.abnormalStartTime = ClockTime::GetCurNano() - 5000000;  // 5ms ago
    checkerPara.abnormalStopTime = 0;
    int64_t abnormalFrameNum = 0;

    checker->CalculateFrameAfterStandby(checkerPara, abnormalFrameNum);
    EXPECT_EQ(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test CalculateFrameAfterOverflow API - record mode
 * @tc.type  : FUNC
 * @tc.number: CalculateFrameAfterOverflow_001
 */
HWTEST(AudioStreamCheckerSecondTest, CalculateFrameAfterOverflow_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    cfg.audioMode = AUDIO_MODE_RECORD;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);

    CheckerParam checkerPara;
    checkerPara.isMonitorNoDataFrame = true;
    checkerPara.abnormalStartTime = ClockTime::GetCurNano() - 200000000;
    checkerPara.abnormalStopTime = 0;
    int64_t abnormalFrameNum = 0;

    checker->CalculateFrameAfterOverflow(checkerPara, abnormalFrameNum);
    EXPECT_EQ(abnormalFrameNum, 0);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API - lastStatus AUDIO_STREAM_START
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_009
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorCheckFrameAction_009, TestSize.Level1)
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
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;
    
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    EXPECT_EQ(checkerPara.lastStatus, DATA_TRANS_RESUME);
}

/**
 * @tc.name  : Test MonitorCheckFrameAction API - lastStatus AUDIO_STREAM_STOP
 * @tc.type  : FUNC
 * @tc.number: MonitorCheckFrameAction_010
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorCheckFrameAction_010, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    CheckerParam checkerPara;
    checkerPara.lastStatus = AUDIO_STREAM_STOP;
    checkerPara.sumFrameCount = 100;
    int64_t abnormalFrameNum = 40;
    float badFrameRatio = 0.5f;

    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
    EXPECT_NE(checkerPara.lastStatus, AUDIO_STREAM_STOP);
}

/**
 * @tc.name  : Test SetVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_001
 */
HWTEST(AudioStreamCheckerSecondTest, SetVolume_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    checker->SetVolume(0.5f);
    float volume = checker->GetVolume();
    EXPECT_EQ(volume, 0.5f);
}

/**
 * @tc.name  : Test SetVolume API - same volume
 * @tc.type  : FUNC
 * @tc.number: SetVolume_002
 */
HWTEST(AudioStreamCheckerSecondTest, SetVolume_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    checker->curVolume_ = 0.5f;
    checker->SetVolume(0.5f);
    float volume = checker->GetVolume();
    EXPECT_EQ(volume, 0.5f);
}

/**
 * @tc.name  : Test RecordStandbyTime API - with empty vector
 * @tc.type  : FUNC
 * @tc.number: RecordStandbyTime_004
 */
HWTEST(AudioStreamCheckerSecondTest, RecordStandbyTime_004, TestSize.Level1)
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    
    // No checker para vector, should do nothing
    checker->RecordStandbyTime(true);
    EXPECT_TRUE(checker->checkParaVector_.empty());
}

/**
 * @tc.name  : Test RecordOverflowTime API - start
 * @tc.type  : FUNC
 * @tc.number: RecordOverflowTime_001
 */
HWTEST(AudioStreamCheckerSecondTest, RecordOverflowTime_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->RecordOverflowTime(true);
    int64_t time = checker->checkParaVector_[0].abnormalStartTime;
    EXPECT_GT(time, 0);
}

/**
 * @tc.name  : Test RecordOverflowTime API - stop
 * @tc.type  : FUNC
 * @tc.number: RecordOverflowTime_002
 */
HWTEST(AudioStreamCheckerSecondTest, RecordOverflowTime_002, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->RecordOverflowTime(false);
    int64_t time = checker->checkParaVector_[0].abnormalStopTime;
    EXPECT_GT(time, 0);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API - DATA_TRANS_STOP
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_006
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorOnAllCallback_006, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->MonitorOnAllCallback(DATA_TRANS_STOP, false);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API - AUDIO_STREAM_PAUSE
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_007
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorOnAllCallback_007, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->MonitorOnAllCallback(AUDIO_STREAM_PAUSE, false);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

/**
 * @tc.name  : Test MonitorOnAllCallback API - AUDIO_STREAM_STOP
 * @tc.type  : FUNC
 * @tc.number: MonitorOnAllCallback_008
 */
HWTEST(AudioStreamCheckerSecondTest, MonitorOnAllCallback_008, TestSize.Level1)
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = 3;
    para.timeInterval = 2000000000;
    para.badFramesRatio = 50;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    checker->InitChecker(para, 100000, 100000);
    
    checker->MonitorOnAllCallback(AUDIO_STREAM_STOP, false);
    int32_t size = checker->checkParaVector_.size();
    EXPECT_GT(size, 0);
}

} // namespace AudioStandard
} // namespace OHOS
