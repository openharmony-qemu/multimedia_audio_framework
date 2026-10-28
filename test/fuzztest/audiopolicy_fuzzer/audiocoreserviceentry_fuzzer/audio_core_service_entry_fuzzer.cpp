/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "stream_flag_info.h"
#include "audio_limiter_manager.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const uint32_t MAX_SESSION_ID = 100000;
const int32_t MIN_TEST_UID = 0;
const int32_t MAX_TEST_UID = 10000;
const int32_t MAX_API_VERSION = 20;
const uint32_t MIN_CUSTOM_SAMPLE_RATE = 8000;
const uint32_t MAX_CUSTOM_SAMPLE_RATE = 192000;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
shared_ptr<AudioCoreService> audioCoreService;
shared_ptr<AudioCoreService::EventEntry> eventEntry;
typedef void (*TestFuncs)();
void Init();

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
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
    STREAM_USAGE_ACCESSIBILITY,
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

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

const vector<DeviceType> OutputDeviceTypeVec = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_NEARLINK,
};

const vector<DeviceType> InputDeviceTypeVec = {
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

const vector<DeviceRole> DeviceRoleVec = {
    INPUT_DEVICE,
    OUTPUT_DEVICE,
};

const vector<DeviceFlag> DeviceFlagVec = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
};

const vector<AudioDeviceUsage> AudioDeviceUsageVec = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

const vector<ContentType> ContentTypeVec = {
    CONTENT_TYPE_MUSIC,
    CONTENT_TYPE_SPEECH,
    CONTENT_TYPE_MOVIE,
    CONTENT_TYPE_SONIFICATION,
    CONTENT_TYPE_RINGTONE,
};

const vector<SourceType> SourceTypeVec = {
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
};

const vector<AudioSamplingRate> SamplingRateVec = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const vector<AudioEncodingType> EncodingVec = {
    ENCODING_PCM,
    ENCODING_AUDIOVIVID,
    ENCODING_EAC3,
};

const vector<AudioSampleFormat> SampleFormatVec = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const vector<AudioChannel> ChannelVec = {
    MONO,
    STEREO,
    CHANNEL_4,
    CHANNEL_6,
};

const vector<AudioChannelLayout> ChannelLayoutVec = {
    CH_LAYOUT_MONO,
    CH_LAYOUT_STEREO,
    CH_LAYOUT_QUAD,
    CH_LAYOUT_5POINT1,
};

const vector<AudioFlag> AudioFlagVec = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
};

const vector<AudioMode> AudioModeVec = {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD,
};

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

AudioStreamInfo MakeStreamInfo(FuzzedDataProvider& fdp)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = PickValue(fdp, SamplingRateVec);
    streamInfo.encoding = PickValue(fdp, EncodingVec);
    streamInfo.format = PickValue(fdp, SampleFormatVec);
    streamInfo.channels = PickValue(fdp, ChannelVec);
    streamInfo.channelLayout = PickValue(fdp, ChannelLayoutVec);
    streamInfo.customSampleRate = fdp.ConsumeBool() ?
        fdp.ConsumeIntegralInRange<uint32_t>(MIN_CUSTOM_SAMPLE_RATE, MAX_CUSTOM_SAMPLE_RATE) : 0;
    return streamInfo;
}

