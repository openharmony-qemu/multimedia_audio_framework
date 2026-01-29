/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

#include <thread>
#include <sstream>
#include "Input.h"
#include "./utils/Utils.h"
#include "./Output.h"
#include "hilog/log.h"
#include "../callback/RegisterCallback.h"
#include "timeline/Timeline.h"
#include "utils/Constant.h"

#include <multimedia/player_framework/native_avcodec_base.h>

const int GLOBAL_RESMGR = 0xFF00;
const char *INPUT_TAG = "[AudioEditTestApp_Input_cpp]";

OH_AudioSuitePipeline *g_audioSuitePipeline = nullptr;

OH_AudioSuiteEngine *g_audioSuiteEngine = nullptr;

char *g_totalBuff = (char *)malloc(8 * 1024 * 1024);

int32_t g_totalSize = 0;

std::map<std::string, std::vector<uint8_t>> g_writeDataBufferMap = {};

std::map<std::string, UserData *> g_userDataMap = {};

OH_AudioFormat g_audioFormatInput = {
    .encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW
};

// Create output builder constructor
OH_AudioNodeBuilder *builderOut = nullptr;

const uint32_t INPUT_ID_INDEX = 0;
const uint32_t OUTPUT_ID_INDEX = 1;
const uint32_t MIXER_ID_INDEX = 2;
const uint32_t NODE_IDS_LENGTH = 3;

napi_status ParseArguments(napi_env env, napi_callback_info info, AudioParams &params)
{
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_valuetype type;
    napi_typeof(env, argv[ARG_0], &type);
    if (type != napi_object) {
        napi_throw_type_error(env, "EINVAL", "nodeIds must be an array");
        delete [] argv;
        return napi_object_expected;
    }
    bool sArray;
    napi_is_array(env, argv[ARG_0], &sArray);
    if (!sArray) {
        napi_throw_type_error(env, "EINVAL", "nodeIds must be an array");
        delete [] argv;
        return napi_array_expected;
    }
    uint32_t nodeIdsLength;
    napi_get_array_length(env, argv[ARG_0], &nodeIdsLength);
    if (nodeIdsLength != NODE_IDS_LENGTH) {
        napi_throw_type_error(env, "EINVAL", "nodeIds length not equal 3");
        delete [] argv;
        return napi_invalid_arg;
    }
    std::vector<std::string> nodeIds;
    for (uint32_t i = 0; i < nodeIdsLength; i++) {
        napi_value element;
        napi_get_element(env, argv[ARG_0], i, &element);
        napi_typeof(env, element, &type);
        if (type != napi_string) {
            napi_throw_type_error(env, "EINVAL", "nodeIds must contain only strings");
            delete [] argv;
            return napi_string_expected;
        }
        std::string tempString;
        napi_status status = ParseNapiString(env, element, tempString);
        nodeIds.push_back(tempString);
    }
    params.inputId = nodeIds[INPUT_ID_INDEX];
    params.outputId = nodeIds[OUTPUT_ID_INDEX];
    params.mixerId = nodeIds[MIXER_ID_INDEX];
    napi_status status = napi_get_value_uint32(env, argv[ARG_1], &params.fd);
    status = napi_get_value_uint32(env, argv[ARG_2], &params.fileLength);
    status = napi_get_value_int64(env, argv[ARG_3], &params.startTime);
    delete[] argv;
    return status;
}

void ResetAllIsResetTotalWriteAudioDataSize()
{
    for (auto &pair : g_userDataMap) {
        pair.second->isResetTotalWriteAudioDataSize = true;
    }
}

