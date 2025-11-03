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
#include "volume_tools.h"
#include "audio_stream_checker_thread.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const float TRANS_PERCENTAGE = 100.0;
const int32_t TRANS_INTEGER = 100;
const int32_t AUDIOSTREAM_LATENCY_MODE_NORMAL = 0;
const int64_t DEFAULT_TIME = 0;
const int64_t NORMAL_FRAME_PER_TIME = 20000000;  // 20ms
const int64_t FAST_FRAME_PER_TIME = 5000000;  // 5ms
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
    AUDIO_INFO_LOG("Delete check para end, pid = %{public}d, callbackId = %{public}d", pid, callbackId);
}

void AudioStreamChecker::CalculateFrameAfterStandby(CheckerParam &para, int64_t &abnormalFrameNum)
{
    int64_t timePerFrame = (streamConfig_.rendererInfo.rendererFlags == AUDIOSTREAM_LATENCY_MODE_NORMAL) ?
        NORMAL_FRAME_PER_TIME : FAST_FRAME_PER_TIME;
    AUDIO_DEBUG_LOG("Frame per time = %{public}" PRId64"", timePerFrame);
    AUDIO_DEBUG_LOG("StandbyStartTime = %{public}" PRId64", StandbyStopTime = %{public}" PRId64"",
        para.standbyStartTime, para.standbyStopTime);
    AUDIO_DEBUG_LOG("IsInStandby = %{public}d", para.isInStandby);
    int64_t calculateFrameNum = 0;
    if (para.standbyStartTime != DEFAULT_TIME && para.standbyStopTime != DEFAULT_TIME) {
        calculateFrameNum = ((para.standbyStopTime - para.standbyStartTime) / timePerFrame);
    } else if (para.standbyStartTime != DEFAULT_TIME && para.standbyStopTime == DEFAULT_TIME) {
        calculateFrameNum = ((ClockTime::GetCurNano() - para.standbyStartTime) / timePerFrame);
        AUDIO_DEBUG_LOG("Current time = %{public}" PRId64"", ClockTime::GetCurNano());
    } else if (para.standbyStartTime == DEFAULT_TIME && para.standbyStopTime != DEFAULT_TIME) {
        calculateFrameNum = ((para.standbyStopTime - para.lastUpdateTime) / timePerFrame);
        AUDIO_DEBUG_LOG("Last update time = %{public}" PRId64"", para.lastUpdateTime);
    } else {
        calculateFrameNum = (para.isInStandby ? (para.para.timeInterval / timePerFrame) : 0);
    }
    AUDIO_DEBUG_LOG("Calculate no data frame num = %{public}" PRId64"", calculateFrameNum);
    if (para.isMonitorNoDataFrame) {
        abnormalFrameNum += calculateFrameNum;
    }
    para.noDataFrameNum += calculateFrameNum;
    AUDIO_DEBUG_LOG("AbnormalFrameNum = %{public}" PRId64", NoDataFrameNum = %{public}" PRId64"",
        abnormalFrameNum, para.noDataFrameNum);
}

