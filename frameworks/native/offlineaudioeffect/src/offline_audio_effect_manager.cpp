/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "OfflineAudioEffectManager"
#endif

#include "offline_audio_effect_manager.h"

#include <securec.h>

#include "audio_errors.h"
#include "audio_service_log.h"
#include "offline_audio_effect_chain_impl.h"

namespace OHOS {
namespace AudioStandard {
std::vector<std::string> OfflineAudioEffectManager::GetOfflineAudioEffectChains()
{
    std::vector<std::string> effectChains{};
    int32_t ret = OfflineStreamInClient::GetOfflineAudioEffectChains(effectChains);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, effectChains, "Get chains failed, errcode is %{public}d", ret);
    return effectChains;
}

std::unique_ptr<OfflineAudioEffectChain> OfflineAudioEffectManager::CreateOfflineAudioEffectChain(
    const std::string &chainName)
{
    std::unique_ptr<OfflineAudioEffectChainImpl> chain = std::make_unique<OfflineAudioEffectChainImpl>(chainName);
    int32_t ret = chain->CreateEffectChain();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "create OfflineEffectChain failed, errcode is %{public}d", ret);
    return chain;
}
} // namespace AudioStandard
} // namespace OHOS
