/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDeviceCommon"
#endif

#include "audio_device_common.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_spatialization_manager.h"
#include "audio_spatialization_service.h"
#include "common/hdi_adapter_info.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_event_utils.h"
#include "audio_recovery_device.h"
#include "audio_bundle_manager.h"
#include "audio_volume.h"

namespace OHOS {
namespace AudioStandard {

static const int64_t WAIT_MODEM_CALL_SET_VOLUME_TIME_US = 120000; // 120ms
static const int64_t OLD_DEVICE_UNAVALIABLE_MUTE_MS = 1000000; // 1s
static const int64_t NEW_DEVICE_AVALIABLE_MUTE_MS = 400000; // 400ms
static const int64_t NEW_DEVICE_AVALIABLE_OFFLOAD_MUTE_MS = 1000000; // 1s
static const int64_t NEW_DEVICE_REMOTE_CAST_AVALIABLE_MUTE_MS = 300000; // 300ms
static const int64_t SELECT_DEVICE_MUTE_MS = 200000; // 200ms
static const int64_t SELECT_OFFLOAD_DEVICE_MUTE_MS = 400000; // 400ms
static const int64_t OLD_DEVICE_UNAVALIABLE_MUTE_SLEEP_MS = 150000; // 150ms
static const int64_t OLD_DEVICE_UNAVALIABLE_EXT_SLEEP_US = 50000; // 50ms
static const int64_t OLD_DEVICE_UNAVALIABLE_EXT_MUTE_MS = 300000; // 300ms
static const int64_t DISTRIBUTED_DEVICE_UNAVALIABLE_MUTE_MS = 1500000;  // 1.5s
static const int64_t DISTRIBUTED_DEVICE_UNAVALIABLE_SLEEP_US = 350000; // 350ms
static const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;
static const int VOLUME_LEVEL_DEFAULT = 5;
static const int VOLUME_LEVEL_MIN_SIZE = 5;
static const int VOLUME_LEVEL_MID_SIZE = 12;
static const int VOLUME_LEVEL_MAX_SIZE = 15;
static const int32_t DISTRIBUTED_DEVICE = 1003;
static const int DEFAULT_ADJUST_TIMES = 10;

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

inline std::string PrintSourceOutput(SourceOutput sourceOutput)
{
    std::stringstream value;
    value << "streamId:[" << sourceOutput.streamId << "] ";
    value << "streamType:[" << sourceOutput.streamType << "] ";
    value << "uid:[" << sourceOutput.uid << "] ";
    value << "pid:[" << sourceOutput.pid << "] ";
    value << "statusMark:[" << sourceOutput.statusMark << "] ";
    value << "deviceSourceId:[" << sourceOutput.deviceSourceId << "] ";
    value << "startTime:[" << sourceOutput.startTime << "]";
    return value.str();
}

static const std::vector<std::string> SourceNames = {
    std::string(PRIMARY_MIC),
    std::string(BLUETOOTH_MIC),
    std::string(USB_MIC),
    std::string(PRIMARY_WAKEUP),
    std::string(FILE_SOURCE),
    std::string(PRIMARY_AI_MIC),
    std::string(PRIMARY_UNPROCESS_MIC),
    std::string(PRIMARY_ULTRASONIC_MIC),
    std::string(PRIMARY_VOICE_RECOGNITION_MIC),
    std::string(PRIMARY_RAW_AI_MIC)
};

static bool IsDistributedOutput(const AudioDeviceDescriptor &desc)
{
    return desc.deviceType_ == DEVICE_TYPE_SPEAKER && desc.networkId_ != LOCAL_NETWORK_ID;
}

#ifdef EXCLUDE_INDIRECT_USB_INPUT_DEVICE
static void ExcludeIndirectUsbArmInput(const AudioDeviceDescriptor &updatedDesc)
{
    CHECK_AND_RETURN(!updatedDesc.hasPair_ && updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET);
    string key = string("is_root_hub#C") + GetField(updatedDesc.macAddress_, "card", ';') + "D0";
    auto val = AudioServerProxy::GetInstance().GetAudioParameterProxy(key);
    AUDIO_INFO_LOG("key=%{public}s, val=%{public}s", key.c_str(), val.c_str());
    CHECK_AND_RETURN(val == "true");
    vector<shared_ptr<AudioDeviceDescriptor>> descs{make_shared<AudioDeviceDescriptor>(updatedDesc)};
    AudioStateManager::GetAudioStateManager().ExcludeDevices(MEDIA_INPUT_DEVICES | CALL_INPUT_DEVICES, descs);
}

static void UnexcludeIndirectUsbArmInput(const AudioDeviceDescriptor &updatedDesc)
{
    CHECK_AND_RETURN(!updatedDesc.hasPair_ && updatedDesc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET);
    vector<shared_ptr<AudioDeviceDescriptor>> descs{make_shared<AudioDeviceDescriptor>(updatedDesc)};
    AudioStateManager::GetAudioStateManager().UnexcludeDevices(MEDIA_INPUT_DEVICES | CALL_INPUT_DEVICES, descs);
}
#endif

void AudioDeviceCommon::Init(std::shared_ptr<AudioPolicyServerHandler> handler)
{
    audioPolicyServerHandler_ = handler;
}

void AudioDeviceCommon::DeInit()
{
    audioPolicyServerHandler_ = nullptr;
}

bool AudioDeviceCommon::IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
        case DEVICE_TYPE_REMOTE_CAST:
            return true;
        default:
            return false;
    }
}

