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
#ifndef LOG_TAG
#define LOG_TAG "AudioPipeSelector"
#endif

#include "audio_pipe_selector.h"
#include "audio_stream_collector.h"
#include "audio_stream_info.h"
#include "audio_definition_adapter_info.h"
#include "audio_policy_utils.h"
#include <algorithm>
#include "audio_errors.h"
#include "audio_policy_interface.h"
#include "audio_service_enum.h"
#include "audio_injector_policy.h"
#include "audio_bus_selector.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B84

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t MAX_FAST_STREAM_COUNT = 6;
static std::map<int, AudioPipeType> flagPipeTypeMap_ = {
    {AUDIO_OUTPUT_FLAG_NORMAL, PIPE_TYPE_OUT_NORMAL},
    {AUDIO_INPUT_FLAG_NORMAL, PIPE_TYPE_IN_NORMAL},
    {AUDIO_OUTPUT_FLAG_FAST, PIPE_TYPE_OUT_NORMAL},
    {AUDIO_INPUT_FLAG_FAST, PIPE_TYPE_IN_NORMAL},
    {AUDIO_OUTPUT_FLAG_LOWPOWER, PIPE_TYPE_OUT_OFFLOAD},
    {AUDIO_OUTPUT_FLAG_MULTICHANNEL, PIPE_TYPE_OUT_MULTICHANNEL},
    {AUDIO_OUTPUT_FLAG_DIRECT, PIPE_TYPE_OUT_DIRECT_NORMAL},
};

static std::map<AudioSampleFormat, std::string> formatStrToHpae = {
    {SAMPLE_U8, "s8"},
    {SAMPLE_S16LE, "s16"},
    {SAMPLE_S24LE, "s24"},
    {SAMPLE_S32LE, "s32"},
    {SAMPLE_F32LE, "f32"},
};

static bool IsVoiceRecognitionMicInEcSupportedInputDeviceType(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_MIC:
        case DEVICE_TYPE_WAKEUP:
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return true;
        default:
            return false;
    }
}

static bool IsVoiceRecognitionMicInEcSupportedInputDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return deviceDesc != nullptr && deviceDesc->networkId_ == LOCAL_NETWORK_ID &&
        IsVoiceRecognitionMicInEcSupportedInputDeviceType(deviceDesc->deviceType_);
}

static bool HasDegradeInputDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    return std::any_of(deviceDescs.begin(), deviceDescs.end(), [](const auto &deviceDesc) {
        return deviceDesc != nullptr && !IsVoiceRecognitionMicInEcSupportedInputDevice(deviceDesc);
    });
}

static bool IsDegradeInputStream(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return HasDegradeInputDevice(streamDesc->newDeviceDescs_) || HasDegradeInputDevice(streamDesc->oldDeviceDescs_);
}

static bool IsVoiceRecognitionMicInEcRequested(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION &&
        !IsDegradeInputStream(streamDesc) &&
        (streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW || streamDesc->ecStreamInfo_.channels > CHANNEL_UNKNOW);
}

static bool IsCamcorderMicInRequested(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_CAMCORDER &&
        streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW;
}

static void UpdateMicInEcModuleInfo(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");

    if (streamDesc->ecStreamInfo_.channels > CHANNEL_UNKNOW) {
        moduleInfo.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
        moduleInfo.ecSamplingRate = std::to_string(streamDesc->ecStreamInfo_.samplingRate);
        moduleInfo.ecChannels = std::to_string(streamDesc->ecStreamInfo_.channels);
        if (formatStrToHpae.count(streamDesc->ecStreamInfo_.format) > 0) {
            moduleInfo.ecFormat = formatStrToHpae[streamDesc->ecStreamInfo_.format];
        }
    }

    if (streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW) {
        moduleInfo.micInRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micInChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatStrToHpae.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micInFormat = formatStrToHpae[streamDesc->micInStreamInfo_.format];
        }
        moduleInfo.micRefRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micRefChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatStrToHpae.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micRefFormat = formatStrToHpae[streamDesc->micInStreamInfo_.format];
        }
    }
}

static void UpdateMicInModuleInfo(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");

    if (streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW) {
        moduleInfo.micInRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micInChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatStrToHpae.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micInFormat = formatStrToHpae[streamDesc->micInStreamInfo_.format];
        }
        moduleInfo.micRefRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micRefChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatStrToHpae.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micRefFormat = formatStrToHpae[streamDesc->micInStreamInfo_.format];
        }
    }
}

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

bool AudioPipeSelector::IsBothFastArmUsbNeedRecreate(std::shared_ptr<AudioPipeInfo> newPipe,
    std::shared_ptr<AudioPipeInfo> oldPipe)
{
    CHECK_AND_RETURN_RET(newPipe != nullptr && oldPipe != nullptr, false);
    CHECK_AND_RETURN_RET(!newPipe->streamDescriptors_.empty(), false);
    const auto streamDesc = newPipe->streamDescriptors_.front();
    CHECK_AND_RETURN_RET(!streamDesc->newDeviceDescs_.empty() && !streamDesc->oldDeviceDescs_.empty(), false);
    const auto newDeviceID = streamDesc->newDeviceDescs_.front()->GetDeviceId();
    const auto oldDeviceID = streamDesc->oldDeviceDescs_.front()->GetDeviceId();
    if (newPipe->IsRouteFast() && oldPipe->IsRouteFast() &&
        newPipe->moduleInfo_.className == "usb" && oldPipe->moduleInfo_.className == "usb" &&
        newDeviceID != oldDeviceID) {
        return true;
    }
    return false;
}

AudioPipeSelector::AudioPipeSelector() : pipeManager_(AudioPipeManager::GetPipeManager())
{
}

std::shared_ptr<AudioPipeSelector> AudioPipeSelector::GetPipeSelector()
{
    static std::shared_ptr<AudioPipeSelector> instance = std::make_shared<AudioPipeSelector>();
    return instance;
}

int32_t AudioPipeSelector::SetAudioRouteSelectorCallback(
    const AudioRouteSelectorCallback &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        audioRouteSelectorCallback_ = callback;
    }
    return SUCCESS;
}

int32_t AudioPipeSelector::UnsetAudioRouteSelectorCallback()
{
    AudioRouteSelectorCallback callback;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        std::swap(callback, audioRouteSelectorCallback_);
    }
    return SUCCESS;
}

std::shared_ptr<AudioRouteSelectInfo> AudioPipeSelector::BuildRouteSelectInfo(
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc) const
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, nullptr, "streamDesc is nullptr");
    auto routeSelectInfo = std::make_shared<AudioRouteSelectInfo>();
    CHECK_AND_RETURN_RET_LOG(routeSelectInfo != nullptr, nullptr, "routeSelectInfo is nullptr");

    routeSelectInfo->audioMode = streamDesc->audioMode_;
    routeSelectInfo->streamInfo = streamDesc->streamInfo_;
    routeSelectInfo->streamUsage = streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK ?
        streamDesc->rendererInfo_.streamUsage : STREAM_USAGE_UNKNOWN;
    routeSelectInfo->sourceType = streamDesc->audioMode_ == AUDIO_MODE_RECORD ?
        streamDesc->capturerInfo_.sourceType : SOURCE_TYPE_INVALID;
    routeSelectInfo->appInfo = streamDesc->appInfo_;
    routeSelectInfo->bundleName = streamDesc->bundleName_;
    routeSelectInfo->routeFlag = static_cast<AudioFlag>(streamDesc->routeFlag_);
    return routeSelectInfo;
}

