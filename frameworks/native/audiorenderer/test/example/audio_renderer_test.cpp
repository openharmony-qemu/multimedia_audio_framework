/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRendererTest"
#endif

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <unistd.h>
#include "audio_renderer_log.h"
#include "audio_renderer.h"
#include "pcm2wav.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace {
    constexpr int32_t ARGS_INDEX_THREE = 3;
    constexpr int32_t ARGS_INDEX_TWO = 2;
    constexpr int32_t ARGS_COUNT_TWO = 2;
    constexpr int32_t ARGS_COUNT_THREE = 3;
    constexpr int32_t ARGS_COUNT_FOUR = 4;
    constexpr int32_t SUCCESS = 0;
#ifndef LATENCY_ACCURACY_TEST
    constexpr int32_t STOP_BUFFER_POSITION = 700000;
    constexpr int32_t PAUSE_BUFFER_POSITION = 1400000;
    constexpr int32_t PAUSE_RENDER_TIME_SECONDS = 1;
    constexpr int32_t STOP_RENDER_TIME_SECONDS = 1;
    constexpr float TRACK_VOLUME = 0.2f;
#endif // LATENCY_ACCURACY_TEST
    
    constexpr int32_t SAMPLE_FORMAT_U8 = 8;
    constexpr int32_t SAMPLE_FORMAT_S16LE = 16;
    constexpr int32_t SAMPLE_FORMAT_S24LE = 24;
    constexpr int32_t SAMPLE_FORMAT_S32LE = 32;
}

class AudioRendererCallbackTestImpl : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {}
    void OnStateChange(const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType) override
    {
        AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl:: OnStateChange");

        switch (state) {
            case RENDERER_PREPARED:
                AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_PREPARED");
                break;
            case RENDERER_RUNNING:
                AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_RUNNING");
                break;
            case RENDERER_STOPPED:
                AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_STOPPED");
                break;
            case RENDERER_PAUSED:
                AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_PAUSED");
                break;
            case RENDERER_RELEASED:
                AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_RELEASED");
                break;
            default:
                AUDIO_ERR_LOG("AudioRendererCallbackTestImpl: OnStateChange NOT A VALID state");
                break;
        }
    }
};

class AudioRendererTest {
public:
    void CheckSupportedParams() const
    {
        vector<AudioSampleFormat> supportedFormatList = AudioRenderer::GetSupportedFormats();
        AUDIO_INFO_LOG("AudioRendererTest: Supported formats:");
        for (auto i = supportedFormatList.begin(); i != supportedFormatList.end(); ++i) {
            AUDIO_INFO_LOG("AudioRendererTest: Format %{public}d", *i);
        }

        vector<AudioChannel> supportedChannelList = AudioRenderer::GetSupportedChannels();
        AUDIO_INFO_LOG("AudioRendererTest: Supported channels:");
        for (auto i = supportedChannelList.begin(); i != supportedChannelList.end(); ++i) {
            AUDIO_INFO_LOG("AudioRendererTest: channel %{public}d", *i);
        }

        vector<AudioEncodingType> supportedEncodingTypes
                                    = AudioRenderer::GetSupportedEncodingTypes();
        AUDIO_INFO_LOG("AudioRendererTest: Supported encoding types:");
        for (auto i = supportedEncodingTypes.begin(); i != supportedEncodingTypes.end(); ++i) {
            AUDIO_INFO_LOG("AudioRendererTest: encoding type %{public}d", *i);
        }

        vector<AudioSamplingRate> supportedSamplingRates = AudioRenderer::GetSupportedSamplingRates();
        AUDIO_INFO_LOG("AudioRendererTest: Supported sampling rates:");
        for (auto i = supportedSamplingRates.begin(); i != supportedSamplingRates.end(); ++i) {
            AUDIO_INFO_LOG("AudioRendererTest: sampling rate %{public}d", *i);
        }
    }

