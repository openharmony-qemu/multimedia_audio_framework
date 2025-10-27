/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <thread>
#include "audio_policy_manager.h"
#include "sle_audio_device_manager.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
bool g_hasPermission = false;
bool g_hasServerInit = false;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

class AudioManagerDeviceChangeCallbackFuzzTest : public AudioManagerDeviceChangeCallback {
public:
    AudioManagerDeviceChangeCallbackFuzzTest() {}
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override {}
};

class AudioPreferredOutputDeviceChangeCallbackFuzzTest : public AudioPreferredOutputDeviceChangeCallback {
public:
    AudioPreferredOutputDeviceChangeCallbackFuzzTest() {}
    void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override {}
};

class AudioPreferredInputDeviceChangeCallbackFuzzTest : public AudioPreferredInputDeviceChangeCallback {
public:
    AudioPreferredInputDeviceChangeCallbackFuzzTest() {}
    void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override {};
};

class DeviceChangeWithInfoCallbackFuzzTest : public DeviceChangeWithInfoCallback {
public:
    DeviceChangeWithInfoCallbackFuzzTest() {}
    void OnDeviceChangeWithInfo(const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReasonExt reason) override {}

    void OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override {}
};

class AudioManagerAvailableDeviceChangeCallbackFuzzTest : public AudioManagerAvailableDeviceChangeCallback {
public:
    AudioManagerAvailableDeviceChangeCallbackFuzzTest() {}
    void OnAvailableDeviceChange(const AudioDeviceUsage usage, const DeviceChangeAction &deviceChangeAction) override {}
};

class AudioDeviceAnahsFuzzTest : public AudioDeviceAnahs {
public:
    AudioDeviceAnahsFuzzTest() {}

    int32_t OnExtPnpDeviceStatusChanged(std::string anahsStatus, std::string anahsShowType) override
    {
        return 0;
    }
};

class AudioQueryDeviceVolumeBehaviorCallbackFuzzTest : public AudioQueryDeviceVolumeBehaviorCallback {
public:
    AudioQueryDeviceVolumeBehaviorCallbackFuzzTest() {}
    VolumeBehavior OnQueryDeviceVolumeBehavior() override
    {
        VolumeBehavior volumeBehavior;
        return volumeBehavior;
    }
};

void AudioPolicyManagerOneFuzzTest()
{
    bool block = GetData<bool>();
    sptr<IAudioPolicy> Ap_gsp = AudioPolicyManager::GetInstance().GetAudioPolicyManagerProxy();
    CallbackChange callbackChange = GetData<CallbackChange>();
    pid_t pid = GetData<pid_t>();
    uid_t uid = GetData<uid_t>();
    AudioServerDiedCallBack func;
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t volumeFlag = GetData<int32_t>();
    int32_t appUid = GetData<int32_t>();
    bool muted = GetData<bool>();
    bool isMute = GetData<bool>();
    int32_t zoneId = GetData<int32_t>();
    uid = GetData<int32_t>();
    bool isLegacy = GetData<bool>();
    DeviceType deviceType = GetData<DeviceType>();
    AudioRingerMode ringMode = GetData<AudioRingerMode>();
    AudioScene scene = GetData<AudioScene>();
    PolicyType policyType = GetData<PolicyType>();

    AudioPolicyManager::GetInstance().GetAudioPolicyManagerProxy(block);
    AudioPolicyManager::GetInstance().RegisterPolicyCallbackClientFunc(Ap_gsp);
    AudioPolicyManager::GetInstance().RecoverAudioPolicyCallbackClient();
    AudioPolicyManager::GetInstance().SetCallbackStreamInfo(callbackChange);
    AudioPolicyManager::GetInstance().AudioPolicyServerDied(pid, uid);
    AudioPolicyManager::GetInstance().RegisterServerDiedCallBack(func);
    AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
    AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType);
    AudioPolicyManager::GetInstance().SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    AudioPolicyManager::GetInstance().SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    AudioPolicyManager::GetInstance().SetAppVolumeMuted(appUid, muted, volumeFlag);
    AudioPolicyManager::GetInstance().IsAppVolumeMute(appUid, muted, isMute);
    AudioPolicyManager::GetInstance().SetAdjustVolumeForZone(zoneId);
    AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, isLegacy, volumeFlag, uid);
    AudioPolicyManager::GetInstance().SetSystemVolumeLevelWithDevice(volumeType, volumeLevel, deviceType, volumeFlag);
    AudioPolicyManager::GetInstance().SetRingerModeLegacy(ringMode);
    AudioPolicyManager::GetInstance().SetRingerMode(ringMode);
    AudioPolicyManager::GetInstance().GetRingerMode();
    AudioPolicyManager::GetInstance().SetAudioScene(scene);
    AudioPolicyManager::GetInstance().SetMicrophoneMute(isMute);
    AudioPolicyManager::GetInstance().SetMicrophoneMuteAudioConfig(isMute);
}

void AudioPolicyManagerTwoFuzzTest()
{
    int32_t volumeLevel = GetData<int32_t>();
    int32_t appUid = GetData<int32_t>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    int32_t uid = GetData<int32_t>();
    bool mute = GetData<bool>();
    bool isLegacy = GetData<bool>();
    DeviceType deviceType = GetData<DeviceType>();
    int32_t streamId = GetData<int32_t>();
    float volume = GetData<float>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    AudioStreamInfo streamInfo;
    SourceType source = GetData<SourceType>();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    int32_t zoneId = GetData<int32_t>();
    CallbackChange callbackChange = GetData<CallbackChange>();
    bool enable = GetData<bool>();
    bool block = GetData<bool>();
    AudioRendererInfo rendererInfo;
    AudioCapturerInfo capturerInfo;
    int32_t clientId = GetData<int32_t>();
    std::shared_ptr<AudioFocusInfoChangeCallback> callback;
    uint32_t sessionId = GetData<uint32_t>();

    AudioPolicyManager::GetInstance().GetSelfAppVolumeLevel(volumeLevel);
    AudioPolicyManager::GetInstance().GetAppVolumeLevel(appUid, volumeLevel);
    AudioPolicyManager::GetInstance().GetSystemVolumeLevel(volumeType, uid);
    AudioPolicyManager::GetInstance().SetStreamMute(volumeType, mute, isLegacy, deviceType);
    AudioPolicyManager::GetInstance().GetStreamMute(volumeType);
    AudioPolicyManager::GetInstance().SetLowPowerVolume(streamId, volume);
    AudioPolicyManager::GetInstance().GetLowPowerVolume(streamId);
    AudioPolicyManager::GetInstance().GetFastStreamInfo(sessionId);
    AudioPolicyManager::GetInstance().GetSingleStreamVolume(streamId);
    AudioPolicyManager::GetInstance().IsStreamActive(volumeType);
    AudioPolicyManager::GetInstance().IsStreamActiveByStreamUsage(streamUsage);
    AudioPolicyManager::GetInstance().IsFastPlaybackSupported(streamInfo, streamUsage);
    AudioPolicyManager::GetInstance().IsFastRecordingSupported(streamInfo, source);
    AudioPolicyManager::GetInstance().GetAudioFocusInfoList(focusInfoList, zoneId);
    AudioPolicyManager::GetInstance().SetClientCallbacksEnable(callbackChange, enable, block);
    AudioPolicyManager::GetInstance().SetCallbackRendererInfo(rendererInfo);
    AudioPolicyManager::GetInstance().SetCallbackCapturerInfo(capturerInfo);
    AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId, callback);
    AudioPolicyManager::GetInstance().UnregisterFocusInfoChangeCallback(clientId);
}

