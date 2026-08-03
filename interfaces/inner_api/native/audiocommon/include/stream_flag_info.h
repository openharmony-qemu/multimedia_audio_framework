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

#ifndef STREAM_FLAG_INFO_H
#define STREAM_FLAG_INFO_H

#include "parcel.h"

namespace OHOS {
namespace AudioStandard {

struct StreamFlagInfo : public Parcelable {
    uint32_t audioFlag = 0;
    uint32_t staticCheckFlag = 0;
    bool isSupportUltraFast = false;

    StreamFlagInfo() = default;
    StreamFlagInfo(uint32_t audioFlag, uint32_t staticCheckFlag, bool isSupportUltraFast = false)
        : audioFlag(audioFlag), staticCheckFlag(staticCheckFlag), isSupportUltraFast(isSupportUltraFast) {}

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteUint32(audioFlag) && parcel.WriteUint32(staticCheckFlag)
            && parcel.WriteBool(isSupportUltraFast);
    }

    static StreamFlagInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) StreamFlagInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->audioFlag = parcel.ReadUint32();
        info->staticCheckFlag = parcel.ReadUint32();
        info->isSupportUltraFast = parcel.ReadBool();
        return info;
    }
};

} // namespace AudioStandard
} // namespace OHOS

#endif // STREAM_FLAG_INFO_H
