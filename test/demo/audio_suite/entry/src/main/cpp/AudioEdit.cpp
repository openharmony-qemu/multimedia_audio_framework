/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <string>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <new>
#include "napi/native_api.h"
#include <unistd.h>
#include "hilog/log.h"
#include "ohaudio/native_audiocapturer.h"
#include "ohaudio/native_audiorenderer.h"
#include "ohaudio/native_audiostreambuilder.h"
#include "ohaudio/native_audiostream_base.h"
#include "ohaudiosuite/native_audio_suite_base.h"
#include "ohaudiosuite/native_audio_suite_engine.h"
#include "NodeManager.h"
#include "audioEffectNode/EffectNode.h"
#include "audioEffectNode/VoiceBeautifier.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filemanagement/file_uri/oh_file_uri.h>
#include "callback/RegisterCallback.h"
#include "audioSuiteError/AudioSuiteError.h"
#include "audioEffectNode/Input.h"
#include "audioEffectNode/Output.h"
#include "audioEffectNode/ParseNapiParam.h"
#include "audioEffectNode/Equalizer.h"
#include "audioEffectNode/SoundField.h"
#include "audioEffectNode/NoiseReduction.h"
#include "audioEffectNode/EnvEffect.h"
#include "audioEffectNode/SpaceRender.h"
#include "audioEffectNode/AissEffect.h"
#include "audioEffectNode/SoundSpeedTone.h"
#include "audioEffectNode/VoiceChange.h"
#include "callback/RegisterCallbackNapi.h"
#include "./utils/Utils.h"
#include "realTimePlay/RealTimePlaying.h"
#include "multiPipelineEdit/MultiPipelineEdit.h"
#include "AudioRenderer.h"
#include "audioRecord/AudioRecord.h"
#include "timeline/Timeline_napi.h"
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <fcntl.h>
#include "ohaudiosuite/native_audio_converter.h"
#include <cmath>

const int GLOBAL_RESMGR = 0xFF00;
static const char *TAG = "[AudioEditTestApp_AudioEdit_cpp]";

const int MAX_PLAY_RESULT_BUFFER_SIZE = 1024 * 1024 * 1024;

const int SAMPLINGRATE_MULTI = 20;
const int CHANNELCOUNT_MULTI = 1000;
const int BITSPERSAMPLE_MULTI = 8;
const int INPUTNODES_SIZE2 = 2;
const int32_t BITDEPTH_MODE_INT = 0;
const int32_t BITDEPTH_MODE_FLOAT = 0;

OH_AudioConverter* g_converter = nullptr;
uint8_t *g_inputData = nullptr;
uint32_t g_inputDataLen = 0;
uint8_t *g_outputData = nullptr;  // Saving the Transformed Output Data
uint32_t g_outputDataLen = 0; // Length of the output data
uint32_t g_dataOffset = 0; // Offset of the read data
int32_t g_inputSampleRate = 0;
int32_t g_outputChannelCount = 0;
int32_t g_outputSampleFormat = 0;

const size_t MAX_FILE_SIZE = 2ULL * 1024 * 1024 * 1024;
const uint32_t WAV_HEADER_SIZE = 44;
const uint32_t DATA_CHUNK_SIZE = 400 * 1024;
const int32_t MAX_PROCESS_ATTEMPTS = 10000;
const uint32_t RETRY_BUFFER_SIZE = 256 * 1024;
const size_t MAX_OUTPUT_SIZE = 4ULL * 1024 * 1024 * 1024;
const size_t MAX_SINGLE_ALLOC = 500 * 1024 * 1024;
const int32_t BASE_BUFFER_SIZE_192K = 16 * 1024 * 1024;
const int32_t BASE_BUFFER_SIZE_96K = 8 * 1024 * 1024;
const int32_t BASE_BUFFER_SIZE_48K = 4 * 1024 * 1024;
const int32_t BASE_BUFFER_SIZE_LOW = 2 * 1024 * 1024;
const int32_t CHANNEL_COUNT_STEREO = 2;
const int32_t ZERO_OUTPUT_THRESHOLD = 10;

const int32_t AUDIOCONVERTER_ERROR_UNKNOWN_CODE = 5;
const int32_t FIRST_PROCESS_ATTEMPT = 1;

const int32_t BYTES_PER_KB = 1024;
const int32_t BYTES_PER_MB = 1024 * 1024;
const int32_t BUFFER_EXPANSION_FACTOR = 2;

const int32_t CHANNEL_COUNT_MONO = 1;
const int32_t CHANNEL_COUNT_2POINT1 = 3;
const int32_t CHANNEL_COUNT_QUAD = 4;
const int32_t CHANNEL_COUNT_5POINT0 = 5;
const int32_t CHANNEL_COUNT_5POINT1 = 6;
const int32_t CHANNEL_COUNT_7POINT0 = 7;
const int32_t CHANNEL_COUNT_7POINT1 = 8;

// Bits per sample constants
const int32_t BITS_PER_SAMPLE_8 = 8;
const int32_t BITS_PER_SAMPLE_16 = 16;
const int32_t BITS_PER_SAMPLE_24 = 24;
const int32_t BITS_PER_SAMPLE_32 = 32;

// Sample format multiplier constants
const int32_t SAMPLE_FORMAT_MULTIPLIER_1 = 1;
const int32_t SAMPLE_FORMAT_MULTIPLIER_2 = 2;

const int32_t ARGVNUM_0 = 0;
const int32_t ARGVNUM_1 = 1;
const int32_t ARGVNUM_2 = 2;
const int32_t ARGVNUM_3 = 3;

// File info structure
struct FileInfo {
    uint32_t samplingRate;
    uint16_t numChannels;
    int64_t channelLayout;
    int32_t sampleFormat;
    off_t fileSize;
};

// Process state structure
struct ProcessState {
    int32_t bufferSize;
    int32_t processCount;
    int32_t zeroOutputCount;
    bool hasError;
    bool retryWithSmallerBuffer;
};

const int32_t SAMPLE_RATE_THRESHOLD_HIGH = 192000;
const int32_t SAMPLE_RATE_THRESHOLD_MID = 96000;
const int32_t SAMPLE_RATE_THRESHOLD_LOW = 44100;

const int32_t WAV_RIFF_HEADER_SIZE = 12; // WAV RIFF: 12
const int32_t WAV_CHUNK_HEADER_SIZE = 8; // WAV: 8
const int32_t WAV_CHUNK_ID_SIZE = 4; // WAV: 4
const int32_t WAV_CHUNK_SIZE_OFFSET = 4; // WAV: 4

const int32_t WAV_AUDIO_FORMAT_OFFSET = 20; // WAV: 20
const int32_t WAV_SAMPLE_RATE_OFFSET = 24; // WAV: 24
const int32_t WAV_BLOCK_ALIGN_OFFSET = 32; // WAV: 32
const int32_t WAV_NUM_CHANNELS_OFFSET = 22; // WAV: 22
const int32_t WAV_BYTE_RATE_OFFSET = 28; // WAV: 28
const int32_t WAV_BITS_PER_SAMPLE_OFFSET = 34; // WAV: 34

const int32_t INDEXTWO = 2;
const int32_t INDEXTHREE = 3;
const int32_t INVALID_FD = -1;
const int32_t HUNDRED = 100;
const char DATA_CHUNK_ID[] = {'d', 'a', 't', 'a'};
const int32_t DATA_HEADER_INDEX[] = {'R', 'I', 'F', 'F'};
const int32_t DIRECTORY_PERMISSIONS = 0755;
const int32_t FILE_PERMISSIONS = 0644;
const int32_t TM_YEAR_BASE = 1900;
const int32_t TM_MONTH_OFFSET = 1;

// Timestamp format constants
const int32_t TIMESTAMP_YEAR_WIDTH = 4;
const int32_t TIMESTAMP_DATE_WIDTH = 2;

// Return codes
const int32_t RETURN_SUCCESS = 0;
const int32_t RETURN_FAILED = 1;

// Pipeline indices
const int32_t FIRST_PIPELINE_INDEX = 0;

// Threadsafe function parameters
const int32_t TSFN_INITIAL_QUEUE_SIZE = 0;
const int32_t TSFN_THREAD_COUNT = 1;

// Callback argument counts
const int32_t ARG_COUNT_1 = 1;
const int32_t ARG_COUNT_2 = 2;
const int32_t ARG_COUNT_3 = 3;
const int32_t ARG_COUNT_4 = 4;
const int32_t ARG_COUNT_5 = 5;

static napi_value AudioEditNodeInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioEditNodeInit start");
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // Parsing Work Mode

    // Create Engine
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_Create(&g_audioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OH_AudioEditEngine_Create result: %{public}d",
        static_cast<int>(result));
    // Determine the current work mode based on the input parameters
    unsigned int mode = -1;
    napi_status status = napi_get_value_uint32(env, argv[0], &mode);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest OH_AudioEditEngine_CreatePipeline"
            "napi_get_value_uint32 error: %{public}d", status);
        result = OH_AudioSuiteEngine_Destroy(g_audioSuiteEngine);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_audioSuiteEngine_Destroy result: %{public}d", static_cast<int>(result));
        delete[] argv;
        return nullptr;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---AudioEditNodeInit--workMode==%{public}d",
        mode);
    OH_AudioSuite_PipelineWorkMode workMode;
    if (mode == OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_EDIT_MODE) {
        workMode = OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_EDIT_MODE;
    } else if (mode == OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_REALTIME_MODE) {
        workMode = OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_REALTIME_MODE;
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioEditEngine_CreatePipeline workMode error: %{public}d", workMode);
        result = OH_AudioSuiteEngine_Destroy(g_audioSuiteEngine);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_audioSuiteEngine_Destroy result: %{public}d", static_cast<int>(result));
        delete[] argv;
        return nullptr;
    }
    // Create Pipeline
    result = OH_AudioSuiteEngine_CreatePipeline(g_audioSuiteEngine, &g_audioSuitePipeline, workMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioEditEngine_CreatePipeline result: %{public}d", static_cast<int>(result));
    // Instantiate NodeManager
    g_singlePipelineNodeManager = std::make_shared<NodeManager>(g_audioSuitePipeline);
    g_nodeManager = g_singlePipelineNodeManager;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest createNodeManager result: %{public}d",
        static_cast<int>(g_nodeManager->getAllNodes().size()));

    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    delete[] argv;
    return napiValue;
}

