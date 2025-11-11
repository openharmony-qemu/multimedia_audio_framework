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
#define LOG_TAG "AudioInjectorPolicy"
#endif

#include "audio_injector_policy.h"
#include "audio_core_service.h"
#include "audio_device_info.h"
#include "audio_policy_manager_factory.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
AudioInjectorPolicy::AudioInjectorPolicy()
    :audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
     audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())
{
    pipeManager_ = AudioPipeManager::GetPipeManager();
    isConnected_ = false;
    isOpened_ = false;
}

int32_t AudioInjectorPolicy::Init()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        AudioModuleInfo moduleInfo = {};
        moduleInfo.lib = "libmodule-hdi-sink.z.so";
        std::string name = VIRTUAL_INJECTOR;
        moduleInfo.name = name;
        moduleInfo.deviceType = std::to_string(static_cast<int32_t>(DEVICE_TYPE_SYSTEM_PRIVATE));
        moduleInfo.className = VIRTUAL_INJECTOR;
        moduleInfo.role = AudioAdapterManager::HDI_AUDIO_PORT_SINK_ROLE;
        moduleInfo.format = "s16le";
        moduleInfo.channels = "2"; // 2 channel
        moduleInfo.rate = "48000";
        moduleInfo.bufferSize = "3840"; // 20ms

        uint32_t paIndex = 0;
        ioHandle_ = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
        CHECK_AND_RETURN_RET_LOG(paIndex != HDI_INVALID_ID, ERR_OPERATION_FAILED,
            "Injector::OpenAudioPort failed paId[%{public}u]", paIndex);

        std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
        pipeInfo->id_ = ioHandle_;
        pipeInfo->paIndex_ = paIndex;
        pipeInfo->name_ = VIRTUAL_INJECTOR;
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        pipeInfo->adapterName_ = moduleInfo.name;
        pipeInfo->moduleInfo_ = moduleInfo;
        pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
        pipeInfo->InitAudioStreamInfo();
        AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.AddIOHandleInfo(VIRTUAL_INJECTOR, ioHandle_);
        isOpened_ = true;
        this->moduleInfo_ = moduleInfo;
        renderPortIdx_ = paIndex;
    }
    return SUCCESS;
}

int32_t AudioInjectorPolicy::DeInit()
{
    if (isOpened_ && rendererStreamMap_.size() == 0) {
        int32_t ret = audioPolicyManager_.CloseAudioPort(ioHandle_, renderPortIdx_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::close port failed");
        std::shared_ptr<AudioPipeInfo> pipeInfo = pipeManager_->GetPipeinfoByNameAndFlag(
            VIRTUAL_INJECTOR, AUDIO_OUTPUT_FLAG_NORMAL);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.DelIOHandleInfo(VIRTUAL_INJECTOR);
        isOpened_ = false;
        renderPortIdx_ = HDI_INVALID_ID;
    }
    return SUCCESS;
}

void AudioInjectorPolicy::UpdateAudioInfo(AudioModuleInfo &info)
{
    audioPolicyManager_.UpdateAudioPortInfo(renderPortIdx_, info);
}

int32_t AudioInjectorPolicy::AddStreamDescriptor(uint32_t renderId, std::shared_ptr<AudioStreamDescriptor> desc)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    rendererStreamMap_[renderId] = desc;
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveStreamDescriptor(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    AUDIO_INFO_LOG("Injector:: renderId: %{public}u", renderId);
    rendererStreamMap_.erase(renderId);
    if (rendererStreamMap_.size() == 0) {
        RemoveCaptureInjectorInner(false);
    }
    rendererMuteStreamMap_.erase(renderId);
    injectorStreamIds_.erase(renderId);
    return SUCCESS;
}

bool AudioInjectorPolicy::IsContainStream(uint32_t renderId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto streamIt = rendererStreamMap_.find(renderId);
    if (streamIt != rendererStreamMap_.end()) {
        return true;
    }
    return false;
}

std::string AudioInjectorPolicy::GetAdapterName()
{
    return moduleInfo_.name;
}

// get the number of rendererStream moved in Injector
int32_t AudioInjectorPolicy::GetRendererStreamCount()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return rendererStreamMap_.size();
}

void AudioInjectorPolicy::SetCapturePortIdx(uint32_t idx)
{
    capturePortIdx_ = idx;
}

uint32_t AudioInjectorPolicy::GetCapturePortIdx()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return capturePortIdx_;
}

void AudioInjectorPolicy::SetRendererPortIdx(uint32_t idx)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    renderPortIdx_ = idx;
}

uint32_t AudioInjectorPolicy::GetRendererPortIdx()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return renderPortIdx_;
}

AudioModuleInfo& AudioInjectorPolicy::GetAudioModuleInfo()
{
    return moduleInfo_;
}

bool AudioInjectorPolicy::GetIsConnected()
{
    return isConnected_;
}

void AudioInjectorPolicy::SetVoipType(VoipType type)
{
    voipType_ = type;
}

