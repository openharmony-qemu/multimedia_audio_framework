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
    // Retrieve the corresponding pipelineManager based on the pipelineId
    std::string pipelineId;
    // Retrieve the corresponding audio data from writeDataBufferMap_ based on inputId
    std::string inputId;
    // Total audio data size
    int32_t bufferSize;
    // Size of audio data already written
    size_t totalWriteAudioDataSize;
    // Is the audio written from the beginning?
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