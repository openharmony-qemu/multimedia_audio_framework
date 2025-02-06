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
#ifndef AUDIO_POLICY_STATE_MONITOR_H
#define AUDIO_POLICY_STATE_MONITOR_H

#include <mutex>
#include <thread>
#include <map>
#include "singleton.h"

namespace OHOS {
namespace AudioStandard {

enum CallbackType {
    ONE_TIME,
    REPEAT,
};

enum {
    INVALID_CB_ID = -1,
    MAX_CB_ID_NUM = 100,
};

class CallBackTimeInfo {
public:
    virtual ~CallBackTimeInfo() = default;
    std::time_t startTimeStamp_; // Callback registration start timestamp, Unit: second
    std::time_t delayTime_; // Callback timeout duration, Unit: second
    CallbackType callbackType_ = ONE_TIME;
};

class AudioPolicyStateMonitorCallback : public CallBackTimeInfo {
public:
    virtual void OnTimeOut() = 0;
};

/**
 *  AudioPolicyStateMonitor's min timeout period is 1 second.
 *  It Support one-time or periodic timeout monitoring.
 *  It is generally used for monitoring with low precison requirement.
 */
class AudioPolicyStateMonitor {
    DECLARE_DELAYED_SINGLETON(AudioPolicyStateMonitor);
public:
    int32_t RegisterCallback(
        const std::shared_ptr<AudioPolicyStateMonitorCallback> &cb, std::time_t delayTime_, CallbackType callbackType);
    void UnRegisterCallback(int32_t cbId);

private:
    void TraverseAndInvokeTimeoutCallbacks();
    void CheckStateThreadMain();
    int32_t AllocateCbId();
    void FreeCbId(int32_t cbId);

private:
    bool idAllocator_[MAX_CB_ID_NUM] = {false};
    std::unordered_map<int32_t, std::shared_ptr<AudioPolicyStateMonitorCallback>> monitoredObj_;
    std::shared_ptr<std::thread> stateMonitorThread_;
    std::mutex monitorMutex_;
    std::mutex condMutex_;
    std::condition_variable stopCond_;
    bool threadStoped_ = false;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_STATE_MONITOR_H
