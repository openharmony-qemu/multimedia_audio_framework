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

#include "hpae_backoff_controller.h"
#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <iostream>
#include <thread>
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeBackoffController::HpaeBackoffController(int32_t minDelay, int32_t maxDelay, int32_t increment)
    : minDelay_(minDelay), maxDelay_(maxDelay), increment_(increment), delay_(minDelay)
{}

void HpaeBackoffController::HandleResult(bool result)
{
    if (result) {
        Reset();
        return;
    }
    delay_ = std::min(delay_ + increment_, maxDelay_);
#ifdef ENABLE_HOOK_PCM
    auto start = std::chrono::system_clock::now();
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_));
#ifdef ENABLE_HOOK_PCM
    auto end = std::chrono::system_clock::now();
    uint64_t sleepTime =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    AUDIO_INFO_LOG("func call fail, sleep %{public}" PRIu64 "ms", sleepTime);
#endif
}

void HpaeBackoffController::Reset()
{
    delay_ = minDelay_;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS