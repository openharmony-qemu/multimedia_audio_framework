/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef READORWRITECALLBACK_ADAPTER_H
#define READORWRITECALLBACK_ADAPTER_H

#include <OpenSLES.h>
#include <OpenSLES_Platform.h>
#include <iostream>
#include <audio_renderer.h>
#include <audio_capturer.h>

namespace OHOS {
namespace AudioStandard {
class ReadOrWriteCallbackAdapter : public AudioRendererWriteCallback, public AudioCapturerReadCallback {
public:
    ReadOrWriteCallbackAdapter (SlOHBufferQueueCallback callback, SLOHBufferQueueItf itf, void *pContext);
    ~ReadOrWriteCallbackAdapter();
    void OnWriteData(size_t length) override;
    void OnReadData(size_t length) override;

private:
    SlOHBufferQueueCallback callback_;
    SLOHBufferQueueItf itf_;
    void *context_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // READORWRITECALLBACK_ADAPTER_H
