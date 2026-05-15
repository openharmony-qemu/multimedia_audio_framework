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

#include <iostream>
#include <array>
#include <cstddef>
#include <cstdint>
#include "audio_policy_server.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_info.h"
#include "audio_concurrency_parser.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const size_t MAX_CONCURRENCY_MAP_SIZE = 16;

const std::array<AudioPipeType, 22> PIPE_TYPES = {
    PIPE_TYPE_UNKNOWN,
    PIPE_TYPE_OUT_NORMAL,
    PIPE_TYPE_IN_NORMAL,
    PIPE_TYPE_OUT_LOWLATENCY,
    PIPE_TYPE_IN_LOWLATENCY,
    PIPE_TYPE_OUT_CELLULAR_CALL,
    PIPE_TYPE_IN_CELLULAR_CALL,
    PIPE_TYPE_OUT_VOIP,
    PIPE_TYPE_IN_VOIP,
    PIPE_TYPE_OUT_OFFLOAD,
    PIPE_TYPE_OUT_MULTICHANNEL,
    PIPE_TYPE_OUT_DIRECT_NORMAL,
    PIPE_TYPE_IN_NORMAL_AI,
    PIPE_TYPE_IN_NORMAL_UNPROCESS,
    PIPE_TYPE_IN_NORMAL_ULTRASONIC,
    PIPE_TYPE_IN_NORMAL_VOICE_RECOGNITION,
    PIPE_TYPE_OUT_HWDECODING,
    PIPE_TYPE_IN_NORMAL_RAW_AI,
    PIPE_TYPE_OUT_3DA_DIRECT,
    PIPE_TYPE_OUT_INTERPHONE,
    PIPE_TYPE_IN_INTERPHONE,
    static_cast<AudioPipeType>(-1),
};

const std::array<ConcurrencyAction, 5> CONCURRENCY_ACTIONS = {
    PLAY_BOTH,
    CONCEDE_INCOMING,
    CONCEDE_EXISTING,
    CONCEDE_BOTH,
    static_cast<ConcurrencyAction>(-1),
};

template<typename T, size_t N>
T PickValue(FuzzedDataProvider &fdp, const std::array<T, N> &values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, N - 1)];
}

void AudioConcurrencyParserLoadConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyMap;
    size_t mapSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_CONCURRENCY_MAP_SIZE);
    for (size_t i = 0; i < mapSize; ++i) {
        AudioPipeType existingPipe = PickValue(fdp, PIPE_TYPES);
        AudioPipeType incomingPipe = PickValue(fdp, PIPE_TYPES);
        ConcurrencyAction action = PickValue(fdp, CONCURRENCY_ACTIONS);
        if (fdp.ConsumeBool()) {
            existingPipe = static_cast<AudioPipeType>(fdp.ConsumeIntegral<int32_t>());
        }
        if (fdp.ConsumeBool()) {
            incomingPipe = static_cast<AudioPipeType>(fdp.ConsumeIntegral<int32_t>());
        }
        if (fdp.ConsumeBool()) {
            action = static_cast<ConcurrencyAction>(fdp.ConsumeIntegral<int32_t>());
        }
        concurrencyMap[std::make_pair(existingPipe, incomingPipe)] = action;
    }
    AudioConcurrencyParser audioConcurrencyParser;
    audioConcurrencyParser.LoadConfig(concurrencyMap);
}
void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioConcurrencyParserLoadConfigFuzzTest,
    });
    func(fdp);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size < LIMITSIZE) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
