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

#ifndef AUDIO_SPATIALIZATION_TYPES_H
#define AUDIO_SPATIALIZATION_TYPES_H

#include <mutex>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
enum AudioSpatialDeviceType {
    EARPHONE_TYPE_NONE = 0,
    EARPHONE_TYPE_INEAR,
    EARPHONE_TYPE_HALF_INEAR,
    EARPHONE_TYPE_HEADPHONE,
    EARPHONE_TYPE_GLASSES,
    EARPHONE_TYPE_OTHERS,
};

enum AudioSpatializationSceneType {
    SPATIALIZATION_SCENE_TYPE_DEFAULT = 0,
    SPATIALIZATION_SCENE_TYPE_MUSIC = 1,
    SPATIALIZATION_SCENE_TYPE_MOVIE = 2,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK = 3,
    SPATIALIZATION_SCENE_TYPE_MAX = SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
};

struct AudioRendererInfoForSpatialization {
    RendererState rendererState;
    std::string deviceMacAddress;
    StreamUsage streamUsage;
};

struct AudioSpatialDeviceState : public Parcelable {
    std::string address;
    bool isSpatializationSupported;
    bool isHeadTrackingSupported;
    AudioSpatialDeviceType spatialDeviceType;

    AudioSpatialDeviceState() = default;
    AudioSpatialDeviceState(const std::string &address, bool isSpatializationSupported,
        bool isHeadTrackingSupported, AudioSpatialDeviceType spatialDeviceType)
        : address(address), isSpatializationSupported(isSpatializationSupported),
        isHeadTrackingSupported(isHeadTrackingSupported), spatialDeviceType(spatialDeviceType)
    {
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(address) &&
            parcel.WriteBool(isSpatializationSupported) &&
            parcel.WriteBool(isHeadTrackingSupported) &&
            parcel.WriteInt32(spatialDeviceType);
    }

    static AudioSpatialDeviceState *Unmarshalling(Parcel &parcel)
    {
        auto deviceState = new(std::nothrow) AudioSpatialDeviceState();
        if (deviceState == nullptr) {
            return nullptr;
        }
        deviceState->address = parcel.ReadString();
        deviceState->isSpatializationSupported = parcel.ReadBool();
        deviceState->isHeadTrackingSupported = parcel.ReadBool();
        deviceState->spatialDeviceType = static_cast<AudioSpatialDeviceType>(parcel.ReadInt32());
        return deviceState;
    }
};

struct AudioSpatializationState : public Parcelable {
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    bool adaptiveSpatialRenderingEnabled = false;

    AudioSpatializationState() = default;
    AudioSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
    {
        this->spatializationEnabled = spatializationEnabled;
        this->headTrackingEnabled = headTrackingEnabled;
    }

    AudioSpatializationState(bool spatializationEnabled, bool headTrackingEnabled, bool adaptiveSpatialRenderingEnabled)
    {
        this->spatializationEnabled = spatializationEnabled;
        this->headTrackingEnabled = headTrackingEnabled;
        this->adaptiveSpatialRenderingEnabled = adaptiveSpatialRenderingEnabled;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteBool(spatializationEnabled)
            && parcel.WriteBool(headTrackingEnabled);
    }

    static AudioSpatializationState *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioSpatializationState();
        if (info == nullptr) {
            return nullptr;
        }
        info->spatializationEnabled = parcel.ReadBool();
        info->headTrackingEnabled = parcel.ReadBool();
        return info;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SPATIALIZATION_TYPES_H
