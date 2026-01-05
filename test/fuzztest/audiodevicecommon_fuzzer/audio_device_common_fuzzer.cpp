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
#include "bluetooth_host.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_count = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
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

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void FilterSourceOutputsFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    int32_t sessionId = GetData<int32_t>();
    audioDeviceCommon.FilterSourceOutputs(sessionId);
    OnStop();
}

void IsRingerOrAlarmerDualDevicesRangeFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    InternalDeviceType deviceType = GetData<DeviceType>();
    audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    OnStop();
}

void IsRingOverPlaybackFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioMode mode = GetData<AudioMode>();
    RendererState state = GetData<RendererState>();
    audioDeviceCommon.IsRingOverPlayback(mode, state);
    OnStop();
}

void GetPreferredInputDeviceDescInnerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);
    DeviceType deviceType = GetData<DeviceType>();
    shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();
    CHECK_AND_RETURN(audioPolicyServerHandler != nullptr);
    audioDeviceCommon.Init(audioPolicyServerHandler);
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = GetData<StreamUsage>();
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");
    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = GetData<SourceType>();
    std::string networkId = "LocalDevice";
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, networkId);
    audioDeviceCommon.DeInit();
    OnStop();
}

void GetPreferredInputStreamTypeInnerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = GetData<SourceType>();
    DeviceType deviceType = GetData<DeviceType>();
    int32_t flags = GetData<int32_t>();
    std::string networkId = "abc";
    AudioSamplingRate samplingRate = GetData<AudioSamplingRate>();
    audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType, deviceType, flags, networkId, samplingRate);
    OnStop();
}

void UpdateDeviceInfoFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = GetData<DeviceType>();
    bool hasBTPermission = GetData<bool>();
    bool hasSystemPermission = GetData<bool>();
    BluetoothOffloadState state = GetData<BluetoothOffloadState>();
    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptorSptr != nullptr);
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, audioDeviceDescriptorSptr, hasBTPermission, hasSystemPermission);
    OnStop();
}

void UpdateConnectedDevicesWhenDisconnectingFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = GetData<DeviceType>();
    updatedDesc.deviceRole_ = GetData<DeviceRole>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptorSptr != nullptr);
    audioDeviceDescriptorSptr->deviceType_ = GetData<DeviceType>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, audioDeviceDescriptorSptrVector);
    OnStop();
}

void UpdateDualToneStateFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool enable = GetData<bool>();
    int32_t sessionId = GetData<int32_t>();
    audioDeviceCommon.UpdateDualToneState(enable, sessionId);
    OnStop();
}

void IsFastFromA2dpToA2dpFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = GetData<DeviceType>();
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = GetData<bool>();
    desc->deviceId_ = GetData<bool>();
    audioDeviceCommon.IsFastFromA2dpToA2dp(desc, rendererChangeInfo, reason);
    OnStop();
}

void SetDeviceConnectedFlagWhenFetchOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.SetDeviceConnectedFlagWhenFetchOutputDevice();
}

void FetchOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
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
    audioDeviceCommon.isFirstScreenOn_ = GetData<bool>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        audioDeviceCommon.GetDeviceDescriptorInner(rendererChangeInfo);
    rendererChangeInfo->rendererInfo.streamUsage = GetData<StreamUsage>();
    rendererChangeInfo->clientUID = 0;
    audioDeviceCommon.GetDeviceDescriptorInner(rendererChangeInfo);
    OnStop();
}

void FetchOutputEndFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = GetData<bool>();
    int32_t runningStreamCount = GetData<int32_t>();
    audioDeviceCommon.FetchOutputEnd(isUpdateActiveDevice, runningStreamCount, AudioStreamDeviceChangeReason::UNKNOWN);
    OnStop();
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
    CHECK_AND_RETURN(desc != nullptr);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    desc->deviceType_ = GetData<DeviceType>();
    desc->networkId_ = "LocalDevice";
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = GetData<ConnectState>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = desc->deviceType_;
    rendererChangeInfo->outputDeviceInfo.networkId_ = desc->networkId_;
    rendererChangeInfo->outputDeviceInfo.macAddress_ = desc->macAddress_;
    rendererChangeInfo->outputDeviceInfo.connectState_ = desc->connectState_;
    audioDeviceCommon.HandleDeviceChangeForFetchOutputDevice(desc, rendererChangeInfo,
        AudioStreamDeviceChangeReason::UNKNOWN);
    OnStop();
}

void MuteSinkForSwitchGeneralDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptorUniqueptr != nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = GetData<DeviceType>();
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchGeneralDevice(rendererChangeInfo, audioDeviceDescriptorUniqueptrVector, reason);
    OnStop();
}

void MuteSinkForSwitchBluetoothDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptorUniqueptr != nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = GetData<DeviceType>();
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    OnStop();
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
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    rendererChangeInfo->rendererInfo.streamUsage = GetData<StreamUsage>();
    rendererChangeInfo->rendererState = GetData<RendererState>();
    rendererChangeInfo->prerunningState = GetData<bool>();
    audioDeviceCommon.IsRendererStreamRunning(rendererChangeInfo);
    OnStop();
}

void ActivateA2dpDeviceWhenDescEnabledFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->isEnable_ = GetData<bool>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.ActivateA2dpDeviceWhenDescEnabled(desc, rendererChangeInfos, reason);
    OnStop();
}

void ActivateA2dpDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.ActivateA2dpDevice(desc, rendererChangeInfos, reason);
}

void HandleScoOutputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.HandleScoOutputDeviceFetched(desc, rendererChangeInfos, reason);
}

void NotifyRecreateRendererStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = GetData<DeviceType>();
    desc->deviceType_ = GetData<DeviceType>();
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = GetData<DeviceType>();
    desc->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.NotifyRecreateRendererStream(desc, rendererChangeInfo, reason);
    OnStop();
}

void NeedRehandleA2DPDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.NeedRehandleA2DPDevice(desc);
    OnStop();
}

void MoveToNewOutputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    rendererChangeInfo->outputDeviceInfo.deviceType_ = GetData<DeviceType>();
    rendererChangeInfo->outputDeviceInfo.macAddress_ = "";
    rendererChangeInfo->outputDeviceInfo.networkId_ = "";
    rendererChangeInfo->outputDeviceInfo.deviceRole_ = GetData<DeviceRole>();
    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(outputdevice != nullptr);
    outputdevice->deviceType_ = GetData<DeviceType>();
    outputdevice->macAddress_ = "";
    outputdevice->networkId_ = "";
    outputdevice->deviceRole_ = GetData<DeviceRole>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    std::vector<SinkInput> sinkInputs;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::OVERRODE;
    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(true);
    audioDeviceCommon.MoveToNewOutputDevice(rendererChangeInfo, outputDevices, sinkInputs, reason);
    audioDeviceCommon.audioConfigManager_.GetUpdateRouteSupport();
    OnStop();
}

void MuteSinkPortFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::vector<std::string> oldSinknameList = {"", "Offload_Speaker"};
    uint32_t oldSinknameCount = GetData<uint32_t>() % oldSinknameList.size();
    std::string oldSinkname = oldSinknameList[oldSinknameCount];
    std::vector<std::string> newSinkNameList = {"", "Offload_Speaker"};
    uint32_t newSinkNameCount = GetData<uint32_t>() % newSinkNameList.size();
    std::string newSinkName = newSinkNameList[newSinkNameCount];
    AudioStreamDeviceChangeReasonExt reason = GetData<AudioStreamDeviceChangeReason>();
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);
    audioDeviceCommon.audioDeviceManager_.ExistsByType(DEVICE_TYPE_DP);
    audioDeviceCommon.audioDeviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_DP, "card=0;port=0");
    OnStop();
}

void TriggerRecreateRendererStreamCallbackFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    int32_t callerPid = 0;
    int32_t sessionId = 0;
    int32_t streamFlag = 0;
    AudioStreamDeviceChangeReasonExt reason = GetData<AudioStreamDeviceChangeReason>();
    audioDeviceCommon.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    CHECK_AND_RETURN(audioDeviceCommon.audioPolicyServerHandler_ != nullptr);
    audioDeviceCommon.TriggerRecreateRendererStreamCallback(callerPid, sessionId, streamFlag, reason);
    OnStop();
}

void IsDualStreamWhenRingDualFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioStreamType streamType = GetData<AudioStreamType>();
    audioDeviceCommon.IsDualStreamWhenRingDual(streamType);
    OnStop();
}

void UpdateRouteFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    CHECK_AND_RETURN(rendererChangeInfo != nullptr);
    rendererChangeInfo->rendererInfo.streamUsage = GetData<StreamUsage>();

    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(outputdevice != nullptr);
    outputdevice->deviceType_ = GetData<DeviceType>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    VolumeUtils::SetPCVolumeEnable(GetData<bool>());
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    OnStop();
}

void IsRingDualToneOnPrimarySpeakerFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    descs.push_back(std::make_shared<AudioDeviceDescriptor>());
    descs.push_back(std::make_shared<AudioDeviceDescriptor>());
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    descs.front()->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    descs.back()->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    descs.front()->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    descs.back()->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1);
    OnStop();
}

void ClearRingMuteWhenCallStartFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool pre = GetData<bool>();
    bool after = GetData<bool>();
    audioDeviceCommon.ClearRingMuteWhenCallStart(pre, after);
    OnStop();
}

void HandleDeviceChangeForFetchInputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    CHECK_AND_RETURN(capturerChangeInfo != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    desc->networkId_ = "";
    capturerChangeInfo->inputDeviceInfo.networkId_ = "";
    desc->macAddress_ = "";
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "";
    desc->connectState_ = GetData<ConnectState>();
    capturerChangeInfo->inputDeviceInfo.connectState_ = GetData<ConnectState>();
    desc->deviceType_ = GetData<DeviceType>();
    capturerChangeInfo->inputDeviceInfo.deviceType_ = GetData<DeviceType>();
    desc->deviceRole_ = GetData<DeviceRole>();
    capturerChangeInfo->inputDeviceInfo.deviceRole_ = GetData<DeviceRole>();
    audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    desc->deviceType_ = GetData<DeviceType>();
    desc->connectState_ = GetData<ConnectState>();
    audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    OnStop();
}

void HandleBluetoothInputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    std::shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    CHECK_AND_RETURN(captureChangeInfo != nullptr);
    vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    captureChangeInfos.push_back(std::move(captureChangeInfo));
    SourceType sourceType = GetData<SourceType>();
    audioDeviceCommon.HandleBluetoothInputDeviceFetched(desc, captureChangeInfos, sourceType);
    OnStop();
}

void NotifyRecreateCapturerStreamFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = true;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    CHECK_AND_RETURN(capturerChangeInfo != nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    capturerChangeInfo->capturerInfo.originalFlag = AUDIO_FLAG_MMAP;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(deviceDescriptor);
    std::vector<std::string> networkIdList = {"test", "LocalDevice"};
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
    CHECK_AND_RETURN(remoteDeviceDescriptor != nullptr);
    audioDeviceCommon.isOpenRemoteDevice = GetData<bool>();
    audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDeviceDescriptor);
    OnStop();
}

void ScoInputDeviceFetchedForRecongnitionFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool handleFlag = GetData<bool>();
    std::string address = "";
    ConnectState connectState = GetData<ConnectState>();
    audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    OnStop();
}

void CheckAndNotifyUserSelectedDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    audioDeviceCommon.CheckAndNotifyUserSelectedDevice(desc);
}

void HasLowLatencyCapabilityFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    DeviceType deviceType = GetData<DeviceType>();
    bool isRemote = GetData<bool>();
    audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    OnStop();
}

void GetSpatialDeviceTypeFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::string macAddress = "F0-FA-C7-8C-46-01";
    audioDeviceCommon.GetSpatialDeviceType(macAddress);

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.macAddress_ = "F0-FA-C7-8C-46-01";
    deviceDescriptor.deviceType_ = GetData<DeviceType>();
    shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();
    CHECK_AND_RETURN(audioPolicyServerHandler != nullptr);
    audioDeviceCommon.Init(audioPolicyServerHandler);
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);
    audioDeviceCommon.GetSpatialDeviceType(macAddress);
    audioDeviceCommon.DeInit();
    OnStop();
}

void IsDeviceConnectedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptorSptr != nullptr);
    audioDeviceCommon.IsDeviceConnected(audioDeviceDescriptorSptr);
}

void IsSameDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    AudioDeviceDescriptor deviceInfo;
    desc->deviceType_ = GetData<DeviceType>();
    deviceInfo.deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    desc->networkId_ = "";
    deviceInfo.networkId_ = "";
    desc->macAddress_ = "";
    deviceInfo.macAddress_ = "";
    desc->connectState_ = GetData<ConnectState>();
    deviceInfo.connectState_ = GetData<ConnectState>();
    desc->deviceType_ = GetData<DeviceType>();
    deviceInfo.deviceType_ = GetData<DeviceType>();
    desc->deviceRole_ = GetData<DeviceRole>();
    deviceInfo.deviceRole_ = GetData<DeviceRole>();
    audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    OnStop();
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
    SourceType sourceType = GetData<SourceType>();
    audioDeviceCommon.GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
    OnStop();
}

