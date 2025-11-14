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
#ifndef LOG_TAG
#define LOG_TAG "AudioPipeSelector"
#endif

#include "audio_pipe_selector.h"
#include "audio_stream_collector.h"
#include "audio_stream_info.h"
#include "audio_definition_adapter_info.h"
#include "audio_policy_utils.h"
#include <algorithm>
#include "audio_service_enum.h"
#include "audio_injector_policy.h"

namespace OHOS {
namespace AudioStandard {

static std::map<int, AudioPipeType> flagPipeTypeMap_ = {
    {AUDIO_OUTPUT_FLAG_NORMAL, PIPE_TYPE_NORMAL_OUT},
    {AUDIO_INPUT_FLAG_NORMAL, PIPE_TYPE_NORMAL_IN},
    {AUDIO_OUTPUT_FLAG_FAST, PIPE_TYPE_NORMAL_OUT},
    {AUDIO_INPUT_FLAG_FAST, PIPE_TYPE_NORMAL_IN},
    {AUDIO_OUTPUT_FLAG_LOWPOWER, PIPE_TYPE_OFFLOAD},
    {AUDIO_OUTPUT_FLAG_MULTICHANNEL, PIPE_TYPE_MULTICHANNEL},
    {AUDIO_OUTPUT_FLAG_DIRECT, PIPE_TYPE_DIRECT_OUT},
};

static bool IsRemoteOffloadNeedRecreate(std::shared_ptr<AudioPipeInfo> newPipe, std::shared_ptr<AudioPipeInfo> oldPipe)
{
    CHECK_AND_RETURN_RET(newPipe != nullptr && oldPipe != nullptr, false);
    CHECK_AND_RETURN_RET(newPipe->moduleInfo_.className == "remote_offload" &&
        oldPipe->moduleInfo_.className == "remote_offload", false);
    return (newPipe->moduleInfo_.format != oldPipe->moduleInfo_.format) ||
        (newPipe->moduleInfo_.rate != oldPipe->moduleInfo_.rate) ||
        (newPipe->moduleInfo_.channels != oldPipe->moduleInfo_.channels) ||
        (newPipe->moduleInfo_.bufferSize != oldPipe->moduleInfo_.bufferSize);
}

AudioPipeSelector::AudioPipeSelector() : configManager_(AudioPolicyConfigManager::GetInstance())
{
}

std::shared_ptr<AudioPipeSelector> AudioPipeSelector::GetPipeSelector()
{
    static std::shared_ptr<AudioPipeSelector> instance = std::make_shared<AudioPipeSelector>();
    return instance;
}

std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeSelector::FetchPipeAndExecute(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = AudioPipeManager::GetPipeManager()->GetPipeList();

    ScanPipeListForStreamDesc(pipeInfoList, streamDesc);
    AUDIO_INFO_LOG("Original Pipelist size: %{public}zu, stream routeFlag: 0x%{public}x to fetch",
        pipeInfoList.size(), streamDesc->routeFlag_);

    std::vector<std::shared_ptr<AudioPipeInfo>> selectedPipeInfoList {};
    for (auto &curPipeInfo : pipeInfoList) {
        if (curPipeInfo->pipeRole_ == static_cast<AudioPipeRole>(streamDesc->audioMode_)) {
            selectedPipeInfoList.push_back(curPipeInfo);
        }
    }

    // Generate pipeInfo by configuration for incoming stream
    streamDesc->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    configManager_.GetStreamPropInfo(streamDesc, streamPropInfo);
    UpdateDeviceStreamInfo(streamDesc, streamPropInfo);
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = streamPropInfo->pipeInfo_.lock();
    if (pipeInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Pipe info is null");
        return selectedPipeInfoList;
    }

    // Find whether any existing pipe matches
    bool findPipe = FindExistingPipe(selectedPipeInfoList, pipeInfoPtr, streamDesc, streamPropInfo);
    CHECK_AND_RETURN_RET(!findPipe, selectedPipeInfoList);

    // Need to open a new pipe for incoming stream
    AudioPipeInfo info = {};
    ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    info.pipeAction_ = PIPE_ACTION_NEW;
    selectedPipeInfoList.push_back(std::make_shared<AudioPipeInfo>(info));
    AUDIO_INFO_LOG("[PipeFetchInfo] use new Pipe %{public}s for stream %{public}u",
        info.ToString().c_str(), streamDesc->sessionId_);

    return selectedPipeInfoList;
}

void AudioPipeSelector::UpdateDeviceStreamInfo(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo)
{
    if (streamDesc->newDeviceDescs_.empty() || streamPropInfo == nullptr || streamDesc->newDeviceDescs_.front() ==
        nullptr) {
        AUDIO_WARNING_LOG("new device desc is empty!");
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> temp = streamDesc->newDeviceDescs_.front();
    DeviceStreamInfo streamInfo;
    streamInfo.format = streamPropInfo->format_;
    streamInfo.samplingRate = {static_cast<AudioSamplingRate>(streamPropInfo->sampleRate_)};
    streamInfo.SetChannels({streamPropInfo->channels_});
    temp->audioStreamInfo_ = {streamInfo};
    std::string info = streamInfo.Serialize();
    AUDIO_INFO_LOG("DeviceStreamInfo:%{public}s", info.c_str());
}

void AudioPipeSelector::ProcessRendererAndCapturerConcurrency(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    bool hasFastVoipCapturer = false;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = AudioPipeManager::GetPipeManager()->GetPipeList();
    for (auto &curPipeInfo : pipeInfoList) {
        CHECK_AND_CONTINUE(curPipeInfo->routeFlag_ == (AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST));
        hasFastVoipCapturer = true;
        break;
    }
    CHECK_AND_RETURN((streamDesc->routeFlag_ == AUDIO_OUTPUT_FLAG_FAST) && hasFastVoipCapturer);
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    AUDIO_INFO_LOG("Set %{public}u to normal flag", streamDesc->GetSessionId());
}

// get each streamDesc's final routeFlag after concurrency
void AudioPipeSelector::DecideFinalRouteFlag(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    CHECK_AND_RETURN_LOG(streamDescs.size() != 0, "streamDescs is empty!");
    SortStreamDescsByStartTime(streamDescs);
    streamDescs[0]->routeFlag_ = GetRouteFlagByStreamDesc(streamDescs[0]);
    // Do not need to move stream, because stream actions are all decided in DecidePipesAndStreamAction(),
    // not in ProcessConcurrency().
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsMoveToNormal;
    if (streamDescs.size() == 1) {
        // modemCommunication streamDescs stored in modemCommunicationIdMap_, need to do extra concurrencyProcess
        ProcessModemCommunicationConcurrency(streamDescs, streamsMoveToNormal);
        return;
    }

    for (size_t cmpStreamIdx = 1; cmpStreamIdx < streamDescs.size(); ++cmpStreamIdx) {
        streamDescs[cmpStreamIdx]->routeFlag_ = GetRouteFlagByStreamDesc(streamDescs[cmpStreamIdx]);
        // calculate concurrency in time order
        for (size_t curStreamDescIdx = 0; curStreamDescIdx < cmpStreamIdx; ++curStreamDescIdx) {
            ProcessConcurrency(streamDescs[curStreamDescIdx], streamDescs[cmpStreamIdx], streamsMoveToNormal);
            ProcessRendererAndCapturerConcurrency(streamDescs[cmpStreamIdx]);
        }
    }
    ProcessModemCommunicationConcurrency(streamDescs, streamsMoveToNormal);
}

// add streamDescs to prefer newPipe based on final routeFlag, create newPipe if needed
void AudioPipeSelector::ProcessNewPipeList(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    std::string adapterName{};
    for (auto &streamDesc : streamDescs) {
        CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is null");
        std::string streamDescAdapterName = "";
        std::vector<std::shared_ptr<AudioPipeInfo>>::iterator newPipeIter = newPipeInfoList.end();
        if (streamDesc->rendererTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
            streamDescAdapterName = AudioInjectorPolicy::GetInstance().GetAdapterName();
            newPipeIter = std::find_if(newPipeInfoList.begin(), newPipeInfoList.end(),
                [&](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
                    return newPipeInfo->adapterName_ == streamDescAdapterName;
                });
        } else {
            streamDescAdapterName = GetAdapterNameByStreamDesc(streamDesc);

            // When the paused offload/multichannel stream needs to recons, in pipeManager the stream has reconstucted,
            // but due to delay recons, the actual recons operation do not occur, the stream is still in old pipe,
            // which may cause two offload/multichannel streams running in concurrency.
            if (IsNeedTempMoveToNormal(streamDesc, streamDescToOldPipeInfo)) {
                AUDIO_INFO_LOG("[PipeFetchInfo] Temporarily move recons stream %{public}d to primary."
                    " routeFlag %{public}d", streamDesc->GetSessionId(), streamDesc->routeFlag_);
                streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
                streamDescAdapterName = "primary";
            }

            // find if curStream's prefer pipe has already exist
            newPipeIter = std::find_if(newPipeInfoList.begin(), newPipeInfoList.end(),
                [&](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
                    return newPipeInfo->routeFlag_ == streamDesc->routeFlag_ &&
                        newPipeInfo->adapterName_ == streamDescAdapterName;
                });
        }
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        configManager_.GetStreamPropInfo(streamDesc, streamPropInfo);
        if (newPipeIter != newPipeInfoList.end()) {
            MatchRemoteOffloadPipe(streamPropInfo, *newPipeIter, streamDesc);

            (*newPipeIter)->streamDescriptors_.push_back(streamDesc);
            (*newPipeIter)->streamDescMap_[streamDesc->sessionId_] = streamDesc;
            continue;
        }
        // if not find, need open
        HandlePipeNotExist(newPipeInfoList, streamDesc);
    }
}

// based on old--new pipeinfo to judge streamAction and pipeAction
void AudioPipeSelector::DecidePipesAndStreamAction(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo)
{
    // get each streamDesc in each newPipe to judge action
    for (auto &newPipeInfo : newPipeInfoList) {
        newPipeInfo->pipeAction_ = (newPipeInfo->pipeAction_ != PIPE_ACTION_NEW &&
            newPipeInfo->pipeAction_ != PIPE_ACTION_RELOAD) ? PIPE_ACTION_UPDATE : newPipeInfo->pipeAction_;
        AUDIO_INFO_LOG("[PipeFetchInfo] Name %{public}s, PipeAction: %{public}d",
            newPipeInfo->name_.c_str(), newPipeInfo->pipeAction_);

        for (auto &streamDesc : newPipeInfo->streamDescriptors_) {
            if (streamDescToOldPipeInfo.find(streamDesc->sessionId_) == streamDescToOldPipeInfo.end()) {
                AUDIO_WARNING_LOG("[PipeFetchInfo] cannot find %{public}d in OldPipeList!", streamDesc->sessionId_);
                continue;
            }
            streamDesc->SetAction(JudgeStreamAction(newPipeInfo, streamDescToOldPipeInfo[streamDesc->GetSessionId()]));
            streamDesc->SetOldRoute(streamDescToOldPipeInfo[streamDesc->GetSessionId()]->GetRoute());
            AUDIO_INFO_LOG("    |--[PipeFetchInfo] Id %{public}d, RouteFlag %{public}d --> %{public}d, "
                "sAction %{public}d", streamDesc->GetSessionId(),
                streamDescToOldPipeInfo[streamDesc->GetSessionId()]->GetRoute(),
                newPipeInfo->GetRoute(), streamDesc->GetAction());
        }
        if (newPipeInfo->streamDescriptors_.size() == 0) {
            AUDIO_INFO_LOG("    |--[PipeFetchInfo] Empty");
        }
    }
}

std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeSelector::FetchPipesAndExecute(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList{};
    if (streamDescs.size() == 0) {
        return oldPipeInfoList;
    }
    // get all existing pipes and select render/capture pipes
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = AudioPipeManager::GetPipeManager()->GetPipeList();
    for (auto &curPipeInfo : pipeInfoList) {
        if (curPipeInfo->pipeRole_ == static_cast<AudioPipeRole>(streamDescs[0]->audioMode_)) {
            oldPipeInfoList.push_back(curPipeInfo);
        }
    }

    // Record current pipe--stream info for later use (Judge stream action)
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToPipeInfo;
    for (auto &pipeInfo : oldPipeInfoList) {
        pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
        for (auto &streamDesc : pipeInfo->streamDescriptors_) {
            streamDescToPipeInfo[streamDesc->sessionId_] = pipeInfo;
        }
    }

    // deep copy to newPipeInfoList and clear all streams
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    for (auto &pipeInfo : oldPipeInfoList) {
        std::shared_ptr<AudioPipeInfo> temp = std::make_shared<AudioPipeInfo>(*pipeInfo);
        temp->streamDescriptors_.clear();
        temp->streamDescMap_.clear();
        newPipeInfoList.push_back(temp);
    }

    DecideFinalRouteFlag(streamDescs);
    ProcessNewPipeList(newPipeInfoList, streamDescToPipeInfo, streamDescs);
    DecidePipesAndStreamAction(newPipeInfoList, streamDescToPipeInfo);

    // check is pipe update
    for (auto &pipeInfo : oldPipeInfoList) {
        if (pipeInfo->streamDescriptors_.size() == 0) {
            pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
        }
    }
    return newPipeInfoList;
}

void AudioPipeSelector::HandlePipeNotExist(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    AudioPipeInfo pipeInfo = {};
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    configManager_.GetStreamPropInfo(streamDesc, streamPropInfo);
    ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, pipeInfo);
    pipeInfo.pipeAction_ = PIPE_ACTION_NEW;
    std::shared_ptr<AudioPipeInfo> tempPipeInfo = std::make_shared<AudioPipeInfo>(pipeInfo);
    newPipeInfoList.push_back(tempPipeInfo);
    AUDIO_INFO_LOG("[PipeFetchInfo] use new Pipe %{public}s for stream %{public}u with action %{public}d, "
        "routeFlag %{public}d", tempPipeInfo->ToString().c_str(), streamDesc->sessionId_, streamDesc->streamAction_,
        streamDesc->routeFlag_);
}

void AudioPipeSelector::ScanPipeListForStreamDesc(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList,
    std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    streamDesc->routeFlag_ = GetRouteFlagByStreamDesc(streamDesc);
    AUDIO_INFO_LOG("Route flag: %{public}u", streamDesc->routeFlag_);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsMoveToNormal;
    for (auto &pipeInfo : pipeInfoList) {
        bool isUpdate = false;
        for (auto &streamDescInPipe : pipeInfo->streamDescriptors_) {
            isUpdate = ProcessConcurrency(streamDescInPipe, streamDesc, streamsMoveToNormal);
            AUDIO_INFO_LOG("isUpdate: %{public}d, action: %{public}d", isUpdate, streamDescInPipe->streamAction_);
        }
        if (isUpdate && pipeInfo->GetAction() != PIPE_ACTION_NEW) {
            pipeInfo->SetAction(PIPE_ACTION_UPDATE);
        }
    }
    // modemCommunication streamDescs stored in modemCommunicationIdMap_, need to do extra concurrencyProcess
    std::vector<std::shared_ptr<AudioStreamDescriptor>> tempStreamDescs{streamDesc};
    ProcessModemCommunicationConcurrency(tempStreamDescs, streamsMoveToNormal);

    // Move concede existing streams to its corresponding normal pipe
    MoveStreamsToNormalPipes(streamsMoveToNormal, pipeInfoList);

    AUDIO_INFO_LOG("Route flag after concurrency: %{public}u", streamDesc->routeFlag_);
}

AudioPipeType AudioPipeSelector::GetPipeType(uint32_t flag, AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        if (flag & AUDIO_OUTPUT_FLAG_FAST) {
            if (flag & AUDIO_OUTPUT_FLAG_VOIP) {
                return PIPE_TYPE_CALL_OUT;
            } else {
                return PIPE_TYPE_LOWLATENCY_OUT;
            }
        } else if (flag & AUDIO_OUTPUT_FLAG_DIRECT) {
            if (flag & AUDIO_OUTPUT_FLAG_VOIP) {
                return PIPE_TYPE_CALL_OUT;
            } else {
                return PIPE_TYPE_DIRECT_OUT;
            }
        } else if (flag & AUDIO_OUTPUT_FLAG_MULTICHANNEL) {
            return PIPE_TYPE_MULTICHANNEL;
        } else if (flag & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) {
            return PIPE_TYPE_OFFLOAD;
        } else if (flag & AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION) {
            return PIPE_TYPE_CALL_OUT;
        } else {
            return PIPE_TYPE_NORMAL_OUT;
        }
    } else {
        if (flag & AUDIO_INPUT_FLAG_FAST) {
            if (flag & AUDIO_INPUT_FLAG_VOIP) {
                return PIPE_TYPE_CALL_IN;
            } else {
                return PIPE_TYPE_LOWLATENCY_IN;
            }
        } else if (flag & AUDIO_INPUT_FLAG_AI) {
            return PIPE_TYPE_NORMAL_IN_AI;
        } else {
            return PIPE_TYPE_NORMAL_IN;
        }
    }
}

void AudioPipeSelector::CheckAndHandleIncomingConcurrency(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream)
{
    // Normal, fast or voip-fast can not run concurrently, both stream need to be conceded
    if (incomingStream->IsRecording() && existingStream->IsRecording()) {
        AUDIO_INFO_LOG("capture in: %{public}u  old: %{public}u",
            incomingStream->sessionId_, existingStream->sessionId_);
        incomingStream->ResetToNormalRoute(false);
    }
}

bool AudioPipeSelector::IsSameAdapter(std::shared_ptr<AudioStreamDescriptor> streamDescA,
    std::shared_ptr<AudioStreamDescriptor> streamDescB)
{
    CHECK_AND_RETURN_RET(streamDescA != nullptr && streamDescB != nullptr && streamDescA->newDeviceDescs_.size() != 0 &&
        streamDescB->newDeviceDescs_.size() != 0, true);
    bool hasRemote = false;
    for (auto deviceDescA : streamDescA->newDeviceDescs_) {
        CHECK_AND_CONTINUE(deviceDescA != nullptr);
        AudioPipeType pipeTypeA = GetPipeType(streamDescA->routeFlag_, streamDescA->audioMode_);
        bool isRemoteA = deviceDescA->networkId_ != LOCAL_NETWORK_ID;
        hasRemote = isRemoteA ? true : hasRemote;
        std::string portNameA = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceDescA->deviceType_, pipeTypeA);

        for (auto deviceDescB : streamDescB->newDeviceDescs_) {
            CHECK_AND_CONTINUE(deviceDescB != nullptr);
            AudioPipeType pipeTypeB = GetPipeType(streamDescB->routeFlag_, streamDescB->audioMode_);
            bool isRemoteB = deviceDescB->networkId_ != LOCAL_NETWORK_ID;
            hasRemote = isRemoteB ? true : hasRemote;
            std::string portNameB = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceDescB->deviceType_,
                pipeTypeB);
            CHECK_AND_RETURN_RET(!(isRemoteA == isRemoteB && portNameA == portNameB), true);
        }
    }
    CHECK_AND_RETURN_RET(hasRemote, true);
    AUDIO_INFO_LOG("diff adapter, not need concurrency");
    return false;
}

