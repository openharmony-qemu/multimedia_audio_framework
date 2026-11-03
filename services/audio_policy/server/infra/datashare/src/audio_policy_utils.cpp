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
#include "audio_bus_selector.h"
#include "audio_bundle_manager.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"
#include "audio_select_interface_service.h"
#include "audio_server_proxy.h"
#include "audio_device_status.h"
#include "audio_rollback_manager.h"

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t NS_PER_MS = 1000000;
static constexpr int32_t MS_PER_S = 1000;
static constexpr int32_t NEARLINK_API_VERSION = 20;
static const char* SETTINGS_DATA_FIELD_VALUE = "VALUE";
static const char* SETTINGS_DATA_FIELD_KEYWORD = "KEYWORD";
static const char* PREDICATES_STRING = "settings.general.device_name";
static const char* SETTINGS_DATA_BASE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static const char* SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
static const char* AUDIO_SERVICE_PKG = "audio_manager_service";
static const std::string NEARLINK_LIST = "audio_nearlink_list";
static const std::map<DeviceType, std::string> SUPPORTED_LIST = {
    {DEVICE_TYPE_NEARLINK, "nearlink_whitelist"},
    {DEVICE_TYPE_NEARLINK_IN, "nearlink_whitelist"},
    {DEVICE_TYPE_HEARING_AID, "hearing_aid_whitelist"},
};

std::map<std::string, ClassType> AudioPolicyUtils::portStrToEnum = {
    {PRIMARY_SPEAKER, TYPE_PRIMARY},
    {PRIMARY_MIC, TYPE_PRIMARY},
    {PRIMARY_AI_MIC, TYPE_PRIMARY},
    {PRIMARY_ULTRASONIC_MIC, TYPE_PRIMARY},
    {PRIMARY_WAKEUP_MIC, TYPE_PRIMARY},
    {BLUETOOTH_SPEAKER, TYPE_A2DP},
    {BLUETOOTH_MIC, TYPE_A2DP},
    {USB_SPEAKER, TYPE_USB},
    {USB_MIC, TYPE_USB},
    {DP_SINK, TYPE_DP},
    {FILE_SINK, TYPE_FILE_IO},
    {FILE_SOURCE, TYPE_FILE_IO},
    {REMOTE_CLASS, TYPE_REMOTE_AUDIO},
    {PRIMARY_UNPROCESS_MIC, TYPE_PRIMARY},
    {PRIMARY_LIVE_MIC, TYPE_PRIMARY},
    {PRIMARY_VOICE_RECOGNITION_MIC, TYPE_PRIMARY},
    {PRIMARY_RAW_AI_MIC, TYPE_PRIMARY},
    {PRIMARY_INTERPHON_MIC, TYPE_PRIMARY},
    {PRIMARY_CAMCORDER, TYPE_PRIMARY},
    {PRIMARY_VOICE_MESSAGE_MIC, TYPE_PRIMARY},
};

std::set<DeviceType> AudioPolicyUtils::needDeclaredDevices = {
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_HEARING_AID,
};

std::map<DeviceType, std::string> AudioPolicyUtils::deviceMsmd = {
    {DEVICE_TYPE_USB_HEADSET, PRIMARY_TYPEC_CLASS},
    {DEVICE_TYPE_NEARLINK_IN, PRIMARY_BT_CLASS},
    {DEVICE_TYPE_BLUETOOTH_SCO, PRIMARY_BT_CLASS},
    {DEVICE_TYPE_WIRED_HEADSET, PRIMARY_WIRED_CLASS}
};

static inline const char* ResolvePrimaryMicPort(uint32_t routeFlag)
{
    switch (routeFlag) {
        case AUDIO_INPUT_FLAG_AI:               return PRIMARY_AI_MIC;
        case AUDIO_INPUT_FLAG_UNPROCESS:        return PRIMARY_UNPROCESS_MIC;
        case AUDIO_INPUT_FLAG_LIVE:             return PRIMARY_LIVE_MIC;
        case AUDIO_INPUT_FLAG_ULTRASONIC:       return PRIMARY_ULTRASONIC_MIC;
        case AUDIO_INPUT_FLAG_VOICE_RECOGNITION:return PRIMARY_VOICE_RECOGNITION_MIC;
        case AUDIO_INPUT_FLAG_RAW_AI:           return PRIMARY_RAW_AI_MIC;
        case AUDIO_INPUT_FLAG_CAMCORDER:        return PRIMARY_CAMCORDER;
        case AUDIO_INPUT_FLAG_VOICE_MESSAGE:    return PRIMARY_VOICE_MESSAGE_MIC;
        case AUDIO_INPUT_FLAG_FAST:             return FAST_CAPTURER_SOURCE;
        case AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP: return FAST_VOIP_CAPTURER_SOURCE;
        default:                                return PRIMARY_MIC;
    }
}

static inline const char* ResolveVaSourcePort(DeviceType deviceType, bool isUnidirectional)
{
    return deviceType == DeviceType::DEVICE_TYPE_NEARLINK_PORT ? VA_INPUT_NEARLINK
        : (deviceType == DeviceType::DEVICE_TYPE_BT_SPP
            ? (isUnidirectional ? VA_INPUT_UNIDIRECTIONAL_SPP : VA_INPUT_SPP) : PORT_NONE);
}

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

std::string AudioPolicyUtils::GetRemoteNetworkId(const std::string &sinkName, DeviceRole role)
{
    std::string networkId = "";
    auto pos = role == DeviceRole::OUTPUT_DEVICE ? sinkName.find("_out") : sinkName.find("_in");
    CHECK_AND_RETURN_RET(pos != std::string::npos, networkId);
    networkId = sinkName.substr(0, pos);
    return networkId;
}

std::string AudioPolicyUtils::GetA2dpModuleName(const std::string &macAddress, DeviceRole role)
{
#ifdef MULTI_USER_ZONE
    return macAddress + (role == DeviceRole::OUTPUT_DEVICE ? "_out" : "_in");
#else
    return role == DeviceRole::OUTPUT_DEVICE ? BLUETOOTH_SPEAKER : BLUETOOTH_MIC;
#endif
}

std::string AudioPolicyUtils::GetA2dpPipeName(const std::string &macAddress, DeviceRole role)
{
    std::string pipeName = (role == OUTPUT_DEVICE) ? "a2dp_output" : "a2dp_input";
#ifdef MULTI_USER_ZONE
    pipeName = (role == OUTPUT_DEVICE) ? "a2dp_" + macAddress + "_output" : pipeName;
#endif
    return pipeName;
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
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "desc is null");
    int32_t ret = SUCCESS;
    bool mediaControllerFlag = IsSelectedByMediaController();
    auto &rollback = AudioRollbackManager::GetInstance();
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    switch (preferredType) {
        case AUDIO_MEDIA_RENDER:
            rollback.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, desc);
            break;
        case AUDIO_CALL_RENDER:
            rollback.SetCallOutputDevice(AudioSelectInterfaceService::GetInstance().GetPreferredUid(uid),
                INVALID_STREAM_ID, desc, caller);
            break;
        case AUDIO_CALL_CAPTURE:
            rollback.SetCallInputDevice(SYSTEM_UID, INVALID_STREAM_ID, desc);
            break;
        case AUDIO_RECORD_CAPTURE:
#ifdef INPUT_FEATURE_INDEPENDENT_MODE
            SetPreferredRecordCaptureDeviceInCar(desc, uid);
#else
            {
                RecordDeviceInfo info {.uid_ = uid, .activeSelectedDevice_ = desc};
                AudioUsrSelectManager::GetAudioUsrSelectManager().UpdateRecordDeviceInfo(
                    UpdateType::SYSTEM_SELECT, info, mediaControllerFlag);
            }
            CHECK_AND_BREAK_LOG(!mediaControllerFlag, "selected by media controller");
            rollback.SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, desc);
#endif
            break;
        case AUDIO_RING_RENDER:
        case AUDIO_TONE_RENDER:
            ret = ERR_INVALID_PARAM;
            break;
        case AUDIO_RECOGNITION_CAPTURE:
            strategy.SetRecognitionInputDevice(desc);
            break;
        default:
            ret = ERR_INVALID_PARAM;
            break;
    }
    if (desc->deviceType_ == DEVICE_TYPE_NONE) {
        AudioRouterInfra::GetInstance().ErasePreferredDeviceByType(preferredType);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "SetPreferredDevice failed, preferredType:%{public}d, ret:%{public}d", preferredType, ret);
    AudioDeviceStatus::GetInstance().NotifyPreferredDeviceSet(preferredType, desc, uid, caller);
    return ret;
}

bool AudioPolicyUtils::IsSelectedByMediaController()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto callerUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = AudioBundleManager::GetBundleNameFromUid(callerUid);
    CHECK_AND_RETURN_RET(audioClientInfoMgrCallback_ != nullptr, false);
    bool ret = false;
    audioClientInfoMgrCallback_->OnCheckMediaControllerBundle(bundleName, ret);
    return ret;
}

bool AudioPolicyUtils::ShouldUseSystemPreferredDeviceForVolume(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = shouldUseSystemPreferredDeviceCache_.find(bundleName);
    if (it != shouldUseSystemPreferredDeviceCache_.end()) {
        return it->second;
    }

    CHECK_AND_RETURN_RET_LOG(audioClientInfoMgrCallback_ != nullptr, false, "callback is nullptr");
    bool ret = false;
    audioClientInfoMgrCallback_->OnQueryUseSystemPreferredDeviceForVolume(bundleName, ret);

    shouldUseSystemPreferredDeviceCache_[bundleName] = ret;
    return ret;
}

