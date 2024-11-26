

#ifndef LOG_TAG
#define LOG_TAG "AudioPolicyCapturerSession"
#endif

#include "audio_policy_capturer_session.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_utils.h"

#include "audio_device_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_router_center.h"

#include "audio_policy_volume.h"
#include "audio_policy_ec.h"
#include "audio_policy_common.h"
#include "audio_policy_config_manager.h"
#include "audio_policy_active_device.h"
#include "audio_policy_device_common.h"
#include "audio_policy_connected_device.h"
#include "audio_a2dp_offload_manager.h"
#include "audio_policy_io_handle_manager.h"

namespace OHOS {
namespace AudioStandard {
const uint32_t PCM_8_BIT = 8;
const float RENDER_FRAME_INTERVAL_IN_SECONDS = 0.02;
static const std::string PIPE_PRIMARY_INPUT = "primary_input";
static const std::string PIPE_WAKEUP_INPUT = "wakeup_input";

inline std::string GetEncryptAddr(const std::string &addr)
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

inline bool IsHigherPrioritySource(SourceType newSource, SourceType currentSource)
{
    std::map<SourceType, int> NORMAL_SOURCE_PRIORITY = {
        // from high to low
        {SOURCE_TYPE_VOICE_CALL, 5},
        {SOURCE_TYPE_VOICE_COMMUNICATION, 4},
        {SOURCE_TYPE_VOICE_TRANSCRIPTION, 3},
        {SOURCE_TYPE_MIC, 2},
        {SOURCE_TYPE_VOICE_RECOGNITION, 1},
    };

    if (NORMAL_SOURCE_PRIORITY.count(newSource) == 0 ||
        NORMAL_SOURCE_PRIORITY.count(currentSource) == 0) {
        return false;
    }
    return NORMAL_SOURCE_PRIORITY[newSource] > NORMAL_SOURCE_PRIORITY[currentSource];
}

void AudioPolicyCapturerSession::SetConfigParserFlag()
{
    isPolicyConfigParsered_ = true;
}

void AudioPolicyCapturerSession::LoadInnerCapturerSink(std::string moduleName, AudioStreamInfo streamInfo)
{
    AUDIO_INFO_LOG("Start");
    uint32_t bufferSize = (streamInfo.samplingRate * AudioPolicyCommon::GetInstance().GetSampleFormatValue(streamInfo.format)
        * streamInfo.channels) / PCM_8_BIT * RENDER_FRAME_INTERVAL_IN_SECONDS;

    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    moduleInfo.format = AudioPolicyCommon::GetInstance().ConvertToHDIAudioFormat(streamInfo.format);
    moduleInfo.name = moduleName;
    moduleInfo.networkId = "LocalDevice";
    moduleInfo.channels = std::to_string(streamInfo.channels);
    moduleInfo.rate = std::to_string(streamInfo.samplingRate);
    moduleInfo.bufferSize = std::to_string(bufferSize);

    AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
}

void AudioPolicyCapturerSession::UnloadInnerCapturerSink(std::string moduleName)
{
    AudioPolicyIOHandleManager::GetInstance().ClosePortAndEraseIOHandle(moduleName);
}

void AudioPolicyCapturerSession::HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo)
{
    AudioDeviceDescriptor updatedDesc = AudioDeviceDescriptor(DEVICE_TYPE_REMOTE_CAST,
        AudioPolicyCommon::GetInstance().GetDeviceRole(DEVICE_TYPE_REMOTE_CAST));
    std::vector<sptr<AudioDeviceDescriptor>> descForCb = {};
    if (isConnected) {
        // If device already in list, remove it else do not modify the list
        AudioPolicyConnectedDevice::GetInstance().DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_, updatedDesc.macAddress_);
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);
        LoadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME, streamInfo);
        AudioPolicyManagerFactory::GetAudioPolicyManager().ResetRemoteCastDeviceVolume();
    } else {
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT);
        UnloadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME);
    }
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);

    // update a2dp offload
    AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream();
}

