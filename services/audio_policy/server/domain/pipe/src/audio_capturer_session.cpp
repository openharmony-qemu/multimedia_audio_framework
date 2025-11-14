/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioCapturerSession"
#endif

#include "audio_capturer_session.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"

#include "audio_policy_utils.h"
#include "audio_core_service.h"
#include "audio_zone_service.h"
namespace {
    #include "v5_0/iaudio_manager.h"
}

namespace OHOS {
namespace AudioStandard {
namespace {
const uint32_t PCM_8_BIT = 8;
const uint32_t SESSION_ID_INVALID = 0;
const float RENDER_FRAME_INTERVAL_IN_SECONDS = 0.02;
const std::string PIPE_PRIMARY_INPUT = "primary_input";
const std::string PIPE_WAKEUP_INPUT = "wakeup_input";
const std::string PIPE_PRIMARY_INPUT_AI = "primary_input_AI";

inline const std::unordered_set<SourceType> specialSourceTypeSet_ = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_REMOTE_CAST
};

const std::map<SourceType, AudioInputType> FWKTYPE_TO_HDITYPE_MAP = {
    { SOURCE_TYPE_INVALID, AUDIO_INPUT_DEFAULT_TYPE},
    { SOURCE_TYPE_MIC, AUDIO_INPUT_MIC_TYPE},
    { SOURCE_TYPE_PLAYBACK_CAPTURE, AUDIO_INPUT_MIC_TYPE},
    { SOURCE_TYPE_ULTRASONIC, AUDIO_INPUT_MIC_TYPE},
    { SOURCE_TYPE_WAKEUP, AUDIO_INPUT_SPEECH_WAKEUP_TYPE},
    { SOURCE_TYPE_VOICE_TRANSCRIPTION, AUDIO_INPUT_VOICE_COMMUNICATION_TYPE},
    { SOURCE_TYPE_VOICE_COMMUNICATION, AUDIO_INPUT_VOICE_COMMUNICATION_TYPE},
    { SOURCE_TYPE_VOICE_RECOGNITION, AUDIO_INPUT_VOICE_RECOGNITION_TYPE},
    { SOURCE_TYPE_VOICE_CALL, AUDIO_INPUT_VOICE_CALL_TYPE},
    { SOURCE_TYPE_CAMCORDER, AUDIO_INPUT_CAMCORDER_TYPE},
    { SOURCE_TYPE_EC, AUDIO_INPUT_EC_TYPE},
    { SOURCE_TYPE_MIC_REF, AUDIO_INPUT_NOISE_REDUCTION_TYPE},
    { SOURCE_TYPE_UNPROCESSED, AUDIO_INPUT_RAW_TYPE},
    { SOURCE_TYPE_LIVE, AUDIO_INPUT_LIVE_TYPE},
};

uint32_t ConvertToHDIAudioInputType(SourceType sourceType)
{
    if (FWKTYPE_TO_HDITYPE_MAP.find(sourceType) != FWKTYPE_TO_HDITYPE_MAP.end()) {
        auto iter = FWKTYPE_TO_HDITYPE_MAP.find(sourceType);
        return static_cast<uint32_t>(iter->second);
    }

    return static_cast<uint32_t>(AUDIO_INPUT_MIC_TYPE);
}

const std::map<SourceType, int> NORMAL_SOURCETYPE_PRIORITY = {
    // from high to low
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

bool IsHigherPrioritySourceType(SourceType newSource, SourceType currentSource)
{
    AUDIO_INFO_LOG("newSource sourceType:%{public}d currentSource sourceType:%{public}d", newSource, currentSource);

    if (!AudioEcManager::GetInstance().GetEcFeatureEnable() &&
        (ConvertToHDIAudioInputType(newSource) == ConvertToHDIAudioInputType(currentSource))) {
            return false;
        }

    auto newIter = NORMAL_SOURCETYPE_PRIORITY.find(newSource);
    auto currIter = NORMAL_SOURCETYPE_PRIORITY.find(currentSource);
    if (newIter == NORMAL_SOURCETYPE_PRIORITY.end() || currIter == NORMAL_SOURCETYPE_PRIORITY.end() ||
        (newSource == currentSource)) {
        return false;
    }
    return newIter->second >= currIter->second;
}
}  // namespace

void AudioCapturerSession::Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager)
{
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
}

void AudioCapturerSession::DeInit()
{
    audioA2dpOffloadManager_ = nullptr;
}

void AudioCapturerSession::SetConfigParserFlag()
{
    isPolicyConfigParsered_ = true;
}

void AudioCapturerSession::LoadInnerCapturerSink(std::string moduleName, AudioStreamInfo streamInfo)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("Start");
    uint32_t bufferSize = streamInfo.samplingRate *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(streamInfo.format) *
        streamInfo.channels * RENDER_FRAME_INTERVAL_IN_SECONDS;

    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    moduleInfo.format = AudioPolicyUtils::GetInstance().ConvertToHDIAudioFormat(streamInfo.format);
    moduleInfo.name = moduleName;
    moduleInfo.networkId = "LocalDevice";
    moduleInfo.channels = std::to_string(streamInfo.channels);
    moduleInfo.rate = std::to_string(streamInfo.samplingRate);
    moduleInfo.bufferSize = std::to_string(bufferSize);

    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
#endif
}