void AudioDeviceCommon::OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor,
    const AudioStreamDeviceChangeReason reason)
{
    Trace trace("AudioDeviceCommon::OnPreferredOutputDeviceUpdated:" + std::to_string(deviceDescriptor.deviceType_));
    AUDIO_INFO_LOG("Start");

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredOutputDeviceUpdated();
        audioPolicyServerHandler_->SendAudioSessionDeviceChange(reason);
    }
    if (deviceDescriptor.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
        spatialDeviceMap_.insert(make_pair(deviceDescriptor.macAddress_, deviceDescriptor.deviceType_));
    }

    if (deviceDescriptor.macAddress_ !=
        AudioSpatializationService::GetAudioSpatializationService().GetCurrentDeviceAddress()) {
        AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(false);
    }
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(deviceDescriptor.deviceType_);
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
            std::make_shared<AudioDeviceDescriptor>(deviceDescriptor);
    AudioSpatializationService::GetAudioSpatializationService().UpdateCurrentDevice(selectedAudioDevice);
}

void AudioDeviceCommon::OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId)
{
    AUDIO_INFO_LOG("Start");

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredInputDeviceUpdated();
    }
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceCommon::GetPreferredOutputDeviceDescInner(
    AudioRendererInfo &rendererInfo, std::string networkId, const int32_t uid)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    RouterType bypassType = RouterType::ROUTER_TYPE_NONE;
    if (rendererInfo.streamUsage <= STREAM_USAGE_UNKNOWN ||
        rendererInfo.streamUsage > STREAM_USAGE_MAX) {
        AUDIO_WARNING_LOG("Invalid usage[%{public}d], return current device.", rendererInfo.streamUsage);
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice());
        deviceList.push_back(devDesc);
        return deviceList;
    }
    if (networkId == LOCAL_NETWORK_ID) {
        auto preferredType = AudioPolicyUtils::GetInstance().GetPreferredTypeByStreamUsage(rendererInfo.streamUsage);
        if (preferredType == AUDIO_CALL_RENDER && uid >= 0) {
            bypassType = RouterType::ROUTER_TYPE_USER_SELECT;
            std::shared_ptr<AudioDeviceDescriptor> preferredDevice =
                AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDeviceForUid(uid);
            CHECK_AND_RETURN_RET_LOG(preferredDevice != nullptr, deviceList, "preferredDevice is nullptr.");
            if (preferredDevice->deviceId_ != 0) {
                deviceList.push_back(preferredDevice);
                return deviceList;
            }
        }
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            audioRouterCenter_.FetchOutputDevices(rendererInfo.streamUsage,
                -1, "GetPreferredOutputDeviceDescInner", bypassType);
        for (size_t i = 0; i < descs.size(); i++) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*descs[i]);
            deviceList.push_back(devDesc);
        }

        FetchDeviceInfo info = { rendererInfo.streamUsage, rendererInfo.streamUsage, -1,
            bypassType, PIPE_TYPE_OUT_NORMAL, PRIVACY_TYPE_PUBLIC };
        info.caller = "GetPreferredOutputDeviceDescInner";
        descs = audioRouterCenter_.FetchDupDevices(info);
        for (size_t i = 0; i < descs.size(); i++) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*descs[i]);
            deviceList.push_back(devDesc);
        }
    } else {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteRenderDevices();
        for (const auto &desc : descs) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
            deviceList.push_back(devDesc);
        }
    }

    return deviceList;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceCommon::GetPreferredInputDeviceDescInner(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (captureInfo.sourceType <= SOURCE_TYPE_INVALID ||
        captureInfo.sourceType > SOURCE_TYPE_MAX) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentInputDevice());
        deviceList.push_back(devDesc);
        return deviceList;
    }

    if (captureInfo.sourceType == SOURCE_TYPE_WAKEUP) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
        devDesc->networkId_ = LOCAL_NETWORK_ID;
        deviceList.push_back(devDesc);
        return deviceList;
    }

    if (networkId == LOCAL_NETWORK_ID) {
        std::shared_ptr<AudioDeviceDescriptor> desc = audioRouterCenter_.FetchInputDevice(captureInfo.sourceType, -1);
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, deviceList, "desc is nullptr.");
        if (desc->deviceType_ == DEVICE_TYPE_NONE && (captureInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE ||
            captureInfo.sourceType == SOURCE_TYPE_REMOTE_CAST)) {
            desc->deviceType_ = DEVICE_TYPE_INVALID;
            desc->deviceRole_ = INPUT_DEVICE;
        }
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
        deviceList.push_back(devDesc);
    } else {
        vector<shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteCaptureDevices();
        for (const auto &desc : descs) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
            deviceList.push_back(devDesc);
        }
    }

    return deviceList;
}

int32_t AudioDeviceCommon::GetPreferredOutputStreamTypeInner(StreamUsage streamUsage, DeviceType deviceType,
    int32_t flags, std::string &networkId, AudioSamplingRate &samplingRate, bool isFirstCreate)
{
    AUDIO_INFO_LOG("Not support, should use AudioPipeSelector");
    return flags;
}

int32_t AudioDeviceCommon::GetPreferredInputStreamTypeInner(SourceType sourceType, DeviceType deviceType,
    int32_t flags, const std::string &networkId, const AudioSamplingRate &samplingRate)
{
    AUDIO_INFO_LOG("Not support, should use AudioPipeSelector");
    return flags;
}

