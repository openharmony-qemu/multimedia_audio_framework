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
#define LOG_TAG "ClientTypeManagerHandler"
#endif

#include "client_type_manager_handler.h"

namespace OHOS {
namespace AudioStandard {
ClientTypeManagerHandler::ClientTypeManagerHandler() : AppExecFwk::EventHandler(
    AppExecFwk::EventRunner::Create("OS_ClientTypeAsyncRunner", AppExecFwk::ThreadMode::FFRT))
{
    AUDIO_INFO_LOG("ctor");
}

ClientTypeManagerHandler::~ClientTypeManagerHandler()
{
    AUDIO_WARNING_LOG("dtor should not happen");
};

void ClientTypeManagerHandler::RegisterClientTypeListener(ClientTypeListener *clientTypeListener)
{
    AUDIO_INFO_LOG("In");
    clientTypeListener_ = clientTypeListener;
}

bool ClientTypeManagerHandler::SendGetClientType(const std::string &bundleName, uint32_t uid)
{
#ifdef FEATURE_APPGALLERY
    auto eventContextObj = std::make_shared<EventObj>();
    CHECK_AND_RETURN_RET_LOG(eventContextObj != nullptr, false, "Event obj is null");
    eventContextObj->bundleName = bundleName;
    eventContextObj->uid = uid;

    std::lock_guard<std::mutex> runnerlock(runnerMutex_);
    bool ret = true;
    ret = SendEvent(AppExecFwk::InnerEvent::Get(EventClientTypeManagerCmd::GET_CLIENT_TYPE, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "Send event failed");
    return ret;
#else
    return true;
#endif
}

void ClientTypeManagerHandler::SetQueryClientTypeCallback(const sptr<IStandardAudioPolicyManagerListener> &callback)
{
    AUDIO_INFO_LOG("In");
    std::lock_guard<ffrt::mutex> lock(callbackMutex_);
    if (queryClientTypeCallback_ != nullptr) {
        AUDIO_INFO_LOG("Already register");
        return;
    }
    queryClientTypeCallback_ = callback;
}

void ClientTypeManagerHandler::HandleGetClientType(const AppExecFwk::InnerEvent::Pointer &event)
{
    AUDIO_INFO_LOG("In");
#ifdef FEATURE_APPGALLERY
    std::shared_ptr<EventObj> eventContextObj = event->GetSharedObject<EventObj>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventtObj get nullptr");
    std::string bundleName = eventContextObj->bundleName;
    uint32_t uid = eventContextObj->uid;

    if (bundleName == "" || uid == 0) {
        AUDIO_ERR_LOG("bundle name: %{public}s, uid: %{public}u", bundleName.c_str(), uid);
        return;
    }

    std::unique_lock<ffrt::mutex> callbackLock(callbackMutex_);
    if (queryClientTypeCallback_ == nullptr) {
        AUDIO_WARNING_LOG("Query callback is not inited");
        return;
    }
    bool isGame = queryClientTypeCallback_->OnQueryClientType(bundleName, uid);
    callbackLock.unlock();
    if (isGame) {
        AUDIO_INFO_LOG("%{public}u is game type", uid);
        clientTypeListener_->OnClientTypeQueryCompleted(uid, CLIENT_TYPE_GAME);
    } else {
        AUDIO_INFO_LOG("%{public}u not is game type", uid);
        clientTypeListener_->OnClientTypeQueryCompleted(uid, CLIENT_TYPE_OTHERS);
    }
#endif
}

void ClientTypeManagerHandler::ReleaseEventRunner()
{
    AUDIO_INFO_LOG("release all events");
    SetEventRunner(nullptr);
}

void ClientTypeManagerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    AUDIO_DEBUG_LOG("handler process eventId:%{public}u", eventId);

    switch (eventId) {
        case EventClientTypeManagerCmd::GET_CLIENT_TYPE:
            HandleGetClientType(event);
            break;
        default:
            break;
    }
}
} // namespace AudioStandard
} // namespace OHOS