// deallocate memory
static void Clear()
{
    // Release the map memory
    g_writeDataBufferMap.clear();
    for (auto &pair : g_userDataMap) {
        delete pair.second; // Delete the object pointed to by the pointer
    }
    g_userDataMap.clear();
    Timeline::GetInstance().DeleteAllAudioTrack();
}

// release selected inputNode
static void ClearByInputId(const std::string& inputId, long startTime)
{
    auto it = g_writeDataBufferMap.find(inputId.c_str() + std::to_string(startTime));
    if (it != g_writeDataBufferMap.end()) {
        g_writeDataBufferMap.erase(it);
    }
    bool ret = Timeline::GetInstance().DeleteAudioTrack(inputId);
}

static napi_value AudioEditDestory(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioEditDestory start");
    Clear();
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_DestroyPipeline(g_audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_DestroyPipeline result: %{public}d", static_cast<int>(result));
    result = OH_AudioSuiteEngine_Destroy(g_audioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OH_audioSuiteEngine_Destroy result: %{public}d",
        static_cast<int>(result));
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

static napi_value SetFormat(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat start");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // Get Number of channels
    unsigned int channels;
    napi_get_value_uint32(env, argv[ARG_0], &channels);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat channels is %{public}d", channels);
    // Get Sampling Rate
    unsigned int sampleRate;
    napi_get_value_uint32(env, argv[ARG_1], &sampleRate);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat sampleRate is %{public}d", sampleRate);
    // Get bit depth
    unsigned int bitsPerSample;
    napi_get_value_uint32(env, argv[ARG_2], &bitsPerSample);
    // Get the bit depth type.
    unsigned int bitsPerSampleMode;
    napi_get_value_uint32(env, argv[ARG_3], &bitsPerSampleMode);
    ConvertBitsPerSample(bitsPerSample, bitsPerSampleMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest SetFormat bitsPerSample: %{public}d, bitsPerSampleMode: %{public}d",
        bitsPerSample, bitsPerSampleMode);

    // Set Sampling Rate
    g_audioFormatOutput.samplingRate = SetSamplingRate(sampleRate);
    // Set audio channels
    g_audioFormatOutput.channelCount = channels;
    g_audioFormatOutput.channelLayout = SetChannelLayout(channels);
    // Set bit depth
    g_audioFormatOutput.sampleFormat = SetSampleFormat(bitsPerSample);
    // Set the encoding format
    g_audioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    const std::vector<Node> outPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &g_audioFormatOutput);
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    delete[] argv;
    return napiValue;
}

static napi_value InitByPipelineCascad(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest InitByPipelineCascad start");
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    AudioParamsByCascad params;
    napi_status status = ParseArgumentsByCascad(env, argv, params);
    
    // Obtains the audio buffer
    void *pcmBuffer = nullptr;
    size_t pcmBufferSize = static_cast<size_t>(params.pcmBufferSize);
    status = napi_get_arraybuffer_info(env, argv[ARG_4], &pcmBuffer, &pcmBufferSize);
    g_totalSize = params.pcmBufferSize;
    if (g_totalBuff != nullptr) {
        free(g_totalBuff);
        g_totalBuff = nullptr;
    }
    g_totalBuff = (char *)malloc(g_totalSize);
    
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(status));
    }
    std::copy(static_cast<char *>(pcmBuffer), static_cast<char *>(pcmBuffer) + g_totalSize, g_totalBuff);
    
    napi_value napiValue;
    OH_AudioSuite_Result result;
    ManageInputNodes(env, params, result, napiValue);
    ManageOutputNodes(env, params.inputId, params.outputId, params.mixerId, result);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

// Import Audio Call
void UpdateRecordAudioParam(int sampleRate, int channels, int bitsPerSample)
{
    g_samplingRate = sampleRate;
    g_channelCount = channels;
    g_bitsPerSample = bitsPerSample;
}

static napi_value AudioInAndOutInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioInAndOutInit start");
    AudioParams params;
    napi_status status = ParseArguments(env, info, params);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(status));
    }
    OH_AVSource *source = OH_AVSource_CreateWithFD(params.fd, 0, params.fileLength);
    if (source == nullptr) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    OH_AVFormat *trackFormat = OH_AVSource_GetTrackFormat(source, 0);
    if (trackFormat == nullptr) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    // Sampling rate, channel, bit depth
    int32_t sampleRate;
    int32_t channels;
    int32_t bitsPerSample;
    if (!GetAudioProperties(trackFormat, &sampleRate, &channels, &bitsPerSample)) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    std::vector<std::string> audioFormat = {
        std::to_string(sampleRate), std::to_string(channels), std::to_string(bitsPerSample)
    };
    CallStringArrayCallback(audioFormat);
    // Create a corresponding unsealer for the resource instance
    UpdateRecordAudioParam(sampleRate, channels, bitsPerSample);
    OH_AVDemuxer *demuxer = OH_AVDemuxer_CreateWithSource(source);
    if (demuxer == nullptr) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    AudioFormat format{sampleRate, channels, bitsPerSample, params.startTime};
    RunAudioThread(demuxer, params.fileLength, params.inputId, format);
    napi_value napiValue;
    OH_AudioSuite_Result result;
    Node inputNode = g_nodeManager->GetNodeById(params.inputId);
    if (inputNode.id.empty()) {
        CreateInputNode(env, params.inputId, napiValue, result);
    } else {
        UpdateInputNodeParams updateInputNodeParams;
        updateInputNodeParams.inputId = params.inputId;
        updateInputNodeParams.channels = channels;
        updateInputNodeParams.sampleRate = sampleRate;
        updateInputNodeParams.bitsPerSample = bitsPerSample;
        UpdateInputNode(result, updateInputNodeParams);
        return ReturnResult(env, static_cast<AudioSuiteResult>(result));
    }
    ManageOutputNodes(env, params.inputId, params.outputId, params.mixerId, result);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

OH_AudioSuite_Result DeleteNodeOfSong(Node &node, int size)
{
    OH_AudioSuite_Result result = OH_AudioSuite_Result::AUDIOSUITE_SUCCESS;
    Node nextNode;
    if (size > INPUTNODES_SIZE2) {
        while (node.type != OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER) {
            nextNode = g_nodeManager->GetNodeById(node.nextNodeId);
            result = g_nodeManager->removeNode(node.id);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                return result;
            }
            node = nextNode;
        }
    } else if (size == INPUTNODES_SIZE2) {
        while (node.type != OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT) {
            nextNode = g_nodeManager->GetNodeById(node.nextNodeId);
            result = g_nodeManager->removeNode(node.id);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                return result;
            }
            node = nextNode;
        }
    } else {
        while (!node.id.empty()) {
            nextNode = g_nodeManager->GetNodeById(node.nextNodeId);
            result = g_nodeManager->removeNode(node.id);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                return result;
            }
            node = nextNode;
        }
    }
    return OH_AudioSuite_Result::AUDIOSUITE_SUCCESS;
}

// Delete audio
static napi_value DeleteSong(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // Get the inputId parameter
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong inputId is %{public}s",
        inputId.c_str());
    
    const std::vector<Node> inputNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong inputNodes length is %{public}d",
        static_cast<int>(inputNodes.size()));

    Node node = g_nodeManager->GetNodeById(inputId);
    Node nextNode;
    if (node.id.empty()) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        delete[] argv;
        return napiValue;
    }
    
    if (inputNodes.size() <= 0) {
        napi_create_int64(env, static_cast<int>(-1), &napiValue);
        delete[] argv;
        return napiValue;
    } else {
        result = DeleteNodeOfSong(node, inputNodes.size());
    }

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    delete[] argv;
    return napiValue;
}

// Delete Node
static napi_value DeleteNode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteNode start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // Get the nodeId parameter
    std::string nodeId;
    napi_status status = ParseNapiString(env, argv[0], nodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteNode nodeId is %{public}s",
        nodeId.c_str());
    
    result = g_nodeManager->removeNode(nodeId);

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    delete[] argv;
    return napiValue;
}

// Method for Setting Equalizer Mode
static napi_value SetEqualizerMode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEqualizerMode start");
    unsigned int equalizerMode = -1;
    std::string equalizerId;
    std::string inputId;
    napi_status status = GetEqModeParameters(env, info, equalizerMode, equalizerId, inputId);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }

    // Create Equalizer Effect Node
    Node eqNode = GetOrCreateEqualizerNodeByMode(equalizerId, inputId);
    if (!eqNode.physicalNode) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_CREATE_NODE_ERROR));
    }
    bool bypass = equalizerMode == 0;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_BypassEffectNode(eqNode.physicalNode, bypass);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---SetEqualizerMode OH_AudioSuiteEngine_BypassEffectNode ERROR %{public}zd", result);
        return ReturnResult(env, static_cast<AudioSuiteResult>(result));
    }
    if (bypass) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(result));
    }
    result =
        OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode.physicalNode, SetEqualizerMode(equalizerMode));
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

// Sets the equalizer band gain
static napi_value SetEqualizerFrequencyBandGains(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEqualizerFrequencyBandGains start");
    OH_EqualizerFrequencyBandGains frequencyBandGains;
    EqBandGainsParams params;
    napi_status status = GetEqBandGainsParameters(env, info, frequencyBandGains, params);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }
    // Create Equalizer Effect Node
    Node eqNode = GetOrCreateEqualizerNodeByGains(params.equalizerId, params.inputId, params.selectedNodeId);
    if (!eqNode.physicalNode) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_CREATE_NODE_ERROR));
    }

    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode.physicalNode, frequencyBandGains);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

