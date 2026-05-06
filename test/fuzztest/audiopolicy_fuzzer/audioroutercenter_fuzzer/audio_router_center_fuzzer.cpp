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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_router_center.h"
#include "wireless_conflict_handler.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static int32_t NUM_2 = 2;
static size_t g_dataSize = 0;
static size_t g_pos;

typedef void (*TestFuncs)();

vector<StreamUsage> StreamUsageVec = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX,
};

vector<SourceType> SourceTypeVec = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX,
};

vector<RouterType> RouterTypeVec = {
    RouterType::ROUTER_TYPE_NONE,
    RouterType::ROUTER_TYPE_USER_SELECT,
    RouterType::ROUTER_TYPE_PRIVACY_PRIORITY,
    RouterType::ROUTER_TYPE_PUBLIC_PRIORITY,
    RouterType::ROUTER_TYPE_PACKAGE_FILTER,
    RouterType::ROUTER_TYPE_STREAM_FILTER,
    RouterType::ROUTER_TYPE_COCKPIT_PHONE,
    RouterType::ROUTER_TYPE_PAIR_DEVICE,
    RouterType::ROUTER_TYPE_DEFAULT,
};

vector<AudioPrivacyType> PrivacyTypeVec = {
    PRIVACY_TYPE_PUBLIC,
    PRIVACY_TYPE_PRIVATE,
};

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void FetchOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    StreamUsage streamUsage = StreamUsageVec[streamUsageCount];
    int32_t clientUID = GetData<int32_t>();
    std::string caller = "fuzzer_test";
    uint32_t routerTypeCount = GetData<uint32_t>() % RouterTypeVec.size();
    RouterType bypassType = RouterTypeVec[routerTypeCount];
    uint32_t privacyTypeCount = GetData<uint32_t>() % PrivacyTypeVec.size();
    AudioPrivacyType privacyType = PrivacyTypeVec[privacyTypeCount];
    
    FetchDeviceInfo fetchDeviceInfo;
    fetchDeviceInfo.streamUsage = streamUsage;
    fetchDeviceInfo.clientUID = clientUID;
    fetchDeviceInfo.routerType = bypassType;
    fetchDeviceInfo.privacyType = privacyType;
    fetchDeviceInfo.caller = caller;
    AudioRouterCenter::GetAudioRouterCenter().FetchOutputDevices(fetchDeviceInfo);
}

void FetchDupDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    FetchDeviceInfo fetchDeviceInfo;
    fetchDeviceInfo.streamUsage = StreamUsageVec[GetData<uint32_t>() % StreamUsageVec.size()];
    fetchDeviceInfo.preStreamUsage = StreamUsageVec[GetData<uint32_t>() % StreamUsageVec.size()];
    fetchDeviceInfo.clientUID = GetData<int32_t>();
    fetchDeviceInfo.routerType = RouterTypeVec[GetData<uint32_t>() % RouterTypeVec.size()];
    fetchDeviceInfo.audioPipeType = GetData<AudioPipeType>();
    fetchDeviceInfo.privacyType = PrivacyTypeVec[GetData<uint32_t>() % PrivacyTypeVec.size()];
    fetchDeviceInfo.caller = "fuzzer_test";
    
    AudioRouterCenter::GetAudioRouterCenter().FetchDupDevices(fetchDeviceInfo);
}

void FetchInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    int32_t clientUID = GetData<int32_t>();
    uint32_t sessionID = GetData<uint32_t>();
    RouterType routerType = ROUTER_TYPE_NONE;
    
    AudioRouterCenter::GetAudioRouterCenter().FetchInputDevice(sourceType, clientUID, routerType, sessionID);
}

void SetAudioDeviceRefinerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> object = nullptr;
    AudioRouterCenter::GetAudioRouterCenter().SetAudioDeviceRefinerCallback(object);
}

void UnsetAudioDeviceRefinerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRouterCenter::GetAudioRouterCenter().UnsetAudioDeviceRefinerCallback();
}

void IsCallRenderRouterFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    StreamUsage streamUsage = StreamUsageVec[streamUsageCount];
    
    AudioRouterCenter::GetAudioRouterCenter().isCallRenderRouter(streamUsage);
}

void GetSplitInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string splitInfo;
    AudioRouterCenter::GetAudioRouterCenter().GetSplitInfo(splitInfo);
}

void NotifyDistributedOutputChangeFuzzTest(FuzzedDataProvider& fdp)
{
    bool isRemote = GetData<uint32_t>() % NUM_2;
    AudioRouterCenter::GetAudioRouterCenter().NotifyDistributedOutputChange(isRemote);
}

void IsConfigRouterStrategyFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    
    AudioRouterCenter::GetAudioRouterCenter().IsConfigRouterStrategy(sourceType);
}

void RegisterConflictHandlersFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<ConflictHandler> handler = std::make_shared<WirelessConflictHandler>();
    AudioRouterCenter::GetAudioRouterCenter().RegisterConflictHandlers(handler);
}

void ClearConflictHandlersFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRouterCenter::GetAudioRouterCenter().ClearConflictHandlers();
}

void SetSplitModeReadyFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRouterCenter::GetAudioRouterCenter().SetSplitModeReady();
}

void UpdateVehiclePriorityFuzzTest(FuzzedDataProvider& fdp)
{
    bool enable = GetData<uint32_t>() % NUM_2;
    AudioRouterCenter::GetAudioRouterCenter().UpdateVehiclePriority(enable);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    FetchOutputDevicesFuzzTest,
    FetchDupDevicesFuzzTest,
    FetchInputDeviceFuzzTest,
    SetAudioDeviceRefinerCallbackFuzzTest,
    UnsetAudioDeviceRefinerCallbackFuzzTest,
    IsCallRenderRouterFuzzTest,
    GetSplitInfoFuzzTest,
    NotifyDistributedOutputChangeFuzzTest,
    IsConfigRouterStrategyFuzzTest,
    RegisterConflictHandlersFuzzTest,
    ClearConflictHandlersFuzzTest,
    SetSplitModeReadyFuzzTest,
    UpdateVehiclePriorityFuzzTest,
    });
    func(fdp);
}
void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}
void Init()
{
}
} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}