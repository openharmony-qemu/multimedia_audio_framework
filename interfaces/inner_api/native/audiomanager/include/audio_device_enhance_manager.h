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

#ifndef AUDIO_DEVICE_ENHANCE_MANAGER_H
#define AUDIO_DEVICE_ENHANCE_MANAGER_H

#include <memory>
#include <vector>

#include "audio_device_descriptor.h"
#include "audio_policy_interface.h"
#include "parcel.h"

namespace OHOS {
namespace AudioStandard {
class AudioCapturer;
class AudioRenderer;

enum SoundCardType {
    ONBOARD = 0,
    USB = 1
};

struct SoundCardInfo : public Parcelable {
    std::string cardId;
    std::string name;
    std::string vendor;
    std::string model;
    std::string busAddress;
    std::string driver;
    SoundCardType cardType;

    SoundCardInfo() : cardId(""), name(""), vendor(""), model(""), busAddress(""), driver(""), cardType(ONBOARD) {}

    bool IsEmpty() const
    {
        return cardId.empty() && name.empty() && vendor.empty() && model.empty() &&
            busAddress.empty() && driver.empty();
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(cardId) &&
            parcel.WriteString(name) &&
            parcel.WriteString(vendor) &&
            parcel.WriteString(model) &&
            parcel.WriteString(busAddress) &&
            parcel.WriteString(driver) &&
            parcel.WriteInt32(static_cast<int32_t>(cardType));
    }

    static SoundCardInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) SoundCardInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->cardId = parcel.ReadString();
        info->name = parcel.ReadString();
        info->vendor = parcel.ReadString();
        info->model = parcel.ReadString();
        info->busAddress = parcel.ReadString();
        info->driver = parcel.ReadString();
        info->cardType = static_cast<SoundCardType>(parcel.ReadInt32());
        return info;
    }
};
class AudioDeviceEnhanceManager {
public:
    static AudioDeviceEnhanceManager &GetInstance();

    int32_t IsEnhancedRoutingSupported(bool &supported);
    int32_t SelectOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    int32_t SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    int32_t SelectInputDeviceForAudioCapturer(std::shared_ptr<AudioCapturer> &capturer,
        const std::shared_ptr<AudioDeviceDescriptor> &desc) const;
    int32_t SelectOutputDeviceForAudioRenderer(std::shared_ptr<AudioRenderer> &renderer,
        const std::shared_ptr<AudioDeviceDescriptor> &desc) const;
    int32_t GetSoundCardInfo(SoundCardInfo &soundCardInfo) const;
    int32_t GetSoundCardInfoList(std::vector<SoundCardInfo> &soundCardList) const;
private:
    int32_t GetOnboardSoundCardInfo(SoundCardInfo &onboardInfo) const;
    int32_t GetUsbSoundCardInfo(std::vector<SoundCardInfo> &soundCardList) const;
    std::string BuildOnboardCardId(const std::string &vendor, const std::string &devPath) const;
    AudioDeviceEnhanceManager() = default;
    ~AudioDeviceEnhanceManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEVICE_ENHANCE_MANAGER_H
