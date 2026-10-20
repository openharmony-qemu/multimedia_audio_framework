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

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "audio_stream_collector.h"
#include "audio_stream_info.h"
#include "audio_definition_adapter_info.h"
#include "audio_core_config_manager.h"
#include "audio_policy_utils.h"
#include "audio_errors.h"
#include "audio_policy_interface.h"
#include "audio_service_enum.h"
#include "audio_injector_policy.h"
#include "audio_bus_selector.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B84

namespace OHOS {
namespace AudioStandard {

namespace {
    constexpr uint32_t MS_PER_S = 1000;
}

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

static const std::map<SourceType, int32_t> g_inputSourceTypePriority = {
    {SOURCE_TYPE_VOICE_CALL, 8},
    {SOURCE_TYPE_VOICE_COMMUNICATION, 7},
    {SOURCE_TYPE_VOICE_MESSAGE, 6},
    {SOURCE_TYPE_LIVE, 5},
    {SOURCE_TYPE_VOICE_RECOGNITION, 4},
    {SOURCE_TYPE_VOICE_TRANSCRIPTION, 4},
    {SOURCE_TYPE_MIC, 3},
    {SOURCE_TYPE_CAMCORDER, 3},
    {SOURCE_TYPE_UNPROCESSED, 2},
    {SOURCE_TYPE_ULTRASONIC, 1},
    {SOURCE_TYPE_INVALID, 0},
};

static int32_t GetInputSourceTypePriority(SourceType sourceType)
{
    auto iter = g_inputSourceTypePriority.find(sourceType);
    return iter != g_inputSourceTypePriority.end() ? iter->second : g_inputSourceTypePriority.at(SOURCE_TYPE_INVALID);
}

static bool IsHigherPriorityInputStream(const std::shared_ptr<AudioStreamDescriptor> &candidate,
    const std::shared_ptr<AudioStreamDescriptor> &current)
{
    CHECK_AND_RETURN_RET(candidate != nullptr, false);
    CHECK_AND_RETURN_RET(current != nullptr, true);
    return GetInputSourceTypePriority(candidate->capturerInfo_.sourceType) >
        GetInputSourceTypePriority(current->capturerInfo_.sourceType);
}

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

static bool IsCamcorderMicInSupportedInputDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return deviceDesc != nullptr && deviceDesc->networkId_ == LOCAL_NETWORK_ID &&
        Util::IsCamcorderMicInSupportedInputDeviceType(deviceDesc->deviceType_);
}

static bool HasCamcorderDegradeInputDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    return std::any_of(deviceDescs.begin(), deviceDescs.end(), [](const auto &deviceDesc) {
        return deviceDesc != nullptr && !IsCamcorderMicInSupportedInputDevice(deviceDesc);
    });
}

static bool IsCamcorderDegradeInputStream(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return HasCamcorderDegradeInputDevice(streamDesc->newDeviceDescs_) ||
        HasCamcorderDegradeInputDevice(streamDesc->oldDeviceDescs_);
}

static bool IsEcEnabled(const std::string &ecType)
{
    return !ecType.empty() && ecType != std::to_string(EC_TYPE_NONE);
}

static std::string NormalizeEcType(const std::string &ecType)
{
    return IsEcEnabled(ecType) ? ecType : "";
}

static bool IsMicRefEnabled(const std::string &openMicRef)
{
    return openMicRef == "1" || openMicRef == "true";
}

static bool IsMicInEnabled(const AudioModuleInfo &moduleInfo)
{
    return !moduleInfo.micInRate.empty() || !moduleInfo.micInChannels.empty() || !moduleInfo.micInFormat.empty();
}

static bool IsHighestPriorityStreamChanged(const std::shared_ptr<AudioStreamDescriptor> &oldHighest,
    const std::shared_ptr<AudioStreamDescriptor> &newHighest)
{
    if ((oldHighest == nullptr) != (newHighest == nullptr)) {
        return true;
    }
    CHECK_AND_RETURN_RET(oldHighest != nullptr && newHighest != nullptr, false);
    return oldHighest->sessionId_ != newHighest->sessionId_ || oldHighest->streamStatus_ != newHighest->streamStatus_;
}

static bool IsPipeStreamPropValid(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo)
{
    return streamPropInfo != nullptr && streamPropInfo->sampleRate_ > 0 &&
        streamPropInfo->channels_ > CHANNEL_UNKNOW && streamPropInfo->channelLayout_ != CH_LAYOUT_UNKNOWN &&
        streamPropInfo->format_ != INVALID_WIDTH;
}

static void ClearEcModuleInfo(AudioModuleInfo &moduleInfo)
{
    moduleInfo.ecType = "";
    moduleInfo.ecAdapter = "";
    moduleInfo.ecSamplingRate = "";
    moduleInfo.ecFormat = "";
    moduleInfo.ecChannels = "";
}

static void UpdateEcModuleInfo(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, const std::string &ecType,
    AudioModuleInfo &moduleInfo)
{
    if (!IsEcEnabled(ecType)) {
        ClearEcModuleInfo(moduleInfo);
        return;
    }

    if (ecType == std::to_string(EC_TYPE_DIFF_ADAPTER)) {
        moduleInfo.ecType = ecType;
        return;
    }

    if (streamDesc == nullptr || streamDesc->ecStreamInfo_.channels <= CHANNEL_UNKNOW) {
        ClearEcModuleInfo(moduleInfo);
        return;
    }

    moduleInfo.ecType = ecType;
    moduleInfo.ecAdapter = "";
    moduleInfo.ecSamplingRate = std::to_string(streamDesc->ecStreamInfo_.samplingRate);
    moduleInfo.ecChannels = std::to_string(streamDesc->ecStreamInfo_.channels);
    if (formatStrToHpae.count(streamDesc->ecStreamInfo_.format) > 0) {
        moduleInfo.ecFormat = formatStrToHpae[streamDesc->ecStreamInfo_.format];
    } else {
        moduleInfo.ecFormat = "";
    }
}

static void ClearMicRefModuleInfo(AudioModuleInfo &moduleInfo)
{
    moduleInfo.openMicRef = IsMicRefEnabled(moduleInfo.openMicRef) ? "0" : "";
    moduleInfo.micRefRate = "";
    moduleInfo.micRefFormat = "";
    moduleInfo.micRefChannels = "";
}

static void ClearMicInModuleInfo(AudioModuleInfo &moduleInfo)
{
    moduleInfo.micInRate = "";
    moduleInfo.micInFormat = "";
    moduleInfo.micInChannels = "";
}

static void UpdateMicInModuleInfo(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool needMicIn,
    AudioModuleInfo &moduleInfo)
{
    if (!needMicIn || streamDesc == nullptr || streamDesc->micInStreamInfo_.channels <= CHANNEL_UNKNOW) {
        ClearMicInModuleInfo(moduleInfo);
        return;
    }

    moduleInfo.micInRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
    moduleInfo.micInChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
    if (formatStrToHpae.count(streamDesc->micInStreamInfo_.format) > 0) {
        moduleInfo.micInFormat = formatStrToHpae[streamDesc->micInStreamInfo_.format];
    } else {
        moduleInfo.micInFormat = "";
    }
}

