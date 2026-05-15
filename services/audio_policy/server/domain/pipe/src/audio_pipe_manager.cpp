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
#define LOG_TAG "AudioPipeManager"
#endif

#include "audio_pipe_manager.h"
#include "audio_injector_policy.h"
#include "audio_definition_adapter_info.h"
#include "audio_definition_policy_utils.h"
#include "audio_policy_utils.h"
#include "parameters.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B84

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t DECIMAL = 10;
const uint32_t FIRST_SESSIONID = 100000;
const uint32_t RING_SESSIONID = 1;
const uint32_t VOIP_SESSIONID = 2;
const int32_t MEDIA_SERVICE_UID = 1013;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
const uint32_t DEFAULT_PIPE_ID = 0;
constexpr const char *MULTI_STREAM_DEVICE_SUPPORT = "const.multimedia.audio.sys_multidevice_capability.enable";
const bool IS_DEVICE_ENHANCED_SUPPORTED = system::GetBoolParameter(MULTI_STREAM_DEVICE_SUPPORT, false);
}
AudioPipeManager::AudioPipeManager()
    : audioConfigManager_(AudioCoreConfigManager::GetInstance()),
      audioConcurrencyManager_(AudioConcurrencyManager::GetInstance())
{
}

bool AudioPipeManager::Init()
{
    audioConcurrencyManager_.Init();
    if (audioConfigManager_.GetAdapterInfoFlag()) {
        return true;
    }
    return audioConfigManager_.Init();
}


bool AudioPipeManager::GetUltraFastFlag()
{
    return audioConfigManager_.GetUltraFastFlag();
}

bool AudioPipeManager::PreferMultiChannelPipe(std::shared_ptr<AudioStreamDescriptor> &desc)
{
    return audioConfigManager_.PreferMultiChannelPipe(desc);
}

uint32_t AudioPipeManager::GetStreamPropInfoSize(const std::string &adapterName, const std::string &pipeName)
{
    return audioConfigManager_.GetStreamPropInfoSize(adapterName, pipeName);
}

bool AudioPipeManager::GetUpdateRouteSupport()
{
    return audioConfigManager_.GetUpdateRouteSupport();
}

bool AudioPipeManager::GetHasEarpiece()
{
    return audioConfigManager_.GetHasEarpiece();
}

bool AudioPipeManager::GetAdapterInfoFlag()
{
    return audioConfigManager_.GetAdapterInfoFlag();
}

bool AudioPipeManager::GetAdapterInfoByType(AudioAdapterType type, std::shared_ptr<PolicyAdapterInfo> &info)
{
    return audioConfigManager_.GetAdapterInfoByType(type, info);
}

bool AudioPipeManager::GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList)
{
    return audioConfigManager_.GetModuleListByType(type, moduleList);
}

void AudioPipeManager::UpdateDynamicCapturerConfig(ClassType type, const AudioModuleInfo moduleInfo)
{
    audioConfigManager_.UpdateDynamicCapturerConfig(type, moduleInfo);
}

DirectPlaybackMode AudioPipeManager::GetDirectPlaybackSupport(std::shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamInfo &streamInfo)
{
    return audioConfigManager_.GetDirectPlaybackSupport(desc, streamInfo);
}

bool AudioPipeManager::NeedUidCheckForOffloadPipe(const std::shared_ptr<AudioStreamDescriptor> &desc)
{
    return audioConfigManager_.NeedUidCheckForOffloadPipe(desc);
}

void AudioPipeManager::UpdateStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
    const std::list<DeviceStreamInfo> &deviceStreamInfo, const std::list<std::string> &supportDevices)
{
    audioConfigManager_.UpdateStreamPropInfo(adapterName, pipeName, deviceStreamInfo, supportDevices);
}

void AudioPipeManager::ClearStreamPropInfo(const std::string &adapterName, const std::string &pipeName)
{
    audioConfigManager_.ClearStreamPropInfo(adapterName, pipeName);
}

uint32_t AudioPipeManager::GetRouteFlag(std::shared_ptr<AudioStreamDescriptor> &desc)
{
    return audioConfigManager_.GetRouteFlag(desc);
}

void AudioPipeManager::GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc,
    std::shared_ptr<PipeStreamPropInfo> &info)
{
    audioConfigManager_.GetStreamPropInfo(desc, info);
}

#ifdef MULTI_BUS_ENABLE
void AudioPipeManager::GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc,
    std::shared_ptr<PipeStreamPropInfo> &info, const std::vector<std::string> &busAddresses)
{
    audioConfigManager_.GetStreamPropInfo(desc, info, busAddresses);
}
#endif

void AudioPipeManager::GetGlobalConfigs(PolicyGlobalConfigs &globalConfigs)
{
    audioConfigManager_.GetGlobalConfigs(globalConfigs);
}

void AudioPipeManager::SetEcEnableState(bool ecEnableState)
{
    audioConfigManager_.SetEcEnableState(ecEnableState);
}

AudioSampleFormat AudioPipeManager::GetFastFormat() const
{
    return audioConfigManager_.GetFastFormat();
}

bool AudioPipeManager::IsSupportInnerCaptureOffload()
{
    return audioConfigManager_.IsSupportInnerCaptureOffload();
}

int32_t AudioPipeManager::GetMaxRendererInstances()
{
    return audioConfigManager_.GetMaxRendererInstances();
}

void AudioPipeManager::GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo)
{
    audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);
}

AudioPipeManager::~AudioPipeManager()
{
    AUDIO_INFO_LOG("Dtor");
    curPipeList_.clear();
}

void AudioPipeManager::AddAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info)
{
    Trace trace("AudioPipeManager::AddAudioPipeInfo");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    auto streamInfo = info->audioStreamInfo_;
    AUDIO_INFO_LOG("Add id:%{public}u, name %{public}s, format %{public}d, rate %{public}d, channel %{public}d",
        info->id_, info->name_.c_str(), streamInfo.format, streamInfo.samplingRate, streamInfo.channels);

    // Action is only used in pipe execution, while pipeManager can only store default action
    info->pipeAction_ = PIPE_ACTION_DEFAULT;
    curPipeList_.push_back(info);
}

