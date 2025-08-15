
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
#define LOG_TAG "AudioDeviceStatus"
#endif

#include "audio_device_status.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "common/hdi_adapter_info.h"

#include "audio_policy_utils.h"
#include "audio_event_utils.h"
#include "audio_server_proxy.h"
#include "audio_core_service.h"
#include "audio_utils_c.h"
#include "sle_audio_device_manager.h"
#include "audio_zone_service.h"

namespace OHOS {
namespace AudioStandard {
const int MEDIA_RENDER_ID = 0;
const int CALL_RENDER_ID = 1;
const int CALL_CAPTURE_ID = 2;
const int RECORD_CAPTURE_ID = 3;
const uint32_t REHANDLE_DEVICE_RETRY_INTERVAL_IN_MICROSECONDS = 30000;
const std::string DEFAULT_BUFFER_SIZE_8000 = "320";

const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;

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

static std::string GetField(const std::string &src, const char* field, const char sep)
{
    auto str = std::string(field) + '=';
    auto pos = src.find(str);
    CHECK_AND_RETURN_RET(pos != std::string::npos, "");
    pos += str.length();
    auto end = src.find(sep, pos);
    return end == std::string::npos ? src.substr(pos) : src.substr(pos, end - pos);
}

static void GetDPModuleInfo(AudioModuleInfo &moduleInfo, string deviceInfo)
{
    auto rate_begin = deviceInfo.find("rate=");
    auto rate_end = deviceInfo.find_first_of(" ", rate_begin);
    if (rate_end > rate_begin) {
        moduleInfo.rate = deviceInfo.substr(rate_begin + std::strlen("rate="),
            rate_end - rate_begin - std::strlen("rate="));
    }
    if (moduleInfo.role == "sink") {
        auto sinkFormat_begin = deviceInfo.find("format=");
        auto sinkFormat_end = deviceInfo.find_first_of(" ", sinkFormat_begin);
        string format = deviceInfo.substr(sinkFormat_begin + std::strlen("format="),
            sinkFormat_end - sinkFormat_begin - std::strlen("format="));
        if (!format.empty()) moduleInfo.format = format;

        auto sinkChannel_begin = deviceInfo.find("channels=");
        auto sinkChannel_end = deviceInfo.find_first_of(" ", sinkChannel_begin);
        string channel = deviceInfo.substr(sinkChannel_begin + std::strlen("channels="),
            sinkChannel_end - sinkChannel_begin - std::strlen("channels="));
        moduleInfo.channels = channel;

        auto sinkBSize_begin = deviceInfo.find("buffer_size=");
        auto sinkBSize_end = deviceInfo.find_first_of(" ", sinkBSize_begin);
        string bufferSize = deviceInfo.substr(sinkBSize_begin + std::strlen("buffer_size="),
            sinkBSize_end - sinkBSize_begin - std::strlen("buffer_size="));
        moduleInfo.bufferSize = bufferSize;
    }
}

void AudioDeviceStatus::Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager,
    std::shared_ptr<AudioPolicyServerHandler> handler)
{
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
    audioPolicyServerHandler_ = handler;
}

void AudioDeviceStatus::DeInit()
{
    audioA2dpOffloadManager_ = nullptr;
    audioPolicyServerHandler_ = nullptr;
}

void AudioDeviceStatus::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    // fill device change action for callback
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};

    int32_t result = HandleSpecialDeviceType(devType, isConnected, macAddress, role);
    CheckAndWriteDeviceChangeExceptionEvent(result == SUCCESS, reason, devType, role, result,
        "handle special deviceType failed.");
    CHECK_AND_RETURN_LOG(result == SUCCESS, "handle special deviceType failed.");

    AUDIO_WARNING_LOG("[ADeviceEvent] device[%{public}d] address[%{public}s] role[%{public}d] connect[%{public}d]",
        devType, GetEncryptStr(macAddress).c_str(), role, isConnected);

    AudioDeviceDescriptor updatedDesc(devType, role == DEVICE_ROLE_NONE ?
        AudioPolicyUtils::GetInstance().GetDeviceRole(devType) : role);
    updatedDesc.hasPair_ = hasPair;
    UpdateLocalGroupInfo(isConnected, macAddress, deviceName, streamInfo, updatedDesc);

    if (isConnected) {
        // If device already in list, remove it else do not modify the list
        audioConnectedDevice_.DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_,
            updatedDesc.macAddress_, updatedDesc.deviceRole_);
        // If the pnp device fails to load, it will not connect
        result = HandleLocalDeviceConnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Connect local device failed.");
        audioDeviceCommon_.UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);

        reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
#ifdef BLUETOOTH_ENABLE
    if (updatedDesc.connectState_ == CONNECTED &&
        updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioRendererInfo rendererInfo = {};
        rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
            audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo);
        if (preferredDeviceList.size() > 0 &&
            preferredDeviceList[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            Bluetooth::AudioHfpManager::SetActiveHfpDevice(preferredDeviceList[0]->macAddress_);
        }
    }
#endif
    } else {
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    }

    TriggerDeviceChangedCallback(descForCb, isConnected);
    TriggerAvailableDeviceChangedCallback(descForCb, isConnected);

    // fetch input&output device
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("OnDeviceStatusUpdated_2", reason);
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("OnDeviceStatusUpdated_2");

    if (!isConnected) {
        result = HandleLocalDeviceDisconnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Disconnect local device failed.");
    }

    // update a2dp offload
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "OnDeviceStatusUpdated 5 param");
}

void AudioDeviceStatus::WriteOutputDeviceChangedSysEvents(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const SinkInput &sinkInput)
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
    AUTO_CTRACE("SYSEVENT BEHAVIOR EVENT DEVICE_CHANGE, ISOUTPUT: 1, STREAMID: %d, STREAMTYPE: %d, DEVICETYPE: %d, "
        "NETWORKID: %s, ADDRESS: %s, DEVICE_NAME: %s, BT_TYPE: %d", sinkInput.streamId, sinkInput.streamType,
        deviceDescriptor->deviceType_, ConvertNetworkId(deviceDescriptor->networkId_).c_str(),
        GetEncryptAddr(deviceDescriptor->macAddress_).c_str(),
        deviceDescriptor->deviceName_.c_str(), deviceDescriptor->deviceCategory_);
}

void AudioDeviceStatus::WriteInputDeviceChangedSysEvents(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const SourceOutput &sourceOutput)
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
    AUTO_CTRACE("SYSEVENT BEHAVIOR EVENT DEVICE_CHANGE, ISOUTPUT: 0, STREAMID: %d, STREAMTYPE: %d, DEVICETYPE: %d, "
        "NETWORKID: %s, ADDRESS: %s, DEVICE_NAME: %s, BT_TYPE: %d", sourceOutput.streamId, sourceOutput.streamType,
        deviceDescriptor->deviceType_, ConvertNetworkId(deviceDescriptor->networkId_).c_str(),
        GetEncryptAddr(deviceDescriptor->macAddress_).c_str(),
        deviceDescriptor->deviceName_.c_str(), deviceDescriptor->deviceCategory_);
}


void AudioDeviceStatus::WriteHeadsetSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &desc, bool isConnected)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    if ((desc->deviceType_ == DEVICE_TYPE_WIRED_HEADSET) ||
        (desc->deviceType_ == DEVICE_TYPE_USB_HEADSET) ||
        (desc->deviceType_ == DEVICE_TYPE_WIRED_HEADPHONES)) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::HEADSET_CHANGE,
            Media::MediaMonitor::BEHAVIOR_EVENT);
        bean->Add("HASMIC", 1);
        bean->Add("ISCONNECT", isConnected ? 1 : 0);
        bean->Add("DEVICETYPE", desc->deviceType_);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    }
}

void AudioDeviceStatus::WriteDeviceChangeSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    if (desc->deviceRole_ == OUTPUT_DEVICE) {
        std::vector<SinkInput> sinkInputs;
        audioPolicyManager_.GetAllSinkInputs(sinkInputs);
        for (SinkInput sinkInput : sinkInputs) {
            WriteOutputDeviceChangedSysEvents(desc, sinkInput);
        }
    } else if (desc->deviceRole_ == INPUT_DEVICE) {
        std::vector<SourceOutput> sourceOutputs = audioDeviceCommon_.GetSourceOutputs();
        for (SourceOutput sourceOutput : sourceOutputs) {
            WriteInputDeviceChangedSysEvents(desc, sourceOutput);
        }
    }
}

