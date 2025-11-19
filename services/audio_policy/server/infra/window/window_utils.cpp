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
#define LOG_TAG "WindowUtils"

#include "window_utils.h"

#include "audio_log.h"
#include "audio_errors.h"
#include "session_manager_lite.h"
#include "window_manager_lite.h"

namespace OHOS {
namespace AudioStandard {
const uint64_t VIRTUAL_DISPLAY_ID = 1000;

int32_t WindowUtils::CheckVirtualDisplay(const int32_t pid, bool &isVirtualDisplay)
{
    std::vector<sptr<Rosen::WindowVisibilityInfo>> infos;
    Rosen::WMError ret = Rosen::WindowManagerLite::GetInstance().GetVisibilityWindowInfo(infos);
    if (ret != Rosen::WMError::WM_OK) {
        AUDIO_INFO_LOG("GetVisibilityWindowInfo failed, ret: %{public}d.", ret);
        return ERROR;
    }
    std::vector<uint64_t> windowIds;
    uint64_t windowId = 0;
    for (auto& window : infos) {
        if (window != nullptr && window->pid_ == pid) {
            windowId = window->windowId_;
            windowIds.push_back(window->windowId_);
            break;
        }
    }
    std::unordered_map<uint64_t, Rosen::DisplayId> windowDisplayIdMap;
    auto res = Rosen::WindowManagerLite::GetInstance().GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    if (res != Rosen::WMError::WM_OK) {
        AUDIO_INFO_LOG("GetDisplayIdByWindowId failed, ret: %{public}d", res);
        return ERROR;
    }
    uint64_t displayId = 0;
    for (auto& window : windowDisplayIdMap) {
        if (window.first == windowId) {
            displayId = window.second;
            break;
        }
    }
    AUDIO_INFO_LOG("CheckVirtualDisplay displayId :%{public}d.", static_cast<int>(displayId));
    if (displayId >= VIRTUAL_DISPLAY_ID) {
        isVirtualDisplay = true;
    }
    return SUCCESS;
}

bool WindowUtils::CheckWindowState(const int32_t pid)
{
    AUDIO_INFO_LOG("CheckWindowState pid:%{public}d", pid);
    bool isVirtualDisplay = false;
    int32_t checkRet = CheckVirtualDisplay(pid, isVirtualDisplay);
    if (checkRet != SUCCESS) {
        AUDIO_INFO_LOG("CheckVirtualDisplay failed");
        return false;
    }
    if (isVirtualDisplay) {
        AUDIO_INFO_LOG("CheckWindowState pid:%{public}d is on virtual display", pid);
        return false;
    }
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (sceneSessionManager == nullptr) {
        AUDIO_INFO_LOG("AudioInterruptService null manager");
        return false;
    }
    std::vector<Rosen::MainWindowState> windowStates;
    Rosen::WSError ret = sceneSessionManager->GetMainWindowStatesByPid(pid, windowStates);
    if (ret != Rosen::WSError::WS_OK || windowStates.empty()) {
        AUDIO_INFO_LOG("AudioInterruptService fail GetWindow");
        return false;
    }
    for (auto &windowState : windowStates) {
        if (windowState.isVisible_ && (windowState.state_ == (int32_t) Rosen::SessionState::STATE_ACTIVE ||
            windowState.state_ == (int32_t) Rosen::SessionState::STATE_FOREGROUND)) {
            AUDIO_INFO_LOG("AudioInterruptService app window front desk,"
                " windowState.state_ = %{public}d", windowState.state_);
            return true;
        }
    }
    return false;
}
}
}
#endif