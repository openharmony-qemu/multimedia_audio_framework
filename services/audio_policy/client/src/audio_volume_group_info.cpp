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
#define LOG_TAG "VolumeGroupInfo"
#endif

#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
VolumeGroupInfo::VolumeGroupInfo(int32_t volumeGroupId, int32_t mappingId, std::string groupName, std::string networkId,
    ConnectType type) : volumeGroupId_(volumeGroupId), mappingId_(mappingId), groupName_(groupName),
    networkId_(networkId), connectType_(type)
{}

VolumeGroupInfo::VolumeGroupInfo()
{}

VolumeGroupInfo::~VolumeGroupInfo()
{}

bool VolumeGroupInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(volumeGroupId_);
    parcel.WriteInt32(mappingId_);
    parcel.WriteString(groupName_);
    parcel.WriteString(networkId_);
    parcel.WriteInt32(connectType_);
    return true;
}

sptr<VolumeGroupInfo> VolumeGroupInfo::Unmarshalling(Parcel &in)
{
    sptr<VolumeGroupInfo> volumeGroupInfo = new(std::nothrow) VolumeGroupInfo();
    if (volumeGroupInfo == nullptr) {
        return nullptr;
    }

    volumeGroupInfo->volumeGroupId_ = in.ReadInt32();
    volumeGroupInfo->mappingId_ = in.ReadInt32();
    volumeGroupInfo->groupName_ = in.ReadString();
    volumeGroupInfo->networkId_ = in.ReadString();
    volumeGroupInfo->connectType_ = static_cast<ConnectType>(in.ReadInt32());
    return volumeGroupInfo;
}
} // namespace AudioStandard
} // namespace OHOS
