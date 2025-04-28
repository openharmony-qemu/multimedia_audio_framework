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
#ifndef LOG_TAG
#define LOG_TAG "AudioDeviceLock"
#endif

#include "audio_device_lock.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "media_monitor_manager.h"
#include "audio_state_manager.h"

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
const int32_t DATA_LINK_CONNECTED = 11;
static constexpr int64_t WAIT_LOAD_DEFAULT_DEVICE_TIME_MS = 200; // 200ms
static constexpr int32_t RETRY_TIMES = 25;

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

void AudioDeviceLock::Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager)
{
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
}

void AudioDeviceLock::DeInit()
{
    audioA2dpOffloadManager_ = nullptr;
}

int32_t AudioDeviceLock::SetAudioScene(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    AUDIO_INFO_LOG("Set audio scene start: %{public}d, lastScene: %{public}d, uid: %{public}d, pid: %{public}d",
        audioScene, audioSceneManager_.GetLastAudioScene(), uid, pid);
    audioSceneManager_.SetAudioScenePre(audioScene);
    audioStateManager_.SetAudioSceneOwnerUid(audioScene == 0 ? 0 : uid);
    bool isSameScene = audioSceneManager_.IsSameAudioScene();

    // fetch input&output device
    audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);
    audioDeviceCommon_.FetchDevice(false);

    if (!isSameScene) {
        int32_t result = audioSceneManager_.SetAudioSceneAfter(audioScene, audioA2dpOffloadFlag_.GetA2dpOffloadFlag());
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "failed [%{public}d]", result);
    }
    audioDeviceCommon_.OnAudioSceneChange(audioScene);

    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        // Make sure the STREAM_VOICE_CALL volume is set before the calling starts.
        audioVolumeManager_.SetVoiceCallVolume(audioVolumeManager_.GetSystemVolumeLevel(STREAM_VOICE_CALL));
    } else {
        audioVolumeManager_.SetVoiceRingtoneMute(false);
    }
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SetAudioScene");
    return SUCCESS;
}

bool AudioDeviceLock::IsArmUsbDevice(const AudioDeviceDescriptor &desc)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioDeviceManager_.IsArmUsbDevice(desc);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetDevices(DeviceFlag deviceFlag)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioConnectedDevice_.GetDevicesInner(deviceFlag);
}

int32_t AudioDeviceLock::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    int32_t ret = audioActiveDevice_.SetDeviceActive(deviceType, active, uid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetDeviceActive failed");

    audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    audioDeviceCommon_.FetchDevice(false);
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SetDevcieActive");
    return SUCCESS;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioDeviceCommon_.GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

std::shared_ptr<AudioDeviceDescriptor> AudioDeviceLock::GetActiveBluetoothDevice()
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);

    std::shared_ptr<AudioDeviceDescriptor> preferredDesc = audioStateManager_.GetPreferredCallRenderDevice();
    if (preferredDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        return preferredDesc;
    }

    std::vector<shared_ptr<AudioDeviceDescriptor>> audioPrivacyDeviceDescriptors =
        audioDeviceManager_.GetCommRenderPrivacyDevices();
    std::vector<shared_ptr<AudioDeviceDescriptor>> activeDeviceDescriptors;

    for (const auto &desc : audioPrivacyDeviceDescriptors) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->isEnable_) {
            activeDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
        }
    }

    uint32_t btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        activeDeviceDescriptors = audioDeviceManager_.GetCommRenderBTCarDevices();
    }
    btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        return make_shared<AudioDeviceDescriptor>();
    } else if (btDeviceSize == 1) {
        shared_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[0]);
        return res;
    }

    uint32_t index = 0;
    for (uint32_t i = 1; i < btDeviceSize; ++i) {
        if (activeDeviceDescriptors[i]->connectTimeStamp_ >
            activeDeviceDescriptors[index]->connectTimeStamp_) {
            index = i;
        }
    }
    shared_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[index]);
    return res;
}

void AudioDeviceLock::UpdateAppVolume(int32_t appUid, int32_t volume)
{
    AUDIO_INFO_LOG("appUid = %{public}d, volume = %{public}d", appUid, volume);
    streamCollector_.UpdateAppVolume(appUid, volume);
}

void AudioDeviceLock::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceInfoUpdated(desc, command);
}

int32_t AudioDeviceLock::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    int32_t ret = audioActiveDevice_.SetCallDeviceActive(deviceType, active, address, uid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetCallDeviceActive failed");
    audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    audioDeviceCommon_.FetchDevice(false);
    return SUCCESS;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(usage);
}

void AudioDeviceLock::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    AUDIO_INFO_LOG("fetch device for track, sessionid:%{public}d start",
        streamChangeInfo.audioRendererChangeInfo.sessionId);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    // Set prerunningState true to refetch devices when device info change before update tracker to running
    streamChangeInfo.audioRendererChangeInfo.prerunningState = true;
    if (streamCollector_.UpdateTrackerInternal(mode, streamChangeInfo) != SUCCESS) {
        return;
    }

    vector<shared_ptr<AudioRendererChangeInfo>> rendererChangeInfo;
    rendererChangeInfo.push_back(
        make_shared<AudioRendererChangeInfo>(streamChangeInfo.audioRendererChangeInfo));
    streamCollector_.GetRendererStreamInfo(streamChangeInfo, *rendererChangeInfo[0]);

    audioDeviceManager_.UpdateDefaultOutputDeviceWhenStarting(streamChangeInfo.audioRendererChangeInfo.sessionId);

    audioDeviceCommon_.FetchOutputDevice(rendererChangeInfo, reason);
}

void AudioDeviceLock::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    AUDIO_INFO_LOG("fetch device for track, sessionid:%{public}d start",
        streamChangeInfo.audioCapturerChangeInfo.sessionId);

    vector<shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfo;
    capturerChangeInfo.push_back(
        make_shared<AudioCapturerChangeInfo>(streamChangeInfo.audioCapturerChangeInfo));
    streamCollector_.GetCapturerStreamInfo(streamChangeInfo, *capturerChangeInfo[0]);

    audioDeviceCommon_.FetchInputDevice(capturerChangeInfo);
}


int32_t AudioDeviceLock::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    if (mode == AUDIO_MODE_RECORD) {
        audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
            streamChangeInfo.audioCapturerChangeInfo.sessionId, DEVICE_TYPE_NONE);
        if (apiVersion > 0 && apiVersion < API_11) {
            audioDeviceCommon_.UpdateDeviceInfo(streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo,
                std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentInputDevice()), false, false);
        }
    } else if (apiVersion > 0 && apiVersion < API_11) {
        audioDeviceCommon_.UpdateDeviceInfo(streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo,
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice()), false, false);
    }
    return streamCollector_.RegisterTracker(mode, streamChangeInfo, object);
}

void AudioDeviceLock::UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state)
{
    AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionID, state);
}

void AudioDeviceLock::SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId)
{
    if ((rendererState == RENDERER_RUNNING) && (audioA2dpOffloadManager_ != nullptr) &&
        !audioA2dpOffloadManager_->IsA2dpOffloadConnecting(sessionId)) {
        AUDIO_INFO_LOG("Notify client not to block.");
        std::thread sendConnectedToClient(&AudioDeviceLock::UpdateSessionConnectionState, this, sessionId,
            DATA_LINK_CONNECTED);
        sendConnectedToClient.detach();
    }
}

void AudioDeviceLock::HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_RECORD &&
        (streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_RELEASED ||
         streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_STOPPED)) {
        if (Util::IsScoSupportSource(streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType)) {
            audioDeviceCommon_.BluetoothScoDisconectForRecongnition();
            Bluetooth::AudioHfpManager::ClearRecongnitionStatus();
        } else if (audioDeviceManager_.GetScoState() &&
            audioSceneManager_.GetAudioScene() == AUDIO_SCENE_DEFAULT) {
            AUDIO_INFO_LOG("close capture app, disconnect sco");
            Bluetooth::AudioHfpManager::DisconnectSco();
        }
        audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptorBySessionID(
            streamChangeInfo.audioCapturerChangeInfo.sessionId);
    }
}

