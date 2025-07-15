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
#ifndef LOG_TAG
#define LOG_TAG "DirectPlayBackEngine"
#endif

#include "audio_common_converter.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "direct_playback_engine.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "audio_performance_monitor.h"
#include "audio_volume.h"
#include "format_converter.h"
#include "audio_service.h"

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t MAX_FAILURE_NUM = 50;
static constexpr int32_t DIRECT_STOP_TIMEOUT_IN_SEC = 8; // 8S
static const std::string EAC3_SINK_NAME = "eac3";
static const char *PRIMARY_ADAPTER_TYPE = "primary";

DirectPlayBackEngine::DirectPlayBackEngine()
    : isStart_(false),
      isInit_(false),
      failedCount_(0),
      latency_(0),
      stream_(nullptr),
      uChannel_(0),
      format_(sizeof(int32_t)),
      uSampleRate_(0)
{
    AUDIO_INFO_LOG("Constructor");
}

DirectPlayBackEngine::~DirectPlayBackEngine()
{
    failedCount_ = 0;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (sink && sink->IsInited()) {
        sink->Stop();
        sink->DeInit();
    }
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
    isStart_ = false;
}

int32_t DirectPlayBackEngine::Init(const AudioDeviceDescriptor &type, bool isVoip)
{
    if (!isInit_) {
        device_ = type;
        return SUCCESS;
    }
    if (type.deviceType_ != device_.deviceType_) {
        device_ = type;
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
        if (sink && sink->IsInited()) {
            sink->Stop();
            sink->DeInit();
        }
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
    }
    return SUCCESS;
}

int32_t DirectPlayBackEngine::Start()
{
    AUDIO_INFO_LOG("Enter in");
    int32_t ret = SUCCESS;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "null sink!");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_NOT_STARTED, "sink Not Inited! Init the sink first!");
    failedCount_ = 0;
    latency_ = 0;
    if (!isStart_) {
        ret = sink->Start();
        isStart_ = true;
    }
    return ret;
}

int32_t DirectPlayBackEngine::Stop()
{
    AUDIO_INFO_LOG("Enter");
    int32_t ret = SUCCESS;
    if (!isStart_) {
        AUDIO_INFO_LOG("already stopped");
        return ret;
    }
    AudioXCollie audioXCollie(
        "DirectPlayBackEngine::Stop", DIRECT_STOP_TIMEOUT_IN_SEC,
        [](void *) { AUDIO_ERR_LOG("stop timeout"); }, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    failedCount_ = 0;
    ret = StopAudioSink();
    isStart_ = false;
    return ret;
}

int32_t DirectPlayBackEngine::StopAudioSink()
{
    int32_t ret = SUCCESS;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (sink && sink->IsInited()) {
        ret = sink->Stop();
    } else {
        AUDIO_ERR_LOG("sink is null or not init");
    }
    return ret;
}

int32_t DirectPlayBackEngine::Pause()
{
    AUDIO_INFO_LOG("Enter");
    if (!isStart_) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }
    AudioXCollie audioXCollie(
        "DirectPlayBackEngine::Pause", DIRECT_STOP_TIMEOUT_IN_SEC,
        [](void *) { AUDIO_ERR_LOG("stop timeout"); }, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    failedCount_ = 0;
    int32_t ret = StopAudioSink();
    isStart_ = false;
    return ret;
}

int32_t DirectPlayBackEngine::Flush()
{
    AUDIO_INFO_LOG("Enter");
    return SUCCESS;
}

void DirectPlayBackEngine::DirectCallback(const RenderCallbackType type)
{
    switch (type) {
        case CB_NONBLOCK_WRITE_COMPLETED: { //need more data
            MixStreams();
            break;
        }
        case CB_DRAIN_COMPLETED:
        case CB_FLUSH_COMPLETED:
        case CB_RENDER_FULL:
        case CB_ERROR_OCCUR:
            break;
        default:
            break;
    }
}

int32_t DirectPlayBackEngine::RegisterWriteCallback()
{
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "null sink");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_NOT_STARTED, "sink Not Inited! Init the sink first");

    std::function<void(const RenderCallbackType type)> callback =
        std::bind(&DirectPlayBackEngine::DirectCallback, this, std::placeholders::_1);
    return sink->RegistDirectHdiCallback(callback);
}

void DirectPlayBackEngine::DoRenderFrame(std::vector<char> &audioBufferConverted, int32_t index, int32_t appUid)
{
    uint64_t written = 0;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN(sink != nullptr);
    sink->RenderFrame(*audioBufferConverted.data(), audioBufferConverted.size(), written);
    CHECK_AND_RETURN(stream_ != nullptr);
    stream_->ReturnIndex(index);
}

