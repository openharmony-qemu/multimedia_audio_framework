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
#include "audio_common_log.h"
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
    AUDIO_INFO_LOG("Connected RegiesterServiceCallback");
    IHpaeManager::GetHpaeManager().RegisterSerivceCallback(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(g_audioServiceAdapterCallback != nullptr, false,
        "g_audioServiceAdapterCallback is nullptr");
    g_audioServiceAdapterCallback->OnSetVolumeDbCb();
    return true;
}

uint32_t ProAudioServiceAdapterImpl::OpenAudioPort(string audioPortName, string moduleArgs)
{
    AUDIO_PRERELEASE_LOGI("ERROR OpenAudioPort enter.");
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::ReloadAudioPort(const std::string &audioPortName,
    const AudioModuleInfo &audioModuleInfo)
{
    AUDIO_PRERELEASE_LOGI("ReloadAudioPort enter.");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::ReloadAudioPort", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] ReloadAudioPort timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("ReloadAudioPort");
    lock_guard<mutex> lock(lock_);
    isFinishReloadAudioPort_ = false;
    IHpaeManager::GetHpaeManager().ReloadAudioPort(audioModuleInfo);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishReloadAudioPort_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("ReloadAudioPort timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("ReloadAudioPort leave");
    return AudioPortIndex_;
}

int32_t ProAudioServiceAdapterImpl::OpenAudioPort(string audioPortName, const AudioModuleInfo &audioModuleInfo)
{
    AUDIO_PRERELEASE_LOGI("OpenAudioPort enter.");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::OpenAudioPort", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] OpenAudioPort timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("OpenAudioPort");
    isFinishOpenAudioPort_ = false;
    IHpaeManager::GetHpaeManager().OpenAudioPort(audioModuleInfo);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishOpenAudioPort_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("OpenAudioPort timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("OpenAudioPort leave");
    return AudioPortIndex_;
}

int32_t ProAudioServiceAdapterImpl::CloseAudioPort(int32_t audioHandleIndex)
{
    if (audioHandleIndex <= 0) {
        AUDIO_ERR_LOG("Core modules, not allowed to close!");
        return ERROR;
    }
    AUDIO_INFO_LOG("CloseAudioPort:audioHandleIndex:%{public}d", audioHandleIndex);
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::CloseAudioPort", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] CloseAudioPort timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace("CloseAudioPort");
    lock_guard<mutex> lock(lock_);

    isFinishCloseAudioPort_ = false;
    IHpaeManager::GetHpaeManager().CloseAudioPort(audioHandleIndex);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCloseAudioPort_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("CloseAudioPort timeout");
        return ERROR;
    }

    AUDIO_INFO_LOG("CloseAudioPort leave");
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SuspendAudioDevice(string &audioPortName, bool isSuspend)
{
    lock_guard<mutex> lock(lock_);
    Trace trace("SuspendAudioDevice");
    AUDIO_INFO_LOG("SuspendAudioDevice [%{public}s] : [%{public}d]", audioPortName.c_str(), isSuspend);
    IHpaeManager::GetHpaeManager().SuspendAudioDevice(audioPortName, isSuspend);
    return SUCCESS;
}

bool ProAudioServiceAdapterImpl::SetSinkMute(const std::string &sinkName, bool isMute, bool isSync)
{
    AUDIO_INFO_LOG("SetSinkMute: [%{public}s] : [%{public}d] isSync [%{public}d]", sinkName.c_str(), isMute, isSync);
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::SetSinkMute", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] SetSinkMute timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("SetSinkMute:" + sinkName + "isMute:" + std::to_string(isMute));
    if (isSync) {
        isFinishSetSinkMute_ = false;
        IHpaeManager::GetHpaeManager().SetSinkMute(sinkName, isMute, isSync);
        std::unique_lock<std::mutex> waitLock(callbackMutex_);
        bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
            return isFinishSetSinkMute_;  // will be true when got notified.
        });
        if (!stopWaiting) {
            AUDIO_ERR_LOG("SetSinkMute timeout");
            return ERROR;
        }
    } else {
        IHpaeManager::GetHpaeManager().SetSinkMute(sinkName, isMute, isSync);
    }
    AUDIO_INFO_LOG("SetSinkMute leave");
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetDefaultSink(string name)
{
    lock_guard<mutex> lock(lock_);
    Trace trace("SetDefaultSink:" + name);
    IHpaeManager::GetHpaeManager().SetDefaultSink(name);
    AUDIO_INFO_LOG("SetDefaultSink: [%{public}s]", name.c_str());
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetDefaultSource(string name)
{
    lock_guard<mutex> lock(lock_);
    Trace trace("SetDefaultSource:" + name);
    IHpaeManager::GetHpaeManager().SetDefaultSource(name);
    AUDIO_INFO_LOG("SetDefaultSource: [%{public}s]", name.c_str());
    return SUCCESS;
}

std::vector<SinkInfo> ProAudioServiceAdapterImpl::GetAllSinks()
{
    AUDIO_INFO_LOG("GetAllSinks: enter");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::GetAllSinks", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetAllSinks timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("GetAllSinks");
    isFinishGetAllSinks_ = false;
    IHpaeManager::GetHpaeManager().GetAllSinks();
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAllSinks_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("GetAllSinks timeout");
        sinks_.clear();
    }
    AUDIO_INFO_LOG("GetAllSinks leave");
    return sinks_;
}

std::vector<uint32_t> ProAudioServiceAdapterImpl::GetTargetSinks(std::string adapterName)
{
    std::vector<SinkInfo> sinkInfos = GetAllSinks();
    Trace trace("GetTargetSinks:" + adapterName);
    std::vector<uint32_t> targetSinkIds = {};
    for (size_t i = 0; i < sinkInfos.size(); i++) {
        if (sinkInfos[i].adapterName == adapterName) {
            targetSinkIds.push_back(sinkInfos[i].sinkId);
        }
    }
    AUDIO_INFO_LOG("GetTargetSinks: adapterName %{public}s", adapterName.c_str());
    return targetSinkIds;
}

int32_t ProAudioServiceAdapterImpl::SetLocalDefaultSink(std::string name)
{
    AUDIO_INFO_LOG("SetLocalDefaultSink sink name: %{public}s", name.c_str());
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::MoveSinkInputByIndexOrName(
    uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName)
{
    AUDIO_INFO_LOG("sinkInputId %{public}d, sinkIndex %{public}d, sinkName %{public}s",
        sinkInputId,
        sinkIndex,
        sinkName.c_str());
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::MoveSinkInputByIndexOrName", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] MoveSinkInputByIndexOrName timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("MoveSinkInputByIndexOrName: " + std::to_string(sinkInputId) + " index:" + std::to_string(sinkIndex) +
                " sink:" + sinkName);
    isFinishMoveSinkInputByIndexOrName_ = false;
    IHpaeManager::GetHpaeManager().MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishMoveSinkInputByIndexOrName_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("MoveSinkInputByIndexOrName timeout");
        return ERROR;
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName(
    uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName)
{
    AUDIO_INFO_LOG(
        "MoveSourceOutputByIndexOrName sourceOutputId %{public}d, sourceIndex %{public}d, sourceName %{public}s",
        sourceOutputId,
        sourceIndex,
        sourceName.c_str());
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::MoveSourceOutputByIndexOrName", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] MoveSourceOutputByIndexOrName timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    Trace trace("MoveSourceOutputByIndexOrName: " + std::to_string(sourceOutputId) +
                " index:" + std::to_string(sourceIndex) + " source:" + sourceName);
    isFinishMoveSourceOutputByIndexOrName_ = false;
    IHpaeManager::GetHpaeManager().MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishMoveSourceOutputByIndexOrName_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("MoveSourceOutputByIndexOrName timeout");
        return ERROR;
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetSourceOutputMute(int32_t uid, bool setMute)
{
    AUDIO_INFO_LOG("SetSourceOutputMute uid %{public}d, setMute %{public}d", uid, setMute);
    lock_guard<mutex> lock(lock_);
    isFinishSetSourceOutputMute_ = false;
    SourceOutputMuteStreamSet_ = 0;
    IHpaeManager::GetHpaeManager().SetSourceOutputMute(uid, setMute);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetSourceOutputMute_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("SetSourceOutputMute timeout");
        return ERROR;
    }
    AUDIO_INFO_LOG("SetSourceOutputMute leave");
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
    isFinishGetAllSinkInputs_ = false;
    IHpaeManager::GetHpaeManager().GetAllSinkInputs();
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAllSinkInputs_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("GetAllSinkInputs timeout");
        sinkInputs_.clear();
    }
    AUDIO_INFO_LOG("GetAllSinkInputs leave");
    return sinkInputs_;
}

std::vector<SourceOutput> ProAudioServiceAdapterImpl::GetAllSourceOutputs()
{
    AUDIO_INFO_LOG("GetAllSourceOutputs");
    AudioXCollie audioXCollie("ProAudioServiceAdapterImpl::GetAllSourceOutputs", HPAE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetAllSourceOutputs timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    lock_guard<mutex> lock(lock_);
    isFinishGetAllSourceOutputs_ = false;
    IHpaeManager::GetHpaeManager().GetAllSourceOutputs();
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAllSourceOutputs_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("GetAllSourceOutputs timeout");
        sourceOutputs_.clear();
    }
    AUDIO_INFO_LOG("GetAllSourceOutputs leave");
    return sourceOutputs_;
}

void ProAudioServiceAdapterImpl::Disconnect()
{
    AUDIO_INFO_LOG("Disconnect not support");
}

int32_t ProAudioServiceAdapterImpl::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    AUDIO_INFO_LOG("GetAudioEffectProperty");
    lock_guard<mutex> lock(lock_);
    isFinishGetAudioEffectPropertyV3_ = false;
    IHpaeManager::GetHpaeManager().GetAudioEffectProperty(propertyArray);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAudioEffectPropertyV3_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    AUDIO_INFO_LOG("GetAudioEffectProperty");
    lock_guard<mutex> lock(lock_);
    isFinishGetAudioEffectProperty_ = false;
    IHpaeManager::GetHpaeManager().GetAudioEffectProperty(propertyArray);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAudioEffectProperty_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    AUDIO_INFO_LOG("GetAudioEnhancePropertyV3");
    lock_guard<mutex> lock(lock_);
    isFinishGetAudioEnhancePropertyV3_ = false;
    IHpaeManager::GetHpaeManager().GetAudioEnhanceProperty(propertyArray);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAudioEnhancePropertyV3_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
    }
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    AUDIO_INFO_LOG("GetAudioEnhanceProperty");
    lock_guard<mutex> lock(lock_);
    isFinishGetAudioEnhanceProperty_ = false;
    IHpaeManager::GetHpaeManager().GetAudioEnhanceProperty(propertyArray);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetAudioEnhanceProperty_;
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("wait for notify timeout");
    }
    return SUCCESS;
}

