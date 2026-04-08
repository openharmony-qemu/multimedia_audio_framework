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

#ifndef RECORD_PRIVACY_MANAGER_H
#define RECORD_PRIVACY_MANAGER_H

#include <cstdint>
#include <map>
#include <mutex>
#include <set>

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class RecordPrivacyManager {
public:
    static RecordPrivacyManager &GetInstance();

    bool StartUsingMicrophone(uint32_t targetTokenId) const;
    bool StopUsingMicrophone(uint32_t targetTokenId) const;
    bool NotifyPrivacyStart(uint32_t targetTokenId, uint32_t sessionId);
    bool NotifyPrivacyStop(uint32_t targetTokenId, uint32_t sessionId);
    bool NeedTurnOnMicIndicator(int32_t callingUid, bool isLoopback, SourceType sourceType) const;

private:
    RecordPrivacyManager() = default;
    ~RecordPrivacyManager() = default;
    RecordPrivacyManager(const RecordPrivacyManager &) = delete;
    RecordPrivacyManager &operator=(const RecordPrivacyManager &) = delete;

private:
    std::mutex recordMapMutex_;
    std::map<uint32_t, std::set<uint32_t>> tokenIdRecordMap_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // RECORD_PRIVACY_MANAGER_H
