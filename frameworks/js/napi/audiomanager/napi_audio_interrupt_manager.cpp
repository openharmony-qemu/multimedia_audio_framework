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
#ifndef LOG_TAG
#define LOG_TAG "NapiAudioInterruptManager"
#endif

#include "napi_audio_interrupt_manager.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
static __thread napi_ref g_interruptManagerConstructor = nullptr;

NapiAudioInterruptManager::NapiAudioInterruptManager()
    : audioSystemMngr_(nullptr), env_(nullptr) {}

NapiAudioInterruptManager::~NapiAudioInterruptManager() = default;

void NapiAudioInterruptManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioInterruptManager *>(nativeObject);
        ObjectRefMap<NapiAudioInterruptManager>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioInterruptManager::Construct(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("Construct");
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    unique_ptr<NapiAudioInterruptManager> napiAudioInterruptManager = make_unique<NapiAudioInterruptManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioInterruptManager != nullptr, result, "No memory");

    napiAudioInterruptManager->audioSystemMngr_ = AudioSystemManager::GetInstance();

    napiAudioInterruptManager->env_ = env;
    ObjectRefMap<NapiAudioInterruptManager>::Insert(napiAudioInterruptManager.get());

    status = napi_wrap(env, thisVar, static_cast<void*>(napiAudioInterruptManager.get()),
        NapiAudioInterruptManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioInterruptManager>::Erase(napiAudioInterruptManager.get());
        return result;
    }
    napiAudioInterruptManager.release();
    return thisVar;
}

napi_value NapiAudioInterruptManager::CreateInterruptManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_interruptManagerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateInterruptManagerWrapper, %{public}d", status);
        goto fail;
    }
    status = napi_new_instance(env, constructor, PARAM0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed, status:%{public}d", status);
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioInterruptManager::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("Init");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_interrupt_manager_properties[] = {};

    status = napi_define_class(env, AUDIO_INTERRUPT_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr, sizeof(audio_interrupt_manager_properties) / sizeof(audio_interrupt_manager_properties[PARAM0]),
        audio_interrupt_manager_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");
    status = napi_create_reference(env, constructor, refCount, &g_interruptManagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_INTERRUPT_MANAGER_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}
} // namespace AudioStandard
} // namespace OHOS