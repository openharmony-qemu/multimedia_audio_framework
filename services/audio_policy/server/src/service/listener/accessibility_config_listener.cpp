/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AccessibilityConfigListener"
#endif

#include "accessibility_config_listener.h"

#include "audio_policy_log.h"

using namespace OHOS::AccessibilityConfig;

namespace OHOS {
namespace AudioStandard {
AccessibilityConfigListener::AccessibilityConfigListener(IAudioAccessibilityConfigObserver &observer)
    : audioAccessibilityConfigObserver_(observer) {}

AccessibilityConfigListener::~AccessibilityConfigListener() {}

void AccessibilityConfigListener::OnConfigChanged(const CONFIG_ID configId, const ConfigValue &value)
{
    AUDIO_DEBUG_LOG("OnConfigChanged: configId %{public}d", configId);
    if (configId == CONFIG_AUDIO_MONO) {
        audioAccessibilityConfigObserver_.OnMonoAudioConfigChanged(value.audioMono);
    } else if (configId == CONFIG_AUDIO_BALANCE) {
        // value.audioBalance should be in the range [-1.0, 1.0]
        float balance = value.audioBalance;
        if (balance < -1.0f || balance > 1.0f) {
            AUDIO_WARNING_LOG("AccessibilityConfigListener: audioBalance value is out of range [-1.0, 1.0]");
        } else {
            audioAccessibilityConfigObserver_.OnAudioBalanceChanged(balance);
        }
    }
}

void AccessibilityConfigListener::SubscribeObserver()
{
    auto &accessibilityConfig = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    (void)accessibilityConfig.InitializeContext();
    accessibilityConfig.SubscribeConfigObserver(CONFIG_AUDIO_MONO, shared_from_this());
    accessibilityConfig.SubscribeConfigObserver(CONFIG_AUDIO_BALANCE, shared_from_this());
}

void AccessibilityConfigListener::UnsubscribeObserver()
{
    auto &accessibilityConfig = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    (void)accessibilityConfig.InitializeContext();
    accessibilityConfig.UnsubscribeConfigObserver(CONFIG_AUDIO_MONO, shared_from_this());
    accessibilityConfig.UnsubscribeConfigObserver(CONFIG_AUDIO_BALANCE, shared_from_this());
}
} // namespace AudioStandard
} // namespace OHOS