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
#include "audio_select_interface_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const uint32_t MAX_SESSION_ID = 100000;
const int32_t MIN_TEST_UID = 0;
const int32_t MAX_TEST_UID = 10000;
const uint32_t MIN_CUSTOM_SAMPLE_RATE = 8000;
const uint32_t MAX_CUSTOM_SAMPLE_RATE = 192000;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
typedef void (*TestFuncs)();
void Init();
shared_ptr<AudioCoreService> audioCoreService;
shared_ptr<AudioCoreService::EventEntry> eventEntry;
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

const vector<DeviceType> g_testOutputDeviceTypes = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_NEARLINK,
};

const vector<DeviceType> g_testInputDeviceTypes = {
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

const vector<DeviceRole> g_testDeviceRoles = {
    INPUT_DEVICE,
    OUTPUT_DEVICE,
};

const vector<DeviceCategory> g_testDeviceCategories = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
};

const vector<AudioDeviceUsage> g_testAudioDeviceUsages = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

const vector<StreamUsage> g_testStreamUsages = {
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_VIDEO_COMMUNICATION,
};

const vector<ContentType> g_testContentTypes = {
    CONTENT_TYPE_MUSIC,
    CONTENT_TYPE_SPEECH,
    CONTENT_TYPE_MOVIE,
    CONTENT_TYPE_SONIFICATION,
    CONTENT_TYPE_RINGTONE,
};

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
};

const vector<AudioSamplingRate> g_testSamplingRates = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const vector<AudioEncodingType> g_testEncodings = {
    ENCODING_PCM,
    ENCODING_AUDIOVIVID,
    ENCODING_EAC3,
};

const vector<AudioSampleFormat> g_testFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const vector<AudioChannel> g_testChannels = {
    MONO,
    STEREO,
    CHANNEL_4,
    CHANNEL_6,
};

const vector<AudioChannelLayout> g_testChannelLayouts = {
    CH_LAYOUT_MONO,
    CH_LAYOUT_STEREO,
    CH_LAYOUT_QUAD,
    CH_LAYOUT_5POINT1,
};

const vector<AudioFlag> g_testAudioFlags = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
};

const vector<AudioMode> g_testAudioModes = {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD,
};

const vector<AudioStreamStatus> g_testStreamStatuses = {
    STREAM_STATUS_NEW,
    STREAM_STATUS_STARTED,
    STREAM_STATUS_PAUSED,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_RELEASED,
};

const vector<AudioStreamAction> g_testStreamActions = {
    AUDIO_STREAM_ACTION_DEFAULT,
    AUDIO_STREAM_ACTION_NEW,
    AUDIO_STREAM_ACTION_MOVE,
    AUDIO_STREAM_ACTION_RECREATE,
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

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

AudioStreamInfo MakeStreamInfo(FuzzedDataProvider& fdp)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = PickValue(fdp, g_testSamplingRates);
    streamInfo.encoding = PickValue(fdp, g_testEncodings);
    streamInfo.format = PickValue(fdp, g_testFormats);
    streamInfo.channels = PickValue(fdp, g_testChannels);
    streamInfo.channelLayout = PickValue(fdp, g_testChannelLayouts);
    streamInfo.customSampleRate = fdp.ConsumeBool() ?
        fdp.ConsumeIntegralInRange<uint32_t>(MIN_CUSTOM_SAMPLE_RATE, MAX_CUSTOM_SAMPLE_RATE) : 0;
    return streamInfo;
}

std::shared_ptr<AudioDeviceDescriptor> MakeDeviceDescriptor(FuzzedDataProvider& fdp, DeviceRole role)
{
    const auto& deviceTypes = (role == INPUT_DEVICE) ? g_testInputDeviceTypes : g_testOutputDeviceTypes;
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceName_ = ConsumeString(fdp, "audio_fuzz_device");
    desc->deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    desc->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    if (fdp.ConsumeBool()) {
        DeviceStreamInfo mmapCap;
        mmapCap.fastSamplingRate.insert(AudioSamplingRate::SAMPLE_RATE_48000);
        mmapCap.ufastSamplingRate.insert(AudioSamplingRate::SAMPLE_RATE_48000);
        desc->audioStreamInfo_.push_back(mmapCap);
    }
    return desc;
}