int32_t AudioPolicyCapturerSession::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    AUDIO_INFO_LOG("sessionID: %{public}" PRIu64 " source: %{public}d", sessionID, sessionInfo.sourceType);
    CHECK_AND_RETURN_RET_LOG(isPolicyConfigParsered_ && AudioPolicyVolume::GetInstance().GetLoadFlag(), ERROR,
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
        int32_t res = FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSource);
        CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "fetch target source info error");

        if (AudioPolicyEc::GetInstance().GetSourceOpened() == SOURCE_TYPE_INVALID) {
            // normal source is not opened before
            AudioPolicyEc::GetInstance().PrepareAndOpenNormalSource(sessionInfo, targetInfo, targetSource);
        } else if (IsHigherPrioritySource(targetSource, AudioPolicyEc::GetInstance().GetSourceOpened())) {
            // reload if higher source come
            AudioPolicyEc::GetInstance().CloseNormalSource();
            AudioPolicyEc::GetInstance().PrepareAndOpenNormalSource(sessionInfo, targetInfo, targetSource);
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

void AudioPolicyCapturerSession::OnCapturerSessionRemoved(uint64_t sessionID)
{
    AUDIO_INFO_LOG("sessionid:%{public}" PRIu64, sessionID);
    if (sessionWithSpecialSourceType_.count(sessionID) > 0) {
        if (sessionWithSpecialSourceType_[sessionID].sourceType == SOURCE_TYPE_REMOTE_CAST) {
            HandleRemoteCastDevice(false);
        }
        sessionWithSpecialSourceType_.erase(sessionID);
        return;
    }

    if (sessionWithNormalSourceType_.count(sessionID) > 0) {
        if (sessionWithNormalSourceType_[sessionID].sourceType == SOURCE_TYPE_VOICE_RECOGNITION) {
            BluetoothScoDisconectForRecongnition();
        }
        if (sessionWithNormalSourceType_[sessionID].sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            AudioPolicyEc::GetInstance().SetAudioEcNone();
        }
        sessionWithNormalSourceType_.erase(sessionID);
        if (!sessionWithNormalSourceType_.empty()) {
            HandleRemainingSource();
            return;
        }
        // close source when all capturer sessions removed
        AudioPolicyEc::GetInstance().CloseNormalSource();
        return;
    }

    AUDIO_INFO_LOG("Sessionid:%{public}" PRIu64 " not added, directly placed into sessionIdisRemovedSet_", sessionID);
    sessionIdisRemovedSet_.insert(sessionID);
}

void AudioPolicyCapturerSession::HandleRemainingSource()
{
    // if remaining sources are all lower than current one, reload with the highest source
    SourceType highestSource = SOURCE_TYPE_MIC;
    uint32_t highestSession = 0;
    for (auto &iter : sessionWithNormalSourceType_) {
        if (IsHigherPrioritySource(iter.second.sourceType, highestSource)) {
            highestSession = iter.first;
            highestSource = iter.second.sourceType;
        }
    }
    if (IsHigherPrioritySource(AudioPolicyEc::GetInstance().GetSourceOpened(), highestSource)) {
        AUDIO_INFO_LOG("reload source %{public}d because higher source removed", highestSource);
        StreamPropInfo targetInfo;
        SourceType targetSource;
        int32_t res = FetchTargetInfoForSessionAdd(
            sessionWithNormalSourceType_[highestSession], targetInfo, targetSource);
        CHECK_AND_RETURN_LOG(res == SUCCESS,
            "FetchTargetInfoForSessionAdd error, maybe device not support recorder");
        AudioPolicyEc::GetInstance().CloseNormalSource();
        AudioPolicyEc::GetInstance().PrepareAndOpenNormalSource(
            sessionWithNormalSourceType_[highestSession], targetInfo, targetSource);
    }
}

int32_t AudioPolicyCapturerSession::FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo,
    StreamPropInfo &targetInfo, SourceType &targetSourceType)
{
    const PipeInfo *pipeInfoPtr = nullptr;
    AudioAdapterInfo adapterInfo;
    bool ret = AudioPolicyConfigManager::GetInstance().GetAdapterInfoByType(
        AdaptersType::TYPE_PRIMARY, adapterInfo);
    if (ret) {
        pipeInfoPtr = adapterInfo.GetPipeByName(PIPE_PRIMARY_INPUT);
    }
    CHECK_AND_RETURN_RET_LOG(pipeInfoPtr != nullptr, ERROR, "pipeInfoPtr is null");

    return AudioPolicyEc::GetInstance().FetchTargetInfo(sessionInfo, pipeInfoPtr, targetInfo, targetSourceType);
}

void AudioPolicyCapturerSession::BluetoothScoDisconectForRecongnition()
{
    AudioDeviceDescriptor tempDesc = AudioPolicyActiveDevice::GetInstance().GetCurrentInputDevice();
    AUDIO_INFO_LOG("Recongnition scoCategory: %{public}d, deviceType: %{public}d, scoState: %{public}d",
        Bluetooth::AudioHfpManager::GetScoCategory(), tempDesc.deviceType_,
        AudioDeviceManager::GetAudioDeviceManager().GetScoState());
    if (tempDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        int32_t ret = AudioPolicyDeviceCommon::GetInstance().ScoInputDeviceFetchedForRecongnition(false, tempDesc.macAddress_, tempDesc.connectState_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "sco [%{public}s] disconnected failed",
            GetEncryptAddr(tempDesc.macAddress_).c_str());
    }
}

bool AudioPolicyCapturerSession::ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
    AudioModuleInfo &audioModuleInfo)
{
    if (!AudioPolicyConfigManager::GetInstance().GetAdapterInfoFlag()) {
        return false;
    }
    AudioAdapterInfo info;
    bool ret = AudioPolicyConfigManager::GetInstance().GetAdapterInfoByType(
        AudioPolicyCommon::GetInstance().GetAdapterType(std::string(PRIMARY_WAKEUP)), info);
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

int32_t AudioPolicyCapturerSession::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    InternalAudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo = config.streamInfo;
    AUDIO_INFO_LOG("set wakeup audio capturer start");
    AudioModuleInfo moduleInfo = {};
    if (!ConstructWakeupAudioModuleInfo(capturerOptions.streamInfo, moduleInfo)) {
        AUDIO_ERR_LOG("failed to construct wakeup audio module info");
        return ERROR;
    }
    AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);

    AUDIO_DEBUG_LOG("set wakeup audio capturer end");
    return SUCCESS;
}

int32_t AudioPolicyCapturerSession::CloseWakeUpAudioCapturer()
{
    AUDIO_INFO_LOG("close wakeup audio capturer start");
    return AudioPolicyIOHandleManager::GetInstance().ClosePortAndEraseIOHandle(std::string(PRIMARY_WAKEUP));
}

// private method
bool AudioPolicyCapturerSession::FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, PipeInfo *pipeInfo,
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
            AUDIO_INFO_LOG("find target pipe info");
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



}
}