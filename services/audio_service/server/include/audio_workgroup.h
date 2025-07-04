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

#ifndef AUDIO_WORKGROUP_H
#define AUDIO_WORKGROUP_H

#include <vector>
#include <unordered_map>
#include "audio_manager_base.h"

namespace OHOS {
namespace AudioStandard {

class AudioWorkgroupCallbackForMonitor {
public:
    virtual ~AudioWorkgroupCallbackForMonitor() = default;
    virtual void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info) = 0;
};

class AudioWorkgroup {
public:
    explicit AudioWorkgroup(int32_t id);
    ~AudioWorkgroup() {};

    int32_t GetWorkgroupId();
    uint32_t GetThreadsNums();
    int32_t AddThread(int32_t tid);
    int32_t RemoveThread(int32_t tid);
    int32_t Start(uint64_t startTime, uint64_t deadlineTime);
    int32_t Stop();
    std::shared_ptr<AudioWorkgroupCallbackForMonitor> callback;

private:
    int32_t workgroupId;
    std::unordered_map<int32_t, bool> threads;
};

} // namespace AudioStandard
} // namespace OHOS

#endif