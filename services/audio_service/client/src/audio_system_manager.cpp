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
#define LOG_TAG "AudioSystemManager"
#endif

#include "audio_system_manager.h"

#include "app_bundle_manager.h"
#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_type_convert.h"
#include "audio_volume_client_manager.h"
#include "audio_stream_client_manager.h"
#include "audio_devices_client_manager.h"
#include "audio_interrupt_client_manager.h"
#include "audio_system_client_engine_manager.h"
#include "audio_system_client_policy_manager.h"
#include "audio_asr_client_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioSystemManager::AudioSystemManager()
{
    AUDIO_DEBUG_LOG("AudioSystemManager start");
}

AudioSystemManager::~AudioSystemManager()
{
}

AudioSystemManager *AudioSystemManager::GetInstance()
{
    static AudioSystemManager audioManager;
    return &audioManager;
}

AudioStreamType AudioSystemManager::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    return AudioTypeConvert::GetStreamType(contentType, streamUsage);
}

int32_t AudioSystemManager::RegisterRendererDataTransferCallback(const DataTransferMonitorParam &param,
    const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback)
{
    return AudioStreamClientManager::GetInstance().RegisterRendererDataTransferCallback(param, callback);
}


int32_t AudioSystemManager::UnregisterRendererDataTransferCallback(
    const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback)
{
    return AudioStreamClientManager::GetInstance().UnregisterRendererDataTransferCallback(callback);
}

int32_t AudioSystemManager::SetRingerMode(AudioRingerMode ringMode)
{
    return AudioSystemClientPolicyManager::GetInstance().SetRingerMode(ringMode);
}

std::string AudioSystemManager::GetSelfBundleName(int32_t uid)
{
    return AppBundleManager::GetSelfBundleName(uid);
}

AudioRingerMode AudioSystemManager::GetRingerMode()
{
    return AudioSystemClientPolicyManager::GetInstance().GetRingerMode();
}

int32_t AudioSystemManager::SetAudioScene(const AudioScene &scene)
{
    return AudioSystemClientPolicyManager::GetInstance().SetAudioScene(scene);
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    return AudioSystemClientPolicyManager::GetInstance().GetAudioScene();
}

int32_t AudioSystemManager::SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientUid) const
{
    return AudioDevicesClientManager::GetInstance().SetDeviceActive(deviceType, flag, clientUid);
}

bool AudioSystemManager::IsDeviceActive(DeviceType deviceType) const
{
    return AudioDevicesClientManager::GetInstance().IsDeviceActive(deviceType);
}

DeviceType AudioSystemManager::GetActiveOutputDevice()
{
    return AudioDevicesClientManager::GetInstance().GetActiveOutputDevice();
}

DeviceType AudioSystemManager::GetActiveInputDevice()
{
    return AudioDevicesClientManager::GetInstance().GetActiveInputDevice();
}

bool AudioSystemManager::IsStreamActive(AudioVolumeType volumeType) const
{
    return AudioVolumeClientManager::GetInstance().IsStreamActive(volumeType);
}

int32_t AudioSystemManager::SetAsrAecMode(const AsrAecMode asrAecMode)
{
    return AudioAsrClientManager::GetInstance().SetAsrAecMode(asrAecMode);
}

int32_t AudioSystemManager::GetAsrAecMode(AsrAecMode &asrAecMode)
{
    return AudioAsrClientManager::GetInstance().GetAsrAecMode(asrAecMode);
}

int32_t AudioSystemManager::SetAsrNoiseSuppressionMode(const AsrNoiseSuppressionMode asrNoiseSuppressionMode)
{
    return AudioAsrClientManager::GetInstance().SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
}

int32_t AudioSystemManager::GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode)
{
    return AudioAsrClientManager::GetInstance().GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
}

int32_t AudioSystemManager::SetAsrWhisperDetectionMode(const AsrWhisperDetectionMode asrWhisperDetectionMode)
{
    return AudioAsrClientManager::GetInstance().SetAsrWhisperDetectionMode(asrWhisperDetectionMode);
}

