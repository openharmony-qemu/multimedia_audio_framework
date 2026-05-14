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
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const int32_t MEDIA_SERVICE_UID = 1013;

typedef void (*TestFuncs)();

template<typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
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

template<typename T>
T PickValue(FuzzedDataProvider &fdp, const std::vector<T> &values)
{
    if (values.empty()) {
        return T {};
    }
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

AudioStreamAction ConsumeStreamAction(FuzzedDataProvider &fdp)
{
    int32_t maxAction = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE);
    return static_cast<AudioStreamAction>(fdp.ConsumeIntegralInRange<int32_t>(0, maxAction));
}

vector<AudioFlag> AudioFlagVec = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_DIRECT,
    AUDIO_OUTPUT_FLAG_HD,
    AUDIO_OUTPUT_FLAG_MULTICHANNEL,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_VOIP,
    AUDIO_OUTPUT_FLAG_VOIP_FAST,
    AUDIO_OUTPUT_FLAG_HWDECODING,
    AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
    AUDIO_INPUT_FLAG_VOIP,
    AUDIO_INPUT_FLAG_VOIP_FAST,
    AUDIO_INPUT_FLAG_WAKEUP,
    AUDIO_INPUT_FLAG_AI,
    AUDIO_INPUT_FLAG_ULTRASONIC,
    AUDIO_FLAG_MAX,
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

const vector<DeviceRole> g_testDeviceRoles = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
};

const vector<SourceType> g_testSourceTypes = {
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

vector<AudioStreamStatus> AudioStreamStatusVec = {
    STREAM_STATUS_NEW,
    STREAM_STATUS_STARTED,
    STREAM_STATUS_PAUSED,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_RELEASED,
};

void GetFastControlParamFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t value = fdp.ConsumeBool();
    SetSysPara("persist.multimedia.audioflag.fastcontrolled", value);
    GetFastControlParam();
}

void GetEcEnableParamFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    GetEcEnableParam();
}

void CoreServicePipeManagerPassThroughFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN(audioCoreService != nullptr);

    audioCoreService->SetEcEnableState(fdp.ConsumeBool());
    audioCoreService->GetFastFormat();
    audioCoreService->IsSupportInnerCaptureOffload();
    audioCoreService->GetEnhancedRoutingSupported();
    audioCoreService->GetMaxRendererInstances();
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    audioCoreService->GetDeviceClassInfo(deviceClassInfo);
}

void TriggerRecreateRendererStreamCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerPid_ = 0;
    streamDesc->sessionId_ = 0;
    streamDesc->routeFlag_ = fdp.ConsumeBool();
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    CHECK_AND_RETURN(handler != nullptr);
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
}

void NeedRehandleA2DPDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    auto audioCoreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    desc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    std::string moduleName = BLUETOOTH_MIC;
    AudioIOHandle moduleId = 0;
    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    audioCoreService->NeedRehandleA2DPDevice(desc);
}

void ProcessOutputPipeUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
}

void TriggerRecreateCapturerStreamCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = fdp.ConsumeIntegral<uint32_t>(),
    streamDesc->callerUid_ = 0;
    streamDesc->appInfo_.appUid = 0;
    streamDesc->appInfo_.appPid = 0;
    streamDesc->appInfo_.appTokenId = 0;
    streamDesc->streamStatus_ = ConsumeEnum<AudioStreamStatus>(fdp);
    streamDesc->routeFlag_ = fdp.ConsumeBool();
    auto audioCoreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);
    SwitchStreamUtil::RemoveAllRecordBySessionId(streamDesc->sessionId_);
}

void ProcessInputPipeUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
}

void ProcessInputPipeNewFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    audioStreamDescriptor->routeFlag_ = PickValue(fdp, AudioFlagVec);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
}

void MoveToNewInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor2);
    std::vector<SourceOutput> sourceOutputs = audioCoreService->GetSourceOutputs();
    audioCoreService->MoveToNewInputDevice(streamDesc, sourceOutputs);
}

void AudioCoreServicePrivatePrepareMoveAttrsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    std::string oldSinkName = "";
    bool isNeedTriggerCallback = true;
    DeviceType oldDeviceType;
    audioCoreService->PrepareMoveAttrs(audioStreamDescriptor,
        oldDeviceType, isNeedTriggerCallback, oldSinkName, reason);
}

void IsNewDevicePlaybackSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    int32_t encodingStart = static_cast<int32_t>(AudioEncodingType::ENCODING_INVALID);
    int32_t encodingEnd = static_cast<int32_t>(AudioEncodingType::ENCODING_EAC3);
    streamDesc->streamInfo_.encoding = static_cast<AudioEncodingType>(
        fdp.ConsumeIntegralInRange<int32_t>(encodingStart, encodingEnd));
    audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
}

