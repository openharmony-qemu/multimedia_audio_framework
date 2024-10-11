/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "IpcStreamInServer"
#endif

#include <memory>
#include <cinttypes>

#include "ipc_stream_in_server.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_schedule.h"

namespace OHOS {
namespace AudioStandard {
StreamListenerHolder::StreamListenerHolder()
{
    AUDIO_INFO_LOG("StreamListenerHolder()");
}

StreamListenerHolder::~StreamListenerHolder()
{
    AUDIO_INFO_LOG("~StreamListenerHolder()");
}

int32_t StreamListenerHolder::RegisterStreamListener(sptr<IpcStreamListener> listener)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    // should only be set once
    if (streamListener_ != nullptr) {
        return ERR_INVALID_OPERATION;
    }
    streamListener_ = listener;
    return SUCCESS;
}

int32_t StreamListenerHolder::OnOperationHandled(Operation operation, int64_t result)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    CHECK_AND_RETURN_RET_LOG(streamListener_ != nullptr, ERR_OPERATION_FAILED, "stream listrener not set");
    return streamListener_->OnOperationHandled(operation, result);
}

sptr<IpcStreamInServer> IpcStreamInServer::Create(const AudioProcessConfig &config, int32_t &ret)
{
    AudioMode mode = config.audioMode;
    sptr<IpcStreamInServer> streamInServer = sptr<IpcStreamInServer>::MakeSptr(config, mode);
    ret = streamInServer->Config();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("IpcStreamInServer Config failed: %{public}d, uid: %{public}d",
            ret, config.appInfo.appUid); // waiting for review: add uid.
        streamInServer = nullptr;
    }
    return streamInServer;
}

IpcStreamInServer::IpcStreamInServer(const AudioProcessConfig &config, AudioMode mode) : config_(config), mode_(mode)
{
    AUDIO_INFO_LOG("IpcStreamInServer(), uid: %{public}d", config.appInfo.appUid); // waiting for review: add uid.
}

IpcStreamInServer::~IpcStreamInServer()
{
    AUDIO_INFO_LOG("~IpcStreamInServer(), uid: %{public}d", config_.appInfo.appUid); // waiting for review: add uid.
}

int32_t IpcStreamInServer::Config()
{
    streamListenerHolder_ = std::make_shared<StreamListenerHolder>();

    if (mode_ == AUDIO_MODE_PLAYBACK) {
        return ConfigRenderer();
    }
    if (mode_ == AUDIO_MODE_RECORD) {
        return ConfigCapturer();
    }
    AUDIO_ERR_LOG("Config failed, mode is %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

std::shared_ptr<RendererInServer> IpcStreamInServer::GetRenderer()
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("GetRenderer failed, mode is %{public}s", (mode_ != AUDIO_MODE_PLAYBACK ? " not playback" :
            "playback, but renderer is null!"));
        return nullptr;
    }
    return rendererInServer_;
}

std::shared_ptr<CapturerInServer> IpcStreamInServer::GetCapturer()
{
    if (mode_ != AUDIO_MODE_RECORD || capturerInServer_ == nullptr) {
        AUDIO_ERR_LOG("GetCapturer failed, mode is %{public}s", (mode_ != AUDIO_MODE_RECORD ? " not record" :
            "record, but capturer is null!"));
        return nullptr;
    }
    return capturerInServer_;
}

int32_t IpcStreamInServer::ConfigRenderer()
{
    rendererInServer_ = std::make_shared<RendererInServer>(config_, streamListenerHolder_);
    CHECK_AND_RETURN_RET_LOG(rendererInServer_ != nullptr, ERR_OPERATION_FAILED, "Create RendererInServer failed");
    int32_t ret = rendererInServer_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Init RendererInServer failed");
    return SUCCESS;
}

int32_t IpcStreamInServer::ConfigCapturer()
{
    capturerInServer_ = std::make_shared<CapturerInServer>(config_, streamListenerHolder_);
    CHECK_AND_RETURN_RET_LOG(capturerInServer_ != nullptr, ERR_OPERATION_FAILED, "create CapturerInServer failed");
    int32_t ret = capturerInServer_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Init CapturerInServer failed");
    return SUCCESS;
}

int32_t IpcStreamInServer::RegisterStreamListener(sptr<IRemoteObject> object)
{
    CHECK_AND_RETURN_RET_LOG(streamListenerHolder_ != nullptr, ERR_OPERATION_FAILED, "RegisterStreamListener failed");
    sptr<IpcStreamListener> listener = iface_cast<IpcStreamListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "RegisterStreamListener obj cast failed");
    streamListenerHolder_->RegisterStreamListener(listener);

    // in plan: get session id, use it as key to find IpcStreamInServer
    // in plan: listener->AddDeathRecipient( server ) // when client died, do release and clear works

    return SUCCESS;
}

