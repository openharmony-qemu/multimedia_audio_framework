/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioA2dpOffloadManager"
#endif

#include "audio_a2dp_offload_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_affinity_manager.h"
#include "audio_stream_collector.h"
#include "audio_policy_manager_factory.h"
#include "audio_spatialization_manager.h"
#include "audio_router_center.h"

#include "audio_policy_service.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const int32_t DATA_LINK_CONNECTING = 10;
const int32_t DATA_LINK_CONNECTED = 11;
const int32_t CONNECTION_TIMEOUT_IN_MS = 300; // 300ms
void AudioA2dpOffloadManager::OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState)
{
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    AUDIO_INFO_LOG("OnA2dpPlayingStateChanged current A2dpOffload MacAddr:%{public}s, incoming MacAddr:%{public}s, "
        "currentStatus:%{public}d, incommingState:%{public}d", GetEncryptAddr(a2dpOffloadDeviceAddress_).c_str(),
        GetEncryptAddr(deviceAddress).c_str(), state, playingState);
    if (deviceAddress != a2dpOffloadDeviceAddress_) {
        if (playingState == A2DP_STOPPED && state == CONNECTION_STATUS_CONNECTED) {
            return;
        }
        // below is A2dp(not offload scenario)
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
        return;
    }

    // deviceAddress matched
    if (playingState == A2DP_PLAYING) {
        if (state == CONNECTION_STATUS_CONNECTING) {
            AUDIO_INFO_LOG("OnA2dpPlayingStateChanged state change "
                "from %{public}d to %{public}d", state, CONNECTION_STATUS_CONNECTED);

            for (int32_t sessionId : connectionTriggerSessionIds_) {
                AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTED);
            }
            std::vector<int32_t>().swap(connectionTriggerSessionIds_);
            connectionCV_.notify_all();
        }
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    } else if (playingState == A2DP_STOPPED) {
        AUDIO_INFO_LOG("OnA2dpPlayingStateChanged state change "
            "from %{public}d to %{public}d", state, CONNECTION_STATUS_DISCONNECTED);
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
        a2dpOffloadDeviceAddress_ = "";
        std::vector<int32_t>().swap(connectionTriggerSessionIds_);
    } else {
        // at the current moment, we only handle the PLAYING and STOPPED state,
        // will handle other state in the future
        AUDIO_INFO_LOG("OnA2dpPlayingStateChanged state: %{public}d, "
            "received unexpected state:%{public}d", state, playingState);
    }
}

void AudioA2dpOffloadManager::ConnectA2dpOffload(const std::string &deviceAddress,
    const std::vector<int32_t> &sessionIds)
{
    AUDIO_INFO_LOG("start connecting a2dpOffload for MacAddr:%{public}s.", GetEncryptAddr(deviceAddress).c_str());
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    a2dpOffloadDeviceAddress_ = deviceAddress;
    connectionTriggerSessionIds_.assign(sessionIds.begin(), sessionIds.end());

    for (int32_t sessionId : connectionTriggerSessionIds_) {
        AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTING);
    }

    if (state == CONNECTION_STATUS_CONNECTED || state == CONNECTION_STATUS_CONNECTING) {
        AUDIO_INFO_LOG("state already in %{public}d, "
            "status, no need to trigger another waiting", state);
        return;
    }

    std::thread switchThread(&AudioA2dpOffloadManager::WaitForConnectionCompleted, this);
    switchThread.detach();
    AUDIO_INFO_LOG("state change from %{public}d to %{public}d",
        state, CONNECTION_STATUS_CONNECTING);
    audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
}

void AudioA2dpOffloadManager::WaitForConnectionCompleted()
{
    std::unique_lock<std::mutex> waitLock(connectionMutex_);
    bool connectionCompleted = connectionCV_.wait_for(waitLock,
        std::chrono::milliseconds(AudioA2dpOffloadManager::CONNECTION_TIMEOUT_IN_MS), [this] {
            return audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState() == CONNECTION_STATUS_CONNECTED;
        });
    // a2dp connection timeout, anyway we should notify client dataLink OK in order to allow the data flow begin
    AUDIO_INFO_LOG("WaitForConnectionCompleted unblocked, connectionCompleted is %{public}d", connectionCompleted);

    if (!connectionCompleted) {
        AUDIO_INFO_LOG("state change from %{public}d to %{public}d",
            audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_TIMEOUT);
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
        for (int32_t sessionId : connectionTriggerSessionIds_) {
            AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTED);
        }
        std::vector<int32_t>().swap(connectionTriggerSessionIds_);
    }
    waitLock.unlock();
    AudioPolicyService::GetAudioPolicyService().FetchStreamForA2dpOffload(false);
    return;
}

bool AudioA2dpOffloadManager::IsA2dpOffloadConnecting(int32_t sessionId)
{
    if (audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState() == CONNECTION_STATUS_CONNECTING) {
        if (std::find(connectionTriggerSessionIds_.begin(), connectionTriggerSessionIds_.end(), sessionId) !=
            connectionTriggerSessionIds_.end()) {
            return true;
        }
    }
    return false;
}

bool AudioA2dpOffloadManager::IsA2dpOffloadConnected()
{
    return audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState() == CONNECTION_STATUS_CONNECTED;
}

int32_t AudioA2dpOffloadManager::OffloadStartPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("OffloadStartPlaying, a2dpOffloadFlag_: %{public}d, sessionIds: %{public}zu",
        GetA2dpOffloadFlag(), sessionIds.size());
    if (GetA2dpOffloadFlag() != A2DP_OFFLOAD || sessionIds.size() == 0) {
        return SUCCESS;
    }
    int32_t ret = audioA2dpOffloadFlag_.OffloadStartPlaying(sessionIds);
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    if (ret == SUCCESS && (state != CONNECTION_STATUS_CONNECTED)) {
        ConnectA2dpOffload(Bluetooth::AudioA2dpManager::GetActiveA2dpDevice(), sessionIds);
    }
    return ret;
#else
    return SUCCESS;
#endif
}

int32_t AudioA2dpOffloadManager::OffloadStopPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_PRERELEASE_LOGI("OffloadStopPlaying, a2dpOffloadFlag_: %{public}d, sessionIds: %{public}zu",
        GetA2dpOffloadFlag(), sessionIds.size());
    if (GetA2dpOffloadFlag() != A2DP_OFFLOAD || sessionIds.size() == 0) {
        return SUCCESS;
    }
    return audioA2dpOffloadFlag_.OffloadStopPlaying(sessionIds);
#else
    return SUCCESS;
#endif
}

void AudioA2dpOffloadManager::SetA2dpOffloadFlag(BluetoothOffloadState state)
{
    audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
}

BluetoothOffloadState AudioA2dpOffloadManager::GetA2dpOffloadFlag()
{
    return audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
}

}
}