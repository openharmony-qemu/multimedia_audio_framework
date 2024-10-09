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

#ifndef AUDIO_PROCESS_STUB_H
#define AUDIO_PROCESS_STUB_H

#include "message_parcel.h"

#include "i_audio_process.h"

namespace OHOS {
namespace AudioStandard {
class ProcessCbProxy : public IRemoteProxy<IProcessCb> {
public:
    explicit ProcessCbProxy(const sptr<IRemoteObject> &impl);
    virtual ~ProcessCbProxy();

    int32_t OnEndpointChange(int32_t status) override;

private:
    static inline BrokerDelegator<ProcessCbProxy> delegator_;
};

class AudioProcessStub : public IRemoteStub<IAudioProcess> {
public:
    virtual ~AudioProcessStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    static bool CheckInterfaceToken(MessageParcel &data);

    int32_t HandleResolveBuffer(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetSessionId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStart(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePause(MessageParcel &data, MessageParcel &reply);
    int32_t HandleResume(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStop(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRequestHandleInfo(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRelease(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterProcessCb(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterThreadPriority(MessageParcel &data, MessageParcel &reply);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_STUB_H