void AudioPipeManager::RemoveAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info)
{
    Trace trace("AudioPipeManager::RemoveAudioPipeInfo");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end();) {
        if (IsSamePipe(info, *iter)) {
            CHECK_AND_CONTINUE_LOG(info != nullptr, "info is nullptr!");
            HILOG_COMM_INFO("[RemoveAudioPipeInfo]Remove id:%{public}u, name: %{public}s",
                info->id_, info->name_.c_str());
            iter = curPipeList_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void AudioPipeManager::RemoveAudioPipeInfo(AudioIOHandle id)
{
    Trace trace("AudioPipeManager::RemoveAudioPipeInfo");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end();) {
        if ((*iter)->id_ == id) {
            HILOG_COMM_INFO("[RemoveAudioPipeInfo]Remove id:%{public}u, name: %{public}s",
                id, (*iter)->name_.c_str());
            iter = curPipeList_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void AudioPipeManager::UpdateAudioPipeInfo(std::shared_ptr<AudioPipeInfo> newPipe)
{
    Trace trace("AudioPipeManager::UpdateAudioPipeInfo");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end(); iter++) {
        if (IsSamePipe(newPipe, *iter)) {
            Assign(*iter, newPipe);
            // Action is only used in pipe execution, while pipeManager can only store default action
            (*iter)->pipeAction_ = PIPE_ACTION_DEFAULT;
            break;
        }
    }
}

bool AudioPipeManager::IsSamePipe(std::shared_ptr<AudioPipeInfo> info, std::shared_ptr<AudioPipeInfo> cmpInfo)
{
    if (info->name_ == cmpInfo->name_ || info->id_ == cmpInfo->id_) {
        return true;
    }
    return false;
}

void AudioPipeManager::Assign(std::shared_ptr<AudioPipeInfo> dst, std::shared_ptr<AudioPipeInfo> src)
{
    dst = src;
}

void AudioPipeManager::StartClient(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::StartClient");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->startTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->isStandby_ = false;
}

void AudioPipeManager::PauseClient(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::PauseClient");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->streamStatus_ = STREAM_STATUS_PAUSED;
}

void AudioPipeManager::StandbyClient(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::StandbyClient");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->isStandby_ = true;
}

void AudioPipeManager::ClearStandbyFlag(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::ClearStandbyFlag");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->isStandby_ = false;
}

void AudioPipeManager::StopClient(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::StopClient");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
}

void AudioPipeManager::RemoveClient(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::RemoveClient");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    AUDIO_INFO_LOG("Cur pipe list size %{public}zu, sessionId %{public}u", curPipeList_.size(), sessionId);
    for (auto pipeInfo : curPipeList_) {
        pipeInfo->streamDescriptors_.erase(std::remove_if(pipeInfo->streamDescriptors_.begin(),
            pipeInfo->streamDescriptors_.end(), [sessionId](std::shared_ptr<AudioStreamDescriptor> streamDesc) {
                return streamDesc->sessionId_ == sessionId;
            }), pipeInfo->streamDescriptors_.end());
    }
}

void AudioPipeManager::AddStreamToPipe(std::shared_ptr<AudioPipeInfo> pipeInfo,
    std::shared_ptr<AudioStreamDescriptor> streamDesc, AudioPipeAction action)
{
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr && streamDesc != nullptr, "pipeInfo or streamDesc is nullptr");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeInfo->streamDescMap_[streamDesc->sessionId_] = streamDesc;
    pipeInfo->pipeAction_ = (pipeInfo->pipeAction_ == PIPE_ACTION_RELOAD) ? PIPE_ACTION_RELOAD : action;
}

void AudioPipeManager::ClearPipeStreams(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    pipeInfo->streamDescriptors_.clear();
    pipeInfo->streamDescMap_.clear();
}

const std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeManager::GetPipeList()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    return curPipeList_;
}

std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeManager::GetUnusedPipe()
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioPipeInfo>> newList;
    for (auto pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        if (pipe->streamDescriptors_.empty() && (IsSpecialPipe(pipe->routeFlag_) ||
            (pipe->adapterName_ == ADAPTER_TYPE_VA &&
            pipe->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL) || pipe->moduleInfo_.className == "primary_extra")) {
            newList.push_back(pipe);
        }
    }
    return newList;
}

std::vector<std::shared_ptr<AudioPipeInfo>> AudioPipeManager::GetUnusedRecordPipe()
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioPipeInfo>> unusedPipeList;
    for (auto pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        if (pipe->pipeRole_ == PIPE_ROLE_INPUT && pipe->streamDescriptors_.empty() && IsNormalRecordPipe(pipe)) {
            if (pipe->softLinkFlag_) {
                pipe->streamDescMap_.clear();
                pipe->streamDescriptors_.clear();
                continue;
            }
            unusedPipeList.push_back(pipe);
        }
    }
    return unusedPipeList;
}

bool AudioPipeManager::IsSpecialPipe(uint32_t routeFlag)
{
    if ((routeFlag & AUDIO_OUTPUT_FLAG_FAST) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_HWDECODING) ||
        (routeFlag & AUDIO_INPUT_FLAG_FAST) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) ||
        (routeFlag & AUDIO_INPUT_FLAG_AI) ||
        (routeFlag & AUDIO_INPUT_FLAG_UNPROCESS) ||
        (routeFlag & AUDIO_INPUT_FLAG_LIVE) ||
        (routeFlag & AUDIO_INPUT_FLAG_ULTRASONIC) ||
        (routeFlag & AUDIO_INPUT_FLAG_VOICE_RECOGNITION) ||
        (routeFlag & AUDIO_INPUT_FLAG_RAW_AI) ||
        (routeFlag & AUDIO_INPUT_FLAG_INTERPHONE) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_INTERPHONE) ||
        (routeFlag & AUDIO_INPUT_FLAG_CAMCORDER)) {
        AUDIO_INFO_LOG("Flag %{public}d", routeFlag);
        return true;
    }
    return false;
}

bool AudioPipeManager::IsNormalRecordPipe(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "Pipe info is null");
    if ((pipeInfo->adapterName_ == PRIMARY_CLASS && pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL) ||
        (pipeInfo->adapterName_ == USB_CLASS && pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL) ||
        (pipeInfo->adapterName_ == ADAPTER_TYPE_VA && pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL)) {
        return true;
    }
    return false;
}

std::shared_ptr<AudioPipeInfo> AudioPipeManager::GetPipeinfoByNameAndFlag(
    const std::string adapterName, const uint32_t routeFlag)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto it : curPipeList_) {
        if (it->adapterName_ == adapterName && it->routeFlag_ == routeFlag) {
            return it;
        }
    }
    AUDIO_ERR_LOG("Can not find pipe %{public}s", adapterName.c_str());
    return nullptr;
}

std::string AudioPipeManager::GetAdapterNameBySessionId(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::GetAdapterNameBySessionId");
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr && desc->newDeviceDescs_.size() > 0 &&
                desc->newDeviceDescs_.front() != nullptr, "desc is nullptr");
            if (desc->sessionId_ != sessionId) {
                continue;
            }
            AUDIO_INFO_LOG("adapter name: %{public}s", pipeInfo->GetAdapterName().c_str());
            return pipeInfo->GetAdapterName();
        }
    }
    AUDIO_WARNING_LOG("cannot find sessionId: %{public}u", sessionId);
    return "";
}

std::string AudioPipeManager::GetModuleNameBySessionId(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::GetModuleNameBySessionId");
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr && desc->newDeviceDescs_.size() > 0 &&
                desc->newDeviceDescs_.front() != nullptr, "desc is nullptr");
            if (desc->sessionId_ != sessionId) {
                continue;
            }
            AUDIO_INFO_LOG("adapter name: %{public}s", pipeInfo->moduleInfo_.name.c_str());
            return desc->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_REMOTE_CAST ?
                "RemoteCastInnerCapturer" : pipeInfo->moduleInfo_.name;
        }
    }
    AUDIO_WARNING_LOG("cannot find sessionId: %{public}u", sessionId);
    return "";
}

