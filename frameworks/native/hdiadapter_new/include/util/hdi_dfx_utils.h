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

#ifndef HDI_DFX_UTILS_H
#define HDI_DFX_UTILS_H

#include "audio_dump_pcm.h"
#include "audio_utils.h"
#include "volume_tools.h"
#include "common/hdi_adapter_type.h"

namespace OHOS {
namespace AudioStandard {
class HdiDfxUtils {
public:
    static void PrintVolumeInfo(char *data, uint64_t &len, const IAudioSourceAttr &attr, std::string logUtilsTag,
        int64_t &volumeDataCount);
    static void DumpData(char *data, uint64_t &len, FILE *dumpFile, std::string dumpFileName);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_DFX_UTILS_H
