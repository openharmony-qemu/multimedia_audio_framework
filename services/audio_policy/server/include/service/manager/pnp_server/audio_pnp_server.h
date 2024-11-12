/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_PNP_SERVER_H
#define ST_AUDIO_PNP_SERVER_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "hdf_types.h"
#include "audio_info.h"
#include "audio_pnp_param.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
class MicrophoneBlocked;

class AudioPnpServer {
public:
    static AudioPnpServer& GetAudioPnpServer()
    {
        static AudioPnpServer audioPnpServer;
        return audioPnpServer;
    }
    ~AudioPnpServer();
    bool init(void);
    int32_t RegisterPnpStatusListener(std::shared_ptr<AudioPnpDeviceChangeCallback> callback);
    int32_t UnRegisterPnpStatusListener();
    void OnPnpDeviceStatusChanged(const std::string &info);
    void StopPnpServer();
    friend class MicrophoneBlocked;
    void OnMicrophoneBlocked(const std::string &info, AudioPnpServer &audioPnpServer);

private:
    std::string eventInfo_;
    std::mutex pnpMutex_;
    std::shared_ptr<AudioPnpDeviceChangeCallback> pnpCallback_ = nullptr;
    std::unique_ptr<std::thread> socketThread_ = nullptr;
    std::unique_ptr<std::thread> inputThread_ = nullptr;
    void OpenAndReadWithSocket();
    void OpenAndReadInput();
    void DetectAudioDevice();
#ifdef AUDIO_DOUBLE_PNP_DETECT
    void UpdateUsbHeadset();
#endif
};

class MicrophoneBlocked {
public:
    static MicrophoneBlocked &GetInstance()
    {
        static MicrophoneBlocked instance_;
        return instance_;
    }
    MicrophoneBlocked() {}
    ~MicrophoneBlocked() {}
    void OnMicrophoneBlocked(const std::string &info, AudioPnpServer &audioPnpServer);
};

enum PnpEventType {
    PNP_EVENT_DEVICE_ADD = 1,
    PNP_EVENT_DEVICE_REMOVE = 2,
    PNP_EVENT_LOAD_SUCCESS = 3,
    PNP_EVENT_LOAD_FAILURE = 4,
    PNP_EVENT_UNLOAD = 5,
    PNP_EVENT_SERVICE_VALID = 7,
    PNP_EVENT_SERVICE_INVALID  = 8,
    PNP_EVENT_CAPTURE_THRESHOLD = 9,
    PNP_EVENT_UNKNOWN = 10,
    PNP_EVENT_MIC_BLOCKED = 11,
    PNP_EVENT_MIC_UNBLOCKED = 12,
};

enum PnpDeviceType {
    PNP_DEVICE_LINEOUT = 1 << 0,
    PNP_DEVICE_HEADPHONE = 1 << 1,
    PNP_DEVICE_HEADSET = 1 << 2,
    PNP_DEVICE_USB_HEADSET = 1 << 3,
    PNP_DEVICE_USB_HEADPHONE = 1 << 4,
    PNP_DEVICE_USBA_HEADSET = 1 << 5,
    PNP_DEVICE_USBA_HEADPHONE = 1 << 6,
    PNP_DEVICE_PRIMARY_DEVICE = 1 << 7,
    PNP_DEVICE_USB_DEVICE = 1 << 8,
    PNP_DEVICE_A2DP_DEVICE = 1 << 9,
    PNP_DEVICE_HDMI_DEVICE = 1 << 10,
    PNP_DEVICE_ADAPTER_DEVICE = 1 << 11,
    PNP_DEVICE_DP_DEVICE = 1 << 12,
    PNP_DEVICE_MIC = 1 << 13,
    PNP_DEVICE_UNKNOWN,
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_PNP_SERVER_H