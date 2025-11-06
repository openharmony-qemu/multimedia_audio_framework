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

#include "audio_log.h"
#include "../fuzz_utils.h"
#include "native_audio_suite_base.h"
#include "OHAudioSuiteEngine.h"
#include <thread>
#include <mutex>
#include <memory>

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint32_t THREADS_NUM = 3;
std::mutex g_getDataMutex;
std::mutex g_destroyMutex;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

static const uint32_t MAX_PIPELINE_NUM = 2;
static const uint32_t MAX_NODE_NUM = 5;
static const uint32_t MAX_FRAME_SIZE = 1000;
static const uint32_t MAX_USER_DATA_SIZE = 1000;
OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
OH_AudioNodeBuilder *builder = nullptr;
OH_AudioSuitePipeline *audioSuitePipeline[MAX_PIPELINE_NUM] = {nullptr};
OH_AudioNode *audioNode[MAX_NODE_NUM] = {nullptr};

typedef void (*TestPtr)();

template<class T>
T GetData()
{
    std::lock_guard<std::mutex> lock(g_getDataMutex);
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

static int32_t WriteDataCallback(OH_AudioNode *audioNode, void *userData,
    void *audioData, int32_t audioDataSize, bool *finished)
{
    if (finished != nullptr) {
        *finished = true;
    }
    return 1;
}

void AudioSuiteEngineCreateEngineFuzzTest()
{
    OH_AudioSuiteEngine_Create(&audioSuiteEngine);
}

void AudioSuiteEngineDestroyEngineFuzzTest()
{
    OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    audioSuiteEngine = nullptr;
}

void AudioSuiteEngineCreatePipelineFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    OH_AudioSuite_PipelineWorkMode workMode = GetData<OH_AudioSuite_PipelineWorkMode>();
    OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline[pipelineIndex], workMode);
}

void AudioSuiteEngineDestroyPipelineFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    OH_AudioSuiteEngine_DestroyPipeline(audioSuitePipeline[pipelineIndex]);
    audioSuitePipeline[pipelineIndex] = nullptr;
}

void AudioSuiteEngineStartPipelineFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    OH_AudioSuiteEngine_StartPipeline(audioSuitePipeline[pipelineIndex]);
}

void AudioSuiteEngineStopPipelineFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    OH_AudioSuiteEngine_StopPipeline(audioSuitePipeline[pipelineIndex]);
}

void AudioSuiteEngineGetPipelineStatePipelineFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    OH_AudioSuite_PipelineState pipelineState;
    OH_AudioSuiteEngine_GetPipelineState(audioSuitePipeline[pipelineIndex], &pipelineState);
}

void AudioSuiteEngineRenderFrameFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    uint8_t audioData[MAX_FRAME_SIZE] = {0};
    uint32_t frameSize = GetData<uint32_t>() % MAX_FRAME_SIZE;
    for (int i = 0; i < frameSize; i++) {
        audioData[i] = GetData<uint8_t>();
    }
    int32_t writeSize;
    bool finishedFlag;
    OH_AudioSuiteEngine_RenderFrame(audioSuitePipeline[pipelineIndex], audioData, frameSize, &writeSize, &finishedFlag);
}

void AudioSuiteEngineMultiRenderFrameFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    OH_AudioDataArray audioDataArray;
    uint8_t audioData[MAX_FRAME_SIZE] = {0};
    audioDataArray.arraySize = GetData<int32_t>() % MAX_FRAME_SIZE;
    for (int i = 0; i < audioDataArray.arraySize; i++) {
        audioData[i] = GetData<uint8_t>();
    }
    audioDataArray.audioDataArray = (void**)&audioData;
    audioDataArray.requestFrameSize = GetData<int32_t>();
    int32_t responseSize = GetData<int32_t>();
    bool finishedFlag = GetData<bool>();
    OH_AudioSuiteEngine_MultiRenderFrame(audioSuitePipeline[pipelineIndex], &audioDataArray,
        &responseSize, &finishedFlag);
}

