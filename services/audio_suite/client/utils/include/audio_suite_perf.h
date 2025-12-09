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

#ifndef AUDIO_SUITE_PERF_H
#define AUDIO_SUITE_PERF_H

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t MILLISECONDS_TO_MICROSECONDS = 1000;
static constexpr uint32_t RTF_OVERTIME_LEVELS = 3;

static constexpr float RTF_OVERTIME_THRESHOLDS[RTF_OVERTIME_LEVELS] = {
    1.0f,  // RTF_OVERTIME_THRESHOLDS_LEVEL1
    1.1f,  // RTF_OVERTIME_THRESHOLDS_LEVEL2
    1.2f,  // RTF_OVERTIME_THRESHOLDS_LEVEL3
};

enum RtfOvertimeLevel {
    OVER_BASE = 0,
    OVER_110BASE = 1,
    OVER_120BASE = 2
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif