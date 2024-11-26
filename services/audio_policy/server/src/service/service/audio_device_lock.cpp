
#ifndef LOG_TAG
#define LOG_TAG "AudioPolicyDeviceLock"
#endif

#include "audio_policy_device_lock.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "media_monitor_manager.h"
#include "audio_state_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_device_manager.h"
#include "audio_stream_collector.h"
#include "audio_affinity_manager.h"

#include "audio_policy_connected_device.h"
#include "audio_policy_device_common.h"
#include "audio_policy_active_device.h"
#include "audio_policy_device_status.h"
#include "audio_policy_volume.h"
#include "audio_a2dp_offload_manager.h"
#include "audio_policy_audioscene.h"
#include "audio_policy_common.h"
#include "audio_policy_recovery_device.h"
#include "audio_policy_config_manager.h"
#include "audio_policy_microphone.h"
#include "audio_policy_serverproxy.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_policy_capturer_session.h"
#include "audio_policy_offload_stream.h"

namespace OHOS {
namespace AudioStandard {
const int32_t DATA_LINK_CONNECTED = 11;

int32_t AudioPolicyDeviceLock::SetAudioScene(AudioScene audioScene)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    AudioPolicyAudioScene::GetInstance().SetAudioScenePre(audioScene);
    // fetch input&output device
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);

    BluetoothOffloadState state = AudioA2dpOffloadFlag::GetInstance().GetA2dpOffloadFlag();
    int32_t result = AudioPolicyAudioScene::GetInstance().SetAudioSceneAfter(audioScene, state);

    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "SetAudioScene failed [%{public}d]", result);

    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        // Make sure the STREAM_VOICE_CALL volume is set before the calling starts.
        AudioPolicyVolume::GetInstance().SetVoiceCallSystemVolume();
    }

    return SUCCESS;
}

bool AudioPolicyDeviceLock::IsArmUsbDevice(const AudioDeviceDescriptor &desc)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioDeviceManager::GetAudioDeviceManager().IsArmUsbDevice(desc);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyDeviceLock::GetDevices(DeviceFlag deviceFlag)
{
    AUDIO_DEBUG_LOG("GetDevices start");
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyConnectedDevice::GetInstance().GetDevicesInner(deviceFlag);
}

int32_t AudioPolicyDeviceLock::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    int32_t ret = AudioPolicyActiveDevice::GetInstance().SetDeviceActive(deviceType, active);
    if (ret == SUCCESS) {
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    }
    return ret;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyDeviceLock::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyDeviceCommon::GetInstance().GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyDeviceLock::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyDeviceCommon::GetInstance().GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

std::unique_ptr<AudioDeviceDescriptor> AudioPolicyDeviceLock::GetActiveBluetoothDevice()
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);

    std::unique_ptr<AudioDeviceDescriptor> preferredDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    if (preferredDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        return preferredDesc;
    }

    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioPrivacyDeviceDescriptors =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> activeDeviceDescriptors;

    for (auto &desc : audioPrivacyDeviceDescriptors) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->isEnable_) {
            activeDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(*desc));
        }
    }

    uint32_t btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        activeDeviceDescriptors = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderBTCarDevices();
    }
    btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        return make_unique<AudioDeviceDescriptor>();
    } else if (btDeviceSize == 1) {
        std::unique_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[0]);
        return res;
    }

    uint32_t index = 0;
    for (uint32_t i = 1; i < btDeviceSize; ++i) {
        if (activeDeviceDescriptors[i]->connectTimeStamp_ >
            activeDeviceDescriptors[index]->connectTimeStamp_) {
                index = i;
        }
    }
    std::unique_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[index]);
    return res;
}

void AudioPolicyDeviceLock::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnDeviceInfoUpdated(desc, command);
}

int32_t AudioPolicyDeviceLock::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    int32_t ret = AudioPolicyActiveDevice::GetInstance().SetCallDeviceActive(deviceType, active, address);
    if (ret == SUCCESS) {
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    }
    return ret;
}

