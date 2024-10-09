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

#ifndef IPC_STREAM_LISTENER_PROXY_H
#define IPC_STREAM_LISTENER_PROXY_H

#include "message_parcel.h"

#include "ipc_stream.h"

namespace OHOS {
namespace AudioStandard {
class IpcStreamListenerProxy : public IRemoteProxy<IpcStreamListener> {
public:
    explicit IpcStreamListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~IpcStreamListenerProxy();

    int32_t OnOperationHandled(Operation operation, int64_t result) override;

private:
    static inline BrokerDelegator<IpcStreamListenerProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // IPC_STREAM_LISTENER_PROXY_H
