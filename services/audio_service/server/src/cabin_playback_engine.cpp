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
#define LOG_TAG "Cabin3DADirectPlaybackEngine"
#endif

#include "audio_common_converter.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "cabin_playback_engine.h"
#include "audio_performance_monitor.h"
#include "audio_volume.h"
#include "format_converter.h"
#include "audio_service.h"
#include "audio_mute_factor_manager.h"
#include "audio_sink_latency_fetcher.h"

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t AUDIO_VIVID_3DA_STOP_TIMEOUT_IN_SEC = 8; // 8S
static const std::string AV_SINK_NAME = "3DA";
static const char *PRIMARY_ADAPTER_TYPE = "primary";
constexpr int32_t DELTA_TIME = 4000000; // 4ms
constexpr int32_t PERIOD_NS_48K_1024 = 21333333; // 21.333ms
constexpr int32_t AUDIO_DEFAULT_LATENCY_US = 160000;
constexpr int32_t AUDIO_FRAME_WORK_LATENCY_US = 40000;
constexpr uint32_t DEFAULT_SINK_LATENCY_MS = 40;

CabinPlayBackEngine::CabinPlayBackEngine()
    : isVoip_(false),
      isStart_(false),
      isInit_(false),
      writeCount_(0),
      fwkSyncTime_(0),
      uChannel_(0),
      uformat_(sizeof(int32_t)),
      uSampleRate_(0),
      firstSetVolume_(true),
      latency_(0),
      stream_(nullptr)
{
    AUDIO_INFO_LOG("Constructor 3da direct");
}

CabinPlayBackEngine::~CabinPlayBackEngine()
{
    writeCount_ = 0;
    fwkSyncTime_ = 0;
    if (playbackThread_) {
        playbackThread_->Stop();
        playbackThread_ = nullptr;
    }
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (sink && sink->IsInited()) {
        sink->Stop();
        sink->DeInit();
    }
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
    isStart_ = false;
    DumpFileUtil::CloseDumpFile(&dump3DA_);
}

int32_t CabinPlayBackEngine::Init(const AudioDeviceDescriptor &type, bool isVoip)
{
    AUDIO_DEBUG_LOG("init enter 3da direct");
    if (isInit_) {
        isVoip_ = isVoip;
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

int32_t CabinPlayBackEngine::Start()
{
    AUDIO_DEBUG_LOG("Enter 3da direct start");
    int32_t ret = SUCCESS;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "null sink.");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_NOT_STARTED, "sink Not Inited! Init the sink first.");
    fwkSyncTime_ = static_cast<uint64_t>(ClockTime::GetCurNano());
    writeCount_ = 0;
    if (!playbackThread_) {
        playbackThread_ = std::make_unique<AudioThreadTask>("3DADirectThread");
        playbackThread_->RegisterJob([this] { this->PollAndWrite(); });
    }
    if (!isStart_) {
        ret = sink->Start();
        isStart_ = true;
    }
    if (!playbackThread_->CheckThreadIsRunning()) {
        playbackThread_->Start();
    }
    return ret;
}

void CabinPlayBackEngine::StandbySleep()
{
    if (fwkSyncTime_ == 0) {
        ClockTime::RelativeSleep(PERIOD_NS_48K_1024);
        return;
    }
    int64_t nextWriteTime = static_cast<int64_t>(fwkSyncTime_) +\
        static_cast<int64_t>(writeCount_) * PERIOD_NS_48K_1024 + DELTA_TIME;
    ClockTime::AbsoluteSleep(nextWriteTime);
}

void CabinPlayBackEngine::PollAndWrite()
{
    AUDIO_DEBUG_LOG("PollAndWrite thread started 3da direct");
    if (stream_ == nullptr) {
        StandbySleep();
        return;
    }
    std::vector<char> audioBuffer;
    int32_t index = -1;
    int32_t result = stream_->Peek(&audioBuffer, index);
    int32_t appUid = stream_->GetAudioProcessConfig().appInfo.appUid;
    uint32_t sessionId = stream_->GetStreamIndex();
    if (result < 0) {
        AUDIO_WARNING_LOG("peek buffer failed.result:%{public}d,buffer size:%{public}d", result, index);
        AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_OUT_3DA_DIRECT, appUid);
        stream_->ReturnIndex(index);
        ClockTime::RelativeSleep(PERIOD_NS_48K_1024);
        return;
    }
    
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_OUT_3DA_DIRECT, appUid);
    AdjustVolume();
    writeCount_++;
    DoRenderFrame(audioBuffer, index, appUid);
    StandbySleep();

    DumpFileUtil::WriteDumpFile(dump3DA_, static_cast<void *>(audioBuffer.data()), audioBuffer.size());
}


