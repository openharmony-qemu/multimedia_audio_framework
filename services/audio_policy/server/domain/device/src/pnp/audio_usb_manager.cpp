/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioUsbManager"
#endif

#include "audio_usb_manager.h"

#include <sstream>
#include <dirent.h>
#include <fstream>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "usb_srv_client.h"

#include "audio_core_service.h"

namespace OHOS {
namespace AudioStandard {

using namespace USB;

static string ReadTextFile(const string &file)
{
    string ret;
    ifstream fin;
    fin.open(file.c_str(), ios::binary | ios::in);
    if (fin) {
        int val;
        while ((val = fin.get()) != EOF) {
            ret.push_back(static_cast<char>(val));
        }
        fin.close();
    }
    return ret;
}

static void FillSoundCard(const string &path, SoundCard &card)
{
    DIR *dir = opendir(path.c_str());
    CHECK_AND_RETURN_RET(dir != nullptr,);
    struct dirent *tmp;
    while ((tmp = readdir(dir)) != nullptr) {
        string file(tmp->d_name);
        if (file == "usbbus") {
            card.usbBus_ = ReadTextFile(path + "/" + file);
            continue;
        } else if (file.find("pcm", 0) == 0) {
            if (file.back() == 'c') {
                card.isCapturer_ = true;
            } else if (file.back() == 'p') {
                card.isPlayer_ = true;
            }
        }
    }
    closedir(dir);
}

static string Trim(const string &str)
{
    static const set<char> WHITE_SPACE{' ', '\r', '\n', '\t'};
    size_t pos = 0;
    size_t end = str.length();
    for (; pos < end; pos++) {
        if (WHITE_SPACE.find(str[pos]) == WHITE_SPACE.end()) {
            break;
        }
    }
    for (; end > pos; end--) {
        if (WHITE_SPACE.find(str[end - 1]) == WHITE_SPACE.end()) {
            break;
        }
    }
    return str.substr(pos, end - pos);
}

static vector<SoundCard> GetUsbSoundCards()
{
    const string baseDir{"/proc/asound"};
    const string card{"card"};
    vector<SoundCard> soundCards;
    DIR *dir = opendir(baseDir.c_str());
    CHECK_AND_RETURN_RET(dir != nullptr, soundCards);
    struct dirent *tmp;
    int cardNum;
    while ((tmp = readdir(dir)) != nullptr) {
        string file(tmp->d_name);
        if (file.length() <= card.length() || !(file.find(card, 0) == 0)) {continue;}
        string cardNumStr = file.substr(card.length());
        if (!StrToInt(cardNumStr, cardNum)) {continue;}
        SoundCard card = {.cardNum_ = static_cast<uint32_t>(cardNum)};
        FillSoundCard(baseDir + "/" + file, card);
        if (card.usbBus_.empty()) {continue;}
        soundCards.push_back(card);
    }
    closedir(dir);
    return soundCards;
}

static string GetDeviceAddr(const uint32_t cardNum)
{
    ostringstream oss;
    oss << "card=" << cardNum << ";device=0";
    return oss.str();
}

static UsbAddr GetUsbAddr(const SoundCard &card)
{
    size_t pos = card.usbBus_.find('/');
    CHECK_AND_RETURN_RET_LOG(pos != string::npos, {}, "Error Parameter: card.usbbus");
    int busNum;
    int devAddr;
    string busNumStr = Trim(card.usbBus_.substr(0, pos));
    string devAddrStr = Trim(card.usbBus_.substr(pos + 1));
    CHECK_AND_RETURN_RET_LOG(StrToInt(busNumStr, busNum) && StrToInt(devAddrStr, devAddr), {}, "StrToInt ERROR");
    return {static_cast<uint8_t>(busNum), static_cast<uint8_t>(devAddr)};
}

static bool IsAudioDevice(USB::UsbDevice &usbDevice)
{
    for (auto &usbConfig : usbDevice.GetConfigs()) {
        for (auto &usbInterface : usbConfig.GetInterfaces()) {
            if (usbInterface.GetClass() == 1 && usbInterface.GetSubClass() == 1) {
                return true;
            }
        }
    }
    return false;
}

static shared_ptr<AudioUsbManager::EventSubscriber> SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    auto subscriber = make_shared<AudioUsbManager::EventSubscriber>(subscribeInfo);
    auto ret = EventFwk::CommonEventManager::NewSubscribeCommonEvent(subscriber);
    CHECK_AND_RETURN_RET_LOG(ret == ERR_OK, nullptr, "NewSubscribeCommonEvent Failed. ret=%{public}d", ret);
    return subscriber;
}

static bool NotSameSoundCard(const UsbAudioDevice &dev1, const UsbAudioDevice &dev2)
{
    return dev1.cardNum_ != dev2.cardNum_ || dev1.isCapturer_ != dev2.isCapturer_ || dev1.isPlayer_ != dev2.isPlayer_;
}

string EncUsbAddr(const string &src)
{
    const string head("card=");
    auto pos = src.find(';', head.length());
    CHECK_AND_RETURN_RET_LOG(pos != string::npos, "", "Illegal usb address");
    auto num = src.substr(head.length(), pos - head.length());
    return string("c**") + num + "**";
}

AudioUsbManager &AudioUsbManager::GetInstance()
{
    static AudioUsbManager sManager;
    return sManager;
}

void AudioUsbManager::Init(std::shared_ptr<IDeviceStatusObserver> observer)
{
    lock_guard<mutex> lock(mutex_);
    if (!initialized_) {
#ifdef DETECT_SOUNDBOX
        AUDIO_INFO_LOG("Entry. DETECT_SOUNDBOX=true");
#else
        AUDIO_INFO_LOG("Entry. DETECT_SOUNDBOX=false");
#endif
        observer_ = observer;
        RefreshUsbAudioDevices();
        initialized_ = true;
    }
}

void AudioUsbManager::Deinit()
{
    lock_guard<mutex> lock(mutex_);
    if (initialized_) {
        if (eventSubscriber_) {
            EventFwk::CommonEventManager::NewUnSubscribeCommonEvent(eventSubscriber_);
            eventSubscriber_.reset();
        }
        audioDevices_.clear();
        soundCardMap_.clear();
        initialized_ = false;
    }
}

void AudioUsbManager::RefreshUsbAudioDevices()
{
    vector<UsbAudioDevice> devices;
    auto ret = GetUsbAudioDevices(devices);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "GetUsbAudioDevices Failed. ret=%{public}d", ret);
    vector<UsbAudioDevice> toAdd;
    for (auto &device : devices) {
        auto it = find_if(audioDevices_.cbegin(), audioDevices_.cend(), [&device](auto &item) {
            return device.usbAddr_ == item.usbAddr_ && device.name_ == item.name_;
        });
        if (it == audioDevices_.cend()) {
            toAdd.push_back(device);
        }
    }
    CHECK_AND_RETURN_RET(!toAdd.empty(),);
    soundCardMap_ = GetUsbSoundCardMap();
    for (auto &device : toAdd) {
        if (!FillUsbAudioDevice(device)) { continue; }
        audioDevices_.push_back(device);
        NotifyDevice(device, true);
    }
}

