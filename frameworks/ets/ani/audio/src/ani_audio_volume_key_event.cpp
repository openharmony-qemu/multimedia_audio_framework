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
#define LOG_TAG "AniAudioVolumeKeyEvent"
#endif

#include "ani_audio_volume_key_event.h"
#include <mutex>
#include <thread>
#include "ani_class_name.h"
#include "ani_param_utils.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
mutex AniAudioVolumeKeyEvent::sWorkerMutex_;

AniAudioVolumeKeyEvent::AniAudioVolumeKeyEvent(ani_env *env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("AniAudioVolumeKeyEvent::Constructor");
}

AniAudioVolumeKeyEvent::~AniAudioVolumeKeyEvent()
{
    AUDIO_DEBUG_LOG("AniAudioVolumeKeyEvent::Destructor");
}

void AniAudioVolumeKeyEvent::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnVolumeKeyEvent is called volumeType=%{public}d, volumeLevel=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.volumeType, volumeEvent.volume, volumeEvent.updateUi);
    CHECK_AND_RETURN_LOG(callback_ != nullptr,
        "AniAudioVolumeKeyEvent:No JS callback registered return");
    std::unique_ptr<AudioVolumeKeyEventJsCallback> cb = std::make_unique<AudioVolumeKeyEventJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = callback_;
    cb->callbackName = VOLUME_KEY_EVENT_CALLBACK_NAME;
    cb->volumeEvent.volumeType = volumeEvent.volumeType;
    cb->volumeEvent.volume = volumeEvent.volume;
    cb->volumeEvent.updateUi = volumeEvent.updateUi;
    cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
    cb->volumeEvent.networkId = volumeEvent.networkId;

    return OnJsCallbackVolumeEvent(cb);
}

void AniAudioVolumeKeyEvent::SaveCallbackReference(const std::string &callbackName, ani_object object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(object != nullptr, "AniAudioVolumeKeyEvent: creating reference for callback fail");
    ani_ref callback = static_cast<ani_ref>(object);
    CHECK_AND_RETURN_LOG(env_ != nullptr, "Invalid env");
    env_->GlobalReference_Create(callback, &callback_);
    if (callbackName != VOLUME_KEY_EVENT_CALLBACK_NAME) {
        AUDIO_ERR_LOG("AniAudioVolumeKeyEvent: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

bool AniAudioVolumeKeyEvent::GetVolumeTsfnFlag()
{
    return regVolumeTsfn_;
}

void AniAudioVolumeKeyEvent::CreateVolumeTsfn(ani_env *env)
{
    regVolumeTsfn_ = true;
}

void AniAudioVolumeKeyEvent::SafeJsCallbackVolumeEventWork(ani_env *env, AudioVolumeKeyEventJsCallback *event)
{
    CHECK_AND_RETURN_LOG(env != nullptr, "Invalid env");
    lock_guard<mutex> lock(sWorkerMutex_);
    ani_vm *etsVm;
    ani_env *etsEnv;
    CHECK_AND_RETURN_LOG(env->GetVM(&etsVm) == ANI_OK, "Get etsVm fail");
    ani_option interopEnabled {"--interop=disable", nullptr};
    ani_options aniArgs {1, &interopEnabled};
    CHECK_AND_RETURN_LOG(etsVm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv) == ANI_OK,
        "AttachCurrentThread fail");
    CHECK_AND_RETURN_LOG(etsEnv != nullptr, "SafeJsCallbackVolumeEventWork: etsEnv is nullptr");
    CHECK_AND_RETURN_LOG(event != nullptr, "SafeJsCallbackVolumeEventWork: event is nullptr");
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackVolumeEventWork: no memory");
    std::shared_ptr<AudioVolumeKeyEventJsCallback> safeContext(
        static_cast<AudioVolumeKeyEventJsCallback*>(event),
        [](AudioVolumeKeyEventJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;

    do {
        ani_status status;
        ani_object result = nullptr;
        status = AniParamUtils::SetValueVolumeEvent(etsEnv, event->volumeEvent, result);
        CHECK_AND_BREAK_LOG(status == ANI_OK && result != nullptr,
            "%{public}s fail to create volumeChange callback", request.c_str());
        auto fnObject = reinterpret_cast<ani_fn_object>(event->callback);
        CHECK_AND_RETURN_LOG(fnObject != nullptr, "SafeJsCallbackVolumeEventWork: fnObject is null");
        std::vector<ani_ref> args = {reinterpret_cast<ani_ref>(result)};
        ani_ref fnObjectResult;
        CHECK_AND_RETURN_LOG(ANI_OK == etsEnv->FunctionalObject_Call(
            fnObject, args.size(), args.data(), &fnObjectResult), "FunctionalObject_Call fail");
        CHECK_AND_RETURN_LOG(etsVm->DetachCurrentThread() == ANI_OK, "DetachCurrentThread fail");
    } while (0);
}

void AniAudioVolumeKeyEvent::OnJsCallbackVolumeEvent(std::unique_ptr<AudioVolumeKeyEventJsCallback> &jsCb)
{
    CHECK_AND_RETURN_LOG(jsCb.get() != nullptr, "OnJsCallbackVolumeEvent: jsCb.get() is null");

    AudioVolumeKeyEventJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    std::thread worker(SafeJsCallbackVolumeEventWork, env_, event);
    worker.join();
}
} // namespace AudioStandard
} // namespace OHOS
