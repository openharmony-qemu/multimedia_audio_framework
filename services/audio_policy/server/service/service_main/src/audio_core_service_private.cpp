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
#define LOG_TAG "AudioCoreServicePrivate"
#endif

#include "audio_core_service.h"

#include <algorithm>
#include <variant>

#include "system_ability.h"
#include "app_mgr_client.h"
#include "hisysevent.h"
#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_utils.h"
#include "iservice_registry.h"
#include "hdi_adapter_info.h"
#include "audio_usb_manager.h"
#include "audio_spatialization_service.h"
#include "audio_collaborative_service.h"
#include "audio_device_factory.h"
#include "audio_stream_id_allocator.h"
#include "ipc_skeleton.h"
#include "audio_volume.h"
#include "audio_bundle_manager.h"
#include "sco_audio_scene_manager.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t MEDIA_SERVICE_UID = 1013;
const int32_t DATA_LINK_CONNECTED = 11;
const uint32_t FIRST_SESSIONID = 100000;
const uid_t MCU_UID = 7500;
const uid_t TV_SERVICE_UID = 7501;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
constexpr int32_t REMOTE_USER_TERMINATED = 200;
constexpr int32_t DUAL_CONNECTION_FAILURE = 201;
static const int VOLUME_LEVEL_DEFAULT_SIZE = 3;
static const int32_t REFETCH_DEVICE = 4;
const int32_t DEFAULT_UID = -1;

// Supported audio parameters for fast audio stream
const std::vector<AudioSamplingRate> AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES {
    SAMPLE_RATE_48000,
};

const std::vector<AudioChannel> AUDIO_FAST_STREAM_SUPPORTED_CHANNELS {
    MONO,
    STEREO,
};

const std::vector<AudioSampleFormat> AUDIO_FAST_STREAM_SUPPORTED_FORMATS {
    SAMPLE_S16LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE
};

static const int64_t WAIT_MODEM_CALL_SET_VOLUME_TIME_US = 120000; // 120ms
static const int64_t RING_DUAL_END_DELAY_US = 100000; // 100ms
static const int64_t OLD_DEVICE_UNAVALIABLE_MUTE_MS = 1000000; // 1s
static const int64_t NEW_DEVICE_AVALIABLE_MUTE_MS = 400000; // 400ms
static const int64_t NEW_DEVICE_AVALIABLE_OFFLOAD_MUTE_MS = 1000000; // 1s
static const int64_t NEW_DEVICE_REMOTE_CAST_AVALIABLE_MUTE_MS = 300000; // 300ms
static const int64_t SELECT_DEVICE_MUTE_MS = 200000; // 200ms
static const int64_t SELECT_OFFLOAD_DEVICE_MUTE_MS = 400000; // 400ms
static const int64_t OLD_DEVICE_UNAVALIABLE_EXT_MUTE_MS = 300000; // 300ms
static const int64_t DISTRIBUTED_DEVICE_UNAVALIABLE_MUTE_MS = 1500000;  // 1.5s
static const uint32_t VOICE_CALL_DEVICE_SWITCH_MUTE_US = 100000; // 100ms
static const uint32_t MUTE_TO_ROUTE_UPDATE_TIMEOUT_MS = 1000; // 1s

static const uint32_t BASE_DEVICE_SWITCH_SLEEP_US = 80000; // 80ms
static const uint32_t OLD_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US = 150000; // 150ms
static const uint32_t DISTRIBUTED_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US = 350000; // 350ms
static const uint32_t HEADSET_TO_SPK_EP_EXTRA_SLEEP_US = 120000; // 120ms
static const uint32_t PRIMARY_OUTPUT_BUFFER_DRAIN_DELAY_US = 120000; // 120ms
static const uint32_t VOICE_CALL_DEVICE_SET_DELAY_US = 120000; // 120ms
static const uint32_t OLD_DEVICE_UNAVALIABLE_SUSPEND_MS = 1000; // 1s
static const int64_t COLLABORATIVE_STATE_CHANGE_MUTE_SINK_PORT_US = 200000; // 200ms

static const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;
static const int32_t WAIT_OFFLOAD_CLOSE_TIME_SEC = 10;
static const int32_t WAIT_MULTICHANNL_CLOSE_TIME_SEC = 5;
static const int32_t WAIT_PRIMARY_EXTRA_CLOSE_TIME_SEC = 5;
static const int32_t VOIP_FAST_PIPE_CLOSE_DELAY_TIME_MS = 200;
static const int32_t VOIP_REC_FAST_PIPE_CLOSE_DELAY_TIME_MS = 60;
static const int32_t DEFAULT_FAST_PIPE_CLOSE_DELAY_TIME_MS = 3000;
static const int32_t A2DP_MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS = 200;
static const int32_t ARMUSB_MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS = 0;
static const int32_t MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS = 0;
static const int32_t ARM_PIPE_RELOAD_DELAY_MS = 5000;
static const std::unordered_map<std::string, int32_t> FAST_PIPE_DELAY_RELEASE_TIME_MS = {
    {"voip_fast_output", VOIP_FAST_PIPE_CLOSE_DELAY_TIME_MS},
    {"voip_fast_input", VOIP_REC_FAST_PIPE_CLOSE_DELAY_TIME_MS},
};
static const std::unordered_map<std::string, int32_t> MOVE_STREAM_FAST_PIPE_DELAY_RELEASE_TIME_MS = {
    {"fast_a2dp_output", A2DP_MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS},
    {"fast_usb_arm_output", ARMUSB_MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS},
    {"fast_usb_arm_input", ARMUSB_MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS},
};
static int32_t GetFastPipeDelayReleaseTimeMs(const std::string &pipeName, const bool hasMoveStream,
    const bool isUltraFast)
{
    if (hasMoveStream && isUltraFast) {
        return MOVE_STREAM_FAST_PIPE_CLOSE_DELAY_TIME_MS;
    }
    const auto &delayReleaseMap = hasMoveStream ?
        MOVE_STREAM_FAST_PIPE_DELAY_RELEASE_TIME_MS : FAST_PIPE_DELAY_RELEASE_TIME_MS;
    auto it = delayReleaseMap.find(pipeName);
    return it != delayReleaseMap.end() ? it->second : DEFAULT_FAST_PIPE_CLOSE_DELAY_TIME_MS;
}

static const char* CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
inline static const std::string EMPTY_ADDRESS{"00:00:00:00:00:00"};
inline static const std::string NULL_ADDRESS{""};
static const uint32_t DEFAULT_PIPE_ID = 0;

static const std::unordered_set<SourceType> specialSourceTypeSet_ = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_REMOTE_CAST
};
static const std::unordered_set<uid_t> skipAddSessionIdUidSet_ = {
    MCU_UID,
    TV_SERVICE_UID
};

static const std::set<std::string> supportUltraFastBundleSet_ = {
};
}

static const std::vector<std::string> SourceNames = {
    std::string(PRIMARY_MIC),
    std::string(BLUETOOTH_MIC),
    std::string(USB_MIC),
    std::string(PRIMARY_WAKEUP),
    std::string(FILE_SOURCE),
    std::string(ACCESSORY_SOURCE),
    std::string(PRIMARY_AI_MIC),
    std::string(PRIMARY_UNPROCESS_MIC),
    std::string(PRIMARY_LIVE_MIC),
    std::string(PRIMARY_ULTRASONIC_MIC),
    std::string(PRIMARY_VOICE_RECOGNITION_MIC),
    std::string(PRIMARY_RAW_AI_MIC),
    std::string(PRIMARY_INTERPHON_MIC),
    std::string(PRIMARY_CAMCORDER),
    std::string(PRIMARY_VOICE_MESSAGE_MIC)
};

std::string AudioCoreService::GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

int32_t AudioCoreService::SetAudioRouteSelectorCallback(const AudioPipeSelector::AudioRouteSelectorCallback &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioPipeSelector_ != nullptr, ERR_NULL_POINTER, "audioPipeSelector_ is nullptr");
    return audioPipeSelector_->SetAudioRouteSelectorCallback(callback);
}

int32_t AudioCoreService::UnsetAudioRouteSelectorCallback()
{
    CHECK_AND_RETURN_RET_LOG(audioPipeSelector_ != nullptr, ERR_NULL_POINTER, "audioPipeSelector_ is nullptr");
    return audioPipeSelector_->UnsetAudioRouteSelectorCallback();
}

bool AudioCoreService::HandleA2dpSuspendWhenFetch(const AudioStreamDeviceChangeReasonExt &reason,
    const AudioDeviceDescriptor &actived, const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams)
{
    if (reason.IsOldDeviceUnavaliable() && !streams.empty()) {
        bool isSameA2dpUsed = any_of(streams.begin(), streams.end(), [](const auto &stream) {
            return stream != nullptr && stream->streamStatus_ == STREAM_STATUS_STARTED &&
                !stream->newDeviceDescs_.empty() &&
                stream->newDeviceDescs_.front() != nullptr &&
                stream->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                !stream->oldDeviceDescs_.empty() &&
                stream->newDeviceDescs_.front()->IsSameDeviceDescPtr(stream->oldDeviceDescs_.front());
        });
        CHECK_AND_RETURN_RET(!isSameA2dpUsed, false);

        bool activedWillChange = any_of(streams.begin(), streams.end(), [&actived](const auto &stream) {
            return stream != nullptr && stream->streamStatus_ == STREAM_STATUS_STARTED &&
                !stream->newDeviceDescs_.empty() &&
                !actived.IsSameDeviceDescPtr(stream->newDeviceDescs_.front());
        });
        if (activedWillChange) {
            HandleA2dpSuspend(actived.macAddress_);
            return true;
        }
    }

    return false;
}

void AudioCoreService::HandleA2dpSuspend(const std::string &macAddress)
{
    CHECK_AND_RETURN_LOG(!macAddress.empty(), "macAddress is empty");
    std::lock_guard<std::mutex> lock(a2dpSuspendMutex_);

    auto action = std::make_shared<RestoreA2dpSinkAction>(macAddress);
    CHECK_AND_RETURN_LOG(action != nullptr, "action is nullptr");
    AsyncActionHandler::AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
    desc.delayTimeMs = OLD_DEVICE_UNAVALIABLE_SUSPEND_MS;

    CHECK_AND_RETURN_LOG(asyncHandler_ != nullptr, "asyncHandler_ is nullptr");
    auto &state = a2dpSuspendStates_[macAddress];
    state.suspendUntil = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(OLD_DEVICE_UNAVALIABLE_SUSPEND_MS);
    CHECK_AND_RETURN_LOG(asyncHandler_->PostAsyncAction(desc), "post async action fail");

    CHECK_AND_RETURN(!state.needSuspend);

    AUDIO_INFO_LOG("suspend a2dp, macAddress: %{public}s", GetEncryptAddr(macAddress).c_str());
    AudioServerProxy::GetInstance().SuspendRenderSinkProxy("a2dp", macAddress);
    state.needSuspend = true;
}

bool AudioCoreService::UpdateActiveDeviceAndVolumeBeforeMoveSession(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    HandleMuteBeforeDeviceSwitch(streamDescs, reason);
    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    int32_t uid = SYSTEM_UID;
    std::vector<int32_t> activatedList;
    for (std::shared_ptr<AudioStreamDescriptor> &streamDesc : streamDescs) {
        // if streamDesc select bluetooth or headset, active it.
        if (!HandleOutputStreamInRunning(streamDesc, reason)) {
            continue;
        }

        if (streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
            streamDesc->newDeviceDescs_[0] != nullptr &&
            (streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_NEARLINK ||
            std::count(activatedList.begin(),
            activatedList.end(), streamDesc->newDeviceDescs_[0]->deviceId_) <= 0)) {
                int32_t outputRet = ActivateOutputDevice(streamDesc, reason);
                CHECK_AND_CONTINUE_LOG(outputRet == SUCCESS, "Activate output device failed");
                activatedList.push_back(streamDesc->newDeviceDescs_[0]->deviceId_);
        }

        // update current output device
        if ((Util::IsEnhancedRoutingSupported() || needUpdateActiveDevice) &&
            streamDesc->streamStatus_ == STREAM_STATUS_STARTED &&
            !pipeManager_->IsFocusStandby(streamDesc->sessionId_)) {
            isUpdateActiveDevice = UpdateOutputDevice(streamDesc->newDeviceDescs_.front(), GetRealUid(streamDesc),
                reason) || isUpdateActiveDevice;
            needUpdateActiveDevice = !isUpdateActiveDevice;
            uid = GetRealUid(streamDesc);
        }
    }
    OnRemoteDeviceStatusUpdated();
    AudioDeviceDescriptor audioDeviceDescriptor = audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid);
    std::shared_ptr<AudioDeviceDescriptor> descPtr =
        std::make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor);
    if (isUpdateActiveDevice && audioDeviceManager_.IsDeviceConnected(descPtr)) {
        OnPreferredOutputDeviceUpdated(audioDeviceDescriptor, reason);
    }
    return isUpdateActiveDevice;
}

void AudioCoreService::SyncVolWhenCurrentOutputDeviceChanged(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN(desc != nullptr && desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP);

#ifdef BLUETOOTH_ENABLE
    Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(desc->macAddress_);
#endif
}

void AudioCoreService::UpdateOffloadState(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN(pipeInfo && pipeInfo->streamDescriptors_.size() > 0);
    CHECK_AND_RETURN(pipeInfo->moduleInfo_.name == OFFLOAD_PRIMARY_SPEAKER ||
        pipeInfo->moduleInfo_.className == "remote_offload");
    OffloadType type = pipeInfo->moduleInfo_.className == "remote_offload" ? REMOTE_OFFLOAD : LOCAL_OFFLOAD;
    isOffloadOpened_[type].store(true);
    offloadCloseCondition_[type].notify_all();
}

void AudioCoreService::UpdateMultiChannelState(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN(pipeInfo && pipeInfo->streamDescriptors_.size() > 0 &&
        pipeInfo->streamDescriptors_[0] != nullptr);
    CHECK_AND_RETURN(pipeInfo->moduleInfo_.name == MCH_PRIMARY_SPEAKER);
    isMultiChannelOpened_.store(true);
    multiChannelCloseCondition_.notify_all();
    CHECK_AND_RETURN(pipeInfo->isCollaborationEnable_);
    audioPolicyManager_.UpdateCollaborativeState(COLLABORATION_NEARLINK_OPEN);
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(pipeInfo->streamDescriptors_[0]->GetMainNewDeviceType());
    UpdateNearLinkCollaborativeState(true);
}

void AudioCoreService::UpdatePrimaryExtraState(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN(pipeInfo && pipeInfo->streamDescriptors_.size() > 0);
    CHECK_AND_RETURN(pipeInfo->moduleInfo_.className == PRIMARY_EXTRA_CLASS);
    {
        std::lock_guard<std::mutex> lock(primaryExtraCloseMutex_);
        isPrimaryExtraOpenedMap_[pipeInfo->name_] = true;
    }
    primaryExtraCloseCondition_.notify_all();
}

void AudioCoreService::CheckAndUpdateOffloadEnableForStream(
    OffloadAction action, std::shared_ptr<AudioStreamDescriptor> &streamDesc, uint32_t ioHandle)
{
    if (action == OFFLOAD_NEW || action == OFFLOAD_MOVE_IN) {
        // Check stream is offload and then set
        if (streamDesc->IsRouteOffload()) {
            OffloadAdapter adapter = (streamDesc->IsDeviceRemote() ? OFFLOAD_IN_REMOTE : OFFLOAD_IN_PRIMARY);
            audioOffloadStream_.SetOffloadStatus(adapter, streamDesc->GetSessionId(), ioHandle);
        }
    } else {
        // Check stream is moved from offload and then unset
        if (streamDesc->IsRouteNormal() && (streamDesc->IsOldRouteOffload())) {
            audioOffloadStream_.UnsetOffloadStatus(streamDesc->GetSessionId());
        }
    }
}

void AudioCoreService::NotifyRouteUpdate(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    for (auto &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty(), "invalid streamDesc");
        std::lock_guard<std::mutex> lock(routeUpdateCallbackMutex_);
        uint32_t sessionId = streamDesc->sessionId_;
        CHECK_AND_CONTINUE_LOG(routeUpdateCallback_.count(sessionId) != 0, "sessionId %{public}u not registed",
            sessionId);
        auto callback = routeUpdateCallback_[sessionId].listener;
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        std::shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_.front();
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "device desc is nullptr");
        callback->OnRouteUpdate(streamDesc->routeFlag_, desc->networkId_);
    }
}

void AudioCoreService::DecidedAudioFlagForStreams(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    AUDIO_INFO_LOG("[PipeFetchStart] all %{public}zu input streams", streamDescs.size());
    bool needSetAllFastStreamNormal = false;
    for (std::shared_ptr<AudioStreamDescriptor> &streamDesc : streamDescs) {
        UpdatePlaybackStreamFlag(streamDesc, false);
        if (streamDesc->GetFastStreamForcedNormalFlag()) {
            AUDIO_INFO_LOG("Cause session: %{public}u, set all fast stream to normal", streamDesc->GetSessionId());
            needSetAllFastStreamNormal = true;
            streamDesc->ResetFastStreamForcedNormalFlag();
        }
    }
    CHECK_AND_RETURN(needSetAllFastStreamNormal);
    for (std::shared_ptr<AudioStreamDescriptor> streamDesc : streamDescs) {
        if (streamDesc->GetAudioFlag() == AUDIO_OUTPUT_FLAG_FAST ||
            streamDesc->GetAudioFlag() == AUDIO_OUTPUT_FLAG_ULTRA_FAST) {
            streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
            AUDIO_INFO_LOG("Set stream %{public}u to normal", streamDesc->sessionId_);
        }
    }
}

int32_t AudioCoreService::FetchRendererPipesAndExecute(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("[PipeFetchStart] all %{public}zu output streams", streamDescs.size());
    if (streamDescs.size() == 0) {
        return SUCCESS;
    }
    DecidedAudioFlagForStreams(streamDescs);
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipesAndExecute(streamDescs);

    // Update a2dp offload flag here because UpdateActiveRoute() need actual flag.
    CHECK_AND_RETURN_RET_LOG(audioA2dpOffloadManager_ != nullptr, ERROR, "audioA2dpOffloadManager_ is nullptr");
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();

    // Check if we need to stop fast sink before creating offload pipe
    // This is to prevent concurrency between fast and offload when fast is in delayed switch state
    bool hasNewOffloadPipe = false;
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW && pipeInfo->IsRouteOffload()) {
            hasNewOffloadPipe = true;
            AUDIO_INFO_LOG("Found new offload pipe to create: %{public}s", pipeInfo->name_.c_str());
            break;
        }
    }

    if (hasNewOffloadPipe && pipeManager_->HasPausedFastPipeInDelayedSwitch()) {
        std::string fastModuleName = pipeManager_->GetPausedFastPipeModuleNameInDelayedSwitch();
        if (!fastModuleName.empty()) {
            AUDIO_INFO_LOG("Stopping fast sink %{public}s before creating offload to prevent concurrency",
                fastModuleName.c_str());
            audioPolicyManager_.StopAudioPort(fastModuleName);
        }
    }

    StreamFlagInfo flagInfo;
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        UpdateOffloadState(pipeInfo);
        UpdateMultiChannelState(pipeInfo);
        UpdatePrimaryExtraState(pipeInfo);
        UpdateFastState(pipeInfo);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessOutputPipeUpdate(pipeInfo, flagInfo, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessOutputPipeNew(pipeInfo, flagInfo, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) {
            ProcessOutputPipeReload(pipeInfo, flagInfo, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    audioIOHandleMap_.NotifyUnmutePort();
    pipeManager_->UpdateRendererPipeInfos(pipeInfos);
    RemoveUnusedPipe(reason);
    NotifyRouteUpdate(streamDescs);
    return SUCCESS;
}

void AudioCoreService::CheckNeedPreRemoveUnusedPipe(std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos)
{
    bool hasFastInputNoStream = false;
    bool hasVoipFastInputNoStream = false;
    bool isFastInputNew = false;
    bool isVoipFastInputNew = false;
    std::shared_ptr<AudioPipeInfo> pipeInfoFastInput = nullptr;
    std::shared_ptr<AudioPipeInfo> pipeInfoVoipFastInput = nullptr;
    for (auto &pipeInfo : pipeInfos) {
        if (pipeInfo->name_ == "fast_input") {
            pipeInfoFastInput = pipeInfo;
            if (pipeInfo->streamDescriptors_.empty()) {
                hasFastInputNoStream = true;
            }
            if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
                isFastInputNew = true;
            }
        }

        if (pipeInfo->name_ == "voip_fast_input") {
            pipeInfoVoipFastInput = pipeInfo;
            if (pipeInfo->streamDescriptors_.empty()) {
                hasVoipFastInputNoStream = true;
            }
            if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
                isVoipFastInputNew = true;
            }
        }
    }
    if (hasFastInputNoStream && isVoipFastInputNew && pipeInfoFastInput != nullptr) {
        ReleaseFastPipeInner(pipeInfoFastInput->id_, pipeInfoFastInput->paIndex_,
            pipeInfoFastInput->name_, pipeInfoFastInput->moduleInfo_.name);
    }
    if (hasVoipFastInputNoStream && isFastInputNew && pipeInfoVoipFastInput != nullptr) {
        ReleaseFastPipeInner(pipeInfoVoipFastInput->id_, pipeInfoVoipFastInput->paIndex_,
            pipeInfoVoipFastInput->name_, pipeInfoVoipFastInput->moduleInfo_.name);
    }
}

int32_t AudioCoreService::FetchCapturerPipesAndExecute(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    AUDIO_INFO_LOG("[NewPipeFetch] === FetchCapturerPipesAndExecute START === stream count=%{public}zu",
        streamDescs.size());
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipesAndExecute(streamDescs);

    bool removeFlag = false;
    uint32_t fetchStreamId = UINT32_INVALID_VALUE;
    audioInjectorPolicy_.FetchCapDeviceInjectPreProc(pipeInfos, removeFlag, fetchStreamId);

    AUDIO_INFO_LOG("[PipeExecStart] for all Pipes");
    CheckNeedPreRemoveUnusedPipe(pipeInfos);
    uint32_t audioFlag;
    RemoveAndCloseUnusedCapturerPipe(pipeInfos);
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        HILOG_COMM_INFO("[PipeExecInfo] Scan Pipe adapter: %{public}s, name: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->name_.c_str(), pipeInfo->pipeAction_);
        UpdateFastState(pipeInfo);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessInputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessInputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) {
            ProcessInputPipeReload(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    pipeManager_->UpdateCapturerPipeInfos(pipeInfos);
    RemoveUnusedPipe();

    audioInjectorPolicy_.FetchCapDeviceInjectPostProc(pipeInfos, removeFlag, fetchStreamId);
    return SUCCESS;
}

int32_t AudioCoreService::CheckModemScene(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    const AudioStreamDeviceChangeReasonExt reason)
{
    if (!pipeManager_->IsModemCommunicationIdExist()) {
        CheckAndUpdateHearingAidCall(DEVICE_TYPE_NONE);
        return SUCCESS;
    }

    auto modemMap = pipeManager_->GetModemCommunicationMap();
    CHECK_AND_RETURN_RET_LOG(!modemMap.empty(), ERROR, "modem communication map is empty");
    auto streamDesc = modemMap.begin()->second;
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERROR, "streamDesc is nullptr");
    auto uid = GetRealUid(streamDesc);

    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    bool isTransferCall = (streamUsage == STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION);
    bool isCallRunning = audioSceneManager_.IsInPhoneCallScene();
    if (isCallRunning) {
        pipeManager_->UpdateModemStreamStatus(STREAM_STATUS_STARTED);
    } else {
        pipeManager_->UpdateModemStreamStatus(STREAM_STATUS_STOPPED);
    }
    FetchDeviceInfo info = { streamUsage, "CheckModemScene" };
    descs = audioRouterCenter_.FetchOutputDevices(info);
    CHECK_AND_RETURN_RET_LOG(descs.size() != 0, ERROR, "Fetch output device for voice modem communication failed");
    pipeManager_->UpdateModemStreamDevice(descs);

    AudioDeviceDescriptor curDesc = audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid);
    AUDIO_INFO_LOG("Current output device %{public}d, update route %{public}d, reason %{public}d",
        curDesc.deviceType_, descs.front()->deviceType_, static_cast<int32_t>(reason));
    int32_t activateRet = ActivateOutputDevice(streamDesc, reason);
    if (activateRet == REFETCH_DEVICE) {
        AUDIO_INFO_LOG("[CheckModemScene]ActivateOutputDevice returned REFETCH_DEVICE, "
            "device refetched in reentrant FetchOutputDeviceAndRoute");
        return REFETCH_DEVICE;
    }

    if (isCallRunning && IsDeviceSwitching(reason) && !curDesc.IsSameDeviceDesc(*descs.front())) {
        SetVoiceCallMuteForSwitchDevice();
        needUnmuteVoiceCall_ = true;
        SetUpdateModemRouteFinished(false);
        uint32_t muteDuration = GetVoiceCallMuteDuration(curDesc, *descs.front());
        std::thread switchThread(
            &AudioCoreService::UnmuteVoiceCallAfterMuteDuration, this, muteDuration, descs.front());
        switchThread.detach();
    }
    if (!isTransferCall) {
        CheckAndUpdateHearingAidCall(descs.front()->deviceType_);
    }
    CheckAndSleepBeforeVoiceCallDeviceSet(reason);
    return SUCCESS;
}

void AudioCoreService::CheckTransferRingtoneScene(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs, const AudioStreamDeviceChangeReasonExt reason)
{
    if (!pipeManager_->IsStreamUsageActive(STREAM_USAGE_VOICE_TRANSFER_RINGTONE)) {
        return;
    }
    
    FetchDeviceInfo info = {
        STREAM_USAGE_VOICE_TRANSFER_RINGTONE, "CheckTransferRingtoneScene"
    };
    descs = audioRouterCenter_.FetchOutputDevices(info);
    CHECK_AND_RETURN_LOG(descs.size() != 0, "Fetch output device for voice transfer ringtone failed");

    std::vector<std::shared_ptr<AudioStreamDescriptor>> transferRingtoneStreams =
        pipeManager_->GetStreamDescsByStreamUsage(STREAM_USAGE_VOICE_TRANSFER_RINGTONE);
    AUDIO_INFO_LOG("Transfer ringtone streams count: %{public}zu, devices count: %{public}zu, "
        "device types: %{public}d, %{public}d",
        transferRingtoneStreams.size(), descs.size(),
        descs.size() > 0 ? descs[0]->deviceType_ : -1,
        descs.size() > 1 ? descs[1]->deviceType_ : -1);
    for (auto &streamDesc : transferRingtoneStreams) {
        CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "streamDesc is nullptr");
        streamDesc->SwapAndUpdateNewDeviceDescs(descs);
    }
}

int32_t AudioCoreService::CheckRingAndVoipScene(const AudioStreamDeviceChangeReasonExt reason)
{
    AudioScene audioScene = audioSceneManager_.GetAudioScene();
    if (audioScene == AUDIO_SCENE_DEFAULT && !CheckRingAndVoipStreamRunning()) {
        return SUCCESS;
    }
    FetchDeviceInfo info = { STREAM_USAGE_NOTIFICATION_RINGTONE, "CheckRingAndVoipScene_1" };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ringDescs =
        audioRouterCenter_.FetchOutputDevices(info);
    CHECK_AND_RETURN_RET_LOG(ringDescs.size() != 0, ERROR, "Fetch output device for ring failed");

    info = { STREAM_USAGE_VOICE_COMMUNICATION, "CheckRingAndVoipScene_2" };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> voipDescs =
        audioRouterCenter_.FetchOutputDevices(info);
    CHECK_AND_RETURN_RET_LOG(voipDescs.size() != 0, ERROR, "Fetch output device for voip failed");

    pipeManager_->UpdateRingAndVoipStreamStatus(audioScene);
    pipeManager_->UpdateRingAndVoipStreamDevice(ringDescs, voipDescs);

    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> ringAndVoipDescMap =
        pipeManager_->GetRingAndVoipDescMap();
    for (auto &entry : ringAndVoipDescMap) {
        CHECK_AND_CONTINUE_LOG(entry.second != nullptr, "StreamDesc is nullptr");
        sleAudioDeviceManager_.UpdateSleStreamTypeCount(entry.second);
    }

    auto streamDesc = pipeManager_->GetStreamDescForAudioScene(audioScene);
    CHECK_AND_RETURN_RET(streamDesc != nullptr, ERROR);
    auto deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERROR, "deviceDesc is nullptr");
    if (deviceDesc->deviceType_ == DEVICE_TYPE_NEARLINK) {
        int32_t activateRet = ActivateOutputDevice(streamDesc, reason);
        if (activateRet == REFETCH_DEVICE) {
            AUDIO_INFO_LOG("[CheckRingAndVoipScene]ActivateOutputDevice returned REFETCH_DEVICE, "
                "device refetched in reentrant FetchOutputDeviceAndRoute");
            return REFETCH_DEVICE;
        }
    }
    return SUCCESS;
}

bool AudioCoreService::CheckRingAndVoipStreamRunning()
{
    return pipeManager_->CheckRingAndVoipStreamRunning();
}

int32_t AudioCoreService::UpdateModemRoute(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    if (!pipeManager_->IsModemCommunicationIdExist()) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(descs.size() != 0, ERROR, "Update device route for voice modem communication failed");
    CHECK_AND_RETURN_RET_LOG(descs.front() != nullptr, ERROR, "Update modem route: desc is nullptr");
    if (audioSceneManager_.IsInPhoneCallScene()) {
        audioActiveDevice_.UpdateActiveDeviceRoute(descs.front()->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG,
            DEFAULT_PIPE_ID, LOCAL_NETWORK_ID);
        if (needUnmuteVoiceCall_) {
            NotifyUnmuteVoiceCall();
            needUnmuteVoiceCall_ = false;
        }
    }
    AudioDeviceDescriptor desc = AudioDeviceDescriptor(descs.front());
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> modemSessionMap =
        pipeManager_->GetModemCommunicationMap();
    for (auto it = modemSessionMap.begin(); it != modemSessionMap.end(); ++it) {
        streamCollector_.UpdateRendererDeviceInfo(GetRealUid(it->second), it->first, desc);
        sleAudioDeviceManager_.UpdateSleStreamTypeCount(it->second);
    }

    bool isCallRunning = audioSceneManager_.IsInPhoneCallScene();
    auto streamDesc = modemSessionMap.begin()->second;
    CheckAndCreateCallTransferForTransferCall(streamDesc, isCallRunning);

    return SUCCESS;
}

// Called after the first route update when call is connected and after SCO is established.
// For transfer call scenario, SCO must be established before voice call can proceed.
void AudioCoreService::CheckAndCreateCallTransferForTransferCall(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, bool isCallRunning)
{
    AUDIO_INFO_LOG("streamDesc=%{public}p, streamUsage=%{public}d, callTransferCreated=%{public}d",
        streamDesc.get(), streamDesc != nullptr ? streamDesc->rendererInfo_.streamUsage : -1, callTransferCreated_);

    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");

    if (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION) {
        return;
    }

    if (callTransferCreated_) {
        return;
    }

    if (!isCallRunning) {
        return;
    }

    AUDIO_INFO_LOG("Transfer call first route update after SCO established, calling CreateCallTransferAfter");
    audioSceneManager_.CreateCallTransferAfter();
    callTransferCreated_ = true;
}

uint32_t AudioCoreService::GetVoiceCallMuteDuration(AudioDeviceDescriptor &curDesc, AudioDeviceDescriptor &newDesc)
{
    uint32_t muteDuration = 0;
    if (!curDesc.IsSameDeviceDesc(newDesc)) {
        muteDuration = VOICE_CALL_DEVICE_SWITCH_MUTE_US;
    }
    return muteDuration;
}

// muteDuration: duration to keep the voice call muted after modem route update
void AudioCoreService::UnmuteVoiceCallAfterMuteDuration(uint32_t muteDuration,
    std::shared_ptr<AudioDeviceDescriptor> desc)
{
    AUDIO_INFO_LOG("mute voice call %{public}d us after update modem route", muteDuration);
    {
        std::unique_lock<std::mutex> lock(updateModemRouteMutex_);
        updateModemRouteCV_.wait_for(lock, std::chrono::milliseconds(MUTE_TO_ROUTE_UPDATE_TIMEOUT_MS),
            [this] { return updateModemRouteFinished_; });
    }
    usleep(muteDuration);
    audioPolicyManager_.UpdateVolumeForAllPipes();
    audioVolumeManager_.SetVoiceCallVolumeOnDeviceSwitch(*desc, true);
}

void AudioCoreService::NotifyUnmuteVoiceCall()
{
    {
        std::unique_lock<std::mutex> lock(updateModemRouteMutex_);
        updateModemRouteFinished_ = true;
    }
    updateModemRouteCV_.notify_all();
}

void AudioCoreService::SetUpdateModemRouteFinished(bool flag)
{
    std::unique_lock<std::mutex> lock(updateModemRouteMutex_);
    updateModemRouteFinished_ = flag;
}

void AudioCoreService::CheckCloseHearingAidCall(const bool isModemCallRunning, const DeviceType type)
{
    if (hearingAidCallFlag_) {
        if ((isModemCallRunning && type != DEVICE_TYPE_HEARING_AID) || !isModemCallRunning) {
            hearingAidCallFlag_ = false;
            AudioServerProxy::GetInstance().SetAudioParameterProxy("mute_call", "false");

            CHECK_AND_RETURN_LOG(softLink_ != nullptr, "softLink is null");
            int32_t ret = softLink_->Stop();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Stop failed");
            ret = softLink_->Release();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Release failed");
            softLink_ = nullptr;

            std::shared_ptr<AudioPipeInfo> pipeInfo = pipeManager_->GetPipeinfoByNameAndFlag("primary",
                AUDIO_INPUT_FLAG_NORMAL);
            CHECK_AND_RETURN_LOG(pipeInfo != nullptr, "pipeInfo is null");
            pipeInfo->softLinkFlag_ = false;
            pipeManager_->UpdateAudioPipeInfo(pipeInfo);

            if (pipeInfo->streamDescriptors_.empty()) {
                RemoveUnusedRecordPipe();
                hearingAidReloadFlag_ = false;
            } else {
                ReloadCaptureSessionSoftLink();
            }
        }
    }
}

void AudioCoreService::CheckOpenHearingAidCall(const bool isModemCallRunning, const DeviceType type)
{
    if (!hearingAidCallFlag_) {
        if (isModemCallRunning && type == DEVICE_TYPE_HEARING_AID) {
            uint32_t paIndex = 0;
            CHECK_AND_CALL_FUNC_RETURN(CheckModuleForHearingAid(paIndex) == SUCCESS,
                HILOG_COMM_ERROR("[CheckOpenHearingAidCall]openAudioPort failed"));

            std::shared_ptr<AudioPipeInfo> pipeInfoOutput = pipeManager_->GetPipeinfoByNameAndFlag("hearing_aid",
                AUDIO_OUTPUT_FLAG_NORMAL);
            CHECK_AND_RETURN_LOG(pipeInfoOutput != nullptr, "Can not find pipe hearing_aid");

            audioActiveDevice_.UpdateActiveDeviceRoute(DeviceType::DEVICE_TYPE_SPEAKER,
                DeviceFlag::OUTPUT_DEVICES_FLAG, DEFAULT_PIPE_ID);
            softLink_ = HPAE::IHpaeSoftLink::CreateSoftLink(pipeInfoOutput->paIndex_, paIndex,
                HPAE::SoftLinkMode::HEARING_AID);
            CHECK_AND_RETURN_LOG(softLink_ != nullptr, "CreateSoftLink failed");
            int32_t ret = softLink_->Start();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Start failed");
            AudioServerProxy::GetInstance().SetAudioParameterProxy("mute_call", "true");
            hearingAidCallFlag_ = true;
        }
    }
}

int32_t AudioCoreService::CheckModuleForHearingAid(uint32_t &paIndex)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool configRet = pipeManager_->GetModuleListByType(ClassType::TYPE_PRIMARY, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(configRet, ERR_OPERATION_FAILED, "HearingAid not exist in config");
    for (auto &moduleInfo : moduleInfoList) {
        if (moduleInfo.role != "source") { continue; }
        AUDIO_INFO_LOG("hearingAidCall connects");
        moduleInfo.networkId = "LocalDevice";
        moduleInfo.deviceType = std::to_string(DEVICE_TYPE_MIC);
        moduleInfo.sourceType = std::to_string(SOURCE_TYPE_VOICE_CALL);

        std::shared_ptr<AudioPipeInfo> pipeInfoInput =
            pipeManager_->GetPipeinfoByNameAndFlag("primary", AUDIO_INPUT_FLAG_NORMAL);
        if (pipeInfoInput == nullptr) {
            AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
            CHECK_AND_CALL_FUNC_RETURN_RET(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
                HILOG_COMM_ERROR("[CheckModuleForHearingAid]OpenAudioPort failed ioHandle[%{public}u]", ioHandle));
            CHECK_AND_CALL_FUNC_RETURN_RET(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
                HILOG_COMM_ERROR("[CheckModuleForHearingAid]OpenAudioPort failed paId[%{public}u]", paIndex));
            audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);
            std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
            pipeInfo->name_ = "primary_input";
            pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
            pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
            pipeInfo->adapterName_ = "primary";
            pipeInfo->moduleInfo_ = moduleInfo;
            pipeInfo->pipeAction_ = PIPE_ACTION_NEW;
            pipeInfo->softLinkFlag_ = true;
            pipeInfo->id_ = ioHandle;
            pipeInfo->paIndex_ = paIndex;
            pipeManager_->AddAudioPipeInfo(pipeInfo);
            AUDIO_INFO_LOG("Add PipeInfo %{public}u in load hearingAidCall.", pipeInfo->id_);
            hearingAidReloadFlag_ = true;
        } else {
            int32_t ret = ReloadCaptureSoftLink(pipeInfoInput, moduleInfo);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "ReloadCaptureSoftLink failed");
            CHECK_AND_RETURN_RET_LOG(pipeInfoInput != nullptr, ERROR, "can not find primary pipeInfo");
            paIndex = pipeInfoInput->paIndex_;
        }
    }
    return SUCCESS;
}

void AudioCoreService::CheckAndUpdateHearingAidCall(const DeviceType type)
{
    bool isModemCallRunning = audioSceneManager_.IsInPhoneCallScene();
    CheckCloseHearingAidCall(isModemCallRunning, type);
    CheckOpenHearingAidCall(isModemCallRunning, type);
}

void AudioCoreService::UpdateDefaultOutputDeviceWhenStopping(int32_t uid)
{
    std::vector<uint32_t> sessionIDSet = streamCollector_.GetAllRendererSessionIDForUID(uid);
    for (const auto &sessionID : sessionIDSet) {
        if (isRingDualToneOnPrimarySpeaker_ && (streamCollector_.GetStreamType(sessionID) == STREAM_RING ||
            streamCollector_.GetStreamType(sessionID) == STREAM_ALARM)) {
            AUDIO_INFO_LOG("disable primary speaker dual tone when ringer renderer died");
            isRingDualToneOnPrimarySpeaker_ = false;
            for (std::pair<uint32_t, AudioStreamType> stream : streamsWhenRingDualOnPrimarySpeaker_) {
                audioPolicyManager_.SetDualStreamVolumeMute(stream.first, false);
            }
            streamsWhenRingDualOnPrimarySpeaker_.clear();
            AudioStreamType streamType = streamCollector_.GetStreamType(sessionID);
            if (streamType == STREAM_MUSIC) {
                audioPolicyManager_.SetDualStreamVolumeMute(sessionID, false);
            }
        }
    }
}

void AudioCoreService::ProcessOutputPipeReload(std::shared_ptr<AudioPipeInfo> pipeInfo, StreamFlagInfo &flagInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    int32_t engineFlag = GetEngineFlag();
    uint32_t paIndex = HDI_INVALID_ID;
    CHECK_AND_CALL_FUNC_RETURN(engineFlag == 1, HILOG_COMM_ERROR("[ProcessOutputPipeReload]not find proaudio port"));

    audioPolicyManager_.ReloadAudioPort(pipeInfo->moduleInfo_, paIndex);
    CHECK_AND_CALL_FUNC_RETURN(paIndex != HDI_INVALID_ID,
        HILOG_COMM_ERROR("[ProcessOutputPipeReload]ReloadAudioPort failed paId[%{public}u]", paIndex));

    audioPolicyManager_.RemoveAudioPipeVolume(pipeInfo->id_);
    pipeInfo->paIndex_ = paIndex;
    ProcessOutputPipeUpdate(pipeInfo, flagInfo, reason);
}

// This API is called by non-split-car to ensure that device can be selected.
int32_t AudioCoreService::SetSplitModeReady()
{
    AUDIO_INFO_LOG("SetSplitModeReady");
    int32_t setRet = AudioRouterCenter::GetAudioRouterCenter().SetSplitModeReady();
    CHECK_AND_RETURN_RET_LOG(setRet == 0, setRet, "SetSplitModeReady failed");
    FetchOutputDeviceAndRoute("SetSplitModeReady");
    return SUCCESS;
}

bool AudioCoreService::IsNeedReloadPipe(const std::string &moduleName, const std::string &splitArgs,
    const std::string &networkId)
{
    AudioIOHandle existingModuleId;
    bool isPortExist = audioIOHandleMap_.GetModuleIdByKey(moduleName, existingModuleId);
    CHECK_AND_RETURN_RET_LOG(isPortExist, false, "pipe not exist");

    std::lock_guard<std::mutex> lock(splitModuleArgsMutex_);
    auto it = splitModuleArgsMap_.find(networkId);
    CHECK_AND_RETURN_RET_LOG(!(it != splitModuleArgsMap_.end() && it->second == splitArgs), false,
        "already loaded with same args");

    return true;
}

int32_t AudioCoreService::SetSupportReload(bool isSupportReload, const std::string &networkId)
{
    CHECK_AND_RETURN_RET_LOG(!networkId.empty(), ERR_INVALID_PARAM, "cannot find device without valid networkId");

    AUDIO_INFO_LOG("SetSupportReload: %{public}d, networkId [%{public}s]", isSupportReload,
        GetEncryptStr(networkId).c_str());
    bool ret = audioConnectedDevice_.SetSupportReload(networkId, DEVICE_TYPE_MIC, INPUT_DEVICE, isSupportReload) &&
        audioDeviceManager_.SetSupportReload(networkId, DEVICE_TYPE_MIC, INPUT_DEVICE, isSupportReload);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "device not found for networkId [%{public}s]",
        GetEncryptStr(networkId).c_str());
    return SUCCESS;
}

int32_t AudioCoreService::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    AUDIO_INFO_LOG("[ADeviceEvent] Start split args: %{public}s", splitArgs.c_str());
    if (splitArgs.empty() || networkId.empty()) {
        std::string anonymousNetworkId = networkId.empty() ? "" : networkId.substr(0, 2) + "***";
        AUDIO_ERR_LOG("invalid param, splitArgs:'%{public}s', networkId:'%{public}s'",
            splitArgs.c_str(), anonymousNetworkId.c_str());
        return ERR_INVALID_PARAM;
    }
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, OUTPUT_DEVICE);
    CHECK_AND_RETURN_RET_LOG(IsNeedReloadPipe(moduleName, splitArgs, networkId), SUCCESS, "no need to reload pipe");
    std::string currentActivePort = REMOTE_CLASS;
    audioPolicyManager_.SuspendAudioDevice(currentActivePort, true);
    AudioIOHandle oldModuleId;
    audioIOHandleMap_.GetModuleIdByKey(moduleName, oldModuleId);
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERR_NULL_POINTER, "pipeManager_ is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescriptors =
        pipeManager_->GetStreamDescsByIoHandle(oldModuleId);
    audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleName);

    AudioModuleInfo moduleInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
        OUTPUT_DEVICE, DEVICE_TYPE_SPEAKER);
    moduleInfo.lib = "libmodule-split-stream-sink.z.so";
    moduleInfo.extra = splitArgs;
    moduleInfo.needEmptyChunk = true;

    int32_t openRet = audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleName, moduleInfo);
    if (openRet != 0) {
        AUDIO_ERR_LOG("open fail, OpenPortAndInsertIOHandle ret: %{public}d", openRet);
    } else {
        std::lock_guard<std::mutex> lock(splitModuleArgsMutex_);
        splitModuleArgsMap_[networkId] = splitArgs;
    }
    // Notify router split mode is ready to ensure device can be selected.
    int32_t setRet = AudioRouterCenter::GetAudioRouterCenter().SetSplitModeReady();
    JUDGE_AND_ERR_LOG(setRet != 0, "set failed, SetSplitModeReady ret: %{public}d", setRet);
    AudioIOHandle newModuleId;
    audioIOHandleMap_.GetModuleIdByKey(moduleName, newModuleId);
    pipeManager_->UpdateOutputStreamDescsByIoHandle(newModuleId, streamDescriptors);
    AudioServerProxy::GetInstance().NotifyDeviceInfoProxy(networkId, true);
    FetchOutputDeviceAndRoute("LoadSplitModule");
    AUDIO_INFO_LOG("fetch device after split stream and open port.");
    return openRet;
}

bool AudioCoreService::IsSameDevice(shared_ptr<AudioDeviceDescriptor> &desc, const AudioDeviceDescriptor &deviceInfo)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NULL_POINTER, "invalid deviceDesc");
    if (desc->networkId_ == deviceInfo.networkId_ && desc->deviceType_ == deviceInfo.deviceType_ &&
        desc->macAddress_ == deviceInfo.macAddress_ && desc->connectState_ == deviceInfo.connectState_) {
        if (deviceInfo.IsAudioDeviceDescriptor()) {
            return true;
        }
        BluetoothOffloadState state = audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
            // switch to A2dp
            ((deviceInfo.a2dpOffloadFlag_ == A2DP_OFFLOAD && state != A2DP_OFFLOAD) ||
            // switch to A2dp offload
            (deviceInfo.a2dpOffloadFlag_ != A2DP_OFFLOAD && state == A2DP_OFFLOAD))) {
            return false;
        }
        if (IsUsb(desc->deviceType_)) {
            return desc->deviceRole_ == deviceInfo.deviceRole_;
        }
        return true;
    } else {
        return false;
    }
}


bool AudioCoreService::IsStreamSupportMmap(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    const auto &streamInfo = streamDesc->streamInfo_;
    // check audio sample rate
    const AudioSamplingRate samplingRate = streamInfo.samplingRate;
    auto rateItem = std::find(AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES.begin(),
        AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES.end(), samplingRate);
    CHECK_AND_RETURN_RET_LOG(rateItem != AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES.end(), false,
        "Sampling rate %{public}d does not meet the requirements", samplingRate);

    // check audio channel
    const AudioChannel channels = streamInfo.channels;
    auto channelItem = std::find(AUDIO_FAST_STREAM_SUPPORTED_CHANNELS.begin(),
        AUDIO_FAST_STREAM_SUPPORTED_CHANNELS.end(), channels);
    CHECK_AND_RETURN_RET_LOG(channelItem != AUDIO_FAST_STREAM_SUPPORTED_CHANNELS.end(), false,
        "Audio channel %{public}d does not meet the requirements", channels);

    // check audio sample format
    const AudioSampleFormat format = streamInfo.format;
    auto formatItem = std::find(AUDIO_FAST_STREAM_SUPPORTED_FORMATS.begin(),
        AUDIO_FAST_STREAM_SUPPORTED_FORMATS.end(), format);
    CHECK_AND_RETURN_RET_LOG(formatItem != AUDIO_FAST_STREAM_SUPPORTED_FORMATS.end(), false,
        "Audio sample format %{public}u does not meet the requirements", format);

    return true;
}

int32_t AudioCoreService::FetchDeviceAndRoute(std::string caller, const AudioStreamDeviceChangeReasonExt reason)
{
    int32_t ret = FetchOutputDeviceAndRoute(caller, reason);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fetch output device failed");
    return FetchInputDeviceAndRoute(caller, reason);
}