void AudioUsbManager::SubscribeEvent()
{
    AUDIO_INFO_LOG("Entry");
    CHECK_AND_RETURN_LOG(eventSubscriber_ == nullptr, "feventSubscriber_ already exists");
    eventSubscriber_ = SubscribeCommonEvent();
    lock_guard<mutex> lock(mutex_);
    RefreshUsbAudioDevices();
}

void AudioUsbManager::NotifyDevice(const UsbAudioDevice &device, const bool isConnected)
{
    DeviceType devType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    string macAddress = GetDeviceAddr(device.cardNum_);
    AudioStreamInfo streamInfo{};
    string deviceName = device.name_ + "-" + to_string(device.cardNum_);
    if (device.isPlayer_) {
        AUDIO_INFO_LOG("Usb out, devType=%{public}d, isConnected=%{public}d, "
            "macAddress=%{public}s, deviceName=%{public}s, role=%{public}d", devType, isConnected,
            EncUsbAddr(macAddress).c_str(), deviceName.c_str(), DeviceRole::OUTPUT_DEVICE);
        CHECK_AND_RETURN_LOG(observer_ != nullptr, "observer is null");
        observer_->OnDeviceStatusUpdated(devType, isConnected, macAddress,
            deviceName, streamInfo, OUTPUT_DEVICE, device.isCapturer_);
    }
    if (device.isCapturer_) {
        AUDIO_INFO_LOG("Usb in, devType=%{public}d, isConnected=%{public}d, "
            "macAddress=%{public}s, deviceName=%{public}s, role=%{public}d", devType, isConnected,
            EncUsbAddr(macAddress).c_str(), deviceName.c_str(), DeviceRole::INPUT_DEVICE);
        CHECK_AND_RETURN_LOG(observer_ != nullptr, "observer is null");
        observer_->OnDeviceStatusUpdated(devType, isConnected, macAddress,
            deviceName, streamInfo, INPUT_DEVICE, device.isPlayer_);
    }
}