std::vector<std::unique_ptr<AudioDeviceDescriptor>> AudioPolicyDeviceLock::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyCommon::GetInstance().GetAvailableDevicesInner(usage);
}

void AudioPolicyDeviceLock::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    AUDIO_INFO_LOG("fetch device for track, sessionid:%{public}d start",
        streamChangeInfo.audioRendererChangeInfo.sessionId);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    // Set prerunningState true to refetch devices when device info change before update tracker to running
    streamChangeInfo.audioRendererChangeInfo.prerunningState = true;
    if (AudioStreamCollector::GetAudioStreamCollector().UpdateTrackerInternal(mode, streamChangeInfo) != SUCCESS) {
        return;
    }

    std::vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfo;
    rendererChangeInfo.push_back(
        make_unique<AudioRendererChangeInfo>(streamChangeInfo.audioRendererChangeInfo));
    AudioStreamCollector::GetAudioStreamCollector().GetRendererStreamInfo(streamChangeInfo, *rendererChangeInfo[0]);

    AudioDeviceManager::GetAudioDeviceManager().UpdateDefaultOutputDeviceWhenStarting(streamChangeInfo.audioRendererChangeInfo.sessionId);

    AudioPolicyDeviceCommon::GetInstance().FetchOutputDevice(rendererChangeInfo, reason);
}

void AudioPolicyDeviceLock::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    AUDIO_INFO_LOG("fetch device for track, sessionid:%{public}d start",
        streamChangeInfo.audioRendererChangeInfo.sessionId);

    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> capturerChangeInfo;
    capturerChangeInfo.push_back(
        make_unique<AudioCapturerChangeInfo>(streamChangeInfo.audioCapturerChangeInfo));
    AudioStreamCollector::GetAudioStreamCollector().GetCapturerStreamInfo(streamChangeInfo, *capturerChangeInfo[0]);

    AudioPolicyDeviceCommon::GetInstance().FetchInputDevice(capturerChangeInfo);
}


int32_t AudioPolicyDeviceLock::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    if (mode == AUDIO_MODE_RECORD) {
        AudioPolicyMicrophone::GetInstance().AddAudioCapturerMicrophoneDescriptor(streamChangeInfo.audioCapturerChangeInfo.sessionId, DEVICE_TYPE_NONE);
        if (apiVersion > 0 && apiVersion < API_11) {
            AudioPolicyDeviceCommon::GetInstance().UpdateDeviceInfo(streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo,
                new AudioDeviceDescriptor(AudioPolicyActiveDevice::GetInstance().GetCurrentInputDevice()), false, false);
        }
    } else if (apiVersion > 0 && apiVersion < API_11) {
        AudioPolicyDeviceCommon::GetInstance().UpdateDeviceInfo(streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo,
            new AudioDeviceDescriptor(AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice()), false, false);
    }
    return AudioStreamCollector::GetAudioStreamCollector().RegisterTracker(mode, streamChangeInfo, object);
}

