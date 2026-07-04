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

#ifndef CLOUD_ROM_DOWNLOAD_CALLBACK_IMPL_H
#define CLOUD_ROM_DOWNLOAD_CALLBACK_IMPL_H

#include "icloud_rom_download_callback.h"
#include "cloud_rom_download_callback_stub.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class DownloadManager;

// CloudRomDownloadCallbackImpl 实现
class CloudRomDownloadCallbackImpl : public CloudRomDownloadCallbackStub {
public:
    explicit CloudRomDownloadCallbackImpl(DownloadManager* downloadManager)
        : CloudRomDownloadCallbackStub(),
          downloadManager_(downloadManager) {}

    ErrCode OnCloudRomCallback(const CloudRomCallback& cloudRomData) override;

private:
    DownloadManager* downloadManager_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // CLOUD_ROM_DOWNLOAD_CALLBACK_IMPL_H