#ifdef INPUT_FEATURE_INDEPENDENT_MODE
bool AudioPolicyUtils::IsSelectedBySystemApp()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET(audioClientInfoMgrCallback_ != nullptr, false);
    auto callerUid = IPCSkeleton::GetCallingUid();
    auto callerPid = IPCSkeleton::GetCallingPid();
    std::string bundleName = AudioBundleManager::GetBundleNameFromUid(callerUid);
    bool ret = false;
    audioClientInfoMgrCallback_->OnCheckClientInfo(bundleName, callerUid, callerPid, ret);
    return callerUid == SYSTEM_UID;
}

void AudioPolicyUtils::SetPreferredRecordCaptureDeviceInCar(
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    if (!IsSelectedBySystemApp()) {
        AudioUsrSelectManager::GetAudioUsrSelectManager().SelectInputDeviceByUid(
            desc, IPCSkeleton::GetCallingUid());
    } else {
        {
            RecordDeviceInfo info {.uid_ = uid, .activeSelectedDevice_ = desc};
            AudioUsrSelectManager::GetAudioUsrSelectManager().UpdateRecordDeviceInfo(
                UpdateType::SYSTEM_SELECT, info);
        }
        auto &rollback = AudioRollbackManager::GetInstance();
        rollback.SetMediaInputDevice(SYSTEM_UID, INVALID_STREAM_ID, desc);
    }
}
#endif

