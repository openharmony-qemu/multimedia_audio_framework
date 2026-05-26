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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_core_service.h"
#include "audio_debug_info.h"
#include "audio_errors.h"
#include "audio_pipe_selector.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const size_t MAX_STRING_LEN = 64;
const size_t THRESHOLD = sizeof(uint64_t) + sizeof(uint16_t);
const uint32_t TEST_SESSION_ID = 1000;

const std::vector<DeviceType> DEVICE_TYPES = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_MIC,
};

const std::vector<StreamUsage> STREAM_USAGES = {
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
};

const std::vector<SourceType> SOURCE_TYPES = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_REMOTE_CAST,
};

const std::vector<AudioScene> AUDIO_SCENES = {
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_VOICE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
};

template<typename T>
T PickValue(FuzzedDataProvider &fdp, const std::vector<T> &values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

template<typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

std::shared_ptr<AudioDeviceDescriptor> MakeDevice(FuzzedDataProvider &fdp, DeviceRole role)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return nullptr;
    }
    desc->deviceType_ = PickValue(fdp, DEVICE_TYPES);
    desc->deviceRole_ = role;
    desc->networkId_ = fdp.ConsumeBool() ? LOCAL_NETWORK_ID : fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    desc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    desc->connectState_ = fdp.ConsumeBool() ? CONNECTED : VIRTUAL_CONNECTED;
    desc->dmDeviceType_ = fdp.ConsumeBool() ? DM_DEVICE_TYPE_WIFI_SOUNDBOX : DM_DEVICE_TYPE_DEFAULT;
    return desc;
}

std::shared_ptr<AudioStreamDescriptor> MakeRendererDesc(FuzzedDataProvider &fdp, uint32_t sessionId = TEST_SESSION_ID)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return nullptr;
    }
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->sessionId_ = sessionId;
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->callerPid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appUid = streamDesc->callerUid_;
    streamDesc->appInfo_.appPid = streamDesc->callerPid_;
    streamDesc->rendererInfo_.streamUsage = PickValue(fdp, STREAM_USAGES);
    streamDesc->rendererInfo_.originalFlag = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->rendererInfo_.rendererFlags = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->streamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->streamInfo_.format = SAMPLE_S16LE;
    streamDesc->streamInfo_.channels = fdp.ConsumeBool() ? STEREO : CHANNEL_6;
    streamDesc->streamInfo_.encoding = fdp.ConsumeBool() ? ENCODING_AUDIOVIVID : ENCODING_PCM;
    streamDesc->streamStatus_ = fdp.ConsumeBool() ? STREAM_STATUS_STARTED : STREAM_STATUS_NEW;
    streamDesc->SetBundleName(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    auto newDevice = MakeDevice(fdp, OUTPUT_DEVICE);
    if (newDevice != nullptr) {
        streamDesc->newDeviceDescs_.push_back(newDevice);
    }
    auto oldDevice = MakeDevice(fdp, OUTPUT_DEVICE);
    if (oldDevice != nullptr) {
        streamDesc->oldDeviceDescs_.push_back(oldDevice);
    }
    return streamDesc;
}

std::shared_ptr<AudioStreamDescriptor> MakeCapturerDesc(FuzzedDataProvider &fdp, uint32_t sessionId = TEST_SESSION_ID)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return nullptr;
    }
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->sessionId_ = sessionId;
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->callerPid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appUid = streamDesc->callerUid_;
    streamDesc->appInfo_.appPid = streamDesc->callerPid_;
    streamDesc->capturerInfo_.sourceType = PickValue(fdp, SOURCE_TYPES);
    streamDesc->capturerInfo_.originalFlag = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->capturerInfo_.capturerFlags = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->streamStatus_ = fdp.ConsumeBool() ? STREAM_STATUS_STARTED : STREAM_STATUS_NEW;
    auto newDevice = MakeDevice(fdp, INPUT_DEVICE);
    if (newDevice != nullptr) {
        streamDesc->newDeviceDescs_.push_back(newDevice);
    }
    return streamDesc;
}

