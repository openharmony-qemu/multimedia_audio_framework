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

#ifndef ST_PRO_AUDIO_SERVICE_ADAPTER_IMPL_H
#define ST_PRO_AUDIO_SERVICE_ADAPTER_IMPL_H
#ifndef LOG_TAG
#define LOG_TAG "ProAudioServiceAdapterImpl"
#endif

#include "pro_audio_service_adapter_impl.h"
#include <sstream>
#include <thread>

#include "audio_errors.h"
#include "audio_engine_log.h"
#include "audio_info.h"
#include "audio_utils.h"
#include <set>
#include <unordered_map>
#include "i_hpae_manager.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {
static unique_ptr<AudioServiceAdapterCallback> g_audioServiceAdapterCallback;
static const int32_t OPERATION_TIMEOUT_IN_MS = 10000;  // 10s
static const int32_t HPAE_SERVICE_IMPL_TIMEOUT = 10; // 10s is better

ProAudioServiceAdapterImpl::~ProAudioServiceAdapterImpl() = default;

ProAudioServiceAdapterImpl::ProAudioServiceAdapterImpl(unique_ptr<AudioServiceAdapterCallback> &cb)
{
    g_audioServiceAdapterCallback = move(cb);
}

bool ProAudioServiceAdapterImpl::Connect()
{
    AUDIO_INFO_LOG("Enter");
    IHpaeManager::GetHpaeManager().RegisterSerivceCallback(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(g_audioServiceAdapterCallback != nullptr, false,
        "g_audioServiceAdapterCallback is nullptr");
    return true;
}

uint32_t ProAudioServiceAdapterImpl::OpenAudioPort(string audioPortName, string moduleArgs)
{
    AUDIO_PRERELEASE_LOGI("ERROR Enter.");
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::ReloadAudioPort(const std::string &audioPortName,
    const AudioModuleInfo &audioModuleInfo)
{
    AUDIO_PRERELEASE_LOGI("Enter");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::ReloadAudioPort", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] ReloadAudioPort timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("ReloadAudioPort");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishReloadAudioPort_ = false;
    IHpaeManager::GetHpaeManager().ReloadAudioPort(audioModuleInfo);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishReloadAudioPort_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("Leave");
    return AudioPortIndex_;
}

int32_t ProAudioServiceAdapterImpl::OpenAudioPort(string audioPortName, const AudioModuleInfo &audioModuleInfo)
{
    AUDIO_PRERELEASE_LOGI("Enter.");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::OpenAudioPort", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] OpenAudioPort timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("OpenAudioPort");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishOpenAudioPort_ = false;
    IHpaeManager::GetHpaeManager().OpenAudioPort(audioModuleInfo);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishOpenAudioPort_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("Leave");
    return AudioPortIndex_;
}

int32_t ProAudioServiceAdapterImpl::CloseAudioPort(int32_t audioHandleIndex)
{
    if (audioHandleIndex <= 0) {
        AUDIO_ERR_LOG("Core modules, not allowed to close!");
        return ERROR;
    }
    AUDIO_INFO_LOG("AudioHandleIndex:%{public}d", audioHandleIndex);
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::CloseAudioPort", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] CloseAudioPort timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("CloseAudioPort");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishCloseAudioPort_ = false;
    IHpaeManager::GetHpaeManager().CloseAudioPort(audioHandleIndex);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCloseAudioPort_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("Leave");
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SuspendAudioDevice(string &audioPortName, bool isSuspend)
{
    Trace trace("SuspendAudioDevice");
    lock_guard<mutex> lock(lock_);
    AUDIO_INFO_LOG("[%{public}s] : [%{public}d]", audioPortName.c_str(), isSuspend);
    IHpaeManager::GetHpaeManager().SuspendAudioDevice(audioPortName, isSuspend);
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::StopAudioPort(const std::string &audioPortName)
{
    Trace trace("StopAudioPort");
    lock_guard<mutex> lock(lock_);
    AUDIO_INFO_LOG("StopAudioPort [%{public}s]", audioPortName.c_str());
    IHpaeManager::GetHpaeManager().StopAudioPort(audioPortName);
    return SUCCESS;
}

bool ProAudioServiceAdapterImpl::SetSinkMute(const std::string &sinkName, bool isMute, bool isSync)
{
    AUDIO_INFO_LOG("[%{public}s] : [%{public}d] isSync [%{public}d]", sinkName.c_str(), isMute, isSync);
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::SetSinkMute", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] SetSinkMute timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("SetSinkMute:" + sinkName + "isMute:" + std::to_string(isMute));
    lock_guard<mutex> lock(lock_);
    if (isSync) {
        std::unique_lock<std::mutex> waitLock(callbackMutex_);
        isFinishSetSinkMute_ = false;
        IHpaeManager::GetHpaeManager().SetSinkMute(sinkName, isMute, isSync);
        bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
            return isFinishSetSinkMute_;  // will be true when got notified.
        });
        if (!stopWaiting) {
            AUDIO_ERR_LOG("Timeout");
            return ERROR;
        }
    } else {
        IHpaeManager::GetHpaeManager().SetSinkMute(sinkName, isMute, isSync);
    }
    AUDIO_INFO_LOG("Leave");
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetDefaultSink(string name)
{
    Trace trace("SetDefaultSink:" + name);
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().SetDefaultSink(name);
    AUDIO_INFO_LOG("[%{public}s]", name.c_str());
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetDefaultSource(string name)
{
    Trace trace("SetDefaultSource:" + name);
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().SetDefaultSource(name);
    AUDIO_INFO_LOG("[%{public}s]", GetEncryptStr(name).c_str());
    return SUCCESS;
}

std::vector<SinkInfo> ProAudioServiceAdapterImpl::GetAllSinks()
{
    AUDIO_INFO_LOG("Enter");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::GetAllSinks", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetAllSinks timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("GetAllSinks");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetAllSinks_ = false;
    IHpaeManager::GetHpaeManager().GetAllSinks();
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAllSinks_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        sinks_.clear();
    }
    AUDIO_INFO_LOG("Leave");
    return sinks_;
}

std::vector<uint32_t> ProAudioServiceAdapterImpl::GetTargetSinks(std::string adapterName)
{
    Trace trace("GetTargetSinks:" + adapterName);
    std::vector<SinkInfo> sinkInfos = GetAllSinks();
    std::vector<uint32_t> targetSinkIds = {};
    for (size_t i = 0; i < sinkInfos.size(); i++) {
        if (sinkInfos[i].adapterName == adapterName) {
            targetSinkIds.push_back(sinkInfos[i].sinkId);
        }
    }
    AUDIO_INFO_LOG("AdapterName %{public}s", adapterName.c_str());
    return targetSinkIds;
}

int32_t ProAudioServiceAdapterImpl::SetLocalDefaultSink(std::string name)
{
    AUDIO_INFO_LOG("Sink name: %{public}s", name.c_str());
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::MoveSinkInputByIndexOrName(
    uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName)
{
    AUDIO_INFO_LOG("sinkInputId %{public}d, sinkIndex %{public}d, sinkName %{public}s",
        sinkInputId, sinkIndex, GetEncryptStr(sinkName).c_str());
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::MoveSinkInputByIndexOrName", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] MoveSinkInputByIndexOrName timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("MoveSinkInputByIndexOrName: " + std::to_string(sinkInputId) + " index:" + std::to_string(sinkIndex) +
                " sink:" + sinkName);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishMoveSinkInputByIndexOrName_ = false;
    IHpaeManager::GetHpaeManager().MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishMoveSinkInputByIndexOrName_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName(
    uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName)
{
    AUDIO_INFO_LOG(
        "SourceOutputId %{public}d, sourceIndex %{public}d, sourceName %{public}s",
        sourceOutputId,
        sourceIndex,
        sourceName.c_str());
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] MoveSourceOutputByIndexOrName timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("MoveSourceOutputByIndexOrName: " + std::to_string(sourceOutputId) +
                " index:" + std::to_string(sourceIndex) + " source:" + sourceName);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishMoveSourceOutputByIndexOrName_ = false;
    IHpaeManager::GetHpaeManager().MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishMoveSourceOutputByIndexOrName_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetSourceOutputMute(int32_t uid, bool setMute)
{
    AUDIO_INFO_LOG("Uid %{public}d, setMute %{public}d", uid, setMute);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetSourceOutputMute_ = false;
    SourceOutputMuteStreamSet_ = 0;
    IHpaeManager::GetHpaeManager().SetSourceOutputMute(uid, setMute);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetSourceOutputMute_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("Leave");
    return SourceOutputMuteStreamSet_;
}

int32_t ProAudioServiceAdapterImpl::SetSourceOutputStreamMuteByStreamId(int32_t sessionId, bool setMute)
{
    AUDIO_INFO_LOG("sessionId %{public}d, setMute %{public}d", sessionId, setMute);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetSourceOutputMute_ = false;
    SourceOutputMuteStreamSet_ = 0;
    IHpaeManager::GetHpaeManager().SetSourceOutputMuteByStreamId(sessionId, setMute);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetSourceOutputMute_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("Leave");
    return SourceOutputMuteStreamSet_;
}

std::vector<SinkInput> ProAudioServiceAdapterImpl::GetAllSinkInputs()
{
    AUDIO_INFO_LOG("Enter");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::GetAllSinkInputs", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetAllSinkInputs timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetAllSinkInputs_ = false;
    IHpaeManager::GetHpaeManager().GetAllSinkInputs();
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAllSinkInputs_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        sinkInputs_.clear();
    }
    AUDIO_INFO_LOG("Leave");
    return sinkInputs_;
}

std::vector<SourceOutput> ProAudioServiceAdapterImpl::GetAllSourceOutputs()
{
    AUDIO_INFO_LOG("Enter");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::GetAllSourceOutputs", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetAllSourceOutputs timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetAllSourceOutputs_ = false;
    IHpaeManager::GetHpaeManager().GetAllSourceOutputs();
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAllSourceOutputs_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Timeout");
        sourceOutputs_.clear();
    }
    AUDIO_INFO_LOG("Leave");
    return sourceOutputs_;
}

void ProAudioServiceAdapterImpl::Disconnect()
{
    AUDIO_INFO_LOG("Disconnect not support");
}

int32_t ProAudioServiceAdapterImpl::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    AUDIO_INFO_LOG("Enter");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetAudioEffectProperty_ = false;
    IHpaeManager::GetHpaeManager().GetAudioEffectProperty(propertyArray);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAudioEffectProperty_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::GetAudioEnhanceProperty(AudioEffectPropertyArray &propertyArray,
    DeviceType deviceType)
{
    AUDIO_INFO_LOG("Enter");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetAudioEnhanceProperty_ = false;
    IHpaeManager::GetHpaeManager().GetAudioEnhanceProperty(propertyArray);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAudioEnhanceProperty_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    AUDIO_INFO_LOG("Enter");
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishUpdateSpatializationState_ = false;
    IHpaeManager::GetHpaeManager().UpdateSpatializationState(spatializationState);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishUpdateSpatializationState_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting == true, SUCCESS, "wait for notify timeout");
    return SUCCESS;
}

void ProAudioServiceAdapterImpl::OnReloadAudioPortCb(int32_t portId)
{
    AUDIO_INFO_LOG("PortId: %{public}d", portId);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishReloadAudioPort_= true;
    AudioPortIndex_ = portId;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnOpenAudioPortCb(int32_t portId)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("PortId: %{public}d", portId);
    isFinishOpenAudioPort_ = true;
    AudioPortIndex_ = portId;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnCloseAudioPortCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishCloseAudioPort_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnSetSinkMuteCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishSetSinkMute_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishGetAllSinkInputs_ = true;
    sinkInputs_ = sinkInputs;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnSetSourceOutputMuteCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishSetSourceOutputMute_ = true;
    SourceOutputMuteStreamSet_ = result;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishGetAllSourceOutputs_ = true;
    sourceOutputs_ = sourceOutputs;
    callbackCV_.notify_all();
}
void ProAudioServiceAdapterImpl::OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishGetAllSinks_ = true;
    sinks_ = sinks;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnMoveSinkInputByIndexOrNameCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishMoveSinkInputByIndexOrName_ = true;
    callbackCV_.notify_all();
}
void ProAudioServiceAdapterImpl::OnMoveSourceOutputByIndexOrNameCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishMoveSourceOutputByIndexOrName_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAudioEffectPropertyCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishGetAudioEffectProperty_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAudioEnhancePropertyCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishGetAudioEnhanceProperty_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnUpdateSpatializationStateCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishUpdateSpatializationState_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::HandleSourceAudioStreamRemoved(uint32_t sessionId)
{
    // todo: code check
    CHECK_AND_RETURN_LOG(g_audioServiceAdapterCallback != nullptr, "g_audioServiceAdapterCallback is nullptr");
    g_audioServiceAdapterCallback->OnAudioStreamRemoved(sessionId);
}

