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

#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteStatusCallbackStubImpl"
#endif

#include "audio_suite_status_callback_stub_impl.h"
#include "audio_suite_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteStatusCallbackStubImpl::AudioSuiteStatusCallbackStubImpl()
{
    AUDIO_INFO_LOG("AudioSuiteStatusCallbackStubImpl constructor");
}

AudioSuiteStatusCallbackStubImpl::~AudioSuiteStatusCallbackStubImpl()
{
    AUDIO_INFO_LOG("AudioSuiteStatusCallbackStubImpl destructor");
    ClearCallback();
}

void AudioSuiteStatusCallbackStubImpl::SetCallback(std::shared_ptr<DownloadStatusCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    downloadStatusCallback_ = callback;
    AUDIO_INFO_LOG("AudioSuiteStatusCallbackStubImpl callback set");
}

void AudioSuiteStatusCallbackStubImpl::ClearCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    downloadStatusCallback_ = nullptr;
    AUDIO_INFO_LOG("AudioSuiteStatusCallbackStubImpl callback cleared");
}

ErrCode AudioSuiteStatusCallbackStubImpl::OnDownloadStatusChanged(
    const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    std::shared_ptr<DownloadStatusCallback> callback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callback = downloadStatusCallback_;
    }

    if (callback != nullptr) {
        AUDIO_WARNING_LOG("OnDownloadStatusChanged, feature size:%{public}zu", status.size());
        callback->OnDownloadStatusChanged(status);
        return SUCCESS;
    } else {
        AUDIO_WARNING_LOG("OnDownloadStatusChanged: callback is nullptr");
        return ERR_INVALID_PARAM;
    }
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS