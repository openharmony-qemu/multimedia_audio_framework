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
#define LOG_TAG "AniAudioManager"
#endif

#include "ani_audio_manager.h"
#include "audio_common_log.h"
#include "ani_audio_error.h"
#include "ani_class_name.h"
#include "ani_param_utils.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {

const std::string INTERRUPT_CALLBACK_NAME = "interrupt";
const std::string RINGERMODE_CALLBACK_NAME = "ringerModeChange";
const std::string DEVICE_CHANGE_CALLBACK_NAME = "deviceChange";

AniAudioManager::AniAudioManager()
    : audioMngr_(nullptr), env_(nullptr) {
}

AniAudioManager::~AniAudioManager()
{
    AUDIO_DEBUG_LOG("Instance destroy");
}

ani_status AniAudioManager::AudioManagerAniInit(ani_env *env)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");
    static const char *className = ANI_CLASS_AUDIO_MANAGER_HANDLE.c_str();
    ani_class cls;
    ani_status status = env->FindClass(className, &cls);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Failed to find class: %{public}s", className);
        return status;
    }

    std::array methods = {
        ani_native_function {"getVolumeManager", nullptr,
            reinterpret_cast<void *>(AniAudioManager::GetVolumeManager) },
        ani_native_function {"onInner", nullptr,
            reinterpret_cast<void *>(AniAudioManager::On) }
    };

    status = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Failed to bind native methods to: %{public}s", className);
        return status;
    }
    return ANI_OK;
}

ani_object AniAudioManager::GetVolumeManager(ani_env *env, ani_object object)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, nullptr, "Invalid env");
    return AudioVolumeManagerAni::CreateVolumeManagerWrapper(env);
}

ani_object AniAudioManager::Constructor([[maybe_unused]] ani_env *env)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, nullptr, "Invalid env");
    ani_object result = nullptr;
    std::unique_ptr<AniAudioManager> audioManagerAni = std::make_unique<AniAudioManager>();
    if (audioManagerAni != nullptr) {
        audioManagerAni->env_ = env;
        audioManagerAni->audioMngr_ = AudioSystemManager::GetInstance();
        audioManagerAni->cachedClientId_ = getpid();
    }

    static const char *className = ANI_CLASS_AUDIO_MANAGER_HANDLE.c_str();
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        AUDIO_ERR_LOG("Failed to find class: %{public}s", className);
        return result;
    }

    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "J:V", &ctor)) {
        AUDIO_ERR_LOG("Failed to find method: %{public}s", "ctor");
        return result;
    }

    if (ANI_OK !=env->Object_New(cls, ctor, &result, reinterpret_cast<ani_long>(audioManagerAni.release()))) {
        AUDIO_ERR_LOG("New PhotoAccessHelper Fail");
    }
    return result;
}

AniAudioManager* AniAudioManager::Unwrap(ani_env *env, ani_object object)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, nullptr, "Invalid env");
    ani_long audioManager;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeAudioManagerContext", &audioManager)) {
        return nullptr;
    }
    return reinterpret_cast<AniAudioManager*>(audioManager);
}

void AniAudioManager::RegisterCallback(ani_env *env, ani_object jsThis,
    const std::string &cbName, ani_object info, ani_object callback)
{
    AniAudioManager* audioManagerAni =  AniAudioManager::Unwrap(env, jsThis);
    CHECK_AND_RETURN_RET_LOG(audioManagerAni != nullptr,
        AniAudioError::ThrowError(env, ANI_ERR_NO_MEMORY), "audioManagerAni is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioManagerAni->audioMngr_ != nullptr,
        AniAudioError::ThrowError(env, ANI_ERR_NO_MEMORY), "audioMngr_ is nullptr");

    if (!cbName.compare(INTERRUPT_CALLBACK_NAME)) {
        RegisterInterruptCallback(env, info, callback, audioManagerAni);
    } else if (!cbName.compare(RINGERMODE_CALLBACK_NAME)) {
        RegisterRingerModeCallback(env, callback, audioManagerAni);
    } else if (!cbName.compare(VOLUME_CHANGE_CALLBACK_NAME)) {
        RegisterVolumeChangeCallback(env, callback, audioManagerAni);
    } else if (!cbName.compare(DEVICE_CHANGE_CALLBACK_NAME)) {
        RegisterDeviceChangeCallback(env, callback, audioManagerAni);
    }
}

void AniAudioManager::On(ani_env *env, ani_object object, ani_string type, ani_object callback, ani_object info)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AniAudioError::ThrowError(env, ANI_ERR_INPUT_INVALID),
        "incorrect parameter types: The type of callback must be function");
    std::string callbackName = AniParamUtils::GetStringArgument(env, type);
    return RegisterCallback(env, object, callbackName, info, callback);
}

void AniAudioManager::RegisterVolumeChangeCallback(ani_env *env, ani_object callback, AniAudioManager *audioManagerAni)
{
    CHECK_AND_RETURN_RET_LOG(audioManagerAni != nullptr,
        AniAudioError::ThrowError(env, ANI_ERR_NO_MEMORY), "audioManagerAni is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioManagerAni->volumeKeyEventCallbackAni_ != nullptr,
        AniAudioError::ThrowError(env, ANI_ERR_NO_MEMORY), "volumeKeyEventCallbackAni_ is nullptr");
    if (audioManagerAni->volumeKeyEventCallbackAni_ == nullptr) {
        audioManagerAni->volumeKeyEventCallbackAni_ = std::make_shared<AniAudioVolumeKeyEvent>(env);
        int32_t ret = audioManagerAni->audioMngr_->RegisterVolumeKeyEventCallback(audioManagerAni->cachedClientId_,
            audioManagerAni->volumeKeyEventCallbackAni_, API_8);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, AniAudioError::ThrowError(env, ret),
            "RegisterVolumeKeyEventCallback Failed %{public}d", ret);
    }
    std::shared_ptr<AniAudioVolumeKeyEvent> cb =
        std::static_pointer_cast<AniAudioVolumeKeyEvent>(audioManagerAni->volumeKeyEventCallbackAni_);
    CHECK_AND_RETURN_RET_LOG(cb != nullptr,
        AniAudioError::ThrowError(env, ANI_ERR_NO_MEMORY), "cb is nullptr");
    cb->SaveCallbackReference(VOLUME_CHANGE_CALLBACK_NAME, callback);
    if (!cb->GetVolumeTsfnFlag()) {
        cb->CreateVolumeTsfn(env);
    }
}

} // namespace AudioStandard
} // namespace OHOS