int32_t AudioPipeSelector::NotifyAudioRouteSelect(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    int32_t &selectResult)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_INVALID_PARAM, "streamDesc is nullptr");
    AudioRouteSelectorCallback callback;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback = audioRouteSelectorCallback_;
    }
    if (!callback) {
        return ERR_CALLBACK_NOT_REGISTERED;
    }
    auto routeSelectInfo = BuildRouteSelectInfo(streamDesc);
    CHECK_AND_RETURN_RET_LOG(routeSelectInfo != nullptr, ERR_OPERATION_FAILED, "routeSelectInfo is nullptr");
    int32_t ret = callback(routeSelectInfo, selectResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "route select callback failed");
    return ret;
}

bool AudioPipeSelector::ApplyRouteSelectDecision(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    bool allowRejectCreate)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is nullptr");
    streamDesc->SetRouteSelectRejectedFlag(false);
    streamDesc->routeFlag_ = GetRouteFlagByStreamDesc(streamDesc);
    const uint32_t normalRouteFlag = streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK ?
        AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;

    int32_t selectResult = AudioRouteSelector::ROUTE_SELECT_RESULT_SUCCESS;
    int32_t ret = NotifyAudioRouteSelect(streamDesc, selectResult);
    CHECK_AND_RETURN_RET(ret == SUCCESS, true);

    if (selectResult == AudioRouteSelector::ROUTE_SELECT_RESULT_FALLBACK_NORMAL) {
        streamDesc->routeFlag_ = normalRouteFlag;
        AUDIO_INFO_LOG("route select fallback to normal for stream %{public}u", streamDesc->sessionId_);
        return true;
    }
    if (selectResult == AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE) {
        if (allowRejectCreate) {
            streamDesc->SetRouteSelectRejectedFlag(true);
            AUDIO_INFO_LOG("route select reject create for stream %{public}u", streamDesc->sessionId_);
            return false;
        }
        streamDesc->routeFlag_ = normalRouteFlag;
        AUDIO_WARNING_LOG("route select reject fallback to normal for existing stream %{public}u",
            streamDesc->sessionId_);
    }
    return true;
}

