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
#ifndef ST_AUDIO_OFFLOAD_STREAM_H
#define ST_AUDIO_OFFLOAD_STREAM_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "power_mgr_client.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_system_manager.h"
#include "audio_router_center.h"
#include "audio_errors.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"
#include "ipc_skeleton.h"

#include "audio_router_map.h"
#include "audio_iohandle_map.h"
#include "audio_active_device.h"
#include "audio_stream_collector.h"

namespace OHOS {
namespace AudioStandard {

class AudioOffloadStream {
public:
    static AudioOffloadStream& GetInstance()
    {
        static AudioOffloadStream instance;
        return instance;
    }
    void OffloadStreamSetCheck(uint32_t sessionId);
    void OffloadStreamReleaseCheck(uint32_t sessionId);
    void HandlePowerStateChanged(PowerMgr::PowerState state);
    void ResetOffloadMode(int32_t sessionId);
    int32_t MoveToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType);
    void RemoteOffloadStreamRelease(uint32_t sessionId);
    void CheckStreamMode(const int64_t activateSessionId);
    bool CheckStreamMultichannelMode(const int64_t activateSessionId);
    int32_t MoveToOutputDevice(uint32_t sessionId, std::string portName);
    std::vector<SinkInput> FilterSinkInputs(int32_t sessionId, std::vector<SinkInput> sinkInputs);
    void SetOffloadAvailableFromXML(AudioModuleInfo &moduleInfo);
    int32_t DynamicUnloadOffloadModule();
    int32_t UnloadMchModule();
    int32_t LoadMchModule();
    bool GetOffloadAvailableFromXml() const;
    void ResetOffloadModeOnSpatializationChanged(std::vector<int32_t> &allSessions);
    int32_t ActivateConcurrencyFromServer(AudioPipeType incomingPipe);
private:
    AudioOffloadStream() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioRouteMap_(AudioRouteMap::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()) {}
    ~AudioOffloadStream() {}
    bool CheckStreamOffloadMode(int64_t activateSessionId, AudioStreamType streamType);
    bool CheckSpatializationAndEffectState();
    void SetOffloadMode();
    int32_t MoveToNewPipeInner(const uint32_t sessionId, const AudioPipeType pipeType);
    int32_t SwitchToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType);
    int32_t LoadOffloadModule();
    int32_t UnloadOffloadModule();
    AudioModuleInfo ConstructMchAudioModuleInfo(DeviceType deviceType);
    AudioModuleInfo ConstructOffloadAudioModuleInfo(DeviceType deviceType);
private:
    std::optional<uint32_t> offloadSessionID_;
    PowerMgr::PowerState currentPowerState_ = PowerMgr::PowerState::AWAKE;
    std::mutex offloadMutex_;
    bool currentOffloadSessionIsBackground_ = false;
    bool isOffloadAvailable_ = false;

    std::mutex offloadCloseMutex_;
    std::mutex offloadOpenMutex_;
    std::atomic<bool> isOffloadOpened_ = false;
    std::condition_variable offloadCloseCondition_;

    IAudioPolicyInterface& audioPolicyManager_;
    AudioRouterCenter& audioRouterCenter_;
    AudioStreamCollector& streamCollector_;
    AudioIOHandleMap& audioIOHandleMap_;
    AudioRouteMap& audioRouteMap_;
    AudioActiveDevice& audioActiveDevice_;
};

}
}

#endif