int32_t ProAudioServiceAdapterImpl::UpdateCollaborativeState(bool isCollaborationEnabled)
{
    AUDIO_INFO_LOG("State %{public}d", isCollaborationEnabled);
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().UpdateCollaborativeState(isCollaborationEnabled);
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetAbsVolumeStateToEffect(const bool absVolumeState)
{
    AUDIO_INFO_LOG("State %{public}d", absVolumeState);
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().SetAbsVolumeStateToEffect(absVolumeState);
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetSystemVolumeToEffect(AudioStreamType streamType, float volume)
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().SetEffectSystemVolume(streamType, volume);
    return SUCCESS;
}

bool ProAudioServiceAdapterImpl::IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout)
{
    lock_guard<mutex> lock(lock_);
    return IHpaeManager::GetHpaeManager().IsChannelLayoutSupportedForDspEffect(channelLayout);
}

void ProAudioServiceAdapterImpl::AddCaptureInjector(const uint32_t &sinkPortIndex,
    const uint32_t &sourcePortIndex, const SourceType &sourceType)
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().AddCaptureInjector(sinkPortIndex, sourcePortIndex, sourceType);
}

void ProAudioServiceAdapterImpl::RemoveCaptureInjector(const uint32_t &sinkPortIndex,
    const uint32_t &sourcePortIndex, const SourceType &sourceType)
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().RemoveCaptureInjector(sinkPortIndex, sourcePortIndex, sourceType);
}

