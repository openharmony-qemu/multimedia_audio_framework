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

#ifndef AUDIO_ROUTE_SELECT_INFO_H
#define AUDIO_ROUTE_SELECT_INFO_H

#include <memory>

#include "audio_info.h"
#include "audio_pipe_types.h"
#include "audio_stream_info.h"
#include "parcel.h"

namespace OHOS {
namespace AudioStandard {
struct AudioRouteSelectInfo : public Parcelable {
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioStreamInfo streamInfo = {};
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    AppInfo appInfo = {};
    std::string bundleName = "";
    AudioFlag routeFlag = AUDIO_FLAG_NONE;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(static_cast<int32_t>(audioMode))
            && streamInfo.Marshalling(parcel)
            && parcel.WriteInt32(static_cast<int32_t>(streamUsage))
            && parcel.WriteInt32(static_cast<int32_t>(sourceType))
            && parcel.WriteInt32(appInfo.appUid)
            && parcel.WriteUint32(appInfo.appTokenId)
            && parcel.WriteInt32(appInfo.appPid)
            && parcel.WriteUint64(appInfo.appFullTokenId)
            && parcel.WriteString(appInfo.deviceId)
            && parcel.WriteString(bundleName)
            && parcel.WriteUint32(static_cast<uint32_t>(routeFlag));
    }

    static AudioRouteSelectInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioRouteSelectInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->audioMode = static_cast<AudioMode>(parcel.ReadInt32());
        std::unique_ptr<AudioStreamInfo> streamInfoPtr(AudioStreamInfo::Unmarshalling(parcel));
        if (streamInfoPtr == nullptr) {
            delete info;
            return nullptr;
        }
        info->streamInfo = *streamInfoPtr;
        info->streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
        info->sourceType = static_cast<SourceType>(parcel.ReadInt32());
        info->appInfo.appUid = parcel.ReadInt32();
        info->appInfo.appTokenId = parcel.ReadUint32();
        info->appInfo.appPid = parcel.ReadInt32();
        info->appInfo.appFullTokenId = parcel.ReadUint64();
        info->appInfo.deviceId = parcel.ReadString();
        info->bundleName = parcel.ReadString();
        info->routeFlag = static_cast<AudioFlag>(parcel.ReadUint32());
        return info;
    }
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_ROUTE_SELECT_INFO_H
