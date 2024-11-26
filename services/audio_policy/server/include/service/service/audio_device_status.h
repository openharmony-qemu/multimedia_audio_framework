
#ifndef LOG_TAG
#define LOG_TAG "AudioPolicyDeviceStatus"
#endif

#include "audio_policy_device_status.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"

#include "audio_policy_a2dp_device_manager.h"
#include "audio_policy_router_map_manager.h"
#include "audio_stream_collector.h"
#include "audio_router_center.h"
#include "audio_device_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_effect_manager.h"

#include "audio_policy_ec.h"
#include "audio_policy_io_handle_manager.h"
#include "audio_policy_connected_device.h"
#include "audio_policy_device_common.h"
#include "audio_policy_common.h"
#include "audio_policy_volume.h"
#include "audio_policy_active_device.h"
#include "audio_policy_config_manager.h"
#include "audio_policy_device_lock.h"
#include "audio_a2dp_offload_manager.h"
#include "audio_policy_offload_stream.h"
#include "audio_policy_serverproxy.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_policy_microphone.h"

namespace OHOS {
namespace AudioStandard {
const float RENDER_FRAME_INTERVAL_IN_SECONDS = 0.02;
const int MEDIA_RENDER_ID = 0;
const int CALL_RENDER_ID = 1;
const int CALL_CAPTURE_ID = 2;
const int RECORD_CAPTURE_ID = 3;
const uint32_t REHANDLE_DEVICE_RETRY_INTERVAL_IN_MICROSECONDS = 30000;

const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;
const uint32_t PCM_8_BIT = 8;

std::map<std::string, AudioSampleFormat> AudioPolicyDeviceStatus::formatStrToEnum = {
    {"s8", SAMPLE_U8},
    {"s16", SAMPLE_S16LE},
    {"s24", SAMPLE_S24LE},
    {"s32", SAMPLE_S32LE},
};

void AudioPolicyDeviceStatus::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    // fill device change action for callback
    std::vector<sptr<AudioDeviceDescriptor>> descForCb = {};

    int32_t result = ERROR;
    result = HandleSpecialDeviceType(devType, isConnected, macAddress);
    CHECK_AND_RETURN_LOG(result == SUCCESS, "handle special deviceType failed.");

    AUDIO_INFO_LOG("Device connection state updated | TYPE[%{public}d] STATUS[%{public}d], address[%{public}s]",
        devType, isConnected, GetEncryptStr(macAddress).c_str());

    AudioDeviceDescriptor updatedDesc(devType, AudioPolicyCommon::GetInstance().GetDeviceRole(devType));
    UpdateLocalGroupInfo(isConnected, macAddress, deviceName, streamInfo, updatedDesc);

    if (isConnected) {
        // If device already in list, remove it else do not modify the list
        AudioPolicyConnectedDevice::GetInstance().DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_, updatedDesc.macAddress_);
        // If the pnp device fails to load, it will not connect
        result = HandleLocalDeviceConnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Connect local device failed.");
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);

        reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    } else {
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, reason); // fix pop, fetch device before unload module
        result = HandleLocalDeviceDisconnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Disconnect local device failed.");
    }

    TriggerDeviceChangedCallback(descForCb, isConnected);
    TriggerAvailableDeviceChangedCallback(descForCb, isConnected);

    // fetch input&output device
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, reason);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);

    // update a2dp offload
    AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream();
}

void AudioPolicyDeviceStatus::WriteInDeviceChangedSysEvents(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
    const SourceOutput &sourceOutput)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::DEVICE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ISOUTPUT", 0);
    bean->Add("STREAMID", sourceOutput.streamId);
    bean->Add("STREAMTYPE", sourceOutput.streamType);
    bean->Add("DEVICETYPE", deviceDescriptor->deviceType_);
    bean->Add("NETWORKID", ConvertNetworkId(deviceDescriptor->networkId_));
    bean->Add("ADDRESS", GetEncryptAddr(deviceDescriptor->macAddress_));
    bean->Add("DEVICE_NAME", deviceDescriptor->deviceName_);
    bean->Add("BT_TYPE", deviceDescriptor->deviceCategory_);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioPolicyDeviceStatus::WriteOutDeviceChangedSysEvents(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
    const SinkInput &sinkInput)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::DEVICE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ISOUTPUT", 1);
    bean->Add("STREAMID", sinkInput.streamId);
    bean->Add("STREAMTYPE", sinkInput.streamType);
    bean->Add("DEVICETYPE", deviceDescriptor->deviceType_);
    bean->Add("NETWORKID", ConvertNetworkId(deviceDescriptor->networkId_));
    bean->Add("ADDRESS", GetEncryptAddr(deviceDescriptor->macAddress_));
    bean->Add("DEVICE_NAME", deviceDescriptor->deviceName_);
    bean->Add("BT_TYPE", deviceDescriptor->deviceCategory_);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioPolicyDeviceStatus::WriteDeviceChangedSysEvents(const vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    Trace trace("AudioPolicyCommon::WriteDeviceChangedSysEvents");
    for (auto deviceDescriptor : desc) {
        if (deviceDescriptor != nullptr) {
            if ((deviceDescriptor->deviceType_ == DEVICE_TYPE_WIRED_HEADSET)
                || (deviceDescriptor->deviceType_ == DEVICE_TYPE_USB_HEADSET)
                || (deviceDescriptor->deviceType_ == DEVICE_TYPE_WIRED_HEADPHONES)) {
                std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
                    Media::MediaMonitor::AUDIO, Media::MediaMonitor::HEADSET_CHANGE,
                    Media::MediaMonitor::BEHAVIOR_EVENT);
                bean->Add("HASMIC", 1);
                bean->Add("ISCONNECT", isConnected ? 1 : 0);
                bean->Add("DEVICETYPE", deviceDescriptor->deviceType_);
                Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
            }

            if (!isConnected) {
                continue;
            }

            if (deviceDescriptor->deviceRole_ == OUTPUT_DEVICE) {
                std::vector<SinkInput> sinkInputs = AudioPolicyManagerFactory::GetAudioPolicyManager().GetAllSinkInputs();
                for (SinkInput sinkInput : sinkInputs) {
                    WriteOutDeviceChangedSysEvents(deviceDescriptor, sinkInput);
                }
            } else if (deviceDescriptor->deviceRole_ == INPUT_DEVICE) {
                std::vector<SourceOutput> sourceOutputs;
                {
                    std::unordered_map<std::string, AudioIOHandle> mapCopy = AudioPolicyIOHandleManager::GetInstance().GetCopy();
                    if (std::any_of(mapCopy.cbegin(), mapCopy.cend(), [](const auto &pair) {
                            return std::find(SourceNames.cbegin(), SourceNames.cend(), pair.first) != SourceNames.end();
                        })) {
                        sourceOutputs = AudioPolicyManagerFactory::GetAudioPolicyManager().GetAllSourceOutputs();
                    }
                }
                for (SourceOutput sourceOutput : sourceOutputs) {
                    WriteInDeviceChangedSysEvents(deviceDescriptor, sourceOutput);
                }
            }
        }
    }
}