void ProAudioServiceAdapterImpl::EnableCaptureCollaboration(uint32_t sourcePortIndex,
    uint32_t collabSourcePortIndex, SourceType sourceType)
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().EnableCaptureCollaboration(sourcePortIndex, collabSourcePortIndex,
        sourceType);
}

void ProAudioServiceAdapterImpl::DisableCaptureCollaboration(uint32_t sourcePortIndex,
    uint32_t collabSourcePortIndex, SourceType sourceType)
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().DisableCaptureCollaboration(sourcePortIndex, collabSourcePortIndex,
        sourceType);
}

void ProAudioServiceAdapterImpl::UpdateAudioPortInfo(const uint32_t &sinkPortIndex,
    const AudioModuleInfo &audioPortInfo)
{
    AUDIO_PRERELEASE_LOGI("Injector::Enter UpdateAudioPortInfo.");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::UpdateAudioPortInfo", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] Timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishOpenAudioPort_ = false;
    IHpaeManager::GetHpaeManager().UpdateAudioPortInfo(sinkPortIndex, audioPortInfo);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishOpenAudioPort_;  // will be true when got notified.
    });
    CHECK_AND_RETURN_LOG(stopWaiting, "TimeOut");
    AUDIO_INFO_LOG("Injector::UpdateAudioPortInfo finish.");
}