int32_t AudioCoreService::FetchRendererPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    uint32_t &sessionId, StreamFlagInfo &flagInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "stream desc is nullptr");
    UpdatePlaybackStreamFlag(streamDesc, true);
    HILOG_COMM_INFO("[PipeFetchStart] AudioFlag 0x%{public}x for stream %{public}u", streamDesc->audioFlag_, sessionId);
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipeAndExecute(streamDesc);
    CHECK_AND_RETURN_RET_LOG(!(pipeInfos.empty() && streamDesc->GetRouteSelectRejectedFlag()), ERR_OPERATION_FAILED,
        "route select rejected renderer create for stream %{public}u", sessionId);

    HandleDirectPipeConflictForVoipDirect(streamDesc, pipeInfos, reason);

    uint32_t sinkId = HDI_INVALID_ID;
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        HILOG_COMM_INFO("[PipeExecInfo] Scan Pipe adapter: %{public}s, name: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->name_.c_str(), pipeInfo->pipeAction_);
        UpdateOffloadState(pipeInfo);
        UpdateMultiChannelState(pipeInfo);
        UpdatePrimaryExtraState(pipeInfo);
        UpdateFastState(pipeInfo);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessOutputPipeUpdate(pipeInfo, flagInfo, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) { // new
            ProcessOutputPipeNew(pipeInfo, flagInfo, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) {
            ProcessOutputPipeReload(pipeInfo, flagInfo, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) { // DEFAULT
            // Do nothing
        }
    }
    RemoveUnusedPipe(reason);
    return SUCCESS;
}

void AudioCoreService::HandleDirectPipeConflictForVoipDirect(
    std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos, const AudioStreamDeviceChangeReasonExt reason)
{
    // Voip direct and music direct share the same direct instance on Hisi.
    // Creating or closing either pipe operates on the same instance, which causes
    // silent or variable-speed playback. Therefore, when a voip direct stream is
    // created, existing direct pipe streams must be moved to primary and the
    // direct pipe must be closed first to avoid the conflict.
    CHECK_AND_RETURN(streamDesc != nullptr && streamDesc->IsVoipDirectRoute());
    std::shared_ptr<AudioPipeInfo> directPipe = pipeManager_->FindDirectOutputPipe();
    CHECK_AND_RETURN(directPipe != nullptr);
    std::shared_ptr<AudioPipeInfo> primaryPipe =
        pipeManager_->GetPipeinfoByNameAndFlag("primary", AUDIO_OUTPUT_FLAG_NORMAL);
    CHECK_AND_RETURN(primaryPipe != nullptr);
    MoveDirectStreamsToPrimary(directPipe, primaryPipe, reason);
    CloseDirectPipe(directPipe);
    pipeInfos.erase(std::remove_if(pipeInfos.begin(), pipeInfos.end(),
        [&directPipe](const std::shared_ptr<AudioPipeInfo> &pipe) { return pipe == directPipe; }),
        pipeInfos.end());
    AUDIO_INFO_LOG("voip direct stream %{public}u conflict handled, "
        "direct pipe closed and streams moved to primary", streamDesc->sessionId_);
}

void AudioCoreService::MoveDirectStreamsToPrimary(std::shared_ptr<AudioPipeInfo> directPipe,
    std::shared_ptr<AudioPipeInfo> primaryPipe, const AudioStreamDeviceChangeReasonExt reason)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams = directPipe->streamDescriptors_;
    for (auto &stream : streams) {
        CHECK_AND_CONTINUE(stream != nullptr);
        MoveStreamSink(stream, primaryPipe, reason);
        stream->ResetToNormalRoute(true);
        stream->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
        audioPipeSelector_->UpdateRendererPipeInfo(stream);
        directPipe->RemoveStream(stream->GetSessionId());
        pipeManager_->AddStreamToPipe(primaryPipe, stream, PIPE_ACTION_UPDATE);
    }
}

void AudioCoreService::CloseDirectPipe(std::shared_ptr<AudioPipeInfo> directPipe)
{
    CHECK_AND_RETURN_LOG(directPipe != nullptr, "directPipe is nullptr");
    audioPolicyManager_.CloseAudioPort(directPipe->id_, directPipe->paIndex_);
    pipeManager_->RemoveAudioPipeInfo(directPipe);
    audioIOHandleMap_.DelIOHandleInfo(directPipe->moduleInfo_.name);
    audioPolicyManager_.RemoveAudioPipeVolume(directPipe->id_);
}

void AudioCoreService::ProcessOutputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, StreamFlagInfo &flagInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    uint32_t paIndex = 0;
    uint32_t id = OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    CHECK_AND_RETURN_LOG(id != HDI_INVALID_ID, "Invalid id: %{public}u", id);
    pipeInfo->id_ = id;
    CHECK_AND_RETURN_LOG(paIndex != OPEN_PORT_FAILURE, "Invalid paIndex: %{public}u", paIndex);
    pipeInfo->paIndex_ = paIndex;
    bool hasMoveStream = false;
    for (auto &desc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        hasMoveStream = hasMoveStream != true && desc->streamAction_ != AUDIO_STREAM_ACTION_MOVE ? false : true;
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_NEW, desc, pipeInfo->id_);
                flagInfo.audioFlag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_MOVE:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, desc, pipeInfo->id_);
                if (desc->streamStatus_ != STREAM_STATUS_STARTED ||
                    pipeManager_->IsFocusStandby(desc->sessionId_)) {
                    MoveStreamSink(desc, pipeInfo, reason);
                } else {
                    MoveToNewOutputDevice(desc, pipeInfo, reason);
                }
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_IN, desc, pipeInfo->id_);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateRendererStreamCallbackEntry(desc, reason);
                break;
            default:
                break;
        }
        audioPipeSelector_->UpdateRendererPipeInfo(desc);
    }
    pipeInfo->SetHasMoveStream(hasMoveStream);
    pipeManager_->AddAudioPipeInfo(pipeInfo);
}

void AudioCoreService::ProcessOutputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, StreamFlagInfo &flagInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::ProcessOutputPipeUpdate");
    if (pipeInfo->moduleInfo_.deviceType == std::to_string(static_cast<int32_t>(DEVICE_TYPE_USB_ARM_HEADSET))) {
        CancelArmReload(pipeInfo);
    }
    bool hasMoveStream = false;
    for (auto &desc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        hasMoveStream = hasMoveStream != true && desc->streamAction_ != AUDIO_STREAM_ACTION_MOVE ? false : true;
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_NEW, desc, pipeInfo->id_);
                flagInfo.audioFlag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, desc, pipeInfo->id_);
                if (desc->streamStatus_ != STREAM_STATUS_STARTED ||
                    pipeManager_->IsFocusStandby(desc->sessionId_)) {
                    MoveStreamSink(desc, pipeInfo, reason);
                } else {
                    MoveToNewOutputDevice(desc, pipeInfo, reason);
                }
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_IN, desc, pipeInfo->id_);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateRendererStreamCallbackEntry(desc, reason);
                break;
            default:
                break;
        }
        audioPipeSelector_->UpdateRendererPipeInfo(desc);
    }
    pipeInfo->SetHasMoveStream(hasMoveStream);
    pipeManager_->UpdateAudioPipeInfo(pipeInfo);
}

int32_t AudioCoreService::FetchCapturerPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    uint32_t &audioFlag, uint32_t &sessionId)
{
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        AUDIO_INFO_LOG("[PipeFetchInfo] playbackcapture, no need fetch pipe");
        audioFlag = AUDIO_INPUT_FLAG_NORMAL;
        return SUCCESS;
    }

    AUDIO_INFO_LOG("[PipeFetchStart] for stream %{public}d", sessionId);
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipeAndExecute(streamDesc);
    CHECK_AND_RETURN_RET_LOG(!(pipeInfos.empty() && streamDesc->GetRouteSelectRejectedFlag()), ERR_OPERATION_FAILED,
        "route select rejected capturer create for stream %{public}u", sessionId);

    CheckNeedPreRemoveUnusedPipe(pipeInfos);

    RemoveAndCloseUnusedCapturerPipe(pipeInfos);
    for (auto &pipeInfo : pipeInfos) {
        HILOG_COMM_INFO("[PipeExecInfo] Scan Pipe adapter: %{public}s, name: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->name_.c_str(), pipeInfo->pipeAction_);
        UpdateFastState(pipeInfo);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessInputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessInputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) {
            ProcessInputPipeReload(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    RemoveUnusedPipe();
    for (const auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE(pipeInfo != nullptr);
        if (pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_ULTRA_FAST &&
            pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT && pipeInfo->id_ == OPEN_PORT_FAILURE) {
            return ERR_DEVICE_INIT;
        }
    }
    CHECK_AND_RETURN_RET(streamDesc->routeFlag_ != AUDIO_INPUT_FLAG_ULTRA_FAST ||
        !pipeManager_->GetModuleNameBySessionId(sessionId).empty(), ERR_DEVICE_INIT);
    return SUCCESS;
}

void AudioCoreService::ProcessInputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    uint32_t paIndex = 0;
    uint32_t sourceId = OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    if (pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_ULTRA_FAST && sourceId == OPEN_PORT_FAILURE) {
        pipeInfo->id_ = OPEN_PORT_FAILURE;
        pipeInfo->SetAction(PIPE_ACTION_DEFAULT);
        AUDIO_ERR_LOG("Ultra fast capture HAL initialization failed");
        return;
    }
    pipeInfo->id_ = sourceId;
    pipeInfo->paIndex_ = paIndex;
    std::vector<SourceOutput> sourceOutputs = GetSourceOutputs();
    bool hasMoveStream = false;
    for (auto &desc : pipeInfo->streamDescriptors_) {
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        hasMoveStream = hasMoveStream != true && desc->streamAction_ != AUDIO_STREAM_ACTION_MOVE ? false : true;
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                if (desc->streamStatus_ != STREAM_STATUS_STARTED) {
                    MoveStreamSource(desc, sourceOutputs);
                } else {
                    MoveToNewInputDevice(desc, sourceOutputs, pipeInfo);
                }
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateCapturerStreamCallback(desc);
                break;
            default:
                break;
        }
    }
    pipeInfo->SetHasMoveStream(hasMoveStream);
    pipeManager_->AddAudioPipeInfo(pipeInfo);
}

bool AudioCoreService::IsDescInSourceStrategyMap(std::shared_ptr<AudioStreamDescriptor> desc)
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET_LOG(sourceStrategyMap != nullptr, false, "sourceStrategyMap is nullptr");

    auto strategyIt = sourceStrategyMap->find(desc->capturerInfo_.sourceType);
    CHECK_AND_RETURN_RET(strategyIt != sourceStrategyMap->end(), false);
    return true;
}

void AudioCoreService::ProcessInputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    std::vector<SourceOutput> sourceOutputs = GetSourceOutputs();
    bool hasMoveStream = false;
    for (auto desc : pipeInfo->streamDescriptors_) {
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        hasMoveStream = hasMoveStream != true && desc->streamAction_ != AUDIO_STREAM_ACTION_MOVE ? false : true;
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                if (desc->streamStatus_ != STREAM_STATUS_STARTED) {
                    MoveStreamSource(desc, sourceOutputs);
                } else {
                    MoveToNewInputDevice(desc, sourceOutputs, pipeInfo);
                }
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateCapturerStreamCallback(desc);
                break;
            default:
                break;
        }
    }
    pipeInfo->SetHasMoveStream(hasMoveStream);
    pipeManager_->UpdateAudioPipeInfo(pipeInfo);
}

void AudioCoreService::ProcessInputPipeReload(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
    AUDIO_INFO_LOG("[NewPipeFetch] === ProcessInputPipeReload EXECUTE === "
        "pipeName=%{public}s, adapter=%{public}s, ecType=%{public}s, openMicRef=%{public}s",
        pipeInfo->moduleInfo_.name.c_str(), pipeInfo->moduleInfo_.adapterName.c_str(),
        pipeInfo->moduleInfo_.ecType.c_str(), pipeInfo->moduleInfo_.openMicRef.c_str());
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;

    uint32_t paIndex = HDI_INVALID_ID;
    audioPolicyManager_.ReloadAudioPort(pipeInfo->moduleInfo_, paIndex);
    CHECK_AND_RETURN_LOG(paIndex != HDI_INVALID_ID, "ReloadAudioPort failed");
    pipeInfo->paIndex_ = paIndex;
    ProcessInputPipeUpdate(pipeInfo, flag);
}

void AudioCoreService::RemoveAndCloseUnusedPipeInternal(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos,
    const AudioStreamDeviceChangeReasonExt &reason)
{
    bool isOldDeviceUnavailable = reason.IsOldDeviceUnavaliable() || reason.IsOldDeviceUnavaliableExt();
    for (auto pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        HILOG_COMM_INFO("[PipeExecInfo] Remove and close Pipe %{public}s", pipeInfo->ToString().c_str());
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER) {
            OffloadType type = pipeInfo->moduleInfo_.className == "remote_offload" ? REMOTE_OFFLOAD : LOCAL_OFFLOAD;
            if (type == REMOTE_OFFLOAD) {
                CHECK_AND_CONTINUE(isOffloadOpened_[type].load());
                isOffloadOpened_[type].store(false);
            } else {
                DelayReleaseOffloadPipe(pipeInfo->id_, pipeInfo->paIndex_, type);
                continue;
            }
        } else if (pipeInfo->IsRouteFast()) {
            DelayReleaseFastPipe(pipeInfo);
            continue;
        } else if (pipeInfo->moduleInfo_.name == MCH_PRIMARY_SPEAKER && pipeInfo->isCollaborationEnable_) {
            if (!isRingDualToneOnPrimarySpeaker_) {
                DelayReleaseMultiChannelPipe(pipeInfo->id_, pipeInfo->paIndex_);
                continue;
            }
            pipeInfo->isCollaborationEnable_ = false;
            UpdateNearLinkCollaborativeState(false);
        } else if ((pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_NORMAL) &&
            pipeInfo->moduleInfo_.className == PRIMARY_EXTRA_CLASS) {
            if (!isOldDeviceUnavailable) {
                DelayReleasePrimaryExtraPipe(pipeInfo->id_, pipeInfo->paIndex_, pipeInfo->name_,
                    pipeInfo->moduleInfo_.name);
                continue;
            }
            {
                std::lock_guard<std::mutex> lock(primaryExtraCloseMutex_);
                isPrimaryExtraOpenedMap_[pipeInfo->name_] = false;
                isPrimaryExtraInReleaseMap_[pipeInfo->name_] = false;
                primaryExtraCloseCondition_.notify_all();
            }
            AUDIO_INFO_LOG("primary_extra %{public}s close (old device unavailable)", pipeInfo->name_.c_str());
        } else if (pipeInfo->moduleInfo_.deviceType ==
            std::to_string(static_cast<int32_t>(DEVICE_TYPE_USB_ARM_HEADSET)) && !isOldDeviceUnavailable) {
            DelayReloadArmPipe(pipeInfo);
            continue;
        }
        audioPolicyManager_.CloseAudioPort(pipeInfo->id_, pipeInfo->paIndex_);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.DelIOHandleInfo(pipeInfo->moduleInfo_.name);
        audioPolicyManager_.RemoveAudioPipeVolume(pipeInfo->id_);
    }
    StopNoRunningPipe();
}

void AudioCoreService::RemoveUnusedPipe(const AudioStreamDeviceChangeReasonExt reason)
{
    RemoveAndCloseUnusedPipeInternal(pipeManager_->GetUnusedPipe(), reason);
}

void AudioCoreService::RemoveAndCloseUnusedCapturerPipe(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos,
    const AudioStreamDeviceChangeReasonExt reason)
{
    // Keep enhanced-routing capability gating in one place for all capturer fetch paths.
    CHECK_AND_RETURN_LOG(Util::IsEnhancedRoutingSupported(), "enhanced routing is not supported");
    auto unusedPipes = pipeManager_->GetUnusedPipeForCapturer(pipeInfos);
    RemoveAndCloseUnusedPipeInternal(unusedPipes, reason);
    // The closed unused pipes are no longer valid, remove them from pipeInfos to avoid
    // processing them again in the subsequent pipe execution loop.
    pipeInfos.erase(std::remove_if(pipeInfos.begin(), pipeInfos.end(),
        [&unusedPipes](const std::shared_ptr<AudioPipeInfo> &pipeInfo) {
            return std::find(unusedPipes.begin(), unusedPipes.end(), pipeInfo) != unusedPipes.end();
        }), pipeInfos.end());
}

void AudioCoreService::RemoveUnusedRecordPipe()
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = pipeManager_->GetUnusedRecordPipe();
    for (auto pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        HILOG_COMM_INFO("[PipeExecInfo] Remove and close Pipe %{public}s", pipeInfo->ToString().c_str());
        if (pipeInfo->IsRouteFast()) {
            DelayReleaseFastPipe(pipeInfo);
            continue;
        }
        audioPolicyManager_.CloseAudioPort(pipeInfo->id_, pipeInfo->paIndex_);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.DelIOHandleInfo(pipeInfo->moduleInfo_.name);
        audioPolicyManager_.RemoveAudioPipeVolume(pipeInfo->id_);
    }
}

std::string AudioCoreService::GetAdapterNameBySessionId(uint32_t sessionId)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    return pipeManager_->GetAdapterNameBySessionId(sessionId);
}

std::string AudioCoreService::GetModuleNameBySessionId(uint32_t sessionId)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    return pipeManager_->GetModuleNameBySessionId(sessionId);
}

std::string AudioCoreService::Get3daDirectBusAddress()
{
    return pipeManager_->Get3daDirectBusAddress();
}

int32_t AudioCoreService::GetProcessDeviceInfoBySessionId(uint32_t sessionId,
    AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo, bool &isUltraFast)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    deviceInfo = AudioDeviceDescriptor(pipeManager_->GetProcessDeviceInfoBySessionId(sessionId, streamInfo));
    isUltraFast = pipeManager_->IsStreamUseUltraFastRoute(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::GetPipeBaseDebugInfo(const std::shared_ptr<AudioPipeInfo> &pipe, AudioDebugInfo &debugInfo)
{
    debugInfo.pipeId = pipe->id_;
    debugInfo.paIndex = pipe->paIndex_;
    debugInfo.pipeRole = pipe->pipeRole_;
    debugInfo.pipeName = pipe->name_;
    debugInfo.routeFlag = pipe->routeFlag_;
    debugInfo.adapterName = pipe->adapterName_;
    debugInfo.moduleName = pipe->moduleInfo_.name;
    debugInfo.pipeFormat = pipe->moduleInfo_.format;
    debugInfo.pipeRate = pipe->moduleInfo_.rate;
    debugInfo.pipeChannels = pipe->moduleInfo_.channels;
    debugInfo.pipeChannelLayout = pipe->moduleInfo_.channelLayout;
    debugInfo.pipeDeviceType = pipe->moduleInfo_.deviceType;
    debugInfo.pipeClassName = pipe->moduleInfo_.className;
    *debugInfo.pipeStreamInfo = pipe->audioStreamInfo_;
    debugInfo.networkId = pipe->moduleInfo_.networkId;
    debugInfo.macAddress = pipe->moduleInfo_.macAddress;
    return SUCCESS;
}

int32_t AudioCoreService::GetStreamDebugInfo(uint32_t sessionId, const std::shared_ptr<AudioPipeInfo> &pipe,
    AudioDebugInfo &debugInfo)
{
    auto streamIter = pipe->streamDescMap_.find(sessionId);
    if (streamIter == pipe->streamDescMap_.end() || streamIter->second == nullptr) {
        return SUCCESS;
    }
    debugInfo.streamDesc = streamIter->second;
    *debugInfo.pipeStreamInfo = streamIter->second->streamInfo_;
    debugInfo.streamParams.samplingRate = static_cast<uint32_t>(streamIter->second->streamInfo_.samplingRate);
    debugInfo.streamParams.encoding = static_cast<uint8_t>(streamIter->second->streamInfo_.encoding);
    debugInfo.streamParams.format = static_cast<uint8_t>(streamIter->second->streamInfo_.format);
    debugInfo.streamParams.channels = static_cast<uint8_t>(streamIter->second->streamInfo_.channels);
    debugInfo.streamParams.channelLayout = static_cast<uint64_t>(streamIter->second->streamInfo_.channelLayout);

    if (debugInfo.pipeRole == PIPE_ROLE_OUTPUT) {
        debugInfo.rendererInfo = streamIter->second->rendererInfo_;
    } else {
        *debugInfo.capturerInfo = streamIter->second->capturerInfo_;
    }

    debugInfo.mainDeviceType = streamIter->second->GetMainNewDeviceType();
    return SUCCESS;
}

int32_t AudioCoreService::GetVolumeDebugInfo(AudioDebugInfo &debugInfo)
{
    if (debugInfo.pipeRole != PIPE_ROLE_OUTPUT) {
        return SUCCESS;
    }
    if (debugInfo.volumeType == STREAM_DEFAULT) {
        debugInfo.volumeType = static_cast<AudioStreamType>(
            VolumeUtils::GetVolumeTypeFromStreamUsage(debugInfo.rendererInfo.streamUsage));
    }
    AudioVolume *audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioVolume != nullptr, ERR_NULL_POINTER, "audioVolume is nullptr");

    VolumeValues volumeValues = {};
    debugInfo.volume = audioVolume->GetVolume(debugInfo.sessionId,
        static_cast<int32_t>(debugInfo.volumeType), debugInfo.pipeId, &volumeValues);
    debugInfo.streamVolume = volumeValues.volumeStream;
    debugInfo.systemVolume = volumeValues.volumePipe;
    return SUCCESS;
}

int32_t AudioCoreService::GetAudioDebugInfoBySessionId(uint32_t sessionId, AudioDebugInfo &audioDebugInfo)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    std::shared_ptr<AudioPipeInfo> pipe = pipeManager_->FindPipeBySessionId(pipeList, sessionId);
    CHECK_AND_RETURN_RET_LOG(pipe != nullptr, ERR_INVALID_PARAM,
        "cannot find pipe by sessionId:%{public}u", sessionId);

    audioDebugInfo.sessionId = sessionId;
    int32_t ret = GetPipeBaseDebugInfo(pipe, audioDebugInfo);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    ret = GetStreamDebugInfo(sessionId, pipe, audioDebugInfo);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    ret = GetVolumeDebugInfo(audioDebugInfo);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    return SUCCESS;
}

uint32_t AudioCoreService::GenerateSessionId()
{
    return AudioStreamIdAllocator::GetAudioStreamIdAllocator().GenerateStreamId();
}

void AudioCoreService::AddSessionId(const uint32_t sessionId)
{
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    AUDIO_INFO_LOG("AddSessionId: %{public}u, callingUid: %{public}u", sessionId, callingUid);
    if (skipAddSessionIdUidSet_.count(callingUid)) {
        // There is no audio stream for the session id of MCU. So no need to save it.
        return;
    }
    std::lock_guard<std::mutex> lock(sessionIdMutex_);
    sessionIdMap_[sessionId] = callingUid;
}

void AudioCoreService::DeleteSessionId(const uint32_t sessionId)
{
    AUDIO_INFO_LOG("DeleteSessionId: %{public}u", sessionId);
    std::lock_guard<std::mutex> lock(sessionIdMutex_);
    if (sessionIdMap_.count(sessionId) == 0) {
        AUDIO_INFO_LOG("The sessionId has been deleted from sessionIdMap_!");
    } else {
        sessionIdMap_.erase(sessionId);
    }
}

void AudioCoreService::ClearMuteVoipState(
    const std::shared_ptr<AudioStreamDescriptor> streamDesc, const uint32_t sessionId)
{
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    if (streamDesc == nullptr) {
        AUDIO_WARNING_LOG("streamDesc is nullptr!");
        // Ensure the sessionId will be cleared.
        streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    } else {
        streamUsage = streamDesc->rendererInfo_.streamUsage;
        sourceType = streamDesc->capturerInfo_.sourceType;
    }

    if (streamUsage != STREAM_USAGE_VOICE_COMMUNICATION &&
        streamUsage != STREAM_USAGE_VIDEO_COMMUNICATION &&
        sourceType != SOURCE_TYPE_VOICE_COMMUNICATION) {
        // It is not a VoIP stream.
        return;
    }
    AUDIO_INFO_LOG("streamUsage %{public}d, sourceType %{public}d, sessionId %{public}u",
        streamUsage, sourceType, sessionId);
    audioVolumeManager_.ClearVoipMuteByStreamId(sessionId);
}

void AudioCoreService::UnmuteDualStreamAfterAlarm(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    CHECK_AND_RETURN(streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_ALARM);
    for (std::pair<uint32_t, AudioStreamType> stream : streamsWhenRingDualOnPrimarySpeaker_) {
        audioPolicyManager_.SetDualStreamVolumeMute(stream.first, false);
    }
    streamsWhenRingDualOnPrimarySpeaker_.clear();
}

void AudioCoreService::ClearRingerFlags(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    CHECK_AND_RETURN(Util::IsRingerOrAlarmerStreamUsage(streamUsage));

    CHECK_AND_RETURN_LOG(!pipeManager_->HasRingerOrAlarmStreamRunning(),
        "other ringer or alarm stream still running, no need to clear flags");

    if (!audioVolumeManager_.IsRingerModeMute()) {
        AUDIO_INFO_LOG("reset ringer mode mute, stream usage:%{public}d", streamUsage);
        audioVolumeManager_.ResetRingerModeMute();
    }

    AUDIO_INFO_LOG("disable primary speaker dual tone flag, no other ringer or alarm stream running");
    isRingDualToneOnPrimarySpeaker_ = false;
}