// Set up the speed and pitch effect node
static napi_value SetSoundSpeedTone(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetSoundSpeedTone start");
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    SoundSpeedToneParams params;
    napi_status status = GetSoundSpeedToneParameters(env, argv, params);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }
    // Create Sonic Sound Adjustment Point
    Node soundSpeedToneNode = GetOrCreateSpeedToneNode(params.soundSpeedToneId, params.inputId, params.selectedNodeId);
    if (!soundSpeedToneNode.physicalNode) {
        return ReturnResult(env, AudioSuiteResult::DEMO_CREATE_NODE_ERROR);
    }
    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetTempoAndPitch(soundSpeedToneNode.physicalNode, params.soundSpeed, params.soundTone);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

static napi_value SaveFileBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SaveFileBuffer start");
    ResetAllIsResetTotalWriteAudioDataSize();
    RenDerFrame();

    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    napi_status status = napi_create_arraybuffer(env, g_totalSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame status: %{public}d", static_cast<int>(status));
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        // Failed to create ArrayBuffer; returned an ArrayBuffer with a size of 0
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        std::copy(g_totalBuff, g_totalBuff + g_totalSize, static_cast<char *>(arrayBufferData));
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        return napiValue;
    }
}

static napi_value startVBEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---IN");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // inputId
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---inputId==%{public}s",
                 inputId.c_str());
    // Get parameters 2, beautify types
    int mode = -1;
    napi_get_value_int32(env, argv[ARG_1], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect--mode==%{public}zd", mode);
    // Get parameters 3 and effect node ID
    std::string voiceBeautifierId;
    status = ParseNapiString(env, argv[ARG_2], voiceBeautifierId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", voiceBeautifierId.c_str());
    // Get the ID of the currently selected node
    std::string selectNodeId;
    status = ParseNapiString(env, argv[ARG_3], selectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---selectNodeId==%{public}s",
                 selectNodeId.c_str());
     // Invoke the interface for adding beautification effects node
    napi_value ret;
    int result = AddVBEffectNode(inputId, mode, voiceBeautifierId, selectNodeId);

    napi_create_int64(env, result, &ret);
    delete[] argv;
    return ret;
}
static napi_value resetVBEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetVBEffect---IN");

    int mode = -1;
    std::string inputId;
    std::string voiceBeautifierId;
    // Parsing Parameters
    napi_status status = getResetVBParameters(env, info, inputId, mode, voiceBeautifierId);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }
    napi_value ret;
    int result = ModifyVBEffectNode(inputId, mode, voiceBeautifierId);
    napi_create_int64(env, result, &ret);
    return ret;
}

static napi_status ParseFieldEffectParams(napi_env env, napi_callback_info info, FieldEffectParams& params)
{
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status = ParseNapiString(env, argv[ARG_0], params.inputId);
    napi_get_value_uint32(env, argv[ARG_1], &params.mode);
    status = ParseNapiString(env, argv[ARG_2], params.fieldEffectId);
    status = ParseNapiString(env, argv[ARG_3], params.selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest FieldEffect inputId==%{public}s mode==%{public}zd, " \
        "fieldEffectId==%{public}s, selectedNodeId==%{public}s", \
        params.inputId.c_str(),
        params.mode,
        params.fieldEffectId.c_str(),
        params.selectedNodeId.c_str());
    delete[] argv;
    return status;
}

static napi_value startFieldEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startFieldEffect start");
    FieldEffectParams params;
    napi_status status = ParseFieldEffectParams(env, info, params);
    OH_SoundFieldType type = getSoundFieldTypeByNum(params.mode);
    napi_value ret;
    Node node = CreateNodeByType(params.fieldEffectId, OH_AudioNode_Type::EFFECT_NODE_TYPE_SOUND_FIELD);
    bool bypass = params.mode == 0;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_BypassEffectNode(node.physicalNode, bypass);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---startFieldEffect OH_AudioSuiteEngine_BypassEffectNode ERROR %{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }
    if (bypass) {
        napi_create_int64(env, result, &ret);
        return ret;
    }
    result = OH_AudioSuiteEngine_SetSoundFieldType(node.physicalNode, type);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest startFieldEffect OH_AudioEditEngine_SetSoundFiledType ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }
    if (params.selectedNodeId.empty()) {
        int res = AddEffectNodeToNodeManager(params.inputId, params.fieldEffectId);
        if (res != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest startFieldEffect AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, res, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(params.fieldEffectId, params.selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, result, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect: operation success");
    return ret;
}

static napi_status ParseResetFieldEffectParams(napi_env env, napi_callback_info info,
    std::string& inputId, unsigned int& mode, std::string& fieldEffectId)
{
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect inputId is %{public}s",
        inputId.c_str());
    napi_get_value_uint32(env, argv[ARG_1], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect mode is %{public}zd", mode);
    status = ParseNapiString(env, argv[ARG_2], fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest fieldEffectId is %{public}s",
        fieldEffectId.c_str());
    delete[] argv;
    return status;
}

static napi_value resetFieldEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect start");
    std::string inputId;
    unsigned int mode = -1;
    std::string fieldEffectId;
    napi_status status = ParseResetFieldEffectParams(env, info, inputId, mode, fieldEffectId);

    OH_SoundFieldType type = getSoundFieldTypeByNum(mode);

    napi_value ret;
    Node node = g_nodeManager->GetNodeById(fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest get node is %{public}s", node.id.c_str());
    bool bypass = mode == 0;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_BypassEffectNode(node.physicalNode, bypass);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---resetFieldEffect OH_AudioSuiteEngine_BypassEffectNode ERROR %{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }
    if (bypass) {
        napi_create_int64(env, result, &ret);
        return ret;
    }
    result = OH_AudioSuiteEngine_SetSoundFieldType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_SetSoundFieldType ERROR %{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, result, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect: operation success");
    return ret;
}

static napi_value getAudioOfTap(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---getAudioOfTap---IN");

    napi_value napiValue = nullptr;
    void *data;
    napi_create_arraybuffer(env, g_tapDataTotalSize, &data, &napiValue);
    std::copy(
        reinterpret_cast<const char*>(g_tapTotalBuff),
        reinterpret_cast<const char*>(g_tapTotalBuff) + g_tapDataTotalSize,
        reinterpret_cast<char*>(data));
    std::fill(
        reinterpret_cast<char*>(g_tapTotalBuff),
        reinterpret_cast<char*>(g_tapTotalBuff) + g_tapDataTotalSize,
        0);
    g_tapDataTotalSize = 0;
    return napiValue;
}

static napi_value Record(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest Record start");
    g_isRecord = true;
    return nullptr;
}

static napi_value RealTimeSaveFileBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest RealTimeSaveFileBuffer start");
    ResetAllIsResetTotalWriteAudioDataSize();
    g_isRecord = false;
    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest RealTimeSaveFileBuffer g_playResultTotalSize is %{public}d", g_playResultTotalSize);
    napi_status status = napi_create_arraybuffer(env, g_playResultTotalSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest napi_create_arraybuffer status: %{public}d",
            static_cast<int>(status));
        g_playResultTotalSize = 0;
        if (g_playTotalAudioData != nullptr) {
            free(g_playTotalAudioData);
            g_playTotalAudioData = nullptr;
        }
        // Failed to create ArrayBuffer; returned an ArrayBuffer with a size of 0
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        std::copy(g_playTotalAudioData, g_playTotalAudioData + g_playResultTotalSize,
            static_cast<char *>(arrayBufferData));
        if (g_playTotalAudioData != nullptr) {
            free(g_playTotalAudioData);
            g_playTotalAudioData = nullptr;
        }
        g_playResultTotalSize = 0;
        return napiValue;
    }
}

static napi_value AudioRendererInit(napi_env env, napi_callback_info info)
{
    ReleaseExistingResources();
    // Create Constructor
    OH_AudioStream_Type type = OH_AudioStream_Type::AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStreamBuilder_Create(&rendererBuilder, type);

    // Get bit depth
    int32_t bitsPerSample = 0;
    OH_AudioStream_SampleFormat streamSampleFormat;
    GetBitsPerSampleAndStreamFormat(g_audioFormatOutput, &bitsPerSample, &streamSampleFormat);

    // Set the audio sampling rate
    OH_AudioStreamBuilder_SetSamplingRate(rendererBuilder, g_audioFormatOutput.samplingRate);
    // Set audio channels
    OH_AudioStreamBuilder_SetChannelCount(rendererBuilder, g_audioFormatOutput.channelCount);
    // Set the audio sampling format
    OH_AudioStreamBuilder_SetSampleFormat(rendererBuilder, streamSampleFormat);
    // Set the encoding type for the audio stream
    OH_AudioStreamBuilder_SetEncodingType(rendererBuilder, AUDIOSTREAM_ENCODING_TYPE_RAW);
    // Set up the working scenario for outputting audio streams
    OH_AudioStreamBuilder_SetRendererInfo(rendererBuilder, AUDIOSTREAM_USAGE_MUSIC);
    // Set the length of audioDataSize (the size of the data to be played)
    g_playDataSize = SAMPLINGRATE_MULTI * g_audioFormatOutput.samplingRate *
        g_audioFormatOutput.channelCount * bitsPerSample / BITSPERSAMPLE_MULTI / CHANNELCOUNT_MULTI;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AudioRendererInit g_playDataSize: %{public}d, samplingRate: %{public}d, "
        "channelCount: %{public}d, bitsPerSample: %{public}d",
        g_playDataSize, g_audioFormatOutput.samplingRate, g_audioFormatOutput.channelCount, bitsPerSample);
    OH_AudioStreamBuilder_SetFrameSizeInCallback(rendererBuilder, g_playDataSize);

    // Configure the callback function for writing audio data
    OH_AudioRenderer_OnWriteDataCallback rendererCallbacks = PlayAudioRendererOnWriteData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(rendererBuilder, rendererCallbacks, nullptr);

    // create OH_AudioRenderer
    OH_AudioStreamBuilder_GenerateRenderer(rendererBuilder, &audioRenderer);
    return nullptr;
}

