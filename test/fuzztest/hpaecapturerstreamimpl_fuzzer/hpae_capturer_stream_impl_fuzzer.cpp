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

#include "hpae_capturer_stream_impl_fuzzer.h"
#include <memory>
#include <string>
#include "i_hpae_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::AudioStandard::HPAE;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);

    if (g_dataSize <= g_pos) {
        return object;
    }

    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void StartFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    stream->Start();
}

void PauseFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    stream->Pause();
}

void FlushFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    stream->Flush();
}

void StopFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    stream->Stop();
}

void ReleaseFuzzTest()
{
    AudioProcessConfig config = {};
    config.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    stream->Start();
    stream->Release();
}

void GetStreamFrameReadFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    uint64_t frameRead = 0;
    stream->GetStreamFramesRead(frameRead);
}

void GetCurrentTimeStampFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    uint64_t timeStamp = 0;
    stream->GetCurrentTimeStamp(timeStamp);
}

void GetLatencyFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    uint64_t latency = 0;
    stream->GetLatency(latency);
}

void OnStreamDataFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    auto readCallback = std::make_shared<IIReadCallback>();
    stream->RegisterReadCallback(readCallback);
    AudioCallBackCapturerStreamInfo streamInfo;
    stream->OnStreamData(streamInfo);
}

void OnStatusUpdateFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    auto statusCallback = std::make_shared<IIStatusCallback>();
    stream->RegisterStatusCallback(statusCallback);
    stream->OnStatusUpdate(OPERATION_INVALID, 0);
}

void BufferOperationFuzzTest()
{
    AudioProcessConfig config = {};
    auto stream = std::make_shared<HpaeCapturerStreamImpl>(config);
    stream->DequeueBuffer(0);
    BufferDesc bufferDesc;
    stream->EnqueueBuffer(bufferDesc);
    stream->DropBuffer();
    stream->AbortCallback(0);
}

TestFuncs g_testFuncs[11] = {
    StartFuzzTest,
    PauseFuzzTest,
    FlushFuzzTest,
    StopFuzzTest,
    ReleaseFuzzTest,
    GetStreamFrameReadFuzzTest,
    GetCurrentTimeStampFuzzTest,
    GetLatencyFuzzTest,
    OnStreamDataFuzzTest,
    OnStatusUpdateFuzzTest,
    BufferOperationFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