int32_t IpcStreamInServer::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    AUDIO_INFO_LOG("Resolve buffer, mode: %{public}d", mode_);
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->ResolveBuffer(buffer);
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->ResolveBuffer(buffer);
    }
    AUDIO_ERR_LOG("GetAudioSessionID failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::UpdatePosition()
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->UpdateWriteIndex();
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->UpdateReadIndex();
    }
    AUDIO_ERR_LOG("UpdatePosition failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetAudioSessionID(uint32_t &sessionId)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetSessionId(sessionId);
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->GetSessionId(sessionId);
    }
    AUDIO_ERR_LOG("GetAudioSessionID failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::Start()
{
    AUDIO_INFO_LOG("IpcStreamInServer::Start()");

    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->Start();
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->Start();
    }
    AUDIO_ERR_LOG("Start failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::Pause()
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->Pause();
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->Pause();
    }
    AUDIO_ERR_LOG("Pause failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::Stop()
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->Stop();
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->Stop();
    }
    AUDIO_ERR_LOG("Stop failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::Release()
{
    UnscheduleReportData(config_.appInfo.appPid, clientTid_, clientBundleName_.c_str());
    clientThreadPriorityRequested_ = false;
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->Release();
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->Release();
    }
    AUDIO_ERR_LOG("Release failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::Flush()
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->Flush();
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->Flush();
    }
    AUDIO_ERR_LOG("Flush failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::Drain(bool stopFlag)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->Drain(stopFlag);
    }
    AUDIO_ERR_LOG("Drain failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_ != nullptr) {
        return capturerInServer_->UpdatePlaybackCaptureConfig(config);
    }
    AUDIO_ERR_LOG("Failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetAudioTime(uint64_t &framePos, uint64_t &timestamp)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetAudioTime(framePos, timestamp);
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->GetAudioTime(framePos, timestamp);
    }
    AUDIO_ERR_LOG("GetAudioTime failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetAudioPosition(uint64_t &framePos, uint64_t &timestamp)
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("unsupported mode: %{public}d or renderer obj is nullptr", static_cast<int32_t>(mode_));
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->GetAudioPosition(framePos, timestamp);
}

int32_t IpcStreamInServer::GetLatency(uint64_t &latency)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetLatency(latency);
    }
    if (mode_ == AUDIO_MODE_RECORD && capturerInServer_!= nullptr) {
        return capturerInServer_->GetLatency(latency);
    }
    AUDIO_ERR_LOG("GetAudioSessionID failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::SetRate(int32_t rate)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->SetRate(rate);
    }
    AUDIO_ERR_LOG("SetRate failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetRate(int32_t &rate)
{
    // In plan
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::SetLowPowerVolume(float volume)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->SetLowPowerVolume(volume);
    }
    AUDIO_ERR_LOG("SetLowPowerVolume failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetLowPowerVolume(float &volume)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetLowPowerVolume(volume);
    }
    AUDIO_ERR_LOG("GetLowPowerVolume failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::SetAudioEffectMode(int32_t effectMode)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->SetAudioEffectMode(effectMode);
    }
    AUDIO_ERR_LOG("SetAudioEffectMode failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetAudioEffectMode(int32_t &effectMode)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetAudioEffectMode(effectMode);
    }
    AUDIO_ERR_LOG("GetAudioEffectMode failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::SetPrivacyType(int32_t privacyType)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->SetPrivacyType(privacyType);
    }
    AUDIO_ERR_LOG("SetPrivacyType failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::GetPrivacyType(int32_t &privacyType)
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetPrivacyType(privacyType);
    }
    AUDIO_ERR_LOG("GetPrivacyType failed, invalid mode: %{public}d", static_cast<int32_t>(mode_));
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::SetOffloadMode(int32_t state, bool isAppBack)
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("failed, invalid mode: %{public}d, or rendererInServer_ is null: %{public}d,",
            static_cast<int32_t>(mode_), rendererInServer_ == nullptr);
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->SetOffloadMode(state, isAppBack);
}

int32_t IpcStreamInServer::UnsetOffloadMode()
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("failed, invalid mode: %{public}d, or rendererInServer_ is null: %{public}d,",
            static_cast<int32_t>(mode_), rendererInServer_ == nullptr);
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->UnsetOffloadMode();
}

int32_t IpcStreamInServer::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("failed, invalid mode: %{public}d, or rendererInServer_ is null: %{public}d,",
            static_cast<int32_t>(mode_), rendererInServer_ == nullptr);
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
}

int32_t IpcStreamInServer::OffloadSetVolume(float volume)
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("failed, invalid mode: %{public}d, or rendererInServer_ is null: %{public}d,",
            static_cast<int32_t>(mode_), rendererInServer_ == nullptr);
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->OffloadSetVolume(volume);
}

int32_t IpcStreamInServer::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("failed, invalid mode: %{public}d, or rendererInServer_ is null: %{public}d,",
            static_cast<int32_t>(mode_), rendererInServer_ == nullptr);
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
}

int32_t IpcStreamInServer::GetStreamManagerType()
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->GetStreamManagerType();
    }
    AUDIO_ERR_LOG("mode is not playback or renderer is null");
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::SetSilentModeAndMixWithOthers(bool on)
{
    if (mode_ != AUDIO_MODE_PLAYBACK || rendererInServer_ == nullptr) {
        AUDIO_ERR_LOG("failed, invalid mode: %{public}d, or rendererInServer_ is null: %{public}d,",
            static_cast<int32_t>(mode_), rendererInServer_ == nullptr);
        return ERR_OPERATION_FAILED;
    }
    return rendererInServer_->SetSilentModeAndMixWithOthers(on);
}

int32_t IpcStreamInServer::SetClientVolume()
{
    if (mode_ == AUDIO_MODE_PLAYBACK && rendererInServer_ != nullptr) {
        return rendererInServer_->SetClientVolume();
    }
    AUDIO_ERR_LOG("mode is not playback or renderer is null");
    return ERR_OPERATION_FAILED;
}

int32_t IpcStreamInServer::RegisterThreadPriority(uint32_t tid, const std::string &bundleName)
{
    if (!clientThreadPriorityRequested_) {
        clientTid_ = tid;
        clientBundleName_ = bundleName;
        ScheduleReportData(config_.appInfo.appPid, tid, bundleName.c_str());
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("client thread priority requested");
        return ERR_OPERATION_FAILED;
    }
}
} // namespace AudioStandard
} // namespace OHOS