static napi_value AudioRendererDestory(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    if (audioRenderer) {
        // Releasing a Playback Instance
        OH_AudioStream_Result result = OH_AudioRenderer_Release(audioRenderer);
        // Release Constructor
        result = OH_AudioStreamBuilder_Destroy(rendererBuilder);
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        audioRenderer = nullptr;
        rendererBuilder = nullptr;
    }
    return napiValue;
}

// Start playing
static napi_value AudioRendererStart(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "g_writeDataBufferMap size %{public}d",
        g_writeDataBufferMap.size());
    ProcessPipeline();
    g_playFinishedFlag = false;
    g_playResultTotalSize = 0;

    if (g_playTotalAudioData != nullptr) {
        free(g_playTotalAudioData);
        g_playTotalAudioData = nullptr;
    }
    g_playTotalAudioData = (char *)calloc(1, MAX_PLAY_RESULT_BUFFER_SIZE);

    // start
    OH_AudioRenderer_Start(audioRenderer);
    return nullptr;
}

// pause playback
static napi_value AudioRendererPause(napi_env env, napi_callback_info info)
{
    // pause
    OH_AudioRenderer_Pause(audioRenderer);
    return nullptr;
}

// Stop playing
static napi_value AudioRendererStop(napi_env env, napi_callback_info info)
{
    // stop
    OH_AudioRenderer_Stop(audioRenderer);
    // Stop pipeline
    OH_AudioSuiteEngine_StopPipeline(g_audioSuitePipeline);
    return nullptr;
}

// Get the playback status.
static napi_value GetRendererState(napi_env env, napi_callback_info info)
{
    OH_AudioStream_State state;
    OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    napi_value sum;
    napi_create_int32(env, state, &sum);

    return sum;
}

// Whether to reset totalWriteAudioDataSize
static napi_value ResetTotalWriteAudioDataSize(napi_env env, napi_callback_info info)
{
    // Buffer for writing audio, starting from the beginning
    ResetAllIsResetTotalWriteAudioDataSize();
    // Save the audio that reported the error from the beginning again
    g_playResultTotalSize = 0;
    return nullptr;
}

// Get the effect node options.
static napi_value getOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value napiValue;
    
    // Get nodeId
    std::string nodeId;
    ParseNapiString(env, argv[0], nodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "getOptions nodeId is %{public}s", nodeId.c_str());
    Node node = g_nodeManager->GetNodeById(nodeId);
    // Get effect parameters based on different effect types
    std::string type = g_nodeManager->GetOptionsByType(node);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "getOptions type is %{public}s", type.c_str());
    napi_create_string_utf8(env, type.c_str(), NAPI_AUTO_LENGTH, &napiValue);
    delete[] argv;
    return napiValue;
}

static napi_value getEffectNodeList(napi_env env, napi_callback_info info)
{
    // Returns the JS array
    return GetSupportedAudioNodeTypes(env);
}

static napi_value SetIsRecord(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetIsRecord start");
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
 
    bool isRecord;
    napi_status status = napi_get_value_bool(env, argv[ARG_0], &isRecord);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "SetIsRecord status: %{public}d", static_cast<int>(status));
        delete[] argv;
        return nullptr;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "SetIsRecord isRecord: %{public}s", isRecord ? "true" : "false");
    g_isRecord = isRecord;
    if (isRecord) {
        g_playResultTotalSize = 0;
    }
    delete[] argv;
    return nullptr;
}
 
static napi_value SetSeparationMode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetSeparationMode start");
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
 
    napi_status status = napi_get_value_uint32(env, argv[0], &g_separationMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "SetSeparationMode g_separationMode: %{public}d", g_separationMode);
    delete[] argv;
    return ReturnResult(env, static_cast<AudioSuiteResult>(status));
}

static napi_value clear(napi_env env, napi_callback_info info)
{
    Clear();
    return nullptr;
}

static napi_value clearByInputId(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
 
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    int64_t startTime = 0;
    status = napi_get_value_int64(env, argv[ARG_1], &startTime);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "inputId is: %{public}s", inputId.c_str());
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "SetIsRecord status: %{public}d", static_cast<int>(status));
        delete[] argv;
        return nullptr;
    }
    ClearByInputId(inputId, startTime);
    delete[] argv;
    return nullptr;
}

static napi_value ModifyRender(napi_env env, napi_callback_info info)
{
    return ModifyRenderTrack(env, info);
}

static napi_value stopPipeline(napi_env env, napi_callback_info info)
{
    // Shut down the pipeline
    OH_AudioSuiteEngine_StopPipeline(g_audioSuitePipeline);
    return nullptr;
}

static napi_value setCurrentTime(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    long currentTime = 0;
    napi_status status = napi_get_value_int64(env, argv[0], &currentTime);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "setCurrentTime g_currentTime: %{public}ld", currentTime);
    Timeline::GetInstance().ResetCurrent(currentTime);
    return nullptr;
}

static napi_value SetEffectNodeBypass(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value result;
    napi_get_boolean(env, false, &result);
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[ARG_0], inputId);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "SetEffectNodeBypass status: %{public}d", static_cast<int>(status));
        delete[] argv;
        return result;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "inputId is: %{public}s", inputId.c_str());
    std::string effectNodeId;
    status = ParseNapiString(env, argv[ARG_1], effectNodeId);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "SetEffectNodeBypass status: %{public}d", static_cast<int>(status));
        delete[] argv;
        return result;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "effectNodeId is: %{public}s", effectNodeId.c_str());
    bool isBypass = false;
    status = napi_get_value_bool(env, argv[ARG_2], &isBypass);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "SetEffectNodeBypass status: %{public}d", static_cast<int>(status));
        delete[] argv;
        return result;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "isBypass is: %{public}d", isBypass);
    delete[] argv;
    Node effectNode = g_nodeManager->GetNodeById(effectNodeId);
    if (!effectNode.physicalNode) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "SetEffectNodeBypass get effectNode error, effectNodeId: %{public}s", effectNodeId.c_str());
        return result;
    }
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_BypassEffectNode(effectNode.physicalNode, isBypass);
    if (ret != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "SetEffectNodeBypass OH_AudioSuiteEngine_BypassEffectNode ERROR %{public}u", ret);
        return result;
    }
    napi_get_boolean(env, true, &result);
    return result;
}

// OH_AudioConverter_Create NAPI binding
// Parse audio format from NAPI object
static bool ParseAudioFormat(napi_env env, napi_value formatObj, OH_AudioConverter_Format &format)
{
    napi_value encodingTypeProp;
    napi_get_named_property(env, formatObj, "encodingType", &encodingTypeProp);
    napi_status status = napi_get_value_int32(env, encodingTypeProp, reinterpret_cast<int32_t*>(&format.encodingType));
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse encodingType failed");
        return false;
    }

    napi_value samplingRateProp;
    napi_get_named_property(env, formatObj, "samplingRate", &samplingRateProp);
    int32_t samplingRateValue;
    status = napi_get_value_int32(env, samplingRateProp, &samplingRateValue);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse samplingRate failed");
        return false;
    }
    format.samplingRate = static_cast<OH_Audio_SampleRate>(samplingRateValue);

    napi_value channelLayoutProp;
    napi_get_named_property(env, formatObj, "channelLayout", &channelLayoutProp);
    status = napi_get_value_int64(env, channelLayoutProp, reinterpret_cast<int64_t*>(&format.channelLayout));
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse channelLayout failed");
        return false;
    }

    napi_value sampleFormatProp;
    napi_get_named_property(env, formatObj, "sampleFormat", &sampleFormatProp);
    status = napi_get_value_int32(env, sampleFormatProp, reinterpret_cast<int32_t*>(&format.sampleFormat));
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse sampleFormat failed");
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "Format: encoding=%{public}d, rate=%{public}d, layout=%{public}lld, format=%{public}d",
                 static_cast<int>(format.encodingType), static_cast<int>(format.samplingRate),
                 static_cast<long long>(format.channelLayout), static_cast<int>(format.sampleFormat));

    return true;
}

// Calculate output channel count from channel layout
static int32_t CalculateOutputChannelCount(OH_AudioChannelLayout channelLayout)
{
    switch (channelLayout) {
        case CH_LAYOUT_MONO:
            return CHANNEL_COUNT_MONO;
        case CH_LAYOUT_STEREO:
        case CH_LAYOUT_STEREO_DOWNMIX:
            return CHANNEL_COUNT_STEREO;
        case CH_LAYOUT_2POINT1:
        case CH_LAYOUT_3POINT0:
        case CH_LAYOUT_SURROUND:
            return CHANNEL_COUNT_2POINT1;
        case CH_LAYOUT_3POINT1:
        case CH_LAYOUT_4POINT0:
        case CH_LAYOUT_QUAD:
        case CH_LAYOUT_QUAD_SIDE:
        case CH_LAYOUT_2POINT0POINT2:
            return CHANNEL_COUNT_QUAD;
        case CH_LAYOUT_4POINT1:
        case CH_LAYOUT_5POINT0:
        case CH_LAYOUT_5POINT0_BACK:
        case CH_LAYOUT_2POINT1POINT2:
        case CH_LAYOUT_3POINT0POINT2:
            return CHANNEL_COUNT_5POINT0;
        case CH_LAYOUT_5POINT1:
        case CH_LAYOUT_5POINT1_BACK:
        case CH_LAYOUT_6POINT0:
        case CH_LAYOUT_3POINT1POINT2:
        case CH_LAYOUT_6POINT0_FRONT:
            return CHANNEL_COUNT_5POINT1;
        case CH_LAYOUT_HEXAGONAL:
        case CH_LAYOUT_6POINT1:
        case CH_LAYOUT_6POINT1_BACK:
        case CH_LAYOUT_6POINT1_FRONT:
        case CH_LAYOUT_7POINT0:
        case CH_LAYOUT_7POINT0_FRONT:
            return CHANNEL_COUNT_7POINT0;
        case CH_LAYOUT_5POINT1POINT2:
        case CH_LAYOUT_7POINT1:
        case CH_LAYOUT_OCTAGONAL:
        case CH_LAYOUT_7POINT1_WIDE:
        case CH_LAYOUT_7POINT1_WIDE_BACK:
            return CHANNEL_COUNT_7POINT1;
        default:
            return CHANNEL_COUNT_STEREO;
    }
}

