/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPrivacyManagerInServer"
#endif

#include <algorithm>
#include <set>
#include "audio_privacy_manager_in_server.h"

#include "audio_info.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "privacy_error.h"
#include "privacy_kit.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TIME_OUT_SECONDS = 10;
}

AudioPrivacyManagerInServer &AudioPrivacyManagerInServer::GetInstance()
{
    static AudioPrivacyManagerInServer instance;
    return instance;
}

bool AudioPrivacyManagerInServer::StartUsingMicrophone(const std::string &ancoPkgName)
{
    AudioXCollie audioXCollie("AudioPrivacyManagerInServer::StartUsingMicrophone", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    Trace trace("PrivacyKit::StartUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StartUsingPermission ancoPkgName:%{public}s permission:%{public}s",
        ancoPkgName.c_str(), MICROPHONE_PERMISSION);
    WatchTimeout startGuard("PrivacyKit::StartUsingPermission:AudioPrivacyManagerInServer::StartUsingMicrophone");
    int32_t res = Security::AccessToken::PrivacyKit::StartUsingPermission(ancoPkgName, MICROPHONE_PERMISSION);
    startGuard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0 || res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING, false,
        "StartUsingPermission for ancoPkgName:%{public}s, PrivacyKit error code:%{public}d", ancoPkgName.c_str(), res);
    return true;
}

bool AudioPrivacyManagerInServer::StartUsingMicrophone(uint32_t targetTokenId)
{
    AudioXCollie audioXCollie("AudioPrivacyManagerInServer::StartUsingMicrophone", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    Trace trace("PrivacyKit::StartUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StartUsingPermission tokenId:%{public}u permission:%{public}s",
        targetTokenId, MICROPHONE_PERMISSION);
    WatchTimeout startGuard("PrivacyKit::StartUsingPermission:AudioPrivacyManagerInServer::StartUsingMicrophone");
    int32_t res = Security::AccessToken::PrivacyKit::StartUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
    startGuard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0 || res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING, false,
        "StartUsingPermission for tokenId:%{public}u, PrivacyKit error code:%{public}d", targetTokenId, res);
    if (res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
        AUDIO_WARNING_LOG("The PrivacyKit return ERR_PERMISSION_ALREADY_START_USING error code:%{public}d", res);
        return true;
    }

    WatchTimeout recordGuard("Security::AccessToken::PrivacyKit::AddPermissionUsedRecord:"
        "AudioPrivacyManagerInServer::StartUsingMicrophone");
    res = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(targetTokenId, MICROPHONE_PERMISSION, 1, 0);
    recordGuard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0, false, "AddPermissionUsedRecord for tokenId %{public}u!"
        "The PrivacyKit error code:%{public}d", targetTokenId, res);
    return true;
}

bool AudioPrivacyManagerInServer::StopUsingMicrophone(const std::string &ancoPkgName)
{
    AudioXCollie audioXCollie("AudioPrivacyManagerInServer::StopUsingMicrophone", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    Trace trace("PrivacyKit::StopUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StopUsingPermission ancoPkgName:%{public}s permission:%{public}s",
        ancoPkgName.c_str(), MICROPHONE_PERMISSION);
    WatchTimeout guard("PrivacyKit::StopUsingPermission:AudioPrivacyManagerInServer::StopUsingMicrophone");
    int32_t res = Security::AccessToken::PrivacyKit::StopUsingPermission(ancoPkgName, MICROPHONE_PERMISSION);
    guard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0, false, "StopUsingPermission for ancoPkgName %{public}s!"
        "The PrivacyKit error code:%{public}d", ancoPkgName.c_str(), res);
    return true;
}

bool AudioPrivacyManagerInServer::StopUsingMicrophone(uint32_t targetTokenId)
{
    AudioXCollie audioXCollie("AudioPrivacyManagerInServer::StopUsingMicrophone", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    Trace trace("PrivacyKit::StopUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StopUsingPermission tokenId:%{public}u permission:%{public}s",
        targetTokenId, MICROPHONE_PERMISSION);
    WatchTimeout guard("PrivacyKit::StopUsingPermission:AudioPrivacyManagerInServer::StopUsingMicrophone");
    int32_t res = Security::AccessToken::PrivacyKit::StopUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
    guard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0, false, "StopUsingPermission for tokenId %{public}u!"
        "The PrivacyKit error code:%{public}d", targetTokenId, res);
    return true;
}

bool AudioPrivacyManagerInServer::UpdateActiveAncoCapturer(const std::vector<std::string> &bundleNameList)
{
    std::lock_guard<std::mutex> lock(activeCaptureMutex_);
    std::set<std::string> currentList;
    for (const auto &bundleName : bundleNameList) {
        CHECK_AND_CONTINUE(!bundleName.empty());
        currentList.insert(bundleName);
    }

    for (const auto &bundleName : currentList) {
        CHECK_AND_CONTINUE(lastAncoBundlenameList_.find(bundleName) == lastAncoBundlenameList_.end());
        AUDIO_INFO_LOG("New app added: %{public}s, calling StartUsingMicrophone", bundleName.c_str());
        bool ret = StartUsingMicrophone(bundleName);
        CHECK_AND_RETURN_RET_LOG(ret, false,
            "StartUsingMicrophone failed for bundleName:%{public}s", bundleName.c_str());
        startUsingMicFlag_ = true;
    }

    for (const auto &bundleName : lastAncoBundlenameList_) {
        CHECK_AND_CONTINUE(currentList.find(bundleName) == currentList.end());
        AUDIO_INFO_LOG("App removed: %{public}s, calling StopUsingMicrophone", bundleName.c_str());
        bool ret = StopUsingMicrophone(bundleName);
        CHECK_AND_RETURN_RET_LOG(ret, false,
            "StopUsingMicrophone failed for bundleName:%{public}s", bundleName.c_str());
    }

    lastAncoBundlenameList_ = currentList;
    AUDIO_INFO_LOG("UpdateActiveCaptureList done, current list size:%{public}zu", currentList.size());
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
