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
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"
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
#include "audioEffectNode/AudioConfigParam.h"
#include "audioEffectNode/Equalizer.h"
#include "audioEffectNode/CompareFile.h"
#include "audioEffectNode/SoundField.h"
#include "audioEffectNode/Env.h"

#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <fcntl.h>

const int GLOBAL_RESMGR = 0xFF00;
const char *TAG = "[AudioEditTestApp_AudioEdit_cpp]";
std::shared_ptr<EffectNode> effectNode = nullptr;
// 写入底层的音频数据缓冲区，目前 - 初始化input、用户保存数据后会释放以前的内存，，调用底层OH_AudioSuiteEngine_RenderFrame后，需要重新给g_totalBuff赋值
const int TOTAL_BUFF = 8 * 1024 * 1024;
char *g_totalBuff = (char *)malloc(TOTAL_BUFF);
char *g_tapTotalBuff = (char *)malloc(TOTAL_BUFF);
bool g_multiRenderFrameFlag = false;
// 需要写入的音频数据大小
size_t g_totalSize = 0;
std::map<std::string, void *> anotherAudioChannel;
std::string g_latestAissNodeId;
ssize_t g_audioDataSize = 0;
// 用于接收处理过后，音频的大小
ssize_t g_frameSize;
std::map<std::string, void *> tapCallbackResultMap;
std::string g_processingAissNode = "";
bool g_couldSetCallbackResult = false;
double g_dataInputProcessing = 0;
int g_writeIndex = 0;

// 定义一个结构体来存储ID和数字
struct UserData {
    std::string id;                      // 根据id去writeDataBufferMap_获取对应的音频数据
    int32_t bufferSize;                  // 音频总数据大小
    ssize_t totalWriteAudioDataSize;     // 已经写入的音频数据大小
    bool isResetTotalWriteAudioDataSize; // 音频是否从头开始写入
};
// 写入音频数据的map
std::map<std::string, std::vector<uint8_t>> writeDataBufferMap_;
// 存储UserData的map
std::map<std::string, UserData *> userDataMap_;
std::shared_ptr<NodeManager> g_nodeManager;
OH_AudioSuiteEngine *audioSuiteEngine;
OH_AudioSuitePipeline *audioSuitePipeline;
// 创造 output builder 构造器
OH_AudioNodeBuilder *builderOut;

struct CallbackOriginData {
    napi_env env;
    napi_value recv;
    napi_value callback;
};
struct SaveBufferCallbackParam {
    void *pcmBuffer;
    std::string filename;
    std::string audioFormate;
    int sampleRate;
    int channels;
    int bitsPerSample;
};
CallbackOriginData *g_callbackData = nullptr;
void *g_aissTapAudioData = (char *)malloc(1024 * 1024 * 100);
bool g_globalFinishFlag = true;
ssize_t g_tapDataTotalSize = 0;

OH_AudioFormat audioFormatInput;
OH_AudioFormat audioFormatOutput;

enum {
    ARG_1 = 0,
    ARG_2 = 1,
    ARG_3 = 2,
    ARG_4 = 3,
    ARG_5 = 4,
    ARG_6 = 5,
    ARG_7 = 6,
    ARG_8 = 7,
    ARG_9 = 8
};

const int AUDIODATA_ARRAYSIZE = 1024 * 4;
const int TOTALSIZE_MULTI = 100;
const int ERROR_RESULT = -1;
const int SAMPLINGRATE_MULTI = 20;
const int CHANNELCOUNT_MULTI = 1000;
const int BITSPERSAMPLE_MULTI = 8;
const int INPUTNODES_SIZE2 = 2;
const int DATAINPUT_PROCESSING = 100;
const int AUDIODATAARRAY_SIZE = 2 * sizeof(void*);
const double DATAINPUTPROCESSING_SIZE = 100;
const int ACCESSAUDIODATA_ARRAY_NUM = 2;
const int TOTAL_AUDIODATA_SIZE = 1024 * 1024 * 100;
const int AUDIODATA_SIZE = 1024 * 4;
const int FRAME_SIZE = 1024 * 4;

struct RenderContext {
    char *totalAudioData = nullptr;
    char *tapTotalAudioData = nullptr;
    char *audioData = nullptr;
    int32_t writeSize = 0;
    int32_t frameSize = 1024 * 4;
    bool finishedFlag = false;
    ssize_t resultTotalSize = 0;
    ssize_t tapResultTotalSize = 0;
    OH_AudioDataArray* ohAudioDataArray = nullptr;
};

napi_value ReturnResult(napi_env env, OH_AudioSuite_Result result)
{
    std::string resultMessage = GetErrorMessage(result);
    napi_value sum;
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "result: %{public}d, resultMessage: %{public}s", result, resultMessage.c_str());
    } else {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "result: %{public}d, resultMessage: %{public}s", result, resultMessage.c_str());
    }
    napi_create_int64(env, static_cast<int>(result), &sum);
}

