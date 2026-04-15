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

#ifndef AUDIO_SAFE_VOLUME_EVENT_SUBSCRIBER_H
#define AUDIO_SAFE_VOLUME_EVENT_SUBSCRIBER_H

#include "singleton.h"

namespace OHOS {
namespace AudioStandard {

class SafeVolumeEventSubscriber {
    DECLARE_SINGLETON(SafeVolumeEventSubscriber)
public:
    void SubscribeSafeVolumeEvent();
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SAFE_VOLUME_EVENT_SUBSCRIBER_H