void AudioPipeSelector::UpdateProcessConcurrency(AudioPipeType existingPipe, AudioPipeType commingPipe,
                                                 ConcurrencyAction &action)
{
    /* becasue call in indicate voip and cell, so can't modify xml */
    CHECK_AND_RETURN(IsInjectEnable() && action != PLAY_BOTH);
    if (existingPipe == PIPE_TYPE_CALL_IN && commingPipe == PIPE_TYPE_CALL_IN) {
        action = PLAY_BOTH;
    }
}

bool AudioPipeSelector::ProcessConcurrency(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove)
{
    AudioPipeType existingPipe = GetPipeType(existingStream->routeFlag_, existingStream->audioMode_);
    AudioPipeType commingPipe = GetPipeType(incomingStream->routeFlag_, incomingStream->audioMode_);
    ConcurrencyAction action = AudioStreamCollector::GetAudioStreamCollector().GetConcurrencyAction(
        existingPipe, commingPipe);
    action = IsSameAdapter(existingStream, incomingStream) ? action : PLAY_BOTH;
    // No running offload can not concede incoming special pipe
    if (action == CONCEDE_INCOMING && existingStream->IsNoRunningOffload()) {
        action = CONCEDE_EXISTING;
    }
    JUDGE_AND_INFO_LOG(action != PLAY_BOTH, "Action: %{public}u "
        "existingStream id: %{public}u, routeFlag: %{public}u; "
        "incomingStream id: %{public}u, routeFlag: %{public}u",
        action,
        existingStream->GetSessionId(), existingStream->GetRoute(),
        incomingStream->GetSessionId(), incomingStream->GetRoute());

    /* temporary handle */
    UpdateProcessConcurrency(existingPipe, commingPipe, action);

    bool isUpdate = false;
    switch (action) {
        case PLAY_BOTH:
            break;
        case CONCEDE_INCOMING:
            incomingStream->ResetToNormalRoute(false);
            SetOriginalFlagForcedNormalIfNeed(incomingStream);
            break;
        case CONCEDE_EXISTING:
            // If action is concede existing, maybe also need to concede incoming
            CheckAndHandleIncomingConcurrency(existingStream, incomingStream);
            isUpdate = true;
            if (existingStream->IsUseMoveToConcedeType()) {
                existingStream->SetAction(AUDIO_STREAM_ACTION_MOVE);
                // Do not move stream here, because it is still in for-each loop
                streamsToMove.push_back(existingStream);
            } else {
                existingStream->SetAction(AUDIO_STREAM_ACTION_RECREATE);
            }
            // Set stream route flag to normal here so it will not affect later streams in loop
            existingStream->ResetToNormalRoute(true);
            SetOriginalFlagForcedNormalIfNeed(existingStream);
            break;
        default:
            break;
    }
    return isUpdate;
}