map<UsbAddr, SoundCard> AudioUsbManager::GetUsbSoundCardMap()
{
    map<UsbAddr, SoundCard> cardMap;
    auto cardList = GetUsbSoundCards();
    for (auto &card : cardList) {
        cardMap[GetUsbAddr(card)] = card;
    }
    return cardMap;
}

int32_t AudioUsbManager::GetUsbAudioDevices(vector<UsbAudioDevice> &result)
{
    vector<USB::UsbDevice> deviceList;
    auto ret = UsbSrvClient::GetInstance().GetDevices(deviceList);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "GetDevices failed. ret=%{public}d. size=%{public}zu", ret, deviceList.size());
    for (auto &usbDevice : deviceList) {
        if (IsAudioDevice(usbDevice)) {
            result.push_back({
                {usbDevice.GetBusNum(), usbDevice.GetDevAddr()},
                usbDevice.GetProductName(),
            });
        }
    }
    return SUCCESS;
}

void AudioUsbManager::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    string action = data.GetWant().GetAction();
    AUDIO_INFO_LOG("OnReceiveEvent Entry. action=%{public}s", action.c_str());
    bool isAttach{false};
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED) {
        isAttach = true;
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED) {
        isAttach = false;
    } else {
        return;
    }
    string devStr = data.GetData();
    CHECK_AND_RETURN_LOG(!devStr.empty(), "Error: data.GetData() returns empty");
    auto *devJson = cJSON_Parse(devStr.c_str());
    CHECK_AND_RETURN_LOG(devJson, "Create devJson error");
    USB::UsbDevice usbDevice(devJson);
    cJSON_Delete(devJson);
    if (!IsAudioDevice(usbDevice)) {
        return;
    }
    UsbAudioDevice device = {
        {usbDevice.GetBusNum(), usbDevice.GetDevAddr()},
        usbDevice.GetProductName()
    };
    AudioUsbManager::GetInstance().HandleAudioDeviceEvent(make_pair(device, isAttach));
}

void AudioUsbManager::HandleAudioDeviceEvent(pair<UsbAudioDevice, bool> &&p)
{
    AUDIO_INFO_LOG("Entry. deviceName=%{public}s, busNum=%{public}d, devAddr=%{public}d, isAttach=%{public}d",
        p.first.name_.c_str(), p.first.usbAddr_.busNum_, p.first.usbAddr_.devAddr_, p.second);
    lock_guard<mutex> lock(mutex_);
    auto it = find(audioDevices_.begin(), audioDevices_.end(), p.first);
    if (p.second) {
        soundCardMap_ = GetUsbSoundCardMap();
        CHECK_AND_RETURN_LOG(FillUsbAudioDevice(p.first), "Error: FillUsbAudioDevice Failed");
        UpdateDevice(p.first, it);
        NotifyDevice(p.first, true);
    } else {
        CHECK_AND_RETURN_LOG(it != audioDevices_.end(), "Detached Device does not exist");
        NotifyDevice(*it, false);
        audioDevices_.erase(it);
    }
}

bool AudioUsbManager::FillUsbAudioDevice(UsbAudioDevice &device)
{
    auto it = soundCardMap_.find(device.usbAddr_);
    CHECK_AND_RETURN_RET_LOG(it != soundCardMap_.end(), false,
        "Error: No sound card matches usb device[%{public}s]", device.name_.c_str());
    auto &card = it->second;
    CHECK_AND_RETURN_RET_LOG(card.isPlayer_ || card.isCapturer_, false,
        "Error: Sound card[%{public}d] is not player and not capturer", card.cardNum_);
    device.cardNum_ = card.cardNum_;
    device.isCapturer_ = card.isCapturer_;
    device.isPlayer_ = card.isPlayer_;
    return true;
}

void AudioUsbManager::UpdateDevice(const UsbAudioDevice &dev, std::__wrap_iter<UsbAudioDevice *> &it)
{
    if (it != audioDevices_.end()) {
        if (NotSameSoundCard(dev, *it)) {
            NotifyDevice(*it, false);
        }
        *it = dev;
    } else {
        audioDevices_.push_back(dev);
    }
}
} // namespace AudioStandard
} // namespace OHOS