void AudioCoreServicePrivateMuteSinkPortForSwitchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor1->networkId_ = "networkId";
    audioDeviceDescriptor1->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPortForSwitchDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateSetVoiceCallMuteForSwitchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->SetVoiceCallMuteForSwitchDevice();
    audioCoreService->GetDisableFastStreamParam();
    audioCoreService->IsSceneRequireMuteAndSleep();
}

void AudioCoreServicePrivateMuteSinkPortLogicFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    const std::string oldSinkName = OFFLOAD_PRIMARY_SPEAKER;
    const std::string newSinkName = OFFLOAD_PRIMARY_SPEAKER;
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPortLogic(oldSinkName, newSinkName, reason);
}

void AudioCoreServicePrivateMuteSinkPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    const std::string oldSinkName = OFFLOAD_PRIMARY_SPEAKER;
    const std::string newSinkName = OFFLOAD_PRIMARY_SPEAKER;
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPort(oldSinkName, newSinkName, reason);
    reason.reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    audioCoreService->MuteSinkPort(oldSinkName, newSinkName, reason);
    reason.reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    audioCoreService->MuteSinkPort(oldSinkName, newSinkName, reason);
}

void AudioCoreServicePrivateOnAudioSceneChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioScene audioScene = AudioScene::AUDIO_SCENE_DEFAULT;
    audioCoreService->OnAudioSceneChange(audioScene);
}

void AudioCoreServicePrivateActivateOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->ActivateOutputDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateHandleOutputStreamInRunningFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->HandleOutputStreamInRunning(audioStreamDescriptor, reason);
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
    audioCoreService->HandleOutputStreamInRunning(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateHandleDualStartClientFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor1);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back({DEVICE_TYPE_ACCESSORY, OUTPUT_DEVICES_FLAG});
    audioCoreService->HandleDualStartClient(activeDevices, audioStreamDescriptor);
}

void AudioCoreServicePrivateHandleInputStreamInRunningFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    audioCoreService->HandleInputStreamInRunning(audioStreamDescriptor);
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
    audioCoreService->HandleInputStreamInRunning(audioStreamDescriptor);
}

void AudioCoreServicePrivateSelectA2dpTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    bool isCreateProcess = true;
    audioCoreService->SelectA2dpType(audioStreamDescriptor, isCreateProcess);
}

void AudioCoreServicePrivateUpdateOffloadStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (audioCoreService == nullptr || pipeInfo == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;
    pipeInfo->moduleInfo_.className = "remote_offload";
    audioCoreService->UpdateOffloadState(pipeInfo);
}

void AudioCoreServiceUpdateModemRouteFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    if (audioCoreService->pipeManager_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    audioCoreService->UpdateModemRoute(descs);
}

void AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest2(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->streamStatus_ = PickValue(fdp, AudioStreamStatusVec);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> insertList1 = {"00:00:00:00:00:00", "abc"};
    std::string macAddress = PickValue(fdp, insertList1);
    audioDeviceDescriptor->macAddress_ = macAddress;
    audioDeviceDescriptor->deviceRole_ = PickValue(fdp, g_testDeviceRoles);
    audioDeviceDescriptor->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioStreamDescriptor->newDeviceDescs_.clear();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioStreamDescriptor->oldDeviceDescs_.clear();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    if (audioCoreService->audioA2dpOffloadManager_ == nullptr) {
        return;
    }
    audioCoreService->FetchRendererPipesAndExecute(streamDescs, reason);
}

void AudioCoreServicePrivateNotifyRouteUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->newDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> networkIdList = {"abc", "networkId"};
    audioDeviceDescriptor->networkId_ = PickValue(fdp, networkIdList);
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    audioCoreService->NotifyRouteUpdate(streamDescs);
}

void AudioCoreServicePrivateUpdateRemoteOffloadModuleNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (audioCoreService == nullptr || pipeInfo == nullptr) {
        return;
    }
    pipeInfo->moduleInfo_.className = "remote_offload";
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;
    std::string moduleName;
    audioCoreService->UpdateRemoteOffloadModuleName(pipeInfo, moduleName);
}

void AudioCoreServicePrivateGetRealPidFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    streamDesc->callerUid_ = MEDIA_SERVICE_UID;
    audioCoreService->GetRealPid(streamDesc);
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->GetRealPid(streamDesc);
}