bool AudioCoreService::IsStreamBelongToUid(const uid_t uid, const uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(sessionIdMutex_);
    if (sessionIdMap_.count(sessionId) == 0) {
        AUDIO_INFO_LOG("The sessionId %{public}u is invalid!", sessionId);
        return false;
    }

    if (sessionIdMap_[sessionId] != uid) {
        AUDIO_INFO_LOG("The sessionId %{public}u does not belong to uid %{public}u!", sessionId, uid);
        return false;
    }

    AUDIO_DEBUG_LOG("The sessionId %{public}u belongs to uid %{public}u!", sessionId, uid);
    return true;
}

bool AudioCoreService::IsValidRenderSessionId(const uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, false, "pipeManager_ is nullptr");
    return pipeManager_->IsValidRenderSessionId(sessionId);
}

int32_t AudioCoreService::SetOffloadAllowedForUid(uint32_t uid, bool allowed)
{
    if (allowed) {
        offloadAllowedUids_.insert(uid);
    } else {
        offloadAllowedUids_.erase(uid);
    }
    AUDIO_INFO_LOG("Set offload allowed uid, uid: %{public}u, allowed: %{public}d", uid, allowed);
    return SUCCESS;
}

void AudioCoreService::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    // Pnp device status update
    audioDeviceStatus_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioCoreService::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    // Bluetooth device status updated
    DeviceType devType = updatedDesc.deviceType_;
    string macAddress = updatedDesc.macAddress_;
    string deviceName = updatedDesc.deviceName_;
    bool isActualConnection = (updatedDesc.connectState_ != VIRTUAL_CONNECTED);
    AUDIO_INFO_LOG("Device connection is actual connection: %{public}d", isActualConnection);

    DeviceStreamInfo audioStreamInfo = updatedDesc.GetDeviceStreamInfo();
    std::set<AudioChannel> channels = audioStreamInfo.GetChannels();
    AudioStreamInfo streamInfo = audioStreamInfo.CheckParams() ?
        AudioStreamInfo(*audioStreamInfo.samplingRate.rbegin(), audioStreamInfo.encoding,
        audioStreamInfo.format, *channels.rbegin()) : AudioStreamInfo();
#ifdef BLUETOOTH_ENABLE
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp device stream info failed!");
    }
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP_IN && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpInDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp input device stream info failed!");
    }
    if (isConnected && isActualConnection
        && devType == DEVICE_TYPE_BLUETOOTH_SCO
        && !audioDeviceManager_.GetScoState()
        && (updatedDesc.deviceCategory_ == BT_HEADPHONE || updatedDesc.deviceCategory_ == BT_GLASSES)) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(macAddress);
    }
#endif
    audioDeviceStatus_.OnDeviceStatusUpdated(updatedDesc, devType,
        macAddress, deviceName, isActualConnection, streamInfo, isConnected);
}

void AudioCoreService::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    // Distributed devices status update
    audioDeviceStatus_.OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioCoreService::MoveStreamSink(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::shared_ptr<AudioPipeInfo> pipeInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::MoveStreamSink");
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.front() != nullptr, "Invalid streamDesc");

    DeviceType oldDeviceType = DEVICE_TYPE_NONE;
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = streamDesc->newDeviceDescs_.front();
    HILOG_COMM_INFO("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s], reason %{public}d",
        streamDesc->sessionId_, newDeviceDesc->deviceType_, GetEncryptAddr(newDeviceDesc->macAddress_).c_str(),
        static_cast<int32_t>(reason));

    std::vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    std::vector<SinkInput> targetSinkInputs = audioOffloadStream_.FilterSinkInputs(streamDesc->sessionId_, sinkInputs);

    auto ret = (newDeviceDesc->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalOutputDevice(targetSinkInputs, pipeInfo, newDeviceDesc)
        : MoveToRemoteOutputDevice(targetSinkInputs, pipeInfo, newDeviceDesc);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Move sink input %{public}d to device %{public}d failed!",
        streamDesc->sessionId_, newDeviceDesc->deviceType_);
    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    streamCollector_.UpdateRendererDeviceInfo(newDeviceDesc);
}

bool AudioCoreService::IsNewDevicePlaybackSupported(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty(), false,
        "invalid streamDesc");
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(newDeviceDesc != nullptr, false, "invalid newDeviceDesc");
    if (streamDesc->streamInfo_.encoding == ENCODING_EAC3 && newDeviceDesc->deviceType_ != DEVICE_TYPE_HDMI &&
        newDeviceDesc->deviceType_ != DEVICE_TYPE_LINE_DIGITAL && audioPolicyServerHandler_) {
        audioPolicyServerHandler_->SendFormatUnsupportedErrorEvent(ERROR_UNSUPPORTED_FORMAT);
        return false;
    }
    return true;
}

/**
 * Determines whether UpdateOutputRoute should be invoked from MoveToNewOutputDevice.
 * Returns false when the audio scene transitions from DEFAULT to RINGING and the stream
 * remains on the same sink, because in that case UpdateOutputRoute is called from
 * StartClient instead of MoveToNewOutputDevice to avoid redundant route updates.
 */
bool AudioCoreService::ShouldUpdateOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AudioScene lastAudioScene = audioSceneManager_.GetLastAudioScene();
    AudioScene audioScene = audioSceneManager_.GetAudioScene();
    bool isSceneChangedFromDefaultToRinging = (lastAudioScene == AUDIO_SCENE_DEFAULT) &&
        (audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING);

    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && !streamDesc->oldDeviceDescs_.empty() &&
        !streamDesc->newDeviceDescs_.empty(), true, "Invalid streamDesc");
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = streamDesc->oldDeviceDescs_.front();
    std::shared_ptr<AudioDeviceDescriptor> newDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET(oldDesc != nullptr && newDesc != nullptr, true);
    std::string oldSinkName = AudioPolicyUtils::GetInstance().GetSinkName(oldDesc, streamDesc->sessionId_);
    std::string newSinkName = AudioPolicyUtils::GetInstance().GetSinkName(newDesc, streamDesc->sessionId_);
    bool isSameSink = oldSinkName == newSinkName;

    return !(reason.IsSetAudioScene() && isSceneChangedFromDefaultToRinging && isSameSink);
}

void AudioCoreService::MoveToNewOutputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::shared_ptr<AudioPipeInfo> pipeInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::MoveToNewOutputDevice");

    DeviceType oldDeviceType = DEVICE_TYPE_NONE;
    bool isNeedTriggerCallback = true;
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = streamDesc->newDeviceDescs_.front();
    std::string oldSinkName = "";
    if (streamDesc->oldDeviceDescs_.size() == 0) {
        HILOG_COMM_INFO("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s], reason %{public}d",
            streamDesc->sessionId_, newDeviceDesc->deviceType_,
            GetEncryptAddr(newDeviceDesc->macAddress_).c_str(), static_cast<int32_t>(reason));
    } else {
        PrepareMoveAttrs(streamDesc, oldDeviceType, isNeedTriggerCallback, oldSinkName, reason);
    }

    std::vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    std::vector<SinkInput> targetSinkInputs = audioOffloadStream_.FilterSinkInputs(streamDesc->sessionId_, sinkInputs);

    if (isNeedTriggerCallback && audioPolicyServerHandler_) {
        std::shared_ptr<AudioDeviceDescriptor> callbackDesc = std::make_shared<AudioDeviceDescriptor>(*newDeviceDesc);
        callbackDesc->descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
        std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc =
            (streamDesc->oldDeviceDescs_.size() > 0 && streamDesc->oldDeviceDescs_.front() != nullptr) ?
            streamDesc->oldDeviceDescs_.front() : streamDesc->newDeviceDescs_.front();
        std::shared_ptr<AudioDeviceDescriptor> preCallbackDesc =
            std::make_shared<AudioDeviceDescriptor>(*oldDeviceDesc);
        preCallbackDesc->descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
        AudioStreamDeviceChangeReasonExt newReason = UpdateRemoteDeviceChangeReason(streamDesc, reason);
        audioPolicyServerHandler_->SendRendererDeviceChangeEvent(streamDesc->callerPid_,
            streamDesc->sessionId_, callbackDesc, newReason, preCallbackDesc);
    }

    SleepForSwitchDevice(streamDesc, reason);

    CHECK_AND_CALL_FUNC_RETURN(IsNewDevicePlaybackSupported(streamDesc),
        HILOG_COMM_ERROR("[MoveToNewOutputDevice]new device not support playback"));
    
    auto pipeDeviceVolumeGroup = AudioPipeManager::GetPipeManager()->GetPipeDeviceVolumeInfoForPipe(pipeInfo);
    AudioAdapterManager::GetInstance().UpdateAudioPipeVolume(pipeDeviceVolumeGroup);

    auto ret = (newDeviceDesc->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalOutputDevice(targetSinkInputs, pipeInfo, newDeviceDesc)
        : MoveToRemoteOutputDevice(targetSinkInputs, pipeInfo, newDeviceDesc);
    CHECK_AND_CALL_FUNC_RETURN_HILOG(ret == SUCCESS,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "MoveToNewOutputDevice failed", false),
        "[MoveToNewOutputDevice]Move sink faild!");

    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    bool shouldUpdateOutputRoute = ShouldUpdateOutputRoute(streamDesc, reason);
    if (pipeManager_->GetUpdateRouteSupport() && shouldUpdateOutputRoute) {
        UpdateOutputRoute(streamDesc, pipeInfo->id_);
    }

    streamCollector_.UpdateRendererDeviceInfo(newDeviceDesc);
}

void AudioCoreService::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    audioDeviceStatus_.OnMicrophoneBlockedUpdate(devType, status);
}

void AudioCoreService::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    audioDeviceStatus_.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreService::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    audioDeviceStatus_.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

int32_t AudioCoreService::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    auto result = audioDeviceStatus_.OnServiceConnected(serviceIndex);
    audioPolicyManager_.SetPrimarySinkExist(pipeManager_->HasPrimarySink());
    return result;
}

void AudioCoreService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter, const std::string &caller)
{
    audioDeviceStatus_.OnForcedDeviceSelected(devType, macAddress, filter, caller);
}


void AudioCoreService::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
    const std::string &caller)
{
    audioDeviceStatus_.OnPrivacyDeviceSelected(devType, macAddress, caller);
}

void AudioCoreService::OnConnectFailed(AudioDeviceDescriptor &desc)
{
    audioDeviceStatus_.OnConnectFailed(desc);
}

void AudioCoreService::OnVehiclePriorityChanged(bool enable)
{
    AUDIO_INFO_LOG("OnVehiclePriorityChanged enable = %{public}d", enable);
    bool changed = audioRouterCenter_.UpdateVehiclePriority(enable);
    CHECK_AND_RETURN_LOG(changed, "vehiclePriority not changed");
    AudioStreamDeviceChangeReasonExt reason{AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE};
    FetchOutputDeviceAndRoute("OnVehiclePriorityChanged", reason);
    FetchInputDeviceAndRoute("OnVehiclePriorityChanged", reason);
}

void AudioCoreService::UpdateSpecialRemoteModuleName(std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &moduleName)
{
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::string className = pipeInfo->moduleInfo_.className;
    CHECK_AND_RETURN(className == "remote_offload" || className == "remote_multichannel");
    moduleName = pipeInfo->moduleInfo_.name;
    AUDIO_INFO_LOG("remote special sink: %{public}s", className.c_str());
}

int32_t AudioCoreService::MoveToRemoteOutputDevice(std::vector<SinkInput> sinkInputIds,
    std::shared_ptr<AudioPipeInfo> pipeInfo,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start for [%{public}zu] sink-inputs", sinkInputIds.size());

    std::string networkId = remoteDeviceDescriptor->networkId_;
    DeviceRole deviceRole = remoteDeviceDescriptor->deviceRole_;
    DeviceType deviceType = remoteDeviceDescriptor->deviceType_;

    // check: networkid
    CHECK_AND_RETURN_RET_LOG(networkId != LOCAL_NETWORK_ID, ERR_INVALID_OPERATION,
        "failed: not a remote device.");

    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);
    UpdateSpecialRemoteModuleName(pipeInfo, moduleName);

    AudioIOHandle moduleId;
    if (audioIOHandleMap_.GetModuleIdByKey(moduleName, moduleId)) {
        (void)moduleId; // mIOHandle is module id, not equal to sink id.
    } else {
        AUDIO_ERR_LOG("no such device.");
        if (!isOpenRemoteDevice) {
            AUDIO_INFO_LOG("directly return");
            return ERR_INVALID_PARAM;
        } else {
            return OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
        }
    }

    // start move.
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, moduleName);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
                ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "MoveToRemoteOutputDevice failed", false),
            "move [%{public}d] failed", sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, moduleName, sinkInputIds[i].pid);
    }

    if (deviceType != DeviceType::DEVICE_TYPE_DEFAULT) {
        AUDIO_WARNING_LOG("Not defult type[%{public}d] on device:[%{public}s]",
            deviceType, GetEncryptStr(networkId).c_str());
    }
    isCurrentRemoteRenderer_ = true;
    return SUCCESS;
}

void AudioCoreService::MoveStreamSource(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    const std::vector<SourceOutput>& sourceOutputs)
{
    Trace trace("AudioCoreService::MoveStreamSource");
    std::vector<SourceOutput> targetSourceOutputs = FilterSourceOutputs(streamDesc->sessionId_, sourceOutputs);

    HILOG_COMM_INFO("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s]",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_,
        GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());

    // MoveSourceOuputByIndexName
    auto ret = (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front(), streamDesc->routeFlag_)
        : MoveToRemoteInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front());
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "Move source output %{public}d to device %{public}d failed!",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_);
    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    streamCollector_.UpdateCapturerDeviceInfo(streamDesc->sessionId_, streamDesc->newDeviceDescs_.front());
}

void AudioCoreService::MoveToNewInputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    const std::vector<SourceOutput>& sourceOutputs, std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    Trace trace("AudioCoreService::MoveToNewInputDevice");
    std::vector<SourceOutput> targetSourceOutputs = FilterSourceOutputs(streamDesc->sessionId_, sourceOutputs);

    if (streamDesc->oldDeviceDescs_.size() == 0) {
        HILOG_COMM_INFO("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s]",
            streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());
    } else {
        HILOG_COMM_INFO("[StreamExecInfo] Move stream %{public}u [%{public}d][%{public}s] to [%{public}d][%{public}s]",
            streamDesc->sessionId_, streamDesc->oldDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->oldDeviceDescs_.front()->macAddress_).c_str(),
            streamDesc->newDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());
    }

    // MoveSourceOuputByIndexName
    auto ret = (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front(), streamDesc->routeFlag_)
        : MoveToRemoteInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front());
    CHECK_AND_CALL_FUNC_RETURN_LOG(ret == SUCCESS,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED, "MoveToNewInputDevice failed", false),
        "Move source output %{public}d to device %{public}d failed!",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_);

    if (pipeManager_->GetUpdateRouteSupport() &&
        streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID) {
        audioActiveDevice_.UpdateActiveDeviceRoute(streamDesc->newDeviceDescs_.front()->deviceType_,
            DeviceFlag::INPUT_DEVICES_FLAG, pipeInfo->id_,
            streamDesc->newDeviceDescs_.front()->networkId_);
    }

    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    streamCollector_.UpdateCapturerDeviceInfo(streamDesc->sessionId_, streamDesc->newDeviceDescs_.front());
}

int32_t AudioCoreService::MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor, uint32_t routeFlag)
{
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_, ERR_INVALID_OPERATION,
        "failed: not a local device.");

    uint32_t sourceId = -1; // invalid source id, use source name instead.
    std::string sourceName = AudioPolicyUtils::GetInstance().GetSourcePortName(localDeviceDescriptor->deviceType_,
        routeFlag, localDeviceDescriptor->isUnidirectional_);
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, sourceName);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
                ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED, "MoveToLocalInputDevice failed", false),
            "move [%{public}d] to local failed", sourceOutputs[i].paStreamId);
    }

    return SUCCESS;
}

int32_t AudioCoreService::MoveToRemoteInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start");

    std::string networkId = remoteDeviceDescriptor->networkId_;
    DeviceRole deviceRole = remoteDeviceDescriptor->deviceRole_;
    DeviceType deviceType = remoteDeviceDescriptor->deviceType_;

    // check: networkid
    CHECK_AND_RETURN_RET_LOG(networkId != LOCAL_NETWORK_ID, ERR_INVALID_OPERATION,
        "failed: not a remote device.");

    uint32_t sourceId = -1; // invalid sink id, use sink name instead.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);

    AudioIOHandle moduleId;
    if (audioIOHandleMap_.GetModuleIdByKey(moduleName, moduleId)) {
        (void)moduleId; // mIOHandle is module id, not equal to sink id.
    } else {
        AUDIO_ERR_LOG("no such device.");
        if (!isOpenRemoteDevice) {
            return ERR_INVALID_PARAM;
        } else {
            return OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
        }
    }

    // start move.
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, moduleName);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
                ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED, "MoveToRemoteInputDevice failed", false),
            "move [%{public}d] failed", sourceOutputs[i].paStreamId);
    }

    if (deviceType != DeviceType::DEVICE_TYPE_DEFAULT) {
        AUDIO_DEBUG_LOG("Not defult type[%{public}d] on device:[%{public}s]",
            deviceType, GetEncryptStr(networkId).c_str());
    }
    return SUCCESS;
}

int32_t AudioCoreService::OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("[PipeExecInfo] open remote pipe device %{public}d", deviceType);
    // open the test device. We should open it when device is online.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);
    AudioModuleInfo remoteDeviceInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
        deviceRole, deviceType);

    auto ret = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_REMOTE, networkId);
    if (ret) {
        AUDIO_ERR_LOG("load adapter fail");
    }
    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleName, remoteDeviceInfo);

    // If device already in list, remove it else do not modify the list.
    audioConnectedDevice_.DelConnectedDevice(networkId, deviceType);
    AudioPolicyUtils::GetInstance().UpdateDisplayName(remoteDeviceDescriptor);
    audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioMicrophoneDescriptor_.AddMicrophoneDescriptor(remoteDeviceDescriptor);
    return SUCCESS;
}

inline std::string PrintSourceOutput(SourceOutput sourceOutput)
{
    std::stringstream value;
    value << "streamId:[" << sourceOutput.streamId << "] ";
    value << "streamType:[" << sourceOutput.streamType << "] ";
    value << "uid:[" << sourceOutput.uid << "] ";
    value << "pid:[" << sourceOutput.pid << "] ";
    value << "statusMark:[" << sourceOutput.statusMark << "] ";
    value << "deviceSourceId:[" << sourceOutput.deviceSourceId << "] ";
    value << "startTime:[" << sourceOutput.startTime << "]";
    return value.str();
}

std::vector<SourceOutput> AudioCoreService::FilterSourceOutputs(int32_t sessionId,
    const std::vector<SourceOutput>& sourceOutputs)
{
    std::vector<SourceOutput> targetSourceOutputs = {};

    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        AUDIO_DEBUG_LOG("sourceOutput[%{public}zu]:%{public}s", i, PrintSourceOutput(sourceOutputs[i]).c_str());
        if (sessionId == sourceOutputs[i].streamId) {
            targetSourceOutputs.push_back(sourceOutputs[i]);
        }
    }
    return targetSourceOutputs;
}

std::vector<SourceOutput> AudioCoreService::GetSourceOutputs()
{
    std::vector<SourceOutput> sourceOutputs;
    sourceOutputs = audioPolicyManager_.GetAllSourceOutputs();
    return sourceOutputs;
}

void AudioCoreService::UpdateRingerOrAlarmerDualDeviceOutputRouter(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, const uint32_t pipeId)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.front() != nullptr, "streamDesc is nullptr");
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    InternalDeviceType deviceType = streamDesc->newDeviceDescs_.front()->deviceType_;
    if (!SelectRingerOrAlarmDevices(streamDesc, pipeId)) {
        audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG,
            pipeId, streamDesc->newDeviceDescs_.front()->networkId_);
    }
    if (streamUsage == STREAM_USAGE_ALARM) {
        shouldUpdateDeviceDueToDualTone_ = true;
        return;
    }
    AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
    if (ringerMode != RINGER_MODE_NORMAL &&
        IsRingerOrAlarmerDualDevicesRange(streamDesc->newDeviceDescs_.front()->getType()) &&
        streamDesc->newDeviceDescs_.front()->getType() != DEVICE_TYPE_SPEAKER) {
        audioPolicyManager_.SetDeviceNoMuteForRinger(streamDesc->newDeviceDescs_.front());
        audioVolumeManager_.SetRingerModeMute(false);
        if (audioVolumeManager_.GetSystemVolumeLevelNoMuteStateInterface(STREAM_RING) <
            audioVolumeManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE) {
            audioVolumeManager_.SetDoubleRingVolumeDb(STREAM_RING,
                audioVolumeManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE);
        }
    } else {
        audioPolicyManager_.ClearDeviceNoMuteForRinger();
        audioVolumeManager_.SetRingerModeMute(true);
    }
    shouldUpdateDeviceDueToDualTone_ = true;
}

bool AudioCoreService::IsDupDeviceChange(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is nullptr");

    if (streamDesc->oldDupDeviceDescs_.size() != streamDesc->newDupDeviceDescs_.size()) {
        return true;
    }

    if (streamDesc->newDupDeviceDescs_.size() == 0) {
        return false;
    }

    if (streamDesc->newDupDeviceDescs_.front() != nullptr &&
        streamDesc->newDupDeviceDescs_.front()->IsSameDeviceDescPtr(streamDesc->oldDupDeviceDescs_.front()) == false) {
        return true;
    }

    return false;
}

void AudioCoreService::UpdateDupDeviceOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    if (streamDesc->newDupDeviceDescs_.size() != 0) {
        std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkName(
            streamDesc->newDupDeviceDescs_.front(), streamDesc->sessionId_);
        UpdateDualToneState(true, streamDesc->sessionId_, sinkName);
        shouldUpdateDeviceDueToDualTone_ = true;
    } else if (streamDesc->oldDupDeviceDescs_.size() != 0) {
        UpdateDualToneState(false, streamDesc->sessionId_);
    }
}

void AudioCoreService::UpdateOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc, const uint32_t pipeId)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.front() != nullptr, "streamDesc is nullptr");
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    InternalDeviceType deviceType = streamDesc->newDeviceDescs_.front()->deviceType_;
    AUDIO_DEBUG_LOG("[PipeExecInfo] Update route streamUsage:%{public}d, devicetype:[%{public}s]",
        streamUsage, streamDesc->GetNewDevicesTypeString().c_str());

    if (Util::IsEnhancedRoutingSupported()) {
        AudioScene pipeScene = CalculatePipeAudioScene(pipeId);
        AudioServerProxy::GetInstance().SetAudioSceneForPipeProxy(pipeId, pipeScene, false);
        AUDIO_INFO_LOG("Set AudioScene %{public}d for pipe %{public}u in multi-device mode", pipeScene, pipeId);
    }
    shouldUpdateDeviceDueToDualTone_ = false;
    if (Util::IsRingerOrAlarmerStreamUsage(streamUsage) && IsRingerOrAlarmerDualDevicesRange(deviceType) &&
        !VolumeUtils::IsPCVolumeEnable()) {
        HandleRingerOrAlarmerRoute(streamDesc, deviceType, pipeId, streamUsage);
    } else if (AudioCollaborativeService::GetAudioCollaborativeService().GetRealCollaborativeState() &&
        (streamDesc->isNearLinkCollaborationEnable_ || deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ||
        CheckCollaborativeStreamRunning())) {
        // for collaboration, the route should be updated
        UpdateRouteForCollaboration(deviceType, pipeId);
    } else {
        HandleNormalOrDualToneRoute(streamDesc, deviceType, pipeId, streamUsage);
    }
}

void AudioCoreService::HandleRingerOrAlarmerRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    InternalDeviceType deviceType, const uint32_t pipeId, StreamUsage streamUsage)
{
    if (!streamDesc->newDeviceDescs_.front()->isSupportWearDetectionEnable_ ||
        streamUsage == STREAM_USAGE_ALARM) {
        UpdateRingerOrAlarmerDualDeviceOutputRouter(streamDesc, pipeId);
        return;
    }

    AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
    std::shared_ptr<AudioDeviceDescriptor> frontDevice = streamDesc->newDeviceDescs_.front();
    if (ringerMode != RINGER_MODE_NORMAL &&
        IsRingerOrAlarmerDualDevicesRange(frontDevice->getType()) &&
        frontDevice->getType() != DEVICE_TYPE_SPEAKER) {
        audioPolicyManager_.SetDeviceNoMuteForRinger(frontDevice);
        audioVolumeManager_.SetRingerModeMute(false);
        if (audioVolumeManager_.GetSystemVolumeLevelNoMuteStateInterface(STREAM_RING) <
            audioVolumeManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE) {
            audioVolumeManager_.SetDoubleRingVolumeDb(STREAM_RING,
                audioVolumeManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE);
        }
    } else {
        audioPolicyManager_.ClearDeviceNoMuteForRinger();
        audioVolumeManager_.SetRingerModeMute(true);
    }
    audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG,
        pipeId, streamDesc->newDeviceDescs_.front()->networkId_);
}

