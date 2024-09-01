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

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include "native_audiostreambuilder.h"
#include <native_audiocapturer.h>
#include <thread>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif
namespace AudioTestConstants {
    constexpr int32_t FIRST_ARG_IDX = 1;
    constexpr int32_t SECOND_ARG_IDX = 2;
    constexpr int32_t THIRD_ARG_IDX = 3;
    constexpr int32_t RECODER_TIME = 10000;
    constexpr int32_t COUNTDOWN_INTERVAL = 1000;
    constexpr int32_t CONVERT_RATE = 1000;
}

std::string g_filePath = "/data/data/oh_test_audio.pcm";
FILE* g_file = nullptr;
int32_t g_samplingRate = 48000;
int32_t g_channelCount = 2;

static int32_t AudioCapturerOnReadData(OH_AudioCapturer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    printf("Get callback buffer, bufferLen:%d  \n", bufferLen);
    size_t count = 1;
    if (fwrite(buffer, bufferLen, count, g_file) != count) {
        printf("buffer fwrite err");
    }

    return 0;
}

static int32_t AudioErrCallback(OH_AudioCapturer* renderer,
    void* userData,
    OH_AudioStream_Result error)
{
    printf("recv err : code %d \n", error);
}

static int32_t AudioInterruptCallback(OH_AudioCapturer* renderer,
    void* userData,
    OH_AudioInterrupt_ForceType type,
    OH_AudioInterrupt_Hint hint)
{
    printf("recv interrupt event : type: %d hint: %d \n", type, hint);
}

static int32_t AudioEventCallback(OH_AudioCapturer* renderer,
    void* userData,
    OH_AudioStream_Event event)
{
    printf("recv event : event: %d \n", event);
}

void SleepWaitRecoder(bool* stop)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(AudioTestConstants::RECODER_TIME));
    *stop = true;
}

void RecorderTest(char *argv[])
{
    OH_AudioStream_Result ret;

    // 1. create builder
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    ret = OH_AudioStreamBuilder_Create(&builder, type);
    printf("create builder: %d \n", ret);

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, g_samplingRate);
    OH_AudioStreamBuilder_SetChannelCount(builder, g_channelCount);

    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    callbacks.OH_AudioCapturer_OnError = AudioErrCallback;
    callbacks.OH_AudioCapturer_OnInterruptEvent = AudioInterruptCallback;
    callbacks.OH_AudioCapturer_OnStreamEvent = AudioEventCallback;
    ret = OH_AudioStreamBuilder_SetCapturerCallback(builder, callbacks, nullptr);
    printf("setcallback: %d \n", ret);

    // 3. create OH_AudioCapturer
    OH_AudioCapturer* audioCapturer;
    ret = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    printf("create capturer client, ret: %d \n", ret);

    // 4. start
    ret = OH_AudioCapturer_Start(audioCapturer);
    printf("start ret: %d \n", ret);

    bool stop = false;
    std::thread stopthread(SleepWaitRecoder, &stop);
    stopthread.detach();

    int timeLeft = AudioTestConstants::RECODER_TIME;
    int64_t framePosition = 0;
    int64_t timestamp = 0;
    while (!stop) {
        OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_MONOTONIC, &framePosition, &timestamp);
        printf("Recording audio is in the countdown ... %d s (framePosition: %" PRId64 ")((timestamp: %" PRId64 "))\n",
            timeLeft / AudioTestConstants::CONVERT_RATE, framePosition, timestamp);
        std::this_thread::sleep_for(std::chrono::milliseconds(AudioTestConstants::COUNTDOWN_INTERVAL));
        timeLeft  = timeLeft - AudioTestConstants::COUNTDOWN_INTERVAL;
    }

    // 5. stop and release client
    ret = OH_AudioCapturer_Stop(audioCapturer);
    printf("stop ret: %d \n", ret);
    ret = OH_AudioCapturer_Release(audioCapturer);
    printf("release ret: %d \n", ret);

    // 6. destroy the builder
    ret = OH_AudioStreamBuilder_Destroy(builder);
    printf("destroy builder ret: %d \n", ret);
}


int main(int argc, char *argv[])
{
    if ((argv == nullptr) || (argc <= AudioTestConstants::THIRD_ARG_IDX)) {
        printf("input parms wrong. input format: samplingRate channelCount \n");
        printf("input demo: ./oh_audio_capturer_test 48000 2 \n");
        return 0;
    }

    printf("argc=%d ", argc);
    printf("argv[1]=%s ", argv[AudioTestConstants::FIRST_ARG_IDX]);
    printf("argv[2]=%s ", argv[AudioTestConstants::SECOND_ARG_IDX]);
    printf("argv[3]=%s \n", argv[AudioTestConstants::THIRD_ARG_IDX]);

    g_samplingRate = atoi(argv[AudioTestConstants::FIRST_ARG_IDX]);
    g_channelCount = atoi(argv[AudioTestConstants::SECOND_ARG_IDX]);

    g_file = fopen(g_filePath.c_str(), "wb");
    if (g_file == nullptr) {
        printf("OHAudioCapturerTest: Unable to open file \n");
        return 0;
    }

    RecorderTest(argv);

    fclose(g_file);
    g_file = nullptr;
    return 0;
}
#ifdef __cplusplus
}
#endif
