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
#ifndef LOG_TAG
#define LOG_TAG "AudioA2dpOffloadFlag"
#endif

#include "audio_a2dp_offload_flag.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

int32_t AudioA2dpOffloadFlag::OffloadStopPlaying(const std::vector<int32_t> &sessionIds)
{
    return Bluetooth::AudioA2dpManager::OffloadStopPlaying(sessionIds);
}

void AudioA2dpOffloadFlag::SetA2dpOffloadFlag(BluetoothOffloadState state)
{
    a2dpOffloadFlag_ = state;
}

BluetoothOffloadState AudioA2dpOffloadFlag::GetA2dpOffloadFlag()
{
    return a2dpOffloadFlag_;
}

int32_t AudioA2dpOffloadFlag::OffloadStartPlaying(const std::vector<int32_t> &sessionIds)
{
    return Bluetooth::AudioA2dpManager::OffloadStartPlaying(sessionIds);
}

A2dpOffloadConnectionState AudioA2dpOffloadFlag::GetCurrentOffloadConnectedState()
{
    return currentOffloadConnectionState_;
}

void AudioA2dpOffloadFlag::SetCurrentOffloadConnectedState(A2dpOffloadConnectionState currentOffloadConnectionState)
{
    currentOffloadConnectionState_ = currentOffloadConnectionState;
}

bool AudioA2dpOffloadFlag::IsA2dpOffloadConnected()
{
    return currentOffloadConnectionState_ == CONNECTION_STATUS_CONNECTED;
}

}
}