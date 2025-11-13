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
#define LOG_TAG "AudioCoreServicePrivate"
#endif

#include "audio_core_service.h"

#include <variant>

#include "system_ability.h"
#include "app_mgr_client.h"
#include "hisysevent.h"
#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "iservice_registry.h"
#include "hdi_adapter_info.h"
#include "audio_usb_manager.h"
#include "audio_spatialization_service.h"
#include "audio_collaborative_service.h"
#include "audio_stream_id_allocator.h"
#include "ipc_skeleton.h"
#include "audio_volume.h"
#include "audio_bundle_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t MEDIA_SERVICE_UID = 1013;
const int32_t DATA_LINK_CONNECTED = 11;
const uint32_t FIRST_SESSIONID = 100000;
const uid_t MCU_UID = 7500;
const uid_t TV_SERVICE_UID = 7501;
const int32_t AUDIO_EXT_UID = 1041;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
constexpr int32_t REMOTE_USER_TERMINATED = 200;
constexpr int32_t DUAL_CONNECTION_FAILURE = 201;
static const int VOLUME_LEVEL_DEFAULT_SIZE = 3;
static const int32_t BLUETOOTH_FETCH_RESULT_DEFAULT = 0;
static const int32_t BLUETOOTH_FETCH_RESULT_CONTINUE = 1;
static const int32_t BLUETOOTH_FETCH_RESULT_ERROR = 2;
static const int32_t REFETCH_DEVICE = 4;

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
static const uint32_t MEDIA_PAUSE_TO_DOUBLE_RING_DELAY_US = 120000; // 120ms
static const uint32_t VOICE_CALL_DEVICE_SET_DELAY_US = 120000; // 120ms

static const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;
static const int32_t WAIT_OFFLOAD_CLOSE_TIME_SEC = 10;
static const char* CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
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
}

static const std::vector<std::string> SourceNames = {
    std::string(PRIMARY_MIC),
    std::string(BLUETOOTH_MIC),
    std::string(USB_MIC),
    std::string(PRIMARY_WAKEUP),
    std::string(FILE_SOURCE),
    std::string(ACCESSORY_SOURCE),
    std::string(PRIMARY_AI_MIC)
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

void AudioCoreService::UpdateActiveDeviceAndVolumeBeforeMoveSession(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    HandleMuteBeforeDeviceSwitch(streamDescs, reason);
    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    uint32_t sessionId = 0;
    isActivateA2dpDeviceForLog_ = false;
    for (std::shared_ptr<AudioStreamDescriptor> &streamDesc : streamDescs) {
        // if streamDesc select bluetooth or headset, active it.
        if (!HandleOutputStreamInRunning(streamDesc, reason)) {
            continue;
        }

        int32_t outputRet = ActivateOutputDevice(streamDesc, reason);
        isActivateA2dpDeviceForLog_ = true;
        CHECK_AND_CONTINUE_LOG(outputRet == SUCCESS, "Activate output device failed");

        // update current output device
        if (needUpdateActiveDevice) {
            isUpdateActiveDevice = UpdateOutputDevice(streamDesc->newDeviceDescs_.front(), GetRealUid(streamDesc),
                reason);
            needUpdateActiveDevice = !isUpdateActiveDevice;
            sessionId = streamDesc->sessionId_;
        }
    }
    isActivateA2dpDeviceForLog_ = false;
    AudioDeviceDescriptor audioDeviceDescriptor = audioActiveDevice_.GetCurrentOutputDevice();
    std::shared_ptr<AudioDeviceDescriptor> descPtr =
        std::make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor);
    if (isUpdateActiveDevice && audioDeviceManager_.IsDeviceConnected(descPtr)) {
        AUDIO_INFO_LOG("active device updated, update volume for %{public}d", sessionId);
        audioVolumeManager_.SetVolumeForSwitchDevice(audioDeviceDescriptor, false);
        OnPreferredOutputDeviceUpdated(audioDeviceDescriptor, reason);
    }
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

void AudioCoreService::CheckAndUpdateOffloadEnableForStream(
    OffloadAction action, std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (action == OFFLOAD_NEW || action == OFFLOAD_MOVE_IN) {
        // Check stream is offload and then set
        if (streamDesc->IsRouteOffload()) {
            OffloadAdapter adapter = (streamDesc->IsDeviceRemote() ? OFFLOAD_IN_REMOTE : OFFLOAD_IN_PRIMARY);
            audioOffloadStream_.SetOffloadStatus(adapter, streamDesc->GetSessionId());
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
        auto callback = routeUpdateCallback_[sessionId];
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        std::shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_.front();
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "device desc is nullptr");
        callback->OnRouteUpdate(streamDesc->routeFlag_, desc->networkId_);
    }
}

int32_t AudioCoreService::FetchRendererPipesAndExecute(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    for (std::shared_ptr<AudioStreamDescriptor> streamDesc : streamDescs) {
        UpdatePlaybackStreamFlag(streamDesc, false);
    }
    AUDIO_INFO_LOG("[PipeFetchStart] all %{public}zu output streams", streamDescs.size());
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipesAndExecute(streamDescs);

    // Update a2dp offload flag here because UpdateActiveRoute() need actual flag.
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();

    uint32_t audioFlag;
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        UpdateOffloadState(pipeInfo);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessOutputPipeUpdate(pipeInfo, audioFlag, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessOutputPipeNew(pipeInfo, audioFlag, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) {
            ProcessOutputPipeReload(pipeInfo, audioFlag, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    audioIOHandleMap_.NotifyUnmutePort();
    pipeManager_->UpdateRendererPipeInfos(pipeInfos);
    RemoveUnusedPipe();
    NotifyRouteUpdate(streamDescs);
    return SUCCESS;
}

int32_t AudioCoreService::FetchCapturerPipesAndExecute(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    AUDIO_INFO_LOG("[PipeFetchStart] all %{public}zu input streams", streamDescs.size());
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipesAndExecute(streamDescs);

    bool removeFlag = false;
    uint32_t fetchStreamId = UINT32_INVALID_VALUE;
    audioInjectorPolicy_.FetchCapDeviceInjectPreProc(pipeInfos, removeFlag, fetchStreamId);

    AUDIO_INFO_LOG("[PipeExecStart] for all Pipes");
    uint32_t audioFlag;
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        HILOG_COMM_INFO("[PipeExecInfo] Scan Pipe adapter: %{public}s, name: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->name_.c_str(), pipeInfo->pipeAction_);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessInputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessInputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    pipeManager_->UpdateCapturerPipeInfos(pipeInfos);
    RemoveUnusedPipe();

    audioInjectorPolicy_.FetchCapDeviceInjectPostProc(pipeInfos, removeFlag, fetchStreamId);
    return SUCCESS;
}

int32_t AudioCoreService::ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
    ConnectState connectState)
{
    HILOG_COMM_INFO("handleflag %{public}d, address %{public}s, connectState %{public}d",
        handleFlag, GetEncryptAddr(address).c_str(), connectState);
    if (handleFlag && connectState != DEACTIVE_CONNECTED) {
        return SUCCESS;
    }
    return Bluetooth::AudioHfpManager::HandleScoWithRecongnition(handleFlag);
}

void AudioCoreService::BluetoothScoFetch(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("AudioCoreService::BluetoothScoFetch");
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, "invalid streamDesc");
    shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_[0];
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch input device");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        FetchInputDeviceAndRoute("BluetoothScoFetch");
        return;
    }

    if (streamDesc->streamStatus_ != STREAM_STATUS_STARTED) {
        return;
    }
    bool hasRunningRecognitionCapturerStream = streamCollector_.HasRunningRecognitionCapturerStream();
    if (Util::IsScoSupportSource(streamDesc->capturerInfo_.sourceType) || hasRunningRecognitionCapturerStream) {
        ret = ScoInputDeviceFetchedForRecongnition(true, desc->macAddress_, desc->connectState_);
    } else {
        ret = Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true), true);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sco [%{public}s] is not connected yet",
            GetEncryptAddr(desc->macAddress_).c_str());
    }
}

void AudioCoreService::CheckModemScene(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    const AudioStreamDeviceChangeReasonExt reason)
{
    if (!pipeManager_->IsModemCommunicationIdExist()) {
        return;
    }

    bool isModemCallRunning = audioSceneManager_.IsInPhoneCallScene();
    if (isModemCallRunning) {
        pipeManager_->UpdateModemStreamStatus(STREAM_STATUS_STARTED);
    } else {
        pipeManager_->UpdateModemStreamStatus(STREAM_STATUS_STOPPED);
    }
    descs = audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, -1, "CheckModemScene");
    CHECK_AND_RETURN_LOG(descs.size() != 0, "Fetch output device for voice modem communication failed");
    pipeManager_->UpdateModemStreamDevice(descs);
    AudioDeviceDescriptor curDesc = audioActiveDevice_.GetCurrentOutputDevice();
    AUDIO_INFO_LOG("Current output device %{public}d, update route %{public}d, reason %{public}d",
        curDesc.deviceType_, descs.front()->deviceType_, static_cast<int32_t>(reason));
    if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        auto modemCommunicationMap = pipeManager_->GetModemCommunicationMap();
        auto modemMap = modemCommunicationMap.begin();
        if (modemMap != modemCommunicationMap.end()) {
            int32_t ret = HandleScoOutputDeviceFetched(modemMap->second, reason);
            AUDIO_INFO_LOG("HandleScoOutputDeviceFetched %{public}d", ret);
        }
    }
    if (descs.front()->deviceType_ == DEVICE_TYPE_HEARING_AID) {
        SwitchActiveHearingAidDevice(std::make_shared<AudioDeviceDescriptor>(descs.front()));
    }
    auto ret = ActivateNearlinkDevice(pipeManager_->GetModemCommunicationMap().begin()->second, reason);
    // If the modem call is in progress, and the device is currently switching,
    // and the current output device is different from the target device, then mute to avoid pop issue.
    if (isModemCallRunning && IsDeviceSwitching(reason) && !curDesc.IsSameDeviceDesc(*descs.front())) {
        SetVoiceCallMuteForSwitchDevice();
        needUnmuteVoiceCall_ = true;
        SetUpdateModemRouteFinished(false);
        uint32_t muteDuration = GetVoiceCallMuteDuration(curDesc, *descs.front());
        std::thread switchThread(
            &AudioCoreService::UnmuteVoiceCallAfterMuteDuration, this, muteDuration, descs.front());
        switchThread.detach();
    }
    CheckAndUpdateHearingAidCall(descs.front()->deviceType_);
    CheckAndSleepBeforeVoiceCallDeviceSet(reason);
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
            descs.front()->deviceName_, LOCAL_NETWORK_ID);
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
    return SUCCESS;
}