void AudioPolicyManagerThreeFuzzTest()
{
    int32_t appUid = GetData<int32_t>();
    std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> volumeTypeChangeCallback;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> appVolumeChangeCallback;
    int32_t clientId = GetData<int32_t>();
    std::shared_ptr<AudioRingerModeCallback> ringerModeCallback;
    API_VERSION api_v = GetData<API_VERSION>();
    std::shared_ptr<AudioManagerMicrophoneBlockedCallback> microphoneBlockedCallback;
    std::shared_ptr<AudioManagerAudioSceneChangedCallback> audioSceneChangedCallback;
    std::shared_ptr<AudioManagerMicStateChangeCallback> micStateChangeCallback;
    uint32_t sessionID = GetData<uint32_t>();
    std::shared_ptr<AudioInterruptCallback> interruptCallback;
    uint32_t clientUid = GetData<int32_t>();
    int32_t zoneId = GetData<int32_t>();
    int32_t displayId = GetData<int32_t>();
    int32_t mode = GetData<int32_t>();
    std::shared_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback;
    std::shared_ptr<AudioQueryBundleNameListCallback> audioQueryBundleNameListCallback;
    AudioInterrupt audioInterrupt;
    bool isUpdatedAudioStrategy = GetData<bool>();

    AudioPolicyManager::GetInstance().SetActiveVolumeTypeCallback(volumeTypeChangeCallback);
    AudioPolicyManager::GetInstance().UnsetActiveVolumeTypeCallback(volumeTypeChangeCallback);
    AudioPolicyManager::GetInstance().SetSelfAppVolumeChangeCallback(appVolumeChangeCallback);
    AudioPolicyManager::GetInstance().UnsetSelfAppVolumeCallback(appVolumeChangeCallback);
    AudioPolicyManager::GetInstance().UnsetAppVolumeCallbackForUid(appVolumeChangeCallback);
    AudioPolicyManager::GetInstance().SetAppVolumeChangeCallbackForUid(appUid, appVolumeChangeCallback);
    AudioPolicyManager::GetInstance().SetRingerModeCallback(clientId, ringerModeCallback, api_v);
    AudioPolicyManager::GetInstance().UnsetRingerModeCallback(clientId);
    AudioPolicyManager::GetInstance().UnsetRingerModeCallback(clientId, ringerModeCallback);
    AudioPolicyManager::GetInstance().SetMicrophoneBlockedCallback(clientId, microphoneBlockedCallback);
    AudioPolicyManager::GetInstance().UnsetMicrophoneBlockedCallback(clientId, microphoneBlockedCallback);
    AudioPolicyManager::GetInstance().SetAudioSceneChangeCallback(clientId, audioSceneChangedCallback);
    AudioPolicyManager::GetInstance().UnsetAudioSceneChangeCallback(audioSceneChangedCallback);
    AudioPolicyManager::GetInstance().SetMicStateChangeCallback(clientId, micStateChangeCallback);
    AudioPolicyManager::GetInstance().UnsetMicStateChangeCallback(micStateChangeCallback);
    AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID, interruptCallback, clientUid, zoneId);
    AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID, zoneId);
    AudioPolicyManager::GetInstance().SetQueryClientTypeCallback(audioQueryClientTypeCallback);
    AudioPolicyManager::GetInstance().SetQueryBundleNameListCallback(audioQueryBundleNameListCallback);
    AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt, zoneId, isUpdatedAudioStrategy);
    AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt, zoneId);
    AudioPolicyManager::GetInstance().ActivatePreemptMode();
    AudioPolicyManager::GetInstance().DeactivatePreemptMode();
    AudioPolicyManager::GetInstance().SetAudioManagerInterruptCallback(clientId, interruptCallback);
    AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
    AudioPolicyManager::GetInstance().SetAppConcurrencyMode(appUid, mode);
    AudioPolicyManager::GetInstance().SetAppSilentOnDisplay(displayId);
}

void AudioPolicyManagerFourFuzzTest()
{
    int32_t clientId = GetData<int32_t>();
    AudioInterrupt audioInterrupt;
    int32_t zoneId = GetData<int32_t>();
    int32_t clientPid = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallback;
    std::shared_ptr<VolumeKeyEventCallback> volumeDegreeEventCallback;
    API_VERSION api_v = GetData<API_VERSION>();
    std::shared_ptr<SystemVolumeChangeCallback> systemVolumeChangeCallback;
    std::shared_ptr<AudioRendererStateChangeCallback> audioRendererStateChangeCallback;
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> audioRendererStateChangeCallbacks;
    std::shared_ptr<AudioCapturerStateChangeCallback> audioCapturerStateChangeCallback;
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    std::shared_ptr<AudioClientTracker> clientTrackerObj;
    AudioRendererInfo rendererInfo;
    AudioCapturerInfo capturerInfo;
    std::shared_ptr<AudioStreamDescriptor> streamDesc;
    uint32_t flag = GetData<uint32_t>();
    uint32_t sessionId = GetData<uint32_t>();
    std::string networkId = "netWorkId";

    AudioPolicyManager::GetInstance().RequestAudioFocus(clientId, audioInterrupt);
    AudioPolicyManager::GetInstance().AbandonAudioFocus(clientId, audioInterrupt);
    AudioPolicyManager::GetInstance().GetStreamInFocus(zoneId);
    AudioPolicyManager::GetInstance().GetStreamInFocusByUid(uid, zoneId);
    AudioPolicyManager::GetInstance().GetSessionInfoInFocus(audioInterrupt, zoneId);
    AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, volumeKeyEventCallback, api_v);
    AudioPolicyManager::GetInstance().UnsetVolumeKeyEventCallback(volumeKeyEventCallback);
    AudioPolicyManager::GetInstance().SetSystemVolumeChangeCallback(clientPid, systemVolumeChangeCallback);
    AudioPolicyManager::GetInstance().UnsetSystemVolumeChangeCallback(systemVolumeChangeCallback);
    AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(audioRendererStateChangeCallback);
    AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(audioRendererStateChangeCallbacks);
    AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(audioRendererStateChangeCallback);
    AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientPid, audioCapturerStateChangeCallback);
    AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(clientPid);
    AudioPolicyManager::GetInstance().RegisterTracker(mode, streamChangeInfo, clientTrackerObj);
    AudioPolicyManager::GetInstance().UpdateTracker(mode, streamChangeInfo);
    AudioPolicyManager::GetInstance().GetPreferredOutputStreamType(rendererInfo);
    AudioPolicyManager::GetInstance().GetPreferredInputStreamType(capturerInfo);
    AudioPolicyManager::GetInstance().CreateRendererClient(streamDesc, flag, sessionId, networkId);
    AudioPolicyManager::GetInstance().CreateCapturerClient(streamDesc, flag, sessionId);
    AudioPolicyManager::GetInstance().SetVolumeDegreeCallback(clientPid, volumeDegreeEventCallback, api_v);
    AudioPolicyManager::GetInstance().UnsetVolumeDegreeCallback(volumeDegreeEventCallback);
}

void AudioPolicyManagerFiveFuzzTest()
{
    int32_t clientUid = GetData<int32_t>();
    StreamSetState streamSetState = GetData<StreamSetState>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    std::string networkId = "networkId";
    std::vector<sptr<VolumeGroupInfo>> infos;
    int32_t groupId = GetData<int32_t>();
    std::string key = "key";
    std::string uri = "uri";
    int32_t clientPid = GetData<int32_t>();
    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> audioRendererPolicyServiceDiedCallback;
    std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> audioCapturerPolicyServiceDiedCallback;
    std::shared_ptr<AudioStreamPolicyServiceDiedCallback> audioStreamPolicyServiceDiedCallback;
    VolumeAdjustType adjustType = GetData<VolumeAdjustType>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    int32_t volumeLevel = GetData<int32_t>();
    DeviceType deviceType = GetData<DeviceType>();
    SupportedEffectConfig supportedEffectConfig;
    std::shared_ptr<AudioDeviceDescriptor> desc;
    int32_t sessionId = GetData<int32_t>();

    AudioPolicyManager::GetInstance().UpdateStreamState(clientUid, streamSetState, streamUsage);
    AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    AudioPolicyManager::GetInstance().GetNetworkIdByGroupId(groupId, networkId);
    AudioPolicyManager::GetInstance().SetSystemSoundUri(key, uri);
    AudioPolicyManager::GetInstance().GetSystemSoundUri(key);
    AudioPolicyManager::GetInstance().GetMinStreamVolume();
    AudioPolicyManager::GetInstance().GetMaxStreamVolume();
    AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(clientPid,
        audioRendererPolicyServiceDiedCallback);
    AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(clientPid,
        audioCapturerPolicyServiceDiedCallback);
    AudioPolicyManager::GetInstance().UnregisterAudioPolicyServerDiedCb(clientPid);
    AudioPolicyManager::GetInstance().RegisterAudioStreamPolicyServerDiedCb(audioStreamPolicyServiceDiedCallback);
    AudioPolicyManager::GetInstance().UnregisterAudioStreamPolicyServerDiedCb(audioStreamPolicyServiceDiedCallback);
    AudioPolicyManager::GetInstance().GetMaxRendererInstances();
    AudioPolicyManager::GetInstance().IsVolumeUnadjustable();
    AudioPolicyManager::GetInstance().AdjustVolumeByStep(adjustType);
    AudioPolicyManager::GetInstance().AdjustSystemVolumeByStep(volumeType, adjustType);
    AudioPolicyManager::GetInstance().GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
    AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    AudioPolicyManager::GetInstance().GetHardwareOutputSamplingRate(desc);
    AudioPolicyManager::GetInstance().GetAudioCapturerMicrophoneDescriptors(sessionId);
    AudioPolicyManager::GetInstance().GetAvailableMicrophones();
}

