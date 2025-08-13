/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 21;
static int32_t NUM_2 = 2;
typedef void (*TestFuncs)();

class RemoteObjectFuzzTestStub : public IRemoteObject {
public:
    RemoteObjectFuzzTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectFuzzTestStub");
};

class AudioClientTrackerFuzzTest : public AudioClientTracker {
    public:
        virtual ~AudioClientTrackerFuzzTest() = default;
        virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void SetLowPowerVolumeImpl(float volume) {};
        virtual void GetLowPowerVolumeImpl(float &volume) {};
        virtual void GetSingleStreamVolumeImpl(float &volume) {};
        virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
        virtual void UnsetOffloadModeImpl() {};
    };

const vector<DeviceFlag> g_testDeviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};

const vector<AudioStreamDeviceChangeReason> g_testReasons = {
    AudioStreamDeviceChangeReason::UNKNOWN,
    AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
    AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
    AudioStreamDeviceChangeReason::OVERRODE,
};

const vector<StreamSetState> g_testStreamSetStates = {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

void AudioCoreServiceDeInitFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->DeInit();
}

void AudioCoreServiceDumpPipeManagerFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::string dumpString = "abc";
    audioCoreService->DumpPipeManager(dumpString);
}

void AudioCoreServiceCheckAndSetCurrentOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t sessionId = 0;
    audioCoreService->CheckAndSetCurrentOutputDevice(desc, sessionId);
}

void AudioCoreServiceCheckAndSetCurrentInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->CheckAndSetCurrentInputDevice(desc);
}

void AudioCoreServiceSetCallDeviceActiveFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    InternalDeviceType deviceType = DEVICE_TYPE_EARPIECE;
    bool active = true;
    std::string address = "11-22-33-44-55-66";
    const int32_t uid = 0;
    audioCoreService->Init();
    audioCoreService->SetCallDeviceActive(deviceType, active, address, uid);
}

void AudioCoreServiceGetExcludedDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    audioCoreService->Init();
    audioCoreService->GetExcludedDevices(audioDevUsage);
}

void AudioCoreServiceFetchOutputDeviceForTrackFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioStreamChangeInfo streamChangeInfo;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->FetchOutputDeviceForTrack(streamChangeInfo, reason);
}

void AudioCoreServiceFetchInputDeviceForTrackFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioStreamChangeInfo streamChangeInfo;
    audioCoreService->FetchInputDeviceForTrack(streamChangeInfo);
}

void AudioCoreServiceExcludeOutputDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioCoreService->Init();
    audioCoreService->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceUnexcludeOutputDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    AudioRecoveryDevice::GetInstance().audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    audioCoreService->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceOnReceiveBluetoothEventFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string macAddress = "11-22-33-44-55-66";
    std::string deviceName = "deviceName";
    audioCoreService->Init();
    audioCoreService->OnReceiveBluetoothEvent(macAddress, deviceName);
}

void AudioCoreServiceNotifyRemoteRenderStateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string networkId = "abc";
    std::string condition = "123456";
    std::string value = "123456";
    audioCoreService->Init();
    audioCoreService->NotifyRemoteRenderState(networkId, condition, value);
}

void AudioCoreServiceOnCapturerSessionAddedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint64_t sessionID = 0;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    sessionInfo.rate = GetData<uint32_t>() % NUM_2;
    sessionInfo.channels = GetData<uint32_t>() % NUM_2;
    AudioStreamInfo streamInfo;
    audioCoreService->Init();
    audioCoreService->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreServiceOnCapturerSessionRemovedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint64_t sessionID = 0;
    audioCoreService->Init();
    audioCoreService->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreServiceTriggerFetchDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->Init();
    audioCoreService->TriggerFetchDevice(reason);
}

void AudioCoreServiceSetAudioDeviceAnahsCallbackFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    sptr<IRemoteObject> object =new RemoteObjectFuzzTestStub();
    audioCoreService->Init();
    audioCoreService->SetAudioDeviceAnahsCallback(object);
}

void AudioCoreServiceUnsetAudioDeviceAnahsCallbackFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->UnsetAudioDeviceAnahsCallback();
}

void AudioCoreServiceOnUpdateAnahsSupportFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string anahsShowType = "";
    audioCoreService->Init();
    audioCoreService->OnUpdateAnahsSupport(anahsShowType);
}

void AudioCoreServiceUnregisterBluetoothListenerFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->UnregisterBluetoothListener();
}

void AudioCoreServiceIsNoRunningStreamFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    audioCoreService->IsNoRunningStream(outputStreamDescs);
}

void AudioCoreServiceBluetoothServiceCrashedCallbackFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    pid_t pid = GetData<pid_t>();
    pid_t uid = GetData<pid_t>();
    audioCoreService->BluetoothServiceCrashedCallback(pid, uid);
}

void LoadSplitModuleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->LoadSplitModule("", "networkId");
    audioCoreService->LoadSplitModule("splitArgs", "networkId");
}

TestFuncs g_testFuncs[TESTSIZE] = {
    AudioCoreServiceDeInitFuzzTest,
    AudioCoreServiceDumpPipeManagerFuzzTest,
    AudioCoreServiceCheckAndSetCurrentOutputDeviceFuzzTest,
    AudioCoreServiceCheckAndSetCurrentInputDeviceFuzzTest,
    AudioCoreServiceSetCallDeviceActiveFuzzTest,
    AudioCoreServiceGetExcludedDevicesFuzzTest,
    AudioCoreServiceFetchOutputDeviceForTrackFuzzTest,
    AudioCoreServiceFetchInputDeviceForTrackFuzzTest,
    AudioCoreServiceExcludeOutputDevicesFuzzTest,
    AudioCoreServiceUnexcludeOutputDevicesFuzzTest,
    AudioCoreServiceOnReceiveBluetoothEventFuzzTest,
    AudioCoreServiceNotifyRemoteRenderStateFuzzTest,
    AudioCoreServiceOnCapturerSessionAddedFuzzTest,
    AudioCoreServiceOnCapturerSessionRemovedFuzzTest,
    AudioCoreServiceTriggerFetchDeviceFuzzTest,
    AudioCoreServiceSetAudioDeviceAnahsCallbackFuzzTest,
    AudioCoreServiceUnsetAudioDeviceAnahsCallbackFuzzTest,
    AudioCoreServiceOnUpdateAnahsSupportFuzzTest,
    AudioCoreServiceUnregisterBluetoothListenerFuzzTest,
    AudioCoreServiceIsNoRunningStreamFuzzTest,
    AudioCoreServiceBluetoothServiceCrashedCallbackFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
