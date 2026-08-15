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

#ifndef AUDIO_HDI_VERSION_MANAGER_H
#define AUDIO_HDI_VERSION_MANAGER_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {

// Both functions are defined in libhdiadapter_new.z.so (see util/audio_hdi_version_manager.cpp).
// Explicit default visibility is required: audio_service calls IsAudioHdiV61OrAbove() across
// DSO boundaries, and some unit tests compile production sources (e.g. local_device_manager.cpp)
// directly into the test binary. The explicit attribute overrides any global
// -fvisibility=hidden that may come from build templates outside this repository.
// Add for flex version. Use for new HDI define compatible.
__attribute__((visibility("default"))) bool IsAudioHdiV61OrAbove();
__attribute__((visibility("default"))) void SetAudioHdiVersion(uint32_t majorVersion, uint32_t minorVersion);

}
}

#endif