int32_t AudioSystemManager::GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode)
{
    return AudioAsrClientManager::GetInstance().GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
}

int32_t AudioSystemManager::SetAsrVoiceControlMode(const AsrVoiceControlMode asrVoiceControlMode, bool on)
{
    return AudioAsrClientManager::GetInstance().SetAsrVoiceControlMode(asrVoiceControlMode, on);
}

int32_t AudioSystemManager::SetAsrVoiceMuteMode(const AsrVoiceMuteMode asrVoiceMuteMode, bool on)
{
    return AudioAsrClientManager::GetInstance().SetAsrVoiceMuteMode(asrVoiceMuteMode, on);
}

int32_t AudioSystemManager::IsWhispering()
{
    return AudioSystemClientEngineManager::GetInstance().IsWhispering();
}

const std::string AudioSystemManager::GetAudioParameter(const std::string key)
{
    return AudioSystemClientEngineManager::GetInstance().GetAudioParameter(key);
}

void AudioSystemManager::SetAudioParameter(const std::string &key, const std::string &value)
{
    return AudioSystemClientEngineManager::GetInstance().SetAudioParameter(key, value);
}

int32_t AudioSystemManager::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    return AudioSystemClientEngineManager::GetInstance().GetExtraParameters(mainKey, subKeys, result);
}

int32_t AudioSystemManager::SetExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    return AudioSystemClientEngineManager::GetInstance().SetExtraParameters(key, kvpairs);
}

uint64_t AudioSystemManager::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    return AudioSystemClientEngineManager::GetInstance().GetTransactionId(deviceType, deviceRole);
}

int32_t AudioSystemManager::SetSelfAppVolume(int32_t volume, int32_t flag)
{
    return AudioVolumeClientManager::GetInstance().SetSelfAppVolume(volume);
}

// LCOV_EXCL_START
int32_t AudioSystemManager::SetAppVolume(int32_t appUid, int32_t volume, int32_t flag)
{
    return AudioVolumeClientManager::GetInstance().SetAppVolume(appUid, volume, flag);
}

int32_t AudioSystemManager::GetAppVolume(int32_t appUid, int32_t &volumeLevel) const
{
    return AudioVolumeClientManager::GetInstance().GetAppVolume(appUid, volumeLevel);
}

int32_t AudioSystemManager::GetSelfAppVolume(int32_t &volumeLevel) const
{
    return AudioVolumeClientManager::GetInstance().GetSelfAppVolume(volumeLevel);
}

int32_t AudioSystemManager::SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag)
{
    return AudioVolumeClientManager::GetInstance().SetAppVolumeMuted(appUid, muted, volumeFlag);
}

int32_t AudioSystemManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    return AudioVolumeClientManager::GetInstance().SetAppRingMuted(appUid, muted);
}
// LCOV_EXCL_STOP

int32_t AudioSystemManager::UnsetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnsetSelfAppVolumeCallback(callback);
}

int32_t AudioSystemManager::SetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().SetSelfAppVolumeCallback(callback);
}

int32_t AudioSystemManager::SetAppVolumeCallbackForUid(const int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().SetAppVolumeCallbackForUid(appUid, callback);
}

int32_t AudioSystemManager::UnsetAppVolumeCallbackForUid(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnsetAppVolumeCallbackForUid(callback);
}

int32_t AudioSystemManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    return AudioVolumeClientManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
}

int32_t AudioSystemManager::UnsetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnsetActiveVolumeTypeCallback(callback);
}

int32_t AudioSystemManager::SetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().SetActiveVolumeTypeCallback(callback);
}

int32_t AudioSystemManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel, int32_t uid)
{
    return AudioVolumeClientManager::GetInstance().SetVolume(volumeType, volumeLevel, uid);
}

int32_t AudioSystemManager::SetVolumeWithDevice(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType)
{
    return AudioVolumeClientManager::GetInstance().SetVolumeWithDevice(volumeType, volumeLevel, deviceType);
}