static OH_AudioSuite_Result StartPipelineAndCheckState()
{
    // 启动管线
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_StartPipeline(audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // 获取管线状态
    OH_AudioSuite_PipelineState pipeLineState;
    result = OH_AudioSuiteEngine_GetPipelineState(audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    return result;
}

static napi_value AudioEditNodeInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioEditNodeInit start");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 解析工作模式

    // 创建引擎
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OH_AudioEditEngine_Create result: %{public}d",
        static_cast<int>(result));
    // 根据入参判断当前的workmode
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[0], &mode);
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
        return nullptr;
    }
    // 创建管线
    result = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline, workMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioEditEngine_CreatePipeline result: %{public}d", static_cast<int>(result));
    // 实例化NodeManager
    g_nodeManager = std::make_shared<NodeManager>(audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest createNodeManager result: %{public}d",
        static_cast<int>(g_nodeManager->getAllNodes().size()));

    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 释放内存
static void Clear()
{
    // 释放map内存
    writeDataBufferMap_.clear();
    for (auto &pair : userDataMap_) {
        delete pair.second; // 删除指针指向的对象
    }
    userDataMap_.clear();
}

static napi_value AudioEditDestory(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioEditDestory start");
    Clear();
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_DestroyPipeline(audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_DestroyPipeline result: %{public}d", static_cast<int>(result));
    result = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OH_audioSuiteEngine_Destroy result: %{public}d",
        static_cast<int>(result));
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

static napi_value SetFormat(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat start");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取通道数
    unsigned int channels;
    napi_get_value_uint32(env, argv[0], &channels);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat channels is %{public}d", channels);
    // 获取采样率
    unsigned int sampleRate;
    napi_get_value_uint32(env, argv[1], &sampleRate);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat sampleRate is %{public}d", sampleRate);
    // 获取位深
    unsigned int bitsPerSample;
    napi_get_value_uint32(env, argv[ARG_3], &bitsPerSample);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat bitsPerSample is %{public}d",
        bitsPerSample);

    // 设置采样率
    audioFormatOutput.samplingRate = SetSamplingRate(sampleRate);
    // 设置声道
    audioFormatOutput.channelCount = channels;
    audioFormatOutput.channelLayout = SetChannelLayout(channels);
    // 设置位深
    audioFormatOutput.sampleFormat = SetSampleFormat(bitsPerSample);
    // 设置编码格式
    audioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    const std::vector<Node> outPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &audioFormatOutput);
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 导入音频调用
static napi_value AudioInAndOutInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioInAndOutInit start");
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    AudioParams params;
    napi_status status = ParseArguments(env, argv, params);
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
    // 采样率，声道，位深
    int32_t sampleRate;
    int32_t channels;
    int32_t bitsPerSample;
    if (!GetAudioProperties(trackFormat, sampleRate, channels, bitsPerSample)) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    // 为资源实例创建对应的解封器
    OH_AVDemuxer *demuxer = OH_AVDemuxer_CreateWithSource(source);
    if (demuxer == nullptr) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    RunAudioThread(demuxer, params.fileLength);
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
        UpdateInputNode(napiValue, result, updateInputNodeParams);
        return ReturnResult(env, static_cast<AudioSuiteResult>(result));
    }
    ManageOutputNodes(env, params.inputId, params.outputId, params.mixerId, result);
    std::vector<std::string> audioFormat = {
        std::to_string(sampleRate), std::to_string(channels), std::to_string(bitsPerSample)
    };
    CallStringArrayCallback(audioFormat);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

OH_AudioSuite_Result DeleteNodeOfSong(Node &node, int size)
{
    OH_AudioSuite_Result result;
    Node nextNode;
    if (size > INPUTNODES_SIZE2) {
        while (node.type != OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER) {
            nextNode = g_nodeManager->GetNodeById(node.nextNodeId);
            result = g_nodeManager->removeNode(node.id);
            return result;
            node = nextNode;
        }
    } else if (size == INPUTNODES_SIZE2) {
        while (node.type != OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT) {
            nextNode = g_nodeManager->GetNodeById(node.nextNodeId);
            result = g_nodeManager->removeNode(node.id);
            return result;
            node = nextNode;
        }
    } else {
        while (!node.id.empty()) {
            nextNode = g_nodeManager->GetNodeById(node.nextNodeId);
            result = g_nodeManager->removeNode(node.id);
            return result;
            node = nextNode;
        }
    }
}