void AudioDeviceCommon::UpdateDeviceInfo(AudioDeviceDescriptor &deviceInfo,
    const std::shared_ptr<AudioDeviceDescriptor> &desc,
    bool hasBTPermission, bool hasSystemPermission)
{
    deviceInfo.deviceType_ = desc->deviceType_;
    deviceInfo.deviceRole_ = desc->deviceRole_;
    deviceInfo.deviceId_ = desc->deviceId_;
    deviceInfo.channelMasks_ = desc->channelMasks_;
    deviceInfo.channelIndexMasks_ = desc->channelIndexMasks_;
    deviceInfo.displayName_ = desc->displayName_;
    deviceInfo.model_ = desc->model_;
    deviceInfo.connectState_ = desc->connectState_;

    if (deviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        deviceInfo.a2dpOffloadFlag_ = audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
    }

    if (hasBTPermission) {
        deviceInfo.deviceName_ = desc->deviceName_;
        deviceInfo.macAddress_ = desc->macAddress_;
        deviceInfo.deviceCategory_ = desc->deviceCategory_;
    } else {
        deviceInfo.deviceName_ = "";
        deviceInfo.macAddress_ = "";
        deviceInfo.deviceCategory_ = CATEGORY_DEFAULT;
    }

    deviceInfo.isLowLatencyDevice_ = HasLowLatencyCapability(deviceInfo.deviceType_,
        desc->networkId_ != LOCAL_NETWORK_ID);

    if (hasSystemPermission) {
        deviceInfo.networkId_ = desc->networkId_;
        deviceInfo.volumeGroupId_ = desc->volumeGroupId_;
        deviceInfo.interruptGroupId_ = desc->interruptGroupId_;
    } else {
        deviceInfo.networkId_ = "";
        deviceInfo.volumeGroupId_ = GROUP_ID_NONE;
        deviceInfo.interruptGroupId_ = GROUP_ID_NONE;
    }
    deviceInfo.audioStreamInfo_ = desc->audioStreamInfo_;
    deviceInfo.capabilities_ = desc->capabilities_;
}

int32_t AudioDeviceCommon::DeviceParamsCheck(DeviceRole targetRole,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) const
{
    size_t targetSize = audioDeviceDescriptors.size();
    CheckAndWriteDeviceChangeExceptionEvent(targetSize == 1,
        AudioStreamDeviceChangeReason::OVERRODE, audioDeviceDescriptors[0]->deviceType_,
        audioDeviceDescriptors[0]->deviceRole_, ERR_INVALID_OPERATION, "device params check: size error");
    CHECK_AND_RETURN_RET_LOG(targetSize == 1, ERR_INVALID_OPERATION,
        "Device error: size[%{public}zu]", targetSize);

    bool isDeviceTypeCorrect = false;
    if (targetRole == DeviceRole::OUTPUT_DEVICE) {
        isDeviceTypeCorrect = IsOutputDevice(audioDeviceDescriptors[0]->deviceType_,
            audioDeviceDescriptors[0]->deviceRole_) && IsDeviceConnected(audioDeviceDescriptors[0]);
    } else if (targetRole == DeviceRole::INPUT_DEVICE) {
        isDeviceTypeCorrect = IsInputDevice(audioDeviceDescriptors[0]->deviceType_,
            audioDeviceDescriptors[0]->deviceRole_) && IsDeviceConnected(audioDeviceDescriptors[0]);
    }

    CheckAndWriteDeviceChangeExceptionEvent(audioDeviceDescriptors[0]->deviceRole_ == targetRole &&
        isDeviceTypeCorrect, AudioStreamDeviceChangeReason::OVERRODE, audioDeviceDescriptors[0]->deviceType_,
        audioDeviceDescriptors[0]->deviceRole_, ERR_INVALID_OPERATION, "device params check: role error");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == targetRole && isDeviceTypeCorrect,
        ERR_INVALID_OPERATION, "Device error: size[%{public}zu] deviceRole[%{public}d] isDeviceCorrect[%{public}d]",
        targetSize, static_cast<int32_t>(audioDeviceDescriptors[0]->deviceRole_), isDeviceTypeCorrect);
    return SUCCESS;
}

void AudioDeviceCommon::UpdateConnectedDevicesWhenConnecting(const AudioDeviceDescriptor &updatedDesc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    AUDIO_INFO_LOG("UpdateConnectedDevicesWhenConnecting In, deviceType: %{public}d", updatedDesc.deviceType_);
    if (IsOutputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        UpdateConnectedDevicesWhenConnectingForOutputDevice(updatedDesc, descForCb);
    }
    if (IsInputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        UpdateConnectedDevicesWhenConnectingForInputDevice(updatedDesc, descForCb);
#ifdef EXCLUDE_INDIRECT_USB_INPUT_DEVICE
        ExcludeIndirectUsbArmInput(updatedDesc);
#endif
    }
}

void AudioDeviceCommon::RemoveOfflineDevice(const AudioDeviceDescriptor& updatedDesc)
{
    if (IsOutputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        audioAffinityManager_.RemoveOfflineRendererDevice(updatedDesc);
    }
    if (IsInputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        audioAffinityManager_.RemoveOfflineCapturerDevice(updatedDesc);
    }
}