void AudioPolicyUtils::ClearScoDeviceSuspendState(std::string macAddress)
{
    AUDIO_DEBUG_LOG("Clear sco suspend state %{public}s", GetEncryptAddr(macAddress).c_str());
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetDevicesByFilter(
        DEVICE_TYPE_NONE, DEVICE_ROLE_NONE, macAddress, "", SUSPEND_CONNECTED);
    for (const auto &desc : descs) {
        if (desc->deviceType_ == DEVICE_TYPE_NEARLINK || desc->deviceType_  == DEVICE_TYPE_NEARLINK_IN) {
            desc->connectState_ = CONNECTED;
        } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            desc->connectState_ = DEACTIVE_CONNECTED;
        }
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

bool AudioPolicyUtils::IsVoiceStreamType(StreamUsage streamUsage)
{
    bool result = false;

    switch (streamUsage) {
        case StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE:
        case StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_RINGTONE:
        case StreamUsage::STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_TRANSFER_RINGTONE:
        case StreamUsage::STREAM_USAGE_INTERPHONE:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool AudioPolicyUtils::IsVoiceSourceType(SourceType sourceType)
{
    bool result = false;

    switch (sourceType) {
        case SourceType::SOURCE_TYPE_VOICE_CALL:
        case SourceType::SOURCE_TYPE_VOICE_COMMUNICATION:
        case SourceType::SOURCE_TYPE_INTERPHONE:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
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

std::string AudioPolicyUtils::GetSinkPortName(DeviceType deviceType, AudioPipeType pipeType,
    const std::string &macAddress)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            // BTH tells us that a2dpoffload is OK
            if (audioA2dpOffloadFlag_.GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
                if (pipeType == PIPE_TYPE_OUT_OFFLOAD) {
                    portName = OFFLOAD_PRIMARY_SPEAKER;
                } else if (pipeType == PIPE_TYPE_OUT_MULTICHANNEL) {
                    portName = MCH_PRIMARY_SPEAKER;
                } else {
                    portName = PRIMARY_SPEAKER;
                }
            } else {
                portName = GetA2dpModuleName(macAddress, OUTPUT_DEVICE);
            }
            break;
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_NEARLINK:
            if (pipeType == PIPE_TYPE_OUT_OFFLOAD) {
                portName = OFFLOAD_PRIMARY_SPEAKER;
            } else if (pipeType == PIPE_TYPE_OUT_MULTICHANNEL) {
                portName = MCH_PRIMARY_SPEAKER;
            } else {
                portName = PRIMARY_SPEAKER;
            }
            break;
        case DeviceType::DEVICE_TYPE_HDMI:
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
            portName = PRIMARY_SPEAKER;
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
#ifdef MULTI_BUS_ENABLE
        if (IsMultiBusDevice(desc.getType())) {
            return AudioBusSelector::GetBusSelector().GetSinkNameByStreamId(sessionId);
        }
#endif
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(sessionId, pipeType);
        return GetSinkPortName(desc.deviceType_, pipeType, desc.macAddress_);
    } else {
        return GetRemoteModuleName(desc.networkId_, desc.deviceRole_);
    }
}

std::string AudioPolicyUtils::GetSinkName(std::shared_ptr<AudioDeviceDescriptor> desc, int32_t sessionId)
{
    if (desc->networkId_ == LOCAL_NETWORK_ID) {
#ifdef MULTI_BUS_ENABLE
        if (IsMultiBusDevice(desc->getType())) {
            return AudioBusSelector::GetBusSelector().GetSinkNameByStreamId(sessionId);
        }
#endif
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(sessionId, pipeType);
        return GetSinkPortName(desc->deviceType_, pipeType, desc->macAddress_);
    } else {
        return GetRemoteModuleName(desc->networkId_, desc->deviceRole_);
    }
}

std::string AudioPolicyUtils::GetSourcePortName(DeviceType deviceType, uint32_t routeFlag,
    bool isUnidirectional)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_MIC:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_NEARLINK_IN:
        case DeviceType::DEVICE_TYPE_WIRED_MICIN:
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
            portName = ResolvePrimaryMicPort(routeFlag);
            AUDIO_INFO_LOG("use %{public}s for devicetype: %{public}d", portName.c_str(), deviceType);
            if (Util::IsEnhancedRoutingSupported() && portName == PRIMARY_MIC) {
                auto it = deviceMsmd.find(deviceType);
                if (it != deviceMsmd.end()) {
                    portName = it->second;
                    AUDIO_INFO_LOG("use %{public}s for devicetype: %{public}d", portName.c_str(), deviceType);
                }
            }
            break;
        case DeviceType::DEVICE_TYPE_BT_SPP:
        case DeviceType::DEVICE_TYPE_NEARLINK_PORT:
            portName = ResolveVaSourcePort(deviceType, isUnidirectional);
            break;
        case InternalDeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
            portName = USB_MIC;
            if (routeFlag == AUDIO_INPUT_FLAG_FAST) {
                portName = "FastUsbArmCapturerSource";
            }
            break;
        case DeviceType::DEVICE_TYPE_WAKEUP:
            portName = PRIMARY_WAKEUP;
            break;
        case DeviceType::DEVICE_TYPE_FILE_SOURCE:
            portName = FILE_SOURCE;
            break;
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN:
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
        {PRIMARY_MMAP_VOIP, MMAP_VOIP_CLASS},
        {DP_SINK, DP_CLASS},
        {FILE_SINK, FILE_CLASS},
        {REMOTE_CAST_INNER_CAPTURER_SINK_NAME, REMOTE_CLASS},
        {MCH_PRIMARY_SPEAKER, MCH_CLASS},
        {PORT_NONE, INVALID_CLASS},
        {PRIMARY_UNPROCESS_MIC, PRIMARY_CLASS},
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
        {PRIMARY_TYPEC_CLASS, PRIMARY_CLASS},
        {PRIMARY_WIRED_CLASS, PRIMARY_CLASS},
        {PRIMARY_BT_CLASS, PRIMARY_CLASS},
        {USB_MIC, USB_CLASS},
        {PRIMARY_WAKEUP, PRIMARY_CLASS},
        {FILE_SOURCE, FILE_CLASS},
        {BLUETOOTH_MIC, A2DP_CLASS},
        {PRIMARY_AI_MIC, PRIMARY_CLASS},
        {PRIMARY_LIVE_MIC, PRIMARY_CLASS},
        {PORT_NONE, INVALID_CLASS},
        {PRIMARY_ULTRASONIC_MIC, PRIMARY_CLASS},
        {PRIMARY_VOICE_RECOGNITION_MIC, PRIMARY_CLASS},
        {PRIMARY_RAW_AI_MIC, PRIMARY_CLASS},
        {PRIMARY_CAMCORDER, PRIMARY_CLASS},
        {PRIMARY_VOICE_MESSAGE_MIC, PRIMARY_CLASS},
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

int32_t AudioPolicyUtils::GetRemoteDeviceName(const std::string &deviceId, std::string &deviceName)
{
#ifdef FEATURE_DEVICE_MANAGER
    std::shared_ptr<DistributedHardware::DmInitCallback> callback = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(AUDIO_SERVICE_PKG, callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "init device failed");
    DistributedHardware::DmDeviceInfo deviceInfo;
    ret = DistributedHardware::DeviceManager::GetInstance().
        GetDeviceInfo(AUDIO_SERVICE_PKG, deviceId, deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "get device info failed");
    deviceName = deviceInfo.deviceName;
#endif
    return SUCCESS;
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
        case DeviceType::DEVICE_TYPE_LINE_DIGITAL:
        case DeviceType::DEVICE_TYPE_NEARLINK: {
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
    } else if (deviceName == "Built_in_mic" || deviceName == PRIMARY_AI_MIC || deviceName == PRIMARY_UNPROCESS_MIC
        || deviceName == PRIMARY_ULTRASONIC_MIC ||
        deviceName == PRIMARY_VOICE_RECOGNITION_MIC || deviceName == PRIMARY_RAW_AI_MIC ||
        deviceName == PRIMARY_INTERPHON_MIC || deviceName == PRIMARY_LIVE_MIC ||
        deviceName == PRIMARY_CAMCORDER || deviceName == PRIMARY_VOICE_MESSAGE_MIC) {
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
    devices.append("device type:id:(category:constate:enable:exceptionflag:deviceusage) ");
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
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->deviceUsage_)));
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
        case StreamUsage::STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT:
        case StreamUsage::STREAM_USAGE_INTERPHONE:
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
        case StreamUsage::STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
        case StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT:
        case StreamUsage::STREAM_USAGE_INTERPHONE:
            return AUDIO_CALL_RENDER;
        default:
            return AUDIO_MEDIA_RENDER;
    }
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

int32_t AudioPolicyUtils::SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "object is nullptr");
    queryBundleNameListCallback_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(queryBundleNameListCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "queryBundleNameListCallback_ is nullptr");
    return SUCCESS;
}