// 删除音频
static napi_value DeleteSong(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取inputId参数
    std::string inputId;
    napi_status status = parseNapiString(env, argv[0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong inputId is %{public}s",
        inputId.c_str());
    
    const std::vector<Node> inputNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong inputNodes length is %{public}d",
        static_cast<int>(inputNodes.size()));

    Node node = g_nodeManager->GetNodeById(inputId);
    Node nextNode;
    if (node.id.empty()) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return napiValue;
    }
    
    if (inputNodes.size() <= 0) {
        napi_create_int64(env, static_cast<int>(-1), &napiValue);
        return napiValue;
    } else {
        result = DeleteNodeOfSong(node, inputNodes.size());
    }

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 删除节点
static napi_value DeleteNode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteNode start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取nodeId参数
    std::string nodeId;
    napi_status status = parseNapiString(env, argv[0], nodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteNode nodeId is %{public}s",
        nodeId.c_str());
    
    result = g_nodeManager->removeNode(nodeId);

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 设置均衡器模式方法
static napi_value SetEquailizerMode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode start");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    unsigned int equailizerMode = -1;
    std::string equailizerId;
    std::string inputId;
    napi_status status = GetEqModeParameters(env, argv, equailizerMode, equailizerId, inputId);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }

    // 创建均衡器效果节点
    Node eqNode = GetOrCreateEqualizerNodeByMode(equailizerId, inputId);
    if (!eqNode.physicalNode) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_CREATE_NODE_ERROR));
    }

    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode.physicalNode, SetEqualizerMode(equailizerMode));
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

// 设置均衡器频带增益
static napi_value SetEqualizerFrequencyBandGains(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEqualizerFrequencyBandGains start");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    OH_EqualizerFrequencyBandGains frequencyBandGains;
    EqBandGainsParams params;
    napi_status status = GetEqBandGainsParameters(env, argv, frequencyBandGains, params);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }
    // 创建均衡器效果节点
    Node eqNode = GetOrCreateEqualizerNodeByGains(params.equailizerId, params.inputId, params.selectedNodeId);
    if (!eqNode.physicalNode) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_CREATE_NODE_ERROR));
    }

    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode.physicalNode, frequencyBandGains);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

static napi_value SaveFileBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SaveFileBuffer start");
    ResetAllIsResetTotalWriteAudioDataSize();
    RenDerFrame();
    Clear();

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
        // 创建 ArrayBuffer 失败， 返回一个大小为 0 的ArrayBuffer
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        std::copy(g_totalBuff, g_totalBuff + g_totalSize, arrayBufferData);
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        return napiValue;
    }
}

static napi_value addNoiseReduction(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addNoiseReduction IN");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取uuid
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[ARG_1], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    // 获取二参inputId
    std::string inputIdStr;
    status = parseNapiString(env, argv[ARG_2], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取当前选中的节点id
    std::string selectNodeId;
    status = parseNapiString(env, argv[ARG_3], selectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addNoiseReduction---selectNodeId==%{public}s",
        selectNodeId.c_str());

    napi_value ret = nullptr;
    napi_create_int32(env, 1, &ret);
    Node node = CreateNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_NODE_TYPE_NOISE_REDUCTION);
    if (node.physicalNode == nullptr) {
        return ret;
    }

    int insertRes = -1;
    if (selectNodeId.empty()) {
        insertRes = AddEffectNodeToNodeManager(inputIdStr, uuidStr);
    } else {
        insertRes = g_nodeManager->insertNode(uuidStr, selectNodeId, Direction::LATER);
    }

    if (insertRes != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---AddEffectNodeToNodeManager ERROR!");
        return ret;
    }
    napi_create_int32(env, 0, &ret);
    return ret;
}
static napi_value stopNoiseReduction(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    unsigned int fd = -1;
    // Convert the incoming file descriptor into a C-side variable.
    napi_get_value_uint32(env, argv[0], &fd);
    napi_value sum;
    napi_create_int64(env, 0, &sum);
    return sum;
}

static napi_value deleteNoiseReduction(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---deleteNoiseReduction IN");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取uuidStr
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[0], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    OH_AudioSuite_Result result;
    napi_value napiValue = nullptr;
    result = g_nodeManager->removeNode(uuidStr);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---removeNode ERROR---%{public}zd", result);
    }
    napi_create_int64(env, result, &napiValue);
    return napiValue;
}

static napi_value startVBEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---IN");
    size_t argc = 4;
    napi_value argv[4] = {nullptr, nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // inputId
    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---inputId==%{public}s",
                 inputId.c_str());
    // 获取二参、美化类型
    int mode = -1;
    napi_get_value_int32(env, argv[ARG_1], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect--mode==%{public}zd", mode);
    // 获取三参、效果节点id
    std::string voiceBeautifierId;
    status = parseNapiString(env, argv[ARG_2], voiceBeautifierId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", voiceBeautifierId.c_str());
    // 获取当前选中的节点id
    std::string selectNodeId;
    status = parseNapiString(env, argv[ARG_3], selectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---selectNodeId==%{public}s",
                 selectNodeId.c_str());
     //调用添加美化效果节点接口
    napi_value ret;
    int result = AddVBEffectNode(params.inputId, params.mode, params.voiceBeautifierId, params.selectNodeId);

    napi_create_int64(env, result, &ret);
    return ret;
}
static napi_value resetVBEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetVBEffect---IN");
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int mode = -1;
    std::string inputId;
    std::string voiceBeautifierId;
    //解析参数
    napi_status status = getResetVBParameters(env, argv, inputId, mode, voiceBeautifierId);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(AudioSuiteResult::DEMO_PARAMETER_ANALYSIS_ERROR));
    }
    napi_value ret;
    int result = ModifyVBEffectNode(inputId, mode, voiceBeautifierId);
    napi_create_int64(env, result, &ret);
    return ret;
}