int32_t AudioDeviceLock::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    HandleAudioCaptureState(mode, streamChangeInfo);

    int32_t ret = streamCollector_.UpdateTracker(mode, streamChangeInfo);

    const auto &rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    if (rendererState == RENDERER_PREPARED || rendererState == RENDERER_NEW || rendererState == RENDERER_INVALID) {
        return ret; // only update tracker in new and prepared
    }

    audioDeviceCommon_.UpdateTracker(mode, streamChangeInfo, rendererState);

    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(audioActiveDevice_.GetCurrentOutputDeviceType());
    }
    SendA2dpConnectedWhileRunning(rendererState, streamChangeInfo.audioRendererChangeInfo.sessionId);
    return ret;
}

void AudioDeviceLock::UpdateDefaultOutputDeviceWhenStopping(int32_t uid)
{
    std::vector<uint32_t> sessionIDSet = streamCollector_.GetAllRendererSessionIDForUID(uid);
    for (const auto &sessionID : sessionIDSet) {
        audioDeviceManager_.UpdateDefaultOutputDeviceWhenStopping(sessionID);
        audioDeviceManager_.RemoveSelectedDefaultOutputDevice(sessionID);
    }
    audioDeviceCommon_.FetchDevice(true);
}

void AudioDeviceLock::RegisteredTrackerClientDied(pid_t uid)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    UpdateDefaultOutputDeviceWhenStopping(static_cast<int32_t>(uid));

    audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptor(static_cast<int32_t>(uid));
    streamCollector_.RegisteredTrackerClientDied(static_cast<int32_t>(uid));

    audioDeviceCommon_.ClientDiedDisconnectScoNormal();
    audioDeviceCommon_.ClientDiedDisconnectScoRecognition();

    if (!streamCollector_.ExistStreamForPipe(PIPE_TYPE_OFFLOAD)) {
        audioOffloadStream_.DynamicUnloadOffloadModule();
    }

    if (!streamCollector_.ExistStreamForPipe(PIPE_TYPE_MULTICHANNEL)) {
        audioOffloadStream_.UnloadMchModule();
    }
}

void AudioDeviceLock::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    // Pnp device status update
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioDeviceLock::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    // Bluetooth device status updated
    DeviceType devType = updatedDesc.deviceType_;
    string macAddress = updatedDesc.macAddress_;
    string deviceName = updatedDesc.deviceName_;
    bool isActualConnection = (updatedDesc.connectState_ != VIRTUAL_CONNECTED);
    AUDIO_INFO_LOG("Device connection is actual connection: %{public}d", isActualConnection);

    AudioStreamInfo streamInfo = {};
#ifdef BLUETOOTH_ENABLE
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp device stream info failed!");
    }
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP_IN && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpInDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp input device stream info failed!");
    }
    if (isConnected && isActualConnection
        && devType == DEVICE_TYPE_BLUETOOTH_SCO
        && updatedDesc.deviceCategory_ != BT_UNWEAR_HEADPHONE
        && !audioDeviceManager_.GetScoState()) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(macAddress);
    }
#endif
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceStatusUpdated(updatedDesc, devType,
        macAddress, deviceName, isActualConnection, streamInfo, isConnected);
}

void AudioDeviceLock::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

static void UpdateRendererInfoWhenNoPermission(const shared_ptr<AudioRendererChangeInfo> &audioRendererChangeInfos,
    bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioRendererChangeInfos->clientUID = 0;
        audioRendererChangeInfos->rendererState = RENDERER_INVALID;
    }
}

