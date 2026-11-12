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

#ifndef TAIHE_AUDIO_SUITE_DOWNLOAD_CALLBACK_H
#define TAIHE_AUDIO_SUITE_DOWNLOAD_CALLBACK_H

#include "event_handler.h"
#include "audio_suite_download_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

inline const std::string DOWNLOAD_STATUS_CHANGE_CALLBACK_NAME = "downloadStatusChange";

class TaiheAudioSuiteDownloadCallback
    : public OHOS::AudioStandard::AudioSuite::DownloadStatusCallback,
      public std::enable_shared_from_this<TaiheAudioSuiteDownloadCallback> {
public:
    explicit TaiheAudioSuiteDownloadCallback();
    ~TaiheAudioSuiteDownloadCallback() override;

    void OnDownloadStatusChanged(
        const std::vector<OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadStatusInfo> &status) override;

    void SaveCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveAllCallbackReference();
    bool IsEmpty();
    bool ContainSameCallback(std::shared_ptr<uintptr_t> callback);

private:
    struct DownloadStatusJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        std::vector<OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadStatusInfo> statusInfo;
    };

    void OnJsCallbackDownloadStatus(std::unique_ptr<DownloadStatusJsCallback> &jsCb);
    static void SafeJsCallbackDownloadStatusWork(DownloadStatusJsCallback *event);

    std::mutex mutex_;
    std::list<std::shared_ptr<AutoRef>> jsCallbacks_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_SUITE_DOWNLOAD_CALLBACK_H
