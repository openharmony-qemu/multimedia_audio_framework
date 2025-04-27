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

#ifndef ST_DFX_UTILS_H
#define ST_DFX_UTILS_H

#include <vector>
#include "dfx_stat.h"

namespace OHOS {
namespace AudioStandard {

static const int32_t MIN_DFX_NUMERIC_COUNT = 1;
static const int32_t MAX_DFX_NUMERIC_PERCENTAGE = 100;
static const int32_t MAX_DFX_ACTION_SIZE = 100;
static const int32_t DFX_INVALID_APP_UID = -1;

class DfxUtils {
public:
    static uint32_t SerializeToUint32(const DfxStatInt32 &data);
    static std::string SerializeToJSONString(const RendererStats &data);
    static std::string SerializeToJSONString(const std::vector<InterruptEffect> &data);
    template<class T>
    static std::string SerializeToJSONString(const std::vector<T> &data);
    static std::string SerializeToJSONString(const CapturerStats &data);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DFX_UTILS_H