AudioStreamInfo AudioPipeManager::DecideStreamInfo(const std::shared_ptr<AudioPipeInfo> pipeInfo,
    const std::shared_ptr<AudioDeviceDescriptor> deviceDesc)
{
    AudioStreamInfo streamInfo = pipeInfo->audioStreamInfo_;
    if (deviceDesc->getType() == DEVICE_TYPE_USB_ARM_HEADSET) {
        const auto &rate = static_cast<int32_t>(std::strtol(pipeInfo->moduleInfo_.rate.c_str(), nullptr, DECIMAL));
        if (rate > AudioSamplingRate::SAMPLE_RATE_96000) {
            streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
            return streamInfo;
        }
        streamInfo.samplingRate = static_cast<AudioSamplingRate>(rate);
        const std::string &format = pipeInfo->moduleInfo_.format;
        auto it = AudioDefinitionPolicyUtils::formatStrToEnum.find(format);
        CHECK_AND_RETURN_RET_LOG(it != AudioDefinitionPolicyUtils::formatStrToEnum.end(), streamInfo,
            "Not found %{public}s in formatStrToEnum", format.c_str());
        streamInfo.format = it->second;
    }
    return streamInfo;
}

std::shared_ptr<AudioDeviceDescriptor> AudioPipeManager::GetProcessDeviceInfoBySessionId(
    uint32_t sessionId, AudioStreamInfo &streamInfo)
{
    Trace trace("AudioPipeManager::GetProcessDeviceInfoBySessionId");
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr && desc->newDeviceDescs_.size() > 0 &&
                desc->newDeviceDescs_.front() != nullptr, "desc is nullptr");
            if (desc->sessionId_ == sessionId) {
                const auto deviceDesc = desc->newDeviceDescs_.front();
                AUDIO_INFO_LOG("Device type: %{public}d", deviceDesc->deviceType_);
                streamInfo = DecideStreamInfo(pipeInfo, deviceDesc);
                return deviceDesc;
            }
        }
    }
    AUDIO_ERR_LOG("Cannot find session: %{public}u", sessionId);
    return nullptr;
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetAllOutputStreamDescs()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        if (it->pipeRole_ == PIPE_ROLE_OUTPUT) {
            streamDescs.insert(streamDescs.end(), it->streamDescriptors_.begin(), it->streamDescriptors_.end());
        }
    }
    return streamDescs;
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetAllOutputStreamDescsInfo()
{
    Trace trace("AudioPipeManager::GetAllOutputStreamDescsInfo");
    // this function only get useful info for volume control
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        CHECK_AND_CONTINUE(it->pipeRole_ == PIPE_ROLE_OUTPUT);
        for (auto desc : it->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            std::shared_ptr<AudioStreamDescriptor> tmp = std::make_shared<AudioStreamDescriptor>();
            tmp->newDeviceDescs_ = desc->GetNewDeviceDescs();
            tmp->rendererInfo_.streamUsage = desc->rendererInfo_.streamUsage;
            tmp->appInfo_ = desc->appInfo_;
            tmp->sessionId_ = desc->sessionId_;
            tmp->callerUid_ = desc->callerUid_;
            tmp->callerPid_ = desc->callerPid_;
            tmp->streamStatus_ = desc->streamStatus_;
            streamDescs.push_back(tmp);
            CHECK_AND_CONTINUE(!tmp->newDeviceDescs_.empty() && tmp->newDeviceDescs_.front() != nullptr);
            tmp->newDeviceDescs_.front()->hasStreamRunning_ = false;
        }
    }
    return streamDescs;
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetAllInputStreamDescs()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        if (it->pipeRole_ == PIPE_ROLE_INPUT) {
            streamDescs.insert(streamDescs.end(), it->streamDescriptors_.begin(), it->streamDescriptors_.end());
        }
    }
    return streamDescs;
}

bool AudioPipeManager::HasRunningScoOutputStream()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        CHECK_AND_CONTINUE_LOG(it->pipeRole_ == PIPE_ROLE_OUTPUT, "not output pipe");
        for (auto &desc : it->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            CHECK_AND_CONTINUE_LOG(desc->streamStatus_ == STREAM_STATUS_STARTED, "stream not started");
            CHECK_AND_CONTINUE_LOG(!desc->newDeviceDescs_.empty(), "newDeviceDescs is empty");
            if (desc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetStreamDescsByIoHandle(AudioIOHandle id)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto it : curPipeList_) {
        if (it != nullptr && it->id_ == id) {
            return it->streamDescriptors_;
        }
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {};
    return streamDescs;
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeManager::GetStreamDescById(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    return GetStreamDescByIdInner(sessionId);
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeManager::GetStreamDescByIdInner(uint32_t sessionId)
{
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (desc->sessionId_ == sessionId) {
                return desc;
            }
        }
    }
    return nullptr;
}

int32_t AudioPipeManager::GetClientUidBySessionId(uint32_t sessionId)
{
    Trace trace("AudioPipeManager::GetClientUidBySessionId");
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (desc->sessionId_ == sessionId) {
                return desc->callerUid_ == MEDIA_SERVICE_UID ? desc->appInfo_.appUid : desc->callerUid_;
            }
        }
    }
    return -1;
}

int32_t AudioPipeManager::GetStreamCount(const std::string adapterName, const uint32_t routeFlag)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    int32_t count = 0;
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        if (it->adapterName_ == adapterName && it->routeFlag_ == routeFlag) {
            count = static_cast<int32_t>(it->streamDescriptors_.size());
        }
    }
    return count;
}

uint32_t AudioPipeManager::GetPaIndexByIoHandle(AudioIOHandle id)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        if (it->id_ == id) {
            return it->paIndex_;
        }
    }
    return HDI_INVALID_ID;
}

uint32_t AudioPipeManager::QueryPipeIdBySessionId(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &it : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(it != nullptr, "pipeInfo is nullptr");
        for (const auto &streamDesc : it->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "streamDesc is nullptr");
            if (streamDesc->sessionId_ == sessionId) {
                return it->id_;
            }
        }
    }
    if (IS_DEVICE_ENHANCED_SUPPORTED) {
        // For multi devices scenario, if sessionId is not found,
        // return invalid id to avoid primary speaker with wrong device.
        return HDI_INVALID_ID;
    }
    return DEFAULT_PIPE_ID;
}

void AudioPipeManager::UpdateRendererPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioPipeInfo>> tempList;
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->pipeRole_ == PIPE_ROLE_INPUT) {
            tempList.push_back(pipeInfo);
        }
    }
    // pipeAction_ should only be used when operating the pipe, while pipeManager only stores the default state
    for (auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        pipe->pipeAction_ = PIPE_ACTION_DEFAULT;
        pipe->isRunning_ = pipe->HasStream() ? true : pipe->isRunning_;
        tempList.push_back(pipe);
    }
    curPipeList_.clear();
    curPipeList_ = tempList;
}

