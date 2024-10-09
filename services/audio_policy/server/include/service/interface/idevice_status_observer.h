/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ST_DEVICE_STATUS_OBSERVER_H
#define ST_DEVICE_STATUS_OBSERVER_H

#include "audio_info.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class IDeviceStatusObserver {
public:
    virtual void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo) = 0;
    virtual void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected) = 0;
    virtual void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &name, const std::string &adderess) = 0;
    virtual void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo) = 0;
    virtual void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false) = 0;
    virtual void OnServiceConnected(AudioServiceIndex serviceIndex) = 0;
    virtual void OnServiceDisconnected(AudioServiceIndex serviceIndex) = 0;
    virtual void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress) = 0;
    virtual void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) = 0;
    virtual void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
