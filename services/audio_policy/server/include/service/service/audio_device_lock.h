#ifndef ST_AUDIO_POLICY_DEVICE_LOCK_H
#define ST_AUDIO_POLICY_DEVICE_LOCK_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "microphone_descriptor.h"
#include "audio_system_manager.h"


namespace OHOS {
namespace AudioStandard {

class AudioPolicyDeviceLock {
public:
    static AudioPolicyDeviceLock& GetInstance()
    {
        static AudioPolicyDeviceLock instance;
        return instance;
    }
    int32_t SetAudioScene(AudioScene audioScene);
    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);
    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
    int32_t SetDeviceActive(DeviceType deviceType, bool active);
    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(AudioRendererInfo &rendererInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(AudioCapturerInfo &captureInfo,
        std::string networkId = LOCAL_NETWORK_ID);
    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
    int32_t SetCallDeviceActive(DeviceType deviceType, bool active, std::string address);
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object, const int32_t apiVersion);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void RegisteredTrackerClientDied(pid_t uid);
    int32_t GetCurrentRendererChangeInfos(std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);
    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);
    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);
    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state);
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    void NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value);
    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);
    void OnCapturerSessionRemoved(uint64_t sessionID);


    /*****IDeviceStatusObserver*****/
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);
    void OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status);
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);
    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected);
    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &name, const std::string &adderess);
    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnServiceConnected(AudioServiceIndex serviceIndex);
    void OnServiceDisconnected(AudioServiceIndex serviceIndex);
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress);
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);
    /*****IDeviceStatusObserver*****/
private:
    AudioPolicyDeviceLock()
    {
    }
    ~AudioPolicyDeviceLock() {}
    void UpdateDefaultOutputDeviceWhenStopping(int32_t uid);
private:
    mutable std::shared_mutex deviceStatusUpdateSharedMutex_;
};

}
}

#endif