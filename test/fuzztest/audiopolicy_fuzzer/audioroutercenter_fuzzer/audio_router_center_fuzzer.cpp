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
#include "securec.h"
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static int32_t NUM_2 = 2;
static constexpr size_t MAX_STRING_SIZE = 64;
static constexpr size_t MAX_STREAM_DEVICE_COUNT = 5;
static const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
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
    STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION,
    STREAM_USAGE_VOICE_TRANSFER_RINGTONE,
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

vector<AudioScene> AudioSceneVec = {
    AUDIO_SCENE_INVALID,
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING,
    AUDIO_SCENE_MAX,
};

vector<AudioPipeType> AudioPipeTypeVec = {
    PIPE_TYPE_UNKNOWN,
    PIPE_TYPE_OUT_NORMAL,
    PIPE_TYPE_IN_NORMAL,
    PIPE_TYPE_OUT_LOWLATENCY,
    PIPE_TYPE_IN_LOWLATENCY,
    PIPE_TYPE_OUT_CELLULAR_CALL,
    PIPE_TYPE_IN_CELLULAR_CALL,
    PIPE_TYPE_OUT_DIRECT_NORMAL,
};

vector<DeviceType> WirelessDeviceTypeVec = {
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_NONE,
};

vector<DeviceType> RouterDeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_WIRED_HEADSET,
};

vector<DeviceCategory> DeviceCategoryVec = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
    BT_WATCH,
    BT_HEARAID,
    BT_UNWEAR_HEADPHONE,
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

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& defaultValue)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_SIZE);
    return value.empty() ? defaultValue : value;
}

std::shared_ptr<AudioDeviceDescriptor> MakeRouterDevice(FuzzedDataProvider& fdp)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = RouterDeviceTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterDeviceTypeVec.size() - 1)];
    device->deviceCategory_ = DeviceCategoryVec[fdp.ConsumeIntegralInRange<size_t>(0, DeviceCategoryVec.size() - 1)];
    device->deviceRole_ = fdp.ConsumeBool() ? OUTPUT_DEVICE : INPUT_DEVICE;
    device->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    device->networkId_ = fdp.ConsumeBool() ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;
    return device;
}

std::shared_ptr<AudioDeviceDescriptor> MakeWirelessDevice(FuzzedDataProvider& fdp)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = WirelessDeviceTypeVec[fdp.ConsumeIntegralInRange<size_t>(
        0, WirelessDeviceTypeVec.size() - 1)];
    device->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    return device;
}

StreamDeviceInfo MakeStreamDeviceInfo(FuzzedDataProvider& fdp, bool isOutput)
{
    StreamDeviceInfo info;
    info.uid = fdp.ConsumeIntegral<int32_t>();
    info.sessionId = fdp.ConsumeIntegral<uint32_t>();
    info.streamUsage = StreamUsageVec[fdp.ConsumeIntegralInRange<size_t>(0, StreamUsageVec.size() - 1)];
    info.sourceType = SourceTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, SourceTypeVec.size() - 1)];
    info.device = MakeWirelessDevice(fdp);
    info.device->deviceRole_ = isOutput ? OUTPUT_DEVICE : INPUT_DEVICE;
    info.routerType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    info.isRunning = fdp.ConsumeBool();
    return info;
}

FetchDeviceInfo MakeFetchDeviceInfo(FuzzedDataProvider& fdp)
{
    FetchDeviceInfo info;
    info.streamUsage = StreamUsageVec[fdp.ConsumeIntegralInRange<size_t>(0, StreamUsageVec.size() - 1)];
    info.preStreamUsage = StreamUsageVec[fdp.ConsumeIntegralInRange<size_t>(0, StreamUsageVec.size() - 1)];
    info.clientUID = fdp.ConsumeIntegral<int32_t>();
    info.routerType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    info.audioPipeType = AudioPipeTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, AudioPipeTypeVec.size() - 1)];
    info.privacyType = PrivacyTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, PrivacyTypeVec.size() - 1)];
    info.caller = "fuzzer_test";
    info.bundleName = ConsumeString(fdp, "fuzzer_bundle");
    info.bypassType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    info.streamId = fdp.ConsumeIntegral<uint32_t>();
    return info;
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

    FetchDeviceInfo info;
    info.sourceType = sourceType;
    info.clientUID = clientUID;
    info.streamId = sessionID;
    AudioRouterCenter::GetAudioRouterCenter().FetchInputDevice(info);
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

    AudioRouterCenter::GetAudioRouterCenter().IsCallRenderRouter(streamUsage);
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

