/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SCO_MANAGER_H
#define BLUETOOTH_SCO_MANAGER_H
 
#include "bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_codec.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_hfp_ag.h"
#include "audio_info.h"
#include "bluetooth_device_utils.h"
#include "audio_timer.h"
 
namespace OHOS {
namespace Bluetooth {
class BluetoothScoManager {
public:
    static BluetoothScoManager &GetInstance();

    void UpdateScoState(HfpScoConnectState scoState, const BluetoothRemoteDevice &device, int reason = 0);
    int32_t HandleScoConnect(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t HandleScoDisconnect(const BluetoothRemoteDevice &device);
    AudioScoState GetAudioScoState();
    ScoCategory GetAudioScoCategory();
    void ResetScoState(const BluetoothRemoteDevice &device);
    BluetoothRemoteDevice GetAudioScoDevice();
    bool IsInScoCategory(ScoCategory scoCategory);

private:
    struct ScoCacheRequest {
        bool connectReq = false;
        ScoCategory category = ScoCategory::SCO_DEFAULT;
        BluetoothRemoteDevice device;
    };

    BluetoothScoManager();
    ~BluetoothScoManager() = default;

    uint32_t scoStateDuration_ = 3; /* 3: seconds */
    BluetoothRemoteDevice currentScoDevice_;
    AudioScoState currentScoState_ = AudioScoState::INIT;
    ScoCategory  currentScoCategory_ = ScoCategory::SCO_DEFAULT;
    std::shared_ptr<ScoCacheRequest> cacheReq_;
    std::shared_ptr<AudioStandard::AudioTimer> scoTimer_;
    std::mutex scoLock_;

    void UpdateScoStateWhenDisconnected(HfpScoConnectState scoState, const BluetoothRemoteDevice &device, int reason);
    void UpdateScoStateWhenConnected(HfpScoConnectState scoState, const BluetoothRemoteDevice &device, int reason);
    void UpdateScoStateWhenConnecting(HfpScoConnectState scoState, const BluetoothRemoteDevice &device, int reason);
    void UpdateScoStateWhenDisconnecting(HfpScoConnectState scoState, const BluetoothRemoteDevice &device, int reason);
    void WriteScoStateFaultEvent(HfpScoConnectState scoState, const BluetoothRemoteDevice &device, int reason);
    int32_t HandleScoConnectNoLock(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t HandleScoDisconnectNoLock(const BluetoothRemoteDevice &device);
    int32_t ProcConnectReqWhenDisconnected(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t ProcConnectReqWhenConnected(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t ProcConnectReqWhenConnecting(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t ProcConnectReqWhenDiconnecting(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t ProcDisconnectReqWhenConnected(const BluetoothRemoteDevice &device);
    int32_t ProcDisconnectReqWhenConnecting(const BluetoothRemoteDevice &device);
    bool IsNeedSwitchScoCategory(ScoCategory scoCategory);
    int32_t SaveRequestToCache(bool isConnect, ScoCategory scoCategory, const BluetoothRemoteDevice &device,
        const std::string &reason);
    bool IsSameHfpDevice(const BluetoothRemoteDevice &device1, const BluetoothRemoteDevice &device2);
    int32_t ConnectSco(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t TryRestoreHfpDevice(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t DisconnectSco(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    int32_t DisconnectScoReliable(ScoCategory scoCategory, const BluetoothRemoteDevice &device);
    void SetAudioScoState(AudioScoState state);
    void OnScoStateTimeOut();
    void ForceUpdateScoCategory();
    void ProcCacheRequest();
};
}
}
#endif