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

#ifndef DOWNLOAD_CALLBACK_STUB_IMPL_H
#define DOWNLOAD_CALLBACK_STUB_IMPL_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include "download_callback_stub.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class DownloadCallbackStubImpl : public DownloadCallbackStub {
public:
    using DownloadStatusCallback = std::function<void(const DownloadStatus&)>;

    DownloadCallbackStubImpl();
    virtual ~DownloadCallbackStubImpl();

    void SetCallback(DownloadStatusCallback callback);
    void ClearCallback();
    ErrCode OnDownloadStatusChanged(const DownloadStatus &status) override;
private:
    std::mutex mutex_;
    DownloadStatusCallback downloadCallback_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // DOWNLOAD_CALLBACK_STUB_IMPL_H