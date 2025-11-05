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
#ifndef ST_AUDIO_POLICY_UTILS_H
#define ST_AUDIO_POLICY_UTILS_H

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
#include "audio_ec_info.h"
#include "datashare_helper.h"
#include "audio_errors.h"
#include "audio_state_manager.h"
#include "audio_device_manager.h"
#include "audio_stream_collector.h"

#include "audio_a2dp_offload_flag.h"
#include "audio_policy_config_manager.h"

namespace OHOS {
namespace AudioStandard {

const int64_t SET_BT_ABS_SCENE_DELAY_MS = 120000; // 120ms
const int64_t CALL_IPC_COST_TIME_MS = 20000000; // 20ms

class AudioPolicyUtils {
public:
    static AudioPolicyUtils& GetInstance()
    {
        static AudioPolicyUtils instance;
        return instance;
    }
    void WriteServiceStartupError(std::string reason);
    void WriteDeviceChangeExceptionEvent(const AudioStreamDeviceChangeReason reason,
        DeviceType deviceType, DeviceRole deviceRole, int32_t errorMsg, const std::string &errorDesc);
    std::string GetRemoteModuleName(std::string networkId, DeviceRole role);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAvailableDevicesInner(AudioDeviceUsage usage);
    void SetBtConnecting(bool flag);
    int32_t SetPreferredDevice(const PreferredType preferredType, const std::shared_ptr<AudioDeviceDescriptor> &desc,
        const int32_t uid = INVALID_UID, const std::string caller = "");
    void ClearScoDeviceSuspendState(std::string macAddress = "");
    int64_t GetCurrentTimeMS();
    std::string GetNewSinkPortName(DeviceType deviceType);
    std::string GetSinkPortName(DeviceType deviceType, AudioPipeType pipeType = PIPE_TYPE_UNKNOWN);
    string ConvertToHDIAudioFormat(AudioSampleFormat sampleFormat);
    std::string GetSinkName(const AudioDeviceDescriptor &desc, int32_t sessionId);
    std::string GetSinkName(std::shared_ptr<AudioDeviceDescriptor> desc, int32_t sessionId);
    uint32_t PcmFormatToBytes(AudioSampleFormat format);
    bool IsVoiceStreamType(StreamUsage streamUsage);
    bool IsVoiceSourceType(SourceType sourceType);
    std::string GetSourcePortName(DeviceType deviceType, uint32_t routeFlag = AUDIO_FLAG_NONE);
    void UpdateDisplayName(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    void UpdateDisplayNameForRemote(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    int32_t GetDeviceNameFromDataShareHelper(std::string &deviceName);
    void UpdateEffectDefaultSink(DeviceType deviceType);
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelperInstance();
    AudioModuleInfo ConstructRemoteAudioModuleInfo(std::string networkId,
        DeviceRole deviceRole, DeviceType deviceType);
    DeviceRole GetDeviceRole(DeviceType deviceType) const;
    DeviceRole GetDeviceRole(const std::string &role);
    DeviceRole GetDeviceRole(AudioPin pin) const;
    DeviceType GetDeviceType(const std::string &deviceName);
    std::string GetEncryptAddr(const std::string &addr);
    std::string GetDevicesStr(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);

    AudioDeviceUsage GetAudioDeviceUsageByStreamUsage(StreamUsage streamUsage);
    PreferredType GetPreferredTypeByStreamUsage(StreamUsage streamUsage);

    int32_t UnexcludeOutputDevices(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);
    std::string GetOutputDeviceClassBySinkPortName(std::string sinkPortName);
    std::string GetInputDeviceClassBySourcePortName(std::string sourcePortName);
    void SetScoExcluded(bool scoExcluded);
    bool GetScoExcluded();
    bool IsDataShareReady();

    int32_t SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object);
    bool IsBundleNameInList(const std::string &bundleName, const std::string &listType);
    bool IsSupportedNearlink(const std::string &bundleName, int32_t apiVersion, bool hasSystemPermission);

    bool IsWirelessDevice(DeviceType deviceType);
private:
    AudioPolicyUtils() : streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioStateManager_(AudioStateManager::GetAudioStateManager()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioConfigManager_(AudioPolicyConfigManager::GetInstance()) {}
    ~AudioPolicyUtils() {}
    int32_t ErasePreferredDeviceByType(const PreferredType preferredType);
public:
    static int32_t startDeviceId;
    static std::map<std::string, ClassType> portStrToEnum;
private:
    bool isBTReconnecting_ = false;
    bool isScoExcluded_ = false;
    DeviceType effectActiveDevice_ = DEVICE_TYPE_NONE;
    AudioStreamCollector& streamCollector_;
    AudioStateManager &audioStateManager_;
    AudioDeviceManager &audioDeviceManager_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    AudioPolicyConfigManager& audioConfigManager_;

    sptr<IStandardAudioPolicyManagerListener> queryBundleNameListCallback_ = nullptr;
};

}
}

#endif
