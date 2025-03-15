/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_CAPTURER_SESSION_H
#define ST_AUDIO_CAPTURER_SESSION_H

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
#include "audio_volume_config.h"
#include "audio_adapter_info.h"
#include "audio_errors.h"

#include "audio_policy_manager_factory.h"
#include "audio_router_center.h"

#include "audio_iohandle_map.h"
#include "audio_active_device.h"
#include "audio_config_manager.h"
#include "audio_connected_device.h"
#include "audio_ec_manager.h"
#include "audio_device_common.h"
#include "audio_volume_manager.h"
#include "audio_a2dp_offload_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioCapturerSession {
public:
    static AudioCapturerSession& GetInstance()
    {
        static AudioCapturerSession instance;
        return instance;
    }
    void Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager);
    void DeInit();
    void SetConfigParserFlag();
    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);
    void OnCapturerSessionRemoved(uint64_t sessionID);

    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);
    int32_t CloseWakeUpAudioCapturer();

    void ReloadSourceForDeviceChange(const AudioDeviceDescriptor &inputDevice,
        const AudioDeviceDescriptor &outputDevice, const std::string &caller);
    void ReloadSourceForEffect(const AudioEffectPropertyArrayV3 &oldPropertyArray,
        const AudioEffectPropertyArrayV3 &newPropertyArray);
    void ReloadSourceForEffect(const AudioEnhancePropertyArray &oldPropertyArray,
        const AudioEnhancePropertyArray &newPropertyArray);
private:
    AudioCapturerSession() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioConfigManager_(AudioConfigManager::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioEcManager_(AudioEcManager::GetInstance()),
        audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance())
    {
        inputDeviceForReload_.deviceType_ = DEVICE_TYPE_DEFAULT;
    }
    ~AudioCapturerSession() {}

    void HandleRemainingSource();
    void HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo = {});

    void LoadInnerCapturerSink(std::string moduleName, AudioStreamInfo streamInfo);
    void UnloadInnerCapturerSink(std::string moduleName);

    bool ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
        AudioModuleInfo &audioModuleInfo);
    bool FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, PipeInfo *pipeInfo,
        AudioModuleInfo &audioModuleInfo);
    int32_t SetWakeUpAudioCapturer(InternalAudioCapturerOptions options);

    void SetInputDeviceTypeForReload(const AudioDeviceDescriptor &inputDevice);
    const AudioDeviceDescriptor& GetInputDeviceTypeForReload();
    bool IsVoipDeviceChanged(const AudioDeviceDescriptor &inputDevcie, const AudioDeviceDescriptor &outputDevice);

    std::string GetEnhancePropByName(const AudioEnhancePropertyArray &propertyArray, const std::string &propName);
    std::string GetEnhancePropByNameV3(const AudioEffectPropertyArrayV3 &oldPropertyArray, const std::string &propName);
private:
    IAudioPolicyInterface& audioPolicyManager_;
    AudioRouterCenter& audioRouterCenter_;
    AudioIOHandleMap& audioIOHandleMap_;
    AudioActiveDevice& audioActiveDevice_;
    AudioConfigManager& audioConfigManager_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioEcManager& audioEcManager_;
    AudioDeviceCommon& audioDeviceCommon_;
    AudioVolumeManager& audioVolumeManager_;

    std::atomic<bool> isPolicyConfigParsered_ = false;
    std::mutex onCapturerSessionChangedMutex_;
    std::unordered_map<uint32_t, SessionInfo> sessionWithNormalSourceType_;
    std::unordered_set<uint32_t> sessionIdisRemovedSet_;
    // sourceType is SOURCE_TYPE_PLAYBACK_CAPTURE, SOURCE_TYPE_WAKEUP or SOURCE_TYPE_VIRTUAL_CAPTURE
    std::unordered_map<uint32_t, SessionInfo> sessionWithSpecialSourceType_;
    uint64_t sessionIdUsedToOpenSource_ = 0;

    std::mutex inputDeviceReloadMutex_;
    AudioDeviceDescriptor inputDeviceForReload_;
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
};

}
}

#endif