void AudioPipeManager::UpdateCapturerPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos)
{
    Trace trace("AudioPipeManager::UpdateCapturerPipeInfos");
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioPipeInfo>> tempList;
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT) {
            tempList.push_back(pipeInfo);
        }
    }
    // pipeAction_ should only be used when operating the pipe, while pipeManager only stores the default state
    for (auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        pipe->pipeAction_ = PIPE_ACTION_DEFAULT;
        tempList.push_back(pipe);
    }
    curPipeList_.clear();
    curPipeList_ = tempList;
}

uint32_t AudioPipeManager::PcmOffloadSessionCount()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER) {
            return pipeInfo->streamDescriptors_.size();
        }
    }
    return 0;
}

void AudioPipeManager::Dump(std::string &dumpString)
{
    Trace trace("AudioPipeManager::Dump");
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    dumpString += "Audio PipeManager Infos\n";
    dumpString += "  - TotalPipeNums: " + std::to_string(curPipeList_.size()) + "\n\n";

    for (auto &pipe : curPipeList_) {
        if (pipe != nullptr) {
            pipe->Dump(dumpString);
        }
    }

    dumpString += "PipeManager dump end\n";
}

bool AudioPipeManager::IsModemCommunicationIdExist()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    return !modemCommunicationIdMap_.empty();
}

bool AudioPipeManager::IsModemCommunicationIdExist(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    return modemCommunicationIdMap_.find(sessionId) != modemCommunicationIdMap_.end();
}

void AudioPipeManager::AddModemCommunicationId(uint32_t sessionId, std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    if (sessionId < FIRST_SESSIONID || sessionId > MAX_VALID_SESSIONID) {
        AUDIO_ERR_LOG("Invalid id %{public}u", sessionId);
    }
    modemCommunicationIdMap_[sessionId] = streamDesc;
}

void AudioPipeManager::RemoveModemCommunicationId(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    if (modemCommunicationIdMap_.find(sessionId) != modemCommunicationIdMap_.end()) {
        modemCommunicationIdMap_.erase(sessionId);
        AUDIO_INFO_LOG("RemoveModemCommunicationId %{public}u success", sessionId);
    } else {
        AUDIO_WARNING_LOG("RemoveModemCommunicationId fail, cannot find id %{public}u", sessionId);
    }
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeManager::GetModemCommunicationStreamDescById(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    if (modemCommunicationIdMap_.find(sessionId) != modemCommunicationIdMap_.end()) {
        AUDIO_INFO_LOG("Get %{public}u success", sessionId);
        return modemCommunicationIdMap_[sessionId];
    } else {
        AUDIO_WARNING_LOG("Cannot find id %{public}u", sessionId);
        return nullptr;
    }
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeManager::GetModemCommunicationStreamDesc()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    CHECK_AND_RETURN_RET_LOG(!modemCommunicationIdMap_.empty(), nullptr, "ModemCommunicationMap is empty!");
    return modemCommunicationIdMap_.begin()->second;
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeManager::GetModemCommunicationStreamDescCopy()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    CHECK_AND_RETURN_RET_LOG(!modemCommunicationIdMap_.empty(), nullptr, "ModemCommunicationMap is empty!");
    return std::make_shared<AudioStreamDescriptor>(modemCommunicationIdMap_.begin()->second);
}

std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetModemCommunicationMap()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    AUDIO_INFO_LOG("map size %{public}zu", modemCommunicationIdMap_.size());
    return modemCommunicationIdMap_;
}

void AudioPipeManager::UpdateModemStreamStatus(AudioStreamStatus streamStatus)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &entry : modemCommunicationIdMap_) {
        CHECK_AND_CONTINUE_LOG(entry.second != nullptr, "StreamDesc is nullptr");
        entry.second->streamStatus_ = streamStatus;
    }
}

void AudioPipeManager::UpdateModemStreamDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    std::lock_guard<std::shared_mutex> pLock(pipeListLock_);
    for (auto &entry : modemCommunicationIdMap_) {
        CHECK_AND_CONTINUE_LOG(entry.second != nullptr, "StreamDesc is nullptr");
        entry.second->SwapAndUpdateNewDeviceDescs(deviceDescs);
    }
}

void AudioPipeManager::UpdateRingAndVoipStreamStatus(const AudioScene audioScene)
{
    std::lock_guard<std::shared_mutex> pLock(pipeListLock_);

    if (ringAndVoipDescMap_[RING_SESSIONID] == nullptr || ringAndVoipDescMap_[VOIP_SESSIONID] == nullptr) {
        AUDIO_INFO_LOG("init map");
        std::shared_ptr<AudioStreamDescriptor> ringStreamDesc = std::make_shared<AudioStreamDescriptor>();
        CHECK_AND_RETURN_LOG(ringStreamDesc != nullptr, "ring is nullptr!");
        ringStreamDesc->rendererInfo_.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
        ringStreamDesc->sessionId_ = RING_SESSIONID;
        ringAndVoipDescMap_[RING_SESSIONID] = ringStreamDesc;
        std::shared_ptr<AudioStreamDescriptor> voipStreamDesc = std::make_shared<AudioStreamDescriptor>();
        CHECK_AND_RETURN_LOG(voipStreamDesc != nullptr, "voip is nullptr!");
        voipStreamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        voipStreamDesc->sessionId_ = VOIP_SESSIONID;
        ringAndVoipDescMap_[VOIP_SESSIONID] = voipStreamDesc;
    }

    if (audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING) {
        ringAndVoipDescMap_[RING_SESSIONID]->streamStatus_ = STREAM_STATUS_STARTED;
        ringAndVoipDescMap_[VOIP_SESSIONID]->streamStatus_ = STREAM_STATUS_STOPPED;
    } else if (audioScene == AUDIO_SCENE_PHONE_CHAT) {
        ringAndVoipDescMap_[VOIP_SESSIONID]->streamStatus_ = STREAM_STATUS_STARTED;
        ringAndVoipDescMap_[RING_SESSIONID]->streamStatus_ = STREAM_STATUS_STOPPED;
    } else {
        ringAndVoipDescMap_[RING_SESSIONID]->streamStatus_ = STREAM_STATUS_STOPPED;
        ringAndVoipDescMap_[VOIP_SESSIONID]->streamStatus_ = STREAM_STATUS_STOPPED;
    }
}

void AudioPipeManager::UpdateRingAndVoipStreamDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &ringDeviceDescs,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &voipDeviceDescs)
{
    std::lock_guard<std::shared_mutex> pLock(pipeListLock_);

    CHECK_AND_RETURN(ringAndVoipDescMap_[RING_SESSIONID] != nullptr && ringAndVoipDescMap_[VOIP_SESSIONID] != nullptr);

    ringAndVoipDescMap_[RING_SESSIONID]->SwapAndUpdateNewDeviceDescs(ringDeviceDescs);
    ringAndVoipDescMap_[VOIP_SESSIONID]->SwapAndUpdateNewDeviceDescs(voipDeviceDescs);
}