static napi_value startFieldEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startFieldEffect start");
    size_t argc = 4;
    napi_value argv[4] = {nullptr, nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect inputId is %{public}s",
        inputId.c_str());
    
    // 获取二参
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[ARG_2], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect mode is %{public}zd", mode);

    // 获取三参
    std::string fieldEffectId;
    status = parseNapiString(env, argv[ARG_3], fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect fieldEffectId is %{public}s",
        fieldEffectId.c_str());

    // 获取四参
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect selectedNodeId is %{public}s",
        selectedNodeId.c_str());
    OH_SoundFieldType type = getSoundFieldTypeByNum(mode);
    napi_value ret;
    Node node = CreateNodeByType(fieldEffectId, OH_AudioNode_Type::EFFECT_NODE_TYPE_SOUND_FIELD);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetSoundFieldType(node.physicalNode, type);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest startFieldEffect OH_AudioEditEngine_SetSoundFiledType ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }
    if (selectedNodeId.empty()) {
        int res = AddEffectNodeToNodeManager(inputId, fieldEffectId);
        if (res != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest startFieldEffect AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, res, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(fieldEffectId, selectedNodeId, Direction::LATER);
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

static napi_value resetFieldEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect start");
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect inputId is %{public}s",
        inputId.c_str());

    // 获取二参
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[ARG_2], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect mode is %{public}zd", mode);

    // 获取三参
    std::string fieldEffectId;
    status = parseNapiString(env, argv[ARG_3], fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest fieldEffectId is %{public}s",
        fieldEffectId.c_str());

    OH_SoundFieldType type = getSoundFieldTypeByNum(mode);

    napi_value ret;
    Node node = g_nodeManager->GetNodeById(fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest get node is %{public}s", node.id.c_str());
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetSoundFieldType(node.physicalNode, type);
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
void OnReadTapDataCallback(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---OnReadTapDataCallback---IN");
    // 检查audioNode参数，底层接口问题
    if (audioNode == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OnReadTapDataCallback audioNode is nullptr");
        return;
    }
    // audioData，底层接口问题
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OnReadTapDataCallback audioData is nullptr");
        return;
    }
    // 处理音频数据  此处如果是nullptr，是demo获取音频数据的问题，非底层接口问题
    if (audioDataSize == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest OnReadTapDataCallback audioDataSize is 0");
        return;
    }

    std::copy(
        reinterpret_cast<const char*>(audioData),
        reinterpret_cast<const char*>(audioData) + audioDataSize,
        reinterpret_cast<char*>(g_aissTapAudioData) + g_tapDataTotalSize);
    g_tapDataTotalSize += audioDataSize;
}

static napi_value addAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addAudioSeparation---IN");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参
    unsigned int arg1 = 0;
    napi_get_value_uint32(env, argv[ARG_1], &arg1);
    // 获取二参uuid
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[ARG_2], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    // 获取三参inputId
    std::string inputIdStr;
    status = parseNapiString(env, argv[ARG_3], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取四参
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest addAudioSeparation selectedNodeId is %{public}s",
        selectedNodeId.c_str());

    napi_value ret;
    napi_create_int64(env, ARG_4, &ret);
    Node node = CreateNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_MULTII_OUTPUT_NODE_TYPE_AUDIO_SEPARATION);
    if (node.physicalNode == nullptr) {
        return ret;
    }
    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputIdStr, uuidStr);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---AddEffectNodeToNodeManager ERROR!");
            return ret;
        }
    } else {
        OH_AudioSuite_Result result = g_nodeManager->insertNode(uuidStr, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest addAudioSeparation insertNode ERROR %{public}u", result);
        }
    }

    g_multiRenderFrameFlag = true;
    napi_create_int64(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addAudioSeparation: operation success");
    return ret;
}