bool AudioPolicyUtils::IsBundleNameInList(const std::string &bundleName, DeviceType deviceType)
{
    bool isBundleNameExist = false;
    auto it = SUPPORTED_LIST.find(deviceType);
    CHECK_AND_RETURN_RET(it != SUPPORTED_LIST.end(), false);
    std::string listType = it->second;
    CHECK_AND_RETURN_RET_LOG(queryBundleNameListCallback_ != nullptr, false, "queryBundleNameListCallback_ is nullptr");
    queryBundleNameListCallback_->OnQueryBundleNameIsInList(bundleName, listType, isBundleNameExist);
    return isBundleNameExist;
}

int32_t AudioPolicyUtils::SetDeclaredDeviceTypes(const std::string &bundleName,
    const std::vector<DeviceType> &deviceTypeVector)
{
    std::unique_lock lock(setDeclaredDevicesMutex_);
    auto &declaredDeviceTypeSet = alreadyDeclaredDevicesByApp_[bundleName];
    std::string deviceTypeLog = "";
    for (auto deviceType : deviceTypeVector) {
        declaredDeviceTypeSet.insert(deviceType);
        if (deviceType == DEVICE_TYPE_NEARLINK) {
            declaredDeviceTypeSet.insert(DEVICE_TYPE_NEARLINK_IN);
        }
        deviceTypeLog += std::to_string(static_cast<int>(deviceType)) + " ";
    }
    AUDIO_INFO_LOG("bundle name: %{public}s, declare supported devicetype: %{public}s", bundleName.c_str(),
        deviceTypeLog.c_str());
    return SUCCESS;
}

bool AudioPolicyUtils::IsDeclaredByApp(const std::string &bundleName, DeviceType deviceType)
{
    std::shared_lock lock(setDeclaredDevicesMutex_);
    auto itr = alreadyDeclaredDevicesByApp_.find(bundleName);
    CHECK_AND_RETURN_RET(itr != alreadyDeclaredDevicesByApp_.end(), false);
    return itr->second.contains(deviceType);
}

bool AudioPolicyUtils::IsSupportedDevice(const std::string &bundleName, DeviceType deviceType, bool hasSystemPermission)
{
    CHECK_AND_RETURN_RET(needDeclaredDevices.contains(deviceType), true);
    return hasSystemPermission ||
        IsDeclaredByApp(bundleName, deviceType) || IsBundleNameInList(bundleName, deviceType);
}

bool AudioPolicyUtils::IsWirelessDevice(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_NEARLINK:
        case DEVICE_TYPE_NEARLINK_IN:
            return true;
        default:
            return false;
    }
}

bool AudioPolicyUtils::CheckInterphone(StreamUsage usage, const shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (desc && usage == STREAM_USAGE_INTERPHONE) {
        AUDIO_INFO_LOG("id=%{public}d, type=%{public}d, isLocal=%{public}d, role=%{public}d, "
            "pairId=%{public}d, pairType=%{public}d", desc->deviceId_, desc->deviceType_,
            desc->networkId_ == LOCAL_NETWORK_ID, desc->deviceRole_,
            desc->pairDeviceDescriptor_ ? desc->pairDeviceDescriptor_->deviceId_ : 0,
            desc->pairDeviceDescriptor_ ? desc->pairDeviceDescriptor_->deviceType_ : 0);
        switch (desc->deviceType_) {
            case DEVICE_TYPE_USB_ARM_HEADSET:
            case DEVICE_TYPE_EARPIECE:
            case DEVICE_TYPE_HEARING_AID:
                return false;
            case DEVICE_TYPE_BLUETOOTH_SCO:
            case DEVICE_TYPE_NEARLINK:
                return desc->deviceCategory_ != BT_WATCH && desc->deviceCategory_ != BT_HEARAID;
            case DEVICE_TYPE_SPEAKER:
                return desc->networkId_ == LOCAL_NETWORK_ID;
            default:
                return desc->pairDeviceDescriptor_ && desc->networkId_ == LOCAL_NETWORK_ID;
        }
    }
    return true;
}