std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeSelector::FetchPipeAndExecute(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = pipeManager_->GetPipeList();

    CHECK_AND_RETURN_RET(ScanPipeListForStreamDesc(pipeInfoList, streamDesc), {});
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

    GetStreamPropInfoWithBusSelector(streamDesc, streamPropInfo);
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
    HILOG_COMM_INFO("[PipeFetchInfo] use new Pipe %{public}s for stream %{public}u",
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
    if (streamPropInfo->channelLayout_ != CH_LAYOUT_UNKNOWN) {
        streamInfo.channelLayout.clear();
        streamInfo.channelLayout.insert(streamPropInfo->channelLayout_);
    }
    temp->audioStreamInfo_ = {streamInfo};
    std::string info = streamInfo.Serialize();
    AUDIO_INFO_LOG("DeviceStreamInfo:%{public}s", info.c_str());
}

void AudioPipeSelector::ProcessRendererAndCapturerConcurrency(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, ConcurrencyAction action)
{
    CHECK_AND_RETURN(action != PLAY_BOTH);
    bool hasFastVoipCapturer = false;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = pipeManager_->GetPipeList();
    for (auto &curPipeInfo : pipeInfoList) {
        CHECK_AND_CONTINUE(curPipeInfo->routeFlag_ == (AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST));
        hasFastVoipCapturer = true;
        break;
    }
    CHECK_AND_RETURN((streamDesc->routeFlag_ == AUDIO_OUTPUT_FLAG_FAST) && hasFastVoipCapturer);
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    AUDIO_INFO_LOG("Set %{public}u to normal flag", streamDesc->GetSessionId());
}

void AudioPipeSelector::CheckFastStreamOverLimitToNormal(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    int32_t fastOutputNum = 0;
    int32_t fastInputNum = 0;
    for (auto &streamDesc : streamDescs) {
        if (streamDesc->GetRoute() == AUDIO_OUTPUT_FLAG_FAST && ++fastOutputNum > MAX_FAST_STREAM_COUNT) {
            AUDIO_INFO_LOG("reach fast limit, set %{public}u to normal", streamDesc->sessionId_);
            streamDesc->ResetToNormalRoute(false);
            continue;
        }
        if (streamDesc->GetRoute() == AUDIO_INPUT_FLAG_FAST && ++fastInputNum > MAX_FAST_STREAM_COUNT) {
            AUDIO_INFO_LOG("reach fast limit, set %{public}u to normal", streamDesc->sessionId_);
            streamDesc->ResetToNormalRoute(false);
            continue;
        }
    }
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeSelector::UpdateStreamDescs(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
    const std::unordered_set<uint32_t> &originalStreamSessionIds)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> result;

    for (auto &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE(originalStreamSessionIds.find(streamDesc->sessionId_) !=
            originalStreamSessionIds.end());
        result.push_back(streamDesc);
    }

    return result;
}

// get each streamDesc's final routeFlag after concurrency
void AudioPipeSelector::DecideFinalRouteFlag(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    CHECK_AND_RETURN_LOG(streamDescs.size() != 0, "streamDescs is empty!");
    SortStreamDescsByStartTime(streamDescs);
    streamDescs.erase(std::remove_if(streamDescs.begin(), streamDescs.end(),
        [this](const std::shared_ptr<AudioStreamDescriptor> &streamDesc) {
            if (streamDesc == nullptr) {
                AUDIO_ERR_LOG("streamDesc is nullptr");
                return true;
            }
            return !ApplyRouteSelectDecision(streamDesc, false);
        }), streamDescs.end());
    CHECK_AND_RETURN_LOG(!streamDescs.empty(), "all streams rejected before concurrency");

    std::unordered_set<uint32_t> originalStreamSessionIds;
    for (auto &streamDesc : streamDescs) {
        originalStreamSessionIds.insert(streamDesc->sessionId_);
    }

    AudioPipeManager::GetPipeManager()->AddSwitchStreams(streamDescs);

    // Do not need to move stream, because stream actions are all decided in DecidePipesAndStreamAction(),
    // not in ProcessConcurrency().
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsMoveToNormal;
    if (streamDescs.size() == 1) {
        // modemCommunication streamDescs stored in modemCommunicationIdMap_, need to do extra concurrencyProcess
        ProcessModemCommunicationConcurrency(streamDescs, streamsMoveToNormal);
        return;
    }

    for (size_t cmpStreamIdx = 1; cmpStreamIdx < streamDescs.size(); ++cmpStreamIdx) {
        // calculate concurrency in time order
        for (size_t curStreamDescIdx = 0; curStreamDescIdx < cmpStreamIdx; ++curStreamDescIdx) {
            ProcessConcurrency(streamDescs[curStreamDescIdx], streamDescs[cmpStreamIdx], streamsMoveToNormal);
        }
    }
    ProcessModemCommunicationConcurrency(streamDescs, streamsMoveToNormal);
    CheckFastStreamOverLimitToNormal(streamDescs);

    streamDescs = UpdateStreamDescs(streamDescs, originalStreamSessionIds);
}

#ifdef MULTI_BUS_ENABLE
bool AudioPipeSelector::IsRecordingPipeMatched(const std::vector<std::string> &busAddresses,
                                               const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
                                               const std::string &adapterName,
                                               const std::shared_ptr<AudioPipeInfo> &newPipeInfo)
{
    if (busAddresses.empty()) {
        return IsPipeMatch(streamDesc, newPipeInfo, adapterName);
    }

    return std::any_of(busAddresses.begin(), busAddresses.end(), [&newPipeInfo](const auto &busAddress) {
        return newPipeInfo->moduleInfo_.busAddress == busAddress;
    });
}

void AudioPipeSelector::HandleFindBusPipe(const std::vector<std::string> &busAddresses,
                                          std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
                                          const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
                                          std::vector<std::shared_ptr<AudioPipeInfo>>::iterator &busPipeIter)
{
    auto FindPipe = [&](std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos, const auto &condition) {
                return std::find_if(pipeInfos.begin(), pipeInfos.end(), condition);
    };
    if (streamDesc != nullptr && streamDesc->IsRecording()) {
        std::string streamDescAdapterName{};
        std::string pipeName{};
        GetAdapterNameByStreamDesc(streamDesc, streamDescAdapterName, pipeName);
        busPipeIter = FindPipe(newPipeInfoList, [&](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
            return IsRecordingPipeMatched(busAddresses, streamDesc, streamDescAdapterName, newPipeInfo);
        });
    } else if (busAddresses.size() == 1) {
        // Vehicle system scenario enter.
        busPipeIter = FindPipe(newPipeInfoList, [&busAddresses](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
            return newPipeInfo->moduleInfo_.name == busAddresses[0];
        });
    } else if (busAddresses.size() > 1) {
        // Vehicle system scenario enter, Filter newPipeInfoList based on busAddress.
        CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        std::shared_ptr<AudioStreamDescriptor> tempDesc = streamDesc;
        pipeManager_->GetStreamPropInfo(tempDesc, streamPropInfo, busAddresses);
        busPipeIter = FindPipe(newPipeInfoList, [&](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
            return std::any_of(busAddresses.begin(), busAddresses.end(),
                               [&](const auto &busAddress) { return newPipeInfo->moduleInfo_.name == busAddress; }) &&
                   newPipeInfo->routeFlag_ == streamDesc->routeFlag_ &&
                   newPipeInfo->audioStreamInfo_.channels == streamPropInfo->channels_;
        });
    }
    if (busPipeIter == newPipeInfoList.end()) {
        HILOG_COMM_ERROR("[HandleFindBusPipe]Fail to find bus.");
    }
}
#endif

void AudioPipeSelector::HandleFindMatchPipe(
    std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> &streamDescToOldPipeInfo,
    std::vector<std::shared_ptr<AudioPipeInfo>>::iterator &matchPipeIter)
{
#ifdef MULTI_BUS_ENABLE
    std::vector<std::string> busAddresses = AudioBusSelector::GetBusSelector().GetBusAddressesByStreamDesc(streamDesc);
        // Vehicle system scenario enter.
    HandleFindBusPipe(busAddresses, newPipeInfoList, streamDesc, matchPipeIter);
#else
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    std::string streamDescAdapterName{};
    std::string pipeName{};
    GetAdapterNameByStreamDesc(streamDesc, streamDescAdapterName, pipeName);
    // When the paused offload/multichannel stream needs recons, pipeManager the stream has reconstucted,
    // but due to delay recons, the actual recons operation do not occur, the stream is still in old pipe,
    // which may cause two offload/multichannel streams running in concurrency.
    if (IsNeedTempMoveToNormal(streamDesc, streamDescToOldPipeInfo)) {
        HILOG_COMM_INFO("[PipeFetchInfo] Temporarily move recons stream %{public}d to primary."
                        " routeFlag %{public}d",
                        streamDesc->GetSessionId(),
                        streamDesc->routeFlag_);
        streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        streamDescAdapterName = "primary";
    }
        // find if curStream's prefer pipe has already exist
    matchPipeIter = std::find_if(newPipeInfoList.begin(), newPipeInfoList.end(),
                                 [&](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
                                     return IsPipeMatch(streamDesc, newPipeInfo, streamDescAdapterName, pipeName);
                                });
#endif
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
            HandleFindMatchPipe(newPipeInfoList, streamDesc, streamDescToOldPipeInfo, newPipeIter);
        }

        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        GetStreamPropInfoWithBusSelector(streamDesc, streamPropInfo);
        if (newPipeIter != newPipeInfoList.end()) {
            MatchRemoteOffloadPipe(streamPropInfo, *newPipeIter, streamDesc);
            MatchUltraFastPipe(*newPipeIter, streamDesc);
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
            newPipeInfo->moduleInfo_.name.c_str(), newPipeInfo->pipeAction_);

        std::vector<uint64_t> sessionIdForLog{};
        for (auto &streamDesc : newPipeInfo->streamDescriptors_) {
            if (streamDescToOldPipeInfo.find(streamDesc->sessionId_) == streamDescToOldPipeInfo.end()) {
                HILOG_COMM_WARN("[PipeFetchInfo] cannot find %{public}d in OldPipeList!", streamDesc->sessionId_);
                continue;
            }
            streamDesc->SetAction(JudgeStreamAction(newPipeInfo, streamDescToOldPipeInfo[streamDesc->GetSessionId()]));
            streamDesc->SetOldRoute(streamDescToOldPipeInfo[streamDesc->GetSessionId()]->GetRoute());
            if (streamDescToOldPipeInfo[streamDesc->GetSessionId()]->GetRoute() == newPipeInfo->GetRoute()) {
                sessionIdForLog.push_back(streamDesc->GetSessionId());
                continue;
            }
            AUDIO_INFO_LOG("    |-[PipeFetchInfo] Id %{public}u, RouteFlag %{public}d -> %{public}d, "
                "sAction %{public}d", streamDesc->GetSessionId(),
                streamDescToOldPipeInfo[streamDesc->GetSessionId()]->GetRoute(),
                newPipeInfo->GetRoute(), streamDesc->GetAction());
        }
        if (!sessionIdForLog.empty()) {
            std::stringstream logstream;
            for (auto sessionid : sessionIdForLog) {
                logstream << sessionid;
                logstream << ", ";
            }
            std::string result = logstream.str();
            result.pop_back();
            AUDIO_INFO_LOG("    |-[PipeFetchInfo] Id %{public}s", result.c_str());
        }
        if (newPipeInfo->streamDescriptors_.size() == 0) {
            AUDIO_INFO_LOG("    |-[PipeFetchInfo] Empty");
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
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList = pipeManager_->GetPipeList();
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
    CHECK_AND_RETURN_RET(!streamDescs.empty(), {});
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
    GetStreamPropInfoWithBusSelector(streamDesc, streamPropInfo);
    ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, pipeInfo);
    pipeInfo.pipeAction_ = PIPE_ACTION_NEW;
    std::shared_ptr<AudioPipeInfo> tempPipeInfo = std::make_shared<AudioPipeInfo>(pipeInfo);
    newPipeInfoList.push_back(tempPipeInfo);
    HILOG_COMM_INFO("[PipeFetchInfo] use new Pipe %{public}s for stream %{public}u with action %{public}d, "
        "routeFlag %{public}d", tempPipeInfo->ToString().c_str(), streamDesc->sessionId_, streamDesc->streamAction_,
        streamDesc->routeFlag_);
}