void AudioCoreService::HandleNormalOrDualToneRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    InternalDeviceType deviceType, const uint32_t pipeId, StreamUsage streamUsage)
{
    CHECK_AND_RETURN_LOG(streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID,
        "remote device no need to update route");
    if (isRingDualToneOnPrimarySpeaker_ && streamUsage != STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
        activeDevices.push_back(make_pair(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG));
        activeDevices.push_back(make_pair(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG));
        audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices, pipeId);
        AUDIO_INFO_LOG("Update desc [%{public}d] with speaker on session [%{public}d]",
            deviceType, streamDesc->sessionId_);
        AudioStreamType streamType = streamCollector_.GetStreamType(streamDesc->sessionId_);
        if (!AudioCoreServiceUtils::IsDualStreamWhenRingDual(streamType) &&
            pipeManager_ != nullptr && pipeManager_->IsOnPrimaryAdapter(streamDesc->sessionId_)) {
            streamsWhenRingDualOnPrimarySpeaker_.push_back(make_pair(streamDesc->sessionId_, streamType));
            audioPolicyManager_.SetDualStreamVolumeMute(streamDesc->sessionId_, true);
        }
        shouldUpdateDeviceDueToDualTone_ = true;
    } else {
        audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG,
            pipeId, streamDesc->newDeviceDescs_.front()->networkId_);
    }
}

AudioScene AudioCoreService::CalculatePipeAudioScene(uint32_t pipeId)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = nullptr;
    auto pipeList = pipeManager_->GetPipeList();
    for (auto &pipe : pipeList) {
        if (pipe != nullptr && pipe->id_ == pipeId) {
            pipeInfo = pipe;
            break;
        }
    }
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, AUDIO_SCENE_DEFAULT, "Pipe %{public}u not found", pipeId);

    AudioScene highestScene = AUDIO_SCENE_DEFAULT;
    ScoAudioScenePriority highestPriority = SCO_PRIORITY_DEFAULT;
    for (auto &streamDesc : pipeInfo->streamDescriptors_) {
        if (streamDesc == nullptr || streamDesc->streamStatus_ != STREAM_STATUS_STARTED) {
            continue;
        }
        AudioScene streamScene = AUDIO_SCENE_DEFAULT;
        bool isRecognition = false;
        if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
            StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
            if (streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
                streamScene = AUDIO_SCENE_PHONE_CALL;
            } else if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
                       streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
                streamScene = AUDIO_SCENE_PHONE_CHAT;
            } else if (streamUsage == STREAM_USAGE_VOICE_RINGTONE ||
               streamUsage == STREAM_USAGE_VOICE_TRANSFER_RINGTONE) {
                streamScene = AUDIO_SCENE_VOICE_RINGING;
            } else if (streamUsage == STREAM_USAGE_RINGTONE) {
                streamScene = AUDIO_SCENE_RINGING;
            }
        } else if (streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
            SourceType sourceType = streamDesc->capturerInfo_.sourceType;
            isRecognition = (sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
                             sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION);
        }
        ScoAudioScenePriority streamPriority = ScoAudioSceneManager::GetInstance().GetAudioScenePriority(
            streamScene, isRecognition);
        if (streamPriority < highestPriority) {
            highestPriority = streamPriority;
            highestScene = streamScene;
        }
    }
    AUDIO_INFO_LOG("Pipe %{public}u highest AudioScene %{public}d, priority %{public}d",
        pipeId, highestScene, highestPriority);
    return highestScene;
}

void AudioCoreService::OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor,
    const AudioStreamDeviceChangeReason reason)
{
    AUDIO_INFO_LOG("In");
    Trace trace("AudioCoreService::OnPreferredOutputDeviceUpdated:" + std::to_string(deviceDescriptor.deviceType_));

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredOutputDeviceUpdated();
        audioPolicyServerHandler_->SendAudioSessionDeviceChange(reason);
    }
    if (deviceDescriptor.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
        spatialDeviceMap_.insert(make_pair(deviceDescriptor.macAddress_, deviceDescriptor.deviceType_));
    }

    if (deviceDescriptor.macAddress_ !=
        AudioSpatializationService::GetAudioSpatializationService().GetCurrentDeviceAddress()) {
        AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(false);
    }
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(deviceDescriptor.deviceType_);
    AudioSpatializationService::GetAudioSpatializationService().UpdateCurrentDevice(deviceDescriptor);
    AudioCollaborativeService::GetAudioCollaborativeService().UpdateCurrentDevice(deviceDescriptor);
}

void AudioCoreService::OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId,
    const AudioStreamDeviceChangeReason reason)
{
    AUDIO_INFO_LOG("OnPreferredInputDeviceUpdated Start");

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredInputDeviceUpdated();
        audioPolicyServerHandler_->SendAudioSessionInputDeviceChange(reason);
    }
}


bool AudioCoreService::IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
        case DEVICE_TYPE_NEARLINK:
        case DEVICE_TYPE_HEARING_AID:
            return true;
        default:
            return false;
    }
}

void AudioCoreService::ClearRingMuteWhenCallStart(bool pre, bool after,
    std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(pre == true && after == false, "ringdual not cancel by call");
    AUDIO_INFO_LOG("disable primary speaker dual tone when call start and ring not over");
    for (std::pair<uint32_t, AudioStreamType> stream : streamsWhenRingDualOnPrimarySpeaker_) {
        audioPolicyManager_.SetDualStreamVolumeMute(stream.first, false);
    }
    streamsWhenRingDualOnPrimarySpeaker_.clear();
    AudioStreamType streamType = streamCollector_.GetStreamType(streamDesc->GetSessionId());
    if (streamType == STREAM_MUSIC) {
        audioPolicyManager_.SetDualStreamVolumeMute(streamDesc->GetSessionId(), false);
    }
}

bool AudioCoreService::GetRingerOrAlarmerDualDevices(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::vector<std::pair<InternalDeviceType, DeviceFlag>> &activeDevices)
{
    bool allDevicesInDualDevicesRange = true;

    for (size_t i = 0; i < streamDesc->newDeviceDescs_.size(); i++) {
        if (IsRingerOrAlarmerDualDevicesRange(streamDesc->newDeviceDescs_[i]->deviceType_)) {
            activeDevices.push_back(make_pair(streamDesc->newDeviceDescs_[i]->deviceType_,
            DeviceFlag::OUTPUT_DEVICES_FLAG));
            AUDIO_INFO_LOG("select ringer/alarm devices devicetype[%{public}zu]:%{public}d",
                i, streamDesc->newDeviceDescs_[i]->deviceType_);
        } else {
            allDevicesInDualDevicesRange = false;
            break;
        }
    }

    return allDevicesInDualDevicesRange;
}

bool AudioCoreService::SelectRingerOrAlarmDevices(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    const uint32_t pipeId)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT, false,
        "audio devices not in range for ringer or alarmer.");
    const int32_t sessionId = static_cast<int32_t>(streamDesc->sessionId_);
    const StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
    bool allDevicesInDualDevicesRange = GetRingerOrAlarmerDualDevices(streamDesc, activeDevices);

    AUDIO_INFO_LOG("select ringer/alarm sessionId:%{public}d, streamUsage:%{public}d", sessionId, streamUsage);
    if (!streamDesc->newDeviceDescs_.empty() && allDevicesInDualDevicesRange) {
        if (streamDesc->newDeviceDescs_.size() == AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT &&
            AudioPolicyUtils::GetInstance().GetSinkName(*streamDesc->newDeviceDescs_.front(), sessionId) !=
            AudioPolicyUtils::GetInstance().GetSinkName(*streamDesc->newDeviceDescs_.back(), sessionId)) {
            AUDIO_INFO_LOG("set dual hal tone, reset primary sink to default before.");
            audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG,
                pipeManager_->QueryPipeIdBySessionId(sessionId));
            if (enableDualHalToneState_ && enableDualHalToneSessionId_ != sessionId) {
                AUDIO_INFO_LOG("sesion changed, disable old dual hal tone.");
                UpdateDualToneState(false, enableDualHalToneSessionId_);
            }
            CHECK_AND_RETURN_RET_LOG(AudioCoreServiceUtils::NeedDualHalToneInStatus(
                audioPolicyManager_.GetRingerMode(), streamUsage,
                VolumeUtils::IsPCVolumeEnable(), audioVolumeManager_.GetStreamMute(STREAM_MUSIC)),
                false, "no normal ringer mode and no alarm, dont dual hal tone.");
            UpdateDualToneState(true, sessionId);
        } else {
            bool pre = isRingDualToneOnPrimarySpeaker_;
            isRingDualToneOnPrimarySpeaker_ = AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(
                streamDesc->newDeviceDescs_, sessionId);
            if (((isRingDualToneOnPrimarySpeaker_ == false && streamDesc->newDupDeviceDescs_.size() == 0) ||
                isRingDualToneOnPrimarySpeaker_ == true) &&
                enableDualHalToneState_ && enableDualHalToneSessionId_ == sessionId) {
                AUDIO_INFO_LOG("device unavailable, disable dual hal tone.");
                UpdateDualToneState(false, enableDualHalToneSessionId_);
            }
            ClearRingMuteWhenCallStart(pre, isRingDualToneOnPrimarySpeaker_, streamDesc);
            if (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID) {
                audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices, pipeId);
            }
        }
        return true;
    }
    return false;
}

void AudioCoreService::UpdateDualToneState(const bool &enable, const int32_t &sessionId, const std::string &dupSinkName)
{
    AUDIO_INFO_LOG("Update dual tone state, enable:%{public}d, sessionId:%{public}d", enable, sessionId);
    enableDualHalToneState_ = enable;
    if (enableDualHalToneState_) {
        enableDualHalToneSessionId_ = sessionId;
    }
    Trace trace("AudioDeviceCommon::UpdateDualToneState sessionId:" + std::to_string(sessionId));
    auto ret = AudioServerProxy::GetInstance().UpdateDualToneStateProxy(enable, sessionId, dupSinkName);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the dual tone state for sessionId:%{public}d", sessionId);
}

int32_t AudioCoreService::MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
    std::shared_ptr<AudioPipeInfo> pipeInfo, std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor)
{
    // check
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_,
        ERR_INVALID_OPERATION, "failed: not a local device.");

    // start move.
    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    std::vector<uint64_t> sessionIdForLog{};
    std::string sinkName = localDeviceDescriptor->deviceType_ == DEVICE_TYPE_REMOTE_CAST ?
            "RemoteCastInnerCapturer" : pipeInfo->moduleInfo_.name;
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        if (sinkName == BLUETOOTH_SPEAKER) {
            std::string activePort = BLUETOOTH_SPEAKER;
            audioPolicyManager_.SuspendAudioDevice(activePort, false);
        }
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, sinkName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sinkInputIds[i].streamId);
        sessionIdForLog.push_back(sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, LOCAL_NETWORK_ID, sinkInputIds[i].pid);
    }
    isCurrentRemoteRenderer_ = false;
    CHECK_AND_RETURN_RET(!sessionIdForLog.empty(), SUCCESS);
    std::stringstream logstream;
    for (auto sessionid : sessionIdForLog) {
        logstream << sessionid;
        logstream << ", ";
    }
    std::string result = logstream.str();
    result.pop_back();
    AUDIO_INFO_LOG("sinkName %{public}s, streamId %{public}s", sinkName.c_str(), result.c_str());
    return SUCCESS;
}

bool AudioCoreService::HasLowLatencyCapability(DeviceType deviceType, bool isRemote)
{
    // Distributed devices are low latency devices
    if (isRemote) {
        return true;
    }

    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
        case DeviceType::DEVICE_TYPE_WIRED_MICIN:
        case DeviceType::DEVICE_TYPE_DP:
            return true;

        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            return false;
        default:
            return false;
    }
}

void AudioCoreService::TriggerRecreateRendererStreamCallback(shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::TriggerRecreateRendererStreamCallback");
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is null");
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is null");
    int32_t callerPid = streamDesc->callerPid_;
    int32_t sessionId = streamDesc->sessionId_;
    uint32_t routeFlag = streamDesc->routeFlag_;

    CHECK_AND_RETURN_LOG(streamDesc->oldDeviceDescs_.size() > 0 && streamDesc->oldDeviceDescs_.front() != nullptr,
        "oldDeviceDesc is invalid");
    CHECK_AND_RETURN_LOG(streamDesc->newDeviceDescs_.size() > 0 && streamDesc->newDeviceDescs_.front() != nullptr,
        "newDeviceDesc is invalid");
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = streamDesc->oldDeviceDescs_.front();
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = streamDesc->newDeviceDescs_.front();
    if (!oldDeviceDesc->IsSameDeviceDesc(newDeviceDesc)) {
        std::shared_ptr<AudioDeviceDescriptor> callbackDesc = std::make_shared<AudioDeviceDescriptor>(newDeviceDesc);
        callbackDesc->descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
        std::shared_ptr<AudioDeviceDescriptor> preCallbackDesc = std::make_shared<AudioDeviceDescriptor>(oldDeviceDesc);
        preCallbackDesc->descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
        audioPolicyServerHandler_->SendRendererDeviceChangeEvent(
            callerPid, sessionId, callbackDesc, reason, preCallbackDesc);
    }

    SleepForSwitchDevice(streamDesc, reason);

    HILOG_COMM_INFO("[TriggerRecreateRendererStreamCallback]Trigger recreate renderer stream %{public}u, pid: "
        "%{public}d, routeflag: 0x%{public}x", sessionId, callerPid, routeFlag);

    pipeManager_->AddSingleSwitchStream(sessionId, streamDesc);

    audioPolicyServerHandler_->SendRecreateRendererStreamEvent(callerPid, sessionId, routeFlag, reason);
}

void AudioCoreService::TriggerRecreateRendererStreamCallbackEntry(shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    TriggerRecreateRendererStreamCallback(streamDesc, reason);
}

CapturerState AudioCoreService::HandleStreamStatusToCapturerState(AudioStreamStatus status)
{
    switch (status) {
        case STREAM_STATUS_NEW:
            return CAPTURER_PREPARED;
        case STREAM_STATUS_STARTED:
            return CAPTURER_RUNNING;
        case STREAM_STATUS_PAUSED:
            return CAPTURER_PAUSED;
        case STREAM_STATUS_STOPPED:
            return CAPTURER_STOPPED;
        case STREAM_STATUS_RELEASED:
            return CAPTURER_RELEASED;
        default:
            return CAPTURER_INVALID;
    }
}

void AudioCoreService::TriggerRecreateCapturerStreamCallback(shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    Trace trace("AudioCoreService::TriggerRecreateCapturerStreamCallback");
    AUDIO_INFO_LOG("Trigger recreate capturer stream %{public}d, pid: %{public}d, routeflag: 0x%{public}x",
        streamDesc->sessionId_, streamDesc->callerPid_, streamDesc->routeFlag_);

    if (audioPolicyServerHandler_ != nullptr) {
        pipeManager_->AddSingleSwitchStream(streamDesc->sessionId_, streamDesc);
        audioPolicyServerHandler_->SendRecreateCapturerStreamEvent(streamDesc->appInfo_.appPid,
            streamDesc->sessionId_, streamDesc->routeFlag_, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    } else {
        AUDIO_WARNING_LOG("No audio policy server handler");
    }
}

uint32_t AudioCoreService::OpenNewAudioPortAndRoute(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex)
{
    uint32_t id = OPEN_PORT_FAILURE;
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr && pipeInfo->streamDescriptors_.size() > 0 &&
        pipeInfo->streamDescriptors_.front() != nullptr, OPEN_PORT_FAILURE, "pipeInfo is invalid");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeInfo->streamDescriptors_[0];
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, OPEN_PORT_FAILURE, "invalid streamDesc");
    if (streamDesc->routeFlag_ & AUDIO_OUTPUT_FLAG_HWDECODING) {
        AUDIO_INFO_LOG("[PipeExecInfo] hwdecoding type do not need open pipe");
        id = streamDesc->sessionId_;
    } else if (streamDesc->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        AUDIO_INFO_LOG("[PipeExecInfo] remote cast device do not need open pipe");
        id = streamDesc->sessionId_;
    } else {
        if (pipeInfo->moduleInfo_.name == BLUETOOTH_MIC &&
            streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
            shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_[0];
            audioActiveDevice_.SetActiveBtInDeviceMac(desc->macAddress_);
            bool ret = audioActiveDevice_.GetActiveA2dpInDeviceStreamInfo(streamDesc->streamInfo_);
            CHECK_AND_RETURN_RET_LOG(ret, OPEN_PORT_FAILURE, "invalid streamDesc");
            SourceType sourceType = streamDesc->capturerInfo_.sourceType;
            audioA2dpDevice_.GetA2dpModuleInfo(pipeInfo->moduleInfo_, streamDesc->streamInfo_);
        }
        HandleCommonSourceOpened(pipeInfo);
        id = audioPolicyManager_.OpenAudioPort(pipeInfo, paIndex);

        AUDIO_INFO_LOG("routeFlag:%{public}d", pipeInfo->routeFlag_);
    }

    CHECK_AND_RETURN_RET_LOG(id != HDI_INVALID_ID, OPEN_PORT_FAILURE,
        "OpenAudioPort failed ioHandle[%{public}u]", id);
    audioIOHandleMap_.AddIOHandleInfo(pipeInfo->moduleInfo_.name, id);
    HILOG_COMM_INFO("[PipeExecInfo] Get HDI id: %{public}u, paIndex %{public}u", id, paIndex);
    return id;
}

int32_t AudioCoreService::GetRealUid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, -1, "Stream desc is nullptr");
    if (streamDesc->callerUid_ == MEDIA_SERVICE_UID) {
        return streamDesc->appInfo_.appUid;
    }
    return streamDesc->callerUid_;
}

int32_t AudioCoreService::GetRealPid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, -1, "Stream desc is nullptr");
    if (streamDesc->callerUid_ == MEDIA_SERVICE_UID) {
        return streamDesc->appInfo_.appPid;
    }
    return streamDesc->callerPid_;
}

void AudioCoreService::UpdateRendererInfoWhenNoPermission(
    const shared_ptr<AudioRendererChangeInfo> &audioRendererChangeInfos, bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioRendererChangeInfos->clientUID = 0;
        audioRendererChangeInfos->rendererState = RENDERER_INVALID;
    }
}

void AudioCoreService::UpdateCapturerInfoWhenNoPermission(
    const shared_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfos, bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioCapturerChangeInfos->clientUID = 0;
        audioCapturerChangeInfos->capturerState = CAPTURER_INVALID;
    }
}

void AudioCoreService::SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId)
{
    if ((rendererState == RENDERER_RUNNING) && (audioA2dpOffloadManager_ != nullptr) &&
        !audioA2dpOffloadManager_->IsA2dpOffloadConnecting(sessionId)) {
        AUDIO_DEBUG_LOG("Notify client not to block.");
        std::thread sendConnectedToClient(&AudioCoreService::UpdateSessionConnectionState, this, sessionId,
            DATA_LINK_CONNECTED);
        sendConnectedToClient.detach();
    }
}

void AudioCoreService::UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state)
{
    AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionID, state);
}

void AudioCoreService::UpdateTrackerDeviceChange(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    AUDIO_INFO_LOG("Start");

    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    for (std::shared_ptr<AudioDeviceDescriptor> deviceDesc : desc) {
        if (deviceDesc->deviceRole_ == OUTPUT_DEVICE) {
            DeviceType type = curOutputDeviceType;
            std::string macAddress = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();
            auto itr = audioConnectedDevice_.CheckExistOutputDevice(type, macAddress);
            if (itr != nullptr) {
                AudioDeviceDescriptor outputDevice(AudioDeviceDescriptor::DEVICE_INFO);
                audioDeviceCommon_.UpdateDeviceInfo(outputDevice, itr, true, true);
                streamCollector_.UpdateTracker(AUDIO_MODE_PLAYBACK, outputDevice);
            }
        }
        if (deviceDesc->deviceRole_ == INPUT_DEVICE) {
            DeviceType type = audioActiveDevice_.GetCurrentInputDeviceType();
            auto itr = audioConnectedDevice_.CheckExistInputDevice(type);
            if (itr != nullptr) {
                AudioDeviceDescriptor inputDevice(AudioDeviceDescriptor::DEVICE_INFO);
                audioDeviceCommon_.UpdateDeviceInfo(inputDevice, itr, true, true);
                audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(itr->deviceType_);
                streamCollector_.UpdateTracker(AUDIO_MODE_RECORD, inputDevice);
            }
        }
    }
}

void AudioCoreService::StoreDistributedRoutingRoleInfo(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    distributedRoutingInfo_.descriptor = descriptor;
    distributedRoutingInfo_.type = type;
}

int32_t AudioCoreService::GetSystemVolumeLevel(AudioStreamType streamType)
{
    return audioVolumeManager_.GetSystemVolumeLevel(streamType);
}

float AudioCoreService::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType) const
{
    return audioPolicyManager_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

bool AudioCoreService::CheckOffloadPipeUidAllowed(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    int32_t realUid) const
{
    if (realUid == AUDIO_EXT_UID) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "the extra uid not support offload.");
        return false;
    }
    if (!pipeManager_->NeedUidCheckForOffloadPipe(streamDesc)) {
        return true;
    }
    if (IsOffloadAllowedForUid(realUid)) {
        return true;
    }
    JUDGE_AND_INFO_LOG(isCreateProcess_, "uid %{public}d is not allowed for uid-gated offload pipe.", realUid);
    return false;
}

bool AudioCoreService::IsStreamSupportLowpower(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportLowpower");
    if (!streamDesc->rendererInfo_.isOffloadAllowed) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream because renderInfo not support offload.");
        return false;
    }
    int32_t realUid = GetRealUid(streamDesc);
    CHECK_AND_RETURN_RET(CheckOffloadPipeUidAllowed(streamDesc, realUid), false);
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    if (streamDesc->streamInfo_.channels > STEREO &&
        (streamUsage != STREAM_USAGE_MOVIE || streamDesc->rendererInfo_.originalFlag != AUDIO_FLAG_PCM_OFFLOAD)) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream because channels.");
        return false;
    }

    if (streamUsage != STREAM_USAGE_MUSIC && streamUsage != STREAM_USAGE_AUDIOBOOK &&
        (streamUsage != STREAM_USAGE_MOVIE || streamDesc->rendererInfo_.originalFlag != AUDIO_FLAG_PCM_OFFLOAD)) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream because streamUsage.");
        return false;
    }

    if (streamDesc->rendererInfo_.playerType == PLAYER_TYPE_SOUND_POOL ||
        streamDesc->rendererInfo_.playerType == PLAYER_TYPE_OPENSL_ES) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream beacuse playerType %{public}d.",
            streamDesc->rendererInfo_.playerType);
        return false;
    }

    AudioSpatializationState spatialState =
        AudioSpatializationService::GetAudioSpatializationService().GetSpatializationState();
    bool effectOffloadFlag = AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
    if (spatialState.spatializationEnabled && !effectOffloadFlag) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "spatialization effect in arm, Skipped.");
        return false;
    }

    if (AudioCollaborativeService::GetAudioCollaborativeService().IsCollaborativePlaybackEnabledForDevice(
        streamDesc->GetMainNewDeviceDesc()) &&
        audioPolicyManager_.IsCollaborationSupportedForStreamUsage(
            streamDesc->GetMainNewDeviceType(), streamDesc->GetRenderUsage())) {
        return false;
    }

    // LowPower: Speaker, USB headset, a2dp offload, Nearlink
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_SPEAKER &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_USB_HEADSET &&
        (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP ||
        streamDesc->newDeviceDescs_[0]->a2dpOffloadFlag_ != A2DP_OFFLOAD) &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_NEARLINK) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream, deviceType: %{public}d",
            streamDesc->newDeviceDescs_[0]->deviceType_);
        return false;
    }
    return true;
}

bool AudioCoreService::IsOffloadAllowedForUid(uint32_t uid) const
{
    return offloadAllowedUids_.find(uid) != offloadAllowedUids_.end();
}

