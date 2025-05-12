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
#define LOG_TAG "NoneMixEngine"
#endif

#include "audio_common_converter.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "none_mix_engine.h"
#include "audio_performance_monitor.h"
#include "audio_volume.h"
#include "format_converter.h"
#include "audio_service.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DELTA_TIME = 4000000; // 4ms
constexpr int32_t PERIOD_NS = 20000000; // 20ms
constexpr int32_t AUDIO_US_PER_MS = 1000;
constexpr int32_t AUDIO_DEFAULT_LATENCY_US = 160000;
constexpr int32_t AUDIO_FRAME_WORK_LATENCY_US = 40000;
constexpr int32_t FADING_MS = 20; // 20ms
constexpr int32_t MAX_ERROR_COUNT = 50;
constexpr int16_t STEREO_CHANNEL_COUNT = 2;
constexpr int16_t HDI_STEREO_CHANNEL_LAYOUT = 3;
constexpr int16_t HDI_MONO_CHANNEL_LAYOUT = 4;
constexpr int32_t DIRECT_STOP_TIMEOUT_IN_SEC = 8; // 8S
constexpr int32_t DIRECT_SINK_STANDBY_TIMES = 8; // 8
const std::string THREAD_NAME = "noneMixThread";
const std::string VOIP_SINK_NAME = "voip";
const std::string DIRECT_SINK_NAME = "direct";
const char *SINK_ADAPTER_NAME = "primary";

NoneMixEngine::NoneMixEngine()
    : isVoip_(false),
      isStart_(false),
      isInit_(false),
      failedCount_(0),
      writeCount_(0),
      fwkSyncTime_(0),
      latency_(0),
      stream_(nullptr),
      startFadein_(false),
      startFadeout_(false),
      uChannel_(0),
      uFormat_(sizeof(int32_t)),
      uSampleRate_(0),
      firstSetVolume_(true)
{
    AUDIO_INFO_LOG("Constructor");
}

NoneMixEngine::~NoneMixEngine()
{
    writeCount_ = 0;
    failedCount_ = 0;
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
    startFadein_ = false;
    startFadeout_ = false;
}

int32_t NoneMixEngine::Init(const AudioDeviceDescriptor &type, bool isVoip)
{
    if (!isInit_) {
        isVoip_ = isVoip;
        device_ = type;
        return SUCCESS;
    }
    if (type.deviceType_ != device_.deviceType_ || isVoip_ != isVoip) {
        isVoip_ = isVoip;
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

int32_t NoneMixEngine::Start()
{
    AUDIO_INFO_LOG("Enter in");
    int32_t ret = SUCCESS;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "null sink");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_NOT_STARTED, "sink Not Inited! Init the sink first");
    fwkSyncTime_ = static_cast<uint64_t>(ClockTime::GetCurNano());
    writeCount_ = 0;
    failedCount_ = 0;
    if (!playbackThread_) {
        playbackThread_ = std::make_unique<AudioThreadTask>(THREAD_NAME);
        playbackThread_->RegisterJob([this] { this->MixStreams(); });
    }
    latency_ = 0;
    if (!isStart_) {
        startFadeout_ = false;
        startFadein_ = true;
        ret = sink->Start();
        isStart_ = true;
    }
    if (!playbackThread_->CheckThreadIsRunning()) {
        playbackThread_->Start();
    }
    return ret;
}

int32_t NoneMixEngine::Stop()
{
    AUDIO_INFO_LOG("Enter");
    int32_t ret = SUCCESS;
    if (!isStart_) {
        AUDIO_INFO_LOG("already stopped");
        return ret;
    }
    AudioXCollie audioXCollie(
        "NoneMixEngine::Stop", DIRECT_STOP_TIMEOUT_IN_SEC,
        [this](void *) { AUDIO_ERR_LOG("%{public}d stop timeout", isVoip_); }, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    writeCount_ = 0;
    failedCount_ = 0;
    if (playbackThread_) {
        startFadein_ = false;
        startFadeout_ = true;
        // wait until fadeout complete
        std::unique_lock fadingLock(fadingMutex_);
        cvFading_.wait_for(
            fadingLock, std::chrono::milliseconds(FADING_MS), [this] { return (!(startFadein_ || startFadeout_)); });
        playbackThread_->Stop();
        playbackThread_ = nullptr;
    }
    ClockTime::RelativeSleep(PERIOD_NS * DIRECT_SINK_STANDBY_TIMES);
    ret = StopAudioSink();
    isStart_ = false;
    return ret;
}

void NoneMixEngine::PauseAsync()
{
    // stop thread when failed 5 times,do not add logic inside.
    if (playbackThread_ && playbackThread_->CheckThreadIsRunning()) {
        playbackThread_->PauseAsync();
    }
    int32_t ret = StopAudioSink();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sink stop failed.ret:%{public}d", ret);
    }
    isStart_ = false;
}

