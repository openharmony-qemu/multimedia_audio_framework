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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "oh_audio_buffer.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const int32_t NUM_1 = 1;

typedef void (*TestFuncs)();

class AudioSharedMemoryFuzz : public AudioSharedMemory {
public:
    explicit AudioSharedMemoryFuzz() = default;
    virtual ~AudioSharedMemoryFuzz() = default;
    uint8_t *GetBase() { return nullptr; };
    size_t GetSize() { return 0; };
    int GetFd() { return 0; };
    std::string GetName() { return ""; };
};

void MarshallingFuzzTest()
{
    shared_ptr<AudioSharedMemoryFuzz> audioSharedMemory =
        std::make_shared<AudioSharedMemoryFuzz>();
    if (audioSharedMemory == nullptr) {
        return;
    }
    Parcel parcel;
    audioSharedMemory->Marshalling(parcel);
}

void UnmarshallingFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<AudioSharedMemoryFuzz> audioSharedMemory =
        std::make_shared<AudioSharedMemoryFuzz>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (audioSharedMemory == nullptr) {
        return;
    }
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    Parcel parcel;
    audioSharedMemory->Marshalling(parcel);
    audioSharedMemory->Unmarshalling(parcel);
    ohAudioBufferBase->Marshalling(parcel);
    ohAudioBufferBase->Unmarshalling(parcel);
}

void GetReadableDataFramesFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->GetReadableDataFrames();
}

void SharedMemoryWriteToParcelFuzzTest()
{
    shared_ptr<AudioSharedMemoryFuzz> audioSharedMemory =
        std::make_shared<AudioSharedMemoryFuzz>();
    if (audioSharedMemory == nullptr) {
        return;
    }
    MessageParcel parcel;
    audioSharedMemory->WriteToParcel(audioSharedMemory, parcel);
}

vector<TestFuncs> g_testFuncs = {
    MarshallingFuzzTest,
    UnmarshallingFuzzTest,
    GetReadableDataFramesFuzzTest,
    SharedMemoryWriteToParcelFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