// Parse WAV file and extract data information
static bool ParseWavFile(int fd, off_t fileSize, uint32_t &wavDataSize, uint32_t &wavDataStart)
{
    uint8_t header[WAV_HEADER_SIZE];
    int headerRead = read(fd, header, WAV_HEADER_SIZE);
    if (headerRead < WAV_HEADER_SIZE) {
        return false;
    }

    if (header[0] != DATA_HEADER_INDEX[0] || header[1] != DATA_HEADER_INDEX[1] ||
        header[INDEXTWO] != DATA_HEADER_INDEX[INDEXTWO] || header[INDEXTHREE] != DATA_HEADER_INDEX[INDEXTHREE]) {
        return false;
    }
    
    lseek(fd, WAV_RIFF_HEADER_SIZE, SEEK_SET);
    bool foundDataChunk = false;
    uint8_t chunkHeader[WAV_CHUNK_HEADER_SIZE];

    while (!foundDataChunk) {
        int bytesRead = read(fd, chunkHeader, WAV_CHUNK_HEADER_SIZE);
        if (bytesRead < WAV_CHUNK_HEADER_SIZE) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Failed to find data chunk");
            break;
        }

        // Check if it's a data chunk
        if (chunkHeader[0] == DATA_CHUNK_ID[0] && chunkHeader[1] == DATA_CHUNK_ID[1] &&
            chunkHeader[INDEXTWO] == DATA_CHUNK_ID[INDEXTWO] && chunkHeader[INDEXTHREE] == DATA_CHUNK_ID[INDEXTHREE]) {
            foundDataChunk = true;
            wavDataSize = *reinterpret_cast<uint32_t*>(&chunkHeader[WAV_CHUNK_SIZE_OFFSET]);
            wavDataStart = lseek(fd, 0, SEEK_CUR);
        } else {
            uint32_t chunkSize = *reinterpret_cast<uint32_t*>(&chunkHeader[WAV_CHUNK_SIZE_OFFSET]);
            lseek(fd, chunkSize, SEEK_CUR);
        }

        // Prevent infinite loops
        if (lseek(fd, 0, SEEK_CUR) >= fileSize) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Reached end of file without finding data chunk");
            break;
        }
    }

    return foundDataChunk;
}

// Read file data into buffer
static bool ReadFileData(int fd, off_t fileSize, bool isWavFile, uint32_t wavDataSize, uint32_t wavDataStart)
{
    uint32_t bytesToRead = 0;

    if (isWavFile) {
        lseek(fd, wavDataStart, SEEK_SET);
        bytesToRead =
            (wavDataSize > 0 && wavDataSize < (fileSize - wavDataStart)) ? wavDataSize : (fileSize - wavDataStart);
        g_inputDataLen = read(fd, g_inputData, bytesToRead);
    } else {
        lseek(fd, 0, SEEK_SET);
        g_inputDataLen = read(fd, g_inputData, fileSize);
    }

    g_dataOffset = 0;
    return g_inputDataLen > 0;
}

// Clear old input and output data
static void ClearOldData()
{
    if (g_inputData != nullptr) {
        delete[] g_inputData;
        g_inputData = nullptr;
        g_inputDataLen = 0;
    }
    if (g_outputData != nullptr) {
        delete[] g_outputData;
        g_outputData = nullptr;
        g_outputDataLen = 0;
    }
}

// Validate and get file size
static bool ValidateAndGetFileSize(int fd, off_t &fileSize)
{
    fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize < 0 || fileSize > MAX_FILE_SIZE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Invalid file size: %{public}ld bytes", fileSize);
        return false;
    }
    lseek(fd, 0, SEEK_SET);
    return true;
}

// Allocate memory for input data
static bool AllocateInputData(off_t fileSize, uint8_t *&inputData)
{
    if (fileSize <= 0 || fileSize > MAX_FILE_SIZE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Invalid memory allocation size: %{public}ld bytes",
                     fileSize);
        return false;
    }

    inputData = new (std::nothrow) uint8_t[fileSize];
    if (inputData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Failed to allocate memory for input data");
        return false;
    }
    std::fill(inputData, inputData + fileSize, 0);
    return true;
}

// Set output parameters
static void SetOutputParameters(OH_AudioConverter_Format &inputFormat, OH_AudioConverter_Format &outputFormat)
{
    g_inputSampleRate = inputFormat.samplingRate;
    g_outputChannelCount = CalculateOutputChannelCount(outputFormat.channelLayout);
    g_outputSampleFormat = outputFormat.sampleFormat;
}

// Create converter result
static napi_value CreateConverterResult(napi_env env, OH_AudioConverter_Result result)
{
    napi_value napiValue;
    if (result == AUDIOCONVERTER_SUCCESS && g_converter != nullptr) {
        napi_create_int64(env, reinterpret_cast<int64_t>(g_converter), &napiValue);
    } else {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "AudioConverterCreate failed with result: %{public}d",
                     static_cast<int>(result));
    }
    return napiValue;
}

static napi_value AudioConverterCreate(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    OH_AudioConverter_Format inputFormat;
    if (!ParseAudioFormat(env, argv[0], inputFormat)) {
        return nullptr;
    }
    OH_AudioConverter_Format outputFormat;
    if (!ParseAudioFormat(env, argv[1], outputFormat)) {
        return nullptr;
    }
    int32_t fd = INVALID_FD;
    napi_status status = napi_get_value_int32(env, argv[2], &fd);
    if (status != napi_ok || fd < 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Invalid file descriptor");
        return nullptr;
    }
    ClearOldData();
    off_t fileSize = 0;
    if (!ValidateAndGetFileSize(fd, fileSize)) {
        return nullptr;
    }
    if (!AllocateInputData(fileSize, g_inputData)) {
        return nullptr;
    }
    uint32_t wavDataSize = 0;
    uint32_t wavDataStart = WAV_HEADER_SIZE;
    bool isWavFile = ParseWavFile(fd, fileSize, wavDataSize, wavDataStart);
    if (!ReadFileData(fd, fileSize, isWavFile, wavDataSize, wavDataStart)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Failed to read file data");
        return nullptr;
    }
    SetOutputParameters(inputFormat, outputFormat);
    OH_AudioConverter_Result result = OH_AudioConverter_Create(&inputFormat, &outputFormat, &g_converter);
    return CreateConverterResult(env, result);
}

// AudioConverter C callback function - called by converter engine
static int32_t AudioConverterRequestDataCallback(void *userData, const void **outInputData,
                                                 OH_AudioConverter_InputStatus *outStatus)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "AudioConverterRequestDataCallback: g_inputData=%{public}p, g_inputDataLen=%{public}u, "
                 "g_dataOffset=%{public}u",
                 g_inputData, g_inputDataLen, g_dataOffset);

    if (g_inputData == nullptr || g_inputDataLen == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "No input data available");
        *outInputData = nullptr;
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        return 0;
    }

    if (g_dataOffset >= g_inputDataLen) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "AudioConverterRequestDataCallback: All input data consumed, g_dataOffset=%{public}u, "
                     "g_inputDataLen=%{public}u",
                     g_dataOffset, g_inputDataLen);
        *outInputData = nullptr;
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        return 0;
    }

    // Returns data each time, improving data transfer efficiency and ensuring
    // that all audio data can be correctly processed
    uint32_t remainingData = g_inputDataLen - g_dataOffset;
    uint32_t dataSizeToReturn = (remainingData > DATA_CHUNK_SIZE) ? DATA_CHUNK_SIZE : remainingData;
    *outInputData = g_inputData + g_dataOffset;
    *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;

    // Update the offset, indicating that the data has been provided.
    g_dataOffset += dataSizeToReturn;

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "AudioConverterRequestDataCallback: Returning %{public}u bytes, remaining=%{public}u",
                 dataSizeToReturn, g_inputDataLen - g_dataOffset);

    return dataSizeToReturn;
}

static napi_value AudioConverterSetInputCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // Parse converter handle
    int64_t converterHandle = 0;
    napi_status status = napi_get_value_int64(env, argv[0], &converterHandle);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse converterHandle failed");
        delete[] argv;
        return nullptr;
    }
    OH_AudioConverter *converter = reinterpret_cast<OH_AudioConverter *>(converterHandle);

    // Set callback
    OH_AudioConverter_Result result =
        OH_AudioConverter_SetInputCallback(converter, AudioConverterRequestDataCallback, nullptr);

    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    delete[] argv;
    return napiValue;
}