int32_t NoneMixEngine::StopAudioSink()
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

int32_t NoneMixEngine::Pause()
{
    AUDIO_INFO_LOG("Enter");
    if (!isStart_) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }
    AudioXCollie audioXCollie(
        "NoneMixEngine::Pause", DIRECT_STOP_TIMEOUT_IN_SEC,
        [this](void *) { AUDIO_ERR_LOG("%{public}d stop timeout", isVoip_); }, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    writeCount_ = 0;
    failedCount_ = 0;
    if (playbackThread_) {
        startFadein_ = false;
        startFadeout_ = true;
        // wait until fadeout complete
        std::unique_lock fadingLock(fadingMutex_);
        cvFading_.wait_for(
            fadingLock, std::chrono::milliseconds(FADING_MS), [this] { return (!(startFadein_ || startFadeout_)); });
        playbackThread_->Pause();
    }
    ClockTime::RelativeSleep(PERIOD_NS * DIRECT_SINK_STANDBY_TIMES);
    int32_t ret = StopAudioSink();
    isStart_ = false;
    return ret;
}

int32_t NoneMixEngine::Flush()
{
    AUDIO_INFO_LOG("Enter");
    return SUCCESS;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
static void DoFadeInOut(T *dest, size_t count, bool isFadeOut, uint32_t channel)
{
    if (count <= 0) {
        return;
    }
    float fadeStep = 1.0f / count;
    for (size_t i = 0; i < count; i++) {
        float fadeFactor;
        if (isFadeOut) {
            fadeFactor = 1.0f - ((i + 1) * fadeStep);
        } else {
            fadeFactor = (i + 1) * fadeStep;
        }
        for (uint32_t j = 0; j < channel; j++) {
            dest[i * channel + j] *= fadeFactor;
        }
    }
}

void NoneMixEngine::DoFadeinOut(bool isFadeOut, char *pBuffer, size_t bufferSize)
{
    CHECK_AND_RETURN_LOG(pBuffer != nullptr && bufferSize > 0 && uChannel_ > 0, "buffer is null.");
    size_t dataLength = bufferSize / (static_cast<uint32_t>(uFormat_) * uChannel_);
    if (uFormat_ == sizeof(int16_t)) {
        AUDIO_INFO_LOG("int16 fading frame length:%{public}zu", dataLength);
        DoFadeInOut(reinterpret_cast<int16_t *>(pBuffer), dataLength, isFadeOut, uChannel_);
    } else if (uFormat_ == sizeof(int32_t)) {
        AUDIO_INFO_LOG("int32 fading frame length:%{public}zu", dataLength);
        DoFadeInOut(reinterpret_cast<int32_t *>(pBuffer), dataLength, isFadeOut, uChannel_);
    }
    if (isFadeOut) {
        startFadeout_.store(false);
    } else {
        startFadein_.store(false);
    }
}

void NoneMixEngine::AdjustVoipVolume()
{
    if (isVoip_) {
        uint32_t streamIndx = stream_->GetStreamIndex();
        AudioProcessConfig config = stream_->GetAudioProcessConfig();
        struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        float volumeEd = AudioVolume::GetInstance()->GetVolume(streamIndx, config.streamType,
            std::string(SINK_ADAPTER_NAME), &volumes);
        float volumeBg = volumes.volumeHistory;
        if ((!firstSetVolume_ && volumeBg != volumeEd) || firstSetVolume_) {
            AUDIO_INFO_LOG("Adjust voip volume");
            AudioVolume::GetInstance()->SetHistoryVolume(streamIndx, volumeEd);
            AudioVolume::GetInstance()->Monitor(streamIndx, true);
            std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
            CHECK_AND_RETURN(sink != nullptr);
            sink->SetVolume(volumeEd, volumeEd);
            firstSetVolume_ = false;
        }
    }
}

void NoneMixEngine::DoRenderFrame(std::vector<char> &audioBufferConverted, int32_t index, int32_t appUid)
{
    uint64_t written = 0;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    CHECK_AND_RETURN(sink != nullptr);
    sink->RenderFrame(*audioBufferConverted.data(), audioBufferConverted.size(), written);
    stream_->ReturnIndex(index);
    sink->UpdateAppsUid({appUid});
}

void NoneMixEngine::MixStreams()
{
    if (stream_ == nullptr) {
        StandbySleep();
        return;
    }
    if (failedCount_ >= MAX_ERROR_COUNT) {
        AUDIO_WARNING_LOG("failed count is overflow.");
        PauseAsync();
        return;
    }
    std::vector<char> audioBuffer;
    int32_t appUid = stream_->GetAudioProcessConfig().appInfo.appUid;
    int32_t index = -1;
    int32_t result = stream_->Peek(&audioBuffer, index);

    uint32_t sessionId = stream_->GetStreamIndex();
    writeCount_++;
    if (index < 0) {
        AUDIO_WARNING_LOG("peek buffer failed.result:%{public}d,buffer size:%{public}d", result, index);
        AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_DIRECT_OUT);
        stream_->ReturnIndex(index);
        failedCount_++;
        if (startFadeout_) {
            startFadeout_.store(false);
            cvFading_.notify_all();
            return;
        }
        ClockTime::RelativeSleep(PERIOD_NS);
        return;
    }
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_DIRECT_OUT);
    AdjustVoipVolume();
    failedCount_ = 0;
    // fade in or fade out
    if (startFadeout_ || startFadein_) {
        if (startFadeout_) {
            stream_->BlockStream();
        }
        DoFadeinOut(startFadeout_, audioBuffer.data(), audioBuffer.size());
        cvFading_.notify_all();
    }
    DoRenderFrame(audioBuffer, index, appUid);
    StandbySleep();
}

