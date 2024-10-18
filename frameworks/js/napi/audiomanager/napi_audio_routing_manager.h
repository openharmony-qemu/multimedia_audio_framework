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
#ifndef NAPI_AUDIO_ROUNTING_MANAGER_H
#define NAPI_AUDIO_ROUNTING_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string NAPI_AUDIO_ROUTING_MANAGER_CLASS_NAME = "AudioRoutingManager";
class NapiAudioRoutingManager {
public:
    NapiAudioRoutingManager();
    ~NapiAudioRoutingManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateRoutingManagerWrapper(napi_env env);

private:
    struct AudioRoutingManagerAsyncContext : public ContextBase {
        int32_t deviceFlag;
        int32_t deviceType;
        int32_t intValue;
        bool isActive;
        bool isTrue;
        bool bArgTransFlag = true;
        bool supported = false;
        AudioRendererInfo rendererInfo;
        AudioCapturerInfo captureInfo;
        sptr<AudioRendererFilter> audioRendererFilter;
        sptr<AudioCapturerFilter> audioCapturerFilter;
        std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors;
        std::vector<sptr<AudioDeviceDescriptor>> outDeviceDescriptors;
        std::vector<sptr<AudioDeviceDescriptor>> inputDeviceDescriptors;
    };

#if !defined(IOS_PLATFORM) && !defined(ANDROID_PLATFORM)
    static napi_value IsMicBlockDetectionSupported(napi_env env, napi_callback_info info);
#endif
    static napi_value GetDevices(napi_env env, napi_callback_info info);
    static napi_value GetDevicesSync(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value SelectOutputDevice(napi_env env, napi_callback_info info);
    static napi_value SelectOutputDeviceByFilter(napi_env env, napi_callback_info info);
    static napi_value SelectInputDevice(napi_env env, napi_callback_info info);
    static napi_value SelectInputDeviceByFilter(napi_env env, napi_callback_info info);
    static napi_value SetCommunicationDevice(napi_env env, napi_callback_info info);
    static napi_value IsCommunicationDeviceActive(napi_env env, napi_callback_info info);
    static napi_value IsCommunicationDeviceActiveSync(napi_env env, napi_callback_info info);
    static napi_value GetActiveOutputDeviceDescriptors(napi_env env, napi_callback_info info);
    static napi_value GetPreferredOutputDeviceForRendererInfo(napi_env env, napi_callback_info info);
    static napi_value GetPreferOutputDeviceForRendererInfo(napi_env env, napi_callback_info info);
    static napi_value GetPreferredOutputDeviceForRendererInfoSync(napi_env env, napi_callback_info info);
    static napi_value GetPreferredInputDeviceForCapturerInfo(napi_env env, napi_callback_info info);
    static napi_value GetPreferredInputDeviceForCapturerInfoSync(napi_env env, napi_callback_info info);
    static napi_value GetAvailableMicrophones(napi_env env, napi_callback_info info);
    static napi_value GetAvailableDevices(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
        const std::string &cbName);
    static void RegisterDeviceChangeCallback(napi_env env, size_t argc, napi_value *args, const std::string &cbName,
        NapiAudioRoutingManager *napiRoutingMgr);
    static void RegisterPreferredOutputDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
        const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr);
    static void RegisterPreferredInputDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
        const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr);
    static void RegisterAvaiableDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
        const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr);
    static void UnregisterDeviceChangeCallback(napi_env env, napi_value callback,
        NapiAudioRoutingManager *napiRoutingMgr);
    static void UnregisterPreferredOutputDeviceChangeCallback(napi_env env, napi_value callback,
        NapiAudioRoutingManager *napiRoutingMgr);
    static void UnregisterPreferredInputDeviceChangeCallback(napi_env env, napi_value callback,
        NapiAudioRoutingManager *napiRoutingMgr);
    static void UnregisterAvailableDeviceChangeCallback(napi_env env, napi_value callback,
        NapiAudioRoutingManager *napiRoutingMgr);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, const std::string &callbackName,
        napi_value callback);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);

    static bool CheckContextStatus(std::shared_ptr<AudioRoutingManagerAsyncContext> context);
    static bool CheckAudioRoutingManagerStatus(NapiAudioRoutingManager *napi,
        std::shared_ptr<AudioRoutingManagerAsyncContext> context);
    static NapiAudioRoutingManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);

    static void RegisterMicrophoneBlockedCallback(napi_env env, size_t argc, napi_value *args,
        const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr);
    static void UnregisterMicrophoneBlockedCallback(napi_env env, napi_value callback,
        NapiAudioRoutingManager *napiRoutingMgr);

    static int32_t NapiAudioRountingMicroPhoneBlockCallback();

    AudioSystemManager *audioMngr_;
    AudioRoutingManager *audioRoutingMngr_ = nullptr;
    std::shared_ptr<AudioManagerDeviceChangeCallback> deviceChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerMicStateChangeCallback> micStateChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> preferredOutputDeviceCallbackNapi_ = nullptr;
    std::shared_ptr<AudioPreferredInputDeviceChangeCallback> preferredInputDeviceCallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> availableDeviceChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerMicrophoneBlockedCallback> microphoneBlockedCallbackNapi_ = nullptr;

    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_ROUNTING_MANAGER_H */