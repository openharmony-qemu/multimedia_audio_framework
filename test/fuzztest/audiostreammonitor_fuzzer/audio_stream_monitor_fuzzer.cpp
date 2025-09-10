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
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_stream_monitor.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

class DataTransferStateChangeCallbackForMonitorTest : public DataTransferStateChangeCallbackForMonitor {
public:
    void OnDataTransferStateChange(const int32_t &pid, const int32_t & callbackId,
        const AudioRendererDataTransferStateChangeInfo& info) override { return; }
    void OnMuteStateChange(const int32_t &pid, const int32_t &callbackId,
        const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override { return; }
};

void UnregisterAudioRendererDataTransferStateListenerFuzzTest()
{
    AudioProcessConfig cfg;
    cfg.originalSessionId = g_fuzzUtils.GetData<uint32_t>();
    cfg.appInfo.appUid = g_fuzzUtils.GetData<int32_t>();
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    DataTransferMonitorParam para;
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t callbackId = g_fuzzUtils.GetData<int32_t>();
    checker->InitChecker(para, pid, callbackId);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    para.clientUID = g_fuzzUtils.GetData<int32_t>();
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para,
        g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(cfg.originalSessionId);
    AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(g_fuzzUtils.GetData<int32_t>(),
        g_fuzzUtils.GetData<int32_t>());
}

void OnCallbackFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t callbackId = g_fuzzUtils.GetData<int32_t>();
    AudioRendererDataTransferStateChangeInfo info;
    DataTransferStateChangeCallbackForMonitorTest *test = new DataTransferStateChangeCallbackForMonitorTest();
    CHECK_AND_RETURN(test != nullptr);
    AudioStreamMonitor::GetInstance().SetAudioServerPtr(test);
    AudioStreamMonitor::GetInstance().OnCallback(pid, callbackId, info);
    delete test;
    test = nullptr;
}

void OnCallbackAppDiedFuzzTest()
{
    DataTransferMonitorParam para;
    para.clientUID = g_fuzzUtils.GetData<int32_t>();
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para,
        g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    AudioProcessConfig cfg;
    cfg.originalSessionId = g_fuzzUtils.GetData<uint32_t>();
    cfg.appInfo.appUid = g_fuzzUtils.GetData<int32_t>();
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    AudioStreamMonitor::GetInstance().OnCallbackAppDied(cfg.originalSessionId);
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(cfg.originalSessionId);
}

void NotifyAppStateChangeFuzzTest()
{
    DataTransferMonitorParam para;
    para.clientUID = g_fuzzUtils.GetData<int32_t>();
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para,
        g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    AudioProcessConfig cfg;
    cfg.originalSessionId = g_fuzzUtils.GetData<uint32_t>();
    cfg.appInfo.appUid = g_fuzzUtils.GetData<int32_t>();
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    AudioStreamMonitor::GetInstance().NotifyAppStateChange(g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<bool>());
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(cfg.originalSessionId);
}

void ReportStreamFreezenFuzzTest()
{
    int32_t intervalTime = g_fuzzUtils.GetData<int32_t>();
    AudioStreamMonitor::GetInstance().ReportStreamFreezen(intervalTime);
}

vector<TestFuncs> g_testFuncs = {
    UnregisterAudioRendererDataTransferStateListenerFuzzTest,
    OnCallbackFuzzTest,
    OnCallbackAppDiedFuzzTest,
    NotifyAppStateChangeFuzzTest,
    ReportStreamFreezenFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