void AudioDeviceCommon::ClearPreferredDevices(const vector<shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    for (const auto& desc : descForCb) {
        if (audioStateManager_.GetPreferredMediaRenderDevice() != nullptr &&
            desc->IsSameDeviceDesc(*audioStateManager_.GetPreferredMediaRenderDevice())) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER,
                std::make_shared<AudioDeviceDescriptor>());
        }
        if (audioStateManager_.GetPreferredCallRenderDevice() != nullptr &&
            desc->IsSameDeviceDesc(*audioStateManager_.GetPreferredCallRenderDevice())) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                std::make_shared<AudioDeviceDescriptor>(), CLEAR_UID, "UpdateConnectedDevicesWhenDisconnecting");
        }
        if (audioStateManager_.GetPreferredCallCaptureDevice() != nullptr &&
            desc->IsSameDeviceDesc(*audioStateManager_.GetPreferredCallCaptureDevice())) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE,
                std::make_shared<AudioDeviceDescriptor>());
        }
        if (audioStateManager_.GetPreferredRecordCaptureDevice() != nullptr &&
            desc->IsSameDeviceDesc(*audioStateManager_.GetPreferredRecordCaptureDevice())) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE,
                std::make_shared<AudioDeviceDescriptor>());
        }
    }
}

void AudioDeviceCommon::UpdateConnectedDevicesWhenDisconnecting(const AudioDeviceDescriptor& updatedDesc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb, bool updateVolume)
{
    RemoveOfflineDevice(updatedDesc);
    AUDIO_INFO_LOG("[%{public}s], devType:[%{public}d]", __func__, updatedDesc.deviceType_);

    // Remember the disconnected device descriptor and remove it
    audioDeviceManager_.GetAllConnectedDeviceByType(updatedDesc.networkId_, updatedDesc.deviceType_,
        updatedDesc.macAddress_, updatedDesc.deviceRole_, descForCb);
    ClearPreferredDevices(descForCb);

    audioConnectedDevice_.DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_,
        updatedDesc.macAddress_, updatedDesc.deviceRole_);

    // reset disconnected device info in stream
    if (IsOutputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        streamCollector_.ResetRendererStreamDeviceInfo(updatedDesc);
    }
    if (IsInputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        streamCollector_.ResetCapturerStreamDeviceInfo(updatedDesc);
#ifdef EXCLUDE_INDIRECT_USB_INPUT_DEVICE
        UnexcludeIndirectUsbArmInput(updatedDesc);
#endif
    }

    std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(updatedDesc);
    CHECK_AND_RETURN_LOG(devDesc != nullptr, "Create device descriptor failed");
    audioDeviceManager_.RemoveNewDevice(devDesc);
    audioMicrophoneDescriptor_.RemoveMicrophoneDescriptor(devDesc);
    if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceMacAddr() == updatedDesc.macAddress_) {
        audioA2dpOffloadFlag_.SetA2dpOffloadFlag(NO_A2DP_DEVICE);
    }
    CHECK_AND_RETURN_LOG(updateVolume, "no need to updateVolume");
    AudioAdapterManager::GetInstance().UpdateVolumeWhenDeviceDisconnect(devDesc);
}

void AudioDeviceCommon::UpdateConnectedDevicesWhenConnectingForOutputDevice(
    const AudioDeviceDescriptor &updatedDesc, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>(updatedDesc);
    audioDescriptor->deviceRole_ = OUTPUT_DEVICE;
    // Use speaker streaminfo for all output devices cap
    if (updatedDesc.deviceType_ != DEVICE_TYPE_HEARING_AID &&
        updatedDesc.deviceType_ != DEVICE_TYPE_USB_ARM_HEADSET) {
        auto itr = audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_SPEAKER);
        if (itr != nullptr) {
            audioDescriptor->SetDeviceCapability(itr->audioStreamInfo_, 0);
        }
    }
    bool wasVirtualConnected = audioDeviceManager_.IsVirtualConnectedDevice(audioDescriptor);
    if (!wasVirtualConnected) {
        audioDescriptor->deviceId_ = AudioPolicyUtils::startDeviceId++;
    } else {
        audioDeviceManager_.UpdateDeviceDescDeviceId(audioDescriptor);
        CheckAndNotifyUserSelectedDevice(audioDescriptor);
        audioDeviceManager_.UpdateVirtualDevices(audioDescriptor, true);
    }
    descForCb.push_back(audioDescriptor);
    AudioPolicyUtils::GetInstance().UpdateDisplayName(audioDescriptor);
    AudioAdapterManager::GetInstance().QueryDeviceVolumeBehavior(audioDescriptor);
    audioConnectedDevice_.AddConnectedDevice(audioDescriptor);
    std::vector<shared_ptr<AudioDeviceDescriptor>> unexcludedDevice = {
        make_shared<AudioDeviceDescriptor>(updatedDesc)};
    AudioPolicyUtils::GetInstance().UnexcludeOutputDevices(D_ALL_DEVICES, unexcludedDevice);
    audioDeviceManager_.AddNewDevice(audioDescriptor);
    AudioAdapterManager::GetInstance().UpdateVolumeWhenDeviceConnect(audioDescriptor);
    if (updatedDesc.connectState_ == VIRTUAL_CONNECTED) {
        AUDIO_INFO_LOG("The device is virtual device, no need to update preferred device");
        return; // No need to update preferred device for virtual device
    }
    DeviceUsage usage = audioDeviceManager_.GetDeviceUsage(updatedDesc);
    if (NeedClearPreferredMediaRenderer(audioStateManager_.GetPreferredMediaRenderDevice(), audioDescriptor,
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1,
            "UpdateConnectedDevicesWhenConnectingForOutputDevice_1", ROUTER_TYPE_USER_SELECT), usage)) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER,
            std::make_shared<AudioDeviceDescriptor>());
    }
    if (audioDescriptor->networkId_ == LOCAL_NETWORK_ID && audioDescriptor->IsSameDeviceDesc(
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION, -1,
        "UpdateConnectedDevicesWhenConnectingForOutputDevice_2",
        ROUTER_TYPE_USER_SELECT).front()) && (usage & VOICE) == VOICE) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), CLEAR_UID,
            "UpdateConnectedDevicesWhenConnectingForOutputDevice");
    }
}