void AudioPipeSelector::SortStreamByStatus(std::vector<std::pair<std::shared_ptr<AudioStreamDescriptor>,
    std::shared_ptr<AudioPipeInfo>>> &streamList)
{
    sort(streamList.begin(), streamList.end(), [](const auto &a, const auto &b) {
        bool aStatus = (a.first->streamStatus_ == STREAM_STATUS_STARTED);
        bool bStatus = (b.first->streamStatus_ == STREAM_STATUS_STARTED);
        if (aStatus != bStatus) {
            return aStatus > bStatus;
        }
        return a.first->sessionId_ < b.first->sessionId_;
    });
}

bool AudioPipeSelector::ScanPipeListForStreamDesc(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfoList,
    std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET(ApplyRouteSelectDecision(streamDesc, true), false);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsMoveToNormal;

    std::vector<std::pair<std::shared_ptr<AudioStreamDescriptor>, std::shared_ptr<AudioPipeInfo>>> streams;

    for (auto &pipeInfo : pipeInfoList) {
        for (auto &streamDescInPipe : pipeInfo->streamDescriptors_) {
            streams.emplace_back(streamDescInPipe, pipeInfo);
        }
    }
    SortStreamByStatus(streams);
    
    for (auto &[streamDescInPipe, pipeInfo] : streams) {
        bool isUpdate = ProcessConcurrency(streamDescInPipe, streamDesc, streamsMoveToNormal);
        JUDGE_AND_INFO_LOG(isUpdate == true, "isUpdate: %{public}d, action: %{public}d", isUpdate,
            streamDescInPipe->streamAction_);
        if (isUpdate && pipeInfo->GetAction() != PIPE_ACTION_NEW) {
            pipeInfo->SetAction(PIPE_ACTION_UPDATE);
        }
    }
    // modemCommunication streamDescs stored in modemCommunicationIdMap_, need to do extra concurrencyProcess
    std::vector<std::shared_ptr<AudioStreamDescriptor>> tempStreamDescs{streamDesc};
    ProcessModemCommunicationConcurrency(tempStreamDescs, streamsMoveToNormal);

    // Move concede existing streams to its corresponding normal pipe
    MoveStreamsToNormalPipes(streamsMoveToNormal, pipeInfoList);

    HILOG_COMM_INFO("[ScanPipeListForStreamDesc]Route flag after concurrency: %{public}u  sessionId: %{public}u",
        streamDesc->routeFlag_, streamDesc->sessionId_);
    return true;
}

AudioPipeType AudioPipeSelector::GetInputNormalPipeType(uint32_t flag)
{
    if (flag & AUDIO_INPUT_FLAG_FAST) {
        if (flag & AUDIO_INPUT_FLAG_VOIP) {
            return PIPE_TYPE_IN_VOIP;
        } else {
            return PIPE_TYPE_IN_LOWLATENCY;
        }
    } else if (flag & AUDIO_INPUT_FLAG_AI) {
        return PIPE_TYPE_IN_NORMAL_AI;
    } else if (flag & AUDIO_INPUT_FLAG_ULTRASONIC) {
        return PIPE_TYPE_IN_NORMAL_ULTRASONIC;
    } else if (flag & AUDIO_INPUT_FLAG_UNPROCESS) {
        return PIPE_TYPE_IN_NORMAL_UNPROCESS;
    } else if (flag & AUDIO_INPUT_FLAG_LIVE) {
        return PIPE_TYPE_IN_NORMAL_LIVE;
    } else if (flag & AUDIO_INPUT_FLAG_VOICE_RECOGNITION) {
        return PIPE_TYPE_IN_NORMAL_VOICE_RECOGNITION;
    } else if (flag & AUDIO_INPUT_FLAG_RAW_AI) {
        return PIPE_TYPE_IN_NORMAL_RAW_AI;
    } else if (flag & AUDIO_INPUT_FLAG_INTERPHONE) {
        return PIPE_TYPE_IN_INTERPHONE;
    } else if (flag & AUDIO_INPUT_FLAG_CAMCORDER) {
        return PIPE_TYPE_IN_CAMCORDER;
    } else {
        return PIPE_TYPE_IN_NORMAL;
    }
}

AudioPipeType AudioPipeSelector::GetPipeType(uint32_t flag, AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        if (flag & AUDIO_OUTPUT_FLAG_FAST) {
            if (flag & AUDIO_OUTPUT_FLAG_VOIP) {
                return PIPE_TYPE_OUT_VOIP;
            } else {
                return PIPE_TYPE_OUT_LOWLATENCY;
            }
        } else if (flag & AUDIO_OUTPUT_FLAG_DIRECT) {
            if (flag & AUDIO_OUTPUT_FLAG_VOIP) {
                return PIPE_TYPE_OUT_VOIP;
            } else {
                return PIPE_TYPE_OUT_DIRECT_NORMAL;
            }
        } else if (flag & AUDIO_OUTPUT_FLAG_MULTICHANNEL) {
            return PIPE_TYPE_OUT_MULTICHANNEL;
        } else if (flag & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) {
            return PIPE_TYPE_OUT_OFFLOAD;
        } else if (flag & AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION) {
            return PIPE_TYPE_OUT_CELLULAR_CALL;
        } else if (flag & AUDIO_OUTPUT_FLAG_INTERPHONE) {
            return PIPE_TYPE_OUT_INTERPHONE;
        } else {
            return PIPE_TYPE_OUT_NORMAL;
        }
    } else {
        return GetInputNormalPipeType(flag);
    }
}

bool AudioPipeSelector::IsSameAdapter(std::shared_ptr<AudioStreamDescriptor> streamDescA,
    std::shared_ptr<AudioStreamDescriptor> streamDescB)
{
    CHECK_AND_RETURN_RET(streamDescA != nullptr && streamDescB != nullptr && streamDescA->newDeviceDescs_.size() != 0 &&
        streamDescB->newDeviceDescs_.size() != 0, true);
    bool hasRemote = false;
#ifdef MULTI_BUS_ENABLE
    auto GetPortName = [](const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
                          const std::shared_ptr<AudioStreamDescriptor> &streamDesc) {
        return AudioPolicyUtils::GetInstance().GetSinkName(deviceDesc, streamDesc->sessionId_);
#else
    auto GetPortName = [this](const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
                          const std::shared_ptr<AudioStreamDescriptor> &streamDesc) {
        AudioPipeType pipeType = this->GetPipeType(streamDesc->routeFlag_, streamDesc->audioMode_);
        return AudioPolicyUtils::GetInstance().GetSinkPortName(deviceDesc->deviceType_, pipeType);
#endif
    };

    for (auto deviceDescA : streamDescA->newDeviceDescs_) {
        CHECK_AND_CONTINUE(deviceDescA != nullptr);
        std::string portNameA = GetPortName(deviceDescA, streamDescA);
        bool isRemoteA = deviceDescA->networkId_ != LOCAL_NETWORK_ID;
        hasRemote = isRemoteA ? true : hasRemote;
        for (auto deviceDescB : streamDescB->newDeviceDescs_) {
            CHECK_AND_CONTINUE(deviceDescB != nullptr);
            std::string portNameB = GetPortName(deviceDescB, streamDescB);
            bool isRemoteB = deviceDescB->networkId_ != LOCAL_NETWORK_ID;
            hasRemote = isRemoteB ? true : hasRemote;
            CHECK_AND_RETURN_RET(!(isRemoteA == isRemoteB && portNameA == portNameB), true);
        }
    }
    CHECK_AND_RETURN_RET(hasRemote, true);
    AUDIO_INFO_LOG("diff adapter, not need concurrency");
    return false;
}

void AudioPipeSelector::SetPipeTypeByStreamType(AudioPipeType &nowPipeType,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        nowPipeType = PIPE_TYPE_OUT_VOIP;
    }
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        nowPipeType = PIPE_TYPE_IN_VOIP;
    }
}

bool AudioPipeSelector::GetIncomingStreamPriority(const std::shared_ptr<AudioStreamDescriptor> &existingStream,
    const std::shared_ptr<AudioStreamDescriptor> &incomingStream)
{
    // When two music streams need to be played on remote offload, select multi-channel audio source, or source with a
 	// higher sampling rate.
    CHECK_AND_RETURN_RET(existingStream != nullptr && incomingStream != nullptr, false);
    CHECK_AND_RETURN_RET(existingStream->IsRunningOffload() && incomingStream->IsRunningOffload(), false);
    CHECK_AND_RETURN_RET(existingStream->IsDeviceRemote() && incomingStream->IsDeviceRemote(), false);

    bool startTimePriority = incomingStream->startTimeStamp_ < existingStream->startTimeStamp_;
    if (startTimePriority) {
        return incomingStream->IsMultiChannelStream() || (!existingStream->IsMultiChannelStream() &&
            incomingStream->GetSamplingRate() >= existingStream->GetSamplingRate());
    }
    return !existingStream->IsMultiChannelStream() && (incomingStream->IsMultiChannelStream() ||
        incomingStream->GetSamplingRate() > existingStream->GetSamplingRate());
}

ConcurrencyAction AudioPipeSelector::DetermineConcurrencyAction(
    std::shared_ptr<AudioStreamDescriptor> &existingStream,
    std::shared_ptr<AudioStreamDescriptor> &incomingStream)
{
    AudioPipeType existingPipe = AudioConcurrencyManager::GetInstance().GetPipeTypeByRouteFlag(
        existingStream->routeFlag_, existingStream->audioMode_);
    AudioPipeType commingPipe = AudioConcurrencyManager::GetInstance().GetPipeTypeByRouteFlag(
        incomingStream->routeFlag_, incomingStream->audioMode_);
    SetPipeTypeByStreamType(existingPipe, existingStream);
    SetPipeTypeByStreamType(commingPipe, incomingStream);
    ConcurrencyAction action = AudioConcurrencyManager::GetInstance().GetConcurrencyAction(existingPipe, commingPipe);
    action = IsSameAdapter(existingStream, incomingStream) ? action : PLAY_BOTH;
    UpdateActionForHdPlay(existingStream, incomingStream, action);
    bool isIncomingStreamIsLoopback = incomingStream->capturerInfo_.isLoopback ||
        incomingStream->rendererInfo_.isLoopback;
    // No running offload can not concede incoming special pipe
    // incoming is loopback, concede existing offload
    // Two remote offload streams, concede the one with lower quality.
    bool isConcedeExisting = action == CONCEDE_INCOMING &&
        (existingStream->IsNoRunningOffload() ||
        (existingStream->IsRouteOffload() && isIncomingStreamIsLoopback) ||
        GetIncomingStreamPriority(existingStream, incomingStream));
    CHECK_AND_RETURN_RET(isConcedeExisting, action);
    action = CONCEDE_EXISTING;
    return action;
}

bool AudioPipeSelector::ShouldAdjustConcedeAction(const std::shared_ptr<AudioStreamDescriptor> &existingStream,
    const std::shared_ptr<AudioStreamDescriptor> &incomingStream,
    const std::shared_ptr<AudioStreamDescriptor> &targetStream)
{
    CHECK_AND_RETURN_RET_LOG(targetStream != nullptr, false, "targetStream is nullptr");
    CHECK_AND_RETURN_RET_LOG(!targetStream->IsLoopbackStream(), false,
        "target stream %{public}u is loopback, skip adjust", targetStream->GetSessionId());
    CHECK_AND_RETURN_RET(IsUpdateActionForDeviceLevelHdPlay(existingStream, incomingStream), false);
    return true;
}

void AudioPipeSelector::AdjustConcedeActionByStatus(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream, ConcurrencyAction &action)
{
    CHECK_AND_RETURN(action == CONCEDE_INCOMING || action == CONCEDE_EXISTING);

    std::shared_ptr<AudioStreamDescriptor> targetStream =
        (action == CONCEDE_INCOMING) ? existingStream : incomingStream;
    CHECK_AND_RETURN(ShouldAdjustConcedeAction(existingStream, incomingStream, targetStream));

    bool isStreamInSwitch = AudioPipeManager::GetPipeManager()->IsSwitchStreamExist(targetStream->sessionId_);
    // Stream is in persistenly pasued state, unrelated to trigger recreation.
    if (targetStream->streamStatus_ != STREAM_STATUS_STARTED && !isStreamInSwitch) {
        action = (action == CONCEDE_EXISTING) ? CONCEDE_INCOMING : CONCEDE_EXISTING;
        AUDIO_INFO_LOG("[AdjustConcedeActionByStatus] sessionId_: %{public}u, streamStatus_: %{public}u,"
            " isStreamInSwitch: %{public}d, action: %{public}d", targetStream->sessionId_,
            targetStream->streamStatus_, isStreamInSwitch, action);
    }
    // Stream requires triggerrecreate to normal pipe, pending execution.
    if (isStreamInSwitch && targetStream->IsFutureRouteNormal()) {
        action = (action == CONCEDE_EXISTING) ? CONCEDE_INCOMING : CONCEDE_EXISTING;
        AUDIO_INFO_LOG("[AdjustConcedeActionByStatus] sessionId_: %{public}u, isStreamInSwitch: %{public}d,"
            " futureRouteFlag: %{public}u, action: %{public}d", targetStream->sessionId_,
            isStreamInSwitch, targetStream->futureRouteFlag_, action);
    }
}

bool AudioPipeSelector::ProcessConcurrency(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove)
{
    ConcurrencyAction action = DetermineConcurrencyAction(existingStream, incomingStream);

    JUDGE_AND_INFO_LOG(action != PLAY_BOTH, "Action: %{public}u "
        "existingStream id: %{public}u, routeFlag: %{public}u; "
        "incomingStream id: %{public}u, routeFlag: %{public}u",
        action,
        existingStream->GetSessionId(), existingStream->GetRoute(),
        incomingStream->GetSessionId(), incomingStream->GetRoute());

    AdjustConcedeActionByStatus(existingStream, incomingStream, action);
    ProcessRendererAndCapturerConcurrency(incomingStream, action);

    bool isUpdate = false;
    switch (action) {
        case PLAY_BOTH:
            break;
        case CONCEDE_INCOMING:
            HandleConcedeIncoming(existingStream, incomingStream);
            break;
        case CONCEDE_EXISTING:
            isUpdate = HandleConcedeExisting(existingStream, incomingStream, streamsToMove);
            break;
        case CONCEDE_BOTH:
            isUpdate = HandleConcedeBoth(existingStream, incomingStream, streamsToMove);
            break;
        default:
            break;
    }
    return isUpdate;
}

void AudioPipeSelector::HandleConcedeIncoming(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream)
{
    incomingStream->ResetToNormalRoute(false);
    SetOriginalFlagForcedNormalIfNeed(incomingStream);
    existingStream->SetFutureRoute(AUDIO_FLAG_NONE);
    incomingStream->SetFutureRoute(incomingStream->GetRoute());
}

bool AudioPipeSelector::HandleConcedeExisting(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove)
{
    bool isUpdate = true;
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
    incomingStream->SetFutureRoute(AUDIO_FLAG_NONE);
    existingStream->SetFutureRoute(existingStream->GetRoute());
    return isUpdate;
}

bool AudioPipeSelector::HandleConcedeBoth(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamsToMove)
{
    bool isUpdate = HandleConcedeExisting(existingStream, incomingStream, streamsToMove);
    incomingStream->ResetToNormalRoute(false);
    SetOriginalFlagForcedNormalIfNeed(incomingStream);
    incomingStream->SetFutureRoute(incomingStream->GetRoute());
    return isUpdate;
}

uint32_t AudioPipeSelector::GetRouteFlagByStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    uint32_t flag = AUDIO_FLAG_NONE;
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, flag, "streamDesc is nullptr");
    flag = pipeManager_->GetRouteFlag(streamDesc);
    return flag;
}

void AudioPipeSelector::GetStreamPropInfoWithBusSelector(std::shared_ptr<AudioStreamDescriptor> &desc,
                                                         std::shared_ptr<PipeStreamPropInfo> &info)
{
#ifdef MULTI_BUS_ENABLE
    std::vector<std::string> busAddresses = AudioBusSelector::GetBusSelector().GetBusAddressesByStreamDesc(desc);
    pipeManager_->GetStreamPropInfo(desc, info, busAddresses);
#else
    pipeManager_->GetStreamPropInfo(desc, info);
#endif
}

void AudioPipeSelector::GetAdapterNameByStreamDesc(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, std::string &adapterName, std::string &pipeName)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    GetStreamPropInfoWithBusSelector(streamDesc, streamPropInfo);
    CHECK_AND_RETURN_LOG(streamPropInfo != nullptr, "StreamProp Info is null");

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = streamPropInfo->pipeInfo_.lock();
    if (pipeInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Adapter info is null");
        return;
    }

    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = pipeInfoPtr->adapterInfo_.lock();
    if (adapterInfoPtr == nullptr) {
        AUDIO_ERR_LOG("Pipe info is null");
        return;
    }
    adapterName = adapterInfoPtr->adapterName;
    pipeName = pipeInfoPtr->name_;
}

static bool IsMultiStreamPrimary(const std::string &pipeName)
{
    return pipeName == "primary_typec_output" || pipeName == "primary_wired_output" ||
           pipeName == "primary_hdmi_output" || pipeName == "primary_wireless_output";
}

static void FillSpecialPipeInfoExt(AudioPipeInfo &info, std::shared_ptr<AdapterPipeInfo> pipeInfoPtr,
    std::shared_ptr<AudioStreamDescriptor> streamDesc, std::shared_ptr<PipeStreamPropInfo> streamPropInfo)
{
    if (pipeInfoPtr->name_ == "fast_a2dp_output") {
        info.moduleInfo_.className = "a2dp_fast";
        info.moduleInfo_.fileName = "a2dp_fast_dump_file";
        info.moduleInfo_.name = "FastA2DPRendererSink";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.role = "sink";
        info.moduleInfo_.bufferSize = "3840";
    } else if (pipeInfoPtr->name_ == "fast_input") {
        info.moduleInfo_.className = "primary_mmap";
        info.moduleInfo_.fileName = "fast_source_dump_file";
        info.moduleInfo_.name = "FastCapturerSource";
        info.moduleInfo_.lib = "libmodule-hdi-source.z.so";
        info.moduleInfo_.role = "source";
    } else if (pipeInfoPtr->name_ == "voip_fast_input") {
        info.moduleInfo_.className = "primary_mmap_voip";
        info.moduleInfo_.fileName = "fast_voip_source_dump_file";
        info.moduleInfo_.name = "FastVOIPCapturerSource";
        info.moduleInfo_.lib = "libmodule-hdi-source.z.so";
        info.moduleInfo_.role = "source";
    } else if (pipeInfoPtr->name_ == "direct_output") {
        info.moduleInfo_.className = "primary_direct";
        info.moduleInfo_.fileName = "direct_dump_file";
        info.moduleInfo_.lib = "libmodule-hdi=-sink.z.so";
        info.moduleInfo_.name = "direct";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "voip_direct_output") {
        info.moduleInfo_.className = "primary_direct_voip";
        info.moduleInfo_.fileName = "direct_dump_file";
        info.moduleInfo_.lib = "libmodule-hdi=-sink.z.so";
        info.moduleInfo_.name = "voip";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "fast_usb_arm_output") {
        info.moduleInfo_.className = info.GetUltraFastFlag() ? "usb_arm_ultra_fast" : "usb_arm_fast";
        info.moduleInfo_.fileName = info.GetUltraFastFlag() ? "usb_arm_ultra_fast_dump_file" : "usb_arm_fast_dump_file";
        info.moduleInfo_.name = info.GetUltraFastFlag() ? "UltraFastUsbArmRendererSink" : "FastUsbArmRendererSink";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.role = "sink";
    }
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
        info.moduleInfo_.suspendIdleTimeout = pipeInfoPtr->suspendIdleTimeout_;
    } else if (pipeInfoPtr->name_ == "offload_distributed_output") {
        info.moduleInfo_.className = "remote_offload";
        info.moduleInfo_.offloadEnable = "1";
        info.moduleInfo_.fixedLatency = "1";
        info.moduleInfo_.fileName = "remote_offload_dump_file";
        info.moduleInfo_.name =
            AudioPolicyUtils::GetInstance().GetRemoteModuleName(streamDesc->newDeviceDescs_[0]->networkId_,
            AudioPolicyUtils::GetInstance().GetDeviceRole(streamDesc->newDeviceDescs_[0]->deviceType_)) + "_offload";
    } else if (pipeInfoPtr->name_ == "fast_output") {
        info.moduleInfo_.className = info.GetUltraFastFlag() ? "ultra_fast" : "primary_mmap";
        info.moduleInfo_.fileName = info.GetUltraFastFlag() ? "ultra_fast_dump_file" : "fast_dump_file";
        info.moduleInfo_.name = info.GetUltraFastFlag() ? "UltraFastRendererSink" : "FastRendererSink";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "voip_fast_output") {
        info.moduleInfo_.className = "primary_mmap_voip";
        info.moduleInfo_.fileName = "fast_voip_dump_file";
        info.moduleInfo_.name = "FastVOIPRendererSink";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.role = "sink";
    } else {
        FillSpecialPipeInfoExt(info, pipeInfoPtr, streamDesc, streamPropInfo);
    }
}

