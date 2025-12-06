/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef OH_AUDIO_WORKGROUP_H
#define OH_AUDIO_WORKGROUP_H

#include <unordered_map>
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioWorkgroup {
public:
    ~OHAudioWorkgroup();
    explicit OHAudioWorkgroup(int id);

    bool AddThread(int32_t tokenId);
    bool RemoveThread(int32_t tokenId);
    bool Start(uint64_t startTime, uint64_t deadlineTime);
    bool Stop();
    int32_t GetWorkgroupId() const;
    bool GetNeedUpdatePrioFlag() const;
    void SetNeedUpdatePrioFlag(bool flag);
private:
    mutable std::mutex mtx_;
    std::unordered_map<int32_t, bool> workgroupThreads_;
    bool isNeedUpdatePrio_ = false;
    const int32_t workgroupId_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif