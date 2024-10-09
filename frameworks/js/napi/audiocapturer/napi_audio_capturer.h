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
#ifndef NAPI_AUDIO_CAPTURER_H
#define NAPI_AUDIO_CAPTURER_H

#include <iostream>
#include <map>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "audio_capturer.h"
#include "napi_async_work.h"
#include "napi_audio_capturer_device_change_callback.h"
#include "napi_audio_capturer_info_change_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const std::string NAPI_AUDIO_CAPTURER_CLASS_NAME = "AudioCapturer";

class NapiAudioCapturer {
public:
    NapiAudioCapturer();
    ~NapiAudioCapturer();

    static napi_value Init(napi_env env, napi_value exports);

    std::unique_ptr<AudioCapturer> audioCapturer_;
    std::mutex readCallbackMutex_;
    std::condition_variable readCallbackCv_;

private:
    struct AudioCapturerAsyncContext : public ContextBase {
        virtual ~AudioCapturerAsyncContext()
        {
            AUDIO_DEBUG_LOG("~AudioCapturerAsyncContext enter");
        }
        uint64_t time;
        int32_t intValue;
        uint32_t userSize;
        uint8_t *buffer = nullptr;
        size_t bytesRead;
        size_t bufferSize;
        uint32_t audioStreamId;
        bool isBlocking;
        bool isTrue;
        SourceType sourceType;
        AudioCapturerOptions capturerOptions;
        AudioCapturerInfo capturerInfo;
        AudioStreamInfo streamInfo;
        uint32_t overflowCount;
    };

    static napi_status InitAudioCapturer(napi_env env, napi_value &constructor);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateAudioCapturerWrapper(napi_env env, const AudioCapturerOptions capturerOptions);
    static napi_value CreateAudioCapturer(napi_env env, napi_callback_info info);
    static napi_value CreateAudioCapturerSync(napi_env env, napi_callback_info info);
    static napi_value GetCapturerInfo(napi_env env, napi_callback_info info);
    static napi_value GetCapturerInfoSync(napi_env env, napi_callback_info info);
    static napi_value GetStreamInfo(napi_env env, napi_callback_info info);
    static napi_value GetStreamInfoSync(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Read(napi_env env, napi_callback_info info);
    static napi_value GetAudioTime(napi_env env, napi_callback_info info);
    static napi_value GetAudioTimeSync(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);
    static napi_value GetBufferSize(napi_env env, napi_callback_info info);
    static napi_value GetBufferSizeSync(napi_env env, napi_callback_info info);
    static napi_value GetAudioStreamId(napi_env env, napi_callback_info info);
    static napi_value GetAudioStreamIdSync(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetCurrentInputDevices(napi_env env, napi_callback_info info);
    static napi_value GetCurrentAudioCapturerChangeInfo(napi_env env, napi_callback_info info);
    static napi_value GetCurrentMicrophones(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value GetOverflowCount(napi_env env, napi_callback_info info);
    static napi_value GetOverflowCountSync(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_value jsThis,
        napi_value *argv, const std::string &cbName);
    static napi_value RegisterCapturerCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioCapturer *napiCapturer);
    static napi_value RegisterPositionCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioCapturer *napiCapturer);
    static napi_value RegisterPeriodPositionCallback(napi_env env, napi_value *argv, const std::string &cbName,
        NapiAudioCapturer *napiCapturer);
    static void RegisterAudioCapturerDeviceChangeCallback(napi_env env, napi_value *argv,
        NapiAudioCapturer *napiCapturer);
    static void RegisterAudioCapturerInfoChangeCallback(napi_env env, napi_value *argv,
        NapiAudioCapturer *napiCapturer);
    static void RegisterCapturerReadDataCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioCapturer *napiCapturer);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
        const std::string &cbName);
    static void UnregisterCapturerCallback(napi_env env, const std::string &cbName,
        NapiAudioCapturer *napiCapturer);
    static void UnregisterAudioCapturerDeviceChangeCallback(napi_env env, size_t argc,
        napi_value *argv, NapiAudioCapturer *napiCapturer);
    static void UnregisterAudioCapturerInfoChangeCallback(napi_env env, size_t argc,
        napi_value *argv, NapiAudioCapturer *napiCapturer);
    static void UnregisterCapturerReadDataCallback(napi_env env, size_t argc, napi_value *argv,
        NapiAudioCapturer *napiCapturer);

    /* common interface in NapiAudioCapturer */
    static bool CheckContextStatus(std::shared_ptr<AudioCapturerAsyncContext> context);
    static bool CheckAudioCapturerStatus(NapiAudioCapturer *napi, std::shared_ptr<AudioCapturerAsyncContext> context);
    static NapiAudioCapturer* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static unique_ptr<NapiAudioCapturer> CreateAudioCapturerNativeObject(napi_env env);
    static napi_status ReadFromNative(shared_ptr<AudioCapturerAsyncContext> context);
    static std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> GetDeviceChangeNapiCallback(napi_value argv,
        NapiAudioCapturer *napiCapturer);
    static std::shared_ptr<NapiAudioCapturerInfoChangeCallback> GetCapturerInfoChangeNapiCallback(
        napi_value argv, NapiAudioCapturer *napiCapturer);

    static std::unique_ptr<AudioCapturerOptions> sCapturerOptions_;
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;

    std::shared_ptr<AudioCapturerCallback> callbackNapi_ = nullptr;
    std::shared_ptr<CapturerPositionCallback> positionCbNapi_ = nullptr;
    std::shared_ptr<CapturerPeriodPositionCallback> periodPositionCbNapi_ = nullptr;
    std::list<std::shared_ptr<NapiAudioCapturerDeviceChangeCallback>> deviceChangeCallbacks_;
    std::list<std::shared_ptr<NapiAudioCapturerInfoChangeCallback>> capturerInfoChangeCallbacks_;
    std::shared_ptr<AudioCapturerReadCallback> capturerReadDataCallbackNapi_ = nullptr;

    SourceType sourceType_;
    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_CAPTURER_H */