void AudioCapturerSession::UnloadInnerCapturerSink(std::string moduleName)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleName);
#endif
}

void AudioCapturerSession::HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("Is connected: %{public}d", isConnected);
    AudioDeviceDescriptor updatedDesc = AudioDeviceDescriptor(DEVICE_TYPE_REMOTE_CAST,
        AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_REMOTE_CAST));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    if (isConnected) {
        // If device already in list, remove it else do not modify the list
        audioConnectedDevice_.DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_,
            updatedDesc.macAddress_);
        audioDeviceCommon_.UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);
        LoadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME, streamInfo);
    } else {
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("HandleRemoteCastDevice_1",
            AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT);
        UnloadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME);
    }
    // remove device from golbal when device has been added to audio zone in superlanch-dual
    int32_t res = AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(
        audioConnectedDevice_.GetConnectedDeviceByType(LOCAL_NETWORK_ID, DEVICE_TYPE_REMOTE_CAST));
    if (res == SUCCESS) {
        AUDIO_INFO_LOG("Enable remotecast device for audio zone, remove from global list");
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    }
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("HandleRemoteCastDevice_2");
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("HandleRemoteCastDevice_2");

    // update a2dp offload
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
#endif
}

bool AudioCapturerSession::IsInvalidPipeRole(const std::shared_ptr<AudioPipeInfo> &pipe)
{
    return pipe->pipeRole_ != PIPE_ROLE_INPUT;
}

bool AudioCapturerSession::CompareIndependentxmlPriority(const std::shared_ptr<AudioPipeInfo> &pipe,
    uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession)
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    if (sourceStrategyMap == nullptr) {
        return false;
    }

    uint32_t maxPriority = 0;
    for (const auto &stream : pipe->streamDescriptors_) {
        if (stream == nullptr || stream->sessionId_ == sessionId || stream->streamStatus_ != STREAM_STATUS_STARTED) {
            continue;
        }

        auto strategyIt = sourceStrategyMap->find(stream->capturerInfo_.sourceType);
        if (strategyIt == sourceStrategyMap->end()) {
            continue;
        }

        if (strategyIt->second.priority > maxPriority) {
            maxPriority = strategyIt->second.priority;
            stream->CopyToStruct(runningSessionInfo);
            hasSession = true;
        }
    }
    AUDIO_INFO_LOG("Independent find ret: %{public}d, session: %{public}d, sourceType: %{public}d",
        static_cast<int32_t>(hasSession), runningSessionInfo.sessionId_, runningSessionInfo.capturerInfo_.sourceType);
    return hasSession;
}

bool AudioCapturerSession::HandleIndependentInputpipe(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
    uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession)
{
    for (const auto &pipe : pipeList) {
        if (pipe && pipe->pipeRole_ == PIPE_ROLE_INPUT && pipe->routeFlag_ == AUDIO_INPUT_FLAG_AI) {
            AUDIO_INFO_LOG("In Independent pipe");
            return CompareIndependentxmlPriority(pipe, sessionId, runningSessionInfo, hasSession);
        }
    }
    return hasSession;
}