static napi_value startEnvEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startEnvEffect---IN");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参
    std::string inputIdStr;
    napi_status status = parseNapiString(env, argv[ARG_1], inputIdStr);

    // 获取二参uuid
    std::string uuidStr;
    status = parseNapiString(env, argv[ARG_2], uuidStr);

    // 获取三参
    unsigned int mode = 0;
    napi_get_value_uint32(env, argv[ARG_3], &mode);

    // 获取四参混音台selectedNodeId
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s, uuid==%{public}s, "
        "selectedNodeId==%{public}s", inputIdStr.c_str(), uuidStr.c_str(), selectedNodeId.c_str());

    OH_EnvironmentType type;
    getEnvEnumByNumber(mode, type);
    napi_value ret;
    Node node = CreateNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_NODE_TYPE_ENVIRONMENT_EFFECT);
    if (node.physicalNode == nullptr) {
        napi_create_int64(env, ARG_4, &ret);
        return ret;
    }
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetEnvironmentType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---OH_AudioSuiteEngine_SetEnvironmentType ERROR---%{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputIdStr, uuidStr);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(uuidStr, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest startEnvEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startEnvEffect: operation success");
    return ret;
}

static napi_value resetEnvEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetEnvEffect---IN");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参
    std::string inputIdStr;
    parseNapiString(env, argv[ARG_1], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取二参uuid
    std::string effectNodeId;
    parseNapiString(env, argv[ARG_2], effectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", effectNodeId.c_str());

    // 获取三参
    unsigned int mode = 0;
    napi_get_value_uint32(env, argv[ARG_3], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---mode==%{public}d", mode);

    OH_EnvironmentType type;
    getEnvEnumByNumber(mode, type);
    napi_value ret;
    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetEnvironmentType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---OH_AudioSuiteEngine_SetEnvironmentType ERROR==%{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }
    napi_create_int64(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetEnvEffect: operation success");
    return ret;
}

int32_t CheckFilePath(std::string &filePath)
{
    if (filePath.size() >= PATH_MAX) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---File path size is too large---%{public}zd", filePath.size());
        return 1;
    }
    char buffer[PATH_MAX] = {0};
    char *path = realpath(filePath.c_str(), buffer);
    if (path == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---Invalid file path---%{public}s",
            filePath.c_str());
        return 1;
    }
    filePath = buffer;
    return 0;
}

static napi_value compareTwoFilesBinary(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---compareTwoFilesBinary---IN");
    napi_value ret;
    napi_create_int32(env, 1, &ret);
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取inputFilePath1
    std::string inputFilePath1;
    parseNapiString(env, argv[0], inputFilePath1);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputFilePath1==%{public}s",
        inputFilePath1.c_str());
    // 获取inputFilePath2;
    std::string inputFilePath2;
    parseNapiString(env, argv[1], inputFilePath2);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputFilePath2==%{public}s",
        inputFilePath2.c_str());
    
    char *downloadPath = nullptr;
    FileManagement_ErrCode status =
        OH_FileUri_GetUriFromPath(inputFilePath2.c_str(), sizeof(inputFilePath2), &downloadPath);
    if (status ==0) {
        printf("Download Path=%s", downloadPath);
    } else {
        printf("GetDownloadPath failed, error code is %d", status);
    }

    std::ifstream file1(inputFilePath1, std::ios::binary);
    std::ifstream file2("/data/storage/el2/100/base/haps/entry/files/test.txt", std::ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---open file failed");
        return ret;
    }

    if (!ValidateFileLength(file1, file2)) {
        return ret;
    }

    if (!CompareFileContent(file1, file2)) {
        return ret;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---files binary is equal");
    napi_create_int32(env, 0, &ret);
    return ret;
}

static napi_value resetAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetAudioSeparation---IN");
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参分离mode
    unsigned int arg1 = 0;
    napi_get_value_uint32(env, argv[0], &arg1);
    // 获取二参aissNodeId
    std::string aissNodeId;
    napi_status status = parseNapiString(env, argv[1], aissNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s, size==%{public}zd",
        aissNodeId.c_str(), aissNodeId.size());

    napi_value ret;
    Node node = g_nodeManager->GetNodeById(aissNodeId);

    OH_AudioSuite_Result result;

    result = g_nodeManager->disconnect(aissNodeId, node.nextNodeId);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---resetAudioSeparation: disconnect ERROR---%{public}u", result);
        napi_create_int32(env, result, &ret);
        return ret;
    }
    result = g_nodeManager->connectByPort(aissNodeId, node.nextNodeId);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---resetAudioSeparation: connectByPort ERROR---%{public}u", result);
        napi_create_int32(env, result, &ret);
        return ret;
    }

    napi_create_int32(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetAudioSeparation: operation success");
    return ret;
}

static napi_value deleteAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---deleteAudioSeparation IN");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取uuid
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[0], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    OH_AudioSuite_Result result;
    napi_value napiValue = nullptr;
    result = g_nodeManager->removeNode(uuidStr);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---removeNode ERROR---%{public}zd", result);
    }
    napi_create_int64(env, result, &napiValue);
    return napiValue;
}
static napi_value getAudioOfTap(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---getAudioOfTap---IN");

    napi_value napiValue = nullptr;
    void *data;
    napi_create_arraybuffer(env, g_tapDataTotalSize, &data, &napiValue);
    std::copy(
        reinterpret_cast<const char*>(g_aissTapAudioData),
        reinterpret_cast<const char*>(g_aissTapAudioData) + g_tapDataTotalSize,
        reinterpret_cast<char*>(data));
    std::fill(
        reinterpret_cast<char*>(g_aissTapAudioData),
        reinterpret_cast<char*>(g_aissTapAudioData) + g_tapDataTotalSize,
        0);
    g_tapDataTotalSize = 0;
    return napiValue;
}

