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
#define LOG_TAG "AudioFuzzTest"
#endif

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_hdi_log.h"
#include "audio_common_converter.h"
#include "message_parcel.h"
#include "audio_info.h"
#include "audio_source_type.h"
#include "audio_ring_cache.h"
#include "audio_thread_task.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
static const std::string THREAD_NAME = "FuzzTestThreadName";

void AudioThreadTaskFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::unique_ptr<AudioThreadTask> audioThreadTask;
    audioThreadTask = std::make_unique<AudioThreadTask>(THREAD_NAME);
    auto myJob = []() {
        AUDIO_INFO_LOG("Hello Fuzz Test!");
    };
    audioThreadTask->RegisterJob(std::move(myJob));
    audioThreadTask->Start();
    audioThreadTask->CheckThreadIsRunning();
    audioThreadTask->Pause();
    audioThreadTask->Start();
    audioThreadTask->PauseAsync();
    audioThreadTask->Start();
    audioThreadTask->StopAsync();
    audioThreadTask->Start();
    audioThreadTask->Stop();
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioThreadTaskFuzzTest(data, size);
    return 0;
}
