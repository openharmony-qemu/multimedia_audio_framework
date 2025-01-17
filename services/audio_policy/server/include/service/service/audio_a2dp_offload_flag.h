
/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_A2DP_OFFLOAD_FLAG_H
#define ST_AUDIO_A2DP_OFFLOAD_FLAG_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_errors.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {

enum A2dpOffloadConnectionState : int32_t {
    CONNECTION_STATUS_DISCONNECTED = 0,
    CONNECTION_STATUS_CONNECTING = 1,
    CONNECTION_STATUS_CONNECTED = 2,
    CONNECTION_STATUS_TIMEOUT = 3,
};

class AudioA2dpOffloadFlag {
public:
    static AudioA2dpOffloadFlag& GetInstance()
    {
        static AudioA2dpOffloadFlag instance;
        return instance;
    }

    void SetA2dpOffloadFlag(BluetoothOffloadState state);
    BluetoothOffloadState GetA2dpOffloadFlag();
    int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionIds);
    int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionIds);
    A2dpOffloadConnectionState GetCurrentOffloadConnectedState();
    void SetCurrentOffloadConnectedState(A2dpOffloadConnectionState currentOffloadConnectionState);
    bool IsA2dpOffloadConnected();
private:
    AudioA2dpOffloadFlag() {}
    ~AudioA2dpOffloadFlag() {}
private:
    BluetoothOffloadState a2dpOffloadFlag_ = NO_A2DP_DEVICE;
    A2dpOffloadConnectionState currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
};
}
}
#endif