std::shared_ptr<AudioDeviceDescriptor> MakeDeviceDescriptor(FuzzedDataProvider& fdp, DeviceRole role)
{
    const auto& deviceTypes = (role == INPUT_DEVICE) ? InputDeviceTypeVec : OutputDeviceTypeVec;
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceName_ = ConsumeString(fdp, "audio_entry_fuzz_device");
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
    streamDesc->audioMode_ = isInput ? AUDIO_MODE_RECORD : AUDIO_MODE_PLAYBACK;
    streamDesc->audioFlag_ = PickValue(fdp, AudioFlagVec);
    streamDesc->routeFlag_ = PickValue(fdp, AudioFlagVec);
    streamDesc->sessionId_ = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    streamDesc->callerUid_ = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    streamDesc->callerPid_ = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    streamDesc->appInfo_.appUid = streamDesc->callerUid_;
    streamDesc->appInfo_.appPid = streamDesc->callerPid_;
    streamDesc->bundleName_ = ConsumeString(fdp, "audio_core_service_entry_fuzzer");
    streamDesc->rendererInfo_.contentType = PickValue(fdp, ContentTypeVec);
    streamDesc->rendererInfo_.streamUsage = PickValue(fdp, StreamUsageVec);
    streamDesc->rendererInfo_.rendererFlags = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    streamDesc->rendererInfo_.originalFlag = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    streamDesc->rendererInfo_.samplingRate = streamDesc->streamInfo_.samplingRate;
    streamDesc->rendererInfo_.format = streamDesc->streamInfo_.format;
    streamDesc->capturerInfo_.sourceType = PickValue(fdp, SourceTypeVec);
    streamDesc->capturerInfo_.capturerFlags = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    streamDesc->capturerInfo_.samplingRate = streamDesc->streamInfo_.samplingRate;
    auto inputDevice = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    streamDesc->preferredInputDevice = *inputDevice;
    streamDesc->newDeviceDescs_.push_back(MakeDeviceDescriptor(fdp, isInput ? INPUT_DEVICE : OUTPUT_DEVICE));
    return streamDesc;
}

void UpdateSessionOperationFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation =
        static_cast<SessionOperation>(fdp.ConsumeIntegralInRange<int32_t>(0, operationCount - 1));
    eventEntry->UpdateSessionOperation(sessionId, operation);
}

void OnServiceDisconnectedFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex =
        static_cast<AudioServiceIndex>(fdp.ConsumeIntegralInRange<int32_t>(0, serviceIndexCount - 1));
    eventEntry->OnServiceDisconnected(serviceIndex);
}

void OnServiceConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex =
        static_cast<AudioServiceIndex>(fdp.ConsumeIntegralInRange<int32_t>(0, serviceIndexCount - 1));
    eventEntry->OnServiceConnected(serviceIndex);
}

void CreateCapturerClientFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, true);
    uint32_t audioFlag = PickValue(fdp, AudioFlagVec);
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    eventEntry->CreateCapturerClient(streamDesc, audioFlag, sessionId);
}

void SetDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, OutputDeviceTypeVec);
    uint32_t sessionID = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    StreamUsage streamUsage = PickValue(fdp, StreamUsageVec);
    bool isRunning = fdp.ConsumeBool();
    eventEntry->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
}

void CreateRendererClientFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);
    StreamFlagInfo flagInfo;
    flagInfo.audioFlag = PickValue(fdp, AudioFlagVec);
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    eventEntry->CreateRendererClient(streamDesc, flagInfo, sessionId, networkId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    AudioDeviceDescriptor deviceInfo;
    AudioStreamInfo info;
    bool isUltraFast = false;
    auto ret = eventEntry->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, isUltraFast);
}

void GetModuleNameBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    eventEntry->GetModuleNameBySessionId(sessionId);
}

void GenerateSessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto ret = eventEntry->GenerateSessionId();
}

void SetAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t audioSceneStart = static_cast<int32_t>(AudioScene::AUDIO_SCENE_INVALID);
    int32_t audioSceneEnd = static_cast<int32_t>(AudioScene::AUDIO_SCENE_MAX);
    AudioScene audioScene = static_cast<AudioScene>(fdp.ConsumeIntegralInRange<int32_t>(audioSceneStart,
        audioSceneEnd));
    eventEntry->SetAudioScene(audioScene);
}

void OnDeviceInfoUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    AudioDeviceDescriptor desc(*MakeDeviceDescriptor(fdp, deviceRole));
    desc.isEnable_ = fdp.ConsumeBool();
    int32_t commandStart = static_cast<int32_t>(DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    int32_t commandEnd = static_cast<int32_t>(DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE);
    DeviceInfoUpdateCommand command =
        static_cast<DeviceInfoUpdateCommand>(fdp.ConsumeIntegralInRange<int32_t>(commandStart, commandEnd));
    eventEntry->OnDeviceInfoUpdated(desc, command);
}

void OnDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    AudioDeviceDescriptor desc(*MakeDeviceDescriptor(fdp, deviceRole));
    bool isConnected = fdp.ConsumeBool();
    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

void OnPnpDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    AudioDeviceDescriptor desc(*MakeDeviceDescriptor(fdp, deviceRole));
    bool isConnected = fdp.ConsumeBool();
    eventEntry->OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreServiceEventEntryLoadSplitModuleFuzzTest(FuzzedDataProvider& fdp)
{
    std::string splitArgs = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    eventEntry->LoadSplitModule(splitArgs, networkId);
}

void OnMicrophoneBlockedUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    int32_t statusCount = static_cast<int32_t>(DeviceBlockStatus::DEVICE_BLOCKED) + 1;
    DeviceBlockStatus status =
        static_cast<DeviceBlockStatus>(fdp.ConsumeIntegralInRange<int32_t>(0, statusCount - 1));
    eventEntry->OnMicrophoneBlockedUpdate(deviceType, status);
}


void AudioCoreServiceEventEntryReloadCaptureSessionFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation =
        static_cast<SessionOperation>(fdp.ConsumeIntegralInRange<int32_t>(0, operationCount - 1));
    eventEntry->ReloadCaptureSession(sessionId, operation);
}

void AudioCoreServiceEventEntryIsArmUsbDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    AudioDeviceDescriptor deviceDesc(*MakeDeviceDescriptor(fdp, deviceRole));
    eventEntry->IsArmUsbDevice(deviceDesc);
}

void AudioCoreServiceEventEntryOnDeviceConfigurationChangedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    std::string deviceName = ConsumeString(fdp, "audio_entry_fuzz_device");
    AudioStreamInfo streamInfo = MakeStreamInfo(fdp);
    eventEntry->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreServiceEventEntryOnForcedDeviceSelectedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = PickValue(fdp, OutputDeviceTypeVec);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    eventEntry->OnPrivacyDeviceSelected(devType, macAddress);
    eventEntry->OnForcedDeviceSelected(devType, macAddress);
     
    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
    auto devDesc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    devDesc->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    devDesc->deviceType_ = devType;
    devDesc->macAddress_ = macAddress;
    devDesc->deviceRole_ = OUTPUT_DEVICE;
    devMan.AddNewDevice(devDesc);
    auto devDesc2 = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    devDesc2->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    devDesc2->deviceType_ = PickValue(fdp, InputDeviceTypeVec);
    devDesc2->macAddress_ = macAddress;
    devDesc2->deviceRole_ = INPUT_DEVICE;
    devMan.AddNewDevice(devDesc2);
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
}

void AudioCoreServiceEventEntryGetDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag deviceFlag = PickValue(fdp, DeviceFlagVec);
    eventEntry->GetDevices(deviceFlag);
}

void AudioCoreServiceEventEntryGetPreferredOutputDeviceDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = PickValue(fdp, ContentTypeVec);
    rendererInfo.streamUsage = PickValue(fdp, StreamUsageVec);
    rendererInfo.rendererFlags = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    eventEntry->GetPreferredOutputDeviceDescriptors(rendererInfo, uid, networkId);
}

void AudioCoreServiceEventEntrySetDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = PickValue(fdp, OutputDeviceTypeVec);
    bool active = fdp.ConsumeBool();
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    eventEntry->SetDeviceActive(deviceType, active, uid);
}

void AudioCoreServiceEventEntryGetActiveBluetoothDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->GetActiveBluetoothDevice();
}

void AudioCoreServiceEventEntrySkipDeviceTypesFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->SkipDeviceTypes(fdp.ConsumeBool(), {PickValue(fdp, DeviceTypeVec)});
}

void AudioCoreServiceEventEntrySetCallDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = PickValue(fdp, OutputDeviceTypeVec);
    bool active = fdp.ConsumeBool();
    std::string address = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    eventEntry->SetCallDeviceActive(deviceType, active, address, uid);
}

void AudioCoreServiceEventEntryGetPreferredInputDeviceDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = PickValue(fdp, SourceTypeVec);
    capturerInfo.capturerFlags = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    int32_t uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    eventEntry->GetPreferredInputDeviceDescriptors(capturerInfo, uid, networkId);
}

void AudioCoreServiceEventEntryUpdateTrackerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioMode mode = PickValue(fdp, AudioModeVec);
    AudioStreamChangeInfo streamChangeInfo;
    eventEntry->UpdateTracker(mode, streamChangeInfo);
}