void AudioDeviceStatus::WriteAllDeviceSysEvents(
    const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    Trace trace("AudioDeviceStatus::WriteAllDeviceSysEvents");
    for (auto deviceDescriptor : desc) {
        WriteHeadsetSysEvents(deviceDescriptor, isConnected);
        if (!isConnected) {
            continue;
        }
        WriteDeviceChangeSysEvents(deviceDescriptor);
    }
}

void AudioDeviceStatus::TriggerAvailableDeviceChangedCallback(
    const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    Trace trace("AudioDeviceStatus::TriggerAvailableDeviceChangedCallback");

    WriteAllDeviceSysEvents(desc, isConnected);

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendAvailableDeviceChange(desc, isConnected);
    }
}

void AudioDeviceStatus::TriggerDeviceChangedCallback(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc,
    bool isConnected)
{
    Trace trace("AudioDeviceStatus::TriggerDeviceChangedCallback");
    WriteAllDeviceSysEvents(desc, isConnected);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendDeviceChangedCallback(desc, isConnected);
    }
}

void AudioDeviceStatus::UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc)
{
    deviceDesc.SetDeviceInfo(deviceName, macAddress);
    deviceDesc.SetDeviceCapability({ streamInfo }, 0);
    audioVolumeManager_.UpdateGroupInfo(VOLUME_TYPE, GROUP_NAME_DEFAULT, deviceDesc.volumeGroupId_, LOCAL_NETWORK_ID,
        isConnected, NO_REMOTE_ID);
    audioVolumeManager_.UpdateGroupInfo(INTERRUPT_TYPE, GROUP_NAME_DEFAULT, deviceDesc.interruptGroupId_,
        LOCAL_NETWORK_ID, isConnected, NO_REMOTE_ID);
    deviceDesc.networkId_ = LOCAL_NETWORK_ID;
}

int32_t AudioDeviceStatus::RehandlePnpDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address)
{
    Trace trace("AudioDeviceStatus::RehandlePnpDevice");

    // Maximum number of attempts, preventing situations where hal has not yet finished coming online.
    int32_t maxRetries = 3;
    int32_t retryCount = 0;
    bool isConnected = true;
    while (retryCount < maxRetries) {
        retryCount++;
        AUDIO_INFO_LOG("rehandle device[%{public}d], retry count[%{public}d]", deviceType, retryCount);

        int32_t ret = HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Rehandle special device type failed");
        if (deviceType == DEVICE_TYPE_USB_HEADSET) {
            AUDIO_INFO_LOG("Hifi device, don't load module");
            return ret;
        }
        if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            if (HandleArmUsbDevice(deviceType, deviceRole, address) == SUCCESS) {
                return SUCCESS;
            }
        } else if (deviceType == DEVICE_TYPE_DP) {
            if (HandleDpDevice(deviceType, address)  == SUCCESS) {
                return SUCCESS;
            }
        } else if (deviceType == DEVICE_TYPE_ACCESSORY) {
            if (HandleAccessoryDevice(deviceType, address)  == SUCCESS) {
                return SUCCESS;
            }
        }
        usleep(REHANDLE_DEVICE_RETRY_INTERVAL_IN_MICROSECONDS);
    }

    AUDIO_ERR_LOG("rehandle device[%{public}d] failed", deviceType);
    return ERROR;
}

int32_t AudioDeviceStatus::HandleDpDevice(DeviceType deviceType, const std::string &address)
{
    Trace trace("AudioDeviceStatus::HandleDpDevice");
    if (deviceType == DEVICE_TYPE_DP) {
        std::string defaulyDPInfo = "";
        std::string getDPInfo = "";
        GetModuleInfo(ClassType::TYPE_DP, defaulyDPInfo);
        CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

        getDPInfo = AudioServerProxy::GetInstance().GetAudioParameterProxy(LOCAL_NETWORK_ID, GET_DP_DEVICE_INFO,
            defaulyDPInfo + " address=" + address + " ");
        AUDIO_DEBUG_LOG("device info from dp hal is \n defaulyDPInfo:%{public}s", defaulyDPInfo.c_str());

        getDPInfo = getDPInfo.empty() ? defaulyDPInfo : getDPInfo;
        int32_t ret = LoadDpModule(getDPInfo);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG ("load dp module failed");
            return ERR_OPERATION_FAILED;
        }
        std::string activePort = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_DP);
        AUDIO_INFO_LOG("port %{public}s, active dp device", activePort.c_str());
    } else if (audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_DP) {
        std::string activePort = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_DP);
        audioPolicyManager_.SuspendAudioDevice(activePort, true);
    }

    return SUCCESS;
}

int32_t AudioDeviceStatus::HandleAccessoryDevice(DeviceType deviceType, const std::string &address)
{
    Trace trace("AudioDeviceStatus::HandleAccessoryDevice");
    std::string defaulyAccessoryInfo = "";
    auto res = GetModuleInfo(ClassType::TYPE_ACCESSORY, defaulyAccessoryInfo);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "get module info failed");
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");
    char sampleRate[10] = {0};
    // default samplerate of accessory is 16000
    GetParameter("hw.pencil.samplerate", "16000", sampleRate, sizeof(sampleRate));

    auto rate_begin = defaulyAccessoryInfo.find("rate=");
    auto rate_end = defaulyAccessoryInfo.find_first_of(" ", rate_begin);
    CHECK_AND_RETURN_RET_LOG(rate_end > rate_begin, ERR_OPERATION_FAILED, "get rate failed");
    defaulyAccessoryInfo.replace(rate_begin + std::strlen("rate="),
        rate_end - rate_begin - std::strlen("rate="), sampleRate);
    if (strncmp(sampleRate, "8000", sizeof("8000")) == 0) { // when double connect samplerate of accessory is 8000
        auto size_begin = defaulyAccessoryInfo.find("buffer_size=");
        auto size_end = defaulyAccessoryInfo.find_first_of(" ", size_begin);
        CHECK_AND_RETURN_RET_LOG(size_end > size_begin, ERR_OPERATION_FAILED, "get size failed");
        defaulyAccessoryInfo.replace(size_begin + std::strlen("buffer_size="),
            size_end - size_begin - std::strlen("buffer_size="), DEFAULT_BUFFER_SIZE_8000);
    }

    AUDIO_INFO_LOG("device info from accessory hal is defaulyAccessoryInfo: %{public}s",
        defaulyAccessoryInfo.c_str());

    int32_t ret = LoadAccessoryModule(defaulyAccessoryInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG ("load accessory module failed");
        return ERR_OPERATION_FAILED;
    }
    std::string activePort = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    AUDIO_INFO_LOG("port %{public}s, active accessory device", activePort.c_str());
    return SUCCESS;
}

int32_t AudioDeviceStatus::HandleLocalDeviceConnected(AudioDeviceDescriptor &updatedDesc)
{
    DeviceStreamInfo audioStreamInfo = updatedDesc.GetDeviceStreamInfo();
    if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        A2dpDeviceConfigInfo configInfo = {audioStreamInfo, false};
        audioA2dpDevice_.AddA2dpDevice(updatedDesc.macAddress_, configInfo);
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        A2dpDeviceConfigInfo configInfo = {audioStreamInfo, false};
        audioA2dpDevice_.AddA2dpInDevice(updatedDesc.macAddress_, configInfo);
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_DP) {
        int32_t result = HandleDpDevice(updatedDesc.deviceType_, updatedDesc.macAddress_);
        if (result != SUCCESS) {
            result = RehandlePnpDevice(updatedDesc.deviceType_, OUTPUT_DEVICE, updatedDesc.macAddress_);
        }
        CheckAndWriteDeviceChangeExceptionEvent(result == SUCCESS,
            AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
            updatedDesc.deviceType_, updatedDesc.deviceRole_, result, "Load dp failed.");
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Load dp failed.");
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_USB_HEADSET ||
        updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_LOCAL, "usb");
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_ACCESSORY) {
        int32_t result = HandleAccessoryDevice(updatedDesc.deviceType_, updatedDesc.macAddress_);
        CheckAndWriteDeviceChangeExceptionEvent(result == SUCCESS,
            AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
            updatedDesc.deviceType_, updatedDesc.deviceRole_, result, "Load accessory failed.");
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Load accessory failed.");
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_NEARLINK) {
        SleAudioDeviceManager::GetInstance().AddNearlinkDevice(updatedDesc);
        audioVolumeManager_.SetNearlinkDeviceVolume(updatedDesc.macAddress_, STREAM_MUSIC,
            SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(STREAM_MUSIC, updatedDesc));
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_HEARING_AID) {
        A2dpDeviceConfigInfo configInfo = {audioStreamInfo, false};
        audioA2dpDevice_.AddHearingAidDevice(updatedDesc.macAddress_, configInfo);
    }
    return SUCCESS;
}