void AudioPolicyDeviceStatus::TriggerAvailableDeviceChangedCallback(
    const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    Trace trace("AudioPolicyDeviceStatus::TriggerAvailableDeviceChangedCallback");

    WriteDeviceChangedSysEvents(desc, isConnected);
    DelayedSingleton<AudioPolicyServerHandler>::GetInstance()->SendAvailableDeviceChange(desc, isConnected);
}

void AudioPolicyDeviceStatus::TriggerDeviceChangedCallback(const vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    Trace trace("AudioPolicyDeviceStatus::TriggerDeviceChangedCallback");
    WriteDeviceChangedSysEvents(desc, isConnected);
    DelayedSingleton<AudioPolicyServerHandler>::GetInstance()->SendDeviceChangedCallback(desc, isConnected);
}

void AudioPolicyDeviceStatus::UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc)
{
    deviceDesc.SetDeviceInfo(deviceName, macAddress);
    deviceDesc.SetDeviceCapability(streamInfo, 0);
    AudioPolicyVolume::GetInstance().UpdateGroupInfo(VOLUME_TYPE, GROUP_NAME_DEFAULT, deviceDesc.volumeGroupId_, LOCAL_NETWORK_ID, isConnected,
        NO_REMOTE_ID);
    AudioPolicyVolume::GetInstance().UpdateGroupInfo(INTERRUPT_TYPE, GROUP_NAME_DEFAULT, deviceDesc.interruptGroupId_, LOCAL_NETWORK_ID, isConnected,
        NO_REMOTE_ID);
    deviceDesc.networkId_ = LOCAL_NETWORK_ID;
}

int32_t AudioPolicyDeviceStatus::HandleLocalDeviceConnected(AudioDeviceDescriptor &updatedDesc)
{
    AUDIO_INFO_LOG("macAddress:[%{public}s]", AudioPolicyCommon::GetInstance().GetEncryptAddr(updatedDesc.macAddress_).c_str());
    if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        A2dpDeviceConfigInfo configInfo = {updatedDesc.audioStreamInfo_, false};
        AudioPolicyA2dpDeviceManager::GetInstance().AddA2dpDevice(updatedDesc.macAddress_, configInfo);
    }

    if (updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        int32_t loadOutputResult = HandleArmUsbDevice(updatedDesc.deviceType_, OUTPUT_DEVICE, updatedDesc.macAddress_);
        if (loadOutputResult != SUCCESS) {
            loadOutputResult = RehandlePnpDevice(updatedDesc.deviceType_, OUTPUT_DEVICE, updatedDesc.macAddress_);
        }
        int32_t loadInputResult = HandleArmUsbDevice(updatedDesc.deviceType_, INPUT_DEVICE, updatedDesc.macAddress_);
        if (loadInputResult != SUCCESS) {
            loadInputResult = RehandlePnpDevice(updatedDesc.deviceType_, INPUT_DEVICE, updatedDesc.macAddress_);
        }
        if (loadOutputResult != SUCCESS && loadInputResult != SUCCESS) {
            hasArmUsbDevice_ = false;
            updatedDesc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
            AUDIO_ERR_LOG("Load usb failed, set arm usb flag to false");
            return ERROR;
        }
        // Distinguish between USB input and output (need fix)
        if (loadOutputResult == SUCCESS && loadInputResult == SUCCESS) {
            updatedDesc.deviceRole_ = DEVICE_ROLE_MAX;
        } else {
            updatedDesc.deviceRole_ = (loadOutputResult == SUCCESS) ? OUTPUT_DEVICE : INPUT_DEVICE;
        }
        AUDIO_INFO_LOG("Load usb role is %{public}d", updatedDesc.deviceRole_);
        return SUCCESS;
    }

    // DP device only for output.
    if (updatedDesc.deviceType_ == DEVICE_TYPE_DP) {
        CHECK_AND_RETURN_RET_LOG(!AudioPolicyDeviceCommon::GetInstance().GetHasDpflag(), ERROR, "DP device already exists, ignore this one.");
        int32_t result = HandleDpDevice(updatedDesc.deviceType_, updatedDesc.macAddress_);
        if (result != SUCCESS) {
            result = RehandlePnpDevice(updatedDesc.deviceType_, OUTPUT_DEVICE, updatedDesc.macAddress_);
        }
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Load dp failed.");
        AudioPolicyDeviceCommon::GetInstance().SetHasDpflag(true);
    }

    return SUCCESS;
}

void AudioPolicyDeviceStatus::UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress)
{
    if (AudioPolicyA2dpDeviceManager::GetInstance().DelA2dpDevice(macAddress) == 0) {
        AudioPolicyActiveDevice::GetInstance().SetActiveBtDeviceMac("");
        AudioPolicyIOHandleManager::GetInstance().ClosePortAndEraseIOHandle(BLUETOOTH_SPEAKER);
        AudioPolicyManagerFactory::GetAudioPolicyManager().SetAbsVolumeScene(false);
        AudioPolicyVolume::GetInstance().SetSharedAbsVolumeScene(false);
#ifdef BLUETOOTH_ENABLE
        Bluetooth::AudioA2dpManager::SetActiveA2dpDevice("");
#endif
        return;
    }
}

