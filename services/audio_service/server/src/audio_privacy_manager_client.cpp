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
#define LOG_TAG "AudioPrivacyManagerClient"
#endif

#include "audio_privacy_manager_client.h"

#include "audio_info.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "privacy_error.h"
#include "privacy_kit.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TIME_OUT_SECONDS = 10;
}

AudioPrivacyManagerClient &AudioPrivacyManagerClient::GetInstance()
{
    static AudioPrivacyManagerClient instance;
    return instance;
}

bool AudioPrivacyManagerClient::StartUsingMicrophone(uint32_t targetTokenId)
{
    AudioXCollie audioXCollie("AudioPrivacyManagerClient::StartUsingMicrophone", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    Trace trace("PrivacyKit::StartUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StartUsingPermission tokenId:%{public}u permission:%{public}s",
        targetTokenId, MICROPHONE_PERMISSION);
    WatchTimeout startGuard("PrivacyKit::StartUsingPermission:AudioPrivacyManagerClient::StartUsingMicrophone");
    int32_t res = Security::AccessToken::PrivacyKit::StartUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
    startGuard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0 || res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING, false,
        "StartUsingPermission for tokenId:%{public}u, PrivacyKit error code:%{public}d", targetTokenId, res);
    if (res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
        AUDIO_ERR_LOG("The PrivacyKit return ERR_PERMISSION_ALREADY_START_USING error code:%{public}d", res);
    }

    WatchTimeout recordGuard("Security::AccessToken::PrivacyKit::AddPermissionUsedRecord:"
        "AudioPrivacyManagerClient::StartUsingMicrophone");
    res = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(targetTokenId, MICROPHONE_PERMISSION, 1, 0);
    recordGuard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0, false, "AddPermissionUsedRecord for tokenId %{public}u!"
        "The PrivacyKit error code:%{public}d", targetTokenId, res);
    return true;
}

bool AudioPrivacyManagerClient::StopUsingMicrophone(uint32_t targetTokenId)
{
    AudioXCollie audioXCollie("AudioPrivacyManagerClient::StopUsingMicrophone", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    Trace trace("PrivacyKit::StopUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StopUsingPermission tokenId:%{public}u permission:%{public}s",
        targetTokenId, MICROPHONE_PERMISSION);
    WatchTimeout guard("PrivacyKit::StopUsingPermission:AudioPrivacyManagerClient::StopUsingMicrophone");
    int32_t res = Security::AccessToken::PrivacyKit::StopUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
    guard.CheckCurrTimeout();
    CHECK_AND_RETURN_RET_LOG(res == 0, false, "StopUsingPermission for tokenId %{public}u!"
        "The PrivacyKit error code:%{public}d", targetTokenId, res);
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
