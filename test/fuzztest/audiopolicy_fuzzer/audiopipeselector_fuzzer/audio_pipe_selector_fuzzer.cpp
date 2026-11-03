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
#include "audio_policy_interface.h"
#include "audio_stream_descriptor.h"
#include "audio_concurrency_parser.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const int32_t MAX_TEST_STREAM_COUNT = 5;
const uint32_t MAX_SESSION_ID = 100000;
const int32_t MAX_TEST_ID = 10000;
const uint32_t MAX_PIPE_BUFFER_SIZE = 4096;
const uint32_t MAX_CONCURRENCY_ACTION = 3;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
const std::string PIPE_CHANNELS = "2";

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

const std::vector<DeviceType> g_outputDeviceTypes = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_NEARLINK,
};

const std::vector<DeviceType> g_inputDeviceTypes = {
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

const std::vector<AudioMode> g_audioModes = {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD,
};

const std::vector<AudioFlag> g_audioFlags = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_OUTPUT_FLAG_MULTICHANNEL,
    AUDIO_OUTPUT_FLAG_DIRECT,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
    AUDIO_INPUT_FLAG_VOIP,
    AUDIO_INPUT_FLAG_AI,
    AUDIO_INPUT_FLAG_ULTRASONIC,
    AUDIO_INPUT_FLAG_UNPROCESS,
    AUDIO_INPUT_FLAG_LIVE,
    AUDIO_INPUT_FLAG_VOICE_RECOGNITION,
    AUDIO_INPUT_FLAG_RAW_AI,
    AUDIO_INPUT_FLAG_INTERPHONE,
    AUDIO_INPUT_FLAG_CAMCORDER,
    AUDIO_INPUT_FLAG_VOICE_MESSAGE,
};

const std::vector<AudioSampleFormat> g_sampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const std::vector<AudioSamplingRate> g_samplingRates = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const std::vector<AudioChannel> g_channels = {
    MONO,
    STEREO,
    CHANNEL_4,
    CHANNEL_6,
};

const std::vector<AudioChannelLayout> g_channelLayouts = {
    CH_LAYOUT_MONO,
    CH_LAYOUT_STEREO,
    CH_LAYOUT_QUAD,
    CH_LAYOUT_5POINT1,
};

const std::vector<SourceType> g_sourceTypes = {
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_UNPROCESSED,
};

const std::vector<StreamUsage> g_streamUsages = {
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_VOICE_MESSAGE,
};

const std::vector<AudioPipeAction> g_pipeActions = {
    PIPE_ACTION_DEFAULT,
    PIPE_ACTION_NEW,
    PIPE_ACTION_UPDATE,
    PIPE_ACTION_RELOAD,
};

const std::vector<AudioStreamStatus> g_streamStatuses = {
    STREAM_STATUS_NEW,
    STREAM_STATUS_STARTED,
    STREAM_STATUS_PAUSED,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_RELEASED,
};

const std::vector<int32_t> g_routeSelectResults = {
    AudioRouteSelector::ROUTE_SELECT_RESULT_SUCCESS,
    AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE,
    AudioRouteSelector::ROUTE_SELECT_RESULT_FALLBACK_NORMAL,
};

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

std::string GetFormatString(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return "s8";
        case SAMPLE_S16LE:
            return "s16";
        case SAMPLE_S24LE:
            return "s24";
        case SAMPLE_S32LE:
            return "s32";
        case SAMPLE_F32LE:
            return "f32";
        default:
            return "s16";
    }
}

AudioStreamInfo MakeStreamInfo(FuzzedDataProvider& fdp)
{
    AudioStreamInfo streamInfo;
    streamInfo.format = PickValue(fdp, g_sampleFormats);
    streamInfo.samplingRate = PickValue(fdp, g_samplingRates);
    streamInfo.channels = PickValue(fdp, g_channels);
    streamInfo.channelLayout = PickValue(fdp, g_channelLayouts);
    streamInfo.encoding = ENCODING_PCM;
    return streamInfo;
}

std::shared_ptr<AudioDeviceDescriptor> MakeDeviceDescriptor(FuzzedDataProvider& fdp, DeviceRole role)
{
    const auto& deviceTypes = (role == INPUT_DEVICE) ? g_inputDeviceTypes : g_outputDeviceTypes;
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = fdp.ConsumeBool() ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_ID);
    if (fdp.ConsumeBool()) {
        DeviceStreamInfo mmapCap;
        mmapCap.fastSamplingRate.insert(AudioSamplingRate::SAMPLE_RATE_48000);
        mmapCap.ufastSamplingRate.insert(AudioSamplingRate::SAMPLE_RATE_48000);
        desc->audioStreamInfo_.push_back(mmapCap);
    }
    return desc;
}