// 音频播放 -------------------------------------
static OH_AudioRenderer *audioRenderer;
static OH_AudioStreamBuilder *rendererBuilder;
// 实时播放， 用于保存音频数据，具体大小根据需要保存的文件大小而变化
char *g_mixTotalAudioData = (char *)malloc(1024 * 1024 * 100);
// 实时播放需要保存的音频总大小
int32_t g_mixResultTotalSize = 0;
// 待播放的数据大小
int32_t g_mixDataSize = 0;
bool g_oneFinishedFlag = false;
char *g_audioData = (char *)malloc(g_mixDataSize * 5);
auto lastCallbackTime = std::chrono::steady_clock::now();
// 是否录制
bool g_isRecord = false;
static napi_ref callbackAudioRendererRef = nullptr;
static napi_threadsafe_function tsfnBoolean = nullptr;
// 线程安全函数的调用
static void CallBoolThread(napi_env env, napi_value js_callback, void *context, void *data)
{
    int result = *(bool *)data;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest CallBoolThread result: %{public}d", result);
    napi_value resultValue;
    napi_get_boolean(env, result, &resultValue);
    napi_call_function(env, NULL, js_callback, 1, &resultValue, NULL);
    free(data);
}
// 注册回调，获取音频播放的finished的值
static napi_value RegisterFinishedCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value callback;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 创建全局引用
    napi_create_reference(env, args[0], 1, &callbackAudioRendererRef);

    // 创建线程安全函数
    napi_value global;
    napi_get_global(env, &global);
    napi_get_reference_value(env, callbackAudioRendererRef, &callback);
    napi_value name;
    napi_status status = napi_create_string_utf8(env, "CallBooleanCallback", NAPI_AUTO_LENGTH, &name);
    napi_create_threadsafe_function(env, callback, NULL, name, 1, 1, NULL, NULL, NULL, CallBoolThread, &tsfnBoolean);

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

// 调用回调函数的方法
void CallBooleanCallback(int result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest CallBooleanCallback result: %{public}d", result);
    if (tsfnBoolean == nullptr) {
        return;
    }

    int *data = (int *)malloc(sizeof(int));
    if (data == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Failed to allocate memory for data");
        return;
    }
    *data = result;

    napi_call_threadsafe_function(tsfnBoolean, data, napi_tsfn_blocking);
}

static napi_value Record(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest Record start");
    g_isRecord = true;
    return nullptr;
}

static OH_AudioSuite_Result ProcessPipeline()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest ProcessPipeline start");
    // 获取管线状态
    OH_AudioSuite_PipelineState pipeLineState;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetPipelineState(audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // 启动管线
    if (pipeLineState != OH_AudioSuite_PipelineState::AUDIOSUITE_PIPELINE_RUNNING) {
        result = OH_AudioSuiteEngine_StartPipeline(audioSuitePipeline);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            return result;
        }
    }
    return result;
}

static OH_AudioSuite_Result OneRenDerFrame(int32_t audioDataSize)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OneRenDerFrame start");
    ProcessPipeline();
    if (audioDataSize <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize is %{public}d",
            static_cast<int>(audioDataSize));
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    char *audioData = (char *)malloc(audioDataSize);
    int32_t writeSize = 0;
    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_RenderFrame(audioSuitePipeline, audioData, audioDataSize, &writeSize, &g_oneFinishedFlag);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize: %{public}d, writeSize:%{public}d "
        "g_oneFinishedFlag : %{public}s, result: %{public}d",
        audioDataSize, writeSize, (g_oneFinishedFlag ? "true" : "false"), static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame result is %{public}d", static_cast<int>(result));
    }
    if (writeSize <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame writeSize is %{public}d", static_cast<int>(writeSize));
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    // 每次保存一次获取的buffer值
    g_audioData = (char *)malloc(writeSize);
    std::copy(
        reinterpret_cast<const char*>(audioData),
        reinterpret_cast<const char*>(audioData) + writeSize,
        reinterpret_cast<char*>(g_audioData));
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_RenderFrame writeSize : %{public}d, g_oneFinishedFlag: %{public}s",
        writeSize, (g_oneFinishedFlag ? "true" : "false"));
    return result;
}

