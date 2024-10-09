/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioOpenslesPlayerTest"
#endif

#include <OpenSLES.h>
#include <OpenSLES_OpenHarmony.h>

#include <cstdio>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "audio_log.h"
#include "pcm2wav.h"

using namespace std;

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void PlayerStart(SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile);

static void PlayerStop(SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf);

static void OpenSlTest();

static void OpenSlTestConcurrent();

const SLuint32 number = 3;
FILE *wavFile_ = nullptr;
FILE *wavFile1_ = nullptr;
FILE *wavFile2_ = nullptr;
wav_hdr wavHeader_;
wav_hdr wavHeader1_;
wav_hdr wavHeader2_;
SLObjectItf engineObject = nullptr;
SLObjectItf outputMixObject = nullptr;
SLPlayItf playItf;
SLPlayItf playItf1;
SLPlayItf playItf2;
SLVolumeItf volumeItf1;
SLVolumeItf volumeItf2;
SLOHBufferQueueItf bufferQueueItf;
SLOHBufferQueueItf bufferQueueItf1;
SLOHBufferQueueItf bufferQueueItf2;
SLObjectItf pcmPlayerObject = nullptr;
SLObjectItf pcmPlayerObject1 = nullptr;
SLObjectItf pcmPlayerObject2 = nullptr;

int main(int argc, char *argv[])
{
    if (argc == 4) {
        size_t headerSize = sizeof(wav_hdr);
        char *inputPath = argv[1];
        char path[PATH_MAX + 1] = {0x00};
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            AUDIO_ERR_LOG("Invalid path");
            return -1;
        }
        wavFile1_ = fopen(path, "rb");
        if (wavFile1_ == nullptr) {
            AUDIO_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader1_, 1, headerSize, wavFile1_);

        headerSize = sizeof(wav_hdr);
        inputPath = argv[2];
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            AUDIO_ERR_LOG("Invalid path");
            return -1;
        }
        wavFile2_ = fopen(path, "rb");
        if (wavFile2_ == nullptr) {
            AUDIO_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader2_, 1, headerSize, wavFile2_);

        OpenSlTestConcurrent();

        while (!feof(wavFile1_) || !feof(wavFile2_)) {
            sleep(1);
        }

        PlayerStop(playItf1, bufferQueueItf1);
        PlayerStop(playItf2, bufferQueueItf2);
        (*pcmPlayerObject1)->Destroy(pcmPlayerObject1);
        (*pcmPlayerObject2)->Destroy(pcmPlayerObject2);
        (*engineObject)->Destroy(engineObject);
        (*outputMixObject)->Destroy(outputMixObject);
        return 0;
    } else {
        if (argc < 2) {
            return -1;
        }
        size_t headerSize = sizeof(wav_hdr);
        char *inputPath = argv[1];
        char path[PATH_MAX + 1] = {0x00};
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            AUDIO_ERR_LOG("Invalid path");
            return -1;
        }
        wavFile_ = fopen(path, "rb");
        if (wavFile_ == nullptr) {
            AUDIO_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader_, 1, headerSize, wavFile_);

        OpenSlTest();

        while (!feof(wavFile_)) {
            sleep(1);
        }
        PlayerStop(playItf, bufferQueueItf);
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);

        if (argc < 3) {
            return 0;
        }
        char *inputPath2 = argv[2];
        char path2[PATH_MAX + 1] = {0x00};
        if ((strlen(inputPath2) > PATH_MAX) || (realpath(inputPath2, path2) == nullptr)) {
            AUDIO_ERR_LOG("Invalid path");
            return -1;
        }
        wavFile_ = fopen(path2, "rb");
        if (wavFile_ == nullptr) {
            AUDIO_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader_, 1, headerSize, wavFile_);

        OpenSlTest();

        while (!feof(wavFile_)) {
            sleep(1);
        }
        PlayerStop(playItf, bufferQueueItf);
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        return 0;
    }
}

