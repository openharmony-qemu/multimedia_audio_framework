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
#ifndef LOG_TAG
#define LOG_TAG "AudioServerProxy"
#endif

#include "audio_server_proxy.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_service_log.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_policy_utils.h"
#include "istandard_audio_service.h"

namespace OHOS {
namespace AudioStandard {

const sptr<IStandardAudioService> AudioServerProxy::GetAudioServerProxy()
{
    AUDIO_DEBUG_LOG("[Policy Service] Start get audio policy service proxy.");
    std::lock_guard<std::mutex> lock(adProxyMutex_);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "[Policy Service] Get samgr failed.");

    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr,
        "[Policy Service] audio service remote object is NULL.");

    const sptr<IStandardAudioService> gsp = iface_cast<IStandardAudioService>(object);
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, nullptr,
        "[Policy Service] init gsp is NULL.");
    return gsp;
}

int32_t AudioServerProxy::SetAudioSceneProxy(AudioScene audioScene, BluetoothOffloadState state)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool scoExcludeFlag = false;
    if (AudioPolicyUtils::GetInstance().GetScoExcluded()) {
        scoExcludeFlag = true;
    }
    int32_t result = gsp->SetAudioScene(audioScene, state, scoExcludeFlag);
    IPCSkeleton::SetCallingIdentity(identity);
    return result;
}

float AudioServerProxy::GetMaxAmplitudeProxy(bool flag, std::string portName, SourceType sourceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, 0, "Service proxy unavailable");

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    float maxAmplitude = 0;
    int32_t rest = gsp->GetMaxAmplitude(flag, portName, sourceType, maxAmplitude);
    IPCSkeleton::SetCallingIdentity(identity);
    return maxAmplitude;
}

int64_t AudioServerProxy::GetVolumeDataCount(std::string sinkName)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, 0, "Service proxy unavailable");

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int64_t volumeDataCount = 0;
    gsp->GetVolumeDataCount(sinkName, volumeDataCount);
    IPCSkeleton::SetCallingIdentity(identity);
    return volumeDataCount;
}

void AudioServerProxy::UpdateEffectBtOffloadSupportedProxy(const bool &isSupported)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->UpdateEffectBtOffloadSupported(isSupported);
    IPCSkeleton::SetCallingIdentity(identity);
    return;
}

void AudioServerProxy::SetOutputDeviceSinkProxy(DeviceType deviceType, std::string sinkName)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetOutputDeviceSink(deviceType, sinkName);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetActiveOutputDeviceProxy(DeviceType deviceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetActiveOutputDevice(deviceType);
    IPCSkeleton::SetCallingIdentity(identity);
}

bool AudioServerProxy::GetEffectOffloadEnabledProxy()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool effectOffloadFlag = false;
    int32_t res = gsp->GetEffectOffloadEnabled(effectOffloadFlag);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, false, "GetEffectOffloadEnabled failed");
    return effectOffloadFlag;
}

int32_t AudioServerProxy::UpdateActiveDevicesRouteProxy(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState state, const std::string &deviceName)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::vector<IntPair> activeDevicesInt;
    for (auto &device : activeDevices) {
        activeDevicesInt.push_back({static_cast<int32_t>(device.first), static_cast<int32_t>(device.second)});
    }
    int32_t ret = gsp->UpdateActiveDevicesRoute(activeDevicesInt, state, deviceName);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

void AudioServerProxy::SetDmDeviceTypeProxy(uint16_t dmDeviceType, DeviceType deviceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetDmDeviceType(dmDeviceType, deviceType);
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioServerProxy::UpdateDualToneStateProxy(const bool &enable, const int32_t &sessionId)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->UpdateDualToneState(enable, sessionId);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

void AudioServerProxy::UpdateSessionConnectionStateProxy(const int32_t &sessionID, const int32_t &state)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->UpdateSessionConnectionState(sessionID, state);
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioServerProxy::CheckRemoteDeviceStateProxy(std::string networkId, DeviceRole deviceRole, bool isStartDevice)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t res = gsp->CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);
    IPCSkeleton::SetCallingIdentity(identity);
    return res;
}

void AudioServerProxy::SetAudioParameterProxy(const std::string &key, const std::string &value)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetAudioParameter(key, value);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::ResetAudioEndpointProxy()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->ResetAudioEndpoint();
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::GetAllSinkInputsProxy(std::vector<SinkInput> &sinkInputs)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->GetAllSinkInputs(sinkInputs);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetDefaultAdapterEnableProxy(bool isEnable)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetDefaultAdapterEnable(isEnable);
    IPCSkeleton::SetCallingIdentity(identity);
}

