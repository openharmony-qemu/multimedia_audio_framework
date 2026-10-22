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
#include <memory>
#include <string>
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
const size_t MAX_STRING_LEN = 64;
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

std::string ConsumeString(FuzzedDataProvider &fdp)
{
    return fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
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

uint32_t ConsumeRouteFlag(FuzzedDataProvider &fdp)
{
    return PickValue(fdp, AudioFlagVec);
}

std::shared_ptr<AudioDeviceDescriptor> ConsumeDeviceDescriptor(FuzzedDataProvider &fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return nullptr;
    }
    desc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    desc->deviceRole_ = PickValue(fdp, g_testDeviceRoles);
    desc->networkId_ = ConsumeString(fdp);
    desc->macAddress_ = ConsumeString(fdp);
    desc->deviceName_ = ConsumeString(fdp);
    return desc;
}

std::shared_ptr<AudioStreamDescriptor> ConsumeStreamDescriptor(FuzzedDataProvider &fdp)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return nullptr;
    }
    streamDesc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->callerPid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appUid = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appPid = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appTokenId = fdp.ConsumeIntegral<uint32_t>();
    streamDesc->streamStatus_ = PickValue(fdp, AudioStreamStatusVec);
    streamDesc->streamAction_ = ConsumeStreamAction(fdp);
    streamDesc->routeFlag_ = ConsumeRouteFlag(fdp);
    streamDesc->capturerInfo_.sourceType = PickValue(fdp, g_testSourceTypes);
    streamDesc->audioFlag_ = PickValue(fdp, AudioFlagVec);
    auto newDevice = ConsumeDeviceDescriptor(fdp);
    if (newDevice != nullptr) {
        streamDesc->newDeviceDescs_.push_back(newDevice);
    }
    return streamDesc;
}

std::shared_ptr<AudioPipeInfo> ConsumePipeInfo(FuzzedDataProvider &fdp)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    if (pipeInfo == nullptr) {
        return nullptr;
    }
    pipeInfo->id_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->paIndex_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->routeFlag_ = ConsumeRouteFlag(fdp);
    pipeInfo->name_ = ConsumeString(fdp);
    pipeInfo->adapterName_ = ConsumeString(fdp);
    pipeInfo->moduleInfo_.name = ConsumeString(fdp);
    pipeInfo->moduleInfo_.className = ConsumeString(fdp);
    pipeInfo->moduleInfo_.networkId = ConsumeString(fdp);
    return pipeInfo;
}

AudioStreamDeviceChangeReasonExt ConsumeChangeReason(FuzzedDataProvider &fdp)
{
    return AudioStreamDeviceChangeReasonExt(ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp));
}

void FillConcurrentCaptureDfxResult(FuzzedDataProvider &fdp, const std::unique_ptr<ConcurrentCaptureDfxResult> &result)
{
    if (result == nullptr) {
        return;
    }
    result->existingAppName.push_back(ConsumeString(fdp));
    result->existingAppState.push_back(fdp.ConsumeIntegral<uint8_t>());
    result->existingSourceType.push_back(fdp.ConsumeIntegral<uint8_t>());
    result->existingCaptureState.push_back(fdp.ConsumeIntegral<uint8_t>());
    result->existingCreateDuration.push_back(fdp.ConsumeIntegral<uint32_t>());
    result->existingStartDuration.push_back(fdp.ConsumeIntegral<uint32_t>());
    result->existingFastFlag.push_back(fdp.ConsumeBool());
    result->hdiSourceType = fdp.ConsumeIntegral<uint8_t>();
    result->hdiSourceAlg = ConsumeString(fdp);
    result->deviceType = fdp.ConsumeIntegral<uint8_t>();
}

void FillMoveDevices(FuzzedDataProvider &fdp, const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamDesc == nullptr) {
        return;
    }
    auto oldDevice = ConsumeDeviceDescriptor(fdp);
    if (oldDevice != nullptr) {
        streamDesc->oldDeviceDescs_.push_back(oldDevice);
    }
    auto newDevice = ConsumeDeviceDescriptor(fdp);
    if (newDevice != nullptr) {
        streamDesc->newDeviceDescs_.push_back(newDevice);
    }
}

void GetFastControlParamFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t value = fdp.ConsumeBool();
    SetSysPara("persist.multimedia.audioflag.fastcontrolled", value);
    GetFastControlParam();
}

void GetEcEnableParamFuzzTest(FuzzedDataProvider& fdp)
{
    SetSysPara("persist.multimedia.audioflag.ecenable", fdp.ConsumeBool());
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
    shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(streamDesc != nullptr);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    CHECK_AND_RETURN(handler != nullptr);
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
}

