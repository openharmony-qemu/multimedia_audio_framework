/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef CLIENT_TYPE_MANAGER_HANDLER_H
#define CLIENT_TYPE_MANAGER_HANDLER_H
#include <mutex>

#include "singleton.h"
#include "ffrt.h"
#include "event_handler.h"
#include "event_runner.h"

#include "audio_policy_log.h"
#include "audio_system_manager.h"
#include "istandard_audio_policy_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

enum ClientType {
    CLIENT_TYPE_OTHERS = 0,
    CLIENT_TYPE_GAME = 1,
};

class ClientTypeListener {
public:
    virtual ~ClientTypeListener() = default;

    virtual void OnClientTypeQueryCompleted(uint32_t uid, ClientType clientType) = 0;
};

class ClientTypeManagerHandler : public AppExecFwk::EventHandler {
public:
    ClientTypeManagerHandler();
    ~ClientTypeManagerHandler();

    void ReleaseEventRunner();

    enum EventClientTypeManagerCmd  {
        GET_CLIENT_TYPE,
    };

    class EventObj {
    public:
        std::string bundleName;
        uint32_t uid = 0;
    };

    void RegisterClientTypeListener(ClientTypeListener *clientTypeListener);
    bool SendGetClientType(const std::string &bundleName, uint32_t uid);

    void SetQueryClientTypeCallback(const sptr<IStandardAudioPolicyManagerListener> &callback);

protected:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    /* Handle Event*/
    void HandleGetClientType(const AppExecFwk::InnerEvent::Pointer &event);
    ClientTypeListener *clientTypeListener_ = nullptr;
    std::mutex runnerMutex_;

    ffrt::mutex callbackMutex_;
    sptr<IStandardAudioPolicyManagerListener> queryClientTypeCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CLIENT_TYPE_MANAGER_HANDLER_H
