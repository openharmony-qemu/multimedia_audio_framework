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
#include "none_mix_engine.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DELTA_TIME = 4000000; // 4ms
constexpr int32_t PERIOD_NS = 20000000; // 20ms
constexpr int32_t FADING_MS = 20; // 20ms
constexpr int32_t MAX_ERROR_COUNT = 50;
constexpr int16_t STEREO_CHANNEL_COUNT = 2;
constexpr int16_t HDI_STEREO_CHANNEL_LAYOUT = 3;
constexpr int16_t HDI_MONO_CHANNEL_LAYOUT = 4;
constexpr int32_t DIRECT_STOP_TIMEOUT_IN_SEC = 8; // 8S
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
      stream_(nullptr),
      startFadein_(false),
      startFadeout_(false),
      uChannel_(0),
      uFormat_(sizeof(int32_t)),
      uSampleRate_(0)
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
    if (renderSink_ && renderSink_->IsInited()) {
        renderSink_->Stop();
        renderSink_->DeInit();
    }
    isStart_ = false;
    startFadein_ = false;
    startFadeout_ = false;
}

int32_t NoneMixEngine::Init(const DeviceInfo &type, bool isVoip)
{
    if (!isInit_) {
        isVoip_ = isVoip;
        device_ = type;
        return SUCCESS;
    }
    if (type.deviceType != device_.deviceType || isVoip_ != isVoip) {
        isVoip_ = isVoip;
        device_ = type;
        if (renderSink_ && renderSink_->IsInited()) {
            renderSink_->Stop();
            renderSink_->DeInit();
        }
        renderSink_ = nullptr;
    }
    return SUCCESS;
}

int32_t NoneMixEngine::Start()
{
    AUDIO_INFO_LOG("Enter in");
    int32_t ret = SUCCESS;
    CHECK_AND_RETURN_RET_LOG(renderSink_ != nullptr, ERR_INVALID_HANDLE, "null sink");
    CHECK_AND_RETURN_RET_LOG(renderSink_->IsInited(), ERR_NOT_STARTED, "sink Not Inited! Init the sink first");
    fwkSyncTime_ = static_cast<uint64_t>(ClockTime::GetCurNano());
    writeCount_ = 0;
    failedCount_ = 0;
    if (!playbackThread_) {
        playbackThread_ = std::make_unique<AudioThreadTask>(THREAD_NAME);
        playbackThread_->RegisterJob([this] { this->MixStreams(); });
    }
    if (!isStart_) {
        startFadeout_ = false;
        startFadein_ = true;
        ret = renderSink_->Start();
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
    ret = SinkStopTimeOut();
    isStart_ = false;
    return ret;
}

void NoneMixEngine::PauseAsync()
{
    // stop thread when failed 5 times,do not add logic inside.
    if (playbackThread_ && playbackThread_->CheckThreadIsRunning()) {
        playbackThread_->PauseAsync();
    }
    int32_t ret = SinkStopTimeOut();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sink stop failed.ret:%{public}d", ret);
    }
    isStart_ = false;
}

int32_t NoneMixEngine::SinkStopTimeOut()
{
    int32_t ret = SUCCESS;
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file,flag 2 die when timeout, restart server
    AudioXCollie audioXCollie(
        "NoneMixEngine::Stop", DIRECT_STOP_TIMEOUT_IN_SEC,
        [this](void *) { AUDIO_ERR_LOG("%{public}d renderSink_ stop timeout, trigger signal", isVoip_); }, nullptr,
        XcollieFlag);
    if (renderSink_ && renderSink_->IsInited()) {
        ret = renderSink_->Stop();
    }
    return ret;
}

