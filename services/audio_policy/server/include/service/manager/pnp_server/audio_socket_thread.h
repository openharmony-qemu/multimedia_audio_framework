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
#include "v4_0/audio_types.h"
#include "audio_pnp_param.h"

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
    static int32_t DetectUsbHeadsetState(AudioEvent *audioEvent);
    static int32_t DetectAnalogHeadsetState(AudioEvent *audioEvent);
    static int32_t DetectDPState(AudioEvent *audioEvent);
    static AudioEvent audioSocketEvent_;

private:
    static int32_t SetAudioPnpServerEventValue(AudioEvent *audioEvent, struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioAnalogHeadsetDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static int32_t CheckUsbDesc(struct UsbDevice *usbDevice);
    static int32_t ReadAndScanUsbDev(const char *devPath);
    static bool FindAudioUsbDevice(const char *devName);
    static bool AddAudioUsbDevice(const char *devName);
    static bool CheckAudioUsbDevice(const char *devName);
    static bool DeleteAudioUsbDevice(const char *devName);
    static int32_t AudioUsbHeadsetDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static inline bool IsBadName(const char *name);
    static int32_t ScanUsbBusSubDir(const char *subDir);
    static int32_t AudioDpDetectDevice(struct AudioPnpUevent *audioPnpUevent);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SOCKET_THREAD_H