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

#ifndef BLUETOOTH_A2DP_INTERFACE_H
#define BLUETOOTH_A2DP_INTERFACE_H

#include <string>
#include <memory>
#include <vector>
#include "bluetooth_a2dp_src.h"
#include "bluetooth_device_utils.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothA2dpInterface {
public:
    static BluetoothA2dpInterface &GetInstance();

    virtual int32_t GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) = 0;
    virtual int32_t GetRenderPosition(const BluetoothRemoteDevice &device, uint32_t &delayValue, uint64_t &sendDataSize,
        uint32_t &timestamp) = 0;
    virtual int32_t OffloadStopPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionId) = 0;
    virtual int32_t OffloadStartPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionId) = 0;
    virtual int32_t A2dpOffloadSessionRequest(const BluetoothRemoteDevice &device,
        const std::vector<A2dpStreamInfo> &info) = 0;
    virtual int32_t SetActiveSinkDevice(const BluetoothRemoteDevice &device) = 0;
    virtual int32_t Connect(const BluetoothRemoteDevice &device) = 0;
    virtual void RegisterObserver(std::shared_ptr<A2dpSourceObserver> observer) = 0;
    virtual void DeregisterObserver(std::shared_ptr<A2dpSourceObserver> observer) = 0;
    virtual void GetVirtualDeviceList(std::vector<std::string> &devices) = 0;
    virtual A2dpCodecStatus GetCodecStatus(const BluetoothRemoteDevice &device) = 0;
    virtual std::string GetActiveA2dpDevice() = 0;

    virtual int32_t GetLastError() = 0;
    virtual std::string GetLastOperation() = 0;

protected:
    BluetoothA2dpInterface() {}
    virtual ~BluetoothA2dpInterface() {}
};
}
}
#endif