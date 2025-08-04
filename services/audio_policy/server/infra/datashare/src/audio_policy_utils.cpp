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
#define LOG_TAG "AudioPolicyUtils"
#endif

#include "audio_policy_utils.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_policy_manager_factory.h"
#include "device_init_callback.h"
#include "audio_recovery_device.h"

#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t NS_PER_MS = 1000000;
static constexpr int32_t MS_PER_S = 1000;
static const char* SETTINGS_DATA_FIELD_VALUE = "VALUE";
static const char* SETTINGS_DATA_FIELD_KEYWORD = "KEYWORD";
static const char* PREDICATES_STRING = "settings.general.device_name";
static const char* SETTINGS_DATA_BASE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static const char* SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
static const char* AUDIO_SERVICE_PKG = "audio_manager_service";

std::map<std::string, ClassType> AudioPolicyUtils::portStrToEnum = {
    {PRIMARY_SPEAKER, TYPE_PRIMARY},
    {PRIMARY_MIC, TYPE_PRIMARY},
    {PRIMARY_WAKEUP_MIC, TYPE_PRIMARY},
    {BLUETOOTH_SPEAKER, TYPE_A2DP},
    {BLUETOOTH_MIC, TYPE_A2DP},
    {USB_SPEAKER, TYPE_USB},
    {USB_MIC, TYPE_USB},
    {DP_SINK, TYPE_DP},
    {FILE_SINK, TYPE_FILE_IO},
    {FILE_SOURCE, TYPE_FILE_IO},
    {REMOTE_CLASS, TYPE_REMOTE_AUDIO},
};

int32_t AudioPolicyUtils::startDeviceId = 1;

std::string AudioPolicyUtils::GetEncryptAddr(const std::string &addr)
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

void AudioPolicyUtils::WriteServiceStartupError(std::string reason)
{
    Trace trace("SYSEVENT FAULT EVENT AUDIO_SERVICE_STARTUP_ERROR, SERVICE_ID: "
        + std::to_string(Media::MediaMonitor::AUDIO_POLICY_SERVICE_ID) +
        ", ERROR_CODE: " + std::to_string(Media::MediaMonitor::AUDIO_POLICY_SERVER));
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_SERVICE_STARTUP_ERROR,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("SERVICE_ID", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVICE_ID));
    bean->Add("ERROR_CODE", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVER));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioPolicyUtils::WriteDeviceChangeExceptionEvent(const AudioStreamDeviceChangeReason reason,
    DeviceType deviceType, DeviceRole deviceRole, int32_t errorMsg, const std::string &errorDesc)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::DEVICE_CHANGE_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);

    std::string fullErrorDesc = "DeviceRole:" + std::to_string(static_cast<uint32_t>(deviceRole)) +
        ", errorDesc:" + errorDesc;
    bean->Add("CHANGE", static_cast<int32_t>(reason));
    bean->Add("DEVICE_TYPE", static_cast<int32_t>(deviceType));
    bean->Add("ERROR_CASE", 0);
    bean->Add("ERROR_MSG", errorMsg);
    bean->Add("ERROR_DESCRIPTION", fullErrorDesc);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

std::string AudioPolicyUtils::GetRemoteModuleName(std::string networkId, DeviceRole role)
{
    return networkId + (role == DeviceRole::OUTPUT_DEVICE ? "_out" : "_in");
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyUtils::GetAvailableDevicesInner(AudioDeviceUsage usage)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    audioDeviceDescriptors = audioDeviceManager_.GetAvailableDevicesByUsage(usage);
    return audioDeviceDescriptors;
}

int32_t AudioPolicyUtils::SetPreferredDevice(const PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid, const std::string caller)
{
    if (desc == nullptr) {
        AUDIO_ERR_LOG("desc is null");
        return ERR_INVALID_PARAM;
    }
    int32_t ret = SUCCESS;
    switch (preferredType) {
        case AUDIO_MEDIA_RENDER:
            audioStateManager_.SetPreferredMediaRenderDevice(desc);
            break;
        case AUDIO_CALL_RENDER:
            audioStateManager_.SetPreferredCallRenderDevice(desc, uid, caller);
            break;
        case AUDIO_CALL_CAPTURE:
            audioStateManager_.SetPreferredCallCaptureDevice(desc);
            break;
        case AUDIO_RECORD_CAPTURE:
            audioStateManager_.SetPreferredRecordCaptureDevice(desc);
            break;
        case AUDIO_RING_RENDER:
        case AUDIO_TONE_RENDER:
            AUDIO_WARNING_LOG("preferredType:%{public}d, not supported", preferredType);
            ret = ERR_INVALID_PARAM;
            break;
        default:
            AUDIO_ERR_LOG("invalid preferredType: %{public}d", preferredType);
            ret = ERR_INVALID_PARAM;
            break;
    }
    if (desc->deviceType_ == DEVICE_TYPE_NONE) {
        ErasePreferredDeviceByType(preferredType);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set preferredType %{public}d failed, ret: %{public}d", preferredType, ret);
    }
    return ret;
}

