/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AniModuleAudio"
#endif

#include "ani_audio_manager.h"
#include "audio_common_log.h"

using namespace OHOS::AudioStandard;

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        AUDIO_ERR_LOG("Unsupported ANI_VERSION_1 %{public}d", ANI_VERSION_1);
        return (ani_status)ANI_ERROR;
    }

    static const char *staticNsName = "L@ohos/multimedia/audio/audio;";
    ani_namespace staticNs;
    if (ANI_OK != env->FindNamespace(staticNsName, &staticNs)) {
        AUDIO_ERR_LOG("Not found %{public}s", staticNsName);
        return ANI_ERROR;
    }

    std::array staticMethods = {
        ani_native_function {"getAudioManager", nullptr,
            reinterpret_cast<void *>(AniAudioManager::Constructor)},
    };

    if (ANI_OK != env->Namespace_BindNativeFunctions(staticNs, staticMethods.data(), staticMethods.size())) {
        AUDIO_ERR_LOG("Cannot bind native methods to %{public}s", staticNsName);
        return ANI_ERROR;
    };

    AniAudioManager::AudioManagerAniInit(env);
    AudioVolumeManagerAni::AudioVolumeManagerAniInit(env);

    *result = ANI_VERSION_1;
    return ANI_OK;
}
