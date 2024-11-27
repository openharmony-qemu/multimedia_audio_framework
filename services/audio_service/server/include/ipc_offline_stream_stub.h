/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef IPC_OFFLINE_STREAM_STUB_H
#define IPC_OFFLINE_STREAM_STUB_H

#include "message_parcel.h"

#include "ipc_offline_stream.h"

namespace OHOS {
namespace AudioStandard {
using HandlerFunction = std::function<int(MessageParcel &data, MessageParcel &reply)>;
class IpcOfflineStreamStub : public IRemoteStub<IpcOfflineStream> {
public:
    IpcOfflineStreamStub();
    virtual ~IpcOfflineStreamStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    static bool CheckInterfaceToken(MessageParcel &data);

    int32_t HandleCreateOfflineEffectChain(MessageParcel &data, MessageParcel &reply);
    int32_t HandleConfigureOfflineEffectChain(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareOfflineEffectChain(MessageParcel &data, MessageParcel &reply);
    int32_t HandleProcessOfflineEffectChain(MessageParcel &data, MessageParcel &reply);
    int32_t HandleReleaseOfflineEffectChain(MessageParcel &data, MessageParcel &reply);

    std::unordered_map<uint32_t, HandlerFunction> handlerMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // IPC_OFFLINE_STREAM_STUB_H
