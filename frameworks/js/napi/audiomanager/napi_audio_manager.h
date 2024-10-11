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
#ifndef NAPI_AUDIO_MANAGER_H
#define NAPI_AUDIO_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string NAPI_AUDIO_MNGR_CLASS_NAME = "AudioManager";
const std::string VOLUME_CHANGE_CALLBACK_NAME = "volumeChange";

class NapiAudioManager {
public:
    NapiAudioManager();
    ~NapiAudioManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_status InitNapiAudioManager(napi_env env, napi_value &constructor);

private:
struct AudioManagerAsyncContext : public ContextBase {
    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    int32_t ringMode;
    int32_t scene;
    int32_t deviceFlag;
    int32_t intValue;
    int32_t focusType;
    int32_t groupId;
    bool isMute;
    bool isActive;
    bool isTrue;
    std::string key;
    std::string valueStr;
    std::string networkId;
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors;
    std::vector<sptr<VolumeGroupInfo>> volumeGroupInfos;
    std::vector<std::pair<std::string, std::string>> subKvpairs;
    std::vector<std::string> subKeys;
};
    static NapiAudioManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args);
    static bool CheckContextStatus(std::shared_ptr<AudioManagerAsyncContext> context);
    static bool CheckAudioManagerStatus(NapiAudioManager *napi,
        std::shared_ptr<AudioManagerAsyncContext> context);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateAudioManagerWrapper(napi_env env);
    static napi_value GetAudioManager(napi_env env, napi_callback_info info);
    static napi_value SetVolume(napi_env env, napi_callback_info info);
    static napi_value GetVolume(napi_env env, napi_callback_info info);
    static napi_value GetMaxVolume(napi_env env, napi_callback_info info);
    static napi_value GetMinVolume(napi_env env, napi_callback_info info);
    static napi_value GetDevices(napi_env env, napi_callback_info info);
    static napi_value SetStreamMute(napi_env env, napi_callback_info info);
    static napi_value IsStreamMute(napi_env env, napi_callback_info info);
    static napi_value IsStreamActive(napi_env env, napi_callback_info info);
    static napi_value SetRingerMode(napi_env env, napi_callback_info info);
    static napi_value GetRingerMode(napi_env env, napi_callback_info info);
    static napi_value SetAudioScene(napi_env env, napi_callback_info info);
    static napi_value GetAudioScene(napi_env env, napi_callback_info info);
    static napi_value GetAudioSceneSync(napi_env env, napi_callback_info info);
    static napi_value SetDeviceActive(napi_env env, napi_callback_info info);
    static napi_value IsDeviceActive(napi_env env, napi_callback_info info);
    static napi_value SetAudioParameter(napi_env env, napi_callback_info info);
    static napi_value GetAudioParameter(napi_env env, napi_callback_info info);
    static napi_value SetExtraParameters(napi_env env, napi_callback_info info);
    static napi_value GetExtraParameters(napi_env env, napi_callback_info info);
    static napi_value SetMicrophoneMute(napi_env env, napi_callback_info info);
    static napi_value IsMicrophoneMute(napi_env env, napi_callback_info info);
    static napi_value RequestIndependentInterrupt(napi_env env, napi_callback_info info);
    static napi_value AbandonIndependentInterrupt(napi_env env, napi_callback_info info);
    static napi_value GetStreamManager(napi_env env, napi_callback_info info);
    static napi_value GetSessionManager(napi_env env, napi_callback_info info);
    static napi_value GetRoutingManager(napi_env env, napi_callback_info info);
    static napi_value GetVolumeManager(napi_env env, napi_callback_info info);
    static napi_value GetInterruptManager(napi_env env, napi_callback_info info);
    static napi_value GetSpatializationManager(napi_env env, napi_callback_info info);
    static napi_value DisableSafeMediaVolume(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static void UnregisterDeviceChangeCallback(napi_env env, napi_value callback, NapiAudioManager *audioMgrNapi);
    template<typename T> static void UnregisterInterruptCallback(napi_env env, const T &argv,
        const size_t argCount, NapiAudioManager *napiAudioManager);
    static napi_value RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *argv, const std::string &cbName);
    template<typename T> static void RegisterInterruptCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager);
    template<typename T> static void RegisterRingerModeCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager);
    template<typename T> static void RegisterVolumeChangeCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager);
    template<typename T> static void RegisterDeviceChangeCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager);

    AudioSystemManager *audioMngr_;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<AudioManagerDeviceChangeCallback> deviceChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerCallback> interruptCallbackNapi_ = nullptr;
    std::shared_ptr<AudioRingerModeCallback> ringerModecallbackNapi_ = nullptr;
    std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallbackNapi_ = nullptr;
    napi_env env_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_MANAGER_H */