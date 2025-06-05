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
#ifndef ST_AUDIO_SESSION_STATE_MONITOR_H
#define ST_AUDIO_SESSION_STATE_MONITOR_H

#include <map>
#include <mutex>


namespace OHOS {
namespace AudioStandard {
class AudioSessionStateMonitor {
public:
    virtual ~AudioSessionStateMonitor() = default;
    virtual void OnAudioSessionTimeOut(int32_t pid) = 0;
    void StartMonitor(int32_t pid);
    void StopMonitor(int32_t pid);
    void RemoveFromMonitorMap(int32_t pid);

private:
    virtual std::shared_ptr<AudioSessionStateMonitor> GetSelfSharedPtr() = 0;

private:
    std::mutex sessionMonitorMutex_;
    std::unordered_map<int32_t, int32_t> pidCbIdMap_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SESSION_STATE_MONITOR_H