uint32_t AudioCoreService::GetVoiceCallMuteDuration(AudioDeviceDescriptor &curDesc, AudioDeviceDescriptor &newDesc)
{
    uint32_t muteDuration = 0;
    if (!curDesc.IsSameDeviceDesc(newDesc) &&
        !(curDesc.IsSpeakerOrEarpiece() && newDesc.IsSpeakerOrEarpiece())) {
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
    audioVolumeManager_.SetVolumeForSwitchDevice(*desc, true);
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
                audioCapturerSession_.SetHearingAidReloadFlag(false);
            } else {
                audioCapturerSession_.ReloadCaptureSessionSoftLink();
            }
        }
    }
}

void AudioCoreService::CheckOpenHearingAidCall(const bool isModemCallRunning, const DeviceType type)
{
    if (!hearingAidCallFlag_) {
        if (isModemCallRunning && type == DEVICE_TYPE_HEARING_AID) {
            uint32_t paIndex = 0;
            CHECK_AND_RETURN_LOG(CheckModuleForHearingAid(paIndex) == SUCCESS, "openAudioPort failed");

            std::shared_ptr<AudioPipeInfo> pipeInfoOutput = pipeManager_->GetPipeinfoByNameAndFlag("hearing_aid",
                AUDIO_OUTPUT_FLAG_NORMAL);
            CHECK_AND_RETURN_LOG(pipeInfoOutput != nullptr, "Can not find pipe hearing_aid");

            audioActiveDevice_.UpdateActiveDeviceRoute(DeviceType::DEVICE_TYPE_SPEAKER,
                DeviceFlag::OUTPUT_DEVICES_FLAG);
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
    bool configRet = policyConfigMananger_.GetModuleListByType(ClassType::TYPE_PRIMARY, moduleInfoList);
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
            CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
                "OpenAudioPort failed ioHandle[%{public}u]", ioHandle);
            CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
                "OpenAudioPort failed paId[%{public}u]", paIndex);
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
            audioCapturerSession_.SetHearingAidReloadFlag(true);
        } else {
            int32_t ret = audioCapturerSession_.ReloadCaptureSoftLink(pipeInfoInput, moduleInfo);
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

void AudioCoreService::HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_RECORD &&
        (streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_RELEASED ||
         streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_STOPPED)) {
        auto sourceType = streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType;
        auto sessionId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
        if (Util::IsScoSupportSource(sourceType)) {
            audioStateManager_.SetPreferredRecognitionCaptureDevice(make_shared<AudioDeviceDescriptor>());
            Bluetooth::AudioHfpManager::HandleScoWithRecongnition(false);
        } else {
            AUDIO_INFO_LOG("close capture app, try to disconnect sco");
            bool isRecord = streamCollector_.HasRunningNormalCapturerStream(DEVICE_TYPE_BLUETOOTH_SCO);
            Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true), isRecord);
        }
        audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptorBySessionID(sessionId);
    }
}

void AudioCoreService::UpdateDefaultOutputDeviceWhenStopping(int32_t uid)
{
    std::vector<uint32_t> sessionIDSet = streamCollector_.GetAllRendererSessionIDForUID(uid);
    for (const auto &sessionID : sessionIDSet) {
        audioDeviceManager_.UpdateDefaultOutputDeviceWhenStopping(sessionID);
        audioDeviceManager_.RemoveSelectedDefaultOutputDevice(sessionID);
	if (isRingDualToneOnPrimarySpeaker_ && (streamCollector_.GetStreamType(sessionID) == STREAM_RING ||
            streamCollector_.GetStreamType(sessionID) == STREAM_ALARM)) {
            AUDIO_INFO_LOG("disable primary speaker dual tone when ringer renderer died");
            isRingDualToneOnPrimarySpeaker_ = false;
            for (std::pair<uint32_t, AudioStreamType> stream : streamsWhenRingDualOnPrimarySpeaker_) {
                AudioVolume::GetInstance()->SetStreamVolumeMute(stream.first, false);
            }
            streamsWhenRingDualOnPrimarySpeaker_.clear();
            AudioStreamType streamType = streamCollector_.GetStreamType(sessionID);
            if (streamType == STREAM_MUSIC) {
                AudioVolume::GetInstance()->SetStreamVolumeMute(sessionID, false);
            }
        }
    }
}

void AudioCoreService::UpdateInputDeviceWhenStopping(int32_t uid)
{
    std::vector<uint32_t> sessionIDSet = streamCollector_.GetAllCapturerSessionIDForUID(uid);
    for (const auto &sessionID : sessionIDSet) {
        audioDeviceManager_.RemoveSelectedInputDevice(sessionID);
        audioDeviceManager_.RemovePreferredInputDevice(sessionID);
    }
    FetchInputDeviceAndRoute("UpdateInputDeviceWhenStopping");
}

int32_t AudioCoreService::BluetoothDeviceFetchOutputHandle(shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason, std::string encryptMacAddr)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, BLUETOOTH_FETCH_RESULT_ERROR, "Stream desc is nullptr");
    std::shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, BLUETOOTH_FETCH_RESULT_CONTINUE, "Device desc is nullptr");

    ResetNearlinkDeviceState(desc, streamDesc->streamStatus_ == STREAM_STATUS_STARTED);

    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string sinkPort = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_BLUETOOTH_A2DP);
        audioPolicyManager_.SuspendAudioDevice(sinkPort, false);
        int32_t ret = ActivateA2dpDeviceWhenDescEnabled(desc, reason);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("Activate a2dp [%{public}s] failed", encryptMacAddr.c_str());
            return BLUETOOTH_FETCH_RESULT_ERROR;
        }
    } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        int32_t ret = HandleScoOutputDeviceFetched(streamDesc, reason);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("sco [%{public}s] is not connected yet", encryptMacAddr.c_str());
            return BLUETOOTH_FETCH_RESULT_ERROR;
        }
    }
    return BLUETOOTH_FETCH_RESULT_DEFAULT;
}

int32_t AudioCoreService::ActivateA2dpDeviceWhenDescEnabled(shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NULL_POINTER, "invalid deviceDesc");
    if (desc->isEnable_) {
        return ActivateA2dpDevice(desc, reason);
    }
    return SUCCESS;
}


int32_t AudioCoreService::ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::ActiveA2dpDevice");
    int32_t ret = SwitchActiveA2dpDevice(desc);
    JUDGE_AND_INFO_LOG(isActivateA2dpDeviceForLog_ == false, "ret : %{public}d", ret);
    // In plan: re-try when failed
    return ret;
}

int32_t AudioCoreService::SwitchActiveA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr &&
        audioA2dpDevice_.CheckA2dpDeviceExist(deviceDescriptor->macAddress_),
        ERR_INVALID_PARAM, "Target A2DP device doesn't exist.");
    int32_t result = ERROR;
#ifdef BLUETOOTH_ENABLE
    std::string lastActiveA2dpDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    audioActiveDevice_.SetActiveBtDeviceMac(deviceDescriptor->macAddress_);
    AudioDeviceDescriptor lastDevice = audioPolicyManager_.GetActiveDeviceDescriptor();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    if (Bluetooth::AudioA2dpManager::GetActiveA2dpDevice() == deviceDescriptor->macAddress_ &&
        audioIOHandleMap_.CheckIOHandleExist(BLUETOOTH_SPEAKER)) {
        AUDIO_WARNING_LOG("A2dp device [%{public}s] [%{public}s] is already active",
            GetEncryptAddr(deviceDescriptor->macAddress_).c_str(), deviceDescriptor->deviceName_.c_str());
        return SUCCESS;
    }

    result = Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(deviceDescriptor->macAddress_);
    if (result != SUCCESS) {
        audioActiveDevice_.SetActiveBtDeviceMac(lastActiveA2dpDevice);
        AUDIO_ERR_LOG("Active [%{public}s] failed, using original [%{public}s] device",
            GetEncryptAddr(audioActiveDevice_.GetActiveBtDeviceMac()).c_str(),
            GetEncryptAddr(lastActiveA2dpDevice).c_str());
        return result;
    }

    AudioStreamInfo audioStreamInfo = {};
    audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo);
    std::string networkId = audioActiveDevice_.GetCurrentOutputDeviceNetworkId();
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(
        audioActiveDevice_.GetCurrentOutputDeviceType());
    result = LoadA2dpModule(DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, networkId, sinkName, SOURCE_TYPE_INVALID);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "LoadA2dpModule failed %{public}d", result);
#endif
    return result;
}

int32_t AudioCoreService::LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
    std::string networkId, std::string sinkName, SourceType sourceType)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = policyConfigMananger_.GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED, "A2dp module is not exist in the configuration file");

    // not load bt_a2dp_fast and bt_hdap, maybe need fix
    int32_t loadRet = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_a2dp");
    if (loadRet) {
        AUDIO_ERR_LOG("load adapter failed");
    }
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "source" ? INPUT_DEVICE : OUTPUT_DEVICE;
        DeviceRole deviceRole = deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load a2dp module [%{public}s], role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            AUDIO_INFO_LOG("A2dp device connects for the first time");
            // a2dp device connects for the first time
            GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
            uint32_t paIndex = 0;
            AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
            CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID,
                ERR_INVALID_HANDLE, "OpenAudioPort failed ioHandle[%{public}u]", ioHandle);
            CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE,
                ERR_OPERATION_FAILED, "OpenAudioPort failed paId[%{public}u]", paIndex);
            audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);

            std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
            pipeInfo->id_ = ioHandle;
            pipeInfo->paIndex_ = paIndex;
            if (moduleInfo.role == "sink") {
                pipeInfo->name_ = "a2dp_output";
                pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
                pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
            } else {
                pipeInfo->name_ = "a2dp_input";
                pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
                pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
            }
            pipeInfo->adapterName_ = "a2dp";
            pipeInfo->moduleInfo_ = moduleInfo;
            pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
            pipeInfo->InitAudioStreamInfo();
            pipeManager_->AddAudioPipeInfo(pipeInfo);
            AUDIO_INFO_LOG("Add PipeInfo %{public}u in load a2dp.", pipeInfo->id_);
        } else {
            // At least one a2dp device is already connected. A new a2dp device is connecting.
            // Need to reload a2dp module when switching to a2dp device.
            int32_t result = ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkId, sinkName,
                sourceType);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "ReloadA2dpAudioPort failed %{public}d", result);
        }
    }

    return SUCCESS;
}