std::shared_ptr<AudioStreamDescriptor> MakeStreamDescriptor(FuzzedDataProvider& fdp, bool isInput)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_ = MakeStreamInfo(fdp);
    streamDesc->micInStreamInfo_ = MakeStreamInfo(fdp);
    streamDesc->ecStreamInfo_ = MakeStreamInfo(fdp);
    streamDesc->audioMode_ = isInput ? AUDIO_MODE_RECORD : AUDIO_MODE_PLAYBACK;
    if (fdp.ConsumeBool()) {
        streamDesc->audioMode_ = PickValue(fdp, g_testAudioModes);
    }
    streamDesc->audioFlag_ = PickValue(fdp, g_testAudioFlags);
    streamDesc->routeFlag_ = PickValue(fdp, g_testAudioFlags);
    streamDesc->oldRouteFlag_ = PickValue(fdp, g_testAudioFlags);
    streamDesc->futureRouteFlag_ = PickValue(fdp, g_testAudioFlags);
    streamDesc->sessionId_ = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    streamDesc->callerUid_ = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    streamDesc->callerPid_ = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    streamDesc->appInfo_.appUid = streamDesc->callerUid_;
    streamDesc->appInfo_.appPid = streamDesc->callerPid_;
    streamDesc->rendererTarget_ = fdp.ConsumeIntegralInRange<uint32_t>(
        NORMAL_PLAYBACK, INJECT_TO_VOICE_COMMUNICATION_CAPTURE);
    streamDesc->streamStatus_ = PickValue(fdp, g_testStreamStatuses);
    streamDesc->streamAction_ = PickValue(fdp, g_testStreamActions);
    streamDesc->bundleName_ = ConsumeString(fdp, "audio_core_service_fuzzer");

    streamDesc->rendererInfo_.contentType = PickValue(fdp, g_testContentTypes);
    streamDesc->rendererInfo_.streamUsage = PickValue(fdp, g_testStreamUsages);
    streamDesc->rendererInfo_.rendererFlags = static_cast<int32_t>(PickValue(fdp, g_testAudioFlags));
    streamDesc->rendererInfo_.originalFlag = static_cast<int32_t>(PickValue(fdp, g_testAudioFlags));
    streamDesc->rendererInfo_.samplingRate = streamDesc->streamInfo_.samplingRate;
    streamDesc->rendererInfo_.format = streamDesc->streamInfo_.format;

    streamDesc->capturerInfo_.sourceType = PickValue(fdp, g_testSourceTypes);
    streamDesc->capturerInfo_.capturerFlags = static_cast<int32_t>(PickValue(fdp, g_testAudioFlags));
    streamDesc->capturerInfo_.originalFlag = static_cast<int32_t>(PickValue(fdp, g_testAudioFlags));
    streamDesc->capturerInfo_.samplingRate = streamDesc->streamInfo_.samplingRate;

    auto inputDevice = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    streamDesc->preferredInputDevice = *inputDevice;
    streamDesc->oldDeviceDescs_.push_back(MakeDeviceDescriptor(fdp, isInput ? INPUT_DEVICE : OUTPUT_DEVICE));
    streamDesc->newDeviceDescs_.push_back(MakeDeviceDescriptor(fdp, isInput ? INPUT_DEVICE : OUTPUT_DEVICE));
    return streamDesc;
}

void AudioCoreServiceDumpPipeManagerFuzzTest(FuzzedDataProvider& fdp)
{
    std::string dumpString = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->DumpPipeManager(dumpString);
}

void AudioCoreServiceCheckAndSetCurrentOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    int32_t sessionId = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_SESSION_ID);
    audioCoreService->CheckAndSetCurrentOutputDevice(desc, sessionId);
}

void AudioCoreServiceCheckAndSetCurrentInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    audioCoreService->CheckAndSetCurrentInputDevice(desc, uid);
}

void AudioCoreServiceSetCallDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = PickValue(fdp, g_testOutputDeviceTypes);
    bool active = fdp.ConsumeBool();
    std::string address = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    audioCoreService->SetCallDeviceActive(deviceType, active, address, uid);
}

void AudioCoreServiceGetExcludedDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = PickValue(fdp, g_testAudioDeviceUsages);
    audioCoreService->GetExcludedDevices(audioDevUsage);
}

void AudioCoreServiceExcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = PickValue(fdp, g_testAudioDeviceUsages);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(MakeDeviceDescriptor(fdp, OUTPUT_DEVICE));
    audioCoreService->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceUnexcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = PickValue(fdp, g_testAudioDeviceUsages);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(MakeDeviceDescriptor(fdp, OUTPUT_DEVICE));
    AudioRecoveryDevice::GetInstance().audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    audioCoreService->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceOnReceiveUpdateDeviceNameEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string deviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
}

void AudioCoreServiceNotifyRemoteRenderStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string condition = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->NotifyRemoteRenderState(networkId, condition, value);
}

void AudioCoreServiceOnCapturerSessionRemovedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegral<uint64_t>();
    audioCoreService->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreServiceTriggerFetchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->TriggerFetchDevice(reason);
}

void AudioCoreServiceOnCapturerSessionAddedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_SESSION_ID);
    SessionInfo sessionInfo;
    sessionInfo.sourceType = PickValue(fdp, g_testSourceTypes);
    sessionInfo.rate = static_cast<uint32_t>(PickValue(fdp, g_testSamplingRates));
    sessionInfo.channels = static_cast<uint32_t>(PickValue(fdp, g_testChannels));
    AudioStreamInfo streamInfo = MakeStreamInfo(fdp);
    audioCoreService->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreServiceUnsetAudioDeviceAnahsCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->UnsetAudioDeviceAnahsCallback();
}

void AudioCoreServiceOnUpdateAnahsSupportFuzzTest(FuzzedDataProvider& fdp)
{
    std::string anahsShowType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->OnUpdateAnahsSupport(anahsShowType);
}

void AudioCoreServiceIsNoRunningStreamFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    audioCoreService->IsNoRunningStream(outputStreamDescs);
}

void AudioCoreServiceBluetoothServiceCrashedCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    pid_t pid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    pid_t uid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    audioCoreService->BluetoothServiceCrashedCallback(pid, uid);
}

void AudioCoreServiceLoadSplitModuleFuzzTest(FuzzedDataProvider& fdp)
{
    CHECK_AND_RETURN(audioCoreService != nullptr);
    std::string splitArgs = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->LoadSplitModule("", networkId);
    audioCoreService->LoadSplitModule(splitArgs, networkId);
}

void AudioCoreServiceSetPreferredInputDeviceIfValidFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, true);
    audioCoreService->SetPreferredInputDeviceIfValid(streamDesc);
}

void AudioSelectInterfaceServiceSetInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, g_testInputDeviceTypes);
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    AudioSelectInterfaceService::GetInstance().SetInputDevice(deviceType, sessionId, uid);
}

void AudioSelectInterfaceServiceSetPreferredInputDeviceIfValidFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, true);
    streamDesc->preferredInputDevice.deviceRole_ = INPUT_DEVICE;
    AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(streamDesc);
}

void AudioCoreServiceParsePreferredInputDeviceHistoryFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, true);
    audioCoreService->ParsePreferredInputDeviceHistory(streamDesc);
}

void AudioCoreServiceGetFlagForMmapStreamFuzzTest(FuzzedDataProvider& fdp)
{
    bool isInput = fdp.ConsumeBool();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, isInput);
    audioCoreService->GetFlagForMmapStream(streamDesc);
}

void AudioCoreServiceGetPaIndexByPortNameFuzzTest(FuzzedDataProvider& fdp)
{
    std::string portName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->GetPaIndexByPortName(portName);
}

void AudioCoreServiceNotifyDistributedOutputChangeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    audioCoreService->NotifyDistributedOutputChange(desc);
}

void AudioCoreServiceSelectInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    audioCapturerFilter->uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    audioCapturerFilter->capturerInfo.sourceType = PickValue(fdp, g_testSourceTypes);
    audioCapturerFilter->capturerInfo.capturerFlags = static_cast<int32_t>(PickValue(fdp, g_testAudioFlags));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(MakeDeviceDescriptor(fdp, INPUT_DEVICE));
    audioCoreService->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioCoreServiceGetPreferBluetoothAndNearlinkRecordByUidFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    audioCoreService->GetPreferBluetoothAndNearlinkRecordByUid(uid);
}

void AudioCoreServiceCloseWakeUpAudioCapturerFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->CloseWakeUpAudioCapturer();
}

void AudioCoreServiceUnregisterBluetoothListenerFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->UnregisterBluetoothListener();
}

void AudioCoreServiceConfigDistributedRoutingRoleFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceRole deviceRole = PickValue(fdp, g_testDeviceRoles);
    std::shared_ptr<AudioDeviceDescriptor> descriptor = MakeDeviceDescriptor(fdp, deviceRole);
    CastType type = ConsumeEnum<CastType>(fdp);
    audioCoreService->ConfigDistributedRoutingRole(descriptor, type);
}

