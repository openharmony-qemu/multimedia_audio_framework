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
#define LOG_TAG "AudioPipeManager"
#endif

#include "audio_pipe_manager.h"

namespace OHOS {
namespace AudioStandard {

const uint32_t FIRST_SESSIONID = 100000;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
AudioPipeManager::AudioPipeManager()
{
}

AudioPipeManager::~AudioPipeManager()
{
    AUDIO_INFO_LOG("Dtor");
    curPipeList_.clear();
}

void AudioPipeManager::AddAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info)
{
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
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end(); iter++) {
        if (IsSamePipe(info, *iter)) {
            AUDIO_INFO_LOG("Remove id:%{public}u, name %{public}s", info->id_, info->name_.c_str());
            curPipeList_.erase(iter);
            break;
        }
    }
}

void AudioPipeManager::RemoveAudioPipeInfo(AudioIOHandle id)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end(); iter++) {
        if ((*iter)->id_ == id) {
            AUDIO_INFO_LOG("Remove id:%{public}u, name: %{public}s", id, (*iter)->name_.c_str());
            curPipeList_.erase(iter);
            break;
        }
    }
}

void AudioPipeManager::UpdateAudioPipeInfo(std::shared_ptr<AudioPipeInfo> newPipe)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto iter = curPipeList_.begin(); iter != curPipeList_.end(); iter++) {
        if (IsSamePipe(newPipe, *iter)) {
            AUDIO_INFO_LOG("Update id:%{public}u, name %{public}s", (*iter)->id_, (*iter)->name_.c_str());
            Assign(*iter, newPipe);
            // Action is only used in pipe execution, while pipeManager can only store default action
            (*iter)->pipeAction_ = PIPE_ACTION_DEFAULT;
            break;
        }
    }
}

bool AudioPipeManager::IsSamePipe(std::shared_ptr<AudioPipeInfo> info, std::shared_ptr<AudioPipeInfo> cmpInfo)
{
    if ((info->adapterName_ == cmpInfo->adapterName_ && info->routeFlag_ == cmpInfo->routeFlag_) ||
        info->id_ == cmpInfo->id_) {
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
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
}

void AudioPipeManager::PauseClient(uint32_t sessionId)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->streamStatus_ = STREAM_STATUS_PAUSED;
}

void AudioPipeManager::StopClient(uint32_t sessionId)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescByIdInner(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "StreamDesc is nullptr");
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
}

void AudioPipeManager::RemoveClient(uint32_t sessionId)
{
    std::unique_lock<std::shared_mutex> pLock(pipeListLock_);
    AUDIO_INFO_LOG("Cur pipe list size %{public}zu, sessionId %{public}u", curPipeList_.size(), sessionId);
    for (auto pipeInfo : curPipeList_) {
        pipeInfo->streamDescriptors_.erase(std::remove_if(pipeInfo->streamDescriptors_.begin(),
            pipeInfo->streamDescriptors_.end(), [sessionId](std::shared_ptr<AudioStreamDescriptor> streamDesc) {
                return streamDesc->sessionId_ == sessionId;
            }), pipeInfo->streamDescriptors_.end());
    }
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
        if (pipe->streamDescriptors_.empty() && IsSpecialPipe(pipe->routeFlag_)) {
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
    AUDIO_INFO_LOG("Flag %{public}d", routeFlag);
    if ((routeFlag & AUDIO_OUTPUT_FLAG_FAST) ||
        (routeFlag & AUDIO_INPUT_FLAG_FAST) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD)) {
        return true;
    }
    return false;
}

bool AudioPipeManager::IsNormalRecordPipe(std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "Pipe info is null");
    if ((pipeInfo->adapterName_ == PRIMARY_CLASS && pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL) ||
        (pipeInfo->adapterName_ == USB_CLASS && pipeInfo->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL)) {
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
    AUDIO_INFO_LOG("Cur Pipe list size %{public}zu, sessionId %{public}u", curPipeList_.size(), sessionId);
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

std::shared_ptr<AudioDeviceDescriptor> AudioPipeManager::GetProcessDeviceInfoBySessionId(
    uint32_t sessionId, AudioStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("Cur pipe list size %{public}zu, sessionId %{public}u", curPipeList_.size(), sessionId);
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipeInfo : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
        for (auto &desc : pipeInfo->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr && desc->newDeviceDescs_.size() > 0 &&
                desc->newDeviceDescs_.front() != nullptr, "desc is nullptr");
            if (desc->sessionId_ == sessionId) {
                AUDIO_INFO_LOG("Device type: %{public}d", desc->newDeviceDescs_.front()->deviceType_);
                streamInfo = pipeInfo->audioStreamInfo_;
                return desc->newDeviceDescs_.front();
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
    AUDIO_INFO_LOG("Cur pipe list size %{public}zu, sessionId %{public}u", curPipeList_.size(), sessionId);
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
        tempList.push_back(pipe);
    }
    curPipeList_.clear();
    curPipeList_ = tempList;
}

void AudioPipeManager::UpdateCapturerPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos)
{
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
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);

    dumpString += "Audio PipeManager Infos\n";
    dumpString += "  - TotalPipeNums: " + std::to_string(curPipeList_.size()) + "\n\n";

    for (auto &pipe : curPipeList_) {
        if (pipe != nullptr) {
            pipe->Dump(dumpString);
        }
    }
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
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &entry : modemCommunicationIdMap_) {
        CHECK_AND_CONTINUE_LOG(entry.second != nullptr, "StreamDesc is nullptr");
        entry.second->oldDeviceDescs_ = entry.second->newDeviceDescs_;
        entry.second->newDeviceDescs_ = deviceDescs;
    }
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
        pipeInfo = GetPipeByModuleAndFlag(BLUETOOTH_MIC, AUDIO_INPUT_FLAG_NORMAL);
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

std::vector<uint32_t> AudioPipeManager::GetStreamIdsByUid(uint32_t uid, uint32_t routeFlagMask)
{
    std::vector<uint32_t> sessionIds = {};
    std::shared_lock<std::shared_mutex> pLock(pipeListLock_);
    for (auto &pipe : curPipeList_) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "pipe is nullptr");
        CHECK_AND_CONTINUE_LOG(pipe->routeFlag_ & routeFlagMask, "not match flag: %{public}u", pipe->routeFlag_);
        for (auto &streamDesc : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "streamDesc is nullptr");
            if (streamDesc->callerUid_ == static_cast<int32_t>(uid)) {
                sessionIds.push_back(streamDesc->sessionId_);
            }
        }
    }
    AUDIO_INFO_LOG("Session number of uid %{public}u: %{public}zu", uid, sessionIds.size());
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
} // namespace AudioStandard
} // namespace OHOS
