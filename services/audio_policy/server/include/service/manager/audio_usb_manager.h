/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef AUDIO_USB_MANAGER_H
#define AUDIO_USB_MANAGER_H

#include <mutex>
#include <map>
#include <vector>
#include "common_event_subscriber.h"
#include "idevice_status_observer.h"

namespace OHOS {
namespace AudioStandard {

using namespace std;

string EncUsbAddr(const string &src);

struct UsbAddr {
    uint8_t busNum_{0};
    uint8_t devAddr_{0};
    inline bool operator==(const UsbAddr &o) const
    {
        return busNum_ == o.busNum_ && devAddr_ == o.devAddr_;
    }
    inline bool operator<(const UsbAddr &o) const
    {
        return busNum_ < o.busNum_ || (busNum_ == o.busNum_ && devAddr_ < o.devAddr_);
    }
};

struct SoundCard {
    uint32_t cardNum_{0};
    string usbBus_;
    bool isCapturer_{false};
    bool isPlayer_{false};
};

struct UsbAudioDevice {
    UsbAddr usbAddr_;
    string name_;
    inline bool operator==(const UsbAudioDevice &o) const
    {
        return usbAddr_ == o.usbAddr_;
    }
};

class AudioUsbManager {
public:
    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        explicit EventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
            : EventFwk::CommonEventSubscriber(subscribeInfo) {}
        void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    };

    static AudioUsbManager& GetInstance();
    static map<UsbAddr, SoundCard> GetUsbSoundCardMap();
    static vector<UsbAudioDevice> GetUsbAudioDevices();
    static vector<UsbAudioDevice> GetPlayerDevices();
    static vector<UsbAudioDevice> GetCapturerDevices();

    void Init(IDeviceStatusObserver *observer);
    void Deinit();
    void SubscribeEvent();

private:
    void HandleUsbAudioDeviceAttach(const UsbAudioDevice &device);
    void HandleUsbAudioDeviceDetach(const UsbAudioDevice &device);
    void RefreshUsbAudioDevices();
    void NotifyDevice(const UsbAudioDevice &device, const bool isConnected);

    vector<UsbAudioDevice> audioDevices_;
    map<UsbAddr, SoundCard> soundCardMap_;
    shared_ptr<EventSubscriber> eventSubscriber_{nullptr};
    bool initialized_{false};

    mutex mutex_;
    IDeviceStatusObserver *observer_{nullptr};
};

} // namespace AudioStandard
} // namespace OHOS

#endif