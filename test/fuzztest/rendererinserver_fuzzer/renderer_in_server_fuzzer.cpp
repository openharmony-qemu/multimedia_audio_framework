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
#include "renderer_in_server.h"
#include "audio_info.h"
#include "i_stream_listener.h"
#include "ring_buffer_wrapper.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

void HandleOperationStartedFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->standByEnable_ = g_fuzzUtils.GetData<bool>();
    RendererInServerPtr->HandleOperationStarted();
}

void ReConfigDupStreamCallbackFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->dupTotalSizeInFrame_ = g_fuzzUtils.GetData<size_t>();
    RendererInServerPtr->ReConfigDupStreamCallback();
}

void DequeueBufferFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    size_t length = g_fuzzUtils.GetData<size_t>();
    RendererInServerPtr->DequeueBuffer(length);
}

void WriteMuteDataSysEventFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    BufferDesc bufferDesc;
    bufferDesc.bufLength = g_fuzzUtils.GetData<size_t>();
    RendererInServerPtr->isInSilentState_ = g_fuzzUtils.GetData<bool>();
    RendererInServerPtr->WriteMuteDataSysEvent(bufferDesc);
}

void DoFadingOutFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RingBufferWrapper bufferDesc;
    RendererInServerPtr->DoFadingOut(bufferDesc);
}

void PrepareOutputBufferFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RingBufferWrapper bufferDesc;
    RendererInServerPtr->PrepareOutputBuffer(bufferDesc);
}

void GetAvailableSizeFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    size_t length = g_fuzzUtils.GetData<size_t>();
    RendererInServerPtr->GetAvailableSize(length);
}

vector<TestFuncs> g_testFuncs = {
    HandleOperationStartedFuzzTest,
    ReConfigDupStreamCallbackFuzzTest,
    DequeueBufferFuzzTest,
    WriteMuteDataSysEventFuzzTest,
    DoFadingOutFuzzTest,
    PrepareOutputBufferFuzzTest,
    GetAvailableSizeFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}