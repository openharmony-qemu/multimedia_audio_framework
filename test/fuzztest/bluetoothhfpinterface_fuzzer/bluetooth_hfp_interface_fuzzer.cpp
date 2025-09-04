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

#include <iostream>
#include "bluetooth_hfp_interface.h"
#include "audio_info.h"
#include "audio_engine_log.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace Bluetooth;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const int32_t TRANSPORT = 2;
const string DEFAULT_BLUETOOTH_MAC_ADDRESS = "00:11:22:33:44:55";

typedef void (*TestFuncs)();

void GetDeviceStateFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    BluetoothHfpInterface::GetInstance().GetDeviceState(device, state);
}

void OpenVoiceRecognitionFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    BluetoothHfpInterface::GetInstance().OpenVoiceRecognition(device);
}

void CloseVoiceRecognitionFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    BluetoothHfpInterface::GetInstance().CloseVoiceRecognition(device);
}

void ConnectFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    BluetoothHfpInterface::GetInstance().Connect(device);
}

void IsInbandRingingEnabledFuzzTest()
{
    bool isEnabled = g_fuzzUtils.GetData<bool>();
    BluetoothHfpInterface::GetInstance().IsInbandRingingEnabled(isEnabled);
}

void GetLastErrorFuzzTest()
{
    BluetoothHfpInterface::GetInstance().GetLastError();
}

void GetLastOprationFuzzTest()
{
    BluetoothHfpInterface::GetInstance().GetLastOpration();
}

vector<TestFuncs> g_testFuncs = {
    GetDeviceStateFuzzTest,
    OpenVoiceRecognitionFuzzTest,
    CloseVoiceRecognitionFuzzTest,
    ConnectFuzzTest,
    IsInbandRingingEnabledFuzzTest,
    GetLastErrorFuzzTest,
    GetLastOprationFuzzTest
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}