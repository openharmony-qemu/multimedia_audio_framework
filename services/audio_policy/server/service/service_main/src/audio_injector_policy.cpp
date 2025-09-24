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
#include "audio_policy_manager_factory.h"
#include "audio_core_service.h"
#include "audio_device_info.h"
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
        AUDIO_INFO_LOG("first time to open port");
        AudioModuleInfo moduleInfo = {};
        moduleInfo.lib = "libmodule-hdi-sink.z.so";
        std::string name = VIRTUAL_INJECTOR;
        moduleInfo.name = name;
        moduleInfo.deviceType = "SYSTEM_PRIVATE";
        moduleInfo.format = "s16le";
        moduleInfo.channels = "2"; // 2 channel
        moduleInfo.rate = "48000";
        moduleInfo.bufferSize = "3840"; // 20ms

        int32_t ret = audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "open port failed");
        isOpened_ = true;
        this->moduleInfo_ = moduleInfo;
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is null");
        renderPortIdx_ = pipeManager_->GetPaIndexByName(moduleInfo.name);
        CHECK_AND_RETURN_RET_LOG(renderPortIdx_ != HDI_INVALID_ID, ERROR, "renderPortIdx error!");
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
    rendererStreamMap_[renderId] = desc;
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveStreamDescriptor(uint32_t renderId)
{
    rendererStreamMap_[renderId] = nullptr;
    rendererStreamMap_.erase(renderId);
    return SUCCESS;
}

bool AudioInjectorPolicy::IsContainStream(uint32_t renderId)
{
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
    return rendererStreamMap_.size();
}

void AudioInjectorPolicy::SetCapturePortIdx(uint32_t idx)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    capturePortIdx_ = idx;
}

uint32_t AudioInjectorPolicy::GetCapturePortIdx()
{
    return capturePortIdx_;
}

void AudioInjectorPolicy::SetRendererPortIdx(uint32_t idx)
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    renderPortIdx_ = idx;
}

uint32_t AudioInjectorPolicy::GetRendererPortIdx()
{
    return renderPortIdx_;
}

AudioModuleInfo& AudioInjectorPolicy::GetAudioModuleInfo()
{
    return moduleInfo_;
}

int32_t AudioInjectorPolicy::AddCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    int32_t ret = ERROR;
    if (!isConnected_) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is null");
        if (pipeManager_->IsCaptureVoipCall() == NORMAL_VOIP) {
            ret = audioPolicyManager_.AddCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
        } else if (pipeManager_->IsCaptureVoipCall() == FAST_VOIP) {
            ret = audioPolicyManager_.AddCaptureInjector();
        }
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "AddCaptureInjector failed");
        isConnected_ = true;
    }
    return SUCCESS;
}
    
int32_t AudioInjectorPolicy::RemoveCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(injectLock_);
    int32_t ret = ERROR;
    if (isConnected_ && rendererStreamMap_.size() == 0) {
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is null");
        if (pipeManager_->IsCaptureVoipCall() == NORMAL_VOIP) {
            ret = audioPolicyManager_.RemoveCaptureInjector(renderPortIdx_, capturePortIdx_,
                SOURCE_TYPE_VOICE_COMMUNICATION);
        } else if (pipeManager_->IsCaptureVoipCall() == FAST_VOIP) {
            ret = audioPolicyManager_.RemoveCaptureInjector();
        }
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "RemoveCaptureInjector failed");
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
            INTERRUPT_HINT_STOP, 1.0f};
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