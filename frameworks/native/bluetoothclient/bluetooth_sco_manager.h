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
 
namespace OHOS {
namespace Bluetooth {
class BluetoothScoManager {
public:
    static void UpdateScoState(HfpScoConnectState scoState, const BluetoothRemoteDevice *device = nullptr);
    static int32_t HandleScoConnect(ScoCategory scoCategory, const BluetoothRemoteDevice *device = nullptr);
    static int32_t HandleScoDisconnect(ScoCategory scoCategory, const BluetoothRemoteDevice *device = nullptr);
    static AudioScoState GetAudioScoState();
    static AudioScoMode GetScoModeFromCategery(ScoCategory scoCategory);
    static ScoCategory GetScoCategeryFromMode(AudioScoMode scoMode);
private:
    static HandsFreeAudioGateway *hfpInstance_;
    static AudioScoState currentScoState_;
    static AudioScoMode  currentScoMode_;
    static AudioScoMode  lastScoMode_;
};
}
}
#endif