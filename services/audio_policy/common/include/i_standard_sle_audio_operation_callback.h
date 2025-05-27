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

#ifndef I_STANDARD_SLE_AUDIO_OPERATION_CALLBACK_H
#define I_STANDARD_SLE_AUDIO_OPERATION_CALLBACK_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AudioStandard {
class IStandardSleAudioOperationCallback : public IRemoteBroker {
public:
    virtual ~IStandardSleAudioOperationCallback() = default;

    virtual void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) = 0;
    virtual void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) = 0;
    virtual bool IsInBandRingOpen(const std::string &device) = 0;
    virtual uint32_t GetSupportStreamType(const std::string &device) = 0;
    virtual int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) = 0;
    virtual int32_t StartPlaying(const std::string &device, uint32_t streamType) = 0;
    virtual int32_t StopPlaying(const std::string &device, uint32_t streamType) = 0;
    virtual int32_t ConnectAllowedProfiles(const std::string &remoteAddr) = 0;
    virtual int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType) = 0;
    virtual int32_t SendUserSelection(const std::string &device, uint32_t streamType) = 0;

    enum AudioSleAudioOperationCallbackMsg {
        ON_ERROR = 0,
        GET_SLE_AUDIO_DEVICE_LIST,
        GET_SLE_VIRTUAL_AUDIO_DEVICE_LIST,
        IS_IN_BAND_RING_OPEN,
        GET_SUPPORT_STREAM_TYPE,
        SET_ACTIVE_SINK_DEVICE,
        START_PLAYING,
        STOP_PLAYING,
        CONNECT_ALLOWED_PROFILES,
        SET_DEVICE_ABS_VOLUME,
        SEND_USER_SELECTION,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardSleAudioOperationCallback");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_STANDARD_SLE_AUDIO_OPERATION_CALLBACK_H
