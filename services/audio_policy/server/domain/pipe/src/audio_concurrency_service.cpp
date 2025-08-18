/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioConcurrencyService"
#endif

#include "audio_utils.h"
#include "audio_stream_collector.h"
#include "audio_concurrency_service.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int64_t DELAY_CONTROL_TIME_NS = 100000000; // 100ms
}
void AudioConcurrencyService::Init()
{
    AUDIO_INFO_LOG("AudioConcurrencyService Init");
    std::unique_ptr<AudioConcurrencyParser> parser = std::make_unique<AudioConcurrencyParser>();
    CHECK_AND_RETURN_LOG(parser != nullptr, "Create audioConcurrency parser failed!");
    CHECK_AND_RETURN_LOG(!parser->LoadConfig(concurrencyCfgMap_), "Load audioConcurrency cfgMap failed!");
}

ConcurrencyAction AudioConcurrencyService::GetConcurrencyAction(
    const AudioPipeType existingPipe, const AudioPipeType commingPipe)
{
    auto target = std::make_pair(existingPipe, commingPipe);
    if (concurrencyCfgMap_.find(target) == concurrencyCfgMap_.end()) {
        AUDIO_ERR_LOG("Can not find matching action for existingPipe %{public}d and commingPipe %{public}d",
            existingPipe, commingPipe);
        return PLAY_BOTH;
    }
    return concurrencyCfgMap_[target];
}
} // namespace AudioStandard
} // namespace OHOS