void WirelessConflictHandlerBasicFuzzTest(FuzzedDataProvider& fdp)
{
    WirelessConflictHandler handler;
    StreamUsage streamUsage = StreamUsageVec[fdp.ConsumeIntegralInRange<size_t>(0, StreamUsageVec.size() - 1)];
    SourceType sourceType = SourceTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, SourceTypeVec.size() - 1)];
    RouterType routerType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    bool isOutput = fdp.ConsumeBool();
    auto device = fdp.ConsumeBool() ? MakeWirelessDevice(fdp) : nullptr;

    handler.IsCallStreamUsage(streamUsage);
    handler.IsForcedRouterType(routerType);
    handler.GetPriority(streamUsage, sourceType, routerType, isOutput);
    handler.IsScoDevice(device);
    handler.IsNearlinkDevice(device);
    handler.IsWirelessDevice(device);
}

void WirelessConflictHandlerDetectFuzzTest(FuzzedDataProvider& fdp)
{
    WirelessConflictHandler handler;
    std::vector<StreamDeviceInfo> outputDevices;
    std::vector<StreamDeviceInfo> inputDevices;
    size_t outputCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_STREAM_DEVICE_COUNT);
    size_t inputCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_STREAM_DEVICE_COUNT);

    for (size_t i = 0; i < outputCount; ++i) {
        outputDevices.push_back(MakeStreamDeviceInfo(fdp, true));
    }
    for (size_t i = 0; i < inputCount; ++i) {
        inputDevices.push_back(MakeStreamDeviceInfo(fdp, false));
    }

    handler.SetSelectedDevices(outputDevices, inputDevices);
    auto device = MakeWirelessDevice(fdp);
    if (fdp.ConsumeBool()) {
        device->macAddress_.clear();
    }
    RouterType routerType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    handler.Detect(device, routerType);
    handler.FindHighestPriorityDevice();
    handler.ClearSelectedDevices();
    handler.Detect(device, routerType);
}

void WirelessConflictHandlerScoPairFuzzTest(FuzzedDataProvider& fdp)
{
    WirelessConflictHandler handler;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);

    StreamDeviceInfo outputInfo = MakeStreamDeviceInfo(fdp, true);
    outputInfo.device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    outputInfo.device->macAddress_ = macAddress;
    outputInfo.routerType = fdp.ConsumeBool() ? ROUTER_TYPE_PAIR_DEVICE : ROUTER_TYPE_USER_SELECT;

    StreamDeviceInfo inputInfo = MakeStreamDeviceInfo(fdp, false);
    inputInfo.device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    inputInfo.device->macAddress_ = macAddress;
    inputInfo.routerType = ROUTER_TYPE_PAIR_DEVICE;

    handler.SetSelectedDevices({ outputInfo }, { inputInfo });
    auto device = MakeWirelessDevice(fdp);
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    device->macAddress_ = macAddress;
    handler.Detect(device, ROUTER_TYPE_PAIR_DEVICE);
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

void AudioRouterCenterStrategyFuzzTest(FuzzedDataProvider& fdp)
{
    auto &center = AudioRouterCenter::GetAudioRouterCenter();
    AudioScene scene = AudioSceneVec[fdp.ConsumeIntegralInRange<size_t>(0, AudioSceneVec.size() - 1)];
    StreamUsage streamUsage = StreamUsageVec[fdp.ConsumeIntegralInRange<size_t>(0, StreamUsageVec.size() - 1)];
    auto device = MakeRouterDevice(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    size_t descCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_STREAM_DEVICE_COUNT);

    for (size_t i = 0; i < descCount; ++i) {
        descs.push_back(MakeRouterDevice(fdp));
    }
    center.NeedSkipSelectAudioOutputDeviceRefined(streamUsage, descs);
    center.GetBypassWithSco(scene);
    center.IsMediaFollowCallStrategy(scene);
    center.IsAlarmFollowRingStrategy(scene, streamUsage);
    center.IsWirelessPrivacyDevice(device);
    center.IsVehicleDevice(device);
    center.IsBlockedDeviceForTransferCall(device);
}

void AudioRouterCenterFetchRenderFuzzTest(FuzzedDataProvider& fdp)
{
    auto &center = AudioRouterCenter::GetAudioRouterCenter();
    FetchDeviceInfo info = MakeFetchDeviceInfo(fdp);
    RouterType routerType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    RouterType bypassType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    RouterType bypassWithSco = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];
    uint32_t streamId = fdp.ConsumeIntegral<uint32_t>();
    StreamUsage ringUsage = fdp.ConsumeBool() ? STREAM_USAGE_RINGTONE : STREAM_USAGE_VOICE_RINGTONE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;

    center.FetchMediaRenderDevice(info.streamUsage, info.clientUID, routerType, { bypassType }, streamId);
    center.FetchMediaRenderDevice(info, routerType, bypassType, streamId);
    center.FetchCallRenderDevice(info.streamUsage, info.clientUID, routerType, bypassType, bypassWithSco);
    center.FetchRingRenderDevices(info.streamUsage, info.clientUID, routerType);
    center.DealRingRenderRouters(descs, info, routerType);
    center.MediaFollowRingStrategy(info, ringUsage, routerType, bypassType, streamId);
    center.FetchOutputDevicesInner(info);
}

void AudioRouterCenterFetchCapturerFuzzTest(FuzzedDataProvider& fdp)
{
    auto &center = AudioRouterCenter::GetAudioRouterCenter();
    SourceType sourceType = SourceTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, SourceTypeVec.size() - 1)];
    int32_t clientUID = fdp.ConsumeIntegral<int32_t>();
    uint32_t sessionID = fdp.ConsumeIntegral<uint32_t>();
    RouterType routerType = RouterTypeVec[fdp.ConsumeIntegralInRange<size_t>(0, RouterTypeVec.size() - 1)];

    center.FetchCapturerInputDevice(sourceType, clientUID, routerType, sessionID);
    center.FetchCallCaptureDevice(sourceType, clientUID, routerType, sessionID);
    center.FetchRecordCaptureDevice(sourceType, clientUID, routerType, sessionID);
    center.FetchVoiceMessageCaptureDevice(sourceType, clientUID, routerType, sessionID);
}

void AudioRouterCenterArbiterFuzzTest(FuzzedDataProvider& fdp)
{
    auto &center = AudioRouterCenter::GetAudioRouterCenter();
    FetchDeviceInfo info = MakeFetchDeviceInfo(fdp);
    center.StartArbiterContext(info);
    if (fdp.ConsumeBool()) {
        center.RegisterConflictHandlers(std::make_shared<WirelessConflictHandler>());
    }
    center.StopArbiterContext();
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
    WirelessConflictHandlerBasicFuzzTest,
    WirelessConflictHandlerDetectFuzzTest,
    WirelessConflictHandlerScoPairFuzzTest,
    ClearConflictHandlersFuzzTest,
    SetSplitModeReadyFuzzTest,
    UpdateVehiclePriorityFuzzTest,
    AudioRouterCenterStrategyFuzzTest,
    AudioRouterCenterFetchRenderFuzzTest,
    AudioRouterCenterFetchCapturerFuzzTest,
    AudioRouterCenterArbiterFuzzTest,
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
