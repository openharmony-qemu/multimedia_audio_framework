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
#include <filemanagement/file_uri/oh_file_uri.h>
#include "callback/RegisterCallback.h"
#include "audioSuiteError/AudioSuiteError.h"
#include "audioEffectNode/Input.h"
#include "audioEffectNode/Output.h"
#include "audioEffectNode/EffectNode.h"
#include "audioEffectNode/ParseNapiParam.h"
#include "audioEffectNode/Equalizer.h"
#include "audioEffectNode/SoundField.h"
#include "callback/RegisterCallback.h"
#include "audioEffectNode/NoiseReduction.h"
#include "audioEffectNode/EnvEffect.h"
#include "audioEffectNode/SpaceRender.h"
#include "audioEffectNode/AissEffect.h"
#include "audioEffectNode/SoundSpeedTone.h"
#include "audioEffectNode/VoiceChange.h"
#include "callback/RegisterCallbackNapi.h"
#include "./utils/Utils.h"
#include "realTimePlay/RealTimePlaying.h"
#include "audioSuiteError/AudioSuiteError.h"
#include "multiPipelineEdit/MultiPipelineEdit.h"
#include "audioRecord/AudioRecord.h"
#include "timeline/Timeline_napi.h"
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <fcntl.h>

const int GLOBAL_RESMGR = 0xFF00;
const char *TAG = "[AudioEditTestApp_AudioEdit_cpp]";

const int MAX_PLAY_RESULT_BUFFER_SIZE = 1024 * 1024 * 1024;

const int SAMPLINGRATE_MULTI = 20;
const int CHANNELCOUNT_MULTI = 1000;
const int BITSPERSAMPLE_MULTI = 8;
const int INPUTNODES_SIZE2 = 2;

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
    Timeline::getInstance().deleteAllAudioTrack();
}

// release selected inputNode
static void ClearByInputId(const std::string& inputId, long startTime)
{
    auto it = g_writeDataBufferMap.find(inputId.c_str() + std::to_string(startTime));
    if (it != g_writeDataBufferMap.end()) {
        g_writeDataBufferMap.erase(it);
    }
    bool ret = Timeline::getInstance().deleteAudioTrack(inputId);
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
    g_playTotalAudioData = (char *)malloc(MAX_PLAY_RESULT_BUFFER_SIZE);
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
        return nullptr;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "SetIsRecord isRecord: %{public}s", isRecord ? "true" : "false");
    if (isRecord) {
        g_playResultTotalSize = 0;
    }
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
        delete[] argv
        return nullptr;
    }
    ClearByInputId(inputId, startTime);
    delete[] argv
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
    Timeline::getInstance().resetCurrent(currentTime);
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
    {"setEffectNodeBypass", nullptr, SetEffectNodeBypass, nullptr, nullptr, nullptr, napi_default, nullptr}
};

EXTERN_C_START static napi_value Init(napi_env env, napi_value exports)
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
            napi_default, nullptr}
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
EXTERN_C_END

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
