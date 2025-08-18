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
#define LOG_TAG "AudioStreamMonitor"
#endif

#include "audio_stream_monitor.h"
#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "audio_utils.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t CHECK_ALL_RENDER_UID = -1;
const int32_t MAX_REGISTER_CALLBACK_NUM = 30;
}

AudioStreamMonitor& AudioStreamMonitor::GetInstance()
{
    static AudioStreamMonitor monitor;
    return monitor;
}

bool AudioStreamMonitor::HasRegistered(const int32_t pid, const int32_t callbackId)
{
    auto iter = registerInfo_.find(std::make_pair(pid, callbackId));
    if (iter != registerInfo_.end()) {
        AUDIO_INFO_LOG("Monitor has registered, pid = %{public}d, callbackId = %{public}d",
            pid, callbackId);
        return true;
    }
    AUDIO_INFO_LOG("Monitor not register, pid = %{public}d, callbackId = %{public}d", pid, callbackId);
    return false;
}

int32_t AudioStreamMonitor::RegisterAudioRendererDataTransferStateListener(
    const DataTransferMonitorParam &param, const int32_t pid, const int32_t callbackId)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    AUDIO_INFO_LOG("Start register, pid = %{public}d, callbackId = %{public}d", pid, callbackId);
    if (registerInfo_.size() > MAX_REGISTER_CALLBACK_NUM) {
        AUDIO_ERR_LOG("Audio stream register num exceed max");
        return ERR_AUDIO_STREAM_REGISTER_EXCEED_MAX;
    }
    if (HasRegistered(pid, callbackId)) {
        AUDIO_ERR_LOG("Audio stream register repeat");
        return ERR_AUDIO_STREAM_REGISTER_REPEAT;
    }

    std::pair<int32_t, int32_t> pairData = std::make_pair(pid, callbackId);
    registerInfo_[pairData] = param;
    AUDIO_INFO_LOG("Register audio stream monitor success");
    for (auto &item : audioStreamCheckers_) {
        if (item.second->GetAppUid() == param.clientUID || param.clientUID == CHECK_ALL_RENDER_UID) {
            AUDIO_INFO_LOG("Find and init checker, sessionId = %{public}u, uid = %{public}d",
                item.first, param.clientUID);
            item.second->InitChecker(param, pid, callbackId);
        }
    }
    return SUCCESS;
}

int32_t AudioStreamMonitor::UnregisterAudioRendererDataTransferStateListener(
    const int32_t pid, const int32_t callbackId)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    AUDIO_INFO_LOG("Start unregister, pid = %{public}d, callbackId = %{public}d", pid, callbackId);
    for (auto iter = registerInfo_.begin(); iter != registerInfo_.end();) {
        if (iter->first.first == pid && iter->first.second == callbackId) {
            AUDIO_INFO_LOG("Unregister callback seccess");
            iter = registerInfo_.erase(iter);
        } else {
            iter++;
        }
    }
    for (auto iter = audioStreamCheckers_.begin(); iter != audioStreamCheckers_.end(); iter++) {
        iter->second->DeleteCheckerPara(pid, callbackId);
    }
    return SUCCESS;
}

void AudioStreamMonitor::OnCallback(int32_t pid, int32_t callbackId,
    const AudioRendererDataTransferStateChangeInfo &info)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (audioServer_ == nullptr) {
        return;
    }
    AUDIO_INFO_LOG("pid = %{public}d, callbackid = %{public}d, sessionId = %{public}d, type = %{public}d",
        pid, callbackId, info.sessionId, info.stateChangeType);
    audioServer_->OnDataTransferStateChange(pid, callbackId, info);
}

void AudioStreamMonitor::OnMuteCallback(const int32_t &pid, const int32_t &callbackId,
    const int32_t &uid, const uint32_t &sessionId, const bool &isMuted)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (audioServer_ == nullptr) {
        return;
    }
    AUDIO_INFO_LOG("pid = %{public}d, uid = %{public}d, sessionId = %{public}d, isMuted = %{public}d",
        pid, uid, sessionId, isMuted);
    audioServer_->OnMuteStateChange(pid, callbackId, uid, sessionId, isMuted);
}

void AudioStreamMonitor::SetAudioServerPtr(DataTransferStateChangeCallbackForMonitor *ptr)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    audioServer_ = ptr;
}

void AudioStreamMonitor::AddCheckForMonitor(uint32_t sessionId, std::shared_ptr<AudioStreamChecker> &checker)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    AUDIO_INFO_LOG("Add checker for monitor, sessionId = %{public}u", sessionId);
    auto iter = audioStreamCheckers_.find(sessionId);
    if (iter == audioStreamCheckers_.end()) {
        audioStreamCheckers_[sessionId] = checker;
        AUDIO_INFO_LOG("Add checker for monitor success, uid = %{public}d", checker->GetAppUid());
    }
    for (auto item : registerInfo_) {
        if (item.second.clientUID == checker->GetAppUid() || item.second.clientUID == CHECK_ALL_RENDER_UID) {
            AUDIO_INFO_LOG("Find register, need init checker, uid = %{public}d", item.second.clientUID);
            checker->InitChecker(item.second, item.first.first, item.first.second);
        }
    }
}

void AudioStreamMonitor::OnCallbackAppDied(const int32_t pid)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    AUDIO_INFO_LOG("On callback app died, pid = %{public}d", pid);
    for (auto iter = registerInfo_.begin(); iter != registerInfo_.end();) {
        if (iter->first.first == pid) {
            AUDIO_INFO_LOG("erase registerInfo seccess by pid");
            iter = registerInfo_.erase(iter);
        } else {
            iter++;
        }
    }
    for (auto iter = audioStreamCheckers_.begin(); iter != audioStreamCheckers_.end(); iter++) {
        iter->second->OnRemoteAppDied(pid);
    }
}

void AudioStreamMonitor::DeleteCheckForMonitor(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    for (auto iter = audioStreamCheckers_.begin(); iter != audioStreamCheckers_.end();) {
        if (iter->first == sessionId) {
            AUDIO_INFO_LOG("Find checker and delete, sessionId = %{public}u", sessionId);
            iter->second->StopCheckStreamThread();
            iter = audioStreamCheckers_.erase(iter);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("Can not find checker, sessionId = %{public}u", sessionId);
}

void AudioStreamMonitor::ReportStreamFreezen(int64_t intervalTime)
{
    // To do report
}

void AudioStreamMonitor::NotifyAppStateChange(const int32_t uid, bool isBackground)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    for (auto iter = audioStreamCheckers_.begin(); iter != audioStreamCheckers_.end();) {
        if (iter->second->GetAppUid() == uid) {
            iter->second->UpdateAppState(isBackground);
        }
        iter++;
    }
}

void AudioStreamMonitor::UpdateMonitorVolume(const uint32_t &sessionId, const float &volume)
{
    std::lock_guard<std::mutex> lock(regStatusMutex_);
    auto iter = audioStreamCheckers_.find(sessionId);
    if (iter != audioStreamCheckers_.end()) {
        iter->second->SetVolume(volume);
    }
}
}
}