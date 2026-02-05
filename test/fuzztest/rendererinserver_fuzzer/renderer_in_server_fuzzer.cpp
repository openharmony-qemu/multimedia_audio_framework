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
#include "audio_stream_enum.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const int32_t FADING_OUT_DONE = 2;
const int32_t NO_FADING = 0;

typedef void (*TestFuncs)();

AudioProcessConfig InitAudioProcessConfig(AudioStreamInfo streamInfo, DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET,
    int32_t rendererFlags = AUDIO_FLAG_NORMAL, AudioStreamType streamType = STREAM_DEFAULT)
{
    AudioProcessConfig processConfig = {};
    processConfig.streamInfo = streamInfo;
    processConfig.deviceType = deviceType;
    processConfig.rendererInfo = {};
    processConfig.capturerInfo = {};
    processConfig.rendererInfo.rendererFlags = rendererFlags;
    processConfig.streamType = streamType;

    return processConfig;
}

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

void UpdateStreamInfoFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.audioMode = AUDIO_MODE_RECORD;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->checkCount_ = 0;
    RendererInServerPtr->UpdateStreamInfo();
}

void ConfigFixedSizeBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    size_t size = 8;
    size_t frame = 4;
    RendererInServerPtr->Init();
    RendererInServerPtr->byteSizePerFrame_ = frame;
    RendererInServerPtr->spanSizeInFrame_ = size;
    RendererInServerPtr->ConfigFixedSizeBuffer();
}

void ProcessManagerTypeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.audioFlag = (AUDIO_OUTPUT_FLAG_HD|AUDIO_OUTPUT_FLAG_DIRECT);
    config.streamInfo.encoding = ENCODING_EAC3;
    config.rendererInfo.rendererFlags = AUDIO_FLAG_3DA_DIRECT;
    config.streamInfo.encoding = ENCODING_AUDIOVIVID;
    config.rendererInfo.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->ProcessManagerType();
}

void OnStatusUpdateSubFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    config.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    IOperation operation = OPERATION_RELEASED;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    operation = OPERATION_UNDERRUN;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    size_t size = 4;
    size_t frame = 1;
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(0);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(0);
    RendererInServerPtr->audioServerBuffer_->totalSizeInFrame_ = size;
    RendererInServerPtr->spanSizeInFrame_ = frame;
    operation = OPERATION_UNDERRUN;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    operation = OPERATION_UNDERFLOW;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    operation = OPERATION_SET_OFFLOAD_ENABLE;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    operation = OPERATION_OFFLOAD_FLUSH_BEGIN;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    operation = OPERATION_OFFLOAD_FLUSH_END;
    RendererInServerPtr->OnStatusUpdateSub(operation);

    operation = static_cast<IOperation>(OPERATION_OFFLOAD_FLUSH_END + 1);
    RendererInServerPtr->OnStatusUpdateSub(operation);
}

void PauseDirectStreamFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->managerType_ = DIRECT_PLAYBACK;
    RendererInServerPtr->PauseDirectStream();
}

void DequeueBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    size_t length = 10;
    RendererInServerPtr->DequeueBuffer(length);
}

void IsInvalidBufferFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    config.streamInfo.format = SAMPLE_U8;
    RendererInServerPtr->isInSilentState_ = 0;
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    RendererInServerPtr->IsInvalidBuffer(buffer, bufferSize);
}

void WriteMuteDataSysEventFuzzTest()
{
    AudioProcessConfig config;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    config.streamInfo.format = SAMPLE_U8;
    RendererInServerPtr->startMuteTime_ = 1;
    RendererInServerPtr->isInSilentState_ = 1;
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    BufferDesc bufferDesc;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;
    RendererInServerPtr->IsInvalidBuffer(buffer, bufferSize);
    RendererInServerPtr->WriteMuteDataSysEvent(bufferDesc);
}

void DoFadingOutFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    BufferDesc bufferDesc;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;
    RendererInServerPtr->Init();
    RendererInServerPtr->fadeoutFlag_ = NO_FADING;
    RingBufferWrapper bufferWrapper = {
        .basicBufferDescs = {{
            {bufferDesc.buffer, bufferDesc.bufLength},
            {}
        }},
        .dataLength = bufferDesc.dataLength
    };
    RendererInServerPtr->DoFadingOut(bufferWrapper);
}

void WriteData1FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    size_t size = 4;
    uint64_t num = 12;
    uint64_t frame = 4;
    uint32_t total = 16;
    RendererInServerPtr->Init();
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(num);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(frame);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = total;
    RendererInServerPtr->spanSizeInFrame_ = size;
    RendererInServerPtr->WriteData();
}

void GetAvailableSizeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.isStatic = true;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    size_t length = 0;
    RendererInServerPtr->GetAvailableSize(length);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);
    RendererInServerPtr->GetAvailableSize(length);
}

void ProcessFadeOutIfNeededFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.streamType = STREAM_VOICE_MESSAGE;
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->fadeoutFlag_ = FADING_OUT_DONE;
    RingBufferWrapper ringBufferDesc;
    uint64_t currentReadFrame = 4;
    uint64_t currentWriteFrame = 8;
    size_t requestDataInFrame = 4;

    RendererInServerPtr->ProcessFadeOutIfNeeded(
        ringBufferDesc, currentReadFrame, currentWriteFrame, requestDataInFrame);
}

void OnWriteDataFinishFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint32_t count = 10;
    RendererInServerPtr->checkCount_ = count;
    RendererInServerPtr->OnWriteDataFinish();
}

void InitLatencyMeasurementFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->InitLatencyMeasurement();
}

void DetectLatencyFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint8_t arr[8] = {0};
    uint8_t *inputData = arr;
    size_t requestDataLen = 4;
    RendererInServerPtr->Init();
    RendererInServerPtr->InitLatencyMeasurement();
    RendererInServerPtr->DetectLatency(inputData, requestDataLen);
}

void UpdateWriteIndexFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET,
        AUDIO_FLAG_VOIP_DIRECT);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->managerType_ = DIRECT_PLAYBACK;
    RendererInServerPtr->needForceWrite_ = 1;
    RendererInServerPtr->afterDrain = true;
    RendererInServerPtr->UpdateWriteIndex();
}

void DrainFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::weak_ptr<IStreamListener> weakListener = std::weak_ptr<IStreamListener>();
    auto sharedListener = weakListener.lock();
    CHECK_AND_RETURN(sharedListener != nullptr);
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->OnStatusUpdate(OPERATION_STARTED);
    RendererInServerPtr->Drain(false);
}

vector<TestFuncs> g_testFuncs = {
    HandleOperationStartedFuzzTest,
    ReConfigDupStreamCallbackFuzzTest,
    PrepareOutputBufferFuzzTest,
    UpdateStreamInfoFuzzTest,
    ConfigFixedSizeBufferFuzzTest,
    ProcessManagerTypeFuzzTest,
    OnStatusUpdateSubFuzzTest,
    PauseDirectStreamFuzzTest,
    DequeueBufferFuzzTest,
    IsInvalidBufferFuzzTest,
    WriteMuteDataSysEventFuzzTest,
    DoFadingOutFuzzTest,
    WriteData1FuzzTest,
    GetAvailableSizeFuzzTest,
    ProcessFadeOutIfNeededFuzzTest,
    OnWriteDataFinishFuzzTest,
    InitLatencyMeasurementFuzzTest,
    DetectLatencyFuzzTest,
    UpdateWriteIndexFuzzTest,
    DrainFuzzTest,
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