int32_t AudioPolicyDeviceStatus::HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc)
{
    if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        UpdateActiveA2dpDeviceWhenDisconnecting(updatedDesc.macAddress_);
    }

    if (updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        AudioPolicyIOHandleManager::GetInstance().ClosePortAndEraseIOHandle(USB_SPEAKER);
        AudioPolicyIOHandleManager::GetInstance().ClosePortAndEraseIOHandle(USB_MIC);
    }
    if (updatedDesc.deviceType_ == DEVICE_TYPE_DP) {
        AudioPolicyIOHandleManager::GetInstance().ClosePortAndEraseIOHandle(DP_SINK);
    }

    AudioPolicyServerProxy::GetInstance().ResetRouteForDisconnectProxy(updatedDesc.deviceType_);
    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::LoadUsbModule(std::string deviceInfo, DeviceRole deviceRole)
{
    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = AudioPolicyConfigManager::GetInstance().GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret == true, ERR_OPERATION_FAILED,
            "usb module is not exist in the configuration file");
    }
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("[module_load]::load module[%{public}s], load role[%{public}d] config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        AudioPolicyCommon::GetInstance().GetUsbModuleInfo(deviceInfo, moduleInfo);
        uint32_t bufferSize = (std::stoi(moduleInfo.rate) *
                AudioPolicyCommon::GetInstance().GetSampleFormatValue(formatStrToEnum[moduleInfo.format]) *
                std::stoi(moduleInfo.channels)) / PCM_8_BIT * RENDER_FRAME_INTERVAL_IN_SECONDS;
        int32_t ret = AudioPolicyEc::GetInstance().HandleUsbModule(moduleInfo, deviceRole, bufferSize);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
                "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), ret);
    }

    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::LoadDefaultUsbModule(DeviceRole deviceRole)
{
    AUDIO_INFO_LOG("LoadDefaultUsbModule");

    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = AudioPolicyConfigManager::GetInstance().GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret == true, ERR_OPERATION_FAILED,
            "A2dp module is not exist in the configuration file");
    }
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("[module_load]::load default module[%{public}s], load role[%{public}d] config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        int32_t ret = AudioPolicyEc::GetInstance().HandleUsbModule(moduleInfo, deviceRole);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
            "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), ret);
    }

    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::HandleArmUsbDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address)
{
    Trace trace("AudioPolicyService::HandleArmUsbDevice");

    if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
        std::string deviceInfo = AudioPolicyServerProxy::GetInstance().GetAudioParameterProxy(LOCAL_NETWORK_ID, USB_DEVICE, address);
        AUDIO_INFO_LOG("device info from usb hal is %{public}s", deviceInfo.c_str());

        int32_t ret;
        if (!deviceInfo.empty()) {
            ret = LoadUsbModule(deviceInfo, deviceRole);
        } else {
            ret = LoadDefaultUsbModule(deviceRole);
        }
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "load usb role[%{public}d] module failed", deviceRole);

        std::string activePort = AudioPolicyCommon::GetInstance().GetSinkPortName(DEVICE_TYPE_USB_ARM_HEADSET);
        AUDIO_DEBUG_LOG("port %{public}s, active arm usb device", activePort.c_str());
    } else if (AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice().deviceType_ == DEVICE_TYPE_USB_HEADSET) {
        std::string activePort = AudioPolicyCommon::GetInstance().GetSinkPortName(DEVICE_TYPE_USB_ARM_HEADSET);
        AudioPolicyManagerFactory::GetAudioPolicyManager().SuspendAudioDevice(activePort, true);
    }

    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::RehandlePnpDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address)
{
    Trace trace("AudioPolicyService::RehandlePnpDevice");

    // Maximum number of attempts, preventing situations where hal has not yet finished coming online.
    int32_t maxRetries = 3;
    int32_t retryCount = 0;
    int32_t ret = ERROR;
    bool isConnected = true;
    while (retryCount < maxRetries) {
        retryCount++;
        AUDIO_INFO_LOG("rehandle device[%{public}d], retry count[%{public}d]", deviceType, retryCount);

        ret = HandleSpecialDeviceType(deviceType, isConnected, address);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Rehandle special device type failed");
        CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_USB_HEADSET, ret, "Hifi device, don't load module");
        if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            if (HandleArmUsbDevice(deviceType, deviceRole, address) == SUCCESS) {
                return SUCCESS;
            }
        } else if (deviceType == DEVICE_TYPE_DP) {
            if (HandleDpDevice(deviceType, address)  == SUCCESS) {
                return SUCCESS;
            }
        }
        usleep(REHANDLE_DEVICE_RETRY_INTERVAL_IN_MICROSECONDS);
    }

    AUDIO_ERR_LOG("rehandle device[%{public}d] failed", deviceType);
    return ERROR;
}

int32_t AudioPolicyDeviceStatus::GetModuleInfo(ClassType classType, std::string &moduleInfoStr)
{
    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = AudioPolicyConfigManager::GetInstance().GetModuleListByType(classType, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret == true, ERR_OPERATION_FAILED,
            "find %{public}d type failed", classType);
    }
    moduleInfoStr = AudioPolicyManagerFactory::GetAudioPolicyManager().GetModuleArgs(*moduleInfoList.begin());
    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::LoadDpModule(std::string deviceInfo)
{
    AUDIO_INFO_LOG("LoadDpModule");
    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = AudioPolicyConfigManager::GetInstance().GetModuleListByType(ClassType::TYPE_DP, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret == true, ERR_OPERATION_FAILED,
            "dp module is not exist in the configuration file");
    }
    for (auto &moduleInfo : moduleInfoList) {
        AUDIO_INFO_LOG("[module_load]::load module[%{public}s]", moduleInfo.name.c_str());
        if (AudioPolicyIOHandleManager::GetInstance().CheckIOHandleExist(moduleInfo.name) == false) {
            AudioPolicyCommon::GetInstance().GetDPModuleInfo(moduleInfo, deviceInfo);
            if (moduleInfo.role == ROLE_SINK) {
                AUDIO_INFO_LOG("save dp sink module info for cust param");
                AudioPolicyEc::GetInstance().SetDpSinkModuleInfo(moduleInfo);
            }
            return AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::HandleDpDevice(DeviceType deviceType, const std::string &address)
{
    Trace trace("AudioPolicyService::HandleDpDevice");
    if (deviceType == DEVICE_TYPE_DP) {
        std::string defaulyDPInfo = "";
        std::string getDPInfo = "";
        GetModuleInfo(ClassType::TYPE_DP, defaulyDPInfo);
        CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

        getDPInfo = AudioPolicyServerProxy::GetInstance().GetAudioParameterProxy(LOCAL_NETWORK_ID, GET_DP_DEVICE_INFO,
            defaulyDPInfo + " address=" + address + " ");
        AUDIO_DEBUG_LOG("device info from dp hal is \n defaulyDPInfo:%{public}s \n getDPInfo:%{public}s",
            defaulyDPInfo.c_str(), getDPInfo.c_str());

        getDPInfo = getDPInfo.empty() ? defaulyDPInfo : getDPInfo;
        int32_t ret = LoadDpModule(getDPInfo);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG ("load dp module failed");
            return ERR_OPERATION_FAILED;
        }
        std::string activePort = AudioPolicyCommon::GetInstance().GetSinkPortName(DEVICE_TYPE_DP);
        AUDIO_INFO_LOG("port %{public}s, active dp device", activePort.c_str());
    } else if (AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice().deviceType_ == DEVICE_TYPE_DP) {
        std::string activePort = AudioPolicyCommon::GetInstance().GetSinkPortName(DEVICE_TYPE_DP);
        AudioPolicyManagerFactory::GetAudioPolicyManager().SuspendAudioDevice(activePort, true);
    }

    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::HandleSpecialDeviceType(DeviceType &devType, bool &isConnected, const std::string &address)
{
    // usb device needs to be distinguished form arm or hifi
    if (devType == DEVICE_TYPE_USB_HEADSET || devType == DEVICE_TYPE_USB_ARM_HEADSET) {
        AUDIO_INFO_LOG("has hifi:%{public}d, has arm:%{public}d", hasHifiUsbDevice_, hasArmUsbDevice_);
        // Hal only support one HiFi device, If HiFi is already online, the following devices should be ARM.
        // But when the second usb device went online, the return value of this interface was not accurate.
        // So special handling was done when usb device was connected and disconnected.
        const std::string value = AudioPolicyServerProxy::GetInstance().GetAudioParameterProxy("need_change_usb_device");

        AUDIO_INFO_LOG("get value %{public}s from hal when usb device connect", value.c_str());
        if (isConnected) {
            bool isArmConnect = (value == "false" || hasHifiUsbDevice_);
            if (isArmConnect) {
                hasArmUsbDevice_ = true;
                devType = DEVICE_TYPE_USB_ARM_HEADSET;
                CHECK_AND_RETURN_RET_LOG(!hasHifiUsbDevice_, ERROR, "Hifi device already exists, ignore this one.");
            } else {
                hasHifiUsbDevice_ = true;
                CHECK_AND_RETURN_RET_LOG(!hasArmUsbDevice_, ERROR, "Arm device already exists, ignore this one.");
            }
        } else {
            bool isArmDisconnect = ((hasArmUsbDevice_ && !hasHifiUsbDevice_) ||
                                    (hasArmUsbDevice_ && hasHifiUsbDevice_ && value == "true"));
            if (isArmDisconnect) {
                devType = DEVICE_TYPE_USB_ARM_HEADSET;
                hasArmUsbDevice_ = false;
            } else {
                hasHifiUsbDevice_ = false;
            }
        }
    }

    // Special logic for extern cable, need refactor
    if (devType == DEVICE_TYPE_EXTERN_CABLE) {
        CHECK_AND_RETURN_RET_LOG(isConnected, ERROR, "Extern cable disconnected, do nothing");
        DeviceType connectedHeadsetType = AudioPolicyConnectedDevice::GetInstance().FindConnectedHeadset();
        if (connectedHeadsetType == DEVICE_TYPE_NONE) {
            AUDIO_INFO_LOG("Extern cable connect without headset connected before, do nothing");
            return ERROR;
        }
        devType = connectedHeadsetType;
        isConnected = false;
    }

    return SUCCESS;
}

void AudioPolicyDeviceStatus::OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    if (!hasModulesLoaded_) {
        AUDIO_WARNING_LOG("modules has not loaded");
        pnpDeviceList_.push_back({devType, isConnected});
        return;
    }
    AudioStreamInfo streamInfo = {};
    OnDeviceStatusUpdated(devType, isConnected, "", "", streamInfo);
}

void AudioPolicyDeviceStatus::OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected,
    const std::string &name, const std::string &adderess)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    if (!hasModulesLoaded_) {
        AUDIO_WARNING_LOG("modules has not loaded");
        pnpDeviceList_.push_back({devType, isConnected});
        return;
    }
    AudioStreamInfo streamInfo = {};
    OnDeviceStatusUpdated(devType, isConnected, adderess, name, streamInfo);
}

void AudioPolicyDeviceStatus::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    OnBlockedStatusUpdated(devType, status);
}

void AudioPolicyDeviceStatus::OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status)
{
    std::vector<sptr<AudioDeviceDescriptor>> descForCb = {};
    sptr<AudioDeviceDescriptor> audioDescriptor = new AudioDeviceDescriptor(devType, AudioPolicyCommon::GetInstance().GetDeviceRole(devType));
    descForCb.push_back(audioDescriptor);

    vector<unique_ptr<AudioCapturerChangeInfo>> audioChangeInfos;
    AudioStreamCollector::GetAudioStreamCollector().GetCurrentCapturerChangeInfos(audioChangeInfos);
    for (auto it = audioChangeInfos.begin(); it != audioChangeInfos.end(); it++) {
        if ((*it)->capturerState == CAPTURER_RUNNING) {
            AUDIO_INFO_LOG("record running");
            TriggerMicrophoneBlockedCallback(descForCb, status);
        }
    }
}

void AudioPolicyDeviceStatus::TriggerMicrophoneBlockedCallback(const vector<sptr<AudioDeviceDescriptor>> &desc,
    DeviceBlockStatus status)
{
    Trace trace("AudioPolicyDeviceStatus::TriggerMicrophoneBlockedCallback");
    DelayedSingleton<AudioPolicyServerHandler>::GetInstance()->SendMicrophoneBlockedCallback(desc, status);
}

void AudioPolicyDeviceStatus::ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    uint32_t bufferSize = (streamInfo.samplingRate * AudioPolicyCommon::GetInstance().GetSampleFormatValue(streamInfo.format)
        * streamInfo.channels) / (PCM_8_BIT * BT_BUFFER_ADJUSTMENT_FACTOR);
    AUDIO_DEBUG_LOG("Updated buffer size: %{public}d", bufferSize);

    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = AudioPolicyConfigManager::GetInstance().GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    if (ret) {
        for (auto &moduleInfo : moduleInfoList) {
            if (AudioPolicyIOHandleManager::GetInstance().CheckIOHandleExist(moduleInfo.name)) {
                moduleInfo.channels = to_string(streamInfo.channels);
                moduleInfo.rate = to_string(streamInfo.samplingRate);
                moduleInfo.format = AudioPolicyCommon::GetInstance().ConvertToHDIAudioFormat(streamInfo.format);
                moduleInfo.bufferSize = to_string(bufferSize);
                moduleInfo.renderInIdleState = "1";
                moduleInfo.sinkLatency = "0";

                // First unload the existing bt sink
                AUDIO_DEBUG_LOG("UnLoad existing a2dp module");
                std::string currentActivePort = AudioPolicyCommon::GetInstance().GetSinkPortName(AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->deviceType_);
                AudioIOHandle activateDeviceIOHandle;
                AudioPolicyIOHandleManager::GetInstance().GetModuleIdByKey(BLUETOOTH_SPEAKER, activateDeviceIOHandle);
                AudioPolicyManagerFactory::GetAudioPolicyManager().SuspendAudioDevice(currentActivePort, true);
                AudioPolicyManagerFactory::GetAudioPolicyManager().CloseAudioPort(activateDeviceIOHandle);

                // Load bt sink module again with new configuration
                AUDIO_DEBUG_LOG("Reload a2dp module [%{public}s]", moduleInfo.name.c_str());
                int32_t result = AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
                CHECK_AND_RETURN_LOG(result == SUCCESS, "OpenPortAndInsertIOHandle failed");

                std::string portName = AudioPolicyCommon::GetInstance().GetSinkPortName(deviceType);
                AudioPolicyManagerFactory::GetAudioPolicyManager().SetDeviceActive(deviceType, portName, true);
                AudioPolicyManagerFactory::GetAudioPolicyManager().SuspendAudioDevice(portName, false);
                AudioPolicyConnectedDevice::GetInstance().UpdateConnectDevice(deviceType, macAddress, deviceName, streamInfo);
                break;
            }
        }
    }
}

