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
#define LOG_TAG "RecordPrivacyManager"
#endif

#include "record_privacy_manager.h"

#include "audio_service_log.h"
#include "audio_utils.h"
#include "privacy_manager_service.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TIME_OUT_SECONDS = 10;
}

RecordPrivacyManager &RecordPrivacyManager::GetInstance()
{
    static RecordPrivacyManager instance;
    return instance;
}

bool RecordPrivacyManager::NotifyPrivacyStart(uint32_t targetTokenId, uint32_t sessionId)
{
    AudioXCollie audioXCollie("RecordPrivacyManager::NotifyPrivacyStart", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::lock_guard<std::mutex> lock(recordMapMutex_);
    if (tokenIdRecordMap_.count(targetTokenId)) {
        if (!tokenIdRecordMap_[targetTokenId].count(sessionId)) {
            tokenIdRecordMap_[targetTokenId].emplace(sessionId);
        } else {
            AUDIO_WARNING_LOG("this stream %{public}u is already running, no need call start", sessionId);
        }
        return true;
    }

    AUDIO_INFO_LOG("Notify PrivacyKit to display the microphone privacy indicator "
        "for tokenId: %{public}u sessionId:%{public}u", targetTokenId, sessionId);
    CHECK_AND_RETURN_RET_LOG(PrivacyManagerService::GetInstance().StartUsingMicrophone(targetTokenId), false,
        "StartUsingMicrophone failed for tokenId:%{public}u sessionId:%{public}u", targetTokenId, sessionId);
    tokenIdRecordMap_[targetTokenId] = {sessionId};
    return true;
}

bool RecordPrivacyManager::NotifyPrivacyStop(uint32_t targetTokenId, uint32_t sessionId)
{
    AudioXCollie audioXCollie("RecordPrivacyManager::NotifyPrivacyStop", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::unique_lock<std::mutex> lock(recordMapMutex_);
    if (!tokenIdRecordMap_.count(targetTokenId)) {
        AUDIO_INFO_LOG("this TokenId %{public}u is already not in using", targetTokenId);
        return true;
    }

    if (tokenIdRecordMap_[targetTokenId].count(sessionId)) {
        tokenIdRecordMap_[targetTokenId].erase(sessionId);
    }
    AUDIO_DEBUG_LOG("this TokenId %{public}u set size is %{public}zu!", targetTokenId,
        tokenIdRecordMap_[targetTokenId].size());
    if (!tokenIdRecordMap_[targetTokenId].empty()) {
        return true;
    }

    tokenIdRecordMap_.erase(targetTokenId);
    AUDIO_INFO_LOG("Notify PrivacyKit to remove the microphone privacy indicator "
        "for tokenId: %{public}u sessionId:%{public}u", targetTokenId, sessionId);
    CHECK_AND_RETURN_RET_LOG(PrivacyManagerService::GetInstance().StopUsingMicrophone(targetTokenId), false,
        "StopUsingMicrophone failed for tokenId:%{public}u sessionId:%{public}u", targetTokenId, sessionId);
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