void NeedRehandleA2DPDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto desc = ConsumeDeviceDescriptor(fdp);
    CHECK_AND_RETURN(desc != nullptr);
    auto audioCoreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    std::string moduleName = BLUETOOTH_MIC;
    AudioIOHandle moduleId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    audioCoreService->NeedRehandleA2DPDevice(desc);
}

void ProcessOutputPipeUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = ConsumePipeInfo(fdp);
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr && audioStreamDescriptor != nullptr);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = ConsumeRouteFlag(fdp);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    CHECK_AND_RETURN(audioDeviceDescriptor != nullptr);
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
}

void TriggerRecreateCapturerStreamCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(streamDesc != nullptr);
    auto audioCoreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);
}

void ProcessInputPipeUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = ConsumePipeInfo(fdp);
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr && audioStreamDescriptor != nullptr);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = ConsumeRouteFlag(fdp);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    CHECK_AND_RETURN(audioDeviceDescriptor != nullptr);
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
}

void ProcessInputPipeNewFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = ConsumePipeInfo(fdp);
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr && audioStreamDescriptor != nullptr);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = ConsumeRouteFlag(fdp);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    CHECK_AND_RETURN(audioDeviceDescriptor != nullptr);
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
}

void MoveToNewInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(streamDesc != nullptr);
    FillMoveDevices(fdp, streamDesc);
    std::vector<SourceOutput> sourceOutputs = audioCoreService->GetSourceOutputs();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    audioCoreService->MoveToNewInputDevice(streamDesc, sourceOutputs, pipeInfo);
}

void AudioCoreServicePrivatePrepareMoveAttrsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    FillMoveDevices(fdp, audioStreamDescriptor);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    std::string oldSinkName = ConsumeString(fdp);
    bool isNeedTriggerCallback = fdp.ConsumeBool();
    DeviceType oldDeviceType = PickValue(fdp, DeviceTypeVec);
    audioCoreService->PrepareMoveAttrs(audioStreamDescriptor,
        oldDeviceType, isNeedTriggerCallback, oldSinkName, reason);
}

void IsNewDevicePlaybackSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(streamDesc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = ConsumeDeviceDescriptor(fdp);
    CHECK_AND_RETURN(newDeviceDesc != nullptr);
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
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    FillMoveDevices(fdp, audioStreamDescriptor);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    audioCoreService->MuteSinkPortForSwitchDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateSetVoiceCallMuteForSwitchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->audioSceneManager_.SetAudioScenePre(ConsumeEnum<AudioScene>(fdp));
    audioCoreService->SetVoiceCallMuteForSwitchDevice();
    audioCoreService->GetDisableFastStreamParam();
}

void AudioCoreServicePrivateMuteSinkPortLogicFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    const std::string oldSinkName = fdp.ConsumeBool() ? OFFLOAD_PRIMARY_SPEAKER : ConsumeString(fdp);
    const std::string newSinkName = fdp.ConsumeBool() ? OFFLOAD_PRIMARY_SPEAKER : ConsumeString(fdp);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    audioCoreService->MuteSinkPortLogic(oldSinkName, newSinkName, reason);
}

void AudioCoreServicePrivateMuteSinkPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    const std::string oldSinkName = fdp.ConsumeBool() ? OFFLOAD_PRIMARY_SPEAKER : ConsumeString(fdp);
    const std::string newSinkName = fdp.ConsumeBool() ? OFFLOAD_PRIMARY_SPEAKER : ConsumeString(fdp);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
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
    AudioScene audioScene = ConsumeEnum<AudioScene>(fdp);
    audioCoreService->OnAudioSceneChange(audioScene);
}

void AudioCoreServicePrivateActivateOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    FillMoveDevices(fdp, audioStreamDescriptor);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    audioCoreService->ActivateOutputDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateHandleOutputStreamInRunningFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    FillMoveDevices(fdp, audioStreamDescriptor);
    AudioStreamDeviceChangeReasonExt reason = ConsumeChangeReason(fdp);
    audioCoreService->HandleOutputStreamInRunning(audioStreamDescriptor, reason);
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
    audioCoreService->HandleOutputStreamInRunning(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateHandleDualStartClientFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    FillMoveDevices(fdp, audioStreamDescriptor);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back({PickValue(fdp, DeviceTypeVec), OUTPUT_DEVICES_FLAG});
    audioCoreService->HandleDualStartClient(activeDevices, audioStreamDescriptor);
}

void AudioCoreServicePrivateHandleInputStreamInRunningFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    FillMoveDevices(fdp, audioStreamDescriptor);
    audioCoreService->HandleInputStreamInRunning(audioStreamDescriptor);
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
    audioCoreService->HandleInputStreamInRunning(audioStreamDescriptor);
}