void AudioPolicyDeviceStatus::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    std::string activeBTDevice = AudioPolicyActiveDevice::GetInstance().GetActiveBtDeviceMac();
    AUDIO_INFO_LOG("OnDeviceConfigurationChanged start, deviceType: %{public}d, currentActiveDevice_: %{public}d, "
        "macAddress:[%{public}s], activeBTDevice:[%{public}s]", deviceType,
        AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice().deviceType_,
        AudioPolicyCommon::GetInstance().GetEncryptAddr(macAddress).c_str(),
        AudioPolicyCommon::GetInstance().GetEncryptAddr(activeBTDevice).c_str());
    // only for the active a2dp device.
    if ((deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) && !macAddress.compare(activeBTDevice)
        && AudioPolicyActiveDevice::GetInstance().IsDeviceActive(deviceType)) {
        auto activeSessionsSize = AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream();
        BluetoothOffloadState a2dpOffloadFlag = AudioA2dpOffloadFlag::GetInstance().GetA2dpOffloadFlag();
        AUDIO_DEBUG_LOG("streamInfo.sampleRate: %{public}d, a2dpOffloadFlag: %{public}d",
            streamInfo.samplingRate, a2dpOffloadFlag);
        if (!IsConfigurationUpdated(deviceType, streamInfo) ||
            (activeSessionsSize > 0 && a2dpOffloadFlag == A2DP_OFFLOAD)) {
            AUDIO_DEBUG_LOG("Audio configuration same");
            return;
        }
        AudioPolicyA2dpDeviceManager::GetInstance().SetA2dpDeviceStreamInfo(macAddress, streamInfo);
        ReloadA2dpOffloadOnDeviceChanged(deviceType, macAddress, deviceName, streamInfo);
    } else if (AudioPolicyA2dpDeviceManager::GetInstance().CheckA2dpDeviceExist(macAddress)) {
        AUDIO_DEBUG_LOG("Audio configuration update, macAddress:[%{public}s], streamInfo.sampleRate: %{public}d",
            AudioPolicyCommon::GetInstance().GetEncryptAddr(macAddress).c_str(), streamInfo.samplingRate);
        AudioPolicyA2dpDeviceManager::GetInstance().SetA2dpDeviceStreamInfo(macAddress, streamInfo);
    }
}

bool AudioPolicyDeviceStatus::IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AudioStreamInfo audioStreamInfo = {};
        if (AudioPolicyActiveDevice::GetInstance().GetActiveA2dpDeviceStreamInfo(deviceType, audioStreamInfo)) {
            AUDIO_DEBUG_LOG("Device configurations current rate: %{public}d, format: %{public}d, channel: %{public}d",
                audioStreamInfo.samplingRate, audioStreamInfo.format, audioStreamInfo.channels);
            AUDIO_DEBUG_LOG("Device configurations updated rate: %{public}d, format: %{public}d, channel: %{public}d",
                streamInfo.samplingRate, streamInfo.format, streamInfo.channels);
            if ((audioStreamInfo.samplingRate != streamInfo.samplingRate)
                || (audioStreamInfo.channels != streamInfo.channels)
                || (audioStreamInfo.format != streamInfo.format)) {
                return true;
            }
        }
    }

    return false;
}

void AudioPolicyDeviceStatus::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    // Distributed devices status update
    AUDIO_INFO_LOG("Device connection updated | HDI_PIN[%{public}d] CONNECT_STATUS[%{public}d] NETWORKID[%{public}s]",
        statusInfo.hdiPin, statusInfo.isConnected, GetEncryptStr(statusInfo.networkId).c_str());
    if (isStop) {
        std::vector<sptr<AudioDeviceDescriptor>> deviceChangeDescriptor = {};
        std::vector<sptr<AudioDeviceDescriptor>> connectedDevices = AudioPolicyConnectedDevice::GetInstance().GetCopy();
        for (auto deviceDesc : connectedDevices) {
            if (deviceDesc != nullptr && deviceDesc->networkId_ != LOCAL_NETWORK_ID) {
                const std::string networkId = deviceDesc->networkId_;
                AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenDisconnecting(deviceDesc, deviceChangeDescriptor);
                std::string moduleName = AudioPolicyCommon::GetInstance().GetRemoteModuleName(networkId, AudioPolicyCommon::GetInstance().GetDeviceRole(deviceDesc->deviceType_));
                std::string sinkName = AudioPolicyCommon::GetInstance().GetSinkPortName(AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->deviceType_);
                AudioPolicyIOHandleManager::GetInstance().CloseRemoteDeviceIOHandle(moduleName, sinkName);
                AudioPolicyRouteMapManager::GetInstance().RemoveDeviceInRouterMap(moduleName);
                AudioPolicyRouteMapManager::GetInstance().RemoveDeviceInFastRouterMap(networkId);
                if (AudioPolicyCommon::GetInstance().GetDeviceRole(deviceDesc->deviceType_) == DeviceRole::INPUT_DEVICE) {
                    remoteCapturerSwitch_ = true;
                }
            }
        }

        TriggerDeviceChangedCallback(deviceChangeDescriptor, false);
        TriggerAvailableDeviceChangedCallback(deviceChangeDescriptor, false);

        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);
        return;
    }
    std::vector<sptr<AudioDeviceDescriptor>> descForCb = {};
    int32_t ret = HandleDistributedDeviceUpdate(statusInfo, descForCb);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "HandleDistributedDeviceUpdate return directly.");

    TriggerDeviceChangedCallback(descForCb, statusInfo.isConnected);
    TriggerAvailableDeviceChangedCallback(descForCb, statusInfo.isConnected);

    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);

    DeviceType devType = AudioPolicyCommon::GetInstance().GetDeviceTypeFromPin(statusInfo.hdiPin);
    if (AudioPolicyCommon::GetInstance().GetDeviceRole(devType) == DeviceRole::INPUT_DEVICE) {
        remoteCapturerSwitch_ = true;
    }

    // update a2dp offload
    AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream();
}

int32_t AudioPolicyDeviceStatus::ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote)
{
    if (isRemote) {
        AudioModuleInfo moduleInfo = AudioPolicyCommon::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
            AudioPolicyCommon::GetInstance().GetDeviceRole(deviceType), deviceType);
        std::string moduleName = AudioPolicyCommon::GetInstance().GetRemoteModuleName(networkId,
            AudioPolicyCommon::GetInstance().GetDeviceRole(deviceType));
        AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleName, moduleInfo);
    }
    return SUCCESS;
}