bool AudioPolicyUtils::IsCallOrRingStreamUsage(StreamUsage streamUsage)
{
    return streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VOICE_CALL_ASSISTANT ||
           streamUsage == STREAM_USAGE_INTERPHONE ||
           streamUsage == STREAM_USAGE_RINGTONE ||
           streamUsage == STREAM_USAGE_VOICE_RINGTONE ||
           streamUsage == STREAM_USAGE_VOICE_TRANSFER_RINGTONE;
}

int32_t AudioPolicyUtils::SetAudioClientInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioClientInfoMgrCallback_ = callback;
    return 0;
}

string AudioPolicyUtils::EncUsbAddr(const string &src)
{
    const string head("card=");
    auto pos = src.find(';', head.length());
    CHECK_AND_RETURN_RET_LOG(pos != string::npos, "", "Illegal usb address");
    auto num = src.substr(head.length(), pos - head.length());
    return string("c**") + num + "**";
}

void AudioPolicyUtils::FilterDevicesByTokenId(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    CHECK_AND_RETURN_LOG(!PermissionUtil::VerifyIsAudio(), "called by audio, skip");

    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    audioDeviceDescriptors.erase(
        std::remove_if(audioDeviceDescriptors.begin(), audioDeviceDescriptors.end(),
            [callingTokenId](const auto &device) {
                bool filter = device != nullptr && device->deviceTokenId_ != 0 &&
                    device->deviceTokenId_ != callingTokenId;
                JUDGE_AND_INFO_LOG(filter, "Filter device with tokenId %{public}u, callingTokenId %{public}u",
                    device->deviceTokenId_, callingTokenId);
                return filter;
            }),
        audioDeviceDescriptors.end());
}

void AudioPolicyUtils::FilterSystemDevices(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    audioDeviceDescriptors.erase(
        std::remove_if(audioDeviceDescriptors.begin(), audioDeviceDescriptors.end(),
            [](const auto &device) {
                bool filter = device != nullptr && device->IsSystemVisibility();
                return filter;
            }),
        audioDeviceDescriptors.end());
}

bool AudioPolicyUtils::IsDeviceMatch(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr && devDesc != nullptr, false, "desc is nullptr");
    return desc->deviceType_ == devDesc->deviceType_ &&
        desc->macAddress_ == devDesc->macAddress_ &&
        desc->networkId_ == devDesc->networkId_;
}

bool AudioPolicyUtils::UpdateDevicesListInfo(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc, DeviceInfoUpdateCommand updateCommand,
    AudioStreamDeviceChangeReasonExt &reason)
{
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, false, "desc is nullptr");

    bool ret = false;
    switch (updateCommand) {
        case CATEGORY_UPDATE:
            ret = UpdateDeviceCategory(devices, devDesc);
            reason = devDesc->deviceCategory_ == BT_UNWEAR_HEADPHONE ?
                AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE :
                AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
            break;
        case CONNECTSTATE_UPDATE:
            ret = UpdateConnectState(devices, devDesc, reason);
            break;
        case ENABLE_UPDATE:
            ret = UpdateEnableState(devices, devDesc);
            reason = devDesc->isEnable_ ? AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE :
                AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
            break;
        case EXCEPTION_FLAG_UPDATE:
            ret = UpdateExceptionFlag(devices, devDesc);
            break;
        case USAGE_UPDATE:
            ret = UpdateDeviceUsage(devices, devDesc, reason);
            break;
        default:
            break;
    }
    if (!ret) {
        int32_t audioId = devDesc->deviceId_;
        AUDIO_ERR_LOG("do not update type:id %{public}d:%{public}d mac:%{public}s\
            networkid:%{public}s in connected list",
            devDesc->deviceType_, audioId, GetEncryptStr(devDesc->macAddress_).c_str(),
            GetEncryptStr(devDesc->networkId_).c_str());
        return ret;
    }
    return ret;
}

bool AudioPolicyUtils::UpdateDeviceCategory(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    std::shared_ptr<AudioDeviceDescriptor> updated = nullptr;
    bool isGlasses = false;
    for (auto &desc : devices) {
        if (IsDeviceMatch(desc, devDesc) && desc->deviceCategory_ != devDesc->deviceCategory_) {
            descForCb.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            isGlasses |= desc->deviceCategory_ == BT_GLASSES;
            desc->deviceCategory_ = devDesc->deviceCategory_;
            descForCb.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            if (devDesc->deviceCategory_ != BT_UNWEAR_HEADPHONE) {
                UpdateDeviceConnectTimestamp(*desc);
            }
            TriggerDeviceInfoUpdatedCallback(*desc, descForCb);
            updated = desc;
            updateFlag = true;
        }
    }

    if (isGlasses && devDesc->deviceCategory_ == BT_UNWEAR_HEADPHONE) {
        UpdateTimestampAfterUnwearGlasses(devices, updated);
    }

    return updateFlag;
}

