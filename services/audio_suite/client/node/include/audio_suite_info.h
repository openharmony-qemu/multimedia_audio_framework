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
#ifndef AUDIO_SUITE_INFO_H
#define AUDIO_SUITE_INFO_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <parcel.h>

#include "timestamp.h"
#include "securec.h"
#include "audio_stream_info.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

enum BufferType {
    HPAE_BUFFER_TYPE_DEFAULT = 0,
    HPAE_BUFFER_TYPE_COBUFFER
};

struct AudioNodeInfo {
    AudioNodeType nodeType;
    uint32_t nodeId;
    float volume = 1.0;
    bool finishedFlag = false;
    bool bypassStatus = false;
    AudioFormat audioFormat;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SUITE_INFO_H