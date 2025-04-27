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
#define LOG_TAG "CallbackWrapper"
#endif

#include "util/callback_wrapper.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
void SinkCallbackWrapper::RegistCallback(uint32_t type, std::shared_ptr<IAudioSinkCallback> cb)
{
    CHECK_AND_RETURN_LOG(type < HDI_CB_TYPE_NUM, "invalid type %{public}u", type);
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback is nullptr");

    std::lock_guard<std::mutex> lock(cbMtx_);
    cbs_[type] = cb;
}

void SinkCallbackWrapper::RegistCallback(uint32_t type, IAudioSinkCallback *cb)
{
    CHECK_AND_RETURN_LOG(type < HDI_CB_TYPE_NUM, "invalid type %{public}u", type);
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback is nullptr");

    std::lock_guard<std::mutex> lock(rawCbMtx_);
    rawCbs_[type] = cb;
}

void SinkCallbackWrapper::RegistCallbackGenerator(uint32_t type,
    std::function<std::shared_ptr<IAudioSinkCallback>(uint32_t)> cbGenerator)
{
    CHECK_AND_RETURN_LOG(type < HDI_CB_TYPE_NUM, "invalid type %{public}u", type);
    CHECK_AND_RETURN_LOG(cbGenerator, "callback generator is nullptr");
    std::lock_guard<std::mutex> lock(cbGeneratorMtx_);
    cbGenerators_[type] = cbGenerator;
}

std::shared_ptr<IAudioSinkCallback> SinkCallbackWrapper::GetCallback(uint32_t type, uint32_t renderId)
{
    CHECK_AND_RETURN_RET_LOG(type < HDI_CB_TYPE_NUM, nullptr, "invalid type %{public}u", type);
    std::lock_guard<std::mutex> cbLock(cbMtx_);
    if (cbs_.count(type)) {
        return cbs_[type];
    }
    std::lock_guard<std::mutex> cbGeneratorLock(cbGeneratorMtx_);
    if (cbGenerators_.count(type) && cbGenerators_[type]) {
        return cbGenerators_[type](renderId);
    }
    return nullptr;
}

IAudioSinkCallback *SinkCallbackWrapper::GetRawCallback(uint32_t type)
{
    CHECK_AND_RETURN_RET_LOG(type < HDI_CB_TYPE_NUM, nullptr, "invalid type %{public}u", type);
    CHECK_AND_RETURN_RET(rawCbs_.count(type), nullptr);
    std::lock_guard<std::mutex> lock(rawCbMtx_);
    return rawCbs_[type];
}

void SinkCallbackWrapper::OnRenderSinkParamChange(const std::string &networkId, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    for (auto &cb : cbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnRenderSinkParamChange(networkId, key, condition, value);
    }
    for (auto &cb : rawCbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnRenderSinkParamChange(networkId, key, condition, value);
    }
}

void SinkCallbackWrapper::OnRenderSinkStateChange(uint32_t uniqueId, bool started)
{
    for (auto &cb : cbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnRenderSinkStateChange(uniqueId, started);
    }
    for (auto &cb : rawCbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnRenderSinkStateChange(uniqueId, started);
    }
}

void SourceCallbackWrapper::RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> cb)
{
    CHECK_AND_RETURN_LOG(type < HDI_CB_TYPE_NUM, "invalid type %{public}u", type);
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback is nullptr");
    std::lock_guard<std::mutex> lock(cbMtx_);
    cbs_[type] = cb;
}

void SourceCallbackWrapper::RegistCallback(uint32_t type, IAudioSourceCallback *cb)
{
    CHECK_AND_RETURN_LOG(type < HDI_CB_TYPE_NUM, "invalid type %{public}u", type);
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback is nullptr");
    std::lock_guard<std::mutex> lock(rawCbMtx_);
    rawCbs_[type] = cb;
}

void SourceCallbackWrapper::RegistCallbackGenerator(uint32_t type,
    std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> cbGenerator)
{
    CHECK_AND_RETURN_LOG(type < HDI_CB_TYPE_NUM, "invalid type %{public}u", type);
    CHECK_AND_RETURN_LOG(cbGenerator, "callback generator is nullptr");
    std::lock_guard<std::mutex> lock(cbGeneratorMtx_);
    cbGenerators_[type] = cbGenerator;
}

std::shared_ptr<IAudioSourceCallback> SourceCallbackWrapper::GetCallback(uint32_t type, uint32_t captureId)
{
    CHECK_AND_RETURN_RET_LOG(type < HDI_CB_TYPE_NUM, nullptr, "invalid type %{public}u", type);
    std::lock_guard<std::mutex> cbLock(cbMtx_);
    if (cbs_.count(type)) {
        return cbs_[type];
    }
    std::lock_guard<std::mutex> cbGeneratorLock(cbGeneratorMtx_);
    if (cbGenerators_.count(type) && cbGenerators_[type]) {
        return cbGenerators_[type](captureId);
    }
    return nullptr;
}

IAudioSourceCallback *SourceCallbackWrapper::GetRawCallback(uint32_t type)
{
    CHECK_AND_RETURN_RET_LOG(type < HDI_CB_TYPE_NUM, nullptr, "invalid type %{public}u", type);
    CHECK_AND_RETURN_RET(rawCbs_.count(type), nullptr);
    std::lock_guard<std::mutex> lock(rawCbMtx_);
    return rawCbs_[type];
}

void SourceCallbackWrapper::OnCaptureSourceParamChange(const std::string &networkId, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    for (auto &cb : cbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnCaptureSourceParamChange(networkId, key, condition, value);
    }
    for (auto &cb : rawCbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnCaptureSourceParamChange(networkId, key, condition, value);
    }
}

void SourceCallbackWrapper::OnCaptureState(bool isActive)
{
    for (auto &cb : cbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnCaptureState(isActive);
    }
    for (auto &cb : rawCbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnCaptureState(isActive);
    }
}

void SourceCallbackWrapper::OnWakeupClose(void)
{
    for (auto &cb : cbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnWakeupClose();
    }
    for (auto &cb : rawCbs_) {
        if (cb.second == nullptr) {
            continue;
        }
        cb.second->OnWakeupClose();
    }
}

} // namespace AudioStandard
} // namespace OHOS