void AudioPipeSelector::UpdateMouleInfoWitchDevice(const std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(deviceDesc, "streamDesc is nullptr");
    moduleInfo.deviceType = std::to_string(deviceDesc->deviceType_);
    moduleInfo.networkId = deviceDesc->networkId_;
    moduleInfo.macAddress = deviceDesc->macAddress_;
    moduleInfo.hdPlayBackMode = std::to_string(static_cast<int32_t>(deviceDesc->hdPlaybackMode_));
    if (deviceDesc->getType() == DEVICE_TYPE_USB_ARM_HEADSET) {
        CHECK_AND_RETURN_LOG(!deviceDesc->GetAudioStreamInfo().empty(), "audio streamInfo empty");
        const DeviceStreamInfo deviceAudioStreamInfo = deviceDesc->GetAudioStreamInfo().back();

        CHECK_AND_RETURN_LOG(!deviceAudioStreamInfo.samplingRate.empty(), "samplingRate set empty");
        moduleInfo.rate = to_string(*(deviceAudioStreamInfo.samplingRate.begin()));

        auto it = AudioDefinitionPolicyUtils::enumToFormatStr.find(deviceAudioStreamInfo.format);
        CHECK_AND_RETURN_LOG(it != AudioDefinitionPolicyUtils::enumToFormatStr.end(),
            "Not found %{public}u in enumToFormatStr", static_cast<uint32_t>(deviceAudioStreamInfo.format));
        moduleInfo.format = it->second;
    }
}

void AudioPipeSelector::ConvertStreamDescToPipeInfo(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo, AudioPipeInfo &info)
{
    CHECK_AND_RETURN_LOG(streamPropInfo != nullptr, "streamPropInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = streamPropInfo->pipeInfo_.lock();
    CHECK_AND_RETURN_LOG(pipeInfoPtr, "pipeInfoPtr is null");
    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = pipeInfoPtr->adapterInfo_.lock();
    CHECK_AND_RETURN_LOG(pipeInfoPtr, "adapterInfoPtr is null");

    info.moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[streamPropInfo->format_];
    info.moduleInfo_.rate = std::to_string(streamPropInfo->sampleRate_);
    info.moduleInfo_.channels = std::to_string(ConvertLayoutToAudioChannel(
        streamPropInfo->channelLayout_));
    info.moduleInfo_.bufferSize = std::to_string(streamPropInfo->bufferSize_);

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT) {
        info.moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
        info.moduleInfo_.ecSamplingRate = std::to_string(streamDesc->ecStreamInfo_.samplingRate);
        info.moduleInfo_.ecChannels = std::to_string(streamDesc->ecStreamInfo_.channels);
        info.moduleInfo_.ecFormat = formatStrToHpae[streamDesc->ecStreamInfo_.format];
    } else if (IsVoiceRecognitionMicInEcRequested(streamDesc)) {
        UpdateMicInEcModuleInfo(streamDesc, info.moduleInfo_);
    } else if (IsCamcorderMicInRequested(streamDesc)) {
        UpdateMicInModuleInfo(streamDesc, info.moduleInfo_);
    }

    info.moduleInfo_.lib = pipeInfoPtr->paProp_.lib_;
    info.moduleInfo_.role = pipeInfoPtr->paProp_.role_;
    info.moduleInfo_.name = pipeInfoPtr->paProp_.moduleName_;
    info.moduleInfo_.busAddress = pipeInfoPtr->paProp_.busAddress_;
    info.moduleInfo_.adapterName = adapterInfoPtr->adapterName;
    info.moduleInfo_.className = adapterInfoPtr->adapterName;
    info.moduleInfo_.OpenMicSpeaker = pipeManager_->GetUpdateRouteSupport() ? "1" : "0";

    HILOG_COMM_INFO("[ConvertStreamDescToPipeInfo]Pipe name: %{public}s", pipeInfoPtr->name_.c_str());
    HILOG_COMM_INFO("[ConvertStreamDescToPipeInfo]channels: %{public}s, channelLayout: %{public}s",
        info.moduleInfo_.channels.c_str(), info.moduleInfo_.channelLayout.c_str());

    info.moduleInfo_.sourceType = std::to_string(streamDesc->capturerInfo_.sourceType);
    info.moduleInfo_.renderInIdleState = pipeInfoPtr->paProp_.renderInIdleState_;

    if (!streamDesc->newDeviceDescs_.empty()) {
        UpdateMouleInfoWitchDevice(streamDesc->newDeviceDescs_[0], info.moduleInfo_);
    }

    if (streamDesc->IsUltraFastRequested() &&
        !AudioPipeManager::GetPipeManager()->HasRunningStream(streamDesc->sessionId_)) {
        info.SetUltraFastFlag(true);
        streamDesc->SetUltraFastImplemented(true);
    }

    // engine uses networkId to distinguish different renders,
    // so we need to set networkId after UpdateMouleInfoWitchDevice;
    FillSpecialPipeInfo(info, pipeInfoPtr, streamDesc, streamPropInfo);

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

    if (AudioPipeManager::GetPipeManager()->IsSamePipe(newPipe, oldPipe)) {
        return AUDIO_STREAM_ACTION_DEFAULT;
    }
    return AUDIO_STREAM_ACTION_MOVE;
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
    HILOG_COMM_INFO("[RemoveTargetStreams]ModemCommunication exists, need process concurrency");
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
    CHECK_AND_RETURN_RET(!streamDesc->IsRunning(), false);
    CHECK_AND_RETURN_RET_LOG(streamDescToOldPipeInfo.size() != 0, false, "streamDescToOldPipeInfo is empty!");
    const auto sessionID = streamDesc->GetSessionId();
    CHECK_AND_RETURN_RET(streamDescToOldPipeInfo[sessionID], false);
    return (streamDescToOldPipeInfo[sessionID]->IsRenderPipeNeedMoveToNormal() &&
        streamDesc->IsRenderStreamNeedRecreate());
}