void AudioPolicyManagerSixFuzzTest()
{
    std::string macAddress = "macAddress";
    bool support = GetData<bool>();
    int32_t volume = GetData<int32_t>();
    bool updateUi = GetData<bool>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    CastType castType = GetData<CastType>();
    std::shared_ptr<AudioDistributedRoutingRoleCallback> audioDistributedRoutingRoleCallback;
    std::string address = "address";
    bool enable = GetData<bool>();
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice;
    std::shared_ptr<AudioSpatializationEnabledChangeCallback> audioSpatializationEnabledChangeCallback;
    std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> currentDeviceCallback;
    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> audioHeadTrackingEnabledChangeCallback;
    std::shared_ptr<AudioNnStateChangeCallback> audioNnStateChangeCallback;

    AudioPolicyManager::GetInstance().SetDeviceAbsVolumeSupported(macAddress, support, volume);
    AudioPolicyManager::GetInstance().IsAbsVolumeScene();
    AudioPolicyManager::GetInstance().SetA2dpDeviceVolume(macAddress, volume, updateUi);
    AudioPolicyManager::GetInstance().SetNearlinkDeviceVolume(macAddress, volumeType, volume, updateUi);
    AudioPolicyManager::GetInstance().ConfigDistributedRoutingRole(descriptor, castType);
    AudioPolicyManager::GetInstance().SetDistributedRoutingRoleCallback(audioDistributedRoutingRoleCallback);
    AudioPolicyManager::GetInstance().UnsetDistributedRoutingRoleCallback();
    AudioPolicyManager::GetInstance().IsSpatializationEnabled();
    AudioPolicyManager::GetInstance().IsSpatializationEnabled(address);
    AudioPolicyManager::GetInstance().IsSpatializationEnabledForCurrentDevice();
    AudioPolicyManager::GetInstance().SetSpatializationEnabled(enable);
    AudioPolicyManager::GetInstance().SetSpatializationEnabled(selectedAudioDevice, enable);
    AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
    AudioPolicyManager::GetInstance().IsHeadTrackingEnabled(address);
    AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(enable);
    AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(selectedAudioDevice, enable);
    AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(
        audioSpatializationEnabledChangeCallback);
    AudioPolicyManager::GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(currentDeviceCallback);
    AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(audioHeadTrackingEnabledChangeCallback);
    AudioPolicyManager::GetInstance().RegisterNnStateEventListener(audioNnStateChangeCallback);
    AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledEventListener();
    AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledForCurrentDeviceEventListener();
    AudioPolicyManager::GetInstance().UnregisterHeadTrackingEnabledEventListener();
    AudioPolicyManager::GetInstance().UnregisterNnStateEventListener();
}

void AudioPolicyManagerSevenFuzzTest()
{
    std::string address = "address";
    AudioSpatialDeviceState audioSpatialDeviceState;
    uint32_t sessionId = GetData<uint32_t>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    std::shared_ptr<AudioSpatializationStateChangeCallback> audioSpatializationStateChangeCallback;
    std::set<int32_t> pids;
    int32_t pid = GetData<int32_t>();
    pids.insert(pid);
    int32_t zoneId = GetData<int32_t>();
    bool highResExist = GetData<bool>();
    AudioSessionStrategy strategy;
    DeviceType deviceType = GetData<DeviceType>();
    SourceType sourceType = GetData<SourceType>();
    bool isRunning = GetData<bool>();
    std::shared_ptr<AudioSessionCallback> audioSessionCallback;
    AudioSessionScene audioSessionScene = GetData<AudioSessionScene>();
    std::shared_ptr<AudioSessionStateChangedCallback> stateChangedCallback;

    AudioPolicyManager::GetInstance().IsHeadTrackingSupported();
    AudioPolicyManager::GetInstance().IsHeadTrackingSupportedForDevice(address);
    AudioPolicyManager::GetInstance().UpdateSpatialDeviceState(audioSpatialDeviceState);
    AudioPolicyManager::GetInstance().RegisterSpatializationStateEventListener(sessionId,
        streamUsage, audioSpatializationStateChangeCallback);
    AudioPolicyManager::GetInstance().UnregisterSpatializationStateEventListener(sessionId);
    AudioPolicyManager::GetInstance().GetConverterConfig();
    AudioPolicyManager::GetInstance().IsHighResolutionExist();
    AudioPolicyManager::GetInstance().SetHighResolutionExist(highResExist);
    AudioPolicyManager::GetInstance().ActivateAudioSession(strategy);
    AudioPolicyManager::GetInstance().DeactivateAudioSession();
    AudioPolicyManager::GetInstance().IsAudioSessionActivated();
    AudioPolicyManager::GetInstance().SetInputDevice(deviceType, sessionId, sourceType, isRunning);
    AudioPolicyManager::GetInstance().SetAudioSessionCallback(audioSessionCallback);
    AudioPolicyManager::GetInstance().UnsetAudioSessionCallback();
    AudioPolicyManager::GetInstance().UnsetAudioSessionCallback(audioSessionCallback);
    AudioPolicyManager::GetInstance().SetAudioSessionScene(audioSessionScene);
    AudioPolicyManager::GetInstance().SetAudioSessionStateChangeCallback(stateChangedCallback);
    AudioPolicyManager::GetInstance().UnsetAudioSessionStateChangeCallback();
    AudioPolicyManager::GetInstance().UnsetAudioSessionStateChangeCallback(stateChangedCallback);
}

