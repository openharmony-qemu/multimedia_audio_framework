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
    MOCK_METHOD1(GetScoState, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(ConnectSco, int32_t(uint8_t));
    MOCK_METHOD1(DisconnectSco, int32_t(uint8_t));
    MOCK_METHOD1(OpenVoiceRecognition, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(CloseVoiceRecognition, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(SetActiveDevice, int32_t(const BluetoothRemoteDevice &));

    static BluetoothHfpMockInterface mockInterface_;
};

BluetoothHfpMockInterface BluetoothHfpMockInterface::mockInterface_;

BluetoothHfpInterface &BluetoothHfpInterface::GetInstance()
{
    return BluetoothHfpMockInterface::mockInterface_;
}
}
}
#endif