int32_t AudioCoreService::ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, DeviceType deviceType,
    const AudioStreamInfo &audioStreamInfo, std::string networkId, std::string sinkName,
    SourceType sourceType)
{
    AUDIO_INFO_LOG("Switch device from a2dp to another a2dp, reload a2dp module");
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        audioIOHandleMap_.MuteDefaultSinkPort(networkId, sinkName);
    }

    // Firstly, unload the existing a2dp sink or source.
    std::string portName = BLUETOOTH_SPEAKER;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        portName = BLUETOOTH_MIC;
    }
    AudioIOHandle activateDeviceIOHandle;
    audioIOHandleMap_.GetModuleIdByKey(portName, activateDeviceIOHandle);
    uint32_t curPaIndex = pipeManager_->GetPaIndexByIoHandle(activateDeviceIOHandle);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs =
        pipeManager_->GetStreamDescsByIoHandle(activateDeviceIOHandle);
    AUDIO_INFO_LOG("IoHandleId: %{public}u, paIndex: %{public}u, stream num: %{public}zu",
        activateDeviceIOHandle, curPaIndex, streamDescs.size());
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag != 1) {
        int32_t result = audioPolicyManager_.CloseAudioPort(activateDeviceIOHandle, curPaIndex);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "CloseAudioPort failed %{public}d", result);
    }
    pipeManager_->RemoveAudioPipeInfo(activateDeviceIOHandle);

    // Load a2dp sink or source module again with the configuration of active a2dp device.
    GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
    uint32_t paIndex = 0;
    AudioIOHandle ioHandle = ReloadOrOpenAudioPort(engineFlag, moduleInfo, paIndex);
    audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    if (moduleInfo.role == "sink") {
        pipeInfo->name_ = "a2dp_output";
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    } else {
        pipeInfo->name_ = "a2dp_input";
        pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
        pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    }
    pipeInfo->adapterName_ = "a2dp";
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->InitAudioStreamInfo();
    pipeInfo->streamDescriptors_.insert(pipeInfo->streamDescriptors_.end(), streamDescs.begin(), streamDescs.end());
    pipeManager_->AddAudioPipeInfo(pipeInfo);
    AUDIO_INFO_LOG("Close paIndex: %{public}u, open paIndex: %{public}u", curPaIndex, paIndex);
    return SUCCESS;
}

AudioIOHandle AudioCoreService::ReloadOrOpenAudioPort(int32_t engineFlag, AudioModuleInfo &moduleInfo,
    uint32_t &paIndex)
{
    AudioIOHandle ioHandle;
    if (engineFlag == 1) {
        ioHandle = audioPolicyManager_.ReloadA2dpAudioPort(moduleInfo, paIndex);
        CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
            "ReloadAudioPort failed ioHandle[%{public}u]", ioHandle);
        CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
            "ReloadAudioPort failed paId[%{public}u]", paIndex);
    } else {
        ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
        CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
            "OpenAudioPort failed ioHandle[%{public}u]", ioHandle);
        CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
            "OpenAudioPort failed paId[%{public}u]", paIndex);
    }
    return ioHandle;
}

void AudioCoreService::ProcessOutputPipeReload(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    int32_t engineFlag = GetEngineFlag();
    uint32_t paIndex = HDI_INVALID_ID;
    CHECK_AND_RETURN_LOG(engineFlag == 1, "not find proaudio port");

    audioPolicyManager_.ReloadAudioPort(pipeInfo->moduleInfo_, paIndex);
    CHECK_AND_RETURN_LOG(paIndex != HDI_INVALID_ID, "ReloadAudioPort failed paId[%{public}u]", paIndex);

    pipeInfo->paIndex_ = paIndex;
    ProcessOutputPipeUpdate(pipeInfo, flag, reason);
}

void AudioCoreService::GetA2dpModuleInfo(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo,
    SourceType sourceType)
{
    uint32_t bufferSize = audioStreamInfo.samplingRate *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(audioStreamInfo.format) *
        audioStreamInfo.channels / BT_BUFFER_ADJUSTMENT_FACTOR;
    AUDIO_INFO_LOG("a2dp rate: %{public}d, format: %{public}d, channel: %{public}d",
        audioStreamInfo.samplingRate, audioStreamInfo.format, audioStreamInfo.channels);
    moduleInfo.channels = to_string(audioStreamInfo.channels);
    moduleInfo.rate = to_string(audioStreamInfo.samplingRate);
    moduleInfo.format = AudioPolicyUtils::GetInstance().ConvertToHDIAudioFormat(audioStreamInfo.format);
    moduleInfo.bufferSize = to_string(bufferSize);
    if (moduleInfo.role != "source") {
        moduleInfo.renderInIdleState = "1";
        moduleInfo.sinkLatency = "0";
    }
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
    }
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

int32_t AudioCoreService::FetchDeviceAndRoute(std::string caller, const AudioStreamDeviceChangeReasonExt reason)
{
    int32_t ret = FetchOutputDeviceAndRoute(caller + "FetchDeviceAndRoute", reason);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fetch output device failed");
    return FetchInputDeviceAndRoute(caller + "FetchDeviceAndRoute", reason);
}

int32_t AudioCoreService::FetchRendererPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    uint32_t &sessionId, uint32_t &audioFlag, const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "stream desc is nullptr");
    UpdatePlaybackStreamFlag(streamDesc, true);
    AUDIO_INFO_LOG("[PipeFetchStart] AudioFlag 0x%{public}x for stream %{public}d", streamDesc->audioFlag_, sessionId);
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipeAndExecute(streamDesc);

    uint32_t sinkId = HDI_INVALID_ID;
    for (auto &pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        HILOG_COMM_INFO("[PipeExecInfo] Scan Pipe adapter: %{public}s, name: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->name_.c_str(), pipeInfo->pipeAction_);
        UpdateOffloadState(pipeInfo);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessOutputPipeUpdate(pipeInfo, audioFlag, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) { // new
            ProcessOutputPipeNew(pipeInfo, audioFlag, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) {
            ProcessOutputPipeReload(pipeInfo, audioFlag, reason);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) { // DEFAULT
            // Do nothing
        }
    }
    RemoveUnusedPipe();
    return SUCCESS;
}

void AudioCoreService::ProcessOutputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    uint32_t paIndex = 0;
    uint32_t id = OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    CHECK_AND_RETURN_LOG(id != HDI_INVALID_ID, "Invalid id: %{public}u", id);
    CHECK_AND_RETURN_LOG(paIndex != OPEN_PORT_FAILURE, "Invalid paIndex: %{public}u", paIndex);
    pipeInfo->id_ = id;
    pipeInfo->paIndex_ = paIndex;

    for (auto &desc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_NEW, desc);
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_MOVE:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, desc);
                if (desc->streamStatus_ != STREAM_STATUS_STARTED) {
                    MoveStreamSink(desc, pipeInfo, reason);
                } else {
                    MoveToNewOutputDevice(desc, pipeInfo, reason);
                }
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_IN, desc);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateRendererStreamCallbackEntry(desc, reason);
                break;
            default:
                break;
        }
    }
    pipeManager_->AddAudioPipeInfo(pipeInfo);
}

void AudioCoreService::ProcessOutputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    for (auto &desc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_NEW, desc);
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, desc);
                if (desc->streamStatus_ != STREAM_STATUS_STARTED) {
                    MoveStreamSink(desc, pipeInfo, reason);
                } else {
                    MoveToNewOutputDevice(desc, pipeInfo, reason);
                }
                CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_IN, desc);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateRendererStreamCallbackEntry(desc, reason);
                break;
            default:
                break;
        }
        audioPipeSelector_->UpdateRendererPipeInfo(desc);
    }
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

    for (auto &pipeInfo : pipeInfos) {
        HILOG_COMM_INFO("[PipeExecInfo] Scan Pipe adapter: %{public}s, name: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->name_.c_str(), pipeInfo->pipeAction_);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessInputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessInputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    RemoveUnusedPipe();
    return SUCCESS;
}

void AudioCoreService::ProcessInputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    uint32_t paIndex = 0;
    uint32_t sourceId = OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    pipeInfo->id_ = sourceId;
    pipeInfo->paIndex_ = paIndex;
    std::vector<SourceOutput> sourceOutputs = GetSourceOutputs();

    for (auto &desc : pipeInfo->streamDescriptors_) {
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                if (desc->streamStatus_ != STREAM_STATUS_STARTED) {
                    MoveStreamSource(desc, sourceOutputs);
                } else {
                    MoveToNewInputDevice(desc, sourceOutputs);
                }
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateCapturerStreamCallback(desc);
                break;
            default:
                break;
        }
    }
    pipeManager_->AddAudioPipeInfo(pipeInfo);
}

void AudioCoreService::ProcessInputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    std::vector<SourceOutput> sourceOutputs = GetSourceOutputs();
    for (auto desc : pipeInfo->streamDescriptors_) {
        HILOG_COMM_INFO("[StreamExecInfo] Stream: %{public}u, action: %{public}d, belong to %{public}s",
            desc->sessionId_, desc->streamAction_, pipeInfo->name_.c_str());
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                if (desc->streamStatus_ != STREAM_STATUS_STARTED) {
                    MoveStreamSource(desc, sourceOutputs);
                } else {
                    MoveToNewInputDevice(desc, sourceOutputs);
                }
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateCapturerStreamCallback(desc);
                break;
            default:
                break;
        }
    }
    pipeManager_->UpdateAudioPipeInfo(pipeInfo);
}

void AudioCoreService::RemoveUnusedPipe()
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = pipeManager_->GetUnusedPipe();
    for (auto pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        AUDIO_INFO_LOG("[PipeExecInfo] Remove and close Pipe %{public}s", pipeInfo->ToString().c_str());
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER) {
            OffloadType type = pipeInfo->moduleInfo_.className == "remote_offload" ? REMOTE_OFFLOAD : LOCAL_OFFLOAD;
            if (type == REMOTE_OFFLOAD) {
                CHECK_AND_CONTINUE(isOffloadOpened_[type].load());
                isOffloadOpened_[type].store(false);
            } else {
                DelayReleaseOffloadPipe(pipeInfo->id_, pipeInfo->paIndex_, type);
                continue;
            }
        }
        audioPolicyManager_.CloseAudioPort(pipeInfo->id_, pipeInfo->paIndex_);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.DelIOHandleInfo(pipeInfo->moduleInfo_.name);
    }
}

