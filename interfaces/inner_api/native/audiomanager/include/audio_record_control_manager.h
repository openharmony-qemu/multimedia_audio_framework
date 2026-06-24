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

#ifndef AUDIO_RECORD_CONTROL_MANAGER_H
#define AUDIO_RECORD_CONTROL_MANAGER_H

#include "audio_policy_interface.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioRecordControlManager {
public:
    static AudioRecordControlManager &GetInstance();

    int32_t EnableSystemRecordController(bool show, const SystemRecordControllerConfig &config);
    int32_t SetRecordControllerEnabledChangeCallback(
        const std::shared_ptr<SystemRecordControllerEnabledChangeCallback> &callback);
    int32_t UnsetRecordControllerEnabledChangeCallback(
        const std::shared_ptr<SystemRecordControllerEnabledChangeCallback> &callback);

private:
    AudioRecordControlManager() = default;
    AudioRecordControlManager(const AudioRecordControlManager &oth) = delete;
    ~AudioRecordControlManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RECORD_CONTROL_MANAGER_H