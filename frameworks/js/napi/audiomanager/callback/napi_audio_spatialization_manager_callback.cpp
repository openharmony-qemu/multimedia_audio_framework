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
#define LOG_TAG "NapiAudioSpatializationMgrCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_spatialization_manager_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
bool NapiAudioSpatializationEnabledChangeCallback::onSpatializationEnabledChangeFlag_;
bool NapiAudioHeadTrackingEnabledChangeCallback::onHeadTrackingEnabledChangeFlag_;
bool NapiAdaptiveSpatialRenderingEnabledChangeCallback::onAdaptiveSpatialRenderingEnabledChangeFlag_;
using namespace std;
NapiAudioSpatializationEnabledChangeCallback::NapiAudioSpatializationEnabledChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSpatializationEnabledChangeCallback: instance create");
}

NapiAudioSpatializationEnabledChangeCallback::~NapiAudioSpatializationEnabledChangeCallback()
{
    if (regAmSpatEnable_) {
        napi_release_threadsafe_function(amSpatEnableTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioSpatializationEnabledChangeCallback: instance destroy");
}

void NapiAudioSpatializationEnabledChangeCallback::CreateSpatEnableTsfn(napi_env env)
{
    napi_value cbName;
    regAmSpatEnable_ = true;
    std::string callbackName = "volumeChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env_, nullptr, nullptr, cbName, 0, 1, nullptr,
        SpatializationEnabledTsfnFinalize, nullptr, SafeJsCallbackSpatializationEnabledWork,
        &amSpatEnableTsfn_);
}

bool NapiAudioSpatializationEnabledChangeCallback::GetSpatEnableTsfnFlag()
{
    return regAmSpatEnable_;
}

void NapiAudioSpatializationEnabledChangeCallback::SaveSpatializationEnabledChangeCallbackReference(napi_value args,
    const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    std::string taskName = "NapiAudioSpatializationEnabledChangeCallback::destroy";

    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioSpatializationEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioSpatializationEnabledChangeCallback: creating callback failed");
        spatializationEnabledChangeCbList_.push_back(cb);
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            it != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioSpatializationEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioSpatializationEnabledChangeCallback: creating callback failed");
        spatializationEnabledChangeCbForAnyDeviceList_.push_back(cb);
    }
}

void NapiAudioSpatializationEnabledChangeCallback::RemoveSpatializationEnabledChangeCallbackReference(napi_env env,
    napi_value args, const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                spatializationEnabledChangeCbList_.erase(it);
                return;
            }
        }
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            it != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                spatializationEnabledChangeCbForAnyDeviceList_.erase(it);
                return;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: js callback no find");
}

void NapiAudioSpatializationEnabledChangeCallback::RemoveAllSpatializationEnabledChangeCallbackReference(
    const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        spatializationEnabledChangeCbList_.clear();
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            it != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        spatializationEnabledChangeCbForAnyDeviceList_.clear();
    }
    AUDIO_INFO_LOG("RemoveAllSpatializationEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiAudioSpatializationEnabledChangeCallback::GetSpatializationEnabledChangeCbListSize(
    const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ((!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) ? spatializationEnabledChangeCbList_.size():
        spatializationEnabledChangeCbForAnyDeviceList_.size());
}

void NapiAudioSpatializationEnabledChangeCallback::OnSpatializationEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = spatializationEnabledChangeCbList_.begin(); it != spatializationEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = true;
        OnJsCallbackSpatializationEnabled(cb);
    }
    return;
}

void NapiAudioSpatializationEnabledChangeCallback::OnSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    AUDIO_INFO_LOG("OnSpatializationEnabledChange by the speified device entered");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = spatializationEnabledChangeCbList_.begin(); it != spatializationEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = true;
        OnJsCallbackSpatializationEnabled(cb);
    }
    for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
        it != spatializationEnabledChangeCbForAnyDeviceList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->deviceDescriptor = deviceDescriptor;
        cb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = false;
        OnJsCallbackSpatializationEnabled(cb);
    }

    return;
}