bool GetAudioProperties(OH_AVFormat *trackFormat, int32_t* sampleRate, int32_t* channels, int32_t* bitsPerSample)
{
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUD_SAMPLE_RATE, sampleRate)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "get sample rate failed");
        return false;
    }
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUD_CHANNEL_COUNT, channels)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "get channel count failed");
        return false;
    }
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, bitsPerSample)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "get bits per sample failed");
        return false;
    }
    OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, INPUT_TAG,
        "sampleRate: %{public}d, channels: %{public}d, bitsPerSample: %{public}d", *sampleRate,
        *channels, *bitsPerSample);
    // Set Sampling Rate
    g_audioFormatInput.samplingRate = SetSamplingRate(*sampleRate);
    // Set audio channels
    g_audioFormatInput.channelCount = *channels;
    g_audioFormatInput.channelLayout = SetChannelLayout(*channels);
    // Set bit depth
    g_audioFormatInput.sampleFormat = SetSampleFormat(*bitsPerSample);
    // Set the encoding format
    g_audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    
    g_audioFormatOutput.samplingRate = g_audioFormatInput.samplingRate;
    g_audioFormatOutput.channelCount = g_audioFormatInput.channelCount;
    g_audioFormatOutput.channelLayout = g_audioFormatInput.channelLayout;
    g_audioFormatOutput.sampleFormat = g_audioFormatInput.sampleFormat;
    g_audioFormatOutput.encodingType = g_audioFormatInput.encodingType;

    return true;
}

void GetPcmBuffer(OH_AVDemuxer *&demuxer, const uint32_t &trackIndex, OH_AVBuffer *&pcmBuffer,
                  const std::shared_ptr<char> &totalBuffer, OH_AVCodecBufferAttr &info)
{
    int32_t ret;
    bool flag = true;
    while (flag) {
        ret = OH_AVDemuxer_ReadSampleBuffer(demuxer, trackIndex, pcmBuffer);
        if (ret == AV_ERR_OK) {
            OH_AVBuffer_GetBufferAttr(pcmBuffer, &info);
            // Copy the data of the current sample into totalBuff
            std::copy(reinterpret_cast<char *>(OH_AVBuffer_GetAddr(pcmBuffer)),
                      reinterpret_cast<char *>(OH_AVBuffer_GetAddr(pcmBuffer)) + info.size,
                      totalBuffer.get() + g_totalSize);
            g_totalSize += info.size;
            if (info.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                flag = false;
            }
        } else {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                         "get pcmBuffer failed, ret: %{public}d, trackIndex: %{public}d", ret, trackIndex);
            break;
        }
    }
}

void ReadTrackSamples(OH_AVDemuxer *demuxer, uint32_t trackIndex, int bufferSize, std::string inputId,
                      AudioFormat format)
{
    g_totalSize = 0;
    g_totalBuff = nullptr;
    // Add Decapsulation Track
    if (OH_AVDemuxer_SelectTrackByID(demuxer, trackIndex) != AV_ERR_OK) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "select audio track failed: %{public}d", trackIndex);
    }
    if (bufferSize <= 0) { return; }
    OH_AVBuffer *pcmBuffer = OH_AVBuffer_Create(bufferSize);
    if (pcmBuffer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "create pcmBuffer failed");
        return;
    }
    auto chars = new char[bufferSize];
    std::shared_ptr<char> totalBuffer(chars, [](char* p) { delete[] p; });
    OH_AVCodecBufferAttr info;
    GetPcmBuffer(demuxer, trackIndex, pcmBuffer, totalBuffer, info);
    runAudioWaveThread(format, totalBuffer);
    // Adds audio and stores the audio buffer to the map.
    std::string key = inputId;
    if (format.startTime > 0) {
        key = inputId.c_str() + std::to_string(format.startTime);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                     "ReadTrackSamples key: %{public}s, inputId: %{public}s, startTime: %{public}ld, "
                     "std::to_string(format.startTime): %{public}s",
            key.c_str(), inputId.c_str(), format.startTime, std::to_string(format.startTime).c_str());
    }
    StoreTotalBuffToMap(totalBuffer.get(), g_totalSize, key);
    AudioAsset asset{
        startTime : format.startTime,
        endTime : format.startTime +
            GetAudioDuration(g_totalSize, format.sampleRate, format.channels, format.bitsPerSample),
        pcmBufferLength: g_totalSize,
        sampleRate: format.sampleRate,
        channels: format.channels,
        bitsPerSample: format.bitsPerSample,
    };
    AudioTrack track{
        trackId : inputId,
        isSilent : false,
        assets : {{format.startTime, asset}},
        maxEndTime : asset.endTime,
        currentTime : 0
    };
    Timeline::GetInstance().AddAudioTrack(track);
    CallAudioCacheCallback(false);
    OH_AVBuffer_Destroy(pcmBuffer);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                 "ReadTrackSamples end, g_totalSize: %{public}d, endTime: %{public}d, maxEndTime: %{public}d",
                 g_totalSize, asset.endTime, track.maxEndTime);
}

