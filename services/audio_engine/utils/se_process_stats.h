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

#ifndef SE_PROCESS_STATS_H
#define SE_PROCESS_STATS_H

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

struct SeProcessStats {
    int64_t framesProcessed = 0;
    int64_t framesOutput = 0;
    int32_t processErrors = 0;
    int32_t underrunCount = 0;
    float lastProcessMs = 0.0f;
    float maxProcessMs = 0.0f;

    void Reset()
    {
        framesProcessed = 0;
        framesOutput = 0;
        processErrors = 0;
        underrunCount = 0;
        lastProcessMs = 0.0f;
        maxProcessMs = 0.0f;
    }

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "frames=" << framesProcessed
            << " out=" << framesOutput
            << " errors=" << processErrors
            << " underruns=" << underrunCount
            << " latency=" << std::fixed << std::setprecision(1)
            << lastProcessMs << "/" << maxProcessMs << "ms";
        return oss.str();
    }
};

#endif // SE_PROCESS_STATS_H