bool AudioPipeManager::CheckRingAndVoipStreamRunning()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    CHECK_AND_RETURN_RET(ringAndVoipDescMap_.find(RING_SESSIONID) != ringAndVoipDescMap_.end(), false);
    CHECK_AND_RETURN_RET(ringAndVoipDescMap_.find(VOIP_SESSIONID) != ringAndVoipDescMap_.end(), false);

    return ringAndVoipDescMap_[RING_SESSIONID]->streamStatus_ == STREAM_STATUS_STARTED ||
        ringAndVoipDescMap_[VOIP_SESSIONID]->streamStatus_ == STREAM_STATUS_STARTED;
}

std::shared_ptr<AudioStreamDescriptor> AudioPipeManager::GetStreamDescForAudioScene(const AudioScene audioScene)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    if (audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING) {
        if (ringAndVoipDescMap_[RING_SESSIONID] != nullptr) {
            return std::make_shared<AudioStreamDescriptor>(ringAndVoipDescMap_[RING_SESSIONID]);
        }
    }
    if (audioScene == AUDIO_SCENE_PHONE_CHAT) {
        if (ringAndVoipDescMap_[VOIP_SESSIONID] != nullptr) {
            return std::make_shared<AudioStreamDescriptor>(ringAndVoipDescMap_[VOIP_SESSIONID]);
        }
    }
    return nullptr;
}

std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetRingAndVoipDescMap()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> copiedMap;
    for (auto &entry : ringAndVoipDescMap_) {
        CHECK_AND_CONTINUE_LOG(entry.second != nullptr, "StreamDesc is nullptr");
        copiedMap[entry.first] = std::make_shared<AudioStreamDescriptor>(entry.second);
    }
    return copiedMap;
}

bool AudioPipeManager::IsModemStreamDeviceChanged(std::shared_ptr<AudioDeviceDescriptor> &deviceDescs)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    CHECK_AND_RETURN_RET_LOG(modemCommunicationIdMap_.size() > 0 &&
        modemCommunicationIdMap_.begin()->second != nullptr &&
        modemCommunicationIdMap_.begin()->second->oldDeviceDescs_.size() > 0 &&
        modemCommunicationIdMap_.begin()->second->oldDeviceDescs_.front() != nullptr,
        false, "Invalid modemCommunicationMap, size: %{public}zu", modemCommunicationIdMap_.size());
    return !modemCommunicationIdMap_.begin()->second->oldDeviceDescs_.front()->IsSameDeviceDescPtr(deviceDescs);
}

std::shared_ptr<AudioPipeInfo> AudioPipeManager::GetNormalSourceInfo(bool isEcFeatureEnable)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = GetPipeByModuleAndFlag(PRIMARY_MIC, AUDIO_INPUT_FLAG_NORMAL);
    CHECK_AND_RETURN_RET(pipeInfo == nullptr, pipeInfo);
    pipeInfo = GetPipeByModuleAndFlag(BLUETOOTH_MIC, AUDIO_INPUT_FLAG_NORMAL);
    CHECK_AND_RETURN_RET(pipeInfo == nullptr, pipeInfo);
    if (isEcFeatureEnable) {
        pipeInfo = GetPipeByModuleAndFlag(USB_MIC, AUDIO_INPUT_FLAG_NORMAL);
    }
    return pipeInfo;
}

std::shared_ptr<AudioPipeInfo> AudioPipeManager::GetPipeByModuleAndFlag(const std::string moduleName,
    const uint32_t routeFlag)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto it : curPipeList_) {
        if (it->moduleInfo_.name == moduleName && it->routeFlag_ == routeFlag) {
            return it;
        }
    }
    AUDIO_ERR_LOG("Can not find pipe %{public}s", moduleName.c_str());
    return nullptr;
}

std::vector<uint32_t> AudioPipeManager::GetStreamIdsByUidAndPid(int32_t uid, int32_t pid)
{
    Trace trace("AudioPipeManager::GetStreamIdsByUidAndPid");
    std::vector<uint32_t> sessionIds = {};
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        for (auto &streamDesc : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "streamDesc is nullptr");
            if (streamDesc->IsSamePidUid(uid, pid)) {
                sessionIds.push_back(streamDesc->sessionId_);
            }
        }
    }
    AUDIO_INFO_LOG("Session number of uid %{public}u: %{public}zu", uid, sessionIds.size());
    return sessionIds;
}

std::vector<uint32_t> AudioPipeManager::GetStreamIdsByPid(int32_t pid)
{
    Trace trace("AudioPipeManager::GetStreamIdsByPid");
    std::vector<uint32_t> sessionIds = {};
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        for (auto &streamDesc : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "streamDesc is nullptr");
            if (streamDesc->IsSamePid(pid)) {
                sessionIds.push_back(streamDesc->sessionId_);
            }
        }
    }
    AUDIO_INFO_LOG("Session number of pid %{public}u: %{public}zu", pid, sessionIds.size());
    return sessionIds;
}

void AudioPipeManager::UpdateOutputStreamDescsByIoHandle(AudioIOHandle id,
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &descs)
{
    std::lock_guard<std::shared_mutex> lock(pipeListLock_);
    for (auto &it : curPipeList_) {
        if (it != nullptr && it->id_ == id) {
            it->streamDescriptors_ = descs;
            AUDIO_INFO_LOG("Update stream desc for %{public}u", id);
            return;
        }
    }
    AUDIO_WARNING_LOG("Cannot find ioHandle: %{public}u", id);
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetAllCapturerStreamDescs()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->pipeRole_ != PIPE_ROLE_INPUT) {
            continue;
        }
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (desc->audioMode_ == AUDIO_MODE_RECORD) {
                streamDescs.push_back(desc);
            }
        }
    }
    return streamDescs;
}

std::shared_ptr<AudioPipeInfo> AudioPipeManager::FindPipeBySessionId(
    const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList, uint32_t sessionId)
{
    for (const auto &pipe : pipeList) {
        if (pipe == nullptr) {
            continue;
        }

        for (const auto &stream : pipe->streamDescriptors_) {
            if (stream == nullptr) {
                continue;
            }
            if (stream->sessionId_ == sessionId) {
                AUDIO_INFO_LOG("find pipe: %{public}s by sessionId: %{public}u", pipe->name_.c_str(), sessionId);
                return pipe;
            }
        }
    }
    return std::shared_ptr<AudioPipeInfo>();
}

bool AudioPipeManager::IsStreamUsageActive(const StreamUsage &usage)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputDescs = GetAllOutputStreamDescs();
    for (auto &desc : outputDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
        if (desc->rendererInfo_.streamUsage == usage && desc->streamStatus_ == STREAM_STATUS_STARTED) {
            return true;
        }
    }
    return false;
}

