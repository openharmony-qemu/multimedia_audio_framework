/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef VA_DEVICE_H
#define VA_DEVICE_H

#include "audio_device_info.h"
#include "audio_info.h"
#include "audio_stream_info.h"

#include "va_device_info.h"

#include "message_parcel.h"

namespace OHOS {
namespace AudioStandard {
struct VADevice : public Parcelable {
    std::string implementor_;
    VADeviceConfiguration configuration_;

    VADevice() = default;

    bool Marshalling(Parcel& parcel)const override
    {
        return parcel.WriteString(implementor_) && configuration_.Marshalling(parcel);
    }
    void UnmarshallingSelf(Parcel& parcel)
    {
        implementor_ = parcel.ReadString();
        configuration_.UnmarshallingSelf(parcel);
    }
    static VADevice* Unmarshalling(Parcel& parcel)
    {
        auto device = new VADevice();
        if (device == nullptr) {
            return nullptr;
        }
        device->UnmarshallingSelf(parcel);
        return device;
    }
};
}  //namespace AudioStandard
}  //namespace OHOS
#endif //VA_DEVICE_INFO_H
