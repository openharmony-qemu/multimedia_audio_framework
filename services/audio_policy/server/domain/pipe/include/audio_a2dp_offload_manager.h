
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
#include "audio_router_center.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

#include "audio_device_common.h"
#include "audio_volume_manager.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_iohandle_map.h"
#include "audio_policy_config_manager.h"
#include "audio_active_device.h"
#include "audio_offload_stream.h"

namespace OHOS {
namespace AudioStandard {

class AudioA2dpOffloadManager final : public Bluetooth::AudioA2dpPlayingStateChangedListener,
    public std::enable_shared_from_this<AudioA2dpOffloadManager> {
public:
    AudioA2dpOffloadManager() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioConfigManager_(AudioPolicyConfigManager::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()),
        audioDeviceCommon_(AudioDeviceCommon::GetInstance())
    {}
    void Init() {Bluetooth::AudioA2dpManager::RegisterA2dpPlayingStateChangedListener(shared_from_this());};
    void ConnectA2dpOffload(const std::string &deviceAddress, const std::vector<int32_t> &sessionIds);
    void OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState) override;

    void WaitForConnectionCompleted();
    bool IsA2dpOffloadConnecting(int32_t sessionId);
    bool IsA2dpOffloadConnected();

    void SetA2dpOffloadFlag(BluetoothOffloadState state);
    BluetoothOffloadState GetA2dpOffloadFlag();
    int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionIds);
    int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionIds);

    void UpdateA2dpOffloadFlagForAllStream(std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t UpdateA2dpOffloadFlagForAllStream(DeviceType deviceType = DEVICE_TYPE_NONE);
    void UpdateOffloadWhenActiveDeviceSwitchFromA2dp();

private:
#ifdef BLUETOOTH_ENABLE
    void UpdateA2dpOffloadFlag(const std::vector<Bluetooth::A2dpStreamInfo> &allActiveSessions,
        DeviceType deviceType = DEVICE_TYPE_NONE);
#endif
    void GetA2dpOffloadCodecAndSendToDsp();
    int32_t HandleA2dpDeviceInOffload(BluetoothOffloadState a2dpOffloadFlag);
    int32_t HandleA2dpDeviceOutOffload(BluetoothOffloadState a2dpOffloadFlag);
    int32_t HandleActiveDevice(AudioDeviceDescriptor deviceDescriptor);
    void FetchStreamForA2dpOffload(const bool &requireReset);
    void GetAllRunningStreamSession(std::vector<int32_t> &allSessions, bool doStop = false);
    std::string GetVolumeGroupType(DeviceType deviceType);
private:
    std::vector<int32_t> connectionTriggerSessionIds_;
    std::string a2dpOffloadDeviceAddress_ = "";
    std::mutex connectionMutex_;
    std::condition_variable connectionCV_;
    std::mutex switchA2dpOffloadMutex_;

    IAudioPolicyInterface& audioPolicyManager_;
    AudioStreamCollector& streamCollector_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    AudioRouterCenter& audioRouterCenter_;
    AudioIOHandleMap& audioIOHandleMap_;
    AudioActiveDevice& audioActiveDevice_;
    AudioPolicyConfigManager& audioConfigManager_;
    AudioVolumeManager& audioVolumeManager_;
    AudioOffloadStream& audioOffloadStream_;
    AudioDeviceCommon& audioDeviceCommon_;
};
}
}
#endif