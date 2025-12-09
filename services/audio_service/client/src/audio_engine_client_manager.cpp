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
#define LOG_TAG "AudioEngineClientManager"
#endif

#include "audio_engine_client_manager.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_common_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

sptr<IStandardAudioService> AudioEngineClientManager::gServerProxy = nullptr;
sptr<AudioEngineClientManager::CallbackHandle> AudioEngineClientManager::gCallbackHandle = nullptr;
std::mutex AudioEngineClientManager::gServerProxyLock;

static sptr<IStandardAudioService> GetAudioServiceProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get samgr failed");

    // Use block mode to handle server not starting case
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio sa remote object failed");

    sptr<IStandardAudioService> proxy = iface_cast<IStandardAudioService>(object);
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, nullptr, "cast interface failed");
    return proxy;
}

const sptr<IStandardAudioService> AudioEngineClientManager::InitAndGetAudioServiceProxy()
{
    sptr<IStandardAudioService> proxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(gServerProxyLock);

        if (gServerProxy != nullptr && gServerProxy->AsObject()->IsObjectDead()) {
            AUDIO_ERR_LOG("server proxy dead, need restore");
            gServerProxy = nullptr;
        }

        if (gServerProxy == nullptr) {
            proxy = GetAudioServiceProxy();
            CHECK_AND_RETURN_RET(proxy != nullptr, nullptr);

            // Callback handle is used to receive all callback events and handle server die
            if (gCallbackHandle == nullptr) {
                gCallbackHandle = new CallbackHandle();
            }
            sptr<IRemoteObject> proxyObject = proxy->AsObject();
            proxyObject->AddDeathRecipient(gCallbackHandle);
            proxy->RegisterCallbackHandle(gCallbackHandle->AsObject());

            // Update global variable after all error cases
            gServerProxy = proxy;
        }
        proxy = gServerProxy;
    }
    return proxy;
}

AudioEngineClientManager::AudioEngineClientManager()
{
    AUDIO_DEBUG_LOG("ctor");
}

AudioEngineClientManager::~AudioEngineClientManager()
{
    AUDIO_WARNING_LOG("dtor should not happen");
}

int32_t AudioEngineClientManager::GetCurrentOutputPipeChangeInfos(
    std::vector<std::shared_ptr<AudioOutputPipeInfo>> &pipeChangeInfos)
{
    const sptr<IStandardAudioService> proxy = InitAndGetAudioServiceProxy();
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_IPC, "can not get audio proxy");

    ErrCode err = proxy->GetCurrentOutputPipeChangeInfos(pipeChangeInfos);
    CHECK_AND_RETURN_RET_LOG(err == NO_ERROR, ERR_IPC, "ipc call failed");
    return SUCCESS;
}

int32_t AudioEngineClientManager::RegisterOutputPipeChangeCallback(std::shared_ptr<AudioOutputPipeCallback> &callback)
{
    const sptr<IStandardAudioService> proxy = InitAndGetAudioServiceProxy();
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_IPC, "can not get audio proxy");

    sptr<CallbackHandle> handle = gCallbackHandle;
    CHECK_AND_RETURN_RET_LOG(handle != nullptr, ERR_INIT_FAILED, "callback handle invalid");
    bool needUpdate = handle->AddOutputPipeChangeCallback(callback);
    if (needUpdate) {
        proxy->SetCallbackHandleEnable(CALLBACK_OUTPUT_PIPE_CHANGE, true);
    }
    return SUCCESS;
}

int32_t AudioEngineClientManager::UnregisterOutputPipeChangeCallback(std::shared_ptr<AudioOutputPipeCallback> &callback)
{
    const sptr<IStandardAudioService> proxy = InitAndGetAudioServiceProxy();
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_IPC, "can not get audio proxy");

    sptr<CallbackHandle> handle = gCallbackHandle;
    CHECK_AND_RETURN_RET_LOG(handle != nullptr, ERR_INIT_FAILED, "callback handle invalid");
    bool needUpdate = handle->RemoveOutputPipeChangeCallback(callback);
    if (needUpdate) {
        proxy->SetCallbackHandleEnable(CALLBACK_OUTPUT_PIPE_CHANGE, false);
    }
    return SUCCESS;
}

int32_t AudioEngineClientManager::GetCurrentInputPipeChangeInfos(
    std::vector<std::shared_ptr<AudioInputPipeInfo>> &pipeChangeInfos)
{
    const sptr<IStandardAudioService> proxy = InitAndGetAudioServiceProxy();
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_IPC, "can not get audio proxy");

    ErrCode err = proxy->GetCurrentInputPipeChangeInfos(pipeChangeInfos);
    CHECK_AND_RETURN_RET_LOG(err == NO_ERROR, ERR_IPC, "ipc call failed");
    return SUCCESS;
}

int32_t AudioEngineClientManager::RegisterInputPipeChangeCallback(std::shared_ptr<AudioInputPipeCallback> &callback)
{
    const sptr<IStandardAudioService> proxy = InitAndGetAudioServiceProxy();
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_IPC, "can not get audio proxy");

    sptr<CallbackHandle> handle = gCallbackHandle;
    CHECK_AND_RETURN_RET_LOG(handle != nullptr, ERR_INIT_FAILED, "callback handle invalid");
    bool needUpdate = handle->AddInputPipeChangeCallback(callback);
    if (needUpdate) {
        proxy->SetCallbackHandleEnable(CALLBACK_INPUT_PIPE_CHANGE, true);
    }
    return SUCCESS;
}