int32_t AudioPolicyDeviceStatus::HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
    std::vector<sptr<AudioDeviceDescriptor>> &descForCb)
{
    DeviceType devType = AudioPolicyCommon::GetInstance().GetDeviceTypeFromPin(statusInfo.hdiPin);
    const std::string networkId = statusInfo.networkId;
    AudioDeviceDescriptor deviceDesc(devType, AudioPolicyCommon::GetInstance().GetDeviceRole(devType));
    deviceDesc.SetDeviceInfo(statusInfo.deviceName, statusInfo.macAddress);
    deviceDesc.SetDeviceCapability(statusInfo.streamInfo, 0);
    deviceDesc.networkId_ = networkId;
    AudioPolicyVolume::GetInstance().UpdateGroupInfo(VOLUME_TYPE, GROUP_NAME_DEFAULT, deviceDesc.volumeGroupId_, networkId, statusInfo.isConnected,
        statusInfo.mappingVolumeId);
    AudioPolicyVolume::GetInstance().UpdateGroupInfo(INTERRUPT_TYPE, GROUP_NAME_DEFAULT, deviceDesc.interruptGroupId_, networkId,
        statusInfo.isConnected, statusInfo.mappingInterruptId);
    if (statusInfo.isConnected) {
        if (AudioPolicyConnectedDevice::GetInstance().GetConnectedDeviceByType(networkId, devType) != nullptr) {
            return ERROR;
        }
        int32_t ret = ActivateNewDevice(statusInfo.networkId, devType,
            statusInfo.connectType == ConnectType::CONNECT_TYPE_DISTRIBUTED);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "DEVICE online but open audio device failed.");
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenConnecting(deviceDesc, descForCb);

        if (statusInfo.connectType == ConnectType::CONNECT_TYPE_DISTRIBUTED) {
            AudioPolicyServerProxy::GetInstance().NotifyDeviceInfoProxy(networkId, true);
        }
    } else {
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenDisconnecting(deviceDesc, descForCb);
        std::string moduleName = AudioPolicyCommon::GetInstance().GetRemoteModuleName(networkId, AudioPolicyCommon::GetInstance().GetDeviceRole(devType));
        std::string sinkName = AudioPolicyCommon::GetInstance().GetSinkPortName(AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->deviceType_);
        AudioPolicyIOHandleManager::GetInstance().CloseRemoteDeviceIOHandle(moduleName, sinkName);
        AudioPolicyRouteMapManager::GetInstance().RemoveDeviceInRouterMap(moduleName);
        AudioPolicyRouteMapManager::GetInstance().RemoveDeviceInFastRouterMap(networkId);
    }
    return SUCCESS;
}

void AudioPolicyDeviceStatus::AddEarpiece()
{
    sptr<AudioDeviceDescriptor> audioDescriptor =
        new (std::nothrow) AudioDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE);
    CHECK_AND_RETURN_LOG(audioDescriptor != nullptr, "Create earpiect device descriptor failed");

    // Use speaker streaminfo for earpiece cap
    auto itr = AudioPolicyConnectedDevice::GetInstance().GetConnectedDeviceByType(DEVICE_TYPE_SPEAKER);
    if (itr != nullptr) {
        audioDescriptor->SetDeviceCapability(itr->audioStreamInfo_, 0);
    }
    audioDescriptor->deviceId_ = AudioPolicyCommon::startDeviceId++;
    AudioPolicyCommon::GetInstance().UpdateDisplayName(audioDescriptor);
    AudioDeviceManager::GetAudioDeviceManager().AddNewDevice(audioDescriptor);
    AudioPolicyConnectedDevice::GetInstance().AddConnectedDevice(audioDescriptor);
    AUDIO_INFO_LOG("Add earpiece to device list");
}

void AudioPolicyDeviceStatus::LoadModernInnerCapSink()
{
    AUDIO_INFO_LOG("Start");
    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    moduleInfo.name = INNER_CAPTURER_SINK;

    moduleInfo.format = "s16le";
    moduleInfo.channels = "2"; // 2 channel
    moduleInfo.rate = "48000";
    moduleInfo.bufferSize = "3840"; // 20ms

    AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
}

int32_t AudioPolicyDeviceStatus::LoadDefaultModule()
{
    int32_t result = ERROR;
    AUDIO_DEBUG_LOG("[module_load]::HDI and AUDIO SERVICE is READY. Loading default modules");
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo;
    AudioPolicyConfigManager::GetInstance().GetDeviceClassInfo(deviceClassInfo);
    for (const auto &device : deviceClassInfo) {
        if (device.first == ClassType::TYPE_PRIMARY || device.first == ClassType::TYPE_FILE_IO) {
            auto moduleInfoList = device.second;
            for (auto &moduleInfo : moduleInfoList) {
                AUDIO_INFO_LOG("[module_load]::Load module[%{public}s]", moduleInfo.name.c_str());
                uint32_t sinkLatencyInMsec = AudioPolicyConfigManager::GetInstance().GetSinkLatencyFromXml();
                moduleInfo.sinkLatency = sinkLatencyInMsec != 0 ? to_string(sinkLatencyInMsec) : "";
                if (OpenPortAndAddDeviceOnServiceConnected(moduleInfo)) {
                    result = SUCCESS;
                }
                AudioPolicyOffloadStream::GetInstance().SetOffloadAvailableFromXML(moduleInfo);
            }
        }
    }
    return result;
}

bool AudioPolicyDeviceStatus::OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo)
{
    auto devType = AudioPolicyCommon::GetInstance().GetDeviceType(moduleInfo.name);
    if (devType != DEVICE_TYPE_MIC) {
        AudioPolicyIOHandleManager::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
        if (devType == DEVICE_TYPE_SPEAKER) {
            auto result = AudioPolicyManagerFactory::GetAudioPolicyManager().SetDeviceActive(devType, moduleInfo.name, true);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, false, "[module_load]::Device failed %{public}d", devType);
        }
    } else {
        AudioPolicyEc::GetInstance().SetPrimaryMicModuleInfo(moduleInfo);
    }

    if (devType == DEVICE_TYPE_SPEAKER || devType == DEVICE_TYPE_MIC) {
        AddAudioDevice(moduleInfo, devType);
    }

    AudioPolicyVolume::GetInstance().NotifyVolumeGroup();

    return true;
}

void AudioPolicyDeviceStatus::AddAudioDevice(AudioModuleInfo& moduleInfo, InternalDeviceType devType)
{
    // add new device into active device list
    std::string volumeGroupName = AudioPolicyConfigManager::GetInstance().GetGroupName(moduleInfo.name, VOLUME_TYPE);
    std::string interruptGroupName = AudioPolicyConfigManager::GetInstance().GetGroupName(moduleInfo.name, INTERRUPT_TYPE);
    int32_t volumeGroupId = GROUP_ID_NONE;
    int32_t interruptGroupId = GROUP_ID_NONE;
    AudioPolicyVolume::GetInstance().UpdateGroupInfo(GroupType::VOLUME_TYPE, volumeGroupName, volumeGroupId, LOCAL_NETWORK_ID, true,
        NO_REMOTE_ID);
    AudioPolicyVolume::GetInstance().UpdateGroupInfo(GroupType::INTERRUPT_TYPE, interruptGroupName, interruptGroupId, LOCAL_NETWORK_ID,
        true, NO_REMOTE_ID);

    sptr<AudioDeviceDescriptor> audioDescriptor = new(std::nothrow) AudioDeviceDescriptor(devType,
        AudioPolicyCommon::GetInstance().GetDeviceRole(moduleInfo.role), volumeGroupId, interruptGroupId, LOCAL_NETWORK_ID);
    if (!moduleInfo.supportedRate_.empty() && !moduleInfo.supportedChannels_.empty()) {
        DeviceStreamInfo streamInfo = {};
        for (auto supportedRate : moduleInfo.supportedRate_) {
            streamInfo.samplingRate.insert(static_cast<AudioSamplingRate>(supportedRate));
        }
        for (auto supportedChannels : moduleInfo.supportedChannels_) {
            streamInfo.channels.insert(static_cast<AudioChannel>(supportedChannels));
        }
        audioDescriptor->SetDeviceCapability(streamInfo, 0);
    }

    audioDescriptor->deviceId_ = AudioPolicyCommon::startDeviceId++;
    AudioPolicyCommon::GetInstance().UpdateDisplayName(audioDescriptor);
    AudioDeviceManager::GetAudioDeviceManager().AddNewDevice(audioDescriptor);
    AudioPolicyConnectedDevice::GetInstance().AddConnectedDevice(audioDescriptor);
    AudioPolicyMicrophone::GetInstance().AddMicrophoneDescriptor(audioDescriptor);
}