int32_t AudioSystemManager::GetVolume(AudioVolumeType volumeType, int32_t uid) const
{
    return AudioVolumeClientManager::GetInstance().GetVolume(volumeType, uid);
}

int32_t AudioSystemManager::SetLowPowerVolume(int32_t streamId, float volume) const
{
    return AudioVolumeClientManager::GetInstance().SetLowPowerVolume(streamId, volume);
}

float AudioSystemManager::GetLowPowerVolume(int32_t streamId) const
{
    return AudioVolumeClientManager::GetInstance().GetLowPowerVolume(streamId);
}

float AudioSystemManager::GetSingleStreamVolume(int32_t streamId) const
{
    return AudioVolumeClientManager::GetInstance().GetSingleStreamVolume(streamId);
}

int32_t AudioSystemManager::GetMaxVolume(AudioVolumeType volumeType)
{
    return AudioVolumeClientManager::GetInstance().GetMaxVolume(volumeType);
}

int32_t AudioSystemManager::GetMinVolume(AudioVolumeType volumeType)
{
    return AudioVolumeClientManager::GetInstance().GetMinVolume(volumeType);
}

int32_t AudioSystemManager::GetDeviceMaxVolume(AudioVolumeType volumeType, DeviceType deviceType)
{
    return AudioVolumeClientManager::GetInstance().GetDeviceMaxVolume(volumeType, deviceType);
}

int32_t AudioSystemManager::GetDeviceMinVolume(AudioVolumeType volumeType, DeviceType deviceType)
{
    return AudioVolumeClientManager::GetInstance().GetDeviceMinVolume(volumeType, deviceType);
}

int32_t AudioSystemManager::SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType)
{
    return AudioVolumeClientManager::GetInstance().SetMute(volumeType, mute, deviceType);
}

bool AudioSystemManager::IsStreamMute(AudioVolumeType volumeType) const
{
    return AudioVolumeClientManager::GetInstance().IsStreamMute(volumeType);
}

float AudioSystemManager::GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device)
{
    return AudioVolumeClientManager::GetInstance().GetVolumeInUnitOfDb(volumeType, volumeLevel, device);
}

int32_t AudioSystemManager::SetDeviceChangeCallback(const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    return AudioDevicesClientManager::GetInstance().SetDeviceChangeCallback(flag, callback);
}

int32_t AudioSystemManager::UnsetDeviceChangeCallback(DeviceFlag flag,
    std::shared_ptr<AudioManagerDeviceChangeCallback> cb)
{
    return AudioDevicesClientManager::GetInstance().UnsetDeviceChangeCallback(flag, cb);
}

int32_t AudioSystemManager::SetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback>& callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetMicrophoneBlockedCallback(callback);
}

int32_t AudioSystemManager::UnsetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback)
{
    return AudioSystemClientPolicyManager::GetInstance().UnsetMicrophoneBlockedCallback(callback);
}

int32_t AudioSystemManager::SetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback>& callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetAudioSceneChangeCallback(callback);
}

int32_t AudioSystemManager::UnsetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback)
{
    return AudioSystemClientPolicyManager::GetInstance().UnsetAudioSceneChangeCallback(callback);
}

int32_t AudioSystemManager::SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetQueryClientTypeCallback(callback);
}

int32_t AudioSystemManager::SetAudioClientInfoMgrCallback(const std::shared_ptr<AudioClientInfoMgrCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetAudioClientInfoMgrCallback(callback);
}

int32_t AudioSystemManager::SetAudioVKBInfoMgrCallback(const std::shared_ptr<AudioVKBInfoMgrCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetAudioVKBInfoMgrCallback(callback);
}

int32_t AudioSystemManager::CheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    return AudioSystemClientPolicyManager::GetInstance().CheckVKBInfo(bundleName, isValid);
}

int32_t AudioSystemManager::SetQueryBundleNameListCallback(
    const std::shared_ptr<AudioQueryBundleNameListCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetQueryBundleNameListCallback(callback);
}

int32_t AudioSystemManager::SetRingerModeCallback(const int32_t clientId,
                                                  const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetRingerModeCallback(clientId, callback);
}