void AudioCoreServiceHandleA2dpSuspendWhenLoadFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString();
    audioCoreService->HandleA2dpSuspendWhenLoad(macAddress);
}

void AudioCoreServiceHandleA2dpRestoreFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString();
    audioCoreService->HandleA2dpRestore(macAddress);
}

void AudioCoreServiceCaptureConcurrentCheckFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    audioCoreService->CaptureConcurrentCheck(sessionId);
}

void AudioCoreServiceUpdateStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string pipeName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string networkId = ConsumeString(fdp, "fuzz_network_id");
    DeviceStreamInfo streamInfo;
    std::list<DeviceStreamInfo> deviceStreamInfo;
    deviceStreamInfo.push_back(streamInfo);
    std::list<std::string> supportDevices;
    supportDevices.push_back(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    audioCoreService->UpdateStreamPropInfo(adapterName, pipeName, deviceStreamInfo, supportDevices, networkId);
}

void AudioCoreServiceGetStreamPropInfoSizeFuzzTest(FuzzedDataProvider& fdp)
{
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string pipeName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->GetStreamPropInfoSize(adapterName, pipeName);
}

void AudioCoreServiceIsA2dpOffloadStreamFuzzTest(FuzzedDataProvider& fdp)
{
    uint sessionId = fdp.ConsumeIntegralInRange<uint>(0, MAX_SESSION_ID);
    audioCoreService->IsA2dpOffloadStream(sessionId);
}

void AudioCoreServiceSetRendererTargetFuzzTest(FuzzedDataProvider& fdp)
{
    RenderTarget target = static_cast<RenderTarget>(
        fdp.ConsumeIntegralInRange<uint32_t>(NORMAL_PLAYBACK, INJECT_TO_VOICE_COMMUNICATION_CAPTURE));
    RenderTarget lastTarget = static_cast<RenderTarget>(
        fdp.ConsumeIntegralInRange<uint32_t>(NORMAL_PLAYBACK, INJECT_TO_VOICE_COMMUNICATION_CAPTURE));
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    audioCoreService->SetRendererTarget(target, lastTarget, sessionId);
}

void AudioCoreServiceStartInjectionFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    audioCoreService->StartInjection(streamId);
}

void AudioCoreServiceRemoveIdForInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    audioCoreService->RemoveIdForInjector(streamId);
}

void AudioCoreServiceReleaseCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->ReleaseCaptureInjector();
}

void AudioCoreServiceRebuildCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    audioCoreService->RebuildCaptureInjector(streamId);
}

void AudioCoreServiceA2dpOffloadGetRenderPositionFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t delayValue = fdp.ConsumeIntegral<uint32_t>();
    uint64_t sendDataSize = fdp.ConsumeIntegral<uint64_t>();
    uint32_t timeStamp = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

void AudioCoreServiceInVideoCommFastBlockListFuzzTest(FuzzedDataProvider& fdp)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->InVideoCommFastBlockList(bundleName);
}

void AudioCoreServiceSetQueryBundleNameListCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    audioCoreService->SetQueryBundleNameListCallback(remoteObject);
}

void AudioCoreServiceOnCheckActiveMusicTimeFuzzTest(FuzzedDataProvider& fdp)
{
    std::string reason = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->OnCheckActiveMusicTime(reason);
    audioCoreService->IsDistributeServiceOnline();
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    audioCoreService->HandleDeviceConfigChanged(selectedAudioDevice);
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    DeviceType deviceType = PickValue(fdp, g_testOutputDeviceTypes);
    bool enable = fdp.ConsumeBool();
    audioCoreService->NotifyRemoteRouteStateChange(networkId, deviceType, enable);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);
    audioCoreService->FetchAndActivateOutputDevice(deviceDesc, streamDesc);
}

void AudioCoreServiceCheckInterphoneFuzzTest(FuzzedDataProvider& fdp)
{
    StreamUsage streamUsage = PickValue(fdp, g_testStreamUsages);
    DeviceRole deviceRole = PickValue(fdp, g_testDeviceRoles);
    std::shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, deviceRole);

    bool hasPairDevice = fdp.ConsumeBool();
    if (hasPairDevice) {
        DeviceRole pairDeviceRole = PickValue(fdp, g_testDeviceRoles);
        desc->pairDeviceDescriptor_ = MakeDeviceDescriptor(fdp, pairDeviceRole);
    }

    AudioPolicyUtils::CheckInterphone(streamUsage, desc);
}