void AudioInjectorPolicy::ReleaseCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "Injector::pipeManager_ is null");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    auto pipeList = pipeManager_->GetPipeList();
    for (auto it = pipeList.rbegin(); it != pipeList.rend(); ++it) {
        CHECK_AND_CONTINUE_LOG((*it) != nullptr, "Injector::it is null");
        // because the paindex of all low_latency pipe is 0, distinguishing between types of low-latency paths
        // requires checking Voip-specific scenarios to trigger the following processing
        bool isPortIdxUsed = (voipType_ != FAST_VOIP ||
            ((*it)->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)));
        CHECK_AND_CONTINUE(isPortIdxUsed);
        if ((*it)->paIndex_ == capturePortIdx_ && (*it)->pipeRole_ == PIPE_ROLE_INPUT) {
            streamVec = (*it)->streamDescriptors_;
            break;
        }
    }
    if (streamVec.size() == 0) {
        RemoveCaptureInjectorInner(true);
        return ;
    }
}

void AudioInjectorPolicy::RebuildCaptureInjector(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    uint32_t paIndex = 0;
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "Injector::pipeManager_ is null");
    auto pipeList = pipeManager_->GetPipeList();
    for (const auto &pipe : pipeList) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            if (stream->sessionId_ == streamId) {
                paIndex = pipe->paIndex_;
                streamDesc = stream;
            }
        }
        // because the paindex of all low_latency pipe is 0, distinguishing between types of low-latency paths
        // requires checking Voip-specific scenarios to trigger the following processing
        bool isPortIdxUsed = (voipType_ != FAST_VOIP ||
            (pipe->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)));
        CHECK_AND_CONTINUE(isPortIdxUsed);
        if (pipe->paIndex_ == capturePortIdx_ && pipe->pipeRole_ == PIPE_ROLE_INPUT) {
            streamVec = pipe->streamDescriptors_;
        }
    }
    bool isRunning = HasRunningVoipStream(streamVec);
    if (isRunning) {
        return;
    }
    RemoveCaptureInjectorInner(true);
    if (rendererStreamMap_.size() == 0) {
        return;
    }
    capturePortIdx_ = paIndex;
    if ((streamDesc->routeFlag_ & AUDIO_INPUT_FLAG_NORMAL) &&
            streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        voipType_ = NORMAL_VOIP;
    } else if (streamDesc->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)) {
        voipType_ = FAST_VOIP;
    }
    AddCaptureInjectorInner();
}

bool AudioInjectorPolicy::HasRunningVoipStream(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamVec)
{
    for (const auto &stream : streamVec) {
        if (stream->IsRunning() && stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            return true;
        }
    }
    return false;
}

int32_t AudioInjectorPolicy::AddCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return AddCaptureInjectorInner();
}

int32_t AudioInjectorPolicy::AddCaptureInjectorInner()
{
    if (!isConnected_) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
        if (voipType_ == NORMAL_VOIP) {
            audioPolicyManager_.AddCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
            isConnected_ = true;
        } else if (voipType_ == FAST_VOIP) {
            int32_t ret = audioPolicyManager_.AddCaptureInjector();
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::FAST_VOIP AddCaptureInjector failed");
            UpdateAudioInfo(moduleInfo_);
            isConnected_ = true;
        }
    }
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveCaptureInjector(bool noCapturer)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return RemoveCaptureInjectorInner(noCapturer);
}

int32_t AudioInjectorPolicy::RemoveCaptureInjectorInner(bool noCapturer)
{
    bool flag = (rendererStreamMap_.size() == 0 || noCapturer);
    if (isConnected_ && flag) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "Injector::pipeManager_ is null");
        if (voipType_ == NORMAL_VOIP) {
            audioPolicyManager_.RemoveCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
            isConnected_ = false;
            capturePortIdx_ = HDI_INVALID_ID;
            voipType_ = NO_VOIP;
        } else if (voipType_ == FAST_VOIP) {
            int32_t ret = audioPolicyManager_.RemoveCaptureInjector();
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::FAST_VOIP RemoveCaptureInjector failed");
            isConnected_ = false;
            capturePortIdx_ = HDI_INVALID_ID;
            voipType_ = NO_VOIP;
        }
    }
    // release capturer stream does not require executing Deinit();
    if (!noCapturer) {
        DeInit();
    }
    return SUCCESS;
}

std::shared_ptr<AudioPipeInfo> AudioInjectorPolicy::FindCaptureVoipPipe(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, uint32_t &streamId)
{
    std::shared_ptr<AudioPipeInfo> voipPipe = nullptr;
    for (const auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            bool isRunning = stream->IsRunning();
            CHECK_AND_CONTINUE_LOG(isRunning == true, "Injector::isRunning is false");
            AudioStreamAction action = stream->streamAction_;
            bool actionFlag = (action == AUDIO_STREAM_ACTION_MOVE);
            CHECK_AND_CONTINUE_LOG(actionFlag, "Injector::streamAction is not right");
            if ((stream->routeFlag_ & AUDIO_INPUT_FLAG_NORMAL) &&
                    stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
                voipPipe = pipe;
                streamId = stream->sessionId_;
                break;
            } else if (stream->routeFlag_ & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)) {
                voipPipe = pipe;
            }
        }
    }
    return voipPipe;
}

