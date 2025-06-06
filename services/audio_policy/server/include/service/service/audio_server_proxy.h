
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

#ifndef ST_AUDIO_SERVER_PROXY_H
#define ST_AUDIO_SERVER_PROXY_H

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

#include "ipc_skeleton.h"

namespace OHOS {
namespace AudioStandard {
class AudioServerProxy {
public:
    static AudioServerProxy& GetInstance()
    {
        static AudioServerProxy instance;
        return instance;
    }
    const sptr<IStandardAudioService> GetAudioServerProxy();
    int32_t SetAudioSceneProxy(AudioScene audioScene, std::vector<DeviceType> activeOutputDevices,
        DeviceType deviceType, BluetoothOffloadState state);
    float GetMaxAmplitudeProxy(bool flag, std::string portName, SourceType sourceType = SOURCE_TYPE_INVALID);
    void UpdateEffectBtOffloadSupportedProxy(const bool &isSupported);
    void SetOutputDeviceSinkProxy(DeviceType deviceType, std::string sinkName);
    void SetActiveOutputDeviceProxy(DeviceType deviceType);
    bool GetEffectOffloadEnabledProxy();
    int32_t UpdateActiveDevicesRouteProxy(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
        BluetoothOffloadState state, const std::string &deviceName = "");
    int32_t UpdateDualToneStateProxy(const bool &enable, const int32_t &sessionId);
    void UpdateSessionConnectionStateProxy(const int32_t &sessionID, const int32_t &state);
    int32_t CheckRemoteDeviceStateProxy(std::string networkId, DeviceRole deviceRole, bool isStartDevice);
    void SetAudioParameterProxy(const std::string &key, const std::string &value);
    void ResetAudioEndpointProxy();
    bool NotifyStreamVolumeChangedProxy(AudioStreamType streamType, float volume);
    void OffloadSetVolumeProxy(float volume);
    void SetVoiceVolumeProxy(float volume);
    void UnsetOffloadModeProxy(uint32_t sessionId);
    void SetOffloadModeProxy(uint32_t sessionId, int32_t state, bool isAppBack);
    void CheckHibernateStateProxy(bool hibernate);
    void RestoreSessionProxy(const uint32_t &sessionID, RestoreInfo RestoreInfo);
    int32_t GetAudioEnhancePropertyProxy(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t SetAudioEnhancePropertyProxy(const AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    void SetSinkMuteForSwitchDeviceProxy(const std::string &devceClass, int32_t durationUs, bool mute);
    int32_t SetMicrophoneMuteProxy(bool isMute);
    void SuspendRenderSinkProxy(const std::string &sinkName);
    void RestoreRenderSinkProxy(const std::string &sinkName);
    void LoadHdiEffectModelProxy();
    void NotifyDeviceInfoProxy(std::string networkId, bool connected);
    std::string GetAudioParameterProxy(const std::string &key);
    std::string GetAudioParameterProxy(const std::string& networkId, const AudioParamKey key,
        const std::string& condition);
    void ResetRouteForDisconnectProxy(DeviceType type);
    bool CreatePlaybackCapturerManagerProxy();
    bool LoadAudioEffectLibrariesProxy(const std::vector<Library> libraries, const std::vector<Effect> effects,
        std::vector<Effect>& successEffectList);
    bool CreateEffectChainManagerProxy(std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam);
    int32_t RegiestPolicyProviderProxy(const sptr<IRemoteObject> &object);
    void SetParameterCallbackProxy(const sptr<IRemoteObject>& object);
    int32_t SetAudioEffectPropertyProxy(const AudioEffectPropertyArrayV3 &propertyArray,
        const DeviceType& deviceType = DEVICE_TYPE_NONE);
    int32_t GetAudioEffectPropertyProxy(AudioEffectPropertyArrayV3 &propertyArray);
    int32_t SetAudioEffectPropertyProxy(const AudioEffectPropertyArray &propertyArray);
    int32_t GetAudioEffectPropertyProxy(AudioEffectPropertyArray &propertyArray);
    void SetRotationToEffectProxy(const uint32_t rotate);
    void SetAudioMonoStateProxy(bool audioMono);
    void SetAudioBalanceValueProxy(float audioBalance);
    void NotifyAccountsChanged();
    void GetAllSinkInputsProxy(std::vector<SinkInput> &sinkInputs);
    void NotifyAudioPolicyReady();
    void SetDefaultAdapterEnableProxy(bool isEnable);
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t SetInnerCapLimitProxy(uint32_t innerCapLimit);
#endif
    int32_t LoadHdiAdapterProxy(uint32_t devMgrType, const std::string &adapterName);
    void UnloadHdiAdapterProxy(uint32_t devMgrType, const std::string &adapterName, bool force);
    void SetDeviceConnectedFlag(bool flag);
    void NotifySettingsDataReady();
private:
    AudioServerProxy() {}
    ~AudioServerProxy() {}
private:
    std::mutex adProxyMutex_;
};
}
}
#endif