void AudioPolicyDeviceStatus::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    int32_t result = LoadDefaultModule();
    if (result == SUCCESS) {
        AUDIO_INFO_LOG("[module_load]::Setting speaker as active device on bootup");
        hasModulesLoaded_ = true;
        unique_ptr<AudioDeviceDescriptor> outDevice = AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice();
        AudioPolicyActiveDevice::GetInstance().SetCurrenOutputDevice(*outDevice);
        unique_ptr<AudioDeviceDescriptor> inDevice = AudioDeviceManager::GetAudioDeviceManager().GetCaptureDefaultDevice();
        AudioPolicyActiveDevice::GetInstance().SetCurrenInputDevice(*inDevice);
        AudioPolicyVolume::GetInstance().SetVolumeForSwitchDevice(outDevice->deviceType_);
        OnPreferredDeviceUpdated(*outDevice, inDevice->deviceType_);
        if (AudioPolicyConfigManager::GetInstance().GetHasEarpiece()) {
            AddEarpiece();
        }
        for (auto it = pnpDeviceList_.begin(); it != pnpDeviceList_.end(); ++it) {
            OnPnpDeviceStatusUpdated((*it).first, (*it).second);
        }
        AudioEffectManager::GetAudioEffectManager().SetMasterSinkAvailable();
    }
    // load inner-cap-sink
    LoadModernInnerCapSink();
    // RegisterBluetoothListener() will be called when bluetooth_host is online
    // load hdi-effect-model
    AudioPolicyServerProxy::GetInstance().LoadHdiEffectModelProxy();
}

void AudioPolicyDeviceStatus::OnPreferredDeviceUpdated(const AudioDeviceDescriptor& activeOutputDevice,
    DeviceType activeInputDevice)
{
    AudioPolicyDeviceCommon::GetInstance().OnPreferredOutputDeviceUpdated(activeOutputDevice);
    AudioPolicyDeviceCommon::GetInstance().OnPreferredInputDeviceUpdated(activeInputDevice, LOCAL_NETWORK_ID);
}

void AudioPolicyDeviceStatus::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    if (macAddress.empty()) {
        AUDIO_ERR_LOG("OnForcedDeviceSelected failed as the macAddress is empty!");
        return;
    }
    AUDIO_INFO_LOG("bt select device type[%{public}d] address[%{public}s]",
        devType, AudioPolicyCommon::GetInstance().GetEncryptAddr(macAddress).c_str());
    std::vector<unique_ptr<AudioDeviceDescriptor>> bluetoothDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableBluetoothDevice(devType, macAddress);
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    for (auto &dec : bluetoothDevices) {
        if (dec->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
            sptr<AudioDeviceDescriptor> tempDec = new(std::nothrow) AudioDeviceDescriptor(*dec);
            audioDeviceDescriptors.push_back(move(tempDec));
        }
    }
    int32_t res = AudioPolicyDeviceCommon::GetInstance().DeviceParamsCheck(DeviceRole::OUTPUT_DEVICE, audioDeviceDescriptors);
    CHECK_AND_RETURN_LOG(res == SUCCESS, "OnForcedDeviceSelected DeviceParamsCheck no success");
    audioDeviceDescriptors[0]->isEnable_ = true;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(audioDeviceDescriptors[0], ENABLE_UPDATE);
    if (devType == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER, audioDeviceDescriptors[0]);
        AudioPolicyCommon::GetInstance().ClearScoDeviceSuspendState(audioDeviceDescriptors[0]->macAddress_);
    } else {
        AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, audioDeviceDescriptors[0]);
    }
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
}

void AudioPolicyDeviceStatus::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, DeviceType devType,
    std::string macAddress, std::string deviceName, bool isActualConnection, AudioStreamInfo streamInfo, bool isConnected)
{
    AUDIO_INFO_LOG("Device connection state updated | TYPE[%{public}d] STATUS[%{public}d], mac[%{public}s]",
        devType, isConnected, GetEncryptStr(macAddress).c_str());

    UpdateLocalGroupInfo(isConnected, macAddress, deviceName, streamInfo, updatedDesc);
    // fill device change action for callback
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    std::vector<sptr<AudioDeviceDescriptor>> descForCb = {};
    UpdateDeviceList(updatedDesc, isConnected, descForCb, reason);

    TriggerDeviceChangedCallback(descForCb, isConnected);
    TriggerAvailableDeviceChangedCallback(descForCb, isConnected);

    if (!isActualConnection) {
        return;
    }
    // fetch input&output device
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, reason);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);
    // update a2dp offload
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream();
    }
}

void AudioPolicyDeviceStatus::UpdateDeviceList(AudioDeviceDescriptor &updatedDesc,  bool isConnected,
    std::vector<sptr<AudioDeviceDescriptor>> &descForCb,
    AudioStreamDeviceChangeReasonExt &reason)
{
    if (isConnected) {
        // deduplicate
        AudioPolicyConnectedDevice::GetInstance().DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_, updatedDesc.macAddress_);
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);
        int32_t result = HandleLocalDeviceConnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Connect local device failed.");
        reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
#ifdef BLUETOOTH_ENABLE
        CheckAndActiveHfpDevice(updatedDesc);
#endif
    } else {
        AudioPolicyDeviceCommon::GetInstance().UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
        AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, reason); //  fix pop, fetch device before unload module
        int32_t result = HandleLocalDeviceDisconnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Disconnect local device failed.");
        reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    }
}

