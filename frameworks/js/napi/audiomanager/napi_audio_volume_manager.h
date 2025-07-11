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
#ifndef NAPI_AUDIO_VOLUME_MANAGER_H
#define NAPI_AUDIO_VOLUME_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "napi_audio_volume_key_event.h"
#include "napi_audio_system_volume_change_callback.h"
#include "napi_audio_stream_volume_change_callback.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_VOLUME_MANAGER_NAPI_CLASS_NAME = "AudioVolumeManager";
class NapiAudioVolumeManager {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateVolumeManagerWrapper(napi_env env);

    NapiAudioVolumeManager();
    ~NapiAudioVolumeManager();

private:
    struct AudioVolumeManagerAsyncContext : public ContextBase {
        int32_t deviceFlag;
        bool bArgTransFlag = true;
        int32_t groupId;
        int32_t intValue;
        int32_t ringMode;
        bool isMute;
        bool isTrue;
        int32_t volLevel;
        int32_t appUid;
        bool isOwned;
        std::string networkId;
        std::vector<sptr<VolumeGroupInfo>> volumeGroupInfos;
        int32_t volumeType;
        int32_t duration;
    };

    static bool CheckContextStatus(std::shared_ptr<AudioVolumeManagerAsyncContext> context);
    static bool CheckAudioVolumeManagerStatus(NapiAudioVolumeManager *napi,
        std::shared_ptr<AudioVolumeManagerAsyncContext> context);
    static NapiAudioVolumeManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static napi_value GetVolumeGroupInfos(napi_env env, napi_callback_info info);
    static napi_value SetAppVolumePercentage(napi_env env, napi_callback_info info);
    static napi_value SetAppVolumePercentageForUid(napi_env env, napi_callback_info info);
    static napi_value GetAppVolumePercentage(napi_env env, napi_callback_info info);
    static napi_value GetAppVolumePercentageForUid(napi_env env, napi_callback_info info);
    static napi_value GetVolumeGroupInfosSync(napi_env env, napi_callback_info info);
    static napi_value GetVolumeGroupManager(napi_env env, napi_callback_info info);
    static napi_value GetVolumeGroupManagerSync(napi_env env, napi_callback_info info);
    static napi_value SetAppVolumeMutedForUid(napi_env env, napi_callback_info info);
    static napi_value IsAppVolumeMutedForUid(napi_env env, napi_callback_info info);
    static napi_value GetSystemVolume(napi_env env, napi_callback_info info);
    static napi_value GetMinSystemVolume(napi_env env, napi_callback_info info);
    static napi_value GetMaxSystemVolume(napi_env env, napi_callback_info info);
    static napi_value IsSystemMuted(napi_env env, napi_callback_info info);
    static napi_value GetVolumeInUnitOfDb(napi_env env, napi_callback_info info);
    static napi_value GetVolumeByStream(napi_env env, napi_callback_info info);
    static napi_value GetMinVolumeByStream(napi_env env, napi_callback_info info);
    static napi_value GetMaxVolumeByStream(napi_env env, napi_callback_info info);
    static napi_value IsSystemMutedForStream(napi_env env, napi_callback_info info);
    static napi_value GetVolumeInUnitOfDbByStream(napi_env env, napi_callback_info info);
    static napi_value GetSupportedAudioVolumeTypes(napi_env env, napi_callback_info info);
    static napi_value GetAudioVolumeTypeByStreamUsage(napi_env env, napi_callback_info info);
    static napi_value GetStreamUsagesByVolumeType(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
        const std::string &cbName);
    static napi_value RegisterSelfAppVolumeChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager);
    static void UnregisterSelfAppVolumeChangeCallback(napi_env env, napi_value callback, size_t argc,
        NapiAudioVolumeManager *napiAudioVolumeManager);
    static napi_value RegisterAppVolumeChangeForUidCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager);
    static void UnregisterAppVolumeChangeForUidCallback(napi_env env, napi_value callback, napi_value *args,
        size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, size_t argc,
        napi_value *args, const std::string &cbName);
    static void UnregisterCallbackFir(napi_env env, napi_value *args,
        size_t argc, const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager);
    static std::shared_ptr<NapiAudioVolumeKeyEvent> GetVolumeEventNapiCallback(napi_value argv,
        NapiAudioVolumeManager *napiVolumeManager);
    static std::shared_ptr<NapiAudioStreamVolumeChangeCallback> GetStreamVolumeChangeNapiCallback(napi_value argv,
        NapiAudioVolumeManager *napiVolumeManager);
    static std::shared_ptr<NapiAudioSystemVolumeChangeCallback> GetSystemVolumeChangeNapiCallback(napi_value argv,
        NapiAudioVolumeManager *napiVolumeManager);
    static napi_value RegisterActiveVolumeTypeChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager);
    static void UnregisterActiveVolumeTypeChangeCallback(napi_env env, napi_value callback, napi_value *args,
        size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager);
    static napi_value RegisterStreamVolumeChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager);
    static void UnregisterStreamVolumeChangeCallback(napi_env env, napi_value *args,
        size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager);
    static napi_value RegisterSystemVolumeChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager);
    static void UnregisterSystemVolumeChangeCallback(napi_env env, napi_value *args,
        size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager);

    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value ForceVolumeKeyControlType(napi_env env, napi_callback_info info);
    AudioSystemManager *audioSystemMngr_;

    int32_t cachedClientId_ = -1;
    std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallbackNapi_ = nullptr;
    std::shared_ptr<StreamVolumeChangeCallback> streamVolumeChangeCallbackNapi_ = nullptr;
    std::shared_ptr<SystemVolumeChangeCallback> systemVolumeChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> selfAppVolumeChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> appVolumeChangeCallbackForUidNapi_ = nullptr;
    std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> activeVolumeTypeChangeCallbackNapi_ = nullptr;
    std::list<std::shared_ptr<NapiAudioVolumeKeyEvent>> volumeKeyEventCallbackNapiList_;
    std::list<std::shared_ptr<NapiAudioStreamVolumeChangeCallback>> streamVolumeChangeCallbackNapiList_;
    std::list<std::shared_ptr<NapiAudioSystemVolumeChangeCallback>> systemVolumeChangeCallbackNapiList_;

    napi_env env_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_VOLUME_MANAGER_H */
