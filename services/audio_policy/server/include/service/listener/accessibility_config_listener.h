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

#ifndef ST_ACCESSIBILITY_CONFIG_LISTENER_H
#define ST_ACCESSIBILITY_CONFIG_LISTENER_H

#include <vector>

#include "accessibility_config.h"
#include "iaudio_accessibility_config_observer.h"

namespace OHOS {
namespace AudioStandard {
class AccessibilityConfigListener : public OHOS::AccessibilityConfig::AccessibilityConfigObserver,
    public std::enable_shared_from_this<AccessibilityConfigListener> {
public:
    AccessibilityConfigListener(IAudioAccessibilityConfigObserver &observer);
    ~AccessibilityConfigListener();

    void OnConfigChanged(const AccessibilityConfig::CONFIG_ID configId,
        const AccessibilityConfig::ConfigValue &value) override;

    void SubscribeObserver();
    void UnsubscribeObserver();

private:
    IAudioAccessibilityConfigObserver &audioAccessibilityConfigObserver_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_ACCESSIBILITY_CONFIG_LISTENER_H