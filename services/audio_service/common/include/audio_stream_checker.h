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

#ifndef AUDIO_STREAM_CHECKER
#define AUDIO_STREAM_CHECKER
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include "audio_info.h"
#include "audio_stutter.h"

namespace OHOS {
namespace AudioStandard {

struct CheckerParam {
    int32_t pid = 0;
    int32_t callbackId = 0;
    DataTransferMonitorParam para;
    bool isMonitorMuteFrame = false;
    bool isMonitorNoDataFrame = false;
    int64_t normalFrameCount = 0;
    int64_t noDataFrameNum = 0;
    int64_t muteFrameNum = 0;
    int64_t sumFrameCount = 0;
    int64_t lastUpdateTime = 0;
    bool hasInitCheck = false;
    int64_t standbyStartTime = 0;
    int64_t standbyStopTime = 0;
    bool isInStandby = false;
    DataTransferStateChangeType lastStatus = DATA_TRANS_RESUME;
};

class AudioStreamChecker : public std::enable_shared_from_this<AudioStreamChecker> {
public:
    AudioStreamChecker(AudioProcessConfig cfg);
    ~AudioStreamChecker();
    void MonitorCheckFrameSub(CheckerParam &para);
    void MonitorCheckFrame();
    void CleanRecordData(CheckerParam &para);
    void MonitorOnCallback(DataTransferStateChangeType type, bool isNeedCallback, CheckerParam &para);
    void MonitorOnAllCallback(DataTransferStateChangeType type, bool isStandby);
    int32_t GetAppUid();
    void InitChecker(DataTransferMonitorParam para, const int32_t pid, const int32_t callbackId);
    void RecordMuteFrame();
    void RecordNodataFrame();
    void RecordNormalFrame();
    void DeleteCheckerPara(const int32_t pid, const int32_t callbackId);
    void OnRemoteAppDied(const int32_t pid);
    void RecordStandbyTime(bool isStart);
    void UpdateAppState(bool isBackground);
    void SetVolume(float volume);
    float GetVolume();

    std::atomic<bool> isNeedCreateThread_ = true;
private:
    bool IsMonitorMuteFrame(const CheckerParam &para);
    bool IsMonitorNoDataFrame(const CheckerParam &para);
    void InitCallbackInfo(DataTransferStateChangeType type, AudioRendererDataTransferStateChangeInfo &callbackInfo);
    void MonitorCheckFrameAction(CheckerParam &para, int64_t abnormalFrameNum, float badFrameRatio);
    void CalculateFrameAfterStandby(CheckerParam &para, int64_t &abnormalFrameNum);
    void CheckVolume();
    std::vector<CheckerParam> checkParaVector_;
    bool monitorSwitch_ = false;
    bool isBackground_ = false;
    std::recursive_mutex checkLock_;
    std::recursive_mutex backgroundStateLock_;
    AudioProcessConfig streamConfig_;
    std::mutex volumeLock_;
    float curVolume_ = 1.0f;
    float preVolume_ = 1.0f;
};
}
}
#endif