int32_t AudioDeviceLock::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);

    int32_t status = streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get renderer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices =
        audioConnectedDevice_.GetDevicesInner(OUTPUT_DEVICES_FLAG);
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    DeviceRole activeDeviceRole = OUTPUT_DEVICE;
    std::string activeDeviceMac = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();

    const auto& itr = std::find_if(outputDevices.begin(), outputDevices.end(),
        [&activeDeviceType, &activeDeviceRole, &activeDeviceMac](const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            // This A2DP device is not the active A2DP device. Skip it.
            return activeDeviceType != DEVICE_TYPE_BLUETOOTH_A2DP || desc->macAddress_ == activeDeviceMac;
        }
        return false;
    });

    if (itr != outputDevices.end()) {
        size_t rendererInfosSize = audioRendererChangeInfos.size();
        for (size_t i = 0; i < rendererInfosSize; i++) {
            UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos[i], hasSystemPermission);
            audioDeviceCommon_.UpdateDeviceInfo(audioRendererChangeInfos[i]->outputDeviceInfo, *itr,
                hasBTPermission, hasSystemPermission);
        }
    }

    return status;
}

std::vector<sptr<MicrophoneDescriptor>> AudioDeviceLock::GetAvailableMicrophones()
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioMicrophoneDescriptor_.GetAvailableMicrophones();
}

std::vector<sptr<MicrophoneDescriptor>> AudioDeviceLock::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioMicrophoneDescriptor_.GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioDeviceLock::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    std::lock_guard<std::shared_mutex> lock(deviceStatusUpdateSharedMutex_);
    audioDeviceManager_.OnReceiveBluetoothEvent(macAddress, deviceName);
    audioConnectedDevice_.SetDisplayName(macAddress, deviceName);
}

void AudioDeviceLock::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    // Distributed devices status update
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioDeviceLock::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnForcedDeviceSelected(devType, macAddress);
}

int32_t AudioDeviceLock::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioRecoveryDevice_.SelectOutputDevice(audioRendererFilter, selectedDesc);
}

int32_t AudioDeviceLock::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioRecoveryDevice_.SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioDeviceLock::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioRecoveryDevice_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioDeviceLock::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioRecoveryDevice_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioStateManager_.GetExcludedDevices(audioDevUsage);
}

void AudioDeviceLock::UpdateTrackerDeviceChange(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    AUDIO_INFO_LOG("Start");

    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    for (std::shared_ptr<AudioDeviceDescriptor> deviceDesc : desc) {
        if (deviceDesc->deviceRole_ == OUTPUT_DEVICE) {
            DeviceType type = curOutputDeviceType;
            std::string macAddress = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();
            auto itr = audioConnectedDevice_.CheckExistOutputDevice(type, macAddress);
            if (itr != nullptr) {
                AudioDeviceDescriptor outputDevice(AudioDeviceDescriptor::DEVICE_INFO);
                audioDeviceCommon_.UpdateDeviceInfo(outputDevice, itr, true, true);
                streamCollector_.UpdateTracker(AUDIO_MODE_PLAYBACK, outputDevice);
            }
        }

        if (deviceDesc->deviceRole_ == INPUT_DEVICE) {
            DeviceType type = audioActiveDevice_.GetCurrentInputDeviceType();
            auto itr = audioConnectedDevice_.CheckExistInputDevice(type);
            if (itr != nullptr) {
                AudioDeviceDescriptor inputDevice(AudioDeviceDescriptor::DEVICE_INFO);
                audioDeviceCommon_.UpdateDeviceInfo(inputDevice, itr, true, true);
                audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(itr->deviceType_);
                streamCollector_.UpdateTracker(AUDIO_MODE_RECORD, inputDevice);
            }
        }
    }
}

void AudioDeviceLock::NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    AUDIO_INFO_LOG("device<%{public}s> condition:%{public}s value:%{public}s",
        GetEncryptStr(networkId).c_str(), condition.c_str(), value.c_str());

    std::vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    std::vector<shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);
    std::vector<SinkInput> targetSinkInputs = {};
    for (auto &changeInfo : rendererChangeInfos) {
        if (changeInfo->outputDeviceInfo.networkId_ != networkId) {
            continue;
        }
        for (auto &sinkInput : sinkInputs) {
            if (changeInfo->sessionId == sinkInput.streamId) {
                targetSinkInputs.push_back(sinkInput);
            }
        }
    }
    AUDIO_DEBUG_LOG("move [%{public}zu] of all [%{public}zu]sink-inputs to local.",
        targetSinkInputs.size(), sinkInputs.size());
    std::shared_ptr<AudioDeviceDescriptor> localDevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN_LOG(localDevice != nullptr, "Device error: null device.");
    localDevice->networkId_ = LOCAL_NETWORK_ID;
    localDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    localDevice->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;

    int32_t ret;
    AudioDeviceDescriptor curOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    if (localDevice->deviceType_ != curOutputDeviceDesc.deviceType_) {
        AUDIO_WARNING_LOG("device[%{public}d] not active, use device[%{public}d] instead.",
            static_cast<int32_t>(localDevice->deviceType_), static_cast<int32_t>(curOutputDeviceDesc.deviceType_));
        ret = audioDeviceCommon_.MoveToLocalOutputDevice(targetSinkInputs,
            std::make_shared<AudioDeviceDescriptor>(curOutputDeviceDesc));
    } else {
        ret = audioDeviceCommon_.MoveToLocalOutputDevice(targetSinkInputs, localDevice);
    }
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "MoveToLocalOutputDevice failed!");

    // Suspend device, notify audio stream manager that device has been changed.
    ret = audioPolicyManager_.SuspendAudioDevice(networkId, true);
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "SuspendAudioDevice failed!");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {};
    desc.push_back(localDevice);
    UpdateTrackerDeviceChange(desc);
    audioDeviceCommon_.OnPreferredOutputDeviceUpdated(curOutputDeviceDesc);
    AUDIO_DEBUG_LOG("Success");
}

int32_t AudioDeviceLock::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioCapturerSession_.OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioDeviceLock::OnCapturerSessionRemoved(uint64_t sessionID)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioCapturerSession_.OnCapturerSessionRemoved(sessionID);
}

int32_t AudioDeviceLock::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioDeviceStatus_.OnServiceConnected(serviceIndex);
}

// new lock
void AudioDeviceLock::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioDeviceLock::OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnBlockedStatusUpdated(devType, status);
}

void AudioDeviceLock::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnMicrophoneBlockedUpdate(devType, status);
}

void AudioDeviceLock::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("Start for [%{public}d]", serviceIndex);
}

void AudioDeviceLock::SetDisplayName(const std::string &deviceName, bool isLocalDevice)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioConnectedDevice_.SetDisplayName(deviceName, isLocalDevice);
}

void AudioDeviceLock::UpdateSpatializationSupported(const std::string macAddress, const bool support)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioConnectedDevice_.UpdateSpatializationSupported(macAddress, support);
}

void AudioDeviceLock::SetDmDeviceType(const uint16_t dmDeviceType)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioConnectedDevice_.SetDmDeviceType(dmDeviceType);
}

int32_t AudioDeviceLock::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceCommon_.FetchDevice(true, reason);
    audioDeviceCommon_.FetchDevice(false, reason);

    // update a2dp offload
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    return SUCCESS;
}

std::vector<sptr<VolumeGroupInfo>> AudioDeviceLock::GetVolumeGroupInfos()
{
    std::vector<sptr<VolumeGroupInfo>> infos = {};
    for (int32_t i = 0; i < RETRY_TIMES; i++) {
        std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
        if (audioVolumeManager_.GetVolumeGroupInfosNotWait(infos)) {
            return infos;
        } else {
            deviceLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LOAD_DEFAULT_DEVICE_TIME_MS));
        }
    }
    AUDIO_ERR_LOG("timeout");
    return infos;
}

}
}