// Calculate buffer size based on audio parameters
static int32_t CalculateBufferSize()
{
    int32_t bufferSize;

    // Determine base size based on sampling rate
    if (g_inputSampleRate >= SAMPLE_RATE_THRESHOLD_HIGH) {
        bufferSize = BASE_BUFFER_SIZE_192K;
    } else if (g_inputSampleRate >= SAMPLE_RATE_THRESHOLD_MID) {
        bufferSize = BASE_BUFFER_SIZE_96K;
    } else if (g_inputSampleRate >= SAMPLE_RATE_THRESHOLD_LOW) {
        bufferSize = BASE_BUFFER_SIZE_48K;
    } else {
        bufferSize = BASE_BUFFER_SIZE_LOW;
    }
    // Adjust buffer size based on channel count
    if (g_outputChannelCount > 0) {
        bufferSize = bufferSize * g_outputChannelCount / CHANNEL_COUNT_STEREO;
    }

    // Adjust buffer size based on sample format
    int32_t sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_1;
    switch (g_outputSampleFormat) {
        case AUDIO_SAMPLE_U8:
            sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_1;
            break;
        case AUDIO_SAMPLE_S16LE:
            sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_1;
            break;
        case AUDIO_SAMPLE_S24LE:
            sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_2;
            break;
        case AUDIO_SAMPLE_S32LE:
            sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_2;
            break;
        case AUDIO_SAMPLE_F32LE:
            sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_2;
            break;
        default:
            sampleFormatMultiplier = SAMPLE_FORMAT_MULTIPLIER_1;
            break;
    }
    bufferSize = bufferSize * sampleFormatMultiplier;

    return bufferSize;
}

// Accumulate output data with buffer expansion
static bool AccumulateOutputData(const void *outputData, int32_t outputSize)
{
    if (g_outputDataLen + outputSize > MAX_OUTPUT_SIZE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "Output size exceeds maximum limit: current=%{public}u, adding=%{public}u, max=%{public}zu",
                     g_outputDataLen, outputSize, MAX_OUTPUT_SIZE);
        return false;
    }
    size_t newBufferSize = g_outputDataLen + outputSize;
    size_t allocatedBufferSize = (g_outputDataLen > 0) ? g_outputDataLen : 1;
    while (allocatedBufferSize < newBufferSize) {
        allocatedBufferSize *= BUFFER_EXPANSION_FACTOR;
    }
    if (allocatedBufferSize > MAX_SINGLE_ALLOC && g_outputDataLen + outputSize < MAX_SINGLE_ALLOC) {
        allocatedBufferSize = g_outputDataLen + outputSize;
    }
    uint8_t *newBuffer = new (std::nothrow) uint8_t[allocatedBufferSize];
    if (newBuffer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "Failed to allocate memory for output buffer: %{public}zu bytes", allocatedBufferSize);
        return false;
    }
    if (g_outputData != nullptr && g_outputDataLen > 0) {
        std::copy(static_cast<const uint8_t *>(g_outputData),
                  static_cast<const uint8_t *>(g_outputData) + g_outputDataLen, newBuffer);
    }
    std::copy(static_cast<const uint8_t *>(outputData), static_cast<const uint8_t *>(outputData) + outputSize,
              newBuffer + g_outputDataLen);
    g_outputDataLen += outputSize;
    if (g_outputData != nullptr) {
        delete[] g_outputData;
    }
    g_outputData = newBuffer;
    return true;
}

// Reset output data and offset
static void ResetOutputData()
{
    if (g_outputData != nullptr) {
        delete[] g_outputData;
        g_outputData = nullptr;
    }
    g_outputDataLen = 0;
    g_dataOffset = 0;
    // Note: Don't reset g_inputDataLen here as input data is still needed
}

// Reset process state for retry
static void ResetProcessState()
{
    g_dataOffset = 0;
    if (g_outputData != nullptr) {
        delete[] g_outputData;
        g_outputData = nullptr;
    }
    g_outputDataLen = 0;
    // Note: Don't reset g_inputDataLen here as input data is still needed for retry
}

// Handle process error
static bool HandleProcessError(OH_AudioConverter_Result result, ProcessState &state, void *outputData)
{
    if (result == AUDIOCONVERTER_ERROR_UNKNOWN_CODE && !state.retryWithSmallerBuffer &&
        state.processCount == FIRST_PROCESS_ATTEMPT) {
        state.retryWithSmallerBuffer = true;
        delete[] static_cast<uint8_t *>(outputData);
        ResetProcessState();
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "HandleProcessError: Retrying with smaller buffer");
        return true; // Continue retry
    }

    state.hasError = true;
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "HandleProcessError: Error occurred, result=%{public}d, hasError=true",
                 static_cast<int>(result));
    delete[] static_cast<uint8_t*>(outputData);
    return false; // Stop processing
}
// Handle process output data
static bool HandleProcessOutput(void *outputData, int32_t outputSize, ProcessState &state)
{
    if (outputSize > 0) {
        if (!AccumulateOutputData(outputData, outputSize)) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                         "HandleProcessOutput: AccumulateOutputData failed");
            delete[] static_cast<uint8_t*>(outputData);
            state.hasError = true;
            return false; // Stop processing
        }
        state.zeroOutputCount = 0;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "HandleProcessOutput: Data accumulated, g_outputDataLen=%{public}u", g_outputDataLen);
    } else {
        state.zeroOutputCount++;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "HandleProcessOutput: Zero output, zeroOutputCount=%{public}d", state.zeroOutputCount);
        if (state.zeroOutputCount >= ZERO_OUTPUT_THRESHOLD) {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                         "HandleProcessOutput: Reached zero output threshold, processing complete");
            delete[] static_cast<uint8_t*>(outputData);
            return false; // Processing complete
        }
    }
    return true; // Continue processing
}

// Create output ArrayBuffer
static napi_value CreateOutputArrayBuffer(napi_env env)
{
    napi_value napiValue;
    void *arrayBufferData = nullptr;
    napi_create_arraybuffer(env, g_outputDataLen, &arrayBufferData, &napiValue);
    std::copy(static_cast<const uint8_t*>(g_outputData),
              static_cast<const uint8_t*>(g_outputData) + g_outputDataLen,
              static_cast<uint8_t*>(arrayBufferData));

    float ratio = (g_inputDataLen > 0) ? static_cast<float>(g_outputDataLen) / g_inputDataLen : 0.0f;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "Process Success", g_inputDataLen, g_outputDataLen, ratio);

    return napiValue;
}

// Parse and validate converter handle
static OH_AudioConverter* ParseAndValidateConverter(napi_env env, napi_value argv[])
{
    int64_t converterHandle = 0;
    napi_status status = napi_get_value_int64(env, argv[0], &converterHandle);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse converterHandle failed");
        return nullptr;
    }
    OH_AudioConverter *converter = reinterpret_cast<OH_AudioConverter *>(converterHandle);

    if (converter == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Converter is null");
        return nullptr;
    }
    return converter;
}

// Validate input data
static bool ValidateInputData()
{
    if (g_inputData == nullptr || g_inputDataLen == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "Input data is null or empty, g_inputData=%{public}p, g_inputDataLen=%{public}u", g_inputData,
                     g_inputDataLen);
        return false;
    }
    return true;
}

// Initialize process parameters
static void InitializeProcessParameters(ProcessState &state)
{
    state.bufferSize = CalculateBufferSize();
    state.processCount = 0;
    state.zeroOutputCount = 0;
    state.hasError = false;
    state.retryWithSmallerBuffer = false;
}

// Process single iteration
static bool ProcessIteration(OH_AudioConverter *converter, ProcessState &state)
{
    int32_t currentBufferSize = state.retryWithSmallerBuffer ? RETRY_BUFFER_SIZE : state.bufferSize;
    void *outputData = new uint8_t[currentBufferSize];
    int32_t outputSize = 0;
    OH_AudioConverter_Result result = OH_AudioConverter_Process(converter, outputData, currentBufferSize, &outputSize);
    state.processCount++;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "ProcessIteration: processCount=%{public}d, result=%{public}d, outputSize=%{public}d, "
                 "zeroOutputCount=%{public}d, g_outputDataLen=%{public}u",
                 state.processCount, static_cast<int>(result), outputSize, state.zeroOutputCount, g_outputDataLen);

    if (result != AUDIOCONVERTER_SUCCESS) {
        return HandleProcessError(result, state, outputData);
    }

    if (!HandleProcessOutput(outputData, outputSize, state)) {
        return false;
    }

    delete[] static_cast<uint8_t*>(outputData);
    return true; // Continue processing
}

// Create final result value
static napi_value CreateFinalResult(napi_env env, bool hasError)
{
    napi_value napiValue;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "CreateFinalResult: hasError=%{public}d, g_outputDataLen=%{public}u, g_outputData=%{public}p",
                 hasError, g_outputDataLen, g_outputData);

    if (hasError || g_outputDataLen == 0) {
        napi_get_null(env, &napiValue);
        OH_LOG_Print(
            LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "Process fails or no data is available, return null: hasError=%{public}d, g_outputDataLen=%{public}u",
            hasError, g_outputDataLen);
    } else {
        napiValue = CreateOutputArrayBuffer(env);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CreateFinalResult: Output array buffer created");
    }
    return napiValue;
}

static napi_value AudioConverterProcess(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    OH_AudioConverter *converter = ParseAndValidateConverter(env, argv);
    if (converter == nullptr) {
        delete[] argv;
        return nullptr;
    }

    if (!ValidateInputData()) {
        delete[] argv;
        return nullptr;
    }

    // Reset output data
    ResetOutputData();
    ProcessState state;
    InitializeProcessParameters(state);

    while (state.processCount < MAX_PROCESS_ATTEMPTS) {
        bool shouldContinue = ProcessIteration(converter, state);
        if (!shouldContinue) {
            break;
        }
    }

    napi_value napiValue = CreateFinalResult(env, state.hasError);
    delete[] argv;
    return napiValue;
}

// OH_AudioConverter_Destroy NAPI binding
static napi_value AudioConverterDestroy(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // Parse converter handle
    int64_t converterHandle = 0;
    napi_status status = napi_get_value_int64(env, argv[0], &converterHandle);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Parse converterHandle failed");
        delete[] argv;
        return nullptr;
    }
    OH_AudioConverter *converter = reinterpret_cast<OH_AudioConverter *>(converterHandle);

    OH_AudioConverter_Destroy(converter); // Directly calling the void function.
    g_converter = nullptr;                // Avoiding dangling pointers
    OH_AudioConverter_Result result = AUDIOCONVERTER_SUCCESS;

    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    delete[] argv;
    return napiValue;
}

