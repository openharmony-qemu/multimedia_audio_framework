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
#ifndef AUDIO_SUITE_MANAGER_PRIVATE_H
#define AUDIO_SUITE_MANAGER_PRIVATE_H

#include <memory>
#include <vector>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <string>
#include <stdexcept>
#include <cstdint>
#include "audio_suite_manager.h"
#include "audio_suite_manager_callback.h"
#include "audio_suite_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteManager : public IAudioSuiteManager,
                          public AudioSuiteManagerCallback {
public:
    AudioSuiteManager() = default;
    ~AudioSuiteManager() = default;

    // engine
    int32_t Init() override;
    int32_t DeInit() override;

    // pipeline
    int32_t CreatePipeline(uint32_t &pipelineId, PipelineWorkMode workMode) override;
    int32_t DestroyPipeline(uint32_t pipelineId) override;
    int32_t StartPipeline(uint32_t pipelineId) override;
    int32_t StopPipeline(uint32_t pipelineId) override;
    int32_t GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state) override;

    // node
    int32_t CreateNode(
        uint32_t pipelineId, AudioNodeBuilder& builder, uint32_t &nodeId) override;
    int32_t DestroyNode(uint32_t nodeId) override;
    int32_t BypassEffectNode(uint32_t nodeId, bool bypass) override;
    int32_t GetNodeBypassStatus(uint32_t nodeId, bool &bypassStatus) override;
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override;
    int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t RenderFrame(uint32_t pipelineId,
        uint8_t *audioData,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override;
    int32_t MultiRenderFrame(uint32_t pipelineId,
        AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag) override;
    int32_t SetEqualizerFrequencyBandGains(
        uint32_t nodeId, AudioEqualizerFrequencyBandGains frequencyBandGains) override;
    int32_t SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType) override;
    int32_t SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType) override;
    int32_t SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType) override;
    int32_t GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType) override;
    int32_t GetSoundFiledType(uint32_t nodeId, SoundFieldType &soundFieldType) override;
    int32_t GetEqualizerFrequencyBandGains(uint32_t nodeId,
        AudioEqualizerFrequencyBandGains &frequencyBandGains) override;
    int32_t GetVoiceBeautifierType(uint32_t nodeId,
        VoiceBeautifierType &voiceBeautifierType) override;

    // callback Member functions
    void OnCreatePipeline(int32_t result, uint32_t pipelineId) override;
    void OnDestroyPipeline(int32_t result) override;
    void OnStartPipeline(int32_t result) override;
    void OnStopPipeline(int32_t result) override;
    void OnGetPipelineState(AudioSuitePipelineState state) override;
    void OnCreateNode(int32_t result, uint32_t nodeId) override;
    void OnDestroyNode(int32_t result) override;
    void OnBypassEffectNode(int32_t result) override;
    void OnGetNodeBypass(int32_t result, bool bypassStatus) override;
    void OnSetAudioFormat(int32_t result) override;
    void OnWriteDataCallback(int32_t result) override;
    void OnConnectNodes(int32_t result) override;
    void OnDisConnectNodes(int32_t result) override;
    void OnRenderFrame(int32_t result, uint32_t pipelineId) override;
    void OnMultiRenderFrame(int32_t result, uint32_t pipelineId) override;
    void OnGetOptions(int32_t result) override;

private:
    void WriteSuiteEngineUtilizationStatsEvent(AudioNodeType nodeType);
    void WriteSuiteEngineExceptionEvent(uint32_t scene, uint32_t result, std::string description);