void LoadA2dpModuleFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    DeviceType deviceType = GetData<DeviceType>();
    AudioStreamInfo audioStreamInfo;
    std::string networkID = "";
    std::string sinkName = "";
    SourceType sourceType = GetData<SourceType>();
    ClassType classType = GetData<ClassType>();
    AudioModuleInfo moduleInfo;
    std::list<AudioModuleInfo> moduleInfoList;
    moduleInfoList.push_back(moduleInfo);
    audioDeviceCommon.audioConfigManager_.deviceClassInfo_.insert({classType, moduleInfoList});
    audioDeviceCommon.LoadA2dpModule(deviceType, audioStreamInfo, networkID, sinkName, sourceType);
    audioDeviceCommon.audioConfigManager_.deviceClassInfo_.clear();
    OnStop();
}

void ReloadA2dpAudioPortFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioModuleInfo moduleInfo;
    DeviceType deviceType = GetData<DeviceType>();
    AudioStreamInfo audioStreamInfo;
    std::string networkID = "";
    std::string sinkName = "";
    SourceType sourceType = GetData<SourceType>();
    audioDeviceCommon.ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkID, sinkName, sourceType);
    OnStop();
}

void SwitchActiveA2dpDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(deviceDescriptor != nullptr);
    deviceDescriptor->macAddress_ = Bluetooth::AudioA2dpManager::GetActiveA2dpDevice();
    deviceDescriptor->deviceName_ = "TestA2dpDevice";
    deviceDescriptor->deviceType_ = GetData<DeviceType>();
    audioDeviceCommon.audioA2dpDevice_.connectedA2dpDeviceMap_[deviceDescriptor->macAddress_] = A2dpDeviceConfigInfo();

    audioDeviceCommon.audioIOHandleMap_.IOHandles_[BLUETOOTH_SPEAKER] = GetData<uint32_t>();
    audioDeviceCommon.SwitchActiveA2dpDevice(deviceDescriptor);
    OnStop();
}

void RingToneVoiceControlFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    InternalDeviceType deviceType = GetData<DeviceType>();
    audioDeviceCommon.RingToneVoiceControl(deviceType);
    OnStop();
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
    bool isVirtual = GetData<bool>();
    audioDeviceCommon.SetVirtualCall(uid, isVirtual);
    OnStop();
}

void SetHeadsetUnpluggedToSpkOrEpFlagFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    InternalDeviceType oldDeviceType = GetData<DeviceType>();
    InternalDeviceType newDeviceType = GetData<DeviceType>();
    audioDeviceCommon.SetHeadsetUnpluggedToSpkOrEpFlag(oldDeviceType, newDeviceType);
    OnStop();
}

void WriteInputRouteChangeEventFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->deviceType_ = GetData<DeviceType>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.WriteInputRouteChangeEvent(desc, reason);
    OnStop();
}

void MoveToNewInputDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    CHECK_AND_RETURN(capturerChangeInfo != nullptr);
    capturerChangeInfo->sessionId = SESSIONID;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "00:11:22:33:44:55";
    capturerChangeInfo->inputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> inputDevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(inputDevice != nullptr);
    inputDevice->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    inputDevice->macAddress_ = "66:77:88:99:AA:BB";
    inputDevice->networkId_ = GetData<bool>() == 0 ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;

    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(GetData<bool>());
    audioDeviceCommon.MoveToNewInputDevice(capturerChangeInfo, inputDevice);
    OnStop();
}

void HandleA2dpInputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    SourceType sourceType = GetData<SourceType>();
    audioDeviceCommon.HandleA2dpInputDeviceFetched(desc, sourceType);
    OnStop();
}

void TriggerRecreateCapturerStreamCallbackFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    CHECK_AND_RETURN(capturerChangeInfo != nullptr);
    int32_t streamFlag = GetData<int32_t>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();
    CHECK_AND_RETURN(audioPolicyServerHandler != nullptr);
    audioDeviceCommon.Init(audioPolicyServerHandler);
    if (GetData<bool>()) {
        audioDeviceCommon.DeInit();
    }
    audioDeviceCommon.TriggerRecreateCapturerStreamCallback(capturerChangeInfo, streamFlag, reason);
    audioDeviceCommon.DeInit();
    OnStop();
}

void HandleScoInputDeviceFetchedFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptorSptr != nullptr);
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    audioDeviceCommon.HandleScoInputDeviceFetched(audioDeviceDescriptorSptr, captureChangeInfos);
    OnStop();
}

