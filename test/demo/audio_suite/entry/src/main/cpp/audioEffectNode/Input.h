/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_INPUT_H
#define AUDIOEDITTESTAPP_INPUT_H

#include <string>
#include <map>
#include "./EffectNode.h"
#include "napi/native_api.h"
#include "ohaudio/native_audio_suite_base.h"

#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avdemuxer.h>

extern OH_AudioSuiteEngine *g_audioSuiteEngine;

extern OH_AudioSuitePipeline *g_audioSuitePipeline;

extern char *g_totalBuff;

// Size of audio data to be written
extern int32_t g_totalSize;

extern OH_AudioFormat g_audioFormatInput;

// Map for writing audio data
extern std::map<std::string, std::vector<uint8_t>> g_writeDataBufferMap;

// Define a structure to store ID and number
struct UserData {
    // Retrieve the corresponding audio data from g_writeDataBufferMap based on the ID
    std::string id;
    int32_t bufferSize;                    // Total audio data size
    int32_t totalWriteAudioDataSize;       // Size of audio data already written
    bool isResetTotalWriteAudioDataSize;   // If the audio written from the beginning
};

// Map for storing UserData
extern std::map<std::string, UserData *> g_userDataMap;

// Create output builder constructor
extern OH_AudioNodeBuilder *builderOut;

struct AudioParams {
    std::string inputId;
    std::string outputId;
    std::string mixerId;
    unsigned int fd;
    unsigned int fileLength;
};

class UpdateInputNodeParams {
public:
    std::string inputId;
    unsigned int channels;
    unsigned int sampleRate;
    unsigned int bitsPerSample;

    UpdateInputNodeParams() {}
    UpdateInputNodeParams(std::string inputId, unsigned int channels, unsigned int sampleRate,
        unsigned int bitsPerSample) : inputId(inputId), channels(channels), sampleRate(sampleRate),
        bitsPerSample(bitsPerSample) {}
};

struct AudioParamsByCascad {
    std::string inputId;
    std::string outputId;
    std::string mixerId;
    int32_t channels;
    int32_t sampleRate;
    int32_t bitsPerSample;
    int32_t pcmBufferSize;
};

napi_status ParseArguments(napi_env env, napi_callback_info info, AudioParams &params);

void ResetAllIsResetTotalWriteAudioDataSize();

bool GetAudioProperties(OH_AVFormat *trackFormat, int32_t &sampleRate, int32_t &channels, int32_t &bitsPerSample);

void RunAudioThread(OH_AVDemuxer *demuxer, int32_t fileLength);

void StoreTotalBuffToMap(const char *totalBuff, int32_t size, const std::string &key);

void CreateInputNode(napi_env env, const std::string &inputId, napi_value &napiValue, OH_AudioSuite_Result &result);

OH_AudioSuite_Result SetParamsAndWriteData(OH_AudioNodeBuilder *builder, std::string inputId, OH_AudioNode_Type type);

bool CheckParameters(OH_AudioNode *audioNode, void *audioData, bool *finished);

int32_t WriteDataCallBack(OH_AudioNode *audioNode, void *userData, void *audioData,
    int32_t audioDataSize, bool *finished);

void UpdateInputNode(napi_value &napiValue, OH_AudioSuite_Result &result, const UpdateInputNodeParams &params);

void ManageOutputNodes(napi_env env, const std::string &inputId, const std::string &outputId,
    const std::string &mixerId, OH_AudioSuite_Result &result);

void ManageExistingOutputNodes(const std::string &inputId, const std::string &mixerId,
    OH_AudioSuite_Result &result, std::vector<Node> outPutNodes);

void CreateAndConnectOutputNodes(const std::string &inputId, const std::string &outputId, OH_AudioSuite_Result &result);

napi_status ParseArgumentsByCascad(napi_env env, napi_value *argv, AudioParamsByCascad &params);

#endif //AUDIOEDITTESTAPP_INPUT_H