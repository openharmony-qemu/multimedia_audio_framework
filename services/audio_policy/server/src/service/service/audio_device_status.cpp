#ifndef ST_AUDIO_POLICY_DEVICE_STATUS_H
#define ST_AUDIO_POLICY_DEVICE_STATUS_H

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
#include "audio_utils.h"
#include "audio_errors.h"


namespace OHOS {
namespace AudioStandard {

using InternalDeviceType = DeviceType;

class AudioPolicyDeviceStatus {
public:
    static AudioPolicyDeviceStatus& GetInstance()
    {
        static AudioPolicyDeviceStatus instance;
        return instance;
    }
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status);
    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected);
    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &name, const std::string &adderess);
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);
    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);
    void OnServiceConnected(AudioServiceIndex serviceIndex);
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, DeviceType devType,
        std::string macAddress, std::string deviceName, bool isActualConnection, AudioStreamInfo streamInfo, bool isConnected);    
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);
private:
    AudioPolicyDeviceStatus() {}
    ~AudioPolicyDeviceStatus() {}

    void UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
        const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc);
    int32_t HandleLocalDeviceConnected(AudioDeviceDescriptor &updatedDesc);
    int32_t HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc);
    void UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress);
    int32_t RehandlePnpDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);
    int32_t HandleArmUsbDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);
    int32_t HandleDpDevice(DeviceType deviceType, const std::string &address);
    int32_t HandleSpecialDeviceType(DeviceType &devType, bool &isConnected, const std::string &address);
    void TriggerAvailableDeviceChangedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    void TriggerDeviceChangedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &devChangeDesc, bool connection);
    void TriggerMicrophoneBlockedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &desc,
        DeviceBlockStatus status);
    int32_t HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);
    void OnPreferredDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor, DeviceType activeInputDevice);
    void UpdateDeviceList(AudioDeviceDescriptor &updatedDesc, bool isConnected,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb,
        AudioStreamDeviceChangeReasonExt &reason);
#ifdef BLUETOOTH_ENABLE
    void CheckAndActiveHfpDevice(AudioDeviceDescriptor &desc);
#endif
    void CheckForA2dpSuspend(AudioDeviceDescriptor &desc);
    void UpdateAllUserSelectDevice(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &userSelectDeviceMap,
        AudioDeviceDescriptor &desc, const sptr<AudioDeviceDescriptor> &selectDesc);
    bool IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo);
    void OnPreferredStateUpdated(AudioDeviceDescriptor &desc,
        const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason);
    void AddEarpiece();
    void LoadModernInnerCapSink();
    void ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);
    int32_t LoadDefaultUsbModule(DeviceRole deviceRole);
    int32_t LoadUsbModule(std::string deviceInfo, DeviceRole deviceRole);
    void AddAudioDevice(AudioModuleInfo& moduleInfo, DeviceType devType);
    bool OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo);
    int32_t LoadDefaultModule();
    int32_t GetModuleInfo(ClassType classType, std::string &moduleInfoStr);
    int32_t LoadDpModule(std::string deviceInfo);

    void WriteInDeviceChangedSysEvents(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SourceOutput &sourceOutput);
    void WriteDeviceChangedSysEvents(const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    void WriteOutDeviceChangedSysEvents(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SinkInput &sinkInput);
    int32_t ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote);
private:
    bool remoteCapturerSwitch_ = false;
    bool hasModulesLoaded_ = false;
    bool hasArmUsbDevice_ = false;
    bool hasHifiUsbDevice_ = false; // Only the first usb device is supported now, hifi or arm.
    std::vector<std::pair<DeviceType, bool>> pnpDeviceList_;

    static std::map<std::string, AudioSampleFormat> formatStrToEnum;
};

}
}

#endif