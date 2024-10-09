/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioBlendmodeTest"
#endif

#include <cstdio>
#include <iostream>

#include "audio_renderer_log.h"
#include "audio_system_manager.h"
#include "pcm2wav.h"
#include "audio_renderer.h"

using namespace OHOS::AudioStandard;
using namespace std;

namespace {
    constexpr uint8_t SLEEP_TIME = 2;
    constexpr int32_t SAMPLE_FORMAT_U8 = 8;
    constexpr int32_t SAMPLE_FORMAT_S16LE = 16;
    constexpr int32_t SAMPLE_FORMAT_S24LE = 24;
    constexpr int32_t SAMPLE_FORMAT_S32LE = 32;
} // namespace

static AudioSampleFormat GetSampleFormat(int32_t wavSampleFormat)
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

static unique_ptr<AudioRenderer> CreateAudioRenderer(wav_hdr &wavHeader)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = GetSampleFormat(wavHeader.bitsPerSample);
    rendererOptions.streamInfo.channels = (AudioChannel)wavHeader.NumOfChan;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;
    return AudioRenderer::Create(rendererOptions);
}

static int32_t StartRendererPlayback(char *inputPath, int mode)
{
    AUDIO_INFO_LOG("================PLAYBACK STARTED==================");
    wav_hdr wavHeader;
    size_t headerSize = sizeof(wav_hdr);
    char path[PATH_MAX + 1] = { 0x00 };
    if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
        AUDIO_ERR_LOG("Invalid path");
        return AUDIO_ERR;
    }
    AUDIO_INFO_LOG("path = %{public}s mode = %{public}d", path, mode);
    FILE* wavFile = fopen(path, "rb");
    if (wavFile == nullptr) {
        AUDIO_ERR_LOG("Unable to open wave file");
        return AUDIO_ERR;
    }

    (void)fread(&wavHeader, 1, headerSize, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = CreateAudioRenderer(wavHeader);
    audioRenderer->SetChannelBlendMode((ChannelBlendMode)mode);
    audioRenderer->Start();

    size_t bufferLen;
    audioRenderer->GetBufferSize(bufferLen);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);

    size_t minBytes = 4;

    while (!feof(wavFile)) {
        size_t bytesRead = fread(buffer, 1, bufferLen, wavFile);
        int32_t bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesRead) &&
            ((static_cast<size_t>(bytesRead) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                bytesRead - static_cast<size_t>(bytesWritten));
            if (bytesWritten < 0) {
                break;
            }
        }
    }

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
    sleep(SLEEP_TIME);
    AUDIO_INFO_LOG("================PLAYBACK OVER==================");
    return 0;
}

static void PrintUsage()
{
    cout << "[Audio BlendMode Test App]" << endl << endl;
    cout << "Supported Functionalities:" << endl;
    cout << "  a) Plays an input audio file with blend mode"<< endl;
    cout << "================================Usage=======================================" << endl << endl;
    cout << "-p\n\tPlays an input audio file with blend mode" << endl;
    cout << "\tUsage : ./audio_blendmode_test -p <audio file path> <blendmode>" << endl;
    cout << "\tExample : ./audio_blendmode_test -p /data/test.wav 1" << endl << endl;
}

int main(int argc, char* argv[])
{
    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "a:c:p:r:h")) != AUDIO_INVALID_PARAM) {
        switch (c) {
            case 'p':
                StartRendererPlayback(optarg, atoi(argv[optind]));
                break;
            case 'h':
                PrintUsage();
                break;
            default:
                cout << "Unsupported option. Exiting!!!" << endl;
                exit(0);
                break;
        }
    }

    return 0;
}
