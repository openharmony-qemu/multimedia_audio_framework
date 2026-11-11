/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "TaiheAudioSuiteDownloadCallback"
#endif

#include "taihe_audio_suite_download_callback.h"
#include "taihe_param_utils.h"
#include "audio_log.h"

namespace ANI::Audio {

static int32_t FeatureNameToType(const std::string &featureName)
{
    using FeatureType = AudioSuiteFeatureType::key_t;
    static const std::map<std::string, FeatureType> featureNameToTypeMap = {
        {"RecordDialogAlgoRes", FeatureType::AUDIO_DIALOG_ENHANCEMENT},
        {"RecordPicAlgoRes", FeatureType::AUDIO_CENTER_VOICE_ENHANCEMENT},
        {"RecordDefinition", FeatureType::AUDIO_VOICE_ENHANCEMENT},
        {"MusicMasterAlgoRes", FeatureType::AUDIO_MUSIC_ENHANCEMENT},
        {"AudioSourceSeparationAlgoRes", FeatureType::AUDIO_SOURCE_SEPARATION},
    };
    auto it = featureNameToTypeMap.find(featureName);
    return (it != featureNameToTypeMap.end()) ? static_cast<int32_t>(it->second) : -1;
}

TaiheAudioSuiteDownloadCallback::TaiheAudioSuiteDownloadCallback()
{
    AUDIO_INFO_LOG("Constructor");
}

TaiheAudioSuiteDownloadCallback::~TaiheAudioSuiteDownloadCallback()
{
    AUDIO_INFO_LOG("Destructor");
    RemoveAllCallbackReference();
}

void TaiheAudioSuiteDownloadCallback::SaveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "callback is nullptr");
    for (auto &item : jsCallbacks_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, item->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "has same callback, nothing to do");
    }
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    jsCallbacks_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size [%{public}zu]", jsCallbacks_.size());

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
}

void TaiheAudioSuiteDownloadCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = jsCallbacks_.begin(); it != jsCallbacks_.end(); ++it) {
        std::shared_ptr<AutoRef> temp = (*it);
        if (temp == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, temp->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("find download status callback, remove it");
            temp->cb_ = nullptr;
            jsCallbacks_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("remove nothing");
}

void TaiheAudioSuiteDownloadCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : jsCallbacks_) {
        if (item == nullptr) {
            continue;
        }
        item->cb_ = nullptr;
    }
    jsCallbacks_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

bool TaiheAudioSuiteDownloadCallback::IsEmpty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return jsCallbacks_.empty();
}

bool TaiheAudioSuiteDownloadCallback::ContainSameCallback(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : jsCallbacks_) {
        if (item == nullptr) {
            continue;
        }
        if (TaiheParamUtils::IsSameRef(callback, item->cb_)) {
            return true;
        }
    }
    return false;
}

void TaiheAudioSuiteDownloadCallback::OnDownloadStatusChanged(
    const std::vector<OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadStatusInfo> &status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : jsCallbacks_) {
        if (item == nullptr) {
            continue;
        }
        std::unique_ptr<DownloadStatusJsCallback> jsCb = std::make_unique<DownloadStatusJsCallback>();
        CHECK_AND_RETURN_LOG(jsCb != nullptr, "No memory");
        jsCb->callback = item;
        jsCb->callbackName = DOWNLOAD_STATUS_CHANGE_CALLBACK_NAME;
        jsCb->statusInfo = status;
        OnJsCallbackDownloadStatus(jsCb);
    }
}

void TaiheAudioSuiteDownloadCallback::OnJsCallbackDownloadStatus(
    std::unique_ptr<DownloadStatusJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackDownloadStatus: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    DownloadStatusJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackDownloadStatusWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnDownloadStatusChanged", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioSuiteDownloadCallback::SafeJsCallbackDownloadStatusWork(
    DownloadStatusJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackDownloadStatus: no memory");
    std::shared_ptr<DownloadStatusJsCallback> safeContext(
        static_cast<DownloadStatusJsCallback*>(event),
        [](DownloadStatusJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
        });
    std::shared_ptr<taihe::callback<void(array_view<AudioSuiteFeatureStatusInfo> data)>> cacheCallback =
        std::reinterpret_pointer_cast<taihe::callback<void(array_view<AudioSuiteFeatureStatusInfo> data)>>(
            event->callback->cb_);
    CHECK_AND_RETURN_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", event->callbackName.c_str());

    std::vector<AudioSuiteFeatureStatusInfo> result;
    for (const auto &info : event->statusInfo) {
        if (info.featureName != "") {
            AudioSuiteFeatureStatusInfo taiheInfo{
                AudioSuiteFeatureType::from_value(FeatureNameToType(info.featureName)),
                AudioSuiteFeatureStatus::from_value(info.state),
                TaiheParamUtils::ToTaiheString(info.romPath),
                info.progress,
                info.errorCode
            };
            result.push_back(taiheInfo);
        }
    }
    (*cacheCallback)(array_view<AudioSuiteFeatureStatusInfo>(result.data(), result.size()));
}
} // namespace ANI::Audio