bool AudioCapturerSession::HandleNormalInputPipes(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
    uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession)
{
    AUDIO_INFO_LOG("normal input");
    for (const auto &pipe : pipeList) {
        if (pipe == nullptr || pipe->pipeRole_ != PIPE_ROLE_INPUT || IsIndependentPipe(pipe)) {
            continue;
        }

        uint32_t flagMask = AUDIO_INPUT_FLAG_AI | AUDIO_INPUT_FLAG_FAST;
        if ((pipe->routeFlag_ & flagMask) != 0) {
            continue;
        }

        for (const auto &stream : pipe->streamDescriptors_) {
            if (stream == nullptr || stream->sessionId_ == sessionId || IsStreamValid(stream) == false) {
                continue;
            }

            SourceType tmpSource = sessionWithNormalSourceType_[stream->sessionId_].sourceType;
            if (IsHigherPrioritySourceType(tmpSource, runningSessionInfo.capturerInfo_.sourceType)) {
                hasSession = true;
                stream->CopyToStruct(runningSessionInfo);
            }
        }
    }
    AUDIO_INFO_LOG("find ret: %{public}d, session: %{public}d, sourceType: %{public}d",
        static_cast<int32_t>(hasSession), runningSessionInfo.sessionId_, runningSessionInfo.capturerInfo_.sourceType);
    return hasSession;
}

bool AudioCapturerSession::IsIndependentPipe(const std::shared_ptr<AudioPipeInfo> &pipe)
{
    CHECK_AND_RETURN_RET_LOG(pipe != nullptr, false, "pipe is nullptr");
    return pipe->adapterName_ == ADAPTER_TYPE_VA;
}

bool AudioCapturerSession::IsStreamValid(const std::shared_ptr<AudioStreamDescriptor> &stream)
{
    return sessionWithNormalSourceType_.find(stream->sessionId_) != sessionWithNormalSourceType_.end() &&
           stream->streamStatus_ == STREAM_STATUS_STARTED &&
           specialSourceTypeSet_.count(sessionWithNormalSourceType_[stream->sessionId_].sourceType) == 0;
}

bool AudioCapturerSession::FindRunningNormalSession(uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo)
{
    bool hasSession = false;
    SourceType tmpSource = SOURCE_TYPE_INVALID;

    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = AudioPipeManager::GetPipeManager()->GetPipeList();
    std::shared_ptr<AudioPipeInfo> incommingPipe =
        AudioPipeManager::GetPipeManager()->FindPipeBySessionId(pipeList, sessionId);
    if (incommingPipe == nullptr) {
        return false;
    }

    AUDIO_INFO_LOG("incommingPipe: %{public}s", incommingPipe->name_.c_str());
    if (IsInvalidPipeRole(incommingPipe)) {
        return false;
    }

    if (incommingPipe->routeFlag_ == AUDIO_INPUT_FLAG_AI) {
        return HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    }

    return HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
}

bool AudioCapturerSession::FindRemainingNormalSession(uint32_t sessionId, bool findRunningSessionRet,
    uint32_t runningSessionId, uint32_t &targetSessionId)
{
    bool hasRemainSession = false;
    targetSessionId = runningSessionId;
    CHECK_AND_RETURN_RET(!findRunningSessionRet, true);

    SessionInfo targetSession;
    targetSession.sourceType = SOURCE_TYPE_INVALID;
    for (auto it : sessionWithNormalSourceType_) {
        CHECK_AND_CONTINUE(it.first != sessionId);
        hasRemainSession = true;
        SourceType higherSourceType = it.second.sourceType;
        CHECK_AND_CONTINUE(IsHigherPrioritySourceType(higherSourceType, targetSession.sourceType));
        targetSession = it.second;
        targetSessionId = it.first;
    }
    return hasRemainSession;
}

int32_t AudioCapturerSession::SetHearingAidReloadFlag(const bool hearingAidReloadFlag)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    hearingAidReloadFlag_ = hearingAidReloadFlag;
    return SUCCESS;
}

int32_t AudioCapturerSession::ReloadCaptureSoftLink(std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const AudioModuleInfo &moduleInfo)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    int32_t ret = audioEcManager_.ReloadSourceSoftLink(pipeInfo, moduleInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "reload softLink failed");
    hearingAidReloadFlag_ = true;
    return SUCCESS;
}

