/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ANAHS_MANAGER_H
#define ST_AUDIO_ANAHS_MANAGER_H

#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceAnahs;
class AudioAnahsManager {
public:
    AudioAnahsManager() = default;
    virtual ~AudioAnahsManager() = default;

    static AudioAnahsManager *GetInstance();
    int32_t SetAudioDeviceAnahsCallback(const std::shared_ptr<AudioDeviceAnahs> &callback);
    int32_t UnsetAudioDeviceAnahsCallback();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ANAHS_MANAGER_H