    void GetRendererStreamInfo(const unique_ptr<AudioRenderer> &audioRenderer) const
    {
        AUDIO_INFO_LOG("AudioRendererTest: GetRendererInfo:");
        AudioRendererInfo rendererInfo;
        if (audioRenderer->GetRendererInfo(rendererInfo) ==  SUCCESS) {
            AUDIO_INFO_LOG("AudioRendererTest: Get ContentType: %{public}d", rendererInfo.contentType);
            AUDIO_INFO_LOG("AudioRendererTest: Get StreamUsage: %{public}d", rendererInfo.streamUsage);
        } else {
            AUDIO_ERR_LOG("AudioRendererTest: GetStreamInfo failed");
        }

        AUDIO_INFO_LOG("AudioRendererTest: GetStreamInfo:");
        AudioStreamInfo streamInfo;
        if (audioRenderer->GetStreamInfo(streamInfo) ==  SUCCESS) {
            AUDIO_INFO_LOG("AudioRendererTest: Get AudioSamplingRate: %{public}d", streamInfo.samplingRate);
            AUDIO_INFO_LOG("AudioRendererTest: Get AudioEncodingType: %{public}d", streamInfo.encoding);
            AUDIO_INFO_LOG("AudioRendererTest: Get AudioSampleFormat: %{public}d", streamInfo.format);
            AUDIO_INFO_LOG("AudioRendererTest: Get AudioChannel: %{public}d", streamInfo.channels);
        } else {
            AUDIO_ERR_LOG("AudioRendererTest: GetStreamInfo failed");
        }
    }

    bool InitRender(const unique_ptr<AudioRenderer> &audioRenderer) const
    {
        AUDIO_INFO_LOG("AudioRendererTest: Starting renderer");
        if (!audioRenderer->Start()) {
            AUDIO_ERR_LOG("AudioRendererTest: Start failed");
            if (!audioRenderer->Release()) {
                AUDIO_ERR_LOG("AudioRendererTest: Release failed");
            }
            return false;
        }
        AUDIO_INFO_LOG("AudioRendererTest: Playback started");
#ifndef LATENCY_ACCURACY_TEST
        if (audioRenderer->SetVolume(TRACK_VOLUME) == SUCCESS) {
            AUDIO_INFO_LOG("AudioRendererTest: volume set to: %{public}f", audioRenderer->GetVolume());
        }
#endif // LATENCY_ACCURACY_TEST

        return true;
    }

#ifndef LATENCY_ACCURACY_TEST
    bool TestPauseStop(const unique_ptr<AudioRenderer> &audioRenderer, bool &pauseTested, bool &stopTested,
                       FILE &wavFile) const
    {
        int64_t currFilePos = ftell(&wavFile);
        if (!stopTested && (currFilePos > STOP_BUFFER_POSITION) && audioRenderer->Stop()) {
            stopTested = true;
            sleep(STOP_RENDER_TIME_SECONDS);
            AUDIO_INFO_LOG("Audio render resume");
            if (!audioRenderer->Start()) {
                AUDIO_ERR_LOG("resume stream failed");
                return false;
            }
        } else if (!pauseTested && (currFilePos > PAUSE_BUFFER_POSITION)
                   && audioRenderer->Pause()) {
            pauseTested = true;
            sleep(PAUSE_RENDER_TIME_SECONDS);
            AUDIO_INFO_LOG("Audio render resume");
            if (audioRenderer->SetVolume(1.0) == SUCCESS) {
                AUDIO_INFO_LOG("AudioRendererTest: after resume volume set to: %{public}f",
                               audioRenderer->GetVolume());
            }
            if (!audioRenderer->Flush()) {
                AUDIO_ERR_LOG("AudioRendererTest: flush failed");
                return false;
            }
            if (!audioRenderer->Start()) {
                AUDIO_ERR_LOG("resume stream failed");
                return false;
            }
        }

        return true;
    }
#endif // LATENCY_ACCURACY_TEST

    bool GetBufferLen(const unique_ptr<AudioRenderer> &audioRenderer, size_t &bufferLen) const
    {
        if (audioRenderer->GetBufferSize(bufferLen)) {
            return false;
        }
        AUDIO_DEBUG_LOG("minimum buffer length: %{public}zu", bufferLen);

        uint32_t frameCount;
        if (audioRenderer->GetFrameCount(frameCount)) {
            return false;
        }
        AUDIO_INFO_LOG("AudioRendererTest: Frame count: %{public}d", frameCount);
        return true;
    }