void AudioDeviceStatus::UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress)
{
    AUDIO_INFO_LOG("In");
    if (audioA2dpDevice_.DelA2dpDevice(macAddress) == 0) {
        audioActiveDevice_.SetActiveBtDeviceMac("");
        audioIOHandleMap_.ClosePortAndEraseIOHandle(BLUETOOTH_SPEAKER);
        audioPolicyManager_.SetAbsVolumeScene(false);
        audioVolumeManager_.SetSharedAbsVolumeScene(false);
#ifdef BLUETOOTH_ENABLE
        Bluetooth::AudioA2dpManager::SetActiveA2dpDevice("");
        AudioServerProxy::GetInstance().SetBtHdiInvalidState();
        AudioServerProxy::GetInstance().UnloadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_a2dp", true);
        AudioServerProxy::GetInstance().UnloadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_a2dp_fast", true);
        AudioServerProxy::GetInstance().UnloadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_hdap", true);
#endif
        return;
    }
}

int32_t AudioDeviceStatus::HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc)
{
    if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        UpdateActiveA2dpDeviceWhenDisconnecting(updatedDesc.macAddress_);
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        if (audioA2dpDevice_.DelA2dpInDevice(updatedDesc.macAddress_) == 0) {
            audioActiveDevice_.SetActiveBtInDeviceMac("");
            audioIOHandleMap_.ClosePortAndEraseIOHandle(BLUETOOTH_MIC);
        }
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_DP) {
        audioIOHandleMap_.ClosePortAndEraseIOHandle(GetModuleNameByType(TYPE_DP));
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioEcManager_.CloseUsbArmDevice(updatedDesc);
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_ACCESSORY) {
        audioIOHandleMap_.ClosePortAndEraseIOHandle(ACCESSORY_SOURCE);
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_HEARING_AID) {
        if (audioA2dpDevice_.DelHearingAidDevice(updatedDesc.macAddress_) == 0) {
            audioIOHandleMap_.ClosePortAndEraseIOHandle(HEARING_AID_SPEAKER);
        }
    }
    SleAudioDeviceManager::GetInstance().RemoveNearlinkDevice(updatedDesc);

    AudioServerProxy::GetInstance().ResetRouteForDisconnectProxy(updatedDesc.deviceType_);
    if (updatedDesc.deviceType_ == DEVICE_TYPE_DP) {
        AudioServerProxy::GetInstance().UnloadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_LOCAL, "dp", false);
    } else if (updatedDesc.deviceType_ == DEVICE_TYPE_USB_HEADSET ||
        updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        AudioServerProxy::GetInstance().UnloadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_LOCAL, "usb", false);
    }
    return SUCCESS;
}

int32_t AudioDeviceStatus::HandleArmUsbDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address)
{
    Trace trace("AudioPolicyService::HandleArmUsbDevice");
    if (deviceType != DEVICE_TYPE_USB_ARM_HEADSET &&
        audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_USB_HEADSET) {
        std::string activePort = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_USB_ARM_HEADSET);
        audioPolicyManager_.SuspendAudioDevice(activePort, true);
    }

    return SUCCESS;
}

int32_t AudioDeviceStatus::GetModuleInfo(ClassType classType, std::string &moduleInfoStr)
{
    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = audioConfigManager_.GetModuleListByType(classType, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED,
            "find %{public}d type failed", classType);
    }
    moduleInfoStr = audioPolicyManager_.GetModuleArgs(*moduleInfoList.begin());
    return SUCCESS;
}

int32_t AudioDeviceStatus::LoadDpModule(std::string deviceInfo)
{
    AUDIO_INFO_LOG("LoadDpModule");
    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_DP, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED,
            "dp module is not exist in the configuration file");
    }
    AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_LOCAL, "dp");
    for (auto &moduleInfo : moduleInfoList) {
        AUDIO_INFO_LOG("[module_load]::load module[%{public}s]", moduleInfo.name.c_str());
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            GetDPModuleInfo(moduleInfo, deviceInfo);
            if (moduleInfo.role == ROLE_SINK) {
                AUDIO_INFO_LOG("save dp sink module info for cust param");
                audioEcManager_.SetDpSinkModuleInfo(moduleInfo);
            }
            return audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
        }
    }

    return SUCCESS;
}

int32_t AudioDeviceStatus::LoadAccessoryModule(std::string deviceInfo)
{
    AUDIO_INFO_LOG("LoadAccessoryModule");
    std::list<AudioModuleInfo> moduleInfoList;
    {
        bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_ACCESSORY, moduleInfoList);
        CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED,
            "accessory module is not exist in the configuration file");
    }
    AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_LOCAL, "accessory");
    for (auto &moduleInfo : moduleInfoList) {
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            AUDIO_INFO_LOG("[module_load]::load module[%{public}s]", moduleInfo.name.c_str());
            GetDPModuleInfo(moduleInfo, deviceInfo);
            moduleInfo.deviceType = std::to_string(static_cast<int32_t>(DEVICE_TYPE_ACCESSORY));
            auto size_begin = deviceInfo.find("buffer_size=");
            auto size_end = deviceInfo.find_first_of(" ", size_begin);
            CHECK_AND_RETURN_RET_LOG(size_end > size_begin, ERR_OPERATION_FAILED, "get size failed");
            string bufferSize = deviceInfo.substr(size_begin + std::strlen("buffer_size="),
                size_end - size_begin - std::strlen("buffer_size"));
            moduleInfo.bufferSize = bufferSize;
            return audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
        }
    }
    return SUCCESS;
}

bool AudioDeviceStatus::NoNeedChangeUsbDevice(const string &address)
{
    auto key = string("need_change_usb_device#C") + GetField(address, "card", ';') + "D0";
    auto ret = AudioServerProxy::GetInstance().GetAudioParameterProxy(key);
    AUDIO_INFO_LOG("key=%{public}s, ret=%{public}s", key.c_str(), ret.c_str());
    return ret == "false";
}

int32_t AudioDeviceStatus::HandleSpecialDeviceType(DeviceType &devType, bool &isConnected,
    const std::string &address, DeviceRole role)
{
    if (devType == DEVICE_TYPE_USB_HEADSET || devType == DEVICE_TYPE_USB_ARM_HEADSET) {
        CHECK_AND_RETURN_RET(!address.empty() && role != DEVICE_ROLE_NONE, ERROR);
        AUDIO_INFO_LOG("Entry. Addr:%{public}s, Role:%{public}d, HasHifi:%{public}d, HasArm:%{public}d",
            GetEncryptAddr(address).c_str(), role,
            audioConnectedDevice_.HasHifi(role), audioConnectedDevice_.HasArm(role));
        if (isConnected) {
            // Usb-c maybe reported repeatedly, the devType remains unchanged
            auto exists = audioConnectedDevice_.GetUsbDeviceDescriptor(address, role);
            if (exists) {
                devType = exists->deviceType_;
                return SUCCESS;
            }
            if (audioConnectedDevice_.HasHifi(role) || NoNeedChangeUsbDevice(address)) {
                devType = DEVICE_TYPE_USB_ARM_HEADSET;
            }
        } else if (audioConnectedDevice_.IsArmDevice(address, role)) {
            devType = DEVICE_TYPE_USB_ARM_HEADSET;
            // Temporary resolution to avoid pcm driver problem
            string condition = string("address=") + address + " role=" + to_string(DEVICE_ROLE_NONE);
            string deviceInfo = AudioServerProxy::GetInstance().GetAudioParameterProxy(LOCAL_NETWORK_ID, USB_DEVICE,
                condition);
        }
    } else if (devType == DEVICE_TYPE_EXTERN_CABLE) {
        CheckAndWriteDeviceChangeExceptionEvent(isConnected,
            AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
            devType, role, ERROR, "Extern cable disconnected, do nothing");
        CHECK_AND_RETURN_RET_LOG(isConnected, ERROR, "Extern cable disconnected, do nothing");
        DeviceType connectedHeadsetType = audioConnectedDevice_.FindConnectedHeadset();
        if (connectedHeadsetType == DEVICE_TYPE_NONE) {
            AUDIO_INFO_LOG("Extern cable connect without headset connected before, do nothing");
            return ERROR;
        }
        devType = connectedHeadsetType;
        isConnected = false;
    }

    return SUCCESS;
}