int32_t NoneMixEngine::AddRenderer(const std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_INFO_LOG("Enter add");
    if (!stream_) {
        AudioProcessConfig config = stream->GetAudioProcessConfig();
        int32_t result = InitSink(config.streamInfo);
        if (result == SUCCESS) {
            stream_ = stream;
            isInit_ = true;
        }
        return result;
    } else if (stream->GetStreamIndex() != stream_->GetStreamIndex()) {
        return ERROR_UNSUPPORTED;
    }
    return SUCCESS;
}

void NoneMixEngine::RemoveRenderer(const std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_INFO_LOG("step in remove");
    if (stream_ == nullptr) {
        AUDIO_INFO_LOG("stream already removed.");
        return;
    }
    if (stream->GetStreamIndex() == stream_->GetStreamIndex()) {
        Stop();
        stream_ = nullptr;
    }
}

bool NoneMixEngine::IsPlaybackEngineRunning() const noexcept
{
    return isStart_;
}

void NoneMixEngine::StandbySleep()
{
    int64_t writeTime = static_cast<int64_t>(fwkSyncTime_) + static_cast<int64_t>(writeCount_) * PERIOD_NS + DELTA_TIME;
    ClockTime::AbsoluteSleep(writeTime);
}

AudioSamplingRate NoneMixEngine::GetDirectSampleRate(AudioSamplingRate sampleRate)
{
    AudioSamplingRate result = sampleRate;
    switch (sampleRate) {
        case AudioSamplingRate::SAMPLE_RATE_44100:
            result = AudioSamplingRate::SAMPLE_RATE_48000;
            break;
        case AudioSamplingRate::SAMPLE_RATE_88200:
            result = AudioSamplingRate::SAMPLE_RATE_96000;
            break;
        case AudioSamplingRate::SAMPLE_RATE_176400:
            result = AudioSamplingRate::SAMPLE_RATE_192000;
            break;
        default:
            break;
    }
    AUDIO_INFO_LOG("GetDirectSampleRate: sampleRate: %{public}d, result: %{public}d", sampleRate, result);
    return result;
}

AudioSamplingRate NoneMixEngine::GetDirectVoipSampleRate(AudioSamplingRate sampleRate)
{
    AudioSamplingRate result = sampleRate;
    if (sampleRate <= AudioSamplingRate::SAMPLE_RATE_16000) {
        result = AudioSamplingRate::SAMPLE_RATE_16000;
    } else {
        result = AudioSamplingRate::SAMPLE_RATE_48000;
    }
    AUDIO_INFO_LOG("GetDirectVoipSampleRate: sampleRate: %{public}d, result: %{public}d", sampleRate, result);
    return result;
}

AudioSampleFormat NoneMixEngine::GetDirectDeviceFormat(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
        case AudioSampleFormat::SAMPLE_S16LE:
            return AudioSampleFormat::SAMPLE_S16LE;
        case AudioSampleFormat::SAMPLE_S24LE:
        case AudioSampleFormat::SAMPLE_S32LE:
            return AudioSampleFormat::SAMPLE_S32LE;
        case AudioSampleFormat::SAMPLE_F32LE:
            return AudioSampleFormat::SAMPLE_F32LE;
        default:
            return AudioSampleFormat::SAMPLE_S16LE;
    }
}