bool AudioServerProxy::NotifyStreamVolumeChangedProxy(AudioStreamType streamType, float volume)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->NotifyStreamVolumeChanged(streamType, volume);
    IPCSkeleton::SetCallingIdentity(identity);
    return true;
}

void AudioServerProxy::OffloadSetVolumeProxy(float volume, const std::string &deviceClass, const std::string &networkId)
{
    const sptr <IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->OffloadSetVolume(volume, deviceClass, networkId);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetVoiceVolumeProxy(float volume)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetVoiceVolume(volume);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::UnsetOffloadModeProxy(uint32_t sessionId)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->UnsetOffloadMode(sessionId);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetOffloadModeProxy(uint32_t sessionId, int32_t state, bool isAppBack)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetOffloadMode(sessionId, state, isAppBack);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::CheckHibernateStateProxy(bool hibernate)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->CheckHibernateState(hibernate);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::RestoreSessionProxy(const uint32_t &sessionID, RestoreInfo restoreInfo)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    RestoreInfoIpc restoreInfoIpc;
    restoreInfoIpc.restoreInfo = restoreInfo;
    gsp->RestoreSession(sessionID, restoreInfoIpc);
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioServerProxy::GetAudioEnhancePropertyProxy(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->GetAudioEnhanceProperty(propertyArray, deviceType);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t AudioServerProxy::SetAudioEnhancePropertyProxy(const AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->SetAudioEnhanceProperty(propertyArray, deviceType);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t AudioServerProxy::SetMicrophoneMuteProxy(bool isMute)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->SetMicrophoneMute(isMute);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

void AudioServerProxy::SetSinkMuteForSwitchDeviceProxy(const std::string &devceClass, int32_t durationUs, bool mute)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetSinkMuteForSwitchDevice(devceClass, durationUs, mute);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SuspendRenderSinkProxy(const std::string &sinkName)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SuspendRenderSink(sinkName);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::RestoreRenderSinkProxy(const std::string &sinkName)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->RestoreRenderSink(sinkName);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::LoadHdiEffectModelProxy()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->LoadHdiEffectModel();
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::NotifyDeviceInfoProxy(std::string networkId, bool connected)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->NotifyDeviceInfo(networkId, connected);
    IPCSkeleton::SetCallingIdentity(identity);
}

std::string AudioServerProxy::GetAudioParameterProxy(const std::string &key)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, "", "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string value = "";
    gsp->GetAudioParameter(key, value);
    IPCSkeleton::SetCallingIdentity(identity);
    return value;
}

std::string AudioServerProxy::GetAudioParameterProxy(const std::string& networkId, const AudioParamKey key,
    const std::string& condition)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, "", "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string value = "";
    gsp->GetAudioParameter(networkId, key, condition, value);
    IPCSkeleton::SetCallingIdentity(identity);
    return value;
}

void AudioServerProxy::ResetRouteForDisconnectProxy(DeviceType type)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->ResetRouteForDisconnect(type);
    IPCSkeleton::SetCallingIdentity(identity);
}

bool AudioServerProxy::CreatePlaybackCapturerManagerProxy()
{
#ifdef HAS_FEATURE_INNERCAPTURER
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool isSuccess = false;
    gsp->CreatePlaybackCapturerManager(isSuccess);
    IPCSkeleton::SetCallingIdentity(identity);
    return isSuccess;
#else
    return false;
#endif
}

bool AudioServerProxy::LoadAudioEffectLibrariesProxy(const std::vector<Library> libraries,
    const std::vector<Effect> effects, std::vector<Effect>& successEffectList)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool hasEffectsLoaded = false;
    int32_t res = gsp->LoadAudioEffectLibraries(libraries, effects, successEffectList, hasEffectsLoaded);
    IPCSkeleton::SetCallingIdentity(identity);
    return hasEffectsLoaded;
}

bool AudioServerProxy::CreateEffectChainManagerProxy(std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t res = gsp->CreateEffectChainManager(effectChains, effectParam, enhanceParam);
    bool isSuccess = res == SUCCESS;
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, false, "CreateEffectChainManager failed");
    return isSuccess;
}

int32_t AudioServerProxy::RegiestPolicyProviderProxy(const sptr<IRemoteObject> &object)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->RegiestPolicyProvider(object);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t AudioServerProxy::RegistCoreServiceProviderProxy(const sptr<IRemoteObject> &object)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->RegistCoreServiceProvider(object);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