void AudioCoreService::RemoveUnusedRecordPipe()
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = pipeManager_->GetUnusedRecordPipe();
    for (auto pipeInfo : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        AUDIO_INFO_LOG("[PipeExecInfo] Remove and close Pipe %{public}s", pipeInfo->ToString().c_str());
        audioPolicyManager_.CloseAudioPort(pipeInfo->id_, pipeInfo->paIndex_);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.DelIOHandleInfo(pipeInfo->moduleInfo_.name);
    }
}

std::string AudioCoreService::GetAdapterNameBySessionId(uint32_t sessionId)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    std::string adapterName = pipeManager_->GetAdapterNameBySessionId(sessionId);
    return adapterName;
}

int32_t AudioCoreService::GetProcessDeviceInfoBySessionId(uint32_t sessionId,
    AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    deviceInfo = AudioDeviceDescriptor(pipeManager_->GetProcessDeviceInfoBySessionId(sessionId, streamInfo));
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
        && updatedDesc.deviceCategory_ != BT_UNWEAR_HEADPHONE
        && !audioDeviceManager_.GetScoState()
        && updatedDesc.deviceCategory_ != BT_WATCH) {
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
    AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s], reason %{public}d",
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

void AudioCoreService::MoveToNewOutputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::shared_ptr<AudioPipeInfo> pipeInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::MoveToNewOutputDevice");

    DeviceType oldDeviceType = DEVICE_TYPE_NONE;
    bool isNeedTriggerCallback = true;
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = streamDesc->newDeviceDescs_.front();
    std::string oldSinkName = "";
    if (streamDesc->oldDeviceDescs_.size() == 0) {
        AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s], reason %{public}d",
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
        audioPolicyServerHandler_->SendRendererDeviceChangeEvent(streamDesc->callerPid_,
            streamDesc->sessionId_, callbackDesc, reason);
    }

    SleepForSwitchDevice(streamDesc, reason);

    CHECK_AND_RETURN_LOG(IsNewDevicePlaybackSupported(streamDesc), "new device not support playback");

    auto ret = (newDeviceDesc->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalOutputDevice(targetSinkInputs, pipeInfo, newDeviceDesc)
        : MoveToRemoteOutputDevice(targetSinkInputs, pipeInfo, newDeviceDesc);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Move sink input %{public}d to device %{public}d failed!",
            streamDesc->sessionId_, newDeviceDesc->deviceType_);
        audioIOHandleMap_.NotifyUnmutePort();
        return;
    }

    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    if (policyConfigMananger_.GetUpdateRouteSupport()) {
        UpdateOutputRoute(streamDesc);
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
    return audioDeviceStatus_.OnServiceConnected(serviceIndex);
}

void AudioCoreService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter)
{
    audioDeviceStatus_.OnForcedDeviceSelected(devType, macAddress, filter);
}


void AudioCoreService::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    audioDeviceStatus_.OnPrivacyDeviceSelected(devType, macAddress);
}

void AudioCoreService::UpdateRemoteOffloadModuleName(std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &moduleName)
{
    CHECK_AND_RETURN(pipeInfo && pipeInfo->moduleInfo_.className == "remote_offload");
    moduleName = pipeInfo->moduleInfo_.name;
    AUDIO_INFO_LOG("remote offload, set module name %{public}s", moduleName.c_str());
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
    UpdateRemoteOffloadModuleName(pipeInfo, moduleName);
    AUDIO_ERR_LOG("moduleName %{public}s", moduleName.c_str());

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
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "move [%{public}d] failed", sinkInputIds[i].streamId);
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

    AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s]",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_,
        GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());

    // MoveSourceOuputByIndexName
    auto ret = (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front(), streamDesc->routeFlag_)
        : MoveToRemoteInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front());
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "Move source output %{public}d to device %{public}d failed!",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_);
    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    streamCollector_.UpdateCapturerDeviceInfo(streamDesc->newDeviceDescs_.front());
}

void AudioCoreService::MoveToNewInputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    const std::vector<SourceOutput>& sourceOutputs)
{
    Trace trace("AudioCoreService::MoveToNewInputDevice");
    std::vector<SourceOutput> targetSourceOutputs = FilterSourceOutputs(streamDesc->sessionId_, sourceOutputs);

    if (streamDesc->oldDeviceDescs_.size() == 0) {
        AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u to [%{public}d][%{public}s]",
            streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());
    } else {
        AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u [%{public}d][%{public}s] to [%{public}d][%{public}s]",
            streamDesc->sessionId_, streamDesc->oldDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->oldDeviceDescs_.front()->macAddress_).c_str(),
            streamDesc->newDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());
    }

    // MoveSourceOuputByIndexName
    auto ret = (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front(), streamDesc->routeFlag_)
        : MoveToRemoteInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front());
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "Move source output %{public}d to device %{public}d failed!",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_);

    if (policyConfigMananger_.GetUpdateRouteSupport() &&
        streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID) {
        audioActiveDevice_.UpdateActiveDeviceRoute(streamDesc->newDeviceDescs_.front()->deviceType_,
            DeviceFlag::INPUT_DEVICES_FLAG, streamDesc->newDeviceDescs_.front()->deviceName_,
            streamDesc->newDeviceDescs_.front()->networkId_);
    }

    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    streamCollector_.UpdateCapturerDeviceInfo(streamDesc->newDeviceDescs_.front());
}

int32_t AudioCoreService::MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor, uint32_t routeFlag)
{
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_, ERR_INVALID_OPERATION,
        "failed: not a local device.");

    uint32_t sourceId = -1; // invalid source id, use source name instead.
    std::string sourceName = AudioPolicyUtils::GetInstance().GetSourcePortName(localDeviceDescriptor->deviceType_,
        routeFlag);
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, sourceName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
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
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
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
    {
        std::unordered_map<std::string, AudioIOHandle> mapCopy = AudioIOHandleMap::GetInstance().GetCopy();
        if (std::any_of(mapCopy.cbegin(), mapCopy.cend(), [](const auto &pair) {
                return std::find(SourceNames.cbegin(), SourceNames.cend(), pair.first) != SourceNames.cend();
            })) {
            sourceOutputs = audioPolicyManager_.GetAllSourceOutputs();
        }
    }
    return sourceOutputs;
}

void AudioCoreService::UpdateRingerOrAlarmerDualDeviceOutputRouter(
    std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.front() != nullptr, "streamDesc is nullptr");
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    InternalDeviceType deviceType = streamDesc->newDeviceDescs_.front()->deviceType_;
    if (!SelectRingerOrAlarmDevices(streamDesc)) {
        audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG,
            streamDesc->newDeviceDescs_.front()->deviceName_, streamDesc->newDeviceDescs_.front()->networkId_);
    }
    if (streamUsage == STREAM_USAGE_ALARM) {
        audioVolumeManager_.SetRingerModeMute(true);
        shouldUpdateDeviceDueToDualTone_ = true;
        return;
    }
    AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
    if (ringerMode != RINGER_MODE_NORMAL &&
        IsRingerOrAlarmerDualDevicesRange(streamDesc->newDeviceDescs_.front()->getType()) &&
        streamDesc->newDeviceDescs_.front()->getType() != DEVICE_TYPE_SPEAKER) {
        audioPolicyManager_.SetDeviceNoMuteForRinger(streamDesc->newDeviceDescs_.front());
        audioVolumeManager_.SetRingerModeMute(false);
        if (audioPolicyManager_.GetSystemVolumeLevel(STREAM_RING) <
            audioPolicyManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE) {
            audioPolicyManager_.SetDoubleRingVolumeDb(STREAM_RING,
                audioPolicyManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE);
        }
    } else {
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

void AudioCoreService::UpdateOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.front() != nullptr, "streamDesc is nullptr");
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    InternalDeviceType deviceType = streamDesc->newDeviceDescs_.front()->deviceType_;
    AUDIO_DEBUG_LOG("[PipeExecInfo] Update route streamUsage:%{public}d, devicetype:[%{public}s]",
        streamUsage, streamDesc->GetNewDevicesTypeString().c_str());
    // for collaboration, the route should be updated
    UpdateRouteForCollaboration(deviceType);
    shouldUpdateDeviceDueToDualTone_ = false;
    if (Util::IsRingerOrAlarmerStreamUsage(streamUsage) && IsRingerOrAlarmerDualDevicesRange(deviceType) &&
        !VolumeUtils::IsPCVolumeEnable()) {
        UpdateRingerOrAlarmerDualDeviceOutputRouter(streamDesc);
    } else {
        audioVolumeManager_.SetRingerModeMute(true);
        if (isRingDualToneOnPrimarySpeaker_ && streamUsage != STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
            std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
            activeDevices.push_back(make_pair(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG));
            activeDevices.push_back(make_pair(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG));
            audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
            AUDIO_INFO_LOG("Update desc [%{public}d] with speaker on session [%{public}d]",
                deviceType, streamDesc->sessionId_);
            AudioStreamType streamType = streamCollector_.GetStreamType(streamDesc->sessionId_);
            if (!AudioCoreServiceUtils::IsDualStreamWhenRingDual(streamType)) {
                streamsWhenRingDualOnPrimarySpeaker_.push_back(make_pair(streamDesc->sessionId_, streamType));
                AudioVolume::GetInstance()->SetStreamVolumeMute(streamDesc->sessionId_, true);
            }
            shouldUpdateDeviceDueToDualTone_ = true;
        } else {
            audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG,
                streamDesc->newDeviceDescs_.front()->deviceName_, streamDesc->newDeviceDescs_.front()->networkId_);
        }
    }
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
    AudioSpatializationService::GetAudioSpatializationService().UpdateCurrentDevice(deviceDescriptor.macAddress_);
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
        AudioVolume::GetInstance()->SetStreamVolumeMute(stream.first, false);
    }
    streamsWhenRingDualOnPrimarySpeaker_.clear();
    AudioStreamType streamType = streamCollector_.GetStreamType(streamDesc->GetSessionId());
    if (streamType == STREAM_MUSIC) {
        AudioVolume::GetInstance()->SetStreamVolumeMute(streamDesc->GetSessionId(), false);
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

bool AudioCoreService::SelectRingerOrAlarmDevices(std::shared_ptr<AudioStreamDescriptor> streamDesc)
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
            audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG);
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
            audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
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
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        std::string sinkName = localDeviceDescriptor->deviceType_ == DEVICE_TYPE_REMOTE_CAST ?
            "RemoteCastInnerCapturer" : pipeInfo->moduleInfo_.name;
        if (sinkName == BLUETOOTH_SPEAKER) {
            std::string activePort = BLUETOOTH_SPEAKER;
            audioPolicyManager_.SuspendAudioDevice(activePort, false);
        }
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, sinkName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sinkInputIds[i].streamId);
        AUDIO_INFO_LOG("streamId %{public}d, sinkName %{public}s", sinkInputIds[i].streamId, sinkName.c_str());
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, LOCAL_NETWORK_ID, sinkInputIds[i].pid);
    }

    isCurrentRemoteRenderer_ = false;
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
        audioPolicyServerHandler_->SendRendererDeviceChangeEvent(callerPid, sessionId, callbackDesc, reason);
    }

    SleepForSwitchDevice(streamDesc, reason);

    AUDIO_INFO_LOG("Trigger recreate renderer stream %{public}d, pid: %{public}d, routeflag: 0x%{public}x",
        sessionId, callerPid, routeFlag);
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
    if (streamDesc->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        AUDIO_INFO_LOG("[PipeExecInfo] remote cast device do not need open pipe");
        id = streamDesc->sessionId_;
    } else {
        if (pipeInfo->moduleInfo_.name == BLUETOOTH_MIC &&
            streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
            shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_[0];
            audioActiveDevice_.SetActiveBtInDeviceMac(desc->macAddress_);
            bool ret = audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP_IN,
                streamDesc->streamInfo_);
            CHECK_AND_RETURN_RET_LOG(ret, OPEN_PORT_FAILURE, "invalid streamDesc");
            SourceType sourceType = streamDesc->capturerInfo_.sourceType;
            GetA2dpModuleInfo(pipeInfo->moduleInfo_, streamDesc->streamInfo_, sourceType);
        }
        HandleCommonSourceOpened(pipeInfo);
        id = audioPolicyManager_.OpenAudioPort(pipeInfo, paIndex);

        AUDIO_INFO_LOG("routeFlag:%{public}d", pipeInfo->routeFlag_);
        if ((audioActiveDevice_.GetCurrentInputDeviceType() == DEVICE_TYPE_MIC ||
            audioActiveDevice_.GetCurrentInputDeviceType() == DEVICE_TYPE_ACCESSORY) &&
            (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_AI)) {
            audioPolicyManager_.SetDeviceActive(audioActiveDevice_.GetCurrentInputDeviceType(),
                pipeInfo->moduleInfo_.name, true, INPUT_DEVICES_FLAG);
        }
    }
    audioIOHandleMap_.AddIOHandleInfo(pipeInfo->moduleInfo_.name, id);
    HILOG_COMM_INFO("[PipeExecInfo] Get HDI id: %{public}u, paIndex %{public}u", id, paIndex);
    return id;
}