void AudioStreamChecker::OnRemoteAppDied(const int32_t pid)
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (auto iter = checkParaVector_.begin(); iter != checkParaVector_.end();) {
        if (iter->pid == pid) {
            iter = checkParaVector_.erase(iter);
            AUDIO_INFO_LOG("Delete check para success when remote app died, pid = %{public}d", pid);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("Delete check para end when remote app died, pid = %{public}d", pid);
}

void AudioStreamChecker::CheckVolume()
{
    std::lock_guard<std::mutex> lock(volumeLock_);
    if (VolumeTools::IsZeroVolume(curVolume_) && !VolumeTools::IsZeroVolume(preVolume_)) {
        AUDIO_INFO_LOG("sessionId %{public}u change to mute", streamConfig_.originalSessionId);
        std::unique_lock<std::recursive_mutex> lock(checkLock_);
        for (size_t index = 0; index < checkParaVector_.size(); index++) {
            AudioStreamMonitor::GetInstance().OnMuteCallback(checkParaVector_[index].pid,
                checkParaVector_[index].callbackId, streamConfig_.appInfo.appUid,
                streamConfig_.originalSessionId, true);
        }
        lock.unlock();
    }
    if (!VolumeTools::IsZeroVolume(curVolume_) && VolumeTools::IsZeroVolume(preVolume_)) {
        AUDIO_INFO_LOG("sessionId %{public}u change to unmute", streamConfig_.originalSessionId);
        std::unique_lock<std::recursive_mutex> lock(checkLock_);
        for (size_t index = 0; index < checkParaVector_.size(); index++) {
            AudioStreamMonitor::GetInstance().OnMuteCallback(checkParaVector_[index].pid,
                checkParaVector_[index].callbackId, streamConfig_.appInfo.appUid,
                streamConfig_.originalSessionId, false);
        }
        lock.unlock();
    }
    preVolume_ = curVolume_;
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

void AudioStreamChecker::MonitorCheckFrameAction(CheckerParam &para, int64_t abnormalFrameNum,
    float badFrameRatio)
{
    if (abnormalFrameNum >= static_cast<int64_t>(para.sumFrameCount * badFrameRatio)) {
        if (para.lastStatus == DATA_TRANS_STOP) {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status still in DATA_TRANS_STOP",
                streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_STOP, false, para);
        } else if (para.lastStatus == AUDIO_STREAM_PAUSE || para.lastStatus == AUDIO_STREAM_STOP) {
            AUDIO_DEBUG_LOG("Last status is %{public}d, no need callback", para.lastStatus);
            CleanRecordData(para);
        } else {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status change in DATA_TRANS_STOP",
                streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_STOP, true, para);
        }
    } else {
        if (para.lastStatus == DATA_TRANS_RESUME) {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status still in DATA_TRANS_RESUME",
                streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_RESUME, false, para);
        } else if (para.lastStatus == AUDIO_STREAM_START || para.lastStatus == AUDIO_STREAM_PAUSE) {
            AUDIO_DEBUG_LOG("Last status is %{public}d, no need callback", para.lastStatus);
            para.lastStatus = para.lastStatus == AUDIO_STREAM_START ? DATA_TRANS_RESUME : AUDIO_STREAM_PAUSE;
            CleanRecordData(para);
        } else {
            AUDIO_DEBUG_LOG("sessionId = %{public}u, status change in DATA_TRANS_RESUME",
                streamConfig_.originalSessionId);
            MonitorOnCallback(DATA_TRANS_RESUME, true, para);
        }
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
        AUDIO_DEBUG_LOG("Check mute frame size = %{public}" PRId64, para.muteFrameNum);
    }
    if (para.isMonitorNoDataFrame) {
        abnormalFrameNum += para.noDataFrameNum;
        AUDIO_DEBUG_LOG("Check no data frame size = %{public}" PRId64, para.noDataFrameNum);
    }

    if (timeCost < para.para.timeInterval) {
        AUDIO_DEBUG_LOG("Check time is not enough");
        return;
    }
    AUDIO_DEBUG_LOG("Before calculate abnormalFrameNum = %{public}" PRId64"", abnormalFrameNum);
    CalculateFrameAfterStandby(para, abnormalFrameNum);
    para.sumFrameCount = para.normalFrameCount + para.noDataFrameNum;
    float badFrameRatio = para.para.badFramesRatio / TRANS_PERCENTAGE;
    AUDIO_DEBUG_LOG("Check frame sum = %{public}" PRId64", abnormal = %{public}" PRId64", badRatio = %{public}f",
        para.sumFrameCount, abnormalFrameNum, badFrameRatio);
    AUDIO_DEBUG_LOG("Last check status = %{public}d", para.lastStatus);
    MonitorCheckFrameAction(para, abnormalFrameNum, badFrameRatio);
}

void AudioStreamChecker::CleanRecordData(CheckerParam &para)
{
    para.muteFrameNum = 0;
    para.noDataFrameNum = 0;
    para.normalFrameCount = 0;
    para.sumFrameCount = 0;
    para.standbyStartTime = 0;
    para.standbyStopTime = 0;
    para.lastUpdateTime = ClockTime::GetCurNano();
    AUDIO_DEBUG_LOG("Clean check para end...");
}

