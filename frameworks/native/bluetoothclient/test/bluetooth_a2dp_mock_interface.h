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

#ifndef BLUETOOTH_A2DP_MOCK_INTERFACE_H
#define BLUETOOTH_A2DP_MOCK_INTERFACE_H

#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "bluetooth_a2dp_interface.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothA2dpMockInterface : public BluetoothA2dpInterface {
public:
    BluetoothA2dpMockInterface() {}
    ~BluetoothA2dpMockInterface() = default;

    MOCK_METHOD2(GetDevicesByStates, int32_t(const std::vector<int> &, std::vector<BluetoothRemoteDevice> &));
    MOCK_METHOD4(GetRenderPosition, int32_t(const BluetoothRemoteDevice &, uint32_t &, uint64_t &, uint32_t &));
    MOCK_METHOD2(OffloadStopPlaying, int32_t(const BluetoothRemoteDevice &, const std::vector<int32_t> &));
    MOCK_METHOD2(OffloadStartPlaying, int32_t(const BluetoothRemoteDevice &, const std::vector<int32_t> &));
    MOCK_METHOD2(A2dpOffloadSessionRequest, int32_t(const BluetoothRemoteDevice &,
        const std::vector<A2dpStreamInfo> &));
    MOCK_METHOD1(SetActiveSinkDevice, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(Connect, int32_t(const BluetoothRemoteDevice &));
    MOCK_METHOD1(RegisterObserver, void(std::shared_ptr<A2dpSourceObserver>));
    MOCK_METHOD1(DeregisterObserver, void(std::shared_ptr<A2dpSourceObserver>));
    MOCK_METHOD1(GetVirtualDeviceList, void(std::vector<std::string> &));
    MOCK_METHOD1(GetCodecStatus, A2dpCodecStatus(const BluetoothRemoteDevice &));
    MOCK_METHOD0(GetActiveA2dpDevice, std::string());

    int32_t GetLastError() override
    {
        return 0;
    }

    std::string GetLastOperation() override
    {
        return "";
    }

    static std::shared_ptr<BluetoothA2dpMockInterface> mockInterface_;
};

std::shared_ptr<BluetoothA2dpMockInterface> BluetoothA2dpMockInterface::mockInterface_ = nullptr;

BluetoothA2dpInterface &BluetoothA2dpInterface::GetInstance()
{
    if (BluetoothA2dpMockInterface::mockInterface_ == nullptr) {
        static BluetoothA2dpMockInterface defaultInterface;
        return defaultInterface;
    }
    return *(BluetoothA2dpMockInterface::mockInterface_.get());
}
}
}
#endif