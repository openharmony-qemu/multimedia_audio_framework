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
#ifndef MICROPHONE_DESCRIPTOR_H
#define MICROPHONE_DESCRIPTOR_H

#include <parcel.h>
#include "audio_device_info.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class MicrophoneDescriptor;
class MicrophoneDescriptor : public Parcelable {
    friend class AudioSystemManager;
public:
    int32_t micId_;
    DeviceType deviceType_;
    int32_t groupId_;
    int32_t sensitivity_;
    Vector3D position_  = {};
    Vector3D orientation_  = {};

    MicrophoneDescriptor();
    MicrophoneDescriptor(int32_t id, DeviceType type);
    MicrophoneDescriptor(const MicrophoneDescriptor &micDescriptor);
    MicrophoneDescriptor(const sptr<MicrophoneDescriptor> &micDescriptor);
    MicrophoneDescriptor(int32_t id, DeviceType type, int32_t groupId, int32_t sensitivity);
    virtual ~MicrophoneDescriptor();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<MicrophoneDescriptor> Unmarshalling(Parcel &in);

    void SetMicPositionInfo(const Vector3D &pos);
    void SetMicOrientationInfo(const Vector3D &orientation);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MICROPHONE_DESCRIPTOR_H