void NapiAudioSpatializationEnabledChangeCallback::SafeJsCallbackSpatializationEnabledWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSpatializationEnabledJsCallback *event = reinterpret_cast<AudioSpatializationEnabledJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSpatializationEnabled: no memory");
    std::shared_ptr<AudioSpatializationEnabledJsCallback> safeContext(
        static_cast<AudioSpatializationEnabledJsCallback*>(data),
        [](AudioSpatializationEnabledJsCallback *ptr) {
            delete ptr;
            ptr = nullptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackSpatializationEnabledWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        if (onSpatializationEnabledChangeFlag_) {
            NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        } else {
            AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            NapiParamUtils::SetAudioSpatialEnabledStateForDevice(env,
                audioSpatialEnabledStateForDevice, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        }

        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call spatialization enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSpatializationEnabledChangeCallback::SpatializationEnabledTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("SpatializationEnabledTsfnFinalize: safe thread resource release.");
}

void NapiAudioSpatializationEnabledChangeCallback::OnJsCallbackSpatializationEnabled(
    std::unique_ptr<AudioSpatializationEnabledJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSpatializationEnabled: jsCb.get() is null");
        return;
    }

    AudioSpatializationEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    event->callbackName = "AudioSpatializationEnabled";

    napi_acquire_threadsafe_function(amSpatEnableTsfn_);
    napi_call_threadsafe_function(amSpatEnableTsfn_, event, napi_tsfn_blocking);
}

NapiAudioCurrentSpatializationEnabledChangeCallback::NapiAudioCurrentSpatializationEnabledChangeCallback(
    napi_env env) : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioCurrentSpatializationEnabledChangeCallback: instance create");
}

