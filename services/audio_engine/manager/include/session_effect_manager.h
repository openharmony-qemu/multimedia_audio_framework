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

#ifndef SESSION_EFFECT_MANAGER_H
#define SESSION_EFFECT_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "effect_instance.h"
#include "hpae_define.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeManager;  // forward declaration

class SessionEffectManager {
public:
    explicit SessionEffectManager(HpaeManager &hpaeManager);
    ~SessionEffectManager();

    // Core interface
    int32_t BindSessionEffect(uint32_t sessionId, const std::string &effectName,
        const AudioEffectConfig &config, std::shared_ptr<EffectInstance> &outInstance,
        uint32_t *preemptedSessionId = nullptr);
    int32_t UnbindSessionEffect(uint32_t sessionId);
    void OnSessionDestroyed(uint32_t sessionId);

    // Lazy Release queries
    bool HasPendingEffectInstance(const std::string &effectName) const;
    std::shared_ptr<EffectInstance> GetPendingEffectInstance(const std::string &effectName) const;
    int32_t FlushPendingEffectInstance(const std::string &effectName);
    void SetLazyReleaseTimeout(uint64_t timeoutMs);
    void SetAudioEffectLibraryForTest(const AudioEffectLibrary *lib) { testLib_ = lib; }

    // Queries
    uint32_t GetBoundSessionId(const std::string &effectName) const;
    std::vector<std::string> GetAllActiveEffectNames() const;

    // Lifecycle
    void Deinit();

private:
    void LazyReleaseExpired(const std::string &effectName, uint32_t generation);
    std::shared_ptr<EffectInstance> FindBySessionId(uint32_t sessionId) const;
    int32_t TryReuseExistingInstance(const std::string &effectName, uint32_t sessionId,
        std::shared_ptr<EffectInstance> &outInstance, uint32_t *preemptedSessionId);
    int32_t CreateNewInstance(const std::string &effectName, uint32_t sessionId,
        const AudioEffectConfig &config, std::shared_ptr<EffectInstance> &outInstance);

    HpaeManager &hpaeManager_;
    std::map<std::string, std::shared_ptr<EffectInstance>> effectInstanceMap_;
    std::shared_ptr<bool> alive_;
    uint64_t lazyReleaseTimeoutMs_ = 3000;
    const AudioEffectLibrary *testLib_ = nullptr;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // SESSION_EFFECT_MANAGER_H