AudioPipeRole GetPipeRoleByMode(AudioMode audioMode)
{
    return audioMode == AUDIO_MODE_RECORD ? PIPE_ROLE_INPUT : PIPE_ROLE_OUTPUT;
}

std::shared_ptr<AudioStreamDescriptor> MakeStreamDescriptor(FuzzedDataProvider& fdp, bool isInput)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_ = MakeStreamInfo(fdp);
    streamDesc->micInStreamInfo_ = MakeStreamInfo(fdp);
    streamDesc->ecStreamInfo_ = MakeStreamInfo(fdp);
    streamDesc->audioMode_ = isInput ? AUDIO_MODE_RECORD : AUDIO_MODE_PLAYBACK;
    streamDesc->audioFlag_ = PickValue(fdp, g_audioFlags);
    streamDesc->routeFlag_ = PickValue(fdp, g_audioFlags);
    streamDesc->oldRouteFlag_ = PickValue(fdp, g_audioFlags);
    streamDesc->futureRouteFlag_ = PickValue(fdp, g_audioFlags);
    streamDesc->sessionId_ = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    streamDesc->capturerInfo_.sourceType = PickValue(fdp, g_sourceTypes);
    streamDesc->capturerInfo_.capturerFlags = static_cast<int32_t>(PickValue(fdp, g_audioFlags));
    streamDesc->rendererInfo_.streamUsage = PickValue(fdp, g_streamUsages);
    streamDesc->rendererInfo_.rendererFlags = static_cast<int32_t>(PickValue(fdp, g_audioFlags));
    streamDesc->streamStatus_ = PickValue(fdp, g_streamStatuses);
    streamDesc->createTimeStamp_ = fdp.ConsumeIntegral<int64_t>();
    streamDesc->startTimeStamp_ = fdp.ConsumeIntegral<int64_t>();
    streamDesc->newDeviceDescs_.push_back(MakeDeviceDescriptor(fdp, isInput ? INPUT_DEVICE : OUTPUT_DEVICE));
    streamDesc->oldDeviceDescs_.push_back(MakeDeviceDescriptor(fdp, isInput ? INPUT_DEVICE : OUTPUT_DEVICE));
    return streamDesc;
}

std::shared_ptr<PipeStreamPropInfo> MakeStreamPropInfo(FuzzedDataProvider& fdp)
{
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = PickValue(fdp, g_sampleFormats);
    streamPropInfo->sampleRate_ = static_cast<uint32_t>(PickValue(fdp, g_samplingRates));
    streamPropInfo->channelLayout_ = PickValue(fdp, g_channelLayouts);
    streamPropInfo->channels_ = PickValue(fdp, g_channels);
    streamPropInfo->bufferSize_ = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_PIPE_BUFFER_SIZE);
    return streamPropInfo;
}

std::shared_ptr<AdapterPipeInfo> MakeAdapterPipeInfo(FuzzedDataProvider& fdp, AudioPipeRole role)
{
    auto pipeInfoPtr = std::make_shared<AdapterPipeInfo>();
    pipeInfoPtr->paProp_.lib_ = ConsumeString(fdp, "libmodule-hdi-sink.z.so");
    pipeInfoPtr->paProp_.role_ = ConsumeString(fdp, role == PIPE_ROLE_INPUT ? "source" : "sink");
    pipeInfoPtr->paProp_.moduleName_ = ConsumeString(fdp, "primary");
    pipeInfoPtr->name_ = ConsumeString(fdp, role == PIPE_ROLE_INPUT ? "primary_input" : "primary_output");
    pipeInfoPtr->role_ = role;
    auto adapterInfoPtr = std::make_shared<PolicyAdapterInfo>();
    adapterInfoPtr->adapterName = ConsumeString(fdp, "primary");
    pipeInfoPtr->adapterInfo_ = adapterInfoPtr;
    return pipeInfoPtr;
}