NapiAudioCurrentSpatializationEnabledChangeCallback::~NapiAudioCurrentSpatializationEnabledChangeCallback()
{
    if (regAmSpatEnableForCurrentDevice_) {
        napi_release_threadsafe_function(amSpatEnableForCurrentDeviceTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioCurrentSpatializationEnabledChangeCallback: instance destroy");
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::SaveCurrentSpatializationEnabledChangeCallbackReference(
    napi_value args, const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    std::string taskName = "NapiAudioCurrentSpatializationEnabledChangeCallback::destroy";
    const int32_t refCount = ARGS_ONE;
    for (auto it = spatializationEnabledChangeCbForCurrentDeviceList_.begin();
        it != spatializationEnabledChangeCbForCurrentDeviceList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioCurrentSpatializationEnabledChangeCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioCurrentSpatializationEnabledChangeCallback:creating callback failed");

    spatializationEnabledChangeCbForCurrentDeviceList_.push_back(cb);
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::CreateCurrentSpatEnableForCurrentDeviceTsfn(napi_env env)
{
    regAmSpatEnableForCurrentDevice_ = true;
    napi_value cbName;
    std::string callbackName = "AudioSpatializationEnabledForCurrentDevice";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        SpatializationEnabledForCurrentDeviceTsfnFinalize, nullptr,
        SafeJsCallbackSpatializationEnabledForCurrentDeviceWork, &amSpatEnableForCurrentDeviceTsfn_);
}

bool NapiAudioCurrentSpatializationEnabledChangeCallback::GetCurrentSpatEnableForCurrentDeviceTsfnFlag()
{
    return regAmSpatEnableForCurrentDevice_;
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::RemoveCurrentSpatializationEnabledChangeCallbackReference(
    napi_env env, napi_value args, const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = spatializationEnabledChangeCbForCurrentDeviceList_.begin();
        it != spatializationEnabledChangeCbForCurrentDeviceList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveCurrentSpatializationEnabledChangeCallbackReference: find js callback,"
                "erase it");
            spatializationEnabledChangeCbForCurrentDeviceList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCurrentSpatializationEnabledChangeCallbackReference: js callback no find");
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::RemoveAllCurrentSpatializationEnabledChangeCallbackReference(
    const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    spatializationEnabledChangeCbForCurrentDeviceList_.clear();

    AUDIO_INFO_LOG("RemoveAllCurrentSpatializationEnabledChangeCallbackReference: remove all js callbacks"
        "success");
}

int32_t NapiAudioCurrentSpatializationEnabledChangeCallback::GetCurrentSpatializationEnabledChangeCbListSize(
    const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return spatializationEnabledChangeCbForCurrentDeviceList_.size();
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::OnSpatializationEnabledChangeForCurrentDevice(
    const bool &enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = spatializationEnabledChangeCbForCurrentDeviceList_.begin();
        it != spatializationEnabledChangeCbForCurrentDeviceList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledForCurrentDeviceJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        OnJsCallbackSpatializationEnabledForCurrentDevice(cb);
    }

    return;
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::SafeJsCallbackSpatializationEnabledForCurrentDeviceWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSpatializationEnabledForCurrentDeviceJsCallback *event =
        reinterpret_cast<AudioSpatializationEnabledForCurrentDeviceJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSpatializationEnabledForCurrentDevice: no memory");
    std::shared_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> safeContext(
        static_cast<AudioSpatializationEnabledForCurrentDeviceJsCallback*>(data),
        [](AudioSpatializationEnabledForCurrentDeviceJsCallback *ptr) {
            delete ptr;
            ptr = nullptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackSpatializationEnabledForCurrentDeviceWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call head tracking enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::SpatializationEnabledForCurrentDeviceTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("SpatializationEnabledForCurrentDeviceTsfnFinalize: safe thread resource release.");
}

void NapiAudioCurrentSpatializationEnabledChangeCallback::OnJsCallbackSpatializationEnabledForCurrentDevice(
    std::unique_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSpatializationEnabledForCurrentDevice: jsCb.get() is null");
        return;
    }

    AudioSpatializationEnabledForCurrentDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amSpatEnableForCurrentDeviceTsfn_);
    napi_call_threadsafe_function(amSpatEnableForCurrentDeviceTsfn_, event, napi_tsfn_blocking);
}

NapiAudioHeadTrackingEnabledChangeCallback::NapiAudioHeadTrackingEnabledChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioHeadTrackingEnabledChangeCallback: instance create");
}

NapiAudioHeadTrackingEnabledChangeCallback::~NapiAudioHeadTrackingEnabledChangeCallback()
{
    if (regAmHeadTrkTsfn_) {
        napi_release_threadsafe_function(amHeadTrkTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioHeadTrackingEnabledChangeCallback: instance destroy");
}

void NapiAudioHeadTrackingEnabledChangeCallback::SaveHeadTrackingEnabledChangeCallbackReference(napi_value args,
    const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    std::string taskName = "NapiAudioHeadTrackingEnabledChangeCallback::destroy";
    if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioHeadTrackingEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioHeadTrackingEnabledChangeCallback: creating callback failed");

        headTrackingEnabledChangeCbList_.push_back(cb);
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioHeadTrackingEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioHeadTrackingEnabledChangeCallback: creating callback failed");

        headTrackingEnabledChangeCbForAnyDeviceList_.push_back(cb);
    }
}

void NapiAudioHeadTrackingEnabledChangeCallback::CreateHeadTrackingTsfn(napi_env env)
{
    regAmHeadTrkTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioHeadTrackingEnabled";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        HeadTrackingEnabledTsfnFinalize, nullptr, SafeJsCallbackHeadTrackingEnabledWork,
        &amHeadTrkTsfn_);
}

bool NapiAudioHeadTrackingEnabledChangeCallback::GetHeadTrackingTsfnFlag()
{
    return regAmHeadTrkTsfn_;
}

void NapiAudioHeadTrackingEnabledChangeCallback::RemoveHeadTrackingEnabledChangeCallbackReference(napi_env env,
    napi_value args, const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                headTrackingEnabledChangeCbList_.erase(it);
                return;
            }
        }
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                headTrackingEnabledChangeCbForAnyDeviceList_.erase(it);
                return;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: js callback no find");
}

void NapiAudioHeadTrackingEnabledChangeCallback::RemoveAllHeadTrackingEnabledChangeCallbackReference(const std::string
    &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        headTrackingEnabledChangeCbList_.clear();
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        headTrackingEnabledChangeCbForAnyDeviceList_.clear();
    }
    AUDIO_INFO_LOG("RemoveAllHeadTrackingEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiAudioHeadTrackingEnabledChangeCallback::GetHeadTrackingEnabledChangeCbListSize(const std::string &cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ((!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) ? headTrackingEnabledChangeCbList_.size():
        headTrackingEnabledChangeCbForAnyDeviceList_.size());
}

void NapiAudioHeadTrackingEnabledChangeCallback::OnHeadTrackingEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> cb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = true;
        OnJsCallbackHeadTrackingEnabled(cb);
    }

    return;
}