void AudioDeviceStatus::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    CHECK_AND_RETURN_LOG(desc.deviceType_ != DEVICE_TYPE_NONE, "devType is none type");
    if (!hasModulesLoaded) {
        AUDIO_WARNING_LOG("modules has not loaded");
        AudioDeviceDescriptor pnpDesc = desc;
        pnpDeviceList_.push_back({pnpDesc, isConnected});
        return;
    }
    if (desc.deviceType_ == DEVICE_TYPE_DP) {
        if (isConnected) {
            auto exists = audioDeviceManager_.ExistsByType(DEVICE_TYPE_DP);
            CHECK_AND_RETURN_LOG(!exists, "DP device already exists, ignore this one.");
        } else {
            auto exists = audioDeviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_DP, desc.macAddress_);
            CHECK_AND_RETURN_LOG(exists, "DP device does not exist, can not disconnect.");
        }
    }
    AudioStreamInfo streamInfo = {};
    OnDeviceStatusUpdated(desc.deviceType_, isConnected, desc.macAddress_, desc.deviceName_, streamInfo);
}

void AudioDeviceStatus::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    OnBlockedStatusUpdated(devType, status);
}

void AudioDeviceStatus::OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor =
        std::make_shared<AudioDeviceDescriptor>(devType, AudioPolicyUtils::GetInstance().GetDeviceRole(devType));
    descForCb.push_back(audioDescriptor);

    vector<shared_ptr<AudioCapturerChangeInfo>> audioChangeInfos;
    streamCollector_.GetCurrentCapturerChangeInfos(audioChangeInfos);
    for (auto it = audioChangeInfos.begin(); it != audioChangeInfos.end(); it++) {
        if ((*it)->capturerState == CAPTURER_RUNNING) {
            AUDIO_INFO_LOG("record running");
            TriggerMicrophoneBlockedCallback(descForCb, status);
        }
    }
}

void AudioDeviceStatus::TriggerMicrophoneBlockedCallback(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc,
    DeviceBlockStatus status)
{
    Trace trace("AudioDeviceStatus::TriggerMicrophoneBlockedCallback");
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendMicrophoneBlockedCallback(desc, status);
    }
}

void AudioDeviceStatus::ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    uint32_t bufferSize = streamInfo.samplingRate *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(streamInfo.format) *
        streamInfo.channels / BT_BUFFER_ADJUSTMENT_FACTOR;
    AUDIO_INFO_LOG("Updated buffer size: %{public}d", bufferSize);

    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    CHECK_AND_RETURN_LOG(ret, "GetModuleListByType failed");
    for (auto &moduleInfo : moduleInfoList) {
        CHECK_AND_CONTINUE_LOG(audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name),
            "Cannot find module %{public}s", moduleInfo.name.c_str());
        moduleInfo.channels = to_string(streamInfo.channels);
        moduleInfo.rate = to_string(streamInfo.samplingRate);
        moduleInfo.format = AudioPolicyUtils::GetInstance().ConvertToHDIAudioFormat(streamInfo.format);
        moduleInfo.bufferSize = to_string(bufferSize);
        moduleInfo.renderInIdleState = "1";
        moduleInfo.sinkLatency = "0";

        // First unload the existing bt sink
        AUDIO_DEBUG_LOG("UnLoad existing a2dp module");
        std::string currentActivePort
            = AudioPolicyUtils::GetInstance().GetSinkPortName(audioActiveDevice_.GetCurrentOutputDeviceType());
        AudioIOHandle activateDeviceIOHandle;
        audioIOHandleMap_.GetModuleIdByKey(BLUETOOTH_SPEAKER, activateDeviceIOHandle);
        audioIOHandleMap_.MuteDefaultSinkPort(audioActiveDevice_.GetCurrentOutputDeviceNetworkId(),
            AudioPolicyUtils::GetInstance().GetSinkPortName(audioActiveDevice_.GetCurrentOutputDeviceType()));
        audioPolicyManager_.SuspendAudioDevice(currentActivePort, true);
        std::shared_ptr<AudioPipeManager> pipeManager = AudioPipeManager::GetPipeManager();
        uint32_t curPaIndex = pipeManager->GetPaIndexByIoHandle(activateDeviceIOHandle);
        std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs =
            pipeManager->GetStreamDescsByIoHandle(activateDeviceIOHandle);
        AUDIO_INFO_LOG("IoHandleId: %{public}u, paIndex: %{public}u, stream count: %{public}zu",
            activateDeviceIOHandle, curPaIndex, streamDescs.size());
        pipeManager->RemoveAudioPipeInfo(activateDeviceIOHandle);
        int32_t engineFlag = GetEngineFlag();
        if (engineFlag != 1) {
            audioPolicyManager_.CloseAudioPort(activateDeviceIOHandle, curPaIndex);
        }

        CHECK_AND_RETURN(RestoreNewA2dpPort(streamDescs, moduleInfo, currentActivePort) == SUCCESS);
        std::string portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
        if (!audioSceneManager_.IsVoiceCallRelatedScene()) {
            audioPolicyManager_.SetDeviceActive(deviceType, portName, true);
        }
        audioPolicyManager_.SuspendAudioDevice(portName, false);
        audioPolicyManager_.SuspendAudioDevice(currentActivePort, false);
        audioConnectedDevice_.UpdateConnectDevice(deviceType, macAddress, deviceName, streamInfo);
        break;
    }
}

void AudioDeviceStatus::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    AUDIO_INFO_LOG("[ADeviceEvent] device[%{public}d] currentOutputDevice[%{public}d] "
        "macAddress:[%{public}s], activeBTDevice:[%{public}s]",
        deviceType, audioActiveDevice_.GetCurrentOutputDeviceType(),
        GetEncryptAddr(macAddress).c_str(), GetEncryptAddr(btDevice).c_str());
    // only for the active a2dp device.
    if ((deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) && !macAddress.compare(btDevice)) {
        int32_t activeSessionsSize = 0;
        BluetoothOffloadState state = NO_A2DP_DEVICE;
        if (audioA2dpOffloadManager_) {
            activeSessionsSize = audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
            state = audioA2dpOffloadManager_->GetA2dpOffloadFlag();
        }
        AUDIO_DEBUG_LOG("streamInfo.sampleRate: %{public}d, a2dpOffloadFlag: %{public}d",
            streamInfo.samplingRate, state);
        if (!IsConfigurationUpdated(deviceType, streamInfo) ||
            (activeSessionsSize > 0 && state == A2DP_OFFLOAD)) {
            AUDIO_DEBUG_LOG("Audio configuration same");
            return;
        }
        audioA2dpDevice_.SetA2dpDeviceStreamInfo(macAddress, streamInfo);
        ReloadA2dpOffloadOnDeviceChanged(deviceType, macAddress, deviceName, streamInfo);
    } else if (audioA2dpDevice_.CheckA2dpDeviceExist(macAddress)) {
        AUDIO_DEBUG_LOG("Audio configuration update, macAddress:[%{public}s], streamInfo.sampleRate: %{public}d",
            GetEncryptAddr(macAddress).c_str(), streamInfo.samplingRate);
        audioA2dpDevice_.SetA2dpDeviceStreamInfo(macAddress, streamInfo);
    }
}

