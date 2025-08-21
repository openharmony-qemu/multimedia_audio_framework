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

#ifndef XPERF_ADAPTER_H
#define XPERF_ADAPTER_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class XperfAdapter {
public:
    static XperfAdapter& GetInstance();

    void ReportStateChangeEventIfNeed(int32_t eventId, StreamUsage usage, uint32_t sessionId, int32_t pid, int32_t uid);

    void ReportFaultEvent(int32_t faultcode, uint32_t uid, uint32_t sessionId);
private:
    XperfAdapter() = default;
    ~XperfAdapter() = default;

    XperfAdapter(XperfAdapter&&) = delete;
    XperfAdapter& operator=(const XperfAdapter&) = delete;
    XperfAdapter& operator=(XperfAdapter&&) = delete;

    bool NeedNotifyXperf(StreamUsage usage);
};
} // namespace AudioStandard
} // namespace OHOS
#endif //XPERF_ADAPTER_H