void AudioPolicyManagerEightFuzzTest()
{
    DeviceType deviceType = GetData<DeviceType>();
    std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> deviceChangedCallback;
    AudioSpatializationSceneType spatializationSceneType = GetData<AudioSpatializationSceneType>();
    int32_t deviceId = GetData<int32_t>();
    std::string macAddress = "macAddress";
    std::shared_ptr<HeadTrackingDataRequestedChangeCallback> HTDRcallback;
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefiner;
    std::shared_ptr<AudioClientInfoMgrCallback> audioClientInfoMgrCallback;
    AudioStreamChangeInfo streamChangeInfo;
    int32_t sessionId = GetData<int32_t>();
    bool isMuted = GetData<bool>();
    int32_t appUid = GetData<int32_t>();

    AudioPolicyManager::GetInstance().GetDefaultOutputDevice(deviceType);
    AudioPolicyManager::GetInstance().SetDefaultOutputDevice(deviceType);
    AudioPolicyManager::GetInstance().SetAudioSessionCurrentDeviceChangeCallback(deviceChangedCallback);
    AudioPolicyManager::GetInstance().UnsetAudioSessionCurrentDeviceChangeCallback();
    AudioPolicyManager::GetInstance().UnsetAudioSessionCurrentDeviceChangeCallback(deviceChangedCallback);
    AudioPolicyManager::GetInstance().GetSpatializationSceneType();
    AudioPolicyManager::GetInstance().SetSpatializationSceneType(spatializationSceneType);
    AudioPolicyManager::GetInstance().GetMaxAmplitude(deviceId);
    AudioPolicyManager::GetInstance().DisableSafeMediaVolume();
    AudioPolicyManager::GetInstance().IsHeadTrackingDataRequested(macAddress);
    AudioPolicyManager::GetInstance().RegisterHeadTrackingDataRequestedEventListener(macAddress, HTDRcallback);
    AudioPolicyManager::GetInstance().UnregisterHeadTrackingDataRequestedEventListener(macAddress);
    AudioPolicyManager::GetInstance().SetAudioDeviceRefinerCallback(audioDeviceRefiner);
    AudioPolicyManager::GetInstance().UnsetAudioDeviceRefinerCallback();
    AudioPolicyManager::GetInstance().SetAudioClientInfoMgrCallback(audioClientInfoMgrCallback);
    AudioPolicyManager::GetInstance().ResetClientTrackerStubMap();
    AudioPolicyManager::GetInstance().CheckAndRemoveClientTrackerStub(AUDIO_MODE_PLAYBACK, streamChangeInfo);
    AudioPolicyManager::GetInstance().RemoveClientTrackerStub(sessionId);
    AudioPolicyManager::GetInstance().SetAppRingMuted(appUid, isMuted);
}

void AudioPolicyManagerNiNeFuzzTest()
{
    bool isMute = GetData<bool>();
    int32_t clientUid = GetData<int32_t>();
    PolicyType type = GetData<PolicyType>();
    std::string countryCode = "countryCode";
    int32_t ltonetype = GetData<int32_t>();
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    StreamUsage streamUsage = GetData<StreamUsage>();
    std::string address = "address";
    int32_t volumeDegree = GetData<int32_t>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    int32_t volumeFlag = GetData<int32_t>();
    uid_t uid = GetData<uid_t>();

    AudioPolicyManager::GetInstance().SetMicrophoneMutePersistent(isMute, type);
    AudioPolicyManager::GetInstance().GetPersistentMicMuteState();
    AudioPolicyManager::GetInstance().IsMicrophoneMute();
    AudioPolicyManager::GetInstance().GetAudioScene();
    AudioPolicyManager::GetInstance().GetSystemActiveVolumeType(clientUid);
    AudioPolicyManager::GetInstance().GetSupportedTones(countryCode);
    AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, countryCode);
    AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    AudioPolicyManager::GetInstance().GetSpatializationState(streamUsage);
    AudioPolicyManager::GetInstance().IsSpatializationSupported();
    AudioPolicyManager::GetInstance().IsSpatializationSupportedForDevice(address);
    AudioPolicyManager::GetInstance().SetSystemVolumeDegree(volumeType, volumeDegree, volumeFlag, uid);
    AudioPolicyManager::GetInstance().GetSystemVolumeDegree(volumeType, uid);
    AudioPolicyManager::GetInstance().GetMinVolumeDegree(volumeType);
}

