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

#ifndef ST_AUDIO_A2DP_OFFLOAD_MANAGER_H
#define ST_AUDIO_A2DP_OFFLOAD_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "audio_errors.h"
#include "audio_policy_log.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

#include "audio_active_device.h"
#include "audio_stream_collector.h"
#include "audio_a2dp_offload_flag.h"

namespace OHOS {
namespace AudioStandard {

class AudioA2dpOffloadManager final : public Bluetooth::AudioA2dpPlayingStateChangedListener,
    public std::enable_shared_from_this<AudioA2dpOffloadManager> {
public:
    AudioA2dpOffloadManager()
        : streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance())
    {}
    void Init() {Bluetooth::AudioA2dpManager::RegisterA2dpPlayingStateChangedListener(shared_from_this());};

    void OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState) override;
    
    bool IsA2dpOffloadConnecting(int32_t sessionId);
    bool IsA2dpOffloadConnected();

    void SetA2dpOffloadFlag(BluetoothOffloadState state);
    BluetoothOffloadState GetA2dpOffloadFlag();

    void UpdateA2dpOffloadFlagForStartStream(int32_t startSessionId);
    uint32_t UpdateA2dpOffloadFlagForAllStream(DeviceType deviceType = DEVICE_TYPE_NONE);
    void UpdateA2dpOffloadFlagForSpatializationChanged(
        std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnabledMap,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    void UpdateA2dpOffloadFlagForA2dpDeviceOut();

private:
    uint32_t UpdateA2dpOffloadFlagCommon(
        int32_t startSessionId, DeviceType deviceType, bool getSpatialFromService,
        std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnabledMap);
#ifdef BLUETOOTH_ENABLE
    void UpdateA2dpOffloadFlagInternal(const std::vector<Bluetooth::A2dpStreamInfo> &allActiveSessions,
        std::vector<int32_t> &allRunningSessions, DeviceType deviceType = DEVICE_TYPE_NONE);
#endif

    int32_t HandleA2dpDeviceInOffload(BluetoothOffloadState a2dpOffloadFlag,
        std::vector<int32_t> &allRunningSessions);
    int32_t HandleA2dpDeviceOutOffload(BluetoothOffloadState a2dpOffloadFlag,
        std::vector<int32_t> &allRunningSessions);
    int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionIds);
    int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionIds);
    void GetA2dpOffloadCodecAndSendToDsp();

    // For spatialization and audio data sync
    void ConnectA2dpOffload(const std::string &deviceAddress, const std::vector<int32_t> &sessionIds);
    void WaitForConnectionCompleted();

    // Tool funcs
    bool GetSpatialAudio(bool getSpatialFromService,
        int32_t sessionId, StreamUsage usage,
        std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnabledMap);
private:
    std::vector<int32_t> connectionTriggerSessionIds_;
    std::string a2dpOffloadDeviceAddress_ = "";
    std::mutex connectionMutex_;
    std::condition_variable connectionCV_;

    std::mutex switchA2dpOffloadMutex_;

    AudioStreamCollector &streamCollector_;
    AudioA2dpOffloadFlag &audioA2dpOffloadFlag_;
    AudioActiveDevice &audioActiveDevice_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