void AudioPolicyUtils::UpdateTimestampAfterUnwearGlasses(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &updated)
{
    CHECK_AND_RETURN(updated != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {};
    // privacy devices exclude updated
    for (auto &desc : devices) {
        CHECK_AND_CONTINUE(desc != nullptr && audioDeviceManager_.GetDevicePrivacyType(desc) == TYPE_PRIVACY);
        CHECK_AND_CONTINUE(!desc->IsSameBtOrSppDeviceDescPtr(updated) &&
            !desc->IsSameNearlinkOrPortDeviceDescPtr(updated));
        descs.push_back(desc);
    }

    CHECK_AND_RETURN(!descs.empty());
    AUDIO_INFO_LOG("update reduced connect timestamp");

    // descending order
    std::sort(descs.begin(), descs.end(),
        [](const auto &desc1, const auto &desc2) {
            return desc2->connectTimeStamp_ < desc1->connectTimeStamp_;
        });

    int64_t ts = GetCurrentTimeMS();
    for (auto &desc : descs) {
        if (desc->isPriorityReduced_) {
            desc->connectTimeStamp_ = ts;
            desc->isPriorityReduced_ = false;
        } else {
            ts = desc->connectTimeStamp_;
        }
        ts = DecrementTimestamp(ts);
    }
}

bool AudioPolicyUtils::UpdateConnectState(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc, AudioStreamDeviceChangeReasonExt &reason)
{
    bool updateFlag = false;
    bool isScoDevice = devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO;
    bool isScoSuspend = false;

    for (const auto &desc : devices) {
        CHECK_AND_CONTINUE(desc != nullptr);
        if (desc->networkId_ != devDesc->networkId_ ||
            desc->macAddress_ != devDesc->macAddress_) {
            continue;
        }
        if (desc->deviceType_ == devDesc->deviceType_) {
            isScoSuspend |= desc->connectState_ == CONNECTED && devDesc->connectState_ == SUSPEND_CONNECTED;
            desc->connectState_ = devDesc->connectState_;
            updateFlag = true;
            continue;
        }
        if (isScoDevice) {
            if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc->connectState_ != VIRTUAL_CONNECTED &&
                devDesc->connectState_ == CONNECTED) {
                desc->connectState_ = SUSPEND_CONNECTED;
                updateFlag = true;
                AUDIO_WARNING_LOG("sco connectState is connected, update a2dp to suspend");
            } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc->connectState_ == SUSPEND_CONNECTED &&
                (devDesc->connectState_ == DEACTIVE_CONNECTED || devDesc->connectState_ == SUSPEND_CONNECTED)) {
                desc->connectState_ = CONNECTED;
                updateFlag = true;
                AUDIO_WARNING_LOG("sco connectState %{public}d, update a2dp to connected", devDesc->connectState_);
            }
        }
    }
    reason = isScoDevice && isScoSuspend ? AudioStreamDeviceChangeReasonExt::ExtEnum::DEVICE_RESELECT : reason;

    return updateFlag;
}

bool AudioPolicyUtils::UpdateEnableState(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    std::shared_ptr<AudioDeviceDescriptor> updated = nullptr;
    for (const auto &desc : devices) {
        if (IsDeviceMatch(desc, devDesc) && desc->isEnable_ != devDesc->isEnable_) {
            descForCb.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            desc->isEnable_ = devDesc->isEnable_;
            desc->isExclusiveMicScoUnavailable_ = devDesc->isExclusiveMicScoUnavailable_;
            descForCb.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            updated = desc;
            updateFlag = true;
            AudioDeviceStatus::GetInstance().TriggerDeviceInfoUpdatedCallback(descForCb);
        }
    }

    if (devDesc->isEnable_) {
        UpdateTimestampWhenBecomeEnable(devices, updated);
    }

    return updateFlag;
}

bool AudioPolicyUtils::UpdateTimestampWhenBecomeEnable(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &updated)
{
    CHECK_AND_RETURN_RET(updated != nullptr, false);

    if (updated->deviceCategory_ == BT_GLASSES) {
        return false;
    }

    if (audioDeviceManager_.GetDevicePrivacyType(updated) != TYPE_PRIVACY) {
        return false;
    }

    if (!IsPrivacyActiveDevice(devices)) {
        return false;
    }

    AUDIO_INFO_LOG("reduce priority %{public}d:%{public}d", updated->deviceType_, updated->deviceId_);
    SetConnectTimestamp(devices, updated, GetMinPrivacyTimestamp(devices));

    return true;
}