bool AudioDeviceCommon::NeedClearPreferredMediaRenderer(const std::shared_ptr<AudioDeviceDescriptor> &preferred,
    const std::shared_ptr<AudioDeviceDescriptor> &updated,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &fetched, const DeviceUsage usage) const
{
    CHECK_AND_RETURN_RET(preferred != nullptr, false);
    if (preferred->deviceType_ == DEVICE_TYPE_NONE) {
        return false;
    }

    CHECK_AND_RETURN_RET(updated != nullptr, false);
    if (updated->networkId_ != LOCAL_NETWORK_ID) {
        return false;
    }

    if ((usage & MEDIA) != MEDIA) {
        return false;
    }

    CHECK_AND_RETURN_RET(!fetched.empty(), false);
    const auto &frontDesc = fetched.front();

    CHECK_AND_RETURN_RET(frontDesc != nullptr, false);
    return updated->IsSameDeviceDescPtr(frontDesc);
}

void AudioDeviceCommon::UpdateConnectedDevicesWhenConnectingForInputDevice(
    const AudioDeviceDescriptor &updatedDesc, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>(updatedDesc);
    audioDescriptor->deviceRole_ = INPUT_DEVICE;
    // Use mic streaminfo for all input devices cap
    if (updatedDesc.deviceType_ != DEVICE_TYPE_USB_ARM_HEADSET) {
        auto itr = audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_MIC);
        if (itr != nullptr) {
            audioDescriptor->SetDeviceCapability(itr->audioStreamInfo_, 0);
        }
    }
    bool wasVirtualConnected = audioDeviceManager_.IsVirtualConnectedDevice(audioDescriptor);
    if (!wasVirtualConnected) {
        audioDescriptor->deviceId_ = AudioPolicyUtils::startDeviceId++;
    } else {
        audioDeviceManager_.UpdateDeviceDescDeviceId(audioDescriptor);
        audioDeviceManager_.UpdateVirtualDevices(audioDescriptor, true);
    }
    descForCb.push_back(audioDescriptor);
    AudioPolicyUtils::GetInstance().UpdateDisplayName(audioDescriptor);
    audioConnectedDevice_.AddConnectedDevice(audioDescriptor);
    audioMicrophoneDescriptor_.AddMicrophoneDescriptor(audioDescriptor);
    audioDeviceManager_.AddNewDevice(audioDescriptor);
    if (updatedDesc.connectState_ == VIRTUAL_CONNECTED) {
        return;
    }
    if (audioDescriptor->deviceCategory_ != BT_UNWEAR_HEADPHONE && audioDescriptor->deviceCategory_ != BT_WATCH) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE,
            std::make_shared<AudioDeviceDescriptor>());
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE,
            std::make_shared<AudioDeviceDescriptor>());
    }
}

bool AudioDeviceCommon::IsFastFromA2dpToA2dp(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    if (rendererChangeInfo->outputDeviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
        rendererChangeInfo->rendererInfo.originalFlag == AUDIO_FLAG_MMAP &&
        rendererChangeInfo->outputDeviceInfo.deviceId_ != desc->deviceId_) {
        TriggerRecreateRendererStreamCallback(rendererChangeInfo->callerPid, rendererChangeInfo->sessionId,
            AUDIO_OUTPUT_FLAG_FAST, reason);
        AUDIO_INFO_LOG("Switch fast stream from a2dp to a2dp");
        return true;
    }
    return false;
}

vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceCommon::GetDeviceDescriptorInner(
    std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    if (VolumeUtils::IsPCVolumeEnable() && !isFirstScreenOn_) {
        descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
    } else {
        descs = audioRouterCenter_.FetchOutputDevices(rendererChangeInfo->rendererInfo.streamUsage,
            rendererChangeInfo->clientUID, "GetDeviceDescriptorInner");
    }
    return descs;
}

bool AudioDeviceCommon::IsRendererStreamRunning(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo)
{
    StreamUsage usage = rendererChangeInfo->rendererInfo.streamUsage;
    RendererState rendererState = rendererChangeInfo->rendererState;
    if ((usage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
        audioSceneManager_.GetAudioScene(true) != AUDIO_SCENE_PHONE_CALL) ||
        (usage != STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
            (rendererState != RENDERER_RUNNING && !rendererChangeInfo->prerunningState))) {
        return false;
    }
    return true;
}

void AudioDeviceCommon::TriggerRecreateRendererStreamCallback(int32_t callerPid, int32_t sessionId,
    int32_t streamFlag, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::TriggerRecreateRendererStreamCallback");
    AUDIO_INFO_LOG("Trigger recreate renderer stream, pid: %{public}d, sessionId: %{public}d, flag: %{public}d",
        callerPid, sessionId, streamFlag);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRecreateRendererStreamEvent(callerPid, sessionId, streamFlag, reason);
    } else {
        AUDIO_WARNING_LOG("No audio policy server handler");
    }
}

bool AudioDeviceCommon::IsRingDualToneOnPrimarySpeaker(const vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    const int32_t sessionId)
{
    if (descs.size() !=  AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT) {
        return false;
    }
    if (AudioPolicyUtils::GetInstance().GetSinkName(*descs.front(), sessionId) != PRIMARY_SPEAKER) {
        return false;
    }
    if (AudioPolicyUtils::GetInstance().GetSinkName(*descs.back(), sessionId) != PRIMARY_SPEAKER) {
        return false;
    }
    if (descs.back()->deviceType_ != DEVICE_TYPE_SPEAKER) {
        return false;
    }
    AUDIO_INFO_LOG("ring dual tone on primary speaker and mute music.");
    return true;
}

void AudioDeviceCommon::ClearRingMuteWhenCallStart(bool pre, bool after)
{
    CHECK_AND_RETURN_LOG(pre == true && after == false, "ringdual not cancel by call");
    AUDIO_INFO_LOG("disable primary speaker dual tone when call start and ring not over");
    for (std::pair<uint32_t, AudioStreamType> stream : streamsWhenRingDualOnPrimarySpeaker_) {
        AudioVolume::GetInstance()->SetStreamVolumeMute(stream.first, false);
    }
    streamsWhenRingDualOnPrimarySpeaker_.clear();
    audioPolicyManager_.SetStreamMute(STREAM_MUSIC, false, STREAM_USAGE_MUSIC);
}

void AudioDeviceCommon::WriteInputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const AudioStreamDeviceChangeReason reason)
{
    int64_t timeStamp = AudioPolicyUtils::GetInstance().GetCurrentTimeMS();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_ROUTE_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("REASON", static_cast<int32_t>(reason));
    bean->Add("TIMESTAMP", static_cast<uint64_t>(timeStamp));
    bean->Add("DEVICE_TYPE_BEFORE_CHANGE", audioActiveDevice_.GetCurrentInputDeviceType());
    bean->Add("DEVICE_TYPE_AFTER_CHANGE", desc->deviceType_);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

std::vector<SourceOutput> AudioDeviceCommon::FilterSourceOutputs(int32_t sessionId)
{
    std::vector<SourceOutput> targetSourceOutputs = {};
    std::vector<SourceOutput> sourceOutputs = GetSourceOutputs();

    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        AUDIO_DEBUG_LOG("sourceOutput[%{public}zu]:%{public}s", i, PrintSourceOutput(sourceOutputs[i]).c_str());
        if (sessionId == sourceOutputs[i].streamId) {
            targetSourceOutputs.push_back(sourceOutputs[i]);
        }
    }
    return targetSourceOutputs;
}

int32_t AudioDeviceCommon::ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
    ConnectState connectState, bool isVrSupported)
{
    if (handleFlag && (connectState != DEACTIVE_CONNECTED || !isVrSupported)) {
        return SUCCESS;
    }
    return Bluetooth::AudioHfpManager::HandleScoWithRecongnition(handleFlag);
}

void AudioDeviceCommon::NotifyDistributedOutputChange(const AudioDeviceDescriptor &deviceDesc)
{
    bool isDistOld = IsDistributedOutput(audioActiveDevice_.GetCurrentOutputDevice());
    bool isDistNew = IsDistributedOutput(deviceDesc);
    AUDIO_INFO_LOG("Check Distributed Output Change[%{public}d-->%{public}d]", isDistOld, isDistNew);
    if (isDistOld != isDistNew) {
        auto ret = audioRouterCenter_.NotifyDistributedOutputChange(isDistNew);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "NotifyDistributedOutputChange Failed. ret=%{public}d", ret);
    }
}

int32_t AudioDeviceCommon::MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
    std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start for [%{public}zu] sink-inputs", sinkInputIds.size());
    // check
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_,
        ERR_INVALID_OPERATION, "failed: not a local device.");

    // start move.
    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(sinkInputIds[i].streamId, pipeType);
        std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(localDeviceDescriptor->deviceType_,
            pipeType);
        if (sinkName == BLUETOOTH_SPEAKER) {
            std::string activePort = BLUETOOTH_SPEAKER;
            audioPolicyManager_.SuspendAudioDevice(activePort, false);
        }
        AUDIO_INFO_LOG("move for session [%{public}d], portName %{public}s pipeType %{public}d",
            sinkInputIds[i].streamId, sinkName.c_str(), pipeType);
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, sinkName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, LOCAL_NETWORK_ID, sinkInputIds[i].pid);
    }

    isCurrentRemoteRenderer = false;
    return SUCCESS;
}

int32_t AudioDeviceCommon::OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    // open the test device. We should open it when device is online.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);
    AudioModuleInfo remoteDeviceInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
        deviceRole, deviceType);

    auto ret = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_REMOTE, networkId);
    if (ret) {
        AUDIO_ERR_LOG("load adapter fail");
    }
    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleName, remoteDeviceInfo);

    // If device already in list, remove it else do not modify the list.
    audioConnectedDevice_.DelConnectedDevice(networkId, deviceType);
    AudioPolicyUtils::GetInstance().UpdateDisplayName(remoteDeviceDescriptor);
    audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioMicrophoneDescriptor_.AddMicrophoneDescriptor(remoteDeviceDescriptor);
    return SUCCESS;
}

void AudioDeviceCommon::CheckAndNotifyUserSelectedDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    shared_ptr<AudioDeviceDescriptor> userSelectedMediaDevice = audioStateManager_.GetPreferredMediaRenderDevice();
    shared_ptr<AudioDeviceDescriptor> userSelectedCallDevice = audioStateManager_.GetPreferredCallRenderDevice();
    if (userSelectedMediaDevice != nullptr
        && userSelectedMediaDevice->connectState_ == VIRTUAL_CONNECTED
        && deviceDescriptor->IsSameDeviceDesc(*userSelectedMediaDevice)) {
        audioActiveDevice_.NotifyUserSelectionEventToBt(deviceDescriptor);
    }
    if (userSelectedCallDevice != nullptr
        && userSelectedCallDevice->connectState_ == VIRTUAL_CONNECTED
        && deviceDescriptor->IsSameDeviceDesc(*userSelectedCallDevice)) {
        audioActiveDevice_.NotifyUserSelectionEventToBt(deviceDescriptor);
    }
}

