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
#include <inttypes.h>
#include "audio_stream_checker.h"
#include "audio_renderer_log.h"
#include "audio_utils.h"
#include "audio_stream_monitor.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const float TRANS_PERCENTAGE = 100.0;
const int32_t TRANS_INTEGER = 100;
const int64_t STREAM_CHECK_INTERVAL_TIME = 500000000;
}

AudioStreamChecker::AudioStreamChecker(AudioProcessConfig cfg) : streamConfig_(cfg)
{
    monitorSwitch_ = true;
}

AudioStreamChecker::~AudioStreamChecker()
{
    AUDIO_INFO_LOG("~AudioStreamChecker(), sessionId = %{public}u, uid = %{public}d",
        streamConfig_.originalSessionId, streamConfig_.appInfo.appUid);
    monitorSwitch_ = false;
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(streamConfig_.originalSessionId);
    isKeepCheck_.store(false);
}

void AudioStreamChecker::InitChecker(DataTransferMonitorParam para, const int32_t pid, const int32_t callbackId)
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (auto item : checkParaVector_) {
        if (item.callbackId == callbackId && item.pid == pid) {
            AUDIO_INFO_LOG("No need init check, callbackid = %{public}d, pid = %{public}d", callbackId, pid);
            return;
        }
    }
    CheckerParam checkPara;
    checkPara.pid = pid;
    checkPara.callbackId = callbackId;
    checkPara.para = para;
    checkPara.hasInitCheck = true;
    checkPara.isMonitorMuteFrame = IsMonitorMuteFrame(checkPara);
    checkPara.isMonitorNoDataFrame = IsMonitorNoDataFrame(checkPara);
    checkPara.lastUpdateTime = ClockTime::GetCurNano();
    checkParaVector_.push_back(checkPara);
    if (isNeedCreateThread_.load()) {
        isKeepCheck_.store(true);
        std::weak_ptr<AudioStreamChecker> self = shared_from_this();
        checkThread_ = std::thread([self] {
            if (auto thisPtr = self.lock()) {
                thisPtr->CheckStreamThread();
            }
        });
        pthread_setname_np(checkThread_.native_handle(), "OS_CheckStreamLoop");
        checkThread_.detach();
        isNeedCreateThread_.store(false);
        AUDIO_INFO_LOG("Start check thread success");
    }
    AUDIO_INFO_LOG("Init checker end, pid = %{public}d, callbackId = %{public}d, uid = %{public}d",
        pid, callbackId, para.clientUID);
}

void AudioStreamChecker::DeleteCheckerPara(const int32_t pid, const int32_t callbackId)
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (auto iter = checkParaVector_.begin(); iter != checkParaVector_.end();) {
        if (iter->pid == pid && iter->callbackId == callbackId) {
            iter = checkParaVector_.erase(iter);
            AUDIO_INFO_LOG("Delete check para success, pid = %{public}d, callbackId = %{public}d,",
                pid, callbackId);
        } else {
            iter++;
        }
    }
    if (checkParaVector_.size() == 0) {
        isNeedCreateThread_.store(true);
        isKeepCheck_.store(false);
        AUDIO_INFO_LOG("Stream has no callback, stop check thread");
    }
    AUDIO_INFO_LOG("Delete check para end, pid = %{public}d, callbackId = %{public}d", pid, callbackId);
}

void AudioStreamChecker::StopCheckStreamThread()
{
    AUDIO_INFO_LOG("Stop check stream thread");
    isKeepCheck_.store(false);
}

void AudioStreamChecker::CheckStreamThread()
{
    AUDIO_INFO_LOG("CheckStreamThread start...");
    while (isKeepCheck_.load()) {
        MonitorCheckFrame();
        ClockTime::RelativeSleep(STREAM_CHECK_INTERVAL_TIME);
    }
    AUDIO_INFO_LOG("CheckStreamThread end...");
}

void AudioStreamChecker::MonitorCheckFrame()
{
    if (!monitorSwitch_) {
        AUDIO_INFO_LOG("Not register monitor callback");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (int32_t index = 0; index < checkParaVector_.size(); index++) {
        MonitorCheckFrameSub(checkParaVector_[index]);
    }
}

void AudioStreamChecker::MonitorCheckFrameSub(CheckerParam &para)
{
    if (!para.hasInitCheck) {
        AUDIO_ERR_LOG("Check para not init, appuid = %{public}d", para.para.clientUID);
        return;
    }
    int64_t timeCost = ClockTime::GetCurNano() - para.lastUpdateTime;
    int64_t abnormalFrameNum = 0;
    if (para.isMonitorMuteFrame) {
        abnormalFrameNum += para.muteFrameNum;
        AUDIO_DEBUG_LOG("Check mute frame size = %{public}lld", para.muteFrameNum);
    }
    if (para.isMonitorNoDataFrame) {
        abnormalFrameNum += para.noDataFrameNum;
        AUDIO_DEBUG_LOG("Check no data frame size = %{public}lld", para.noDataFrameNum);
    }
    if (timeCost < para.para.timeInterval) {
        AUDIO_DEBUG_LOG("Check time is not enough");
        return;
    }
    para.sumFrameCount = para.normalFrameCount + para.noDataFrameNum;
    float badFrameRatio = para.para.badFramesRatio / TRANS_PERCENTAGE;
    AUDIO_DEBUG_LOG("Check frame sum = %{public}lld, abnormal = %{public}lld, badRatio = %{public}f",
        para.sumFrameCount, abnormalFrameNum, badFrameRatio);
    AUDIO_DEBUG_LOG("Last check status = %{public}d", para.lastStatus);
    if (abnormalFrameNum >= static_cast<int64_t>(para.sumFrameCount * badFrameRatio)) {
        if (para.lastStatus == DATA_TRANS_STOP) {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status still in DATA_TRANS_STOP", streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_STOP, false, para);
        } else {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status change in DATA_TRANS_STOP", streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_STOP, true, para);
        }
    } else {
        if (para.lastStatus == DATA_TRANS_RESUME) {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status still in DATA_TRANS_RESUME", streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_RESUME, false, para);
        } else {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status change in DATA_TRANS_RESUME", streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_RESUME, true, para);
        }
    }
}

void AudioStreamChecker::CleanRecordData(CheckerParam &para)
{
    para.muteFrameNum = 0;
    para.noDataFrameNum = 0;
    para.normalFrameCount = 0;
    para.sumFrameCount = 0;
    AUDIO_DEBUG_LOG("Clean check para end...");
}

void AudioStreamChecker::MonitorOnAllCallback(DataTransferStateChangeType type)
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    if (!monitorSwitch_) {
        AUDIO_ERR_LOG("Not register monitor callback");
        return;
    }
    AudioRendererDataTransferStateChangeInfo callbackInfo;
    InitCallbackInfo(type, callbackInfo);
    for (int32_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].lastStatus = type;
        AUDIO_INFO_LOG("MonitorOnAllCallback type = %{public}d", type);
        checkParaVector_[index].lastUpdateTime = ClockTime::GetCurNano();
        CleanRecordData(checkParaVector_[index]);
        AudioRendererDataTransferStateChangeInfo callbackInfo;
        InitCallbackInfo(type, callbackInfo);
        AudioStreamMonitor::GetInstance().OnCallback(checkParaVector_[index].pid,
            checkParaVector_[index].callbackId, callbackInfo);
    }
}