private:
    std::mutex lock_;
    std::shared_ptr<IAudioSuiteEngine> suiteEngine_ = nullptr;

    std::unordered_map<uint32_t, std::unique_ptr<std::mutex>> pipelineLockMap_;
    std::unordered_map<uint32_t, std::unique_ptr<std::mutex>> pipelineCallbackMutexMap_;
    std::unordered_map<uint32_t, std::unique_ptr<std::condition_variable>> pipelineCallbackCVMap_;

    // for status operation wait and notify
    std::mutex callbackMutex_;
    std::condition_variable callbackCV_;

    bool isFinishCreatePipeline_ = false;
    uint32_t engineCreatePipelineId_ = INVALID_PIPELINE_ID;
    int32_t engineCreateResult_ = 0;
    bool isFinishDestroyPipeline_ = false;
    int32_t destroyPipelineResult_ = 0;
    bool isFinishStartPipeline_ = false;
    int32_t startPipelineResult_ = 0;
    bool isFinishStopPipeline_ = false;
    int32_t stopPipelineResult_ = 0;
    bool isFinishGetPipelineState_ = false;
    AudioSuitePipelineState getPipelineState_ = PIPELINE_STOPPED;
    bool isFinishCreateNode_ = false;
    int32_t engineCreateNodeResult_ = 0;
    uint32_t engineCreateNodeId_ = INVALID_NODE_ID;
    bool isFinishDestroyNode_ = false;
    int32_t destroyNodeResult_ = 0;
    bool isFinishBypassEffectNode_ = false;
    int32_t bypassEffectNodeResult_ = 0;
    bool isFinishGetNodeBypassStatus_ = false;
    bool getNodeBypassResult_ = false;
    bool isFinishSetFormat_ = false;
    int32_t setFormatResult_ = 0;
    bool isFinishSetWriteData_ = false;
    int32_t setWriteDataResult_ = 0;
    bool isFinishConnectNodes_ = false;
    int32_t connectNodesResult_ = 0;
    bool isFinishDisConnectNodes_ = false;
    int32_t disConnectNodesResult_ = 0;
    bool isFinishGetOptions_ = false;
    int32_t getOptionsResult_ = 0;
    std::unordered_map<uint32_t, bool> isFinishRenderFrameMap_;
    std::unordered_map<uint32_t, int32_t> renderFrameResultMap_;
    std::unordered_map<uint32_t, bool> isFinishMultiRenderFrameMap_;
    std::unordered_map<uint32_t, int32_t> multiRenderFrameResultMap_;
};

// tool
template<typename T>
void ParseValue(const std::string valueStr, T &result)
{
    std::istringstream iss(valueStr);
    float value;
    iss >> value;
    result = static_cast<T>(value);
}

void ParseValue(const std::string &valueStr, int32_t *result)
{
    if (result == nullptr) {
        return;
    }
    std::istringstream iss(valueStr);
    std::string token;
    std::vector<int32_t> temp;

    while (std::getline(iss, token, ':')) {
        token.erase(0, token.find_first_not_of(' ')); // Remove leading spaces
        token.erase(token.find_last_not_of(' ') + 1); // Remove trailing spaces
        if (!token.empty()) {
            char* end;
            errno = 0; // Reset error flag
            long val = std::strtol(token.c_str(), &end, 10);
            // Check if conversion was fully successful and without overflow
            if (end != token.c_str() + token.size() || // Not entire string consumed
                errno == ERANGE || // Numeric overflow
                val < INT32_MIN || val > INT32_MAX) { // Out of int32_t range
                return; // Conversion failed, return immediately
            }
            temp.push_back(static_cast<int32_t>(val));
        }
    }

    for (size_t i = 0; i < temp.size(); ++i) {
        result[i] = temp[i];
    }
}

int32_t StringToInt32(std::string &str)
{
    char* end;
    errno = 0; // Reset error flag
    long value = std::strtol(str.c_str(), &end, 10); // Decimal conversion

    // Check if entire string was parsed
    if (end == str.c_str()) {
        return INT32_MAX;
    }

    // Check if remaining characters are only whitespace (optional)
    while (*end != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*end))) {
            return INT32_MAX;
        }
        ++end;
    }

    // Check overflow/underflow
    if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
        return INT32_MAX;
    }

    return static_cast<int32_t>(value);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_MANAGER_PRIVATE_H