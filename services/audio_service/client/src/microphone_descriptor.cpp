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

#include "microphone_descriptor.h"
#include "audio_system_manager.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {

MicrophoneDescriptor::MicrophoneDescriptor(int32_t id, DeviceType type, int32_t groupId, int32_t sensitivity)
    : micId_(id), deviceType_(type), groupId_(groupId), sensitivity_(sensitivity)
{
    position_ = {};
    orientation_ = {};
}

MicrophoneDescriptor::MicrophoneDescriptor(int32_t id, DeviceType type) : micId_(id), deviceType_(type)
{
    sensitivity_ = 0;
    groupId_ = 0;
    position_ = {};
    orientation_ = {};
}

MicrophoneDescriptor::MicrophoneDescriptor()
    : MicrophoneDescriptor(0, DEVICE_TYPE_NONE)
{}

MicrophoneDescriptor::MicrophoneDescriptor(const MicrophoneDescriptor &micDescriptor)
{
    micId_ = micDescriptor.micId_;
    deviceType_ = micDescriptor.deviceType_;
    groupId_ = micDescriptor.groupId_;
    sensitivity_ = micDescriptor.sensitivity_;
    position_ = micDescriptor.position_;
    orientation_ = micDescriptor.orientation_;
}

MicrophoneDescriptor::MicrophoneDescriptor(const sptr<MicrophoneDescriptor> &micDescriptor)
{
    micId_ = micDescriptor->micId_;
    deviceType_ = micDescriptor->deviceType_;
    groupId_ = micDescriptor->groupId_;
    sensitivity_ = micDescriptor->sensitivity_;
    position_ = micDescriptor->position_;
    orientation_ = micDescriptor->orientation_;
}

MicrophoneDescriptor::~MicrophoneDescriptor()
{}

bool MicrophoneDescriptor::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(micId_);
    parcel.WriteInt32(deviceType_);
    parcel.WriteInt32(groupId_);
    parcel.WriteInt32(sensitivity_);
    parcel.WriteFloat(position_.x);
    parcel.WriteFloat(position_.y);
    parcel.WriteFloat(position_.z);
    parcel.WriteFloat(orientation_.x);
    parcel.WriteFloat(orientation_.y);
    parcel.WriteFloat(orientation_.z);
    return true;
}

sptr<MicrophoneDescriptor> MicrophoneDescriptor::Unmarshalling(Parcel &in)
{
    sptr<MicrophoneDescriptor> microphoneDescriptor = new(std::nothrow) MicrophoneDescriptor();
    CHECK_AND_RETURN_RET(microphoneDescriptor != nullptr, nullptr);

    microphoneDescriptor->micId_ = in.ReadInt32();
    microphoneDescriptor->deviceType_ = static_cast<DeviceType>(in.ReadInt32());
    microphoneDescriptor->groupId_ = in.ReadInt32();
    microphoneDescriptor->sensitivity_ = in.ReadInt32();
    microphoneDescriptor->position_.x = in.ReadFloat();
    microphoneDescriptor->position_.y = in.ReadFloat();
    microphoneDescriptor->position_.z = in.ReadFloat();
    microphoneDescriptor->orientation_.x = in.ReadFloat();
    microphoneDescriptor->orientation_.y = in.ReadFloat();
    microphoneDescriptor->orientation_.z = in.ReadFloat();

    return microphoneDescriptor;
}

void MicrophoneDescriptor::SetMicPositionInfo(const Vector3D &pos)
{
    position_ =  pos;
}

void MicrophoneDescriptor::SetMicOrientationInfo(const Vector3D &orientation)
{
    orientation_ = orientation;
}
} // namespace AudioStandard
} // namespace OHOS
