/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H
#include "NodeManager.h"
#include <map>
#include <ohaudio/native_audiostream_base.h>
#include <queue>
#include <thread>

class MultiUserData {
public:
    // 根据pipelineId去获取对应的pipelineManager
    std::string pipelineId;
    // 根据inputId去writeDataBufferMap_获取对应的音频数据
    std::string inputId;
    // 音频总数据大小
    int32_t bufferSize;
    // 已经写入的音频数据大小
    size_t totalWriteAudioDataSize;
     // 音频是否从头开始写入
    bool isResetTotalWriteAudioDataSize;

    MultiUserData();
    MultiUserData(std::string pipelineId, std::string inputId);
    ~MultiUserData();
};

class RenderFrameAsyncParam {
public:
    OH_AudioSuitePipeline* audioSuitePipeline;
    OH_AudioDataArray* ohAudioDataArray;
    char** firstAudioBuffer;
    void* audioData;
    int32_t requestFrameSize;
    int32_t responseSize;
    bool finishedFlag;
    char* firAudioData;
    size_t* firstBufferSize;
    void* secAudioData;
    char** secondAudioBuffer;
    size_t* secondBufferSize;
};

class PipelineManager {
public:
    std::string pipelineId;
    OH_AudioSuitePipeline* audioSuitePipeline;
    std::shared_ptr<NodeManager> nodeManager;
    OH_AudioFormat audioFormatInput;
    OH_AudioFormat audioFormatOutput;
    char* firstAudioBuffer = nullptr;
    char* secondAudioBuffer = nullptr;
    char* playAudioBuffer = nullptr;
    char* inputBuffer = nullptr;
    OH_AudioRenderer *audioRenderer = nullptr;
    bool multiRenderFrameFlag = false;
    size_t firstBufferSize = 0;
    size_t secondBufferSize = 0;
    size_t playAudioBufferSize = 0;
    size_t totalInputDataSize = 0;
    bool renderFrameFinishFlag = false;
    bool recordFlag = false;
    float inputDataProgress = 0.f;
    std::map<std::string, FILE*> writeDataFileMap;
    std::map<std::string, std::shared_ptr<MultiUserData>> userDataMap;
public:
    PipelineManager(std::string pipelineId, OH_AudioSuitePipeline *audioSuitePipeLine,
        std::shared_ptr<NodeManager> nodeManager);
    ~PipelineManager();
};

#endif