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
#ifndef NAPI_AUDIO_SESSION_MANAGER_H
#define NAPI_AUDIO_SESSION_MANAGER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_session_info.h"
#include "audio_session_manager.h"
#include "napi_audio_session_state_callback.h"
#include "napi_audio_session_device_callback.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIOSESSION_CALLBACK_NAME = "audioSessionDeactivated";
const std::string AUDIOSESSION_STATE_CALLBACK_NAME = "audioSessionStateChanged";
const std::string AUDIOSESSION_DEVICE_CALLBACK_NAME = "currentOutputDeviceChanged";

class NapiAudioSessionMgr {
public:
    NapiAudioSessionMgr();
    ~NapiAudioSessionMgr();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateSessionManagerWrapper(napi_env env);

private:
    struct AudioSessionMgrAsyncContext : public ContextBase {
        int32_t intValue;
        int32_t volType;
        int32_t contentType;
        bool isTrue;
        bool isActive;
        NapiAudioSessionMgr *objectInfo;
        AudioSessionStrategy audioSessionStrategy;
        int32_t deviceType;
    };

    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static bool CheckContextStatus(std::shared_ptr<AudioSessionMgrAsyncContext> context);
    static bool CheckAudioSessionStatus(NapiAudioSessionMgr *napi,
        std::shared_ptr<AudioSessionMgrAsyncContext> context);
    static napi_value ActivateAudioSession(napi_env env, napi_callback_info info);
    static napi_value DeactivateAudioSession(napi_env env, napi_callback_info info);
    static napi_value IsAudioSessionActivated(napi_env env, napi_callback_info info);
    static void RegisterCallback(napi_env env, napi_value jsThis, napi_value *args, const std::string &cbName);
    static void UnregisterCallback(napi_env env, napi_value jsThis);
    static void UnregisterCallbackCarryParam(napi_env env, napi_value jsThis, napi_value *args, size_t len);
    static void RegisterAudioSessionCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr);
    static void UnsetAudioSessionCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr);

    static NapiAudioSessionMgr *GetParamWithSync(const napi_env &env,
        napi_callback_info info, size_t &argc, napi_value *args);
    static napi_value SetAudioSessionScene(napi_env env, napi_callback_info info);
    static void RegisterAudioSessionStateCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr);
    static void UnregisterSessionStateCallback(napi_env env, napi_value jsThis);
    static void UnregisterSessionStateCallbackCarryParam(napi_env env, napi_value jsThis, napi_value *args, size_t len);
    static napi_value GetDefaultOutputDevice(napi_env env, napi_callback_info info);
    static napi_value SetDefaultOutputDevice(napi_env env, napi_callback_info info);
    static std::shared_ptr<NapiAudioSessionStateCallback> GetAudioSessionStateCallback(
        napi_value argv, NapiAudioSessionMgr *napiSessionMgr);
    static void RegisterAudioSessionDeviceCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr);
    static void UnregisterSessionDeviceCallback(napi_env env, napi_value jsThis);
    static void UnregisterSessionDeviceCallbackCarryParam(
        napi_env env, napi_value jsThis, napi_value *args, size_t len);
    static std::shared_ptr<NapiAudioSessionDeviceCallback> GetAudioSessionDeviceCallback(
        napi_value argv, NapiAudioSessionMgr *napiSessionMgr);

    napi_env env_;
    AudioSessionManager *audioSessionMngr_;
    std::shared_ptr<AudioSessionCallback> audioSessionCallbackNapi_ = nullptr;
    std::list<std::shared_ptr<NapiAudioSessionStateCallback>> sessionStateCallbackList_;
    std::list<std::shared_ptr<NapiAudioSessionDeviceCallback>> sessionDeviceCallbackList_;

    std::mutex sessionStateCbMutex_;
    std::mutex sessionDeviceCbMutex_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_SESSION_MANAGER_H */