int32_t AudioPolicyUtils::ErasePreferredDeviceByType(const PreferredType preferredType)
{
    if (isBTReconnecting_) {
        return SUCCESS;
    }
    auto type = static_cast<Media::MediaMonitor::PreferredType>(preferredType);
    int32_t ret = Media::MediaMonitor::MediaMonitorManager::GetInstance().ErasePreferredDeviceByType(type);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Erase preferredType %{public}d failed, ret: %{public}d", preferredType, ret);
        return ERROR;
    }
    return SUCCESS;
}

void AudioPolicyUtils::SetBtConnecting(bool flag)
{
    isBTReconnecting_ = flag;
}

void AudioPolicyUtils::ClearScoDeviceSuspendState(std::string macAddress)
{
    AUDIO_DEBUG_LOG("Clear sco suspend state %{public}s", GetEncryptAddr(macAddress).c_str());
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetDevicesByFilter(
        DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_ROLE_NONE, macAddress, "", SUSPEND_CONNECTED);
    for (const auto &desc : descs) {
        desc->connectState_ = DEACTIVE_CONNECTED;
    }
}

int64_t AudioPolicyUtils::GetCurrentTimeMS()
{
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

uint32_t AudioPolicyUtils::PcmFormatToBytes(AudioSampleFormat format)
{
    // AudioSampleFormat / PCM_8_BIT
    switch (format) {
        case SAMPLE_U8:
            return 1; // 1 byte
        case SAMPLE_S16LE:
            return 2; // 2 byte
        case SAMPLE_S24LE:
            return 3; // 3 byte
        case SAMPLE_S32LE:
            return 4; // 4 byte
        case SAMPLE_F32LE:
            return 4; // 4 byte
        default:
            return 2; // 2 byte
    }
}

std::string AudioPolicyUtils::GetNewSinkPortName(DeviceType deviceType)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
            portName = USB_SPEAKER;
            break;
        case DeviceType::DEVICE_TYPE_DP:
            portName = DP_SINK;
            break;
        case DeviceType::DEVICE_TYPE_FILE_SINK:
            portName = FILE_SINK;
            break;
        case DeviceType::DEVICE_TYPE_REMOTE_CAST:
            portName = REMOTE_CAST_INNER_CAPTURER_SINK_NAME;
            break;
        case DeviceType::DEVICE_TYPE_ACCESSORY:
            portName = ACCESSORY_SOURCE;
            break;
        case DeviceType::DEVICE_TYPE_HEARING_AID:
            portName = HEARING_AID_SPEAKER;
            break;
        default:
            portName = PORT_NONE;
            break;
    }
    return portName;
}

std::string AudioPolicyUtils::GetSinkPortName(DeviceType deviceType, AudioPipeType pipeType)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            // BTH tells us that a2dpoffload is OK
            if (audioA2dpOffloadFlag_.GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
                if (pipeType == PIPE_TYPE_OFFLOAD) {
                    portName = OFFLOAD_PRIMARY_SPEAKER;
                } else if (pipeType == PIPE_TYPE_MULTICHANNEL) {
                    portName = MCH_PRIMARY_SPEAKER;
                } else {
                    portName = PRIMARY_SPEAKER;
                }
            } else {
                portName = BLUETOOTH_SPEAKER;
            }
            break;
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_NEARLINK:
            if (pipeType == PIPE_TYPE_OFFLOAD) {
                portName = OFFLOAD_PRIMARY_SPEAKER;
            } else if (pipeType == PIPE_TYPE_MULTICHANNEL) {
                portName = MCH_PRIMARY_SPEAKER;
            } else {
                portName = PRIMARY_SPEAKER;
            }
            break;
        case DeviceType::DEVICE_TYPE_HDMI:
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
            portName = AudioPolicyConfigManager::GetInstance().GetDefaultAdapterEnable() ? DP_SINK : PRIMARY_SPEAKER;
            break;
        default:
            portName = GetNewSinkPortName(deviceType);
            break;
    }

    return portName;
}