int32_t AudioPipeManager::IsCaptureVoipCall()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    VoipType type = VoipType::NO_VOIP;
    AudioInjectorPolicy &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    for (const auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is null");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "stream is null");
            bool isRunning = stream->IsRunning();
            CHECK_AND_CONTINUE_LOG(isRunning == true, "isRunning is false");
            if ((stream->routeFlag_ & AUDIO_INPUT_FLAG_NORMAL) &&
                    stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
                audioInjectorPolicy.SetVoipType(NORMAL_VOIP);
                audioInjectorPolicy.SetCapturePortIdx(pipe->paIndex_);
                type = NORMAL_VOIP;
            } else if (stream->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)) {
                audioInjectorPolicy.SetVoipType(FAST_VOIP);
                audioInjectorPolicy.SetCapturePortIdx(pipe->paIndex_);
                return FAST_VOIP;
            }
        }
    }
    return type;
}

uint32_t AudioPipeManager::GetPaIndexByName(std::string portName)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end(); iter++) {
        CHECK_AND_CONTINUE_LOG((*iter) != nullptr, "iter is null");
        if ((*iter)->name_ == portName) {
            return (*iter)->paIndex_;
        }
    }
    return HDI_INVALID_ID;
}

bool AudioPipeManager::HasPrimarySink()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->adapterName_ == PRIMARY_CLASS) {
            return true;
        }
    }
    return false;
}

bool AudioPipeManager::HasRunningStream(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            CHECK_AND_CONTINUE(desc->IsRunning());
            CHECK_AND_CONTINUE_LOG(sessionId != desc->sessionId_, "sessionId matches %{public}u", sessionId);
            return true;
        }
    }
    return false;
}

bool AudioPipeManager::IsA2dpUsed()
{
    auto outputStreamDescs = GetAllOutputStreamDescs();
    return std::any_of(outputStreamDescs.begin(), outputStreamDescs.end(), [](const auto &streamDesc) {
        return streamDesc && streamDesc->streamStatus_ == STREAM_STATUS_STARTED &&
            !streamDesc->newDeviceDescs_.empty() && streamDesc->newDeviceDescs_.front() &&
            streamDesc->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP;
    });
}

bool AudioPipeManager::HasFastOutputPipe()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if ((pipeInfo->GetRoute() & AUDIO_OUTPUT_FLAG_FAST)) {
            return true;
        }
    }
    return false;
}

bool AudioPipeManager::HasUltraFastStreamRequest()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            CHECK_AND_RETURN_RET_LOG(desc->rendererInfo_.originalFlag != AUDIO_FLAG_ULTRA_FAST, true,
                "stream %{public}u is request ultra fast route", desc->GetSessionId());
        }
    }
    return false;
}

bool AudioPipeManager::IsStreamUseUltraFastRoute(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            CHECK_AND_CONTINUE(desc->GetSessionId() == sessionId);
            return pipeInfo->GetUltraFastFlag();
        }
    }
    return false;
}

bool AudioPipeManager::HasRunningRecognitionCapturerStream()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    bool hasRunningRecognitionCapturerStream = false;
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if ((desc->streamStatus_ == STREAM_STATUS_STARTED) && (desc->audioMode_ == AUDIO_MODE_RECORD) &&
            (desc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
            desc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION)) {
                return true;
            }
        }
    }
    AUDIO_INFO_LOG("Has Running Recognition stream : %{public}d", hasRunningRecognitionCapturerStream);
    return hasRunningRecognitionCapturerStream;
}

bool AudioPipeManager::HasRunningNormalCapturerStream(DeviceType type)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if ((desc->streamStatus_ == STREAM_STATUS_STARTED) && (desc->audioMode_ == AUDIO_MODE_RECORD) &&
                (desc->capturerInfo_.sourceType == SOURCE_TYPE_MIC ||
                 desc->capturerInfo_.sourceType == SOURCE_TYPE_WAKEUP ||
                 desc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_MESSAGE ||
                 desc->capturerInfo_.sourceType == SOURCE_TYPE_CAMCORDER ||
                 desc->capturerInfo_.sourceType == SOURCE_TYPE_UNPROCESSED) &&
                ((type == DEVICE_TYPE_NONE) ||
                 (!desc->newDeviceDescs_.empty() && desc->newDeviceDescs_[0] != nullptr &&
                  desc->newDeviceDescs_[0]->deviceType_ == type))) {
                AUDIO_INFO_LOG("Running Normal Capturer stream : %{public}d with device %{public}d",
                    desc->sessionId_, type);
                return true;
            }
        }
    }
    return false;
}

bool AudioPipeManager::IsOnPrimaryAdapter(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (pipeInfo->adapterName_ == ADAPTER_TYPE_PRIMARY &&
            pipeInfo->streamDescMap_.find(sessionId) != pipeInfo->streamDescMap_.end()) {
            return true;
        }
    }
    return false;
}

void AudioPipeManager::AddSingleSwitchStream(uint32_t sessionId, std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    std::unique_lock<std::shared_mutex> lock(switchStreamMapLock_);
    switchStreamMap_[sessionId] = streamDesc;
}

void AudioPipeManager::RemoveSwitchStream(uint32_t sessionId)
{
    std::unique_lock<std::shared_mutex> lock(switchStreamMapLock_);
    switchStreamMap_.erase(sessionId);
}

bool AudioPipeManager::IsSwitchStreamExist(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(switchStreamMapLock_);
    return switchStreamMap_.count(sessionId) > 0;
}

void AudioPipeManager::AddSwitchStreams(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    CHECK_AND_RETURN(!switchStreamMap_.empty());

    std::shared_lock<std::shared_mutex> lock(switchStreamMapLock_);
    std::unordered_set<uint32_t> existingSessionIds;
    existingSessionIds.reserve(streamDescs.size());
    for (auto &streamDesc : streamDescs) {
        existingSessionIds.insert(streamDesc->sessionId_);
    }

    for (auto &[sessionId, streamDesc] : switchStreamMap_) {
        CHECK_AND_CONTINUE(existingSessionIds.find(streamDesc->sessionId_) == existingSessionIds.end());
        streamDescs.push_back(streamDesc);
        existingSessionIds.insert(sessionId);
    }
}

void AudioPipeManager::UpdateNewDeviceDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDesc)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    streamDesc->UpdateNewDeviceWithoutCheck(newDeviceDesc);
}

void AudioPipeManager::UpdateNewDeviceDescWithCheck(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDesc)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    streamDesc->UpdateNewDevice(newDeviceDesc);
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetStreamDescsByStreamUsage(StreamUsage streamUsage)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> result;
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (desc->audioMode_ == AUDIO_MODE_PLAYBACK &&
                desc->rendererInfo_.streamUsage == streamUsage) {
                result.push_back(desc);
            }
        }
    }
    return result;
}

