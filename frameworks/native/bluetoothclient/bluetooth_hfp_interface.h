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

#ifndef BLUETOOTH_HFP_INTERFACE_H
#define BLUETOOTH_HFP_INTERFACE_H

#include <string>
#include <memory>
#include <vector>
#include "bluetooth_hfp_ag.h"
#include "bluetooth_device_utils.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHfpInterface {
public:
    static BluetoothHfpInterface &GetInstance();

    virtual int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) = 0;
    virtual AudioScoState GetScoState(const BluetoothRemoteDevice &device) = 0;
    virtual int32_t GetCurrentCategory(ScoCategory &category) = 0;
    virtual int32_t ConnectSco(uint8_t callType) = 0;
    virtual int32_t DisconnectSco(uint8_t callType) = 0;
    virtual int32_t OpenVoiceRecognition(const BluetoothRemoteDevice &device) = 0;
    virtual int32_t CloseVoiceRecognition(const BluetoothRemoteDevice &device) = 0;
    virtual int32_t SetActiveDevice(const BluetoothRemoteDevice &device) = 0;

    virtual void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer) = 0;
    virtual void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer) = 0;
    virtual std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states) = 0;
    virtual void GetVirtualDeviceList(std::vector<std::string> &devices) = 0;
    virtual BluetoothRemoteDevice GetActiveDevice() = 0;
    virtual int32_t Connect(const BluetoothRemoteDevice &device) = 0;
    virtual int32_t IsInbandRingingEnabled(bool &isEnabled) = 0;
    virtual int32_t GetLastError() = 0;
    virtual std::string GetLastOpration() = 0;

protected:
    BluetoothHfpInterface() {}
    virtual ~BluetoothHfpInterface() {}
};
}
}
#endif