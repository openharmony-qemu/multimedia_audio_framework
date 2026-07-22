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

#ifndef AUDIO_SUITE_STATUS_CALLBACK_STUB_IMPL_H
#define AUDIO_SUITE_STATUS_CALLBACK_STUB_IMPL_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include "audio_suite_download_manager.h"
#include "audio_suite_status_callback_stub.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteStatusCallbackStubImpl : public AudioSuiteStatusCallbackStub {
public:
    AudioSuiteStatusCallbackStubImpl();
    virtual ~AudioSuiteStatusCallbackStubImpl();

    void SetCallback(std::shared_ptr<DownloadStatusCallback> callback);
    void ClearCallback();
    ErrCode OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status) override;

private:
    std::mutex mutex_;
    std::shared_ptr<DownloadStatusCallback> downloadStatusCallback_ = nullptr;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_STATUS_CALLBACK_STUB_IMPL_H