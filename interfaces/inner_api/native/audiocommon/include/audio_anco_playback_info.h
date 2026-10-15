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

#ifndef AUDIO_ANCO_PLAYBACK_INFO_H
#define AUDIO_ANCO_PLAYBACK_INFO_H

#include "parcel.h"

namespace OHOS {
namespace AudioStandard {

enum AncoPlaybackState {
    ANCO_PLAYBACK_STATE_IDLE = 0,
    ANCO_PLAYBACK_STATE_PREPARING = 1,
    ANCO_PLAYBACK_STATE_PLAYING = 2,
    ANCO_PLAYBACK_STATE_PAUSED = 3,
    ANCO_PLAYBACK_STATE_STOPPED = 4,
};

class AncoPlaybackInfo : public Parcelable {
public:
    AncoPlaybackInfo() = default;
    ~AncoPlaybackInfo() = default;

    static AncoPlaybackInfo* Unmarshalling(Parcel& data)
    {
        auto* info = new (std::nothrow) AncoPlaybackInfo();
        if (info == nullptr) {
            return nullptr;
        }
        if (!data.ReadInt32(info->uid_) ||
            !data.ReadInt32(info->pid_) ||
            !data.ReadInt32(info->playbackState_) ||
            !data.ReadString(info->packageName_) ||
            !data.ReadInt32(info->usage_)) {
            delete info;
            return nullptr;
        }
        return info;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(uid_) &&
               parcel.WriteInt32(pid_) &&
               parcel.WriteInt32(playbackState_) &&
               parcel.WriteString(packageName_) &&
               parcel.WriteInt32(usage_);
    }

    void SetUid(int32_t uid)
    {
        uid_ = uid;
    }

    int32_t GetUid() const
    {
        return uid_;
    }

    void SetPid(int32_t pid)
    {
        pid_ = pid;
    }

    int32_t GetPid() const
    {
        return pid_;
    }

    void SetPlaybackState(int32_t playbackState)
    {
        playbackState_ = playbackState;
    }

    int32_t GetPlaybackState() const
    {
        return playbackState_;
    }

    void SetPackageName(const std::string& packageName)
    {
        packageName_ = packageName;
    }

    const std::string& GetPackageName() const
    {
        return packageName_;
    }

    void SetUsage(int32_t usage)
    {
        usage_ = usage;
    }

    int32_t GetUsage() const
    {
        return usage_;
    }

private:
    int32_t uid_ = 0;
    int32_t pid_ = 0;
    int32_t playbackState_ = 0;
    std::string packageName_ = "";
    int32_t usage_ = 0;
};

}
}

#endif