string AudioPolicyUtils::ConvertToHDIAudioFormat(AudioSampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case SAMPLE_U8:
            return "u8";
        case SAMPLE_S16LE:
            return "s16le";
        case SAMPLE_S24LE:
            return "s24le";
        case SAMPLE_S32LE:
            return "s32le";
        default:
            return "";
    }
}

std::string AudioPolicyUtils::GetSinkName(const AudioDeviceDescriptor &desc, int32_t sessionId)
{
    if (desc.networkId_ == LOCAL_NETWORK_ID) {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(sessionId, pipeType);
        return GetSinkPortName(desc.deviceType_, pipeType);
    } else {
        return GetRemoteModuleName(desc.networkId_, desc.deviceRole_);
    }
}

std::string AudioPolicyUtils::GetSinkName(std::shared_ptr<AudioDeviceDescriptor> desc, int32_t sessionId)
{
    if (desc->networkId_ == LOCAL_NETWORK_ID) {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(sessionId, pipeType);
        return GetSinkPortName(desc->deviceType_, pipeType);
    } else {
        return GetRemoteModuleName(desc->networkId_, desc->deviceRole_);
    }
}

std::string AudioPolicyUtils::GetSourcePortName(DeviceType deviceType)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case InternalDeviceType::DEVICE_TYPE_MIC:
        case InternalDeviceType::DEVICE_TYPE_NEARLINK_IN:
            portName = PRIMARY_MIC;
            break;
        case InternalDeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
            portName = USB_MIC;
            break;
        case InternalDeviceType::DEVICE_TYPE_WAKEUP:
            portName = PRIMARY_WAKEUP;
            break;
        case InternalDeviceType::DEVICE_TYPE_FILE_SOURCE:
            portName = FILE_SOURCE;
            break;
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN:
            portName = BLUETOOTH_MIC;
            break;
        case InternalDeviceType::DEVICE_TYPE_ACCESSORY:
            portName = ACCESSORY_SOURCE;
            break;
        default:
            portName = PORT_NONE;
            break;
    }

    return portName;
}

std::string AudioPolicyUtils::GetOutputDeviceClassBySinkPortName(std::string sinkPortName)
{
    std::map<std::string, std::string> sinkPortStrToClassStrMap_ = {
        {PRIMARY_SPEAKER, PRIMARY_CLASS},
        {OFFLOAD_PRIMARY_SPEAKER, OFFLOAD_CLASS},
        {BLUETOOTH_SPEAKER, A2DP_CLASS},
        {USB_SPEAKER, USB_CLASS},
        {PRIMARY_DIRECT_VOIP, DIRECT_VOIP_CLASS},
        {DP_SINK, DP_CLASS},
        {FILE_SINK, FILE_CLASS},
        {REMOTE_CAST_INNER_CAPTURER_SINK_NAME, REMOTE_CLASS},
        {MCH_PRIMARY_SPEAKER, MCH_CLASS},
        {PORT_NONE, INVALID_CLASS}
    };
    std::string deviceClass = INVALID_CLASS;
    if (sinkPortStrToClassStrMap_.count(sinkPortName) > 0) {
        deviceClass = sinkPortStrToClassStrMap_.at(sinkPortName);
    }
    return deviceClass;
}

std::string AudioPolicyUtils::GetInputDeviceClassBySourcePortName(std::string sourcePortName)
{
    std::map<std::string, std::string> sourcePortStrToClassStrMap_ = {
        {PRIMARY_MIC, PRIMARY_CLASS},
        {USB_MIC, USB_CLASS},
        {PRIMARY_WAKEUP, PRIMARY_CLASS},
        {FILE_SOURCE, FILE_CLASS},
        {BLUETOOTH_MIC, A2DP_CLASS},
        {PORT_NONE, INVALID_CLASS}
    };
    std::string deviceClass = INVALID_CLASS;
    if (sourcePortStrToClassStrMap_.count(sourcePortName) > 0) {
        deviceClass = sourcePortStrToClassStrMap_.at(sourcePortName);
    }
    return deviceClass;
}

