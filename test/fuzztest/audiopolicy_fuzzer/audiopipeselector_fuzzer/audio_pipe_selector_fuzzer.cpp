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
const uint32_t DEVICE_SUPPORT_MMAP = 1;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
const std::string PIPE_CHANNELS = "2";

typedef void (*TestFuncs)();

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
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
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
};

const std::vector<AudioPipeAction> g_pipeActions = {
    PIPE_ACTION_DEFAULT,
    PIPE_ACTION_NEW,
    PIPE_ACTION_UPDATE,
    PIPE_ACTION_RELOAD,
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
    desc->SetDeviceSupportMmap(fdp.ConsumeBool() ? DEVICE_SUPPORT_MMAP : 0);
    return desc;
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
    pipeInfo->moduleInfo_.className = fdp.ConsumeBool() ? "remote_offload" : "name";
    return pipeInfo;
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

void CheckFastStreamOverLimitToNormalFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    size_t numStreams = fdp.ConsumeIntegralInRange<size_t>(0, MAX_TEST_STREAM_COUNT - 1);
    for (size_t i = 0; i < numStreams; ++i) {
        std::shared_ptr<AudioStreamDescriptor> streamDesc = MakeStreamDescriptor(fdp, false);
        streamDescs.push_back(streamDesc);
    }

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->CheckFastStreamOverLimitToNormal(streamDescs);
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

void MatchRemoteOffloadPipeFuzzTest(FuzzedDataProvider& fdp)
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
    audioPipeSelector->MatchRemoteOffloadPipe(streamPropInfo, pipeInfo, streamDesc);
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

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    GetPipeTypeFuzzTest,
    GetAdapterNameByStreamDescFuzzTest,
    ConvertStreamDescToPipeInfoFuzzTest,
    JudgeStreamActionFuzzTest,
    FetchPipeAndExecuteFuzzTest,
    FetchPipesAndExecuteFuzzTest,
    ProcessConcurrencyFuzzTest,
    ProcessRendererAndCapturerConcurrencyFuzzTest,
    CheckFastStreamOverLimitToNormalFuzzTest,
    IsSameAdapterFuzzTest,
    AddStreamToPipeAndUpdateActionFuzzTest,
    SetOriginalFlagForcedNormalIfNeedFuzzTest,
    FindExistingPipeFuzzTest,
    MatchRemoteOffloadPipeFuzzTest,
    IsPipeFormatMatchFuzzTest,
    UpdatePipeInfoFromStreamPropFuzzTest,
    IsPipeMatchFuzzTest,
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
