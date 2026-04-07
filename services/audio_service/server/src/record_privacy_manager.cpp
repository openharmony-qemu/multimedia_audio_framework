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
#include "audio_privacy_manager_client.h"
 
namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TIME_OUT_SECONDS = 10;
constexpr int32_t UID_AUDIO = 1041;
constexpr int32_t UID_MSDP_SA = 6699;
constexpr int32_t UID_INTELLIGENT_VOICE_SA = 1042;
constexpr int32_t UID_CAAS_SA = 5527;
constexpr int32_t UID_DISTRIBUTED_AUDIO_SA = 3055;
constexpr int32_t UID_TELEPHONY_SA = 1001;
constexpr int32_t UID_DMSDP_SA = 7071;
 
std::set<int32_t> RECORD_PRIVACY_BLACK_LIST = {
#ifdef AUDIO_BUILD_VARIANT_ROOT
    0, // UID_ROOT
#endif
    UID_MSDP_SA,
    UID_INTELLIGENT_VOICE_SA,
    UID_CAAS_SA,
    UID_DISTRIBUTED_AUDIO_SA,
    UID_AUDIO,
    UID_TELEPHONY_SA, // used in distributed communication call
    UID_DMSDP_SA
};
}// namespace
 
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
    CHECK_AND_RETURN_RET_LOG(AudioPrivacyManagerClient::GetInstance().StartUsingMicrophone(targetTokenId), false,
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
    CHECK_AND_RETURN_RET_LOG(AudioPrivacyManagerClient::GetInstance().StopUsingMicrophone(targetTokenId), false,
        "StopUsingMicrophone failed for tokenId:%{public}u sessionId:%{public}u", targetTokenId, sessionId);
    return true;
}
 
bool RecordPrivacyManager::NeedTurnOnMicIndicator(int32_t callingUid) const
{
    if (RECORD_PRIVACY_BLACK_LIST.count(callingUid)) {
        AUDIO_INFO_LOG("internal sa(%{public}d) user directly recording", callingUid);
        return false;
    }
    return true;
}
} // namespace AudioStandard
} // namespace OHOS