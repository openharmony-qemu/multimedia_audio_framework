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
#define LOG_TAG "SessionEffectManager"
#endif

#include "session_effect_manager.h"
#include "hpae_manager.h"
#include "audio_utils.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

SessionEffectManager::SessionEffectManager(HpaeManager &hpaeManager)
    : hpaeManager_(hpaeManager), alive_(std::make_shared<bool>(true)) {}

SessionEffectManager::~SessionEffectManager()
{
    Deinit();
}

int32_t SessionEffectManager::TryReuseExistingInstance(const std::string &effectName,
    uint32_t sessionId, std::shared_ptr<EffectInstance> &outInstance, uint32_t *preemptedSessionId)
{
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    if (instance == nullptr) {
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
    }

    auto state = instance->GetState();
    if (state == EffectInstance::EffectState::PENDING_RELEASE) {
        hpaeManager_.CancelDelayedTask(instance->GetPendingTaskId());
        instance->Flush();
        instance->SetBoundSessionId(sessionId);
        instance->SetState(EffectInstance::EffectState::ACTIVE);
        outInstance = instance;
        AUDIO_INFO_LOG("[SE::SessionEffectManager] effect rebound reason=reusePending "
            "effect=%{public}s session=%{public}u", effectName.c_str(), sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }

    if (state == EffectInstance::EffectState::ACTIVE) {
        if (instance->GetBoundSessionId() == sessionId) {
            outInstance = instance;
            return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
        }
        AUDIO_INFO_LOG("[SE::SessionEffectManager] effect rebound reason=preempt "
            "effect=%{public}s oldSession=%{public}u newSession=%{public}u",
            effectName.c_str(), instance->GetBoundSessionId(), sessionId);
        if (preemptedSessionId != nullptr) {
            *preemptedSessionId = instance->GetBoundSessionId();
        }
        hpaeManager_.CancelDelayedTask(instance->GetPendingTaskId());
        instance->Flush();
        instance->SetBoundSessionId(sessionId);
        instance->SetState(EffectInstance::EffectState::ACTIVE);
        outInstance = instance;
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }

    // IDLE/ERROR: release old instance so caller can create a new one
    if (state == EffectInstance::EffectState::IDLE || state == EffectInstance::EffectState::ERROR) {
        AUDIO_INFO_LOG("[SE::SessionEffectManager] effect released reason=staleState "
            "effect=%{public}s state=%{public}d", effectName.c_str(), static_cast<int>(state));
        instance->Release();
        effectInstanceMap_.erase(effectName);
    }
    return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
}

int32_t SessionEffectManager::CreateNewInstance(const std::string &effectName, uint32_t sessionId,
    const AudioEffectConfig &config, std::shared_ptr<EffectInstance> &outInstance)
{
    auto instance = std::make_shared<EffectInstance>();
    int32_t ret;
    auto *libEntry = GetEffectLibEntry(effectName);
    if (libEntry != nullptr && libEntry->audioEffectLibHandle != nullptr) {
        ret = instance->Init(effectName, config, libEntry->audioEffectLibHandle);
    } else {
        ret = instance->Init(effectName, config);
    }
    if (ret != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        AUDIO_ERR_LOG("Failed to init effect %{public}s, ret=%{public}d", effectName.c_str(), ret);
        return ret;
    }
    instance->SetBoundSessionId(sessionId);
    instance->SetState(EffectInstance::EffectState::ACTIVE);
    effectInstanceMap_[effectName] = instance;
    // Re-apply persisted parameters (e.g. from a previous IDLE/ERROR -> recreate cycle).
    // Best-effort: a failed re-apply does not fail CreateNewInstance.
    auto savedIt = savedEffectParameters_.find(effectName);
    if (savedIt != savedEffectParameters_.end()) {
        for (const auto &param : savedIt->second) {
            int32_t applyRet = instance->SetParameter(param.first, param.second);
            if (applyRet != 0) {
                AUDIO_WARNING_LOG("[SE::SessionEffectManager] re-apply param failed "
                    "effect=%{public}s type=%{public}d ret=%{public}d",
                    effectName.c_str(), param.first, applyRet);
            }
        }
    }
    outInstance = instance;
    AUDIO_INFO_LOG("[SE::SessionEffectManager] effect created reason=new "
        "effect=%{public}s session=%{public}u", effectName.c_str(), sessionId);
    return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
}

int32_t SessionEffectManager::BindSessionEffect(uint32_t sessionId, const std::string &effectName,
    const AudioEffectConfig &config, std::shared_ptr<EffectInstance> &outInstance,
    uint32_t *preemptedSessionId)
{
    int32_t reuseRet = TryReuseExistingInstance(effectName, sessionId, outInstance, preemptedSessionId);
    if (reuseRet == static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        return reuseRet;
    }

    // Guard: reject if session already bound to a different effectName
    auto existingInstance = FindBySessionId(sessionId);
    if (existingInstance != nullptr) {
        AUDIO_WARNING_LOG("Session %{public}u already bound to a different effect", sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND);
    }

    return CreateNewInstance(effectName, sessionId, config, outInstance);
}

int32_t SessionEffectManager::UnbindSessionEffect(uint32_t sessionId)
{
    auto instance = FindBySessionId(sessionId);
    if (!instance) {
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
    }

    const std::string &effectName = instance->GetEffectName();
    instance->MarkPendingRelease();
    uint32_t gen = instance->GetReleaseGeneration();

    auto self = this;
    auto alive = alive_;
    uint64_t taskId = hpaeManager_.PostDelayedTask(lazyReleaseTimeoutMs_,
        [self, alive, effectName, gen]() {
            if (!*alive) {
                return;
            }
            self->LazyReleaseExpired(effectName, gen);
        });
    instance->SetPendingTaskId(taskId);
    AUDIO_INFO_LOG("[SE::SessionEffectManager] effect pendingRelease reason=unbind "
        "effect=%{public}s session=%{public}u gen=%{public}u",
        effectName.c_str(), sessionId, gen);
    return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
}

void SessionEffectManager::LazyReleaseExpired(const std::string &effectName, uint32_t generation)
{
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    if (instance == nullptr) {
        return;
    }
    if (instance->GetState() != EffectInstance::EffectState::PENDING_RELEASE ||
        instance->GetReleaseGeneration() != generation) {
        return;
    }
    instance->Release();
    effectInstanceMap_.erase(effectName);
    AUDIO_INFO_LOG("[SE::SessionEffectManager] effect lazyReleased "
        "effect=%{public}s gen=%{public}u", effectName.c_str(), generation);
}

std::shared_ptr<EffectInstance> SessionEffectManager::FindBySessionId(uint32_t sessionId) const
{
    for (const auto &pair : effectInstanceMap_) {
        if (pair.second->GetBoundSessionId() == sessionId) {
            return pair.second;
        }
    }
    return nullptr;
}

void SessionEffectManager::OnSessionDestroyed(uint32_t sessionId)
{
    auto instance = FindBySessionId(sessionId);
    if (!instance) {
        return;
    }
    // Per se-state-management.md §2.3:
    //   ACTIVE → OnSessionDestroyed(sid==bound) → PENDING_RELEASE → 启动 lazy 计时
    //   ERROR  → OnSessionDestroyed(sid==bound) → PENDING_RELEASE → 清理错误，启动 lazy 计时
    // If already PENDING_RELEASE (e.g. previously unbound), timer already running — no-op
    if (instance->GetState() == EffectInstance::EffectState::PENDING_RELEASE) {
        return;
    }

    const std::string &effectName = instance->GetEffectName();
    instance->MarkPendingRelease();
    uint32_t gen = instance->GetReleaseGeneration();

    auto self = this;
    auto alive = alive_;
    uint64_t taskId = hpaeManager_.PostDelayedTask(lazyReleaseTimeoutMs_,
        [self, alive, effectName, gen]() {
            if (!*alive) {
                return;
            }
            self->LazyReleaseExpired(effectName, gen);
        });
    instance->SetPendingTaskId(taskId);
    AUDIO_INFO_LOG("[SE::SessionEffectManager] effect pendingRelease reason=sessionDestroyed "
        "effect=%{public}s session=%{public}u gen=%{public}u",
        effectName.c_str(), sessionId, gen);
}

bool SessionEffectManager::HasPendingEffectInstance(const std::string &effectName) const
{
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    if (instance == nullptr) {
        return false;
    }
    return instance->GetState() == EffectInstance::EffectState::PENDING_RELEASE;
}

std::shared_ptr<EffectInstance> SessionEffectManager::GetPendingEffectInstance(
    const std::string &effectName) const
{
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    if (instance == nullptr) {
        return nullptr;
    }
    if (instance->GetState() == EffectInstance::EffectState::PENDING_RELEASE) {
        return instance;
    }
    return nullptr;
}

int32_t SessionEffectManager::FlushPendingEffectInstance(const std::string &effectName)
{
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    if (instance == nullptr) {
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
    }
    if (instance->GetState() == EffectInstance::EffectState::PENDING_RELEASE) {
        instance->Flush();
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }
    return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
}

void SessionEffectManager::SetLazyReleaseTimeout(uint64_t timeoutMs)
{
    lazyReleaseTimeoutMs_ = timeoutMs;
}

uint32_t SessionEffectManager::GetBoundSessionId(const std::string &effectName) const
{
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    if (instance == nullptr) {
        return 0;
    }
    return instance->GetBoundSessionId();
}

std::vector<std::string> SessionEffectManager::GetAllActiveEffectNames() const
{
    std::vector<std::string> names;
    for (const auto &pair : effectInstanceMap_) {
        if (pair.second->GetState() == EffectInstance::EffectState::ACTIVE) {
            names.push_back(pair.first);
        }
    }
    return names;
}

int32_t SessionEffectManager::SetEffectParameter(const std::string &effectName, int32_t type, float value)
{
    savedEffectParameters_[effectName][type] = value;
    auto instance = SafeGetMap(effectInstanceMap_, effectName);
    CHECK_AND_RETURN_RET(instance != nullptr, SUCCESS);
    // Persist so the parameter can be re-applied when the instance is recreated
    // (e.g. after an IDLE/ERROR -> CreateNewInstance cycle). Cleared on LazyReleaseExpired.
    int32_t ret = instance->SetParameter(type, value);
    CHECK_AND_RETURN_RET(ret == 0, ret);
    return ret;
}

void SessionEffectManager::InitEffectLibraries(
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libEntries)
{
    for (const auto &entry : libEntries) {
        effectLibMap_[entry->libraryName] = entry;
    }
    AUDIO_INFO_LOG("InitEffectLibraries: loaded %{public}zu entries", effectLibMap_.size());
}

AudioEffectLibEntry *SessionEffectManager::GetEffectLibEntry(const std::string &libraryName) const
{
    auto it = effectLibMap_.find(libraryName);
    if (it != effectLibMap_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void SessionEffectManager::Deinit()
{
    // IMPORTANT: This method must only be called after HpaeManagerThread has stopped,
    // so that no delayed task (ProcessExpiredTasks) can fire concurrently.
    // HpaeManager::DeInit() guarantees this ordering: stop thread → call Deinit().
    *alive_ = false;
    for (auto &pair : effectInstanceMap_) {
        hpaeManager_.CancelDelayedTask(pair.second->GetPendingTaskId());
        pair.second->Release();
    }
    effectInstanceMap_.clear();
    AUDIO_INFO_LOG("SessionEffectManager deinit completed");
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
