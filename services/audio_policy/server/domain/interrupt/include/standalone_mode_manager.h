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

#ifndef ST_STANDALONE_MODE_MANAGER_H
#define ST_STANDALONE_MODE_MANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t INVALID_ID = INT_MIN;

class AudioInterruptService;
class StandaloneModeManager {
private:
    static std::mutex instanceMutex;
    static StandaloneModeManager* instance;
public:
    static StandaloneModeManager &GetInstance();
    void Init(std::shared_ptr<AudioInterruptService> interruptService);
    bool CheckAndRecordStandaloneApp(const int32_t appUid, const bool isOnlyRecordUid = true,
        const int32_t sessionId = -1);
    int32_t SetAppSilentOnDisplay(const int32_t ownerPid, const int32_t displayId);
    int32_t SetAppConcurrencyMode(const int32_t ownerPid,
        const int32_t appUid, const int32_t mode);
    void EraseDeactivateAudioStream(const int32_t appUid,
        const int32_t sessionId);
    void ResumeAllStandaloneApp(const int32_t appPid);

private:
    StandaloneModeManager() = default;
    ~StandaloneModeManager();
    StandaloneModeManager(const StandaloneModeManager&) = delete;
    StandaloneModeManager &operator = (const StandaloneModeManager&) = delete;

    void CleanAllStandaloneInfo();
    void RemoveExistingFocus(const int32_t appUid);
    bool CheckOwnerPidPermissions(const int32_t ownerPid);
    void ExitStandaloneAndResumeFocus(const int32_t appUid);
    bool CheckAppOnVirtualScreenByUid(const int32_t appUid);
    void RecordStandaloneAppSessionIdInfo(const int32_t appUid, const bool isOnlyRecordUid = true,
        const int32_t sessionId = -1);

    std::recursive_mutex mutex_;
    std::shared_ptr<AudioInterruptService> interruptService_;
    int32_t ownerPid_ = INVALID_ID;
    int32_t displayId_ = INVALID_ID;
    bool isSetSilentDisplay_ = false;
    std::unordered_map<int32_t,
        std::unordered_set<int32_t>>activeZoneSessionsMap_; //{appUid {sessionId}}
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_STANDALONE_MODE_MANAGER_H