int32_t AudioCapturerSession::ReloadCaptureSessionSoftLink()
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    hearingAidReloadFlag_ = false;
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipes = AudioPipeManager::GetPipeManager()->GetPipeList();
    CHECK_AND_RETURN_RET_LOG(!pipes.empty(), ERR_INVALID_OPERATION, "pipes invalid");
    AudioStreamDescriptor targetStream;
    bool hasSession = false;
    hasSession = HandleNormalInputPipes(pipes, SESSION_ID_INVALID, targetStream, hasSession);

    CHECK_AND_RETURN_RET_LOG(hasSession, SUCCESS, "no need to reload session");
    AUDIO_INFO_LOG("start reload session: %{public}u", targetStream.sessionId_);

    int32_t result = audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[targetStream.sessionId_],
        targetStream.sessionId_);
    if (result == SUCCESS) {
        audioEcManager_.SetOpenedNormalSourceSessionId(targetStream.sessionId_);
    }
    return result;
}

bool AudioCapturerSession::IsValidSessionIdForReload(uint32_t sessionId)
{
    return sessionWithNormalSourceType_.count(sessionId) > 0 &&
        specialSourceTypeSet_.count(sessionWithNormalSourceType_[sessionId].sourceType) == 0;
}

int32_t AudioCapturerSession::ReloadCaptureSession(uint32_t sessionId, SessionOperation operation)
{
    AUDIO_INFO_LOG("prepare reload session: %{public}u with operation: %{public}d", sessionId, operation);
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    CHECK_AND_RETURN_RET_LOG(!hearingAidReloadFlag_, SUCCESS, "no need to reload session for hearingAid");
    uint32_t targetSessionId = sessionId;
    AudioStreamDescriptor runningSessionInfo = {};
    bool needReload = false;
    CHECK_AND_RETURN_RET_LOG(IsValidSessionIdForReload(sessionId), ERROR, "sessionId error!");
    SessionInfo targetSession = sessionWithNormalSourceType_[sessionId];
    bool findRunningSessionRet = FindRunningNormalSession(targetSessionId, runningSessionInfo);
    CHECK_AND_RETURN_RET_LOG(!(runningSessionInfo.capturerInfo_.sourceType == SOURCE_TYPE_MIC &&
        IsVirtualAudioRecognitionSession(sessionId)), ERROR, "skipping reload: va recognition stream detected");
    switch (operation) {
        case SESSION_OPERATION_START:
            if (findRunningSessionRet &&
                IsHigherPrioritySourceType(targetSession.sourceType, runningSessionInfo.capturerInfo_.sourceType)) {
                needReload = true;
            } else if (!findRunningSessionRet && (audioEcManager_.GetSourceOpened() != targetSession.sourceType)) {
                needReload = true;
            }
            break;
        case SESSION_OPERATION_PAUSE:
        case SESSION_OPERATION_STOP:
            if (findRunningSessionRet && (targetSession.sourceType == audioEcManager_.GetSourceOpened())) {
                needReload = true;
                targetSessionId = runningSessionInfo.sessionId_;
                targetSession = sessionWithNormalSourceType_[targetSessionId];
            }
            break;
        case SESSION_OPERATION_RELEASE:
            CHECK_AND_BREAK_LOG((targetSession.sourceType == audioEcManager_.GetSourceOpened()) &&
                FindRemainingNormalSession(sessionId, findRunningSessionRet,
                runningSessionInfo.sessionId_, targetSessionId), "no remain stream.");
            needReload = true;
            targetSession = sessionWithNormalSourceType_[targetSessionId];
            break;
        default:
            AUDIO_ERR_LOG("operation parameter error!");
            break;
    }

    CHECK_AND_RETURN_RET_LOG(needReload, ERROR, "no need to reload session");
    AUDIO_INFO_LOG("start reload session: %{public}u", targetSessionId);
    int32_t result = audioEcManager_.ReloadSourceForSession(targetSession, targetSessionId);
    if (result == SUCCESS) {
        audioEcManager_.SetOpenedNormalSourceSessionId(targetSessionId);
    }
    return result;
}

