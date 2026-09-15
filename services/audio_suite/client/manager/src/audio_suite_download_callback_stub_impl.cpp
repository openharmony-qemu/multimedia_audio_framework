/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSuiteDownloadCallbackStubImpl"
#endif

#include "audio_suite_download_callback_stub_impl.h"

#include "audio_errors.h"
#include "audio_suite_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuiteDownloadCallbackStubImpl::AudioSuiteDownloadCallbackStubImpl()
{}

AudioSuiteDownloadCallbackStubImpl::~AudioSuiteDownloadCallbackStubImpl()
{}

int32_t AudioSuiteDownloadCallbackStubImpl::SetAudioSuiteRomDownloadCallback(
    const std::weak_ptr<AudioSuiteRomDownloadCallback> &callback)
{
    std::lock_guard<std::mutex> lock(downloadCallback);
    downloadCallback_ = callback;
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");
    return SUCCESS;
}

void AudioSuiteDownloadCallbackStubImpl::ClearCallback()
{
    std::lock_guard<std::mutex> lock(downloadCallback);
    downloadCallback_.reset();
}

int32_t AudioSuiteDownloadCallbackStubImpl::GetFeatureNewVersion(
    const std::vector<std::string> &featureNames, std::vector<AudioSuiteCloudRomVersionInfo> &newVersionInfos,
    int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERROR, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->GetFeatureNewVersion(featureNames, newVersionInfos);
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::GetFeatureStatus(const std::vector<std::string> &featureNames,
                                                             std::vector<AudioSuiteDownloadStatusInfo> &statusInfos,
                                                             int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->GetFeatureStatus(featureNames, statusInfos);
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::DownloadAndInstall(const bool allowedCellularNet,
    const std::vector<std::string> &featureNames, int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->DownloadAndInstall(allowedCellularNet, featureNames);
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::PauseCloudRom(
    const std::vector<std::string> &featureNames, int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->PauseCloudRom(featureNames);
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::CancelCloudRom(
    const std::vector<std::string> &featureNames, int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->CancelCloudRom(featureNames);
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::UninstallCloudRom(
    const std::vector<std::string> &featureNames, int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback =
        downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->UninstallCloudRom(featureNames);
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::RegisterDownloadStatusCallback(int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback =
        downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->RegisterDownloadStatusCallback();
    return SUCCESS;
}

int32_t AudioSuiteDownloadCallbackStubImpl::UnregisterDownloadStatusCallback(int32_t &funcResult)
{
    std::unique_lock lock(downloadCallback);
    std::shared_ptr<AudioSuiteRomDownloadCallback> downloadCallback = downloadCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(downloadCallback != nullptr, ERR_INVALID_PARAM, "downloadCallback_ is nullptr");

    funcResult = downloadCallback->UnregisterDownloadStatusCallback();
    return SUCCESS;
}
}
}  // namespace AudioStandard
}  // namespace OHOS
