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

#include <securec.h>

#include "audio_log.h"
#include "capturer_in_server.h"
#include "ipc_stream_in_server.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t APPID_LENGTH = 10;
const int64_t STOP_TIME = 100;
const uint32_t NUM_10 = 10;

typedef void (*TestFuncs)();

vector<IOperation> IOperationVec = {
    OPERATION_INVALID,
    OPERATION_STARTED,
    OPERATION_PAUSED,
    OPERATION_STOPPED,
    OPERATION_FLUSHED,
    OPERATION_DRAINED,
    OPERATION_RELEASED,
    OPERATION_UNDERRUN,
    OPERATION_UNDERFLOW,
    OPERATION_SET_OFFLOAD_ENABLE,
    OPERATION_UNSET_OFFLOAD_ENABLE,
    OPERATION_DATA_LINK_CONNECTING,
    OPERATION_DATA_LINK_CONNECTED,
};

vector<IStatus> IStatusVec = {
    I_STATUS_INVALID,
    I_STATUS_IDLE,
    I_STATUS_STARTING,
    I_STATUS_STARTED,
    I_STATUS_PAUSING,
    I_STATUS_PAUSED,
    I_STATUS_FLUSHING_WHEN_STARTED,
    I_STATUS_FLUSHING_WHEN_PAUSED,
    I_STATUS_FLUSHING_WHEN_STOPPED,
    I_STATUS_DRAINING,
    I_STATUS_DRAINED,
    I_STATUS_STOPPING,
    I_STATUS_STOPPED,
    I_STATUS_RELEASING,
    I_STATUS_RELEASED,
};

vector<AudioSampleFormat> AudioSampleFormatVec = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH,
};

vector<SourceType> SourceTypeVec = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX,
};

class ICapturerStreamTest : public ICapturerStream {
public:
    int32_t GetStreamFramesRead(uint64_t &framesRead) override { return 0; }
    int32_t GetCurrentTimeStamp(uint64_t &timestamp) override { return 0; }
    int32_t GetLatency(uint64_t &latency) override { return 0; }
    void RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback) override { return; }
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override { return 0; }
    void GetByteSizePerFrame(size_t &byteSizePerFrame) const override { return; }
    void GetSpanSizePerFrame(size_t &spanSizeInFrame) const override { spanSizeInFrame = 0; }
    int32_t DropBuffer() override { return 0; }
    void SetStreamIndex(uint32_t index) override { return; }
    uint32_t GetStreamIndex() override { return 0; }
    int32_t Start() override { return 0; }
    int32_t Pause(bool isStandby = false) override { return 0; }
    int32_t Flush() override { return 0; }
    int32_t Drain(bool stopFlag = false) override { return 0; }
    int32_t Stop() override { return 0; }
    int32_t Release() override { return 0; }
    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override { return; }
    BufferDesc DequeueBuffer(size_t length) override
    {
        BufferDesc bufferDesc;
        return bufferDesc;
    }
};
int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override { return 0; }
class ConcreteIStreamListener : public IStreamListener {
    int32_t OnOperationHandled(Operation operation, int64_t result) { return SUCCESS; }
};

static AudioProcessConfig GetInnerCapConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = APPID_LENGTH;
    config.appInfo.appPid = APPID_LENGTH;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

std::shared_ptr<CapturerInServer> GetCapturerInServer()
{
    AudioProcessConfig config = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> innerListener = std::weak_ptr<IStreamListener>();
    return std::make_shared<CapturerInServer>(config, innerListener);
}

void OnStatusUpdateFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    if (streamListenerHolder == nullptr) {
        return;
    }
    capturerInServer_->streamListener_ = std::weak_ptr<IStreamListener>();
    if (capturerInServer_->streamListener_.lock() == nullptr) {
        capturerInServer_->streamListener_ = streamListenerHolder;
    }
    AppInfo appInfo;
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    capturerInServer_->recorderDfx_ = std::make_unique<RecorderDfxWriter>(appInfo, index);
    for (size_t i = 0; i < IOperationVec.size(); i++) {
        IOperation operation = IOperationVec[i];
        capturerInServer_->OnStatusUpdate(operation);
    }
}

void HandleOperationFlushedFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    for (size_t i = 0; i < IStatusVec.size(); i++) {
        capturerInServer_->status_ = IStatusVec[i];
        capturerInServer_->HandleOperationFlushed();
    }
}

void DequeueBufferFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    size_t length = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    capturerInServer_->DequeueBuffer(length);
}

void IsReadDataOverFlowFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    size_t length = g_fuzzUtils.GetData<size_t>();
    uint64_t currentWriteFrame = g_fuzzUtils.GetData<uint64_t>();
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    std::shared_ptr<IStreamListener> stateListener = std::make_shared<ConcreteIStreamListener>();
    if (stateListener == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    if (bufferInfo == nullptr) {
        return;
    }
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    if (capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ == nullptr) {
        return;
    }
    size_t frame = 100;
    capturerInServer_->spanSizeInFrame_ = frame;
    capturerInServer_->IsReadDataOverFlow(length, currentWriteFrame, stateListener);
}

void UpdateBufferTimeStampFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    uint32_t capturerSampleRate = g_fuzzUtils.GetData<uint32_t>();
    size_t readLen = NUM_10;
    capturerInServer_->capturerClock_ = std::make_shared<CapturerClock>(capturerSampleRate);
    if (capturerInServer_->capturerClock_ == nullptr) {
        return;
    }
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    capturerInServer_->processConfig_ = GetInnerCapConfig();
    for (size_t i = 0; i < AudioSampleFormatVec.size(); i++) {
        capturerInServer_->processConfig_.streamInfo.format = AudioSampleFormatVec[i];
        capturerInServer_->UpdateBufferTimeStamp(readLen);
    }
}

void ReadDataFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    if (streamListenerHolder == nullptr) {
        return;
    }
    capturerInServer_->streamListener_ = std::weak_ptr<IStreamListener>();
    if (capturerInServer_->streamListener_.lock() == nullptr) {
        capturerInServer_->streamListener_ = streamListenerHolder;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
    totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->muteFlag_ = true;
    size_t length = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ReadData(length);
    capturerInServer_->UpdateReadIndex();
    capturerInServer_->ResolveBuffer(capturerInServer_->audioServerBuffer_);
}

void OnReadDataFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    if (streamListenerHolder == nullptr) {
        return;
    }
    capturerInServer_->streamListener_ = std::weak_ptr<IStreamListener>();
    if (capturerInServer_->streamListener_.lock() == nullptr) {
        capturerInServer_->streamListener_ = streamListenerHolder;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
    totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    size_t length = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->OnReadData(length);
    int8_t outputData = g_fuzzUtils.GetData<int8_t>();
    size_t requestDataLen = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->OnReadData(&outputData, requestDataLen);
}

void StopSessionFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    capturerInServer_->StopSession();
}

void GetLastAudioDurationFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    capturerInServer_->lastStopTime_ = STOP_TIME;
    capturerInServer_->lastStartTime_ = 0;
    capturerInServer_->GetLastAudioDuration();
}

void RestoreSessionFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    RestoreInfo restoreInfo;
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    capturerInServer_->RestoreSession(restoreInfo);
}

void GetLatencyFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    capturerInServer_->GetLatency(latency);
}

void GetAudioTimeFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint64_t framePos = g_fuzzUtils.GetData<uint64_t>();
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % IStatusVec.size();
    capturerInServer_->status_ = IStatusVec[index];
    capturerInServer_->resetTime_ = g_fuzzUtils.GetData<bool>();
    capturerInServer_->GetAudioTime(framePos, timestamp);
}

void UpdatePlaybackCaptureConfigFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    #ifdef HAS_FEATURE_INNERCAPTURER
    AudioPlaybackCaptureConfig config;
    uint32_t sourceTypeCount = g_fuzzUtils.GetData<uint32_t>() % SourceTypeVec.size();
    capturerInServer_->processConfig_.capturerInfo.sourceType = SourceTypeVec[sourceTypeCount];
    capturerInServer_->UpdatePlaybackCaptureConfig(config);
    #endif
}

void UpdatePlaybackCaptureConfigInLegacyFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    #ifdef HAS_FEATURE_INNERCAPTURER
    AudioPlaybackCaptureConfig config;
    capturerInServer_->UpdatePlaybackCaptureConfigInLegacy(config);
    #endif
}

void PauseFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    capturerInServer_->status_ = I_STATUS_STARTED;
    capturerInServer_->needCheckBackground_ = g_fuzzUtils.GetData<bool>();
    capturerInServer_->streamIndex_ = g_fuzzUtils.GetData<uint32_t>();
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    capturerInServer_->Pause();
}

void FlushFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % IStatusVec.size();
    capturerInServer_->status_ = IStatusVec[index];
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    bufferInfo->curReadFrame = NUM_10;
    bufferInfo->curWriteFrame = NUM_10 + 1;
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    for (size_t i = 0; i < IStatusVec.size(); i++) {
        capturerInServer_->status_ = IStatusVec[i];
        capturerInServer_->Flush();
    }
    capturerInServer_->DrainAudioBuffer();
}

void StopFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % IStatusVec.size();
    capturerInServer_->status_ = IStatusVec[index];
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint32_t capturerSampleRate = g_fuzzUtils.GetData<uint32_t>();
    capturerInServer_->capturerClock_ = std::make_shared<CapturerClock>(capturerSampleRate);
    if (capturerInServer_->capturerClock_ == nullptr) {
        return;
    }
    capturerInServer_->needCheckBackground_ = g_fuzzUtils.GetData<bool>();
    capturerInServer_->Stop();
}

void ReleaseFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % IStatusVec.size();
    capturerInServer_->status_ = IStatusVec[index];
    capturerInServer_->needCheckBackground_ = g_fuzzUtils.GetData<bool>();
    capturerInServer_->processConfig_.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    capturerInServer_->Release();
}

void InitCacheBufferFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    size_t targetSize = g_fuzzUtils.GetData<size_t>();
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->spanSizeInBytes_ = 1 + g_fuzzUtils.GetData<size_t>();
    capturerInServer_->InitCacheBuffer(targetSize);
    capturerInServer_->InitCacheBuffer(targetSize);
    capturerInServer_->SetNonInterruptMute(true);
}

void RecordOverflowStatusTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    bool currentStatus = g_fuzzUtils.GetData<bool>();
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->RecordOverflowStatus(currentStatus);
    capturerInServer_->RecordOverflowStatus(!currentStatus);
}

void RebuildCaptureInjectorTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->rebuildFlag_ = true;
    capturerInServer_->processConfig_.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    capturerInServer_->RebuildCaptureInjector();
}

void RequestUserPrivacyAuthorityTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    if (streamListenerHolder == nullptr) {
        return;
    }
    capturerInServer_->streamListener_ = std::weak_ptr<IStreamListener>();
    if (capturerInServer_->streamListener_.lock() == nullptr) {
        capturerInServer_->streamListener_ = streamListenerHolder;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
    totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->processConfig_.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    capturerInServer_->status_ = I_STATUS_STARTING;
    capturerInServer_->RequestUserPrivacyAuthority();
    capturerInServer_->status_ = I_STATUS_PAUSING;
    capturerInServer_->RequestUserPrivacyAuthority();
}

void SetRebuildFlagTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->SetRebuildFlag();
}

void GetLastAudioDurationTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->GetLastAudioDuration();
}


void Start()
{
    std::shared_ptr<CapturerInServer> capturerInServer_ = GetCapturerInServer();
    if (capturerInServer_ == nullptr) {
        return;
    }
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    if (streamListenerHolder == nullptr) {
        return;
    }
    capturerInServer_->streamListener_ = std::weak_ptr<IStreamListener>();
    if (capturerInServer_->streamListener_.lock() == nullptr) {
        capturerInServer_->streamListener_ = streamListenerHolder;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    capturerInServer_->ringCache_ = AudioRingCache::Create(cacheSize);
    if (capturerInServer_->ringCache_ == nullptr) {
        return;
    }
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest>();
    if (capturerInServer_->stream_ == nullptr) {
        return;
    }
    uint32_t totalSizeInFrame = NUM_10;
    uint32_t spanSizeInFrame = NUM_10;
    uint32_t byteSizePerFrame = NUM_10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
    totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    if (capturerInServer_->audioServerBuffer_ == nullptr) {
        return;
    }
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->processConfig_.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    AppInfo appInfo;
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    capturerInServer_->recorderDfx_ = std::make_unique<RecorderDfxWriter>(appInfo, index);
    capturerInServer_->Start();
    capturerInServer_->processConfig_.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    capturerInServer_->needCheckBackground_ = false;
    capturerInServer_->StartInner();
}

vector<TestFuncs> g_testFuncs = {
    OnStatusUpdateFuzzTest,
    HandleOperationFlushedFuzzTest,
    DequeueBufferFuzzTest,
    IsReadDataOverFlowFuzzTest,
    UpdateBufferTimeStampFuzzTest,
    ReadDataFuzzTest,
    OnReadDataFuzzTest,
    StopSessionFuzzTest,
    GetLastAudioDurationFuzzTest,
    RestoreSessionFuzzTest,
    GetLatencyFuzzTest,
    GetAudioTimeFuzzTest,
    UpdatePlaybackCaptureConfigFuzzTest,
    UpdatePlaybackCaptureConfigInLegacyFuzzTest,
    PauseFuzzTest,
    FlushFuzzTest,
    StopFuzzTest,
    ReleaseFuzzTest,
    InitCacheBufferFuzzTest,
    RecordOverflowStatusTest,
    RebuildCaptureInjectorTest,
    RequestUserPrivacyAuthorityTest,
    SetRebuildFlagTest,
    GetLastAudioDurationTest,
    Start,
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
