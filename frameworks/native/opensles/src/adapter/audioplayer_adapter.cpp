/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPlayerAdapter"
#endif

#include <common.h>

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
AudioPlayerAdapter::AudioPlayerAdapter() { }

AudioPlayerAdapter::~AudioPlayerAdapter() { }

AudioPlayerAdapter* AudioPlayerAdapter::GetInstance()
{
    static AudioPlayerAdapter audioPlayerAdapter_;
    return &audioPlayerAdapter_;
}

AudioRenderer* AudioPlayerAdapter::GetAudioRenderById(SLuint32 id)
{
    auto it = renderMap_.find(id);
    if (it == renderMap_.end()) {
        return nullptr;
    }
    return it->second;
}

void AudioPlayerAdapter::EraseAudioRenderById(SLuint32 id)
{
    AUDIO_INFO_LOG("AudioPlayerAdapter::EraseAudioRenderById: %{public}lu", id);
    AudioRenderer* pRender = GetAudioRenderById(id);
    renderMap_.erase(id);
    callbackMap_.erase(id);
    if (pRender) {
        delete pRender;
        pRender = nullptr;
    }
    return;
}

SLresult AudioPlayerAdapter::CreateAudioPlayerAdapter
    (SLuint32 id, SLDataSource *dataSource, SLDataSink *dataSink, AudioStreamType streamType)
{
    SLDataFormat_PCM *pcmFormat = (SLDataFormat_PCM *)dataSource->pFormat;
    AudioRendererParams rendererParams;
    ConvertPcmFormat(pcmFormat, &rendererParams);
    streamType = AudioStreamType::STREAM_MUSIC;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = rendererParams.sampleRate;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = rendererParams.sampleFormat;
    rendererOptions.streamInfo.channels = rendererParams.channelCount;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RENDERER_NEW;
    string cachePath = "/data/storage/el2/base/cache";
    unique_ptr<AudioRenderer> rendererHolder = AudioRenderer::Create(cachePath.c_str(), rendererOptions);
    if (!rendererHolder) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::CreateAudioPlayer fail, ID: %{public}lu", id);
        return SL_RESULT_RESOURCE_ERROR;
    }
    // use release to get raw pointer, so need to delete explicitly when destory object
    AudioRenderer *renderer = rendererHolder.release();
    AUDIO_INFO_LOG("AudioPlayerAdapter::CreateAudioPlayer ID: %{public}lu", id);
    renderer->SetRenderMode(RENDER_MODE_CALLBACK);
    renderer->SetOffloadAllowed(false);
    renderMap_.insert(make_pair(id, renderer));
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::SetPlayStateAdapter(SLuint32 id, SLuint32 state)
{
    AudioRenderer* pRender = GetAudioRenderById(id);
    if (pRender == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::SetPlayStateAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    SLresult slResult = SL_RESULT_SUCCESS;
    bool result = false;
    bool rtStop = false;
    bool rtRelease = false;
    int32_t rtClear = -1;
    switch (state) {
        case SL_PLAYSTATE_PLAYING:
            result = pRender->Start();
            break;
        case SL_PLAYSTATE_PAUSED:
            result = pRender->Pause();
            break;
        case SL_PLAYSTATE_STOPPED: {
            rtStop = pRender->Stop();
            rtClear = pRender->Clear();
            rtRelease = pRender->Release();
            result = rtStop && !rtClear && rtRelease;
            break;
        }
        default:
            AUDIO_ERR_LOG("AudioPlayerAdapter::play state not supported ");
            break;
    }
    slResult = result ? SL_RESULT_SUCCESS : SL_RESULT_RESOURCE_ERROR;
    return slResult;
}

SLresult AudioPlayerAdapter::GetPlayStateAdapter(SLuint32 id, SLuint32 *state)
{
    AudioRenderer* pRender = GetAudioRenderById(id);
    if (pRender == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::GetPlayStateAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    RendererState rendererState = pRender->GetStatus();
    switch (rendererState) {
        case RENDERER_RUNNING:
            *state = SL_PLAYSTATE_PLAYING;
            break;
        case RENDERER_PAUSED:
            *state = SL_PLAYSTATE_PAUSED;
            break;
        case RENDERER_STOPPED:
            *state = SL_PLAYSTATE_STOPPED;
            break;
        default:
            *state = -1;
            break;
    }
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::SetVolumeLevelAdapter(SLuint32 id, SLmillibel level)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::SetVolumeLevelAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    int base = 10;
    float volume = pow(base, level / MAGNIFICATION);
    float volumeMaxLevel = 15;
    audioRenderer->SetVolume(volume / volumeMaxLevel);
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::GetVolumeLevelAdapter(SLuint32 id, SLmillibel *level)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::GetVolumeLevelAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    float volume = audioRenderer->GetVolume();
    *level = (SLmillibel) (MAGNIFICATION * log10(volume));
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::GetMaxVolumeLevelAdapter(SLuint32 id, SLmillibel *level)
{
    float volume = AudioSystemManager::GetInstance()->GetMaxVolume(STREAM_MUSIC);
    *level = (SLmillibel) (MAGNIFICATION * log10(volume));
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::EnqueueAdapter(SLuint32 id, const void *buffer, SLuint32 size)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::EnqueueAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    BufferDesc bufDesc = {};
    bufDesc.buffer = (uint8_t*) buffer;
    bufDesc.bufLength = size;
    bufDesc.dataLength = size;
    audioRenderer->Enqueue(bufDesc);
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::ClearAdapter(SLuint32 id)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::ClearAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    audioRenderer->Clear();
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::GetStateAdapter(SLuint32 id, SLOHBufferQueueState *state)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::GetStateAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    BufferQueueState queueState = {0, 0};
    audioRenderer->GetBufQueueState(queueState);
    state->count = queueState.numBuffers;
    state->index = queueState.currentIndex;
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::GetBufferAdapter(SLuint32 id, SLuint8 **buffer, SLuint32 *size)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::GetBufferAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    BufferDesc bufferDesc = {};
    audioRenderer->GetBufferDesc(bufferDesc);
    *buffer = bufferDesc.buffer;
    *size = bufferDesc.bufLength;
    return SL_RESULT_SUCCESS;
}

SLresult AudioPlayerAdapter::RegisterCallbackAdapter
    (SLOHBufferQueueItf itf, SlOHBufferQueueCallback callback, void *pContext)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *)itf;
    AudioRenderer *audioRenderer = GetAudioRenderById(thiz->mId);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("AudioPlayerAdapter::RegisterCallbackAdapter invalid id.");
        return SL_RESULT_RESOURCE_ERROR;
    }

    callbackPtr_ = make_shared<ReadOrWriteCallbackAdapter>(callback, itf, pContext);
    audioRenderer->SetRendererWriteCallback(static_pointer_cast<AudioRendererWriteCallback>(callbackPtr_));
    callbackMap_.insert(make_pair(thiz->mId, callbackPtr_));
    return SL_RESULT_SUCCESS;
}

void AudioPlayerAdapter::ConvertPcmFormat(SLDataFormat_PCM *slFormat, AudioRendererParams *rendererParams)
{
    AudioSampleFormat sampleFormat = SlToOhosSampelFormat(slFormat);
    AudioSamplingRate sampleRate = SlToOhosSamplingRate(slFormat);
    AudioChannel channelCount = SlToOhosChannel(slFormat);
    rendererParams->sampleFormat = sampleFormat;
    rendererParams->sampleRate = sampleRate;
    rendererParams->channelCount = channelCount;
    rendererParams->encodingType = ENCODING_PCM;
}

AudioSampleFormat AudioPlayerAdapter::SlToOhosSampelFormat(SLDataFormat_PCM *pcmFormat)
{
    AudioSampleFormat sampleFormat;
    switch (pcmFormat->bitsPerSample) {
        case SL_PCMSAMPLEFORMAT_FIXED_8:
            sampleFormat = SAMPLE_U8;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_16:
            sampleFormat = SAMPLE_S16LE;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_20:
            sampleFormat = INVALID_WIDTH;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_24:
            sampleFormat = SAMPLE_S24LE;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_28:
            sampleFormat = INVALID_WIDTH;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_32:
            sampleFormat = SAMPLE_S32LE;
            break;
        default:
            sampleFormat = INVALID_WIDTH;
    }
    return sampleFormat;
}

AudioSamplingRate AudioPlayerAdapter::SlToOhosSamplingRate(SLDataFormat_PCM *pcmFormat)
{
    AudioSamplingRate sampleRate;
    switch (pcmFormat->samplesPerSec) {
        case SL_SAMPLINGRATE_8:
            sampleRate = SAMPLE_RATE_8000;
            break;
        case SL_SAMPLINGRATE_11_025:
            sampleRate = SAMPLE_RATE_11025;
            break;
        case SL_SAMPLINGRATE_12:
            sampleRate = SAMPLE_RATE_12000;
            break;
        case SL_SAMPLINGRATE_16:
            sampleRate = SAMPLE_RATE_16000;
            break;
        case SL_SAMPLINGRATE_22_05:
            sampleRate = SAMPLE_RATE_22050;
            break;
        case SL_SAMPLINGRATE_24:
            sampleRate = SAMPLE_RATE_24000;
            break;
        case SL_SAMPLINGRATE_32:
            sampleRate = SAMPLE_RATE_32000;
            break;
        case SL_SAMPLINGRATE_44_1:
            sampleRate = SAMPLE_RATE_44100;
            break;
        case SL_SAMPLINGRATE_48:
            sampleRate = SAMPLE_RATE_48000;
            break;
        case SL_SAMPLINGRATE_64:
            sampleRate = SAMPLE_RATE_64000;
            break;
        case SL_SAMPLINGRATE_88_2:
            sampleRate = SAMPLE_RATE_44100;
            break;
        case SL_SAMPLINGRATE_96:
            sampleRate = SAMPLE_RATE_96000;
            break;
        case SL_SAMPLINGRATE_192:
            sampleRate = SAMPLE_RATE_44100;
            break;
        default:
            sampleRate = SAMPLE_RATE_44100;
    }
    return sampleRate;
}

AudioChannel AudioPlayerAdapter::SlToOhosChannel(SLDataFormat_PCM *pcmFormat)
{
    AudioChannel channelCount;
    switch (pcmFormat->numChannels) {
        case MONO:
            channelCount = MONO;
            break;
        case STEREO:
            channelCount = STEREO;
            break;
        default:
            channelCount = MONO;
            AUDIO_ERR_LOG("AudioPlayerAdapter::channel count not supported ");
    }
    return channelCount;
}
}  // namespace AudioStandard
}  // namespace OHOS