void AudioSuiteEngineNodeBuilderCreateFuzzTest()
{
    OH_AudioSuiteNodeBuilder_Create(&builder);
}

void AudioSuiteEngineNodeBuilderDestroyFuzzTest()
{
    if (!g_destroyMutex.try_lock()) {
        return;
    }
    OH_AudioSuiteNodeBuilder_Destroy(builder);
    builder = nullptr;
    g_destroyMutex.unlock();
}

void AudioSuiteEngineNodeBuilderSetFormatFuzzTest()
{
    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = GetData<OH_Audio_SampleRate>();
    audioFormat.channelLayout = GetData<OH_AudioChannelLayout>();
    audioFormat.channelCount = GetData<int32_t>();
    audioFormat.encodingType = GetData<OH_Audio_EncodingType>();
    audioFormat.sampleFormat = GetData<OH_Audio_SampleFormat>();
    OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
}

void AudioSuiteEngineNodeBuilderSetRequestDataCallbackFuzzTest()
{
    uint8_t userData[MAX_FRAME_SIZE] = {0};
    uint32_t userDataSize = GetData<uint32_t>() % MAX_USER_DATA_SIZE;
    for (int i = 0; i < userDataSize; i++) {
        userData[i] = GetData<uint8_t>();
    }
    OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, WriteDataCallback, userData);
}

void AudioSuiteEngineCreateNodeFuzzTest()
{
    uint32_t pipelineIndex = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_AudioSuiteEngine_CreateNode(audioSuitePipeline[pipelineIndex], builder, &audioNode[nodeIndex]);
}

void AudioSuiteEngineDestroyNodeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_AudioSuiteEngine_DestroyNode(audioNode[nodeIndex]);
    audioNode[nodeIndex] = nullptr;
}

void AudioSuiteEngineGetNodeBypassStatusFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    bool bypassStatus;
    OH_AudioSuiteEngine_GetNodeBypassStatus(audioNode[nodeIndex], &bypassStatus);
}

void AudioSuiteEngineBypassEffectNodeStatusFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    bool bypassStatus = GetData<bool>();
    OH_AudioSuiteEngine_BypassEffectNode(audioNode[nodeIndex], bypassStatus);
}

void AudioSuiteEngineSetAudioFormat()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = GetData<OH_Audio_SampleRate>();
    audioFormat.channelLayout = GetData<OH_AudioChannelLayout>();
    audioFormat.channelCount = GetData<int32_t>();
    audioFormat.encodingType = GetData<OH_Audio_EncodingType>();
    audioFormat.sampleFormat = GetData<OH_Audio_SampleFormat>();
    OH_AudioSuiteEngine_SetAudioFormat(audioNode[nodeIndex], &audioFormat);
}

void AudioSuiteEngineConnectNodesFuzzTest()
{
    uint32_t srcNodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    uint32_t dstNodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_AudioSuiteEngine_ConnectNodes(audioNode[srcNodeIndex], audioNode[dstNodeIndex]);
}

void AudioSuiteEngineDisconnectNodesFuzzTest()
{
    uint32_t srcNodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    uint32_t dstNodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_AudioSuiteEngine_DisconnectNodes(audioNode[srcNodeIndex], audioNode[dstNodeIndex]);
}

void AudioSuiteEngineSetEqualizerFrequencyBandGainsFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_EqualizerFrequencyBandGains frequencyBandGains;
    for (int i = 0; i < EQUALIZER_BAND_NUM; i++) {
        frequencyBandGains.gains[i] = GetData<int32_t>();
    }
    OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(audioNode[nodeIndex], frequencyBandGains);
}

void AudioSuiteEngineSetSoundFieldTypeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_SoundFieldType soundFieldType = GetData<OH_SoundFieldType>();
    OH_AudioSuiteEngine_SetSoundFieldType(audioNode[nodeIndex], soundFieldType);
}