bool AudioCapturerSession::IsVirtualAudioRecognitionSession(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(sessionWithNormalSourceType_.count(sessionId) != 0,
        false, "session %{public}d not found", sessionId);

    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = AudioPipeManager::GetPipeManager()->GetPipeList();
    std::shared_ptr<AudioPipeInfo> pipe = AudioPipeManager::GetPipeManager()->FindPipeBySessionId(pipeList, sessionId);
    CHECK_AND_RETURN_RET_LOG(pipe != nullptr, false, "pipe is nullptr");

    SessionInfo sessionInfo = sessionWithNormalSourceType_[sessionId];

    bool isVARecognitionSession =
        pipe->adapterName_ == ADAPTER_TYPE_VA && sessionInfo.sourceType == SOURCE_TYPE_VOICE_RECOGNITION;
    AUDIO_INFO_LOG("sessionID %{public}d isVARecognitionSession: %{public}s",
        sessionId, isVARecognitionSession ? "true" : "false");
    return isVARecognitionSession;
}

int32_t AudioCapturerSession::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    AUDIO_INFO_LOG("sessionID: %{public}" PRIu64 " source: %{public}d", sessionID, sessionInfo.sourceType);
    CHECK_AND_RETURN_RET_LOG(isPolicyConfigParsered_ && audioVolumeManager_.GetLoadFlag(), ERROR,
        "policyConfig not loaded");

    if (sessionIdisRemovedSet_.count(sessionID) > 0) {
        sessionIdisRemovedSet_.erase(sessionID);
        AUDIO_INFO_LOG("sessionID: %{public}" PRIu64 " had already been removed earlier", sessionID);
        return SUCCESS;
    }
    if (specialSourceTypeSet_.count(sessionInfo.sourceType) == 0) {
        if (audioEcManager_.GetSourceOpened() == SOURCE_TYPE_INVALID) {
            // normal source is not opened before -- it should not be happen!!
            AUDIO_WARNING_LOG("Record route should not be opened here!");
            return SUCCESS;
        }
        sessionWithNormalSourceType_[sessionID] = sessionInfo;
    } else if (sessionInfo.sourceType == SOURCE_TYPE_REMOTE_CAST) {
        HandleRemoteCastDevice(true, streamInfo);
        sessionWithSpecialSourceType_[sessionID] = sessionInfo;
    } else {
        sessionWithSpecialSourceType_[sessionID] = sessionInfo;
    }
    return SUCCESS;
}

bool AudioCapturerSession::IsRemainingSourceIndependent()
{
    auto sourceStrategyMapget = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET(sourceStrategyMapget != nullptr, false);
    for (auto it = sessionWithNormalSourceType_.begin(); it !=sessionWithNormalSourceType_.end(); it++) {
        SourceType sourceType = it->second.sourceType;
        auto smapIt = sourceStrategyMapget->find(sourceType);
        CHECK_AND_RETURN_RET_LOG(smapIt != sourceStrategyMapget->end(), false,
            "not find sourceType:%{public}d", sourceType);
        CHECK_AND_RETURN_RET_LOG(smapIt->second.pipeName == PIPE_PRIMARY_INPUT_AI, false,
            "invalid pipeName:%{public}s", smapIt->second.pipeName.c_str());
    }
    return true;
}

void AudioCapturerSession::OnCapturerSessionRemoved(uint64_t sessionID)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    AUDIO_INFO_LOG("sessionid:%{public}" PRIu64, sessionID);
    if (sessionWithSpecialSourceType_.count(sessionID) > 0) {
        if (sessionWithSpecialSourceType_[sessionID].sourceType == SOURCE_TYPE_REMOTE_CAST) {
            HandleRemoteCastDevice(false);
        }
        sessionWithSpecialSourceType_.erase(sessionID);
        return;
    }

    if (sessionWithNormalSourceType_.count(sessionID) > 0) {
        if (sessionWithNormalSourceType_[sessionID].sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            audioEcManager_.ResetAudioEcInfo();
        }
        sessionWithNormalSourceType_.erase(sessionID);
        if (!sessionWithNormalSourceType_.empty() && !IsRemainingSourceIndependent()) {
            return;
        }
        // close source when all capturer sessions removed
        audioEcManager_.CloseNormalSource();
        return;
    }

    AUDIO_INFO_LOG("Sessionid:%{public}" PRIu64 " not added, directly placed into sessionIdisRemovedSet_", sessionID);
    sessionIdisRemovedSet_.insert(sessionID);
}

