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
#ifndef LOG_TAG
#define LOG_TAG "AudioFormatUtils"
#endif

#include "audio_format_utils.h"

namespace OHOS {
namespace AudioStandard {
int32_t FormatUtils::StringToInt32(const std::string &str, int32_t dftValue)
{
    CHECK_AND_RETURN_RET_LOG(!str.empty(), dftValue, "str is empty");
    int32_t result = 0;
    const auto *first = str.data();
    const auto *last = first + str.size();
    std::from_chars_result res = std::from_chars(first, last, result);
    CHECK_AND_RETURN_RET_LOG(res.ec == std::errc{} && res.ptr == last, dftValue, "from_chars failed");
    return result;
}
}
} // namespace OHOS