void AudioCoreServicePrivateSetWakeUpAudioCapturerFromAudioServerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioProcessConfig config;
    audioCoreService->SetWakeUpAudioCapturerFromAudioServer(config);
}

void AudioCoreServicePrivateLogCapturerConcurrentResultFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    audioCoreService->LogCapturerConcurrentResult(result);
}

void AudioCoreServicePrivateWriteCapturerConcurrentEventFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    audioCoreService->WriteCapturerConcurrentEvent(result);
}

void AudioCoreServicePrivateWriteCapturerConcurrentMsgFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioStreamDescriptor->newDeviceDescs_.clear();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->WriteCapturerConcurrentMsg(audioStreamDescriptor, result);
}

void AudioCoreServicePrivateIsFastAllowedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<std::string> bundleNameList = {"abc", "bundleName"};
    std::string bundleName = PickValue(fdp, bundleNameList);
    audioCoreService->IsFastAllowed(bundleName);
}

void AudioCoreServicePrivateSleepForSwitchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    streamDesc->oldDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    streamDesc->newDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor1 == nullptr) {
        return;
    }
    audioDeviceDescriptor1->networkId_ = "networkId";
    audioDeviceDescriptor1->deviceType_ = PickValue(fdp, DeviceTypeVec);
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor1);

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
}

void AudioCoreServicePrivateIsHeadsetToSpkOrEpFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>();
    if (oldDesc == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>();
    if (newDesc == nullptr) {
        return;
    }
    oldDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    newDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioCoreService->IsHeadsetToSpkOrEp(oldDesc, newDesc);
}

void AudioCoreServicePrivateUpdateRouteForCollaborationFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    InternalDeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    AudioCollaborativeService::GetAudioCollaborativeService().isCollaborativeStateEnabled_ = true;
    uint32_t sessionID = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->UpdateRouteForCollaboration(deviceType, sessionID);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    GetFastControlParamFuzzTest,
    GetEcEnableParamFuzzTest,
    CoreServicePipeManagerPassThroughFuzzTest,
    NeedRehandleA2DPDeviceFuzzTest,
    TriggerRecreateRendererStreamCallbackFuzzTest,

    TriggerRecreateCapturerStreamCallbackFuzzTest,
    ProcessOutputPipeUpdateFuzzTest,
    ProcessInputPipeNewFuzzTest,
    ProcessInputPipeUpdateFuzzTest,
    MoveToNewInputDeviceFuzzTest,
    IsNewDevicePlaybackSupportedFuzzTest,
    AudioCoreServicePrivatePrepareMoveAttrsFuzzTest,
    AudioCoreServicePrivateMuteSinkPortForSwitchDeviceFuzzTest,
    AudioCoreServicePrivateSetVoiceCallMuteForSwitchDeviceFuzzTest,
    AudioCoreServicePrivateMuteSinkPortFuzzTest,
    AudioCoreServicePrivateMuteSinkPortLogicFuzzTest,
    AudioCoreServicePrivateActivateOutputDeviceFuzzTest,
    AudioCoreServicePrivateOnAudioSceneChangeFuzzTest,
    AudioCoreServicePrivateHandleOutputStreamInRunningFuzzTest,
    AudioCoreServicePrivateHandleInputStreamInRunningFuzzTest,
    AudioCoreServicePrivateHandleDualStartClientFuzzTest,
    AudioCoreServicePrivateSelectA2dpTypeFuzzTest,
    AudioCoreServiceUpdateModemRouteFuzzTest,
    AudioCoreServicePrivateUpdateOffloadStateFuzzTest,
    AudioCoreServicePrivateNotifyRouteUpdateFuzzTest,
    AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest2,
    AudioCoreServicePrivateUpdateRemoteOffloadModuleNameFuzzTest,
    AudioCoreServicePrivateGetRealPidFuzzTest,
    AudioCoreServicePrivateSetWakeUpAudioCapturerFromAudioServerFuzzTest,
    AudioCoreServicePrivateWriteCapturerConcurrentEventFuzzTest,
    AudioCoreServicePrivateLogCapturerConcurrentResultFuzzTest,
    AudioCoreServicePrivateWriteCapturerConcurrentMsgFuzzTest,
    AudioCoreServicePrivateIsFastAllowedFuzzTest,
    AudioCoreServicePrivateIsHeadsetToSpkOrEpFuzzTest,
    AudioCoreServicePrivateSleepForSwitchDeviceFuzzTest,
    AudioCoreServicePrivateUpdateRouteForCollaborationFuzzTest,
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
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
