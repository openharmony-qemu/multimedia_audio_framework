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

namespace OHOS {
namespace AudioStandard {
const uint32_t PCM_8_BIT = 8;
const float RENDER_FRAME_INTERVAL_IN_SECONDS = 0.02;
static const std::string PIPE_PRIMARY_INPUT = "primary_input";
static const std::string PIPE_WAKEUP_INPUT = "wakeup_input";

static inline const std::unordered_set<SourceType> specialSourceTypeSet_ = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_REMOTE_CAST
};

static std::map<SourceType, int> NORMAL_SOURCE_PRIORITY = {
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

static bool IsHigherPrioritySource(SourceType newSource, SourceType currentSource)
{
    AUDIO_INFO_LOG("newSource sourceType:%{public}d, currentSource sourceType:%{public}d", newSource, currentSource);
    if (NORMAL_SOURCE_PRIORITY.count(newSource) == 0 ||
        NORMAL_SOURCE_PRIORITY.count(currentSource) == 0 ||
        (newSource == currentSource)) {
        return false;
    }
    return NORMAL_SOURCE_PRIORITY[newSource] >= NORMAL_SOURCE_PRIORITY[currentSource];
}

static bool IsLowerPrioritySource(SourceType newSource, SourceType currentSource)
{
    AUDIO_INFO_LOG("newSource sourceType:%{public}d currentSource sourceType:%{public}d", newSource, currentSource);
    if (NORMAL_SOURCE_PRIORITY.count(newSource) == 0 ||
        NORMAL_SOURCE_PRIORITY.count(currentSource) == 0 ||
        (newSource == currentSource)) {
        return false;
    }
    return NORMAL_SOURCE_PRIORITY[newSource] < NORMAL_SOURCE_PRIORITY[currentSource];
}

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
        audioPolicyManager_.ResetRemoteCastDeviceVolume();
    } else {
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute(
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
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute();
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute();

    // update a2dp offload
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
#endif
}

CapturerState AudioCapturerSession::GetCapturerState()
{
    AudioStreamCollector &streamCollector = AudioStreamCollector::GetAudioStreamCollector();
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    streamCollector.GetCurrentCapturerChangeInfos(capturerChangeInfos);
    for (auto it = capturerChangeInfos.begin(); it != capturerChangeInfos.end(); it++) {
        if (audioEcManager_.GetOpenedNormalSourceSessionId() == static_cast<uint64_t>((*it)->sessionId)) {
            AUDIO_INFO_LOG("exiting session %{public}d, captureSate %{public}d",
                (*it)->sessionId, (*it)->capturerState);
            return (*it)->capturerState;
        }
    }
    return CAPTURER_INVALID;
}

bool AudioCapturerSession::FindRunningNormalSession(uint32_t sessionId, AudioCapturerChangeInfo &runingSessionInfo)
{
    AudioStreamCollector &streamCollector = AudioStreamCollector::GetAudioStreamCollector();
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    streamCollector.GetCurrentCapturerChangeInfos(captureChangeInfos);
    AUDIO_INFO_LOG("new sessionId %{public}d", sessionId);
    for (auto it = captureChangeInfos.begin(); it != captureChangeInfos.end(); it++) {
        AUDIO_INFO_LOG("sessionId %{public}d, captureState %{public}d", (*it)->sessionId, (*it)->capturerState);
        if ((*it)->capturerState == CAPTURER_RUNNING &&
            (specialSourceTypeSet_.count(sessionWithNormalSourceType_[(*it)->sessionId].sourceType) == 0 &&
            (specialSourceTypeSet_.count(sessionWithNormalSourceType_[sessionId].sourceType) == 0))) {
            AUDIO_INFO_LOG("existing running normal session %{public}d, sourceType %{public}d",
                (*it)->sessionId, (*it)->capturerInfo.sourceType);
            runingSessionInfo = *(*it);
            return true;
        }
    }
    return false;
}

int32_t AudioCapturerSession::ReloadCaptureSession(uint32_t sessionId, SessionOperation operation)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    AudioCapturerChangeInfo runingSessionInfo;
    PipeStreamPropInfo targetInfo;
    if (FindRunningNormalSession(sessionId, runingSessionInfo) == true) {
        AUDIO_INFO_LOG("operation %{public}d", operation);
        switch (operation) {
            case SESSION_OPERATION_START:
                if (IsHigherPrioritySource(sessionWithNormalSourceType_[sessionId].sourceType,
                    runingSessionInfo.capturerInfo.sourceType) &&
                    (Util::ConvertToHDIAudioInputType(runingSessionInfo.capturerInfo.sourceType) !=
                    Util::ConvertToHDIAudioInputType(sessionWithNormalSourceType_[sessionId].sourceType))) {
                    int32_t res = audioEcManager_.FetchTargetInfoForSessionAdd(
                        sessionWithNormalSourceType_[sessionId],
                        targetInfo, sessionWithNormalSourceType_[sessionId].sourceType);
                    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "fetch target source info error");
                    audioEcManager_.ReloadNormalSource(sessionWithNormalSourceType_[sessionId],
                        targetInfo, sessionWithNormalSourceType_[sessionId].sourceType);
                    audioEcManager_.SetOpenedNormalSourceSessionId(sessionId);
                    AUDIO_INFO_LOG("reload session %{public}d, sourceType %{public}d",
                        sessionId, sessionWithNormalSourceType_[sessionId].sourceType);
                    return SUCCESS;
                }
                AUDIO_INFO_LOG("no need reload session");
                return ERROR;
            case SESSION_OPERATION_PAUSE:
            case SESSION_OPERATION_STOP:
            case SESSION_OPERATION_RELEASE:
                if (IsLowerPrioritySource(runingSessionInfo.capturerInfo.sourceType,
                    sessionWithNormalSourceType_[sessionId].sourceType) &&
                    (Util::ConvertToHDIAudioInputType(runingSessionInfo.capturerInfo.sourceType) !=
                    Util::ConvertToHDIAudioInputType(sessionWithNormalSourceType_[sessionId].sourceType))) {
                    int32_t lowPrioritySessionId = runingSessionInfo.sessionId;
                    int32_t res = audioEcManager_.FetchTargetInfoForSessionAdd(
                        sessionWithNormalSourceType_[lowPrioritySessionId],
                        targetInfo, sessionWithNormalSourceType_[lowPrioritySessionId].sourceType);
                    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "fetch target source info error");
                    audioEcManager_.ReloadNormalSource(sessionWithNormalSourceType_[lowPrioritySessionId],
                        targetInfo, sessionWithNormalSourceType_[lowPrioritySessionId].sourceType);
                    audioEcManager_.SetOpenedNormalSourceSessionId(lowPrioritySessionId);
                    AUDIO_INFO_LOG("reload session %{public}d, sourceType %{public}d",
                        lowPrioritySessionId, sessionWithNormalSourceType_[lowPrioritySessionId].sourceType);
                    return SUCCESS;
                }
                AUDIO_INFO_LOG("no need reload session");
                return ERROR;
            default:
                return ERROR;
        }
    }
    return ERROR;
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
        // normal source types, dynamic open
        PipeStreamPropInfo targetInfo;
        SourceType targetSource;
        int32_t res = audioEcManager_.FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSource);
        CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "fetch target source info error");

        AUDIO_INFO_LOG("Current source: %{public}d, target: %{public}d",
            audioEcManager_.GetSourceOpened(), targetSource);
        if (audioEcManager_.GetSourceOpened() == SOURCE_TYPE_INVALID) {
            // normal source is not opened before -- it should not be happen!!
            AUDIO_WARNING_LOG("Record route should not be opened here!");
            return SUCCESS;
        } else if (GetCapturerState() == CAPTURER_RUNNING &&
            IsHigherPrioritySource(targetSource, audioEcManager_.GetSourceOpened())) {
            // exit running stream, High-priority targetSource stream be created CapturerInServer::Start!
            AUDIO_WARNING_LOG("High-priority targetSource no need to created!");
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
        if (!sessionWithNormalSourceType_.empty()) {
            HandleRemainingSource();
            return;
        }
        // close source when all capturer sessions removed
        audioEcManager_.CloseNormalSource();
        return;
    }

    AUDIO_INFO_LOG("Sessionid:%{public}" PRIu64 " not added, directly placed into sessionIdisRemovedSet_", sessionID);
    sessionIdisRemovedSet_.insert(sessionID);
}