std::shared_ptr<DataShare::DataShareHelper> AudioPolicyUtils::CreateDataShareHelperInstance()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "[Policy Service] Get samgr failed.");

    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, nullptr, "[Policy Service] audio service remote object is NULL.");

    int64_t startTime = ClockTime::GetCurNano();
    sptr<IRemoteObject> dataSharedServer = samgr->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    int64_t cost = ClockTime::GetCurNano() - startTime;
    if (cost > CALL_IPC_COST_TIME_MS) {
        AUDIO_WARNING_LOG("Call get DataShare server cost too long: %{public}" PRId64"ms.", cost / AUDIO_US_PER_SECOND);
    }

    CHECK_AND_RETURN_RET_LOG(dataSharedServer != nullptr, nullptr, "DataShare server is not started!");

    WatchTimeout guard("DataShare::DataShareHelper::Create", CALL_IPC_COST_TIME_MS);
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> res = DataShare::DataShareHelper::Create(remoteObject,
        SETTINGS_DATA_BASE_URI, SETTINGS_DATA_EXT_URI);
    guard.CheckCurrTimeout();
    if (res.first == DataShare::E_DATA_SHARE_NOT_READY) {
        AUDIO_WARNING_LOG("DataShareHelper::Create failed: E_DATA_SHARE_NOT_READY");
        return nullptr;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = res.second;
    CHECK_AND_RETURN_RET_LOG(res.first == DataShare::E_OK && dataShareHelper != nullptr, nullptr, "fail:%{public}d",
        res.first);
    return dataShareHelper;
}

int32_t AudioPolicyUtils::GetDeviceNameFromDataShareHelper(std::string &deviceName)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelperInstance();
    CHECK_AND_RETURN_RET_LOG(dataShareHelper != nullptr, ERROR, "GetDeviceNameFromDataShareHelper NULL");

    std::shared_ptr<Uri> uri = std::make_shared<Uri>(SETTINGS_DATA_BASE_URI);
    std::vector<std::string> columns;
    columns.emplace_back(SETTINGS_DATA_FIELD_VALUE);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTINGS_DATA_FIELD_KEYWORD, PREDICATES_STRING);

    WatchTimeout guard("dataShareHelper->Query:DefaultDeviceName");
    auto resultSet = dataShareHelper->Query(*uri, predicates, columns);
    if (resultSet == nullptr) {
        AUDIO_ERR_LOG("Failed to query device name from dataShareHelper!");
        dataShareHelper->Release();
        return ERROR;
    }
    guard.CheckCurrTimeout();

    int32_t numRows = 0;
    resultSet->GetRowCount(numRows);
    if (numRows <= 0) {
        AUDIO_ERR_LOG("The result of querying is zero row!");
        resultSet->Close();
        dataShareHelper->Release();
        return ERROR;
    }

    int columnIndex;
    resultSet->GoToFirstRow();
    resultSet->GetColumnIndex(SETTINGS_DATA_FIELD_VALUE, columnIndex);
    resultSet->GetString(columnIndex, deviceName);
    AUDIO_INFO_LOG("GetDeviceNameFromDataShareHelper");

    resultSet->Close();
    dataShareHelper->Release();
    return SUCCESS;
}


void AudioPolicyUtils::UpdateDisplayName(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    if (deviceDescriptor->networkId_ == LOCAL_NETWORK_ID) {
        std::string devicesName = "";
        int32_t ret  = GetDeviceNameFromDataShareHelper(devicesName);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Local UpdateDisplayName init device failed");
        deviceDescriptor->displayName_ = devicesName;
    } else {
        UpdateDisplayNameForRemote(deviceDescriptor);
    }
}

void AudioPolicyUtils::UpdateDisplayNameForRemote(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
#ifdef FEATURE_DEVICE_MANAGER
    std::shared_ptr<DistributedHardware::DmInitCallback> callback = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(AUDIO_SERVICE_PKG, callback);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "init device failed");
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    if (DistributedHardware::DeviceManager::GetInstance()
        .GetTrustedDeviceList(AUDIO_SERVICE_PKG, "", deviceList) == SUCCESS) {
        for (auto deviceInfo : deviceList) {
            std::string strNetworkId(deviceInfo.networkId);
            if (strNetworkId == desc->networkId_) {
                AUDIO_INFO_LOG("remote name [%{public}s]", deviceInfo.deviceName);
                desc->displayName_ = deviceInfo.deviceName;
                break;
            }
        }
    };
#endif
}