void RunAudioThread(OH_AVDemuxer *demuxer, int32_t fileLength, std::string inputId, AudioFormat& format)
{
    std::thread audioThread(ReadTrackSamples, demuxer, 0, fileLength, inputId, format);
    audioThread.detach();
}

void StoreTotalBuffToMap(const char *totalBuff, int32_t size, const std::string &key)
{
    if (size > 0 && totalBuff != nullptr) {
        std::vector<uint8_t> buffer(totalBuff, totalBuff + size);
        g_writeDataBufferMap[key] = buffer;
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "audioEditTest StoreTotalBuffToMap success");
        auto it = g_writeDataBufferMap.find(key);
        if (it == g_writeDataBufferMap.end()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG,
                         "audioEditTest StoreTotalBuffToMap failed, oldKey is not exist");
        }
        return;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "audioEditTest StoreTotalBuffToMap failed");
}

void CreateInputNode(napi_env env, const std::string &inputId, napi_value &napiValue, OH_AudioSuite_Result &result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG, "audioEditTest CreateInputNode start");
    auto it = g_writeDataBufferMap.find(inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                 "audioEditTest AudioInAndOutInit g_writeDataBufferMap[inputId] length: %zu", it->second.size());
    // Creating a builder constructor
    OH_AudioNodeBuilder *builderIn;
    result = OH_AudioSuiteNodeBuilder_Create(&builderIn);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_Create result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }
    // Transparent transmission node type
    result = OH_AudioSuiteNodeBuilder_SetNodeType(builderIn, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "NodeManagerTest createNode OH_AudioSuiteNodeBuilder_SetNodeType result: %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }

    // Packaging method, setting parameters for audio files, and writing audio files to a buffer
    result = SetParamsAndWriteData(builderIn, inputId, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest SetParamsAndWriteData result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }

    // Creating an input node
    g_nodeManager->createNode(inputId, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT, builderIn);
}

OH_AudioSuite_Result SetParamsAndWriteData(OH_AudioNodeBuilder *builder, std::string inputId, OH_AudioNode_Type type)
{
    OH_AudioSuite_Result result = OH_AudioSuiteNodeBuilder_SetFormat(builder, g_audioFormatInput);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_SetFormat result is %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    if (type != OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT) {
        return result;
    }
    UserData *data = new UserData();
    data->id = inputId;
    void *userData = data;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_SetRequestDataCallback g_totalSize %{public}d", g_totalSize);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest OH_AudioNodeBuilder_SetFormat userData inputId is %{public}s",
        static_cast<UserData *>(userData)->id.c_str());
    // Set the OH_AudioSuiteNodeBuilder_SetRequestDataCallback callback before creating the node
    result = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, WriteDataCallBack, userData);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_SetRequestDataCallback result is %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    // Store the UserData instance in the mapping table
    g_userDataMap[inputId] = data;
    return result;
}

bool CheckParameters(OH_AudioNode *audioNode, void *audioData, bool *finished)
{
    if (audioNode == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest WriteDataCallBack audioNode is nullptr");
        *finished = true;
        return false;
    }
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest WriteDataCallBack audioData is nullptr");
        *finished = true;
        return false;
    }
    if (finished == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest WriteDataCallBack finished is nullptr");
        *finished = true;
        return false;
    }
    return true;
}

void UpdateCurrentTimeAndFinished(bool *&finished, AudioTrack *&track, const long &oneAudioTime)
{
    track->currentTime = track->currentTime + oneAudioTime;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                 "audioEditTest WriteDataCallBack track->currentTime: %{public}d, maxEndTime: %{public}d, "
                 "oneAudioTime: %{public}d",
                 track->currentTime, track->maxEndTime, oneAudioTime);
    if (track->currentTime >= track->maxEndTime) {
        *finished = true;
    }
}

