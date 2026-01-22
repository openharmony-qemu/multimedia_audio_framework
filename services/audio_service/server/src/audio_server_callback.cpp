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
#define LOG_TAG "AudioServer"
#endif

#include "audio_server.h"

#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {

AudioServer::CallbackHandle::CallbackHandle(AudioServer *owner,
    const sptr<IAudioEngineCallbackHandle> &cbHandle, pid_t pid, uid_t uid)
    : owner_(owner), cbHandle_(cbHandle), pid_(pid), uid_(uid)
{
    // Set all id enable state to false, avoid to find id later
    for (size_t i = 0; i < CALLBACK_ID_MAX; ++i) {
        cbEnableMap_[static_cast<AudioEngineCallbackId>(i)] = false;
    }
}

void AudioServer::CallbackHandle::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<CallbackHandle> keep(this);
    owner_->RemoveCallbackHandle(pid_);
    (void)(uid_);
}

void AudioServer::CallbackHandle::SetCallbackHandleEnable(uint32_t callbackId, bool enable)
{
    std::lock_guard<std::mutex> lock(lock_);
    cbEnableMap_[static_cast<AudioEngineCallbackId>(callbackId)] = enable;
}

void AudioServer::CallbackHandle::OnOutputPipeChange(AudioPipeChangeType changeType,
    std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN(cbHandle_ && cbEnableMap_[CALLBACK_OUTPUT_PIPE_CHANGE]);
    cbHandle_->OnOutputPipeChange(static_cast<int32_t>(changeType), changedPipeInfo);
}

void AudioServer::CallbackHandle::OnInputPipeChange(AudioPipeChangeType changeType,
    std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN(cbHandle_ && cbEnableMap_[CALLBACK_INPUT_PIPE_CHANGE]);
    cbHandle_->OnInputPipeChange(static_cast<int32_t>(changeType), changedPipeInfo);
}

void AudioServer::OutputChangeCallbackAction::Exec()
{
    owner_->DispatchOutputPipeChangeEvent(changeType_, changedPipeInfo_);
}

void AudioServer::InputChangeCallbackAction::Exec()
{
    owner_->DispatchInputPipeChangeEvent(changeType_, changedPipeInfo_);
}

int32_t AudioServer::RegisterCallbackHandle(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "remote object invalid");
    sptr<IAudioEngineCallbackHandle> handle = iface_cast<IAudioEngineCallbackHandle>(object);
    CHECK_AND_RETURN_RET_LOG(handle != nullptr, ERR_INVALID_PARAM, "remote object invalid when cast interface");
    pid_t pid = IPCSkeleton::GetCallingPid();
    uid_t uid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    {
        std::lock_guard<std::mutex> lock(cbLock_);
        CHECK_AND_RETURN_RET_LOG(cbHandles_.find(pid) == cbHandles_.end(),
            ERR_INVALID_OPERATION, "do not allow register twice");
        sptr<CallbackHandle> cbHandle = new CallbackHandle(this, handle, pid, uid);
        CHECK_AND_RETURN_RET_LOG(cbHandle != nullptr, ERR_NO_MEMORY, "create handle failed");
        cbHandles_[pid] = cbHandle;
        object->AddDeathRecipient(cbHandle);
    }
    return SUCCESS;
}

void AudioServer::RemoveCallbackHandle(pid_t pid)
{
    std::lock_guard<std::mutex> lock(cbLock_);
    cbHandles_.erase(pid);

    // Note: stream death handler can also move to here later
}

int32_t AudioServer::SetCallbackHandleEnable(uint32_t callbackId, bool enable)
{
    CHECK_AND_RETURN_RET_LOG(callbackId < CALLBACK_ID_MAX, ERR_INVALID_PARAM, "callback id invalid");

    std::lock_guard<std::mutex> lock(cbLock_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_RET_LOG(cbHandles_.find(pid) != cbHandles_.end(), ERR_ILLEGAL_STATE, "not register client yet");
    cbHandles_[pid]->SetCallbackHandleEnable(callbackId, enable);
    return SUCCESS;
}

// IAudioSinkCallback Callback implement, called from HdiAdapterManager internal
void AudioServer::OnOutputPipeChange(AudioPipeChangeType changeType,
    std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
{
    CHECK_AND_RETURN_LOG(asyncHandler_ != nullptr, "async handler not inited");
    auto action = std::make_shared<OutputChangeCallbackAction>(this, changeType, changedPipeInfo);
    AsyncActionHandler::AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
    asyncHandler_->PostAsyncAction(desc);
}

// IAudioSourceCallback Callback implement, called from HdiAdapterManager internal
void AudioServer::OnInputPipeChange(AudioPipeChangeType changeType,
    std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
{
    CHECK_AND_RETURN_LOG(asyncHandler_ != nullptr, "async handler not inited");
    auto action = std::make_shared<InputChangeCallbackAction>(this, changeType, changedPipeInfo);
    AsyncActionHandler::AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
    asyncHandler_->PostAsyncAction(desc);
}

void AudioServer::DispatchOutputPipeChangeEvent(AudioPipeChangeType changeType,
    std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
{
    std::lock_guard<std::mutex> lock(cbLock_);
    for (auto &iter : cbHandles_) {
        iter.second->OnOutputPipeChange(changeType, changedPipeInfo);
    }
}

void AudioServer::DispatchInputPipeChangeEvent(AudioPipeChangeType changeType,
    std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
{
    std::lock_guard<std::mutex> lock(cbLock_);
    for (auto &iter : cbHandles_) {
        iter.second->OnInputPipeChange(changeType, changedPipeInfo);
    }
}

int32_t AudioServer::GetCurrentOutputPipeChangeInfos(
    std::vector<std::shared_ptr<AudioOutputPipeInfo>> &pipeChangeInfos)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    int32_t status = manager.GetCurrentOutputPipeChangeInfos(pipeChangeInfos);
    return status;
}

int32_t AudioServer::GetCurrentInputPipeChangeInfos(
    std::vector<std::shared_ptr<AudioInputPipeInfo>> &pipeChangeInfos)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    int32_t status = manager.GetCurrentInputPipeChangeInfos(pipeChangeInfos);
    return status;
}

} // namespace AudioStandard
} // namespace OHOS