void AudioCoreServicePrivateSelectA2dpTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    CHECK_AND_RETURN(audioStreamDescriptor != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    CHECK_AND_RETURN(audioDeviceDescriptor != nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    auto oldDevice = ConsumeDeviceDescriptor(fdp);
    if (oldDevice != nullptr) {
        audioStreamDescriptor->oldDeviceDescs_.push_back(oldDevice);
    }
    bool isCreateProcess = fdp.ConsumeBool();
    audioCoreService->SelectA2dpType(audioStreamDescriptor, isCreateProcess);
}

void AudioCoreServicePrivateUpdateOffloadStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = ConsumePipeInfo(fdp);
    if (audioCoreService == nullptr || pipeInfo == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->streamAction_ = ConsumeStreamAction(fdp);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;
    pipeInfo->moduleInfo_.className = fdp.ConsumeBool() ? "remote_offload" : ConsumeString(fdp);
    audioCoreService->UpdateOffloadState(pipeInfo);
}

void AudioCoreServiceUpdateModemRouteFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
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
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
    if (streamDesc == nullptr) {
        return;
    }
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(
        std::make_pair(fdp.ConsumeIntegral<uint32_t>(), streamDesc));
    audioCoreService->UpdateModemRoute(descs);
}

void AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest2(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->streamStatus_ = PickValue(fdp, AudioStreamStatusVec);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> insertList1 = {"00:00:00:00:00:00", ConsumeString(fdp)};
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
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->newDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> networkIdList = {ConsumeString(fdp), "networkId"};
    audioDeviceDescriptor->networkId_ = PickValue(fdp, networkIdList);
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    audioCoreService->NotifyRouteUpdate(streamDescs);
}

void AudioCoreServicePrivateUpdateSpecialRemoteModuleNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = ConsumePipeInfo(fdp);
    if (audioCoreService == nullptr || pipeInfo == nullptr) {
        return;
    }
    pipeInfo->moduleInfo_.className = fdp.ConsumeBool() ? "remote_offload" : ConsumeString(fdp);
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;
    std::string moduleName;
    audioCoreService->UpdateSpecialRemoteModuleName(pipeInfo, moduleName);
}

void AudioCoreServicePrivateGetRealPidFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
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
    config.appInfo.appUid = fdp.ConsumeIntegral<int32_t>();
    config.appInfo.appPid = fdp.ConsumeIntegral<int32_t>();
    config.capturerInfo.sourceType = PickValue(fdp, g_testSourceTypes);
    audioCoreService->SetWakeUpAudioCapturerFromAudioServer(config);
}

void AudioCoreServicePrivateLogCapturerConcurrentResultFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    FillConcurrentCaptureDfxResult(fdp, result);
    audioCoreService->LogCapturerConcurrentResult(result);
}

void AudioCoreServicePrivateWriteCapturerConcurrentEventFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    FillConcurrentCaptureDfxResult(fdp, result);
    audioCoreService->WriteCapturerConcurrentEvent(result);
}

void AudioCoreServicePrivateWriteCapturerConcurrentMsgFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    FillConcurrentCaptureDfxResult(fdp, result);
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = ConsumeStreamDescriptor(fdp);
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
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
    std::vector<std::string> bundleNameList = {ConsumeString(fdp), "bundleName"};
    std::string bundleName = PickValue(fdp, bundleNameList);
    audioCoreService->IsFastAllowed(bundleName);
}

void AudioCoreServicePrivateSleepForSwitchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConsumeStreamDescriptor(fdp);
    if (streamDesc == nullptr) {
        return;
    }
    streamDesc->oldDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = ConsumeDeviceDescriptor(fdp);
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    streamDesc->newDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = ConsumeDeviceDescriptor(fdp);
    if (audioDeviceDescriptor1 == nullptr) {
        return;
    }
    audioDeviceDescriptor1->networkId_ = fdp.ConsumeBool() ? "networkId" : ConsumeString(fdp);
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
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = ConsumeDeviceDescriptor(fdp);
    if (oldDesc == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> newDesc = ConsumeDeviceDescriptor(fdp);
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
    AudioCollaborativeService::GetAudioCollaborativeService().isCollaborativeStateEnabled_ =
        static_cast<CollaborationState>(fdp.ConsumeIntegralInRange(0, 2)); // 2 enum value
    uint32_t pipeId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->UpdateRouteForCollaboration(deviceType, pipeId);
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
    AudioCoreServicePrivateUpdateSpecialRemoteModuleNameFuzzTest,
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
extern "C" int LLVMFuzzerInitialize(int *, char ***)
{
    OHOS::AudioStandard::Init();
    return 0;
}