    bool StartRender(const unique_ptr<AudioRenderer> &audioRenderer, FILE* wavFile) const
    {
        size_t bufferLen = 0;
        if (!GetBufferLen(audioRenderer, bufferLen)) {
            return false;
        }

        int32_t n = 2;
        auto buffer = std::make_unique<uint8_t[]>(n * bufferLen);
        if (buffer == nullptr) {
            AUDIO_ERR_LOG("AudioRendererTest: Failed to allocate buffer");
            return false;
        }

        size_t bytesToWrite = 0;
        size_t bytesWritten = 0;
        size_t minBytes = 4;
        uint64_t latency;
#ifndef LATENCY_ACCURACY_TEST
        bool stopTested = false;
        bool pauseTested = false;
#endif // LATENCY_ACCURACY_TEST
#ifdef LATENCY_ACCURACY_TEST
        uint32_t writeCount {0};
#endif // LATENCY_ACCURACY_TEST

        while (!feof(wavFile)) {
            bytesToWrite = fread(buffer.get(), 1, bufferLen, wavFile);
            bytesWritten = 0;
            AUDIO_INFO_LOG("AudioRendererTest: Bytes to write: %{public}zu", bytesToWrite);

#ifndef LATENCY_ACCURACY_TEST
            if (!TestPauseStop(audioRenderer, pauseTested, stopTested, *wavFile)) {
                break;
            }
#endif // LATENCY_ACCURACY_TEST
#ifdef LATENCY_ACCURACY_TEST
            AUDIO_DEBUG_LOG("start: %{public}d", ++writeCount);
#endif // LATENCY_ACCURACY_TEST
            while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
                bytesWritten += audioRenderer->Write(buffer.get() + bytesWritten,
                                                     bytesToWrite - bytesWritten);
                AUDIO_INFO_LOG("AudioRendererTest: Bytes written: %{public}zu", bytesWritten);
                if (bytesWritten < 0) {
                    break;
                }
            }
#ifdef LATENCY_ACCURACY_TEST
            AUDIO_DEBUG_LOG("complete: %{public}d", writeCount);
#endif // LATENCY_ACCURACY_TEST

            if (audioRenderer->GetLatency(latency)) {
                AUDIO_ERR_LOG("AudioRendererTest: GetLatency failed");
                break;
#ifdef LATENCY_ACCURACY_TEST
            } else {
                AUDIO_DEBUG_LOG("GetLatency: %{public}" PRIu64, latency);
#endif // LATENCY_ACCURACY_TEST
            }
        }

        if (!audioRenderer->Drain()) {
            AUDIO_ERR_LOG("AudioRendererTest: Drain failed");
        }

        if (audioRenderer->GetLatency(latency)) {
            AUDIO_ERR_LOG("AudioRendererTest: GetLatency failed after Drain");
#ifdef LATENCY_ACCURACY_TEST
        } else {
            AUDIO_DEBUG_LOG("GetLatency after Drain: %{public}" PRIu64, latency);
#endif // LATENCY_ACCURACY_TEST
        }