int32_t CabinPlayBackEngine::Stop()
{
    AUDIO_DEBUG_LOG("Enter stop");
    int32_t ret = SUCCESS;
    if (!isStart_) {
        AUDIO_INFO_LOG("already stopped");
        return ret;
    }
    AudioXCollie audioXCollie(
        "CabinPlayBackEngine::Stop", AUDIO_VIVID_3DA_STOP_TIMEOUT_IN_SEC,
        [](void *) { AUDIO_ERR_LOG("stop timeout"); }, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    writeCount_ = 0;
    if (playbackThread_) {
        playbackThread_ -> Stop();
    }

    ret = StopAudioSink();
    isStart_ = false;
    return ret;
}

int32_t CabinPlayBackEngine::StopAudioSink()
{
    int32_t ret = SUCCESS;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (sink && sink->IsInited()) {
        ret = sink->Stop();
    } else {
        AUDIO_ERR_LOG("sink is null or not init.");
    }
    return ret;
}

int32_t CabinPlayBackEngine::Pause(bool isStandby)
{
    AUDIO_DEBUG_LOG("Enter pause");
    if (!isStart_) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }

    AudioXCollie audioXCollie(
        "CabinPlayBackEngine::Pause", AUDIO_VIVID_3DA_STOP_TIMEOUT_IN_SEC,
        [](void *) { AUDIO_ERR_LOG("stop timeout"); }, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    writeCount_ = 0;
    if (playbackThread_) {
        playbackThread_->Pause();
    }

    int32_t ret = StopAudioSink();
    isStart_ = false;
    return ret;
}

int32_t CabinPlayBackEngine::Flush()
{
    AUDIO_INFO_LOG("Enter");
    writeCount_ =0;
    fwkSyncTime_ = static_cast<uint64_t>(ClockTime::GetCurNano());
    return SUCCESS;
}

void CabinPlayBackEngine::AdjustVolume()
{
    uint32_t streamIndx = stream_->GetStreamIndex();
    AudioProcessConfig config = stream_->GetAudioProcessConfig();
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0u};
    float volumeEd = AudioVolume::GetInstance()->GetVolume(streamIndx, config.streamType,
        std::string(PRIMARY_ADAPTER_TYPE), &volumes);
    float volumeBg = volumes.volumeHistory;
    if ((!firstSetVolume_ && abs(volumeBg - volumeEd) > FLOAT_EPS) || firstSetVolume_) {
        AudioVolume::GetInstance()->SetHistoryVolume(streamIndx, volumeEd);
        AudioVolume::GetInstance()->Monitor(streamIndx, true);
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
        CHECK_AND_RETURN(sink != nullptr);
        sink->SetVolume(volumeEd, volumeEd);
        firstSetVolume_ = false;
    }
}

void CabinPlayBackEngine::DoRenderFrame(std::vector<char> &audioBufferConverted, int32_t index, int32_t appUid)
{
    uint64_t written = 0;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN(sink != nullptr);
    sink->RenderFrame(*audioBufferConverted.data(), audioBufferConverted.size(), written);
    CHECK_AND_RETURN(stream_ != nullptr);
    stream_->ReturnIndex(index);
}


int32_t CabinPlayBackEngine::AddRenderer(const std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_INFO_LOG("Enter add 3da direct");
    CHECK_AND_RETURN_RET_LOG(stream != nullptr, ERR_INVALID_PARAM, "stream is null");
    if (!stream_) {
        AudioProcessConfig config = stream->GetAudioProcessConfig();
        int32_t result = InitSink(config.streamInfo);
        if (result == SUCCESS) {
            stream_ = stream;
            isInit_ = true;
            RegisterSinkLatencyFetcherToStreamIfNeeded();
        }
        return result;
    } else if (stream->GetStreamIndex() != stream_->GetStreamIndex()) {
        return ERROR_UNSUPPORTED;
    }
    return SUCCESS;
}

void CabinPlayBackEngine::RemoveRenderer(const std::shared_ptr<IRendererStream> &stream)
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

int32_t CabinPlayBackEngine::GetFormatByteSize(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::SAMPLE_S16LE:
            return sizeof(int16_t);
        case AudioSampleFormat::SAMPLE_S24LE:
            return 3u;
        default:
            return sizeof(int32_t);
    }
}