void NapiAudioHeadTrackingEnabledChangeCallback::OnHeadTrackingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    AUDIO_INFO_LOG("OnHeadTrackingEnabledChange by the specified device entered");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> cb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = true;
        OnJsCallbackHeadTrackingEnabled(cb);
    }
    for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
        it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); it++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> cb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->deviceDescriptor = deviceDescriptor;
        cb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = false;
        OnJsCallbackHeadTrackingEnabled(cb);
    }

    return;
}

void NapiAudioHeadTrackingEnabledChangeCallback::SafeJsCallbackHeadTrackingEnabledWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioHeadTrackingEnabledJsCallback *event = reinterpret_cast<AudioHeadTrackingEnabledJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackHeadTrackingEnabled: no memory");
    std::shared_ptr<AudioHeadTrackingEnabledJsCallback> safeContext(
        static_cast<AudioHeadTrackingEnabledJsCallback*>(data),
        [](AudioHeadTrackingEnabledJsCallback *ptr) {
            delete ptr;
            ptr = nullptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackHeadTrackingEnabledWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        if (onHeadTrackingEnabledChangeFlag_) {
            NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        } else {
            AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            NapiParamUtils::SetAudioSpatialEnabledStateForDevice(env,
                audioSpatialEnabledStateForDevice, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        }
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call head tracking enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioHeadTrackingEnabledChangeCallback::HeadTrackingEnabledTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("HeadTrackingEnabledTsfnFinalize: safe thread resource release.");
}

void NapiAudioHeadTrackingEnabledChangeCallback::OnJsCallbackHeadTrackingEnabled(
    std::unique_ptr<AudioHeadTrackingEnabledJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackHeadTrackingEnabled: jsCb.get() is null");
        return;
    }

    AudioHeadTrackingEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amHeadTrkTsfn_);
    napi_call_threadsafe_function(amHeadTrkTsfn_, event, napi_tsfn_blocking);
}


NapiAdaptiveSpatialRenderingEnabledChangeCallback::NapiAdaptiveSpatialRenderingEnabledChangeCallback(
    napi_env env) : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAdaptiveSpatialRenderingEnabledChangeCallback: instance create");
}