int32_t AudioCoreService::HandleFetchOutputWhenNoRunningStream(const AudioStreamDeviceChangeReasonExt reason)
{
    FetchDeviceInfo info = { STREAM_USAGE_MEDIA, "HandleFetchOutputWhenNoRunningStream" };
    AudioScene audioScene = audioSceneManager_.GetAudioScene(true);
    if (!Util::IsEnhancedRoutingSupported()) {
        if (audioScene == AUDIO_SCENE_PHONE_CALL) {
            info.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
        } else if (audioScene == AUDIO_SCENE_PHONE_CHAT) {
            info.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        }
    }
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioRouterCenter_.FetchOutputDevices(info);
    CHECK_AND_RETURN_RET_LOG(!descs.empty(), ERROR, "descs is empty");
    AudioDeviceDescriptor tmpOutputDeviceDesc = audioRouterSelectStrategy_.Get1stCurrentOutputDevice();
    if (descs.front()->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(descs.front(), tmpOutputDeviceDesc)) {
        AUDIO_DEBUG_LOG("output device is not change");
        return SUCCESS;
    }
    OnRemoteDeviceStatusUpdatedWhenNoRunningStream(descs.front());
    audioRouterSelectStrategy_.UpdateCurrentOutputDevice(SYSTEM_UID, descs);
    AUDIO_DEBUG_LOG("currentActiveDevice %{public}d", audioActiveDevice_.GetCurrentOutputDeviceType());
    bool enableSetVoiceCallVolume = !IsDeviceSwitching(reason);
    audioPolicyManager_.UpdateVolumeForAllPipes();
    audioVolumeManager_.SetVoiceCallVolumeOnDeviceSwitch(*descs.front(), enableSetVoiceCallVolume);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_ = descs;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    ActivateOutputDevice(streamDesc, reason);
    SyncVolWhenCurrentOutputDeviceChanged(descs.front());

    if (audioSceneManager_.GetAudioScene(true) != AUDIO_SCENE_DEFAULT) {
        audioActiveDevice_.UpdateActiveDeviceRoute(descs.front()->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG,
            pipeManager_->QueryPipeIdBySessionId(streamDesc->sessionId_), descs.front()->networkId_);
    }
    OnPreferredOutputDeviceUpdated(audioRouterSelectStrategy_.Get1stCurrentOutputDevice(), reason);
    return SUCCESS;
}

int32_t AudioCoreService::HandleFetchInputWhenNoRunningStream()
{
    std::shared_ptr<AudioDeviceDescriptor> desc;
    AudioDeviceDescriptor tempDesc = audioRouterSelectStrategy_.Get1stCurrentInputDevice(SYSTEM_UID);
    if (tempDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && Bluetooth::AudioHfpManager::IsRecognitionStatus()) {
        FetchDeviceInfo info;
        info.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
        info.clientUID = -1;
        desc = audioRouterCenter_.FetchInputDevice(info);
    } else {
        FetchDeviceInfo info;
        info.sourceType = SOURCE_TYPE_MIC;
        info.clientUID = -1;
        desc = audioRouterCenter_.FetchInputDevice(info);
    }
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERROR, "desc is nullptr");

    if (desc->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(desc, tempDesc)) {
        AUDIO_DEBUG_LOG("input device is not change");
        return SUCCESS;
    }
    audioRouterSelectStrategy_.UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    if (desc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        PresetArmIdleInput(desc);
    }
    DeviceType deviceType = audioActiveDevice_.GetCurrentInputDeviceType(DEFAULT_UID);
    AUDIO_DEBUG_LOG("currentActiveInputDevice update %{public}d", deviceType);
    OnPreferredInputDeviceUpdated(deviceType, ""); // networkId is not used
    return SUCCESS;
}

bool AudioCoreService::UpdateOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AudioDeviceDescriptor tmpOutputDeviceDesc = audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid);
    if (!desc->IsSameDeviceInfo(tmpOutputDeviceDesc)) {
        WriteOutputRouteChangeEvent(desc, reason);
        audioRouterSelectStrategy_.UpdateCurrentOutputDevice(uid, {desc});
        AUDIO_DEBUG_LOG("currentActiveDevice update %{public}d", audioActiveDevice_.GetCurrentOutputDeviceType());
        SyncVolWhenCurrentOutputDeviceChanged(desc);
        return true;
    }
    return false;
}

bool AudioCoreService::UpdateInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
    const AudioStreamDeviceChangeReasonExt reason)
{
    if (!IsSameDevice(desc, audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid))) {
        WriteInputRouteChangeEvent(desc, reason, uid);
        audioRouterSelectStrategy_.UpdateCurrentInputDevice(uid, {desc});
        AUDIO_DEBUG_LOG("currentActiveInputDevice update %{public}d",
            audioActiveDevice_.GetCurrentInputDeviceType(uid));
        return true;
    }
    return false;
}

void AudioCoreService::WriteOutputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const AudioStreamDeviceChangeReason reason)
{
    int64_t timeStamp = AudioPolicyUtils::GetInstance().GetCurrentTimeMS();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_ROUTE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    bean->Add("REASON", static_cast<int32_t>(reason));
    bean->Add("TIMESTAMP", static_cast<uint64_t>(timeStamp));
    bean->Add("DEVICE_TYPE_BEFORE_CHANGE", curOutputDeviceType);
    bean->Add("DEVICE_TYPE_AFTER_CHANGE", desc->deviceType_);
    bean->Add("PRE_AUDIO_SCENE", static_cast<int32_t>(audioSceneManager_.GetLastAudioScene()));
    bean->Add("CUR_AUDIO_SCENE", static_cast<int32_t>(audioSceneManager_.GetAudioScene(true)));
    bean->Add("DEVICE_LIST", audioDeviceManager_.GetConnDevicesStr());
    bean->Add("ROUTER_TYPE", static_cast<int32_t>(desc->routerType_));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioCoreService::WriteInputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const AudioStreamDeviceChangeReason reason, const int32_t uid)
{
    int64_t timeStamp = AudioPolicyUtils::GetInstance().GetCurrentTimeMS();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_ROUTE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("REASON", static_cast<int32_t>(reason));
    bean->Add("TIMESTAMP", static_cast<uint64_t>(timeStamp));
    bean->Add("DEVICE_TYPE_BEFORE_CHANGE", audioActiveDevice_.GetCurrentInputDeviceType(uid));
    bean->Add("DEVICE_TYPE_AFTER_CHANGE", desc->deviceType_);
    bean->Add("PRE_AUDIO_SCENE", static_cast<int32_t>(audioSceneManager_.GetLastAudioScene()));
    bean->Add("CUR_AUDIO_SCENE", static_cast<int32_t>(audioSceneManager_.GetAudioScene(true)));
    bean->Add("DEVICE_LIST", audioDeviceManager_.GetConnDevicesStr());
    bean->Add("ROUTER_TYPE", static_cast<int32_t>(desc->routerType_));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioCoreService::HandleDeviceChangeForFetchOutputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    if (streamDesc->oldDeviceDescs_.size() == 0) {
        AUDIO_INFO_LOG("No old device info");
        return SUCCESS;
    }
    std::shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_.front();
    auto uid = GetRealUid(streamDesc);
    if (desc->deviceType_ == DEVICE_TYPE_NONE || (IsSameDevice(desc, streamDesc->oldDeviceDescs_.front()) &&
        !NeedRehandleA2DPDevice(desc) && desc->connectState_ != DEACTIVE_CONNECTED &&
        audioSceneManager_.IsSameAudioScene() && !shouldUpdateDeviceDueToDualTone_)) {
        AUDIO_WARNING_LOG("stream %{public}d device not change, no need move device", streamDesc->sessionId_);
        AudioDeviceDescriptor tmpOutputDeviceDesc = audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid);
        if (!IsSameDevice(desc, tmpOutputDeviceDesc)) {
            audioRouterSelectStrategy_.UpdateCurrentOutputDevice(uid, {desc});
            AudioDeviceDescriptor curOutputDevice = audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid);
            audioVolumeManager_.SetVoiceCallVolumeOnDeviceSwitch(curOutputDevice);
            audioActiveDevice_.UpdateActiveDeviceRoute(curOutputDevice.deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG,
                pipeManager_->QueryPipeIdBySessionId(streamDesc->sessionId_), curOutputDevice.networkId_);
            OnPreferredOutputDeviceUpdated(audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid), reason);
        }
        return ERR_NEED_NOT_SWITCH_DEVICE;
    }
    return SUCCESS;
}

int32_t AudioCoreService::HandleDeviceChangeForFetchInputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamDesc->oldDeviceDescs_.size() == 0) {
        AUDIO_INFO_LOG("No old device info");
        return SUCCESS;
    }
    std::shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_.front();
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = streamDesc->oldDeviceDescs_.front();

    if (desc->deviceType_ == DEVICE_TYPE_NONE ||
        (IsSameDevice(desc, oldDeviceDesc) && desc->connectState_ != DEACTIVE_CONNECTED)) {
        AUDIO_WARNING_LOG("stream %{public}d device not change, no need move device", streamDesc->sessionId_);
        int32_t uid = GetRealUid(streamDesc);
        if (IsSameDevice(desc, oldDeviceDesc)) {
            audioRouterSelectStrategy_.UpdateCurrentInputDevice(uid, {desc});
            // networkId is not used.
            OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(uid), "");
            audioActiveDevice_.UpdateActiveDeviceRoute(audioActiveDevice_.GetCurrentInputDeviceType(uid),
                DeviceFlag::INPUT_DEVICES_FLAG, pipeManager_->QueryPipeIdBySessionId(streamDesc->sessionId_),
                audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid).networkId_);
        }
        return ERR_NEED_NOT_SWITCH_DEVICE;
    }
    return SUCCESS;
}

bool AudioCoreService::NeedRehandleA2DPDevice(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP
        && audioIOHandleMap_.CheckIOHandleExist(BLUETOOTH_SPEAKER) == false) {
        AUDIO_WARNING_LOG("A2DP module is not loaded, need rehandle");
        return true;
    }
    return false;
}

bool AudioCoreService::IsDeviceSwitching(const AudioStreamDeviceChangeReasonExt reason)
{
    return reason.IsOverride() || reason.IsOldDeviceUnavaliable() || reason.IsNewDeviceAvailable();
}

void AudioCoreService::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    RendererState rendererState)
{
    const StreamUsage streamUsage = streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage;
    const auto &capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    if (mode == AUDIO_MODE_RECORD && capturerState == CAPTURER_RELEASED) {
        AUDIO_INFO_LOG("[ADeviceEvent] fetch device for capturer stream %{public}d released",
            streamChangeInfo.audioCapturerChangeInfo.sessionId);
        FetchInputDeviceAndRoute("UpdateTracker");
    }
}

void AudioCoreService::HandleCommonSourceOpened(std::shared_ptr<AudioPipeInfo> &pipeInfo)
{
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr && pipeInfo->pipeRole_ == PIPE_ROLE_INPUT &&
        pipeInfo->streamDescriptors_.size() > 0 && pipeInfo->streamDescriptors_.front() != nullptr, "Invalid pipeInfo");
    auto streamDesc = pipeInfo->streamDescriptors_.front();
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is null");
    SourceType sourceType = streamDesc->capturerInfo_.sourceType;
    if (specialSourceTypeSet_.count(sourceType) == 0) {
        CHECK_AND_RETURN_LOG(pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL,
            "Special Pipe need not PrepareNormalSource");
        PrepareNormalSource(pipeInfo, streamDesc);
    }
}

void AudioCoreService::DelayReleaseOffloadPipe(AudioIOHandle id, uint32_t paIndex, OffloadType type)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_LOG(type < OFFLOAD_TYPE_NUM && !isOffloadInRelease_[type].load(), "Offload is releasing");
    isOffloadInRelease_[type].store(true);
    isOffloadOpened_[type].store(false);
    auto unloadOffloadThreadFuc = [this, id, paIndex, type] { this->ReleaseOffloadPipe(id, paIndex, type); };
    std::thread unloadOffloadThread(unloadOffloadThreadFuc);
    unloadOffloadThread.detach();
}

int32_t AudioCoreService::ReleaseOffloadPipe(AudioIOHandle id, uint32_t paIndex, OffloadType type)
{
    HILOG_COMM_INFO("[ReleaseOffloadPipe]unload offload module");
    std::unique_lock<std::mutex> lock(offloadCloseMutex_);
    // Try to wait 10 seconds before unloading the module, because the audio driver takes some time to process
    // the shutdown process..
    CHECK_AND_RETURN_RET_LOG(type < OFFLOAD_TYPE_NUM, ERR_INVALID_PARAM, "invalid type");
    offloadCloseCondition_[type].wait_for(lock, std::chrono::seconds(WAIT_OFFLOAD_CLOSE_TIME_SEC), [this, type] () {
        return isOffloadOpened_[type].load();
    });

    CHECK_AND_RETURN_RET_LOG(GetEventEntry(), ERR_INVALID_PARAM, "GetEventEntry() return nullptr");
    return GetEventEntry()->ReleaseOffloadPipe(id, paIndex, type);
}

void AudioCoreService::DelayReleaseMultiChannelPipe(AudioIOHandle id, uint32_t paIndex)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_LOG(!isMultiChannelInRelease_.load(), "MultiChannel is releasing");
    isMultiChannelInRelease_.store(true);
    isMultiChannelOpened_.store(false);
    auto unloadMultiChannelThreadFuc = [this, id, paIndex] { this->ReleaseMultiChannelPipe(id, paIndex); };
    std::thread unloadMultiChannelThread(unloadMultiChannelThreadFuc);
    unloadMultiChannelThread.detach();
}

int32_t AudioCoreService::ReleaseMultiChannelPipe(AudioIOHandle id, uint32_t paIndex)
{
    HILOG_COMM_INFO("[ReleaseMultiChannelPipe]unload MultiChannel module");
    std::unique_lock<std::mutex> lock(multiChannelCloseMutex_);
    // Try to wait 10 seconds before unloading the module, because the audio driver takes some time to process
    // the shutdown process..
    multiChannelCloseCondition_.wait_for(lock, std::chrono::seconds(WAIT_MULTICHANNL_CLOSE_TIME_SEC), [this] () {
        return isMultiChannelOpened_.load();
    });

    if (!isMultiChannelOpened_.load()) {
        UpdateNearLinkCollaborativeState(false);
    }
    CHECK_AND_RETURN_RET_LOG(GetEventEntry(), ERR_INVALID_PARAM, "GetEventEntry() return nullptr");
    return GetEventEntry()->ReleaseMultiChannelPipe(id, paIndex);
}

void AudioCoreService::DelayReleasePrimaryExtraPipe(AudioIOHandle id, uint32_t paIndex,
    const std::string &pipeName, const std::string &moduleName)
{
    AUDIO_INFO_LOG("In");
    std::lock_guard<std::mutex> lock(primaryExtraCloseMutex_);
    CHECK_AND_RETURN_LOG(!isPrimaryExtraInReleaseMap_[pipeName], "PrimaryExtra is releasing");
    isPrimaryExtraInReleaseMap_[pipeName] = true;
    isPrimaryExtraOpenedMap_[pipeName] = false;
    auto unloadThreadFunc = [this, id, paIndex, pipeName, moduleName] {
        this->ReleasePrimaryExtraPipe(id, paIndex, pipeName, moduleName);
    };
    std::thread unloadThread(unloadThreadFunc);
    unloadThread.detach();
}

int32_t AudioCoreService::ReleasePrimaryExtraPipe(AudioIOHandle id, uint32_t paIndex,
    const std::string &pipeName, const std::string &moduleName)
{
    AUDIO_INFO_LOG("[ReleasePrimaryExtraPipe]unload primary_extra module");
    {
        std::unique_lock<std::mutex> lock(primaryExtraCloseMutex_);
        primaryExtraCloseCondition_.wait_for(lock, std::chrono::seconds(WAIT_PRIMARY_EXTRA_CLOSE_TIME_SEC),
            [this, &pipeName] () { return isPrimaryExtraOpenedMap_[pipeName]; });
    }
    CHECK_AND_RETURN_RET_LOG(GetEventEntry(), ERR_INVALID_PARAM, "GetEventEntry() return nullptr");
    return GetEventEntry()->ReleasePrimaryExtraPipe(id, paIndex, pipeName, moduleName);
}

void AudioCoreService::DelayReleaseFastPipe(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    const AudioIOHandle id = pipeInfo->id_;
    const uint32_t paIndex = pipeInfo->paIndex_;
    const std::string &pipeName = pipeInfo->name_;
    const std::string &moduleName = pipeInfo->moduleInfo_.name;
    const uint32_t routeFlag = pipeInfo->routeFlag_;
    const bool hasMoveStream = pipeInfo->GetHasMoveStream();
    const bool ultraFastFlag = pipeInfo->GetUltraFastFlag();
    int32_t delayMs = GetFastPipeDelayReleaseTimeMs(pipeName, hasMoveStream, ultraFastFlag);
    AUDIO_INFO_LOG("DelayReleaseFastPipe id:%{public}u, paIndex:%{public}u, pipeName:%{public}s, delay:%{public}dms " \
        "hasMoveStream:%{public}s, ultraFastFlag:%{public}s",
        id, paIndex, pipeName.c_str(), delayMs, hasMoveStream ? "true" : "false", ultraFastFlag ? "true" : "false");
    std::lock_guard<std::mutex> lock(fastCloseMutex_);
    CHECK_AND_RETURN_LOG(!isFastInReleaseMap_[pipeName], "%{public}s is already in release",
        pipeName.c_str());
    isFastInReleaseMap_[pipeName] = true;
    isFastOpenedMap_[pipeName] = false;
    auto unloadFastThreadFuc = [this, id, paIndex, pipeName, moduleName, delayMs] {
        this->ReleaseFastPipe(id, paIndex, pipeName, moduleName, delayMs);
    };
    std::thread unloadFastThread(unloadFastThreadFuc);
    unloadFastThread.detach();
}

int32_t AudioCoreService::ReleaseFastPipe(AudioIOHandle id, uint32_t paIndex, const std::string &pipeName,
    const std::string &moduleName, int32_t delayMs)
{
    AUDIO_INFO_LOG("[ReleaseFastPipe]unload FAST module, delay:%{public}dms, pipeName:%{public}s",
        delayMs, pipeName.c_str());
    {
        std::unique_lock<std::mutex> lock(fastCloseMutex_);
        // Shared condition variable: each wait checks only its own pipeName key.
        fastCloseCondition_.wait_for(lock, std::chrono::milliseconds(delayMs), [this, &pipeName] () {
            return isFastOpenedMap_[pipeName];
        });
    }

    CHECK_AND_RETURN_RET_LOG(GetEventEntry(), ERR_INVALID_PARAM, "GetEventEntry() return nullptr");
    return GetEventEntry()->ReleaseFastPipe(id, paIndex, pipeName, moduleName);
}

void AudioCoreService::UpdateFastState(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN(pipeInfo != nullptr && pipeInfo->GetStreamSize() > 0);
    CHECK_AND_RETURN(pipeInfo->IsRouteFast());
    {
        std::lock_guard<std::mutex> lock(fastCloseMutex_);
        isFastOpenedMap_[pipeInfo->name_] = true;
    }
    fastCloseCondition_.notify_all();
}

AudioModuleInfo AudioCoreService::BuildDefaultArmModuleInfo(const AudioModuleInfo &src)
{
    AudioModuleInfo info = src;
    info.rate = std::to_string(SAMPLE_RATE_48000);
    info.channels = "2";
    info.channelLayout = std::to_string(CH_LAYOUT_STEREO);
    AudioSampleFormat format = AudioDefinitionPolicyUtils::formatStrToEnum[info.format];
    format = format == SAMPLE_U8 ? SAMPLE_S16LE : format;
    info.bufferSize = std::to_string(static_cast<uint32_t>(SAMPLE_RATE_48000) * STEREO *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(format) * DEFAULT_PERIOD_IN_MS / AUDIO_MS_PER_SECOND);
    return info;
}

void AudioCoreService::DelayReloadArmPipe(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN(pipeInfo != nullptr);
    const AudioIOHandle id = pipeInfo->id_;
    const std::string reloadKey = pipeInfo->moduleInfo_.macAddress + "|" + pipeInfo->moduleInfo_.role;
    AUDIO_INFO_LOG("DelayReloadArmPipe id:%{public}u, mac:%{public}s, role:%{public}s", id,
        AudioPolicyUtils::GetInstance().EncUsbAddr(pipeInfo->moduleInfo_.macAddress).c_str(),
        pipeInfo->moduleInfo_.role.c_str());
    {
        std::lock_guard<std::mutex> lock(armReloadMutex_);
        CHECK_AND_RETURN_LOG(!isArmReloadPendingMap_[reloadKey], "already pending");
        isArmReloadPendingMap_[reloadKey] = true;
        isArmReopenedMap_[reloadKey] = false;
    }
    AudioModuleInfo moduleInfo = BuildDefaultArmModuleInfo(pipeInfo->moduleInfo_);
    auto reloadFunc = [this, id, reloadKey, moduleInfo] {
        {
            std::unique_lock<std::mutex> lock(armReloadMutex_);
            armReloadCondition_.wait_for(lock, std::chrono::milliseconds(ARM_PIPE_RELOAD_DELAY_MS),
                [this, &reloadKey] { return isArmReopenedMap_[reloadKey]; });
        }
        if (isArmReopenedMap_[reloadKey]) {
            std::lock_guard<std::mutex> lock(armReloadMutex_);
            isArmReloadPendingMap_[reloadKey] = false;
            return;
        }
        uint32_t paIndex = HDI_INVALID_ID;
        audioPolicyManager_.ReloadAudioPort(moduleInfo, paIndex);
        if (paIndex != HDI_INVALID_ID) {
            auto pipe = pipeManager_->GetPipeinfoById(id);
            if (pipe != nullptr) {
                pipe->moduleInfo_ = moduleInfo;
                pipe->paIndex_ = paIndex;
                pipe->InitAudioStreamInfo();
            }
        }
        std::lock_guard<std::mutex> lock(armReloadMutex_);
        isArmReloadPendingMap_[reloadKey] = false;
    };
    std::thread reloadThread(reloadFunc);
    reloadThread.detach();
}

void AudioCoreService::CancelArmReload(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN(pipeInfo != nullptr);
    const std::string reloadKey = pipeInfo->moduleInfo_.macAddress + "|" + pipeInfo->moduleInfo_.role;
    std::lock_guard<std::mutex> lock(armReloadMutex_);
    auto it = isArmReloadPendingMap_.find(reloadKey);
    if (it != isArmReloadPendingMap_.end() && it->second) {
        isArmReopenedMap_[reloadKey] = true;
        armReloadCondition_.notify_all();
        AUDIO_INFO_LOG("CancelArmReload mac:%{public}s, role:%{public}s",
            AudioPolicyUtils::GetInstance().EncUsbAddr(pipeInfo->moduleInfo_.macAddress).c_str(),
            pipeInfo->moduleInfo_.role.c_str());
    }
}

void AudioCoreService::PrepareMoveAttrs(std::shared_ptr<AudioStreamDescriptor> &streamDesc, DeviceType &oldDeviceType,
    bool &isNeedTriggerCallback, std::string &oldSinkName, const AudioStreamDeviceChangeReasonExt reason)
{
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = streamDesc->newDeviceDescs_.front();
    oldDeviceType = streamDesc->oldDeviceDescs_.front()->deviceType_;
    if (streamDesc->oldDeviceDescs_.front()->IsSameDeviceDesc(newDeviceDesc)) {
        isNeedTriggerCallback = false;
    }
    oldSinkName = AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->oldDeviceDescs_.front(),
        streamDesc->sessionId_);

    AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u [%{public}d][%{public}s] to [%{public}d][%{public}s]" \
        " reason %{public}d",
        streamDesc->sessionId_, streamDesc->oldDeviceDescs_.front()->deviceType_,
        GetEncryptAddr(streamDesc->oldDeviceDescs_.front()->macAddress_).c_str(), newDeviceDesc->deviceType_,
        GetEncryptAddr(newDeviceDesc->macAddress_).c_str(), static_cast<int32_t>(reason));
}

bool AudioCoreService::HandleMuteBeforeDeviceSwitch(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    bool isEnhancedRouting = Util::IsEnhancedRoutingSupported();
    for (std::shared_ptr<AudioStreamDescriptor> &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE(streamDesc != nullptr);
        if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
#ifndef MUTE_SINK_DISABLE
            AudioScene lastAudioScene = audioSceneManager_.GetLastAudioScene();
            AudioScene audioScene = audioSceneManager_.GetAudioScene();
            auto muteReason = reason.IsSetAudioScene() && IsCallOrRingToDefault(lastAudioScene, audioScene) ?
                AudioStreamDeviceChangeReasonExt::ExtEnum::CALL_OR_RING_TO_DEFAULT : reason;
            std::string oldClassName;
            std::string oldModuleName;
            CaptureOldPipeInfo(isEnhancedRouting, streamDesc->sessionId_, oldClassName, oldModuleName);
            std::string overrideCls = (oldClassName == PRIMARY_EXTRA_CLASS) ? oldClassName : "";
            std::string overrideInfo = (oldClassName == PRIMARY_EXTRA_CLASS) ? oldModuleName : "";
            MuteSinkPortForSwitchDevice(streamDesc, muteReason, overrideCls, overrideInfo);
#endif
        }
    }

    return true;
}

