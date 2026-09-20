/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <functional>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "audio_pipe_manager.h"
#include "audio_stream_info.h"
#include "audio_concurrency_parser.h"
#include "audio_zone_info.h"

namespace OHOS {
namespace AudioStandard {
enum class CapturerPipeExecOrder : int32_t {
    CLOSE_BEFORE_OPEN = 0,
    NORMAL = 1,
    OPEN_AFTER_CLOSE = 2,
};
struct AudioRouteSelectInfo;

class AudioPipeSelector {
public:
    using AudioRouteSelectorCallback = std::function<int32_t(
        const std::shared_ptr<AudioRouteSelectInfo> &, int32_t &)>;

    AudioPipeSelector();
    ~AudioPipeSelector() = default;

    static std::shared_ptr<AudioPipeSelector> GetPipeSelector();

    std::vector<std::shared_ptr<AudioPipeInfo>> FetchPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    std::vector<std::shared_ptr<AudioPipeInfo>> FetchPipesAndExecute(
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void UpdateRendererPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    int32_t SetCustomAudioMix(const std::string &zoneName, const std::vector<AudioZoneMix> &audioMixes);
    int32_t SetAudioRouteSelectorCallback(const AudioRouteSelectorCallback &callback);
    int32_t UnsetAudioRouteSelectorCallback();

private:
    using InputPipeStreamPair = std::pair<std::shared_ptr<AudioPipeInfo>, std::shared_ptr<AudioStreamDescriptor>>;
    using InputPipeStreamPairList = std::vector<InputPipeStreamPair>;

    std::shared_ptr<AudioRouteSelectInfo> BuildRouteSelectInfo(
        const std::shared_ptr<AudioStreamDescriptor> &streamDesc) const;
    void UpdateDeviceStreamInfo(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo);
    bool ScanPipeListForStreamDesc(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList,
        std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool ApplyRouteSelectDecision(std::shared_ptr<AudioStreamDescriptor> streamDesc, bool allowRejectCreate);
    int32_t NotifyAudioRouteSelect(std::shared_ptr<AudioStreamDescriptor> streamDesc, int32_t &selectResult);
    bool ProcessConcurrency(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove);
    void HandleConcedeIncoming(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream);
    bool HandleConcedeExisting(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove);
    bool HandleConcedeBoth(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove);
    uint32_t GetRouteFlagByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void GetAdapterNameByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::string &adapterName, std::string &pipeName);
    void ConvertStreamDescToPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info);
    AudioStreamAction JudgeStreamAction(std::shared_ptr<AudioPipeInfo> newPipe, std::shared_ptr<AudioPipeInfo> oldPipe);
    void SortStreamDescsByStartTime(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    AudioPipeType GetInputNormalPipeType(uint32_t flag);
    AudioPipeType GetPipeType(uint32_t flag, AudioMode audioMode);
    void HandlePipeNotExist(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsSameAdapter(std::shared_ptr<AudioStreamDescriptor> streamDescA,
        std::shared_ptr<AudioStreamDescriptor> streamDescB);
    void DecideFinalRouteFlag(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void ProcessNewPipeList(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
#ifdef MULTI_BUS_ENABLE
    bool IsRecordingPipeMatched(const std::vector<std::string> &busAddresses,
        const std::shared_ptr<AudioStreamDescriptor> &streamDesc, const std::string &adapterName,
        const std::shared_ptr<AudioPipeInfo> &newPipeInfo);
    void HandleFindBusPipe(const std::vector<std::string> &busAddresses,
                           std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
                           const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
                           std::vector<std::shared_ptr<AudioPipeInfo>>::iterator &busPipeIter);
#endif
    void UpdatePipeAndStreamAction(std::shared_ptr<AudioPipeInfo> &newPipeInfo,
        const std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> &streamDescToOldPipeInfo);
    void LogStreamsInfo(const std::vector<std::pair<uint32_t, uint32_t>> &sessionList);
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> BuildOldPipeMap(
        const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList) const;
    void UpdateInputPipeReloadAction(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
        const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList);
    void HandleFindMatchPipe(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
                             const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
                             const std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> &streamDescToOldPipeInfo,
                             std::vector<std::shared_ptr<AudioPipeInfo>>::iterator &matchPipeIter);
    void DecidePipesAndStreamAction(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo,
        const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList);
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
    void ProcessRendererAndCapturerConcurrency(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        ConcurrencyAction action);
    bool IsNeedTempMoveToNormal(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo);
    bool FindExistingPipe(std::vector<std::shared_ptr<AudioPipeInfo>> &selectedPipeInfoList,
        const std::shared_ptr<AdapterPipeInfo> &pipeInfoPtr, std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo);
    bool IsSpecialRemotePipe(std::shared_ptr<AudioPipeInfo> pipeInfo);
    void MatchSpecialRemotePipe(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
        std::shared_ptr<AudioPipeInfo> pipeInfo, const std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void MatchUltraFastPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void UpdatePipeInfoFromStreamProp(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info);
    bool IsPipeFormatMatch(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
        std::shared_ptr<AudioPipeInfo> pipeInfo);
    void HandleFastStreamLimitOnCreate(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        const std::shared_ptr<AudioStreamDescriptor> &newStreamDesc);
    void HandleFastStreamLimitAndNearLinkState(
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void SetPipeTypeByStreamType(AudioPipeType &nowPipeType, std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsBothFastArmUsbNeedRecreate(std::shared_ptr<AudioPipeInfo> newPipe, std::shared_ptr<AudioPipeInfo> oldPipe);
    void UpdateMouleInfoWitchDevice(const std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
        AudioModuleInfo &moduleInfo);
    bool IsPipeMatch(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const std::shared_ptr<AudioPipeInfo> &pipeInfo, const std::string &adapterName, const std::string &pipeName);
    ConcurrencyAction DetermineConcurrencyAction(std::shared_ptr<AudioStreamDescriptor> &existingStream,
        std::shared_ptr<AudioStreamDescriptor> &incomingStream);
    void GetStreamPropInfoWithBusSelector(std::shared_ptr<AudioStreamDescriptor> &desc,
                                          std::shared_ptr<PipeStreamPropInfo> &info);
    void SortStreamByStatus(std::vector<std::pair<std::shared_ptr<AudioStreamDescriptor>,
        std::shared_ptr<AudioPipeInfo>>> &streamList);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> UpdateStreamDescs(
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        const std::unordered_set<uint32_t> &originalStreamSessionIds);
    void AdjustConcedeActionByStatus(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream, ConcurrencyAction &action);
    bool GetIncomingStreamPriority(const std::shared_ptr<AudioStreamDescriptor> &existingStream,
        const std::shared_ptr<AudioStreamDescriptor> &incomingStream);
    void AdjustFastRouteForUltraFastIfNeeded(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void AdjustFastRouteForUltraFastIfNeeded(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    bool IsMultiStreamPrimary(const std::string &pipeName);
    void UpdateActionForHdPlay(std::shared_ptr<AudioStreamDescriptor> existingStream,
        std::shared_ptr<AudioStreamDescriptor> incomingStream, ConcurrencyAction &action);
    InputPipeStreamPairList DetermineInputPipeBaseConfig(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList);
    void DetermineInputPipeEcAndMicRef(const InputPipeStreamPairList &pipePairs);
    int32_t GetPipePriority(const std::shared_ptr<AudioPipeInfo> &pipeInfo) const;
    std::shared_ptr<AudioStreamDescriptor> GetHighestPriorityStream(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams) const;
    std::string DetermineTargetEcType(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const std::string &currentEcType) const;
    std::vector<std::shared_ptr<AudioPipeInfo>> GetSortedInputPipes(
        std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList) const;
    std::shared_ptr<AudioPipeInfo> FindOldInputPipe(const std::shared_ptr<AudioPipeInfo> &newPipe,
        const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
        const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList) const;
    void UpdateInputPipeParamByHighestStream(const std::shared_ptr<AudioStreamDescriptor> &highestStream,
        std::shared_ptr<AudioPipeInfo> &newPipe);
    void UpdateInputPipeEcAndMicRefByHighestStream(const std::shared_ptr<AudioStreamDescriptor> &highestStream,
        std::shared_ptr<AudioPipeInfo> &newPipe, std::unordered_set<std::string> &ecOccupiedAdapters,
        std::unordered_set<std::string> &micRefOccupiedAdapters);
    bool IsNeedReloadForInputPipe(const std::shared_ptr<AudioPipeInfo> &newPipe,
        const std::shared_ptr<AudioPipeInfo> &oldPipe, const std::shared_ptr<AudioStreamDescriptor> &highestStream,
        const std::shared_ptr<AudioStreamDescriptor> &oldHighestStream) const;
    CapturerPipeExecOrder GetCapturerPipeExecOrder(const std::shared_ptr<AudioPipeInfo> &newPipe,
        const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
        const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList) const;
    void SortCapturerPipesByExecOrder(const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList,
        std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList) const;
    void HandleEmptyInputPipeReload(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
        const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
        const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList);
    bool IsUpdateActionForDeviceLevelHdPlay(const std::shared_ptr<AudioStreamDescriptor> &existingStream,
        const std::shared_ptr<AudioStreamDescriptor> &incomingStream);
    bool IsPipeNotSupportReload(const std::shared_ptr<AudioPipeInfo> &pipe) const;
    void MatchCollaborativePipe(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
        std::shared_ptr<AudioPipeInfo> pipeInfo, const std::shared_ptr<AudioStreamDescriptor> &streamDesc);

    std::shared_ptr<AudioPipeManager> pipeManager_;
    std::mutex callbackMutex_;
    AudioRouteSelectorCallback audioRouteSelectorCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PIPE_SELECTOR_H
