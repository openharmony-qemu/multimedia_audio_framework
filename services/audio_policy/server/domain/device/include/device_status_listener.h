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

#ifndef ST_DEVICE_STATUS_LISTENER_H
#define ST_DEVICE_STATUS_LISTENER_H

#include <servmgr_hdi.h>

#include "audio_adapter_info.h"
#include "idevice_status_observer.h"
#include "audio_pnp_server.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "istandard_audio_routing_manager_listener.h"
#include "istandard_audio_anahs_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioPnpStatusCallback;
class DeviceStatusListener {
public:
    DeviceStatusListener(IDeviceStatusObserver &observer);
    ~DeviceStatusListener();

    int32_t RegisterDeviceStatusListener();
    int32_t UnRegisterDeviceStatusListener();

    IDeviceStatusObserver &deviceObserver_;
    void OnPnpDeviceStatusChanged(const std::string &info);
    void OnMicrophoneBlocked(const std::string &info);

    int32_t SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object);
    int32_t UnsetAudioDeviceAnahsCallback();
    void UpdateAnahsPlatformType(std::string anahsShowType);

private:
#ifdef AUDIO_WIRED_DETECT
    AudioPnpServer *audioPnpServer_;
    std::shared_ptr<AudioPnpStatusCallback> pnpDeviceCB_ = nullptr;
#endif
    struct HDIServiceManager *hdiServiceManager_;
    struct ServiceStatusListener *listener_;
    sptr<IStandardAudioAnahsManagerListener> audioDeviceAnahsCb_;
    std::string anahsShowType_ = "Dialog";
};

#ifdef AUDIO_WIRED_DETECT
class AudioPnpStatusCallback : public AudioPnpDeviceChangeCallback {
public:
    AudioPnpStatusCallback();

    virtual ~AudioPnpStatusCallback();

    void OnPnpDeviceStatusChanged(const std::string &info);

    void OnMicrophoneBlocked(const std::string &info);

    void SetDeviceStatusListener(DeviceStatusListener *listener);
private:
    DeviceStatusListener *listener_ = nullptr;
};
#endif
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DEVICE_STATUS_LISTENER_H