void AudioPolicyManagerDeviceOneFuzzTest()
{
    sptr<AudioRendererFilter> audioRendererFilter;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    AudioStreamType streamType = GetData<AudioStreamType>();
    sptr<AudioCapturerFilter> audioCapturerFilter;
    AudioDeviceUsage audioDevUsage = GetData<AudioDeviceUsage>();
    DeviceFlag deviceFlag = GetData<DeviceFlag>();
    AudioRendererInfo rendererInfo;
    bool forceNoBTPermission = GetData<bool>();
    AudioCapturerInfo capturerInfo;
    bool active = GetData<bool>();
    int32_t clientId = GetData<int32_t>();
    std::shared_ptr<AudioManagerDeviceChangeCallback> audioManagerDeviceChangeCallback;
    std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> outputDeviceChangeCallback;
    std::shared_ptr<AudioPreferredInputDeviceChangeCallback> inputDeviceChangeCallback;

    AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
    AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
    AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    AudioPolicyManager::GetInstance().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AudioPolicyManager::GetInstance().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AudioPolicyManager::GetInstance().GetExcludedDevices(audioDevUsage);
    AudioPolicyManager::GetInstance().GetDevices(deviceFlag);
    AudioPolicyManager::GetInstance().GetDevicesInner(deviceFlag);
    AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo, forceNoBTPermission);
    AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    AudioPolicyManager::GetInstance().GetOutputDevice(audioRendererFilter);
    AudioPolicyManager::GetInstance().GetInputDevice(audioCapturerFilter);
    AudioPolicyManager::GetInstance().SetDeviceActive(DEVICE_TYPE_SPEAKER, active, uid);
    AudioPolicyManager::GetInstance().GetActiveOutputDevice();
    AudioPolicyManager::GetInstance().GetDmDeviceType();
    AudioPolicyManager::GetInstance().GetActiveInputDevice();
    AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, deviceFlag, audioManagerDeviceChangeCallback);
    AudioPolicyManager::GetInstance().UnsetDeviceChangeCallback(clientId, deviceFlag, audioManagerDeviceChangeCallback);
    AudioPolicyManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, outputDeviceChangeCallback);
    AudioPolicyManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, inputDeviceChangeCallback);
    AudioPolicyManager::GetInstance().UnsetPreferredOutputDeviceChangeCallback(outputDeviceChangeCallback);
    AudioPolicyManager::GetInstance().UnsetPreferredInputDeviceChangeCallback(inputDeviceChangeCallback);
}

void AudioPolicyManagerDeviceTwoFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    std::weak_ptr<DeviceChangeWithInfoCallback> deviceChangeWithInfoCallback;
    AudioDeviceUsage audioDevUsage = GetData<AudioDeviceUsage>();
    int32_t clientId = GetData<int32_t>();
    std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> AMADCCallback;
    bool active = GetData<bool>();
    std::string address = "address";
    int32_t uid = GetData<int32_t>();
    AudioStreamChangeInfo streamChangeInfo;
    AudioStreamDeviceChangeReasonExt reason;
    PreferredType preferredType = GetData<PreferredType>();
    std::shared_ptr<AudioDeviceDescriptor> desc;
    std::shared_ptr<AudioDeviceAnahs> audioDeviceAnahs;
    AudioPipeType pipeType = GetData<AudioPipeType>();
    bool isConnected = GetData<bool>();
    DeviceInfoUpdateCommand command = GetData<DeviceInfoUpdateCommand>();
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;

    AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(sessionId, deviceChangeWithInfoCallback);
    AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(sessionId);
    AudioPolicyManager::GetInstance().GetAvailableDevices(audioDevUsage);
    AudioPolicyManager::GetInstance().SetAvailableDeviceChangeCallback(clientId, audioDevUsage, AMADCCallback);
    AudioPolicyManager::GetInstance().UnsetAvailableDeviceChangeCallback(clientId, audioDevUsage);
    AudioPolicyManager::GetInstance().SetCallDeviceActive(DEVICE_TYPE_SPEAKER, active, address, uid);
    AudioPolicyManager::GetInstance().GetActiveBluetoothDevice();
    AudioPolicyManager::GetInstance().FetchOutputDeviceForTrack(streamChangeInfo, reason);
    AudioPolicyManager::GetInstance().FetchInputDeviceForTrack(streamChangeInfo);
    AudioPolicyManager::GetInstance().TriggerFetchDevice(reason);
    AudioPolicyManager::GetInstance().SetPreferredDevice(preferredType, desc, uid);
    AudioPolicyManager::GetInstance().SetAudioDeviceAnahsCallback(audioDeviceAnahs);
    AudioPolicyManager::GetInstance().UnsetAudioDeviceAnahsCallback();
    AudioPolicyManager::GetInstance().SetDeviceConnectionStatus(desc, isConnected);
    AudioPolicyManager::GetInstance().UpdateDeviceInfo(desc, command);
    AudioPolicyManager::GetInstance().SetSleAudioOperationCallback(sleAudioOperationCallback);
}

void AudioPolicyManagerIsDeviceActiveFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    InternalDeviceType deviceType = GetData<InternalDeviceType>();
    audioPolicyManager.IsDeviceActive(deviceType);
}

void AudioPolicyManagerUnsetDeviceChangeCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    int32_t clientId = GetData<int32_t>();
    DeviceFlag flag = GetData<DeviceFlag>();
    std::shared_ptr<AudioManagerDeviceChangeCallback> cb = std::make_shared<AudioManagerDeviceChangeCallbackFuzzTest>();
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();

    audioPolicyManager.UnsetDeviceChangeCallback(clientId, flag, cb);
}