static void UpdateMicRefModuleInfo(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool shouldOpenMicRef,
    AudioModuleInfo &moduleInfo)
{
    if (!shouldOpenMicRef || streamDesc == nullptr || streamDesc->micInStreamInfo_.channels <= CHANNEL_UNKNOW) {
        ClearMicRefModuleInfo(moduleInfo);
        return;
    }

    moduleInfo.openMicRef = "1";
    moduleInfo.micRefRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
    moduleInfo.micRefChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
    if (formatStrToHpae.count(streamDesc->micInStreamInfo_.format) > 0) {
        moduleInfo.micRefFormat = formatStrToHpae[streamDesc->micInStreamInfo_.format];
    } else {
        moduleInfo.micRefFormat = "";
    }
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
        !IsCamcorderDegradeInputStream(streamDesc) &&
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
    
    if (streamDesc->isNearLinkCollaborationEnable_ && (!(streamDesc->GetRoute() & AUDIO_OUTPUT_FLAG_MULTICHANNEL))) {
        streamDesc->isNearLinkCollaborationEnable_ = false;
    }

    AdjustFastRouteForUltraFastIfNeeded(streamDesc);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = streamDesc->IsPlayback() ?
        pipeManager_->GetAllOutputStreamDescs() : pipeManager_->GetAllInputStreamDescs();
    HandleFastStreamLimitOnCreate(streamDescs, streamDesc);

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

void AudioPipeSelector::AdjustFastRouteForUltraFastIfNeeded(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(!streamDesc->newDeviceDescs_.empty() && streamDesc->newDeviceDescs_.front() != nullptr,
        "new device desc is empty!");

    std::string adapterName = AudioPipeManager::GetPipeManager()->GetAdapterNameBySessionId(streamDesc->sessionId_);
    DeviceType deviceType = streamDesc->newDeviceDescs_.front()->getType();
    if (streamDesc->GetRoute() == AUDIO_OUTPUT_FLAG_FAST && pipeManager_->HasUltraFastPipe() &&
        adapterName == "primary" && !pipeManager_->IsUltraFastSupportedDevice(deviceType)) {
        streamDesc->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    }
}

void AudioPipeSelector::AdjustFastRouteForUltraFastIfNeeded(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    for (auto &streamDesc : streamDescs) {
        AdjustFastRouteForUltraFastIfNeeded(streamDesc);
    }
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

void AudioPipeSelector::HandleFastStreamLimitOnCreate(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
    const std::shared_ptr<AudioStreamDescriptor> &newStreamDesc)
{
    CHECK_AND_RETURN_LOG(newStreamDesc != nullptr, "New stream desc is nullptr");
    uint32_t routeFlag = newStreamDesc->GetRoute();
    CHECK_AND_RETURN(routeFlag == AUDIO_OUTPUT_FLAG_FAST || routeFlag == AUDIO_INPUT_FLAG_FAST ||
        routeFlag == AUDIO_INPUT_FLAG_ULTRA_FAST);

    int32_t ultraFastCount = 0;
    int32_t fastInputCount = 0;
    for (const auto &desc : streamDescs) {
        CHECK_AND_CONTINUE(desc != nullptr && desc->sessionId_ != newStreamDesc->sessionId_);
        ultraFastCount += desc->GetRoute() == AUDIO_INPUT_FLAG_ULTRA_FAST;
        fastInputCount += desc->GetRoute() == AUDIO_INPUT_FLAG_FAST || desc->GetRoute() == AUDIO_INPUT_FLAG_ULTRA_FAST;
    }
    if (routeFlag == AUDIO_INPUT_FLAG_ULTRA_FAST &&
        (ultraFastCount >= 2 || fastInputCount >= MAX_FAST_STREAM_COUNT)) {
        newStreamDesc->ResetToNormalRoute(false);
        return;
    }
    if (routeFlag == AUDIO_INPUT_FLAG_FAST && fastInputCount >= MAX_FAST_STREAM_COUNT) {
        newStreamDesc->ResetToNormalRoute(false);
        return;
    }

    int32_t fastStreamNum = static_cast<int32_t>(std::count_if(streamDescs.begin(), streamDescs.end(),
        [routeFlag](const std::shared_ptr<AudioStreamDescriptor> &streamDesc) {
            return streamDesc != nullptr && streamDesc->GetRoute() == routeFlag;
        }));
    CHECK_AND_RETURN(fastStreamNum >= MAX_FAST_STREAM_COUNT);

    AUDIO_INFO_LOG("reach fast limit, set new stream %{public}u to normal", newStreamDesc->sessionId_);
    newStreamDesc->ResetToNormalRoute(false);
}

void AudioPipeSelector::HandleFastStreamLimitAndNearLinkState(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    int32_t fastOutputNum = 0;
    int32_t fastInputNum = 0;
    int32_t ultraFastInputNum = 0;
    for (auto &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE(streamDesc != nullptr);
        if (streamDesc->isNearLinkCollaborationEnable_ &&
            (!(streamDesc->GetRoute() & AUDIO_OUTPUT_FLAG_MULTICHANNEL))) {
            streamDesc->isNearLinkCollaborationEnable_ = false;
        }
        if (streamDesc->GetRoute() == AUDIO_OUTPUT_FLAG_FAST && ++fastOutputNum > MAX_FAST_STREAM_COUNT) {
            AUDIO_INFO_LOG("reach fast limit, set %{public}u to normal", streamDesc->sessionId_);
            streamDesc->ResetToNormalRoute(false);
            continue;
        }
        if (streamDesc->GetRoute() == AUDIO_INPUT_FLAG_ULTRA_FAST) {
            if (ultraFastInputNum >= 2 || fastInputNum >= MAX_FAST_STREAM_COUNT) {
                streamDesc->ResetToNormalRoute(false);
            } else {
                ++ultraFastInputNum;
                ++fastInputNum;
            }
            continue;
        }
        if (streamDesc->GetRoute() == AUDIO_INPUT_FLAG_FAST) {
            if (fastInputNum >= MAX_FAST_STREAM_COUNT) {
                streamDesc->ResetToNormalRoute(false);
            } else {
                ++fastInputNum;
            }
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
        AdjustFastRouteForUltraFastIfNeeded(streamDescs);
        return;
    }

    for (size_t cmpStreamIdx = 1; cmpStreamIdx < streamDescs.size(); ++cmpStreamIdx) {
        // calculate concurrency in time order
        for (size_t curStreamDescIdx = 0; curStreamDescIdx < cmpStreamIdx; ++curStreamDescIdx) {
            ProcessConcurrency(streamDescs[curStreamDescIdx], streamDescs[cmpStreamIdx], streamsMoveToNormal);
        }
    }
    ProcessModemCommunicationConcurrency(streamDescs, streamsMoveToNormal);
    AdjustFastRouteForUltraFastIfNeeded(streamDescs);
    HandleFastStreamLimitAndNearLinkState(streamDescs);

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
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
#ifdef MULTI_BUS_ENABLE
    auto deviceType = streamDesc->GetMainNewDeviceType();
    if (AudioPolicyUtils::GetInstance().IsMultiBusDevice(deviceType)) {
        std::vector<std::string> busAddresses =
            AudioBusSelector::GetBusSelector().GetBusAddressesByStreamDesc(streamDesc);
        // Vehicle system scenario enter.
        HandleFindBusPipe(busAddresses, newPipeInfoList, streamDesc, matchPipeIter);
        return;
    }
#endif
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
        } else if (streamDesc->IsRemoteCastStream()) {
            newPipeIter = std::find_if(newPipeInfoList.begin(), newPipeInfoList.end(),
                [&](const std::shared_ptr<AudioPipeInfo> &newPipeInfo) {
                    return newPipeInfo->name_ == REMOTE_CAST_INNER_CAPTURER_SINK_NAME;
                });
        } else {
            HandleFindMatchPipe(newPipeInfoList, streamDesc, streamDescToOldPipeInfo, newPipeIter);
        }

        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        GetStreamPropInfoWithBusSelector(streamDesc, streamPropInfo);
        if (newPipeIter != newPipeInfoList.end()) {
            MatchSpecialRemotePipe(streamPropInfo, *newPipeIter, streamDesc);
            MatchCollaborativePipe(streamPropInfo, *newPipeIter, streamDesc);
            MatchUltraFastPipe(*newPipeIter, streamDesc);
            MatchFastArmusbPipe(*newPipeIter, streamDesc);
            MatchArmusbInputPipe(*newPipeIter, streamDesc);
            (*newPipeIter)->streamDescriptors_.push_back(streamDesc);
            (*newPipeIter)->streamDescMap_[streamDesc->sessionId_] = streamDesc;
            continue;
        }
        // if not find, need open
        HandlePipeNotExist(newPipeInfoList, streamDesc);
    }

    if (Util::IsEnhancedRoutingSupported()) {
        auto pipePairs = DetermineInputPipeBaseConfig(newPipeInfoList);
        DetermineInputPipeEcAndMicRef(pipePairs);
    }
}

AudioPipeSelector::InputPipeStreamPairList AudioPipeSelector::DetermineInputPipeBaseConfig(
    std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList)
{
    InputPipeStreamPairList pipePairs;
    auto sortedInputPipes = GetSortedInputPipes(newPipeInfoList);
    pipePairs.reserve(sortedInputPipes.size());
    for (auto &newPipe : sortedInputPipes) {
        auto highestStream = GetHighestPriorityStream(newPipe->streamDescriptors_);
        CHECK_AND_CONTINUE(highestStream != nullptr);
        UpdateInputPipeParamByHighestStream(highestStream, newPipe);
        pipePairs.emplace_back(newPipe, highestStream);
    }
    return pipePairs;
}

void AudioPipeSelector::DetermineInputPipeEcAndMicRef(const InputPipeStreamPairList &pipePairs)
{
    std::unordered_set<std::string> ecOccupiedAdapters;
    std::unordered_set<std::string> micRefOccupiedAdapters;
    for (const auto &pipePair : pipePairs) {
        auto newPipe = pipePair.first;
        auto highestStream = pipePair.second;
        CHECK_AND_CONTINUE(newPipe != nullptr && highestStream != nullptr);
        UpdateInputPipeEcAndMicRefByHighestStream(highestStream, newPipe, ecOccupiedAdapters,
            micRefOccupiedAdapters);
    }
}

int32_t AudioPipeSelector::GetPipePriority(const std::shared_ptr<AudioPipeInfo> &pipeInfo) const
{
    CHECK_AND_RETURN_RET(pipeInfo != nullptr, 0);
    auto highestStream = GetHighestPriorityStream(pipeInfo->streamDescriptors_);
    CHECK_AND_RETURN_RET(highestStream != nullptr, 0);
    return GetInputSourceTypePriority(highestStream->capturerInfo_.sourceType);
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeSelector::GetHighestPriorityStream(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams) const
{
    std::shared_ptr<AudioStreamDescriptor> highestStartedStream = nullptr;
    for (const auto &stream : streams) {
        CHECK_AND_CONTINUE(stream != nullptr);
        if (stream->audioMode_ == AUDIO_MODE_RECORD && stream->streamStatus_ == STREAM_STATUS_STARTED) {
            if (IsHigherPriorityInputStream(stream, highestStartedStream)) {
                highestStartedStream = stream;
            }
        }
    }
    if (highestStartedStream != nullptr) {
        return highestStartedStream;
    }

    std::shared_ptr<AudioStreamDescriptor> highestStream = nullptr;
    for (const auto &stream : streams) {
        CHECK_AND_CONTINUE(stream != nullptr);
        if (stream->audioMode_ == AUDIO_MODE_RECORD) {
            if (IsHigherPriorityInputStream(stream, highestStream)) {
                highestStream = stream;
            }
        }
    }
    return highestStream;
}

std::string AudioPipeSelector::DetermineTargetEcType(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const std::string &currentEcType) const
{
    if (streamDesc == nullptr || streamDesc->ecStreamInfo_.channels <= CHANNEL_UNKNOW) {
        return "";
    }
    if (IsEcEnabled(currentEcType)) {
        return currentEcType;
    }
    return std::to_string(EC_TYPE_SAME_ADAPTER);
}

std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeSelector::GetSortedInputPipes(
    std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList) const
{
    std::vector<std::shared_ptr<AudioPipeInfo>> sortedInputPipes;
    for (auto &newPipe : newPipeInfoList) {
        CHECK_AND_CONTINUE(newPipe != nullptr);
        CHECK_AND_CONTINUE(newPipe->pipeRole_ == PIPE_ROLE_INPUT);
        CHECK_AND_CONTINUE(!newPipe->streamDescriptors_.empty());
        sortedInputPipes.push_back(newPipe);
    }
    std::stable_sort(sortedInputPipes.begin(), sortedInputPipes.end(),
        [this](const std::shared_ptr<AudioPipeInfo> &left, const std::shared_ptr<AudioPipeInfo> &right) {
            return GetPipePriority(left) > GetPipePriority(right);
        });
    return sortedInputPipes;
}

std::shared_ptr<AudioPipeInfo> AudioPipeSelector::FindOldInputPipe(const std::shared_ptr<AudioPipeInfo> &newPipe,
    const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
    const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList) const
{
    CHECK_AND_RETURN_RET(newPipe != nullptr, nullptr);
    auto oldPipeIt = oldPipeMap.find(newPipe->id_);
    if (oldPipeIt != oldPipeMap.end() && oldPipeIt->second != nullptr) {
        return oldPipeIt->second;
    }
    for (const auto &candidate : oldPipeInfoList) {
        CHECK_AND_CONTINUE(candidate != nullptr);
        CHECK_AND_CONTINUE(candidate->pipeRole_ == PIPE_ROLE_INPUT);
        CHECK_AND_CONTINUE(candidate->routeFlag_ == newPipe->routeFlag_);
        CHECK_AND_CONTINUE(candidate->adapterName_ == newPipe->adapterName_);
        return candidate;
    }
    return nullptr;
}

void AudioPipeSelector::UpdateInputPipeParamByHighestStream(const std::shared_ptr<AudioStreamDescriptor> &highestStream,
    std::shared_ptr<AudioPipeInfo> &newPipe)
{
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    auto streamDesc = highestStream;
    pipeManager_->GetStreamPropInfo(streamDesc, streamPropInfo);
    if (IsPipeStreamPropValid(streamPropInfo)) {
        UpdatePipeInfoFromStreamProp(highestStream, streamPropInfo, *newPipe);
    }

    bool needMicIn = highestStream->micInStreamInfo_.channels > CHANNEL_UNKNOW;
    UpdateMicInModuleInfo(highestStream, needMicIn, newPipe->moduleInfo_);
}

void AudioPipeSelector::UpdateInputPipeEcAndMicRefByHighestStream(
    const std::shared_ptr<AudioStreamDescriptor> &highestStream, std::shared_ptr<AudioPipeInfo> &newPipe,
    std::unordered_set<std::string> &ecOccupiedAdapters, std::unordered_set<std::string> &micRefOccupiedAdapters)
{
    const std::string adapterName = !newPipe->moduleInfo_.adapterName.empty() ?
        newPipe->moduleInfo_.adapterName : newPipe->adapterName_;
    std::string targetEcType = DetermineTargetEcType(highestStream, newPipe->moduleInfo_.ecType);
    if (IsEcEnabled(targetEcType) && !adapterName.empty() && !ecOccupiedAdapters.emplace(adapterName).second) {
        targetEcType = "";
    }
    UpdateEcModuleInfo(highestStream, targetEcType, newPipe->moduleInfo_);

    bool needMicRef = highestStream->micInStreamInfo_.channels > CHANNEL_UNKNOW;
    if (needMicRef && !adapterName.empty() && !micRefOccupiedAdapters.emplace(adapterName).second) {
        needMicRef = false;
    }
    UpdateMicRefModuleInfo(highestStream, needMicRef, newPipe->moduleInfo_);
}

bool AudioPipeSelector::IsNeedReloadForInputPipe(const std::shared_ptr<AudioPipeInfo> &newPipe,
    const std::shared_ptr<AudioPipeInfo> &oldPipe, const std::shared_ptr<AudioStreamDescriptor> &highestStream,
    const std::shared_ptr<AudioStreamDescriptor> &oldHighestStream) const
{
    CHECK_AND_RETURN_RET_LOG(!IsPipeNotSupportReload(newPipe), false, "remote pipe not support reload");
    if (newPipe->audioStreamInfo_.samplingRate != oldPipe->audioStreamInfo_.samplingRate) {
        return true;
    }
    if (newPipe->audioStreamInfo_.channels != oldPipe->audioStreamInfo_.channels) {
        return true;
    }
    if (NormalizeEcType(newPipe->moduleInfo_.ecType) != NormalizeEcType(oldPipe->moduleInfo_.ecType)) {
        return true;
    }
    if (IsMicRefEnabled(newPipe->moduleInfo_.openMicRef) != IsMicRefEnabled(oldPipe->moduleInfo_.openMicRef)) {
        return true;
    }
    if (IsHighestPriorityStreamChanged(oldHighestStream, highestStream)) {
        return true;
    }
    return false;
}

CapturerPipeExecOrder AudioPipeSelector::GetCapturerPipeExecOrder(const std::shared_ptr<AudioPipeInfo> &newPipe,
    const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
    const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList) const
{
    CHECK_AND_RETURN_RET(newPipe != nullptr, CapturerPipeExecOrder::NORMAL);
    CHECK_AND_RETURN_RET(newPipe->pipeRole_ == PIPE_ROLE_INPUT, CapturerPipeExecOrder::NORMAL);
    auto oldPipe = FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
    bool oldHasEc = oldPipe != nullptr && IsEcEnabled(oldPipe->moduleInfo_.ecType);
    bool oldHasMicRef = oldPipe != nullptr && IsMicRefEnabled(oldPipe->moduleInfo_.openMicRef);
    bool oldHasMicIn = oldPipe != nullptr && IsMicInEnabled(oldPipe->moduleInfo_);
    bool newHasEc = IsEcEnabled(newPipe->moduleInfo_.ecType);
    bool newHasMicRef = IsMicRefEnabled(newPipe->moduleInfo_.openMicRef);
    bool newHasMicIn = IsMicInEnabled(newPipe->moduleInfo_);

    bool closeBeforeOpen = (oldHasEc && !newHasEc) || (oldHasMicRef && !newHasMicRef) || (oldHasMicIn && !newHasMicIn);
    bool openAfterClose = (!oldHasEc && newHasEc) || (!oldHasMicRef && newHasMicRef) || (!oldHasMicIn && newHasMicIn);
    if (closeBeforeOpen) {
        return CapturerPipeExecOrder::CLOSE_BEFORE_OPEN;
    }
    if (openAfterClose) {
        return CapturerPipeExecOrder::OPEN_AFTER_CLOSE;
    }
    return CapturerPipeExecOrder::NORMAL;
}

void AudioPipeSelector::SortCapturerPipesByExecOrder(const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList,
    std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList) const
{
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    bool hasInputPipe = false;
    for (const auto &oldPipe : oldPipeInfoList) {
        CHECK_AND_CONTINUE(oldPipe != nullptr);
        CHECK_AND_CONTINUE(oldPipe->pipeRole_ == PIPE_ROLE_INPUT);
        oldPipeMap[oldPipe->id_] = oldPipe;
        hasInputPipe = true;
    }
    CHECK_AND_RETURN(hasInputPipe);

    std::stable_sort(newPipeInfoList.begin(), newPipeInfoList.end(),
        [this, &oldPipeMap, &oldPipeInfoList](const std::shared_ptr<AudioPipeInfo> &left,
            const std::shared_ptr<AudioPipeInfo> &right) {
            return GetCapturerPipeExecOrder(left, oldPipeMap, oldPipeInfoList) <
                GetCapturerPipeExecOrder(right, oldPipeMap, oldPipeInfoList);
        });
}

void AudioPipeSelector::HandleEmptyInputPipeReload(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
    const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList)
{
    for (auto &newPipe : newPipeInfoList) {
        CHECK_AND_CONTINUE(newPipe != nullptr);
        CHECK_AND_CONTINUE(newPipe->pipeRole_ == PIPE_ROLE_INPUT);
        CHECK_AND_CONTINUE(newPipe->streamDescriptors_.empty());
        auto oldPipe = FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
        CHECK_AND_CONTINUE(oldPipe != nullptr);

        bool oldHasEc = IsEcEnabled(oldPipe->moduleInfo_.ecType);
        bool oldHasMicRef = IsMicRefEnabled(oldPipe->moduleInfo_.openMicRef);
        bool oldHasMicIn = IsMicInEnabled(oldPipe->moduleInfo_);
        CHECK_AND_CONTINUE(oldHasEc || oldHasMicRef || oldHasMicIn);

        ClearEcModuleInfo(newPipe->moduleInfo_);
        ClearMicInModuleInfo(newPipe->moduleInfo_);
        ClearMicRefModuleInfo(newPipe->moduleInfo_);
        if (newPipe->pipeAction_ != PIPE_ACTION_NEW) {
            newPipe->pipeAction_ = IsPipeNotSupportReload(newPipe) ? newPipe->pipeAction_ : PIPE_ACTION_RELOAD;
        }
    }
}

void AudioPipeSelector::LogStreamsInfo(const std::vector<std::pair<uint32_t, uint32_t>> &sessionList)
{
    CHECK_AND_RETURN(!sessionList.empty());
    std::stringstream logstream;
    for (const auto &sessionPair : sessionList) {
        logstream << sessionPair.first << ":" << sessionPair.second << ", ";
    }
    std::string result = logstream.str();
    AUDIO_INFO_LOG("    |-[PipeFetchInfo] Id %{public}s", result.c_str());
}

void AudioPipeSelector::UpdatePipeAndStreamAction(std::shared_ptr<AudioPipeInfo> &newPipeInfo,
    const std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> &streamDescToOldPipeInfo)
{
    CHECK_AND_RETURN(newPipeInfo != nullptr);
    newPipeInfo->pipeAction_ = (newPipeInfo->pipeAction_ != PIPE_ACTION_NEW &&
        newPipeInfo->pipeAction_ != PIPE_ACTION_RELOAD) ? PIPE_ACTION_UPDATE : newPipeInfo->pipeAction_;
    AUDIO_INFO_LOG("[PipeFetchInfo] Id %{public}u, Name %{public}s, PipeAction: %{public}d",
        newPipeInfo->id_, newPipeInfo->name_.c_str(), newPipeInfo->pipeAction_);

    std::vector<std::pair<uint32_t, uint32_t>> sessionIdForLog {};
    for (auto &streamDesc : newPipeInfo->streamDescriptors_) {
        if (streamDescToOldPipeInfo.find(streamDesc->sessionId_) == streamDescToOldPipeInfo.end()) {
            HILOG_COMM_WARN("[PipeFetchInfo] cannot find %{public}d in OldPipeList!", streamDesc->sessionId_);
            continue;
        }
        streamDesc->SetAction(JudgeStreamAction(newPipeInfo, streamDescToOldPipeInfo.at(streamDesc->GetSessionId())));
        streamDesc->SetOldRoute(streamDescToOldPipeInfo.at(streamDesc->GetSessionId())->GetRoute());
        if (streamDescToOldPipeInfo.at(streamDesc->GetSessionId())->GetRoute() == newPipeInfo->GetRoute()) {
            sessionIdForLog.push_back({streamDesc->GetSessionId(), streamDesc->GetStatus()});
            continue;
        }
        AUDIO_INFO_LOG("    |-[PipeFetchInfo] Id %{public}u:%{public}u, RouteFlag %{public}d -> %{public}d, "
            "sAction %{public}d", streamDesc->GetSessionId(), streamDesc->GetStatus(),
            streamDescToOldPipeInfo.at(streamDesc->GetSessionId())->GetRoute(),
            newPipeInfo->GetRoute(), streamDesc->GetAction());
    }
    LogStreamsInfo(sessionIdForLog);
    if (newPipeInfo->streamDescriptors_.empty()) {
        AUDIO_INFO_LOG("    |-[PipeFetchInfo] Empty");
    }
}

std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> AudioPipeSelector::BuildOldPipeMap(
    const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList) const
{
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    for (const auto &oldPipe : oldPipeInfoList) {
        CHECK_AND_CONTINUE(oldPipe != nullptr);
        oldPipeMap[oldPipe->id_] = oldPipe;
    }
    return oldPipeMap;
}

void AudioPipeSelector::UpdateInputPipeReloadAction(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    const std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> &oldPipeMap,
    const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList)
{
    HandleEmptyInputPipeReload(newPipeInfoList, oldPipeMap, oldPipeInfoList);

    auto sortedInputPipes = GetSortedInputPipes(newPipeInfoList);
    for (auto &newPipe : sortedInputPipes) {
        auto oldPipe = FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
        CHECK_AND_CONTINUE(oldPipe != nullptr);
        auto highestStream = GetHighestPriorityStream(newPipe->streamDescriptors_);
        CHECK_AND_CONTINUE(highestStream != nullptr);
        auto oldHighestStream = GetHighestPriorityStream(oldPipe->streamDescriptors_);
        if (IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream) &&
            newPipe->pipeAction_ != PIPE_ACTION_NEW) {
            newPipe->pipeAction_ = PIPE_ACTION_RELOAD;
        }
    }
}

// based on old--new pipeinfo to judge streamAction and pipeAction
void AudioPipeSelector::DecidePipesAndStreamAction(std::vector<std::shared_ptr<AudioPipeInfo>> &newPipeInfoList,
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo,
    const std::vector<std::shared_ptr<AudioPipeInfo>> &oldPipeInfoList)
{
    for (auto &newPipeInfo : newPipeInfoList) {
        UpdatePipeAndStreamAction(newPipeInfo, streamDescToOldPipeInfo);
    }

    if (Util::IsEnhancedRoutingSupported()) {
        auto oldPipeMap = BuildOldPipeMap(oldPipeInfoList);
        UpdateInputPipeReloadAction(newPipeInfoList, oldPipeMap, oldPipeInfoList);
        SortCapturerPipesByExecOrder(oldPipeInfoList, newPipeInfoList);
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
    DecidePipesAndStreamAction(newPipeInfoList, streamDescToPipeInfo, oldPipeInfoList);

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
    if (flag & AUDIO_INPUT_FLAG_ULTRA_FAST) {
        return PIPE_TYPE_IN_ULTRA_FAST;
    } else if (flag & AUDIO_INPUT_FLAG_FAST) {
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
    } else if (flag & AUDIO_INPUT_FLAG_VOICE_MESSAGE) {
        return PIPE_TYPE_IN_NORMAL_VOICE_MESSAGE;
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
        } else if (flag & AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT) {
            return PIPE_TYPE_OUT_VOICE_CALL_ASSISTANT;
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

void AudioPipeSelector::AdjustConcedeActionByStatus(std::shared_ptr<AudioStreamDescriptor> existingStream,
    std::shared_ptr<AudioStreamDescriptor> incomingStream, ConcurrencyAction &action)
{
    CHECK_AND_RETURN(action == CONCEDE_INCOMING || action == CONCEDE_EXISTING);
    CHECK_AND_RETURN_LOG(existingStream != nullptr && incomingStream != nullptr,
        "existingStream or incomingStream is nullptr");
    /* The existingStream is a low-latency recording that is not in the running state,
     * and the incomingStream is a low-latency VoIP recording that is not in the running state.
     * The low-latency VoIP recording needs to be CONCEDE to avoid the failure of moving the
     * existingStream because the built-in microphone channel does not exist when the existingStream is CONCEDE.
    */
    CHECK_AND_RETURN_LOG(existingStream->IsRunning() || incomingStream->IsRunning() ||
        ((existingStream->GetRoute() == AUDIO_INPUT_FLAG_FAST) &&
        (incomingStream->GetRoute() == (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP))),
        "both streams are not running");

    std::shared_ptr<AudioStreamDescriptor> targetStream =
        (action == CONCEDE_INCOMING) ? existingStream : incomingStream;
    CHECK_AND_RETURN_LOG(!targetStream->IsLoopbackStream(),
        "target stream %{public}u is loopback, skip adjust", targetStream->GetSessionId());
    CHECK_AND_RETURN(IsUpdateActionForDeviceLevelHdPlay(existingStream, incomingStream));
    CHECK_AND_RETURN(targetStream->GetRoute() != (AUDIO_OUTPUT_FLAG_VOIP | AUDIO_OUTPUT_FLAG_DIRECT));

    if (!targetStream->IsRunning()) {
        action = (action == CONCEDE_EXISTING) ? CONCEDE_INCOMING : CONCEDE_EXISTING;
        AUDIO_INFO_LOG("sessionId_: %{public}u, streamStatus_: %{public}u, action: %{public}d",
            targetStream->sessionId_, targetStream->streamStatus_, action);
    }
}

void AudioPipeSelector::HandleSpecialStreamLimitOnMultiDevice(
    const std::shared_ptr<AudioStreamDescriptor> &existingStream,
    const std::shared_ptr<AudioStreamDescriptor> &cmpStream)
{
    CHECK_AND_RETURN(Util::IsEnhancedRoutingSupported());
    CHECK_AND_RETURN_LOG(cmpStream != nullptr && existingStream != nullptr, "stream is nullptr");
    CHECK_AND_RETURN(cmpStream->IsEnhancedOutputRoute() && existingStream->GetRoute() == cmpStream->GetRoute());

    DeviceType existingDeviceType = existingStream->GetMainNewDeviceType();
    DeviceType cmpDeviceType = cmpStream->GetMainNewDeviceType();
    CHECK_AND_RETURN(existingDeviceType != cmpDeviceType &&
        AudioCoreConfigManager::GetInstance().IsSupportedDeviceTypeByAdapterType(
            AudioAdapterType::TYPE_PRIMARY, existingDeviceType) &&
        AudioCoreConfigManager::GetInstance().IsSupportedDeviceTypeByAdapterType(
            AudioAdapterType::TYPE_PRIMARY, cmpDeviceType));
    AUDIO_INFO_LOG("stream %{public}u routeFlag 0x%{public}x fallback to normal, existing %{public}u "
        "deviceType %{public}d, cmp deviceType %{public}d",
        cmpStream->GetSessionId(), cmpStream->GetRoute(), existingStream->GetSessionId(),
        static_cast<int32_t>(existingDeviceType), static_cast<int32_t>(cmpDeviceType));
    cmpStream->ResetToNormalRoute(false);
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
    HandleSpecialStreamLimitOnMultiDevice(existingStream, incomingStream);
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
    // The xml pipe flag (e.g. AUDIO_OUTPUT_FLAG_3DA_DIRECT of bus10_3da_out) must be visible on
    // the stream, otherwise the 3DA direct path is never selected for clients that do not send
    // the flag themselves. OR only the 3DA bit into routeFlag_: supportFlags_ is the pipe
    // capability set and must neither replace flags already computed by the route decision
    // (ApplyRouteSelectDecision) nor inject flags the stream never requested.
    std::shared_ptr<AdapterPipeInfo> matchedPipeInfo = info->pipeInfo_.lock();
    if (matchedPipeInfo != nullptr && (matchedPipeInfo->supportFlags_ & AUDIO_OUTPUT_FLAG_3DA_DIRECT)) {
        desc->routeFlag_ = static_cast<AudioFlag>(desc->routeFlag_ | AUDIO_OUTPUT_FLAG_3DA_DIRECT);
    }
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

static bool IsMultiStreamCaptureInfo(const std::string &pipeName)
{
    return pipeName == "primary_typec_in" || pipeName == "primary_wired_in" ||
        pipeName == "primary_bt_nearlink_in";
}

static void FillSpecialPipeInfoExtOther(AudioPipeInfo &info, std::shared_ptr<AdapterPipeInfo> pipeInfoPtr,
    std::shared_ptr<AudioStreamDescriptor> streamDesc, std::shared_ptr<PipeStreamPropInfo> streamPropInfo)
{
    if (pipeInfoPtr->name_ == "fast_usb_arm_output") {
        info.moduleInfo_.className = info.GetUltraFastFlag() ? "usb_arm_ultra_fast" : "usb_arm_fast";
        info.moduleInfo_.fileName = info.GetUltraFastFlag() ? "usb_arm_ultra_fast_dump_file" : "usb_arm_fast_dump_file";
        info.moduleInfo_.name = info.GetUltraFastFlag() ? "UltraFastUsbArmRendererSink" : "FastUsbArmRendererSink";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "tts_out") {
        info.moduleInfo_.className = "primary_voice_call_assistant";
        info.moduleInfo_.fileName = "tts_dump_file";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.name = "tts";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "dp_fast_output") {
        info.moduleInfo_.className = "dp_fast";
        info.moduleInfo_.fileName = "dp_fast_dump_file";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.name = "FastDpRendererSink";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "usb_ultra_fast_input") {
        info.moduleInfo_.className = "usb_arm_ultra_fast";
        info.moduleInfo_.fileName = "usb_ultra_fast_source_dump_file";
        info.moduleInfo_.name = "UltraFastUsbArmCapturerSource";
        info.moduleInfo_.lib = "libmodule-hdi-source.z.so";
        info.moduleInfo_.role = "source";
    } else if (pipeInfoPtr->name_ == "fast_usb_arm_input") {
        info.moduleInfo_.className = "primary_mmap";
        info.moduleInfo_.fileName = "usb_arm_fast_source_dump_file";
        info.moduleInfo_.name = "FastUsbArmCapturerSource";
        info.moduleInfo_.lib = "libmodule-hdi-source.z.so";
        info.moduleInfo_.role = "source";
    }
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
    } else if (pipeInfoPtr->name_ == "primary_ultra_fast_input") {
        info.moduleInfo_.className = "primary_ultra_fast";
        info.moduleInfo_.fileName = "primary_ultra_fast_source_dump_file";
        info.moduleInfo_.name = "UltraFastCapturerSource";
        info.moduleInfo_.lib = "libmodule-hdi-source.z.so";
        info.moduleInfo_.role = "source";
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
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.name = "direct";
        info.moduleInfo_.role = "sink";
    } else if (pipeInfoPtr->name_ == "voip_direct_output") {
        info.moduleInfo_.className = "primary_direct_voip";
        info.moduleInfo_.fileName = "direct_dump_file";
        info.moduleInfo_.lib = "libmodule-hdi-sink.z.so";
        info.moduleInfo_.name = "voip";
        info.moduleInfo_.role = "sink";
    } else {
        FillSpecialPipeInfoExtOther(info, pipeInfoPtr, streamDesc, streamPropInfo);
    }

    if (IsMultiStreamPrimary(pipeInfoPtr->name_)) {
        info.moduleInfo_.className = PRIMARY_EXTRA_CLASS;
        info.moduleInfo_.networkId = info.moduleInfo_.name;
    } else if (IsMultiStreamCaptureInfo(pipeInfoPtr->name_)) {
        info.moduleInfo_.className = "primary_extra_capturer";
        info.moduleInfo_.networkId = info.moduleInfo_.name;
    }
}

static void FillRemoteMultichannelPipeInfo(AudioPipeInfo &info, std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    info.moduleInfo_.className = "remote_multichannel";
    info.moduleInfo_.fileName = "remote_multichannel_dump_file";
    info.moduleInfo_.fixedLatency = "1";
    CHECK_AND_RETURN(streamDesc != nullptr);
    auto deviceDesc = streamDesc->GetMainNewDeviceDesc();
    CHECK_AND_RETURN(deviceDesc != nullptr);
    info.moduleInfo_.name =
        AudioPolicyUtils::GetInstance().GetRemoteModuleName(deviceDesc->networkId_,
        AudioPolicyUtils::GetInstance().GetDeviceRole(deviceDesc->deviceType_)) + "_multichannel";
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
    } else if (pipeInfoPtr->name_ == "multichannel_distributed_output") {
        FillRemoteMultichannelPipeInfo(info, streamDesc);
    } else {
        FillSpecialPipeInfoExt(info, pipeInfoPtr, streamDesc, streamPropInfo);
    }
}

void AudioPipeSelector::UpdateModuleInfoWithDevice(const std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(deviceDesc, "streamDesc is nullptr");
    moduleInfo.deviceType = std::to_string(deviceDesc->deviceType_);
    moduleInfo.networkId = deviceDesc->networkId_;
    moduleInfo.macAddress = deviceDesc->macAddress_;
    moduleInfo.hdPlayBackMode = std::to_string(static_cast<int32_t>(deviceDesc->hdPlaybackMode_));
    if (deviceDesc->getType() == DEVICE_TYPE_USB_ARM_HEADSET) {
        CHECK_AND_RETURN_LOG(!deviceDesc->GetAudioStreamInfo().empty(), "audio streamInfo empty");
        DeviceStreamInfo deviceAudioStreamInfo = deviceDesc->GetAudioStreamInfo().back();

        CHECK_AND_RETURN_LOG(!deviceAudioStreamInfo.samplingRate.empty(), "samplingRate set empty");
        moduleInfo.rate = to_string(*(deviceAudioStreamInfo.samplingRate.begin()));

        auto it = AudioDefinitionPolicyUtils::enumToFormatStr.find(deviceAudioStreamInfo.format);
        CHECK_AND_RETURN_LOG(it != AudioDefinitionPolicyUtils::enumToFormatStr.end(),
            "Not found %{public}u in enumToFormatStr", static_cast<uint32_t>(deviceAudioStreamInfo.format));
        moduleInfo.format = it->second;

        AudioChannelLayout halChannelLayout = deviceAudioStreamInfo.GetMainChannelLayout();
        std::string halChannels = std::to_string(ConvertLayoutToAudioChannel(halChannelLayout));
        if (halChannelLayout != CH_LAYOUT_UNKNOWN) {
            moduleInfo.channels = halChannels;
            moduleInfo.channelLayout = std::to_string(halChannelLayout);
        }

        uint32_t buffCalcMs = 20;
        uint32_t rateValue = 0;
        uint32_t channelValue = 0;
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.rate, rateValue),
            "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.channels, channelValue),
            "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());

        uint32_t bufferSize = rateValue * channelValue * AudioPolicyUtils::GetInstance().PcmFormatToBytes(
            AudioDefinitionPolicyUtils::formatStrToEnum[moduleInfo.format]) * buffCalcMs / MS_PER_S;
        moduleInfo.bufferSize = std::to_string(bufferSize);
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
    info.moduleInfo_.channels = std::to_string(ConvertLayoutToAudioChannel(streamPropInfo->channelLayout_));
    info.moduleInfo_.channelLayout = std::to_string(streamPropInfo->channelLayout_);
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
        UpdateModuleInfoWithDevice(streamDesc->newDeviceDescs_[0], info.moduleInfo_);
    }
    if (streamDesc->routeFlag_ == AUDIO_INPUT_FLAG_ULTRA_FAST) {
        // USB device descriptors can contain several formats. Keep the selected XML 2.5 ms profile.
        info.moduleInfo_.rate = std::to_string(streamPropInfo->sampleRate_);
        info.moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[streamPropInfo->format_];
        info.moduleInfo_.channels = std::to_string(streamPropInfo->channels_);
        info.moduleInfo_.channelLayout = std::to_string(streamPropInfo->channelLayout_);
        info.moduleInfo_.bufferSize = std::to_string(streamPropInfo->bufferSize_);
    }

    if (streamDesc->IsUltraFastRequested() &&
        !AudioPipeManager::GetPipeManager()->HasRunningStream(streamDesc->sessionId_)) {
        info.SetUltraFastFlag(true);
        streamDesc->SetUltraFastImplemented(true);
    }

    // engine uses networkId to distinguish different renders,
    // so we need to set networkId after UpdateModuleInfoWithDevice;
    FillSpecialPipeInfo(info, pipeInfoPtr, streamDesc, streamPropInfo);

    info.streamDescriptors_.push_back(streamDesc);
    info.streamDescMap_[streamDesc->sessionId_] = streamDesc;
    info.routeFlag_ = streamDesc->routeFlag_;
    info.adapterName_ = adapterInfoPtr->adapterName;
    info.pipeRole_ = pipeInfoPtr->role_;
    info.name_ = pipeInfoPtr->name_;
    info.isCollaborationEnable_ = streamPropInfo->isCollaborationEnable_;
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
    CHECK_AND_RETURN(streamDesc->NeedForcedNormal());
    AUDIO_INFO_LOG("Session %{public}u has been conceded to FORCED_NORMAL", streamDesc->sessionId_);
    streamDesc->SetOriginalFlagForcedNormal();
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
        auto deviceType = streamDesc->GetMainNewDeviceType();
        if (!AudioPolicyUtils::GetInstance().IsMultiBusDevice(deviceType)) {
            CHECK_AND_CONTINUE(IsPipeMatch(streamDesc, pipeInfo, adapterInfoPtr->adapterName, pipeInfoPtr->name_));
        }
        CHECK_AND_CONTINUE(pipeInfo->adapterName_ == adapterInfoPtr->adapterName &&
                               pipeInfo->routeFlag_ == streamDesc->routeFlag_ &&
                               pipeInfo->audioStreamInfo_.channels == streamPropInfo->channels_);
        if (streamDesc->IsRecording() && !pipeInfo->moduleInfo_.busAddress.empty()) {
            CHECK_AND_CONTINUE(pipeInfoPtr->paProp_.busAddress_ == pipeInfo->moduleInfo_.busAddress);
        }
#else
        CHECK_AND_CONTINUE(IsPipeMatch(streamDesc, pipeInfo, adapterInfoPtr->adapterName, pipeInfoPtr->name_));
#endif

        MatchSpecialRemotePipe(streamPropInfo, pipeInfo, streamDesc);
        MatchCollaborativePipe(streamPropInfo, pipeInfo, streamDesc);

        MatchUltraFastPipe(pipeInfo, streamDesc);
        MatchFastArmusbPipe(pipeInfo, streamDesc);
        MatchArmusbInputPipe(pipeInfo, streamDesc);

        pipeManager_->AddStreamToPipe(pipeInfo, streamDesc, PIPE_ACTION_UPDATE);
        HILOG_COMM_INFO("[PipeFetchInfo] use existing Pipe %{public}s for stream %{public}u, pipeAction: %{public}d",
            pipeInfo->ToString().c_str(), streamDesc->sessionId_, pipeInfo->pipeAction_);
        return true;
    }
    return false;
}

bool AudioPipeSelector::IsSpecialRemotePipe(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "pipeInfo is nullptr");
    CHECK_AND_RETURN_RET_LOG(pipeInfo->adapterName_ == "remote", false, "not remote pipe");

    return (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER) ||
        (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_MULTICHANNEL);
}

