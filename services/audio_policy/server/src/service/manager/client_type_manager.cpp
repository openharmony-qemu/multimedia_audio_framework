/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "ClientTypeManager"
#endif

#include "audio_policy_log.h"
#include "client_type_manager.h"

namespace OHOS {
namespace AudioStandard {
ClientTypeManager *ClientTypeManager::GetInstance()
{
    static ClientTypeManager clientTypeManager;
    return &clientTypeManager;
}

void ClientTypeManager::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
    AUDIO_INFO_LOG("uid: %{public}u, bundle name %{public}s", uid, bundleName.c_str());
#ifdef FEATURE_APPGALLERY
    std::unique_lock<std::mutex> handlerLock(handlerMutex_);
    if (clientTypeManagerHandler_ == nullptr) {
        AUDIO_INFO_LOG("Init client type manager");
        clientTypeManagerHandler_ = std::make_shared<ClientTypeManagerHandler>();
        if (clientTypeManagerHandler_ != nullptr) {
            clientTypeManagerHandler_->RegisterClientTypeListener(this);
        }
    }
    handlerLock.unlock();

    std::unique_lock<std::mutex> lock(clientTypeMapMutex_);
    auto it = clientTypeMap_.find(uid);
    if (it != clientTypeMap_.end()) {
        AUDIO_INFO_LOG("Uid already in map");
        return;
    }
    lock.unlock();
    if (bundleName == "" || uid == 0) {
        AUDIO_WARNING_LOG("Get bundle name %{public}s for %{public}u failed", bundleName.c_str(), uid);
        return;
    }
    clientTypeManagerHandler_->SendGetClientType(bundleName.c_str(), uid);
#else
    AUDIO_WARNING_LOG("Get client type is not supported");
#endif
}

void ClientTypeManager::SetQueryClientTypeCallback(const sptr<IStandardAudioPolicyManagerListener> &callback)
{
    AUDIO_INFO_LOG("In");
#ifdef FEATURE_APPGALLERY
    std::lock_guard<std::mutex> handlerLock(handlerMutex_);
    if (clientTypeManagerHandler_ == nullptr) {
        AUDIO_INFO_LOG("Init client type manager");
        clientTypeManagerHandler_ = std::make_shared<ClientTypeManagerHandler>();
        if (clientTypeManagerHandler_ != nullptr) {
            clientTypeManagerHandler_->RegisterClientTypeListener(this);
        }
    }
    clientTypeManagerHandler_->SetQueryClientTypeCallback(callback);
#endif
}

ClientType ClientTypeManager::GetClientTypeByUid(uint32_t uid)
{
    std::lock_guard<std::mutex> lock(clientTypeMapMutex_);
    AUDIO_INFO_LOG("uid %{public}u", uid);
    auto it = clientTypeMap_.find(uid);
    if (it == clientTypeMap_.end()) {
        AUDIO_INFO_LOG("Cannot find uid");
        return CLIENT_TYPE_OTHERS;
    }
    return it->second;
}

void ClientTypeManager::OnClientTypeQueryCompleted(uint32_t uid, ClientType clientType)
{
    std::lock_guard<std::mutex> lock(clientTypeMapMutex_);
    AUDIO_INFO_LOG("uid: %{public}u, client type: %{public}d", uid, clientType);
    clientTypeMap_.insert_or_assign(uid, clientType);
}
} // namespace AudioStandard
} // namespace OHOS