std::vector<std::shared_ptr<AudioStreamDescriptor>> AudioPipeManager::GetStreamDescsBySourceType(SourceType sourceType)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> result;
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (desc->audioMode_ == AUDIO_MODE_RECORD &&
                desc->capturerInfo_.sourceType == sourceType) {
                result.push_back(desc);
            }
        }
    }
    return result;
}

bool AudioPipeManager::IsInterphoneStreamActive()
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputDescs = GetAllOutputStreamDescs();
    for (auto &desc : outputDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
        if (desc->rendererInfo_.streamUsage == STREAM_USAGE_INTERPHONE &&
            desc->streamStatus_ == STREAM_STATUS_STARTED) {
            return true;
        }
    }
    return false;
}

bool AudioPipeManager::IsInterphoneCapturerActive()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (const auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is null");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "stream is null");
            if (stream->capturerInfo_.sourceType == SOURCE_TYPE_INTERPHONE &&
                stream->streamStatus_ == STREAM_STATUS_STARTED) {
                return true;
            }
        }
    }
    return false;
}

bool AudioPipeManager::IsVoipOrCellularStreamActive()
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputDescs = GetAllOutputStreamDescs();
    for (auto &desc : outputDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
        if ((desc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
            desc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
            desc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) &&
            desc->streamStatus_ == STREAM_STATUS_STARTED) {
            return true;
        }
    }
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (const auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is null");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "stream is null");
            if ((stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION ||
                stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_CALL) &&
                stream->streamStatus_ == STREAM_STATUS_STARTED) {
                return true;
            }
        }
    }
    return false;
}

StreamUsage AudioPipeManager::GetLastestRunningCallStreamUsage()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &entry : modemCommunicationIdMap_) {
        CHECK_AND_CONTINUE_LOG(entry.second != nullptr, "streamDesc is nullptr");
        if (entry.second->streamStatus_ == STREAM_STATUS_STARTED) {
            return entry.second->rendererInfo_.streamUsage;
        }
    }

    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (desc->streamStatus_ == STREAM_STATUS_STARTED &&
                (desc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
                desc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
                desc->rendererInfo_.streamUsage == STREAM_USAGE_INTERPHONE)) {
                return desc->rendererInfo_.streamUsage;
            }
        }
    }
    return STREAM_USAGE_UNKNOWN;
}

bool AudioPipeManager::IsOnlyAppLevelHdPlaySupported(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "pipe is nullptr");
    for (auto streamDesc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty(), "invalid streamDesc");
        auto deviceDescIt = std::find_if(streamDesc->newDeviceDescs_.begin(), streamDesc->newDeviceDescs_.end(),
            [](std::shared_ptr<AudioDeviceDescriptor> desc) {
                return desc->GetHdPlaybackMode() == HdPlaybackMode::APP_LEVEL;
            });
        CHECK_AND_RETURN_RET_LOG(deviceDescIt == streamDesc->newDeviceDescs_.end(), true, "only support app level");
    }
    return false;
}

std::vector<std::string> AudioPipeManager::GetNoRunningPipeModuleName()
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::vector<std::string> newList;
    for (auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE(CheckNoRunningCondition(pipe));
        CHECK_AND_CONTINUE(IsPipeNeedStop(pipe));
        newList.push_back(GetModuleName(pipe));
    }
    return newList;
}

// must be called with pipeListLock_ held
bool AudioPipeManager::CheckNoRunningCondition(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "pipe is nullptr");
    if (pipeInfo->IsPipeMatch(ADAPTER_TYPE_REMOTE, AUDIO_OUTPUT_FLAG_NORMAL)) {
        return CheckRemoteNoRunningCondition(pipeInfo);
    }
    return false;
}

// must be called with pipeListLock_ held
bool AudioPipeManager::CheckRemoteNoRunningCondition(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "pipe is nullptr");
    auto remoteOffloadPipe = std::shared_ptr<AudioPipeInfo>();
    for (auto &pipe : curPipeList_) {
        if (pipe->IsPipeMatch(ADAPTER_TYPE_REMOTE, AUDIO_OUTPUT_FLAG_LOWPOWER)) {
            remoteOffloadPipe = pipe;
            break;
        }
    }
    CHECK_AND_RETURN_RET(IsOnlyAppLevelHdPlaySupported(remoteOffloadPipe) && !pipeInfo->HasStream(), false);
    return true;
}

// must be called with pipeListLock_ held
bool AudioPipeManager::IsPipeNeedStop(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "pipe is nullptr");
    if (pipeInfo->isRunning_) {
        pipeInfo->isRunning_ = false;
        return true;
    } else {
        AUDIO_INFO_LOG("skip stop no running pipe");
        return false;
    }
}

std::string AudioPipeManager::GetModuleName(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, "", "pipe is nullptr");
    auto networkId = pipeInfo->moduleInfo_.networkId;
    auto deviceRole = pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT ? OUTPUT_DEVICE : INPUT_DEVICE;
    std::string moduleName = networkId != LOCAL_NETWORK_ID ?
        AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole) : "";
    return moduleName;
}

bool AudioPipeManager::IsValidRenderSessionId(const uint32_t sessionId)
{
    auto outputDescs = GetAllOutputStreamDescs();
    auto it = std::find_if(outputDescs.begin(), outputDescs.end(),
        [sessionId](const std::shared_ptr<AudioStreamDescriptor> &desc) {
            return desc != nullptr && desc->sessionId_ == sessionId;
        });
    return it != outputDescs.end();
}

AudioVolumeType AudioPipeManager::GetVolumeTypeFromStreamDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_ASSISTANT &&
        !CheckoutSystemAppUtil::CheckoutSystemApp(AudioActiveDevice::GetInstance().GetRealUid(streamDesc))) {
        return STREAM_MUSIC;
    }
    return VolumeUtils::GetVolumeTypeFromStreamUsage(streamDesc->rendererInfo_.streamUsage);
}

void AudioPipeManager::AddOrUpdateVolumeInfo(std::vector<PipeDeviceVolumeInfo>& result,
    AudioIOHandle ioHandle, std::shared_ptr<AudioDeviceDescriptor> deviceDesc, AudioVolumeType volumeType)
{
    for (auto &info : result) {
        if (info.ioHandle_ == ioHandle && info.deviceDesc_->IsSameDeviceDesc(*deviceDesc)) {
            info.volumeTypes_.insert(volumeType);
            return;
        }
    }
    PipeDeviceVolumeInfo newInfo(ioHandle, deviceDesc);
    newInfo.volumeTypes_.insert(volumeType);
    result.push_back(newInfo);
}

