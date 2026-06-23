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

#ifndef AUDIO_RECORD_CONTROLLER_H
#define AUDIO_RECORD_CONTROLLER_H

#include <mutex>
#include <vector>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioRecordController {
public:
    static AudioRecordController &GetInstance();

    int32_t EnableSystemRecordController(bool show, const SystemRecordControllerConfig &config,
        int32_t uid, int32_t pid);
    void OnRecordStart(uint32_t sessionId, int32_t uid, int32_t pid, SourceType sourceType);
    void OnRecordStop(uint32_t sessionId, int32_t uid, int32_t pid, SourceType sourceType);

    void OnAppStateChanged(int32_t uid, int32_t pid, bool isForeground);
    void OnAppDied(int32_t uid, int32_t pid);

private:
    AudioRecordController() = default;
    AudioRecordController(const AudioRecordController &oth) = delete;
    ~AudioRecordController() = default;

    struct MonitoredAppInfo {
        int32_t uid = -1;
        int32_t pid = -1;
        uint32_t sessionId = 0;
        bool isForeground = false;
        SourceType sourceType = SOURCE_TYPE_INVALID;
    };

    void NotifyEnabledChange();
    bool IsValidSourceType(SourceType sourceType);
    void StartMonitoringApp(int32_t uid, int32_t pid, uint32_t sessionId, bool isForeground, SourceType sourceType);
    void StopMonitoringApp(int32_t uid, int32_t pid, uint32_t sessionId, SourceType sourceType);

    std::mutex mutex_;
    std::vector<MonitoredAppInfo> monitoredApps_;
    SystemRecordControllerChangeInfo lastChangeInfo_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RECORD_CONTROLLER_H