void AudioDeviceCommonOpenRemoteAudioDeviceFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::string networkId = "testNetworkId";
    DeviceRole deviceRole = GetData<DeviceRole>();
    DeviceType deviceType = GetData<DeviceType>();
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(remoteDeviceDescriptor != nullptr);
    audioDeviceCommon.OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
    OnStop();
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
    CHECK_AND_RETURN(desc != nullptr);
    if (GetData<bool>()) {
        desc->networkId_.clear();
    }

    audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    OnStop();
}

void AudioDeviceCommonDeviceParamsCheckFuzzTest()
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    DeviceRole targetRole = GetData<DeviceRole>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptor != nullptr);
    audioDeviceDescriptor->deviceType_ = GetData<DeviceType>();
    audioDeviceDescriptor->deviceRole_ = GetData<DeviceRole>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(audioDeviceDescriptor);

    audioDeviceCommon.DeviceParamsCheck(targetRole, audioDeviceDescriptors);
    OnStop();
}

TestFuncs g_testFuncs[] = {
    FilterSourceOutputsFuzzTest,
    IsRingerOrAlarmerDualDevicesRangeFuzzTest,
    IsRingOverPlaybackFuzzTest,
    GetPreferredInputDeviceDescInnerFuzzTest,
    GetPreferredInputStreamTypeInnerFuzzTest,
    UpdateDeviceInfoFuzzTest,
    UpdateConnectedDevicesWhenDisconnectingFuzzTest,
    UpdateDualToneStateFuzzTest,
    IsFastFromA2dpToA2dpFuzzTest,
    GetDeviceDescriptorInnerFuzzTest,
    FetchOutputEndFuzzTest,
    HandleDeviceChangeForFetchOutputDeviceFuzzTest,
    MuteSinkForSwitchGeneralDeviceFuzzTest,
    MuteSinkForSwitchBluetoothDeviceFuzzTest,
    IsRendererStreamRunningFuzzTest,
    ActivateA2dpDeviceWhenDescEnabledFuzzTest,
    NotifyRecreateRendererStreamFuzzTest,
    NeedRehandleA2DPDeviceFuzzTest,
    MoveToNewOutputDeviceFuzzTest,
    MuteSinkPortFuzzTest,
    TriggerRecreateRendererStreamCallbackFuzzTest,
    IsDualStreamWhenRingDualFuzzTest,
    UpdateRouteFuzzTest,
    IsRingDualToneOnPrimarySpeakerFuzzTest,
    ClearRingMuteWhenCallStartFuzzTest,
    HandleDeviceChangeForFetchInputDeviceFuzzTest,
    HandleBluetoothInputDeviceFetchedFuzzTest,
    MoveToRemoteInputDeviceFuzzTest,
    ScoInputDeviceFetchedForRecongnitionFuzzTest,
    HasLowLatencyCapabilityFuzzTest,
    GetSpatialDeviceTypeFuzzTest,
    IsSameDeviceFuzzTest,
    GetA2dpModuleInfoFuzzTest,
    LoadA2dpModuleFuzzTest,
    ReloadA2dpAudioPortFuzzTest,
    SwitchActiveA2dpDeviceFuzzTest,
    RingToneVoiceControlFuzzTest,
    SetVirtualCallFuzzTest,
    SetHeadsetUnpluggedToSpkOrEpFlagFuzzTest,
    WriteInputRouteChangeEventFuzzTest,
    MoveToNewInputDeviceFuzzTest,
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

    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[g_count % len]();
        g_count++;
        OnStop();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    g_count = g_count == len ? 0 : g_count;
    return;
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::SetDeviceConnectedFlagWhenFetchOutputDeviceFuzzTest();
    OHOS::AudioStandard::FetchOutputDeviceFuzzTest();
    OHOS::AudioStandard::FetchOutputDeviceWhenNoRunningStreamFuzzTest();
    OHOS::AudioStandard::SetVoiceCallMuteForSwitchDeviceFuzzTest();
    OHOS::AudioStandard::ActivateA2dpDeviceFuzzTest();
    OHOS::AudioStandard::NotifyRecreateCapturerStreamFuzzTest();
    OHOS::AudioStandard::CheckAndNotifyUserSelectedDeviceFuzzTest();
    OHOS::AudioStandard::IsDeviceConnectedFuzzTest();
    OHOS::AudioStandard::GetSourceOutputsFuzzTest();
    OHOS::AudioStandard::SetFirstScreenOnFuzzTest();
    OHOS::AudioStandard::HandleScoOutputDeviceFetchedFuzzTest();
    OHOS::AudioStandard::ClientDiedDisconnectScoNormalFuzzTest();
    OHOS::AudioStandard::ClientDiedDisconnectScoRecognitionFuzzTest();
    OHOS::AudioStandard::OnStop();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