void AudioCoreServiceEventEntryRegisteredTrackerClientDiedFuzzTest(FuzzedDataProvider& fdp)
{
    pid_t uid = static_cast<pid_t>(fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID));
    pid_t pid = static_cast<pid_t>(fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID));
    eventEntry->RegisteredTrackerClientDied(uid, pid);
}

void AudioCoreServiceEventEntryRegisterTrackerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioMode mode = PickValue(fdp, AudioModeVec);
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_VERSION);
    eventEntry->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

void AudioCoreServiceEventEntryGetAvailableMicrophonesFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->GetAvailableMicrophones();
}

void AudioCoreServiceEventEntryOnReceiveUpdateDeviceNameEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    std::string deviceName = ConsumeString(fdp, "audio_entry_fuzz_device");
    eventEntry->OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
}

void AudioCoreServiceEventEntryGetAudioCapturerMicrophoneDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t sessionId = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_SESSION_ID);
    eventEntry->GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioCoreServiceEventEntrySelectInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    selectedDesc.push_back(audioDevDesc);
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN(audioCapturerFilter != nullptr);
    audioCapturerFilter->uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    audioCapturerFilter->capturerInfo.sourceType = PickValue(fdp, SourceTypeVec);
    audioCapturerFilter->capturerInfo.capturerFlags = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    eventEntry->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioCoreServiceEventEntryGetCurrentRendererChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool hasBTPermission = fdp.ConsumeBool();
    bool hasSystemPermission = fdp.ConsumeBool();
    eventEntry->GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission, hasSystemPermission);
}

void AudioCoreServiceEventEntrySelectOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    CHECK_AND_RETURN(audioRendererFilter != nullptr);
    audioRendererFilter->uid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    audioRendererFilter->rendererInfo.contentType = PickValue(fdp, ContentTypeVec);
    audioRendererFilter->rendererInfo.streamUsage = PickValue(fdp, StreamUsageVec);
    audioRendererFilter->rendererInfo.rendererFlags = static_cast<int32_t>(PickValue(fdp, AudioFlagVec));
    audioRendererFilter->streamId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    selectedDesc.push_back(audioDevDesc);
    eventEntry->SelectOutputDevice(audioRendererFilter, selectedDesc);
}


void AudioCoreServiceEventEntryGetCurrentCapturerChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasBTPermission = fdp.ConsumeBool();
    bool hasSystemPermission = fdp.ConsumeBool();
    eventEntry->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission, hasSystemPermission);
}

void AudioCoreServiceEventEntryOnCapturerSessionAddedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_SESSION_ID);
    SessionInfo sessionInfo;
    sessionInfo.sourceType = PickValue(fdp, SourceTypeVec);
    sessionInfo.rate = static_cast<uint32_t>(PickValue(fdp, SamplingRateVec));
    sessionInfo.channels = static_cast<uint32_t>(PickValue(fdp, ChannelVec));
    AudioStreamInfo streamInfo = MakeStreamInfo(fdp);
    eventEntry->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreServiceEventEntryNotifyRemoteRenderStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    std::string condition = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    eventEntry->NotifyRemoteRenderState(networkId, condition, value);
}

void AudioCoreServiceEventEntryOnCapturerSessionRemovedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_SESSION_ID);
    eventEntry->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreServiceEventEntryGetVolumeGroupInfosFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->GetVolumeGroupInfos();
}

void AudioCoreServiceEventEntryTriggerFetchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    eventEntry->TriggerFetchDevice(reason);
}

void AudioCoreServiceEventEntryExcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = PickValue(fdp, AudioDeviceUsageVec);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(MakeDeviceDescriptor(fdp, OUTPUT_DEVICE));
    eventEntry->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceEventEntryGetExcludedDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = PickValue(fdp, AudioDeviceUsageVec);
    eventEntry->GetExcludedDevices(audioDevUsage);
}

void AudioCoreServiceEventEntryUnexcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = PickValue(fdp, AudioDeviceUsageVec);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(MakeDeviceDescriptor(fdp, OUTPUT_DEVICE));
    eventEntry->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceEventEntrySetSessionDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, OutputDeviceTypeVec);
    int32_t callerPid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    eventEntry->SetSessionDefaultOutputDevice(callerPid, deviceType);
}

void AudioCoreServiceEventEntryGetSessionDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, OutputDeviceTypeVec);
    int32_t callerPid = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEST_UID, MAX_TEST_UID);
    eventEntry->GetSessionDefaultOutputDevice(callerPid, deviceType);
}

void Test(FuzzedDataProvider& fdp)
{
    if (audioCoreService == nullptr || eventEntry == nullptr) {
        Init();
    }
    auto func = fdp.PickValueInArray({
    UpdateSessionOperationFuzzTest, OnServiceConnectedFuzzTest,
    OnServiceDisconnectedFuzzTest, CreateRendererClientFuzzTest,
    CreateCapturerClientFuzzTest, SetDefaultOutputDeviceFuzzTest,
    GetModuleNameBySessionIdFuzzTest, GetProcessDeviceInfoBySessionIdFuzzTest,
    GenerateSessionIdFuzzTest, OnDeviceInfoUpdatedFuzzTest,
    SetAudioSceneFuzzTest, OnDeviceStatusUpdatedFuzzTest,
    OnMicrophoneBlockedUpdateFuzzTest, OnPnpDeviceStatusUpdatedFuzzTest,
    AudioCoreServiceEventEntryReloadCaptureSessionFuzzTest,
    AudioCoreServiceEventEntryLoadSplitModuleFuzzTest,
    AudioCoreServiceEventEntryOnDeviceConfigurationChangedFuzzTest,
    AudioCoreServiceEventEntryOnForcedDeviceSelectedFuzzTest,
    AudioCoreServiceEventEntryIsArmUsbDeviceFuzzTest,
    AudioCoreServiceEventEntryGetDevicesFuzzTest,
    AudioCoreServiceEventEntrySetDeviceActiveFuzzTest,
    AudioCoreServiceEventEntryGetPreferredOutputDeviceDescriptorsFuzzTest,
    AudioCoreServiceEventEntryGetPreferredInputDeviceDescriptorsFuzzTest,
    AudioCoreServiceEventEntryGetActiveBluetoothDeviceFuzzTest,
    AudioCoreServiceEventEntrySkipDeviceTypesFuzzTest,
    AudioCoreServiceEventEntrySetCallDeviceActiveFuzzTest,
    AudioCoreServiceEventEntryRegisterTrackerFuzzTest,
    AudioCoreServiceEventEntryUpdateTrackerFuzzTest,
    AudioCoreServiceEventEntryRegisteredTrackerClientDiedFuzzTest,
    AudioCoreServiceEventEntryGetAvailableMicrophonesFuzzTest,
    AudioCoreServiceEventEntryGetAudioCapturerMicrophoneDescriptorsFuzzTest,
    AudioCoreServiceEventEntryOnReceiveUpdateDeviceNameEventFuzzTest,
    AudioCoreServiceEventEntrySelectOutputDeviceFuzzTest,
    AudioCoreServiceEventEntrySelectInputDeviceFuzzTest,
    AudioCoreServiceEventEntryGetCurrentRendererChangeInfosFuzzTest,
    AudioCoreServiceEventEntryGetCurrentCapturerChangeInfosFuzzTest,
    AudioCoreServiceEventEntryNotifyRemoteRenderStateFuzzTest,
    AudioCoreServiceEventEntryOnCapturerSessionAddedFuzzTest,
    AudioCoreServiceEventEntryOnCapturerSessionRemovedFuzzTest,
    AudioCoreServiceEventEntryTriggerFetchDeviceFuzzTest,
    AudioCoreServiceEventEntryGetVolumeGroupInfosFuzzTest,
    AudioCoreServiceEventEntryExcludeOutputDevicesFuzzTest,
    AudioCoreServiceEventEntryUnexcludeOutputDevicesFuzzTest,
    AudioCoreServiceEventEntryGetExcludedDevicesFuzzTest,
    AudioCoreServiceEventEntrySetSessionDefaultOutputDeviceFuzzTest,
    AudioCoreServiceEventEntryGetSessionDefaultOutputDeviceFuzzTest,
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