void Test(FuzzedDataProvider& fdp)
{
    if (audioCoreService == nullptr) {
        Init();
    }
    auto func = fdp.PickValueInArray({
    AudioCoreServiceDumpPipeManagerFuzzTest,
    AudioCoreServiceCheckAndSetCurrentOutputDeviceFuzzTest,
    AudioCoreServiceCheckAndSetCurrentInputDeviceFuzzTest,
    AudioCoreServiceSetCallDeviceActiveFuzzTest,
    AudioCoreServiceGetExcludedDevicesFuzzTest,
    AudioCoreServiceExcludeOutputDevicesFuzzTest,
    AudioCoreServiceUnexcludeOutputDevicesFuzzTest,
    AudioCoreServiceOnReceiveUpdateDeviceNameEventFuzzTest,
    AudioCoreServiceNotifyRemoteRenderStateFuzzTest,
    AudioCoreServiceOnCapturerSessionAddedFuzzTest,
    AudioCoreServiceOnCapturerSessionRemovedFuzzTest,
    AudioCoreServiceTriggerFetchDeviceFuzzTest,
    AudioCoreServiceUnsetAudioDeviceAnahsCallbackFuzzTest,
    AudioCoreServiceOnUpdateAnahsSupportFuzzTest, AudioCoreServiceIsNoRunningStreamFuzzTest,
    AudioCoreServiceBluetoothServiceCrashedCallbackFuzzTest,
    AudioCoreServiceSetPreferredInputDeviceIfValidFuzzTest,
    AudioSelectInterfaceServiceSetInputDeviceFuzzTest,
    AudioSelectInterfaceServiceSetPreferredInputDeviceIfValidFuzzTest,
    AudioCoreServiceParsePreferredInputDeviceHistoryFuzzTest,
    AudioCoreServiceGetFlagForMmapStreamFuzzTest,
    AudioCoreServiceGetPaIndexByPortNameFuzzTest,
    AudioCoreServiceNotifyDistributedOutputChangeFuzzTest,
    AudioCoreServiceSelectInputDeviceFuzzTest,
    AudioCoreServiceGetPreferBluetoothAndNearlinkRecordByUidFuzzTest,
    AudioCoreServiceCloseWakeUpAudioCapturerFuzzTest,
    AudioCoreServiceUnregisterBluetoothListenerFuzzTest,
    AudioCoreServiceConfigDistributedRoutingRoleFuzzTest,
    AudioCoreServiceHandleA2dpSuspendWhenLoadFuzzTest,
    AudioCoreServiceHandleA2dpRestoreFuzzTest,
    AudioCoreServiceCaptureConcurrentCheckFuzzTest,
    AudioCoreServiceUpdateStreamPropInfoFuzzTest,
    AudioCoreServiceGetStreamPropInfoSizeFuzzTest,
    AudioCoreServiceIsA2dpOffloadStreamFuzzTest,
    AudioCoreServiceSetRendererTargetFuzzTest,
    AudioCoreServiceStartInjectionFuzzTest,
    AudioCoreServiceRemoveIdForInjectorFuzzTest,
    AudioCoreServiceReleaseCaptureInjectorFuzzTest,
    AudioCoreServiceRebuildCaptureInjectorFuzzTest,
    AudioCoreServiceA2dpOffloadGetRenderPositionFuzzTest,
    AudioCoreServiceInVideoCommFastBlockListFuzzTest,
    AudioCoreServiceSetQueryBundleNameListCallbackFuzzTest,
    AudioCoreServiceOnCheckActiveMusicTimeFuzzTest, AudioCoreServiceCheckInterphoneFuzzTest,
    });
    func(fdp);
}
void Init()
{
    audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    eventEntry = std::make_shared<AudioCoreService::EventEntry>(
        audioCoreService, audioCoreService->readyConditionNotifier_);
    audioCoreService->pipeManager_->SetRequiredPipesReadyCallback([]() {
        audioCoreService->NotifyRequiredPipesReady();
    });
    audioCoreService->pipeManager_->InitRequiredPipes();
    eventEntry->NotifyServiceReady();
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void)argc;
    (void)argv;
    OHOS::AudioStandard::Init();
    return 0;
}