void AudioServerProxy::SetParameterCallbackProxy(const sptr<IRemoteObject>& object)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetParameterCallback(object);
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioServerProxy::SetAudioEffectPropertyProxy(const AudioEffectPropertyArrayV3 &propertyArray,
    const DeviceType& deviceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->SetAudioEffectProperty(propertyArray, deviceType);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t AudioServerProxy::GetAudioEffectPropertyProxy(AudioEffectPropertyArrayV3 &propertyArray)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->GetAudioEffectProperty(propertyArray, DeviceType::DEVICE_TYPE_NONE);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t AudioServerProxy::SetAudioEffectPropertyProxy(const AudioEffectPropertyArray &propertyArray)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->SetAudioEffectProperty(propertyArray);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t AudioServerProxy::GetAudioEffectPropertyProxy(AudioEffectPropertyArray &propertyArray)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_HANDLE, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->GetAudioEffectProperty(propertyArray);
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

void AudioServerProxy::SetRotationToEffectProxy(const uint32_t rotate)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetRotationToEffect(rotate);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetAudioMonoStateProxy(bool audioMono)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetAudioMonoState(audioMono);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetAudioBalanceValueProxy(float audioBalance)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetAudioBalanceValue(audioBalance);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::NotifyAccountsChanged()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->NotifyAccountsChanged();
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::NotifySettingsDataReady()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->NotifySettingsDataReady();
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::NotifyAudioPolicyReady()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->NotifyAudioPolicyReady();
    IPCSkeleton::SetCallingIdentity(identity);
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioServerProxy::SetInnerCapLimitProxy(uint32_t innerCapLimit)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t res = gsp->SetInnerCapLimit(innerCapLimit);
    IPCSkeleton::SetCallingIdentity(identity);
    return res;
}
#endif

int32_t AudioServerProxy::LoadHdiAdapterProxy(uint32_t devMgrType, const std::string &adapterName)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t res = gsp->LoadHdiAdapter(devMgrType, adapterName);
    IPCSkeleton::SetCallingIdentity(identity);
    return res;
}

void AudioServerProxy::UnloadHdiAdapterProxy(uint32_t devMgrType, const std::string &adapterName, bool force)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->UnloadHdiAdapter(devMgrType, adapterName, force);
    IPCSkeleton::SetCallingIdentity(identity);
}

uint32_t AudioServerProxy::CreateHdiSinkPortProxy(const std::string &deviceClass, const std::string &idInfo,
    const IAudioSinkAttr &attr)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, HDI_INVALID_ID, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    uint32_t renderId = 0;
    int32_t res = gsp->CreateHdiSinkPort(deviceClass, idInfo, attr, renderId);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, HDI_INVALID_ID, "CreateHdiSinkPort failed");
    return renderId;
}

uint32_t AudioServerProxy::CreateHdiSourcePortProxy(const std::string &deviceClass, const std::string &idInfo,
    const IAudioSourceAttr &attr)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, HDI_INVALID_ID, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    uint32_t captureId = 0;
    int32_t res = gsp->CreateHdiSourcePort(deviceClass, idInfo, attr, captureId);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, HDI_INVALID_ID, "CreateHdiSourcePort failed");
    return captureId;
}

void AudioServerProxy::DestroyHdiPortProxy(uint32_t id)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->DestroyHdiPort(id);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetDeviceConnectedFlag(bool flag)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetDeviceConnectedFlag(flag);
    IPCSkeleton::SetCallingIdentity(identity);
}

bool AudioServerProxy::IsAcousticEchoCancelerSupported(SourceType sourceType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool isSupported = false;
    gsp->IsAcousticEchoCancelerSupported(sourceType, isSupported);
    IPCSkeleton::SetCallingIdentity(identity);
    return isSupported;
}

bool AudioServerProxy::SetKaraokeParameters(const std::string &parameters)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool res = false;
    gsp->SetKaraokeParameters(parameters, res);
    IPCSkeleton::SetCallingIdentity(identity);
    return res;
}

bool AudioServerProxy::IsAudioLoopbackSupported(AudioLoopbackMode mode)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool res = false;
    gsp->IsAudioLoopbackSupported(mode, res);
    IPCSkeleton::SetCallingIdentity(identity);
    return res;
}

void AudioServerProxy::SetLatestMuteState(const uint32_t sessionId, const bool muteFlag)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetLatestMuteState(sessionId, muteFlag);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetSessionMuteState(const uint32_t sessionId, const bool insert, const bool muteFlag)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetSessionMuteState(sessionId, insert, muteFlag);
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioServerProxy::SetBtHdiInvalidState()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    gsp->SetBtHdiInvalidState();
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioServerProxy::ForceStopAudioStreamProxy(StopAudioType audioType)
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_OPERATION, "Service proxy unavailable");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t res = gsp->ForceStopAudioStream(audioType);
    IPCSkeleton::SetCallingIdentity(identity);
    return res;
}
}
}