// replaced by using xml configuration later
AudioSampleFormat NoneMixEngine::GetDirectVoipDeviceFormat(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
        case AudioSampleFormat::SAMPLE_S16LE:
        case AudioSampleFormat::SAMPLE_F32LE:
            return AudioSampleFormat::SAMPLE_S16LE;
        case AudioSampleFormat::SAMPLE_S24LE:
        case AudioSampleFormat::SAMPLE_S32LE:
            return AudioSampleFormat::SAMPLE_S32LE;
        default:
            return AudioSampleFormat::SAMPLE_S16LE;
    }
}

int32_t NoneMixEngine::GetDirectFormatByteSize(AudioSampleFormat format)
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

void NoneMixEngine::GetTargetSinkStreamInfo(const AudioStreamInfo &clientStreamInfo, uint32_t &targetSampleRate,
    uint32_t &targetChannel, AudioSampleFormat &targetFormat, bool &isVoip)
{
    targetChannel = clientStreamInfo.channels >= STEREO_CHANNEL_COUNT ? STEREO_CHANNEL_COUNT : 1;

    if (isVoip) {
        targetSampleRate = GetDirectVoipSampleRate(clientStreamInfo.samplingRate);
        targetFormat = GetDirectVoipDeviceFormat(clientStreamInfo.format);
    } else {
        targetSampleRate = GetDirectSampleRate(clientStreamInfo.samplingRate);
        targetFormat = GetDirectDeviceFormat(clientStreamInfo.format);
    }
}

int32_t NoneMixEngine::InitSink(const AudioStreamInfo &clientStreamInfo)
{
    uint32_t targetSampleRate;
    uint32_t targetChannel;
    AudioSampleFormat targetFormat;
    GetTargetSinkStreamInfo(clientStreamInfo, targetSampleRate, targetChannel, targetFormat, isVoip_);

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (isInit_ && sink) {
        if (uChannel_ != targetChannel || uFormat_ != targetFormat || targetSampleRate != uSampleRate_) {
            if (sink && sink->IsInited()) {
                sink->Stop();
                sink->DeInit();
            }
        } else {
            return SUCCESS;
        }
    }
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
    return InitSink(targetChannel, targetFormat, targetSampleRate);
}

int32_t NoneMixEngine::InitSink(uint32_t channel, AudioSampleFormat format, uint32_t rate)
{
    std::string sinkName = DIRECT_SINK_NAME;
    if (isVoip_) {
        sinkName = VOIP_SINK_NAME;
    }
    renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, sinkName, true);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("get render fail, sinkName: %{public}s", sinkName.c_str());
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERR_INVALID_HANDLE;
    }
    IAudioSinkAttr attr = {};
    bool isDefaultAdapterEnable = AudioService::GetInstance()->GetDefaultAdapterEnable();
    attr.adapterName = isDefaultAdapterEnable ? "dp" : SINK_ADAPTER_NAME;
    attr.sampleRate = rate;
    attr.channel = channel;
    attr.format = format;
    attr.channelLayout = channel >= STEREO_CHANNEL_COUNT ? HDI_STEREO_CHANNEL_LAYOUT : HDI_MONO_CHANNEL_LAYOUT;
    attr.deviceType = device_.deviceType_;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    AUDIO_INFO_LOG("sinkName:%{public}s,device:%{public}d,sample rate:%{public}d,format:%{public}d,channel:%{public}d",
        sinkName.c_str(), attr.deviceType, attr.sampleRate, attr.format, attr.channel);
    int32_t ret = sink->Init(attr);
    if (ret != SUCCESS) {
        return ret;
    }
    float volume = 1.0f;
    ret = sink->SetVolume(volume, volume);
    uChannel_ = attr.channel;
    uSampleRate_ = attr.sampleRate;
    uFormat_ = GetDirectFormatByteSize(attr.format);

    return ret;
}

int32_t NoneMixEngine::SwitchSink(const AudioStreamInfo &streamInfo, bool isVoip)
{
    Stop();
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_);
    if (sink != nullptr) {
        sink->DeInit();
    }
    isVoip_ = isVoip;
    return InitSink(streamInfo);
}

uint64_t NoneMixEngine::GetLatency() noexcept
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
} // namespace AudioStandard
} // namespace OHOS