bool AudioCoreService::IsPaRoute(uint32_t routeFlag)
{
    if ((routeFlag & AUDIO_OUTPUT_FLAG_DIRECT) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_FAST) ||
        (routeFlag & AUDIO_INPUT_FLAG_FAST)) {
        return false;
    }
    return true;
}

bool AudioCoreService::RecoverFetchedDescs(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    for (auto &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "Stream desc is nullptr");
        streamDesc->newDeviceDescs_ = streamDesc->oldDeviceDescs_;
    }

    return true;
}

int32_t AudioCoreService::HandleScoOutputDeviceFetched(
    shared_ptr<AudioDeviceDescriptor> &desc, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("In");
    Trace trace("AudioCoreService::HandleScoOutputDeviceFetched");
#ifdef BLUETOOTH_ENABLE
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        RecoverFetchedDescs(pipeManager_->GetAllOutputStreamDescs());
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch output device.");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        FetchOutputDeviceAndRoute("HandleScoOutputDeviceFetched_1", reason);
        return ERROR;
    }
    Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true));
#endif
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t AudioCoreService::HandleScoOutputDeviceFetched(
    shared_ptr<AudioStreamDescriptor> &streamDesc, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("In");
    Trace trace("AudioCoreService::HandleScoOutputDeviceFetched");
#ifdef BLUETOOTH_ENABLE
    std::shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_.front();
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        RecoverFetchedDescs(pipeManager_->GetAllOutputStreamDescs());
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch output device.");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        FetchOutputDeviceAndRoute("HandleScoOutputDeviceFetched_2", reason);
        return ERROR;
    }
    if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
        Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true));
    }
#endif
    AUDIO_INFO_LOG("out");
    return SUCCESS;
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

bool AudioCoreService::GetFastControlParam()
{
    int32_t fastControlFlag = 1; // default 1, set isFastControlled_ true
    GetSysPara("persist.multimedia.audioflag.fastcontrolled", fastControlFlag);
    if (fastControlFlag == 0) {
        isFastControlled_ = false;
    }
    return isFastControlled_;
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

bool AudioCoreService::IsStreamSupportLowpower(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportLowpower");
    if (!streamDesc->rendererInfo_.isOffloadAllowed) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream because renderInfo not support offload.");
        return false;
    }
    if (GetRealUid(streamDesc) == AUDIO_EXT_UID) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "the extra uid not support offload.");
        return false;
    }
    if (streamDesc->streamInfo_.channels > STEREO &&
        (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MOVIE ||
         streamDesc->rendererInfo_.originalFlag != AUDIO_FLAG_PCM_OFFLOAD)) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream because channels.");
        return false;
    }

    if (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MUSIC &&
        streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_AUDIOBOOK &&
        (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MOVIE ||
         streamDesc->rendererInfo_.originalFlag != AUDIO_FLAG_PCM_OFFLOAD)) {
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

int32_t AudioCoreService::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning, bool skipForce)
{
    CHECK_AND_RETURN_RET_LOG(policyConfigMananger_.GetHasEarpiece(), ERR_NOT_SUPPORTED, "the device has no earpiece");
    CHECK_AND_RETURN_RET_LOG(pipeManager_->GetStreamDescById(sessionID) != nullptr, ERR_NOT_SUPPORTED,
        "sessionId is not exist");

    if (!audioSessionService_.IsStreamAllowedToSetDevice(sessionID)) {
        AUDIO_ERR_LOG("current stream is contained in a session which had set default output device");
        return ERR_NOT_SUPPORTED;
    }

    AUDIO_INFO_LOG("[ADeviceEvent] device %{public}d for %{public}s stream %{public}u", deviceType,
        isRunning ? "running" : "not running", sessionID);
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    bool forceFetch = false;
    for (auto &changeInfo : audioRendererChangeInfos) {
        if (changeInfo->sessionId == static_cast<int32_t>(sessionID) &&
            (changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
                changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
                changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION)) {
            CHECK_AND_CONTINUE(!skipForce);
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                std::make_shared<AudioDeviceDescriptor>(), changeInfo->clientUID, "SetDefaultOutputDevice");
            forceFetch = true;
        }
    }
    int32_t ret = audioDeviceManager_.SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    if (ret == NEED_TO_FETCH || forceFetch) {
        FetchOutputDeviceAndRoute("SetDefaultOutputDevice",
            AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
        return SUCCESS;
    }
    return ret;
}

int32_t AudioCoreService::HandleFetchOutputWhenNoRunningStream(const AudioStreamDeviceChangeReasonExt reason)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1, "HandleFetchOutputWhenNoRunningStream");
    CHECK_AND_RETURN_RET_LOG(!descs.empty(), ERROR, "descs is empty");
    AudioDeviceDescriptor tmpOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    if (descs.front()->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(descs.front(), tmpOutputDeviceDesc)) {
        AUDIO_DEBUG_LOG("output device is not change");
        return SUCCESS;
    }
    audioActiveDevice_.SetCurrentOutputDevice(*descs.front());
    AUDIO_DEBUG_LOG("currentActiveDevice %{public}d", audioActiveDevice_.GetCurrentOutputDeviceType());
    audioVolumeManager_.SetVolumeForSwitchDevice(*descs.front());

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_ = descs;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    ActivateOutputDevice(streamDesc, reason);

    if (audioSceneManager_.GetAudioScene(true) != AUDIO_SCENE_DEFAULT) {
        audioActiveDevice_.UpdateActiveDeviceRoute(descs.front()->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG,
            descs.front()->deviceName_, descs.front()->networkId_);
    }
    if (descs.front()->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        string condition = string("address=") + descs.front()->macAddress_ + " role=" + to_string(OUTPUT_DEVICE);
        string deviceInfo = AudioServerProxy::GetInstance().GetAudioParameterProxy(LOCAL_NETWORK_ID, USB_DEVICE,
            condition);
        if (!deviceInfo.empty()) {
            AUDIO_DEBUG_LOG("[GetAudioParameterProxy]deviceInfo: %{public}s", deviceInfo.c_str());
        }
    }
    OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(), reason);
    return SUCCESS;
}

int32_t AudioCoreService::HandleFetchInputWhenNoRunningStream()
{
    std::shared_ptr<AudioDeviceDescriptor> desc;
    AudioDeviceDescriptor tempDesc = audioActiveDevice_.GetCurrentInputDevice();
    if (tempDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && Bluetooth::AudioHfpManager::IsRecognitionStatus()) {
        desc = audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_VOICE_RECOGNITION, -1);
    } else {
        desc = audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_MIC, -1);
    }
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERROR, "desc is nullptr");

    if (desc->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(desc, tempDesc)) {
        AUDIO_DEBUG_LOG("input device is not change");
        return SUCCESS;
    }
    audioActiveDevice_.SetCurrentInputDevice(*desc);
    if (desc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioEcManager_.PresetArmIdleInput(desc->macAddress_);
    }
    DeviceType deviceType = audioActiveDevice_.GetCurrentInputDeviceType();
    AUDIO_DEBUG_LOG("currentActiveInputDevice update %{public}d", deviceType);
    OnPreferredInputDeviceUpdated(deviceType, ""); // networkId is not used
    return SUCCESS;
}

bool AudioCoreService::UpdateOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
    const AudioStreamDeviceChangeReasonExt reason)
{
    std::shared_ptr<AudioDeviceDescriptor> preferredDesc = audioAffinityManager_.GetRendererDevice(uid);
    AudioDeviceDescriptor tmpOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) && !desc->IsSameDeviceInfo(tmpOutputDeviceDesc)
        && desc->deviceType_ != preferredDesc->deviceType_)
        || ((preferredDesc->deviceType_ == DEVICE_TYPE_NONE) && !desc->IsSameDeviceInfo(tmpOutputDeviceDesc))) {
        WriteOutputRouteChangeEvent(desc, reason);
        audioActiveDevice_.SetCurrentOutputDevice(*desc);
        AUDIO_DEBUG_LOG("currentActiveDevice update %{public}d", audioActiveDevice_.GetCurrentOutputDeviceType());
        return true;
    }
    return false;
}

