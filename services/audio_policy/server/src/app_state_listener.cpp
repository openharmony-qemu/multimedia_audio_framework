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
#define LOG_TAG "AudioServiceAppStateListener"
#endif

#include "app_state_listener.h"
#include "audio_policy_server.h"

namespace OHOS {
namespace AudioStandard {

AppStateListener::AppStateListener(std::weak_ptr<AudioPolicyServer> audioPolicyServer)
    : audioPolicyServer_(audioPolicyServer)
{
    AUDIO_INFO_LOG("enter");
}

void AppStateListener::OnAppStateChanged(const AppExecFwk::AppProcessData& appProcessData)
{
    std::shared_ptr<AudioPolicyServer> policyServer = audioPolicyServer_.lock();
    CHECK_AND_RETURN_LOG(policyServer, "audioPolicyServer is null");

    for (const auto& appData : appProcessData.appDatas) {
        AUDIO_INFO_LOG("bundleName=%{public}s uid=%{public}d pid=%{public}d state=%{public}d",
            appData.appName.c_str(), appData.uid, appProcessData.pid, appProcessData.appState);
        policyServer->NotifyAppStateChanged(appData.uid, appProcessData.pid,
            static_cast<int32_t>(appProcessData.appState));
    }
}
}
}
