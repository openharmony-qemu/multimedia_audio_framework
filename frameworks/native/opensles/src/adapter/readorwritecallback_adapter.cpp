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

#include <common.h>
#include <cstddef>
#include "OpenSLES_OpenHarmony.h"
#include "readorwritecallback_adapter.h"

namespace OHOS {
namespace AudioStandard {
    ReadOrWriteCallbackAdapter::ReadOrWriteCallbackAdapter
        (SlOHBufferQueueCallback callback, SLOHBufferQueueItf itf, void *context)
    {
        callback_ = callback;
        itf_ = itf;
        context_ = context;
    }

    ReadOrWriteCallbackAdapter::~ReadOrWriteCallbackAdapter() { }

    void ReadOrWriteCallbackAdapter::OnWriteData(size_t length)
    {
        callback_(itf_, context_, length);
        return;
    }

    void ReadOrWriteCallbackAdapter::OnReadData(size_t length)
    {
        callback_(itf_, context_, length);
    }
}  // namespace AudioStandard
}  // namespace OHOS
