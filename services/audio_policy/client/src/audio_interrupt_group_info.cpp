/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "InterruptGroupInfo"
#endif

#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
InterruptGroupInfo::InterruptGroupInfo()
{
}

InterruptGroupInfo::InterruptGroupInfo(int32_t interruptGroupId, int32_t mappingId, std::string groupName,
    std::string networkId, ConnectType type) : interruptGroupId_(interruptGroupId), mappingId_(mappingId),
    groupName_(groupName), networkId_(networkId), connectType_(type)
{}

InterruptGroupInfo::~InterruptGroupInfo()
{}

bool InterruptGroupInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(interruptGroupId_);
    parcel.WriteInt32(mappingId_);
    parcel.WriteString(groupName_);
    parcel.WriteString(networkId_);
    parcel.WriteInt32(connectType_);
    return true;
}

sptr<InterruptGroupInfo> InterruptGroupInfo::Unmarshalling(Parcel &in)
{
    sptr<InterruptGroupInfo> interruptGroupInfo = new(std::nothrow) InterruptGroupInfo();
    if (interruptGroupInfo == nullptr) {
        return nullptr;
    }

    interruptGroupInfo->interruptGroupId_ = in.ReadInt32();
    interruptGroupInfo->mappingId_ = in.ReadInt32();
    interruptGroupInfo->groupName_ = in.ReadString();
    interruptGroupInfo->networkId_ = in.ReadString();
    interruptGroupInfo->connectType_ = static_cast<ConnectType>(in.ReadInt32());
    return interruptGroupInfo;
}
} // namespace AudioStandard
} // namespace OHOS
