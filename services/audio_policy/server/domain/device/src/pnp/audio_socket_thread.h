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
#include "v5_0/audio_types.h"
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
    static int32_t DetectAnalogHeadsetState(AudioEvent *audioEvent);
    static int32_t DetectDPState(AudioEvent *audioEvent);
    static void SetAudioPnpUevent(AudioEvent *audioEvent, char switchState);
    static AudioEvent audioSocketEvent_;

private:
    static int32_t SetAudioPnpServerEventValue(AudioEvent *audioEvent, struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioAnalogHeadsetDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioDpDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioAnahsDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioHDMIDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static int32_t SetAudioAnahsEventValue(AudioEvent *audioEvent, struct AudioPnpUevent *audioPnpUevent);
    static int32_t ReadAndScanDpState(const std::string &path, uint32_t &eventType);
    static int32_t ReadAndScanDpName(const std::string &path, std::string &name);
    static int32_t AudioNnDetectDevice(struct AudioPnpUevent *audioPnpUevent);
    static int32_t AudioMicBlockDevice(struct AudioPnpUevent *audioPnpUevent);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SOCKET_THREAD_H