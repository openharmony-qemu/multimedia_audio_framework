/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <cstddef>
#include <cstdint>

#include "message_parcel.h"

#include "audio_manager_base.h"
#include "audio_server.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;

float Convert2Float(const uint8_t *ptr)
{
    float floatValue = static_cast<float>(*ptr);
    return floatValue / 128.0f - 1.0f;
}

void AudioServerBalanceFuzzer(const uint8_t *rawData, size_t size, std::shared_ptr<AudioServer> AudioServerPtr)
{
    float balanceValue = Convert2Float(rawData);
    MessageParcel data;
    data.WriteFloat(balanceValue);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE), data, reply, option);
}

bool Convert2Bool(const uint8_t *ptr)
{
    return (ptr[0] & 1) ? true : false;
}

void AudioServerMonoFuzzer(const uint8_t *rawData, size_t size, std::shared_ptr<AudioServer> AudioServerPtr)
{
    bool monoState = Convert2Bool(rawData);
    MessageParcel data;
    data.WriteBool(monoState);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE), data, reply, option);
}

void AudioServerBalanceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> AudioServerPtr =
        std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerBalanceFuzzer(rawData, size, AudioServerPtr);
    AudioServerMonoFuzzer(rawData, size, AudioServerPtr);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerBalanceFuzzTest(data, size);
    return 0;
}