bool AudioDeviceCommon::HasLowLatencyCapability(DeviceType deviceType, bool isRemote)
{
    // Distributed devices are low latency devices
    if (isRemote) {
        return true;
    }

    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_DP:
        case DeviceType::DEVICE_TYPE_ACCESSORY:
            return true;

        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            return false;
        default:
            return false;
    }
}

DeviceType AudioDeviceCommon::GetSpatialDeviceType(const std::string& macAddress)
{
    auto it = spatialDeviceMap_.find(macAddress);
    DeviceType spatialDevice;
    if (it != spatialDeviceMap_.end()) {
        spatialDevice = it->second;
    } else {
        AUDIO_DEBUG_LOG("we can't find the spatialDevice of hvs");
        spatialDevice = DEVICE_TYPE_NONE;
    }
    AUDIO_INFO_LOG("Update a2dpOffloadFlag spatialDevice: %{public}d", spatialDevice);
    return spatialDevice;
}

bool AudioDeviceCommon::IsRingOverPlayback(AudioMode &mode, RendererState rendererState)
{
    if (mode != AUDIO_MODE_PLAYBACK) {
        return false;
    }
    if (rendererState != RENDERER_STOPPED && rendererState != RENDERER_RELEASED &&
        rendererState != RENDERER_PAUSED) {
        return false;
    }
    return true;
}

bool AudioDeviceCommon::IsDeviceConnected(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptors) const
{
    return audioDeviceManager_.IsDeviceConnected(audioDeviceDescriptors);
}

bool AudioDeviceCommon::IsSameDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, AudioDeviceDescriptor &deviceInfo)
{
    if (desc->networkId_ == deviceInfo.networkId_ && desc->deviceType_ == deviceInfo.deviceType_ &&
        desc->macAddress_ == deviceInfo.macAddress_ && desc->connectState_ == deviceInfo.connectState_) {
        if (deviceInfo.IsAudioDeviceDescriptor()) {
            return true;
        }
        BluetoothOffloadState state = audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
            // switch to A2dp
            ((deviceInfo.a2dpOffloadFlag_ == A2DP_OFFLOAD && state != A2DP_OFFLOAD) ||
            // switch to A2dp offload
            (deviceInfo.a2dpOffloadFlag_ != A2DP_OFFLOAD && state == A2DP_OFFLOAD))) {
            return false;
        }
        if (IsUsb(desc->deviceType_)) {
            return desc->deviceRole_ == deviceInfo.deviceRole_;
        }
        return true;
    } else {
        return false;
    }
}

bool AudioDeviceCommon::IsSameDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const AudioDeviceDescriptor &deviceDesc)
{
    if (desc->networkId_ == deviceDesc.networkId_ && desc->deviceType_ == deviceDesc.deviceType_ &&
        desc->macAddress_ == deviceDesc.macAddress_ && desc->connectState_ == deviceDesc.connectState_ &&
        (!IsUsb(desc->deviceType_) || desc->deviceRole_ == deviceDesc.deviceRole_)) {
        return true;
    } else {
        return false;
    }
}

std::vector<SourceOutput> AudioDeviceCommon::GetSourceOutputs()
{
    std::vector<SourceOutput> sourceOutputs;
    {
        std::unordered_map<std::string, AudioIOHandle> mapCopy = AudioIOHandleMap::GetInstance().GetCopy();
        if (std::any_of(mapCopy.cbegin(), mapCopy.cend(), [](const auto &pair) {
                return std::find(SourceNames.cbegin(), SourceNames.cend(), pair.first) != SourceNames.cend();
            })) {
            sourceOutputs = audioPolicyManager_.GetAllSourceOutputs();
        }
    }
    return sourceOutputs;
}

void AudioDeviceCommon::ClientDiedDisconnectScoNormal()
{
    bool isRecord = streamCollector_.HasRunningNormalCapturerStream(DEVICE_TYPE_BLUETOOTH_SCO);
    AudioScene scene = audioSceneManager_.GetAudioScene(true);
    Bluetooth::AudioHfpManager::UpdateAudioScene(scene, isRecord);
}

void AudioDeviceCommon::ClientDiedDisconnectScoRecognition()
{
    bool hasRunningRecognitionCapturerStream = pipeManager_->HasRunningRecognitionCapturerStream();
    if (hasRunningRecognitionCapturerStream) {
        return;
    }
    audioStateManager_.SetPreferredRecognitionCaptureDevice(make_shared<AudioDeviceDescriptor>());
    Bluetooth::AudioHfpManager::HandleScoWithRecongnition(false);
}

void AudioDeviceCommon::GetA2dpModuleInfo(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo,
    SourceType sourceType)
{
    uint32_t bufferSize = audioStreamInfo.samplingRate *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(audioStreamInfo.format) *
        audioStreamInfo.channels / BT_BUFFER_ADJUSTMENT_FACTOR;
    AUDIO_INFO_LOG("a2dp rate: %{public}d, format: %{public}d, channel: %{public}d",
        audioStreamInfo.samplingRate, audioStreamInfo.format, audioStreamInfo.channels);
    moduleInfo.channels = to_string(audioStreamInfo.channels);
    moduleInfo.rate = to_string(audioStreamInfo.samplingRate);
    moduleInfo.format = AudioPolicyUtils::GetInstance().ConvertToHDIAudioFormat(audioStreamInfo.format);
    moduleInfo.bufferSize = to_string(bufferSize);
    if (moduleInfo.role != "source") {
        moduleInfo.renderInIdleState = "1";
        moduleInfo.sinkLatency = "0";
    }
    audioEcManager_.UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
}