int32_t AudioEngineClientManager::UnregisterInputPipeChangeCallback(std::shared_ptr<AudioInputPipeCallback> &callback)
{
    const sptr<IStandardAudioService> proxy = InitAndGetAudioServiceProxy();
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_IPC, "can not get audio proxy");

    sptr<CallbackHandle> handle = gCallbackHandle;
    CHECK_AND_RETURN_RET_LOG(handle != nullptr, ERR_INIT_FAILED, "callback handle invalid");
    bool needUpdate = handle->RemoveInputPipeChangeCallback(callback);
    if (needUpdate) {
        proxy->SetCallbackHandleEnable(CALLBACK_INPUT_PIPE_CHANGE, false);
    }
    return SUCCESS;
}

bool AudioEngineClientManager::CallbackHandle::AddOutputPipeChangeCallback(
    std::shared_ptr<AudioOutputPipeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(lock_);

    if (callback == nullptr) {
        AUDIO_ERR_LOG("input callback invalid");
        return false;
    }

    for (size_t i = 0; i < outputPipeCbs_.size(); ++i) {
        if (outputPipeCbs_[i] == callback) {
            AUDIO_ERR_LOG("do not allow to add same callback");
            return false;
        }
    }
    outputPipeCbs_.push_back(callback);
    // Should update enable for 0 to 1 case.
    return (outputPipeCbs_.size() == 1);
}

bool AudioEngineClientManager::CallbackHandle::RemoveOutputPipeChangeCallback(
    std::shared_ptr<AudioOutputPipeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(lock_);

    if (callback == nullptr) {
        outputPipeCbs_.clear();
        return true;
    }

    size_t i;
    for (i = 0; i < outputPipeCbs_.size(); ++i) {
        if (outputPipeCbs_[i] == callback) {
            // Already deduplication in add func, so only one can be find here.
            break;
        }
    }
    if (i == outputPipeCbs_.size()) {
        AUDIO_ERR_LOG("callback not added yet");
        return false;
    }
    outputPipeCbs_.erase(outputPipeCbs_.begin() + i);
    // Should update enable for 1 to 0 case.
    return (outputPipeCbs_.size() == 0);
}

int32_t AudioEngineClientManager::CallbackHandle::OnOutputPipeChange(
    int32_t changeType, const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
{
    std::lock_guard<std::mutex> lock(lock_);
    for (size_t i = 0; i < outputPipeCbs_.size(); ++i) {
        outputPipeCbs_[i]->OnOutputPipeChange(static_cast<AudioPipeChangeType>(changeType), changedPipeInfo);
    }
    return SUCCESS;
}

bool AudioEngineClientManager::CallbackHandle::AddInputPipeChangeCallback(
    std::shared_ptr<AudioInputPipeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(lock_);

    if (callback == nullptr) {
        AUDIO_ERR_LOG("input callback invalid");
        return false;
    }

    for (size_t i = 0; i < inputPipeCbs_.size(); ++i) {
        if (inputPipeCbs_[i] == callback) {
            AUDIO_ERR_LOG("do not allow to add same callback");
            return false;
        }
    }
    inputPipeCbs_.push_back(callback);
    // Should update enable for 0 to 1 case.
    return (inputPipeCbs_.size() == 1);
}

bool AudioEngineClientManager::CallbackHandle::RemoveInputPipeChangeCallback(
    std::shared_ptr<AudioInputPipeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(lock_);

    if (callback == nullptr) {
        inputPipeCbs_.clear();
        return true;
    }

    size_t i;
    for (i = 0; i < inputPipeCbs_.size(); ++i) {
        if (inputPipeCbs_[i] == callback) {
            // Already deduplication in add func, so only one can be find here.
            break;
        }
    }
    if (i == inputPipeCbs_.size()) {
        AUDIO_ERR_LOG("callback not added yet");
        return false;
    }
    inputPipeCbs_.erase(inputPipeCbs_.begin() + i);
    // Should update enable for 1 to 0 case.
    return (inputPipeCbs_.size() == 0);
}

int32_t AudioEngineClientManager::CallbackHandle::OnInputPipeChange(
    int32_t changeType, const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
{
    std::lock_guard<std::mutex> lock(lock_);
    for (size_t i = 0; i < inputPipeCbs_.size(); ++i) {
        inputPipeCbs_[i]->OnInputPipeChange(static_cast<AudioPipeChangeType>(changeType), changedPipeInfo);
    }
    return SUCCESS;
}

void AudioEngineClientManager::CallbackHandle::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    AUDIO_INFO_LOG("audio server died, try to restore proxy and callback handle");
    // Do not need to reset gServerProxy here, because in InitAndGetAudioServiceProxy(), proxy object
    // will be reset when object is dead. Mutex is not needed either.
    DelayedSingleton<AudioEngineClientManager>::GetInstance()->InitAndGetAudioServiceProxy();
}

} // namespace AudioStandard
} // namespace OHOS