bool AudioDeviceStatus::IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AudioStreamInfo audioStreamInfo = {};
        if (audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(deviceType, audioStreamInfo)) {
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

DeviceType AudioDeviceStatus::GetDeviceTypeFromPin(AudioPin hdiPin)
{
    AUDIO_INFO_LOG("Pin: %{public}d", hdiPin);
    switch (hdiPin) {
        case OHOS::AudioStandard::AUDIO_PIN_NONE:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_SPEAKER:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_DAUDIO_DEFAULT:
            return DeviceType::DEVICE_TYPE_SPEAKER;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HEADSET:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_LINEOUT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HDMI:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB_HEADSET:
        case OHOS::AudioStandard::AUDIO_PIN_IN_USB_HEADSET:
            return DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
        case OHOS::AudioStandard::AUDIO_PIN_IN_MIC:
        case OHOS::AudioStandard::AUDIO_PIN_IN_DAUDIO_DEFAULT:
            return DeviceType::DEVICE_TYPE_MIC;
        case OHOS::AudioStandard::AUDIO_PIN_IN_PENCIL:
        case OHOS::AudioStandard::AUDIO_PIN_IN_UWB:
            return DeviceType::DEVICE_TYPE_ACCESSORY;
        case OHOS::AudioStandard::AUDIO_PIN_IN_HS_MIC:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_LINEIN:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_USB_EXT:
            break;
        default:
            break;
    }
    return DeviceType::DEVICE_TYPE_DEFAULT;
}

string AudioDeviceStatus::GetModuleNameByType(ClassType type)
{
    list<AudioModuleInfo> moduleList;
    bool ret = audioConfigManager_.GetModuleListByType(type, moduleList);
    CHECK_AND_RETURN_RET_LOG(ret && !moduleList.empty(), "", "Get module info of type[%{public}d] failed", type);
    return moduleList.front().name;
}

std::shared_ptr<AudioDeviceDescriptor> AudioDeviceStatus::GetDeviceByStatusInfo(const DStatusInfo &statusInfo)
{
    DeviceType devType = GetDeviceTypeFromPin(statusInfo.hdiPin);
    AudioDeviceDescriptor deviceDesc(devType, AudioPolicyUtils::GetInstance().GetDeviceRole(devType));
    deviceDesc.SetDeviceInfo(statusInfo.deviceName, statusInfo.macAddress);
    DeviceStreamInfo streamInfo = {};
    std::list<DeviceStreamInfo> streamInfoList = statusInfo.streamInfo.empty() ?
        std::list<DeviceStreamInfo>{ streamInfo } : statusInfo.streamInfo;
    deviceDesc.SetDeviceCapability(streamInfoList, 0);
    deviceDesc.networkId_ = statusInfo.networkId;
    return std::make_shared<AudioDeviceDescriptor>(deviceDesc);
}

void AudioDeviceStatus::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    AUDIO_WARNING_LOG("[ADeviceEvent] remote HDI_PIN[%{public}d] connet[%{public}d] "
        "networkId[%{public}s]", statusInfo.hdiPin, statusInfo.isConnected,
        GetEncryptStr(statusInfo.networkId).c_str());
    if (isStop) {
        std::shared_ptr<AudioDeviceDescriptor> device = GetDeviceByStatusInfo(statusInfo);
        AudioZoneService::GetInstance().MoveDeviceToGlobalFromZones(device);

        HandleOfflineDistributedDevice();
        audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
            audioActiveDevice_.GetCurrentOutputDevice(), "OnDeviceStatusUpdated 2.1 param");
        return;
    }
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    int32_t ret = HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "HandleDistributedDeviceUpdate return directly.");

    TriggerDeviceChangedCallback(descForCb, statusInfo.isConnected);
    TriggerAvailableDeviceChangedCallback(descForCb, statusInfo.isConnected);

    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("OnDeviceStatusUpdated_3", reason);
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("OnDeviceStatusUpdated_3");
    DeviceType devType = GetDeviceTypeFromPin(statusInfo.hdiPin);
    DeviceRole deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(devType);
    if (!statusInfo.isConnected) {
        std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(statusInfo.networkId, deviceRole);
        audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleName);
    }
    if (deviceRole == DeviceRole::INPUT_DEVICE) {
        remoteCapturerSwitch_ = true;
    }

    // update a2dp offload
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "OnDeviceStatusUpdated 2.2 param");
}

int32_t AudioDeviceStatus::ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote)
{
    if (isRemote) {
        AudioModuleInfo moduleInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
            AudioPolicyUtils::GetInstance().GetDeviceRole(deviceType), deviceType);
        std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId,
            AudioPolicyUtils::GetInstance().GetDeviceRole(deviceType));
        AUDIO_INFO_LOG("Module name: %{public}s, adapter name: %{public}s",
            moduleName.c_str(), moduleInfo.adapterName.c_str());
        uint32_t paIndex = 0;
        AudioIOHandle ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
        CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
            "OpenAudioPort failed ioHandle[%{public}u]", ioHandle);
        CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
            "OpenAudioPort failed paId[%{public}u]", paIndex);
        std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
        pipeInfo->id_ = ioHandle;
        pipeInfo->paIndex_ = paIndex;
        if (moduleInfo.role == "sink") {
            pipeInfo->name_ = "distributed_output";
            pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
            pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        } else {
            pipeInfo->name_ = "distributed_input";
            pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
            pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        }
        pipeInfo->adapterName_ = moduleInfo.adapterName;
        pipeInfo->moduleInfo_ = moduleInfo;
        pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
        pipeInfo->InitAudioStreamInfo();
        AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);
        audioIOHandleMap_.AddIOHandleInfo(moduleName, ioHandle);
    }
    return SUCCESS;
}

int32_t AudioDeviceStatus::HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb, AudioStreamDeviceChangeReasonExt &reason)
{
    DeviceType devType = GetDeviceTypeFromPin(statusInfo.hdiPin);
    DeviceRole devRole = AudioPolicyUtils::GetInstance().GetDeviceRole(devType);
    const std::string networkId = statusInfo.networkId;
    AudioDeviceDescriptor deviceDesc(devType, devRole);
    deviceDesc.SetDeviceInfo(statusInfo.deviceName, statusInfo.macAddress);
    DeviceStreamInfo streamInfo = {};
    std::list<DeviceStreamInfo> streamInfoList = statusInfo.streamInfo.empty() ?
        std::list<DeviceStreamInfo>{ streamInfo } : statusInfo.streamInfo;
    deviceDesc.SetDeviceCapability(streamInfoList, 0);
    deviceDesc.networkId_ = networkId;
    audioVolumeManager_.UpdateGroupInfo(VOLUME_TYPE, GROUP_NAME_DEFAULT, deviceDesc.volumeGroupId_, networkId,
        statusInfo.isConnected, statusInfo.mappingVolumeId);
    audioVolumeManager_.UpdateGroupInfo(INTERRUPT_TYPE, GROUP_NAME_DEFAULT, deviceDesc.interruptGroupId_, networkId,
        statusInfo.isConnected, statusInfo.mappingInterruptId);
    if (statusInfo.isConnected) {
        if (audioConnectedDevice_.GetConnectedDeviceByType(networkId, devType) != nullptr) {
            return ERROR;
        }
        int32_t ret = ActivateNewDevice(statusInfo.networkId, devType,
            statusInfo.connectType == ConnectType::CONNECT_TYPE_DISTRIBUTED);
        CheckAndWriteDeviceChangeExceptionEvent(ret == SUCCESS,
            AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE, devType, devRole, ret,
            "DEVICE online but open audio device failed.");
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "DEVICE online but open audio device failed.");
        audioDeviceCommon_.UpdateConnectedDevicesWhenConnecting(deviceDesc, descForCb);

        if (statusInfo.connectType == ConnectType::CONNECT_TYPE_DISTRIBUTED) {
            AudioServerProxy::GetInstance().NotifyDeviceInfoProxy(networkId, true);
        }
    } else {
        std::shared_ptr<AudioDeviceDescriptor> device = GetDeviceByStatusInfo(statusInfo);
        AudioZoneService::GetInstance().MoveDeviceToGlobalFromZones(device);
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(deviceDesc, descForCb);
        reason = AudioStreamDeviceChangeReasonExt::ExtEnum::DISTRIBUTED_DEVICE_UNAVAILABLE;
        std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId,
            AudioPolicyUtils::GetInstance().GetDeviceRole(devType));
        std::string currentActivePort = REMOTE_CLASS;
        audioPolicyManager_.SuspendAudioDevice(currentActivePort, true);
        audioRouteMap_.RemoveDeviceInRouterMap(moduleName);
        audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);
    }
    return SUCCESS;
}

