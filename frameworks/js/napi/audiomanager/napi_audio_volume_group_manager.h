/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef NAPI_AUDIO_VOLUME_GROUP_MANAGER_H
#define NAPI_AUDIO_VOLUME_GROUP_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioVolumeGroupManager {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateAudioVolumeGroupManagerWrapper(napi_env env, int32_t groupId);

    static int32_t isConstructSuccess_;

private:
    struct AudioVolumeGroupManagerAsyncContext : public ContextBase {
        int32_t volType;
        int32_t volLevel;
        int32_t volFlag;
        int32_t deviceType;
        int32_t ringMode;
        int32_t scene;
        int32_t deviceFlag;
        int32_t intValue;
        int32_t groupId;
        int32_t adjustType;
        int32_t volumeAdjustStatus;
        bool isMute;
        bool isActive;
        bool isTrue;
        double volumeInDb;
        std::string key;
        std::string valueStr;
        int32_t networkId;
        double inputMaxAmplitude;
        double outputMaxAmplitude;
        bool inputBArgTransFlag;
        bool outputBArgTransFlag;
        int32_t policyType;
        std::shared_ptr<AudioDeviceDescriptor> inputDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();;
        std::shared_ptr<AudioDeviceDescriptor> outputDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();;
    };

    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value GetActiveVolumeTypeSync(napi_env env, napi_callback_info info);
    static napi_value GetVolume(napi_env env, napi_callback_info info);
    static napi_value GetVolumeSync(napi_env env, napi_callback_info info);
    static napi_value SetVolume(napi_env env, napi_callback_info info);
    static napi_value SetVolumeWithFlag(napi_env env, napi_callback_info info);
    static napi_value GetMaxVolume(napi_env env, napi_callback_info info);
    static napi_value GetMaxVolumeSync(napi_env env, napi_callback_info info);
    static napi_value GetMinVolume(napi_env env, napi_callback_info info);
    static napi_value GetMinVolumeSync(napi_env env, napi_callback_info info);
    static napi_value SetMute(napi_env env, napi_callback_info info);
    static napi_value IsStreamMute(napi_env env, napi_callback_info info);
    static napi_value IsStreamMuteSync(napi_env env, napi_callback_info info);
    static napi_value SetRingerMode(napi_env env, napi_callback_info info);
    static napi_value GetRingerMode(napi_env env, napi_callback_info info);
    static napi_value GetRingerModeSync(napi_env env, napi_callback_info info);
    static napi_value SetMicrophoneMute(napi_env env, napi_callback_info info);
    static napi_value IsMicrophoneMute(napi_env env, napi_callback_info info);
    static napi_value IsMicrophoneMuteSync(napi_env env, napi_callback_info info);
    static napi_value SetMicMute(napi_env env, napi_callback_info info);
    static napi_value SetMicMutePersistent(napi_env env, napi_callback_info info);
    static napi_value GetPersistentMicMuteState(napi_env env, napi_callback_info info);
    static napi_value IsVolumeUnadjustable(napi_env env, napi_callback_info info);
    static napi_value AdjustVolumeByStep(napi_env env, napi_callback_info info);
    static napi_value AdjustSystemVolumeByStep(napi_env env, napi_callback_info info);
    static napi_value GetSystemVolumeInDb(napi_env env, napi_callback_info info);
    static napi_value GetSystemVolumeInDbSync(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetMaxAmplitudeForOutputDevice(napi_env env, napi_callback_info info);
    static napi_value GetMaxAmplitudeForInputDevice(napi_env env, napi_callback_info info);

    static napi_value RegisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
        const std::string &cbName);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
        const std::string &cbName);
    static void UnregisterRingerModeCallback(NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager,
        size_t argc, napi_value *args);
    static void UnregisterMicStateChangeCallback(NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager,
        size_t argc, napi_value *args);
    static napi_value RegisterMicStateChangeCallback(napi_env env, napi_value *args, const std::string &cbName,
        NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager);
    static napi_value RegisterRingModeCallback(napi_env env, napi_value *args, const std::string &cbName,
        NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager);
    static napi_status InitNapiAudioVolumeGroupManager(napi_env env, napi_value &constructor);
    static bool CheckContextStatus(std::shared_ptr<AudioVolumeGroupManagerAsyncContext> context);
    static bool CheckAudioVolumeGroupManagerStatus(NapiAudioVolumeGroupManager *napi,
        std::shared_ptr<AudioVolumeGroupManagerAsyncContext> context);
    static NapiAudioVolumeGroupManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);

    static std::mutex volumeGroupManagerMutex_;

    std::shared_ptr<AudioGroupManager> audioGroupMngr_ = nullptr;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<AudioRingerModeCallback> ringerModecallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerMicStateChangeCallback> micStateChangeCallbackNapi_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_VOLUME_GROUP_MANAGER_H */