std::vector<PipeDeviceVolumeInfo> AudioPipeManager::ProcessPipeForVolumeInfo(
    std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    std::vector<PipeDeviceVolumeInfo> pipeDeviceVolumeInfo;
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, pipeDeviceVolumeInfo, "pipeInfo is null");
    CHECK_AND_RETURN_RET_LOG(!pipeInfo->streamDescriptors_.empty(), pipeDeviceVolumeInfo,
        "pipe has no stream descriptors");

    for (auto &streamDesc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE(streamDesc != nullptr);
        CHECK_AND_CONTINUE(!streamDesc->newDeviceDescs_.empty());

        auto firstDevice = streamDesc->newDeviceDescs_.front();
        CHECK_AND_CONTINUE(firstDevice != nullptr);
        AudioVolumeType volumeType = GetVolumeTypeFromStreamDesc(streamDesc);
        AddOrUpdateVolumeInfo(pipeDeviceVolumeInfo, pipeInfo->id_, firstDevice, volumeType);
        AUDIO_INFO_LOG("pipeName:%{public}s, ioHandle:%{public}u, sessionId:%{public}u, device:%{public}s",
            pipeInfo->name_.c_str(), pipeInfo->id_, streamDesc->sessionId_, firstDevice->GetName().c_str());

        for (size_t i = 1; i < streamDesc->newDeviceDescs_.size(); i++) {
            auto device = streamDesc->newDeviceDescs_[i];
            CHECK_AND_CONTINUE(device != nullptr);

            std::string sinkName = AudioPolicyUtils::GetInstance()
                .GetSinkName(device, streamDesc->sessionId_);
            CHECK_AND_CONTINUE(!sinkName.empty());

            AudioIOHandle ioHandle = OPEN_PORT_FAILURE;
            CHECK_AND_CONTINUE_LOG(AudioIOHandleMap::GetInstance().GetModuleIdByKey(sinkName, ioHandle),
                "can not find %{public}s in io map", sinkName.c_str());
            AddOrUpdateVolumeInfo(pipeDeviceVolumeInfo, ioHandle, device, volumeType);
            AUDIO_INFO_LOG("sinkName:%{public}s, ioHandle:%{public}u, sessionId:%{public}u, device:%{public}s",
                sinkName.c_str(), ioHandle, streamDesc->sessionId_, device->GetName().c_str());
        }
    }

    return pipeDeviceVolumeInfo;
}

std::vector<PipeDeviceVolumeInfo> AudioPipeManager::GetAllPipeDeviceVolumeInfo()
{
    std::vector<PipeDeviceVolumeInfo> pipeDeviceVolumeInfo;

    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE(pipeInfo != nullptr);
        CHECK_AND_CONTINUE(pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT);
        auto info = ProcessPipeForVolumeInfo(pipeInfo);
        pipeDeviceVolumeInfo.insert(pipeDeviceVolumeInfo.end(), std::make_move_iterator(info.begin()),
            std::make_move_iterator(info.end()));
    }

    return pipeDeviceVolumeInfo;
}

std::vector<PipeDeviceVolumeInfo> AudioPipeManager::GetPipeDeviceVolumeInfoForPipe(AudioIOHandle id)
{
    std::vector<PipeDeviceVolumeInfo> pipeDeviceVolumeInfo;

    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE(pipeInfo != nullptr);
        CHECK_AND_CONTINUE(pipeInfo->id_ == id);
        CHECK_AND_CONTINUE(pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT);

        return ProcessPipeForVolumeInfo(pipeInfo);
    }

    return pipeDeviceVolumeInfo;
}

std::vector<PipeDeviceVolumeInfo> AudioPipeManager::GetPipeDeviceVolumeInfoForDevice(
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, {}, "deviceDesc is null");

    std::vector<PipeDeviceVolumeInfo> pipeDeviceVolumeInfo;

    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    for (auto &pipeInfo : curPipeList_) {
        std::shared_ptr<AudioPipeInfo> targetPipe = nullptr;
        CHECK_AND_CONTINUE(pipeInfo != nullptr);
        CHECK_AND_CONTINUE(pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT);
        AUDIO_INFO_LOG("pipeId:%{public}d", pipeInfo->id_);
        CHECK_AND_CONTINUE(!pipeInfo->streamDescriptors_.empty());

        for (auto &streamDesc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE(streamDesc != nullptr);
            AUDIO_INFO_LOG("sessionId:%{public}u", streamDesc->sessionId_);
            CHECK_AND_CONTINUE(!streamDesc->newDeviceDescs_.empty());

            auto firstDevice = streamDesc->newDeviceDescs_.front();
            CHECK_AND_CONTINUE(firstDevice != nullptr);

            if (firstDevice->IsSameDeviceDescPtr(deviceDesc)) {
                AUDIO_INFO_LOG("pipeId:%{public}d, sessionId:%{public}d", pipeInfo->id_, streamDesc->sessionId_);
                targetPipe = pipeInfo;
                break;
            }
        }

        if (targetPipe != nullptr) {
            auto info = ProcessPipeForVolumeInfo(targetPipe);
            pipeDeviceVolumeInfo.insert(pipeDeviceVolumeInfo.end(), std::make_move_iterator(info.begin()),
                std::make_move_iterator(info.end()));
        }
    }

    return pipeDeviceVolumeInfo;
}

bool AudioPipeManager::HasPausedFastPipeInDelayedSwitch()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_lock<std::shared_mutex> sLock(switchStreamMapLock_);

    if (switchStreamMap_.empty()) {
        return false;
    }

    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (!pipeInfo->IsRouteFast()) {
            continue;
        }

        bool hasDelayedSwitchStream = false;
        bool allStreamsPaused = true;

        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (switchStreamMap_.count(desc->sessionId_) > 0) {
                hasDelayedSwitchStream = true;
            }
            if (desc->streamStatus_ == STREAM_STATUS_STARTED) {
                allStreamsPaused = false;
            }
        }

        if (hasDelayedSwitchStream && allStreamsPaused && !pipeInfo->streamDescriptors_.empty()) {
            AUDIO_INFO_LOG("Found paused fast pipe in delayed switch, pipe: %{public}s",
                pipeInfo->name_.c_str());
            return true;
        }
    }
    return false;
}

std::string AudioPipeManager::GetPausedFastPipeModuleNameInDelayedSwitch()
{
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_lock<std::shared_mutex> sLock(switchStreamMapLock_);

    if (switchStreamMap_.empty()) {
        return "";
    }

    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        if (!pipeInfo->IsRouteFast()) {
            continue;
        }

        bool hasDelayedSwitchStream = false;
        bool allStreamsPaused = true;

        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is nullptr");
            if (switchStreamMap_.count(desc->sessionId_) > 0) {
                hasDelayedSwitchStream = true;
            }
            if (desc->streamStatus_ == STREAM_STATUS_STARTED) {
                allStreamsPaused = false;
            }
        }

        if (hasDelayedSwitchStream && allStreamsPaused && !pipeInfo->streamDescriptors_.empty()) {
            AUDIO_INFO_LOG("Get paused fast pipe module name in delayed switch: %{public}s",
                pipeInfo->moduleInfo_.name.c_str());
            return pipeInfo->moduleInfo_.name;
        }
    }
    return "";
}
} // namespace AudioStandard
} // namespace OHOS