int32_t NoneMixEngine::Pause()
{
    AUDIO_INFO_LOG("Enter");
    int32_t ret = SUCCESS;
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
    ret = SinkStopTimeOut();
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
    writeCount_++;
    if (index < 0) {
        AUDIO_WARNING_LOG("peek buffer failed.result:%{public}d,buffer size:%{public}d", result, index);
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
    failedCount_ = 0;
    uint64_t written = 0;
    // fade in or fade out
    if (startFadeout_ || startFadein_) {
        DoFadeinOut(startFadeout_, audioBuffer.data(), audioBuffer.size());
        cvFading_.notify_all();
    }
    renderSink_->RenderFrame(*audioBuffer.data(), audioBuffer.size(), written);
    stream_->ReturnIndex(index);
    renderSink_->UpdateAppsUid({appUid});
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

HdiAdapterFormat NoneMixEngine::GetDirectDeviceFormate(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
        case AudioSampleFormat::SAMPLE_S16LE:
            return HdiAdapterFormat::SAMPLE_S16;
        case AudioSampleFormat::SAMPLE_S24LE:
        case AudioSampleFormat::SAMPLE_S32LE:
            return HdiAdapterFormat::SAMPLE_S32;
        case AudioSampleFormat::SAMPLE_F32LE:
            return HdiAdapterFormat::SAMPLE_F32;
        default:
            return HdiAdapterFormat::SAMPLE_S16;
    }
}

int32_t NoneMixEngine::GetDirectFormatByteSize(HdiAdapterFormat format)
{
    switch (format) {
        case HdiAdapterFormat::SAMPLE_S16:
            return sizeof(int16_t);
        case HdiAdapterFormat::SAMPLE_S32:
        case HdiAdapterFormat::SAMPLE_F32:
            return sizeof(int32_t);
        default:
            return sizeof(int32_t);
    }
}

int32_t NoneMixEngine::InitSink(const AudioStreamInfo &streamInfo)
{
    uint32_t targetChannel = streamInfo.channels >= STEREO_CHANNEL_COUNT ? STEREO_CHANNEL_COUNT : 1;
    HdiAdapterFormat format = GetDirectDeviceFormate(streamInfo.format);
    uint32_t sampleRate =
        isVoip_ ? GetDirectVoipSampleRate(streamInfo.samplingRate) : GetDirectSampleRate(streamInfo.samplingRate);
    if (isInit_ && renderSink_) {
        if (uChannel_ != targetChannel || uFormat_ != format || sampleRate != uSampleRate_) {
            if (renderSink_ && renderSink_->IsInited()) {
                renderSink_->Stop();
                renderSink_->DeInit();
            }
            renderSink_ = nullptr;
        } else {
            return SUCCESS;
        }
    }
    return InitSink(targetChannel, format, sampleRate);
}

int32_t NoneMixEngine::InitSink(uint32_t channel, HdiAdapterFormat format, uint32_t rate)
{
    std::string sinkName = DIRECT_SINK_NAME;
    if (isVoip_) {
        sinkName = VOIP_SINK_NAME;
    }
    renderSink_ = AudioRendererSink::GetInstance(sinkName);
    IAudioSinkAttr attr = {};
    attr.adapterName = SINK_ADAPTER_NAME;
    attr.sampleRate = rate;
    attr.channel = channel;
    attr.format = format;
    attr.channelLayout = channel >= STEREO_CHANNEL_COUNT ? HDI_STEREO_CHANNEL_LAYOUT : HDI_MONO_CHANNEL_LAYOUT;
    attr.deviceType = device_.deviceType;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    AUDIO_INFO_LOG("sinkName:%{public}s,device:%{public}d,sample rate:%{public}d,format:%{public}d,channel:%{public}d",
        sinkName.c_str(), attr.deviceType, attr.sampleRate, attr.format, attr.channel);
    int32_t ret = renderSink_->Init(attr);
    if (ret != SUCCESS) {
        return ret;
    }
    float volume = 1.0f;
    ret = renderSink_->SetVolume(volume, volume);
    uChannel_ = attr.channel;
    uSampleRate_ = attr.sampleRate;
    uFormat_ = GetDirectFormatByteSize(attr.format);

    return ret;
}

int32_t NoneMixEngine::SwitchSink(const AudioStreamInfo &streamInfo, bool isVoip)
{
    Stop();
    renderSink_->DeInit();
    isVoip_ = isVoip;
    return InitSink(streamInfo);
}
} // namespace AudioStandard
} // namespace OHOS