bool AudioCoreService::UpdateInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
    const AudioStreamDeviceChangeReasonExt reason)
{
    std::shared_ptr<AudioDeviceDescriptor> preferredDesc = audioAffinityManager_.GetCapturerDevice(uid);
    if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) &&
        !IsSameDevice(desc, audioActiveDevice_.GetCurrentInputDevice())
        && desc->deviceType_ != preferredDesc->deviceType_)
        || ((preferredDesc->deviceType_ == DEVICE_TYPE_NONE)
        && !IsSameDevice(desc, audioActiveDevice_.GetCurrentInputDevice()))) {
        WriteInputRouteChangeEvent(desc, reason);
        audioActiveDevice_.SetCurrentInputDevice(*desc);
        AUDIO_DEBUG_LOG("currentActiveInputDevice update %{public}d",
            audioActiveDevice_.GetCurrentInputDeviceType());
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
    const AudioStreamDeviceChangeReason reason)
{
    int64_t timeStamp = AudioPolicyUtils::GetInstance().GetCurrentTimeMS();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_ROUTE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("REASON", static_cast<int32_t>(reason));
    bean->Add("TIMESTAMP", static_cast<uint64_t>(timeStamp));
    bean->Add("DEVICE_TYPE_BEFORE_CHANGE", audioActiveDevice_.GetCurrentInputDeviceType());
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

    if (desc->deviceType_ == DEVICE_TYPE_NONE || (IsSameDevice(desc, streamDesc->oldDeviceDescs_.front()) &&
        !NeedRehandleA2DPDevice(desc) && desc->connectState_ != DEACTIVE_CONNECTED &&
        audioSceneManager_.IsSameAudioScene() && !shouldUpdateDeviceDueToDualTone_)) {
        AUDIO_WARNING_LOG("stream %{public}d device not change, no need move device", streamDesc->sessionId_);
        AudioDeviceDescriptor tmpOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
        std::shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetRendererDevice(GetRealUid(streamDesc));
        if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) && !IsSameDevice(desc, tmpOutputDeviceDesc)
            && desc->deviceType_ != preferredDesc->deviceType_)
            || ((preferredDesc->deviceType_ == DEVICE_TYPE_NONE) && !IsSameDevice(desc, tmpOutputDeviceDesc))) {
            audioActiveDevice_.SetCurrentOutputDevice(*desc);
            AudioDeviceDescriptor curOutputDevice = audioActiveDevice_.GetCurrentOutputDevice();
            audioVolumeManager_.SetVolumeForSwitchDevice(curOutputDevice);
            audioActiveDevice_.UpdateActiveDeviceRoute(curOutputDevice.deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG,
                curOutputDevice.deviceName_, curOutputDevice.networkId_);
            OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(), reason);
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
        AudioDeviceDescriptor tempDesc = audioActiveDevice_.GetCurrentInputDevice();
        std::shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetCapturerDevice(GetRealUid(streamDesc));
        if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) && !IsSameDevice(desc, tempDesc) &&
            desc->deviceType_ != preferredDesc->deviceType_) ||
            IsSameDevice(desc, oldDeviceDesc)) {
            audioActiveDevice_.SetCurrentInputDevice(*desc);
            // networkId is not used.
            OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(), "");
            audioActiveDevice_.UpdateActiveDeviceRoute(audioActiveDevice_.GetCurrentInputDeviceType(),
                DeviceFlag::INPUT_DEVICES_FLAG, audioActiveDevice_.GetCurrentInputDevice().deviceName_,
                audioActiveDevice_.GetCurrentInputDevice().networkId_);
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

    if (mode == AUDIO_MODE_PLAYBACK && (rendererState == RENDERER_STOPPED || rendererState == RENDERER_PAUSED ||
        rendererState == RENDERER_RELEASED)) {
        audioDeviceManager_.UpdateDefaultOutputDeviceWhenStopping(streamChangeInfo.audioRendererChangeInfo.sessionId);
        if (rendererState == RENDERER_RELEASED) {
            audioDeviceManager_.RemoveSelectedDefaultOutputDevice(streamChangeInfo.audioRendererChangeInfo.sessionId);
        }
        FetchOutputDeviceAndRoute("UpdateTracker_1");
    }

    const auto &capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    if (mode == AUDIO_MODE_RECORD && capturerState == CAPTURER_RELEASED) {
        AUDIO_INFO_LOG("[ADeviceEvent] fetch device for capturer stream %{public}d released",
            streamChangeInfo.audioCapturerChangeInfo.sessionId);
        audioDeviceManager_.RemoveSelectedInputDevice(streamChangeInfo.audioCapturerChangeInfo.sessionId);
        FetchInputDeviceAndRoute("UpdateTracker");
    }

    const int32_t sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    if (enableDualHalToneState_ && mode == AUDIO_MODE_PLAYBACK && sessionId == enableDualHalToneSessionId_) {
        FetchOutputDeviceAndRoute("UpdateTracker_ForDualHalTone");
        if ((rendererState == RENDERER_STOPPED || rendererState == RENDERER_RELEASED) &&
            Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
            AUDIO_INFO_LOG("disable dual hal tone when ringer/alarm renderer stop/release.");
            UpdateDualToneState(false, enableDualHalToneSessionId_);
        }
    }

    if (isRingDualToneOnPrimarySpeaker_ && AudioCoreServiceUtils::IsOverRunPlayback(mode, rendererState) &&
        Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
        CHECK_AND_RETURN_LOG(!AudioCoreServiceUtils::IsDualOnActive(), "Dual still on active");
        AUDIO_INFO_LOG("[ADeviceEvent] disable primary speaker dual tone when ringer renderer run over");
        isRingDualToneOnPrimarySpeaker_ = false;
        // Add delay between end of double ringtone and device switch.
        // After the ringtone ends, there may still be residual audio data in the pipeline.
        // Switching the device immediately can cause pop noise due the undrained buffers.
        usleep(RING_DUAL_END_DELAY_US);
        FetchOutputDeviceAndRoute("UpdateTracker_2");
        CHECK_AND_RETURN_LOG(!isRingDualToneOnPrimarySpeaker_, "no need to execute SetInnerStreamMute false");
        for (std::pair<uint32_t, AudioStreamType> stream :  streamsWhenRingDualOnPrimarySpeaker_) {
            AudioVolume::GetInstance()->SetStreamVolumeMute(stream.first, false);
        }
        streamsWhenRingDualOnPrimarySpeaker_.clear();
        AudioStreamType streamType = streamCollector_.GetStreamType(streamChangeInfo.audioRendererChangeInfo.sessionId);
        if (streamType == STREAM_MUSIC) {
            AudioVolume::GetInstance()->SetStreamVolumeMute(streamChangeInfo.audioRendererChangeInfo.sessionId, false);
        }
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
        audioEcManager_.PrepareNormalSource(pipeInfo, streamDesc);
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
    AUDIO_INFO_LOG("unload offload module");
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

    AUDIO_INFO_LOG("[StreamExecInfo] Move stream %{public}u, [%{public}d][%{public}s] to [%{public}d][%{public}s]" \
        " reason %{public}d",
        streamDesc->sessionId_, streamDesc->oldDeviceDescs_.front()->deviceType_,
        GetEncryptAddr(streamDesc->oldDeviceDescs_.front()->macAddress_).c_str(), newDeviceDesc->deviceType_,
        GetEncryptAddr(newDeviceDesc->macAddress_).c_str(), static_cast<int32_t>(reason));
}

bool AudioCoreService::HandleMuteBeforeDeviceSwitch(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    for (std::shared_ptr<AudioStreamDescriptor> &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE(streamDesc != nullptr);
        // running stream need to mute when switch device
        if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
#ifndef MUTE_SINK_DISABLE
            MuteSinkPortForSwitchDevice(streamDesc, reason);
#endif
        }
    }

    return true;
}

