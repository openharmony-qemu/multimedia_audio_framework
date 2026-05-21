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
#include "bluetooth_device_manager.h"
#include "bluetooth_device_utils.h"
#include "audio_info.h"
#include "audio_engine_log.h"
#include "idevice_status_observer.h"
#include "bluetooth_sco_manager.h"
#include "bluetooth_host.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace Bluetooth;
 
const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const int32_t TRANSPORT = 2;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
typedef void (*TestPtr)(FuzzedDataProvider&);
 
template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

const vector<AudioScoState> g_testScoStates = {
    AudioScoState::DISCONNECTED,
    AudioScoState::CONNECTED,
    AudioScoState::CONNECTING,
    AudioScoState::DISCONNECTING,
};

const vector<HfpScoConnectState> g_testHfpScoConnectStates = {
    HfpScoConnectState::SCO_DISCONNECTED,
    HfpScoConnectState::SCO_CONNECTING,
    HfpScoConnectState::SCO_DISCONNECTING,
    HfpScoConnectState::SCO_CONNECTED,
};

const vector<ScoCategory> g_testScoCategories = {
    ScoCategory::SCO_DEFAULT,
    ScoCategory::SCO_CALLULAR,
    ScoCategory::SCO_VIRTUAL,
    ScoCategory::SCO_RECOGNITION,
};

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void UpdateScoStateWhenDisconnectedFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    HfpScoConnectState scoState = PickValue(fdp, g_testHfpScoConnectStates);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int reason = fdp.ConsumeIntegral<int>();
    scoManager.UpdateScoStateWhenDisconnected(scoState, device, reason);
    OnStop();
}
 
void UpdateScoStateWhenConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    HfpScoConnectState scoState = PickValue(fdp, g_testHfpScoConnectStates);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int reason = fdp.ConsumeIntegral<int>();
    scoManager.UpdateScoStateWhenConnected(scoState, device, reason);
    HfpScoConnectState scoState2 = PickValue(fdp, g_testHfpScoConnectStates);
    scoManager.UpdateScoStateWhenConnected(scoState2, device, reason);
    OnStop();
}
 
void UpdateScoStateWhenConnectingFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    HfpScoConnectState scoState = PickValue(fdp, g_testHfpScoConnectStates);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int reason = fdp.ConsumeIntegral<int>();
    scoManager.UpdateScoStateWhenConnecting(scoState, device, reason);
    OnStop();
}
 
void UpdateScoStateWhenDisconnectingFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    HfpScoConnectState scoState = PickValue(fdp, g_testHfpScoConnectStates);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int reason = fdp.ConsumeIntegral<int>();
    scoManager.UpdateScoStateWhenDisconnecting(scoState, device, reason);
    OnStop();
}
 
void WriteScoStateFaultEventFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    HfpScoConnectState scoState = PickValue(fdp, g_testHfpScoConnectStates);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int reason = fdp.ConsumeIntegral<int>();
    scoManager.WriteScoStateFaultEvent(scoState, device, reason);
    OnStop();
}
 
void ForceUpdateScoCategoryFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    scoManager.currentScoCategory_ = PickValue(fdp, g_testScoCategories);
    scoManager.ForceUpdateScoCategory();
    OnStop();
}
 
void ProcCacheRequestFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    scoManager.currentScoCategory_ = PickValue(fdp, g_testScoCategories);
    scoManager.ProcCacheRequest();
    OnStop();
}
 
void HandleScoConnectFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    scoManager.HandleScoConnect(category, device);
    OnStop();
}

void HandleScoDisconnectFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.currentScoState_ = PickValue(fdp, g_testScoStates);
    scoManager.HandleScoDisconnect(device);
    OnStop();
}

void HandleScoConnectNoLockFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    scoManager.HandleScoConnectNoLock(category, device);
    scoManager.currentScoState_ = PickValue(fdp, g_testScoStates);
    ScoCategory category2 = PickValue(fdp, g_testScoCategories);
    scoManager.HandleScoConnectNoLock(category2, device);
    OnStop();
}
 
void ProcConnectReqWhenDisconnectedFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    scoManager.ProcConnectReqWhenDisconnected(category, device);
    OnStop();
}
 
void ProcConnectReqWhenConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    scoManager.ProcConnectReqWhenConnected(category, device);
    OnStop();
}
 
void ProcConnectReqWhenConnectingFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    scoManager.ProcConnectReqWhenConnecting(category, device);
    OnStop();
}
 
void IsNeedSwitchScoCategoryFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    scoManager.IsNeedSwitchScoCategory(category);
    ScoCategory currentCategory = PickValue(fdp, g_testScoCategories);
    scoManager.currentScoCategory_ = currentCategory;
    scoManager.IsNeedSwitchScoCategory(category);
    OnStop();
}
 
void ProcDisconnectReqWhenConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.ProcDisconnectReqWhenConnected(device);
    OnStop();
}
 
void ProcDisconnectReqWhenConnectingFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.ProcDisconnectReqWhenConnecting(device);
    OnStop();
}
 
void IsSameHfpDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress1 = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    std::string macAddress2 = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device1(macAddress1, TRANSPORT);
    BluetoothRemoteDevice device2(macAddress2, TRANSPORT);
    scoManager.IsSameHfpDevice(device1, device2);
    OnStop();
}
 
void ConnectAndDisconnectScoFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.ConnectSco(category, device);
    scoManager.DisconnectSco(category, device);
    ScoCategory category2 = PickValue(fdp, g_testScoCategories);
    scoManager.ConnectSco(category2, device);
    scoManager.DisconnectSco(category2, device);
    scoManager.DisconnectSco(category, device);
    OnStop();
}
 
void TryRestoreHfpDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.TryRestoreHfpDevice(category, device);
    OnStop();
}
 
void DisconnectScoReliableFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    ScoCategory category = PickValue(fdp, g_testScoCategories);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.DisconnectScoReliable(category, device);
    OnStop();
}
 
void GetAudioScoStateFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    scoManager.currentScoState_ = PickValue(fdp, g_testScoStates);
    scoManager.GetAudioScoState();
    OnStop();
}
 
void GetAudioScoCategoryFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    scoManager.currentScoCategory_ = PickValue(fdp, g_testScoCategories);
    scoManager.GetAudioScoCategory();
    OnStop();
}
 
void ResetScoStateFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.ResetScoState(device);
    OnStop();
}
 
void SetAudioScoStateFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    AudioScoState state = PickValue(fdp, g_testScoStates);
    scoManager.SetAudioScoState(state);
    OnStop();
}
 
void OnScoStateTimeOutFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    scoManager.currentScoState_ = PickValue(fdp, g_testScoStates);
    scoManager.OnScoStateTimeOut();
    OnStop();
}
 
void GetAudioScoDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.currentScoDevice_ = device;
    scoManager.GetAudioScoDevice();
    OnStop();
}

void UpdateScoStateFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothScoManager scoManager;
    HfpScoConnectState scoState = PickValue(fdp, g_testHfpScoConnectStates);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    scoManager.currentScoDevice_ = device;
    int reason = fdp.ConsumeIntegral<int>();
    scoManager.currentScoState_ = PickValue(fdp, g_testScoStates);
    scoManager.UpdateScoState(scoState, device, reason);
    OnStop();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    UpdateScoStateWhenDisconnectedFuzzTest,
    UpdateScoStateWhenConnectedFuzzTest,
    UpdateScoStateWhenConnectingFuzzTest,
    UpdateScoStateWhenDisconnectingFuzzTest,
    WriteScoStateFaultEventFuzzTest,
    ForceUpdateScoCategoryFuzzTest,
    ProcCacheRequestFuzzTest,
    HandleScoConnectFuzzTest,
    HandleScoDisconnectFuzzTest,
    HandleScoConnectNoLockFuzzTest,
    ProcConnectReqWhenDisconnectedFuzzTest,
    ProcConnectReqWhenConnectedFuzzTest,
    ProcConnectReqWhenConnectingFuzzTest,
    IsNeedSwitchScoCategoryFuzzTest,
    ProcDisconnectReqWhenConnectedFuzzTest,
    ProcDisconnectReqWhenConnectingFuzzTest,
    IsSameHfpDeviceFuzzTest,
    ConnectAndDisconnectScoFuzzTest,
    TryRestoreHfpDeviceFuzzTest,
    DisconnectScoReliableFuzzTest,
    GetAudioScoStateFuzzTest,
    GetAudioScoCategoryFuzzTest,
    ResetScoStateFuzzTest,
    SetAudioScoStateFuzzTest,
    OnScoStateTimeOutFuzzTest,
    GetAudioScoDeviceFuzzTest,
    UpdateScoStateFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS
 
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}