static napi_value RealTimeSaveFileBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest RealTimeSaveFileBuffer start");
    ResetAllIsResetTotalWriteAudioDataSize();
    Clear();
    g_isRecord = false;
    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest RealTimeSaveFileBuffer g_mixResultTotalSize is %{public}d", g_mixResultTotalSize);
    napi_status status = napi_create_arraybuffer(env, g_mixResultTotalSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest napi_create_arraybuffer status: %{public}d",
            static_cast<int>(status));
        g_mixResultTotalSize = 0;
        if (g_mixTotalAudioData != nullptr) {
            free(g_mixTotalAudioData);
            g_mixTotalAudioData = nullptr;
        }
        // 创建 ArrayBuffer 失败，返回一个大小为 0 的ArrayBuffer
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        std::copy(
            reinterpret_cast<const char*>(g_mixTotalAudioData),
            reinterpret_cast<const char*>(g_mixTotalAudioData) + g_mixResultTotalSize,
            reinterpret_cast<char*>(arrayBufferData));
        if (g_mixTotalAudioData != nullptr) {
            free(g_mixTotalAudioData);
            g_mixTotalAudioData = nullptr;
        }
        g_mixResultTotalSize = 0;
        return napiValue;
    }
}

static OH_AudioData_Callback_Result NewAudioRendererOnWriteData(OH_AudioRenderer * renderer, void *userData,
                                                                void *audioData, int32_t audioDataSize)
                                                                {
    if (renderer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest NewAudioRendererOnWriteData renderer is nullptr");
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest NewAudioRendererOnWriteData audioData is nullptr");
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    
    if (!g_oneFinishedFlag) {
        OneRenDerFrame(audioDataSize);
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "g_isRecord: %{public}s", g_isRecord ? "true" : "false");
        // 每次保存一次获取的buffer值
        if (audioDataSize != 0 && g_isRecord == true) {
            std::copy(
                reinterpret_cast<const char*>(g_audioData),
                reinterpret_cast<const char*>(g_audioData) + audioDataSize,
                reinterpret_cast<char*>(g_mixTotalAudioData) + g_mixResultTotalSize);
            g_mixResultTotalSize += audioDataSize;
        }
    }
    // 播放音频数据
    if (g_audioData != nullptr) {
        std::copy(
            reinterpret_cast<const char*>(g_audioData),
            reinterpret_cast<const char*>(g_audioData) + audioDataSize,
            reinterpret_cast<char*>(audioData));
    }
    if (g_oneFinishedFlag) {
        OH_AudioRenderer_Stop(audioRenderer);
        ResetAllIsResetTotalWriteAudioDataSize();
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest NewAudioRendererOnWriteData g_mixResultTotalSize is %{public}d",
            g_mixResultTotalSize);
        CallBooleanCallback(g_oneFinishedFlag);
        g_oneFinishedFlag = false;
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        g_writeIndex = 0;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest NewAudioRendererOnWriteData g_mixResultTotalSize: %{public}d, audioDataSize: %{public}d",
        g_mixResultTotalSize, audioDataSize);
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

static napi_value AudioRendererInit(napi_env env, napi_callback_info info)
{
    if (audioRenderer) {
        // 释放播放实例
        OH_AudioRenderer_Release(audioRenderer);
        // 释放构造器
        OH_AudioStreamBuilder_Destroy(rendererBuilder);

        audioRenderer = nullptr;
        rendererBuilder = nullptr;
    }
    // create builder
    OH_AudioStream_Type type = OH_AudioStream_Type::AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStreamBuilder_Create(&rendererBuilder, type);

    // 获取位深
    int32_t bitsPerSample = 0;
    OH_AudioStream_SampleFormat streamSampleFormat;
    if (audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_U8) {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_U8);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_U8;
    } else if (audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE) {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_S16LE);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S16LE;
    } else if (audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE) {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_S24LE);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S24LE;
    }  else {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_F32LE);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_F32LE;
    }

    // 设置音频采样率。
    OH_AudioStreamBuilder_SetSamplingRate(rendererBuilder, audioFormatOutput.samplingRate);
    // 设置音频声道。
    OH_AudioStreamBuilder_SetChannelCount(rendererBuilder, audioFormatOutput.channelCount);
    // 设置音频采样格式。
    OH_AudioStreamBuilder_SetSampleFormat(rendererBuilder, streamSampleFormat);
    // 设置音频流的编码类型。
    OH_AudioStreamBuilder_SetEncodingType(rendererBuilder, AUDIOSTREAM_ENCODING_TYPE_RAW);
    // 设置输出音频流的工作场景。
    OH_AudioStreamBuilder_SetRendererInfo(rendererBuilder, AUDIOSTREAM_USAGE_MUSIC);
    // 设置 audioDataSize 长度 （待播放的数据大小）
    g_mixDataSize = SAMPLINGRATE_MULTI * audioFormatOutput.samplingRate *
        audioFormatOutput.channelCount / CHANNELCOUNT_MULTI * bitsPerSample / BITSPERSAMPLE_MULTI;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AudioRendererInit g_mixDataSize: %{public}d, samplingRate: %{public}d, "
        "channelCount: %{public}d, bitsPerSample: %{public}d",
        g_mixDataSize, audioFormatOutput.samplingRate, audioFormatOutput.channelCount, bitsPerSample);
    OH_AudioStreamBuilder_SetFrameSizeInCallback(rendererBuilder, g_mixDataSize);

    // 配置写入音频数据回调函数。
    OH_AudioRenderer_OnWriteDataCallback rendererCallbacks = NewAudioRendererOnWriteData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(rendererBuilder, rendererCallbacks, nullptr);

    // create OH_AudioRenderer
    OH_AudioStreamBuilder_GenerateRenderer(rendererBuilder, &audioRenderer);
    return nullptr;
}

