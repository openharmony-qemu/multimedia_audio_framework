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
#ifndef AUDIO_SESSION_DEVICE_INFO_H
#define AUDIO_SESSION_DEVICE_INFO_H

#include "audio_device_descriptor.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {

/**
 * Enumerates the recommend action when device changed.
 * @since 20
 */
enum class OutputDeviceChangeRecommendedAction {
    /**
     * No special recommendations, the playback can be continue or not.
     */
    RECOMMEND_TO_CONTINUE = 0,
    /**
     * Recommend to stop the playback.
     */
    RECOMMEND_TO_STOP = 1,
};

/**
 * Audio session device change info.
 * @since 20
 */
struct CurrentOutputDeviceChangedEvent : public Parcelable {
    /**
     * Audio device descriptors after changed.
     * @since 20
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    /**
     * Audio device changed reason.
     * @since 20
     */
    AudioStreamDeviceChangeReason changeReason;
    /**
     * Recommend action when device changed.
     * @since 20
     */
    OutputDeviceChangeRecommendedAction recommendedAction;
    static constexpr int32_t DEVICE_CHANGE_VALID_SIZE = 128;

    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteInt32(static_cast<int32_t>(changeReason));
        parcel.WriteInt32(static_cast<int32_t>(recommendedAction));
        int32_t size = static_cast<int32_t>(devices.size());
        parcel.WriteInt32(size);
        for (int i = 0; i < size; i++) {
            if (devices[i] != nullptr) {
                devices[i]->Marshalling(parcel);
            }
        }
        return true;
    }

    static CurrentOutputDeviceChangedEvent *Unmarshalling(Parcel &parcel)
    {
        auto event = new(std::nothrow) CurrentOutputDeviceChangedEvent();
        if (event == nullptr) {
            return nullptr;
        }

        event->changeReason = static_cast<AudioStreamDeviceChangeReason>(parcel.ReadInt32());
        event->recommendedAction = static_cast<OutputDeviceChangeRecommendedAction>(parcel.ReadInt32());
        int32_t size = parcel.ReadInt32();
        if (size < 0 || size >= DEVICE_CHANGE_VALID_SIZE) {
            delete event;
            return nullptr;
        }
        for (int32_t i = 0; i < size; i++) {
            auto device = AudioDeviceDescriptor::Unmarshalling(parcel);
            if (device != nullptr) {
                event->devices.emplace_back(std::shared_ptr<AudioDeviceDescriptor>(device));
            }
        }
        return event;
    }
};

/**
 * Audio session device change info.
 * @since 21
 */
struct CurrentInputDeviceChangedEvent : public Parcelable {
    /**
     * Audio device descriptors after changed.
     * @since 21
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    /**
     * Audio device changed reason.
     * @since 21
     */
    AudioStreamDeviceChangeReason changeReason;

    static constexpr int32_t DEVICE_CHANGE_VALID_SIZE = 128;

    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteInt32(static_cast<int32_t>(changeReason));
        int32_t size = static_cast<int32_t>(devices.size());
        parcel.WriteInt32(size);
        for (int i = 0; i < size; i++) {
            if (devices[i] != nullptr) {
                devices[i]->Marshalling(parcel);
            }
        }
        return true;
    }

    static CurrentInputDeviceChangedEvent *Unmarshalling(Parcel &parcel)
    {
        auto event = new(std::nothrow) CurrentInputDeviceChangedEvent();
        if (event == nullptr) {
            return nullptr;
        }

        event->changeReason = static_cast<AudioStreamDeviceChangeReason>(parcel.ReadInt32());
        int32_t size = parcel.ReadInt32();
        if (size < 0 || size >= DEVICE_CHANGE_VALID_SIZE) {
            delete event;
            return nullptr;
        }
        for (int32_t i = 0; i < size; i++) {
            auto device = AudioDeviceDescriptor::Unmarshalling(parcel);
            if (device != nullptr) {
                event->devices.emplace_back(std::shared_ptr<AudioDeviceDescriptor>(device));
            }
        }
        return event;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SESSION_DEVICE_INFO_H
