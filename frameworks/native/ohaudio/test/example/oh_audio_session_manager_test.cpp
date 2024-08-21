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

#include "native_audio_session_manager.h"
#include "native_audio_common.h"
#include "audio_session_manager.h"
#include <ostream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include <cinttypes>
#include "common/native_audiostreambuilder.h"
#include "native_audiorenderer.h"

namespace OHOS {
namespace AudioStandard {

class SessionNdkTest {
public:
    SessionNdkTest() = default;
    ~SessionNdkTest() = default;
    void Init();
    void RegisterCallback(OH_AudioSession_DeactivatedCallback callback);
    void UnRegisterCallback(OH_AudioSession_DeactivatedCallback callback);
    void ActivateAudioSession(OH_AudioSession_Strategy strategy);
    void DeActivateAudioSession();
    void IsAudioSessionActivated();
    void RegisterAndActive(OH_AudioSession_DeactivatedCallback callback, OH_AudioSession_Strategy strategy);
    int32_t MyCallbackFunction(OH_AudioSession_DeactivatedEvent event);
    OH_AudioRenderer* StartPlay();
    void StopPlay(OH_AudioRenderer* audioRenderer);
    OH_AudioRenderer* PlayMusic(OH_AudioSession_DeactivatedCallback callback,
        OH_AudioSession_ConcurrencyMode mode);
    void LogicPathCheck(int operate, OH_AudioSession_DeactivatedCallback callback);
    void PlayCheck(int operate, OH_AudioSession_DeactivatedCallback callback);
};

OH_AudioSessionManager *audioSessionManager;

OH_AudioSession_DeactivatedEvent event;

std::shared_ptr<SessionNdkTest> g_sessionNdkTest = nullptr;

OH_AudioSession_Strategy strategy;

const int CASE_EXIT = 0;

const int CASE_REGISTER = 1;

const int CASE_UN_REGISTER = 2;

const int CASE_ACTIVE = 3;

const int CASE_DEACTIVE = 4;

const int CASE_IS_ACTIVE = 5;

const int CASE_REGISTER_ACTIVE = 6;

const int CASE_PAUSE_PLAY = 7;

const int CASE_MIX_PLAY = 8;

const int CASE_DUCK_PLAY = 9;

const int CASE_DEFAULT_PLAY = 10;

const int CASE_STOP_PLAY = 100;

// audio renderer
OH_AudioRenderer* audioRenderer;
std::string g_filePath = "/data/data/oh_test_audio_session.pcm";
FILE* g_file = nullptr;
bool g_readEnd = false;
int32_t g_samplingRate = 48000;
int32_t g_channelCount = 2;
int32_t g_latencyMode = 0;
int32_t g_sampleFormat = 1;
int32_t g_frameSize = 240;
float g_speed = 1.0f;

static int32_t AudioRendererOnWriteData(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    size_t readCount = fread(buffer, bufferLen, 1, g_file);
    if (!readCount) {
        if (ferror(g_file)) {
            std::cout << "Error reading myfile" << std::endl;
        } else if (feof(g_file)) {
            std::cout << "EOF found" << std::endl;
            g_readEnd = true;
        }
    }
    return 0;
}

static void AudioRendererDeviceChangeCb(OH_AudioRenderer* renderer, void* userData,
    OH_AudioStream_DeviceChangeReason reason)
{
    std::cout << "AudioRendererDeviceChangeCb reason: " << reason << std::endl;
}

static void UserOperateDsiplay()
{
    std::cout << "Please Input Operate" << std::endl;
    std::cout << "1 -----> Register Callback" << std::endl;
    std::cout << "2 -----> UnRegister Callback" << std::endl;
    std::cout << "3 -----> Activate AudioSession" << std::endl;
    std::cout << "4 -----> DeActivate AudioSession" << std::endl;
    std::cout << "5 -----> Is Activate Judging" << std::endl;
    std::cout << "6 -----> Register and Active" << std::endl;
    std::cout << "7 -----> Pause Play" << std::endl;
    std::cout << "8 -----> Mix Play" << std::endl;
    std::cout << "9 -----> Duck Play" << std::endl;
    std::cout << "10 -----> Default Play" << std::endl;
    std::cout << "100 -----> Stop Play" << std::endl;
    std::cout << "0 -----> Exit" << std::endl;
}

void SessionNdkTest::Init()
{
    OH_AudioManager_GetAudioSessionManager(&audioSessionManager);
}

void SessionNdkTest::RegisterCallback(OH_AudioSession_DeactivatedCallback callback)
{
    std::cout << "Start Register callback" << std::endl;
    OH_AudioCommon_Result result =
        OH_AudioSessionManager_RegisterSessionDeactivatedCallback(audioSessionManager, callback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        std::cout << "Register callback SUCCESS" << std::endl;
    } else {
        std::cout << "Register callback FAILED" << std::endl;
    }
}

void SessionNdkTest::UnRegisterCallback(OH_AudioSession_DeactivatedCallback callback)
{
    std::cout << "Start UnRegister callback" << std::endl;
    OH_AudioCommon_Result result =
        OH_AudioSessionManager_UnregisterSessionDeactivatedCallback(audioSessionManager, callback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        std::cout << "UnRegister callback SUCCESS" << std::endl;
    } else {
        std::cout << "UnRegister callback FAILED" << std::endl;
    }
}

void SessionNdkTest::ActivateAudioSession(OH_AudioSession_Strategy strategy)
{
    std::cout << "Start Activate AudioSession" << std::endl;
    OH_AudioCommon_Result result =
        OH_AudioSessionManager_ActivateAudioSession(audioSessionManager, &strategy);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        std::cout << "Activate AudioSession SUCCESS" << std::endl;
    } else {
        std::cout << "Activate AudioSession FAILED" << std::endl;
    }
}

void SessionNdkTest::DeActivateAudioSession()
{
    std::cout << "Start DeActivate AudioSession" << std::endl;
    OH_AudioCommon_Result result = OH_AudioSessionManager_DeactivateAudioSession(audioSessionManager);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        std::cout << "DeActivate AudioSession SUCCESS" << std::endl;
    } else {
        std::cout << "DeActivate AudioSession FAILED" << std::endl;
    }
}

