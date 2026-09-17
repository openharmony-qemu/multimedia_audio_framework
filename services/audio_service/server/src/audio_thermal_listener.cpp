/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioThermalLevelListener"
#endif

#include <cinttypes>

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_thermal_listener.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect.h"
#include "thermal_mgr_client.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr int32_t POST_THERMAL_LEVEL_TASK_DELAY_MS = 300;
    const char *THERMAL_LEVEL_TASK = "ThermalLevelTask";

    //ThermalLevel 合法范围：[COOL=0, ESCAPE=7], 避免首次订阅未初始化补发的 INT_MIN 等非法值进入处理
    bool IsValidThermalLevel(const PowerMgr::ThermalLevel &level)
    {
        int32_t value = static_cast<int32_t>(level);
        return value >= static_cast<int32_t>(PowerMgr::ThermalLevel::COOL) &&
            value <= static_cast<int32_t>(PowerMgr::ThermalLevel::ESCAPE);
    }

    std::shared_ptr<PowerMgr::ThermalMgrListener> thermalListener_ = nullptr;
    std::shared_ptr<AudioThermalLevelListener> thermalLevelListener_ = nullptr;
    std::shared_ptr<AudioThermalLevelListenerHandler> ListenerHandler_ =
        std::make_shared<AudioThermalLevelListenerHandler>;
}

AudioThermalLevelListenerHandler::AudioThermalLevelListenerHandler()
    : AppExecFwk::EventHandler(
        AppExecFwk::EventRunner::Create("AudioThermalLevelRunner", AppExecFwk::ThreadMode::FFRT))
{
    AUDIO_DEBUG_LOG("AudioThermalLevelListenerHandler created");
}

AudioThermalLevelListenerHandler::~AudioThermalLevelListenerHandler()
{
    AUDIO_DEBUG_LOG("~AudioThermalLevelListenerHandler destroyed");
}

void AudioThermalLevelListener::RegisterThermalLevelListener()
{
    CHECK_AND_RETURN_LOG(thermalListener_ != nullptr && thermalLevelListener_ != nullptr,
        "thermal listener not ready, register failed");
    int32_t ret = thermalListener_->SubscribeLevelEvent(thermalLevelListener_);
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("SubscribeLevelEvent failed, ret is %{public}d", ret);
    }
}

void AudioThermalLevelListener::UnregisterThermalLevelListener()
{
    CHECK_AND_RETURN_LOG(thermalListener_ != nullptr, "thermal listener is nullptr, unRegister failed");
    thermalListener_->UnSubscribeLevelEvent();
    thermalListener_ = nullptr;
    thermalLevelListener_ = nullptr;
}

void AudioThermalLevelListener::OnThermalLevelResult(const PowerMgr::ThermalLevel &level)
{
    if (IsValidThermalLevel(level)) {
        PostThermalLevelTask(level);
    }
}

void AudioThermalLevelListener::PostThermalLevelTask(const PowerMgr::ThermalLevel &thermalLevel)
{
    CHECK_AND_RETURN_LOG(ListenerHandler_ != nullptr, "ListenerHandler_ is nullptr");
    auto task = [thermalLevel]() {
        AudioEffectChainManager::GetInstance()->UpdateParamExtra("audio_effect", THERMAL_LEVEL_SUBKEY,
            std::to_string(static_cast<int32_t>(thermalLevel)));
    };
    ListenerHandler_->PostTask(task, THERMAL_LEVEL_TASK, POST_THERMAL_LEVEL_TASK_DELAY_MS);
}

void RegisterAudioThermalLevelListener()
{
    auto thermalLevel = PowerMgr::ThermalMgrClient::GetInstance().GetThermalLevel();
    if (IsValidThermalLevel(thermalLevel)) {
        AudioEffectChainManager::GetInstance()->UpdateParamExtra("audio_effect", THERMAL_LEVEL_SUBKEY,
            std::to_string(static_cast<int32_t>(thermalLevel)));
    }

    if (thermalListener_ == nullptr) {
        thermalListener_ = std::make_shared<PowerMgr::ThermalMgrListener>();
    }
    if (thermalLevelListener_ == nullptr) {
        thermalLevelListener_ = std::make_shared<AudioThermalLevelListener>();
    }
    thermalLevelListener_->RegisterThermalLevelListener();
}

void UnregisterAudioThermalLevelListener()
{
    if (thermalLevelListener_ != nullptr) {
        thermalLevelListener_->UnregisterThermalLevelListener();
    }
}

} // namespace AudioStandard
} // namespace OHOS