void ProcessCallBack(void *&audioData,const int32_t &audioDataSize,const std::string &inputId, AudioTrack *&track,
                     long &oneAudioTime)
{
    for (auto &pair : track->assets) {
        long key = pair.first;
        AudioAsset value = pair.second;
        // Determine whether to write the data based on endTime in the assets.
        if (value.endTime < track->currentTime) {
            value.currentPcmBufferIndex = 0;
            continue;
        }
        oneAudioTime = GetAudioDuration(audioDataSize, value.sampleRate, value.channels, value.bitsPerSample);
        // Determine whether to write the data based on startTime and oneAudioTime in the assets.
        if (value.startTime > (track->currentTime + oneAudioTime)) {
            continue;
        }
        // Obtain the key of g_writeDataBufferMap.
        std::string bufferMapKey = inputId;
        if (value.startTime > 0) {
            bufferMapKey = bufferMapKey.c_str() + std::to_string(value.startTime);
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG, "bufferMapKey: %{public}s",
                         bufferMapKey.c_str());
        }
        auto it = g_writeDataBufferMap.find(bufferMapKey);
        if (it == g_writeDataBufferMap.end()) {
            // The map does not find the corresponding audio buffer.
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, INPUT_TAG,
                         "audioEditTest WriteDataCallBack g_writeDataBufferMap is end");
            continue;
        }
        // Calculating the remaining data volume
        int remainingDataSize = 0;
        // Determine the actual amount of data written this time.
        int actualDataSize = 0;
        // Length of each audio callback written this time
        int oneStartIndex = oneAudioTime * GetAudioSize(value.sampleRate, value.channels, value.bitsPerSample) / 1000;
        if (track->currentTime <= value.startTime) {
            value.currentPcmBufferIndex = 0;
        }
        if (value.currentPcmBufferIndex >= value.pcmBufferLength) {
            continue;
        }
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                     "audioEditTest WriteDataCallBack bufferMapKey: %{public}s, oneStartIndex: %{public}d, "
                     "value.pcmBufferLength: %{public}d, it->second->size: %{public}d",
                     bufferMapKey.c_str(), oneStartIndex, value.pcmBufferLength, it->second.size());
        // if track->currentTime < value.startTime < track->currentTime + oneAudioTime;
        // In this case, the value.startTime - track->currentTime part is filled with 0s.
        remainingDataSize = value.pcmBufferLength - value.currentPcmBufferIndex;
        actualDataSize = std::min(audioDataSize, remainingDataSize);
        // Write audio data normally
        std::copy(it->second.data() + value.currentPcmBufferIndex,
                  it->second.data() + value.currentPcmBufferIndex + actualDataSize, static_cast<char *>(audioData));
        // If there are not enough, add zeros.
        int32_t padSize = audioDataSize - actualDataSize;
        if (padSize > 0) {
            std::fill_n(static_cast<char *>(audioData) + actualDataSize, padSize, 0);
        }
        pair.second.currentPcmBufferIndex =
            std::min(value.currentPcmBufferIndex + actualDataSize, value.pcmBufferLength);
    }
}

int32_t WriteDataCallBack(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize,
                          bool *finished)
{
    if (!CheckParameters(audioNode, audioData, finished)) {
        return 0;
    }
    // Processing audio is nullptr here, it is an issue with the demo's method of obtaining audio data,
    // not a problem with the underlying interface
    std::string inputId = static_cast<UserData *>(userData)->id;
    // Find the track in audioTrackMap based on inputId.
    AudioTrack* track = Timeline::GetInstance().GetAudioTrack(inputId);
    // There is no this audio track.
    if (track == nullptr) {
        *finished = true;
        return 0;
    }
    // Determine whether muting is required
    if (track->isSilent) {
        return audioDataSize;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
                 "audioEditTest WriteDataCallBack inputId: %{public}s, assets size: %{public}d", inputId.c_str(),
                 track->assets.size());
    long oneAudioTime = 0;
    // Traverse the assets in the track
    ProcessCallBack(audioData, audioDataSize, inputId, track, oneAudioTime);
    // update track->currentTime
    UpdateCurrentTimeAndFinished(finished, track, oneAudioTime);
    return audioDataSize;
}