// HandleRemainingSource must be called with onCapturerSessionChangedMutex_ held
void AudioCapturerSession::HandleRemainingSource()
{
    SourceType highestSource = SOURCE_TYPE_INVALID;
    SourceType normalSourceInHdi = SOURCE_TYPE_INVALID;
    SourceType openSource = audioEcManager_.GetSourceOpened();
    bool useMatchingPropInfo = false;
    uint32_t highestSession = 0;
    // find highest source in remaining session
    for (const auto &iter : sessionWithNormalSourceType_) {
        // Convert sessionWithNormalSourceType to normalSourceInHdi
        audioEcManager_.GetTargetSourceTypeAndMatchingFlag(iter.second.sourceType,
            normalSourceInHdi, useMatchingPropInfo);
        AUDIO_INFO_LOG("Get session:%{public}d from map，convert original sourceType:%{public}d into "
            "HdiSource:%{public}d", iter.first, iter.second.sourceType, normalSourceInHdi);
        if (IsHigherPrioritySource(normalSourceInHdi, highestSource)) {
            highestSession = iter.first;
            highestSource = normalSourceInHdi;
        }
    }

    // if remaining sources are all lower than current removeed one, reload with the highest source in remaining
    AUDIO_INFO_LOG("the highestSource:%{public}d, normalSourceOpened_:%{public}d", highestSource, openSource);
    if (highestSource != SOURCE_TYPE_INVALID && IsHigherPrioritySource(openSource, highestSource)) {
        audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[highestSession]);
        audioEcManager_.SetOpenedNormalSourceSessionId(highestSession);
    }
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
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION, -1);
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
    AUDIO_INFO_LOG("voice source reload ignore for device not change");
    return false;
}

void AudioCapturerSession::ReloadSourceForDeviceChange(const AudioDeviceDescriptor &inputDevice,
    const AudioDeviceDescriptor &outputDevice, const std::string &caller)
{
    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    AUDIO_INFO_LOG("form caller: %{public}s, inDevice: %{public}d, outDevice: %{public}d", caller.c_str(),
        inputDevice.deviceType_, outputDevice.deviceType_);
    if (!audioEcManager_.GetEcFeatureEnable()) {
        AUDIO_INFO_LOG("reload ignore for feature not enable");
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