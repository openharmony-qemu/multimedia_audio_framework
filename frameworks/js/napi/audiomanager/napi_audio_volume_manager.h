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
#ifndef NAPI_AUDIO_VOLUME_MANAGER_H
#define NAPI_AUDIO_VOLUME_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "napi_audio_volume_key_event.h"

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
        std::string networkId;
        std::vector<sptr<VolumeGroupInfo>> volumeGroupInfos;
    };

    static bool CheckContextStatus(std::shared_ptr<AudioVolumeManagerAsyncContext> context);
    static bool CheckAudioVolumeManagerStatus(NapiAudioVolumeManager *napi,
        std::shared_ptr<AudioVolumeManagerAsyncContext> context);
    static NapiAudioVolumeManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static napi_value GetVolumeGroupInfos(napi_env env, napi_callback_info info);
    static napi_value GetVolumeGroupInfosSync(napi_env env, napi_callback_info info);
    static napi_value GetVolumeGroupManager(napi_env env, napi_callback_info info);
    static napi_value GetVolumeGroupManagerSync(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
        const std::string &cbName);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, size_t argc,
        napi_value *args, const std::string &cbName);
    static std::shared_ptr<NapiAudioVolumeKeyEvent> GetVolumeEventNapiCallback(napi_value argv,
        NapiAudioVolumeManager *napiVolumeManager);

    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    AudioSystemManager *audioSystemMngr_;

    int32_t cachedClientId_ = -1;
    std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallbackNapi_ = nullptr;
    std::list<std::shared_ptr<NapiAudioVolumeKeyEvent>> volumeKeyEventCallbackNapiList_;

    napi_env env_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_VOLUME_MANAGER_H */