static void OpenSlTest()
{
    AUDIO_INFO_LOG("OpenSlTest");
    engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;
    slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    outputMixObject = nullptr;
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, nullptr, nullptr);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slSource, &slSink, number, nullptr, nullptr);
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &playItf);
    SLVolumeItf volumeItf;
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &volumeItf);
    SLmillibel pLevel = 0;
    (*volumeItf)->GetVolumeLevel(volumeItf, &pLevel);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf);
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BufferQueueCallback, wavFile_);

    PlayerStart(playItf, bufferQueueItf, wavFile_);
}

static void OpenSlTestConcurrent()
{
    AUDIO_INFO_LOG("OpenSlTestConcurrent");
    engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;

    slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    outputMixObject = nullptr;
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, nullptr, nullptr);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat1 = {
        SL_DATAFORMAT_PCM,
        wavHeader1_.NumOfChan,
        wavHeader1_.SamplesPerSec * 1000,
        wavHeader1_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataFormat_PCM pcmFormat2 = {
        SL_DATAFORMAT_PCM,
        wavHeader2_.NumOfChan,
        wavHeader2_.SamplesPerSec * 1000,
        wavHeader2_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource1 = {&slBufferQueue, &pcmFormat1};
    SLDataSource slSource2 = {&slBufferQueue, &pcmFormat2};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject1, &slSource1, &slSink, number, nullptr, nullptr);
    (*pcmPlayerObject1)->Realize(pcmPlayerObject1, SL_BOOLEAN_FALSE);

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject2, &slSource2, &slSink, number, nullptr, nullptr);
    (*pcmPlayerObject2)->Realize(pcmPlayerObject2, SL_BOOLEAN_FALSE);

    (*pcmPlayerObject1)->GetInterface(pcmPlayerObject1, SL_IID_PLAY, &playItf1);
    (*pcmPlayerObject2)->GetInterface(pcmPlayerObject2, SL_IID_PLAY, &playItf2);
    (*pcmPlayerObject1)->GetInterface(pcmPlayerObject1, SL_IID_VOLUME, &volumeItf1);

    SLmillibel level1 = 0;
    (*volumeItf1)->GetMaxVolumeLevel(volumeItf1, &level1);
    SLmillibel temp = 2;
    level1 = (SLmillibel) (level1 / temp);
    (*volumeItf1)->SetVolumeLevel(volumeItf1, level1);
    (*pcmPlayerObject2)->GetInterface(pcmPlayerObject2, SL_IID_VOLUME, &volumeItf2);
    SLmillibel level2 = 0;
    (*volumeItf2)->GetMaxVolumeLevel(volumeItf2, &level2);
    temp = 15; // MaxVolumeLevel
    level2 = (SLmillibel) (level2 / temp);
    (*volumeItf2)->SetVolumeLevel(volumeItf2, level2);

    (*pcmPlayerObject1)->GetInterface(pcmPlayerObject1, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf1);
    (*pcmPlayerObject2)->GetInterface(pcmPlayerObject2, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf2);
    (*bufferQueueItf1)->RegisterCallback(bufferQueueItf1, BufferQueueCallback, wavFile1_);
    (*bufferQueueItf2)->RegisterCallback(bufferQueueItf2, BufferQueueCallback, wavFile2_);
    PlayerStart(playItf1, bufferQueueItf1, wavFile1_);
    PlayerStart(playItf2, bufferQueueItf2, wavFile2_);
}

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    FILE *wavFile = (FILE *)pContext;
    if (!feof(wavFile)) {
        SLuint8 *buffer = nullptr;
        SLuint32 bufferSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, &bufferSize);
        if (buffer != nullptr) {
            fread(buffer, 1, size, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        }
    }
    return;
}

static void PlayerStart(SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile)
{
    AUDIO_INFO_LOG("PlayerStart");
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    return;
}

static void PlayerStop(SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf)
{
    AUDIO_INFO_LOG("PlayerStop");
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
    return;
}