void DirectPlayBackEngine::MixStreams()
{
    if (stream_ == nullptr) {
        AUDIO_INFO_LOG("stream is nullptr");
        return;
    }
    if (failedCount_ >= MAX_FAILURE_NUM) {
        AUDIO_WARNING_LOG("failed count is overflow.");
        return;
    }
    std::vector<char> audioBuffer;
    int32_t appUid = stream_->GetAudioProcessConfig().appInfo.appUid;
    int32_t index = -1;
    int32_t result = stream_->Peek(&audioBuffer, index);
    uint32_t sessionId = stream_->GetStreamIndex();
    if (index < 0) {
        AUDIO_WARNING_LOG("peek buffer failed.result:%{public}d,buffer size:%{public}d", result, index);
        AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_DIRECT_OUT, appUid);
        stream_->ReturnIndex(index);
        failedCount_++;
        return;
    }
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_DIRECT_OUT, appUid);
    failedCount_ = 0;
    DoRenderFrame(audioBuffer, index, appUid);
}

int32_t DirectPlayBackEngine::AddRenderer(const std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_INFO_LOG("Enter add");
    CHECK_AND_RETURN_RET_LOG(stream != nullptr, ERR_INVALID_PARAM, "stream is null");
    if (!stream_) {
        AudioProcessConfig config = stream->GetAudioProcessConfig();
        int32_t result = InitSink(config.streamInfo);
        if (result == SUCCESS) {
            stream_ = stream;
            isInit_ = true;
            RegisterWriteCallback();
        }
        return result;
    } else if (stream->GetStreamIndex() != stream_->GetStreamIndex()) {
        return ERROR_UNSUPPORTED;
    }
    return SUCCESS;
}

void DirectPlayBackEngine::RemoveRenderer(const std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_INFO_LOG("step in remove");
    CHECK_AND_RETURN_LOG(stream != nullptr, "stream is null");
    if (stream_ == nullptr) {
        AUDIO_INFO_LOG("stream already removed.");
        return;
    }
    if (stream->GetStreamIndex() == stream_->GetStreamIndex()) {
        Stop();
        stream_ = nullptr;
    }
}

bool DirectPlayBackEngine::IsPlaybackEngineRunning() const noexcept
{
    return isStart_;
}

int32_t DirectPlayBackEngine::GetDirectFormatByteSize(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::SAMPLE_S16LE:
            return sizeof(int16_t);
        case AudioSampleFormat::SAMPLE_S32LE:
        case AudioSampleFormat::SAMPLE_F32LE:
            return sizeof(int32_t);
        default:
            return sizeof(int32_t);
    }
}

int32_t DirectPlayBackEngine::InitSink(const AudioStreamInfo &clientStreamInfo)
{
    uint32_t channel = clientStreamInfo.channels;
    uint32_t samplingRate = clientStreamInfo.samplingRate;
    AudioSampleFormat format = clientStreamInfo.format;
    AudioChannelLayout channelLayout = clientStreamInfo.channelLayout;

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (isInit_ && sink) {
        if (uChannel_ != channel || format_ != format || uSampleRate_ != samplingRate) {
            if (sink && sink->IsInited()) {
                sink->Stop();
                sink->DeInit();
            }
        } else {
            return SUCCESS;
        }
    }
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
    return InitSink(channel, format, samplingRate, channelLayout);
}

int32_t DirectPlayBackEngine::InitSink(uint32_t channel, AudioSampleFormat format, uint32_t rate,
    AudioChannelLayout layout)
{
    std::string sinkName = EAC3_SINK_NAME;
    renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_EAC3, sinkName, true);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("get render fail, sinkName: %{public}s", sinkName.c_str());
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERR_INVALID_HANDLE;
    }
    IAudioSinkAttr attr = {};
    attr.adapterName = PRIMARY_ADAPTER_TYPE;
    attr.sampleRate = rate;
    attr.channel = channel;
    attr.format = format;
    attr.channelLayout = layout;
    attr.deviceType = device_.deviceType_;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    AUDIO_INFO_LOG("sinkName:%{public}s,device:%{public}d,sample rate:%{public}d,format:%{public}d,channel:%{public}d",
        sinkName.c_str(), attr.deviceType, attr.sampleRate, attr.format, attr.channel);
    int32_t ret = sink->Init(attr);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("init sink fail, sinkName: %{public}s", sinkName.c_str());
        return ret;
    }
    float volume = 1.0f;
    ret = sink->SetVolume(volume, volume);
    uChannel_ = attr.channel;
    uSampleRate_ = attr.sampleRate;
    format_ = GetDirectFormatByteSize(attr.format);

    return ret;
}

uint64_t DirectPlayBackEngine::GetLatency() noexcept
{
    return latency_;
}
} // namespace AudioStandard
} // namespace OHOS
