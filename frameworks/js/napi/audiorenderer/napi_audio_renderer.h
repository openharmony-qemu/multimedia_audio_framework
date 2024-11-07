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

#ifndef NAPI_AUDIO_RENDERER_H
#define NAPI_AUDIO_RENDERER_H

#include <iostream>
#include <map>
#include <queue>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "audio_stream_manager.h"
#include "audio_renderer.h"
#include "napi_async_work.h"
#include "napi_audio_renderer_device_change_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace HiviewDFX;
using namespace std;

const std::string NAPI_AUDIO_RENDERER_CLASS_NAME = "AudioRenderer";

class NapiAudioRenderer {
public:
    NapiAudioRenderer();
    ~NapiAudioRenderer() = default;

    static napi_value Init(napi_env env, napi_value exports);
    void DestroyCallbacks();
    void DestroyNAPICallbacks();

    std::unique_ptr<AudioRenderer> audioRenderer_;
    std::mutex writeCallbackMutex_;
    std::condition_variable writeCallbackCv_;

private:
    struct AudioRendererAsyncContext : public ContextBase {
        int32_t intValue;
        int32_t audioRendererRate;
        int32_t rendererFlags;
        int32_t interruptMode;
        bool isTrue;
        uint64_t time;
        size_t bufferLen;
        size_t bufferSize;
        int32_t volType;
        double volLevel;
        uint32_t rendererSampleRate;
        uint32_t audioStreamId;
        size_t totalBytesWritten;
        uint32_t underflowCount;
        void *data;
        int32_t audioEffectMode;
        int32_t channelBlendMode;
        DeviceRole deviceRole;
        int32_t deviceType;
        AudioRendererOptions rendererOptions;
        DeviceInfo deviceInfo;
        AudioRendererInfo rendererInfo;
        AudioStreamInfo streamInfo;
    };