void SessionNdkTest::IsAudioSessionActivated()
{
    std::cout << "Start Is Activate AudioSession" << std::endl;
    bool result = OH_AudioSessionManager_IsAudioSessionActivated(audioSessionManager);
    if (result) {
        std::cout << "Is Activate AudioSession SUCCESS" << std::endl;
    } else {
        std::cout << "Is Activate AudioSession FAILED" << std::endl;
    }
}
 
void SessionNdkTest::RegisterAndActive(OH_AudioSession_DeactivatedCallback callback,
    OH_AudioSession_Strategy strategy)
{
    RegisterCallback(callback);
    ActivateAudioSession(strategy);
}

OH_AudioRenderer* SessionNdkTest::StartPlay()
{
    OH_AudioStream_Result ret;

    // 1. create builder
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    ret = OH_AudioStreamBuilder_Create(&builder, type);
    std::cout << "[Renderer] createcallback ret: " << ret << std::endl;

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, g_samplingRate);
    OH_AudioStreamBuilder_SetChannelCount(builder, g_channelCount);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)g_latencyMode);
    OH_AudioStreamBuilder_SetSampleFormat(builder, (OH_AudioStream_SampleFormat)g_sampleFormat);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    ret = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    std::cout << "[Renderer] setcallback ret: " << ret << std::endl;

    OH_AudioRenderer_OutputDeviceChangeCallback deviceChangeCb = AudioRendererDeviceChangeCb;
    ret = OH_AudioStreamBuilder_SetRendererOutputDeviceChangeCallback(builder, deviceChangeCb, nullptr);
    std::cout << "[Renderer] set device change callback ret: " << ret << std::endl;

    //  set buffer size to g_frameSize
    ret = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder, g_frameSize);
    std::cout << "[Renderer] set buffer size, ret: " << ret << std::endl;

    // 3. create OH_AudioRenderer
    ret = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    std::cout << "[Renderer] create renderer client, ret: " << ret << std::endl;

    // 4. setspeed
    ret = OH_AudioRenderer_SetSpeed(audioRenderer, g_speed);
    std::cout << "[Renderer] speed ret: " << ret << std::endl;
    
    // 5. start
    ret = OH_AudioRenderer_Start(audioRenderer);
    std::cout << "[Renderer] start ret: " << ret << std::endl;

    int32_t frameSize;
    OH_AudioRenderer_GetFrameSizeInCallback(audioRenderer, &frameSize);
    std::cout << "[Renderer] framesize: " << frameSize << std::endl;
    return audioRenderer;
}

void SessionNdkTest::StopPlay(OH_AudioRenderer* audioRenderer)
{
    DeActivateAudioSession();
    OH_AudioStream_Result ret = OH_AudioRenderer_Stop(audioRenderer);
    std::cout << "[Renderer] stop ret: " << ret << std::endl;
    ret = OH_AudioRenderer_Release(audioRenderer);
    std::cout << "[Renderer] release ret: " << ret << std::endl;
}

OH_AudioRenderer* SessionNdkTest::PlayMusic(OH_AudioSession_DeactivatedCallback callback,
    OH_AudioSession_ConcurrencyMode mode)
{
    event.reason = DEACTIVATED_LOWER_PRIORITY;
    callback(event);
    strategy.concurrencyMode = mode;
    RegisterAndActive(callback, strategy);
    g_file = fopen(g_filePath.c_str(), "rb");
    if (g_file == nullptr) {
        std::cout << "Unable to open file" << std::endl;
        return nullptr;
    }
    return StartPlay();
}

