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
    {SOURCE_TYPE_VOICE_CALL, 7},
    {SOURCE_TYPE_VOICE_COMMUNICATION, 6},
    {SOURCE_TYPE_VOICE_TRANSCRIPTION, 5},
    {SOURCE_TYPE_VOICE_RECOGNITION, 4},
    {SOURCE_TYPE_CAMCORDER, 2},
    {SOURCE_TYPE_MIC, 2},
    {SOURCE_TYPE_UNPROCESSED, 1},
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
        audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT);
        UnloadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME);
    }
    audioDeviceCommon_.FetchDevice(true);
    audioDeviceCommon_.FetchDevice(false);

    // update a2dp offload
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
#endif
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
        StreamPropInfo targetInfo;
        SourceType targetSource;
        int32_t res = audioEcManager_.FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSource);
        CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res,
            "fetch target source info error");

        if (audioEcManager_.GetSourceOpened() == SOURCE_TYPE_INVALID) {
            // normal source is not opened before
            audioEcManager_.PrepareAndOpenNormalSource(sessionInfo, targetInfo, targetSource);
            sessionIdUsedToOpenSource_ = sessionID;
        } else if (IsHigherPrioritySource(targetSource, audioEcManager_.GetSourceOpened())) {
            // reload if higher source come
            audioEcManager_.CloseNormalSource();
            audioEcManager_.PrepareAndOpenNormalSource(sessionInfo, targetInfo, targetSource);
            sessionIdUsedToOpenSource_ = sessionID;
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
    uint32_t highestSession = 0;
    // find highest source in remaining session
    for (const auto &iter : sessionWithNormalSourceType_) {
        if (IsHigherPrioritySource(iter.second.sourceType, highestSource)) {
            highestSession = iter.first;
            highestSource = iter.second.sourceType;
        }
    }

    // Convert highestSource to highestSourceInHdi
    SourceType highestSourceInHdi = SOURCE_TYPE_INVALID;
    bool useMatchingPropInfo = false;
    audioEcManager_.GetTargetSourceTypeAndMatchingFlag(highestSource, highestSourceInHdi, useMatchingPropInfo);

    // if remaining sources are all lower than current removeed one, reload with the highest source in remaining
    if (highestSource != SOURCE_TYPE_INVALID && IsHigherPrioritySource(audioEcManager_.GetSourceOpened(),
        highestSourceInHdi)) {
        AUDIO_INFO_LOG("reload source %{pblic}d because higher source removed, normalSourceOpened:%{public}d, "
            "highestSourceInHdi:%{public}d ", highestSource, audioEcManager_.GetSourceOpened(), highestSourceInHdi);
        audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[highestSession]);
        sessionIdUsedToOpenSource_ = highestSession;
    }
}

bool AudioCapturerSession::ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
    AudioModuleInfo &audioModuleInfo)
{
    if (!audioConfigManager_.GetAdapterInfoFlag()) {
        return false;
    }

    AudioAdapterInfo info;
    AdaptersType type = static_cast<AdaptersType>(AudioPolicyUtils::portStrToEnum[std::string(PRIMARY_WAKEUP)]);
    bool ret = audioConfigManager_.GetAdapterInfoByType(type, info);
    if (!ret) {
        AUDIO_ERR_LOG("can not find adapter info");
        return false;
    }

    auto pipeInfo = info.GetPipeByName(PIPE_WAKEUP_INPUT);
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("wakeup pipe info is nullptr");
        return false;
    }

    if (!FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo)) {
        AUDIO_ERR_LOG("failed to fill pipe stream prop info");
        return false;
    }

    audioModuleInfo.adapterName = info.adapterName_;
    audioModuleInfo.name = pipeInfo->moduleName_;
    audioModuleInfo.lib = pipeInfo->lib_;
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
bool AudioCapturerSession::FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, PipeInfo *pipeInfo,
    AudioModuleInfo &audioModuleInfo)
{
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("wakeup pipe info is nullptr");
        return false;
    }

    if (pipeInfo->streamPropInfos_.size() == 0) {
        AUDIO_ERR_LOG("no stream prop info");
        return false;
    }

    auto targetIt = pipeInfo->streamPropInfos_.begin();
    for (auto it = pipeInfo->streamPropInfos_.begin(); it != pipeInfo->streamPropInfos_.end(); ++it) {
        if (it -> channelLayout_ == static_cast<uint32_t>(streamInfo.channels)) {
            targetIt = it;
            break;
        }
    }

    audioModuleInfo.format = targetIt->format_;
    audioModuleInfo.channels = std::to_string(targetIt->channelLayout_);
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
    AudioEcInfo lastEcInfo = audioEcManager_.GetAudioEcInfo();
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
    AUDIO_INFO_LOG("form caller: %{public}s", caller.c_str());
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
            return;
        }
    } else {
        if (inputDevice.deviceType_ == DEVICE_TYPE_DEFAULT ||
            inputDevice.IsSameDeviceDesc(GetInputDeviceTypeForReload())) {
            AUDIO_INFO_LOG("mic source reload ignore for device not changed");
            return;
        }
    }

    std::lock_guard<std::mutex> lock(onCapturerSessionChangedMutex_);
    // reload for device change, used session is not changed
    if (sessionWithNormalSourceType_.find(sessionIdUsedToOpenSource_) == sessionWithNormalSourceType_.end()) {
        AUDIO_ERR_LOG("target session not found");
        return;
    }
    SetInputDeviceTypeForReload(inputDevice);
    audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[sessionIdUsedToOpenSource_]);
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
        audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[sessionIdUsedToOpenSource_]);
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
        audioEcManager_.ReloadSourceForSession(sessionWithNormalSourceType_[sessionIdUsedToOpenSource_]);
    }
}

}
}