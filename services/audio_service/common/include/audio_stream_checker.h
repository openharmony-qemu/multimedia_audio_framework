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
    void MonitorOnAllCallback(DataTransferStateChangeType type);
    int32_t GetAppUid();
    void InitChecker(DataTransferMonitorParam para, const int32_t pid, const int32_t callbackId);
    void RecordMuteFrame();
    void RecordNodataFrame();
    void RecordNormalFrame();
    void DeleteCheckerPara(const int32_t pid, const int32_t callbackId);
    void StopCheckStreamThread();
private:
    bool IsMonitorMuteFrame(const CheckerParam &para);
    bool IsMonitorNoDataFrame(const CheckerParam &para);
    void InitCallbackInfo(DataTransferStateChangeType type, AudioRendererDataTransferStateChangeInfo &callbackInfo);
    void CheckStreamThread();
    std::vector<CheckerParam> checkParaVector_;
    bool monitorSwitch_ = false;
    std::recursive_mutex checkLock_;
    AudioProcessConfig streamConfig_;
    std::thread checkThread_;
    std::atomic<bool> isKeepCheck_ = false;
    std::atomic<bool> isNeedCreateThread_ = true;
};

}
}
#endif
