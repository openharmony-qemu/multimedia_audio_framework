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

#include <map>

#include "audio_common_log.h"
#include "app_state_listener.h"
#include "dfx_msg_manager.h"
#include "audio_utils.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "audio_background_manager.h"
#include "iservice_registry.h"
#include "audio_bundle_manager.h"
#include "audio_stream_monitor.h"
#include "audio_session_service.h"
#include "audio_usr_select_manager.h"


namespace OHOS {
namespace AudioStandard {

static const std::map<AppExecFwk::ApplicationState, DfxAppState> DFX_APPSTATE_MAP = {
    {AppExecFwk::ApplicationState::APP_STATE_CREATE, DFX_APP_STATE_START},
    {AppExecFwk::ApplicationState::APP_STATE_FOREGROUND, DFX_APP_STATE_FOREGROUND},
    {AppExecFwk::ApplicationState::APP_STATE_BACKGROUND, DFX_APP_STATE_BACKGROUND},
    {AppExecFwk::ApplicationState::APP_STATE_END, DFX_APP_STATE_END},
    {AppExecFwk::ApplicationState::APP_STATE_TERMINATED, DFX_APP_STATE_END}
};

static const std::map<AppExecFwk::ApplicationState, AppIsBackState> BACKGROUND_APPSTATE_MAP = {
    {AppExecFwk::ApplicationState::APP_STATE_FOREGROUND, STATE_FOREGROUND},
    {AppExecFwk::ApplicationState::APP_STATE_BACKGROUND, STATE_BACKGROUND},
    {AppExecFwk::ApplicationState::APP_STATE_END, STATE_END},
    {AppExecFwk::ApplicationState::APP_STATE_TERMINATED, STATE_END}
};
AppStateListener::AppStateListener()
{
    AUDIO_INFO_LOG("enter");
}

void AppStateListener::OnAppStateChanged(const AppExecFwk::AppProcessData& appProcessData)
{
    for (const auto& appData : appProcessData.appDatas) {
        AUDIO_INFO_LOG("app state changed, bundleName=%{public}s uid=%{public}d pid=%{public}d state=%{public}d",
            appData.appName.c_str(), appData.uid, appProcessData.pid, appProcessData.appState);
        HandleAppStateChange(appProcessData.pid, appData.uid, static_cast<int32_t>(appProcessData.appState));
        HandleBackgroundAppStateChange(appProcessData.pid, appData.uid, static_cast<int32_t>(appProcessData.appState));
        AudioStreamMonitor::GetInstance().NotifyAppStateChange(appData.uid,
            (appProcessData.appState == AppExecFwk::ApplicationState::APP_STATE_BACKGROUND));
    }

    OHOS::Singleton<AudioSessionService>::GetInstance().NotifyAppStateChange(appProcessData.pid,
        (appProcessData.appState == AppExecFwk::ApplicationState::APP_STATE_BACKGROUND));
}

void AppStateListener::HandleAppStateChange(int32_t pid, int32_t uid, int32_t state)
{
    auto pos = DFX_APPSTATE_MAP.find(static_cast<AppExecFwk::ApplicationState>(state));
    CHECK_AND_RETURN(pos != DFX_APPSTATE_MAP.end());
    auto appState = pos->second;

    auto &manager = DfxMsgManager::GetInstance();
    if (appState == DFX_APP_STATE_START) {
        if (manager.CheckCanAddAppInfo(uid)) {
            auto info = AudioBundleManager::GetBundleInfoFromUid(uid);
            manager.SaveAppInfo({uid, info.name, info.versionName});
        }
        manager.UpdateAppState(uid, appState, true);
    } else {
        manager.UpdateAppState(uid, appState);
    }
}

void AppStateListener::HandleBackgroundAppStateChange(int32_t pid, int32_t uid, int32_t state)
{
    auto pos = BACKGROUND_APPSTATE_MAP.find(static_cast<AppExecFwk::ApplicationState>(state));
    CHECK_AND_RETURN(pos != BACKGROUND_APPSTATE_MAP.end());
    auto appState = pos->second;

    AudioBackgroundManager::GetInstance().NotifyAppStateChange(uid, pid, appState);
    AudioUsrSelectManager::GetAudioUsrSelectManager().UpdateAppIsBackState(uid, appState);
}
}
}