uint32_t AudioPipeSelector::GetRouteFlagByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    uint32_t flag = AUDIO_FLAG_NONE;
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, flag, "streamDesc is nullptr");
    flag = configManager_.GetRouteFlag(streamDesc);
    return flag;
}

std::string AudioPipeSelector::GetAdapterNameByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    std::string name = "";
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, name, "streamDesc is nullptr");
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    configManager_.GetStreamPropInfo(streamDesc, streamPropInfo);
    CHECK_AND_RETURN_RET_LOG(streamPropInfo != nullptr, "", "StreamProp Info is null");

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = streamPropInfo->pipeInfo_.lock();
    if (pipeInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Adapter info is null");
        return "";
    }

    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = pipeInfoPtr->adapterInfo_.lock();
    if (adapterInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Pipe info is null");
        return "";
    }
    name = adapterInfoPtr->adapterName;
    return name;
}

static void FillSpecialPipeInfo(AudioPipeInfo &info, std::shared_ptr<AdapterPipeInfo> pipeInfoPtr,
    std::shared_ptr<AudioStreamDescriptor> streamDesc, std::shared_ptr<PipeStreamPropInfo> streamPropInfo)
{
    if (pipeInfoPtr->name_ == "multichannel_output") {
        info.moduleInfo_.className = "multichannel";
        info.moduleInfo_.fileName = "mch_dump_file";
        info.moduleInfo_.fixedLatency = "1"; // for fix max request
        AUDIO_INFO_LOG("Buffer size: %{public}s channels: %{public}s channelLayout:%{public}s",
            info.moduleInfo_.bufferSize.c_str(), info.moduleInfo_.channels.c_str(),
            info.moduleInfo_.channelLayout.c_str());
    } else if (pipeInfoPtr->name_ == "offload_output") {
        info.moduleInfo_.className = "offload";
        info.moduleInfo_.offloadEnable = "1";
        info.moduleInfo_.fixedLatency = "1";
        info.moduleInfo_.fileName = "offload_dump_file";
    } else if (pipeInfoPtr->name_ == "dp_multichannel_output") {
        info.moduleInfo_.className = "dp_multichannel";
        info.moduleInfo_.fileName = "mch_dump_file";
        info.moduleInfo_.fixedLatency = "1";
        info.moduleInfo_.bufferSize = std::to_string(streamPropInfo->bufferSize_);
    } else if (pipeInfoPtr->name_ == "offload_distributed_output") {
        info.moduleInfo_.className = "remote_offload";
        info.moduleInfo_.offloadEnable = "1";
        info.moduleInfo_.fixedLatency = "1";
        info.moduleInfo_.fileName = "remote_offload_dump_file";
        info.moduleInfo_.name =
            AudioPolicyUtils::GetInstance().GetRemoteModuleName(streamDesc->newDeviceDescs_[0]->networkId_,
            AudioPolicyUtils::GetInstance().GetDeviceRole(streamDesc->newDeviceDescs_[0]->deviceType_)) + "_offload";
    }
}

