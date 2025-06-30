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
#ifndef AUDIO_BUFFER_DESC_H
#define AUDIO_BUFFER_DESC_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <parcel.h>

#include "securec.h"

namespace OHOS {
namespace AudioStandard {
struct BufferDesc {
    uint8_t *buffer = nullptr;
    size_t bufLength = 0;
    size_t dataLength = 0;
    uint8_t *metaBuffer = nullptr;
    size_t metaLength = 0;
    uint64_t position = 0;
    uint64_t timeStampInNs = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_BUFFER_DESC_H
