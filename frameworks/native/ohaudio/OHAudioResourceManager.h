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

#ifndef OH_AUDIO_RESOURCE_MANAGER_H
#define OH_AUDIO_RESOURCE_MANAGER_H

#include "audio_info.h"
#include "audio_log.h"
#include "OHAudioWorkgroup.h"
#include "audio_manager_base.h"
#include "native_audio_common.h"
#include "native_audio_resource_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioResourceManager {
public:
    ~OHAudioResourceManager() {};

    static OHAudioResourceManager *GetInstance();
    OHAudioWorkgroup *CreateWorkgroup();
    bool ReleaseWorkgroup(OHAudioWorkgroup *group);
private:
    OHAudioResourceManager() {};
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_RESOURCE_MANAGER_H