// SaveConverterFile NAPI binding - Saving the converted audio data.
static napi_value SaveConverterFile(napi_env env, napi_callback_info info)
{
    // Checking whether there is output data.
    if (g_outputData == nullptr || g_outputDataLen == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "No converter output data available");
        napi_value emptyArray;
        void *emptyData = nullptr;
        napi_create_arraybuffer(env, 0, &emptyData, &emptyArray);
        return emptyArray;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "Saving converter output data, size: %{public}d",
                 g_outputDataLen);

    // Create an ArrayBuffer and return it.
    void *data = nullptr;
    napi_value arrayBuffer;
    napi_create_arraybuffer(env, g_outputDataLen, &data, &arrayBuffer);

    if (data != nullptr && g_outputDataLen > 0) {
        // Copying Output Data to ArrayBuffer
        std::copy(static_cast<const uint8_t *>(g_outputData),
                  static_cast<const uint8_t *>(g_outputData) + g_outputDataLen, static_cast<uint8_t *>(data));
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "ArrayBuffer created and data copied successfully");
    }

    return arrayBuffer;
}

static int32_t GetSampleFormatFromBits(uint16_t bitsPerSample)
{
    int32_t sampleFormat;
    switch (bitsPerSample) {
        case BITS_PER_SAMPLE_8: sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8; break;
        case BITS_PER_SAMPLE_16: sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE; break;
        case BITS_PER_SAMPLE_24: sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE; break;
        case BITS_PER_SAMPLE_32: sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE; break;
        default:
            OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, TAG,
                         "Unsupported bits per sample: %{public}u, defaulting to F32LE", bitsPerSample);
            sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;
    }
    return sampleFormat;
}

static int64_t GetChannelLayoutFromChannels(uint16_t numChannels)
{
    int64_t channelLayout;
    switch (numChannels) {
        case CHANNEL_COUNT_MONO: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO; break;
        case CHANNEL_COUNT_STEREO: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO; break;
        case CHANNEL_COUNT_2POINT1: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_2POINT1; break;
        case CHANNEL_COUNT_QUAD: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_QUAD; break;
        case CHANNEL_COUNT_5POINT0: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_5POINT0; break;
        case CHANNEL_COUNT_5POINT1: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_5POINT1; break;
        case CHANNEL_COUNT_7POINT0: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_6POINT1; break;
        case CHANNEL_COUNT_7POINT1: channelLayout = OH_AudioChannelLayout::CH_LAYOUT_7POINT1; break;
        default:
            OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, TAG,
                         "Unsupported channel count: %{public}u (max supported: 8), using default stereo layout",
                         numChannels);
            OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, TAG,
                         "Note: Audio processing will use the actual channel count (%{public}u) from file",
                         numChannels);
            channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    }
    return channelLayout;
}

static bool ReadWavHeader(int fd, uint8_t* header, int size)
{
    int headerRead = read(fd, header, size);
    if (headerRead < size) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "Failed to read WAV header");
        return false;
    }

    if (header[0] != DATA_HEADER_INDEX[0] || header[1] != DATA_HEADER_INDEX[1] ||
        header[INDEXTWO] != DATA_HEADER_INDEX[INDEXTWO] ||
        header[INDEXTHREE] != DATA_HEADER_INDEX[INDEXTHREE]) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "Not a valid WAV file");
        return false;
    }

    return true;
}

static napi_value CreateFileInfoObject(napi_env env, const FileInfo &fileInfo)
{
    napi_value resultObj;
    napi_create_object(env, &resultObj);

    napi_value samplingRateValue;
    napi_create_int32(env, static_cast<int32_t>(fileInfo.samplingRate), &samplingRateValue);
    napi_set_named_property(env, resultObj, "samplingRate", samplingRateValue);

    napi_value channelLayoutValue;
    napi_create_int64(env, fileInfo.channelLayout, &channelLayoutValue);
    napi_set_named_property(env, resultObj, "channelLayout", channelLayoutValue);

    napi_value numChannelsValue;
    napi_create_int32(env, static_cast<int32_t>(fileInfo.numChannels), &numChannelsValue);
    napi_set_named_property(env, resultObj, "numChannels", numChannelsValue);

    napi_value sampleFormatValue;
    napi_create_int32(env, fileInfo.sampleFormat, &sampleFormatValue);
    napi_set_named_property(env, resultObj, "sampleFormat", sampleFormatValue);

    napi_value encodingTypeValue;
    napi_create_int32(env, OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW,
                      &encodingTypeValue);
    napi_set_named_property(env, resultObj, "encodingType", encodingTypeValue);

    napi_value fileSizeValue;
    napi_create_int64(env, static_cast<int64_t>(fileInfo.fileSize), &fileSizeValue);
    napi_set_named_property(env, resultObj, "fileSize", fileSizeValue);

    return resultObj;
}

static bool ValidateFileDescriptor(napi_env env, napi_value argv[], int32_t& fd)
{
    napi_status status = napi_get_value_int32(env, argv[0], &fd);
    if (status != napi_ok || fd < 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "Invalid file descriptor");
        return false;
    }
    return true;
}

static bool ReadAndParseWavHeader(int fd, uint8_t *header, uint32_t &samplingRate, uint16_t &numChannels,
                                  uint16_t &bitsPerSample)
{
    if (!ReadWavHeader(fd, header, WAV_HEADER_SIZE)) {
        return false;
    }

    samplingRate = *reinterpret_cast<uint32_t*>(&header[WAV_SAMPLE_RATE_OFFSET]);
    numChannels = *reinterpret_cast<uint16_t*>(&header[WAV_NUM_CHANNELS_OFFSET]);
    bitsPerSample = *reinterpret_cast<uint16_t*>(&header[WAV_BITS_PER_SAMPLE_OFFSET]);

    return true;
}

static napi_value AudioConverterGetFileInfo(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "AudioConverterGetFileInfo start");

    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int32_t fd = INVALID_FD;
    if (!ValidateFileDescriptor(env, argv, fd)) {
        return nullptr;
    }

    off_t fileSize = 0;
    if (!ValidateAndGetFileSize(fd, fileSize)) {
        return nullptr;
    }

    uint8_t header[WAV_HEADER_SIZE];
    uint32_t samplingRate = 0;
    uint16_t numChannels = 0;
    uint16_t bitsPerSample = 0;

    if (!ReadAndParseWavHeader(fd, header, samplingRate, numChannels, bitsPerSample)) {
        close(fd);
        return nullptr;
    }

    close(fd);

    int32_t sampleFormat = GetSampleFormatFromBits(bitsPerSample);
    int64_t channelLayout = GetChannelLayoutFromChannels(numChannels);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "Audio file info: samplingRate=%{public}u, channels=%{public}u, "
                 "bitsPerSample=%{public}u", samplingRate, numChannels, bitsPerSample);

    FileInfo fileInfo;
    fileInfo.samplingRate = samplingRate;
    fileInfo.numChannels = numChannels;
    fileInfo.channelLayout = channelLayout;
    fileInfo.sampleFormat = sampleFormat;
    fileInfo.fileSize = fileSize;

    napi_value resultObj = CreateFileInfoObject(env, fileInfo);
    return resultObj;
}

// Test the function of outputting PrintInfo to a file.（fd >= 0）
static napi_value TestPrintInfoToFile(napi_env env, napi_callback_info info)
{
    napi_value result;
    napi_get_boolean(env, false, &result);

    if (!g_audioSuiteEngine) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "TestPrintInfoToFile: g_audioSuiteEngine is nullptr");
        return result;
    }

    // Generate timestamp and file path
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&time);
    std::ostringstream timestampStream;
    timestampStream << std::setfill('0') << std::setw(TIMESTAMP_YEAR_WIDTH) << (tm->tm_year + TM_YEAR_BASE)
                    << std::setw(TIMESTAMP_DATE_WIDTH) << (tm->tm_mon + TM_MONTH_OFFSET)
                    << std::setw(TIMESTAMP_DATE_WIDTH) << tm->tm_mday << "_" << std::setw(TIMESTAMP_DATE_WIDTH)
                    << tm->tm_hour << std::setw(TIMESTAMP_DATE_WIDTH) << tm->tm_min << std::setw(TIMESTAMP_DATE_WIDTH)
                    << tm->tm_sec;

    std::string dirPath = "/storage/Users/currentUser/Download/src.main.audiodemo/printfile/";
    std::string filePath = dirPath + "/audio_snapshot_" + timestampStream.str() + ".txt";
    mkdir(dirPath.c_str(), DIRECTORY_PERMISSIONS);

    // Open file
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT, FILE_PERMISSIONS);
    if (fd < 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "TestPrintInfoToFile: open failed, errno=%{public}d",
                     errno);
        OH_AudioSuite_Result ret = OH_AudioSuite_PrintInfo(g_audioSuiteEngine, nullptr, INVALID_FD);
        if (ret == AUDIOSUITE_SUCCESS) {
            napi_get_boolean(env, true, &result);
        }
        return result;
    }

    // Output to file
    OH_AudioSuite_Result ret = OH_AudioSuite_PrintInfo(g_audioSuiteEngine, nullptr, fd);
    close(fd);

    if (ret != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "TestPrintInfoToFile failed: %{public}u", ret);
        return result;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "TestPrintInfoToFile success");
    napi_get_boolean(env, true, &result);
    return result;
}