int32_t AudioSystemManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    return AudioSystemClientPolicyManager::GetInstance().UnsetRingerModeCallback(clientId);
}

int32_t AudioSystemManager::SetMicrophoneMute(bool isMute)
{
    return AudioVolumeClientManager::GetInstance().SetMicrophoneMute(isMute);
}

int32_t AudioSystemManager::SetVoiceRingtoneMute(bool isMute)
{
    return AudioVolumeClientManager::GetInstance().SetVoiceRingtoneMute(isMute);
}

bool AudioSystemManager::IsMicrophoneMute()
{
    return AudioVolumeClientManager::GetInstance().IsMicrophoneMute();
}

int32_t AudioSystemManager::SelectOutputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    return AudioDevicesClientManager::GetInstance().SelectOutputDevice(audioDeviceDescriptors);
}

int32_t AudioSystemManager::SelectInputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    return AudioDevicesClientManager::GetInstance().SelectInputDevice(audioDeviceDescriptors);
}

std::string AudioSystemManager::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const
{
    return AudioDevicesClientManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioSystemManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors,
    const int32_t audioDeviceSelectMode) const
{
    return AudioDevicesClientManager::GetInstance().SelectOutputDevice(audioRendererFilter,
        audioDeviceDescriptors, audioDeviceSelectMode);
}

int32_t AudioSystemManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    return AudioDevicesClientManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

// LCOV_EXCL_START
int32_t AudioSystemManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    return AudioDevicesClientManager::GetInstance().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioSystemManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    return AudioDevicesClientManager::GetInstance().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioSystemManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage) const
{
    return AudioDevicesClientManager::GetInstance().UnexcludeOutputDevices(audioDevUsage);
}
// LCOV_EXCL_STOP

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage) const
{
    return AudioDevicesClientManager::GetInstance().GetExcludedDevices(audioDevUsage);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevices(DeviceFlag deviceFlag)
{
    return AudioDevicesClientManager::GetInstance().GetDevices(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevicesInner(DeviceFlag deviceFlag)
{
    return AudioDevicesClientManager::GetInstance().GetDevicesInner(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetActiveOutputDeviceDescriptors()
{
    return AudioDevicesClientManager::GetInstance().GetActiveOutputDeviceDescriptors();
}

int32_t AudioSystemManager::GetPreferredInputDeviceDescriptors()
{
    return AudioDevicesClientManager::GetInstance().GetPreferredInputDeviceDescriptors();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    return AudioDevicesClientManager::GetInstance().GetOutputDevice(audioRendererFilter);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    return AudioDevicesClientManager::GetInstance().GetInputDevice(audioCapturerFilter);
}

int32_t AudioSystemManager::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    return AudioInterruptClientManager::GetInstance().GetAudioFocusInfoList(focusInfoList);
}

int32_t AudioSystemManager::RegisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    return AudioInterruptClientManager::GetInstance().RegisterFocusInfoChangeCallback(callback);
}

int32_t AudioSystemManager::UnregisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    return AudioInterruptClientManager::GetInstance().UnregisterFocusInfoChangeCallback(callback);
}

int32_t AudioSystemManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    return AudioVolumeClientManager::GetInstance().RegisterVolumeKeyEventCallback(clientPid, callback, api_v);
}

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnregisterVolumeKeyEventCallback(clientPid, callback);
}

int32_t AudioSystemManager::RegisterVolumeDegreeCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    return AudioVolumeClientManager::GetInstance().RegisterVolumeDegreeCallback(clientPid, callback);
}

int32_t AudioSystemManager::UnregisterVolumeDegreeCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnregisterVolumeDegreeCallback(clientPid, callback);
}

int32_t AudioSystemManager::RegisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().RegisterSystemVolumeChangeCallback(clientPid, callback);
}

int32_t AudioSystemManager::UnregisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnregisterSystemVolumeChangeCallback(clientPid, callback);
}