int32_t ProAudioServiceAdapterImpl::UpdateCollaborativeProductId(const std::string &productId)
{
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    IHpaeManager::GetHpaeManager().UpdateCollaborativeProductId(productId);
    collaborativeUpdateResult_ = ERROR;
    isFinishCollaborativeUpdate_ = false;
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCollaborativeUpdate_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
        return ERROR;
    }
    return collaborativeUpdateResult_;
}

int32_t ProAudioServiceAdapterImpl::UpdatePersonalizedHRTFBin(const int32_t &fd, const long &length)
{
    lock_guard<mutex> lock(lock_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    IHpaeManager::GetHpaeManager().UpdatePersonalizedHRTFBin(fd, length);
    personalizedHRTFBinUpdateResult_ = ERROR;
    isFinishPersonalizedHRTFBinUpdate_ = false;
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishPersonalizedHRTFBinUpdate_;
    });

    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_SAVE_HRTF_TIMEOUT, "TimeOut");
    return personalizedHRTFBinUpdateResult_;
}

void ProAudioServiceAdapterImpl::LoadCollaborationConfig()
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().LoadCollaborationConfig();
}

void ProAudioServiceAdapterImpl::OnGetCollaborativeUpdateCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishCollaborativeUpdate_ = true;
    collaborativeUpdateResult_ = result;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnUpdatePersonalizedHRTFBinCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("Result: %{public}d", result);
    isFinishPersonalizedHRTFBinUpdate_ = true;
    personalizedHRTFBinUpdateResult_ = result;
    callbackCV_.notify_all();
}
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
