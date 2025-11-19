/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_POLICY_CONFIG_MANAGER_H
#define AUDIO_POLICY_CONFIG_MANAGER_H

#include <string>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_definition_adapter_info.h"
#include "audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioPolicyConfigManager {
public:
    static AudioPolicyConfigManager& GetInstance()
    {
        static AudioPolicyConfigManager instance;
        return instance;
    }
    bool Init(bool isRefresh = false);
    
    void OnAudioPolicyConfigXmlParsingCompleted();

    // custom parser callback
    void OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmldata);
    void OnAudioLatencyParsed(uint64_t latency);
    void OnFastFormatParsed(AudioSampleFormat format);
    void OnSinkLatencyParsed(uint32_t latency);
    void OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData);
    void OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData);
    void OnGlobalConfigsParsed(PolicyGlobalConfigs &globalConfigs);
    void OnVoipConfigParsed(bool enableFastVoip);
    void OnUpdateRouteSupport(bool isSupported);
    void OnUpdateDefaultAdapter(bool isEnable);
    void OnUpdateAnahsSupport(std::string anahsShowType);
    void OnUpdateEac3Support(bool isSupported);
    void OnHasEarpiece();
    
    // update
    void SetNormalVoipFlag(const bool &normalVoipFlag);
    void UpdateStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
        const std::list<DeviceStreamInfo> &deviceStreamInfo, const std::list<std::string> &supportDevices);
    void ClearStreamPropInfo(const std::string &adapterName, const std::string &pipeName);
    void UpdateDynamicCapturerConfig(ClassType type, const AudioModuleInfo moduleInfo);

    // query
    bool GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList);
    void GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo);
    std::string GetGroupName(const std::string& deviceName, const GroupType type);
    int32_t GetMaxRendererInstances();
    int32_t GetMaxCapturersInstances();
    int32_t GetMaxFastRenderersInstances();
    bool IsSupportInnerCaptureOffload();
    int32_t GetVoipRendererFlag(const std::string &sinkPortName, const std::string &networkId,
        const AudioSamplingRate &samplingRate);
    bool GetNormalVoipFlag();
    int32_t GetAudioLatencyFromXml() const;
    AudioSampleFormat GetFastFormat() const;
    uint32_t GetSinkLatencyFromXml() const;
    void GetAudioAdapterInfos(std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> &adapterInfoMap);
    void GetVolumeGroupData(std::unordered_map<std::string, std::string>& volumeGroupData);
    void GetInterruptGroupData(std::unordered_map<std::string, std::string>& interruptGroupData);
    void GetGlobalConfigs(PolicyGlobalConfigs &globalConfigs);
    bool GetVoipConfig();
    bool GetUpdateRouteSupport();
    bool GetDefaultAdapterEnable();
    bool GetAdapterInfoFlag();
    bool GetAdapterInfoByType(AudioAdapterType type, std::shared_ptr<PolicyAdapterInfo> &info);
    bool GetHasEarpiece();
    bool IsFastStreamSupported(AudioStreamInfo &streamInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    bool GetFastStreamSupport(AudioStreamInfo &streamInfo, std::shared_ptr<AdapterDeviceInfo> &deviceInfo);
    uint32_t GetStreamPropInfoSize(const std::string &adapterName, const std::string &pipeName);

    uint32_t GetRouteFlag(std::shared_ptr<AudioStreamDescriptor> &desc);
    void GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc, std::shared_ptr<PipeStreamPropInfo> &info);
    std::shared_ptr<PipeStreamPropInfo> GetStreamPropInfoFromPipe(std::shared_ptr<AdapterPipeInfo> &info,
        const AudioStreamInfo &streamInfo);
    bool MatchStreamPropInfo(std::shared_ptr<PipeStreamPropInfo> &info,
        std::shared_ptr<AdapterPipeInfo> &adapterPipeInfo, const AudioStreamInfo &streamInfo);
    bool SupportImplicitConversion(uint32_t routeFlag);
    void GetTargetSourceTypeAndMatchingFlag(SourceType source, bool &useMatchingPropInfo);
    DirectPlaybackMode GetDirectPlaybackSupport(std::shared_ptr<AudioDeviceDescriptor> desc,
        const AudioStreamInfo &streamInfo);
    bool IsStreamPropMatch(const AudioStreamInfo &streamInfo, std::list<std::shared_ptr<PipeStreamPropInfo>> &infos);
    bool PreferMultiChannelPipe(std::shared_ptr<AudioStreamDescriptor> &desc);
    std::shared_ptr<PipeStreamPropInfo> GetStreamPropInfoForMultiChannel(
        std::shared_ptr<AudioStreamDescriptor> &desc, std::shared_ptr<AdapterPipeInfo> &info,
        AudioChannelLayout channelLayout);

    AudioPolicyConfigManager() : audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioPolicyConfig_(AudioPolicyConfigData::GetInstance())
    {
    }
    ~AudioPolicyConfigManager()
    {
    }
private:
    void UpdateStreamSampleInfo(std::shared_ptr<AudioStreamDescriptor> desc,
                                AudioStreamInfo &streamInfo);
    void UpdateBasicStreamInfo(std::shared_ptr<AudioStreamDescriptor> desc, std::shared_ptr<AdapterPipeInfo> pipeInfo,
        AudioStreamInfo &streamInfo);
    void GetStreamPropInfoForRecord(std::shared_ptr<AudioStreamDescriptor> desc,
        std::shared_ptr<AdapterPipeInfo> adapterPipeInfo, std::shared_ptr<PipeStreamPropInfo> &info,
        const AudioStreamInfo &streamInfo);
    std::shared_ptr<AdapterPipeInfo> GetNormalRecordAdapterInfo(std::shared_ptr<AudioStreamDescriptor> desc);
    std::shared_ptr<PipeStreamPropInfo> GetSuitableStreamPropInfo(
        std::list<std::shared_ptr<PipeStreamPropInfo>> &dynamicStreamPropInfos, uint32_t sampleRate);
    std::shared_ptr<PipeStreamPropInfo> GetDynamicStreamPropInfoFromPipe(std::shared_ptr<AdapterPipeInfo> &info,
        const AudioStreamInfo &streamInfo);
    AudioSampleFormat ParseFormat(const std::string format);
    void CheckDynamicCapturerConfig(std::shared_ptr<AudioStreamDescriptor> desc,
        std::shared_ptr<PipeStreamPropInfo> &info);

    bool xmlHasLoaded_ = false;

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo_ = {};
    std::unordered_map<ClassType, AudioModuleInfo> dynamicCapturerConfig_ = {};
    bool hasEarpiece_ = false;
    bool isUpdateRouteSupported_ = true;
    bool isDefaultAdapterEnable_ = false;
    bool isSupportEac3_ = false;
    std::unordered_map<std::string, std::string> volumeGroupData_;
    std::unordered_map<std::string, std::string> interruptGroupData_;
    PolicyGlobalConfigs globalConfigs_;
    bool enableFastVoip_ = false;
    uint64_t audioLatencyInMsec_ = 50;
    uint32_t sinkLatencyInMsec_ {0};
    AudioSampleFormat fastFormat_ = SAMPLE_S16LE;
    bool normalVoipFlag_ = false;

    std::atomic<bool> isAdapterInfoMap_ = false;
    AudioDeviceManager &audioDeviceManager_;

    AudioPolicyConfigData &audioPolicyConfig_;

    std::optional<float> isSupportInnerCaptureOffload_ = std::nullopt;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_POLICY_CONFIG_MANAGER_H