void AudioCoreService::MuteSinkPortForSwitchDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
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
        } else if (routeFlag == AUDIO_OUTPUT_FLAG_FAST && defaultSinkPortName == PRIMARY_SPEAKER) {
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

    AUDIO_INFO_LOG("mute sink old:[%{public}s] new:[%{public}s]", oldSinkPortName.c_str(), newSinkPortName.c_str());
    MuteSinkPort(oldSinkPortName, newSinkPortName, reason);
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

// After media playback is interrupted by the alarm or ring,
// a delay is required before switching to dual output (e.g., speaker + headset).
// This ensures that the remaining audio buffer is drained,
// preventing any residual media sound from leaking through the speaker.
void AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty(), "Invalid streamDesc");
    bool isRingOrAlarmStream = Util::IsRingerOrAlarmerStreamUsage(streamDesc->rendererInfo_.streamUsage);
    DeviceType deviceType = streamDesc->newDeviceDescs_.front()->deviceType_;
    if (streamDesc->streamStatus_ == STREAM_STATUS_NEW &&
        streamDesc->newDeviceDescs_.size() > 1 && streamCollector_.IsMediaPlaying() &&
        IsRingerOrAlarmerDualDevicesRange(deviceType) && isRingOrAlarmStream) {
        if (AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(
            streamDesc->newDeviceDescs_, streamDesc->sessionId_)) {
            vector<std::int32_t> sessionIdList = streamCollector_.GetPlayingMediaSessionIdList();
            for (const auto &sessionId : sessionIdList) {
                AudioStreamType streamType = streamCollector_.GetStreamType(sessionId);
                streamsWhenRingDualOnPrimarySpeaker_.push_back(make_pair(sessionId, streamType));
                AudioVolume::GetInstance()->SetStreamVolumeMute(sessionId, true);
            }
        }
        usleep(MEDIA_PAUSE_TO_DOUBLE_RING_DELAY_US);
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
    bool isSleepScene = IsSceneRequireMuteAndSleep();

    struct SleepStrategy {
        std::function<bool()> condition;
        std::vector<uint32_t> sleepDurations;
    };

    std::vector<SleepStrategy> strategies = {
        {
            [&]() { return reason.IsOverride() || reason.IsSetDefaultOutputDevice() || reason.IsNewDeviceAvailable(); },
            {BASE_DEVICE_SWITCH_SLEEP_US, BASE_DEVICE_SWITCH_SLEEP_US}
        },
        {
            [&]() { return reason.IsDistributedDeviceUnavailable(); },
            {BASE_DEVICE_SWITCH_SLEEP_US, DISTRIBUTED_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US}
        },
        {
            [&]() { return isOldDeviceUnavailable && isSleepScene && isHeadsetToSpkOrEp; },
            {BASE_DEVICE_SWITCH_SLEEP_US, OLD_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US, HEADSET_TO_SPK_EP_EXTRA_SLEEP_US}
        },
        {
            [&]() { return isOldDeviceUnavailable && isSleepScene; },
            {BASE_DEVICE_SWITCH_SLEEP_US, OLD_DEVICE_UNAVAILABLE_EXTRA_SLEEP_US}
        },
        {
            [&]() { return reason.IsUnknown() && oldSinkName == REMOTE_CAST_INNER_CAPTURER_SINK_NAME; },
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

/**
 * Check whether the current audio scene requires mute and sleep handling.
 * This function is only used in audio switching logic when disconnecting a device,
 * specifically within MuteSinkPortForSwitchDevice and SleepForSwitchDevice.
*/
bool AudioCoreService::IsSceneRequireMuteAndSleep()
{
    AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
    AudioScene scene = audioSceneManager_.GetAudioScene(true);
    return (scene == AUDIO_SCENE_DEFAULT) || (scene == AUDIO_SCENE_PHONE_CHAT) ||
        ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) && ringerMode != RINGER_MODE_NORMAL);
}

void AudioCoreService::SetVoiceCallMuteForSwitchDevice()
{
    Trace trace("AudioCoreService::SetVoiceMuteForSwitchDevice");
    AudioServerProxy::GetInstance().SetVoiceVolumeProxy(0);

    AUDIO_INFO_LOG("%{public}" PRId64" us for modem call update route", WAIT_MODEM_CALL_SET_VOLUME_TIME_US);
    usleep(WAIT_MODEM_CALL_SET_VOLUME_TIME_US);
    // Unmute in SetVolumeForSwitchDevice after update route.
}

void AudioCoreService::MuteSinkPort(const std::string &oldSinkName, const std::string &newSinkName,
    AudioStreamDeviceChangeReasonExt reason)
{
    if (reason.IsOverride() || reason.IsSetDefaultOutputDevice()) {
        int64_t muteTime = SELECT_DEVICE_MUTE_MS;
        if (newSinkName == OFFLOAD_PRIMARY_SPEAKER || oldSinkName == OFFLOAD_PRIMARY_SPEAKER) {
            muteTime = SELECT_OFFLOAD_DEVICE_MUTE_MS;
        }
        MutePrimaryOrOffloadSink(newSinkName, muteTime);
        audioIOHandleMap_.MuteSinkPort(newSinkName, SELECT_DEVICE_MUTE_MS, true, false);
        audioIOHandleMap_.MuteSinkPort(oldSinkName, muteTime, true, false);
    } else if (reason == AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE) {
        int64_t muteTime = NEW_DEVICE_AVALIABLE_MUTE_MS;
        if (newSinkName == OFFLOAD_PRIMARY_SPEAKER || oldSinkName == OFFLOAD_PRIMARY_SPEAKER) {
            muteTime = NEW_DEVICE_AVALIABLE_OFFLOAD_MUTE_MS;
        }
        MutePrimaryOrOffloadSink(oldSinkName, muteTime);
        audioIOHandleMap_.MuteSinkPort(newSinkName, NEW_DEVICE_AVALIABLE_MUTE_MS, true, false);
        audioIOHandleMap_.MuteSinkPort(oldSinkName, muteTime, true, false);
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
    } else if (reason.IsOldDeviceUnavaliable() && ((scene == AUDIO_SCENE_DEFAULT) ||
        ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) &&
        ringermode != RINGER_MODE_NORMAL) || (scene == AUDIO_SCENE_PHONE_CHAT))) {
        MutePrimaryOrOffloadSink(newSinkName, OLD_DEVICE_UNAVALIABLE_MUTE_MS);
        audioIOHandleMap_.MuteSinkPort(newSinkName, OLD_DEVICE_UNAVALIABLE_MUTE_MS, true, false);
    } else if (reason.IsOldDeviceUnavaliableExt() && ((scene == AUDIO_SCENE_DEFAULT) ||
        ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) &&
        ringermode != RINGER_MODE_NORMAL) || (scene == AUDIO_SCENE_PHONE_CHAT))) {
        audioIOHandleMap_.MuteSinkPort(newSinkName, OLD_DEVICE_UNAVALIABLE_EXT_MUTE_MS, true, false);
    } else if (reason == AudioStreamDeviceChangeReason::UNKNOWN &&
        oldSinkName == REMOTE_CAST_INNER_CAPTURER_SINK_NAME) {
        // remote cast -> earpiece 300ms fix sound leak
        audioIOHandleMap_.MuteSinkPort(newSinkName, NEW_DEVICE_REMOTE_CAST_AVALIABLE_MUTE_MS, true, false);
    }
}

int32_t AudioCoreService::ActivateOutputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "Stream desc is nullptr");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();

    std::string encryptMacAddr = GetEncryptAddr(deviceDesc->macAddress_);
    int32_t bluetoothFetchResult = BluetoothDeviceFetchOutputHandle(streamDesc, reason, encryptMacAddr);
    CheckAndWriteDeviceChangeExceptionEvent(bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_DEFAULT, reason,
        deviceDesc->deviceType_, deviceDesc->deviceRole_, bluetoothFetchResult, "bluetooth fetch output device failed");
    CHECK_AND_RETURN_RET(bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_DEFAULT, ERR_OPERATION_FAILED);

    int32_t nearlinkFetchResult = ActivateNearlinkDevice(streamDesc, reason);
    CheckAndWriteDeviceChangeExceptionEvent(nearlinkFetchResult == SUCCESS, reason,
        deviceDesc->deviceType_, deviceDesc->deviceRole_, nearlinkFetchResult, "nearlink fetch output device failed");
    CHECK_AND_RETURN_RET_LOG(nearlinkFetchResult == SUCCESS, REFETCH_DEVICE, "nearlink fetch output device failed");

    if (deviceDesc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioEcManager_.ActivateArmDevice(deviceDesc->macAddress_, deviceDesc->deviceRole_);
    }
    if (deviceDesc->deviceType_ == DEVICE_TYPE_HEARING_AID) {
        SwitchActiveHearingAidDevice(std::make_shared<AudioDeviceDescriptor>(deviceDesc));
    }
    return SUCCESS;
}

int32_t AudioCoreService::ActivateInputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, ERR_INVALID_PARAM, "Invalid stream desc");
    if (streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        BluetoothScoFetch(streamDesc);
    }

    int32_t nearlinkFetchResult = ActivateNearlinkDevice(streamDesc, reason);
    CHECK_AND_RETURN_RET_LOG(nearlinkFetchResult == SUCCESS, REFETCH_DEVICE, "nearlink fetch input device failed");

    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, ERR_INVALID_PARAM, "Invalid stream desc");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    if (deviceDesc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioEcManager_.ActivateArmDevice(deviceDesc->macAddress_, deviceDesc->deviceRole_);
    }

    return SUCCESS;
}

void AudioCoreService::OnAudioSceneChange(const AudioScene& audioScene)
{
    Trace trace("AudioCoreService::OnAudioSceneChange:" + std::to_string(audioScene));
    AUDIO_INFO_LOG("scene change to %{public}d", audioScene);
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audio policy server handler is null");
    audioPolicyServerHandler_->SendAudioSceneChangeEvent(audioScene);
}

bool AudioCoreService::HandleOutputStreamInRunning(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    AudioStreamDeviceChangeReasonExt reason)
{
    if (streamDesc->streamStatus_ != STREAM_STATUS_STARTED) {
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
    AUDIO_INFO_LOG("originalFlag: %{public}d, oldOriginalFlag: %{public}d", streamDesc->rendererInfo_.originalFlag,
        streamDesc->oldOriginalFlag_);
    streamDesc->ResetOriginalFlag();
}

void AudioCoreService::UpdateStreamDevicesForStart(
    std::shared_ptr<AudioStreamDescriptor> &streamDesc, std::string caller)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "Invalid stream desc");
    streamDesc->UpdateOldDevice(streamDesc->newDeviceDescs_);

    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_INVALID;
    streamUsage = audioSessionService_.GetAudioSessionStreamUsage(GetRealPid(streamDesc));
    streamUsage = (streamUsage != StreamUsage::STREAM_USAGE_INVALID) ? streamUsage :
    streamDesc->rendererInfo_.streamUsage;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    if (VolumeUtils::IsPCVolumeEnable() && !isFirstScreenOn_) {
        devices = std::vector<std::shared_ptr<AudioDeviceDescriptor>> {
            AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice()
        };
    } else if (streamDesc->rendererTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        devices = std::vector<std::shared_ptr<AudioDeviceDescriptor>> {
            make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SYSTEM_PRIVATE,
                DeviceRole::OUTPUT_DEVICE)
        };
    } else {
        devices = audioRouterCenter_.FetchOutputDevices(streamUsage, GetRealUid(streamDesc),
            caller, RouterType::ROUTER_TYPE_NONE);
    }
    CHECK_AND_RETURN_LOG(devices.size() > 0 && devices[0] != nullptr, "failed to get devices!");
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(devices[0]->deviceType_);

    streamDesc->UpdateNewDevice(devices);
    if (streamDesc->IsMediaScene() && devices[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
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
    streamDesc->UpdateOldDevice(streamDesc->newDeviceDescs_);
    auto devices = audioRouterCenter_.FetchOutputDevices(streamDesc->GetRenderUsage(),
        GetRealUid(streamDesc), caller, RouterType::ROUTER_TYPE_NONE, streamDesc->GetRenderPrivacyType());
    CHECK_AND_RETURN_LOG(devices.size() > 0 && devices[0] != nullptr, "failed to get devices!");
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(devices[0]->deviceType_);

    streamDesc->UpdateNewDeviceWithoutCheck(devices);
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
#endif
}

bool AudioCoreService::GetDisableFastStreamParam()
{
    int32_t disableFastStream = 1; // default 1, set disableFastStream true
    GetSysPara("persist.multimedia.audioflag.fastcontrolled", disableFastStream);
    return disableFastStream == 0 ? false : true;
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

void AudioCoreService::ResetNearlinkDeviceState(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    bool isRunning)
{
    CHECK_AND_RETURN(isRunning);

    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
    CHECK_AND_RETURN_LOG(deviceDesc->networkId_ == LOCAL_NETWORK_ID, "device network ID is RemoteDevice");

    auto currentOutputDevice = audioActiveDevice_.GetCurrentOutputDevice();
    auto currentInputDevice = audioActiveDevice_.GetCurrentInputDevice();
    if (deviceDesc->deviceRole_ == OUTPUT_DEVICE && currentOutputDevice.deviceType_ == DEVICE_TYPE_NEARLINK) {
        if (!deviceDesc->IsSameDeviceDesc(currentOutputDevice)) {
            AUDIO_INFO_LOG("Reset nearlink output device state, macAddress: %{public}s",
                AudioPolicyUtils::GetInstance().GetEncryptAddr(currentOutputDevice.macAddress_).c_str());
            sleAudioDeviceManager_.ResetSleStreamTypeCount(
                std::make_shared<AudioDeviceDescriptor>(currentOutputDevice));
        }
    }
    if (deviceDesc->deviceRole_ == INPUT_DEVICE && currentInputDevice.deviceType_ == DEVICE_TYPE_NEARLINK_IN) {
        if (!deviceDesc->IsSameDeviceDesc(currentInputDevice)) {
            AUDIO_INFO_LOG("Reset nearlink input device state, macAddress: %{public}s",
                AudioPolicyUtils::GetInstance().GetEncryptAddr(currentInputDevice.macAddress_).c_str());
            sleAudioDeviceManager_.ResetSleStreamTypeCount(
                std::make_shared<AudioDeviceDescriptor>(currentInputDevice));
        }
    }
}

int32_t AudioCoreService::ActivateNearlinkDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_INVALID_PARAM, "Stream desc is nullptr");
    int32_t realUid = GetRealUid(streamDesc);

    auto deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "Device desc is nullptr");

    std::variant<StreamUsage, SourceType> audioStreamConfig;
    bool isRunning = streamDesc->streamStatus_ == STREAM_STATUS_STARTED;
    bool isVoiceType = true;
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        audioStreamConfig = streamDesc->rendererInfo_.streamUsage;
        isVoiceType = AudioPolicyUtils::GetInstance().IsVoiceStreamType(streamDesc->rendererInfo_.streamUsage);
    } else {
        audioStreamConfig = streamDesc->capturerInfo_.sourceType;
        isVoiceType = AudioPolicyUtils::GetInstance().IsVoiceSourceType(streamDesc->capturerInfo_.sourceType);
    }
    if (deviceDesc->deviceType_ == DEVICE_TYPE_NEARLINK || deviceDesc->deviceType_ == DEVICE_TYPE_NEARLINK_IN) {
        auto runDeviceActivationFlow = [this, &deviceDesc, &isRunning, &realUid](auto &&config) -> int32_t {
            int32_t ret = sleAudioDeviceManager_.SetActiveDevice(*deviceDesc, config);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Activating Nearlink device fails");
            CHECK_AND_RETURN_RET_LOG(isRunning, ret, "Stream is not running, no needs start playing");
            return sleAudioDeviceManager_.StartPlaying(*deviceDesc, config, realUid);
        };

        ResetNearlinkDeviceState(deviceDesc, isRunning);

        Bluetooth::AudioHfpManager::SetActiveHfpDevice("");

        int32_t result = std::visit(runDeviceActivationFlow, audioStreamConfig);
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("Nearlink device activation failed, macAddress: %{public}s, result: %{public}d",
                GetEncryptAddr(deviceDesc->macAddress_).c_str(), result);
            HandleNearlinkErrResult(result, deviceDesc, isVoiceType);
            FetchOutputDeviceAndRoute("ActivateNearlinkDevice", reason);
            FetchInputDeviceAndRoute("ActivateNearlinkDevice", reason);
            return REFETCH_DEVICE;
        }
        sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    }
    return SUCCESS;
}