bool AudioCapturerSession::ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
    AudioModuleInfo &audioModuleInfo)
{
    if (!audioConfigManager_.GetAdapterInfoFlag()) {
        return false;
    }

    std::shared_ptr<PolicyAdapterInfo> info;
    AudioAdapterType type = static_cast<AudioAdapterType>(AudioPolicyUtils::portStrToEnum[std::string(PRIMARY_WAKEUP)]);
    bool ret = audioConfigManager_.GetAdapterInfoByType(type, info);
    if (!ret) {
        AUDIO_ERR_LOG("can not find adapter info");
        return false;
    }

    std::shared_ptr<AdapterPipeInfo> pipeInfo = info->GetPipeInfoByName(PIPE_WAKEUP_INPUT);
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("wakeup pipe info is nullptr");
        return false;
    }

    if (!FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo)) {
        AUDIO_ERR_LOG("failed to fill pipe stream prop info");
        return false;
    }

    audioModuleInfo.adapterName = info->adapterName;
    audioModuleInfo.name = pipeInfo->paProp_.moduleName_;
    audioModuleInfo.lib = pipeInfo->paProp_.lib_;
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.className = "primary";
    audioModuleInfo.fileName = "";
    audioModuleInfo.OpenMicSpeaker = "1";
    audioModuleInfo.sourceType = std::to_string(SourceType::SOURCE_TYPE_WAKEUP);

    AUDIO_INFO_LOG("wakeup auido module info, adapter name:%{public}s, name:%{public}s, lib:%{public}s",
        audioModuleInfo.adapterName.c_str(), audioModuleInfo.name.c_str(), audioModuleInfo.lib.c_str());
    return true;
}

int32_t AudioCapturerSession::SetWakeUpAudioCapturer(InternalAudioCapturerOptions options)
{
    AUDIO_INFO_LOG("set wakeup audio capturer start");
    AudioModuleInfo moduleInfo = {};
    if (!ConstructWakeupAudioModuleInfo(options.streamInfo, moduleInfo)) {
        AUDIO_ERR_LOG("failed to construct wakeup audio module info");
        return ERROR;
    }
    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);

    AUDIO_DEBUG_LOG("set wakeup audio capturer end");
    return SUCCESS;
}

int32_t AudioCapturerSession::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    InternalAudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo = config.streamInfo;
    return SetWakeUpAudioCapturer(capturerOptions);
}

int32_t AudioCapturerSession::CloseWakeUpAudioCapturer()
{
    AUDIO_INFO_LOG("close wakeup audio capturer start");
    return audioIOHandleMap_.ClosePortAndEraseIOHandle(std::string(PRIMARY_WAKEUP));
}

// private method
bool AudioCapturerSession::FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo,
    std::shared_ptr<AdapterPipeInfo> pipeInfo, AudioModuleInfo &audioModuleInfo)
{
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("wakeup pipe info is nullptr");
        return false;
    }

    if (pipeInfo->streamPropInfos_.size() == 0) {
        AUDIO_ERR_LOG("no stream prop info");
        return false;
    }

    auto targetIt = *pipeInfo->streamPropInfos_.begin();
    for (auto it : pipeInfo->streamPropInfos_) {
        if (it -> channels_ == static_cast<uint32_t>(streamInfo.channels)) {
            targetIt = it;
            break;
        }
    }

    audioModuleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetIt->format_];
    audioModuleInfo.channels = std::to_string(targetIt->channels_);
    audioModuleInfo.rate = std::to_string(targetIt->sampleRate_);
    audioModuleInfo.bufferSize =  std::to_string(targetIt->bufferSize_);

    AUDIO_INFO_LOG("stream prop info, format:%{public}s, channels:%{public}s, rate:%{public}s, buffer size:%{public}s",
        audioModuleInfo.format.c_str(), audioModuleInfo.channels.c_str(),
        audioModuleInfo.rate.c_str(), audioModuleInfo.bufferSize.c_str());
    return true;
}

