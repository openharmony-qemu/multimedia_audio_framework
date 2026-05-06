/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <utility>
#include "renderer_in_server.h"
#include "audio_info.h"
#include "i_stream_listener.h"
#include "i_hpae_soft_link.h"
#include "ipc_stream_in_server.h"
#include "ring_buffer_wrapper.h"
#include "../fuzz_utils.h"
#include "audio_stream_enum.h"
#include "audio_stream_info.h"
#include "native_audio_suite_base.h"
#include "hpae_soft_link.h"
#include "pro_renderer_stream_impl.h"

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
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->playerDfx_ = std::make_unique<PlayerDfxWriter>(config.appInfo, 0);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    uint64_t num = 12;
    uint64_t frame = 4;
    uint32_t total = 16;
    BasicBufferInfo bufferInfo;
    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_ = &bufferInfo;
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(num);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(frame);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = total;
    RendererInServerPtr->standByEnable_ = g_fuzzUtils.GetData<bool>();
    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->HandleOperationStarted();
}

void ReConfigDupStreamCallbackFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    size_t size = 4;
    int32_t innerCapId = 0;
    uint32_t streamIndex = 0;
    size_t cacheSize = 16;

    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId] = std::make_shared<StreamCallbacks>(streamIndex,
        RendererInServerPtr);
    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId]->dupRingBuffer_ =
        std::make_unique<AudioRingCache>(cacheSize);
    RendererInServerPtr->offloadEnable_ = true;
    RendererInServerPtr->dupSpanSizeInFrame_ = size;
    RendererInServerPtr->dupTotalSizeInFrame_ = g_fuzzUtils.GetData<size_t>();
    RendererInServerPtr->ReConfigDupStreamCallback();
}

void PrepareOutputBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int len = 2;
    RingBufferWrapper bufferDesc;
    bufferDesc.dataLength = len;
    bufferDesc.basicBufferDescs[0] = BasicBufferDesc{};
    bufferDesc.basicBufferDescs[1] = BasicBufferDesc{};
    RendererInServerPtr->PrepareOutputBuffer(bufferDesc);
}

void UpdateStreamInfoFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.audioMode = AUDIO_MODE_RECORD;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->ProcessManagerType();
}

void OnStatusUpdateSubFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    BasicBufferInfo bufferInfo;
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_ = &bufferInfo;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    size_t length = 10;
    RendererInServerPtr->DequeueBuffer(length);
}

void IsInvalidBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    size_t size = 4;
    uint64_t num = 12;
    uint64_t frame = 4;
    uint32_t total = 16;
    BasicBufferInfo bufferInfo;
    RendererInServerPtr->Init();
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_ = &bufferInfo;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
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
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->InitLatencyMeasurement();
}

void DetectLatencyFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint8_t arr[8] = {0};
    uint8_t *inputData = arr;
    size_t requestDataLen = 0;
    RendererInServerPtr->Init();
    RendererInServerPtr->InitLatencyMeasurement();
    RendererInServerPtr->DetectLatency(inputData, requestDataLen);
}

void WriteData2FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    int8_t data[8] = {0};
    int8_t *inputData = data;
    size_t requestDataLen = 0;
    size_t size = 4;
    uint64_t num = 12;
    uint64_t frame = 4;
    uint32_t total = 16;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    BasicBufferInfo bufferInfo;
    RendererInServerPtr->Init();
    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_ = &bufferInfo;
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(num);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(frame);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = total;
    RendererInServerPtr->spanSizeInFrame_ = size;
    RendererInServerPtr->WriteData(inputData, requestDataLen);
}

void WriteDataInStaticModeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    int8_t data[8] = {0};
    int8_t *inputData = data;
    size_t requestDataLen = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    uint32_t frame = 2;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    RendererInServerPtr->staticBufferProvider_ = std::make_shared<AudioStaticBufferProvider>(config.streamInfo,
        RendererInServerPtr->audioServerBuffer_);
    RendererInServerPtr->byteSizePerFrame_ = frame;
    RendererInServerPtr->WriteDataInStaticMode(inputData, requestDataLen);
}

void InnerCaptureOtherStreamFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    BufferDesc bufferDesc;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;
    CaptureInfo captureInfo;
    int32_t innerCapId = 0;

    captureInfo.isInnerCapEnabled = true;
    captureInfo.dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    captureInfo.dualDeviceName = "test";
    RendererInServerPtr->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);
}

void OnWriteData1FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    size_t length = 4;
    RendererInServerPtr->OnWriteData(length);
}

void RequestHandleDataFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    uint64_t syncFramePts = 4;
    uint32_t size = 8;
    RendererInServerPtr->RequestHandleData(syncFramePts, size);
}

void UpdateWriteIndexFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET,
        AUDIO_FLAG_VOIP_DIRECT);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->managerType_ = DIRECT_PLAYBACK;
    RendererInServerPtr->needForceWrite_ = 1;
    RendererInServerPtr->afterDrain = true;
    RendererInServerPtr->UpdateWriteIndex();
}

void StandbyFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    bool isStandby = g_fuzzUtils.GetData<bool>();
    RendererInServerPtr->Standby(isStandby);
}

void StartInnerDuringStandbyFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->StartInnerDuringStandby();
}

void StartStreamByTypeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->StartStreamByType();
}

void DualToneStreamInStartFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->isDualToneEnabled_ = true;
    RendererInServerPtr->StartStreamByType();
}

void FlushOhAudioBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);
    RendererInServerPtr->FlushOhAudioBuffer();
}

void DrainFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->OnStatusUpdate(OPERATION_STARTED);
    RendererInServerPtr->Drain(false);
}

void RemoveIdForInjectorFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    RendererInServerPtr->RemoveIdForInjector();
}

void DisableAllInnerCapFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->DisableAllInnerCap();
}

void DisableDualToneFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->isDualToneEnabled_ = false;
    RendererInServerPtr->DisableDualTone();

    RendererInServerPtr->isDualToneEnabled_ = true;
    RendererInServerPtr->DisableDualTone();
}

void PreDualToneBufferSilenceForOffloadFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->offloadEnable_ = true;
    std::string dupSinkName = "test";
    int32_t ret = IStreamManager::GetDualPlaybackManager().CreateRender(config, RendererInServerPtr->dualToneStream_,
        dupSinkName);
    CHECK_AND_RETURN(ret == SUCCESS && RendererInServerPtr->dualToneStream_ != nullptr);
    RendererInServerPtr->PreDualToneBufferSilenceForOffload();
}

void OnWriteData2FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int8_t buf[8] = {0};
    int8_t *inputData = buf;
    size_t requestDataLen = 0;
    RendererInServerPtr->audioStreamChecker_ = std::make_shared<AudioStreamChecker>(config);
    CHECK_AND_RETURN(RendererInServerPtr->audioStreamChecker_ != nullptr);
    RendererInServerPtr->OnWriteData(inputData, requestDataLen);
}

void SetFirstWriteDataFlagFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint32_t streamIndex = 0;
    bool isFirstWriteDataFlag = true;
    auto streamCbPtr = std::make_shared<StreamCallbacks>(streamIndex, RendererInServerPtr);
    CHECK_AND_RETURN(streamCbPtr != nullptr);
    streamCbPtr->SetFirstWriteDataFlag(isFirstWriteDataFlag);
}

void CheckIsWriteFirstFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint32_t streamIndex = 0;
    auto streamCbPtr = std::make_shared<StreamCallbacks>(streamIndex, RendererInServerPtr);
    CHECK_AND_RETURN(streamCbPtr != nullptr);
    streamCbPtr->CheckIsWriteFirst();
}

void GetDupRingBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    uint32_t streamIndex = 0;
    auto streamCbPtr = std::make_shared<StreamCallbacks>(streamIndex, RendererInServerPtr);
    CHECK_AND_RETURN(streamCbPtr != nullptr);
    streamCbPtr->GetDupRingBuffer();
}

void SetOffloadModeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int32_t state = 0;
    bool isAppBack = false;
    std::string dupSinkName = "test";
    int32_t ret = IStreamManager::GetDualPlaybackManager().CreateRender(config, RendererInServerPtr->dualToneStream_,
        dupSinkName);
    CHECK_AND_RETURN(ret == SUCCESS && RendererInServerPtr->dualToneStream_ != nullptr);
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->isDualToneEnabled_ = true;
    RendererInServerPtr->SetOffloadMode(state, isAppBack);
}

void UnsetOffloadModeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->isDualToneEnabled_ = true;
    std::string dupSinkName = "test";
    int32_t ret = IStreamManager::GetDualPlaybackManager().CreateRender(config, RendererInServerPtr->dualToneStream_,
        dupSinkName);
    CHECK_AND_RETURN(ret == SUCCESS && RendererInServerPtr->dualToneStream_ != nullptr);
    RendererInServerPtr->UnsetOffloadMode();
}

void IsHighResolution1FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.deviceType = DEVICE_TYPE_MIC;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->IsHighResolution();
}

void IsHighResolution2FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.streamInfo.format = SAMPLE_S16LE;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->IsHighResolution();
}

void IsHighResolution3FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.streamType = STREAM_MUSIC;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_384000;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->IsHighResolution();
}

void IsHighResolution4FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.streamType = STREAM_MUSIC;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_96000;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->IsHighResolution();
}

void SetLoudnessGainFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    float loudnessGain = 2.0;
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->SetLoudnessGain(loudnessGain);
}

void SetMuteFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    bool isMute = true;
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->isDualToneEnabled_ = true;
    RendererInServerPtr->offloadEnable_ = true;
    RendererInServerPtr->SetMute(isMute);
}

void WriteDupBufferInnerFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    BufferDesc bufferDesc;
    int32_t innerCapId = 0;
    uint32_t streamIndex = 0;
    size_t cacheSize = 16;

    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId] = std::make_shared<StreamCallbacks>(streamIndex,
        RendererInServerPtr);
    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId]->dupRingBuffer_ =
        std::make_unique<AudioRingCache>(cacheSize);
    RendererInServerPtr->WriteDupBufferInner(bufferDesc, innerCapId);
}

void WriteSilenceDupBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    BufferDesc bufferDesc;
    BufferWrap bufferWrap;
    int32_t innerCapId = 0;
    int32_t size = 4;
    uint32_t streamIndex = 0;
    size_t cacheSize = 16;

    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId] = std::make_shared<StreamCallbacks>(streamIndex,
        RendererInServerPtr);
    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId]->dupRingBuffer_ =
        std::make_unique<AudioRingCache>(cacheSize);
    bufferWrap.dataSize = size;
    RendererInServerPtr->WriteSilenceDupBuffer(bufferDesc, bufferWrap, innerCapId);
}

void UpdateLatestForWorkgroupFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    float systemVolume = 1.0;

    RendererInServerPtr->UpdateLatestForWorkgroup(systemVolume);
}

void CollectInfosForWorkgroupFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);
    RendererInServerPtr->Init();
    float systemVolume = 1.0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);
    RendererInServerPtr->latestForWorkgroupInited_ = false;
    RendererInServerPtr->CollectInfosForWorkgroup(systemVolume);
}

void InitDupBufferInnerFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int32_t innerCapId = 0;
    uint32_t streamIndex = 0;
    size_t cacheSize = 16;

    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId] = std::make_shared<StreamCallbacks>(streamIndex,
        RendererInServerPtr);
    RendererInServerPtr->innerCapIdToDupStreamCallbackMap_[innerCapId]->dupRingBuffer_ =
        std::make_unique<AudioRingCache>(cacheSize);
    RendererInServerPtr->InitDupBufferInner(innerCapId);
}

void InitSoftLinkFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int32_t innerCapId = 0;
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->InitSoftLink(innerCapId);
}

void DestroySoftLinkFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int32_t innerCapId = 0;
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->DestroySoftLink(innerCapId);
}

void InitSoftLinkVolumeFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;
    std::shared_ptr<HPAE::IHpaeSoftLink> softLinkPtr = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    CHECK_AND_RETURN(softLinkPtr != nullptr);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);

    RendererInServerPtr->InitSoftLinkVolume(softLinkPtr);
}

void IsEnabledAndValidSoftLinkFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    SoftLinkInfo softLinkInfo;

    softLinkInfo.isSoftLinkEnabled = true;
    RendererInServerPtr->IsEnabledAndValidSoftLink(softLinkInfo);
}

void IsMovieOffloadStreamFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->IsMovieOffloadStream();
}

void SetTargetFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RenderTarget target = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    int32_t ret = 0;

    RendererInServerPtr->status_ = I_STATUS_STOPPED;
    RendererInServerPtr->SetTarget(target, ret);
}

void ClearInnerCapBufferForInjectFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->captureInfos_[0].isInnerCapEnabled = true;
    RendererInServerPtr->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->captureInfos_[0].dualDeviceName = "test";
    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->ClearInnerCapBufferForInject();
}

void WaitForDataConnectionFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->isDataLinkConnected_ = false;
    RendererInServerPtr->WaitForDataConnection();
}

void SetLoopTimesFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int64_t bufferLoopTimes = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    RendererInServerPtr->staticBufferProvider_ = std::make_shared<AudioStaticBufferProvider>(config.streamInfo,
        RendererInServerPtr->audioServerBuffer_);
    RendererInServerPtr->SetLoopTimes(bufferLoopTimes);
}

void ProcessAndSetStaticBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    RendererInServerPtr->staticBufferProvider_ = std::make_shared<AudioStaticBufferProvider>(config.streamInfo,
        RendererInServerPtr->audioServerBuffer_);
    RendererInServerPtr->ProcessAndSetStaticBuffer();
}

void SelectModeAndWriteData1FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.isStatic = true;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int8_t arr[8] = {0};
    int8_t *inputData = arr;
    size_t requestDataLen = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    RendererInServerPtr->staticBufferProvider_ = std::make_shared<AudioStaticBufferProvider>(config.streamInfo,
        RendererInServerPtr->audioServerBuffer_);
    RendererInServerPtr->SelectModeAndWriteData(inputData, requestDataLen, false);
}

void SelectModeAndWriteData2FuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.isStatic = false;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int8_t arr[8] = {0};
    int8_t *inputData = arr;
    size_t requestDataLen = 0;

    RendererInServerPtr->SelectModeAndWriteData(inputData, requestDataLen, false);
}

void CreateServerBufferFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.isStatic = false;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    RendererInServerPtr->CreateServerBuffer();
}

void MarkStaticFadeOutFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.isStatic = true;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    RendererInServerPtr->staticBufferProvider_ = std::make_shared<AudioStaticBufferProvider>(config.streamInfo,
        RendererInServerPtr->audioServerBuffer_);
    RendererInServerPtr->MarkStaticFadeOut();
}

void MarkStaticFadeInFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.rendererInfo.isStatic = true;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;

    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    RendererInServerPtr->staticBufferProvider_ = std::make_shared<AudioStaticBufferProvider>(config.streamInfo,
        RendererInServerPtr->audioServerBuffer_);
    RendererInServerPtr->MarkStaticFadeIn(true);
}

void GetLatencyWithFlagFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    uint64_t latency = 0;
    LatencyFlag flag = LATENCY_FLAG_ALL;

    RendererInServerPtr->GetLatencyWithFlag(latency, flag);
}

void HandleIsWriteFirstFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    bool isWriteFirst = true;
    RendererInServerPtr->HandleIsWriteFirst(isWriteFirst);
}

void IsWriteFirstFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->IsWriteFirst();
}

void PauseDuringStandbyFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 8;
    uint32_t byteSizePerFrame = 4;
    uint64_t num = 12;
    uint64_t frame = 4;
    uint32_t total = 16;
    BasicBufferInfo bufferInfo;
    RendererInServerPtr->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    CHECK_AND_RETURN(RendererInServerPtr->audioServerBuffer_ != nullptr);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_ = &bufferInfo;
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(num);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(frame);
    RendererInServerPtr->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = total;
    RendererInServerPtr->playerDfx_ = std::make_unique<PlayerDfxWriter>(config.appInfo, 0);
    RendererInServerPtr->streamIndex_ = 0;
    RendererInServerPtr->PauseDuringStandby();
}

void IsMovieStreamFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->IsMovieStream();
}

void HandleOffloadStreamFuzzTest()
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AudioProcessConfig config = InitAudioProcessConfig(testStreamInfo);
    config.streamType = STREAM_MOVIE;
    config.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> weakListener = streamListenerHolder;
    auto RendererInServerPtr = std::make_shared<RendererInServer>(config, weakListener);
    CHECK_AND_RETURN(RendererInServerPtr != nullptr);

    RendererInServerPtr->Init();
    int32_t captureId = 1;
    CaptureInfo captureInfo;
    uint32_t sinkIdx = 0;
    uint32_t sourceIdx = 0;
    HPAE::SoftLinkMode mode = HPAE::SoftLinkMode::HEARING_AID;

    RendererInServerPtr->softLinkInfos_[0].isSoftLinkEnabled = true;
    RendererInServerPtr->softLinkInfos_[0].softLink = std::make_shared<HPAE::HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    RendererInServerPtr->status_ = I_STATUS_STARTED;
    captureInfo.dupStream = std::make_shared<ProRendererStreamImpl>(config, true);
    RendererInServerPtr->HandleOffloadStream(captureId, captureInfo);
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
    WriteData2FuzzTest,
    WriteDataInStaticModeFuzzTest,
    InnerCaptureOtherStreamFuzzTest,
    OnWriteData1FuzzTest,
    RequestHandleDataFuzzTest,
    UpdateWriteIndexFuzzTest,
    StandbyFuzzTest,
    StartInnerDuringStandbyFuzzTest,
    StartStreamByTypeFuzzTest,
    DualToneStreamInStartFuzzTest,
    FlushOhAudioBufferFuzzTest,
    DrainFuzzTest,
    RemoveIdForInjectorFuzzTest,
    DisableAllInnerCapFuzzTest,
    DisableDualToneFuzzTest,
    PreDualToneBufferSilenceForOffloadFuzzTest,
    OnWriteData2FuzzTest,
    SetFirstWriteDataFlagFuzzTest,
    CheckIsWriteFirstFuzzTest,
    SetOffloadModeFuzzTest,
    UnsetOffloadModeFuzzTest,
    IsHighResolution1FuzzTest,
    IsHighResolution2FuzzTest,
    IsHighResolution3FuzzTest,
    IsHighResolution4FuzzTest,
    SetLoudnessGainFuzzTest,
    SetMuteFuzzTest,
    WriteDupBufferInnerFuzzTest,
    WriteSilenceDupBufferFuzzTest,
    UpdateLatestForWorkgroupFuzzTest,
    CollectInfosForWorkgroupFuzzTest,
    InitDupBufferInnerFuzzTest,
    InitSoftLinkFuzzTest,
    DestroySoftLinkFuzzTest,
    InitSoftLinkVolumeFuzzTest,
    IsEnabledAndValidSoftLinkFuzzTest,
    IsMovieOffloadStreamFuzzTest,
    SetTargetFuzzTest,
    ClearInnerCapBufferForInjectFuzzTest,
    WaitForDataConnectionFuzzTest,
    SetLoopTimesFuzzTest,
    ProcessAndSetStaticBufferFuzzTest,
    SelectModeAndWriteData1FuzzTest,
    SelectModeAndWriteData2FuzzTest,
    CreateServerBufferFuzzTest,
    MarkStaticFadeOutFuzzTest,
    MarkStaticFadeInFuzzTest,
    GetLatencyWithFlagFuzzTest,
    HandleIsWriteFirstFuzzTest,
    IsWriteFirstFuzzTest,
    PauseDuringStandbyFuzzTest,
    IsMovieStreamFuzzTest,
    GetDupRingBufferFuzzTest,
    HandleOffloadStreamFuzzTest,
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