/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "IpcStreamListenerImpl"
#endif

#include "cinttypes"
#include "ipc_stream_listener_impl.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
IpcStreamListenerImpl::IpcStreamListenerImpl(std::shared_ptr<IStreamListener> innerListener)
{
    if (innerListener == nullptr) {
        AUDIO_ERR_LOG("IpcStreamListenerImpl() find null rendererInClinet");
    }
    innerListener_ = innerListener;
}

int32_t IpcStreamListenerImpl::OnOperationHandled(Operation operation, int64_t result)
{
    std::shared_ptr<IStreamListener> listener = innerListener_.lock();
    if (listener == nullptr) {
        AUDIO_WARNING_LOG("OnOperationHandled() find innerListener_ is null, operation:%{public}d result:"
            "%{public}" PRId64".", operation, result);
        return ERR_ILLEGAL_STATE;
    }
    return listener->OnOperationHandled(operation, result);
}
} // namespace AudioStandard
} // namespace OHOS
