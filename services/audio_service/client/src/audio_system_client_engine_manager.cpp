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

#include "audio_system_client_engine_manager.h"

#include "audio_service_proxy.h"
#include "audio_log.h"
#include "ipc_skeleton.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioSystemClientEngineManager &AudioSystemClientEngineManager::GetInstance()
{
    static AudioSystemClientEngineManager instance;
    return instance;
}

int32_t AudioSystemClientEngineManager::IsWhispering()
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t whisperRes = 0;
    gasp->IsWhispering(whisperRes);
    return whisperRes;
}

const std::string AudioSystemClientEngineManager::GetAudioParameter(const std::string key)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, "", "Audio service unavailable.");
    std::string value = "";
    gasp->GetAudioParameter(key, value);
    return value;
}

void AudioSystemClientEngineManager::SetAudioParameter(const std::string &key, const std::string &value)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_LOG(gasp != nullptr, "Audio service unavailable.");
    gasp->SetAudioParameter(key, value);
}

int32_t AudioSystemClientEngineManager::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    std::vector<StringPair> resultPair;
    int32_t ret = gasp->GetExtraParameters(mainKey, subKeys, resultPair);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "Get extra parameters failed");
    for (auto &pair : resultPair) {
        result.push_back(std::make_pair(pair.firstParam, pair.secondParam));
    }
    return ret;
}

int32_t AudioSystemClientEngineManager::SetExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    std::vector<StringPair> pairs;
    for (const auto &pair : kvpairs) {
        pairs.push_back({pair.first, pair.second});
    }
    return gasp->SetExtraParameters(key, pairs);
}

uint64_t AudioSystemClientEngineManager::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    uint64_t transactionId = 0;
    int32_t res = gasp->GetTransactionId(deviceType, deviceRole, transactionId);
    CHECK_AND_RETURN_RET_LOG(res == 0, 0, "GetTransactionId failed");
    return transactionId;
}

void AudioSystemClientEngineManager::SetAudioMonoState(bool monoState)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_LOG(gasp != nullptr, "Audio service unavailable.");
    gasp->SetAudioMonoState(monoState);
}

void AudioSystemClientEngineManager::SetAudioBalanceValue(float balanceValue)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_LOG(gasp != nullptr, "Audio service unavailable.");
    gasp->SetAudioBalanceValue(balanceValue);
}

int32_t AudioSystemClientEngineManager::SetRenderWhitelist(std::vector<std::string> list)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gasp->SetRenderWhitelist(list);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

uint32_t AudioSystemClientEngineManager::GetEffectLatency(const std::string &sessionId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    uint32_t latency = 0;
    int32_t res = gasp->GetEffectLatency(sessionId, latency);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "GetEffectLatency failed");
    return latency;
}

int32_t AudioSystemClientEngineManager::SetForegroundList(std::vector<std::string> list)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gasp->SetForegroundList(list);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

int32_t AudioSystemClientEngineManager::GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    int32_t ret = gasp->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioSystemClientEngineManager::CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    int32_t ret = gasp->CheckCaptureLimit(config, innerCapId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

int32_t AudioSystemClientEngineManager::ReleaseCaptureLimit(int32_t innerCapId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    int32_t ret = gasp->ReleaseCaptureLimit(innerCapId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}
#endif

int32_t AudioSystemClientEngineManager::GenerateSessionId(uint32_t &sessionId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t ret = gasp->GenerateSessionId(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get sessionId failed");
    return 0;
}
} // namespace AudioStandard
} // namespace OHOS
