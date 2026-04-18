/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef NAPI_AUDIO_DEVICE_ENHANCE_MANAGER_H
#define NAPI_AUDIO_DEVICE_ENHANCE_MANAGER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "audio_device_enhance_manager.h"
#include "napi_async_work.h"
#include "napi_audio_capturer.h"
#include "napi_audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
inline const std::string AUDIO_DEVICE_ENHANCE_MANAGER_NAPI_CLASS_NAME = "AudioDeviceEnhanceManager";
class NapiAudioDeviceEnhanceManager {
public:
    NapiAudioDeviceEnhanceManager();
    ~NapiAudioDeviceEnhanceManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateDeviceEnhanceManagerWrapper(napi_env env);

private:
    struct AudioDeviceEnhanceManagerAsyncContext : public ContextBase {
        int32_t intValue = SUCCESS;
        bool supported = false;
        bool argTransFlag = true;
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
        std::shared_ptr<AudioRenderer> audioRenderer = nullptr;
        std::shared_ptr<AudioCapturer> audioCapturer = nullptr;
        SoundCardInfo soundCardInfo;
    };
    static bool CheckContextStatus(std::shared_ptr<AudioDeviceEnhanceManagerAsyncContext> context);
    static bool CheckDeviceEnhanceManagerStatus(
        NapiAudioDeviceEnhanceManager *napi, std::shared_ptr<AudioDeviceEnhanceManagerAsyncContext> context);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value IsEnhancedRoutingSupported(napi_env env, napi_callback_info info);
    static napi_value SelectOutputDevice(napi_env env, napi_callback_info info);
    static napi_value SelectInputDevice(napi_env env, napi_callback_info info);
    static napi_value SelectOutputDeviceForAudioRenderer(napi_env env, napi_callback_info info);
    static napi_value SelectInputDeviceForAudioCapturer(napi_env env, napi_callback_info info);
    static napi_value GetSoundCardInfo(napi_env env, napi_callback_info info);
private:
    AudioDeviceEnhanceManager *audioDeviceEnhanceMngr_;
    napi_env env_;
};
} // AudioStandard
} // OHOS

#endif // NAPI_AUDIO_DEVICE_ENHANCE_MANAGER_H
