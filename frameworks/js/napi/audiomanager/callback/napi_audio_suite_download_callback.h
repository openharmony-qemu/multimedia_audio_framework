/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NAPI_AUDIO_SUITE_DOWNLOAD_CALLBACK_H
#define NAPI_AUDIO_SUITE_DOWNLOAD_CALLBACK_H

#include <mutex>
#include <vector>
#include "audio_suite_download_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {

inline const std::string DOWNLOAD_STATUS_CHANGE_CALLBACK_NAME = "downloadStatusChange";

class NapiAudioSuiteDownloadCallback : public AudioSuite::DownloadStatusCallback {
public:
    explicit NapiAudioSuiteDownloadCallback(napi_env env);
    virtual ~NapiAudioSuiteDownloadCallback();

    void OnDownloadStatusChanged(const std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> &status) override;
    void SaveCallbackReference(napi_value args);
    bool RemoveCallbackReference(napi_env env, napi_value args);
    void ClearCallbacks();
    bool IsEmpty() const;
    void CreateDownloadStatusTsfn(napi_env env);
    bool GetDownloadStatusTsfnFlag() const;
    bool ContainSameJsCallback(napi_env env, napi_value args) const;

private:
    struct DownloadStatusJsCallback {
        std::vector<std::shared_ptr<AutoRef>> callbacks;
        std::string callbackName = "unknown";
        std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> statusInfo;
    };

    void OnJsCallbackDownloadStatus(std::unique_ptr<DownloadStatusJsCallback> &jsCb);
    static void DownloadStatusTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackDownloadStatusWork(napi_env env, napi_value js_cb, void *context, void *data);

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<AutoRef>> jsCallbacks_;
    napi_env env_ = nullptr;
    bool regDownloadStatusTsfn_ = false;
    napi_threadsafe_function downloadStatusTsfn_ = nullptr;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif  // NAPI_AUDIO_SUITE_DOWNLOAD_CALLBACK_H