void AudioDeviceStatus::AddEarpiece()
{
    if (!audioConfigManager_.GetHasEarpiece()) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE);
    CHECK_AND_RETURN_LOG(audioDescriptor != nullptr, "Create earpiect device descriptor failed");

    // Use speaker streaminfo for earpiece cap
    auto itr = audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_SPEAKER);
    if (itr != nullptr) {
        audioDescriptor->SetDeviceCapability(itr->audioStreamInfo_, 0);
    }
    audioDescriptor->deviceId_ = AudioPolicyUtils::startDeviceId++;
    AudioPolicyUtils::GetInstance().UpdateDisplayName(audioDescriptor);
    audioDeviceManager_.AddNewDevice(audioDescriptor);
    audioConnectedDevice_.AddConnectedDevice(audioDescriptor);
    AUDIO_INFO_LOG("Add earpiece to device list");
}

bool AudioDeviceStatus::OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo)
{
    auto devType = AudioPolicyUtils::GetInstance().GetDeviceType(moduleInfo.name);
    if (devType != DEVICE_TYPE_MIC) {
        audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);

        if (devType == DEVICE_TYPE_SPEAKER) {
            auto result = audioPolicyManager_.SetDeviceActive(devType, moduleInfo.name, true);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, false, "[module_load]::Device failed %{public}d", devType);
        }
    }

    if (devType == DEVICE_TYPE_MIC) {
        audioEcManager_.SetPrimaryMicModuleInfo(moduleInfo);
    }

    if (devType == DEVICE_TYPE_SPEAKER || devType == DEVICE_TYPE_MIC) {
        AddAudioDevice(moduleInfo, devType);
    }

    audioVolumeManager_.NotifyVolumeGroup();

    return true;
}

void AudioDeviceStatus::AddAudioDevice(AudioModuleInfo& moduleInfo, DeviceType devType)
{
    // add new device into active device list
    std::string volumeGroupName = audioConfigManager_.GetGroupName(moduleInfo.name, VOLUME_TYPE);
    std::string interruptGroupName = audioConfigManager_.GetGroupName(moduleInfo.name, INTERRUPT_TYPE);
    int32_t volumeGroupId = GROUP_ID_NONE;
    int32_t interruptGroupId = GROUP_ID_NONE;
    audioVolumeManager_.UpdateGroupInfo(GroupType::VOLUME_TYPE, volumeGroupName, volumeGroupId, LOCAL_NETWORK_ID, true,
        NO_REMOTE_ID);
    audioVolumeManager_.UpdateGroupInfo(GroupType::INTERRUPT_TYPE, interruptGroupName, interruptGroupId,
        LOCAL_NETWORK_ID, true, NO_REMOTE_ID);

    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>(devType,
        AudioPolicyUtils::GetInstance().GetDeviceRole(moduleInfo.role), volumeGroupId, interruptGroupId,
        LOCAL_NETWORK_ID);
    CHECK_AND_RETURN_LOG(audioDescriptor != nullptr, "audioDescriptor is nullptr.");
    if (!moduleInfo.supportedRate_.empty() && !moduleInfo.supportedChannelLayout_.empty()) {
        DeviceStreamInfo streamInfo = {};
        for (auto supportedRate : moduleInfo.supportedRate_) {
            streamInfo.samplingRate.insert(static_cast<AudioSamplingRate>(supportedRate));
        }
        for (auto supportedChannelLayout : moduleInfo.supportedChannelLayout_) {
            streamInfo.channelLayout.insert(static_cast<AudioChannelLayout>(supportedChannelLayout));
        }
        audioDescriptor->SetDeviceCapability({ streamInfo }, 0);
    }

    audioDescriptor->deviceId_ = AudioPolicyUtils::startDeviceId++;
    AudioPolicyUtils::GetInstance().UpdateDisplayName(audioDescriptor);
    audioDeviceManager_.AddNewDevice(audioDescriptor);
    audioConnectedDevice_.AddConnectedDevice(audioDescriptor);
    audioMicrophoneDescriptor_.AddMicrophoneDescriptor(audioDescriptor);
}

int32_t AudioDeviceStatus::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    int32_t result = ERROR;
    AUDIO_DEBUG_LOG("[module_load]::HDI and AUDIO SERVICE is READY. Loading default modules");
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);
    for (const auto &device : deviceClassInfo) {
        if (device.first != ClassType::TYPE_PRIMARY && device.first != ClassType::TYPE_FILE_IO) {
            continue;
        }
        if (device.first == ClassType::TYPE_PRIMARY) {
            AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary");
        }
        auto moduleInfoList = device.second;
        for (auto &moduleInfo : moduleInfoList) {
            AUDIO_INFO_LOG("[module_load]::Load module[%{public}s]", moduleInfo.name.c_str());
            uint32_t sinkLatencyInMsec = audioConfigManager_.GetSinkLatencyFromXml();
            moduleInfo.sinkLatency = sinkLatencyInMsec != 0 ? to_string(sinkLatencyInMsec) : "";
            if (OpenPortAndAddDeviceOnServiceConnected(moduleInfo)) {
                result = SUCCESS;
            }
            audioOffloadStream_.SetOffloadAvailableFromXML(moduleInfo);
        }
    }

    if (result == SUCCESS) {
        AUDIO_INFO_LOG("[module_load]::Setting speaker as active device on bootup");
        hasModulesLoaded = true;
        shared_ptr<AudioDeviceDescriptor> outDevice = audioDeviceManager_.GetRenderDefaultDevice();
        audioActiveDevice_.SetCurrentOutputDevice(*outDevice);
        shared_ptr<AudioDeviceDescriptor> inDevice = audioDeviceManager_.GetCaptureDefaultDevice();
        audioActiveDevice_.SetCurrentInputDevice(*inDevice);
        AudioDeviceDescriptor curDevice = audioActiveDevice_.GetCurrentOutputDevice();
        audioVolumeManager_.SetVolumeForSwitchDevice(curDevice);
        OnPreferredDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(),
            audioActiveDevice_.GetCurrentInputDeviceType());
        AddEarpiece();
        for (auto it = pnpDeviceList_.begin(); it != pnpDeviceList_.end(); ++it) {
            OnPnpDeviceStatusUpdated((*it).first, (*it).second);
        }
    }
    return result;
}

void AudioDeviceStatus::OnPreferredDeviceUpdated(const AudioDeviceDescriptor& activeOutputDevice,
    DeviceType activeInputDevice)
{
    audioDeviceCommon_.OnPreferredOutputDeviceUpdated(activeOutputDevice,
        AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioDeviceCommon_.OnPreferredInputDeviceUpdated(activeInputDevice, LOCAL_NETWORK_ID);
}

void AudioDeviceStatus::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    if (macAddress.empty()) {
        AUDIO_ERR_LOG("failed as the macAddress is empty!");
        return;
    }
    AUDIO_INFO_LOG("[ADeviceEvent] bt select device type[%{public}d] address[%{public}s]",
        devType, GetEncryptAddr(macAddress).c_str());
    std::vector<shared_ptr<AudioDeviceDescriptor>> bluetoothDevices =
        audioDeviceManager_.GetAvailableBluetoothDevice(devType, macAddress);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    for (const auto &dec : bluetoothDevices) {
        if (dec->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
            std::shared_ptr<AudioDeviceDescriptor> tempDec = std::make_shared<AudioDeviceDescriptor>(*dec);
            audioDeviceDescriptors.push_back(move(tempDec));
        }
    }
    int32_t res = audioDeviceCommon_.DeviceParamsCheck(DeviceRole::OUTPUT_DEVICE, audioDeviceDescriptors);
    CHECK_AND_RETURN_LOG(res == SUCCESS, "DeviceParamsCheck no success");
    audioDeviceDescriptors[0]->isEnable_ = true;
    audioDeviceManager_.UpdateDevicesListInfo(audioDeviceDescriptors[0], ENABLE_UPDATE);
    if (devType == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER, audioDeviceDescriptors[0], SYSTEM_UID,
            "OnForcedDeviceSelected");
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(audioDeviceDescriptors[0]->macAddress_);
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, audioDeviceDescriptors[0]);
    }
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("OnForcedDeviceSelected",
        AudioStreamDeviceChangeReason::OVERRODE);
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "OnForcedDeviceSelected");
}