#ifdef BLUETOOTH_ENABLE
void AudioPolicyDeviceStatus::CheckAndActiveHfpDevice(AudioDeviceDescriptor &desc)
{
    if (desc.connectState_ == CONNECTED &&
        desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioRendererInfo rendererInfo = {};
        rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        std::vector<sptr<AudioDeviceDescriptor>> preferredDeviceList =
            AudioPolicyDeviceCommon::GetInstance().GetPreferredOutputDeviceDescInner(rendererInfo);
        if (preferredDeviceList.size() > 0 &&
            preferredDeviceList[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            Bluetooth::AudioHfpManager::SetActiveHfpDevice(preferredDeviceList[0]->macAddress_);
        }
    }
}
#endif

void AudioPolicyDeviceStatus::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    AUDIO_INFO_LOG("[%{public}s] type[%{public}d] command: %{public}d category[%{public}d] connectState[%{public}d] " \
        "isEnable[%{public}d]", AudioPolicyCommon::GetInstance().GetEncryptAddr(desc.macAddress_).c_str(), desc.deviceType_,
        command, desc.deviceCategory_, desc.connectState_, desc.isEnable_);
    if (command == ENABLE_UPDATE && desc.isEnable_ == true) {
        if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            AudioPolicyCommon::GetInstance().ClearScoDeviceSuspendState(desc.macAddress_);
        }
        unique_ptr<AudioDeviceDescriptor> userSelectMediaDevice =
            AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice();
        unique_ptr<AudioDeviceDescriptor> userSelectCallDevice =
            AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
        if ((userSelectMediaDevice->deviceType_ == desc.deviceType_ &&
            userSelectMediaDevice->macAddress_ == desc.macAddress_ &&
            userSelectMediaDevice->isEnable_ == desc.isEnable_) ||
            (userSelectCallDevice->deviceType_ == desc.deviceType_ &&
            userSelectCallDevice->macAddress_ == desc.macAddress_ &&
            userSelectCallDevice->isEnable_ == desc.isEnable_)) {
            AUDIO_INFO_LOG("Current enable state has been set true during user selection, no need to be set again.");
            return;
        }
    } else if (command == ENABLE_UPDATE && !desc.isEnable_ && desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
        AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice().macAddress_ == desc.macAddress_) {
        std::string sinkName = AudioPolicyCommon::GetInstance().GetSinkPortName(AudioPolicyActiveDevice::GetInstance().GetActiveOutputDeviceDescriptor()->deviceType_);
        AudioPolicyIOHandleManager::GetInstance().CloseRemoteDeviceIOHandle(BLUETOOTH_SPEAKER, sinkName);
    }
    sptr<AudioDeviceDescriptor> audioDescriptor = new(std::nothrow) AudioDeviceDescriptor(desc);
    AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(audioDescriptor, command);
    CheckForA2dpSuspend(desc);

    // VGS feature
    AudioPolicyVolume::GetInstance().HandleVGSVolume(desc);

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    OnPreferredStateUpdated(desc, command, reason);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(true, reason);
    AudioPolicyDeviceCommon::GetInstance().FetchDevice(false);
    AudioA2dpOffloadManager::GetInstance().UpdateA2dpOffloadFlagForAllStream();
}

void AudioPolicyDeviceStatus::CheckForA2dpSuspend(AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
        return;
    }
    if (AudioDeviceManager::GetAudioDeviceManager().GetScoState()) {
        AudioPolicyServerProxy::GetInstance().SuspendRenderSinkProxy("a2dp");
    } else {
        AudioPolicyServerProxy::GetInstance().RestoreRenderSinkProxy("a2dp");
    }
}

void AudioPolicyDeviceStatus::OnPreferredStateUpdated(AudioDeviceDescriptor &desc,
    const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason)
{
    AudioStateManager& stateManager = AudioStateManager::GetAudioStateManager();
    unique_ptr<AudioDeviceDescriptor> userSelectMediaRenderDevice = stateManager.GetPreferredMediaRenderDevice();
    unique_ptr<AudioDeviceDescriptor> userSelectCallRenderDevice = stateManager.GetPreferredCallRenderDevice();
    unique_ptr<AudioDeviceDescriptor> userSelectCallCaptureDevice = stateManager.GetPreferredCallCaptureDevice();
    unique_ptr<AudioDeviceDescriptor> userSelectRecordCaptureDevice = stateManager.GetPreferredRecordCaptureDevice();
    vector<unique_ptr<AudioDeviceDescriptor>> userSelectDeviceMap;
    userSelectDeviceMap.push_back(make_unique<AudioDeviceDescriptor>(*userSelectMediaRenderDevice));
    userSelectDeviceMap.push_back(make_unique<AudioDeviceDescriptor>(*userSelectCallRenderDevice));
    userSelectDeviceMap.push_back(make_unique<AudioDeviceDescriptor>(*userSelectCallCaptureDevice));
    userSelectDeviceMap.push_back(make_unique<AudioDeviceDescriptor>(*userSelectRecordCaptureDevice));
    if (updateCommand == CATEGORY_UPDATE) {
        if (desc.deviceCategory_ == BT_UNWEAR_HEADPHONE) {
            reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
            UpdateAllUserSelectDevice(userSelectDeviceMap, desc, new(std::nothrow) AudioDeviceDescriptor());
#ifdef BLUETOOTH_ENABLE
            if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc.macAddress_ == AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice().macAddress_) {
                Bluetooth::AudioA2dpManager::SetActiveA2dpDevice("");
            } else if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
                desc.macAddress_ == AudioPolicyActiveDevice::GetInstance().GetCurrentOutputDevice().macAddress_) {
                Bluetooth::AudioHfpManager::DisconnectSco();
            }
#endif
        } else {
            reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
            if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
                AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, new(std::nothrow) AudioDeviceDescriptor());
                AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE, new(std::nothrow) AudioDeviceDescriptor());
            } else {
                AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER, new(std::nothrow) AudioDeviceDescriptor());
                AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE, new(std::nothrow) AudioDeviceDescriptor());
                AudioPolicyCommon::GetInstance().ClearScoDeviceSuspendState(desc.macAddress_);
            }
        }
    } else if (updateCommand == ENABLE_UPDATE) {
        UpdateAllUserSelectDevice(userSelectDeviceMap, desc, new(std::nothrow) AudioDeviceDescriptor(desc));
        reason = desc.isEnable_ ? AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE :
            AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    }
}

void AudioPolicyDeviceStatus::UpdateAllUserSelectDevice(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &userSelectDeviceMap,
    AudioDeviceDescriptor &desc, const sptr<AudioDeviceDescriptor> &selectDesc)
{
    if (userSelectDeviceMap[MEDIA_RENDER_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[MEDIA_RENDER_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[MEDIA_RENDER_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, new(std::nothrow) AudioDeviceDescriptor(selectDesc));
        } else {
            AudioStateManager::GetAudioStateManager().UpdatePreferredMediaRenderDeviceConnectState(desc.connectState_);
        }
    }
    if (userSelectDeviceMap[CALL_RENDER_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[CALL_RENDER_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[CALL_RENDER_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER, new(std::nothrow) AudioDeviceDescriptor(selectDesc));
        } else {
            AudioStateManager::GetAudioStateManager().UpdatePreferredCallRenderDeviceConnectState(desc.connectState_);
        }
    }
    if (userSelectDeviceMap[CALL_CAPTURE_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[CALL_CAPTURE_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[CALL_CAPTURE_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE, new(std::nothrow) AudioDeviceDescriptor(selectDesc));
        } else {
            AudioStateManager::GetAudioStateManager().UpdatePreferredCallCaptureDeviceConnectState(desc.connectState_);
        }
    }
    if (userSelectDeviceMap[RECORD_CAPTURE_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[RECORD_CAPTURE_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[RECORD_CAPTURE_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyCommon::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE, new(std::nothrow) AudioDeviceDescriptor(selectDesc));
        } else {
            AudioStateManager::GetAudioStateManager().UpdatePreferredRecordCaptureDeviceConnectState(desc.connectState_);
        }
    }
}



}
}