void AudioCoreService::CaptureOldPipeInfo(bool isEnhancedRouting, uint32_t sessionId,
    std::string &oldClassName, std::string &oldModuleName)
{
    CHECK_AND_RETURN(isEnhancedRouting);
    CHECK_AND_RETURN(pipeManager_ != nullptr);
    oldClassName = pipeManager_->GetClassNameBySessionId(sessionId);
    oldModuleName = pipeManager_->GetModuleNameBySessionId(sessionId);
}

void AudioCoreService::MuteSinkPortForSwitchDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason, const std::string &oldClassName,
    const std::string &oldModuleName)
{
    Trace trace("AudioCoreService::MuteSinkPortForSwitchDevice");
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && !streamDesc->oldDeviceDescs_.empty() &&
        !streamDesc->newDeviceDescs_.empty(), "Invalid streamDesc");
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = streamDesc->oldDeviceDescs_.front();
    std::shared_ptr<AudioDeviceDescriptor> newDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN(oldDesc != nullptr && newDesc != nullptr);
    if (oldDesc->IsSameDeviceDesc(*newDesc)) { return; }

    audioIOHandleMap_.SetMoveFinish(false);

    std::string oldSinkPortName = AudioPolicyUtils::GetInstance().GetSinkName(oldDesc, streamDesc->sessionId_);
    std::string newSinkPortName = AudioPolicyUtils::GetInstance().GetSinkName(newDesc, streamDesc->sessionId_);

    auto GetFinalSinkPortName = [](uint32_t routeFlag, const std::string &defaultSinkPortName) -> std::string {
        if (routeFlag == (AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP)) {
            return PRIMARY_MMAP_VOIP;
        } else if (routeFlag == (AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_VOIP)) {
            return PRIMARY_DIRECT_VOIP;
        } else if (routeFlag == AUDIO_OUTPUT_FLAG_ULTRA_FAST &&
            defaultSinkPortName == PRIMARY_SPEAKER) {
            return TRUE_ULTRA_FAST_SPEAKER;
        } else if (routeFlag == AUDIO_OUTPUT_FLAG_FAST &&
            defaultSinkPortName == PRIMARY_SPEAKER) {
            return PRIMARY_MMAP;
        } else if (routeFlag == AUDIO_OUTPUT_FLAG_FAST && defaultSinkPortName == BLUETOOTH_SPEAKER) {
            return BLUETOOTH_A2DP_FAST;
        } else if (routeFlag == (AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_HD)) {
            return PRIMARY_DIRECT;
        }
        return defaultSinkPortName;
    };
    oldSinkPortName = GetFinalSinkPortName(streamDesc->oldRouteFlag_, oldSinkPortName);
    newSinkPortName = GetFinalSinkPortName(streamDesc->routeFlag_, newSinkPortName);

    AUDIO_INFO_LOG("mute sink new:[%{public}s]", newSinkPortName.c_str());
    MuteSinkPort(oldSinkPortName, newSinkPortName, reason, oldClassName, oldModuleName);
}

/**
 * After a voice call is answered during an incoming ringtone,
 * a delay is required before setting the voice call device.
 * This ensures the remaining ringtone buffer is drained,
 * preventing any residual ringtone sound from leaking into the call path.
 *
 * This function should only be called in the voice call scenario.
*/
void AudioCoreService::CheckAndSleepBeforeVoiceCallDeviceSet(const AudioStreamDeviceChangeReasonExt reason)
{
    if (reason.IsSetAudioScene() && streamCollector_.IsStreamRunning(STREAM_USAGE_VOICE_RINGTONE)) {
        usleep(VOICE_CALL_DEVICE_SET_DELAY_US);
    }
}

/**
 * Mutes voice message streams on the primary sink when a ringtone is playing.
 * Prevents voice message audio from leaking through the speaker
 */
void AudioCoreService::HandleVoiceMessageMuteForSingleRing(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty() &&
        streamDesc->newDeviceDescs_.front() != nullptr, "Invalid streamDesc");
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager is nullptr");
    if (streamDesc->newDeviceDescs_.size() != 1) {
        return;
    }
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetPlayingVoiceMessageRendererChangeInfos(rendererChangeInfos);
    for (const auto &changeInfo : rendererChangeInfos) {
        if (changeInfo != nullptr && pipeManager_->IsOnPrimaryAdapter(changeInfo->sessionId)) {
            // Reuse the dual-ring mute marker to track voice message streams muted in the
            // single-ring scenario, so they can be restored together during unmute recovery.
            streamsWhenRingDualOnPrimarySpeaker_.push_back(make_pair(changeInfo->sessionId, STREAM_VOICE_MESSAGE));
            audioPolicyManager_.SetDualStreamVolumeMute(changeInfo->sessionId, true);
        }
    }
}

/**
 * Mutes non-dual-output streams on the primary sink when a dual-tone ringtone is playing.
 * Prevents non-dual-output audio (e.g. media, voice call) from leaking through the speaker
 * while the dual-output ring stream occupies both primary and secondary outputs simultaneously.
 */
void AudioCoreService::HandlePrimaryNonDualOutputStreamMuteForDualRing(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty() &&
        streamDesc->newDeviceDescs_.front() != nullptr, "Invalid streamDesc");
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager is nullptr");
    if (!IsRingerOrAlarmerDualDevicesRange(streamDesc->newDeviceDescs_.front()->deviceType_)) {
        return;
    }
    if (!AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(streamDesc->newDeviceDescs_, streamDesc->sessionId_)) {
        return;
    }
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetPlayingNonDualOutputRendererChangeInfos(rendererChangeInfos);
    for (const auto &changeInfo : rendererChangeInfos) {
        if (changeInfo != nullptr && pipeManager_->IsOnPrimaryAdapter(changeInfo->sessionId)) {
            AudioStreamType streamType = streamCollector_.GetStreamType(changeInfo->sessionId);
            streamsWhenRingDualOnPrimarySpeaker_.push_back(make_pair(changeInfo->sessionId, streamType));
            audioPolicyManager_.SetDualStreamVolumeMute(changeInfo->sessionId, true);
        }
    }
}

/**
 * When a stream playing on the primary output (e.g. headset) is interrupted by an alarm or ring,
 * a delay is required before switching to the other output (e.g., speaker + headset).
 * This allows the primary output buffer to be fully drained,
 * preventing residual audio from leaking through the speaker.
 */
void AudioCoreService::CheckAndSleepBeforeRingDeviceSet(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager is nullptr");
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty(), "Invalid streamDesc");
    bool isRingOrAlarmStream = Util::IsRingerOrAlarmerStreamUsage(streamDesc->rendererInfo_.streamUsage);
    uint32_t inPastMs = PRIMARY_OUTPUT_BUFFER_DRAIN_DELAY_US / AUDIO_US_PER_MS * 2;
    if (streamDesc->streamStatus_ == STREAM_STATUS_NEW && isRingOrAlarmStream &&
        pipeManager_->IsOnPrimaryAdapter(streamDesc->sessionId_) &&
        pipeManager_->HasActiveStreamOnPrimary(inPastMs)) {
        HandleVoiceMessageMuteForSingleRing(streamDesc);
        HandlePrimaryNonDualOutputStreamMuteForDualRing(streamDesc);
        usleep(PRIMARY_OUTPUT_BUFFER_DRAIN_DELAY_US);
    }
}

/**
 * Sleep for a short duration after muting during device switching.
 * This allows the underlying audio buffer to drain residual data before switching to the new output device,
 * helping to avoid audio artifacts such as leakage or pop noise.
*/
void AudioCoreService::SleepForSwitchDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && !streamDesc->oldDeviceDescs_.empty() &&
        !streamDesc->newDeviceDescs_.empty(), "Invalid streamDesc");
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = streamDesc->oldDeviceDescs_.front();
    std::shared_ptr<AudioDeviceDescriptor> newDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN(oldDesc != nullptr && newDesc != nullptr && streamDesc->streamStatus_ == STREAM_STATUS_STARTED);
    if (oldDesc->IsSameDeviceDesc(*newDesc)) { return; }

    std::string oldSinkName = AudioPolicyUtils::GetInstance().GetSinkName(oldDesc, streamDesc->sessionId_);
    bool isOldDeviceUnavailable = reason.IsOldDeviceUnavaliable() || reason.IsOldDeviceUnavaliableExt();
    bool isHeadsetToSpkOrEp = IsHeadsetToSpkOrEp(oldDesc, newDesc);

    struct SleepStrategy {
        std::function<bool()> condition;
        std::vector<uint32_t> sleepDurations;
    };

    std::vector<SleepStrategy> strategies = {
        {
            [&]() { return reason.IsOverride() || reason.IsSetDefaultOutputDevice() || reason.IsNewDeviceAvailable() ||
                reason.IsDeviceReselect(); },
            {BASE_DEVICE_SWITCH_SLEEP_US, BASE_DEVICE_SWITCH_SLEEP_US}
        },
        {
            [&]() { return reason.IsDistributedDeviceUnavailable(); },
            {BASE_DEVICE_SWITCH_SLEEP_US, DISTRIBUTED_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US}
        },
        {
            [&]() { return isOldDeviceUnavailable && isHeadsetToSpkOrEp; },
            {BASE_DEVICE_SWITCH_SLEEP_US, OLD_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US, HEADSET_TO_SPK_EP_EXTRA_SLEEP_US}
        },
        {
            [&]() { return isOldDeviceUnavailable; },
            {BASE_DEVICE_SWITCH_SLEEP_US, OLD_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US}
        },
        {
            [&]() { return (reason.IsUnknown() && oldSinkName == REMOTE_CAST_INNER_CAPTURER_SINK_NAME) ||
                reason.IsCollaborativeStateChange(); },
            {BASE_DEVICE_SWITCH_SLEEP_US}
        },
    };

    for (const auto &strategy : strategies) {
        if (strategy.condition()) {
            for (auto sleepTime : strategy.sleepDurations) {
                usleep(sleepTime);
            }
            return;
        }
    }
}

bool AudioCoreService::IsHeadsetToSpkOrEp(const std::shared_ptr<AudioDeviceDescriptor> &oldDesc,
    const std::shared_ptr<AudioDeviceDescriptor> &newDesc)
{
    CHECK_AND_RETURN_RET(oldDesc != nullptr, false);
    CHECK_AND_RETURN_RET(newDesc != nullptr, false);
    DeviceType oldDeviceType = oldDesc->deviceType_;
    DeviceType newDeviceType = newDesc->deviceType_;
    return (oldDeviceType == DEVICE_TYPE_USB_HEADSET || oldDeviceType == DEVICE_TYPE_USB_ARM_HEADSET) &&
        (newDeviceType == DEVICE_TYPE_SPEAKER || newDeviceType == DEVICE_TYPE_EARPIECE);
}

void AudioCoreService::SetVoiceCallMuteForSwitchDevice()
{
    Trace trace("AudioCoreService::SetVoiceMuteForSwitchDevice");
    AudioServerProxy::GetInstance().SetVoiceVolumeProxy(0);

    AUDIO_INFO_LOG("%{public}" PRId64" us for modem call update route", WAIT_MODEM_CALL_SET_VOLUME_TIME_US);
    usleep(WAIT_MODEM_CALL_SET_VOLUME_TIME_US);
    // Unmute in SetVoiceCallVolumeOnDeviceSwitch after update route.
}

void AudioCoreService::MuteSinkPort(const std::string &oldSinkName, const std::string &newSinkName,
    AudioStreamDeviceChangeReasonExt reason,
    const std::string &oldClassName, const std::string &oldModuleName)
{
    if (reason.IsOverride() || reason.IsSetDefaultOutputDevice() || reason.IsCallOrRingToDefault() ||
        reason.IsDeviceReselect()) {
        int64_t muteTime = SELECT_DEVICE_MUTE_MS;
        if (newSinkName == OFFLOAD_PRIMARY_SPEAKER || oldSinkName == OFFLOAD_PRIMARY_SPEAKER) {
            muteTime = SELECT_OFFLOAD_DEVICE_MUTE_MS;
        }
        MutePrimaryOrOffloadSink(newSinkName, muteTime);
        audioIOHandleMap_.MuteSinkPort(newSinkName, SELECT_DEVICE_MUTE_MS, true, false);
        audioIOHandleMap_.MuteSinkPort(oldSinkName, muteTime, true, false, oldClassName, oldModuleName);
    } else if (reason == AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE) {
        int64_t muteTime = NEW_DEVICE_AVALIABLE_MUTE_MS;
        if (newSinkName == OFFLOAD_PRIMARY_SPEAKER || oldSinkName == OFFLOAD_PRIMARY_SPEAKER) {
            muteTime = NEW_DEVICE_AVALIABLE_OFFLOAD_MUTE_MS;
        }
        MutePrimaryOrOffloadSink(oldSinkName, muteTime);
        audioIOHandleMap_.MuteSinkPort(newSinkName, NEW_DEVICE_AVALIABLE_MUTE_MS, true, false);
        audioIOHandleMap_.MuteSinkPort(oldSinkName, muteTime, true, false, oldClassName, oldModuleName);
    }
    MuteSinkPortLogic(oldSinkName, newSinkName, reason);
}

void AudioCoreService::MutePrimaryOrOffloadSink(const std::string &sinkName, int64_t muteTime)
{
    // Fix sinkPort mute error caused by incorrect pipeType
    if (sinkName == OFFLOAD_PRIMARY_SPEAKER) {
        audioIOHandleMap_.MuteSinkPort(PRIMARY_SPEAKER, muteTime, true, false);
    } else if (sinkName == PRIMARY_SPEAKER) {
        audioIOHandleMap_.MuteSinkPort(OFFLOAD_PRIMARY_SPEAKER, muteTime, true, false);
    }
}

void AudioCoreService::MuteSinkPortLogic(const std::string &oldSinkName, const std::string &newSinkName,
    AudioStreamDeviceChangeReasonExt reason)
{
    auto ringermode = audioPolicyManager_.GetRingerMode();
    AudioScene scene = audioSceneManager_.GetAudioScene(true);
    if (reason.IsDistributedDeviceUnavailable()) {
        audioIOHandleMap_.MuteSinkPort(newSinkName, DISTRIBUTED_DEVICE_UNAVALIABLE_MUTE_MS, true, false);
    } else if (reason.IsOldDeviceUnavaliable()) {
        MutePrimaryOrOffloadSink(newSinkName, OLD_DEVICE_UNAVALIABLE_MUTE_MS);
        audioIOHandleMap_.MuteSinkPort(newSinkName, OLD_DEVICE_UNAVALIABLE_MUTE_MS, true, false);
    } else if (reason.IsOldDeviceUnavaliableExt()) {
        audioIOHandleMap_.MuteSinkPort(newSinkName, OLD_DEVICE_UNAVALIABLE_EXT_MUTE_MS, true, false);
    } else if (reason == AudioStreamDeviceChangeReason::UNKNOWN &&
        oldSinkName == REMOTE_CAST_INNER_CAPTURER_SINK_NAME) {
        // remote cast -> earpiece 300ms fix sound leak
        audioIOHandleMap_.MuteSinkPort(newSinkName, NEW_DEVICE_REMOTE_CAST_AVALIABLE_MUTE_MS, true, false);
    } else if (reason.IsCollaborativeStateChange()) {
        audioIOHandleMap_.MuteSinkPort(PRIMARY_SPEAKER, COLLABORATIVE_STATE_CHANGE_MUTE_SINK_PORT_US, true, false);
        audioIOHandleMap_.MuteSinkPort(BLUETOOTH_SPEAKER, COLLABORATIVE_STATE_CHANGE_MUTE_SINK_PORT_US, true, false);
    }
}

int32_t AudioCoreService::ActivateOutputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, ERR_INVALID_PARAM, "Invalid stream desc");

    auto result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    CheckScoState(streamDesc, true);
    if (result == REFETCH_DEVICE) {
        FetchOutputDeviceAndRoute("ActivateOutputDevice", reason);
        FetchInputDeviceAndRoute("ActivateOutputDevice", reason);
    }
    return result;
}

int32_t AudioCoreService::ActivateInputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, ERR_INVALID_PARAM, "Invalid stream desc");

    auto result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    CheckScoState(streamDesc, true);
    if (result == REFETCH_DEVICE) {
        FetchOutputDeviceAndRoute("ActivateInputDevice", reason);
        FetchInputDeviceAndRoute("ActivateInputDevice", reason);
    }
    return result;
}

void AudioCoreService::OnAudioSceneChange(const AudioScene& audioScene)
{
    Trace trace("AudioCoreService::OnAudioSceneChange:" + std::to_string(audioScene));
    AUDIO_INFO_LOG("scene change to %{public}d", audioScene);
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audio policy server handler is null");
    audioPolicyServerHandler_->SendAudioSceneChangeEvent(audioScene);
}

void AudioCoreService::HandleSceneChangePreferredDevice(AudioScene audioScene, AudioScene lastAudioScene, int32_t uid)
{
    SetSleVoiceStatusFlag(audioScene);
    OnAudioSceneChange(audioScene);
    auto currentOutputDevice = audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid);
    if (Util::IsEnhancedRoutingSupported()) {
        OnPreferredOutputDeviceUpdated(currentOutputDevice, AudioStreamDeviceChangeReason::UNKNOWN);
        OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(uid), "",
            AudioStreamDeviceChangeReason::UNKNOWN);
    } else if (currentOutputDevice.deviceType_ == DEVICE_TYPE_NEARLINK &&
        lastAudioScene == AUDIO_SCENE_DEFAULT && audioScene != AUDIO_SCENE_DEFAULT) {
        OnPreferredOutputDeviceUpdated(currentOutputDevice, AudioStreamDeviceChangeReason::UNKNOWN);
    }
    if (audioScene == AUDIO_SCENE_DEFAULT && currentOutputDevice.IsRemoteDevice()) {
        OnPreferredOutputDeviceUpdated(currentOutputDevice, AudioStreamDeviceChangeReason::OVERRODE);
    }
}

void AudioCoreService::HandleAudioSceneVolumeChange(AudioScene audioScene, AudioScene lastAudioScene, int32_t uid)
{
    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        // Make sure the STREAM_VOICE_CALL volume is set before the calling starts.
        audioVolumeManager_.SetVoiceCallVolume(audioVolumeManager_.GetSystemVolumeLevel(STREAM_VOICE_CALL));
    } else if (lastAudioScene == AUDIO_SCENE_PHONE_CALL && audioScene != AUDIO_SCENE_PHONE_CALL) {
        audioVolumeManager_.SetVoiceRingtoneMute(false);
    }
    if (audioSceneManager_.IsCallEnded()) {
        audioVolumeManager_.ClearLoudVolumeHoldMapForCall();
        callTransferCreated_ = false;
    }
    if (audioSceneManager_.IsHangUpScene()) {
        audioVolumeManager_.RefreshActiveDeviceVolume();
    }
    if (lastAudioScene == AUDIO_SCENE_RINGING && audioScene != AUDIO_SCENE_RINGING &&
        audioVolumeManager_.IsAppRingMuted(uid)) {
        audioVolumeManager_.SetAppRingMuted(uid, false); // unmute the STREAM_RING for the app.
    }
}

bool AudioCoreService::HandleOutputStreamInRunning(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    AudioStreamDeviceChangeReasonExt reason)
{
    if (streamDesc->streamStatus_ != STREAM_STATUS_STARTED ||
        pipeManager_->IsFocusStandby(streamDesc->sessionId_)) {
        return true;
    }
    if (HandleDeviceChangeForFetchOutputDevice(streamDesc, reason) == ERR_NEED_NOT_SWITCH_DEVICE &&
        !Util::IsRingerOrAlarmerStreamUsage(streamDesc->rendererInfo_.streamUsage)) {
        return false;
    }
    return true;
}

bool AudioCoreService::HandleInputStreamInRunning(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamDesc->streamStatus_ != STREAM_STATUS_STARTED) {
        return true;
    }
    if (HandleDeviceChangeForFetchInputDevice(streamDesc) == ERR_NEED_NOT_SWITCH_DEVICE) {
        return false;
    }
    return true;
}

void AudioCoreService::HandleDualStartClient(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 1, "Invalid params");
    std::string firstSinkName =
        AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->newDeviceDescs_[0], streamDesc->sessionId_);
    std::string secondSinkName =
        AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->newDeviceDescs_[1], streamDesc->sessionId_);
    AUDIO_INFO_LOG("firstSinkName %{public}s, secondSinkName %{public}s",
        firstSinkName.c_str(), secondSinkName.c_str());
    if (firstSinkName == secondSinkName) {
        activeDevices.push_back(
            make_pair(streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG));
        activeDevices.push_back(
            make_pair(streamDesc->newDeviceDescs_[1]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG));
    }
}

void AudioCoreService::ResetOriginalFlagForRemote(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN(streamDesc != nullptr && streamDesc->IsDeviceRemote());
    auto hdPlayMode = streamDesc->GetHdPlaybackMode();
    CHECK_AND_RETURN_LOG(hdPlayMode != HdPlaybackMode::DEVICE_LEVEL, "device level hdPlay, skip reset");
    AUDIO_INFO_LOG("originalFlag: %{public}d, oldOriginalFlag: %{public}d", streamDesc->rendererInfo_.originalFlag,
        streamDesc->oldOriginalFlag_);
    streamDesc->ResetOriginalFlag();
}

void AudioCoreService::BatchFetchAndStartOutputStreams(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputStreamDescs, const std::string &caller)
{
    std::string fetchCaller = caller + "FetchOutputDeviceAndRoute";
    std::vector<FetchDeviceInfo> outInfos;
    for (auto &streamDesc : outputStreamDescs) {
        CHECK_AND_CONTINUE(streamDesc != nullptr);
        CHECK_AND_CONTINUE(!(VolumeUtils::IsPCVolumeEnable() && !isFirstScreenOn_.load()));
        CHECK_AND_CONTINUE(streamDesc->rendererTarget_ != INJECT_TO_VOICE_COMMUNICATION_CAPTURE);
        StreamUsage streamUsage = audioSessionService_.GetAudioSessionStreamUsageForDevice(GetRealPid(streamDesc));
        streamUsage = (streamUsage != StreamUsage::STREAM_USAGE_INVALID) ? streamUsage :
            streamDesc->rendererInfo_.streamUsage;
        FetchDeviceInfo info = { streamUsage, GetRealUid(streamDesc), streamDesc->GetRenderPrivacyType(),
            fetchCaller, streamDesc->bundleName_ };
        info.streamId = streamDesc->sessionId_;
        outInfos.push_back(info);
    }
    auto outResult = audioRouterCenter_.FetchOutputDevicesForAllOutputStreams(outInfos);
    for (auto &streamDesc : outputStreamDescs) {
        UpdateStreamDevicesForStart(streamDesc, fetchCaller, outResult);
    }
}

void AudioCoreService::UpdateStreamDevicesForStart(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc, std::string caller,
    const AudioRouterCenter::OutputDeviceMap &outResult)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "Invalid stream desc");
    pipeManager_->UpdateOldDeviceDesc(streamDesc, streamDesc->GetNewDeviceDescs());

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    if (VolumeUtils::IsPCVolumeEnable() && !isFirstScreenOn_.load()) {
        devices = std::vector<std::shared_ptr<AudioDeviceDescriptor>> {
            AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice()
        };
    } else if (streamDesc->rendererTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        devices = std::vector<std::shared_ptr<AudioDeviceDescriptor>> {
            make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SYSTEM_PRIVATE,
                DeviceRole::OUTPUT_DEVICE)
        };
    } else {
        auto it = outResult.find(streamDesc->sessionId_);
        if (it != outResult.end()) {
            devices = it->second;
        }
    }

    CHECK_AND_RETURN_LOG(!devices.empty(), "devices is empty");
    pipeManager_->UpdateNewDeviceDescWithCheck(streamDesc, devices);
    if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED &&
        streamDesc->IsMediaScene() && devices[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        !streamDesc->oldDeviceDescs_.empty() && streamDesc->oldDeviceDescs_.front() &&
        streamDesc->oldDeviceDescs_.front()->deviceType_ != devices[0]->deviceType_) {
        WriteScoStateFaultEvent(devices[0]);
    }
    FetchOutputDupDevice(caller, streamDesc->GetSessionId(), streamDesc);

    ResetOriginalFlagForRemote(streamDesc);
}

