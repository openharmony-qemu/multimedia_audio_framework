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
#ifndef AUDIO_EVENT_UTILS_H
#define AUDIO_EVENT_UTILS_H

#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {

inline void CheckAndWriteDeviceChangeExceptionEvent(bool cond,
    AudioStreamDeviceChangeReason reason,
    DeviceType devType,
    DeviceRole devRole,
    int32_t errMsg,
    const std::string &errDesc)
{
    if (!(cond)) {
        AudioPolicyUtils::GetInstance().WriteDeviceChangeExceptionEvent(
            reason, devType, devRole, errMsg, errDesc);
    }
}

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_EVENT_UTILS_H