void UpdateInputNode(OH_AudioSuite_Result &result, const UpdateInputNodeParams &params)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG, "audioEditTest UpdateInputNode start");
    // Set Sampling Rate
    g_audioFormatInput.samplingRate = SetSamplingRate(params.sampleRate);
    // Set audio channels
    g_audioFormatInput.channelCount = params.channels;
    g_audioFormatInput.channelLayout = SetChannelLayout(params.channels);
    // Set bit depth
    g_audioFormatInput.sampleFormat = SetSampleFormat(params.bitsPerSample);
    // Set the encoding format
    g_audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;

    g_audioFormatOutput.samplingRate = g_audioFormatInput.samplingRate;
    g_audioFormatOutput.channelCount = g_audioFormatInput.channelCount;
    g_audioFormatOutput.channelLayout = g_audioFormatInput.channelLayout;
    g_audioFormatOutput.sampleFormat = g_audioFormatInput.sampleFormat;
    g_audioFormatOutput.encodingType = g_audioFormatInput.encodingType;
    
    const std::vector<Node> inPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    result = OH_AudioSuiteEngine_SetAudioFormat(inPutNodes[0].physicalNode, &g_audioFormatInput);
    const std::vector<Node> outPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    result = OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &g_audioFormatOutput);
    // Add audio, store the audio buffer in the map. The memcpy in the previous line can be considered for removal
    if (g_writeDataBufferMap.find(params.inputId) != g_writeDataBufferMap.end()) {
        // Key exists, proceed with deletion
        g_writeDataBufferMap.erase(params.inputId);
    }
    auto it = g_writeDataBufferMap.find(params.inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest AudioInAndOutInit g_writeDataBufferMap[inputId] length: %zu", it->second.size());
    UserData *data = new UserData();
    data->id = params.inputId;
    g_userDataMap[params.inputId] = data;
}

void ManageOutputNodes(napi_env env, const std::string &inputId,
    const std::string &outputId, const std::string &mixerId, OH_AudioSuite_Result &result)
{
    const std::vector<Node> outPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    if (outPutNodes.size() > 0) {
        ManageExistingOutputNodes(inputId, mixerId, result, outPutNodes);
    } else {
        CreateAndConnectOutputNodes(inputId, outputId, result);
    }
}