void AudioPolicyUtils::UpdateEffectDefaultSink(DeviceType deviceType)
{
    Trace trace("AudioPolicyUtils::UpdateEffectDefaultSink:" + std::to_string(deviceType));
    effectActiveDevice_ = deviceType;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_FILE_SINK:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_DP:
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_HDMI:
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL: {
            std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
            AudioServerProxy::GetInstance().SetOutputDeviceSinkProxy(deviceType, sinkName);
            break;
        }
        default:
            break;
    }
}

AudioModuleInfo AudioPolicyUtils::ConstructRemoteAudioModuleInfo(std::string networkId, DeviceRole deviceRole,
    DeviceType deviceType)
{
    AudioModuleInfo audioModuleInfo = {};
    if (deviceRole == DeviceRole::OUTPUT_DEVICE) {
        audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
        audioModuleInfo.format = "s16le"; // 16bit little endian
        audioModuleInfo.fixedLatency = "1"; // here we need to set latency fixed for a fixed buffer size.
        audioModuleInfo.renderInIdleState = "1";
        audioModuleInfo.role = "sink";
    } else if (deviceRole == DeviceRole::INPUT_DEVICE) {
        audioModuleInfo.lib = "libmodule-hdi-source.z.so";
        audioModuleInfo.format = "s16le"; // we assume it is bigger endian
        audioModuleInfo.role = "source";
    } else {
        AUDIO_WARNING_LOG("Invalid flag provided %{public}d", static_cast<int32_t>(deviceType));
    }

    // used as "sink_name" in hdi_sink.c, hope we could use name to find target sink.
    audioModuleInfo.name = GetRemoteModuleName(networkId, deviceRole);
    audioModuleInfo.networkId = networkId;

    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(deviceType);
    audioModuleInfo.deviceType = typeValue.str();

    audioModuleInfo.adapterName = "remote";
    audioModuleInfo.className = "remote"; // used in renderer_sink_adapter.c
    audioModuleInfo.fileName = "remote_dump_file";

    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.bufferSize = "3840";

    if (deviceType == DEVICE_TYPE_SPEAKER) {
        std::string splitInfo = "";
        if ((AudioRouterCenter::GetAudioRouterCenter().GetSplitInfo(splitInfo) == SUCCESS) && (splitInfo != "")) {
            audioModuleInfo.lib = "libmodule-split-stream-sink.z.so";
            audioModuleInfo.extra = splitInfo;
        }
    }

    return audioModuleInfo;
}

DeviceRole AudioPolicyUtils::GetDeviceRole(DeviceType deviceType) const
{
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_DP:
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
        case DeviceType::DEVICE_TYPE_REMOTE_CAST:
        case DeviceType::DEVICE_TYPE_HDMI:
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
            return DeviceRole::OUTPUT_DEVICE;
        case DeviceType::DEVICE_TYPE_MIC:
        case DeviceType::DEVICE_TYPE_WAKEUP:
        case DeviceType::DEVICE_TYPE_ACCESSORY:
            return DeviceRole::INPUT_DEVICE;
        default:
            return DeviceRole::DEVICE_ROLE_NONE;
    }
}

DeviceRole AudioPolicyUtils::GetDeviceRole(const std::string &role)
{
    if (role == ROLE_SINK) {
        return DeviceRole::OUTPUT_DEVICE;
    } else if (role == ROLE_SOURCE) {
        return DeviceRole::INPUT_DEVICE;
    } else {
        return DeviceRole::DEVICE_ROLE_NONE;
    }
}

DeviceRole AudioPolicyUtils::GetDeviceRole(AudioPin pin) const
{
    switch (pin) {
        case OHOS::AudioStandard::AUDIO_PIN_NONE:
            return DeviceRole::DEVICE_ROLE_NONE;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_SPEAKER:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HEADSET:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HEADPHONE:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_LINEOUT:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HDMI:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB_EXT:
        case OHOS::AudioStandard::AUDIO_PIN_OUT_DAUDIO_DEFAULT:
            return DeviceRole::OUTPUT_DEVICE;
        case OHOS::AudioStandard::AUDIO_PIN_IN_MIC:
        case OHOS::AudioStandard::AUDIO_PIN_IN_HS_MIC:
        case OHOS::AudioStandard::AUDIO_PIN_IN_LINEIN:
        case OHOS::AudioStandard::AUDIO_PIN_IN_USB_EXT:
        case OHOS::AudioStandard::AUDIO_PIN_IN_PENCIL:
        case OHOS::AudioStandard::AUDIO_PIN_IN_UWB:
        case OHOS::AudioStandard::AUDIO_PIN_IN_DAUDIO_DEFAULT:
            return DeviceRole::INPUT_DEVICE;
        default:
            return DeviceRole::DEVICE_ROLE_NONE;
    }
}

