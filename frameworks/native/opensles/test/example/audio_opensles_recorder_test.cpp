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
#define LOG_TAG "AudioOpenslesRecorderTest"
#endif

#include <OpenSLES.h>
#include <OpenSLES_OpenHarmony.h>
#include <OpenSLES_Platform.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <securec.h>

#include "audio_info.h"
#include "audio_log.h"

using namespace std;

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void CaptureOption(void);

static void OperationTime(uint64_t time);

static void CaptureStart(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile);

static void CapturePause(SLRecordItf recordItf);

static void CaptureStop(SLRecordItf recordItf);

static void OpenSLESCaptureTest();

const int PARAMETERS = 8;
FILE *wavFile_ = nullptr;
SLObjectItf engineObject = nullptr;
SLRecordItf  recordItf;
SLOHBufferQueueItf bufferQueueItf;
SLObjectItf pcmCapturerObject = nullptr;
struct timespec tv1 = {0};
struct timespec tv2 = {0};

int main(int argc, char *argv[])
{
    AUDIO_INFO_LOG("OpenSL ES capture test in");
    if (argc > PARAMETERS) {
        AUDIO_ERR_LOG("Incorrect number(%{public}d) of parameters", argc);
        return -1;
    }

    int opt = 0;
    string filePath = "/data/test.pcm";
    wavFile_ = fopen(filePath.c_str(), "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("OpenSL ES capture: Unable to open file");
        return -1;
    }

    OpenSLESCaptureTest();
    while ((opt = getopt(argc, argv, "s:p:S")) != -1) {
        switch (opt) {
            case 's':
                CaptureOption();
                break;
            case 'p':
                CapturePause(recordItf);
                break;
            case 'S':
            default:
                CaptureStop(recordItf);
                break;
        }
    }
}

static void CaptureOption(void)
{
    AUDIO_INFO_LOG("Enter CaptureOption.");
    uint64_t totalTime = strtoull(optarg, nullptr, 10);
    CaptureStart(recordItf, bufferQueueItf, wavFile_);
    OperationTime(totalTime);
}

static void OpenSLESCaptureTest()
{
    AUDIO_INFO_LOG("Enter OpenSLESCaptureTest");
    engineObject = nullptr;
    SLEngineItf engineItf = nullptr;

    slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        1,
        SL_SAMPLINGRATE_48,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };

    (*engineItf)->CreateAudioRecorder(engineItf, &pcmCapturerObject,
        &audioSource, &audioSink, 0, nullptr, nullptr);
    (*pcmCapturerObject)->Realize(pcmCapturerObject, SL_BOOLEAN_FALSE);
    
    (*pcmCapturerObject)->GetInterface(pcmCapturerObject, SL_IID_RECORD, &recordItf);
    (*pcmCapturerObject)->GetInterface(pcmCapturerObject, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf);
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BufferQueueCallback, wavFile_);

    return;
}

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    AUDIO_INFO_LOG("Enter BufferQueueCallback");
    FILE *wavFile = (FILE *)pContext;
    if (wavFile != nullptr) {
        SLuint8 *buffer = nullptr;
        SLuint32 bufferSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, &bufferSize);
        if (buffer != nullptr) {
            fwrite(buffer, 1, bufferSize, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        } else {
            AUDIO_ERR_LOG("buffer is null or bufferSize: %{public}lu, size: %{public}lu.", bufferSize, size);
        }
    }

    return;
}

static void CaptureStart(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile)
{
    AUDIO_INFO_LOG("Enter CaptureStart");
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);

    return;
}

static void CapturePause(SLRecordItf recordItf)
{
    AUDIO_INFO_LOG("Enter CapturePause");
    uint64_t totalTime = strtoull(optarg, nullptr, 10);
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_PAUSED);
    OperationTime(totalTime);

    return;
}

static void CaptureStop(SLRecordItf recordItf)
{
    AUDIO_INFO_LOG("Enter CaptureStop");
    fflush(wavFile_);
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_STOPPED);
    (*pcmCapturerObject)->Destroy(pcmCapturerObject);
    fclose(wavFile_);
    wavFile_ = nullptr;
    return;
}

static void OperationTime(uint64_t time)
{
    uint64_t usecTimes = 1000000000;
    time *= usecTimes;
    clock_gettime(CLOCK_REALTIME, &tv1);
    clock_gettime(CLOCK_REALTIME, &tv2);
    while (((tv2.tv_sec * usecTimes + tv2.tv_nsec) - (tv1.tv_sec * usecTimes + tv1.tv_nsec)) <= time) {
        sleep(1);
        clock_gettime(CLOCK_REALTIME, &tv2);
    }

    return;
}