void AudioStreamChecker::InitCallbackInfo(DataTransferStateChangeType type,
    AudioRendererDataTransferStateChangeInfo &callbackInfo)
{
    callbackInfo.stateChangeType = type;
    callbackInfo.clientPid = streamConfig_.appInfo.appPid;
    callbackInfo.clientUID = streamConfig_.appInfo.appUid;
    callbackInfo.streamUsage = streamConfig_.rendererInfo.streamUsage;
    callbackInfo.sessionId = streamConfig_.originalSessionId;
}

void AudioStreamChecker::MonitorOnCallback(DataTransferStateChangeType type, bool isNeedCallback, CheckerParam &para)
{
    para.lastUpdateTime = ClockTime::GetCurNano();
    if (!monitorSwitch_ || !para.hasInitCheck) {
        return;
    }
    if (para.sumFrameCount == 0) {
        AUDIO_DEBUG_LOG("Audio stream not start, callbackId = %{public}d", para.callbackId);
        return;
    }
    para.lastStatus = type;
    AudioRendererDataTransferStateChangeInfo callbackInfo;
    InitCallbackInfo(type, callbackInfo);
    callbackInfo.badDataRatio[NO_DATA_TRANS] = (para.noDataFrameNum * TRANS_INTEGER) / para.sumFrameCount;
    callbackInfo.badDataRatio[SLIENCE_DATA_TRANS] = (para.muteFrameNum * TRANS_INTEGER) / para.sumFrameCount;
    AUDIO_DEBUG_LOG("NO_DATA_TRANS ration = %{public}d, SLIENCE_DATA_TRANS ratio = %{public}d",
        callbackInfo.badDataRatio[NO_DATA_TRANS], callbackInfo.badDataRatio[SLIENCE_DATA_TRANS]);
    if (isNeedCallback) {
        AUDIO_DEBUG_LOG("Callback stream status, pid = %{public}d, callbackId = %{public}d",
            para.pid, para.callbackId);
        AudioStreamMonitor::GetInstance().OnCallback(para.pid, para.callbackId, callbackInfo);
    }
    CleanRecordData(para);
}

bool AudioStreamChecker::IsMonitorMuteFrame(const CheckerParam &para)
{
    AUDIO_INFO_LOG("badDataTransferTypeBitMap = %{public}d", para.para.badDataTransferTypeBitMap);
    if (para.hasInitCheck) {
        return para.para.badDataTransferTypeBitMap & (1 << SLIENCE_DATA_TRANS);
    }
    return false;
}

bool AudioStreamChecker::IsMonitorNoDataFrame(const CheckerParam &para)
{
    AUDIO_INFO_LOG("badDataTransferTypeBitMap = %{public}d", para.para.badDataTransferTypeBitMap);
    if (para.hasInitCheck) {
        return para.para.badDataTransferTypeBitMap & (1 << NO_DATA_TRANS);
    }
    return false;
}

int32_t AudioStreamChecker::GetAppUid()
{
    return streamConfig_.appInfo.appUid;
}

void AudioStreamChecker::RecordMuteFrame()
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (int32_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].muteFrameNum++;
        AUDIO_DEBUG_LOG("Mute frame num = %{public}lld, callbackId = %{public}d",
            checkParaVector_[index].muteFrameNum, checkParaVector_[index].callbackId);
    }
}

void AudioStreamChecker::RecordNodataFrame()
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (int32_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].noDataFrameNum++;
        AUDIO_DEBUG_LOG("No data frame num = %{public}lld, callbackId = %{public}d",
            checkParaVector_[index].noDataFrameNum, checkParaVector_[index].callbackId);
    }
}

void AudioStreamChecker::RecordNormalFrame()
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (int32_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].normalFrameCount++;
        AUDIO_DEBUG_LOG("Normal frame num = %{public}lld, callbackId = %{public}d",
            checkParaVector_[index].normalFrameCount, checkParaVector_[index].callbackId);
    }
}

}
}
