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

#ifndef NAPI_AUDIO_SUITE_DOWNLOAD_MANAGER_H
#define NAPI_AUDIO_SUITE_DOWNLOAD_MANAGER_H

#include <mutex>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_suite_download_manager.h"
#include "napi_audio_suite_download_callback.h"

namespace OHOS {
namespace AudioStandard {

inline const std::string NAPI_AUDIO_SUITE_DOWNLOAD_MGR_CLASS_NAME = "AudioSuiteDownloadManager";
inline const std::string DOWNLOAD_STATUS_CHANGE_CALLBACK_NAME_NAPI = "downloadStatusChange";

class NapiAudioSuiteDownloadManager {
public:
    NapiAudioSuiteDownloadManager();
    ~NapiAudioSuiteDownloadManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateSuiteDownloadManagerWrapper(napi_env env);

private:
    struct AudioSuiteDownloadMgrAsyncContext : public ContextBase {
        int32_t intValue = 0;
        int32_t networkType = 0;
        std::vector<int32_t> featureTypes;
        std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> statusInfos;
        std::vector<bool> installStatus;
        std::vector<AudioSuite::AudioSuiteCloudRomVersionInfo> versionInfos;
    };

    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static bool CheckContextStatus(std::shared_ptr<AudioSuiteDownloadMgrAsyncContext> context);
    static bool CheckSuiteDownloadManagerStatus(NapiAudioSuiteDownloadManager *napi,
        std::shared_ptr<AudioSuiteDownloadMgrAsyncContext> context);
    static int32_t ConvertNativeErrorToNapi(int32_t nativeError);

    static napi_value StartDownload(napi_env env, napi_callback_info info);
    static napi_value PauseDownload(napi_env env, napi_callback_info info);
    static napi_value CancelDownload(napi_env env, napi_callback_info info);
    static napi_value StartBackgroundDownload(napi_env env, napi_callback_info info);
    static napi_value GetFeatureStatus(napi_env env, napi_callback_info info);
    static napi_value UninstallFeature(napi_env env, napi_callback_info info);
    static napi_value IsFeatureInstalled(napi_env env, napi_callback_info info);
    static napi_value GetNewVersionInfo(napi_env env, napi_callback_info info);
    static napi_value OnDownloadStatusChange(napi_env env, napi_callback_info info);
    static napi_value OffDownloadStatusChange(napi_env env, napi_callback_info info);
    static void RegisterDownloadStatusCallback(napi_env env, napi_value *args,
        NapiAudioSuiteDownloadManager *napi);
    static void UnregisterDownloadStatusCallback(napi_env env, napi_value *args,
        NapiAudioSuiteDownloadManager *napi);

    AudioSuite::AudioSuiteDownloadManager *audioSuiteDownloadMgr_ = nullptr;
    std::shared_ptr<NapiAudioSuiteDownloadCallback> downloadCallback_ = nullptr;
    std::mutex downloadCbMutex_;
    napi_env env_ = nullptr;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif  // NAPI_AUDIO_SUITE_DOWNLOAD_MANAGER_H