void SessionNdkTest::LogicPathCheck(int operate, OH_AudioSession_DeactivatedCallback callback)
{
    switch (operate) {
        case CASE_REGISTER:
            event.reason = DEACTIVATED_LOWER_PRIORITY;
            callback(event);
            RegisterCallback(callback);
            break;
        case CASE_UN_REGISTER:
            event.reason = DEACTIVATED_TIMEOUT;
            callback(event);
            UnRegisterCallback(callback);
            break;
        case CASE_ACTIVE:
            strategy.concurrencyMode = CONCURRENCY_MIX_WITH_OTHERS;
            ActivateAudioSession(strategy);
            break;
        case CASE_DEACTIVE:
            DeActivateAudioSession();
            break;
        case CASE_IS_ACTIVE:
            IsAudioSessionActivated();
            break;
        case CASE_REGISTER_ACTIVE:
            event.reason = DEACTIVATED_LOWER_PRIORITY;
            callback(event);
            strategy.concurrencyMode = CONCURRENCY_MIX_WITH_OTHERS;
            RegisterAndActive(callback, strategy);
            break;
        default:
            std::cout << "Continue" << std::endl;
    }
}

void SessionNdkTest::PlayCheck(int operate, OH_AudioSession_DeactivatedCallback callback)
{
    switch (operate) {
        case CASE_PAUSE_PLAY:
            audioRenderer = PlayMusic(callback, CONCURRENCY_PAUSE_OTHERS);
            if (audioRenderer == nullptr) {
                std::cout << "[PAUSE] Start play failed" << std::endl;
            } else {
                std::cout << "[PAUSE] Start play completed" << std::endl;
            }
            break;
        case CASE_MIX_PLAY:
            audioRenderer = PlayMusic(callback, CONCURRENCY_MIX_WITH_OTHERS);
            if (audioRenderer == nullptr) {
                std::cout << "[MIX] Start play failed" << std::endl;
            } else {
                std::cout << "[MIX] Start play completed" << std::endl;
            }
            break;
        case CASE_DUCK_PLAY:
            audioRenderer = PlayMusic(callback, CONCURRENCY_DUCK_OTHERS);
            if (audioRenderer == nullptr) {
                std::cout << "[DUCK] Start play failed" << std::endl;
            } else {
                std::cout << "[DUCK] Start play completed" << std::endl;
            }
            break;
        case CASE_DEFAULT_PLAY:
            audioRenderer = PlayMusic(callback, CONCURRENCY_DEFAULT);
            if (audioRenderer == nullptr) {
                std::cout << "[DEFAULT] Start play failed" << std::endl;
            } else {
                std::cout << "[DEFAULT] Start play completed" << std::endl;
            }
            break;
        case CASE_STOP_PLAY:
            StopPlay(audioRenderer);
            std::cout << "Stop play completed" << std::endl;
            break;
        default:
            std::cout << "End" << std::endl;
    }
}

int32_t MyCallbackFunction(OH_AudioSession_DeactivatedEvent event)
{
    std::cout << "Callback For Event Reason: " << static_cast<int>(event.reason) << std::endl;
    return 0;
}

} // namespace AudioStandard
} // namespace OHOS


using namespace OHOS::AudioStandard;
int main()
{
    // Init Get AudioSessionManager
    g_sessionNdkTest = std::make_shared<SessionNdkTest>();
    
    g_sessionNdkTest->Init();

    std::cout << "Init Completed, Start Test" << std::endl;

    bool runFlag = true;
    int operate;

    OH_AudioSession_DeactivatedCallback callback = MyCallbackFunction;

    while (runFlag) {
        UserOperateDsiplay();

        std::cin >> operate;

        if (operate == CASE_EXIT) {
            runFlag = false;
        }

        g_sessionNdkTest->LogicPathCheck(operate, callback);
        g_sessionNdkTest->PlayCheck(operate, callback);

        if (operate != CASE_REGISTER && operate != CASE_UN_REGISTER && operate != CASE_ACTIVE &&
            operate != CASE_DEACTIVE && operate != CASE_IS_ACTIVE && operate != CASE_REGISTER_ACTIVE &&
            operate != CASE_PAUSE_PLAY && operate != CASE_MIX_PLAY && operate != CASE_DUCK_PLAY &&
            operate != CASE_DEFAULT_PLAY && operate != CASE_STOP_PLAY) {
                std::cout << "Input Valid, RE Input";
        }
    }
    std::cout << "End Test" << std::endl;
}