void AudioPipeSelector::ConvertStreamDescToPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info)
{
    CHECK_AND_RETURN_LOG(streamPropInfo != nullptr, "streamPropInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = streamPropInfo->pipeInfo_.lock();
    if (pipeInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Adapter info is null");
        return ;
    }

    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = pipeInfoPtr->adapterInfo_.lock();
    if (adapterInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Pipe info is null");
        return ;
    }

    info.moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[streamPropInfo->format_];
    info.moduleInfo_.rate = std::to_string(streamPropInfo->sampleRate_);
    info.moduleInfo_.channels = std::to_string(AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(
        streamPropInfo->channelLayout_));
    info.moduleInfo_.bufferSize = std::to_string(streamPropInfo->bufferSize_);

    info.moduleInfo_.lib = pipeInfoPtr->paProp_.lib_;
    info.moduleInfo_.role = pipeInfoPtr->paProp_.role_;
    info.moduleInfo_.name = pipeInfoPtr->paProp_.moduleName_;
    info.moduleInfo_.adapterName = adapterInfoPtr->adapterName;
    info.moduleInfo_.className = adapterInfoPtr->adapterName;
    info.moduleInfo_.OpenMicSpeaker = configManager_.GetUpdateRouteSupport() ? "1" : "0";

    AUDIO_INFO_LOG("Pipe name: %{public}s", pipeInfoPtr->name_.c_str());
    FillSpecialPipeInfo(info, pipeInfoPtr, streamDesc, streamPropInfo);

    info.moduleInfo_.deviceType = std::to_string(streamDesc->newDeviceDescs_[0]->deviceType_);
    info.moduleInfo_.networkId = streamDesc->newDeviceDescs_[0]->networkId_;
    info.moduleInfo_.macAddress = streamDesc->newDeviceDescs_[0]->macAddress_;
    info.moduleInfo_.sourceType = std::to_string(streamDesc->capturerInfo_.sourceType);

    info.streamDescriptors_.push_back(streamDesc);
    info.streamDescMap_[streamDesc->sessionId_] = streamDesc;
    info.routeFlag_ = streamDesc->routeFlag_;
    info.adapterName_ = adapterInfoPtr->adapterName;
    info.pipeRole_ = pipeInfoPtr->role_;
    info.name_ = pipeInfoPtr->name_;
    info.InitAudioStreamInfo();
}

AudioStreamAction AudioPipeSelector::JudgeStreamAction(
    std::shared_ptr<AudioPipeInfo> newPipe, std::shared_ptr<AudioPipeInfo> oldPipe)
{
    CHECK_AND_RETURN_RET(!IsRemoteOffloadNeedRecreate(newPipe, oldPipe), AUDIO_STREAM_ACTION_RECREATE);
    if (newPipe->adapterName_ == oldPipe->adapterName_ && newPipe->routeFlag_ == oldPipe->routeFlag_) {
        return AUDIO_STREAM_ACTION_DEFAULT;
    }
    if ((oldPipe->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) || (newPipe->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) ||
        (oldPipe->routeFlag_ & AUDIO_OUTPUT_FLAG_DIRECT) || (newPipe->routeFlag_ & AUDIO_OUTPUT_FLAG_DIRECT)) {
        return AUDIO_STREAM_ACTION_RECREATE;
    } else {
        return AUDIO_STREAM_ACTION_MOVE;
    }
}

void AudioPipeSelector::SortStreamDescsByStartTime(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    sort(streamDescs.begin(), streamDescs.end(), [](const std::shared_ptr<AudioStreamDescriptor> &streamDesc1,
        const std::shared_ptr<AudioStreamDescriptor> &streamDesc2) {
            return streamDesc1->createTimeStamp_ < streamDesc2->createTimeStamp_;
        });
}

void AudioPipeSelector::MoveStreamsToNormalPipes(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove,
    std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList)
{
    std::map<std::shared_ptr<AudioStreamDescriptor>, std::string> streamToAdapter;
    RemoveTargetStreams(streamsToMove, pipeInfoList, streamToAdapter);

    // Put each stream to its according normal pipe
    for (auto &stream : streamsToMove) {
        for (auto &pipe : pipeInfoList) {
            if (pipe->IsSameRole(stream) && pipe->IsRouteNormal() && pipe->IsSameAdapter(streamToAdapter[stream])) {
                AddStreamToPipeAndUpdateAction(stream, pipe);
                break;
            }
        }
    }
}

void AudioPipeSelector::AddStreamToPipeAndUpdateAction(
    std::shared_ptr<AudioStreamDescriptor> &streamToAdd, std::shared_ptr<AudioPipeInfo> &pipe)
{
    AUDIO_INFO_LOG("Put stream %{public}u to pipe %{public}s",
        streamToAdd->GetSessionId(), pipe->GetName().c_str());
    pipe->AddStream(streamToAdd);
    // When fetching, pipe action may already be PIPE_ACTION_NEW before,
    // do not change it to PIPE_ACTION_UPDATE.
    if (pipe->GetAction() != PIPE_ACTION_NEW) {
        pipe->SetAction(PIPE_ACTION_UPDATE);
    }
}

void AudioPipeSelector::RemoveTargetStreams(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove,
    std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList,
    std::map<std::shared_ptr<AudioStreamDescriptor>, std::string> &streamToAdapter)
{
    // Remove streams from old pipes and record old pipe adapter which is used to find
    // normal pipe in the same adapter.
    for (auto &stream : streamsToMove) {
        for (auto &pipe : pipeInfoList) {
            if (pipe->ContainStream(stream->GetSessionId())) {
                streamToAdapter[stream] = pipe->GetAdapterName();
                pipe->RemoveStream(stream->GetSessionId());
                // Should be only one matching pipe
                break;
            }
        }
    }
}

void AudioPipeSelector::ProcessModemCommunicationConcurrency(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsMoveToNormal)
{
    CHECK_AND_RETURN(AudioPipeManager::GetPipeManager()->IsModemCommunicationIdExist());
    AUDIO_INFO_LOG("ModemCommunication exists, need process concurrency");
    std::shared_ptr<AudioStreamDescriptor> modemCommStream =
        AudioPipeManager::GetPipeManager()->GetModemCommunicationStreamDesc();
    for (auto &streamDesc : streamDescs) {
        ProcessConcurrency(modemCommStream, streamDesc, streamsMoveToNormal);
    }
}

// Once a stream is conceded from offload/direct to normal, it cannot be restored to offload/direct
void AudioPipeSelector::SetOriginalFlagForcedNormalIfNeed(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    if ((streamDesc->IsSelectFlagOffload() || streamDesc->IsSelectFlagHd()) && streamDesc->IsRouteNormal()) {
        AUDIO_INFO_LOG("Session %{public}u has been conceded to FORCED_NORMAL", streamDesc->sessionId_);
        streamDesc->SetOriginalFlagForcedNormal();
    }
}

bool AudioPipeSelector::IsNeedTempMoveToNormal(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(streamDescToOldPipeInfo.size() != 0, false, "streamDescToOldPipeInfo is empty!");
    return (streamDescToOldPipeInfo[streamDesc->GetSessionId()]->IsRenderPipeNeedMoveToNormal() &&
        streamDesc->IsRenderStreamNeedRecreate());
}

bool AudioPipeSelector::FindExistingPipe(std::vector<std::shared_ptr<AudioPipeInfo>> &selectedPipeInfoList,
    const std::shared_ptr<AdapterPipeInfo> &pipeInfoPtr, const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo)
{
    for (auto &pipeInfo : selectedPipeInfoList) {
        std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = pipeInfoPtr->adapterInfo_.lock();
        CHECK_AND_CONTINUE_LOG(adapterInfoPtr != nullptr, "Adapter info is null");

        AUDIO_INFO_LOG("action %{public}d adapter[%{public}s] pipeRoute[0x%{public}x] streamRoute[0x%{public}x]",
            pipeInfo->GetAction(), pipeInfo->GetAdapterName().c_str(), pipeInfo->GetRoute(), streamDesc->GetRoute());

        CHECK_AND_CONTINUE(pipeInfo->adapterName_ == adapterInfoPtr->adapterName &&
            pipeInfo->routeFlag_ == streamDesc->routeFlag_);

        MatchRemoteOffloadPipe(streamPropInfo, pipeInfo, streamDesc);

        pipeInfo->streamDescriptors_.push_back(streamDesc);
        pipeInfo->streamDescMap_[streamDesc->sessionId_] = streamDesc;
        pipeInfo->pipeAction_ = pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD ? PIPE_ACTION_RELOAD : PIPE_ACTION_UPDATE;
        AUDIO_INFO_LOG("[PipeFetchInfo] use existing Pipe %{public}s for stream %{public}u, pipeAction: %{public}d",
            pipeInfo->ToString().c_str(), streamDesc->sessionId_, pipeInfo->pipeAction_);
        return true;
    }
    return false;
}

void AudioPipeSelector::MatchRemoteOffloadPipe(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
    std::shared_ptr<AudioPipeInfo> pipeInfo, const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    bool matchState = IsPipeFormatMatch(streamPropInfo, pipeInfo);
    CHECK_AND_RETURN(!matchState && (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER) &&
        pipeInfo->adapterName_ == "remote");

    AUDIO_INFO_LOG("existing mismatching remote offload pipe need to recreate to match music format");
    UpdatePipeInfoFromStreamProp(streamDesc, streamPropInfo, *pipeInfo);
    pipeInfo->pipeAction_ = PIPE_ACTION_RELOAD;
}

bool AudioPipeSelector::IsPipeFormatMatch(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
    std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    std::string channels = std::to_string(AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(
        streamPropInfo->channelLayout_));
    std::string channelLayout = std::to_string(streamPropInfo->channelLayout_);
    auto format = AudioDefinitionPolicyUtils::enumToFormatStr[streamPropInfo->format_];
    std::string sampleRate = std::to_string(streamPropInfo->sampleRate_);

    return channels == pipeInfo->moduleInfo_.channels && channelLayout == pipeInfo->moduleInfo_.channelLayout &&
        format == pipeInfo->moduleInfo_.format && sampleRate == pipeInfo->moduleInfo_.rate;
}

void AudioPipeSelector::UpdatePipeInfoFromStreamProp(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info)
{
    CHECK_AND_RETURN_LOG(streamPropInfo != nullptr, "streamPropInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = streamPropInfo->pipeInfo_.lock();
    CHECK_AND_RETURN_LOG(pipeInfoPtr != nullptr, "Adapter info is null");

    info.moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[streamPropInfo->format_];
    info.moduleInfo_.rate = std::to_string(streamPropInfo->sampleRate_);
    info.moduleInfo_.channels = std::to_string(AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(
        streamPropInfo->channelLayout_));
    info.moduleInfo_.channelLayout = std::to_string(streamPropInfo->channelLayout_);
    info.moduleInfo_.bufferSize = std::to_string(streamPropInfo->bufferSize_);

    AUDIO_INFO_LOG("Pipe name: %{public}s, channels: %{public}s, channelLayout: %{public}s",
        pipeInfoPtr->name_.c_str(), info.moduleInfo_.channels.c_str(), info.moduleInfo_.channelLayout.c_str());

    info.InitAudioStreamInfo();
}

void AudioPipeSelector::UpdateRendererPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is null");
 
    AudioPipeType type = GetPipeType(streamDesc->routeFlag_, streamDesc->audioMode_);
    AudioStreamCollector::GetAudioStreamCollector().UpdateRendererPipeInfo(streamDesc->sessionId_, type);
}
} // namespace AudioStandard
} // namespace OHOS