void AudioPipeSelector::MatchSpecialRemotePipe(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
    std::shared_ptr<AudioPipeInfo> pipeInfo, const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    bool matchState = IsPipeFormatMatch(streamPropInfo, pipeInfo);
    // special remote pipe need to reload if format is mismatching
    CHECK_AND_RETURN(!matchState && IsSpecialRemotePipe(pipeInfo));

    AUDIO_INFO_LOG("mismatching special remote pipe need to recreate to match stream format");
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

void AudioPipeSelector::MatchFastArmusbPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN(streamDesc && !streamDesc->newDeviceDescs_.empty() &&
        pipeInfo && pipeInfo->name_ == "fast_usb_arm_output");
    auto deviceDesc = streamDesc->GetMainNewDeviceDesc();
    CHECK_AND_RETURN(deviceDesc && deviceDesc->macAddress_ != pipeInfo->moduleInfo_.macAddress);

    AUDIO_INFO_LOG("FastArmUsb pipe device changed, update moduleInfo with new device info");
    UpdateModuleInfoWithDevice(deviceDesc, pipeInfo->moduleInfo_);
    pipeInfo->InitAudioStreamInfo();
    pipeInfo->pipeAction_ = PIPE_ACTION_RELOAD;
}

void AudioPipeSelector::MatchArmusbInputPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN(streamDesc && !streamDesc->newDeviceDescs_.empty() &&
        pipeInfo && pipeInfo->name_ == USB_MIC_PIPENAME);
    auto deviceDesc = streamDesc->GetMainNewDeviceDesc();
    CHECK_AND_RETURN(!pipeInfo->IsSameUsbDevice(deviceDesc));

    AUDIO_INFO_LOG("ArmUsb input pipe device changed, update moduleInfo with new device info");
    UpdateModuleInfoWithDevice(deviceDesc, pipeInfo->moduleInfo_);
    pipeInfo->InitAudioStreamInfo();
    pipeInfo->pipeAction_ = PIPE_ACTION_RELOAD;
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
    info.isCollaborationEnable_ = streamPropInfo->isCollaborationEnable_;
    if (!streamDesc->newDeviceDescs_.empty()) {
        UpdateModuleInfoWithDevice(streamDesc->newDeviceDescs_[0], info.moduleInfo_);
    }

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
        (pipeInfo->IsSameName(pipeName) || pipeInfo->IsSameAdapter("remote")), false);

    // Use networkId to distinguish multiple remote devices that may exist
    auto deviceDesc = streamDesc->GetMainNewDeviceDesc();
