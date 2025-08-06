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
#include "audio_effect_volume.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 65;
static int32_t NUM_2 = 2;
const int32_t SESSIONID = 12345;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
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

vector<AudioSamplingRate> AudioSamplingRateVec = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_11025,
    SAMPLE_RATE_12000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_22050,
    SAMPLE_RATE_24000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
    SAMPLE_RATE_64000,
    SAMPLE_RATE_88200,
    SAMPLE_RATE_96000,
    SAMPLE_RATE_176400,
    SAMPLE_RATE_192000,
};

vector<DeviceRole> DeviceRoleVec = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
};

const vector<AudioStreamType> AudioStreamTypeVec = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_ACCESSIBILITY,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

void FilterSourceOutputsFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    int32_t sessionId = GetData<int32_t>();
    audioDeviceCommon.FilterSourceOutputs(sessionId);
}

void IsRingerOrAlarmerDualDevicesRangeFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    if (DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    InternalDeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
}

void IsRingOverPlaybackFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    AudioMode mode = static_cast<AudioMode>(GetData<uint8_t>() % modeCount);
    int32_t rendererStateCount =
        static_cast<int32_t>(RendererState::RENDERER_PAUSED - RendererState::RENDERER_INVALID) + 1;
    RendererState state = static_cast<RendererState>(GetData<int32_t>() % rendererStateCount- 1);
    audioDeviceCommon.IsRingOverPlayback(mode, state);
}

void GetPreferredInputDeviceDescInnerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);
    if (DeviceTypeVec.size() == 0 || StreamUsageVec.size() == 0 || SourceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();
    audioDeviceCommon.Init(audioPolicyServerHandler);
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    rendererInfo.streamUsage = StreamUsageVec[streamUsageCount];
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");
    AudioCapturerInfo captureInfo;
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    captureInfo.sourceType = SourceTypeVec[sourceTypeCount];
    std::string networkId = "LocalDevice";
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, networkId);
    audioDeviceCommon.DeInit();
}

void GetPreferredInputStreamTypeInnerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    if (DeviceTypeVec.size() == 0 || AudioSamplingRateVec.size() == 0 || SourceTypeVec.size() == 0) {
        return;
    }
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    int32_t flags = GetData<int32_t>();
    std::string networkId = "abc";
    uint32_t samplingRateCount = GetData<uint32_t>() % AudioSamplingRateVec.size();
    AudioSamplingRate samplingRate = AudioSamplingRateVec[samplingRateCount];
    audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType, deviceType, flags, networkId, samplingRate);
}

void UpdateDeviceInfoFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = GetData<DeviceType>();
    bool hasBTPermission = GetData<uint32_t>() % NUM_2;
    bool hasSystemPermission = GetData<uint32_t>() % NUM_2;
    BluetoothOffloadState state = NO_A2DP_DEVICE;
    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, std::make_shared<AudioDeviceDescriptor>(),
        hasBTPermission, hasSystemPermission);
}

void UpdateConnectedDevicesWhenDisconnectingFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    if (DeviceTypeVec.size() == 0 || DeviceRoleVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    updatedDesc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    updatedDesc.deviceRole_ = DeviceRoleVec[roleCount];
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptorSptr->deviceType_ = DeviceTypeVec[deviceTypeCount];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, audioDeviceDescriptorSptrVector);
}

void UpdateDualToneStateFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool enable = GetData<uint32_t>() % NUM_2;
    int32_t sessionId = GetData<int32_t>();
    audioDeviceCommon.UpdateDualToneState(enable, sessionId);
}

void IsFastFromA2dpToA2dpFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    if (rendererChangeInfo == nullptr || desc == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = 0;
    desc->deviceId_ = GetData<uint32_t>() % NUM_2;
    audioDeviceCommon.IsFastFromA2dpToA2dp(desc, rendererChangeInfo, reason);
}

void SetDeviceConnectedFlagWhenFetchOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.SetDeviceConnectedFlagWhenFetchOutputDevice();
}

void FetchOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.FetchOutputDevice(rendererChangeInfos, reason);
}

void GetDeviceDescriptorInnerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    VolumeUtils::SetPCVolumeEnable(true);
    audioDeviceCommon.isFirstScreenOn_ = GetData<uint32_t>() % NUM_2;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        audioDeviceCommon.GetDeviceDescriptorInner(rendererChangeInfo);
    if (rendererChangeInfo == nullptr || StreamUsageVec.size() == 0) {
        return;
    }
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    rendererChangeInfo->rendererInfo.streamUsage = StreamUsageVec[streamUsageCount];
    rendererChangeInfo->clientUID = 0;
    audioDeviceCommon.GetDeviceDescriptorInner(rendererChangeInfo);
}

void FetchOutputEndFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = GetData<uint32_t>() % NUM_2;
    int32_t runningStreamCount = GetData<int32_t>();
    audioDeviceCommon.FetchOutputEnd(isUpdateActiveDevice, runningStreamCount, AudioStreamDeviceChangeReason::UNKNOWN);
}

void FetchOutputDeviceWhenNoRunningStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.FetchOutputDeviceWhenNoRunningStream(AudioStreamDeviceChangeReason::UNKNOWN);
}

void HandleDeviceChangeForFetchOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (desc == nullptr || DeviceTypeVec.size() == 0 || rendererChangeInfo == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    desc->networkId_ = "LocalDevice";
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = GetData<ConnectState>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = desc->deviceType_;
    rendererChangeInfo->outputDeviceInfo.networkId_ = desc->networkId_;
    rendererChangeInfo->outputDeviceInfo.macAddress_ = desc->macAddress_;
    rendererChangeInfo->outputDeviceInfo.connectState_ = desc->connectState_;
    audioDeviceCommon.HandleDeviceChangeForFetchOutputDevice(desc, rendererChangeInfo,
        AudioStreamDeviceChangeReason::UNKNOWN);
}

void MuteSinkForSwitchGeneralDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    if (audioDeviceDescriptorUniqueptr == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptorUniqueptr->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchGeneralDevice(rendererChangeInfo, audioDeviceDescriptorUniqueptrVector, reason);
}

void MuteSinkForSwitchBluetoothDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    if (audioDeviceDescriptorUniqueptr == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptorUniqueptr->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
}

void SetVoiceCallMuteForSwitchDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.SetVoiceCallMuteForSwitchDevice();
}

void IsRendererStreamRunningFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (StreamUsageVec.size() == 0) {
        return;
    }
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    rendererChangeInfo->rendererInfo.streamUsage = StreamUsageVec[streamUsageCount];
    int32_t audioSceneCount = static_cast<int32_t>(AudioScene::AUDIO_SCENE_MAX - AudioScene::AUDIO_SCENE_INVALID) + 1;
    AudioScene audioScene = static_cast<AudioScene>(GetData<uint8_t>() % audioSceneCount - 1);
    audioDeviceCommon.audioSceneManager_.SetAudioScenePre(audioScene);
    audioDeviceCommon.IsRendererStreamRunning(rendererChangeInfo);
}

void ActivateA2dpDeviceWhenDescEnabledFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return;
    }
    desc->isEnable_ = GetData<uint32_t>() % NUM_2;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.ActivateA2dpDeviceWhenDescEnabled(desc, rendererChangeInfos, reason);
}

void ActivateA2dpDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.ActivateA2dpDevice(desc, rendererChangeInfos, reason);
}

void HandleScoOutputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.HandleScoOutputDeviceFetched(desc, rendererChangeInfos, reason);
}

void NotifyRecreateRendererStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    if (rendererChangeInfo == nullptr || desc == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.NotifyRecreateRendererStream(desc, rendererChangeInfo, reason);
}

void NeedRehandleA2DPDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.NeedRehandleA2DPDevice(desc);
}

void MoveToNewOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    if (rendererChangeInfo == nullptr || DeviceTypeVec.size() == 0 || DeviceRoleVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    rendererChangeInfo->outputDeviceInfo.macAddress_ = "";
    rendererChangeInfo->outputDeviceInfo.networkId_ = "";
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    rendererChangeInfo->outputDeviceInfo.deviceRole_ = DeviceRoleVec[roleCount];
    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    if (outputdevice == nullptr) {
        return;
    }
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    outputdevice->deviceType_ = DeviceTypeVec[deviceTypeCount];
    outputdevice->macAddress_ = "";
    outputdevice->networkId_ = "";
    roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    outputdevice->deviceRole_ = DeviceRoleVec[roleCount];
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    std::vector<SinkInput> sinkInputs;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::OVERRODE;
    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(true);
    audioDeviceCommon.MoveToNewOutputDevice(rendererChangeInfo, outputDevices, sinkInputs, reason);
    audioDeviceCommon.audioConfigManager_.GetUpdateRouteSupport();
}

void MuteSinkPortFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::vector<std::string> oldSinknameList = {"", "Offload_Speaker"};
    if (oldSinknameList.size() == 0) {
        return;
    }
    uint32_t oldSinknameCount = GetData<uint32_t>() % oldSinknameList.size();
    std::string oldSinkname = oldSinknameList[oldSinknameCount];
    std::vector<std::string> newSinkNameList = {"", "Offload_Speaker"};
    if (newSinkNameList.size() == 0) {
        return;
    }
    uint32_t newSinkNameCount = GetData<uint32_t>() % newSinkNameList.size();
    std::string newSinkName = newSinkNameList[newSinkNameCount];
    int32_t reasonCount = static_cast<int32_t>(AudioStreamDeviceChangeReason::OVERRODE) + 1;
    AudioStreamDeviceChangeReasonExt reason =
        static_cast<AudioStreamDeviceChangeReason>(GetData<uint8_t>() % reasonCount);
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);
    audioDeviceCommon.audioDeviceManager_.ExistsByType(DEVICE_TYPE_DP);
    audioDeviceCommon.audioDeviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_DP, "card=0;port=0");
}

void TriggerRecreateRendererStreamCallbackFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    int32_t callerPid = 0;
    int32_t sessionId = 0;
    int32_t streamFlag = 0;
    int32_t reasonCount = static_cast<int32_t>(AudioStreamDeviceChangeReason::OVERRODE) + 1;
    AudioStreamDeviceChangeReasonExt reason =
        static_cast<AudioStreamDeviceChangeReason>(GetData<uint8_t>() % reasonCount);
    audioDeviceCommon.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    audioDeviceCommon.TriggerRecreateRendererStreamCallback(callerPid, sessionId, streamFlag, reason);
}

void IsDualStreamWhenRingDualFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    if (AudioStreamTypeVec.size() == 0) {
        return;
    }
    uint32_t index = GetData<uint32_t>() % AudioStreamTypeVec.size();
    AudioStreamType streamType = AudioStreamTypeVec[index];
    audioDeviceCommon.IsDualStreamWhenRingDual(streamType);
}

void UpdateRouteFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    if (rendererChangeInfo == nullptr || DeviceTypeVec.size() == 0 || StreamUsageVec.size() == 0) {
        return;
    }
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    rendererChangeInfo->rendererInfo.streamUsage = StreamUsageVec[streamUsageCount];

    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    if (outputdevice == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    outputdevice->deviceType_ = DeviceTypeVec[deviceTypeCount];
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    VolumeUtils::SetPCVolumeEnable(GetData<uint32_t>() % NUM_2);
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
}

void ResetOffloadAndMchModeFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> outputDevice = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::string> networkIdList = {"", "LocalDevice"};
    if (outputDevice == nullptr || DeviceTypeVec.size() == 0 || networkIdList.size() == 0) {
        return;
    }
    uint32_t networkIdCount = GetData<uint32_t>() % networkIdList.size();
    outputDevice->networkId_ = networkIdList[networkIdCount];
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    outputDevice->deviceType_ = DeviceTypeVec[deviceTypeCount];
    outputDevices.push_back(std::move(outputDevice));
    audioDeviceCommon.ResetOffloadAndMchMode(rendererChangeInfo, outputDevices);
}

void JudgeIfLoadMchModuleFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.JudgeIfLoadMchModule();
    AudioIOHandle moduleId = 0;
    std::string moduleName = "MCH_Speaker";
    audioDeviceCommon.audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    audioDeviceCommon.JudgeIfLoadMchModule();
}

void FetchStreamForA2dpMchStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    descs.push_back(std::move(desc));
    audioDeviceCommon.FetchStreamForA2dpMchStream(rendererChangeInfo, descs);
}

void FetchStreamForSpkMchStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    descs.push_back(std::move(desc));
    audioDeviceCommon.FetchStreamForSpkMchStream(rendererChangeInfo, descs);
}

void IsRingDualToneOnPrimarySpeakerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    descs.push_back(std::make_shared<AudioDeviceDescriptor>());
    descs.push_back(std::make_shared<AudioDeviceDescriptor>());
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    if (DeviceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    descs.front()->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    descs.back()->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    descs.front()->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    descs.back()->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
}

void ClearRingMuteWhenCallStartFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool pre = GetData<uint32_t>() % NUM_2;
    bool after = GetData<uint32_t>() % NUM_2;
    audioDeviceCommon.ClearRingMuteWhenCallStart(pre, after);
}

void HandleDeviceChangeForFetchInputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr || capturerChangeInfo == nullptr || DeviceTypeVec.size() == 0 || DeviceRoleVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    desc->networkId_ = "";
    capturerChangeInfo->inputDeviceInfo.networkId_ = "";
    desc->macAddress_ = "";
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "";
    int32_t connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    desc->connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    capturerChangeInfo->inputDeviceInfo.connectState_ =
        static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    desc->deviceRole_ = DeviceRoleVec[roleCount];
    roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    capturerChangeInfo->inputDeviceInfo.deviceRole_ = DeviceRoleVec[roleCount];
    audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    desc->connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
}

void HandleBluetoothInputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr || DeviceTypeVec.size() == 0 || SourceTypeVec.size() == 0) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    std::shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    captureChangeInfos.push_back(std::move(captureChangeInfo));
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    audioDeviceCommon.HandleBluetoothInputDeviceFetched(desc, captureChangeInfos, sourceType);
}

void NotifyRecreateCapturerStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = true;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    if (capturerChangeInfo == nullptr) {
        return;
    }
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    capturerChangeInfo->capturerInfo.originalFlag = AUDIO_FLAG_MMAP;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(deviceDescriptor);
    std::vector<std::string> networkIdList = {"test", "LocalDevice"};
    if (networkIdList.size() == 0) {
        return;
    }
    uint32_t networkIdCount = GetData<uint32_t>() % networkIdList.size();
    capturerChangeInfo->inputDeviceInfo.networkId_ = networkIdList[networkIdCount];
    audioDeviceCommon.NotifyRecreateCapturerStream(isUpdateActiveDevice, capturerChangeInfo, reason);
}

void MoveToRemoteInputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceOutput sourceOutput;
    std::vector<SourceOutput> sourceOutputs;
    sourceOutputs.push_back(sourceOutput);
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceCommon.isOpenRemoteDevice = GetData<uint32_t>() % NUM_2;
    audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDeviceDescriptor);
}

void ScoInputDeviceFetchedForRecongnitionFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool handleFlag = GetData<uint32_t>() % NUM_2;
    std::string address = "";
    int32_t connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    ConnectState connectState = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
}

void CheckAndNotifyUserSelectedDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceCommon.CheckAndNotifyUserSelectedDevice(desc);
    audioDeviceCommon.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
}

void HasLowLatencyCapabilityFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    bool isRemote = GetData<uint32_t>() % NUM_2;
    audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
}

void GetSpatialDeviceTypeFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::string macAddress = "F0-FA-C7-8C-46-01";
    audioDeviceCommon.GetSpatialDeviceType(macAddress);

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.macAddress_ = "F0-FA-C7-8C-46-01";
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceDescriptor.deviceType_ = DeviceTypeVec[deviceTypeCount];
    shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();
    audioDeviceCommon.Init(audioPolicyServerHandler);
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);
    audioDeviceCommon.GetSpatialDeviceType(macAddress);
    audioDeviceCommon.DeInit();
}

void IsDeviceConnectedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceCommon.IsDeviceConnected(audioDeviceDescriptorSptr);
}

void IsSameDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceDescriptor deviceInfo;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    desc->networkId_ = "";
    deviceInfo.networkId_ = "";
    desc->macAddress_ = "";
    deviceInfo.macAddress_ = "";
    int32_t connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    desc->connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    deviceInfo.connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    desc->deviceRole_ = DeviceRoleVec[roleCount];
    roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    deviceInfo.deviceRole_ = DeviceRoleVec[roleCount];
    audioDeviceCommon.IsSameDevice(desc, deviceInfo);
}

void GetSourceOutputsFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.GetSourceOutputs();
}

void ClientDiedDisconnectScoNormalFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.ClientDiedDisconnectScoNormal();
}

void ClientDiedDisconnectScoRecognitionFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.ClientDiedDisconnectScoRecognition();
}

void GetA2dpModuleInfoFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioModuleInfo moduleInfo;
    AudioStreamInfo audioStreamInfo;
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    audioDeviceCommon.GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
}

void LoadA2dpModuleFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioStreamInfo audioStreamInfo;
    std::string networkID = "";
    std::string sinkName = "";
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    ClassType classType = GetData<ClassType>();
    AudioModuleInfo moduleInfo;
    std::list<AudioModuleInfo> moduleInfoList;
    moduleInfoList.push_back(moduleInfo);
    audioDeviceCommon.audioConfigManager_.deviceClassInfo_.insert({classType, moduleInfoList});
    audioDeviceCommon.LoadA2dpModule(deviceType, audioStreamInfo, networkID, sinkName, sourceType);
    audioDeviceCommon.audioConfigManager_.deviceClassInfo_.clear();
}

