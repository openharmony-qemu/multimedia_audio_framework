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

#include "dfx_msg_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t DEFAULT_DFX_REPORT_INTERVAL_MIN = 24 * 60;
static constexpr int32_t MAX_DFX_MSG_MEMBER_SIZE = 100;

void DfxMsgManagerUnitTest::SetUpTestCase(void) {}
void DfxMsgManagerUnitTest::TearDownTestCase(void) {}
void DfxMsgManagerUnitTest::SetUp(void) {}
void DfxMsgManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: SaveAppInfo_001
* @tc.desc  : Test DfxMsgManager::SaveAppInfo
*/
HWTEST(DfxMsgManagerUnitTest, SaveAppInfo_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxRunningAppInfo info;

    info.appUid = 1;
    dfxMsgManager.SaveAppInfo(info);
    EXPECT_NE(dfxMsgManager.appInfo_.count(info.appUid), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: ProcessCheck_001
* @tc.desc  : Test DfxMsgManager::ProcessCheck
*/
HWTEST(DfxMsgManagerUnitTest, ProcessCheck_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;
    InterruptDfxInfo interruptInfo;
    CapturerDfxInfo captureInfo;

    msg.appUid = 1;
    for (int i = 0; i < 100; i++) {
        msg.renderInfo.push_back(renderInfo);
    }

    bool ret = dfxMsgManager.ProcessCheck(msg);
    EXPECT_EQ(ret, false);

    msg.renderInfo.clear();
    for (int i = 0; i < 100; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }

    ret = dfxMsgManager.ProcessCheck(msg);
    EXPECT_EQ(ret, false);

    msg.interruptInfo.clear();
    for (int i = 0; i < 100; i++) {
        msg.captureInfo.push_back(captureInfo);
    }

    ret = dfxMsgManager.ProcessCheck(msg);
    EXPECT_EQ(ret, false);
    msg.captureInfo.clear();

    dfxMsgManager.isFull_ = true;
    ret = dfxMsgManager.ProcessCheck(msg);
    EXPECT_EQ(ret, false);

    dfxMsgManager.isFull_ = false;
    for (uint32_t i = 0; i < 20; i++) {
        msg.appUid = i;
        dfxMsgManager.reportQueue_.insert(std::make_pair(i, msg));
    }
    ret = dfxMsgManager.ProcessCheck(msg);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: Process_001
* @tc.desc  : Test DfxMsgManager::Process
*/
HWTEST(DfxMsgManagerUnitTest, Process_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;

    msg.appUid = 105;
    bool ret = dfxMsgManager.Process(msg);
    EXPECT_EQ(ret, true);

    msg.appUid = 5;
    for (uint32_t i = 0; i < 10; i++) {
        msg.appUid = i;
        dfxMsgManager.reportQueue_.insert(std::make_pair(i, msg));
    }
    ret = dfxMsgManager.Process(msg);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: InsertReportQueue_001
* @tc.desc  : Test DfxMsgManager::InsertReportQueue
*/
HWTEST(DfxMsgManagerUnitTest, InsertReportQueue_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;

    dfxMsgManager.reportQueue_.clear();
    for (uint32_t i = 0; i < 10; i++) {
        msg.appUid = i;
        dfxMsgManager.reportQueue_.insert(std::make_pair(i, msg));
    }
    dfxMsgManager.InsertReportQueue(msg);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 10);

    dfxMsgManager.reportQueue_.clear();
    for (uint32_t i = 0; i < 5; i++) {
        msg.appUid = i;
        dfxMsgManager.reportQueue_.insert(std::make_pair(i, msg));
    }
    msg.appUid = 100;
    dfxMsgManager.InsertReportQueue(msg);
    EXPECT_NE(dfxMsgManager.reportQueue_.size(), 0);

    dfxMsgManager.reportQueue_.clear();
    for (uint32_t i = 0; i < 5; i++) {
        msg.appUid = i;
        msg.renderInfo.push_back(renderInfo);
        dfxMsgManager.reportQueue_.insert(std::make_pair(i, msg));
    }
    msg.appUid = 1;
    dfxMsgManager.InsertReportQueue(msg);
    EXPECT_NE(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: ProcessInner_001
* @tc.desc  : Test DfxMsgManager::ProcessInner
*/
HWTEST(DfxMsgManagerUnitTest, ProcessInner_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<RenderDfxInfo> dfxInfo;
    std::list<RenderDfxInfo> curDfxInfo;
    RenderDfxInfo renderInfo;

    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(ret, false);

    for (int i = 0; i < 10; i++) {
        dfxInfo.push_back(renderInfo);
    }
    dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: ProcessInner_002
* @tc.desc  : Test DfxMsgManager::ProcessInner
*/
HWTEST(DfxMsgManagerUnitTest, ProcessInner_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<InterruptDfxInfo> dfxInfo;
    std::list<InterruptDfxInfo> curDfxInfo;
    InterruptDfxInfo renderInfo;

    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(ret, false);

    for (int i = 0; i < 10; i++) {
        dfxInfo.push_back(renderInfo);
    }
    dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: ProcessInner_003
* @tc.desc  : Test DfxMsgManager::ProcessInner
*/
HWTEST(DfxMsgManagerUnitTest, ProcessInner_003, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<CapturerDfxInfo> dfxInfo;
    std::list<CapturerDfxInfo> curDfxInfo;
    CapturerDfxInfo renderInfo;

    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(ret, false);

    for (int i = 0; i < 10; i++) {
        dfxInfo.push_back(renderInfo);
    }
    dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: Enqueue_001
* @tc.desc  : Test DfxMsgManager::Enqueue
*/
HWTEST(DfxMsgManagerUnitTest, Enqueue_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;

    dfxMsgManager.isFull_ = true;
    bool ret = dfxMsgManager.Enqueue(msg);
    EXPECT_EQ(ret, false);

    dfxMsgManager.isFull_ = false;
    ret = dfxMsgManager.Enqueue(msg);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: HandleToHiSysEvent_001
* @tc.desc  : Test DfxMsgManager::HandleToHiSysEvent
*/
HWTEST(DfxMsgManagerUnitTest, HandleToHiSysEvent_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;

    dfxMsgManager.reportedCnt_ = 100;
    dfxMsgManager.reportQueue_.clear();
    dfxMsgManager.HandleToHiSysEvent(msg);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);

    dfxMsgManager.reportedCnt_ = 0;
    dfxMsgManager.reportQueue_.clear();
    dfxMsgManager.HandleToHiSysEvent(msg);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: WriteRenderMsg_001
* @tc.desc  : Test DfxMsgManager::WriteRenderMsg
*/
HWTEST(DfxMsgManagerUnitTest, WriteRenderMsg_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();

    msg.appUid = 1;
    dfxMsgManager.reportQueue_.clear();
    for (int i = 0; i < 10; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.WriteRenderMsg(msg, bean);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: WriteInterruptMsg_001
* @tc.desc  : Test DfxMsgManager::WriteInterruptMsg
*/
HWTEST(DfxMsgManagerUnitTest, WriteInterruptMsg_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    InterruptDfxInfo interruptInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();

    msg.appUid = 1;
    for (int i = 0; i < 10; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }
    dfxMsgManager.WriteInterruptMsg(msg, bean);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: WriteCapturerMsg_001
* @tc.desc  : Test DfxMsgManager::WriteCapturerMsg
*/
HWTEST(DfxMsgManagerUnitTest, WriteCapturerMsg_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    CapturerDfxInfo captureInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();

    msg.appUid = 1;
    captureInfo.capturerAction.fourthByte = CapturerStage::CAPTURER_STAGE_PAUSE_OK;
    for (int i = 0; i < 10; i++) {
        msg.captureInfo.push_back(captureInfo);
    }
    dfxMsgManager.WriteCapturerMsg(msg, bean);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);

    captureInfo.capturerAction.fourthByte = CapturerStage::CAPTURER_STAGE_START_FAIL;
    msg.captureInfo.push_back(captureInfo);
    dfxMsgManager.WriteCapturerMsg(msg, bean);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: WritePlayAudioStatsEvent_001
* @tc.desc  : Test DfxMsgManager::WritePlayAudioStatsEvent
*/
HWTEST(DfxMsgManagerUnitTest, WritePlayAudioStatsEvent_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    std::unique_ptr<DfxReportResult> result = nullptr;

    dfxMsgManager.WritePlayAudioStatsEvent(result);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);

    result = std::make_unique<DfxReportResult>();
    dfxMsgManager.WritePlayAudioStatsEvent(result);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: OnHandle_001
* @tc.desc  : Test DfxMsgManager::OnHandle
*/
HWTEST(DfxMsgManagerUnitTest, OnHandle_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t code = 0;
    int64_t data = 0;

    dfxMsgManager.OnHandle(code, data);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);

    code = 100;
    dfxMsgManager.OnHandle(code, data);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: SafeSendCallBackEvent_001
* @tc.desc  : Test DfxMsgManager::SafeSendCallBackEvent
*/
HWTEST(DfxMsgManagerUnitTest, SafeSendCallBackEvent_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t eventCode = 0;
    int64_t data = 0;
    int64_t delayTime = 0;

    dfxMsgManager.SafeSendCallBackEvent(eventCode, data, delayTime);
    EXPECT_EQ(dfxMsgManager.callbackHandler_, nullptr);

    dfxMsgManager.Init();
    dfxMsgManager.SafeSendCallBackEvent(eventCode, data, delayTime);
    EXPECT_NE(dfxMsgManager.callbackHandler_, nullptr);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: CheckReportDfxMsg_001
* @tc.desc  : Test DfxMsgManager::CheckReportDfxMsg
*/
HWTEST(DfxMsgManagerUnitTest, CheckReportDfxMsg_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;

    msg.appUid = 1;
    dfxMsgManager.InsertReportQueue(msg);
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    dfxMsgManager.lastReportTime_ = now - DEFAULT_DFX_REPORT_INTERVAL_MIN - 1;
    dfxMsgManager.CheckReportDfxMsg();
    EXPECT_EQ(dfxMsgManager.isFull_, false);

    dfxMsgManager.lastReportTime_ = now;
    dfxMsgManager.InsertReportQueue(msg);
    dfxMsgManager.reportedCnt_ = 30;
    dfxMsgManager.CheckReportDfxMsg();
    EXPECT_EQ(dfxMsgManager.isFull_, true);

    dfxMsgManager.InsertReportQueue(msg);
    dfxMsgManager.reportedCnt_ = 2;
    dfxMsgManager.CheckReportDfxMsg();
    EXPECT_EQ(dfxMsgManager.isFull_, true);

    dfxMsgManager.reportQueue_.clear();
    dfxMsgManager.reportedCnt_ = 30;
    dfxMsgManager.CheckReportDfxMsg();
    EXPECT_EQ(dfxMsgManager.isFull_, true);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: IsMsgReady_001
* @tc.desc  : Test DfxMsgManager::IsMsgReady
*/
HWTEST(DfxMsgManagerUnitTest, IsMsgReady_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;
    InterruptDfxInfo interruptInfo;
    CapturerDfxInfo captureInfo;

    msg.appUid = 1;
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }

    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        msg.renderInfo.push_back(renderInfo);
    }

    bool ret = dfxMsgManager.IsMsgReady(msg);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: HandleThreadExit_001
* @tc.desc  : Test DfxMsgManager::HandleThreadExit
*/
HWTEST(DfxMsgManagerUnitTest, HandleThreadExit_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();

    std::unique_ptr<DfxReportResult> result = std::make_unique<DfxReportResult>();
    result->appName = "appName";
    result->appVersion = "1.0";
    result->summary = 2;
    dfxMsgManager.LogDfxResult(result);
    dfxMsgManager.HandleThreadExit();
    EXPECT_EQ(dfxMsgManager.isFull_, false);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: WriteRunningAppMsg_001
* @tc.desc  : Test DfxMsgManager::WriteRunningAppMsg
*/
HWTEST(DfxMsgManagerUnitTest, WriteRunningAppMsg_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    std::unique_ptr<DfxReportResult> result = std::make_unique<DfxReportResult>();
    DfxRunningAppInfo appinfo;

    msg.appUid = 1;
    appinfo.appUid = 1;
    appinfo.appName = "appName";
    appinfo.versionName = "1.0";
    appinfo.appStateVec.push_back(1);
    appinfo.appStateTimeStampVec.push_back(1);
    dfxMsgManager.WriteRunningAppMsg(msg, result);
    EXPECT_EQ(dfxMsgManager.appInfo_.count(msg.appUid), 0);

    dfxMsgManager.appInfo_[msg.appUid] = appinfo;
    dfxMsgManager.WriteRunningAppMsg(msg, result);
    EXPECT_NE(dfxMsgManager.appInfo_.count(msg.appUid), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: CheckCanAddAppInfo_001
* @tc.desc  : Test DfxMsgManager::CheckCanAddAppInfo
*/
HWTEST(DfxMsgManagerUnitTest, CheckCanAddAppInfo_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1003;

    EXPECT_EQ(dfxMsgManager.CheckCanAddAppInfo(appUid), false);

    appUid = static_cast<int32_t>(getuid());
    EXPECT_EQ(dfxMsgManager.CheckCanAddAppInfo(appUid), false);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: UpdateAppState_001
* @tc.desc  : Test DfxMsgManager::UpdateAppState
*/
HWTEST(DfxMsgManagerUnitTest, UpdateAppState_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    DfxAppState appState = DFX_APP_STATE_START;
    bool forceUpdate = false;
    DfxRunningAppInfo appinfo;

    appinfo.appUid = 1;
    appinfo.appName = "appName";
    appinfo.versionName = "1.0";
    appinfo.appStateVec.push_back(0);
    appinfo.appStateTimeStampVec.push_back(1);
    dfxMsgManager.appInfo_[appUid] = appinfo;

    dfxMsgManager.UpdateAppState(appUid, appState, forceUpdate);
    EXPECT_NE(dfxMsgManager.appInfo_.count(appUid), 0);

    appState = DFX_APP_STATE_FOREGROUND;
    appinfo.appStateVec.clear();
    appinfo.appStateVec.push_back(2);
    dfxMsgManager.appInfo_.clear();
    dfxMsgManager.appInfo_[appUid] = appinfo;
    dfxMsgManager.UpdateAppState(appUid, appState, forceUpdate);
    EXPECT_NE(dfxMsgManager.appInfo_.count(appUid), 0);

    appState = DFX_APP_STATE_FOREGROUND;
    appinfo.appStateVec.clear();
    appinfo.appStateVec.push_back(4);
    dfxMsgManager.appInfo_.clear();
    dfxMsgManager.appInfo_[appUid] = appinfo;
    dfxMsgManager.UpdateAppState(appUid, appState, forceUpdate);
    EXPECT_NE(dfxMsgManager.appInfo_.count(appUid), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: UpdateAction_001
* @tc.desc  : Test DfxMsgManager::UpdateAction
*/
HWTEST(DfxMsgManagerUnitTest, UpdateAction_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<RenderDfxInfo> renderInfo;
    RenderDfxInfo renderdfxInfo;

    renderdfxInfo.rendererAction.fourthByte = RendererStage::RENDERER_STAGE_START_FAIL;
    renderInfo.push_back(renderdfxInfo);
    dfxMsgManager.UpdateAction(appUid, renderInfo);
    EXPECT_NE(dfxMsgManager.appInfo_.count(appUid), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: UpdateAction_002
* @tc.desc  : Test DfxMsgManager::UpdateAction
*/
HWTEST(DfxMsgManagerUnitTest, UpdateAction_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<CapturerDfxInfo> capturerInfo;
    CapturerDfxInfo capturerdfxInfo;

    capturerdfxInfo.capturerAction.fourthByte = CapturerStage::CAPTURER_STAGE_START_FAIL;
    capturerInfo.push_back(capturerdfxInfo);
    dfxMsgManager.UpdateAction(appUid, capturerInfo);
    EXPECT_NE(dfxMsgManager.appInfo_.count(appUid), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: UpdateAction_003
* @tc.desc  : Test DfxMsgManager::UpdateAction
*/
HWTEST(DfxMsgManagerUnitTest, UpdateAction_003, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<InterruptDfxInfo> interruptInfo;
    InterruptDfxInfo interruptdfxInfo;

    interruptdfxInfo.interruptAction.fourthByte = InterruptStage::INTERRUPT_STAGE_RESTART;
    interruptInfo.push_back(interruptdfxInfo);
    dfxMsgManager.UpdateAction(appUid, interruptInfo);
    EXPECT_NE(dfxMsgManager.appInfo_.count(appUid), 0);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: GetDfxIndexByType_001
* @tc.desc  : Test DfxMsgManager::GetDfxIndexByType
*/
HWTEST(DfxMsgManagerUnitTest, GetDfxIndexByType_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    DfxMsgIndexType type = DfxMsgIndexType::DFX_MSG_IDX_TYPE_RENDER_INFO;

    uint8_t ret = dfxMsgManager.GetDfxIndexByType(appUid, type);
    EXPECT_NE(ret, 0);

    type = DfxMsgIndexType::DFX_MSG_IDX_TYPE_CAPTURE_INFO;
    ret = dfxMsgManager.GetDfxIndexByType(appUid, type);
    EXPECT_NE(ret, 0);

    type = DfxMsgIndexType::DFX_MSG_IDX_TYPE_INTERRUPT_INFO;
    ret = dfxMsgManager.GetDfxIndexByType(appUid, type);
    EXPECT_NE(ret, 0);

    type = DfxMsgIndexType::DFX_MSG_IDX_TYPE_INTERRUPT_EFFECT;
    ret = dfxMsgManager.GetDfxIndexByType(appUid, type);
    EXPECT_EQ(ret, 0);

    type = static_cast<DfxMsgIndexType>(-1);
    ret = dfxMsgManager.GetDfxIndexByType(appUid, type);
    EXPECT_EQ(ret, 1);
}

/**
* @tc.name  : Test DfxMsgManager.
* @tc.number: CheckIsInterrupted_001
* @tc.desc  : Test DfxMsgManager::CheckIsInterrupted
*/
HWTEST(DfxMsgManagerUnitTest, CheckIsInterrupted_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    InterruptStage stage = INTERRUPT_STAGE_STOPPED;

    bool ret = dfxMsgManager.CheckIsInterrupted(stage);
    EXPECT_EQ(ret, true);

    stage = INTERRUPT_STAGE_DUCK_BEGIN;
    ret = dfxMsgManager.CheckIsInterrupted(stage);
    EXPECT_EQ(ret, true);

    stage = INTERRUPT_STAGE_PAUSED;
    ret = dfxMsgManager.CheckIsInterrupted(stage);
    EXPECT_EQ(ret, true);

    stage = INTERRUPT_STAGE_RESUMED;
    ret = dfxMsgManager.CheckIsInterrupted(stage);
    EXPECT_EQ(ret, true);

    stage = INTERRUPT_STAGE_DUCK_END;
    ret = dfxMsgManager.CheckIsInterrupted(stage);
    EXPECT_EQ(ret, true);

    stage = INTERRUPT_STAGE_TIMEOUT;
    ret = dfxMsgManager.CheckIsInterrupted(stage);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : ProcessInner.
 * @tc.number: CapturerDfxInfo_ProcessInner_001
 * @tc.desc  : Test DfxMsgManager::ProcessInner of parameter is CapturerDfxInfo
 */
HWTEST(DfxMsgManagerUnitTest, CapturerDfxInfo_ProcessInner_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<CapturerDfxInfo> dfxInfo;
    std::list<CapturerDfxInfo> curDfxInfo;
    CapturerDfxInfo renderInfo;

    dfxInfo.push_back(renderInfo);
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        curDfxInfo.push_back(renderInfo);
    }
    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(curDfxInfo.size(), 100);
    EXPECT_EQ(dfxInfo.size(), 1);
}

/**
 * @tc.name  : ProcessInner.
 * @tc.number: CapturerDfxInfo_ProcessInner_002
 * @tc.desc  : Test DfxMsgManager::ProcessInner of parameter is CapturerDfxInfo
 */
HWTEST(DfxMsgManagerUnitTest, CapturerDfxInfo_ProcessInner_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<CapturerDfxInfo> dfxInfo;
    std::list<CapturerDfxInfo> curDfxInfo;
    CapturerDfxInfo renderInfo;

    curDfxInfo.push_back(renderInfo);
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        dfxInfo.push_back(renderInfo);
    }
    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(curDfxInfo.size(), 100);
    EXPECT_EQ(dfxInfo.size(), 100);
}

/**
 * @tc.name  : ProcessInner.
 * @tc.number: RenderDfxInfo_ProcessInner_001
 * @tc.desc  : Test DfxMsgManager::ProcessInner of parameter is RenderDfxInfo
 */
HWTEST(DfxMsgManagerUnitTest, RenderDfxInfo_ProcessInner_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<RenderDfxInfo> dfxInfo;
    std::list<RenderDfxInfo> curDfxInfo;
    RenderDfxInfo renderInfo;

    dfxInfo.push_back(renderInfo);
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        curDfxInfo.push_back(renderInfo);
    }
    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(curDfxInfo.size(), 100);
    EXPECT_EQ(dfxInfo.size(), 1);
}

/**
 * @tc.name  : ProcessInner.
 * @tc.number: RenderDfxInfo_ProcessInner_002
 * @tc.desc  : Test DfxMsgManager::ProcessInner of parameter is RenderDfxInfo
 */
HWTEST(DfxMsgManagerUnitTest, RenderDfxInfo_ProcessInner_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<RenderDfxInfo> dfxInfo;
    std::list<RenderDfxInfo> curDfxInfo;
    RenderDfxInfo renderInfo;

    curDfxInfo.push_back(renderInfo);
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        dfxInfo.push_back(renderInfo);
    }
    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(curDfxInfo.size(), 100);
    EXPECT_EQ(dfxInfo.size(), 100);
}

/**
 * @tc.name  : ProcessInner.
 * @tc.number: InterruptDfxInfo_ProcessInner_001
 * @tc.desc  : Test DfxMsgManager::ProcessInner of parameter is InterruptDfxInfo
 */
HWTEST(DfxMsgManagerUnitTest, InterruptDfxInfo_ProcessInner_001, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<InterruptDfxInfo> dfxInfo;
    std::list<InterruptDfxInfo> curDfxInfo;
    InterruptDfxInfo renderInfo;

    dfxInfo.push_back(renderInfo);
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        curDfxInfo.push_back(renderInfo);
    }
    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(curDfxInfo.size(), 100);
    EXPECT_EQ(dfxInfo.size(), 1);
}

/**
 * @tc.name  : ProcessInner.
 * @tc.number: InterruptDfxInfo_ProcessInner_002
 * @tc.desc  : Test DfxMsgManager::ProcessInner of parameter is InterruptDfxInfo
 */
HWTEST(DfxMsgManagerUnitTest, InterruptDfxInfo_ProcessInner_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<InterruptDfxInfo> dfxInfo;
    std::list<InterruptDfxInfo> curDfxInfo;
    InterruptDfxInfo renderInfo;

    curDfxInfo.push_back(renderInfo);
    for (int i = 0; i < MAX_DFX_MSG_MEMBER_SIZE; i++) {
        dfxInfo.push_back(renderInfo);
    }
    bool ret = dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    EXPECT_EQ(curDfxInfo.size(), 100);
    EXPECT_EQ(dfxInfo.size(), 100);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: WriteRenderMsg_002
 * @tc.desc  : Test DfxMsgManager::WriteRenderMsg
 */
HWTEST(DfxMsgManagerUnitTest, WriteRenderMsg_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    RenderDfxInfo renderInfo;
    renderInfo.rendererAction.fourthByte = static_cast<uint8_t>(RendererStage::RENDERER_STAGE_START_OK);
    msg.appUid = 1;
    for (int i = 0; i < 10; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.WriteRenderMsg(msg, bean);
    EXPECT_EQ(bean->renderInfo.size(), 10);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: WriteRenderMsg_003
 * @tc.desc  : Test DfxMsgManager::WriteRenderMsg
 */
HWTEST(DfxMsgManagerUnitTest, WriteRenderMsg_003, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    RenderDfxInfo renderInfo;
    renderInfo.rendererAction.fourthByte = static_cast<uint8_t>(RendererStage::RENDERER_STAGE_START_FAIL);
    msg.appUid = 1;
    for (int i = 0; i < 10; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.WriteRenderMsg(msg, bean);
    EXPECT_EQ(bean->renderInfo.size(), 10);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: WriteRenderMsg_004
 * @tc.desc  : Test DfxMsgManager::WriteRenderMsg
 */
HWTEST(DfxMsgManagerUnitTest, WriteRenderMsg_004, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    RenderDfxInfo renderInfo;
    renderInfo.rendererAction.fourthByte = static_cast<uint8_t>(RendererStage::RENDERER_STAGE_STOP_OK);
    msg.appUid = 1;
    for (int i = 0; i < 10; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.WriteRenderMsg(msg, bean);
    EXPECT_EQ(bean->renderInfo.size(), 0);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: UpdateAction_004
 * @tc.desc  : Test DfxMsgManager::UpdateAction
 */
HWTEST(DfxMsgManagerUnitTest, UpdateAction_004, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<RenderDfxInfo> renderInfo;
    RenderDfxInfo renderdfxInfo;

    renderdfxInfo.rendererAction.fourthByte = static_cast<uint8_t>(RendererStage::RENDERER_STAGE_START_OK);
    renderdfxInfo.rendererAction.firstByte = 1;
    renderInfo.push_back(renderdfxInfo);
    dfxMsgManager.UpdateAction(appUid, renderInfo);
    EXPECT_EQ(renderInfo.front().rendererAction.firstByte, 22);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: UpdateAction_005
 * @tc.desc  : Test DfxMsgManager::UpdateAction
 */
HWTEST(DfxMsgManagerUnitTest, UpdateAction_005, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<CapturerDfxInfo> capturerInfo;
    CapturerDfxInfo capturerdfxInfo;

    capturerdfxInfo.capturerAction.fourthByte = static_cast<uint8_t>(CapturerStage::CAPTURER_STAGE_START_OK);
    capturerdfxInfo.capturerAction.firstByte = 2;
    capturerInfo.push_back(capturerdfxInfo);
    dfxMsgManager.UpdateAction(appUid, capturerInfo);
    EXPECT_EQ(capturerInfo.front().capturerAction.firstByte, 2);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: UpdateAction_006
 * @tc.desc  : Test DfxMsgManager::UpdateAction
 */
HWTEST(DfxMsgManagerUnitTest, UpdateAction_006, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<InterruptDfxInfo> interruptInfo;
    InterruptDfxInfo interruptdfxInfo;

    interruptdfxInfo.interruptAction.fourthByte = static_cast<uint8_t>(InterruptStage::INTERRUPT_STAGE_START);
    interruptdfxInfo.interruptAction.firstByte = 3;
    interruptInfo.push_back(interruptdfxInfo);
    dfxMsgManager.UpdateAction(appUid, interruptInfo);
    EXPECT_EQ(interruptInfo.front().interruptAction.firstByte, 3);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: UpdateAction_007
 * @tc.desc  : Test DfxMsgManager::UpdateAction
 */
HWTEST(DfxMsgManagerUnitTest, UpdateAction_007, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<RenderDfxInfo> renderInfo;
    RenderDfxInfo renderdfxInfo;
    renderdfxInfo.rendererAction.firstByte = 2;
    renderdfxInfo.rendererAction.fourthByte = 1;
    renderInfo.push_back(renderdfxInfo);
    dfxMsgManager.UpdateAction(appUid, renderInfo);
    EXPECT_EQ(renderInfo.front().rendererAction.firstByte, 22);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: UpdateAction_008
 * @tc.desc  : Test DfxMsgManager::UpdateAction
 */
HWTEST(DfxMsgManagerUnitTest, UpdateAction_008, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<CapturerDfxInfo> capturerInfo;
    CapturerDfxInfo capturerdfxInfo;
    capturerdfxInfo.capturerAction.firstByte = 2;
    capturerdfxInfo.capturerAction.fourthByte = 1;
    capturerInfo.push_back(capturerdfxInfo);
    dfxMsgManager.UpdateAction(appUid, capturerInfo);
    EXPECT_EQ(capturerInfo.front().capturerAction.firstByte, 2);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: UpdateAction_009
 * @tc.desc  : Test DfxMsgManager::UpdateAction
 */
HWTEST(DfxMsgManagerUnitTest, UpdateAction_009, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = 1;
    std::list<InterruptDfxInfo> interruptInfo;
    InterruptDfxInfo interruptdfxInfo;
    interruptdfxInfo.interruptAction.firstByte = 2;
    interruptdfxInfo.interruptAction.secondByte = 1;
    interruptdfxInfo.interruptAction.fourthByte = 1;
    interruptInfo.push_back(interruptdfxInfo);
    dfxMsgManager.UpdateAction(appUid, interruptInfo);
    EXPECT_EQ(interruptInfo.front().interruptAction.firstByte, 2);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: WriteInterruptMsg_002
 * @tc.desc  : Test DfxMsgManager::WriteInterruptMsg
 */
HWTEST(DfxMsgManagerUnitTest, WriteInterruptMsg_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    InterruptDfxInfo interruptInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    interruptInfo.interruptAction.fourthByte = InterruptStage::INTERRUPT_STAGE_START;

    msg.appUid = 1;
    for (int i = 0; i < 10; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }
    dfxMsgManager.WriteInterruptMsg(msg, bean);
    EXPECT_EQ(bean->interruptInfo.size(), 10);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: WriteInterruptMsg_003
 * @tc.desc  : Test DfxMsgManager::WriteInterruptMsg
 */
HWTEST(DfxMsgManagerUnitTest, WriteInterruptMsg_003, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    InterruptDfxInfo interruptInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    interruptInfo.interruptAction.fourthByte = InterruptStage::INTERRUPT_STAGE_RESTART;

    msg.appUid = 1;
    for (int i = 0; i < 10; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }
    dfxMsgManager.WriteInterruptMsg(msg, bean);
    EXPECT_EQ(bean->interruptInfo.size(), 10);
}

/**
 * @tc.name  : Test DfxMsgManager.
 * @tc.number: WriteCapturerMsg_002
 * @tc.desc  : Test DfxMsgManager::WriteCapturerMsg
 */
HWTEST(DfxMsgManagerUnitTest, WriteCapturerMsg_002, TestSize.Level1)
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    CapturerDfxInfo captureInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();

    msg.appUid = 1;
    captureInfo.capturerAction.fourthByte = CapturerStage::CAPTURER_STAGE_STOP_OK;
    for (int i = 0; i < 10; i++) {
        msg.captureInfo.push_back(captureInfo);
    }
    dfxMsgManager.WriteCapturerMsg(msg, bean);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 1);

    captureInfo.capturerAction.fourthByte = CapturerStage::CAPTURER_STAGE_START_OK;
    msg.captureInfo.push_back(captureInfo);
    dfxMsgManager.WriteCapturerMsg(msg, bean);
    EXPECT_EQ(dfxMsgManager.reportQueue_.size(), 1);
}
} // AudioStandard
} // OHOS
