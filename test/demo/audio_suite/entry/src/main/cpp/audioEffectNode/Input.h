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

// 需要写入的音频数据大小
extern int32_t g_totalSize;

extern OH_AudioFormat g_audioFormatInput;

// 写入音频数据的map
extern std::map<std::string, std::vector<uint8_t>> g_writeDataBufferMap;

// 定义一个结构体来存储ID和数字
struct UserData {
    std::string id;                        // 根据id去g_writeDataBufferMap获取对应的音频数据
    int32_t bufferSize;                    // 音频总数据大小
    int32_t totalWriteAudioDataSize;       // 已经写入的音频数据大小
    bool isResetTotalWriteAudioDataSize;   // 音频是否从头开始写入
};

// 存储UserData的map
extern std::map<std::string, UserData *> g_userDataMap;

// 创造 output builder 构造器
extern OH_AudioNodeBuilder *builderOut;

struct AudioParams {
    std::string inputId;
    std::string outputId;
    std::string mixerId;
    unsigned int fd;
    unsigned int fileLength;
};

struct UpdateInputNodeParams {
    std::string inputId;
    unsigned int channels;
    unsigned int sampleRate;
    unsigned int bitsPerSample;
};

napi_status parseArguments(napi_env env, napi_value *argv, AudioParams &params);

void resetAllIsResetTotalWriteAudioDataSize();

bool getAudioProperties(OH_AVFormat *trackFormat, int32_t &sampleRate, int32_t &channels, int32_t &bitsPerSample);

void runAudioThread(OH_AVDemuxer *demuxer, int32_t fileLength);

void storeTotalBuffToMap(const char *totalBuff, int32_t size, const std::string &key);

void createInputNode(napi_env env, const std::string &inputId, napi_value &napiValue, OH_AudioSuite_Result &result);

OH_AudioSuite_Result setParamsAndWriteData(OH_AudioNodeBuilder *builder, std::string inputId, OH_AudioNode_Type type);

bool checkParameters(OH_AudioNode *audioNode, void *audioData, bool *finished);

int32_t writeDataCallBack(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize, bool *finished);

void updateInputNode(napi_value &napiValue, OH_AudioSuite_Result &result, const UpdateInputNodeParams &params);

void manageOutputNodes(napi_env env, const std::string &inputId, const std::string &outputId, const std::string &mixerId, OH_AudioSuite_Result &result);

void manageExistingOutputNodes(const std::string &inputId, const std::string &mixerId, OH_AudioSuite_Result &result, std::vector<Node> outPutNodes);

void createAndConnectOutputNodes(const std::string &inputId, const std::string &outputId, OH_AudioSuite_Result &result);

#endif //AUDIOEDITTESTAPP_INPUT_H