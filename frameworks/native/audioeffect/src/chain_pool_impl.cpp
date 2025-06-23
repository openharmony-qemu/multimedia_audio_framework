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
#define LOG_TAG "ChainPoolImpl"
#endif

#include "chain_pool.h"

#include <cinttypes>
#include <map>
#include <mutex>

#include "audio_effect_log.h"
#include "audio_errors.h"
#include "nocopyable.h"

namespace OHOS {
namespace AudioStandard {
class ChainPoolImpl final : public ChainPool, public NoCopyable {
public:
    int32_t AddChain(const std::shared_ptr<AudioEnhanceChain> chain) override;

    int32_t DeleteChain(uint64_t chainId) override;

    std::shared_ptr<AudioEnhanceChain> GetChainById(uint64_t chainId) override;

    std::vector<std::shared_ptr<AudioEnhanceChain>> GetAllChain(void) override;

private:
    friend class ChainPool;
    ChainPoolImpl() = default;
    ~ChainPoolImpl() override = default;
    std::map<uint64_t, std::shared_ptr<AudioEnhanceChain>> chainMap_;
    std::mutex mutex_;
};

int32_t ChainPoolImpl::AddChain(const std::shared_ptr<AudioEnhanceChain> chain)
{
    CHECK_AND_RETURN_RET_LOG(chain != nullptr, ERROR, "chain is null");

    std::lock_guard<std::mutex> lock(mutex_);
    auto chainId = chain->GetChainId();
    if (chainMap_.find(chainId) != chainMap_.end()) {
        AUDIO_INFO_LOG("the chain has been added, Id: %{public}" PRIu64, chainId);
        return SUCCESS;
    }

    chainMap_.emplace(chainId, chain);
    AUDIO_INFO_LOG("add chain success, chainId: %{public}" PRIu64, chainId);

    return SUCCESS;
}

int32_t ChainPoolImpl::DeleteChain(uint64_t chainId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (chainMap_.erase(chainId) != 0) {
        AUDIO_INFO_LOG("delete chain success, chainId: %{public}" PRIu64, chainId);
        return SUCCESS;
    }

    AUDIO_ERR_LOG("chain is not found, chainId: %{public}" PRIu64, chainId);
    return ERROR;
}

std::shared_ptr<AudioEnhanceChain> ChainPoolImpl::GetChainById(uint64_t chainId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = chainMap_.find(chainId);
    if (iter == chainMap_.end()) {
        return nullptr;
    }

    return iter->second;
}

std::vector<std::shared_ptr<AudioEnhanceChain>> ChainPoolImpl::GetAllChain(void)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<AudioEnhanceChain>> chainArray;
    for (const auto &[id, chain] : chainMap_) {
        chainArray.emplace_back(chain);
    }

    return chainArray;
}

ChainPool &ChainPool::GetInstance()
{
    static ChainPoolImpl impl;
    return impl;
}
} // namespace AudioStandard
} // namespace OHOS