std::shared_ptr<AudioPipeInfo> MakeAudioPipeInfo(FuzzedDataProvider& fdp, AudioPipeRole role)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = role;
    pipeInfo->adapterName_ = ConsumeString(fdp, "primary");
    pipeInfo->name_ = ConsumeString(fdp, role == PIPE_ROLE_INPUT ? "primary_input" : "primary_output");
    pipeInfo->routeFlag_ = PickValue(fdp, g_audioFlags);
    pipeInfo->pipeAction_ = PickValue(fdp, g_pipeActions);
    pipeInfo->audioStreamInfo_ = MakeStreamInfo(fdp);
    pipeInfo->moduleInfo_.format = GetFormatString(pipeInfo->audioStreamInfo_.format);
    pipeInfo->moduleInfo_.rate = std::to_string(pipeInfo->audioStreamInfo_.samplingRate);
    pipeInfo->moduleInfo_.channels = std::to_string(pipeInfo->audioStreamInfo_.channels);
    pipeInfo->moduleInfo_.channelLayout = std::to_string(pipeInfo->audioStreamInfo_.channelLayout);
    pipeInfo->moduleInfo_.adapterName = pipeInfo->adapterName_;
    pipeInfo->moduleInfo_.name = pipeInfo->name_;
    pipeInfo->moduleInfo_.className = fdp.ConsumeBool() ? "remote_offload" : "name";
    return pipeInfo;
}

void AddStreamToPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (pipeInfo == nullptr || streamDesc == nullptr) {
        return;
    }
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeInfo->streamDescMap_[streamDesc->sessionId_] = streamDesc;
}

void GetPipeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioMode selectedAudioMode = PickValue(fdp, g_audioModes);
    uint32_t selectedFlag = PickValue(fdp, g_audioFlags);
    AudioPipeSelector::GetPipeSelector()->GetPipeType(selectedFlag, selectedAudioMode);
}

void GetAdapterNameByStreamDescFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::string adapterName;
    std::string pipeName;
    audioPipeSelector->GetAdapterNameByStreamDesc(streamDesc, adapterName, pipeName);
}

void ConvertStreamDescToPipeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    bool isInput = fdp.ConsumeBool();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, isInput);
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = MakeStreamPropInfo(fdp);
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = MakeAdapterPipeInfo(
        fdp, isInput ? PIPE_ROLE_INPUT : PIPE_ROLE_OUTPUT);
    streamPropInfo->pipeInfo_ = pipeInfoPtr;

    AudioPipeInfo info;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
}

void JudgeStreamActionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> newPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    std::vector<std::string> testStrings = {"test_adapter", "new_adapter", "old_adapter"};
    std::string newAdapterName = PickValue(fdp, testStrings);
    newPipe->adapterName_ = newAdapterName;
    AudioFlag newAudioFlag = PickValue(fdp, g_audioFlags);
    newPipe->routeFlag_ = newAudioFlag;
    std::vector<std::string> className = {
        "remote_offload",
        "name"
    };
    size_t idx = fdp.ConsumeIntegralInRange<size_t>(0, className.size() - 1);
    newPipe->moduleInfo_.className = className[idx];

    std::shared_ptr<AudioPipeInfo> oldPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    std::string oldAdapterName = PickValue(fdp, testStrings);
    oldPipe->adapterName_ = oldAdapterName;
    AudioFlag oldAudioFlag = PickValue(fdp, g_audioFlags);
    oldPipe->routeFlag_ = oldAudioFlag;
    idx = fdp.ConsumeIntegralInRange<size_t>(0, className.size() - 1);
    oldPipe->moduleInfo_.className = className[idx];

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
}

void FetchPipeAndExecuteFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipeAndExecute(streamDesc);
}

void FetchPipesAndExecuteFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT - 1);
    for (size_t i = 0; i < streamCount; ++i) {
        streamDescs.push_back(MakeStreamDescriptor(fdp, fdp.ConsumeBool()));
    }
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->FetchPipesAndExecute(streamDescs);
}

void ProcessConcurrencyFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> existingStream = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    CHECK_AND_RETURN(existingStream != nullptr);
    std::shared_ptr<AudioStreamDescriptor> incomingStream = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    CHECK_AND_RETURN(incomingStream != nullptr);
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    CHECK_AND_RETURN(audioPipeSelector != nullptr);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove;
    audioPipeSelector->ProcessConcurrency(existingStream, incomingStream, streamsToMove);
}

void ProcessRendererAndCapturerConcurrencyFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    ConcurrencyAction action =
        static_cast<ConcurrencyAction>(fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_CONCURRENCY_ACTION));
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->ProcessRendererAndCapturerConcurrency(streamDesc, action);
}

void HandleFastStreamLimitAndNearLinkStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    size_t numStreams = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT - 1);
    for (size_t i = 0; i < numStreams; ++i) {
        std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);
        streamDescs.push_back(streamDesc);
    }

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->HandleFastStreamLimitAndNearLinkState(streamDescs);
}

void IsSameAdapterFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDescA = MakeStreamDescriptor(fdp, false);
    streamDescA->newDeviceDescs_.front()->networkId_ =
        fdp.ConsumeBool() ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;

    std::shared_ptr<AudioStreamDescriptor> streamDescB = MakeStreamDescriptor(fdp, false);
    streamDescB->newDeviceDescs_.front()->networkId_ =
        fdp.ConsumeBool() ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->IsSameAdapter(streamDescA, streamDescB);
}

void AddStreamToPipeAndUpdateActionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamToAdd = MakeStreamDescriptor(fdp, fdp.ConsumeBool());

    std::shared_ptr<AudioPipeInfo> pipe = MakeAudioPipeInfo(
        fdp, fdp.ConsumeBool() ? PIPE_ROLE_INPUT : PIPE_ROLE_OUTPUT);

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->AddStreamToPipeAndUpdateAction(streamToAdd, pipe);
}

void SetOriginalFlagForcedNormalIfNeedFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->SetOriginalFlagForcedNormalIfNeed(streamDesc);
}

void FindExistingPipeFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioPipeInfo>> selectedPipeInfoList;
    size_t numPipes = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT - 1);
    for (size_t i = 0; i < numPipes; ++i) {
        std::shared_ptr<AudioPipeInfo> pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
        pipeInfo->name_ = "test_pipe";
        pipeInfo->adapterName_ = "test_adapter";
        selectedPipeInfoList.push_back(pipeInfo);
    }

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = MakeAdapterPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfoPtr->name_ = "test_pipe";
    pipeInfoPtr->role_ = PIPE_ROLE_OUTPUT;

    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);

    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = MakeStreamPropInfo(fdp);
    streamPropInfo->pipeInfo_ = pipeInfoPtr;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->FindExistingPipe(selectedPipeInfoList, pipeInfoPtr, streamDesc, streamPropInfo);
}

void MatchSpecialRemotePipeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = MakeStreamPropInfo(fdp);

    std::shared_ptr<AudioPipeInfo> pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfo->adapterName_ = fdp.ConsumeBool() ? "test_adapter" : REMOTE_NETWORK_ID;
    pipeInfo->moduleInfo_.channels = PIPE_CHANNELS;

    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = MakeAdapterPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfoPtr->name_ = "test_pipe";
    streamPropInfo->pipeInfo_ = pipeInfoPtr;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->MatchSpecialRemotePipe(streamPropInfo, pipeInfo, streamDesc);
}

void IsPipeFormatMatchFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = MakeStreamPropInfo(fdp);

    std::shared_ptr<AudioPipeInfo> pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->IsPipeFormatMatch(streamPropInfo, pipeInfo);
}

void UpdatePipeInfoFromStreamPropFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());

    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = MakeStreamPropInfo(fdp);

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = MakeAdapterPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfoPtr->name_ = "test_pipe";
    
    streamPropInfo->pipeInfo_ = pipeInfoPtr;

    AudioPipeInfo info;
    info.moduleInfo_.format = GetFormatString(PickValue(fdp, g_sampleFormats));

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->UpdatePipeInfoFromStreamProp(streamDesc, streamPropInfo, info);
}

void IsPipeMatchFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);
    streamDesc->newDeviceDescs_.front()->networkId_ =
        fdp.ConsumeBool() ? LOCAL_NETWORK_ID : REMOTE_NETWORK_ID;

    std::shared_ptr<AudioPipeInfo> pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfo->adapterName_ = "test_adapter";

    std::string adapterName = "test_adapter";
    std::string pipeName = "primary_output";

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->IsPipeMatch(streamDesc, pipeInfo, adapterName, pipeName);
}

void SetAndUnsetAudioRouteSelectorCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    if (fdp.ConsumeBool()) {
        int32_t retCode = fdp.ConsumeBool() ? SUCCESS : ERR_OPERATION_FAILED;
        int32_t result = PickValue(fdp, g_routeSelectResults);
        AudioPipeSelector::AudioRouteSelectorCallback callback =
            [retCode, result](const std::shared_ptr<AudioRouteSelectInfo> &info, int32_t &selectResult) {
                (void)info;
                selectResult = result;
                return retCode;
            };
        audioPipeSelector->SetAudioRouteSelectorCallback(callback);
    } else {
        audioPipeSelector->SetAudioRouteSelectorCallback(nullptr);
    }
    audioPipeSelector->UnsetAudioRouteSelectorCallback();
}

void BuildRouteSelectInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    audioPipeSelector->BuildRouteSelectInfo(streamDesc);
}

void NotifyAudioRouteSelectFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    int32_t retCode = fdp.ConsumeBool() ? SUCCESS : ERR_OPERATION_FAILED;
    int32_t result = PickValue(fdp, g_routeSelectResults);
    audioPipeSelector->SetAudioRouteSelectorCallback(
        [retCode, result](const std::shared_ptr<AudioRouteSelectInfo> &info, int32_t &selectResult) {
            (void)info;
            selectResult = result;
            return retCode;
        });
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    int32_t selectResult = PickValue(fdp, g_routeSelectResults);
    audioPipeSelector->NotifyAudioRouteSelect(streamDesc, selectResult);
    audioPipeSelector->UnsetAudioRouteSelectorCallback();
}

void ApplyRouteSelectDecisionFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    int32_t result = PickValue(fdp, g_routeSelectResults);
    audioPipeSelector->SetAudioRouteSelectorCallback(
        [result](const std::shared_ptr<AudioRouteSelectInfo> &info, int32_t &selectResult) {
            (void)info;
            selectResult = result;
            return SUCCESS;
        });
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    bool allowRejectCreate = fdp.ConsumeBool();
    audioPipeSelector->ApplyRouteSelectDecision(streamDesc, allowRejectCreate);
    audioPipeSelector->UnsetAudioRouteSelectorCallback();
}

void UpdateDeviceStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = MakeStreamPropInfo(fdp);
    audioPipeSelector->UpdateDeviceStreamInfo(streamDesc, streamPropInfo);
}

void UpdateStreamDescsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::unordered_set<uint32_t> originalStreamSessionIds;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT);
    for (size_t i = 0; i < streamCount; ++i) {
        auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
        streamDescs.push_back(streamDesc);
        if (fdp.ConsumeBool()) {
            originalStreamSessionIds.insert(streamDesc->sessionId_);
        }
    }
    audioPipeSelector->UpdateStreamDescs(streamDescs, originalStreamSessionIds);
}

void DecideFinalRouteFlagFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT);
    for (size_t i = 0; i < streamCount; ++i) {
        streamDescs.push_back(MakeStreamDescriptor(fdp, fdp.ConsumeBool()));
    }
    audioPipeSelector->DecideFinalRouteFlag(streamDescs);
}

void HandleFindMatchPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    auto pipeInfo = MakeAudioPipeInfo(fdp, GetPipeRoleByMode(streamDesc->audioMode_));
    newPipeInfoList.push_back(pipeInfo);
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    if (fdp.ConsumeBool()) {
        streamDescToOldPipeInfo[streamDesc->sessionId_] = pipeInfo;
    }
    auto matchPipeIter = newPipeInfoList.end();
    audioPipeSelector->HandleFindMatchPipe(newPipeInfoList, streamDesc, streamDescToOldPipeInfo, matchPipeIter);
}

void ProcessNewPipeListFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(1, MAX_TEST_STREAM_COUNT);
    for (size_t i = 0; i < streamCount; ++i) {
        auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
        auto pipeInfo = MakeAudioPipeInfo(fdp, GetPipeRoleByMode(streamDesc->audioMode_));
        streamDescs.push_back(streamDesc);
        newPipeInfoList.push_back(pipeInfo);
        if (fdp.ConsumeBool()) {
            streamDescToOldPipeInfo[streamDesc->sessionId_] = pipeInfo;
        }
    }
    audioPipeSelector->ProcessNewPipeList(newPipeInfoList, streamDescToOldPipeInfo, streamDescs);
}

void DecidePipesAndStreamActionFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;

    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    auto newPipe = MakeAudioPipeInfo(fdp, GetPipeRoleByMode(streamDesc->audioMode_));
    auto oldPipe = MakeAudioPipeInfo(fdp, GetPipeRoleByMode(streamDesc->audioMode_));
    AddStreamToPipe(newPipe, streamDesc);
    AddStreamToPipe(oldPipe, streamDesc);
    streamDescToOldPipeInfo[streamDesc->sessionId_] = oldPipe;
    newPipeInfoList.push_back(newPipe);
    oldPipeInfoList.push_back(oldPipe);
    audioPipeSelector->DecidePipesAndStreamAction(newPipeInfoList, streamDescToOldPipeInfo, oldPipeInfoList);
}

void HandlePipeNotExistFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    audioPipeSelector->HandlePipeNotExist(newPipeInfoList, streamDesc);
}

void SortStreamByStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::pair<std::shared_ptr<AudioStreamDescriptor>, std::shared_ptr<AudioPipeInfo>>> streamList;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT);
    for (size_t i = 0; i < streamCount; ++i) {
        auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
        auto pipeInfo = MakeAudioPipeInfo(fdp, GetPipeRoleByMode(streamDesc->audioMode_));
        streamList.emplace_back(streamDesc, pipeInfo);
    }
    audioPipeSelector->SortStreamByStatus(streamList);
}

void ScanPipeListForStreamDescFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    auto pipeInfo = MakeAudioPipeInfo(fdp, GetPipeRoleByMode(streamDesc->audioMode_));
    AddStreamToPipe(pipeInfo, MakeStreamDescriptor(fdp, streamDesc->audioMode_ == AUDIO_MODE_RECORD));
    pipeInfoList.push_back(pipeInfo);
    audioPipeSelector->ScanPipeListForStreamDesc(pipeInfoList, streamDesc);
}

void GetInputNormalPipeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t flag = fdp.ConsumeBool() ? PickValue(fdp, g_audioFlags) : fdp.ConsumeIntegral<uint32_t>();
    AudioPipeSelector::GetPipeSelector()->GetInputNormalPipeType(flag);
}

void SetPipeTypeByStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    audioPipeSelector->SetPipeTypeByStreamType(pipeType, streamDesc);
}

void GetIncomingStreamPriorityFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto existingStream = MakeStreamDescriptor(fdp, false);
    auto incomingStream = MakeStreamDescriptor(fdp, false);
    existingStream->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    incomingStream->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    existingStream->newDeviceDescs_.front()->networkId_ = REMOTE_NETWORK_ID;
    incomingStream->newDeviceDescs_.front()->networkId_ = REMOTE_NETWORK_ID;
    audioPipeSelector->GetIncomingStreamPriority(existingStream, incomingStream);
}

void DetermineConcurrencyActionFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto existingStream = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    auto incomingStream = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    audioPipeSelector->DetermineConcurrencyAction(existingStream, incomingStream);
}

void AdjustConcedeActionByStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto existingStream = MakeStreamDescriptor(fdp, false);
    auto incomingStream = MakeStreamDescriptor(fdp, false);
    ConcurrencyAction action = fdp.ConsumeBool() ? CONCEDE_INCOMING : CONCEDE_EXISTING;
    auto targetStream = action == CONCEDE_INCOMING ? existingStream : incomingStream;
    targetStream->rendererInfo_.isLoopback = fdp.ConsumeBool();
    targetStream->capturerInfo_.isLoopback = fdp.ConsumeBool();
    targetStream->routeFlag_ = fdp.ConsumeBool() ? (AUDIO_OUTPUT_FLAG_VOIP | AUDIO_OUTPUT_FLAG_DIRECT) :
        PickValue(fdp, g_audioFlags);
    audioPipeSelector->AdjustConcedeActionByStatus(existingStream, incomingStream, action);
}

void HandleConcedeActionsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto existingStream = MakeStreamDescriptor(fdp, false);
    auto incomingStream = MakeStreamDescriptor(fdp, false);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove;
    switch (fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_CONCURRENCY_ACTION)) {
        case CONCEDE_INCOMING:
            audioPipeSelector->HandleConcedeIncoming(existingStream, incomingStream);
            break;
        case CONCEDE_EXISTING:
            audioPipeSelector->HandleConcedeExisting(existingStream, incomingStream, streamsToMove);
            break;
        case CONCEDE_BOTH:
            audioPipeSelector->HandleConcedeBoth(existingStream, incomingStream, streamsToMove);
            break;
        default:
            break;
    }
}

void GetRouteFlagByStreamDescFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    audioPipeSelector->GetRouteFlagByStreamDesc(streamDesc);
}

void GetStreamPropInfoWithBusSelectorFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto streamDesc = MakeStreamDescriptor(fdp, fdp.ConsumeBool());
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    audioPipeSelector->GetStreamPropInfoWithBusSelector(streamDesc, streamPropInfo);
}

void SortStreamDescsByStartTimeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT);
    for (size_t i = 0; i < streamCount; ++i) {
        streamDescs.push_back(MakeStreamDescriptor(fdp, fdp.ConsumeBool()));
    }
    audioPipeSelector->SortStreamDescsByStartTime(streamDescs);
}

void MoveAndRemoveStreamsToNormalPipesFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto streamDesc = MakeStreamDescriptor(fdp, false);
    auto oldPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    oldPipe->adapterName_ = "primary";
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    AddStreamToPipe(oldPipe, streamDesc);
    auto normalPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    normalPipe->adapterName_ = "primary";
    normalPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove = {streamDesc};
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = {oldPipe, normalPipe};
    std::map<std::shared_ptr<AudioStreamDescriptor>, std::string> streamToAdapter;
    if (fdp.ConsumeBool()) {
        audioPipeSelector->RemoveTargetStreams(streamsToMove, pipeInfoList, streamToAdapter);
    } else {
        audioPipeSelector->MoveStreamsToNormalPipes(streamsToMove, pipeInfoList);
    }
}

void ProcessModemCommunicationConcurrencyFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsMoveToNormal;
    size_t streamCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT);
    for (size_t i = 0; i < streamCount; ++i) {
        streamDescs.push_back(MakeStreamDescriptor(fdp, fdp.ConsumeBool()));
    }
    audioPipeSelector->ProcessModemCommunicationConcurrency(streamDescs, streamsMoveToNormal);
}

void IsNeedTempMoveToNormalFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto streamDesc = MakeStreamDescriptor(fdp, false);
    streamDesc->streamStatus_ = fdp.ConsumeBool() ? STREAM_STATUS_STARTED : STREAM_STATUS_PAUSED;
    auto oldPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    streamDescToOldPipeInfo[streamDesc->sessionId_] = oldPipe;
    audioPipeSelector->IsNeedTempMoveToNormal(streamDesc, streamDescToOldPipeInfo);
}

void IsSpecialRemotePipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    pipeInfo->adapterName_ = fdp.ConsumeBool() ? "remote" : "primary";
    pipeInfo->routeFlag_ = fdp.ConsumeBool() ? AUDIO_OUTPUT_FLAG_LOWPOWER : AUDIO_OUTPUT_FLAG_NORMAL;
    AudioPipeSelector::GetPipeSelector()->IsSpecialRemotePipe(pipeInfo);
}

void MatchUltraFastPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto pipeInfo = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    auto streamDesc = MakeStreamDescriptor(fdp, false);
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeInfo->SetUltraFastFlag(fdp.ConsumeBool());
    AudioPipeSelector::GetPipeSelector()->MatchUltraFastPipe(pipeInfo, streamDesc);
}

void UpdateModuleInfoWithDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto deviceDesc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    if (fdp.ConsumeBool()) {
        deviceDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
        DeviceStreamInfo deviceStreamInfo;
        deviceStreamInfo.format = PickValue(fdp, g_sampleFormats);
        deviceStreamInfo.samplingRate = {PickValue(fdp, g_samplingRates)};
        deviceStreamInfo.SetChannels({PickValue(fdp, g_channels)});
        deviceDesc->audioStreamInfo_ = {deviceStreamInfo};
    }
    AudioModuleInfo moduleInfo;
    audioPipeSelector->UpdateModuleInfoWithDevice(deviceDesc, moduleInfo);
}

void UpdateRendererPipeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto streamDesc = MakeStreamDescriptor(fdp, false);
    AudioPipeSelector::GetPipeSelector()->UpdateRendererPipeInfo(streamDesc);
}

void SetCustomAudioMixFuzzTest(FuzzedDataProvider& fdp)
{
    std::string zoneName = ConsumeString(fdp, "zone");
    std::vector<AudioZoneMix> audioMixes;
    AudioPipeSelector::GetPipeSelector()->SetCustomAudioMix(zoneName, audioMixes);
}

void UpdateActionForHdPlayFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto existingStream = MakeStreamDescriptor(fdp, false);
    auto incomingStream = MakeStreamDescriptor(fdp, false);
    existingStream->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    incomingStream->routeFlag_ = fdp.ConsumeBool() ? AUDIO_OUTPUT_FLAG_LOWPOWER : AUDIO_OUTPUT_FLAG_NORMAL;
    existingStream->streamStatus_ = STREAM_STATUS_STARTED;
    incomingStream->streamStatus_ = STREAM_STATUS_STARTED;
    existingStream->newDeviceDescs_.front()->hdPlaybackMode_ = APP_LEVEL;
    incomingStream->newDeviceDescs_.front()->hdPlaybackMode_ = fdp.ConsumeBool() ? APP_LEVEL : DEVICE_LEVEL;
    ConcurrencyAction action = static_cast<ConcurrencyAction>(
        fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_CONCURRENCY_ACTION));
    audioPipeSelector->UpdateActionForHdPlay(existingStream, incomingStream, action);
    audioPipeSelector->IsUpdateActionForDeviceLevelHdPlay(existingStream, incomingStream);
}

void IsBothFastArmUsbNeedRecreateFuzzTest(FuzzedDataProvider& fdp)
{
    auto newPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    auto oldPipe = MakeAudioPipeInfo(fdp, PIPE_ROLE_OUTPUT);
    auto streamDesc = MakeStreamDescriptor(fdp, false);
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    streamDesc->oldDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    streamDesc->newDeviceDescs_.front()->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_ID);
    streamDesc->oldDeviceDescs_.front()->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_ID);
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    newPipe->moduleInfo_.className = "usb";
    oldPipe->moduleInfo_.className = "usb";
    AddStreamToPipe(newPipe, streamDesc);
    AudioPipeSelector::GetPipeSelector()->IsBothFastArmUsbNeedRecreate(newPipe, oldPipe);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    SetAndUnsetAudioRouteSelectorCallbackFuzzTest, BuildRouteSelectInfoFuzzTest,
    NotifyAudioRouteSelectFuzzTest, ApplyRouteSelectDecisionFuzzTest,
    GetPipeTypeFuzzTest, GetInputNormalPipeTypeFuzzTest,
    GetAdapterNameByStreamDescFuzzTest, GetRouteFlagByStreamDescFuzzTest,
    GetStreamPropInfoWithBusSelectorFuzzTest, ConvertStreamDescToPipeInfoFuzzTest,
    JudgeStreamActionFuzzTest, FetchPipeAndExecuteFuzzTest,
    FetchPipesAndExecuteFuzzTest, UpdateDeviceStreamInfoFuzzTest,
    UpdateStreamDescsFuzzTest, DecideFinalRouteFlagFuzzTest,
    ProcessConcurrencyFuzzTest, ProcessRendererAndCapturerConcurrencyFuzzTest,
    HandleFastStreamLimitAndNearLinkStateFuzzTest, SetPipeTypeByStreamTypeFuzzTest,
    GetIncomingStreamPriorityFuzzTest,
    DetermineConcurrencyActionFuzzTest,
    AdjustConcedeActionByStatusFuzzTest,
    HandleConcedeActionsFuzzTest,
    IsSameAdapterFuzzTest,
    HandleFindMatchPipeFuzzTest,
    ProcessNewPipeListFuzzTest,
    DecidePipesAndStreamActionFuzzTest,
    HandlePipeNotExistFuzzTest,
    SortStreamByStatusFuzzTest,
    ScanPipeListForStreamDescFuzzTest,
    AddStreamToPipeAndUpdateActionFuzzTest,
    SetOriginalFlagForcedNormalIfNeedFuzzTest,
    SortStreamDescsByStartTimeFuzzTest,
    MoveAndRemoveStreamsToNormalPipesFuzzTest,
    ProcessModemCommunicationConcurrencyFuzzTest,
    IsNeedTempMoveToNormalFuzzTest,
    FindExistingPipeFuzzTest,
    IsSpecialRemotePipeFuzzTest,
    MatchSpecialRemotePipeFuzzTest,
    MatchUltraFastPipeFuzzTest,
    UpdateModuleInfoWithDeviceFuzzTest,
    IsPipeFormatMatchFuzzTest,
    UpdatePipeInfoFromStreamPropFuzzTest,
    UpdateRendererPipeInfoFuzzTest,
    IsPipeMatchFuzzTest,
    SetCustomAudioMixFuzzTest,
    UpdateActionForHdPlayFuzzTest,
    IsBothFastArmUsbNeedRecreateFuzzTest,
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
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    (void)argc;
    (void)argv;
    OHOS::AudioStandard::Init();
    return 0;
}