void AudioSuiteEngineSetEnvironmentTypeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_EnvironmentType environmentType = GetData<OH_EnvironmentType>();
    OH_AudioSuiteEngine_SetEnvironmentType(audioNode[nodeIndex], environmentType);
}

void AudioSuiteEngineSetVoiceBeautifierTypeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_VoiceBeautifierType voiceBeautifierType = GetData<OH_VoiceBeautifierType>();
    OH_AudioSuiteEngine_SetVoiceBeautifierType(audioNode[nodeIndex], voiceBeautifierType);
}

void AudioSuiteEngineGetEqualizerFrequencyBandGainsFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_EqualizerFrequencyBandGains frequencyBandGains;
    OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(audioNode[nodeIndex], &frequencyBandGains);
}

void AudioSuiteEngineGetSoundFieldTypeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_SoundFieldType soundFieldType;
    OH_AudioSuiteEngine_GetSoundFieldType(audioNode[nodeIndex], &soundFieldType);
}

void AudioSuiteEngineGetEnvironmentTypeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_EnvironmentType environmentType;
    OH_AudioSuiteEngine_GetEnvironmentType(audioNode[nodeIndex], &environmentType);
}

void AudioSuiteEngineGetVoiceBeautifierTypeFuzzTest()
{
    uint32_t nodeIndex = GetData<uint32_t>() % MAX_NODE_NUM;
    OH_VoiceBeautifierType voiceBeautifierType;
    OH_AudioSuiteEngine_GetVoiceBeautifierType(audioNode[nodeIndex], &voiceBeautifierType);
}

vector g_testFuncs = {
    AudioSuiteEngineCreateEngineFuzzTest,
    AudioSuiteEngineDestroyEngineFuzzTest,
    AudioSuiteEngineCreatePipelineFuzzTest,
    AudioSuiteEngineDestroyPipelineFuzzTest,
    AudioSuiteEngineStartPipelineFuzzTest,
    AudioSuiteEngineStopPipelineFuzzTest,
    AudioSuiteEngineGetPipelineStatePipelineFuzzTest,
    AudioSuiteEngineRenderFrameFuzzTest,
    AudioSuiteEngineMultiRenderFrameFuzzTest,
    AudioSuiteEngineNodeBuilderCreateFuzzTest,
    AudioSuiteEngineNodeBuilderDestroyFuzzTest,
    AudioSuiteEngineNodeBuilderSetFormatFuzzTest,
    AudioSuiteEngineNodeBuilderSetRequestDataCallbackFuzzTest,
    AudioSuiteEngineCreateNodeFuzzTest,
    AudioSuiteEngineDestroyNodeFuzzTest,
    AudioSuiteEngineGetNodeBypassStatusFuzzTest,
    AudioSuiteEngineBypassEffectNodeStatusFuzzTest,
    AudioSuiteEngineSetAudioFormat,
    AudioSuiteEngineConnectNodesFuzzTest,
    AudioSuiteEngineDisconnectNodesFuzzTest,
    AudioSuiteEngineSetEqualizerFrequencyBandGainsFuzzTest,
    AudioSuiteEngineSetSoundFieldTypeFuzzTest,
    AudioSuiteEngineSetEnvironmentTypeFuzzTest,
    AudioSuiteEngineSetVoiceBeautifierTypeFuzzTest,
    AudioSuiteEngineGetEqualizerFrequencyBandGainsFuzzTest,
    AudioSuiteEngineGetSoundFieldTypeFuzzTest,
    AudioSuiteEngineGetEnvironmentTypeFuzzTest,
    AudioSuiteEngineGetVoiceBeautifierTypeFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }
    uint32_t len = g_testFuncs.size();
    if (len <= 0) {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
        return false;
    }

    //initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code;
    std::vector<std::unique_ptr<std::thread>> threads;
    for (uint32_t i = 0; i < THREADS_NUM; i++) {
        code = GetData<uint32_t>();
        threads.emplace_back(std::make_unique<std::thread>(g_testFuncs[code % len]));
    }
    for (auto& t : threads) {
        t->join();
    }

    return true;
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}