DeviceType AudioPolicyUtils::GetDeviceType(const std::string &deviceName)
{
    DeviceType devType = DeviceType::DEVICE_TYPE_NONE;
    if (deviceName == "Speaker") {
        devType = DeviceType::DEVICE_TYPE_SPEAKER;
    } else if (deviceName == "Built_in_mic") {
        devType = DeviceType::DEVICE_TYPE_MIC;
    } else if (deviceName == "Built_in_wakeup") {
        devType = DeviceType::DEVICE_TYPE_WAKEUP;
    } else if (deviceName == "fifo_output" || deviceName == "fifo_input") {
        devType = DEVICE_TYPE_BLUETOOTH_SCO;
    } else if (deviceName == "file_sink") {
        devType = DEVICE_TYPE_FILE_SINK;
    } else if (deviceName == "file_source") {
        devType = DEVICE_TYPE_FILE_SOURCE;
    }
    return devType;
}

std::string AudioPolicyUtils::GetDevicesStr(const vector<shared_ptr<AudioDeviceDescriptor>> &descs)
{
    std::string devices;
    devices.append("device type:id:(category:constate:enable:exceptionflag) ");
    for (auto iter : descs) {
        CHECK_AND_CONTINUE_LOG(iter != nullptr, "iter is nullptr");
        devices.append(std::to_string(static_cast<uint32_t>(iter->getType())));
        devices.append(":" + std::to_string(static_cast<uint32_t>(iter->deviceId_)));
        if (iter->getType() == DEVICE_TYPE_BLUETOOTH_A2DP ||
            iter->getType() == DEVICE_TYPE_BLUETOOTH_SCO ||
            iter->getType() == DEVICE_TYPE_NEARLINK ||
            iter->getType() == DEVICE_TYPE_NEARLINK_IN) {
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->deviceCategory_)));
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->connectState_)));
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->isEnable_)));
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->exceptionFlag_)));
        } else if (IsUsb(iter->getType())) {
            devices.append(":" + GetEncryptAddr(iter->macAddress_));
        }
        devices.append(" ");
    }
    return devices;
}

AudioDeviceUsage AudioPolicyUtils::GetAudioDeviceUsageByStreamUsage(StreamUsage streamUsage)
{
    switch (streamUsage) {
        case StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
            return CALL_OUTPUT_DEVICES;
        default:
            return MEDIA_OUTPUT_DEVICES;
    }
}

PreferredType AudioPolicyUtils::GetPreferredTypeByStreamUsage(StreamUsage streamUsage)
{
    switch (streamUsage) {
        case StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
            return AUDIO_CALL_RENDER;
        default:
            return AUDIO_MEDIA_RENDER;
    }
}

int32_t AudioPolicyUtils::UnexcludeOutputDevices(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    if (isBTReconnecting_) {
        return SUCCESS;
    }

    AudioRecoveryDevice::GetInstance().UnexcludeOutputDevicesInner(MEDIA_OUTPUT_DEVICES, descs);
    AudioRecoveryDevice::GetInstance().UnexcludeOutputDevicesInner(CALL_OUTPUT_DEVICES, descs);

    return SUCCESS;
}

void AudioPolicyUtils::SetScoExcluded(bool scoExcluded)
{
    isScoExcluded_ = scoExcluded;
}

bool AudioPolicyUtils::GetScoExcluded()
{
    return isScoExcluded_;
}

bool AudioPolicyUtils::IsDataShareReady()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, false, "[Policy Service] Get samgr failed.");
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, false, "[Policy Service] audio service remote object is NULL.");
    WatchTimeout guard("DataShare::DataShareHelper::Create:IsDataShareReady", CALL_IPC_COST_TIME_MS);
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> res = DataShare::DataShareHelper::Create(remoteObject,
        SETTINGS_DATA_BASE_URI, SETTINGS_DATA_EXT_URI);
    guard.CheckCurrTimeout();
    if (res.first == DataShare::E_OK) {
        AUDIO_INFO_LOG("DataShareHelper is ready.");
        auto helper = res.second;
        if (helper != nullptr) {
            helper->Release();
        }
        return true;
    } else {
        AUDIO_WARNING_LOG("DataShareHelper::Create failed: E_DATA_SHARE_NOT_READY");
        return false;
    }
}

} // namespace AudioStandard
} // namespace OHOS