std::shared_ptr<AudioPipeInfo> MakePipeInfo(FuzzedDataProvider &fdp, AudioPipeRole role)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    if (pipeInfo == nullptr) {
        return nullptr;
    }
    pipeInfo->id_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->paIndex_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->pipeRole_ = role;
    pipeInfo->name_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    pipeInfo->routeFlag_ = (role == PIPE_ROLE_OUTPUT) ? AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;
    pipeInfo->adapterName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    pipeInfo->moduleInfo_.name = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    pipeInfo->moduleInfo_.adapterName = pipeInfo->adapterName_;
    pipeInfo->moduleInfo_.format = "s16le";
    pipeInfo->moduleInfo_.rate = std::to_string(SAMPLE_RATE_48000);
    pipeInfo->moduleInfo_.channels = std::to_string(STEREO);
    pipeInfo->moduleInfo_.deviceType = std::to_string(static_cast<int32_t>(
        role == PIPE_ROLE_OUTPUT ? DEVICE_TYPE_SPEAKER : DEVICE_TYPE_MIC));
    pipeInfo->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    pipeInfo->audioStreamInfo_.format = SAMPLE_S16LE;
    pipeInfo->audioStreamInfo_.channels = STEREO;
    pipeInfo->audioStreamInfo_.encoding = ENCODING_PCM;
    return pipeInfo;
}

void AudioCoreServicePrivateRouteSelectorGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioPipeSelector::AudioRouteSelectorCallback emptyCallback = nullptr;
    audioCoreService->SetAudioRouteSelectorCallback(emptyCallback);
    AudioPipeSelector::AudioRouteSelectorCallback callback =
        [](const std::shared_ptr<AudioRouteSelectInfo> &, int32_t &selectResult) -> int32_t {
            selectResult = SUCCESS;
            return SUCCESS;
        };
    if (fdp.ConsumeBool()) {
        audioCoreService->SetAudioRouteSelectorCallback(callback);
    }
    audioCoreService->UnsetAudioRouteSelectorCallback();
}

void AudioCoreServicePrivateFlagAndSceneGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto rendererDesc = MakeRendererDesc(fdp);
    if (rendererDesc != nullptr) {
        if (fdp.ConsumeBool()) {
            rendererDesc->SetFastStreamForcedNormalFlag(true);
            rendererDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_FAST);
        }
        streamDescs.push_back(rendererDesc);
    }
    streamDescs.push_back(nullptr);
    audioCoreService->DecidedAudioFlagForStreams(streamDescs);
    audioCoreService->CheckRingAndVoipStreamRunning();
    audioCoreService->CheckCloseHearingAidCall(fdp.ConsumeBool(), PickValue(fdp, DEVICE_TYPES));
    audioCoreService->UpdateDefaultOutputDeviceWhenStopping(fdp.ConsumeIntegral<int32_t>());
    audioCoreService->OnVehiclePriorityChanged(fdp.ConsumeBool());
    audioCoreService->SetSleVoiceStatusFlag(PickValue(fdp, AUDIO_SCENES));
    audioCoreService->HandleRingToNonRingSceneChange(PickValue(fdp, AUDIO_SCENES), PickValue(fdp, AUDIO_SCENES));
    audioCoreService->IsCallOrRingToDefault(PickValue(fdp, AUDIO_SCENES), PickValue(fdp, AUDIO_SCENES));
    auto outputDevice = MakeDevice(fdp, OUTPUT_DEVICE);
    if (outputDevice != nullptr) {
        audioCoreService->SetUpdateModemRouteFinished(true);
        audioCoreService->UnmuteVoiceCallAfterMuteDuration(0, outputDevice);
    }
}

void AudioCoreServicePrivateDebugGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto pipeInfo = MakePipeInfo(fdp, fdp.ConsumeBool() ? PIPE_ROLE_OUTPUT : PIPE_ROLE_INPUT);
    if (pipeInfo == nullptr) {
        return;
    }
    auto streamDesc = pipeInfo->IsOutput() ? MakeRendererDesc(fdp, TEST_SESSION_ID) :
        MakeCapturerDesc(fdp, TEST_SESSION_ID);
    if (streamDesc != nullptr) {
        pipeInfo->streamDescriptors_.push_back(streamDesc);
        pipeInfo->streamDescMap_[streamDesc->sessionId_] = streamDesc;
    }
    AudioDebugInfo debugInfo;
    audioCoreService->GetPipeBaseDebugInfo(pipeInfo, debugInfo);
    audioCoreService->GetStreamDebugInfo(TEST_SESSION_ID, pipeInfo, debugInfo);
    audioCoreService->GetVolumeDebugInfo(debugInfo);
    audioCoreService->GetAudioDebugInfoBySessionId(fdp.ConsumeBool() ? TEST_SESSION_ID :
        fdp.ConsumeIntegral<uint32_t>(), debugInfo);
    audioCoreService->IsValidRenderSessionId(fdp.ConsumeIntegral<uint32_t>());
    audioCoreService->CalculatePipeAudioScene(pipeInfo->id_);
}

void AudioCoreServicePrivateMoveGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto outputPipe = MakePipeInfo(fdp, PIPE_ROLE_OUTPUT);
    auto rendererDesc = MakeRendererDesc(fdp);
    if (outputPipe != nullptr && rendererDesc != nullptr) {
        std::vector<SinkInput> sinkInputs;
        audioCoreService->MoveToLocalOutputDevice(sinkInputs, outputPipe, rendererDesc->newDeviceDescs_.front());
        audioCoreService->MoveStreamSink(rendererDesc, outputPipe,
            ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp));
    }

    auto capturerDesc = MakeCapturerDesc(fdp);
    if (capturerDesc != nullptr) {
        std::vector<SourceOutput> sourceOutputs;
        audioCoreService->MoveToLocalInputDevice(sourceOutputs, capturerDesc->newDeviceDescs_.front(),
            capturerDesc->routeFlag_);
        audioCoreService->MoveStreamSource(capturerDesc, sourceOutputs);
    }
}

void AudioCoreServicePrivateDupAndRemoteGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeRendererDesc(fdp);
    if (streamDesc == nullptr) {
        return;
    }
    if (fdp.ConsumeBool()) {
        auto oldDup = MakeDevice(fdp, OUTPUT_DEVICE);
        auto newDup = MakeDevice(fdp, OUTPUT_DEVICE);
        if (oldDup != nullptr) {
            streamDesc->oldDupDeviceDescs_.push_back(oldDup);
        }
        if (newDup != nullptr) {
            streamDesc->newDupDeviceDescs_.push_back(newDup);
        }
    }
    audioCoreService->IsDupDeviceChange(streamDesc);
    audioCoreService->UpdateDupDeviceOutputRoute(streamDesc);
    AudioStreamDeviceChangeReasonExt reason(ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp));
    audioCoreService->UpdateRemoteDeviceChangeReason(streamDesc, reason);
    audioCoreService->TriggerRecreateRendererStreamCallbackEntry(streamDesc, reason);
    audioCoreService->StopNoRunningPipe();
}

void AudioCoreServicePrivateOutputFetchGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeRendererDesc(fdp);
    if (streamDesc == nullptr) {
        return;
    }
    uint32_t sessionId = streamDesc->sessionId_;
    uint32_t audioFlag = AUDIO_FLAG_NONE;
    audioCoreService->FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag,
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp));
    audioCoreService->HandleFetchOutputWhenNoRunningStream(
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp));
    audioCoreService->UpdateStreamDevicesForCreate(streamDesc,
        fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    audioCoreService->GetFlagForUltraFastStream(streamDesc, fdp.ConsumeBool());
}

void AudioCoreServicePrivateInputFetchGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeCapturerDesc(fdp);
    if (streamDesc == nullptr) {
        return;
    }
    uint32_t audioFlag = AUDIO_FLAG_NONE;
    uint32_t sessionId = streamDesc->sessionId_;
    audioCoreService->IsDescInSourceStrategyMap(streamDesc);
    audioCoreService->FetchCapturerPipeAndExecute(streamDesc, audioFlag, sessionId);
}

void AudioCoreServicePrivateBluetoothAndOffloadGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeRendererDesc(fdp);
    if (streamDesc != nullptr) {
        audioCoreService->CheckOffloadPipeUidAllowed(streamDesc, fdp.ConsumeIntegral<int32_t>());
        audioCoreService->ForceRemoveSleStreamType(streamDesc);
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> excludeDeviceDescs;
    excludeDeviceDescs.push_back(MakeDevice(fdp, OUTPUT_DEVICE));
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    outputStreamDescs.push_back(streamDesc);
    audioCoreService->DeactivateA2dpAfterExclude(excludeDeviceDescs, outputStreamDescs);
    audioCoreService->MutePrimaryOrOffloadSink(fdp.ConsumeBool() ? PRIMARY_SPEAKER : OFFLOAD_PRIMARY_SPEAKER,
        fdp.ConsumeIntegral<int64_t>());
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        AudioCoreServicePrivateRouteSelectorGapFuzzTest,
        AudioCoreServicePrivateFlagAndSceneGapFuzzTest,
        AudioCoreServicePrivateDebugGapFuzzTest,
        AudioCoreServicePrivateMoveGapFuzzTest,
        AudioCoreServicePrivateDupAndRemoteGapFuzzTest,
        AudioCoreServicePrivateOutputFetchGapFuzzTest,
        AudioCoreServicePrivateInputFetchGapFuzzTest,
        AudioCoreServicePrivateBluetoothAndOffloadGapFuzzTest,
    });
    func(fdp);
}
} // namespace
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
