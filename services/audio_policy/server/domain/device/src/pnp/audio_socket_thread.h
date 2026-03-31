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
#ifndef ST_AUDIO_SOCKET_THREAD_H
#define ST_AUDIO_SOCKET_THREAD_H

#include <memory>
#include <string>

#include "hdf_device_desc.h"
#include "hdf_types.h"
#include "v6_0/audio_types.h"
#include "audio_pnp_param.h"
#include "audio_pnp_server.h"
#include <mutex>

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioSocketThread {
public:
    static bool IsUpdatePnpDeviceState(AudioEvent *pnpDeviceEvent);
    static void UpdatePnpDeviceState(AudioEvent *pnpDeviceEvent);
    static int AudioPnpUeventOpen(int *fd);
    static ssize_t AudioPnpReadUeventMsg(int sockFd, char *buffer, size_t length);
    static bool AudioPnpUeventParse(const char *msg, const ssize_t strLength);
    static void UpdateDeviceState(AudioEvent audioEvent);
    static int32_t DetectAnalogHeadsetState(AudioEvent *audioEvent);
    static int32_t DetectPostAudioDevices(AudioEvent *audioEvent);
    static void SetAudioPnpUevent(AudioEvent *audioEvent, char switchState);
    static int32_t DetectDPState(AudioEvent *audioEvent);
    static AudioEvent audioSocketEvent_;

private:
    static int32_t ProcessSwitchDeviceConfig(AudioEvent *audioEvent, const char *switchName,
        char switchState, const char *devName, const char *eventName);
    static int32_t SetAudioPnpServerEventValue(AudioEvent *audioEvent, const struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioAnalogHeadsetDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioMicInDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioLineInDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioLineOutDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t DetectSwitchDeviceState(const char* switchPath, const char* switchName,
        AudioEvent* audioEvent, const char* logPrefix);
    static int32_t AudioDpDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static std::mutex eventMutex_;
#ifdef USB_ENABLE
    static int32_t AudioDetectUsbSoundCard(const AudioPnpUevent &audioPnpUevent);
#endif
    static int32_t AudioAnahsDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioHDMIDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t SetAudioAnahsEventValue(AudioEvent *audioEvent, const struct AudioPnpUevent *audioPnpUevent);
    static int32_t ReadAndScanDpState(const std::string &path, uint32_t &eventType);
    static int32_t ReadAndScanDpName(const std::string &path, std::string &name);
    static int32_t AudioNnDetectDevice(const struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioMicBlockDevice(const struct AudioPnpUevent *audioPnpUevent);
    static bool ProcessSwitchSubsystem(const struct AudioPnpUevent *uevent);
    static bool ProcessOtherSubsystem(const struct AudioPnpUevent *uevent);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SOCKET_THREAD_H