#ifdef MULTI_BUS_ENABLE
    auto deviceType = streamDesc->GetMainNewDeviceType();
    if (!AudioPolicyUtils::GetInstance().IsMultiBusDevice(deviceType)) {
        CHECK_AND_RETURN_RET(deviceDesc != nullptr && deviceDesc->networkId_ != LOCAL_NETWORK_ID, true);
    }
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

bool AudioPipeSelector::IsPipeNotSupportReload(const std::shared_ptr<AudioPipeInfo> &pipe) const
{
    CHECK_AND_RETURN_RET(pipe != nullptr && pipe->IsSameAdapter(ADAPTER_TYPE_REMOTE) &&
        pipe->pipeRole_ == PIPE_ROLE_INPUT && !pipe->streamDescriptors_.empty(), false);
    auto mainDevice = pipe->streamDescriptors_.front()->GetMainNewDeviceDesc();
    return mainDevice != nullptr && mainDevice->IsRemoteDevice() && !mainDevice->isSupportReload_;
}

void AudioPipeSelector::MatchCollaborativePipe(const std::shared_ptr<PipeStreamPropInfo> &streamPropInfo,
    std::shared_ptr<AudioPipeInfo> pipeInfo, const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamPropInfo->isCollaborationEnable_ == pipeInfo->isCollaborationEnable_) {
        return;
    }

    AUDIO_INFO_LOG("MatchCollaborativePipe isCollaborationEnable_ old:%{public}d, new:%{public}d",
        pipeInfo->isCollaborationEnable_, streamPropInfo->isCollaborationEnable_);

    UpdatePipeInfoFromStreamProp(streamDesc, streamPropInfo, *pipeInfo);

    pipeInfo->pipeAction_ = PIPE_ACTION_RELOAD;
}
} // namespace AudioStandard
} // namespace OHOS
