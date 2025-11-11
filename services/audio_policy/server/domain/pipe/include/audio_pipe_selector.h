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
#ifndef AUDIO_PIPE_SELECTOR_H
#define AUDIO_PIPE_SELECTOR_H

#include <vector>
#include "audio_pipe_manager.h"
#include "audio_stream_info.h"
#include "audio_policy_config_manager.h"
#include "audio_concurrency_parser.h"

namespace OHOS {
namespace AudioStandard {

class AudioPipeSelector {
public:
    AudioPipeSelector();
    ~AudioPipeSelector() = default;

    static std::shared_ptr<AudioPipeSelector> GetPipeSelector();

    std::vector<std::shared_ptr<AudioPipeInfo>> FetchPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    std::vector<std::shared_ptr<AudioPipeInfo>> FetchPipesAndExecute(
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void UpdateRendererPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc);

private:
    void UpdateDeviceStreamInfo(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo);
    void ScanPipeListForStreamDesc(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList,
        std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void UpdateProcessConcurrency(AudioPipeType existingPipe, AudioPipeType commingPipe,
        ConcurrencyAction &action);
    bool ProcessConcurrency(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove);
    void CheckAndHandleIncomingConcurrency(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream);
    uint32_t GetRouteFlagByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    std::string GetAdapterNameByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void ConvertStreamDescToPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info);
    AudioStreamAction JudgeStreamAction(std::shared_ptr<AudioPipeInfo> newPipe, std::shared_ptr<AudioPipeInfo> oldPipe);
    void SortStreamDescsByStartTime(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    AudioPipeType GetPipeType(uint32_t flag, AudioMode audioMode);
    void HandlePipeNotExist(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsSameAdapter(std::shared_ptr<AudioStreamDescriptor> streamDescA,
        std::shared_ptr<AudioStreamDescriptor> streamDescB);
    void DecideFinalRouteFlag(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void ProcessNewPipeList(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void DecidePipesAndStreamAction(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo);
    void MoveStreamsToNormalPipes(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove,
        std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList);
    void RemoveTargetStreams(std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove,
        std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList,
        std::map<std::shared_ptr<AudioStreamDescriptor>, std::string> &streamToAdapter);
    void AddStreamToPipeAndUpdateAction(std::shared_ptr<AudioStreamDescriptor> &streamToAdd,
        std::shared_ptr<AudioPipeInfo> &pipe);
    void ProcessModemCommunicationConcurrency(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsMoveToNormal);
    void SetOriginalFlagForcedNormalIfNeed(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void ProcessRendererAndCapturerConcurrency(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsNeedTempMoveToNormal(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo);
    bool FindExistingPipe(std::vector<std::shared_ptr<AudioPipeInfo>> &selectedPipeInfoList,
        const std::shared_ptr<AdapterPipeInfo> &pipeInfoPtr, const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo);
    void MatchRemoteOffloadPipe(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
        std::shared_ptr<AudioPipeInfo> pipeInfo, const std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void UpdatePipeInfoFromStreamProp(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info);
    bool IsPipeFormatMatch(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
        std::shared_ptr<AudioPipeInfo> pipeInfo);

    AudioPolicyConfigManager& configManager_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PIPE_SELECTOR_H