int32_t AudioPolicyDeviceLock::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    if (mode == AUDIO_MODE_RECORD && streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_RELEASED) {
        AudioAffinityManager::GetAudioAffinityManager().DelSelectCapturerDevice(streamChangeInfo.audioCapturerChangeInfo.clientUID);
        AudioPolicyMicrophone::GetInstance().RemoveAudioCapturerMicrophoneDescriptorBySessionID(
            streamChangeInfo.audioCapturerChangeInfo.sessionId);
    }

    int32_t ret = AudioStreamCollector::GetAudioStreamCollector().UpdateTracker(mode, streamChangeInfo);

    const auto &rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    if (rendererState == RENDERER_PREPARED || rendererState == RENDERER_NEW || rendererState == RENDERER_INVALID) {
        return ret; // only update tracker in new and prepared
    }

    if (rendererState == RENDERER_RELEASED && !AudioStreamCollector::GetAudioStreamCollector().ExistStreamForPipe(PIPE_TYPE_MULTICHANNEL)) {
        AudioPolicyOffloadStream::GetInstance().UnloadMchModule();
    }

    if (mode == AUDIO_MODE_PLAYBACK && (rendererState == RENDERER_STOPPED || rendererState == RENDERER_PAUSED ||
        rendererState == RENDERER_RELEASED)) {
        AudioDeviceManager::GetAudioDeviceManager().UpdateDefaultOutputDeviceWhenStopping(streamChangeInfo.audioRendererChangeInfo.sessionId);
        if (rendererState == RENDERER_RELEASED) {
            AudioDeviceManager::GetAudioDeviceManager().RemoveSelectedDefaultOutputDevice(streamChangeInfo.audioRendererChangeInfo.sessionId);
            AudioAffinityManager::GetAudioAffinityManager().DelSelectRendererDevice(streamChangeInfo.audioRendererChangeInfo.clientUID);
        }
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true);
    }

    const int32_t sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    const StreamUsage streamUsage = streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage;
    if (Util::IsRingerOrAlarmerStreamUsage(streamUsage) 
        && (mode == AUDIO_MODE_PLAYBACK)
        && (rendererState == RENDERER_STOPPED || rendererState == RENDERER_RELEASED)) {
        AudioPolicyDeviceCommon::GetInstance().UpdateDualToneStateBySessionID(sessionId);
    }

    AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream(AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->deviceType_);
    if ((rendererState == RENDERER_RUNNING) &&
        !AudioA2dpOffloadManager::GetInstance().IsA2dpOffloadConnecting(sessionId)) {
        AUDIO_INFO_LOG("Notify client not to block.");
        std::thread sendConnectedToClient(&AudioPolicyDeviceLock::UpdateSessionConnectionState, this, sessionId,
            DATA_LINK_CONNECTED);
        sendConnectedToClient.detach();
    }
    return ret;
}

void AudioPolicyDeviceLock::UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state)
{
    AudioPolicyServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionID, state);
}

void AudioPolicyDeviceLock::UpdateDefaultOutputDeviceWhenStopping(int32_t uid)
{
    std::vector<uint32_t> sessionIDSet = AudioStreamCollector::GetAudioStreamCollector().GetAllRendererSessionIDForUID(uid);
    for (const auto &sessionID : sessionIDSet) {
        AudioDeviceManager::GetAudioDeviceManager().UpdateDefaultOutputDeviceWhenStopping(sessionID);
        AudioDeviceManager::GetAudioDeviceManager().RemoveSelectedDefaultOutputDevice(sessionID);
    }
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true);
}

void AudioPolicyDeviceLock::RegisteredTrackerClientDied(pid_t uid)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);

    UpdateDefaultOutputDeviceWhenStopping(static_cast<int32_t>(uid));

    AudioPolicyMicrophone::GetInstance().RemoveAudioCapturerMicrophoneDescriptor(static_cast<int32_t>(uid));
    AudioStreamCollector::GetAudioStreamCollector().RegisteredTrackerClientDied(static_cast<int32_t>(uid));

    if (!AudioStreamCollector::GetAudioStreamCollector().ExistStreamForPipe(PIPE_TYPE_OFFLOAD)) {
        AudioPolicyOffloadStream::GetInstance().DynamicUnloadOffloadModule();
    }

    if (!AudioStreamCollector::GetAudioStreamCollector().ExistStreamForPipe(PIPE_TYPE_MULTICHANNEL)) {
        AudioPolicyOffloadStream::GetInstance().UnloadMchModule();
    }
}

void AudioPolicyDeviceLock::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo)
{
    // Pnp device status update
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo);
}

void AudioPolicyDeviceLock::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
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
#endif
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnDeviceStatusUpdated(updatedDesc, devType,
        macAddress, deviceName, isActualConnection, streamInfo, isConnected);
}

void AudioPolicyDeviceLock::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