void ProAudioServiceAdapterImpl::OnReloadAudioPortCb(int32_t portId)
{
    AUDIO_INFO_LOG("OnReloadAudioPortCb portId: %{public}d", portId);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishReloadAudioPort_= true;
    AudioPortIndex_ = portId;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnOpenAudioPortCb(int32_t portId)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnOpenAudioPortCb portId: %{public}d", portId);
    isFinishOpenAudioPort_ = true;
    AudioPortIndex_ = portId;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnCloseAudioPortCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnCloseAudioPortCb result: %{public}d", result);
    isFinishCloseAudioPort_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnSetSinkMuteCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnSetSinkMuteCb result: %{public}d", result);
    isFinishSetSinkMute_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAllSinkInputsCb result: %{public}d", result);
    isFinishGetAllSinkInputs_ = true;
    sinkInputs_ = sinkInputs;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnSetSourceOutputMuteCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnSetSourceOutputMuteCb result: %{public}d", result);
    isFinishSetSourceOutputMute_ = true;
    SourceOutputMuteStreamSet_ = result;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAllSourceOutputsCb result: %{public}d", result);
    isFinishGetAllSourceOutputs_ = true;
    sourceOutputs_ = sourceOutputs;
    callbackCV_.notify_all();
}
void ProAudioServiceAdapterImpl::OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAllSinksCb result: %{public}d", result);
    isFinishGetAllSinks_ = true;
    sinks_ = sinks;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnMoveSinkInputByIndexOrNameCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnMoveSinkInputByIndexOrNameCb result: %{public}d", result);
    isFinishMoveSinkInputByIndexOrName_ = true;
    callbackCV_.notify_all();
}
void ProAudioServiceAdapterImpl::OnMoveSourceOutputByIndexOrNameCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnMoveSourceOutputByIndexOrNameCb result: %{public}d", result);
    isFinishMoveSourceOutputByIndexOrName_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAudioEffectPropertyCbV3(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAudioEffectPropertyCbV3 result: %{public}d", result);
    isFinishGetAudioEffectPropertyV3_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAudioEffectPropertyCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAudioEffectPropertyCb result: %{public}d", result);
    isFinishGetAudioEffectProperty_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAudioEnhancePropertyCbV3(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAudioEnhancePropertyCbV3 result: %{public}d", result);
    isFinishGetAudioEnhancePropertyV3_ = true;
    callbackCV_.notify_all();
}

void ProAudioServiceAdapterImpl::OnGetAudioEnhancePropertyCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnGetAudioEnhancePropertyCb result: %{public}d", result);
    isFinishGetAudioEnhanceProperty_ = true;
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
    AUDIO_INFO_LOG("UpdateCollaborativeState, state %{public}d", isCollaborationEnabled);
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().UpdateCollaborativeState(isCollaborationEnabled);
    return SUCCESS;
}

int32_t ProAudioServiceAdapterImpl::SetAbsVolumeStateToEffect(const bool absVolumeState)
{
    AUDIO_INFO_LOG("SetAbsVolumeStateToEffect, state %{public}d", absVolumeState);
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().SetAbsVolumeStateToEffect(absVolumeState);
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