void AudioCoreService::UpdateStreamDevicesForCreate(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc, std::string caller)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "Invalid stream desc");
    AUDIO_INFO_LOG("[DeviceFetchStart] for stream %{public}d", streamDesc->GetSessionId());
    pipeManager_->UpdateOldDeviceDesc(streamDesc, streamDesc->GetNewDeviceDescs());

    FetchDeviceInfo info = { streamDesc->GetRenderUsage(), GetRealUid(streamDesc),
        streamDesc->GetRenderPrivacyType(), caller, streamDesc->bundleName_ };
    info.streamId = streamDesc->sessionId_;
    auto devices = audioRouterCenter_.FetchOutputDevices(info);

    pipeManager_->UpdateNewDeviceDesc(streamDesc, devices);
    HILOG_COMM_INFO("[DeviceFetchInfo] device %{public}s for stream %{public}d",
        streamDesc->GetNewDevicesTypeString().c_str(), streamDesc->GetSessionId());
    if (streamDesc->IsMediaScene() && devices[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        WriteScoStateFaultEvent(devices[0]);
    }
    FetchOutputDupDevice(caller, streamDesc->GetSessionId(), streamDesc);
}

void AudioCoreService::SelectA2dpType(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    bool isCreateProcess)
{
#ifdef BLUETOOTH_ENABLE
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, "Invalid stream desc");
    CHECK_AND_RETURN(streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP); // no need log
    vector<Bluetooth::A2dpStreamInfo> allSessionInfos;
    auto flag =
        static_cast<BluetoothOffloadState>(Bluetooth::AudioA2dpManager::A2dpOffloadSessionRequest(allSessionInfos));
    streamDesc->newDeviceDescs_[0]->a2dpOffloadFlag_ = flag;
    JUDGE_AND_INFO_LOG(isCreateProcess, "A2dp offload flag:%{public}d", flag);
    AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(flag == A2DP_OFFLOAD);
#endif
}

bool AudioCoreService::GetDisableFastStreamParam()
{
    return GetFastControlParam();
}

bool AudioCoreService::IsFastAllowed(std::string &bundleName)
{
    CHECK_AND_RETURN_RET(bundleName != "", true);
    std::string bundleNamePre = CHECK_FAST_BLOCK_PREFIX + bundleName;
    std::string result = AudioServerProxy::GetInstance().GetAudioParameterProxy(bundleNamePre);
    if (result == "true") { // "true" means in control
        AUDIO_INFO_LOG("%{public}s not in fast list", bundleName.c_str());
        return false;
    }
    return true;
}

int32_t AudioCoreService::ForceRemoveSleStreamType(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERROR, "Stream desc is nullptr");
    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, true);
    return SUCCESS;
}

void AudioCoreService::DeactivateBluetoothDevice(bool isRunning)
{
    CHECK_AND_RETURN(isRunning);
    audioPolicyManager_.StopAudioPort(BLUETOOTH_SPEAKER);
    if (Bluetooth::AudioA2dpManager::GetActiveA2dpDeviceLocal() != EMPTY_ADDRESS) {
        Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(NULL_ADDRESS);
    }
    if (Bluetooth::AudioHfpManager::GetActiveHfpDeviceLocal() != EMPTY_ADDRESS) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(NULL_ADDRESS);
    }
}

bool AudioCoreService::DeactivateA2dpAfterExclude(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &excludeDeviceDescs,
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputStreamDescs)
{
    bool excludeA2dp = std::any_of(excludeDeviceDescs.begin(), excludeDeviceDescs.end(), [](const auto &deviceDesc) {
        return deviceDesc && deviceDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP;
    });
    if (!excludeA2dp) {
        return false;
    }

    bool willUseA2dp = std::any_of(outputStreamDescs.begin(), outputStreamDescs.end(), [](const auto &streamDesc) {
        return streamDesc && streamDesc->streamStatus_ == STREAM_STATUS_STARTED &&
            !streamDesc->newDeviceDescs_.empty() && streamDesc->newDeviceDescs_.front() &&
            streamDesc->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP;
    });
    if (willUseA2dp) {
        return false;
    }

    for (const auto &deviceDesc : excludeDeviceDescs) {
        CHECK_AND_CONTINUE(deviceDesc != nullptr && deviceDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP);
        std::string moduleName = AudioPolicyUtils::GetInstance().GetA2dpModuleName(
            deviceDesc->macAddress_, OUTPUT_DEVICE);
        audioPolicyManager_.StopAudioPort(moduleName);
    }
    if (Bluetooth::AudioA2dpManager::GetActiveA2dpDeviceLocal() != EMPTY_ADDRESS) {
        Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(NULL_ADDRESS);
    }
    return true;
}

static AppExecFwk::AppProcessState GetAppState(int32_t appPid)
{
    OHOS::AppExecFwk::AppMgrClient appManager;
    OHOS::AppExecFwk::RunningProcessInfo infos;
    int32_t res = appManager.GetRunningProcessInfoByPid(appPid, infos);
    if (res != ERR_OK) {
        AUDIO_WARNING_LOG("GetRunningProcessInfoByPid failed, appPid=%{public}d", appPid);
    }
    return infos.state_;
}

static uint32_t GetTimeCostFrom(int64_t timeNS)
{
    return static_cast<uint32_t>((ClockTime::GetCurNano() - timeNS) / AUDIO_NS_PER_SECOND);
}

static void GetHdiInfo(uint8_t &hdiSourceType, std::string &hdiSourceAlg)
{
    std::string hdiInfoStr = AudioServerProxy::GetInstance().GetAudioParameterProxy("concurrent_capture_stream_info");
    AUDIO_INFO_LOG("hdiInfo = %{public}s", hdiInfoStr.c_str());

    std::vector<std::string> hdiSegments;
    std::istringstream infoStream(hdiInfoStr);
    std::string segment;
    while (std::getline(infoStream, segment, '#')) {
        if (!segment.empty()) {
            hdiSegments.push_back(segment);
        }
    }

    if (hdiSegments.size() != CONCURRENT_CAPTURE_DFX_HDI_SEGMENTS) {
        hdiSourceType = 0;
        hdiSourceAlg.clear();
        return;
    }

    int sourceTypeInt = std::atoi(hdiSegments[0].c_str());
    if (sourceTypeInt == 0 && hdiSegments[0] != "0") {
        AUDIO_ERR_LOG("Failed to convert hdiSegments[0] to uint8_t");
        hdiSourceType = 0;
        hdiSourceAlg.clear();
        return;
    }

    hdiSourceType = static_cast<uint8_t>(sourceTypeInt);
    hdiSourceAlg = hdiSegments[1];
}

bool AudioCoreService::WriteCapturerConcurrentMsg(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    const std::unique_ptr<ConcurrentCaptureDfxResult> &result)
{
    CHECK_AND_RETURN_RET_LOG(result != nullptr, false, "result is null");
    std::vector<std::string> existingAppName{};
    std::vector<uint8_t> existingAppState{};
    std::vector<uint8_t> existingSourceType{};
    std::vector<uint8_t> existingCaptureState{};
    std::vector<uint32_t> existingCreateDuration{};
    std::vector<uint32_t> existingStartDuration{};
    std::vector<bool> existingFastFlag{};
    std::vector<std::shared_ptr<AudioStreamDescriptor>> capturerStreamDescs = pipeManager_->GetAllCapturerStreamDescs();
    if (capturerStreamDescs.size() < CONCURRENT_CAPTURE_DFX_THRESHOLD) {
        return false;
    }
    for (auto &desc : capturerStreamDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        if (existingAppName.size() >= CONCURRENT_CAPTURE_DFX_MSG_ARRAY_MAX) {
            break;
        }
        int32_t uid = desc->appInfo_.appUid;
        std::string bundleName = AudioBundleManager::GetBundleNameFromUid(uid);
        existingAppName.push_back(bundleName);
        existingAppState.push_back(static_cast<uint8_t>(GetAppState(desc->appInfo_.appPid)));
        existingSourceType.push_back(static_cast<uint8_t>(desc->capturerInfo_.sourceType));
        existingCaptureState.push_back(static_cast<uint8_t>(desc->streamStatus_));
        existingCreateDuration.push_back(GetTimeCostFrom(desc->createTimeStamp_));
        existingStartDuration.push_back(GetTimeCostFrom(desc->stateStartTimeStamp_));
        existingFastFlag.push_back(static_cast<bool>(desc->routeFlag_ & AUDIO_INPUT_FLAG_FAST));
    }
    result->existingAppName = std::move(existingAppName);
    result->existingAppState = std::move(existingAppState);
    result->existingSourceType = std::move(existingSourceType);
    result->existingCaptureState = std::move(existingCaptureState);
    result->existingCreateDuration = std::move(existingCreateDuration);
    result->existingStartDuration = std::move(existingStartDuration);
    result->existingFastFlag = std::move(existingFastFlag);
    GetHdiInfo(result->hdiSourceType, result->hdiSourceAlg);
    result->deviceType = streamDesc->newDeviceDescs_[0]->deviceType_;
    return true;
}

void AudioCoreService::LogCapturerConcurrentResult(const std::unique_ptr<ConcurrentCaptureDfxResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    size_t count = result->existingAppName.size();
    for (size_t i = 0; i < count; ++i) {
        AUDIO_INFO_LOG("------------------APP%{public}zu begin---------------------", i);
        AUDIO_INFO_LOG("AppState:         %{public}d", result->existingAppState[i]);
        AUDIO_INFO_LOG("SourceType:       %{public}d", result->existingSourceType[i]);
        AUDIO_INFO_LOG("CaptureState:     %{public}d", result->existingCaptureState[i]);
        AUDIO_INFO_LOG("CreateDuration: 0x%{public}u", result->existingCreateDuration[i]);
        AUDIO_INFO_LOG("StartDuration:  0x%{public}u", result->existingStartDuration[i]);
        AUDIO_INFO_LOG("FastFlag:         %{public}d", static_cast<uint32_t>(result->existingFastFlag[i]));
        AUDIO_INFO_LOG("hdiSourceType:    %{public}d", result->hdiSourceType);
        AUDIO_INFO_LOG("hdiSourceAlg:     %{public}s", result->hdiSourceAlg.c_str());
        AUDIO_INFO_LOG("deviceType:       %{public}d", result->deviceType);
        AUDIO_INFO_LOG("------------------APP%{public}zu end-----------------------", i);
    }
}

void AudioCoreService::WriteCapturerConcurrentEvent(const std::unique_ptr<ConcurrentCaptureDfxResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "CONCURRENT_CAPTURE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "EXISTING_APP_NAME", result->existingAppName,
        "EXISTING_APP_STATE", result->existingAppState,
        "EXISTING_SOURCE_TYPE", result->existingSourceType,
        "EXISTING_CAPTURE_STATE", result->existingCaptureState,
        "EXISTING_CREATE_DURATION", result->existingCreateDuration,
        "EXISTING_START_DURATION", result->existingStartDuration,
        "EXISTING_FAST_FLAG", result->existingFastFlag,
        "HDI_SOURCE_TYPE", result->hdiSourceType,
        "HDI_SOURCE_ALG", result->hdiSourceAlg,
        "DEVICE_TYPE", result->deviceType);
    if (ret) {
        AUDIO_ERR_LOG("Write event fail: CONCURRENT_CAPTURE, ret = %{public}d", ret);
    }
}

void AudioCoreService::UpdateRouteForCollaboration(InternalDeviceType deviceType, uint32_t pipeId)
{
    std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back(make_pair(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG));
    activeDevices.push_back(make_pair(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG));
    audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices, pipeId);
    AUDIO_INFO_LOG("collaboration Update desc [%{public}d] with speaker", deviceType);
}

int32_t AudioCoreService::SetSleVoiceStatusFlag(AudioScene audioScene)
{
    if (audioScene == AUDIO_SCENE_DEFAULT) {
        audioVolumeManager_.SetSleVoiceStatusFlag(false);
    } else {
        audioVolumeManager_.SetSleVoiceStatusFlag(true);
    }
    return SUCCESS;
}

int32_t AudioCoreService::PlayBackToInjection(uint32_t sessionId)
{
    int32_t ret = audioInjectorPolicy_.Init();
    audioInjectorPolicy_.SetInjectStreamsMuteForInjection(sessionId);
    return ret;
}

int32_t AudioCoreService::InjectionToPlayBack(uint32_t sessionId)
{
    int32_t ret = ERROR;
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is null");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERROR, "get streamDesc failed");
    streamDesc->rendererTarget_ = NORMAL_PLAYBACK;
    ret = AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("OnForcedDeviceSelected",
        AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "move stream out failed");
    audioInjectorPolicy_.SetInjectStreamsMuteForPlayback(sessionId);
    audioInjectorPolicy_.RemoveStreamDescriptor(sessionId);
    return SUCCESS;
}

void AudioCoreService::WriteScoStateFaultEvent(const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
#ifdef BLUETOOTH_ENABLE
    CHECK_AND_RETURN_LOG(devDesc != nullptr, "dev desc is null");
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipe manager is null");
    std::string eventString = "";
    eventString += "scene: " + std::to_string(audioSceneManager_.GetAudioScene())
        + " sco type: " + std::to_string(Bluetooth::AudioHfpManager::GetScoCategory())
        + " device type: " + std::to_string(devDesc->deviceType_)
        + " dm device type: " + std::to_string(devDesc->dmDeviceType_);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs = pipeManager_->GetAllOutputStreamDescs();
    for (auto &desc : outputStreamDescs) {
        CHECK_AND_RETURN_LOG(desc != nullptr, "stream desc is null");
        if (desc->streamStatus_ == STREAM_STATUS_STARTED || desc->audioFlag_ == AUDIO_OUTPUT_FLAG_VOIP)
        eventString += " stream session id: " + std::to_string(desc->sessionId_)
            + " stream status: " + std::to_string(desc->streamStatus_)
            + " stream usage: " + std::to_string(desc->rendererInfo_.streamUsage)
            + " bundle name: " + AudioBundleManager::GetBundleNameFromUid(desc->appInfo_.appUid);
    }
    AUDIO_INFO_LOG("Current audio: %{public}s", eventString.c_str());
#endif
}

bool AudioCoreService::HasRunningScoOutputStream(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputStreamDescs)
{
    for (const auto &desc : outputStreamDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        if (desc->streamStatus_ != STREAM_STATUS_STARTED || desc->newDeviceDescs_.empty()) {
            continue;
        }
        if (desc->newDeviceDescs_[0] != nullptr &&
            desc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            return true;
        }
    }
    return false;
}

bool AudioCoreService::HasRunningScoNormalCapturerStream(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &inputStreamDescs)
{
    for (const auto &desc : inputStreamDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        if ((desc->streamStatus_ == STREAM_STATUS_STARTED) && (desc->audioMode_ == AUDIO_MODE_RECORD) &&
            !Util::IsScoSupportSource(desc->capturerInfo_.sourceType) &&
            (!desc->newDeviceDescs_.empty() && desc->newDeviceDescs_[0] != nullptr &&
            desc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) {
            AUDIO_INFO_LOG("Running Normal Capturer stream : %{public}d", desc->sessionId_);
            return true;
        }
    }
    return false;
}

bool AudioCoreService::HasRunningScoRecognitionCapturerStream(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &inputStreamDescs)
{
    for (const auto &desc : inputStreamDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        if ((desc->streamStatus_ == STREAM_STATUS_STARTED) && (desc->audioMode_ == AUDIO_MODE_RECORD) &&
            Util::IsScoSupportSource(desc->capturerInfo_.sourceType) &&
            (!desc->newDeviceDescs_.empty() && desc->newDeviceDescs_[0] != nullptr &&
            desc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
            desc->newDeviceDescs_[0]->isVrSupported_)) {
            return true;
        }
    }
    return false;
}

void AudioCoreService::FetchDevicesForDescs(std::vector<std::shared_ptr<AudioStreamDescriptor>> &outStreamDescs,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &inStreamDescs)
{
    std::vector<FetchDeviceInfo> outInfos;
    for (auto &desc : outStreamDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
        FetchDeviceInfo info = { desc->rendererInfo_.streamUsage, GetRealUid(desc),
            desc->rendererInfo_.privacyType, "Arbiter", desc->bundleName_ };
        info.streamId = desc->sessionId_;
        outInfos.push_back(info);
    }
    auto outResult = audioRouterCenter_.FetchOutputDevicesForAllOutputStreams(outInfos);
    for (auto &desc : outStreamDescs) {
        CHECK_AND_CONTINUE(desc != nullptr);
        auto it = outResult.find(desc->sessionId_);
        CHECK_AND_CONTINUE(it != outResult.end());
        desc->UpdateNewDeviceWithoutCheck(it->second);
    }
    std::vector<FetchDeviceInfo> inInfos;
    for (auto &desc : inStreamDescs) {
        CHECK_AND_CONTINUE(desc != nullptr);
        FetchDeviceInfo info;
        info.sourceType = desc->capturerInfo_.sourceType;
        info.clientUID = GetRealUid(desc);
        info.streamId = desc->sessionId_;
        info.caller = "Arbiter";
        inInfos.push_back(info);
    }
    auto inResult = audioRouterCenter_.FetchInputDeviceForAllInputStreams(inInfos);
    for (auto &desc : inStreamDescs) {
        CHECK_AND_CONTINUE(desc != nullptr);
        auto it = inResult.find(desc->sessionId_);
        CHECK_AND_CONTINUE(it != inResult.end() && it->second != nullptr);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = {it->second};
        desc->UpdateNewDeviceWithoutCheck(devices);
    }
}

bool AudioCoreService::HandleRingToNonRingSceneChange(AudioScene lastAudioScene, AudioScene audioScene)
{
    bool ret = false;
    if ((lastAudioScene == AUDIO_SCENE_VOICE_RINGING || lastAudioScene == AUDIO_SCENE_RINGING) &&
        (audioScene == AUDIO_SCENE_DEFAULT || audioScene == AUDIO_SCENE_PHONE_CALL ||
            audioScene == AUDIO_SCENE_PHONE_CHAT)) {
        AUDIO_INFO_LOG("disable primary speaker dual tone when audio scene change from ring to non-ring");
        isRingDualToneOnPrimarySpeaker_ = false;
        ret = true;
    }
    return ret;
}

bool AudioCoreService::IsCallOrRingToDefault(AudioScene lastAudioScene, AudioScene audioScene)
{
    return (lastAudioScene == AUDIO_SCENE_VOICE_RINGING || lastAudioScene == AUDIO_SCENE_RINGING ||
        lastAudioScene == AUDIO_SCENE_PHONE_CALL || lastAudioScene == AUDIO_SCENE_PHONE_CHAT) &&
        audioScene == AUDIO_SCENE_DEFAULT;
}

AudioStreamDeviceChangeReasonExt AudioCoreService::UpdateRemoteDeviceChangeReason(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->oldDeviceDescs_.size() != 0, reason,
        "Invalid params");
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = streamDesc->oldDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(oldDeviceDesc != nullptr, reason, "oldDeviceDesc is nullptr");
    AudioStreamDeviceChangeReasonExt newReason = (oldDeviceDesc->dmDeviceType_ == DM_DEVICE_TYPE_WIFI_SOUNDBOX &&
        reason.IsDistributedDeviceUnavailable()) ?
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE : reason;
    return newReason;
}

void AudioCoreService::OnRemoteDeviceStatusUpdatedWhenNoRunningStream(std::shared_ptr<AudioDeviceDescriptor> newDesc)
{
    // For special remote devices, e.g. wifi soundbox, when switching from remote to other device
    // with no running stream, update device status
    auto currentDesc = std::make_shared<AudioDeviceDescriptor>(audioRouterSelectStrategy_.Get1stCurrentOutputDevice());
    CHECK_AND_RETURN_LOG(currentDesc != nullptr && newDesc != nullptr, "desc is nullptr");
    CHECK_AND_RETURN((currentDesc->dmDeviceType_ == DM_DEVICE_TYPE_WIFI_SOUNDBOX &&
        newDesc->dmDeviceType_ != DM_DEVICE_TYPE_WIFI_SOUNDBOX) ||
        (currentDesc->dmDeviceType_ == DM_DEVICE_TYPE_MUSIC_HOST &&
        newDesc->dmDeviceType_ != DM_DEVICE_TYPE_MUSIC_HOST));
    NotifyRemoteDeviceStatusUpdate(currentDesc);
}

void AudioCoreService::OnRemoteDeviceStatusUpdated()
{
    // For special remote devices, e.g. wifi soundbox, when all running streams switching from remote to other device,
    // update device status
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager_ is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs = pipeManager_->GetAllOutputStreamDescs();
    CHECK_AND_RETURN(outputStreamDescs.size() != 0);
    bool isNeedUpdate = true;
    auto oldDesc = std::make_shared<AudioDeviceDescriptor>();
    auto newDesc = std::make_shared<AudioDeviceDescriptor>();
    for (auto &streamDesc : outputStreamDescs) {
        CHECK_AND_CONTINUE(streamDesc != nullptr && streamDesc->oldDeviceDescs_.size() != 0 &&
            streamDesc->newDeviceDescs_.size() != 0);
        oldDesc = streamDesc->oldDeviceDescs_.front();
        newDesc = streamDesc->newDeviceDescs_.front();
        CHECK_AND_CONTINUE(oldDesc != nullptr && newDesc != nullptr);
        CHECK_AND_CONTINUE((oldDesc->dmDeviceType_ != DM_DEVICE_TYPE_WIFI_SOUNDBOX ||
            newDesc->dmDeviceType_ == DM_DEVICE_TYPE_WIFI_SOUNDBOX) &&
            (oldDesc->dmDeviceType_ != DM_DEVICE_TYPE_MUSIC_HOST ||
            newDesc->dmDeviceType_ == DM_DEVICE_TYPE_MUSIC_HOST));
        isNeedUpdate = false;
    }
    CHECK_AND_RETURN(isNeedUpdate);
    NotifyRemoteDeviceStatusUpdate(oldDesc);
}

void AudioCoreService::StopNoRunningPipe()
{
    std::vector<std::string> moduleNames = pipeManager_->GetNoRunningPipeModuleName();
    for (auto moduleName : moduleNames) {
        audioPolicyManager_.StopAudioPort(moduleName);
    }
}

void AudioCoreService::GetFlagForUltraFastStream(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.front() != nullptr, "Invalid stream desc");
    DeviceType deviceType = streamDesc->GetMainNewDeviceType();
    bool isUltraFastSupported = IsUltraFastSupportedForDevice(deviceType);
    CHECK_AND_RETURN_LOG(isUltraFastSupported, "Ultra fast is not supported for device type %{public}d", deviceType);
    CHECK_AND_RETURN_LOG(
        supportUltraFastBundleSet_.find(streamDesc->GetBundleName()) != supportUltraFastBundleSet_.end(),
        "Bundle name %{public}s is not in ultra fast support list", streamDesc->GetBundleName().c_str());
    if (!pipeManager_->IsUltraFastSupportedDevice(deviceType)) {
        AUDIO_INFO_LOG("Device type %{public}d is not support ultra fast mode", deviceType);
        // Issue: Missing check for recreate triggered route reselection.
        // Consequence: Every "device without primary ultra-fast support" whill be forced to use normal route,
        // in the presence of a 2.5ms stream.
        // This needs further consideration.
        if (AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType) != PRIMARY_SPEAKER) {
            streamDesc->SetAudioFlag(GetFlagForMmapStream(streamDesc));
        } else {
            streamDesc->SetFastStreamForcedNormalFlag(true);
            streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
        }
        return;
    }
    streamDesc->SetUltraFastRequested(true);
    streamDesc->SetAudioFlag(GetFlagForMmapStream(streamDesc));
    AUDIO_INFO_LOG("[StaticCheck] Enable ultra fast mode for stream %{public}d", streamDesc->GetSessionId());
}

bool AudioCoreService::CheckCollaborativeStreamRunning()
{
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, false, "pipeManager_ is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs = pipeManager_->GetAllOutputStreamDescs();
    for (auto &desc : outputStreamDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
        if (desc->isNearLinkCollaborationEnable_ && desc->streamStatus_ == STREAM_STATUS_STARTED) {
            return true;
        }
    }
    return false;
}

void AudioCoreService::UpdateNearLinkCollaborativeState(bool nearLinkCollaborativeState)
{
    AudioServerProxy::GetInstance().SetAudioParameterProxy("nearLink_collaborative_state",
        std::to_string(static_cast<int32_t>(nearLinkCollaborativeState)));
    AUDIO_INFO_LOG("UpdateNearLinkCollaborativeState, value:%{public}d", nearLinkCollaborativeState);
}

bool AudioCoreService::IsUltraFastSupportedForDevice(DeviceType deviceType)
{
    return (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) ? isSupportArmUsbUltraFast_ : isSupportUltraFast_;
}

void AudioCoreService::MutePrimarySinkByCollaborative(int32_t duration)
{
    audioIOHandleMap_.MuteSinkPort(PRIMARY_SPEAKER, duration, true, true);
}
} // namespace AudioStandard
} // namespace OHOS