    static napi_status InitNapiAudioRenderer(napi_env env, napi_value &constructor);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static void CreateRendererFailed();
    static napi_value Construct(napi_env env, napi_callback_info info);
    static unique_ptr<NapiAudioRenderer> CreateAudioRendererNativeObject(napi_env env);
    static napi_value CreateAudioRendererWrapper(napi_env env, const AudioRendererOptions rendererOptions);
    static napi_value CreateAudioRenderer(napi_env env, napi_callback_info info);
    static napi_value CreateAudioRendererSync(napi_env env, napi_callback_info info);
    static napi_value SetRenderRate(napi_env env, napi_callback_info info);
    static napi_value GetRenderRate(napi_env env, napi_callback_info info);
    static napi_value GetRenderRateSync(napi_env env, napi_callback_info info);
    static napi_value SetRendererSamplingRate(napi_env env, napi_callback_info info);
    static napi_value GetRendererSamplingRate(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Write(napi_env env, napi_callback_info info);
    static napi_value GetAudioTime(napi_env env, napi_callback_info info);
    static napi_value GetAudioTimeSync(napi_env env, napi_callback_info info);
    static napi_value Drain(napi_env env, napi_callback_info info);
    static napi_value Flush(napi_env env, napi_callback_info info);
    static napi_value Pause(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);
    static napi_value GetBufferSize(napi_env env, napi_callback_info info);
    static napi_value GetBufferSizeSync(napi_env env, napi_callback_info info);
    static napi_value GetAudioStreamId(napi_env env, napi_callback_info info);
    static napi_value GetAudioStreamIdSync(napi_env env, napi_callback_info info);
    static napi_value SetVolume(napi_env env, napi_callback_info info);
    static napi_value GetVolume(napi_env env, napi_callback_info info);
    static napi_value GetRendererInfo(napi_env env, napi_callback_info info);
    static napi_value GetRendererInfoSync(napi_env env, napi_callback_info info);
    static napi_value GetStreamInfo(napi_env env, napi_callback_info info);
    static napi_value GetStreamInfoSync(napi_env env, napi_callback_info info);
    static napi_value SetInterruptMode(napi_env env, napi_callback_info info);
    static napi_value SetInterruptModeSync(napi_env env, napi_callback_info info);
    static napi_value GetMinStreamVolume(napi_env env, napi_callback_info info);
    static napi_value GetMinStreamVolumeSync(napi_env env, napi_callback_info info);
    static napi_value GetMaxStreamVolume(napi_env env, napi_callback_info info);
    static napi_value GetMaxStreamVolumeSync(napi_env env, napi_callback_info info);
    static napi_value GetCurrentOutputDevices(napi_env env, napi_callback_info info);
    static napi_value GetCurrentOutputDevicesSync(napi_env env, napi_callback_info info);
    static napi_value GetUnderflowCount(napi_env env, napi_callback_info info);
    static napi_value GetUnderflowCountSync(napi_env env, napi_callback_info info);
    static napi_value GetAudioEffectMode(napi_env env, napi_callback_info info);
    static napi_value SetAudioEffectMode(napi_env env, napi_callback_info info);
    static napi_value SetChannelBlendMode(napi_env env, napi_callback_info info);
    static napi_value SetVolumeWithRamp(napi_env env, napi_callback_info info);
    static napi_value SetSpeed(napi_env env, napi_callback_info info);
    static napi_value GetSpeed(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value SetSilentModeAndMixWithOthers(napi_env env, napi_callback_info info);
    static napi_value GetSilentModeAndMixWithOthers(napi_env env, napi_callback_info info);
    static napi_value SetDefaultOutputDevice(napi_env env, napi_callback_info info);

    static napi_status WriteArrayBufferToNative(std::shared_ptr<AudioRendererAsyncContext> context);

    static napi_value RegisterCallback(napi_env env, napi_value jsThis,
        napi_value *argv, const std::string &cbName);
    static napi_value RegisterRendererCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioRenderer *napiRenderer);
    static napi_value RegisterPositionCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioRenderer *napiRenderer);
    static napi_value RegisterPeriodPositionCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioRenderer *napiRenderer);
    static napi_value RegisterDataRequestCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioRenderer *napiRenderer);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
        const std::string &cbName);
    static void RegisterRendererDeviceChangeCallback(napi_env env, napi_value *argv, NapiAudioRenderer *napiRenderer);
    static void UnregisterRendererCallback(napi_env env,
        const std::string &cbName, NapiAudioRenderer *napiRenderer);
    static void UnregisterRendererDeviceChangeCallback(napi_env env, size_t argc, const napi_value *argv,
        NapiAudioRenderer *napiRenderer);

    static void RegisterRendererOutputDeviceChangeWithInfoCallback(napi_env env, napi_value *argv,
        NapiAudioRenderer *napiRenderer);
    static void UnregisterRendererOutputDeviceChangeWithInfoCallback(napi_env env, size_t argc, const napi_value *argv,
        NapiAudioRenderer *napiRenderer);

    static void RegisterRendererWriteDataCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioRenderer *napiRenderer);
    static void UnregisterRendererWriteDataCallback(napi_env env, size_t argc, const napi_value *argv,
        NapiAudioRenderer *napiRenderer);
    /* common interface in AudioRendererNapi */
    static bool CheckContextStatus(std::shared_ptr<AudioRendererAsyncContext> context);
    static bool CheckAudioRendererStatus(NapiAudioRenderer *napi, std::shared_ptr<AudioRendererAsyncContext> context);
    static NapiAudioRenderer* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);

    static constexpr double MIN_VOLUME_IN_DOUBLE = 0.0;
    static constexpr double MAX_VOLUME_IN_DOUBLE = 1.0;
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;
    static std::unique_ptr<AudioRendererOptions> sRendererOptions_;

    ContentType contentType_;
    StreamUsage streamUsage_;
    napi_env env_;
    std::shared_ptr<AudioRendererCallback> callbackNapi_ = nullptr;
    std::shared_ptr<RendererPositionCallback> positionCbNapi_ = nullptr;
    std::shared_ptr<RendererPeriodPositionCallback> periodPositionCbNapi_ = nullptr;
    std::shared_ptr<AudioRendererWriteCallback> dataRequestCbNapi_ = nullptr;
    std::shared_ptr<NapiAudioRendererDeviceChangeCallback> rendererDeviceChangeCallbackNapi_ = nullptr;
    std::shared_ptr<NapiAudioRendererOutputDeviceChangeWithInfoCallback>
        rendererOutputDeviceChangeWithInfoCallbackNapi_ = nullptr;
    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> rendererPolicyServiceDiedCallbackNapi_ = nullptr;
    std::shared_ptr<AudioRendererWriteCallback> rendererWriteDataCallbackNapi_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_RENDERER_NAPI_H */
