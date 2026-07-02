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
#define LOG_TAG "DownloadCallbackStubImpl"
#endif

#include "download_callback_stub_impl.h"
#include "audio_suite_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

DownloadCallbackStubImpl::DownloadCallbackStubImpl()
    : downloadCallback_(nullptr)
{
    AUDIO_INFO_LOG("DownloadCallbackStubImpl constructor");
}

DownloadCallbackStubImpl::~DownloadCallbackStubImpl()
{
    AUDIO_INFO_LOG("DownloadCallbackStubImpl destructor");
    ClearCallback();
}

void DownloadCallbackStubImpl::SetCallback(DownloadStatusCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    downloadCallback_ = callback;
    AUDIO_INFO_LOG("DownloadCallbackStubImpl callback set");
}

void DownloadCallbackStubImpl::ClearCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    downloadCallback_ = nullptr;
    AUDIO_INFO_LOG("DownloadCallbackStubImpl callback cleared");
}

ErrCode DownloadCallbackStubImpl::OnDownloadStatusChanged(const DownloadStatus& status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (downloadCallback_ != nullptr) {
        downloadCallback_(status);
    } else {
        AUDIO_WARNING_LOG("OnDownloadStatusChanged: callback is null");
    }
    return SUCCESS;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS