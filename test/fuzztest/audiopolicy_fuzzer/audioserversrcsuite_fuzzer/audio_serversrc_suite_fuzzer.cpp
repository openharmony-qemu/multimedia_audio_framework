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

/* Combined fuzz test suite for all source files under:
 * services/audio_policy/server/src/
 *
 * Covers 2 source files:
 *   audio_client_tracker_callback_service.cpp (9 test functions)
 *   audio_policy_manager_listener.cpp (9 test functions)
 */

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <fuzzer/FuzzedDataProvider.h>
#include "securec.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_interrupt_info.h"
#include "audio_policy_interface.h"
#include "audio_client_tracker_callback_service.h"
#include "audio_policy_manager_listener.h"
#include "audio_stream_types.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

// ==================== Mock: AudioClientTracker ====================

class AudioClientTrackerFuzzMock : public AudioClientTracker {
public:
    AudioClientTrackerFuzzMock() = default;
    ~AudioClientTrackerFuzzMock() = default;
    void MuteStreamImpl(const StreamSetStateEventInternal &e) override { (void)e; }
    void UnmuteStreamImpl(const StreamSetStateEventInternal &e) override { (void)e; }
    void PausedStreamImpl(const StreamSetStateEventInternal &e) override { (void)e; }
    void ResumeStreamImpl(const StreamSetStateEventInternal &e) override { (void)e; }
    void SetLowPowerVolumeImpl(float volume) override { (void)volume; }
    void GetLowPowerVolumeImpl(float &volume) override { (void)volume; }
    void SetOffloadModeImpl(int32_t state, bool isAppBack) override
    {
        (void)state;
        (void)isAppBack;
    }
    void UnsetOffloadModeImpl() override {}
    void GetSingleStreamVolumeImpl(float &volume) override { (void)volume; }
};

// ==================== Mock: IStandardAudioPolicyManagerListener ====================

class PolicyManagerListenerFuzzMock : public IStandardAudioPolicyManagerListener {
public:
    PolicyManagerListenerFuzzMock() = default;
    ~PolicyManagerListenerFuzzMock() = default;
    sptr<IRemoteObject> AsObject() override { return nullptr; }
    ErrCode OnInterrupt(const InterruptEventInternal &) override { return SUCCESS; }
    ErrCode OnRouteUpdate(uint32_t, const std::string &) override { return SUCCESS; }
    ErrCode OnAvailableDeviceChange(uint32_t, const DeviceChangeAction &) override { return SUCCESS; }
    ErrCode OnQueryClientType(const std::string &, uint32_t, bool &ret) override
    {
        ret = true;
        return SUCCESS;
    }

    ErrCode OnCheckClientInfo(const std::string &, int32_t &, int32_t, bool &ret) override
    {
        ret = true;
        return SUCCESS;
    }

    ErrCode OnCheckMediaControllerBundle(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnQueryIsForceGetZoneDevice(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnCheckVKBInfo(const std::string &, bool &isValid) override
    {
        isValid = false;
        return SUCCESS;
    }

    ErrCode OnQueryAllowedPlayback(int32_t, int32_t, bool &ret) override
    {
        ret = true;
        return SUCCESS;
    }

    ErrCode OnBackgroundMute(int32_t) override { return SUCCESS; }
    ErrCode OnQueryBundleNameIsInList(const std::string &, const std::string &, bool &ret) override
    {
        ret = true;
        return SUCCESS;
    }

    ErrCode OnQueryDeviceVolumeBehavior(VolumeBehavior &) override { return SUCCESS; }
    ErrCode OnQueryIsForceGetDevByVolumeType(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnQueryUseSystemPreferredDeviceForVolume(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode StartVoiceBroadcast(uint32_t, const std::string &) override { return SUCCESS; }
};

// ==================== Helpers ====================

template <class T>
uint32_t GetArrLength(T &arr)
{
    if (arr == nullptr) { return 0; }
    return sizeof(arr) / sizeof(arr[0]);
}

template <class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) { return object; }
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) { return object; }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) { return {}; }
    g_pos += objectSize;
    return object;
}

std::string GetFuzzString()
{
    std::string str;
    uint8_t len = GetData<uint8_t>() % 64;
    for (uint8_t i = 0; i < len; ++i) {
        str.push_back(static_cast<char>(GetData<uint8_t>()));
    }
    return str;
}

void BuildStreamSetStateEventInternal(StreamSetStateEventInternal &event)
{
    event.streamSetState = static_cast<StreamSetState>(GetData<int32_t>());
    event.streamUsage = static_cast<StreamUsage>(GetData<int32_t>());
}

// ==================== audio_client_tracker_callback_service.cpp ====================

void SvcMuteStreamImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    StreamSetStateEventInternal event;
    BuildStreamSetStateEventInternal(event);
    callback->MuteStreamImpl(event);
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcUnmuteStreamImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    StreamSetStateEventInternal event;
    BuildStreamSetStateEventInternal(event);
    callback->UnmuteStreamImpl(event);
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcPausedStreamImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    StreamSetStateEventInternal event;
    BuildStreamSetStateEventInternal(event);
    callback->PausedStreamImpl(event);
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcSetLowPowerVolumeImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    callback->SetLowPowerVolumeImpl(GetData<float>());
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcResumeStreamImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    StreamSetStateEventInternal event;
    BuildStreamSetStateEventInternal(event);
    callback->ResumeStreamImpl(event);
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcSetOffloadModeImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    callback->SetOffloadModeImpl(GetData<int32_t>(), GetData<bool>());
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcUnsetOffloadModeImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    callback->UnsetOffloadModeImpl();
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcGetLowPowerVolumeImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    float volume = GetData<float>();
    callback->GetLowPowerVolumeImpl(volume);
    callback->UnsetClientTrackerCallback();
    delete callback;
}

void SvcGetSingleStreamVolumeImplFuzzTest()
{
    auto callback = new (std::nothrow) AudioClientTrackerCallbackService();
    CHECK_AND_RETURN(callback != nullptr);
    auto tracker = std::make_shared<AudioClientTrackerFuzzMock>();
    callback->SetClientTrackerCallback(tracker);
    float volume = GetData<float>();
    callback->GetSingleStreamVolumeImpl(volume);
    callback->UnsetClientTrackerCallback();
    delete callback;
}

// ==================== audio_policy_manager_listener.cpp ====================

void LstOnInterruptFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    InterruptEventInternal event;
    event.eventType = static_cast<InterruptType>(GetData<int32_t>());
    event.forceType = static_cast<InterruptForceType>(GetData<int32_t>());
    event.hintType = static_cast<InterruptHint>(GetData<int32_t>());
    event.duckVolume = GetData<float>();
    event.callbackToApp = GetData<bool>();
    event.eventTimestamp = GetData<int64_t>();
    callback.OnInterrupt(event);
}

void LstOnAvailableDeviceChangeFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(GetData<uint32_t>());
    DeviceChangeAction deviceChangeAction;
    deviceChangeAction.type = static_cast<DeviceChangeType>(GetData<int32_t>());
    deviceChangeAction.flag = static_cast<DeviceFlag>(GetData<int32_t>());
    callback.OnAvailableDeviceChange(usage, deviceChangeAction);
}

void LstOnQueryClientTypeFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    callback.OnQueryClientType(GetFuzzString(), GetData<uint32_t>());
}

void LstOnCheckClientInfoFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    int32_t uid = GetData<int32_t>();
    callback.OnCheckClientInfo(GetFuzzString(), uid, GetData<int32_t>());
}

void LstOnQueryAllowedPlaybackFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    callback.OnQueryAllowedPlayback(GetData<int32_t>(), GetData<int32_t>());
}

void LstOnBackgroundMuteFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    callback.OnBackgroundMute(GetData<int32_t>());
}

void LstOnQueryBundleNameIsInListFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    callback.OnQueryBundleNameIsInList(GetFuzzString(), GetFuzzString());
}

void LstOnQueryIsForceGetDevByVolumeTypeFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    callback.OnQueryIsForceGetDevByVolumeType(GetFuzzString());
}

void LstOnQueryUseSystemPreferredDeviceForVolumeFuzzTest()
{
    sptr<PolicyManagerListenerFuzzMock> listener = sptr<PolicyManagerListenerFuzzMock>::MakeSptr();
    CHECK_AND_RETURN(listener != nullptr);
    AudioPolicyManagerListenerCallback callback(listener);
    callback.OnQueryUseSystemPreferredDeviceForVolume(GetFuzzString());
}

// ==================== Dispatch Table ====================

TestPtr g_testPtrs[] = {
    // audio_client_tracker_callback_service.cpp
    SvcMuteStreamImplFuzzTest,
    SvcUnmuteStreamImplFuzzTest,
    SvcPausedStreamImplFuzzTest,
    SvcSetLowPowerVolumeImplFuzzTest,
    SvcResumeStreamImplFuzzTest,
    SvcSetOffloadModeImplFuzzTest,
    SvcUnsetOffloadModeImplFuzzTest,
    SvcGetLowPowerVolumeImplFuzzTest,
    SvcGetSingleStreamVolumeImplFuzzTest,
    // audio_policy_manager_listener.cpp
    LstOnInterruptFuzzTest,
    LstOnAvailableDeviceChangeFuzzTest,
    LstOnQueryClientTypeFuzzTest,
    LstOnCheckClientInfoFuzzTest,
    LstOnQueryAllowedPlaybackFuzzTest,
    LstOnBackgroundMuteFuzzTest,
    LstOnQueryBundleNameIsInListFuzzTest,
    LstOnQueryIsForceGetDevByVolumeTypeFuzzTest,
    LstOnQueryUseSystemPreferredDeviceForVolumeFuzzTest,
};

void FuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr) { return; }
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;
    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    }
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) { return 0; }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