bool AudioCapturerSession::IsVoipDeviceChanged(const AudioDeviceDescriptor &inputDevice,
    const AudioDeviceDescriptor &outputDevice)
{
    AudioDeviceDescriptor realInputDevice = inputDevice;
    AudioDeviceDescriptor realOutputDevice = outputDevice;
    shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_VOICE_COMMUNICATION, -1);
    if (inputDesc != nullptr) {
        realInputDevice = *inputDesc;
    }
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDesc =
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION, -1, "IsVoipDeviceChanged");
    if (outputDesc.size() > 0 && outputDesc.front() != nullptr) {
        realOutputDevice = *outputDesc.front();
    }
    if (!inputDevice.IsSameDeviceDesc(realInputDevice) || !outputDevice.IsSameDeviceDesc(realOutputDevice)) {
        AUDIO_INFO_LOG("target device is not ready, so ignore reload");
        return false;
    }
    AudioEcInfo lastEcInfo = audioEcManager_.GetAudioEcInfo();
    AUDIO_INFO_LOG("curInDevice: %{public}d, curOutDevice: %{public}d", lastEcInfo.inputDevice.deviceType_,
        lastEcInfo.outputDevice.deviceType_);
    if (!lastEcInfo.inputDevice.IsSameDeviceDesc(realInputDevice) ||
        !lastEcInfo.outputDevice.IsSameDeviceDesc(realOutputDevice)) {
        return true;
    }
    return false;
}

void AudioCapturerSession::ReloadSourceForDeviceChange(const AudioDeviceDescriptor &inputDevice,
    const AudioDeviceDescriptor &outputDevice, const std::string &caller)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    AUDIO_DEBUG_LOG("form caller: %{public}s, inDevice: %{public}d, outDevice: %{public}d", caller.c_str(),
        inputDevice.deviceType_, outputDevice.deviceType_);
    if (!audioEcManager_.GetEcFeatureEnable()) {
        AUDIO_DEBUG_LOG("reload ignore for feature not enable");
        return;
    }
    SourceType normalSourceOpened = audioEcManager_.GetSourceOpened();
    if (normalSourceOpened != SOURCE_TYPE_VOICE_COMMUNICATION && normalSourceOpened != SOURCE_TYPE_MIC) {
        AUDIO_INFO_LOG("reload ignore for source not voip or mic");
        return;
    }

    if (normalSourceOpened == SOURCE_TYPE_VOICE_COMMUNICATION) {
        if (!IsVoipDeviceChanged(inputDevice, outputDevice)) {
            AUDIO_INFO_LOG("voip reload ignore for device not change");
            return;
        }
    } else {
        if (inputDevice.deviceType_ != DEVICE_TYPE_DEFAULT &&
            GetInputDeviceTypeForReload().deviceType_ == DEVICE_TYPE_DEFAULT) {
            SetInputDeviceTypeForReload(inputDevice);
            AUDIO_INFO_LOG("mic source reload ignore for inputDeviceForReload_ not update");
            return;
        }
        if (inputDevice.deviceType_ == DEVICE_TYPE_DEFAULT ||
            inputDevice.IsSameDeviceDesc(GetInputDeviceTypeForReload())) {
            AUDIO_INFO_LOG("mic source reload ignore for device not changed");
            return;
        }
    }

    // reload for device change, used session is not changed
    uint64_t sessionId = audioEcManager_.GetOpenedNormalSourceSessionId();
    if (sessionWithNormalSourceType_.find(sessionId) == sessionWithNormalSourceType_.end()) {
        AUDIO_ERR_LOG("target session: %{public}" PRIu64 " not found", sessionId);
        return;
    }
    SetInputDeviceTypeForReload(inputDevice);
    AUDIO_INFO_LOG("start reload session: %{public}" PRIu64 " for device change", sessionId);
    audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[sessionId]);
}

void AudioCapturerSession::SetInputDeviceTypeForReload(const AudioDeviceDescriptor &inputDevice)
{
    std::lock_guard<std::mutex> lock(inputDeviceReloadMutex_);
    inputDeviceForReload_ = inputDevice;
}

