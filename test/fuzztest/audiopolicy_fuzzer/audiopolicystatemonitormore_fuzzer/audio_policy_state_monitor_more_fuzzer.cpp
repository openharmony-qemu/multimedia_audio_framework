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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <fuzzer/FuzzedDataProvider.h>
#include "audio_policy_state_monitor.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;

class StateMonitorCallbackMocker : public AudioPolicyStateMonitorCallback {
public:
    void OnTimeOut() override {}
};

void StopMonitorThread(const shared_ptr<AudioPolicyStateMonitor> &monitor)
{
    {
        unique_lock<mutex> lock(monitor->condMutex_);
        monitor->threadStoped_ = true;
    }
    monitor->stopCond_.notify_one();
    if (monitor->stateMonitorThread_ != nullptr && monitor->stateMonitorThread_->joinable()) {
        monitor->stateMonitorThread_->join();
    }
    monitor->stateMonitorThread_ = nullptr;
}

void ConstructorFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    StopMonitorThread(monitor);
}

void RegisterCallbackNullCbFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    shared_ptr<AudioPolicyStateMonitorCallback> nullCb = nullptr;
    time_t delayTime = fdp.ConsumeIntegral<int64_t>();
    int32_t callbackTypeCount = static_cast<int32_t>(CallbackType::REPEAT) + 1;
    CallbackType callbackType = static_cast<CallbackType>(fdp.ConsumeIntegral<uint8_t>() % callbackTypeCount);
    monitor->RegisterCallback(nullCb, delayTime, callbackType);
    StopMonitorThread(monitor);
}

void RegisterCallbackNoThreadFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    StopMonitorThread(monitor);
    auto cb = make_shared<StateMonitorCallbackMocker>();
    time_t delayTime = fdp.ConsumeIntegral<int64_t>();
    int32_t callbackTypeCount = static_cast<int32_t>(CallbackType::REPEAT) + 1;
    CallbackType callbackType = static_cast<CallbackType>(fdp.ConsumeIntegral<uint8_t>() % callbackTypeCount);
    monitor->RegisterCallback(cb, delayTime, callbackType);
}

void UnRegisterCallbackNoThreadFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    StopMonitorThread(monitor);
    int32_t cbId = fdp.ConsumeIntegral<int32_t>();
    monitor->UnRegisterCallback(cbId);
}

void TraverseNullCbFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    StopMonitorThread(monitor);
    int32_t cbId = fdp.ConsumeIntegral<int32_t>();
    monitor->monitoredObj_[cbId] = nullptr;
    monitor->TraverseAndInvokeTimeoutCallbacks();
}

void TraverseTimeoutCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    StopMonitorThread(monitor);
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());

    auto cbOneTime = make_shared<StateMonitorCallbackMocker>();
    cbOneTime->delayTime_ = 0;
    cbOneTime->callbackType_ = CallbackType::ONE_TIME;
    cbOneTime->startTimeStamp_ = now;
    int32_t cbId1 = monitor->AllocateCbId();
    if (cbId1 != INVALID_CB_ID) {
        monitor->monitoredObj_[cbId1] = cbOneTime;
    }

    auto cbRepeat = make_shared<StateMonitorCallbackMocker>();
    cbRepeat->delayTime_ = 0;
    cbRepeat->callbackType_ = CallbackType::REPEAT;
    cbRepeat->startTimeStamp_ = now;
    int32_t cbId2 = monitor->AllocateCbId();
    if (cbId2 != INVALID_CB_ID) {
        monitor->monitoredObj_[cbId2] = cbRepeat;
    }

    monitor->TraverseAndInvokeTimeoutCallbacks();
}

void AllocateCbIdExhaustFuzzTest(FuzzedDataProvider &fdp)
{
    auto monitor = make_shared<AudioPolicyStateMonitor>();
    StopMonitorThread(monitor);
    for (int32_t i = 0; i < MAX_CB_ID_NUM; ++i) {
        monitor->idAllocator_[i] = true;
    }
    int32_t cbId = monitor->AllocateCbId();
    (void)cbId;
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        ConstructorFuzzTest,
        RegisterCallbackNullCbFuzzTest,
        RegisterCallbackNoThreadFuzzTest,
        UnRegisterCallbackNoThreadFuzzTest,
        TraverseNullCbFuzzTest,
        TraverseTimeoutCallbackFuzzTest,
        AllocateCbIdExhaustFuzzTest,
    });
    func(fdp);
}
} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
