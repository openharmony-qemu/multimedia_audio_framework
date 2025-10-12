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
        AUDIO_INFO_LOG("first time to open port!!!");
        AudioModuleInfo moduleInfo = {};
        moduleInfo.lib = "libmodule-hdi-sink.z.so";
        std::string name = VIRTUAL_INJECTOR;
        moduleInfo.name = name;
        moduleInfo.deviceType = "SYSTEM_PRIVATE";
        moduleInfo.format = "s16le";
        moduleInfo.channels = "2"; // 2 channel
        moduleInfo.rate = "48000";
        moduleInfo.bufferSize = "3840"; // 20ms

        uint32_t paIndex = 0;
        AudioIOHandle ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
        CHECK_AND_RETURN_RET_LOG(paIndex != HDI_INVALID_ID, ERR_OPERATION_FAILED,
            "OpenAudioPort failed paId[%{public}u]", paIndex);

        std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
        pipeInfo->id_ = ioHandle;
        pipeInfo->paIndex_ = paIndex;
        pipeInfo->name_ = VIRTUAL_INJECTOR;
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        pipeInfo->adapterName_ = moduleInfo.adapterName;
        pipeInfo->moduleInfo_ = moduleInfo;
        pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
        pipeInfo->InitAudioStreamInfo();
        AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.AddIOHandleInfo(VIRTUAL_INJECTOR, ioHandle);
        isOpened_ = true;
        this->moduleInfo_ = moduleInfo;
        renderPortIdx_ = paIndex;
    }
    return SUCCESS;
}

int32_t AudioInjectorPolicy::DeInit()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (isOpened_ && rendererStreamMap_.size() == 0) {
        int32_t ret = audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleInfo_.name);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "close port failed");
        isOpened_ = false;
        renderPortIdx_ = HDI_INVALID_ID;
    }
    return SUCCESS;
}

int32_t AudioInjectorPolicy::UpdateAudioInfo(AudioModuleInfo &info)
{
    return SUCCESS;
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
    rendererStreamMap_.erase(renderId);
    if (rendererStreamMap_.size() == 0) {
        RemoveCaptureInjector(false);
    }
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

void AudioInjectorPolicy::ReleaseCaptureInjector(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager_ is null");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    auto pipeList = pipeManager_->GetPipeList();
    for (auto it = pipeList.rbegin(); it != pipeList.rend(); ++it) {
        CHECK_AND_CONTINUE_LOG((*it) != nullptr, "it is null");
        if ((*it)->paIndex_ == capturePortIdx_ && (*it)->pipeRole_ == PIPE_ROLE_INPUT) {
            streamVec = (*it)->streamDescriptors_;
            break;
        }
    }
    if (streamVec.size() == 0) {
        RemoveCaptureInjector(true);
        return ;
    }
}

int32_t AudioInjectorPolicy::AddCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    if (!isConnected_) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is null");
        if (voipType_ == NORMAL_VOIP) {
            audioPolicyManager_.AddCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
        } else if (voipType_ == FAST_VOIP) {
            int32_t ret = audioPolicyManager_.AddCaptureInjector();
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "AddCaptureInjector failed");
        }
        isConnected_ = true;
    }
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveCaptureInjector(bool noCapturer)
{
    // std::lock_guard<std::shared_mutex> lock(injectLock_);
    bool flag = (rendererStreamMap_.size() == 0 || noCapturer);
    if (isConnected_ && flag) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is null");
        if (pipeManager_->IsCaptureVoipCall() == NORMAL_VOIP) {
            audioPolicyManager_.RemoveCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
        } else if (pipeManager_->IsCaptureVoipCall() == FAST_VOIP) {
            int32_t ret = audioPolicyManager_.RemoveCaptureInjector();
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "RemoveCaptureInjector failed");
        }
        isConnected_ = false;
    }
    return SUCCESS;
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
        INTERRUPT_HINT_PAUSE, 1.0f};
    for (const auto& pair : rendererStreamMap_) {
        if (handler != nullptr) {
            handler->SendInterruptEventWithStreamIdCallback(interruptEvent, pair.first);
        }
    }
}

int32_t AudioInjectorPolicy::SetInjectorStreamsMute(bool newMicrophoneMute)
{
    int32_t ret = SUCCESS;
    for (const auto& pair : rendererStreamMap_) {
        ret = AudioServerProxy::GetInstance().SetNonInterruptMuteProxy(pair.first, newMicrophoneMute);
    }
    return ret;
}
}  //  namespace AudioStandard
}  //  namespace OHOS