int32_t CabinPlayBackEngine::InitSink(const AudioStreamInfo &clientStreamInfo)
{
    uint32_t channel = clientStreamInfo.channels;
    uint32_t samplingRate = clientStreamInfo.samplingRate;
    AudioSampleFormat format = clientStreamInfo.format;
    AudioChannelLayout channelLayout = clientStreamInfo.channelLayout;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (isInit_ && sink) {
        if (uChannel_ != channel || uformat_ != format || uSampleRate_ != samplingRate) {
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

int32_t CabinPlayBackEngine::InitSink(uint32_t channel, AudioSampleFormat format, uint32_t rate,
    AudioChannelLayout layout)
{
    std::string sinkName = AV_SINK_NAME;
    renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT,
        sinkName, true);
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
    attr.encodingType = ENCODING_AUDIOVIVID_3DA_DIRECT;
    AUDIO_INFO_LOG("sinkName:%{public}s,device:%{public}d,sample rate:%{public}d,format:%{public}d,channel:%{public}d,"
        "encodingType: %{public}d",
        sinkName.c_str(), attr.deviceType, attr.sampleRate, attr.format, attr.channel, attr.encodingType);
    int32_t ret = sink->Init(attr);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("init sink fail, sinkName: %{public}s", sinkName.c_str());
        return ret;
    }
    RegisterSinkLatencyFetcher(renderId_);
    auto mdmMute = AudioMuteFactorManager::GetInstance().GetMdmMuteStatus();
    float volume = mdmMute ? 0.0f : 1.0f;
    ret = sink->SetVolume(volume, volume);
    uChannel_ = attr.channel;
    uSampleRate_ = attr.sampleRate;
    uformat_ = GetFormatByteSize(attr.format);

    dumpFileName_ = "3da_playback_engine_.pcm";
    AUDIO_DEBUG_LOG("dump file name: %{public}s", dumpFileName_.c_str());
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dump3DA_);

    return ret;
}

uint64_t CabinPlayBackEngine::GetLatency() noexcept
{
    if (!isStart_) {
        return 0;
    }
    if (latency_ > 0) {
        return latency_;
    }
    uint32_t latency = 0;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN_RET(sink != nullptr, 0);
    if (sink->GetLatency(latency) == 0) {
        latency_ = latency * AUDIO_US_PER_MS + AUDIO_FRAME_WORK_LATENCY_US;
    } else {
        AUDIO_INFO_LOG("get latency failed,use default");
        latency_ = AUDIO_DEFAULT_LATENCY_US;
    }
    AUDIO_INFO_LOG("latency value:%{public}" PRId64 " ns", latency_);
    return latency_;
}

void CabinPlayBackEngine::RegisterSinkLatencyFetcher(uint32_t renderId)
{
    SinkLatencyFetcherManager::GetInstance().RegisterProvider(renderId, [] (uint32_t renderId, uint32_t &latency)
        -> int32_t {
        latency = DEFAULT_SINK_LATENCY_MS; // preset default latency in ms from hdi provider
        std::shared_ptr<IAudioRenderSink> audioRendererSink =
            HdiAdapterManager::GetInstance().GetRenderSink(renderId, false);
        CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_OPERATION,
            "audioRendererSink is null, renderId %{public}u", renderId);
        int32_t ret = audioRendererSink->GetLatency(latency);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_LATENCY_DEFAULT_VALUE,
            "GetLatency failed, renderId %{public}u ret %{public}d, use default", renderId, ret);
        return SUCCESS;
    });
    auto fetcher = SinkLatencyFetcherManager::GetInstance().EnsureFetcher(renderId);
    CHECK_AND_RETURN_LOG(fetcher, "sinkLatencyFetcher is null, renderId %{public}u", renderId);
    uint32_t dummyLatency = 0;
    int32_t ret = fetcher(dummyLatency);
    CHECK_AND_RETURN_LOG(ret == SUCCESS,
        "Preload sink latency failed, renderId %{public}u, ret %{public}d", renderId, ret);
    sinkLatencyFetcher_ = fetcher;
}

void CabinPlayBackEngine::RegisterSinkLatencyFetcherToStreamIfNeeded()
{
    CHECK_AND_RETURN(stream_ != nullptr);
    auto fetcher = sinkLatencyFetcher_;
    CHECK_AND_RETURN(fetcher);
    stream_->RegisterSinkLatencyFetcher(fetcher);
}
} // namespace AudioStandard
} // namespace OHOS