void AudioDeviceStatus::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, DeviceType devType,
    std::string macAddress, std::string deviceName, bool isActualConnection, AudioStreamInfo streamInfo,
    bool isConnected)
{
    AUDIO_WARNING_LOG("[ADeviceEvent] bt device[%{public}d] mac[%{public}s] connect[%{public}d]",
        devType, GetEncryptStr(macAddress).c_str(), isConnected);

    auto devDesc = make_shared<AudioDeviceDescriptor>(updatedDesc);
    if (!isActualConnection && audioDeviceManager_.IsConnectedDevices(devDesc)) {
        audioDeviceManager_.UpdateVirtualDevices(devDesc, isConnected);
        return;
    }

    AudioServerProxy::GetInstance().SetDmDeviceTypeProxy(isConnected ? updatedDesc.dmDeviceType_ : 0,
        updatedDesc.deviceType_);

    UpdateLocalGroupInfo(isConnected, macAddress, deviceName, streamInfo, updatedDesc);
    // fill device change action for callback
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    updatedDesc.spatializationSupported_ = (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP)
        && AudioSpatializationService::GetAudioSpatializationService().
        IsSpatializationSupportedForDevice(updatedDesc.macAddress_)
        && AudioSpatializationService::GetAudioSpatializationService().IsSpatializationSupported();
    UpdateDeviceList(updatedDesc, isConnected, descForCb, reason);

    TriggerDeviceChangedCallback(descForCb, isConnected);
    TriggerAvailableDeviceChangedCallback(descForCb, isConnected);

    if (!isActualConnection) {
        return;
    }
    // fetch input&output device
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("OnDeviceStatusUpdated_4", reason);
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("OnDeviceStatusUpdated_4");
    // update a2dp offload
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP && audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "OnDeviceStatusUpdated 2 param");
}

void AudioDeviceStatus::UpdateDeviceList(AudioDeviceDescriptor &updatedDesc,  bool isConnected,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb,
    AudioStreamDeviceChangeReasonExt &reason)
{
    if (isConnected) {
        // deduplicate
        audioConnectedDevice_.DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_,
            updatedDesc.macAddress_, updatedDesc.deviceRole_);
        audioDeviceCommon_.UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);
        int32_t result = HandleLocalDeviceConnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Connect local device failed.");
        reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    } else {
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
        CheckForA2dpSuspend(updatedDesc);
        // fix pop, fetch device before unload module
        if (IsOutputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
            AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("UpdateDeviceList", reason);
        }
        if (IsInputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
            AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("UpdateDeviceList");
        }
        int32_t result = HandleLocalDeviceDisconnected(updatedDesc);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Disconnect local device failed.");
        reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    }
    AUDIO_INFO_LOG("Device: %{public}d, isConnected: %{public}d", updatedDesc.deviceType_, isConnected);
}

#ifdef BLUETOOTH_ENABLE
void AudioDeviceStatus::CheckAndActiveHfpDevice(AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && !audioDeviceManager_.GetScoState()) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc.macAddress_);
    }
}
#endif

void AudioDeviceStatus::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    AUDIO_WARNING_LOG("[ADeviceEvent] bt [%{public}s] type[%{public}d] command: %{public}d category[%{public}d] " \
        "connectState[%{public}d] isEnable[%{public}d] deviceUsage[%{public}d]",
        GetEncryptAddr(desc.macAddress_).c_str(), desc.deviceType_, command, desc.deviceCategory_,
        desc.connectState_, desc.isEnable_, desc.deviceUsage_);
    std::string portNeedClose = "";
    uint32_t oldPaIndex = OPEN_PORT_FAILURE;
    if (command == ENABLE_UPDATE && desc.isEnable_ == true) {
        if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(desc.macAddress_);
        }
        shared_ptr<AudioDeviceDescriptor> userSelectMediaDevice =
            AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice();
        shared_ptr<AudioDeviceDescriptor> userSelectCallDevice =
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
        audioActiveDevice_.GetCurrentOutputDeviceMacAddr() == desc.macAddress_) {
        audioIOHandleMap_.MuteDefaultSinkPort(audioActiveDevice_.GetCurrentOutputDeviceNetworkId(),
            AudioPolicyUtils::GetInstance().GetSinkPortName(audioActiveDevice_.GetCurrentOutputDeviceType()));
        portNeedClose = BLUETOOTH_SPEAKER;
        oldPaIndex = GetPaIndexByPortName(portNeedClose);
    }
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>(desc);
    reason = audioDeviceManager_.UpdateDevicesListInfo(audioDescriptor, command);
    CheckForA2dpSuspend(desc);

    OnPreferredStateUpdated(desc, command, reason);
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("OnDeviceInfoUpdated", reason);
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("OnDeviceInfoUpdated");
    if (portNeedClose != "" && oldPaIndex == GetPaIndexByPortName(portNeedClose)) {
        audioIOHandleMap_.ClosePortAndEraseIOHandle(portNeedClose);
    }
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "OnDeviceInfoUpdated");
}

void AudioDeviceStatus::CheckForA2dpSuspend(AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
        return;
    }
    if (audioDeviceManager_.GetScoState()) {
        AudioServerProxy::GetInstance().SuspendRenderSinkProxy("a2dp");
    } else {
        AudioServerProxy::GetInstance().RestoreRenderSinkProxy("a2dp");
    }
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceStatus::UserSelectDeviceMapInit()
{
    AudioStateManager& stateManager = AudioStateManager::GetAudioStateManager();
    shared_ptr<AudioDeviceDescriptor> userSelectMediaRenderDevice = stateManager.GetPreferredMediaRenderDevice();
    shared_ptr<AudioDeviceDescriptor> userSelectCallRenderDevice = stateManager.GetPreferredCallRenderDevice();
    shared_ptr<AudioDeviceDescriptor> userSelectCallCaptureDevice = stateManager.GetPreferredCallCaptureDevice();
    shared_ptr<AudioDeviceDescriptor> userSelectRecordCaptureDevice = stateManager.GetPreferredRecordCaptureDevice();
    vector<shared_ptr<AudioDeviceDescriptor>> userSelectDeviceMap;
    userSelectDeviceMap.push_back(make_shared<AudioDeviceDescriptor>(*userSelectMediaRenderDevice));
    userSelectDeviceMap.push_back(make_shared<AudioDeviceDescriptor>(*userSelectCallRenderDevice));
    userSelectDeviceMap.push_back(make_shared<AudioDeviceDescriptor>(*userSelectCallCaptureDevice));
    userSelectDeviceMap.push_back(make_shared<AudioDeviceDescriptor>(*userSelectRecordCaptureDevice));
    return userSelectDeviceMap;
}

void AudioDeviceStatus::DeactivateNearlinkDevice(AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ == DEVICE_TYPE_NEARLINK || desc.deviceType_ == DEVICE_TYPE_NEARLINK_IN) {
        if (desc.macAddress_ == audioActiveDevice_.GetCurrentOutputDeviceMacAddr()) {
            SleAudioDeviceManager::GetInstance().SetActiveDevice(desc, STREAM_USAGE_INVALID);
        }
    }
}