void AudioSystemManager::SetAudioMonoState(bool monoState)
{
    AudioSystemClientEngineManager::GetInstance().SetAudioMonoState(monoState);
}

void AudioSystemManager::SetAudioBalanceValue(float balanceValue)
{
    AudioSystemClientEngineManager::GetInstance().SetAudioBalanceValue(balanceValue);
}

int32_t AudioSystemManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    return AudioSystemClientPolicyManager::GetInstance().SetSystemSoundUri(key, uri);
}

std::string AudioSystemManager::GetSystemSoundUri(const std::string &key)
{
    return AudioSystemClientPolicyManager::GetInstance().GetSystemSoundUri(key);
}

std::string AudioSystemManager::GetSystemSoundPath(const int32_t systemSoundType)
{
    return AudioSystemClientPolicyManager::GetInstance().GetSystemSoundPath(systemSoundType);
}

// Below stub implementation is added to handle compilation error in call manager
// Once call manager adapt to new interrupt implementation, this will be removed
int32_t AudioSystemManager::SetAudioManagerCallback(const AudioVolumeType streamType,
                                                    const std::shared_ptr<AudioManagerCallback> &callback)
{
    AUDIO_DEBUG_LOG("stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerCallback(const AudioVolumeType streamType) const
{
    AUDIO_DEBUG_LOG("stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::ActivateAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    return AudioInterruptClientManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
}

int32_t AudioSystemManager::SetAppConcurrencyMode(const int32_t appUid, const int32_t mode)
{
    return AudioInterruptClientManager::GetInstance().SetAppConcurrencyMode(appUid, mode);
}

int32_t AudioSystemManager::SetAppSilentOnDisplay(const int32_t displayId)
{
    return AudioSystemClientPolicyManager::GetInstance().SetAppSilentOnDisplay(displayId);
}

int32_t AudioSystemManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const
{
    return AudioInterruptClientManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
}

int32_t AudioSystemManager::ActivatePreemptMode() const
{
    return AudioSystemClientPolicyManager::GetInstance().ActivatePreemptMode();
}

int32_t AudioSystemManager::DeactivatePreemptMode() const
{
    return AudioSystemClientPolicyManager::GetInstance().DeactivatePreemptMode();
}

int32_t AudioSystemManager::SetForegroundList(std::vector<std::string> list)
{
    return AudioSystemClientEngineManager::GetInstance().SetForegroundList(list);
}

int32_t AudioSystemManager::GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime)
{
    return AudioSystemClientEngineManager::GetInstance().GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioSystemManager::CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId)
{
    return AudioSystemClientEngineManager::GetInstance().CheckCaptureLimit(config, innerCapId);
}

int32_t AudioSystemManager::ReleaseCaptureLimit(int32_t innerCapId)
{
    return AudioSystemClientEngineManager::GetInstance().ReleaseCaptureLimit(innerCapId);
}
#endif

int32_t AudioSystemManager::GenerateSessionId(uint32_t &sessionId)
{
    return AudioSystemClientEngineManager::GetInstance().GenerateSessionId(sessionId);
}

int32_t AudioSystemManager::SetAudioInterruptCallback(const uint32_t sessionID,
    const std::shared_ptr<AudioInterruptCallback> &callback, uint32_t clientUid, const int32_t zoneID)
{
    return AudioInterruptClientManager::GetInstance().SetAudioInterruptCallback(sessionID, callback, clientUid, zoneID);
}

int32_t AudioSystemManager::UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId)
{
    return AudioInterruptClientManager::GetInstance().UnsetAudioInterruptCallback(zoneId, sessionId);
}

int32_t AudioSystemManager::SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback)
{
    return AudioInterruptClientManager::GetInstance().SetAudioManagerInterruptCallback(callback);
}

int32_t AudioSystemManager::UnsetAudioManagerInterruptCallback()
{
    return AudioInterruptClientManager::GetInstance().UnsetAudioManagerInterruptCallback();
}