bool AudioPipeSelector::FindExistingPipe(std::vector<std::shared_ptr<AudioPipeInfo>> &selectedPipeInfoList,
    const std::shared_ptr<AdapterPipeInfo> &pipeInfoPtr, std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo)
{
    for (auto &pipeInfo : selectedPipeInfoList) {
        std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = pipeInfoPtr->adapterInfo_.lock();
        CHECK_AND_CONTINUE_LOG(adapterInfoPtr != nullptr, "Adapter info is null");

        HILOG_COMM_INFO("PipeName %{public}s action %{public}d adapter[%{public}s] pipeRoute[0x%{public}x] "
                       "streamRoute[0x%{public}x] "
                       "pipeChannel[%{public}d] streamChannel[%{public}d] pipeFormat[%{public}d] "
                       "streamFormat[%{public}d] pipeSamplingRate[%{public}d] streamSamplingRate[%{public}d]",
                        pipeInfo->name_.c_str(), pipeInfo->GetAction(), pipeInfo->GetAdapterName().c_str(),
                        pipeInfo->GetRoute(), streamDesc->GetRoute(), pipeInfo->audioStreamInfo_.channels,
                        streamPropInfo->channels_, pipeInfo->audioStreamInfo_.format, streamPropInfo->format_,
                        pipeInfo->audioStreamInfo_.samplingRate, streamPropInfo->sampleRate_);

#ifdef MULTI_BUS_ENABLE
        CHECK_AND_CONTINUE(pipeInfo->adapterName_ == adapterInfoPtr->adapterName &&
                               pipeInfo->routeFlag_ == streamDesc->routeFlag_ &&
                               pipeInfo->audioStreamInfo_.channels == streamPropInfo->channels_);
        if (streamDesc->IsRecording() && !pipeInfo->moduleInfo_.busAddress.empty()) {
            CHECK_AND_CONTINUE(pipeInfoPtr->paProp_.busAddress_ == pipeInfo->moduleInfo_.busAddress);
        }
#else
        CHECK_AND_CONTINUE(IsPipeMatch(streamDesc, pipeInfo, adapterInfoPtr->adapterName, pipeInfoPtr->name_));
#endif

        MatchRemoteOffloadPipe(streamPropInfo, pipeInfo, streamDesc);

        if (((pipeInfo->GetRoute() & AUDIO_OUTPUT_FLAG_FAST) || (pipeInfo->GetRoute() & AUDIO_INPUT_FLAG_FAST)) &&
            pipeInfo->streamDescriptors_.size() == MAX_FAST_STREAM_COUNT) {
            AUDIO_INFO_LOG("reach fast limit, set %{public}u to normal", streamDesc->sessionId_);
            streamDesc->ResetToNormalRoute(false);
            return FindExistingPipe(selectedPipeInfoList, pipeInfoPtr, streamDesc, streamPropInfo);
        }
        MatchUltraFastPipe(pipeInfo, streamDesc);

        pipeManager_->AddStreamToPipe(pipeInfo, streamDesc, PIPE_ACTION_UPDATE);
        HILOG_COMM_INFO("[PipeFetchInfo] use existing Pipe %{public}s for stream %{public}u, pipeAction: %{public}d",
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

void AudioPipeSelector::MatchUltraFastPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN(streamDesc != nullptr && pipeInfo != nullptr, "streamDesc or pipeInfo is nullptr");
    CHECK_AND_RETURN(pipeInfo->GetRoute() & AUDIO_OUTPUT_FLAG_FAST);
    streamDesc->SetUltraFastImplemented(pipeInfo->GetUltraFastFlag());
}

bool AudioPipeSelector::IsPipeFormatMatch(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
    std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    std::string channels = std::to_string(ConvertLayoutToAudioChannel(streamPropInfo->channelLayout_));
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
    info.moduleInfo_.channels = std::to_string(ConvertLayoutToAudioChannel(streamPropInfo->channelLayout_));
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

bool AudioPipeSelector::IsPipeMatch(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const std::shared_ptr<AudioPipeInfo> &pipeInfo, const std::string &adapterName, const std::string &pipeName)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr && pipeInfo != nullptr, false);

    CHECK_AND_RETURN_RET(pipeInfo->GetRoute() == streamDesc->GetRoute() && pipeInfo->IsSameAdapter(adapterName) &&
        pipeInfo->IsSameName(pipeName), false);

    // Use networkId to distinguish multiple remote devices that may exist
    auto deviceDesc = streamDesc->GetMainNewDeviceDesc();
#ifdef MULTI_BUS_ENABLE
    CHECK_AND_RETURN_RET(deviceDesc != nullptr && deviceDesc->macAddress_ != pipeInfo->moduleInfo_.busAddress, true);
#endif
    CHECK_AND_RETURN_RET(deviceDesc != nullptr && deviceDesc->networkId_ != LOCAL_NETWORK_ID, true);

    return pipeInfo->IsSameNetworkId(deviceDesc->networkId_);
}

int32_t AudioPipeSelector::SetCustomAudioMix(const std::string &zoneName, const std::vector<AudioZoneMix> &audioMixes)
{
#ifdef MULTI_BUS_ENABLE
    return AudioBusSelector::GetBusSelector().SetCustomAudioMix(zoneName, audioMixes);
#else
    return SUCCESS;
#endif
}

// If hd playback, set normal routeflag for more than two active streams.
void AudioPipeSelector::UpdateActionForHdPlay(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream, ConcurrencyAction &action)
{
    CHECK_AND_RETURN_LOG(existingStream != nullptr && incomingStream != nullptr, "stream is nullptr");
    auto existingHdPlayMode = existingStream->GetHdPlaybackMode();
    auto incomingHdPlayMode = incomingStream->GetHdPlaybackMode();
    CHECK_AND_RETURN(existingHdPlayMode != HdPlaybackMode::HD_NOT_SUPPORTED &&
        incomingHdPlayMode != HdPlaybackMode::HD_NOT_SUPPORTED &&
        (existingStream->IsRunning() && incomingStream->IsRunning()) &&
        (existingStream->IsRouteOffload() || incomingStream->IsRouteOffload()));

    ConcurrencyAction newAction = action;
    if (existingStream->IsRouteOffload()) {
        newAction = (action == CONCEDE_INCOMING) ? CONCEDE_BOTH :
            (action == PLAY_BOTH) ? CONCEDE_EXISTING : action;
    } else if (incomingStream->IsRouteOffload()) {
        newAction = (action == PLAY_BOTH) ? CONCEDE_INCOMING : action;
    }
    AUDIO_INFO_LOG("old action: %{public}d, update as %{public}d", action, newAction);
    action = newAction;
}

bool AudioPipeSelector::IsUpdateActionForDeviceLevelHdPlay(
    const std::shared_ptr<AudioStreamDescriptor> &existingStream,
    const std::shared_ptr<AudioStreamDescriptor> &incomingStream)
{
    CHECK_AND_RETURN_RET_LOG(existingStream != nullptr && incomingStream != nullptr, false, "stream is nullptr");
    auto existingHdPlayMode = existingStream->GetHdPlaybackMode();
    auto incomingHdPlayMode = incomingStream->GetHdPlaybackMode();
    CHECK_AND_RETURN_RET_LOG(existingHdPlayMode != HdPlaybackMode::DEVICE_LEVEL ||
        incomingHdPlayMode != HdPlaybackMode::DEVICE_LEVEL, false, "device level, not update action");
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