void AudioDeviceStatus::OnPreferredStateUpdated(AudioDeviceDescriptor &desc,
    const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason)
{
    vector<shared_ptr<AudioDeviceDescriptor>> userSelectDeviceMap = UserSelectDeviceMapInit();
    if (updateCommand == CATEGORY_UPDATE) {
        if (desc.deviceCategory_ == BT_UNWEAR_HEADPHONE) {
            reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
            UpdateAllUserSelectDevice(userSelectDeviceMap, desc, std::make_shared<AudioDeviceDescriptor>());
#ifdef BLUETOOTH_ENABLE
            if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc.macAddress_ == audioActiveDevice_.GetCurrentOutputDeviceMacAddr()) {
                Bluetooth::AudioA2dpManager::SetActiveA2dpDevice("");
            }
#endif
            // Handle Nearlink Device
            DeactivateNearlinkDevice(desc);
        } else {
            reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
            if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP || desc.deviceType_ == DEVICE_TYPE_NEARLINK) {
                AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER,
                    std::make_shared<AudioDeviceDescriptor>());
                AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE,
                    std::make_shared<AudioDeviceDescriptor>());
            }
            if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO || desc.deviceType_ == DEVICE_TYPE_NEARLINK) {
                AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                    std::make_shared<AudioDeviceDescriptor>(), CLEAR_UID, "OnPreferredStateUpdated");
                AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE,
                    std::make_shared<AudioDeviceDescriptor>());
                AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(desc.macAddress_);
#ifdef BLUETOOTH_ENABLE
                CheckAndActiveHfpDevice(desc);
#endif
            }
        }
    } else if (updateCommand == ENABLE_UPDATE) {
        UpdateAllUserSelectDevice(userSelectDeviceMap, desc, std::make_shared<AudioDeviceDescriptor>(desc));
        reason = desc.isEnable_ ? AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE :
            AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    } else if (updateCommand == USAGE_UPDATE) {
        UpdateAllUserSelectDevice(userSelectDeviceMap, desc, std::make_shared<AudioDeviceDescriptor>(desc));
    }
}

void AudioDeviceStatus::UpdateAllUserSelectDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &userSelectDeviceMap,
    AudioDeviceDescriptor &desc, const std::shared_ptr<AudioDeviceDescriptor> &selectDesc)
{
    if (userSelectDeviceMap[MEDIA_RENDER_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[MEDIA_RENDER_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[MEDIA_RENDER_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, selectDesc);
        } else {
            audioStateManager_.UpdatePreferredMediaRenderDeviceConnectState(desc.connectState_);
        }
    }
    if (userSelectDeviceMap[CALL_RENDER_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[CALL_RENDER_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[CALL_RENDER_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                selectDesc, SYSTEM_UID, "UpdateAllUserSelectDevice");
        } else {
            audioStateManager_.UpdatePreferredCallRenderDeviceConnectState(desc.connectState_);
        }
    }
    if (userSelectDeviceMap[CALL_CAPTURE_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[CALL_CAPTURE_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[CALL_CAPTURE_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE, selectDesc);
        } else {
            audioStateManager_.UpdatePreferredCallCaptureDeviceConnectState(desc.connectState_);
        }
    }
    if (userSelectDeviceMap[RECORD_CAPTURE_ID]->deviceType_ == desc.deviceType_ &&
        userSelectDeviceMap[RECORD_CAPTURE_ID]->macAddress_ == desc.macAddress_) {
        if (userSelectDeviceMap[RECORD_CAPTURE_ID]->connectState_ != VIRTUAL_CONNECTED) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE, selectDesc);
        } else {
            audioStateManager_.UpdatePreferredRecordCaptureDeviceConnectState(desc.connectState_);
        }
    }
}

void AudioDeviceStatus::RemoveDeviceFromGlobalOnly(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
    AUDIO_INFO_LOG("remove device from global list only");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(desc, descForCb);
    TriggerDeviceChangedCallback(descForCb, false);
    TriggerAvailableDeviceChangedCallback(descForCb, false);
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("RemoveDeviceFromGlobalOnly");
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("RemoveDeviceFromGlobalOnly");
}

void AudioDeviceStatus::AddDeviceBackToGlobalOnly(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
    AUDIO_INFO_LOG("add device back to global list only");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    audioDeviceCommon_.UpdateConnectedDevicesWhenConnecting(desc, descForCb);
    TriggerDeviceChangedCallback(descForCb, true);
    TriggerAvailableDeviceChangedCallback(descForCb, true);
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("AddDeviceBackToGlobalOnly");
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("AddDeviceBackToGlobalOnly");
}

void AudioDeviceStatus::HandleOfflineDistributedDevice()
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceChangeDescriptor = {};

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectedDevices = audioConnectedDevice_.GetCopy();
    std::vector<std::string> modulesNeedClose = {};
    for (auto deviceDesc : connectedDevices) {
        if (deviceDesc != nullptr && deviceDesc->networkId_ != LOCAL_NETWORK_ID) {
            const std::string networkId = deviceDesc->networkId_;
            audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(deviceDesc, deviceChangeDescriptor);
            std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId,
                AudioPolicyUtils::GetInstance().GetDeviceRole(deviceDesc->deviceType_));
            audioIOHandleMap_.MuteDefaultSinkPort(audioActiveDevice_.GetCurrentOutputDeviceNetworkId(),
                AudioPolicyUtils::GetInstance().GetSinkPortName(audioActiveDevice_.GetCurrentOutputDeviceType()));
            modulesNeedClose.push_back(moduleName);
            audioRouteMap_.RemoveDeviceInRouterMap(moduleName);
            audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);
            if (AudioPolicyUtils::GetInstance().GetDeviceRole(deviceDesc->deviceType_) == DeviceRole::INPUT_DEVICE) {
                remoteCapturerSwitch_ = true;
            }
        }
    }

    TriggerDeviceChangedCallback(deviceChangeDescriptor, false);
    TriggerAvailableDeviceChangedCallback(deviceChangeDescriptor, false);
    AUDIO_INFO_LOG("onDeviceStatusUpdated reson:%{public}d",
        AudioStreamDeviceChangeReasonExt::ExtEnum::DISTRIBUTED_DEVICE_UNAVAILABLE);
    AudioCoreService::GetCoreService()->FetchOutputDeviceAndRoute("HandleOfflineDistributedDevice",
        AudioStreamDeviceChangeReasonExt::ExtEnum::DISTRIBUTED_DEVICE_UNAVAILABLE);
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("HandleOfflineDistributedDevice");
    for (auto &moduleName : modulesNeedClose) {
        audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleName);
    }
}

uint16_t AudioDeviceStatus::GetDmDeviceType()
{
    return dmDeviceType_;
}

int32_t AudioDeviceStatus::RestoreNewA2dpPort(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
    AudioModuleInfo &moduleInfo, std::string &currentActivePort)
{
    // Load bt sink module again with new configuration
    AUDIO_INFO_LOG("Reload a2dp module [%{public}s]", moduleInfo.name.c_str());
    uint32_t paIndex;
    AudioIOHandle ioHandle;
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        ioHandle = audioPolicyManager_.ReloadAudioPort(moduleInfo, paIndex);
    } else {
        ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
    }
    if (ioHandle == HDI_INVALID_ID || paIndex == OPEN_PORT_FAILURE) {
        audioPolicyManager_.SuspendAudioDevice(currentActivePort, false);
        AUDIO_ERR_LOG("AudioPort failed, ioHandle: %{public}u, paIndex: %{public}u", ioHandle, paIndex);
        return ERROR;
    }
    audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    if (moduleInfo.role == "sink") {
        pipeInfo->name_ = "a2dp_output";
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    } else {
        pipeInfo->name_ = "a2dp_input";
        pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
        pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    }
    pipeInfo->adapterName_ = "a2dp";
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->InitAudioStreamInfo();
    pipeInfo->streamDescriptors_.insert(pipeInfo->streamDescriptors_.end(), streamDescs.begin(), streamDescs.end());
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);
    return SUCCESS;
}

uint32_t AudioDeviceStatus::GetPaIndexByPortName(std::string &portName)
{
    AudioIOHandle ioHandle;
    CHECK_AND_RETURN_RET_LOG(audioIOHandleMap_.GetModuleIdByKey(portName, ioHandle), OPEN_PORT_FAILURE,
        "can not find %{public}s in io map", portName.c_str());
    std::shared_ptr<AudioPipeManager> pipeManager = AudioPipeManager::GetPipeManager();
    uint32_t paIndex = pipeManager->GetPaIndexByIoHandle(ioHandle);
    AUDIO_INFO_LOG("Port %{public}s, paIndex: %{public}u", portName.c_str(), paIndex);
    return paIndex;
}
}
}