int32_t AudioSystemManager::RequestAudioFocus(const AudioInterrupt &audioInterrupt)
{
    return AudioInterruptClientManager::GetInstance().RequestAudioFocus(audioInterrupt);
}

int32_t AudioSystemManager::AbandonAudioFocus(const AudioInterrupt &audioInterrupt)
{
    return AudioInterruptClientManager::GetInstance().AbandonAudioFocus(audioInterrupt);
}

int32_t AudioSystemManager::GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return AudioVolumeClientManager::GetInstance().GetVolumeGroups(networkId, infos);
}

std::shared_ptr<AudioGroupManager> AudioSystemManager::GetGroupManager(int32_t groupId)
{
    return AudioVolumeClientManager::GetInstance().GetGroupManager(groupId);
}

bool AudioSystemManager::RequestIndependentInterrupt(FocusType focusType)
{
    return AudioInterruptClientManager::GetInstance().RequestIndependentInterrupt(focusType);
}
bool AudioSystemManager::AbandonIndependentInterrupt(FocusType focusType)
{
    return AudioInterruptClientManager::GetInstance().AbandonIndependentInterrupt(focusType);
}

int32_t AudioSystemManager::UpdateStreamState(const int32_t clientUid,
    StreamSetState streamSetState, StreamUsage streamUsage)
{
    return AudioSystemClientPolicyManager::GetInstance().UpdateStreamState(clientUid, streamSetState, streamUsage);
}

std::string AudioSystemManager::GetSelfBundleName()
{
    return AppBundleManager::GetSelfBundleName();
}

int32_t AudioSystemManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support,
    int32_t volume)
{
    return AudioVolumeClientManager::GetInstance().SetDeviceAbsVolumeSupported(macAddress, support, volume);
}

int32_t AudioSystemManager::SetAdjustVolumeForZone(int32_t zoneId)
{
    return AudioVolumeClientManager::GetInstance().SetAdjustVolumeForZone(zoneId);
}

int32_t AudioSystemManager::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    return AudioVolumeClientManager::GetInstance().SetA2dpDeviceVolume(macAddress, volume, updateUi);
}

int32_t AudioSystemManager::SetNearlinkDeviceVolume(const std::string &macAddress, AudioVolumeType volumeType,
    const int32_t volume, const bool updateUi)
{
    return AudioVolumeClientManager::GetInstance().SetNearlinkDeviceVolume(macAddress, volumeType, volume, updateUi);
}

int32_t AudioSystemManager::SetSleVoiceStatusFlag(bool isSleVoiceStatus)
{
    return AudioSystemClientPolicyManager::GetInstance().SetSleVoiceStatusFlag(isSleVoiceStatus);
}

AudioPin AudioSystemManager::GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole) const
{
    return AudioTypeConvert::GetPinValueFromType(deviceType, deviceRole);
}

DeviceType AudioSystemManager::GetTypeValueFromPin(AudioPin pin) const
{
    return AudioTypeConvert::GetTypeValueFromPin(pin);
}

int32_t AudioSystemManager::SetAudioCapturerSourceCallback(
    const std::shared_ptr<AudioCapturerSourceCallback> &callback)
{
    return AudioWakeupClientManager::GetInstance().SetAudioCapturerSourceCallback(callback);
}

int32_t AudioSystemManager::SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback> &callback)
{
    return AudioWakeupClientManager::GetInstance().SetWakeUpSourceCloseCallback(callback);
}

int32_t AudioSystemManager::SetAvailableDeviceChangeCallback(const AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetAvailableDeviceChangeCallback(usage, callback);
}

int32_t AudioSystemManager::UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage)
{
    return AudioSystemClientPolicyManager::GetInstance().UnsetAvailableDeviceChangeCallback(usage);
}

int32_t AudioSystemManager::ConfigDistributedRoutingRole(
    std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    return AudioSystemClientPolicyManager::GetInstance().ConfigDistributedRoutingRole(descriptor, type);
}

int32_t AudioSystemManager::SetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetDistributedRoutingRoleCallback(callback);
}

int32_t AudioSystemManager::UnsetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().UnsetDistributedRoutingRoleCallback(callback);
}

int32_t AudioSystemManager::SetCallDeviceActive(DeviceType deviceType, bool flag, std::string address,
    const int32_t clientUid) const
{
    return AudioSystemClientPolicyManager::GetInstance().SetCallDeviceActive(deviceType, flag, address, clientUid);
}

uint32_t AudioSystemManager::GetEffectLatency(const std::string &sessionId)
{
    return AudioSystemClientEngineManager::GetInstance().GetEffectLatency(sessionId);
}

int32_t AudioSystemManager::DisableSafeMediaVolume()
{
    return AudioSystemClientPolicyManager::GetInstance().DisableSafeMediaVolume();
}

int32_t AudioSystemManager::InjectInterruption(const std::string networkId, InterruptEvent &event)
{
    return AudioInterruptClientManager::GetInstance().InjectInterruption(networkId, event);
}

int32_t AudioSystemManager::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    return AudioSystemClientPolicyManager::GetInstance().LoadSplitModule(splitArgs, networkId);
}

int32_t AudioSystemManager::SetVirtualCall(const bool isVirtual)
{
    return AudioSystemClientPolicyManager::GetInstance().SetVirtualCall(isVirtual);
}

bool AudioSystemManager::GetVirtualCall()
{
    return AudioSystemClientPolicyManager::GetInstance().GetVirtualCall();
}

int32_t AudioSystemManager::SetQueryAllowedPlaybackCallback(
    const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetQueryAllowedPlaybackCallback(callback);
}

int32_t AudioSystemManager::SetBackgroundMuteCallback(
    const std::shared_ptr<AudioBackgroundMuteCallback> &callback)
{
    return AudioSystemClientPolicyManager::GetInstance().SetBackgroundMuteCallback(callback);
}

int32_t AudioSystemManager::OnVoiceWakeupState(bool state)
{
    AUDIO_INFO_LOG("%{public}d", state);
    return SUCCESS;
}

int32_t AudioSystemManager::NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession)
{
    return AudioSystemClientPolicyManager::GetInstance().NotifySessionStateChange(uid, pid, hasSession);
}

int32_t AudioSystemManager::NotifyFreezeStateChange(const std::set<int32_t> &pidList, const bool isFreeze)
{
    return AudioSystemClientPolicyManager::GetInstance().NotifyFreezeStateChange(pidList, isFreeze);
}

int32_t AudioSystemManager::ResetAllProxy()
{
    return AudioSystemClientPolicyManager::GetInstance().ResetAllProxy();
}

int32_t AudioSystemManager::NotifyProcessBackgroundState(const int32_t uid, const int32_t pid)
{
    return AudioSystemClientPolicyManager::GetInstance().NotifyProcessBackgroundState(uid, pid);
}

int32_t AudioSystemManager::GetMaxVolumeByUsage(StreamUsage streamUsage)
{
    return AudioVolumeClientManager::GetInstance().GetMaxVolumeByUsage(streamUsage);
}

int32_t AudioSystemManager::GetMinVolumeByUsage(StreamUsage streamUsage)
{
    return AudioVolumeClientManager::GetInstance().GetMinVolumeByUsage(streamUsage);
}

int32_t AudioSystemManager::GetVolumeByUsage(StreamUsage streamUsage)
{
    return AudioVolumeClientManager::GetInstance().GetVolumeByUsage(streamUsage);
}

int32_t AudioSystemManager::IsStreamMuteByUsage(StreamUsage streamUsage, bool &isMute)
{
    return AudioVolumeClientManager::GetInstance().IsStreamMuteByUsage(streamUsage, isMute);
}

float AudioSystemManager::GetVolumeInDbByStream(StreamUsage streamUsage, int32_t volumeLevel, DeviceType deviceType)
{
    return AudioVolumeClientManager::GetInstance().GetVolumeInDbByStream(streamUsage, volumeLevel, deviceType);
}

std::vector<AudioVolumeType> AudioSystemManager::GetSupportedAudioVolumeTypes()
{
    return AudioVolumeClientManager::GetInstance().GetSupportedAudioVolumeTypes();
}

AudioVolumeType AudioSystemManager::GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage)
{
    return AudioVolumeClientManager::GetInstance().GetAudioVolumeTypeByStreamUsage(streamUsage);
}

std::vector<StreamUsage> AudioSystemManager::GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType)
{
    return AudioVolumeClientManager::GetInstance().GetStreamUsagesByVolumeType(audioVolumeType);
}

int32_t AudioSystemManager::RegisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::set<StreamUsage> &streamUsages, const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().RegisterStreamVolumeChangeCallback(
        clientPid, streamUsages, callback);
}

int32_t AudioSystemManager::UnregisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    return AudioVolumeClientManager::GetInstance().UnregisterStreamVolumeChangeCallback(clientPid, callback);
}

int32_t AudioSystemManager::CreateAudioWorkgroup()
{
    return WorkgroupPrioRecorderManager::GetInstance().CreateAudioWorkgroup();
}

int32_t AudioSystemManager::ReleaseAudioWorkgroup(int32_t workgroupId)
{
    return WorkgroupPrioRecorderManager::GetInstance().ReleaseAudioWorkgroup(workgroupId);
}

int32_t AudioSystemManager::AddThreadToGroup(int32_t workgroupId, int32_t tokenId)
{
    return WorkgroupPrioRecorderManager::GetInstance().AddThreadToGroup(workgroupId, tokenId);
}

int32_t AudioSystemManager::RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId)
{
    return WorkgroupPrioRecorderManager::GetInstance().RemoveThreadFromGroup(workgroupId, tokenId);
}

int32_t AudioSystemManager::ExecuteAudioWorkgroupPrioImprove(int32_t workgroupId,
    const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio)
{
    return WorkgroupPrioRecorderManager::GetInstance().ExecuteAudioWorkgroupPrioImprove(
        workgroupId, threads, needUpdatePrio);
}

int32_t AudioSystemManager::StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime,
    const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio)
{
    return WorkgroupPrioRecorderManager::GetInstance().StartGroup(workgroupId, startTime,
        deadlineTime, threads, needUpdatePrio);
}

int32_t AudioSystemManager::StopGroup(int32_t workgroupId)
{
    return WorkgroupPrioRecorderManager::GetInstance().StopGroup(workgroupId);
}

int32_t AudioSystemManager::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    return AudioSystemClientPolicyManager::GetInstance().ForceVolumeKeyControlType(volumeType, duration);
}

int32_t AudioSystemManager::SetRenderWhitelist(std::vector<std::string> list)
{
    return AudioSystemClientEngineManager::GetInstance().SetRenderWhitelist(list);
}
 
std::shared_ptr<WorkgroupPrioRecorder> AudioSystemManager::GetRecorderByGrpId(int32_t grpId)
{
    return WorkgroupPrioRecorderManager::GetInstance().GetRecorderByGrpId(grpId);
}

int32_t AudioSystemManager::GetVolumeBySessionId(const uint32_t &sessionId, float &volume)
{
    return AudioStreamClientManager::GetInstance().GetVolumeBySessionId(sessionId, volume);
}

void AudioSystemManager::CleanUpResource()
{
    AudioSystemClientPolicyManager::GetInstance().CleanUpResource();
}

int32_t AudioSystemManager::SetVolumeDegree(AudioVolumeType volumeType, int32_t degree, int32_t uid)
{
    return AudioVolumeClientManager::GetInstance().SetVolumeDegree(volumeType, degree, uid);
}

int32_t AudioSystemManager::GetVolumeDegree(AudioVolumeType volumeType, int32_t uid)
{
    return AudioVolumeClientManager::GetInstance().GetVolumeDegree(volumeType, uid);
}

int32_t AudioSystemManager::GetMinVolumeDegree(AudioVolumeType volumeType)
{
    return AudioVolumeClientManager::GetInstance().GetMinVolumeDegree(volumeType);
}
} // namespace AudioStandard
} // namespace OHOS
