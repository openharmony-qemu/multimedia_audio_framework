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

#ifndef HDI_MONITOR_H
#define HDI_MONITOR_H

#include <string>

namespace OHOS {
namespace AudioStandard {
enum HdiType : uint8_t {
    LOCAL = 0,
    A2DP = 1,
    REMOTE = 2,
};

enum ErrorCase : uint32_t {
    CALL_HDI_FAILED = 0,
    CALL_HDI_TIMEOUT = 1,
    CHECK_HDI_FAILED = 2,
};

class HdiMonitor {
public:
    static void ReportHdiException(HdiType hdiType, ErrorCase errorCase, int32_t errorMsg, const std::string &desc);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_MONITOR_H