int32_t AudioDeviceCommon::LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
    std::string networkID, std::string sinkName, SourceType sourceType)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED,
        "A2dp module is not exist in the configuration file");

    // not load bt_a2dp_fast and bt_hdap, maybe need fix
    int32_t loadRet = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_a2dp");
    if (loadRet) {
        AUDIO_ERR_LOG("load adapter fail");
    }
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "source" ? INPUT_DEVICE : OUTPUT_DEVICE;
        DeviceRole deviceRole = deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load a2dp module [%{public}s], load role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            // a2dp device connects for the first time
            GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
            uint32_t temp = 0;
            AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, temp);
            CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_OPERATION_FAILED,
                "OpenAudioPort failed %{public}d", ioHandle);
            audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);
        } else {
            // At least one a2dp device is already connected. A new a2dp device is connecting.
            // Need to reload a2dp module when switching to a2dp device.
            int32_t result = ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkID, sinkName,
                sourceType);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "ReloadA2dpAudioPort failed %{public}d", result);
        }
    }

    return SUCCESS;
}

int32_t AudioDeviceCommon::ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, DeviceType deviceType,
    const AudioStreamInfo& audioStreamInfo, std::string networkID, std::string sinkName,
    SourceType sourceType)
{
    AUDIO_INFO_LOG("switch device from a2dp to another a2dp, reload a2dp module");
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        audioIOHandleMap_.MuteDefaultSinkPort(networkID, sinkName);
    }

    // Firstly, unload the existing a2dp sink or source.
    std::string portName = BLUETOOTH_SPEAKER;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        portName = BLUETOOTH_MIC;
    }
    AudioIOHandle activateDeviceIOHandle;
    audioIOHandleMap_.GetModuleIdByKey(portName, activateDeviceIOHandle);
    int32_t result = audioPolicyManager_.CloseAudioPort(activateDeviceIOHandle);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
        "CloseAudioPort failed %{public}d", result);

    // Load a2dp sink or source module again with the configuration of active a2dp device.
    GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
    uint32_t temp = 0;
    AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, temp);
    CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_OPERATION_FAILED,
        "OpenAudioPort failed %{public}d", ioHandle);
    audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);
    return SUCCESS;
}

int32_t AudioDeviceCommon::RingToneVoiceControl(const InternalDeviceType &deviceType)
{
    int32_t curVoiceCallLevel = audioPolicyManager_.GetSystemVolumeLevel(STREAM_VOICE_CALL);
    float curVoiceCallDb = audioPolicyManager_.GetSystemVolumeInDb(STREAM_VOICE_CALL, curVoiceCallLevel, deviceType);
    int32_t curRingToneLevel = audioPolicyManager_.GetSystemVolumeLevel(STREAM_RING);
    float curRingToneDb = audioPolicyManager_.GetSystemVolumeInDb(STREAM_RING, curRingToneLevel, deviceType);
    int32_t maxVoiceCall = audioPolicyManager_.GetMaxVolumeLevel(STREAM_VOICE_CALL);
    int32_t maxRingTone = audioPolicyManager_.GetMaxVolumeLevel(STREAM_RING);
    float curVoiceRingMixDb = curVoiceCallDb * curRingToneDb;
    float minMixDbDefault = audioPolicyManager_.GetSystemVolumeInDb(STREAM_VOICE_CALL,
        maxVoiceCall * VOLUME_LEVEL_MIN_SIZE / VOLUME_LEVEL_MAX_SIZE, deviceType) *
        audioPolicyManager_.GetSystemVolumeInDb(STREAM_RING, maxRingTone, deviceType);
    float maxMixDbDefault = audioPolicyManager_.GetSystemVolumeInDb(STREAM_VOICE_CALL,
        maxVoiceCall * VOLUME_LEVEL_MID_SIZE / VOLUME_LEVEL_MAX_SIZE, deviceType) *
        audioPolicyManager_.GetSystemVolumeInDb(STREAM_RING, maxRingTone, deviceType);

    if (curVoiceCallLevel > VOLUME_LEVEL_DEFAULT) {
        for (int i = 0; i < DEFAULT_ADJUST_TIMES; i++) {
            if (curVoiceRingMixDb < minMixDbDefault && curRingToneLevel <= VOLUME_LEVEL_MAX_SIZE) {
                curRingToneLevel++;
                curRingToneDb = audioPolicyManager_.GetSystemVolumeInDb(STREAM_RING, curRingToneLevel, deviceType);
                curVoiceRingMixDb = curVoiceCallDb * curRingToneDb;
            } else if (curVoiceRingMixDb > maxMixDbDefault && curRingToneLevel > 0) {
                curRingToneLevel--;
                curRingToneDb = audioPolicyManager_.GetSystemVolumeInDb(STREAM_RING, curRingToneLevel, deviceType);
                curVoiceRingMixDb = curVoiceCallDb * curRingToneDb;
            } else {
                break;
            }
        }
    }
    return curRingToneLevel;
}

void AudioDeviceCommon::SetFirstScreenOn()
{
    isFirstScreenOn_ = true;
}

int32_t AudioDeviceCommon::SetVirtualCall(pid_t uid, const bool isVirtual)
{
    return Bluetooth::AudioHfpManager::SetVirtualCall(uid, isVirtual);
}

bool AudioDeviceCommon::GetVirtualCall(pid_t uid)
{
    return Bluetooth::AudioHfpManager::IsVirtualCall();
}
}
}
