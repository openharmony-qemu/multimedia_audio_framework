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

#ifndef ST_AUDIO_ZONE_INTERRUPT_ADAPTER_H
#define ST_AUDIO_ZONE_INTERRUPT_ADAPTER_H

#include <unordered_map>
#include <memory>
#include <list>
#include <mutex>
#include <vector>
#include "audio_interrupt_service.h"
#include "audio_zone_client_manager.h"
#include "audio_zone_info.h"

namespace OHOS {
namespace AudioStandard {

using AudioZoneFocusList = std::list<std::pair<AudioInterrupt, AudioFocuState>>;

class AudioZoneInterruptReporter {
public:
    using ReportItem = std::pair<int32_t, std::string>;
    using ReportItemList = std::list<ReportItem>;
    using ReportMap = std::unordered_map<int32_t, ReportItemList>;
    using Reporter  =  std::shared_ptr<AudioZoneInterruptReporter>;
    using ReporterVector = std::vector<Reporter>;

    AudioZoneInterruptReporter() = default;
    ~AudioZoneInterruptReporter() = default;

    static int32_t EnableInterruptReport(pid_t clientPid, int32_t zoneId,
        const std::string &deviceTag, bool enable);
    static void DisableInterruptReport(pid_t clientPid);
    static void DisableAllInterruptReport();
    static ReporterVector CreateReporter(std::shared_ptr<AudioInterruptService> interruptService,
        std::shared_ptr<AudioZoneClientManager> zoneClientManager,
        AudioZoneInterruptReason reason);
    static ReporterVector CreateReporter(int32_t zoneId,
        std::shared_ptr<AudioInterruptService> interruptService,
        std::shared_ptr<AudioZoneClientManager> zoneClientManager,
        AudioZoneInterruptReason reason);
    
    void ReportInterrupt();

private:
    std::shared_ptr<AudioInterruptService> interruptService_;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager_;
    pid_t clientPid_ = -1;
    int32_t zoneId_ = 0;
    std::string deviceTag_ = "";
    AudioZoneFocusList oldFocusList_;
    AudioZoneInterruptReason reportReason_ = AudioZoneInterruptReason::UNKNOWN;

    static ReportMap interruptEnableMaps_;
    static std::mutex interruptEnableMutex_;

    static int32_t RegisterInterruptReport(pid_t clientPid, int32_t zoneId,
        const std::string &deviceTag);
    static void UnRegisterInterruptReport(pid_t clientPid, int32_t zoneId,
        const std::string &deviceTag);
    
    AudioZoneFocusList GetFocusList();
    bool IsFocusListEqual(const AudioZoneFocusList &a, const AudioZoneFocusList &b);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_ZONE_INTERRUPT_ADAPTER_H