void AudioPolicyManagerSetPreferredInputDeviceChangeCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;

    std::shared_ptr<AudioPreferredInputDeviceChangeCallback> callback =
        std::make_shared<AudioPreferredInputDeviceChangeCallbackFuzzTest>();
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    AudioCapturerInfo capturerInfo;
    audioPolicyManager.isAudioPolicyClientRegisted_ = GetData<bool>();
    audioPolicyManager.SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
}

void AudioPolicyManagerUnsetPreferredOutputDeviceChangeCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> callback =
        std::make_shared<AudioPreferredOutputDeviceChangeCallbackFuzzTest>();
    audioPolicyManager.UnsetPreferredOutputDeviceChangeCallback(callback);
}

void AudioPolicyManagerUnsetPreferredInputDeviceChangeCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    std::shared_ptr<AudioPreferredInputDeviceChangeCallback> callback =
        std::make_shared<AudioPreferredInputDeviceChangeCallbackFuzzTest>();
    audioPolicyManager.UnsetPreferredInputDeviceChangeCallback(callback);
}

void AudioPolicyManagerRegisterDeviceChangeWithInfoCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    std::shared_ptr<DeviceChangeWithInfoCallback> callbackByshared =
        std::make_shared<DeviceChangeWithInfoCallbackFuzzTest>();
    std::weak_ptr<DeviceChangeWithInfoCallback> callback = callbackByshared;
    uint32_t sessionID = GetData<uint32_t>();
    audioPolicyManager.isAudioPolicyClientRegisted_ = GetData<bool>();
    audioPolicyManager.RegisterDeviceChangeWithInfoCallback(sessionID, callback);
}

void AudioPolicyManagerUnregisterDeviceChangeWithInfoCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    uint32_t sessionID = GetData<uint32_t>();
    audioPolicyManager.UnregisterDeviceChangeWithInfoCallback(sessionID);
}

void AudioPolicyManagerSetAvailableDeviceChangeCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    int32_t clientId = GetData<int32_t>();
    AudioDeviceUsage usage = GetData<AudioDeviceUsage>();
    std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> callback =
        make_shared<AudioManagerAvailableDeviceChangeCallbackFuzzTest>();

    audioPolicyManager.SetAvailableDeviceChangeCallback(clientId, usage, callback);
}

void AudioPolicyManagerSetAudioDeviceAnahsCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    audioPolicyManager.audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    bool isNullptr = GetData<bool>();
    if (isNullptr) {
        audioPolicyManager.audioPolicyClientStubCB_ = nullptr;
    }
    std::shared_ptr<AudioDeviceAnahs> callback = make_shared<AudioDeviceAnahsFuzzTest>();

    audioPolicyManager.SetAudioDeviceAnahsCallback(callback);
}

void AudioPolicyManagerSetSleAudioOperationCallbackFuzzTest()
{
    AudioPolicyManager audioPolicyManager;
    std::shared_ptr<SleAudioOperationCallback> callback = make_shared<SleAudioDeviceManager>();

    audioPolicyManager.SetSleAudioOperationCallback(callback);
}

typedef void (*TestFuncs[21])();

TestFuncs g_testFuncs = {
    AudioPolicyManagerOneFuzzTest,
    AudioPolicyManagerTwoFuzzTest,
    AudioPolicyManagerThreeFuzzTest,
    AudioPolicyManagerFourFuzzTest,
    AudioPolicyManagerFiveFuzzTest,
    AudioPolicyManagerSixFuzzTest,
    AudioPolicyManagerSevenFuzzTest,
    AudioPolicyManagerEightFuzzTest,
    AudioPolicyManagerNiNeFuzzTest,
    AudioPolicyManagerDeviceOneFuzzTest,
    AudioPolicyManagerDeviceTwoFuzzTest,
    AudioPolicyManagerIsDeviceActiveFuzzTest,
    AudioPolicyManagerUnsetDeviceChangeCallbackFuzzTest,
    AudioPolicyManagerSetPreferredInputDeviceChangeCallbackFuzzTest,
    AudioPolicyManagerUnsetPreferredOutputDeviceChangeCallbackFuzzTest,
    AudioPolicyManagerUnsetPreferredInputDeviceChangeCallbackFuzzTest,
    AudioPolicyManagerRegisterDeviceChangeWithInfoCallbackFuzzTest,
    AudioPolicyManagerUnregisterDeviceChangeWithInfoCallbackFuzzTest,
    AudioPolicyManagerSetAvailableDeviceChangeCallbackFuzzTest,
    AudioPolicyManagerSetAudioDeviceAnahsCallbackFuzzTest,
    AudioPolicyManagerSetSleAudioOperationCallbackFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}