void AudioCoreService::HandleNearlinkErrResult(int32_t result, shared_ptr<AudioDeviceDescriptor> devDesc,
    bool isVoiceType)
{
    CHECK_AND_RETURN(result != SUCCESS);
    if (result == REMOTE_USER_TERMINATED) {
        auto deviceDescriptor = make_shared<AudioDeviceDescriptor>(devDesc);
        AUDIO_INFO_LOG("Set connect state to SUSPEND_CONNECTED");
        deviceDescriptor->connectState_ = SUSPEND_CONNECTED;
        deviceDescriptor->deviceType_ = DEVICE_TYPE_NEARLINK;
        audioDeviceManager_.UpdateDevicesListInfo(deviceDescriptor, CONNECTSTATE_UPDATE);
        deviceDescriptor->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
        audioDeviceManager_.UpdateDevicesListInfo(deviceDescriptor, CONNECTSTATE_UPDATE);
    } else if (result == DUAL_CONNECTION_FAILURE) {
        if (isVoiceType) {
            devDesc->deviceUsage_ = static_cast<DeviceUsage>(static_cast<uint32_t>(devDesc->deviceUsage_) &
                ~static_cast<uint32_t>(DeviceUsage::VOICE));
        } else {
            devDesc->deviceUsage_ = static_cast<DeviceUsage>(static_cast<uint32_t>(devDesc->deviceUsage_) &
                ~static_cast<uint32_t>(DeviceUsage::MEDIA));
        }
        audioDeviceManager_.UpdateDevicesListInfo(devDesc, USAGE_UPDATE);
    } else {
        devDesc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(devDesc, EXCEPTION_FLAG_UPDATE);
    }
}

int32_t AudioCoreService::SwitchActiveHearingAidDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr &&
        audioA2dpDevice_.CheckHearingAidDeviceExist(deviceDescriptor->macAddress_),
        ERR_INVALID_PARAM, "Target HearingAid device doesn't exist.");
    int32_t result = ERROR;
#ifdef BLUETOOTH_ENABLE
    if (audioIOHandleMap_.CheckIOHandleExist(HEARING_AID_SPEAKER)) {
        AUDIO_WARNING_LOG("HearingAid device [%{public}s] [%{public}s] is already active",
            GetEncryptAddr(deviceDescriptor->macAddress_).c_str(), deviceDescriptor->deviceName_.c_str());
        return SUCCESS;
    }

    AudioStreamInfo audioStreamInfo = {};
    DeviceStreamInfo hearingAidStreamInfo = deviceDescriptor->GetDeviceStreamInfo();
    audioStreamInfo.samplingRate = hearingAidStreamInfo.samplingRate.empty() ? AudioSamplingRate::SAMPLE_RATE_16000 :
        *hearingAidStreamInfo.samplingRate.rbegin();
    audioStreamInfo.encoding = hearingAidStreamInfo.encoding;
    audioStreamInfo.format = hearingAidStreamInfo.format;
    audioStreamInfo.channelLayout = hearingAidStreamInfo.channelLayout.empty() ? AudioChannelLayout::CH_LAYOUT_UNKNOWN :
        *hearingAidStreamInfo.channelLayout.rbegin();
    audioStreamInfo.channels = hearingAidStreamInfo.GetChannels().empty() ? AudioChannel::CHANNEL_UNKNOW :
        *hearingAidStreamInfo.GetChannels().rbegin();

    std::string networkId = audioActiveDevice_.GetCurrentOutputDeviceNetworkId();
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_HEARING_AID);
    result = LoadHearingAidModule(DEVICE_TYPE_HEARING_AID, audioStreamInfo, networkId, sinkName, SOURCE_TYPE_INVALID);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "LoadHearingAidModule failed %{public}d", result);
#endif
    return result;
}

int32_t AudioCoreService::LoadHearingAidModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
    std::string networkId, std::string sinkName, SourceType sourceType)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = policyConfigMananger_.GetModuleListByType(ClassType::TYPE_HEARING_AID, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED, "HearingAid module is not exist in the configuration file");

    int32_t loadRet = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH,
        "bt_hearing_aid");
    if (loadRet) {
        AUDIO_ERR_LOG("load adapter failed");
    }
    for (auto &moduleInfo : moduleInfoList) {
        if (moduleInfo.role != "sink") {
            AUDIO_INFO_LOG("Load hearingAid module [%{public}s], role[%{public}s]",
                moduleInfo.name.c_str(), moduleInfo.role.c_str());
            continue;
        }
        DeviceRole configRole = OUTPUT_DEVICE;
        DeviceRole deviceRole = deviceType == DEVICE_TYPE_HEARING_AID ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load hearingAid module [%{public}s], role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            AUDIO_INFO_LOG("hearingAid device connects for the first time");
            // HearingAid device connects for the first time
            GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
            uint32_t paIndex = 0;
            AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
            CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID,
                ERR_INVALID_HANDLE, "OpenAudioPort failed ioHandle[%{public}u]", ioHandle);
            CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE,
                ERR_OPERATION_FAILED, "OpenAudioPort failed paId[%{public}u]", paIndex);
            audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);

            std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
            pipeInfo->id_ = ioHandle;
            pipeInfo->paIndex_ = paIndex;
            pipeInfo->name_ = "hearing_aid_output";
            pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
            pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
            pipeInfo->adapterName_ = "hearing_aid";
            pipeInfo->moduleInfo_ = moduleInfo;
            pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
            pipeInfo->InitAudioStreamInfo();
            pipeManager_->AddAudioPipeInfo(pipeInfo);
            AUDIO_INFO_LOG("Add PipeInfo %{public}u in load hearingAid.", pipeInfo->id_);
        }
    }

    return SUCCESS;
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
        existingStartDuration.push_back(GetTimeCostFrom(desc->startTimeStamp_));
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
        AUDIO_INFO_LOG("AppName:          %{public}s", result->existingAppName[i].c_str());
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

int32_t AudioCoreService::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    return audioCapturerSession_.SetWakeUpAudioCapturerFromAudioServer(config);
}

void AudioCoreService::UpdateRouteForCollaboration(InternalDeviceType deviceType)
{
    if (AudioCollaborativeService::GetAudioCollaborativeService().GetRealCollaborativeState()) {
        std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
        activeDevices.push_back(make_pair(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG));
        activeDevices.push_back(make_pair(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG));
        audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
        AUDIO_INFO_LOG("collaboration Update desc [%{public}d] with speaker", deviceType);
    }
}

int32_t AudioCoreService::SetSleVoiceStatusFlag(AudioScene audioScene)
{
    if (audioScene == AUDIO_SCENE_DEFAULT) {
        audioPolicyManager_.SetSleVoiceStatusFlag(false);
    } else {
        audioPolicyManager_.SetSleVoiceStatusFlag(true);
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

void AudioCoreService::FetchOutputDevicesForDescs(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputDescs)
{
    for (auto &desc : outputDescs) {
    CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
    desc->newDeviceDescs_ = audioRouterCenter_.FetchOutputDevices(desc->rendererInfo_.streamUsage,
        GetRealUid(desc), "StartClient", RouterType::ROUTER_TYPE_NONE,
        streamDesc->rendererInfo_.privacyType);
    }
}
} // namespace AudioStandard
} // namespace OHOS
