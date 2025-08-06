/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_WORKGROUP_IPC_H
#define AUDIO_WORKGROUP_IPC_H

#include <parcel.h>

namespace OHOS {
namespace AudioStandard {

struct AudioWorkgroupChangeInfo {
    int32_t pid;
    uint32_t groupId;
    bool startAllowed;
};

struct AudioWorkgroupChangeInfoIpc : public Parcelable {
    AudioWorkgroupChangeInfo changeInfo;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(changeInfo.pid) &&
            parcel.WriteUint32(changeInfo.groupId) &&
            parcel.WriteBool(changeInfo.startAllowed);
    }

    static AudioWorkgroupChangeInfoIpc *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioWorkgroupChangeInfoIpc();
        if (info == nullptr) {
            return nullptr;
        }

        info->changeInfo.pid = parcel.ReadInt32();
        info->changeInfo.groupId = parcel.ReadUint32();
        info->changeInfo.startAllowed = parcel.ReadBool();
        return info;
    }
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_WORKGROUP_IPC_H