int64_t AudioPolicyUtils::GetMinPrivacyTimestamp(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    int64_t min = GetCurrentTimeMS();
    for (const auto &desc : devices) {
        CHECK_AND_CONTINUE(desc != nullptr && audioDeviceManager_.GetDevicePrivacyType(desc) == TYPE_PRIVACY);
        min = min < desc->connectTimeStamp_ ? min : desc->connectTimeStamp_;
    }

    return min;
}

void AudioPolicyUtils::SetConnectTimestamp(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &updated, int64_t value)
{
    auto finalValue = updated->connectTimeStamp_ == value ? value : DecrementTimestamp(value);
    for (const auto &desc : devices) {
        CHECK_AND_CONTINUE(updated->IsSameBtOrSppDeviceDescPtr(desc) ||
            updated->IsSameNearlinkOrPortDeviceDescPtr(desc));
        desc->connectTimeStamp_ = finalValue;
        desc->isPriorityReduced_ = true;
    }
}

bool AudioPolicyUtils::IsPrivacyActiveDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    CHECK_AND_RETURN_RET_LOG(!simpleDescs.empty(), false, "active is invalid");

    auto it = std::find_if(simpleDescs.cbegin(), simpleDescs.cend(), [&devices, this](auto &desc) {
        auto devIt = std::find_if(devices.cbegin(), devices.cend(), [&desc](auto &item) {
            return item != nullptr && desc != nullptr && item->deviceId_ == desc->deviceId_;
        });
        return devIt != devices.cend() &&
            this->audioDeviceManager_.GetDevicePrivacyType(*devIt) == TYPE_PRIVACY;
    });
    return it != simpleDescs.cend();
}

int64_t AudioPolicyUtils::DecrementTimestamp(int64_t ts)
{
    // decrement connect timestamp for reduce priority, avoid underflow
    CHECK_AND_RETURN_RET_LOG(ts != INT64_MIN, ts, "timestamp is the min value");
    return ts - 1;
}

bool AudioPolicyUtils::UpdateExceptionFlag(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    for (const auto &desc : devices) {
        if (IsDeviceMatch(desc, devDesc) && desc->exceptionFlag_ != devDesc->exceptionFlag_) {
            desc->exceptionFlag_ = devDesc->exceptionFlag_;
            updateFlag = true;
        }
    }
    return updateFlag;
}

bool AudioPolicyUtils::UpdateDeviceUsage(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc, AudioStreamDeviceChangeReasonExt &reason)
{
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, false, "device is nullptr");
    bool updateFlag = false;
    std::shared_ptr<AudioDeviceDescriptor> updated = nullptr;
    for (const auto &desc : devices) {
        CHECK_AND_CONTINUE(desc != nullptr);
        if (IsDeviceMatch(desc, devDesc) && desc->deviceUsage_ != devDesc->deviceUsage_) {
            reason = (desc->deviceUsage_ > devDesc->deviceUsage_) ?
                AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE :
                AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
            desc->deviceUsage_ = devDesc->deviceUsage_;
            updated = desc;
            updateFlag = true;
        }
    }
    CHECK_AND_RETURN_RET_LOG(updateFlag == true, false, "Update usage fails");

    if (reason == AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE) {
        UpdateTimestampWhenBecomeEnable(devices, updated);
    }

    return true;
}

bool AudioPolicyUtils::TriggerDeviceInfoUpdatedCallback(AudioDeviceDescriptor &desc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    if (desc.glassesWearCount_ == AudioDeviceDescriptor::WEAR_FIRST && desc.deviceCategory_ == BT_UNWEAR_HEADPHONE) {
        return false;
    }

    if (desc.glassesWearCount_ < AudioDeviceDescriptor::WEAR_AGAIN) {
        AudioDeviceStatus::GetInstance().TriggerDeviceInfoUpdatedCallback(descForCb);
        return true;
    }

    return false;
}

void AudioPolicyUtils::UpdateDeviceConnectTimestamp(AudioDeviceDescriptor &desc)
{
    if (desc.deviceCategory_ == BT_GLASSES) {
        ++desc.glassesWearCount_;
        AUDIO_INFO_LOG("glasses wear count: %{public}u", desc.glassesWearCount_);
        if (desc.glassesWearCount_ < AudioDeviceDescriptor::WEAR_AGAIN) {
            desc.connectTimeStamp_ = GetCurrentTimeMS();
        } else {
            desc.connectTimeStamp_ = GetCurrentTimeMS() - AudioDeviceDescriptor::TEN_YEARS_MS;
        }
    } else {
        desc.connectTimeStamp_ = GetCurrentTimeMS();
    }
}

bool AudioPolicyUtils::IsMultiBusDevice(DeviceType deviceType)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ||
        deviceType == DEVICE_TYPE_BLUETOOTH_SCO ||
        deviceType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        return false;
    }
    return true;
}

} // namespace AudioStandard
} // namespace OHOS
