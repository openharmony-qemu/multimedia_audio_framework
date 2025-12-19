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

#ifndef AUDIO_INTERRUPT_TYPES_H
#define AUDIO_INTERRUPT_TYPES_H

#include "audio_interrupt_info.h"
#include "audio_interrupt_callback.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class InterruptGroupInfo : public Parcelable {
public:
    int32_t interruptGroupId_ = 0;
    int32_t mappingId_ = 0;
    std::string groupName_;
    std::string networkId_;
    ConnectType connectType_ = CONNECT_TYPE_LOCAL;

    InterruptGroupInfo() {};
    InterruptGroupInfo(int32_t interruptGroupId, int32_t mappingId, std::string groupName,
        std::string networkId, ConnectType type) : interruptGroupId_(interruptGroupId),
        mappingId_(mappingId), groupName_(groupName), networkId_(networkId), connectType_(type) {};

    virtual ~InterruptGroupInfo() {};
    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteInt32(interruptGroupId_);
        parcel.WriteInt32(mappingId_);
        parcel.WriteString(groupName_);
        parcel.WriteString(networkId_);
        parcel.WriteInt32(connectType_);
        return true;
    };

    static InterruptGroupInfo *Unmarshalling(Parcel &in)
    {
        auto interruptGroupInfo = new(std::nothrow) InterruptGroupInfo();
        if (interruptGroupInfo == nullptr) {
            return nullptr;
        }

        interruptGroupInfo->interruptGroupId_ = in.ReadInt32();
        interruptGroupInfo->mappingId_ = in.ReadInt32();
        interruptGroupInfo->groupName_ = in.ReadString();
        interruptGroupInfo->networkId_ = in.ReadString();
        interruptGroupInfo->connectType_ = static_cast<ConnectType>(in.ReadInt32());
        return interruptGroupInfo;
    };
};

// AudioManagerCallback OnInterrupt is added to handle compilation error in call manager
// Once call manager adapt to new interrupt APIs, this will be removed
class AudioManagerCallback {
public:
    virtual ~AudioManagerCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param interruptAction Indicates the InterruptAction information needed by client.
     * For details, refer InterruptAction struct in audio_info.h
     */
    virtual void OnInterrupt(const InterruptAction &interruptAction) = 0;
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INTERRUPT_TYPES_H
