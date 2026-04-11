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
#ifndef ST_AUDIO_POLICY_DUMP_H
#define ST_AUDIO_POLICY_DUMP_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_device_descriptor.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_errors.h"
#include "audio_stream_collector.h"
#include "audio_policy_manager_factory.h"
#include "audio_effect_service.h"

#include "audio_active_device.h"
#include "audio_core_config_manager.h"
#include "audio_scene_manager.h"
#include "audio_volume_manager.h"
#include "audio_connected_device.h"
#include "audio_microphone_descriptor.h"
#include "audio_offload_stream.h"
#include "audio_a2dp_offload_flag.h"

#include "audio_device_common.h"
#include "audio_device_lock.h"
#include "audio_device_manager.h"
#include "boot_animation_state_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioPolicyDump {
public:
    static constexpr uint32_t STREAM_FLAG_NORMAL = 0;
    static AudioPolicyDump& GetInstance()
    {
        static AudioPolicyDump instance;
        return instance;
    }
    void DevicesInfoDump(std::string &dumpString);
    void AudioModeDump(std::string &dumpString);
    void StreamVolumesDump(std::string &dumpString);
    void AudioPolicyParserDump(std::string &dumpString);
    void AudioStreamDump(std::string &dumpString);
    void XmlParsedDataMapDump(std::string &dumpString);
    void EffectManagerInfoDump(std::string &dumpString);
    void MicrophoneMuteInfoDump(std::string &dumpString);
    void AudioPolicyBootDump(std::string &dumpString);
    void RecordBootStateTime(AudioPolicyServerBootState state, int64_t curTime);
    void RecordBootStateTime(BootAnimationState state, int32_t uid, int64_t curTime);
    void RecordBootAnimationClientStateTime(const BootAnimationInfo &info);
private:
    std::string BootStateTime(AudioPolicyServerBootState state);
    std::string BootStateTime(BootAnimationState state);
    void BootAnimationDump(std::string &dumpString);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDumpDeviceInfo(
        std::string &dumpString, DeviceFlag deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDumpDevices(DeviceFlag deviceFlag);
    void GetMicrophoneDescriptorsDump(std::string &dumpString);
    void GetOffloadStatusDump(std::string &dumpString);

    void GetCallStatusDump(std::string &dumpString);
    void GetRingerModeDump(std::string &dumpString);

    void GetVolumeConfigDump(std::string &dumpString);
    void DeviceVolumeInfosDump(std::string &dumpString, DeviceVolumeInfoMap &deviceVolumeInfos);
    void GetGroupInfoDump(std::string &dumpString);

    void GetCapturerStreamDump(std::string &dumpString);
    void AudioPolicyParserDumpAdapterInfo(std::string &dumpString,
        std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>>& adapterInfoMap);
    void AudioPolicyParserDumpPipeInfo(std::string &dumpString, std::shared_ptr<PolicyAdapterInfo> &adapterInfo);
    void AudioPolicyParserDumpInner(std::string &dumpString,
        const std::unordered_map<std::string, std::string>& volumeGroupData,
        std::unordered_map<std::string, std::string>& interruptGroupData,
        PolicyGlobalConfigs globalConfigs);
    void GetEffectManagerInfo();
    bool IsStreamSupported(AudioStreamType streamType);
    std::string GetAudioStreamType(AudioStreamType streamType);
    std::string GetRingerModeType(AudioRingerMode ringerMode);
    void GetAdjustVolumeDump(std::string &dumpString);
    void AdjustVolumeAppend(std::vector<AdjustStreamVolumeInfo> adjustInfo, std::string &dumpString);
    void GetRingerModeInfoDump(std::string &dumpString);
    void AllDeviceVolumeInfoDump(std::string &dumpString);
private:
    AudioPolicyDump() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        audioEffectService_(AudioEffectService::GetAudioEffectService()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioConfigManager_(AudioCoreConfigManager::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager())
        {
            policyBootInfo_ = std::vector<int64_t>(static_cast<size_t>(AudioPolicyServerBootState::MAX), 0);
            bootAnimationInfo_ = std::vector<int64_t>(static_cast<size_t>(BootAnimationState::MAX), 0);
        }
    ~AudioPolicyDump() {}
private:
    IAudioPolicyInterface& audioPolicyManager_;
    AudioEffectService& audioEffectService_;
    AudioStreamCollector& streamCollector_;
    AudioActiveDevice& audioActiveDevice_;
    AudioCoreConfigManager& audioConfigManager_;
    AudioSceneManager& audioSceneManager_;
    AudioVolumeManager& audioVolumeManager_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioMicrophoneDescriptor& audioMicrophoneDescriptor_;
    AudioOffloadStream& audioOffloadStream_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    AudioDeviceManager &audioDeviceManager_;

    DeviceType priorityOutputDevice_ = DEVICE_TYPE_INVALID;
    DeviceType priorityInputDevice_ = DEVICE_TYPE_INVALID;
    ConnectType conneceType_ = CONNECT_TYPE_LOCAL;
    SupportedEffectConfig supportedEffectConfig_;
    ConverterConfig converterConfig_;
    AudioPolicyServerBootInfo policyBootInfo_;
    BootAnimationInfo bootAnimationInfo_;
};

}
}

#endif