void ReloadA2dpAudioPortFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioModuleInfo moduleInfo;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioStreamInfo audioStreamInfo;
    std::string networkID = "";
    std::string sinkName = "";
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    audioDeviceCommon.ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkID, sinkName, sourceType);
}

void SwitchActiveA2dpDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->macAddress_ = Bluetooth::AudioA2dpManager::GetActiveA2dpDevice();
    deviceDescriptor->deviceName_ = "TestA2dpDevice";
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceDescriptor->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.audioA2dpDevice_.connectedA2dpDeviceMap_[deviceDescriptor->macAddress_] = A2dpDeviceConfigInfo();

    audioDeviceCommon.audioIOHandleMap_.IOHandles_[BLUETOOTH_SPEAKER] = GetData<uint32_t>();
    audioDeviceCommon.SwitchActiveA2dpDevice(deviceDescriptor);
}

void RingToneVoiceControlFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    InternalDeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.RingToneVoiceControl(deviceType);
}

void SetFirstScreenOnFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.SetFirstScreenOn();
}

void SetVirtualCallFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    pid_t uid = GetData<pid_t>();
    bool isVirtual = GetData<uint32_t>() % NUM_2;
    audioDeviceCommon.SetVirtualCall(uid, isVirtual);
}

void SetHeadsetUnpluggedToSpkOrEpFlagFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    InternalDeviceType oldDeviceType = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    InternalDeviceType newDeviceType = DeviceTypeVec[deviceTypeCount];
    audioDeviceCommon.SetHeadsetUnpluggedToSpkOrEpFlag(oldDeviceType, newDeviceType);
}

void WriteInputRouteChangeEventFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.WriteInputRouteChangeEvent(desc, reason);
}

void MoveToNewInputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    if (capturerChangeInfo == nullptr) {
        return;
    }
    capturerChangeInfo->sessionId = SESSIONID;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "00:11:22:33:44:55";
    capturerChangeInfo->inputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> inputDevice = std::make_shared<AudioDeviceDescriptor>();
    if (inputDevice == nullptr) {
        return;
    }
    inputDevice->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    inputDevice->macAddress_ = "66:77:88:99:AA:BB";
    inputDevice->networkId_ = GetData<int32_t>() % NUM_2 == 0 ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;

    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(GetData<uint32_t>() % NUM_2);
    audioDeviceCommon.MoveToNewInputDevice(capturerChangeInfo, inputDevice);
}

void BluetoothScoFetchFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->networkId_ = GetData<uint32_t>() % NUM_2 == 0 ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    capturerChangeInfo->capturerInfo.sourceType = SourceTypeVec[sourceTypeCount];
    capturerChangeInfos.push_back(capturerChangeInfo);

    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(*desc);
    sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    audioDeviceCommon.BluetoothScoFetch(desc, capturerChangeInfos, sourceType);
    audioDeviceCommon.audioActiveDevice_.GetCurrentInputDevice();
}

void HandleA2dpInputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    audioDeviceCommon.HandleA2dpInputDeviceFetched(desc, sourceType);
}

void TriggerRecreateCapturerStreamCallbackFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    int32_t streamFlag = GetData<int32_t>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();
    audioDeviceCommon.Init(audioPolicyServerHandler);
    bool isDeInit = GetData<bool>();
    if (isDeInit) {
        audioDeviceCommon.DeInit();
    }
    audioDeviceCommon.TriggerRecreateCapturerStreamCallback(capturerChangeInfo, streamFlag, reason);
    audioDeviceCommon.DeInit();
}

void HandleScoInputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    audioDeviceCommon.HandleScoInputDeviceFetched(audioDeviceDescriptorSptr, captureChangeInfos);
}

void AudioDeviceCommonOpenRemoteAudioDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::string networkId = "testNetworkId";
    DeviceRole deviceRole = GetData<DeviceRole>();
    DeviceType deviceType = GetData<DeviceType>();
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceCommon.OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
}

void AudioDeviceCommonIsSameDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.networkId_ = "testNetworkId";
    deviceDesc.deviceType_ = GetData<DeviceType>();
    deviceDesc.macAddress_ = "00:11:22:33:44:55";
    deviceDesc.connectState_ = GetData<ConnectState>();
    deviceDesc.deviceRole_ = GetData<DeviceRole>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(deviceDesc);
    if (desc == nullptr) {
        return;
    }
    bool isNoClear = GetData<bool>();
    if (!isNoClear) {
        desc->networkId_.clear();
    }

    audioDeviceCommon.IsSameDevice(desc, deviceDesc);
}

void AudioDeviceCommonDeviceParamsCheckFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    DeviceRole targetRole = GetData<DeviceRole>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = GetData<DeviceType>();
    audioDeviceDescriptor->deviceRole_ = GetData<DeviceRole>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(audioDeviceDescriptor);

    audioDeviceCommon.DeviceParamsCheck(targetRole, audioDeviceDescriptors);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    FilterSourceOutputsFuzzTest,
    IsRingerOrAlarmerDualDevicesRangeFuzzTest,
    IsRingOverPlaybackFuzzTest,
    GetPreferredInputDeviceDescInnerFuzzTest,
    GetPreferredInputStreamTypeInnerFuzzTest,
    UpdateDeviceInfoFuzzTest,
    UpdateConnectedDevicesWhenDisconnectingFuzzTest,
    UpdateDualToneStateFuzzTest,
    IsFastFromA2dpToA2dpFuzzTest,
    SetDeviceConnectedFlagWhenFetchOutputDeviceFuzzTest,
    FetchOutputDeviceFuzzTest,
    GetDeviceDescriptorInnerFuzzTest,
    FetchOutputEndFuzzTest,
    FetchOutputDeviceWhenNoRunningStreamFuzzTest,
    HandleDeviceChangeForFetchOutputDeviceFuzzTest,
    MuteSinkForSwitchGeneralDeviceFuzzTest,
    MuteSinkForSwitchBluetoothDeviceFuzzTest,
    SetVoiceCallMuteForSwitchDeviceFuzzTest,
    IsRendererStreamRunningFuzzTest,
    ActivateA2dpDeviceWhenDescEnabledFuzzTest,
    ActivateA2dpDeviceFuzzTest,
    HandleScoOutputDeviceFetchedFuzzTest,
    NotifyRecreateRendererStreamFuzzTest,
    NeedRehandleA2DPDeviceFuzzTest,
    MoveToNewOutputDeviceFuzzTest,
    MuteSinkPortFuzzTest,
    TriggerRecreateRendererStreamCallbackFuzzTest,
    IsDualStreamWhenRingDualFuzzTest,
    UpdateRouteFuzzTest,
    ResetOffloadAndMchModeFuzzTest,
    JudgeIfLoadMchModuleFuzzTest,
    FetchStreamForA2dpMchStreamFuzzTest,
    FetchStreamForSpkMchStreamFuzzTest,
    IsRingDualToneOnPrimarySpeakerFuzzTest,
    ClearRingMuteWhenCallStartFuzzTest,
    HandleDeviceChangeForFetchInputDeviceFuzzTest,
    HandleBluetoothInputDeviceFetchedFuzzTest,
    NotifyRecreateCapturerStreamFuzzTest,
    MoveToRemoteInputDeviceFuzzTest,
    ScoInputDeviceFetchedForRecongnitionFuzzTest,
    CheckAndNotifyUserSelectedDeviceFuzzTest,
    HasLowLatencyCapabilityFuzzTest,
    GetSpatialDeviceTypeFuzzTest,
    IsDeviceConnectedFuzzTest,
    IsSameDeviceFuzzTest,
    GetSourceOutputsFuzzTest,
    ClientDiedDisconnectScoNormalFuzzTest,
    ClientDiedDisconnectScoRecognitionFuzzTest,
    GetA2dpModuleInfoFuzzTest,
    LoadA2dpModuleFuzzTest,
    ReloadA2dpAudioPortFuzzTest,
    SwitchActiveA2dpDeviceFuzzTest,
    RingToneVoiceControlFuzzTest,
    SetFirstScreenOnFuzzTest,
    SetVirtualCallFuzzTest,
    SetHeadsetUnpluggedToSpkOrEpFlagFuzzTest,
    WriteInputRouteChangeEventFuzzTest,
    MoveToNewInputDeviceFuzzTest,
    BluetoothScoFetchFuzzTest,
    HandleA2dpInputDeviceFetchedFuzzTest,
    TriggerRecreateCapturerStreamCallbackFuzzTest,
    HandleScoInputDeviceFetchedFuzzTest,
    AudioDeviceCommonOpenRemoteAudioDeviceFuzzTest,
    AudioDeviceCommonIsSameDeviceFuzzTest,
    AudioDeviceCommonDeviceParamsCheckFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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

    return;
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
