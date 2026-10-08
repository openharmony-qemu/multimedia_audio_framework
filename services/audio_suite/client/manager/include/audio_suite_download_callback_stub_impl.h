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

#ifndef AUDIO_SUITE_DOWNLOAD_CALLBACK_STUB_IMPL_H
#define AUDIO_SUITE_DOWNLOAD_CALLBACK_STUB_IMPL_H

#include "audio_suite_download_manager.h"
#include "audio_suite_download_callback_stub.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteDownloadCallbackStubImpl : public AudioSuiteDownloadCallbackStub {
public:
    AudioSuiteDownloadCallbackStubImpl();
    virtual ~AudioSuiteDownloadCallbackStubImpl();
    int32_t SetAudioSuiteRomDownloadCallback(const std::weak_ptr<AudioSuiteRomDownloadCallback> &callback);
    void ClearCallback();

    int32_t GetFeatureNewVersion(const std::vector<std::string> &featureNames,
        std::vector<AudioSuiteCloudRomVersionInfo> &newVersionInfos, int32_t &funcResult) override;
    int32_t GetFeatureStatus(const std::vector<std::string> &featureNames,
        std::vector<AudioSuiteDownloadStatusInfo> &statusInfos, int32_t &funcResult) override;
    int32_t PauseCloudRom(const std::vector<std::string> &featureNames, int32_t &funcResult) override;
    int32_t CancelCloudRom(const std::vector<std::string> &featureNames, int32_t &funcResult) override;
    int32_t DownloadAndInstall(
        const bool allowedCellularNet, const std::vector<std::string> &featureNames, int32_t &funcResult) override;
    int32_t UninstallCloudRom(const std::vector<std::string> &featureNames, int32_t &funcResult) override;
    int32_t RegisterDownloadStatusCallback(int32_t &funcResult) override;
    int32_t UnregisterDownloadStatusCallback(int32_t &funcResult) override;

private:
    std::mutex downloadCallback;
    std::weak_ptr<AudioSuiteRomDownloadCallback> downloadCallback_;
};
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_SUITE_DOWNLOAD_CALLBACK_STUB_IMPL_H