const std::vector<napi_property_descriptor> recordDescriptors = {
    {"audioCapturerInit", nullptr, AudioCapturerInit,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"audioCapturerStart", nullptr, AudioCapturerStart, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"audioCapturerStop", nullptr, AudioCapturerStop, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"audioCapturerRelease", nullptr, AudioCapturerRelease, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"getAudioFrames", nullptr, GetAudioFrames, nullptr, nullptr, 0, napi_default, nullptr },
    {"audioCapturerPause", nullptr, AudioCapturerPause, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"mixRecordBuffer", nullptr, MixRecordBuffer, nullptr, nullptr, 0, napi_default, nullptr },
    {"mixPlayInitBuffer", nullptr, MixPlayInitBuffer, nullptr, nullptr, 0, napi_default, nullptr },
    {"clearRecordBuffer", nullptr, ClearRecordBuffer, nullptr, nullptr, 0, napi_default, nullptr },
    {"realPlayRecordBuffer", nullptr, RealPlayRecordBuffer, nullptr, nullptr, 0, napi_default, nullptr }
};

const std::vector<napi_property_descriptor> multiPipelineDescriptors = {
    {"audioEditNodeInitMultiPipeline", nullptr, AudioEditNodeInitMultiPipeline,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiAudioInAndOutInit", nullptr, MultiAudioInAndOutInit,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiPipelineEnvPrepare", nullptr, MultiPipelineEnvPrepare,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiSetFormat", nullptr, MultiSetFormat,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiSaveFileBuffer", nullptr, MultiSaveFileBuffer,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiGetSecondOutputAudio", nullptr, MultiGetSecondOutputAudio,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiDeleteSong", nullptr, MultiDeleteSong,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"destroyMultiPipeline", nullptr, DestroyMultiPipeline,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiAudioRendererInit", nullptr, MultiAudioRendererInit,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiAudioRendererStart", nullptr, MultiAudioRendererStart,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"multiRealTimeSaveFileBuffer", nullptr, MultiRealTimeSaveFileBuffer,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"getAutoTestProcess", nullptr, GetAutoTestProcess,
        nullptr, nullptr, nullptr, napi_default, nullptr},
};

const std::vector<napi_property_descriptor> voiceChangeDescriptors = {
    {"startGeneralVoiceChange", nullptr, StartGeneralVoiceChange, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"resetGeneralVoiceChange", nullptr, ResetGeneralVoiceChange, nullptr, nullptr, nullptr, napi_default, nullptr },
    {"startPureVoiceChange", nullptr, StartPureVoiceChange, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"resetPureVoiceChange", nullptr, ResetPureVoiceChange, nullptr, nullptr, nullptr, napi_default, nullptr },
};

const std::vector<napi_property_descriptor> spaceRenderDescriptors = {
    {"StartFixedPositionEffect", nullptr, StartFixedPositionEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"StartDynamicRenderEffect", nullptr, StartDynamicRenderEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"StartExpandEffect", nullptr, StartExpandEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"ResetFixedPositionEffect", nullptr, ResetFixedPositionEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"ResetDynamicRenderEffect", nullptr, ResetDynamicRenderEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"ResetExpandEffect", nullptr, ResetExpandEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetFixedPositionParams", nullptr, GetFixedPositionParams, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetDynamicRenderParams", nullptr, GetDynamicRenderParams, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"GetExpandParams", nullptr, GetExpandParams, nullptr, nullptr, nullptr, napi_default, nullptr}
};

const std::vector<napi_property_descriptor> timelineDescriptors = {
    {"addAudioTrack", nullptr, AddAudioTrack, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"deleteAudioTrack", nullptr, DeleteAudioTrack, nullptr, nullptr, nullptr, napi_default, nullptr },
    {"setAudioTrackSilent", nullptr, SetAudioTrackSilent, nullptr, nullptr, nullptr, napi_default, nullptr },
    {"addAudioAsset", nullptr, AddAudioAsset, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"updateAudioAsset", nullptr, UpdateAudioAsset, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"deleteAudioAsset", nullptr, DeleteAudioAsset, nullptr, nullptr, nullptr, napi_default, nullptr },
    {"setAudioAssetStartTime", nullptr, SetAudioAssetStartTime, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setAudioAssetPcmBufferLength", nullptr, SetAudioAssetPcmBufferLength,
        nullptr, nullptr, nullptr, napi_default, nullptr },
    {"addAudioAssetEffectNode", nullptr, AddAudioAssetEffectNode, nullptr, nullptr, nullptr, napi_default, nullptr },
    {"deleteAudioAssetEffectNode", nullptr, DeleteAudioAssetEffectNode,
        nullptr, nullptr, nullptr, napi_default, nullptr},
    {"clearTimeline", nullptr, ClearTimeline, nullptr, nullptr, nullptr, napi_default, nullptr },
};

const std::vector<napi_property_descriptor> callbackDescriptors = {
    {"registerFinishedCallback", nullptr, RegisterFinishedCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"registerAudioFormatCallback", nullptr, RegisterAudioFormatCallback, nullptr, nullptr, nullptr, napi_default,
        nullptr},
    {"registerStringCallback", nullptr, RegisterStringCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"registerAudioCacheCallback", nullptr, RegisterAudioCacheCallback, nullptr, nullptr, nullptr, napi_default,
        nullptr},
    {"unregisterFinishedCallback", nullptr, UnregisterFinishedCallback, nullptr, nullptr, nullptr, napi_default,
        nullptr},
    {"unregisterAudioFormatCallback", nullptr, UnregisterAudioFormatCallback, nullptr, nullptr, nullptr, napi_default,
        nullptr},
    {"unregisterStringCallback", nullptr, UnregisterStringCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"unregisterAudioCacheCallback", nullptr, UnregisterAudioCacheCallback, nullptr, nullptr, nullptr, napi_default,
        nullptr}
};

const std::vector<napi_property_descriptor> otherDescriptors = {
    {"saveFileBuffer", nullptr, SaveFileBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"startFieldEffect", nullptr, startFieldEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"startVBEffect", nullptr, startVBEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"addAudioSeparation", nullptr, addAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"addNoiseReduction", nullptr, addNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"resetFieldEffect", nullptr, resetFieldEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"resetVBEffect", nullptr, resetVBEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"deleteNoiseReduction", nullptr, deleteNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"deleteSong", nullptr, DeleteSong, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"deleteAudioSeparation", nullptr, deleteAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"getAudioOfTap", nullptr, getAudioOfTap, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"startEnvEffect", nullptr, startEnvEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"resetEnvEffect", nullptr, resetEnvEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"deleteNode", nullptr, DeleteNode, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"getOptions", nullptr, getOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"getEffectNodeList", nullptr, getEffectNodeList, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setSoundSpeedTone", nullptr, SetSoundSpeedTone, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setIsRecord", nullptr, SetIsRecord, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setSeparationMode", nullptr, SetSeparationMode, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"clear", nullptr, clear, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"clearByInputId", nullptr, clearByInputId, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"ModifyRender", nullptr, ModifyRender, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"stopPipeline", nullptr, stopPipeline, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setCurrentTime", nullptr, setCurrentTime, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setEffectNodeBypass", nullptr, SetEffectNodeBypass, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"testPrintInfoToFile", nullptr, TestPrintInfoToFile, nullptr, nullptr, nullptr, napi_default, nullptr},
    // Real-time playback methods
    {"initAudioRenderer", nullptr, InitAudioRenderer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"startAudioRenderer", nullptr, StartAudioRenderer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"stopAudioRenderer", nullptr, StopAudioRenderer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"releaseAudioRenderer", nullptr, ReleaseAudioRenderer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"setRecordFlag", nullptr, SetRecordFlag, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"getRecordedAudioData", nullptr, GetRecordedAudioData, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"registerPlaybackFinishCallback", nullptr, RegisterPlaybackFinishCallback, nullptr, nullptr, nullptr, napi_default,
     nullptr},
    {"unregisterPlaybackFinishCallback", nullptr, UnregisterPlaybackFinishCallback, nullptr, nullptr, nullptr,
     napi_default, nullptr},
};

static napi_value Init(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> desc = {
        {"record", nullptr, Record, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererInit", nullptr, AudioRendererInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererDestory", nullptr, AudioRendererDestory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererStart", nullptr, AudioRendererStart, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererPause", nullptr, AudioRendererPause, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererStop", nullptr, AudioRendererStop, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getRendererState", nullptr, GetRendererState, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetTotalWriteAudioDataSize", nullptr, ResetTotalWriteAudioDataSize, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"realTimeSaveFileBuffer", nullptr, RealTimeSaveFileBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioEditNodeInit", nullptr, AudioEditNodeInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioInAndOutInit", nullptr, AudioInAndOutInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"initByPipelineCascad", nullptr, InitByPipelineCascad, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioEditDestory", nullptr, AudioEditDestory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFormat", nullptr, SetFormat, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setEqualizerMode", nullptr, SetEqualizerMode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setEqualizerFrequencyBandGains", nullptr, SetEqualizerFrequencyBandGains, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"audioConverterCreate", nullptr, AudioConverterCreate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioConverterSetInputCallback", nullptr, AudioConverterSetInputCallback, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"audioConverterProcess", nullptr, AudioConverterProcess, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioConverterDestroy", nullptr, AudioConverterDestroy, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"saveConverterFile", nullptr, SaveConverterFile, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"audioConverterGetFileInfo", nullptr, AudioConverterGetFileInfo, nullptr, nullptr,
         nullptr, napi_default, nullptr}
    };
    desc.insert(desc.end(), multiPipelineDescriptors.begin(), multiPipelineDescriptors.end());
    desc.insert(desc.end(), voiceChangeDescriptors.begin(), voiceChangeDescriptors.end());
    desc.insert(desc.end(), spaceRenderDescriptors.begin(), spaceRenderDescriptors.end());
    desc.insert(desc.end(), recordDescriptors.begin(), recordDescriptors.end());
    desc.insert(desc.end(), timelineDescriptors.begin(), timelineDescriptors.end());
    desc.insert(desc.end(), callbackDescriptors.begin(), callbackDescriptors.end());
    desc.insert(desc.end(), otherDescriptors.begin(), otherDescriptors.end());
    napi_define_properties(env, exports, desc.size(), desc.data());
    return exports;
}

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }