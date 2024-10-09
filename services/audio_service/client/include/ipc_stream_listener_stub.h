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

#ifndef IPC_STREAM_LISTERNER_STUB_H
#define IPC_STREAM_LISTERNER_STUB_H

#include "message_parcel.h"

#include "ipc_stream.h"

namespace OHOS {
namespace AudioStandard {
class IpcStreamListenerStub : public IRemoteStub<IpcStreamListener> {
public:
    virtual ~IpcStreamListenerStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    static bool CheckInterfaceToken(MessageParcel &data);

    int32_t HandleOnOperationHandled(MessageParcel &data, MessageParcel &reply);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // IPC_STREAM_LISTERNER_STUB_H
