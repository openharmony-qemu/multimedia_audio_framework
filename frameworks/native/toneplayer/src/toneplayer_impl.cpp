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
#define LOG_TAG "TonePlayerImpl"
#endif

#include <sys/time.h>
#include <utility>

#include <climits>
#include <cmath>
#include <cfloat>
#include "securec.h"
#include "audio_log.h"
#include "audio_policy_manager.h"
#include "tone_player_impl.h"
#include "audio_utils.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t C20MS = 20;
constexpr int32_t C1000MS = 1000;
constexpr int32_t CDOUBLE = 2;
constexpr int32_t DIGITAMPLITUDE = 800;
constexpr int32_t AMPLITUDE = 8000;
constexpr int32_t BIT8 = 8;

static const std::vector<ToneType> TONE_TYPE_LIST = {
    TONE_TYPE_DIAL_0,
    TONE_TYPE_DIAL_1,
    TONE_TYPE_DIAL_2,
    TONE_TYPE_DIAL_3,
    TONE_TYPE_DIAL_4,
    TONE_TYPE_DIAL_5,
    TONE_TYPE_DIAL_6,
    TONE_TYPE_DIAL_7,
    TONE_TYPE_DIAL_8,
    TONE_TYPE_DIAL_9,
    TONE_TYPE_DIAL_S,
    TONE_TYPE_DIAL_P
};
}

TonePlayerImpl::TonePlayerImpl(const std::string cachePath, const AudioRendererInfo &rendereInfo)
{
    toneState_ = TONE_IDLE;
    rendererOptions_.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions_.streamInfo.samplingRate = SAMPLE_RATE_48000;
    rendererOptions_.streamInfo.format = SAMPLE_S16LE;
    rendererOptions_.streamInfo.channels = MONO;

    // contentType::CONTENT_TYPE_MUSIC;
    rendererOptions_.rendererInfo.contentType = rendereInfo.contentType;

    // streamUsage::STREAM_USAGE_MEDIA;
    rendererOptions_.rendererInfo.streamUsage = rendereInfo.streamUsage;
    rendererOptions_.rendererInfo.rendererFlags = AUDIO_FLAG_FORCED_NORMAL; // use AUDIO_FLAG_FORCED_NORMAL
    supportedTones_ = AudioPolicyManager::GetInstance().GetSupportedTones();
    toneInfo_ = NULL;
    initialToneInfo_ = NULL;
    samplingRate_ = rendererOptions_.streamInfo.samplingRate;
    if (!cachePath.empty()) {
        AUDIO_INFO_LOG("copy application cache path");
        cachePath_.assign(cachePath);
    }
}

TonePlayerImpl::~TonePlayerImpl()
{
    AUDIO_INFO_LOG("TonePlayerImpl destructor");
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Stop();
        audioRenderer_->Release();
    }
    audioRenderer_ = nullptr;
}

std::shared_ptr<TonePlayer> TonePlayer::Create(const AudioRendererInfo &rendererInfo)
{
    if (!PermissionUtil::VerifySelfPermission()) {
        AUDIO_ERR_LOG("Create: No system permission");
        return nullptr;
    }
    return std::make_shared<TonePlayerImpl>("", rendererInfo);
}

std::shared_ptr<TonePlayer> TonePlayer::Create(const std::string cachePath, const AudioRendererInfo &rendererInfo)
{
    bool checkPermission = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(checkPermission, nullptr, "Create: No system permission");
    return std::make_shared<TonePlayerImpl>(cachePath, rendererInfo);
}

void TonePlayerImpl::OnInterrupt(const InterruptEvent &interruptEvent)
{
    AUDIO_INFO_LOG("ToneType %{public}d eventType: %{public}d", toneType_, interruptEvent.eventType);
    return;
}

void TonePlayerImpl::OnStateChange(const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType)
{
    AUDIO_INFO_LOG("ToneType %{public}d  OnStateChange state: %{public}d", toneType_, state);
}

void TonePlayerImpl::OnWriteData(size_t length)
{
    if (toneState_ == TONE_RELEASED) {
        AUDIO_WARNING_LOG("Tone %{public}d is already released", toneType_);
        return;
    }
    std::lock_guard<std::mutex> lock(optMutex_);
    BufferDesc bufDesc = {};
    if (audioRenderer_ != nullptr) {
        audioRenderer_->GetBufferDesc(bufDesc);
    } else {
        AUDIO_ERR_LOG("OnWriteData audioRenderer_ is null");
    }
    bufDesc.dataLength = 0;
    if (bufDesc.bufLength == 0) {
        AUDIO_WARNING_LOG(" bufDesc bufLength is 0");
        return;
    }

    // Clear output buffer: WaveGenerator accumulates into audioBuffer buffer
    memset_s(bufDesc.buffer, bufDesc.bufLength, 0, bufDesc.bufLength);
    if (AudioToneSequenceGen(bufDesc) == false) {
        AUDIO_WARNING_LOG("SequenceGen error");
        bufDesc.dataLength = bufDesc.bufLength;
    }
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(bufDesc.buffer), bufDesc.dataLength);
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Enqueue(bufDesc);
    } else {
        AUDIO_ERR_LOG("AudioToneDataThreadFunc Enqueue audioRenderer_ is null");
    }
}

bool TonePlayerImpl::LoadTone(ToneType toneType)
{
    std::lock_guard<std::mutex> lock(optMutex_);
    AUDIO_INFO_LOG("LoadTone type: %{public}d", toneType);
    bool result = false;
    bool checkPermission = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(checkPermission, false, "LoadTone: No system permission");
    if (toneType >= NUM_TONES) {
        return result;
    }
    if (std::find(supportedTones_.begin(), supportedTones_.end(), (int32_t)toneType) == supportedTones_.end()) {
        return result;
    }
    toneType_ = toneType;
    amplitudeType_ = std::count(TONE_TYPE_LIST.begin(), TONE_TYPE_LIST.end(), toneType_) > 0 ?
        DIGITAMPLITUDE : AMPLITUDE;
    initialToneInfo_ = AudioPolicyManager::GetInstance().GetToneConfig(toneType);
    if (initialToneInfo_->segmentCnt == 0) {
        AUDIO_ERR_LOG("LoadTone failed, calling GetToneConfig returned invalid");
        return result;
    }
    if (!isRendererInited_) {
        isRendererInited_ = InitAudioRenderer();
        CHECK_AND_RETURN_RET_LOG(isRendererInited_, false, "InitAudioRenderer failed");
    }
    result = InitToneWaveInfo();
    CHECK_AND_RETURN_RET_LOG(result, false, "InitToneWaveInfo failed");
    toneState_ = TONE_INIT;
    DumpFileUtil::OpenDumpFile(DUMP_CLIENT_PARA, DUMP_TONEPLAYER_FILENAME, &dumpFile_);
    return result;
}

bool TonePlayerImpl::StartTone()
{
    std::lock_guard<std::mutex> lock(optMutex_);
    AUDIO_INFO_LOG("STARTTONE ToneType %{public}d", toneType_);
    CHECK_AND_RETURN_RET_LOG(toneState_ == TONE_INIT || toneState_ == TONE_STOPPED, false,
        "Start audioRenderer_ is null");

    if (!isRendererInited_ || audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Start audioRenderer_ is null");
        return false;
    }
    bool result = audioRenderer_->Start();
    CHECK_AND_RETURN_RET_LOG(result, result, "Start audioRenderer_ failed");
    toneState_ = TONE_STARTING;
    return result;
}

bool TonePlayerImpl::StopTone()
{
    std::lock_guard<std::mutex> lock(optMutex_);
    AUDIO_INFO_LOG("STOPTONE ToneType %{public}d", toneType_);

    if (!isRendererInited_ || audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Stop audioRenderer_ is null");
        return false;
    }
    // in plan: mark state stopping, smooth volume in onwritedata
    bool result = audioRenderer_->Stop();
    CHECK_AND_RETURN_RET_LOG(result, result, "Stop audioRenderer_ failed");
    toneState_ = TONE_STOPPING;
    return result;
}

bool TonePlayerImpl::Release()
{
    std::unique_lock<std::mutex> lock(optMutex_);
    toneState_ = TONE_RELEASED;
    lock.unlock();
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Stop();
        audioRenderer_->Release();
        audioRenderer_ = nullptr;
    }
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    return true;
}

void TonePlayerImpl::GetCurrentSegmentUpdated()
{
    if (toneInfo_->segments[currSegment_].loopCnt) {
        if (loopCounter_ < toneInfo_->segments[currSegment_].loopCnt) {
            currSegment_ = toneInfo_->segments[currSegment_].loopIndx;
            ++loopCounter_;
        } else {
            // completed loop. go to next segment
            loopCounter_ = 0;
            currSegment_++;
        }
    } else {
        // no looping required , go to next segment
        currSegment_++;
    }
    AUDIO_INFO_LOG("GetCurrentSegmentUpdated loopCounter_: %{public}d, currSegment_: %{public}d",
        loopCounter_, currSegment_);
}

bool TonePlayerImpl::CheckToneContinuity()
{
    AUDIO_INFO_LOG("CheckToneContinuity Entry loopCounter_: %{public}d, currSegment_: %{public}d",
        loopCounter_, currSegment_);
    bool retVal = false;
    GetCurrentSegmentUpdated();

    // Handle loop if last segment reached
    if (toneInfo_->segments[currSegment_].duration == 0) {
        AUDIO_DEBUG_LOG("Last Seg: %{public}d", currSegment_);
        if (currCount_ < toneInfo_->repeatCnt) {
            currSegment_ = toneInfo_->repeatSegment;
            ++currCount_;
            retVal = true;
        } else {
            retVal = false;
        }
    } else {
        retVal = true;
    }
    AUDIO_DEBUG_LOG("CheckToneContinuity End loopCounter_: %{public}d, currSegment_: %{public}d currCount_: %{public}d",
        loopCounter_, currSegment_, currCount_);
    return retVal;
}

bool TonePlayerImpl::ContinueToneplay(uint32_t reqSample, int8_t *audioBuffer)
{
    if (toneState_ != TONE_RUNNING) {
        return false;
    }
    if (totalSample_ <= nextSegSample_) {
        if (toneInfo_->segments[currSegment_].duration != 0) {
            GetSamples(toneInfo_->segments[currSegment_].waveFreq, audioBuffer, reqSample);
        }
        return true;
    }

    if (CheckToneContinuity()) {
        if (toneInfo_->segments[currSegment_].duration != 0) {
            sampleCount_ = 0;
            GetSamples(toneInfo_->segments[currSegment_].waveFreq, audioBuffer, reqSample);
        }
    }
    nextSegSample_ += (toneInfo_->segments[currSegment_].duration * samplingRate_) / C1000MS;
    AUDIO_INFO_LOG("ContinueToneplay nextSegSample_: %{public}d", nextSegSample_);
    return true;
}

int32_t TonePlayerImpl::GetSamples(uint16_t *freqs, int8_t *buffer, uint32_t reqSamples)
{
    uint32_t index;
    uint8_t *data;
    uint16_t freqVal;
    float pi = 3.1428f;
    for (uint32_t i = 0; i <= TONEINFO_MAX_WAVES; i++) {
        if (freqs[i] == 0) {
            break;
        }
        freqVal = freqs[i];
        AUDIO_DEBUG_LOG("GetSamples Freq: %{public}d sampleCount_: %{public}d", freqVal, sampleCount_);
        index = sampleCount_;
        data = reinterpret_cast<uint8_t*>(buffer);
        double factor = freqVal * 2 * pi / samplingRate_; // 2 is a parameter in the sine wave formula
        for (uint32_t idx = 0; idx < reqSamples; idx++) {
            int16_t sample = amplitudeType_ * sin(factor * index);
            uint32_t result;
            if (i == 0) {
                result = (sample & 0xFF);
                *data = result & 0xFF;
                data++;
                *data = ((sample & 0xFF00) >> BIT8);
                data++;
            } else {
                result = *data + (static_cast<uint16_t>(sample) & 0xFF);
                *data = result & 0xFF;
                data++;
                *data += (result >> BIT8) + ((sample & 0xFF00) >> BIT8);
                data++;
            }
            index++;
        }
    }
    sampleCount_ += reqSamples;
    return 0;
}

bool TonePlayerImpl::CheckToneStarted(uint32_t reqSample, int8_t *audioBuffer)
{
    if (toneState_ != TONE_STARTING) {
        return false;
    }
    toneState_ = TONE_RUNNING;
    if (toneInfo_->segments[currSegment_].duration != 0) {
        sampleCount_ = 0;
        GetSamples(toneInfo_->segments[currSegment_].waveFreq, audioBuffer, reqSample);
    }
    return true;
}

bool TonePlayerImpl::CheckToneStopped()
{
    if (toneState_ == TONE_STOPPED) {
        return true;
    }
    if (toneInfo_->segments[currSegment_].duration == 0 || totalSample_ > maxSample_ || toneState_ == TONE_STOPPING) {
        if (toneState_ == TONE_RUNNING) {
            toneState_ = TONE_STOPPING;
            AUDIO_DEBUG_LOG("Audicallback move playing to stoping");
        }
        return true;
    }
    return false;
}

bool TonePlayerImpl::AudioToneSequenceGen(BufferDesc &bufDesc)
{
    int8_t *audioBuffer = reinterpret_cast<int8_t *>(bufDesc.buffer);
    uint32_t totalBufAvailable = bufDesc.bufLength / sizeof(int16_t);
    bool retVal = true;
    while (totalBufAvailable) {
        uint32_t reqSamples = totalBufAvailable < processSize_ * CDOUBLE ? totalBufAvailable : processSize_;
        AUDIO_DEBUG_LOG("AudioToneDataThreadFunc, lReqSmp: %{public}d totalBufAvailable: %{public}d",
            reqSamples, totalBufAvailable);
        // Update pcm frame count and end time (current time at the end of this process)
        totalSample_ += reqSamples;
        if (CheckToneStopped()) {
            // in plan: do smooth works
            AUDIO_INFO_LOG("CheckToneStopped true toneType_ %{public}d", toneType_);
            if (toneState_ == TONE_STOPPING) {
                toneState_ = TONE_STOPPED;
                totalBufAvailable = 0;
            }
            return false;
        } else if (CheckToneStarted(reqSamples, audioBuffer)) {
            bufDesc.dataLength += reqSamples * sizeof(int16_t);
        } else {
            if (ContinueToneplay(reqSamples, audioBuffer)) {
                bufDesc.dataLength += reqSamples * sizeof(int16_t);
            }
        }
        totalBufAvailable -= reqSamples;
        audioBuffer += reqSamples * sizeof(int16_t);
    }
    return retVal;
}

bool TonePlayerImpl::InitToneWaveInfo()
{
    AUDIO_INFO_LOG("InitToneWaveInfo ToneType %{public}d", toneType_);
    if (initialToneInfo_ == NULL) {
        return false;
    }
    toneInfo_ = initialToneInfo_;
    maxSample_ = TONEINFO_INF;

    // Initialize tone sequencer
    totalSample_ = 0;
    currSegment_ = 0;
    currCount_ = 0;
    loopCounter_ = 0;
    if (toneInfo_->segments[0].duration == TONEINFO_INF) {
        nextSegSample_ = TONEINFO_INF;
    } else {
        nextSegSample_ = (toneInfo_->segments[0].duration * samplingRate_) / C1000MS;
    }
    AUDIO_INFO_LOG("Prepare wave, nextSegSample_: %{public}d", nextSegSample_);
    return true;
}


bool TonePlayerImpl::InitAudioRenderer()
{
    processSize_ = (rendererOptions_.streamInfo.samplingRate * C20MS) / C1000MS;
    if (!cachePath_.empty()) {
        audioRenderer_ = AudioRenderer::Create(cachePath_, rendererOptions_);
    } else {
        audioRenderer_ = AudioRenderer::Create(rendererOptions_);
    }
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, false,
        "Renderer create failed");

    size_t targetSize = 0;
    int32_t ret = audioRenderer_->GetBufferSize(targetSize);

    AUDIO_DEBUG_LOG("Playback renderer created");
    int32_t setRenderMode = audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);
    CHECK_AND_RETURN_RET_LOG(!setRenderMode, false, "initAudioRenderer: SetRenderMode failed");
    AUDIO_DEBUG_LOG("SetRenderMode Sucessful");

    if (ret == 0 && targetSize != 0) {
        size_t bufferDuration = 20; // 20 -> 20ms
        audioRenderer_->SetBufferDuration(bufferDuration);
        AUDIO_INFO_LOG("Init renderer with buffer %{public}zu, duration %{public}zu", targetSize, bufferDuration);
    }

    audioRenderer_->SetAudioEffectMode(EFFECT_NONE);

    int32_t setRendererWrite = audioRenderer_->SetRendererWriteCallback(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(!setRendererWrite, false, "SetRendererWriteCallback failed");
    AUDIO_DEBUG_LOG("SetRendererWriteCallback Sucessful");

    int32_t setRendererCallback = audioRenderer_->SetRendererCallback(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(!setRendererCallback, false, "initAudioRenderer: SetRendererCallbackfailed");
    AUDIO_DEBUG_LOG("SetRendererCallback Sucessful");
    return true;
}
} // end namespace AudioStandard
} // end OHOS