static napi_value AudioRendererDestory(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    if (audioRenderer) {
        // 释放播放实例
        OH_AudioStream_Result result = OH_AudioRenderer_Release(audioRenderer);
        // 释放构造器
        result = OH_AudioStreamBuilder_Destroy(rendererBuilder);
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        audioRenderer = nullptr;
        rendererBuilder = nullptr;
    }
    return napiValue;
}

// 开始播放
static napi_value AudioRendererStart(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "writeDataBufferMap_ size %{public}d",
        writeDataBufferMap_.size());
    ProcessPipeline();
    // start
    OH_AudioRenderer_Start(audioRenderer);
    return nullptr;
}

// 暂停播放
static napi_value AudioRendererPause(napi_env env, napi_callback_info info)
{
    // pause
    OH_AudioRenderer_Pause(audioRenderer);
    return nullptr;
}

// 停止播放
static napi_value AudioRendererStop(napi_env env, napi_callback_info info)
{
    // stop
    g_isRecord = false;
    OH_AudioRenderer_Stop(audioRenderer);
    return nullptr;
}

// 获取播放状态
static napi_value GetRendererState(napi_env env, napi_callback_info info)
{
    OH_AudioStream_State state;
    OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    napi_value sum;
    napi_create_int32(env, state, &sum);

    return sum;
}

// 是否重置totalWriteAudioDataSize
static napi_value ResetTotalWriteAudioDataSize(napi_env env, napi_callback_info info)
{
    // 写入音频的buffer，重头开始
    ResetAllIsResetTotalWriteAudioDataSize();
    // 报错音频的也重头开始保存
    g_mixResultTotalSize = 0;
    return nullptr;
}

//获取效果节点options
static napi_value getOptions(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value napiValue;
    
    //获取nodeId
    std::string nodeId;
    parseNapiString(env, argv[0], nodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "getOptions nodeId is %{public}s", nodeId.c_str());
    Node node = g_nodeManager->GetNodeById(nodeId);
    //根据不同效果类型获取效果参数
    std::string type = g_nodeManager->getOptionsByType(node);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "getOptions type is %{public}s", type.c_str());
    napi_create_string_utf8(env, type.c_str(), NAPI_AUTO_LENGTH, &napiValue);
    return napiValue;
}

EXTERN_C_START static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"record", nullptr, Record, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererInit", nullptr, AudioRendererInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererDestory", nullptr, AudioRendererDestory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererStart", nullptr, AudioRendererStart, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererPause", nullptr, AudioRendererPause, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererStop", nullptr, AudioRendererStop, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getRendererState", nullptr, GetRendererState, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"registerFinishedCallback", nullptr, RegisterFinishedCallback, nullptr, nullptr, nullptr, napi_default,
            nullptr},
        {"resetTotalWriteAudioDataSize", nullptr, ResetTotalWriteAudioDataSize, nullptr, nullptr, nullptr, napi_default,
            nullptr},
        {"realTimeSaveFileBuffer", nullptr, RealTimeSaveFileBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioEditNodeInit", nullptr, AudioEditNodeInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioInAndOutInit", nullptr, AudioInAndOutInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioEditDestory", nullptr, AudioEditDestory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFormat", nullptr, SetFormat, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setEquailizerMode", nullptr, SetEquailizerMode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setEqualizerFrequencyBandGains", nullptr, SetEqualizerFrequencyBandGains, nullptr, nullptr, nullptr,
            napi_default, nullptr},
        {"saveFileBuffer", nullptr, SaveFileBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startFieldEffect", nullptr, startFieldEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startVBEffect", nullptr, startVBEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addAudioSeparation", nullptr, addAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addNoiseReduction", nullptr, addNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stopNoiseReduction", nullptr, stopNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetFieldEffect", nullptr, resetFieldEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetVBEffect", nullptr, resetVBEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetAudioSeparation", nullptr, resetAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteNoiseReduction", nullptr, deleteNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteSong", nullptr, DeleteSong, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteAudioSeparation", nullptr, deleteAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getAudioOfTap", nullptr, getAudioOfTap, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startEnvEffect", nullptr, startEnvEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetEnvEffect", nullptr, resetEnvEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"compareTwoFilesBinary", nullptr, compareTwoFilesBinary, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteNode", nullptr, DeleteNode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getOptions", nullptr, getOptions, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
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
