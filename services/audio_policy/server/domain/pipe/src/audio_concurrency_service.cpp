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
#include "audio_stream_enum.h"
#include "audio_stream_collector.h"
#include "audio_concurrency_service.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int64_t DELAY_CONTROL_TIME_NS = 100000000; // 100ms
}

static const std::vector<std::pair<AudioMode, uint32_t>> priorityOrder = {
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_FAST},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_VOIP},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_DIRECT},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_MULTICHANNEL},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION},
    {AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_NORMAL},
    {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP},
    {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_FAST},
    {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_AI},
    {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_NORMAL},
};

static const std::map<std::pair<AudioMode, uint32_t>, AudioPipeType> pipeTypeMap_ = {
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP}, PIPE_TYPE_OUT_VOIP},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_FAST}, PIPE_TYPE_OUT_LOWLATENCY},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_VOIP}, PIPE_TYPE_OUT_VOIP},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_DIRECT}, PIPE_TYPE_OUT_DIRECT_NORMAL},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_MULTICHANNEL}, PIPE_TYPE_OUT_MULTICHANNEL},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD}, PIPE_TYPE_OUT_OFFLOAD},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION}, PIPE_TYPE_OUT_CELLULAR_CALL},
    {{AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_NORMAL}, PIPE_TYPE_OUT_NORMAL},
    {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP}, PIPE_TYPE_IN_VOIP},
    {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_FAST}, PIPE_TYPE_IN_LOWLATENCY},
    {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_AI}, PIPE_TYPE_IN_NORMAL_AI},
    {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_NORMAL}, PIPE_TYPE_IN_NORMAL},
};

void AudioConcurrencyService::Init()
{
    AUDIO_INFO_LOG("AudioConcurrencyService Init");
    std::unique_ptr<AudioConcurrencyParser> parser = std::make_unique<AudioConcurrencyParser>();
    CHECK_AND_RETURN_LOG(parser != nullptr, "Create audioConcurrency parser failed!");
    CHECK_AND_RETURN_LOG(!parser->LoadConfig(concurrencyConfigMap_), "Load audioConcurrency cfgMap failed!");
}

ConcurrencyAction AudioConcurrencyService::GetConcurrencyAction(
    const AudioPipeType existingPipe, const AudioPipeType commingPipe)
{
    auto target = std::make_pair(existingPipe, commingPipe);
    if (concurrencyConfigMap_.find(target) == concurrencyConfigMap_.end()) {
        AUDIO_ERR_LOG("Can not find matching action for existingPipe %{public}d and commingPipe %{public}d",
            existingPipe, commingPipe);
        return PLAY_BOTH;
    }
    return concurrencyConfigMap.at(target);
}

AudioPipeType AudioConcurrencyService::GetPipeTypeByRouteFlag(uint32_t flag, AudioMode audioMode)
{
    for (const auto &check : priorityOrder) {
        if (check.first != audioMode) continue;
        if ((flag & check.second) != check.second) continue;
        
        auto it = pipeTypeMap_.find(check);
        if (it != pipeTypeMap_.end()) {
            return it->second;
        }
    }
    AUDIO_ERR_LOG("Can not find matching pipeType for routeFlag %{public}u and audioMode %{public}d", flag, audioMode);
    return (audioMode == AUDIO_MODE_PLAYBACK) ? PIPE_TYPE_OUT_NORMAL : PIPE_TYPE_IN_NORMAL;
}
} // namespace AudioStandard
} // namespace OHOS