        return true;
    }
    
    AudioSampleFormat GetSampleFormat(int32_t wavSampleFormat) const
    {
        switch (wavSampleFormat) {
            case SAMPLE_FORMAT_U8:
                return AudioSampleFormat::SAMPLE_U8;
            case SAMPLE_FORMAT_S16LE:
                return AudioSampleFormat::SAMPLE_S16LE;
            case SAMPLE_FORMAT_S24LE:
                return AudioSampleFormat::SAMPLE_S24LE;
            case SAMPLE_FORMAT_S32LE:
                return AudioSampleFormat::SAMPLE_S32LE;
            default:
                return AudioSampleFormat::INVALID_WIDTH;
        }
    }

    bool TestPlayback(int argc, char *argv[]) const
    {
        AUDIO_INFO_LOG("AudioRendererTest: TestPlayback start ");

        int numBase = 10;
        wav_hdr wavHeader;
        size_t headerSize = sizeof(wav_hdr);
        char *inputPath = argv[1];
        char path[PATH_MAX + 1] = {0x00};
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            AUDIO_ERR_LOG("Invalid path");
            return false;
        }
        AUDIO_INFO_LOG("AudioRendererTest: path = %{public}s", path);
        FILE* wavFile = fopen(path, "rb");
        if (wavFile == nullptr) {
            AUDIO_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return false;
        }
        size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
        AUDIO_INFO_LOG("AudioRendererTest: Header Read in bytes %{public}zu", bytesRead);

        ContentType contentType = ContentType::CONTENT_TYPE_MUSIC;
        StreamUsage streamUsage = StreamUsage::STREAM_USAGE_MEDIA;

        float speed = 1.0;
        if (argc == ARGS_COUNT_THREE) {
            speed = static_cast<float>(atof(argv[ARGS_COUNT_TWO]));
        } else if (argc > ARGS_COUNT_THREE) {
            contentType = static_cast<ContentType>(strtol(argv[ARGS_INDEX_TWO], NULL, numBase));
            streamUsage = static_cast<StreamUsage>(strtol(argv[ARGS_INDEX_THREE], NULL, numBase));
        }
        int32_t bufferMsec = 0;
        if (argc > ARGS_COUNT_FOUR) {
            bufferMsec = strtol(argv[ARGS_COUNT_FOUR], nullptr, numBase);
        }

        AudioRendererOptions rendererOptions = {};
        rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
        rendererOptions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(wavHeader.SamplesPerSec);
        rendererOptions.streamInfo.format = GetSampleFormat(wavHeader.bitsPerSample);
        rendererOptions.streamInfo.channels = static_cast<AudioChannel>(wavHeader.NumOfChan);
        rendererOptions.rendererInfo.contentType = contentType;
        rendererOptions.rendererInfo.streamUsage = streamUsage;
        rendererOptions.rendererInfo.rendererFlags = 0;

        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);

        if (audioRenderer == nullptr) {
            AUDIO_ERR_LOG("AudioRendererTest: Create failed");
            fclose(wavFile);
            return false;
        }

        int32_t ret = 0;
        shared_ptr<AudioRendererCallback> cb1 = make_shared<AudioRendererCallbackTestImpl>();
        ret = audioRenderer->SetRendererCallback(cb1);
        if (ret) {
            AUDIO_ERR_LOG("AudioRendererTest: SetRendererCallback failed %{public}d", ret);
            fclose(wavFile);
            return false;
        }

        GetRendererStreamInfo(audioRenderer);

        CheckSupportedParams();
        AUDIO_ERR_LOG("AudioRendererTest: buffermsec = %{public}d", bufferMsec);
        int32_t status = audioRenderer->SetBufferDuration(bufferMsec);
        if (status) {
            AUDIO_ERR_LOG("Failed to set buffer duration");
        }

        if (!InitRender(audioRenderer)) {
            AUDIO_ERR_LOG("AudioRendererTest: Init render failed");
            fclose(wavFile);
            return false;
        }
        audioRenderer->SetSpeed(speed);

        if (!StartRender(audioRenderer, wavFile)) {
            AUDIO_ERR_LOG("AudioRendererTest: Start render failed");
            fclose(wavFile);
            return false;
        }

        if (!audioRenderer->Stop()) {
            AUDIO_ERR_LOG("AudioRendererTest: Stop failed");
        }

        if (!audioRenderer->Release()) {
            AUDIO_ERR_LOG("AudioRendererTest: Release failed");
        }

        fclose(wavFile);
        AUDIO_INFO_LOG("AudioRendererTest: TestPlayback end");

        return true;
    }
};

int main(int argc, char *argv[])
{
    AUDIO_INFO_LOG("AudioRendererTest: Render test in");

    if (argv == nullptr) {
        AUDIO_ERR_LOG("AudioRendererTest: argv is null");
        return 0;
    }

    if (argc < ARGS_COUNT_TWO) {
        AUDIO_ERR_LOG("AudioRendererTest: incorrect argc. Enter either 2 or 3 or 4 args");
        return 0;
    }

    AUDIO_INFO_LOG("AudioRendererTest: argc=%{public}d", argc);
    AUDIO_INFO_LOG("file path argv[1]=%{public}s", argv[1]);

    AudioRendererTest testObj;
    bool ret = testObj.TestPlayback(argc, argv);

    return ret;
}
