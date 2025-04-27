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
#ifndef ST_AUDIO_CONFIG_MANAGER_H
#define ST_AUDIO_CONFIG_MANAGER_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "iport_observer.h"
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_errors.h"
#include "audio_policy_parser_factory.h"
#include "audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioConfigManager : public IPortObserver {
public:
    static AudioConfigManager& GetInstance()
    {
        static AudioConfigManager instance;
        return instance;
    }
    bool Init();
    // Audio Policy Parser callbacks
    void OnAudioPolicyXmlParsingCompleted(const std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap);

    // Parser callbacks
    void OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmldata);

    void OnUpdateRouteSupport(bool isSupported);

    void OnUpdateAnahsSupport(std::string anahsShowType);

    void OnUpdateDefaultAdapter(bool isEnable);

    void OnAudioLatencyParsed(uint64_t latency);

    void OnSinkLatencyParsed(uint32_t latency);

    void OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData);

    void OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData);

    void OnGlobalConfigsParsed(GlobalConfigs &globalConfigs);

    void OnVoipConfigParsed(bool enableFastVoip);

    // other

    bool GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList);

    void GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo);

    std::string GetGroupName(const std::string& deviceName, const GroupType type);

    int32_t GetMaxRendererInstances();
    void SetNormalVoipFlag(const bool &normalVoipFlag);

    bool GetNormalVoipFlag();

    int32_t GetVoipRendererFlag(const std::string &sinkPortName, const std::string &networkId,
        const AudioSamplingRate &samplingRate);

    bool GetVoipConfig();

    int32_t GetAudioLatencyFromXml() const;

    uint32_t GetSinkLatencyFromXml() const;

    void GetAudioAdapterInfos(std::unordered_map<AdaptersType, AudioAdapterInfo> &adapterInfoMap);

    void GetVolumeGroupData(std::unordered_map<std::string, std::string>& volumeGroupData);

    void GetInterruptGroupData(std::unordered_map<std::string, std::string>& interruptGroupData);

    bool GetUpdateRouteSupport();

    bool GetDefaultAdapterEnable();

    bool GetAdapterInfoFlag();

    bool GetAdapterInfoByType(AdaptersType type, AudioAdapterInfo &info);

    bool GetHasEarpiece();

    void GetGlobalConfigs(GlobalConfigs &globalConfigs);
private:
    AudioConfigManager() : audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager())
    {
    }
    ~AudioConfigManager()
    {
    }
private:
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo_ = {};
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap_ {};

    bool hasEarpiece_ = false;
    bool isUpdateRouteSupported_ = true;
    bool isDefaultAdapterEnable_ = false;
    std::unordered_map<std::string, std::string> volumeGroupData_;
    std::unordered_map<std::string, std::string> interruptGroupData_;
    GlobalConfigs globalConfigs_;
    bool enableFastVoip_ = false;
    uint64_t audioLatencyInMsec_ = 50;
    uint32_t sinkLatencyInMsec_ {0};
    bool normalVoipFlag_ = false;

    std::atomic<bool> isAdapterInfoMap_ = false;
    AudioDeviceManager &audioDeviceManager_;
};

}
}

#endif