NapiAdaptiveSpatialRenderingEnabledChangeCallback::~NapiAdaptiveSpatialRenderingEnabledChangeCallback()
{
    if (regAmAdaptiveSpatialRenderingTsfn_) {
        napi_release_threadsafe_function(amAdaptiveSpatialRenderingTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAdaptiveSpatialRenderingEnabledChangeCallback: instance destroy");
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::SaveAdaptiveSpatialRenderingEnabledChangeCallbackReference(
    napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    std::string taskName = "NapiAdaptiveSpatialRenderingEnabledChangeCallback::destroy";
    for (auto it = adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.begin();
        it != adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAdaptiveSpatialRenderingEnabledChangeCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(cb != nullptr,
        "NapiAdaptiveSpatialRenderingEnabledChangeCallback: creating callback failed");

    adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.push_back(cb);
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::CreateAdaptiveSpatialRenderingTsfn(napi_env env)
{
    regAmAdaptiveSpatialRenderingTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioAdaptiveSpatialRenderingEnabled";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        AdaptiveSpatialRenderingEnabledTsfnFinalize, nullptr, SafeJsCallbackAdaptiveSpatialRenderingEnabledWork,
        &amAdaptiveSpatialRenderingTsfn_);
}

bool NapiAdaptiveSpatialRenderingEnabledChangeCallback::GetAdaptiveSpatialRenderingTsfnFlag()
{
    return regAmAdaptiveSpatialRenderingTsfn_;
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::RemoveAdaptiveSpatialRenderingEnabledChangeCallbackReference(
    napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.begin();
        it != adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveAdaptiveSpatialRenderingEnabledChangeCallback: find js callback, delete it");
            napi_delete_reference(env, (*it)->cb_);
            (*it)->cb_ = nullptr;
            adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveAdaptiveSpatialRenderingEnabledChangeCallbackReference: js callback no find");
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::RemoveAllAdaptiveSpatialRenderingEnabledChangeCallbackReference(
    )
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.begin();
        it != adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
        napi_delete_reference(env_, (*it)->cb_);
        (*it)->cb_ = nullptr;
    }
    adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.clear();
    AUDIO_INFO_LOG("RemoveAllAdaptiveSpatialRenderingEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiAdaptiveSpatialRenderingEnabledChangeCallback::GetAdaptiveSpatialRenderingEnabledChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.size();
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    AUDIO_INFO_LOG("OnAdaptiveSpatialRenderingEnabledChange by the specified device entered");
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.begin();
        it != adaptiveSpatialRenderingEnabledChangeCbForAnyDeviceList_.end(); it++) {
        std::unique_ptr<AudioAdaptiveSpatialRenderingEnabledJsCallback> cb =
            std::make_unique<AudioAdaptiveSpatialRenderingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->deviceDescriptor = deviceDescriptor;
        cb->enabled = enabled;
        onAdaptiveSpatialRenderingEnabledChangeFlag_ = false;
        OnJsCallbackAdaptiveSpatialRenderingEnabled(cb);
    }

    return;
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::SafeJsCallbackAdaptiveSpatialRenderingEnabledWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioAdaptiveSpatialRenderingEnabledJsCallback *event =
        reinterpret_cast<AudioAdaptiveSpatialRenderingEnabledJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAdaptiveSpatialRenderingEnabled: no memory");
    std::shared_ptr<AudioAdaptiveSpatialRenderingEnabledJsCallback> safeContext(
        static_cast<AudioAdaptiveSpatialRenderingEnabledJsCallback*>(data),
        [](AudioAdaptiveSpatialRenderingEnabledJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAdaptiveSpatialRenderingEnabledWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        if (onAdaptiveSpatialRenderingEnabledChangeFlag_) {
            NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        } else {
            AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            NapiParamUtils::SetAudioSpatialEnabledStateForDevice(env,
                audioSpatialEnabledStateForDevice, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        }
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call adaptive spatial rendering enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::AdaptiveSpatialRenderingEnabledTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AdaptiveSpatialRenderingEnabledTsfnFinalize: safe thread resource release.");
}

void NapiAdaptiveSpatialRenderingEnabledChangeCallback::OnJsCallbackAdaptiveSpatialRenderingEnabled(
    std::unique_ptr<AudioAdaptiveSpatialRenderingEnabledJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAdaptiveSpatialRenderingEnabled: jsCb.get() is null");
        return;
    }

    AudioAdaptiveSpatialRenderingEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amAdaptiveSpatialRenderingTsfn_);
    napi_call_threadsafe_function(amAdaptiveSpatialRenderingTsfn_, event, napi_tsfn_blocking);
}

NapiSpatialAudioSourceTypeCallback::NapiSpatialAudioSourceTypeCallback(
    napi_env env) : env_(env)
{
    AUDIO_DEBUG_LOG("NapiSpatialAudioSourceTypeCallback: instance create");
}

NapiSpatialAudioSourceTypeCallback::~NapiSpatialAudioSourceTypeCallback()
{
    if (regAmSpatialAudioSourceTypeTsfn_) {
        napi_release_threadsafe_function(amSpatialAudioSourceTypeTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiSpatialAudioSourceTypeCallback: instance destroy");
}

void NapiSpatialAudioSourceTypeCallback::SaveSpatialAudioSourceTypeChangeCallbackReference(
    napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;

    std::string taskName = "NapiSpatialAudioSourceTypeCallback::destroy";
    for (auto it = spatialAudioSourceTypeChangeCbList_.begin();
        it != spatialAudioSourceTypeChangeCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiSpatialAudioSourceTypeCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(cb != nullptr,
        "NapiSpatialAudioSourceTypeCallback: creating callback failed");

    spatialAudioSourceTypeChangeCbList_.push_back(cb);
}

void NapiSpatialAudioSourceTypeCallback::CreateSpatialAudioSourceTypeTsfn(napi_env env)
{
    regAmSpatialAudioSourceTypeTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioSourceType";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        SpatialAudioSourceTypeTsfnFinalize, nullptr, SafeJsCallbackSpatialAudioSourceTypeWork,
        &amSpatialAudioSourceTypeTsfn_);
}

bool NapiSpatialAudioSourceTypeCallback::GetCurrentSpatialAudioSourceTypeTsfnFlag()
{
    return regAmSpatialAudioSourceTypeTsfn_;
}

void NapiSpatialAudioSourceTypeCallback::RemoveSpatialAudioSourceTypeChangeCallbackReference(
    napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = spatialAudioSourceTypeChangeCbList_.begin();
        it != spatialAudioSourceTypeChangeCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveSpatialAudioSourceTypeChangeCallback: find js callback, delete it");
            napi_delete_reference(env, (*it)->cb_);
            (*it)->cb_ = nullptr;
            spatialAudioSourceTypeChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveSpatialAudioSourceTypeChangeCallbackReference: js callback no find");
}

void NapiSpatialAudioSourceTypeCallback::RemoveAllSpatialAudioSourceTypeChangeCallbackReference(
    )
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = spatialAudioSourceTypeChangeCbList_.begin();
        it != spatialAudioSourceTypeChangeCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it)->cb_);
        (*it)->cb_ = nullptr;
    }
    spatialAudioSourceTypeChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllSpatialAudioSourceTypeChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiSpatialAudioSourceTypeCallback::GetCurrentSpatialAudioSourceTypeChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return spatialAudioSourceTypeChangeCbList_.size();
}

void NapiSpatialAudioSourceTypeCallback::OnSpatialAudioSourceTypeChange(const SpatialAudioSourceType &mode)
{
    AUDIO_INFO_LOG("OnSpatialAudioSourceTypeChange by the specified device entered");
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = spatialAudioSourceTypeChangeCbList_.begin();
        it != spatialAudioSourceTypeChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSpatialAudioSourceTypeJsCallback> cb =
            std::make_unique<AudioSpatialAudioSourceTypeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->mode = mode;
        OnJsCallbackSpatialAudioSourceType(cb);
    }

    return;
}

void NapiSpatialAudioSourceTypeCallback::SafeJsCallbackSpatialAudioSourceTypeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSpatialAudioSourceTypeJsCallback *event =
        reinterpret_cast<AudioSpatialAudioSourceTypeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSpatialAudioSourceType: no memory");
    std::shared_ptr<AudioSpatialAudioSourceTypeJsCallback> safeContext(
        static_cast<AudioSpatialAudioSourceTypeJsCallback*>(data),
        [](AudioSpatialAudioSourceTypeJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackSpatialAudioSourceTypeWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        NapiParamUtils::SetValueInt32(env, event->mode, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call effect mode change callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiSpatialAudioSourceTypeCallback::SpatialAudioSourceTypeTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AdaptiveSpatialRenderingEnabledTsfnFinalize: safe thread resource release.");
}

void NapiSpatialAudioSourceTypeCallback::OnJsCallbackSpatialAudioSourceType(
    std::unique_ptr<AudioSpatialAudioSourceTypeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSpatialAudioSourceType: jsCb.get() is null");
        return;
    }

    AudioSpatialAudioSourceTypeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amSpatialAudioSourceTypeTsfn_);
    napi_call_threadsafe_function(amSpatialAudioSourceTypeTsfn_, event, napi_tsfn_blocking);
}

NapiPersonalizedSpatializationEnabledChangeCallback::NapiPersonalizedSpatializationEnabledChangeCallback(
    napi_env env) : env_(env)
{
    AUDIO_DEBUG_LOG("NapiPersonalizedSpatializationEnabledChangeCallback: instance create");
}

NapiPersonalizedSpatializationEnabledChangeCallback::~NapiPersonalizedSpatializationEnabledChangeCallback()
{
    if (regAmPersonalizedSpatializationTsfn_) {
        napi_release_threadsafe_function(amPersonalizedSpatializationTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiPersonalizedSpatializationEnabledChangeCallback: instance destroy");
}

void NapiPersonalizedSpatializationEnabledChangeCallback::SavePersonalizedSpatializationEnabledChangeCallbackReference(
    napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;

    std::string taskName = "NapiPersonalizedSpatializationEnabledChangeCallback::destroy";
    for (auto it = personalizedSpatializationEnabledChangeCbForAnyDeviceList_.begin();
        it != personalizedSpatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiPersonalizedSpatializationEnabledChangeCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(cb != nullptr,
        "NapiPersonalizedSpatializationEnabledChangeCallback: creating callback failed");

    personalizedSpatializationEnabledChangeCbForAnyDeviceList_.push_back(cb);
}

void NapiPersonalizedSpatializationEnabledChangeCallback::CreatePersonalizedSpatializationTsfn(napi_env env)
{
    regAmPersonalizedSpatializationTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioPersonalizedSpatializationEnabled";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        PersonalizedSpatializationEnabledTsfnFinalize, nullptr, SafeJsCallbackPersonalizedSpatializationEnabledWork,
        &amPersonalizedSpatializationTsfn_);
}

bool NapiPersonalizedSpatializationEnabledChangeCallback::GetPersonalizedSpatializationTsfnFlag()
{
    return regAmPersonalizedSpatializationTsfn_;
}

void NapiPersonalizedSpatializationEnabledChangeCallback::RemovePersonalizedSpatialEnabledChangeCallbackReference(
    napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = personalizedSpatializationEnabledChangeCbForAnyDeviceList_.begin();
        it != personalizedSpatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemovePersonalizedSpatializationEnabledChangeCallback: find js callback, delete it");
            napi_delete_reference(env, (*it)->cb_);
            (*it)->cb_ = nullptr;
            personalizedSpatializationEnabledChangeCbForAnyDeviceList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemovePersonalizedSpatialEnabledChangeCallbackReference: js callback no find");
}

void NapiPersonalizedSpatializationEnabledChangeCallback::RemoveAllPersonalizedSpatialEnabledChangeCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = personalizedSpatializationEnabledChangeCbForAnyDeviceList_.begin();
        it != personalizedSpatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
        napi_delete_reference(env_, (*it)->cb_);
        (*it)->cb_ = nullptr;
    }
    personalizedSpatializationEnabledChangeCbForAnyDeviceList_.clear();
    AUDIO_INFO_LOG("RemoveAllPersonalizedSpatialEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiPersonalizedSpatializationEnabledChangeCallback::GetPersonalizedSpatializationEnabledChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return personalizedSpatializationEnabledChangeCbForAnyDeviceList_.size();
}

void NapiPersonalizedSpatializationEnabledChangeCallback::OnPersonalizedSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    AUDIO_INFO_LOG("OnPersonalizedSpatializationEnabledChange by the specified device entered");
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = personalizedSpatializationEnabledChangeCbForAnyDeviceList_.begin();
        it != personalizedSpatializationEnabledChangeCbForAnyDeviceList_.end(); it++) {
        std::unique_ptr<AudioPersonalizedSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioPersonalizedSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->deviceDescriptor = deviceDescriptor;
        cb->enabled = enabled;
        OnJsCallbackPersonalizedSpatializationEnabled(cb);
    }

    return;
}

void NapiPersonalizedSpatializationEnabledChangeCallback::SafeJsCallbackPersonalizedSpatializationEnabledWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioPersonalizedSpatializationEnabledJsCallback *event =
        reinterpret_cast<AudioPersonalizedSpatializationEnabledJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackPersonalizedSpatializationEnabled: no memory");
    std::shared_ptr<AudioPersonalizedSpatializationEnabledJsCallback> safeContext(
        static_cast<AudioPersonalizedSpatializationEnabledJsCallback*>(data),
        [](AudioPersonalizedSpatializationEnabledJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackPersonalizedSpatializationEnabledWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        AudioPersonalizedSpatialEnabledChangeForAnyDevice audioPersonalizedSpatialEnabledChangeForAnyDevice;
        audioPersonalizedSpatialEnabledChangeForAnyDevice.deviceDescriptor = event->deviceDescriptor;
        audioPersonalizedSpatialEnabledChangeForAnyDevice.enabled = event->enabled;
        NapiParamUtils::SetPersonalizedSpatialEnabledStateForDevice(env,
            audioPersonalizedSpatialEnabledChangeForAnyDevice, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call personalized spatial rendering enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiPersonalizedSpatializationEnabledChangeCallback::PersonalizedSpatializationEnabledTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("PersonalizedSpatializationEnabledTsfnFinalize: safe thread resource release.");
}

void NapiPersonalizedSpatializationEnabledChangeCallback::OnJsCallbackPersonalizedSpatializationEnabled(
    std::unique_ptr<AudioPersonalizedSpatializationEnabledJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackPersonalizedSpatializationEnabled: jsCb.get() is null");
        return;
    }

    AudioPersonalizedSpatializationEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amPersonalizedSpatializationTsfn_);
    napi_call_threadsafe_function(amPersonalizedSpatializationTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS