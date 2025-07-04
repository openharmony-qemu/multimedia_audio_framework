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

#ifndef BLUETOOTH_HFP_MOCK_INTERFACE_H
#define BLUETOOTH_HFP_MOCK_INTERFACE_H

#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "bluetooth_hfp_interface.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHfpMockInterface : public BluetoothHfpInterface {
public:
    BluetoothHfpMockInterface() {}
    ~BluetoothHfpMockInterface() = default;

    MOCK_METHOD2(GetDeviceState, int32_t(const BluetoothRemoteDevice &, int32_t &));
    MOCK_METHOD1(GetScoState, AudioScoState(const BluetoothRemoteDevice &));
    MOCK_METHOD1(GetCurrentCategory, int32_t(ScoCategory &));
    MOCK_METHOD1(ConnectSco, int32_t(uint8_t));
    MOCK_METHOD1(DisconnectSco, int32_t(uint8_t));
    MOCK_METHOD1(OpenVoiceRecognition, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(CloseVoiceRecognition, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(SetActiveDevice, int32_t(const BluetoothRemoteDevice &));

    MOCK_METHOD1(RegisterObserver, void(std::shared_ptr<HandsFreeAudioGatewayObserver>));
    MOCK_METHOD1(DeregisterObserver, void(std::shared_ptr<HandsFreeAudioGatewayObserver>));
    MOCK_METHOD1(GetDevicesByStates, std::vector<BluetoothRemoteDevice>(std::vector<int>));
    MOCK_METHOD1(GetVirtualDeviceList, void(std::vector<std::string> &));
    MOCK_METHOD0(GetActiveDevice, BluetoothRemoteDevice());
    MOCK_METHOD1(Connect, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(IsInbandRingingEnabled, int32_t(bool &));

    int32_t GetLastError() override
    {
        return 0;
    }

    std::string GetLastOpration() override
    {
        return "";
    }

    static std::shared_ptr<BluetoothHfpMockInterface> mockInterface_;
};

std::shared_ptr<BluetoothHfpMockInterface> BluetoothHfpMockInterface::mockInterface_ = nullptr;

BluetoothHfpInterface &BluetoothHfpInterface::GetInstance()
{
    if (BluetoothHfpMockInterface::mockInterface_ == nullptr) {
        static BluetoothHfpMockInterface defaultInterface;
        return defaultInterface;
    }
    return *(BluetoothHfpMockInterface::mockInterface_.get());
}
}
}
#endif