napi_value ManageInputNodes(napi_env env, AudioParamsByCascad params, OH_AudioSuite_Result &result,
                            napi_value napiValue)
{
    Node inputNode = g_nodeManager->GetNodeById(params.inputId);
    if (inputNode.id.empty()) {
        CreateInputNode(env, params.inputId, napiValue, result);
    } else {
        UpdateInputNodeParams updateInputNodeParams;
        updateInputNodeParams.inputId = params.inputId;
        updateInputNodeParams.channels = params.channels;
        updateInputNodeParams.sampleRate = params.sampleRate;
        updateInputNodeParams.bitsPerSample = params.bitsPerSample;
        UpdateInputNode(result, updateInputNodeParams);
    }
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

void ManageExistingOutputNodes(const std::string &inputId, const std::string &mixerId,
    OH_AudioSuite_Result &result, std::vector<Node> outPutNodes)
{
    const std::vector<Node> mixerNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
    if (mixerNodes.size() > 0) {
        result = g_nodeManager->connect(inputId, mixerNodes[0].id);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest connect input and mixer result: %{public}d", static_cast<int>(result));
    } else {
        result = g_nodeManager->createNode(mixerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest nodeManagerCreateMixerNode result: %{public}d", static_cast<int>(result));

        result = g_nodeManager->insertNode(mixerId, outPutNodes[0].id, Direction::BEFORE);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest insertMixerNode result: %{public}d", static_cast<int>(result));

        result = g_nodeManager->connect(inputId, mixerId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
            "audioEditTest connect inputId and mixerId result: %{public}d", static_cast<int>(result));
    }
}

void CreateAndConnectOutputNodes(const std::string &inputId, const std::string &outputId, OH_AudioSuite_Result &result)
{
    result = OH_AudioSuiteNodeBuilder_Create(&builderOut);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_Create output builder result: %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return;
    }

    result = OH_AudioSuiteNodeBuilder_SetNodeType(builderOut, OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "NodeManagerTest createNode OH_AudioSuiteNodeBuilder_SetNodeType result: %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return;
    }
    // Packaging method, setting parameters for audio files, and writing audio files to a buffer
    result = SetParamsAndWriteData(builderOut, inputId, OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest SetParamsAndWriteData result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return;
    }

    result = g_nodeManager->createNode(outputId, OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT, builderOut);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest nodeManagerCreateOutputNode result: %{public}d", static_cast<int>(result));

    result = g_nodeManager->connect(inputId, outputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest nodeManagerConnectInputAndOutput result: %{public}d", static_cast<int>(result));
}

napi_status ParseArgumentsByCascad(napi_env env, napi_value *argv, AudioParamsByCascad &params)
{
    napi_status status = ParseNapiString(env, argv[ARG_0], params.inputId);
    status = ParseNapiString(env, argv[ARG_1], params.outputId);
    status = ParseNapiString(env, argv[ARG_2], params.mixerId);
    std::string audioFormat;
    status = ParseNapiString(env, argv[ARG_3], audioFormat);
    std::istringstream iss(audioFormat);
    iss >> params.sampleRate >> params.channels >> params.bitsPerSample >> params.pcmBufferSize;
 
    // Set Sampling Rate
    g_audioFormatInput.samplingRate = SetSamplingRate(params.sampleRate);
    // Set audio channels
    g_audioFormatInput.channelCount = params.channels;
    g_audioFormatInput.channelLayout = SetChannelLayout(params.channels);
    // Set bit depth
    g_audioFormatInput.sampleFormat = SetSampleFormat(params.bitsPerSample);
    // Set the encoding format
    g_audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    
    g_audioFormatOutput.samplingRate = g_audioFormatInput.samplingRate;
    g_audioFormatOutput.channelCount = g_audioFormatInput.channelCount;
    g_audioFormatOutput.channelLayout = g_audioFormatInput.channelLayout;
    g_audioFormatOutput.sampleFormat = g_audioFormatInput.sampleFormat;
    g_audioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
 
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, INPUT_TAG,
        "audioEditTest ParseArgumentsByCascad inputId: %{public}s, outputId: %{public}s, mixerId: %{public}s, "
        "sampleRate: %{public}d, channels: %{public}d, bitsPerSample: %{public}d, pcmBufferSize: %{public}d",
        params.inputId.c_str(), params.outputId.c_str(), params.mixerId.c_str(), params.sampleRate,
        params.channels, params.bitsPerSample, params.pcmBufferSize);
    
    return status;
}

void waveThread(const AudioFormat& format, const std::shared_ptr<char>& totalBuffer)
{
    int bytesPerSample = UINT_0;
    switch (format.bitsPerSample) {
        case UINT_0:
            bytesPerSample = UINT_1;
            break;
        case UINT_1:
            bytesPerSample = UINT_2;
            break;
        case UINT_2:
            bytesPerSample = UINT_3;
            break;
        case UINT_3:
        case UINT_4:
            bytesPerSample = UINT_4;
            break;
        default:
            bytesPerSample = UINT_1;
            break;
    }
    int totalSamples = g_totalSize / bytesPerSample;
    float *outResult = new float[totalSamples];
    ConvertToFloat(format.bitsPerSample, totalSamples, totalBuffer.get(), outResult);
    int pointsPerSegment = (format.sampleRate * format.channels) / 50; // 每20ms的采样点数 = (采样率 × 声道数) / 50
    if (pointsPerSegment <= 0) {
        pointsPerSegment = 1;
    }
    std::string floatString = "";
    int count = 0;
    float max = 0;
    for (int i = 0; i < totalSamples; i++) {
        float absValue = std::fabs(outResult[i]);
        if (max < absValue) {
            max = absValue;
        }
        count++;
        if (count >= pointsPerSegment) {
            floatString += " " + std::to_string(max);
            count = 0;
            max = 0;
        }
    }
    delete[] outResult;
    CallStringCallback(floatString);
}

void runAudioWaveThread(const AudioFormat& format, const std::shared_ptr<char>& totalBuffer)
{
    std::thread audioThread(waveThread, format, totalBuffer);
    audioThread.detach();
}