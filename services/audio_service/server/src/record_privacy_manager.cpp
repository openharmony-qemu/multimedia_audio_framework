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
#include "audio_privacy_manager_in_server.h"
 
namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TIME_OUT_SECONDS = 10;
constexpr int32_t UID_AUDIO = 1041;
constexpr int32_t UID_MSDP_SA = 6699;
constexpr int32_t UID_INTELLIGENT_VOICE_SA = 1042;
constexpr int32_t UID_CAAS_SA = 5527;
constexpr int32_t UID_D2D_SA = 7040;
constexpr int32_t UID_DISTRIBUTED_AUDIO_SA = 3055;
constexpr int32_t UID_TELEPHONY_SA = 1001;
constexpr int32_t UID_DMSDP_SA = 7071;
 
const std::set<int32_t> RECORD_PRIVACY_SKIP_UIDS = {
    UID_MSDP_SA,
    UID_INTELLIGENT_VOICE_SA,
    UID_CAAS_SA,
    UID_D2D_SA,
    UID_DISTRIBUTED_AUDIO_SA,
    UID_AUDIO,
    UID_TELEPHONY_SA, // used in distributed communication call
    UID_DMSDP_SA
};
} // namespace
 
RecordPrivacyManager &RecordPrivacyManager::GetInstance()
{
    static RecordPrivacyManager instance;
    return instance;
}
 
bool RecordPrivacyManager::StartUsingMicrophone(uint32_t targetTokenId) const
{
    return AudioPrivacyManagerInServer::GetInstance().StartUsingMicrophone(targetTokenId);
}

bool RecordPrivacyManager::StopUsingMicrophone(uint32_t targetTokenId) const
{
    return AudioPrivacyManagerInServer::GetInstance().StopUsingMicrophone(targetTokenId);
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
    CHECK_AND_RETURN_RET_LOG(StartUsingMicrophone(targetTokenId), false,
        "StartUsingMicrophone failed for tokenId:%{public}u sessionId:%{public}u", targetTokenId, sessionId);
    tokenIdRecordMap_[targetTokenId] = {sessionId};
    return true;
}
 
bool RecordPrivacyManager::NotifyPrivacyStop(uint32_t targetTokenId, uint32_t sessionId)
{
    AudioXCollie audioXCollie("RecordPrivacyManager::NotifyPrivacyStop", TIME_OUT_SECONDS,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::unique_lock<std::mutex> lock(recordMapMutex_);
    auto tokenIter = tokenIdRecordMap_.find(targetTokenId);
    if (tokenIter == tokenIdRecordMap_.end()) {
        AUDIO_INFO_LOG("this TokenId %{public}u is already not in using", targetTokenId);
        return true;
    }

    tokenIter->second.erase(sessionId);
    AUDIO_DEBUG_LOG("this TokenId %{public}u set size is %{public}zu!", targetTokenId,
        tokenIter->second.size());
    if (!tokenIter->second.empty()) {
        return true;
    }
    tokenIdRecordMap_.erase(tokenIter);

    AUDIO_INFO_LOG("Notify PrivacyKit to remove the microphone privacy indicator "
        "for tokenId: %{public}u sessionId:%{public}u", targetTokenId, sessionId);
    CHECK_AND_RETURN_RET_LOG(StopUsingMicrophone(targetTokenId), false,
        "StopUsingMicrophone failed for tokenId:%{public}u sessionId:%{public}u", targetTokenId, sessionId);
    return true;
}
 
bool RecordPrivacyManager::NeedTurnOnMicIndicator(int32_t callingUid, bool isLoopback, SourceType sourceType) const
{
    if (sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE || sourceType == SOURCE_TYPE_REMOTE_CAST ||
        sourceType == SOURCE_TYPE_VOICE_CALL || isLoopback) {
        AUDIO_INFO_LOG("sourceType(%{public}d) need not turn on mic indicator", sourceType);
        return false;
    }
    if (RECORD_PRIVACY_SKIP_UIDS.count(callingUid)) {
        AUDIO_INFO_LOG("internal sa(%{public}d) user directly recording", callingUid);
        return false;
    }
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
