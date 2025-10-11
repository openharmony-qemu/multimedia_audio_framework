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
    int32_t CreatePipeline(uint32_t &pipelineId) override;
    int32_t DestroyPipeline(uint32_t pipelineId) override;
    int32_t StartPipeline(uint32_t pipelineId) override;
    int32_t StopPipeline(uint32_t pipelineId) override;
    int32_t GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state) override;

    // node
    int32_t CreateNode(
        uint32_t pipelineId, AudioNodeBuilder& builder, uint32_t &nodeId) override;
    int32_t DestroyNode(uint32_t nodeId) override;
    int32_t EnableNode(uint32_t nodeId, AudioNodeEnable audioNodeEnable) override;
    int32_t GetNodeEnableStatus(uint32_t nodeId, AudioNodeEnable &nodeEnable) override;
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override;
    int32_t SetOnWriteDataCallback(uint32_t nodeId,
        std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId,
        AudioNodePortType srcPortType, AudioNodePortType destPortType) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t InstallTap(uint32_t nodeId, AudioNodePortType portType,
        std::shared_ptr<SuiteNodeReadTapDataCallback> callback) override;
    int32_t RemoveTap(uint32_t nodeId, AudioNodePortType portType) override;
    int32_t RenderFrame(uint32_t pipelineId,
        uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag) override;
    int32_t MultiRenderFrame(uint32_t pipelineId,
        AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag) override;
    int32_t SetEqualizerMode(uint32_t nodeId, EqualizerMode eqMode) override;
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
    void OnEnableNode(int32_t result) override;
    void OnGetNodeEnable(AudioNodeEnable enable) override;
    void OnSetAudioFormat(int32_t result) override;
    void OnWriteDataCallback(int32_t result) override;
    void OnConnectNodes(int32_t result) override;
    void OnDisConnectNodes(int32_t result) override;
    void OnInstallTap(int32_t result) override;
    void OnRemoveTap(int32_t result) override;
    void OnRenderFrame(int32_t result) override;
    void OnMultiRenderFrame(int32_t result) override;

private:
    std::mutex lock_;
    std::shared_ptr<IAudioSuiteEngine> suiteEngine_ = nullptr;

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
    bool isFinishEnableNode_ = false;
    int32_t enableNodeResult_ = 0;
    bool isFinishGetNodeEnable_ = false;
    AudioNodeEnable getNodeEnable_ = NODE_DISABLE;
    bool isFinishSetFormat_ = false;
    int32_t setFormatResult_ = 0;
    bool isFinishSetWriteData_ = false;
    int32_t setWriteDataResult_ = 0;
    bool isFinishConnectNodes_ = false;
    int32_t connectNodesResult_ = 0;
    bool isFinishDisConnectNodes_ = false;
    int32_t disConnectNodesResult_ = 0;
    bool isFinisRenderFrame_ = false;
    int32_t renderFrameResult_ = 0;
    bool isFinisMultiRenderFrame_ = false;
    int32_t MultiRenderFrameResult_ = 0;
    bool isFinisInstallTap_ = false;
    int32_t installTapResult_ = 0;
    bool isFinisRemoveTap_ = false;
    int32_t removeTapResult_ = 0;
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
    std::istringstream iss(valueStr);
    std::string token;
    std::vector<int32_t> temp;

    while (std::getline(iss, token, ':')) {
        token.erase(0, token.find_first_not_of(' ')); // 移除前导空格
        token.erase(token.find_last_not_of(' ') + 1); // 移除后缀空格
        if (!token.empty()) {
            temp.push_back(std::stoi(token));
        }
    }

    for (size_t i = 0; i < temp.size(); ++i) {
        result[i] = temp[i];
    }
}

int32_t StringToInt32(std::string &str)
{
    char* end;
    errno = 0; // 重置错误标志
    long value = std::strtol(str.c_str(), &end, 10); // 十进制转换

    // 检查整个字符串是否被解析
    if (end == str.c_str()) {
        return INT32_MAX;
    }

    // 检查剩余字符是否仅为空白符（可选）
    while (*end != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*end))) {
            return INT32_MAX;
        }
        ++end;
    }

    // 检查溢出/下溢
    if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
        return INT32_MAX;
    }

    return static_cast<int32_t>(value);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_MANAGER_PRIVATE_H