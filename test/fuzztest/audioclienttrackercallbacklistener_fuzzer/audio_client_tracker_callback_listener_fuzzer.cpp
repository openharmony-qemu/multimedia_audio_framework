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

#include <securec.h>

#include "audio_log.h"
#include "audio_client_tracker_callback_listener.h"
#include "audio_client_tracker_callback_service.h"
#include "../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

typedef void (*TestFuncs)();

void ClientTrackerCallbackListenerMuteStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    std::shared_ptr<ClientTrackerCallbackListener> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    CHECK_AND_RETURN(callback != nullptr);
    StreamSetStateEventInternal streamSetStateEventInternal;
    callback->MuteStreamImpl(streamSetStateEventInternal);
    callback->UnmuteStreamImpl(streamSetStateEventInternal);
    callback->PausedStreamImpl(streamSetStateEventInternal);
    callback->ResumeStreamImpl(streamSetStateEventInternal);
}

void ClientTrackerCallbackListenerGetSingleStreamVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    std::shared_ptr<ClientTrackerCallbackListener> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    CHECK_AND_RETURN(callback != nullptr);
    float volume = g_fuzzUtils.GetData<float>();
    callback->SetLowPowerVolumeImpl(volume);
    callback->GetLowPowerVolumeImpl(volume);
    callback->GetSingleStreamVolumeImpl(volume);
}

void ClientTrackerCallbackListenerSetOffloadModeImplFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    std::shared_ptr<ClientTrackerCallbackListener> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    CHECK_AND_RETURN(callback != nullptr);
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    bool isAppBack = g_fuzzUtils.GetData<bool>();
    callback->SetOffloadModeImpl(state, isAppBack);
}

void AudioClientTrackerCallbackServiceUnsetClientTrackerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    service.UnsetClientTrackerCallback();
}

void AudioClientTrackerCallbackServiceMuteStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal streamSetStateEventInternal;
    service.MuteStreamImpl(streamSetStateEventInternal);
    service.UnmuteStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackServicePausedStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal streamSetStateEventInternal;
    service.PausedStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackServiceSetLowPowerVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    float volume = g_fuzzUtils.GetData<float>();
    service.SetLowPowerVolumeImpl(volume);
}

void AudioClientTrackerCallbackServiceResumeStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal streamSetStateEventInternal;
    service.ResumeStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackServiceSetOffloadModeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    bool isAppBack = g_fuzzUtils.GetData<bool>();
    service.SetOffloadModeImpl(state, isAppBack);
}

void AudioClientTrackerCallbackServiceGetSingleStreamVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    float volume = g_fuzzUtils.GetData<float>();
    service.UnsetOffloadModeImpl();
    service.GetLowPowerVolumeImpl(volume);
    service.GetSingleStreamVolumeImpl(volume);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    ClientTrackerCallbackListenerMuteStreamImplFuzzTest,
    ClientTrackerCallbackListenerGetSingleStreamVolumeImplFuzzTest,
    ClientTrackerCallbackListenerSetOffloadModeImplFuzzTest,
    AudioClientTrackerCallbackServiceUnsetClientTrackerCallbackFuzzTest,
    AudioClientTrackerCallbackServiceMuteStreamImplFuzzTest,
    AudioClientTrackerCallbackServiceSetLowPowerVolumeImplFuzzTest,
    AudioClientTrackerCallbackServiceResumeStreamImplFuzzTest,
    AudioClientTrackerCallbackServiceSetOffloadModeImplFuzzTest,
    AudioClientTrackerCallbackServiceGetSingleStreamVolumeImplFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}