const AudioDeviceDescriptor& AudioCapturerSession::GetInputDeviceTypeForReload()
{
    std::lock_guard<std::mutex> lock(inputDeviceReloadMutex_);
    return inputDeviceForReload_;
}

std::string AudioCapturerSession::GetEnhancePropByNameV3(const AudioEffectPropertyArrayV3 &propertyArray,
    const std::string &propName)
{
    std::string propValue = "";
    auto iter = std::find_if(propertyArray.property.begin(), propertyArray.property.end(),
        [&propName](const AudioEffectPropertyV3 &prop) {
            return prop.name == propName;
        });
    if (iter != propertyArray.property.end()) {
        propValue = iter->category;
    }
    return propValue;
}

void AudioCapturerSession::ReloadSourceForEffect(const AudioEffectPropertyArrayV3 &oldPropertyArray,
    const AudioEffectPropertyArrayV3 &newPropertyArray)
{
    if (!audioEcManager_.GetMicRefFeatureEnable()) {
        AUDIO_INFO_LOG("reload ignore for feature not enable");
        return;
    }
    if (audioEcManager_.GetSourceOpened() != SOURCE_TYPE_VOICE_COMMUNICATION &&
        audioEcManager_.GetSourceOpened() != SOURCE_TYPE_MIC) {
        AUDIO_INFO_LOG("reload ignore for source not voip or record");
        return;
    }
    std::string oldRecordProp = GetEnhancePropByNameV3(oldPropertyArray, "record");
    std::string oldVoipUpProp = GetEnhancePropByNameV3(oldPropertyArray, "voip_up");
    std::string newRecordProp = GetEnhancePropByNameV3(newPropertyArray, "record");
    std::string newVoipUpProp = GetEnhancePropByNameV3(newPropertyArray, "voip_up");
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    if ((!newVoipUpProp.empty() && ((oldVoipUpProp == "PNR") ^ (newVoipUpProp == "PNR"))) ||
        (!newRecordProp.empty() && oldRecordProp != newRecordProp)) {
        uint64_t sessionId = audioEcManager_.GetOpenedNormalSourceSessionId();
        AUDIO_INFO_LOG("start reload session: %{public}" PRIu64 " for effect change", sessionId);
        audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[sessionId]);
    }
}

std::string AudioCapturerSession::GetEnhancePropByName(const AudioEnhancePropertyArray &propertyArray,
    const std::string &propName)
{
    std::string propValue = "";
    auto iter = std::find_if(propertyArray.property.begin(), propertyArray.property.end(),
        [&propName](const AudioEnhanceProperty &prop) {
            return prop.enhanceClass == propName;
        });
    if (iter != propertyArray.property.end()) {
        propValue = iter->enhanceProp;
    }
    return propValue;
}

void AudioCapturerSession::ReloadSourceForEffect(const AudioEnhancePropertyArray &oldPropertyArray,
    const AudioEnhancePropertyArray &newPropertyArray)
{
    if (!audioEcManager_.GetMicRefFeatureEnable()) {
        AUDIO_INFO_LOG("reload ignore for feature not enable");
        return;
    }
    if (audioEcManager_.GetSourceOpened() != SOURCE_TYPE_VOICE_COMMUNICATION &&
        audioEcManager_.GetSourceOpened() != SOURCE_TYPE_MIC) {
        AUDIO_INFO_LOG("reload ignore for source not voip or record");
        return;
    }
    std::string oldRecordProp = GetEnhancePropByName(oldPropertyArray, "record");
    std::string oldVoipUpProp = GetEnhancePropByName(oldPropertyArray, "voip_up");
    std::string newRecordProp = GetEnhancePropByName(newPropertyArray, "record");
    std::string newVoipUpProp = GetEnhancePropByName(newPropertyArray, "voip_up");
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    if ((!newVoipUpProp.empty() && ((oldVoipUpProp == "PNR") ^ (newVoipUpProp == "PNR"))) ||
        (!newRecordProp.empty() && oldRecordProp != newRecordProp)) {
        uint64_t sessionId = audioEcManager_.GetOpenedNormalSourceSessionId();
        AUDIO_INFO_LOG("start reload session: %{public}" PRIu64 " for enhance effect change", sessionId);
        audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[sessionId]);
    }
}

}
}