void AudioStreamChecker::MonitorOnAllCallback(DataTransferStateChangeType type, bool isStandby)
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    if (!monitorSwitch_) {
        AUDIO_ERR_LOG("Not register monitor callback");
        return;
    }
    AudioRendererDataTransferStateChangeInfo callbackInfo;
    InitCallbackInfo(type, callbackInfo);
    for (size_t index = 0; index < checkParaVector_.size(); index++) {
        if (isStandby && type == DATA_TRANS_RESUME && !checkParaVector_[index].isMonitorNoDataFrame) {
            AUDIO_INFO_LOG("Start during standby and no monitor nodata frame, no need callback");
            continue;
        }
        checkParaVector_[index].lastStatus = type;
        AUDIO_INFO_LOG("type = %{public}d", type);
        checkParaVector_[index].lastUpdateTime = ClockTime::GetCurNano();
        CleanRecordData(checkParaVector_[index]);
        AudioRendererDataTransferStateChangeInfo callbackInfo;
        InitCallbackInfo(type, callbackInfo);
        AudioStreamMonitor::GetInstance().OnCallback(checkParaVector_[index].pid,
            checkParaVector_[index].callbackId, callbackInfo);
    }
}

void AudioStreamChecker::RecordStandbyTime(bool isStandbyStart)
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (size_t index = 0; index < checkParaVector_.size(); index++) {
        if (isStandbyStart) {
            checkParaVector_[index].standbyStartTime = ClockTime::GetCurNano();
        } else {
            checkParaVector_[index].standbyStopTime = ClockTime::GetCurNano();
        }
        checkParaVector_[index].isInStandby = isStandbyStart;
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
    {
        std::lock_guard<std::recursive_mutex> lock(backgroundStateLock_);
        callbackInfo.isBackground = isBackground_;
    }
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
    callbackInfo.badDataRatio[SILENCE_DATA_TRANS] = (para.muteFrameNum * TRANS_INTEGER) / para.sumFrameCount;
    AUDIO_DEBUG_LOG("NO_DATA_TRANS ration = %{public}d, SILENCE_DATA_TRANS ratio = %{public}d",
        callbackInfo.badDataRatio[NO_DATA_TRANS], callbackInfo.badDataRatio[SILENCE_DATA_TRANS]);
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
        return para.para.badDataTransferTypeBitMap & (1 << SILENCE_DATA_TRANS);
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
    for (size_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].muteFrameNum++;
        AUDIO_DEBUG_LOG("Mute frame num = %{public}" PRId64", callbackId = %{public}d",
            checkParaVector_[index].muteFrameNum, checkParaVector_[index].callbackId);
    }
}

void AudioStreamChecker::RecordNodataFrame()
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (size_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].noDataFrameNum++;
        AUDIO_DEBUG_LOG("No data frame num = %{public}" PRId64", callbackId = %{public}d",
            checkParaVector_[index].noDataFrameNum, checkParaVector_[index].callbackId);
    }
}

void AudioStreamChecker::RecordNormalFrame()
{
    std::lock_guard<std::recursive_mutex> lock(checkLock_);
    for (size_t index = 0; index < checkParaVector_.size(); index++) {
        checkParaVector_[index].normalFrameCount++;
        AUDIO_DEBUG_LOG("Normal frame num = %{public}" PRId64", callbackId = %{public}d",
            checkParaVector_[index].normalFrameCount, checkParaVector_[index].callbackId);
    }
}

void AudioStreamChecker::UpdateAppState(bool isBackground)
{
    {
        std::lock_guard<std::recursive_mutex> lock(backgroundStateLock_);
        isBackground_ = isBackground;
    }
}

void AudioStreamChecker::SetVolume(float volume)
{
    std::lock_guard<std::mutex> lock(volumeLock_);
    CHECK_AND_RETURN(curVolume_ != volume);
    AUDIO_INFO_LOG("sessionId:%{public}u volume change from %{public}f to %{public}f",
        streamConfig_.originalSessionId, curVolume_, volume);
    curVolume_ = volume;
}

float AudioStreamChecker::GetVolume()
{
    std::lock_guard<std::mutex> lock(volumeLock_);
    return curVolume_;
}
}
}
