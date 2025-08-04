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

#ifndef ST_AUDIO_EC_MANAGER_H
#define ST_AUDIO_EC_MANAGER_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_ec_info.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_router_center.h"
#include "audio_policy_manager_factory.h"

#include "audio_policy_config_manager.h"
#include "audio_active_device.h"
#include "audio_iohandle_map.h"

namespace OHOS {
namespace AudioStandard {

class AudioEcManager {
public:
    static AudioEcManager& GetInstance()
    {
        static AudioEcManager instance;
        return instance;
    }

    void Init(int32_t ecEnableState, int32_t micRefEnableState);
    void CloseNormalSource();
    AudioEcInfo GetAudioEcInfo();
    void ResetAudioEcInfo();

    void PresetArmIdleInput(const string &address);
    void ActivateArmDevice(const string &address, const DeviceRole role);
    void CloseUsbArmDevice(const AudioDeviceDescriptor &device);
    void GetTargetSourceTypeAndMatchingFlag(SourceType source, SourceType &targetSource, bool &useMatchingPropInfo);

    int32_t FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, PipeStreamPropInfo &targetInfo,
        SourceType &targetSourceType);

    void ReloadSourceForSession(SessionInfo sessionInfo);

    void SetDpSinkModuleInfo(const AudioModuleInfo &moduleInfo);
    void SetPrimaryMicModuleInfo(const AudioModuleInfo &moduleInfo);
    SourceType GetSourceOpened();
    bool GetEcFeatureEnable();
    bool GetMicRefFeatureEnable();
    void UpdateStreamEcAndMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
    void SetOpenedNormalSource(SourceType sourceType);
    void PrepareNormalSource(AudioModuleInfo &moduleInfo, std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void SetOpenedNormalSourceSessionId(uint64_t sessionId);
    uint64_t GetOpenedNormalSourceSessionId();
    int32_t ReloadNormalSource(SessionInfo &sessionInfo, PipeStreamPropInfo &targetInfo, SourceType targetSource);
    void UpdateStreamEcInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
    void UpdateStreamMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
private:
    AudioEcManager() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioConfigManager_(AudioPolicyConfigManager::GetInstance()) {}
    ~AudioEcManager() {}

    void UpdateEnhanceEffectState(SourceType source);
    void UpdateStreamCommonInfo(AudioModuleInfo &moduleInfo, PipeStreamPropInfo &targetInfo, SourceType sourceType);
    void UpdateAudioEcInfo(const AudioDeviceDescriptor &inputDevice, const AudioDeviceDescriptor &outputDevice);
    void UpdateModuleInfoForEc(AudioModuleInfo &moduleInfo);
    void UpdateModuleInfoForMicRef(AudioModuleInfo &moduleInfo, SourceType sourceType);
    std::string ShouldOpenMicRef(SourceType source);

    EcType GetEcType(const DeviceType inputDevice, const DeviceType outputDevice);
    std::string GetEcSamplingRate(const std::string &halName, std::shared_ptr<PipeStreamPropInfo> &outModuleInfo);
    std::string GetEcFormat(const std::string &halName, std::shared_ptr<PipeStreamPropInfo> &outModuleInfo);
    std::string GetEcChannels(const std::string &halName, std::shared_ptr<PipeStreamPropInfo> &outModuleInfo);

    int32_t GetPipeInfoByDeviceTypeForEc(const std::string &role, const DeviceType deviceType,
        std::shared_ptr<AdapterPipeInfo> &pipeInfo);

    void UpdateArmModuleInfo(const string &address, const DeviceRole role, AudioModuleInfo &moduleInfo);
    std::string GetHalNameForDevice(const std::string &role, const DeviceType deviceType);
    std::string GetPipeNameByDeviceForEc(const std::string &role, const DeviceType deviceType);
private:
    bool isEcFeatureEnable_ = false;
    bool isMicRefFeatureEnable_ = false;

    AudioModuleInfo usbSinkModuleInfo_ = {};
    AudioModuleInfo usbSourceModuleInfo_ = {};
    AudioModuleInfo primaryMicModuleInfo_ = {};
    AudioModuleInfo dpSinkModuleInfo_ = {};
    SourceType normalSourceOpened_ = SOURCE_TYPE_INVALID;
    uint64_t sessionIdUsedToOpenSource_ = 0;

    std::mutex audioEcInfoMutex_;
    AudioEcInfo audioEcInfo_;
    std::string activeArmInputAddr_;
    std::string activeArmOutputAddr_;

    bool isMicRefVoipUpOn_ = false;
    bool isMicRefRecordOn_ = false;

    IAudioPolicyInterface& audioPolicyManager_;
    AudioRouterCenter& audioRouterCenter_;
    AudioIOHandleMap& audioIOHandleMap_;
    AudioActiveDevice& audioActiveDevice_;
    AudioPolicyConfigManager& audioConfigManager_;
};

}
}
#endif