std::shared_ptr<AudioPipeInfo> AudioInjectorPolicy::FindPipeByStreamId(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, VoipType &type, uint32_t &streamId)
{
    std::shared_ptr<AudioPipeInfo> voipPipe = nullptr;
    CHECK_AND_RETURN_RET_LOG(streamId != UINT32_INVALID_VALUE, voipPipe, "Injector:: streamId is wrong!");
    for (const auto &pipe : pipeInfos) {
        CHECK_AND_CONTINUE_LOG(pipe != nullptr, "Injector::pipeInfo is nullptr");
        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_CONTINUE_LOG(stream != nullptr, "Injector::stream is nullptr");
            bool isRunning = stream->IsRunning();
            CHECK_AND_CONTINUE_LOG(isRunning == true, "Injector::isRunning is false");
            if (streamId == stream->sessionId_) {
                voipPipe = pipe;
                type = NORMAL_VOIP;
                break;
            }
        }
    }
    return voipPipe;
}

void AudioInjectorPolicy::FetchCapDeviceInjectPreProc(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, bool &removeFlag, uint32_t &streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        return;
    }
    VoipType type = VoipType::NO_VOIP;
    std::shared_ptr<AudioPipeInfo> tempPipe = FindCaptureVoipPipe(pipeInfos, streamId);
    if (tempPipe != nullptr && tempPipe->paIndex_ != capturePortIdx_) {
        RemoveCaptureInjectorInner(true);
        removeFlag = true;
    }
}

void AudioInjectorPolicy::FetchCapDeviceInjectPostProc(
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, bool &removeFlag, uint32_t &streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isOpened_) {
        return;
    }
    VoipType type = VoipType::NO_VOIP;
    std::shared_ptr<AudioPipeInfo> tempPipe = FindPipeByStreamId(pipeInfos, type, streamId);
    if (tempPipe != nullptr && removeFlag) {
        capturePortIdx_ = tempPipe->paIndex_;
        voipType_ = type;
        AddCaptureInjectorInner();
    }
}

void AudioInjectorPolicy::AddInjectorStreamId(const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    injectorStreamIds_.insert(streamId);
}

void AudioInjectorPolicy::DeleteInjectorStreamId(const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    injectorStreamIds_.erase(streamId);
}

bool AudioInjectorPolicy::IsActivateInterruptStreamId(const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    return injectorStreamIds_.count(streamId) > 0;
}

void AudioInjectorPolicy::SendInterruptEventToInjectorStreams(const std::shared_ptr<AudioPolicyServerHandler> &handler)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
        INTERRUPT_HINT_STOP, 1.0f};
    for (const auto& pair : rendererStreamMap_) {
        if (handler != nullptr) {
            handler->SendInterruptEventWithStreamIdCallback(interruptEvent, pair.first);
        }
    }
}

void AudioInjectorPolicy::SetInjectStreamsMuteForInjection(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto mute = rendererMuteStreamMap_.find(streamId);
    AUDIO_INFO_LOG("Injector:: streamId: %{public}u, mapIsExist: %{public}d, mute: %{public}d",
        streamId, mute == rendererMuteStreamMap_.end(), isNeedMuteRenderer_);
    if (mute == rendererMuteStreamMap_.end() && isNeedMuteRenderer_) {
        rendererMuteStreamMap_.insert(std::make_pair(streamId, isNeedMuteRenderer_));
        AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(streamId, isNeedMuteRenderer_);
    }
}

void AudioInjectorPolicy::SetInjectStreamsMuteForPlayback(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    auto mute = rendererMuteStreamMap_.find(streamId);
    AUDIO_INFO_LOG("Injector:: streamId: %{public}u, mapIsExist: %{public}d", streamId,
        mute != rendererMuteStreamMap_.end());
    if (mute != rendererMuteStreamMap_.end() && mute->second == true) {
        AUDIO_INFO_LOG("Injector:: setMuteFalse: %{public}u", streamId);
        AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(streamId, false);
        rendererMuteStreamMap_[streamId] = false;
    }
}

void AudioInjectorPolicy::SetInjectorStreamsMute(bool newMicrophoneMute)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    isNeedMuteRenderer_ = newMicrophoneMute;
    AUDIO_INFO_LOG("Injector:: %{public}d", newMicrophoneMute);
    for (const auto& streamId : injectorStreamIds_) {
        auto mute = rendererMuteStreamMap_.find(streamId);
        AUDIO_INFO_LOG("Injector:: streamId: %{public}u, mapIsExist: %{public}d", streamId,
            mute == rendererMuteStreamMap_.end());
        if (mute == rendererMuteStreamMap_.end()) {
            rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
        } else {
            rendererMuteStreamMap_[streamId] = newMicrophoneMute;
        }
        AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(streamId, newMicrophoneMute);
    }
}
}  //  namespace AudioStandard
}  //  namespace OHOS