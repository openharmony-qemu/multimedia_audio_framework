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

#ifndef TAIHE_AUDIO_SUITE_DOWNLOAD_MANAGER_H
#define TAIHE_AUDIO_SUITE_DOWNLOAD_MANAGER_H

#include "audio_suite_download_manager.h"
#include "audio_utils.h"
#include "taihe_work.h"
#include "taihe_audio_error.h"
#include "taihe_audio_suite_download_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioSuiteDownloadManagerImpl {
public:
    AudioSuiteDownloadManagerImpl();
    explicit AudioSuiteDownloadManagerImpl(std::shared_ptr<AudioSuiteDownloadManagerImpl> obj);
    ~AudioSuiteDownloadManagerImpl();

    static AudioSuiteDownloadManager CreateSuiteDownloadManagerWrapper();

    void StartDownloadSync(array_view<AudioSuiteFeatureType> featureTypes, NetworkType networkType);
    void PauseDownloadSync(array_view<AudioSuiteFeatureType> featureTypes);
    void CancelDownloadSync(array_view<AudioSuiteFeatureType> featureTypes);
    void StartBackgroundDownloadSync(array_view<AudioSuiteFeatureType> featureTypes);
    array<AudioSuiteFeatureStatusInfo> GetFeatureStatusSync(array_view<AudioSuiteFeatureType> featureTypes);
    void UninstallFeatureSync(array_view<AudioSuiteFeatureType> featureTypes);
    array<bool> IsFeatureInstalledSync(array_view<AudioSuiteFeatureType> featureTypes);
    array<AudioSuiteFeatureVersionInfo> GetNewVersionInfoSync(array_view<AudioSuiteFeatureType> featureTypes);

    void OnDownloadStatusChange(callback_view<void(array_view<AudioSuiteFeatureStatusInfo> data)> callback);
    void OffDownloadStatusChange(
        optional_view<callback<void(array_view<AudioSuiteFeatureStatusInfo> data)>> callback);

private:
    static void RegisterDownloadStatusCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSuiteDownloadManagerImpl *impl);
    static void UnregisterDownloadStatusCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSuiteDownloadManagerImpl *impl);

    std::shared_ptr<TaiheAudioSuiteDownloadCallback> downloadCallback_ = nullptr;
    std::mutex downloadCbMutex_;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_SUITE_DOWNLOAD_MANAGER_H