static void UpdateRendererInfoWhenNoPermission(const unique_ptr<AudioRendererChangeInfo> &audioRendererChangeInfos,
    bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioRendererChangeInfos->clientUID = 0;
        audioRendererChangeInfos->rendererState = RENDERER_INVALID;
    }
}

int32_t AudioPolicyDeviceLock::GetCurrentRendererChangeInfos(std::vector<std::unique_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);

    int32_t status = AudioStreamCollector::GetAudioStreamCollector().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get renderer change info failed");

    std::vector<sptr<AudioDeviceDescriptor>> outputDevices = AudioPolicyConnectedDevice::GetInstance().GetDevicesInner(OUTPUT_DEVICES_FLAG);
    DeviceType activeDeviceType = AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->deviceType_;
    std::string activeMacAddress = AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->macAddress_;
    DeviceRole activeDeviceRole = OUTPUT_DEVICE;
    for (sptr<AudioDeviceDescriptor> desc : outputDevices) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            if (activeDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc->macAddress_ != activeMacAddress) {
                // This A2DP device is not the active A2DP device. Skip it.
                continue;
            }
            size_t rendererInfosSize = audioRendererChangeInfos.size();
            for (size_t i = 0; i < rendererInfosSize; i++) {
                UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos[i], hasSystemPermission);
                AudioPolicyDeviceCommon::GetInstance().UpdateDeviceInfo(audioRendererChangeInfos[i]->outputDeviceInfo, desc, hasBTPermission,
                    hasSystemPermission);
            }
            break;
        }
    }

    return status;
}

std::vector<sptr<MicrophoneDescriptor>> AudioPolicyDeviceLock::GetAvailableMicrophones()
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyMicrophone::GetInstance().GetAvailableMicrophones();
}

std::vector<sptr<MicrophoneDescriptor>> AudioPolicyDeviceLock::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyMicrophone::GetInstance().GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioPolicyDeviceLock::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    std::lock_guard<std::shared_mutex> lock(deviceStatusUpdateSharedMutex_);
    AudioDeviceManager::GetAudioDeviceManager().OnReceiveBluetoothEvent(macAddress, deviceName);
    AudioPolicyConnectedDevice::GetInstance().SetDisplayName(macAddress, deviceName);
}

void AudioPolicyDeviceLock::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    // Distributed devices status update
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioPolicyDeviceLock::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnForcedDeviceSelected(devType, macAddress);
}

int32_t AudioPolicyDeviceLock::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<sptr<AudioDeviceDescriptor>> selectedDesc)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyRecoveryDevice::GetInstance().SelectOutputDevice(audioRendererFilter, selectedDesc);
}

int32_t AudioPolicyDeviceLock::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<sptr<AudioDeviceDescriptor>> selectedDesc)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyRecoveryDevice::GetInstance().SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioPolicyDeviceLock::NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceCommon::GetInstance().NotifyRemoteRenderState(networkId, condition, value);
}

int32_t AudioPolicyDeviceLock::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return AudioPolicyCapturerSession::GetInstance().OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioPolicyDeviceLock::OnCapturerSessionRemoved(uint64_t sessionID)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyCapturerSession::GetInstance().OnCapturerSessionRemoved(sessionID);
}

void AudioPolicyDeviceLock::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnServiceConnected(serviceIndex);
}

// new lock
void AudioPolicyDeviceLock::OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnPnpDeviceStatusUpdated(devType, isConnected);
}

void AudioPolicyDeviceLock::OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnBlockedStatusUpdated(devType, status);
}

void AudioPolicyDeviceLock::OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected,
    const std::string &name, const std::string &adderess)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnPnpDeviceStatusUpdated(devType, isConnected, name, adderess);
}

void AudioPolicyDeviceLock::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    AudioPolicyDeviceStatus::GetInstance().OnMicrophoneBlockedUpdate(devType, status);
}

void AudioPolicyDeviceLock::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("Start for [%{public}d]", serviceIndex);
}




}
}