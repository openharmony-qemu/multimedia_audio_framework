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
    std::string(FILE_SOURCE)
};

static bool IsDistributedOutput(const AudioDeviceDescriptor &desc)
{
    return desc.deviceType_ == DEVICE_TYPE_SPEAKER && desc.networkId_ != LOCAL_NETWORK_ID;
}

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
    AudioSpatializationService::GetAudioSpatializationService().UpdateCurrentDevice(deviceDescriptor.macAddress_);
}

void AudioDeviceCommon::OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId)
{
    AUDIO_INFO_LOG("Start");

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredInputDeviceUpdated();
    }
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceCommon::GetPreferredOutputDeviceDescInner(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (rendererInfo.streamUsage <= STREAM_USAGE_UNKNOWN ||
        rendererInfo.streamUsage > STREAM_USAGE_MAX) {
        AUDIO_WARNING_LOG("Invalid usage[%{public}d], return current device.", rendererInfo.streamUsage);
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice());
        deviceList.push_back(devDesc);
        return deviceList;
    }
    if (networkId == LOCAL_NETWORK_ID) {
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            audioRouterCenter_.FetchOutputDevices(rendererInfo.streamUsage,
                -1, "GetPreferredOutputDeviceDescInner");
        for (size_t i = 0; i < descs.size(); i++) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*descs[i]);
            deviceList.push_back(devDesc);
        }
    } else {
        vector<shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteRenderDevices();
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

void AudioDeviceCommon::UpdateConnectedDevicesWhenDisconnecting(const AudioDeviceDescriptor& updatedDesc,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    RemoveOfflineDevice(updatedDesc);
    AUDIO_INFO_LOG("[%{public}s], devType:[%{public}d]", __func__, updatedDesc.deviceType_);

    // Remember the disconnected device descriptor and remove it
    audioDeviceManager_.GetAllConnectedDeviceByType(updatedDesc.networkId_, updatedDesc.deviceType_,
        updatedDesc.macAddress_, updatedDesc.deviceRole_, descForCb);
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

    AudioPolicyUtils::GetInstance().UnexcludeOutputDevices(descForCb);

    audioConnectedDevice_.DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_,
        updatedDesc.macAddress_, updatedDesc.deviceRole_);

    // reset disconnected device info in stream
    if (IsOutputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        streamCollector_.ResetRendererStreamDeviceInfo(updatedDesc);
    }
    if (IsInputDevice(updatedDesc.deviceType_, updatedDesc.deviceRole_)) {
        streamCollector_.ResetCapturerStreamDeviceInfo(updatedDesc);
    }

    std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(updatedDesc);
    CHECK_AND_RETURN_LOG(devDesc != nullptr, "Create device descriptor failed");
    audioDeviceManager_.RemoveNewDevice(devDesc);
    audioMicrophoneDescriptor_.RemoveMicrophoneDescriptor(devDesc);
    if (updatedDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceMacAddr() == updatedDesc.macAddress_) {
        audioA2dpOffloadFlag_.SetA2dpOffloadFlag(NO_A2DP_DEVICE);
    }
}

void AudioDeviceCommon::UpdateConnectedDevicesWhenConnectingForOutputDevice(
    const AudioDeviceDescriptor &updatedDesc, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>(updatedDesc);
    audioDescriptor->deviceRole_ = OUTPUT_DEVICE;
    // Use speaker streaminfo for all output devices cap
    if (updatedDesc.deviceType_ != DEVICE_TYPE_HEARING_AID) {
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
    audioConnectedDevice_.AddConnectedDevice(audioDescriptor);
    audioDeviceManager_.AddNewDevice(audioDescriptor);

    if (updatedDesc.connectState_ == VIRTUAL_CONNECTED) {
        AUDIO_INFO_LOG("The device is virtual device, no need to update preferred device");
        return; // No need to update preferred device for virtual device
    }
    DeviceUsage usage = audioDeviceManager_.GetDeviceUsage(updatedDesc);
    if (audioDescriptor->networkId_ == LOCAL_NETWORK_ID && audioDescriptor->IsSameDeviceDesc(
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1,
        "UpdateConnectedDevicesWhenConnectingForOutputDevice_1",
        ROUTER_TYPE_USER_SELECT).front()) && (usage & MEDIA) == MEDIA) {
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

void AudioDeviceCommon::UpdateConnectedDevicesWhenConnectingForInputDevice(
    const AudioDeviceDescriptor &updatedDesc, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>(updatedDesc);
    audioDescriptor->deviceRole_ = INPUT_DEVICE;
    // Use mic streaminfo for all input devices cap
    auto itr = audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_MIC);
    if (itr != nullptr) {
        audioDescriptor->SetDeviceCapability(itr->audioStreamInfo_, 0);
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

void AudioDeviceCommon::UpdateDualToneState(const bool &enable, const int32_t &sessionId)
{
    AUDIO_INFO_LOG("update dual tone state, enable:%{public}d, sessionId:%{public}d", enable, sessionId);
    enableDualHalToneState_ = enable;
    if (enableDualHalToneState_) {
        enableDualHalToneSessionId_ = sessionId;
    }
    Trace trace("AudioDeviceCommon::UpdateDualToneState sessionId:" + std::to_string(sessionId));
    auto ret = AudioServerProxy::GetInstance().UpdateDualToneStateProxy(enable, sessionId);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the dual tone state for sessionId:%{public}d", sessionId);
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

void AudioDeviceCommon::SetDeviceConnectedFlagWhenFetchOutputDevice()
{
    AudioDeviceDescriptor currentActiveDevice = audioActiveDevice_.GetCurrentOutputDevice();
    if (currentActiveDevice.deviceType_ == DEVICE_TYPE_USB_HEADSET ||
        currentActiveDevice.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        AudioServerProxy::GetInstance().SetDeviceConnectedFlag(false);
    }
}

void AudioDeviceCommon::FetchOutputDevice(std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::FetchOutputDevice");
    AUDIO_PRERELEASE_LOGI("Start for %{public}zu stream, connected %{public}s",
        rendererChangeInfos.size(), audioDeviceManager_.GetConnDevicesStr().c_str());
    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    int32_t runningStreamCount = 0;
    bool hasDirectChangeDevice = false;
    std::vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    for (auto &rendererChangeInfo : rendererChangeInfos) {
        if (!IsRendererStreamRunning(rendererChangeInfo) ||
            (audioSceneManager_.GetAudioScene(true) == AUDIO_SCENE_DEFAULT &&
            audioRouterCenter_.isCallRenderRouter(rendererChangeInfo->rendererInfo.streamUsage))) {
            AUDIO_WARNING_LOG("stream %{public}d not running, no need fetch device", rendererChangeInfo->sessionId);
            continue;
        }
        runningStreamCount++;
        SetDeviceConnectedFlagWhenFetchOutputDevice();
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs = GetDeviceDescriptorInner(rendererChangeInfo);
        if (HandleDeviceChangeForFetchOutputDevice(descs.front(), rendererChangeInfo, reason) ==
            ERR_NEED_NOT_SWITCH_DEVICE &&
            !Util::IsRingerOrAlarmerStreamUsage(rendererChangeInfo->rendererInfo.streamUsage)) {
            continue;
        }
        MuteSinkForSwitchBluetoothDevice(rendererChangeInfo, descs, reason);
        std::string encryptMacAddr = GetEncryptAddr(descs.front()->macAddress_);
        if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
            if (IsFastFromA2dpToA2dp(descs.front(), rendererChangeInfo, reason)) { continue; }
            int32_t ret = ActivateA2dpDeviceWhenDescEnabled(descs.front(), rendererChangeInfos, reason);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "activate a2dp [%{public}s] failed", encryptMacAddr.c_str());
        } else if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            int32_t ret = HandleScoOutputDeviceFetched(descs.front(), rendererChangeInfos, reason);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "sco [%{public}s] is not connected yet", encryptMacAddr.c_str());
        } else if (descs.front()->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
            audioEcManager_.ActivateArmDevice(descs.front()->macAddress_, descs.front()->deviceRole_);
        }
        if (needUpdateActiveDevice) {
            isUpdateActiveDevice = audioActiveDevice_.UpdateDevice(descs.front(), reason, rendererChangeInfo);
            needUpdateActiveDevice = !isUpdateActiveDevice;
        }
        if (!hasDirectChangeDevice && !IsSameDevice(descs.front(), rendererChangeInfo->outputDeviceInfo)) {
            hasDirectChangeDevice = true;
        }
        NotifyRecreateRendererStream(descs.front(), rendererChangeInfo, reason);
        MoveToNewOutputDevice(rendererChangeInfo, descs, sinkInputs, reason);
    }
    FetchOutputEnd(isUpdateActiveDevice, runningStreamCount, reason);
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

void AudioDeviceCommon::FetchOutputEnd(const bool isUpdateActiveDevice, const int32_t runningStreamCount,
    const AudioStreamDeviceChangeReason reason)
{
    if (isUpdateActiveDevice) {
        OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(), reason);
    }
    if (runningStreamCount == 0) {
        FetchOutputDeviceWhenNoRunningStream(reason);
    }
}

void AudioDeviceCommon::FetchOutputDeviceWhenNoRunningStream(const AudioStreamDeviceChangeReason reason)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1, "FetchOutputDeviceWhenNoRunningStream");
    CHECK_AND_RETURN_LOG(!descs.empty(), "descs is empty");
    AudioDeviceDescriptor tmpOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    if (descs.front()->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(descs.front(), tmpOutputDeviceDesc)) {
        AUDIO_DEBUG_LOG("output device is not change");
        return;
    }
    audioActiveDevice_.SetCurrentOutputDevice(*descs.front());
    AUDIO_DEBUG_LOG("currentActiveDevice update %{public}d", audioActiveDevice_.GetCurrentOutputDeviceType());
    audioVolumeManager_.SetVolumeForSwitchDevice(*descs.front());

    AudioServerProxy::GetInstance().SetActiveOutputDeviceProxy(audioActiveDevice_.GetCurrentOutputDeviceType());

    if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        SwitchActiveA2dpDevice(std::make_shared<AudioDeviceDescriptor>(*descs.front()));
    }
    OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(), reason);
}

int32_t AudioDeviceCommon::HandleDeviceChangeForFetchOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo, const AudioStreamDeviceChangeReason reason)
{
    if (desc->deviceType_ == DEVICE_TYPE_NONE || (IsSameDevice(desc, rendererChangeInfo->outputDeviceInfo) &&
        !NeedRehandleA2DPDevice(desc) && desc->connectState_ != DEACTIVE_CONNECTED &&
        audioSceneManager_.IsSameAudioScene() && !shouldUpdateDeviceDueToDualTone_)) {
        AUDIO_WARNING_LOG("stream %{public}d device not change, no need move device", rendererChangeInfo->sessionId);
        AudioDeviceDescriptor tmpOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
        std::shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetRendererDevice(rendererChangeInfo->clientUID);
        if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) && !IsSameDevice(desc, tmpOutputDeviceDesc)
            && desc->deviceType_ != preferredDesc->deviceType_)
            || ((preferredDesc->deviceType_ == DEVICE_TYPE_NONE) && !IsSameDevice(desc, tmpOutputDeviceDesc))) {
            audioActiveDevice_.SetCurrentOutputDevice(*desc);
            AudioDeviceDescriptor curOutputDevice = audioActiveDevice_.GetCurrentOutputDevice();
            audioVolumeManager_.SetVolumeForSwitchDevice(curOutputDevice);
            audioActiveDevice_.UpdateActiveDeviceRoute(curOutputDevice.deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG);
            OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(), reason);
        }
        return ERR_NEED_NOT_SWITCH_DEVICE;
    }
    return SUCCESS;
}

void AudioDeviceCommon::MuteSinkPortForSwitchDevice(std::shared_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>>& outputDevices, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::MuteSinkPortForSwitchDevice");
    if (outputDevices.front()->IsSameDeviceDesc(rendererChangeInfo->outputDeviceInfo)) return;

    SetHeadsetUnpluggedToSpkOrEpFlag(rendererChangeInfo->outputDeviceInfo.deviceType_,
        outputDevices.front()->deviceType_);

    audioIOHandleMap_.SetMoveFinish(false);

    if (audioSceneManager_.GetAudioScene(true) == AUDIO_SCENE_PHONE_CALL &&
        rendererChangeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
        audioSceneManager_.CheckVoiceCallActive(rendererChangeInfo->sessionId)) {
        return SetVoiceCallMuteForSwitchDevice();
    }

    std::string oldSinkName = AudioPolicyUtils::GetInstance().GetSinkName(rendererChangeInfo->outputDeviceInfo,
        rendererChangeInfo->sessionId);
    std::string newSinkName = AudioPolicyUtils::GetInstance().GetSinkName(*outputDevices.front(),
        rendererChangeInfo->sessionId);
    int32_t oldStreamClass = GetPreferredOutputStreamTypeInner(rendererChangeInfo->rendererInfo.streamUsage,
        rendererChangeInfo->outputDeviceInfo.deviceType_, rendererChangeInfo->rendererInfo.originalFlag,
        rendererChangeInfo->outputDeviceInfo.networkId_, rendererChangeInfo->rendererInfo.samplingRate, false);
    int32_t newStreamClass = GetPreferredOutputStreamTypeInner(rendererChangeInfo->rendererInfo.streamUsage,
        outputDevices.front()->deviceType_, rendererChangeInfo->rendererInfo.originalFlag,
        outputDevices.front()->networkId_, rendererChangeInfo->rendererInfo.samplingRate, false);
    oldSinkName = oldStreamClass == AUDIO_FLAG_VOIP_DIRECT ? PRIMARY_DIRECT_VOIP : oldSinkName;
    oldSinkName = oldStreamClass == AUDIO_FLAG_VOIP_FAST ? PRIMARY_MMAP_VOIP : oldSinkName;
    newSinkName = newStreamClass == AUDIO_FLAG_VOIP_DIRECT ? PRIMARY_DIRECT_VOIP : newSinkName;
    newSinkName = newStreamClass == AUDIO_FLAG_VOIP_FAST ? PRIMARY_MMAP_VOIP : newSinkName;
    AUDIO_INFO_LOG("mute sink old:[%{public}s] new:[%{public}s]", oldSinkName.c_str(), newSinkName.c_str());
    MuteSinkPort(oldSinkName, newSinkName, reason);
}

void AudioDeviceCommon::MuteSinkForSwitchGeneralDevice(std::shared_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>>& outputDevices, const AudioStreamDeviceChangeReasonExt reason)
{
    if (outputDevices.front() != nullptr && (outputDevices.front()->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP &&
        outputDevices.front()->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO)) {
        MuteSinkPortForSwitchDevice(rendererChangeInfo, outputDevices, reason);
    }
}

void AudioDeviceCommon::MuteSinkForSwitchBluetoothDevice(std::shared_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>>& outputDevices, const AudioStreamDeviceChangeReasonExt reason)
{
    if (outputDevices.front() != nullptr && (outputDevices.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
        outputDevices.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) {
        MuteSinkPortForSwitchDevice(rendererChangeInfo, outputDevices, reason);
    }
}

void AudioDeviceCommon::SetVoiceCallMuteForSwitchDevice()
{
    Trace trace("SetVoiceMuteForSwitchDevice");
    AudioServerProxy::GetInstance().SetVoiceVolumeProxy(0);

    AUDIO_INFO_LOG("%{public}" PRId64" us for modem call update route", WAIT_MODEM_CALL_SET_VOLUME_TIME_US);
    usleep(WAIT_MODEM_CALL_SET_VOLUME_TIME_US);
    // Unmute in SetVolumeForSwitchDevice after update route.
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

int32_t AudioDeviceCommon::ActivateA2dpDeviceWhenDescEnabled(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
    const AudioStreamDeviceChangeReasonExt reason)
{
    if (desc->isEnable_) {
        AUDIO_INFO_LOG("descs front is enabled");
        return ActivateA2dpDevice(desc, rendererChangeInfos, reason);
    }
    return SUCCESS;
}

int32_t AudioDeviceCommon::ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::ActivateA2dpDevice");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
    int32_t ret = SwitchActiveA2dpDevice(deviceDesc);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active A2DP device failed, retrigger fetch output device");
        deviceDesc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(deviceDesc, EXCEPTION_FLAG_UPDATE);
        audioIOHandleMap_.NotifyUnmutePort();
        FetchOutputDevice(rendererChangeInfos, reason);
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioDeviceCommon::HandleScoOutputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::HandleScoOutputDeviceFetched");
#ifdef BLUETOOTH_ENABLE
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch output device.");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        FetchOutputDevice(rendererChangeInfos, reason);
        return ERROR;
    }
    Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true));
#endif
    return SUCCESS;
}

bool AudioDeviceCommon::NotifyRecreateRendererStream(std::shared_ptr<AudioDeviceDescriptor> &desc,
    const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Not support, should use AudioPipeSelector");
    return false;
}

bool AudioDeviceCommon::NeedRehandleA2DPDevice(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP
        && audioIOHandleMap_.CheckIOHandleExist(BLUETOOTH_SPEAKER) == false) {
        AUDIO_WARNING_LOG("A2DP module is not loaded, need rehandle");
        return true;
    }
    return false;
}

void AudioDeviceCommon::MoveToNewOutputDevice(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &outputDevices, std::vector<SinkInput> sinkInputs,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::MoveToNewOutputDevice");
    std::vector<SinkInput> targetSinkInputs = audioOffloadStream_.FilterSinkInputs(rendererChangeInfo->sessionId,
        sinkInputs);

    bool needTriggerCallback = true;
    if (outputDevices.front()->IsSameDeviceDesc(rendererChangeInfo->outputDeviceInfo)) {
        needTriggerCallback = false;
    }

    AUDIO_WARNING_LOG("move session %{public}d [%{public}d][%{public}s]-->[%{public}d][%{public}s], reason %{public}d",
        rendererChangeInfo->sessionId, rendererChangeInfo->outputDeviceInfo.deviceType_,
        GetEncryptAddr(rendererChangeInfo->outputDeviceInfo.macAddress_).c_str(),
        outputDevices.front()->deviceType_, GetEncryptAddr(outputDevices.front()->macAddress_).c_str(),
        static_cast<int>(reason));

    DeviceType oldDevice = rendererChangeInfo->outputDeviceInfo.deviceType_;
    auto oldRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>(*rendererChangeInfo.get());

    UpdateDeviceInfo(rendererChangeInfo->outputDeviceInfo,
        std::make_shared<AudioDeviceDescriptor>(*outputDevices.front()), true, true);

    if (needTriggerCallback && audioPolicyServerHandler_) {
        audioPolicyServerHandler_->SendRendererDeviceChangeEvent(rendererChangeInfo->callerPid,
            rendererChangeInfo->sessionId, rendererChangeInfo->outputDeviceInfo, reason);
    }
    MuteSinkForSwitchGeneralDevice(oldRendererChangeInfo, outputDevices, reason);

    // MoveSinkInputByIndexOrName
    auto ret = (outputDevices.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalOutputDevice(targetSinkInputs, std::make_shared<AudioDeviceDescriptor>(*outputDevices.front()))
        : MoveToRemoteOutputDevice(targetSinkInputs, std::make_shared<AudioDeviceDescriptor>(*outputDevices.front()));
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Move sink input %{public}d to device %{public}d failed!",
            rendererChangeInfo->sessionId, outputDevices.front()->deviceType_);
        audioIOHandleMap_.NotifyUnmutePort();
        return;
    }

    if (audioConfigManager_.GetUpdateRouteSupport()) {
        UpdateRoute(oldRendererChangeInfo, outputDevices);
    }

    std::string newSinkName = AudioPolicyUtils::GetInstance().GetSinkName(*outputDevices.front(),
        rendererChangeInfo->sessionId);
    audioVolumeManager_.SetVolumeForSwitchDevice(*outputDevices.front(), newSinkName);

    streamCollector_.UpdateRendererDeviceInfo(rendererChangeInfo->clientUID, rendererChangeInfo->sessionId,
        rendererChangeInfo->outputDeviceInfo);
    ResetOffloadAndMchMode(rendererChangeInfo, outputDevices);
    audioIOHandleMap_.NotifyUnmutePort();
}

void AudioDeviceCommon::MuteOtherSink(const std::string &sinkName, int64_t muteTime)
{
    // fix pop when switching devices during multiple concurrent streams
    if (sinkName == OFFLOAD_PRIMARY_SPEAKER ||
        ((sinkName == PRIMARY_DIRECT_VOIP || sinkName == PRIMARY_MMAP_VOIP) &&
        streamCollector_.HasRunningRendererStream())) {
        audioIOHandleMap_.MuteSinkPort(PRIMARY_SPEAKER, muteTime, true, false);
    } else if (sinkName == PRIMARY_SPEAKER && streamCollector_.IsVoipStreamActive()) {
        audioIOHandleMap_.MuteSinkPort(PRIMARY_DIRECT_VOIP, muteTime, true, false);
        audioIOHandleMap_.MuteSinkPort(PRIMARY_MMAP_VOIP, muteTime, true, false);
    } else if (sinkName == PRIMARY_SPEAKER) {
        audioIOHandleMap_.MuteSinkPort(OFFLOAD_PRIMARY_SPEAKER, muteTime, true, false);
    }
}

void AudioDeviceCommon::MuteSinkPort(const std::string &oldSinkName, const std::string &newSinkName,
    AudioStreamDeviceChangeReasonExt reason)
{
    if (reason.IsOverride() || reason.IsSetDefaultOutputDevice()) {
        int64_t muteTime = SELECT_DEVICE_MUTE_MS;
        if (newSinkName == OFFLOAD_PRIMARY_SPEAKER || oldSinkName == OFFLOAD_PRIMARY_SPEAKER) {
            muteTime = SELECT_OFFLOAD_DEVICE_MUTE_MS;
        }
        MuteOtherSink(newSinkName, muteTime);
        audioIOHandleMap_.MuteSinkPort(newSinkName, SELECT_DEVICE_MUTE_MS, true);
        audioIOHandleMap_.MuteSinkPort(oldSinkName, muteTime, true);
    } else if (reason == AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE) {
        int64_t muteTime = NEW_DEVICE_AVALIABLE_MUTE_MS;
        if (newSinkName == OFFLOAD_PRIMARY_SPEAKER || oldSinkName == OFFLOAD_PRIMARY_SPEAKER) {
            muteTime = NEW_DEVICE_AVALIABLE_OFFLOAD_MUTE_MS;
        }
        MuteOtherSink(oldSinkName, muteTime);
        audioIOHandleMap_.MuteSinkPort(newSinkName, NEW_DEVICE_AVALIABLE_MUTE_MS, true);
        audioIOHandleMap_.MuteSinkPort(oldSinkName, muteTime, true);
    }
    MuteSinkPortLogic(oldSinkName, newSinkName, reason);
}

void AudioDeviceCommon::MuteSinkPortLogic(const std::string &oldSinkName, const std::string &newSinkName,
    AudioStreamDeviceChangeReasonExt reason)
{
    auto ringermode = audioPolicyManager_.GetRingerMode();
    AudioScene scene = audioSceneManager_.GetAudioScene(true);
    if (reason == DISTRIBUTED_DEVICE) {
        audioIOHandleMap_.MuteSinkPort(newSinkName, DISTRIBUTED_DEVICE_UNAVALIABLE_MUTE_MS, true);
        usleep(DISTRIBUTED_DEVICE_UNAVALIABLE_SLEEP_US);
    } else if (reason.IsOldDeviceUnavaliable() && ((scene == AUDIO_SCENE_DEFAULT) ||
        ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) &&
        ringermode != RINGER_MODE_NORMAL) || (scene == AUDIO_SCENE_PHONE_CHAT))) {
        MuteOtherSink(newSinkName, OLD_DEVICE_UNAVALIABLE_MUTE_MS);
        audioIOHandleMap_.MuteSinkPort(newSinkName, OLD_DEVICE_UNAVALIABLE_MUTE_MS, true);
        usleep(OLD_DEVICE_UNAVALIABLE_MUTE_SLEEP_MS); // sleep fix data cache pop.
        if (isHeadsetUnpluggedToSpkOrEpFlag_) {
            usleep(OLD_DEVICE_UNAVALIABLE_EXT_SLEEP_US); // sleep fix data cache pop.
            isHeadsetUnpluggedToSpkOrEpFlag_ = false;
        }
    } else if (reason.IsOldDeviceUnavaliableExt() && ((scene == AUDIO_SCENE_DEFAULT) ||
        ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) &&
        ringermode != RINGER_MODE_NORMAL) || (scene == AUDIO_SCENE_PHONE_CHAT))) {
        audioIOHandleMap_.MuteSinkPort(newSinkName, OLD_DEVICE_UNAVALIABLE_EXT_MUTE_MS, true);
        usleep(OLD_DEVICE_UNAVALIABLE_MUTE_SLEEP_MS); // sleep fix data cache pop.
    } else if (reason == AudioStreamDeviceChangeReason::UNKNOWN &&
        oldSinkName == REMOTE_CAST_INNER_CAPTURER_SINK_NAME) {
        // remote cast -> earpiece 300ms fix sound leak
        audioIOHandleMap_.MuteSinkPort(newSinkName, NEW_DEVICE_REMOTE_CAST_AVALIABLE_MUTE_MS, true);
    }
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

bool AudioDeviceCommon::IsDualStreamWhenRingDual(AudioStreamType streamType)
{
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType == STREAM_RING || volumeType == STREAM_ALARM || volumeType == STREAM_ACCESSIBILITY) {
        return true;
    }
    return false;
}

void AudioDeviceCommon::UpdateRoute(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &outputDevices)
{
    StreamUsage streamUsage = rendererChangeInfo->rendererInfo.streamUsage;
    InternalDeviceType deviceType = outputDevices.front()->deviceType_;
    AUDIO_INFO_LOG("update route, streamUsage:%{public}d, 1st devicetype:%{public}d", streamUsage, deviceType);
    if (Util::IsRingerOrAlarmerStreamUsage(streamUsage) && IsRingerOrAlarmerDualDevicesRange(deviceType) &&
        !VolumeUtils::IsPCVolumeEnable()) {
        if (!IsSameDevice(outputDevices.front(), rendererChangeInfo->outputDeviceInfo) &&
            !SelectRingerOrAlarmDevices(outputDevices, rendererChangeInfo)) {
            audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG);
        }

        AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
        if (ringerMode != RINGER_MODE_NORMAL && IsRingerOrAlarmerDualDevicesRange(outputDevices.front()->getType()) &&
             outputDevices.front()->getType() != DEVICE_TYPE_SPEAKER) {
            audioPolicyManager_.SetStreamMute(STREAM_RING, false, streamUsage);
            audioVolumeManager_.SetRingerModeMute(false);
            int32_t curRingToneLevel = RingToneVoiceControl(outputDevices.front()->getType());
            audioPolicyManager_.SetDoubleRingVolumeDb(STREAM_RING, curRingToneLevel);
        } else {
            audioVolumeManager_.SetRingerModeMute(true);
        }
        shouldUpdateDeviceDueToDualTone_ = true;
    } else {
        audioVolumeManager_.SetRingerModeMute(true);
        if (isRingDualToneOnPrimarySpeaker_ && streamUsage != STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
            std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
            activeDevices.push_back(make_pair(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG));
            activeDevices.push_back(make_pair(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG));
            audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
            AUDIO_INFO_LOG("update desc [%{public}d] with speaker on session [%{public}d]",
                deviceType, rendererChangeInfo->sessionId);
            AudioStreamType streamType = streamCollector_.GetStreamType(rendererChangeInfo->sessionId);
            if (!IsDualStreamWhenRingDual(streamType)) {
                streamsWhenRingDualOnPrimarySpeaker_.push_back(make_pair(streamType, streamUsage));
                audioPolicyManager_.SetStreamMute(streamType, true, streamUsage);
            }
        } else {
            audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG);
        }
        shouldUpdateDeviceDueToDualTone_ = false;
    }
}

void AudioDeviceCommon::ResetOffloadAndMchMode(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
    vector<std::shared_ptr<AudioDeviceDescriptor>> &outputDevices)
{
    if (outputDevices.front()->networkId_ != LOCAL_NETWORK_ID
        || outputDevices.front()->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        audioOffloadStream_.RemoteOffloadStreamRelease(rendererChangeInfo->sessionId);
    } else {
        FetchStreamForSpkMchStream(rendererChangeInfo, outputDevices);
    }
}

void AudioDeviceCommon::JudgeIfLoadMchModule()
{
    bool isNeedLoadMchModule = false;
    {
        if (audioIOHandleMap_.CheckIOHandleExist(MCH_PRIMARY_SPEAKER) == false) {
            isNeedLoadMchModule = true;
        }
    }
    if (isNeedLoadMchModule) {
        audioOffloadStream_.LoadMchModule();
    }
}

void AudioDeviceCommon::FetchStreamForA2dpMchStream(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
    vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    if (audioOffloadStream_.CheckStreamMultichannelMode(rendererChangeInfo->sessionId)) {
        JudgeIfLoadMchModule();
        audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceFlag::OUTPUT_DEVICES_FLAG);
        std::string portName = AudioPolicyUtils::GetInstance().GetSinkPortName(descs.front()->deviceType_,
            PIPE_TYPE_MULTICHANNEL);
        int32_t ret  = audioOffloadStream_.MoveToOutputDevice(rendererChangeInfo->sessionId, portName);
        if (ret == SUCCESS) {
            streamCollector_.UpdateRendererPipeInfo(rendererChangeInfo->sessionId, PIPE_TYPE_MULTICHANNEL);
        }
    } else {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(rendererChangeInfo->sessionId, pipeType);
        if (pipeType == PIPE_TYPE_MULTICHANNEL) {
            std::string currentActivePort = MCH_PRIMARY_SPEAKER;
            AudioIOHandle activateDeviceIOHandle;
            CHECK_AND_RETURN_LOG(audioIOHandleMap_.GetModuleIdByKey(currentActivePort, activateDeviceIOHandle),
                "Can not find port MCH_PRIMARY_SPEAKER in io map");
            audioPolicyManager_.SuspendAudioDevice(currentActivePort, true);
            audioPolicyManager_.CloseAudioPort(activateDeviceIOHandle);
            audioIOHandleMap_.DelIOHandleInfo(currentActivePort);
            streamCollector_.UpdateRendererPipeInfo(rendererChangeInfo->sessionId, PIPE_TYPE_NORMAL_OUT);
        }
        audioOffloadStream_.ResetOffloadMode(rendererChangeInfo->sessionId);
        std::vector<SinkInput> sinkInputs;
        audioPolicyManager_.GetAllSinkInputs(sinkInputs);
        MoveToNewOutputDevice(rendererChangeInfo, descs, sinkInputs);
    }
}

void AudioDeviceCommon::FetchStreamForSpkMchStream(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
    vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    if (audioOffloadStream_.CheckStreamMultichannelMode(rendererChangeInfo->sessionId)) {
        JudgeIfLoadMchModule();
        std::string oldSinkName = AudioPolicyUtils::GetInstance().GetSinkName(rendererChangeInfo->outputDeviceInfo,
            rendererChangeInfo->sessionId);
        std::string newSinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(descs.front()->deviceType_,
            PIPE_TYPE_MULTICHANNEL);
        if (oldSinkName != newSinkName) {
            AUDIO_INFO_LOG("mute sink old:[%{public}s] new:[%{public}s]", oldSinkName.c_str(), newSinkName.c_str());
            MuteSinkPort(oldSinkName, newSinkName, AudioStreamDeviceChangeReason::OVERRODE);
        }
        int32_t ret  = audioOffloadStream_.MoveToOutputDevice(rendererChangeInfo->sessionId, newSinkName);
        if (ret == SUCCESS) {
            streamCollector_.UpdateRendererPipeInfo(rendererChangeInfo->sessionId, PIPE_TYPE_MULTICHANNEL);
        }
    } else {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(rendererChangeInfo->sessionId, pipeType);
        if (pipeType == PIPE_TYPE_MULTICHANNEL) {
            {
                AUDIO_INFO_LOG("unload multichannel module");
                std::string currentActivePort = MCH_PRIMARY_SPEAKER;
                AudioIOHandle activateDeviceIOHandle;
                CHECK_AND_RETURN_LOG(audioIOHandleMap_.GetModuleIdByKey(currentActivePort, activateDeviceIOHandle),
                    "Can not find port MCH_PRIMARY_SPEAKER in io map");
                audioPolicyManager_.SuspendAudioDevice(currentActivePort, true);
                audioPolicyManager_.CloseAudioPort(activateDeviceIOHandle);
                audioIOHandleMap_.DelIOHandleInfo(currentActivePort);
            }
        }
        audioOffloadStream_.ResetOffloadMode(rendererChangeInfo->sessionId);
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
    audioPolicyManager_.SetInnerStreamMute(STREAM_MUSIC, true, STREAM_USAGE_MUSIC);
    return true;
}

void AudioDeviceCommon::ClearRingMuteWhenCallStart(bool pre, bool after)
{
    CHECK_AND_RETURN_LOG(pre == true && after == false, "ringdual not cancel by call");
    AUDIO_INFO_LOG("disable primary speaker dual tone when call start and ring not over");
    for (std::pair<AudioStreamType, StreamUsage> stream : streamsWhenRingDualOnPrimarySpeaker_) {
        audioPolicyManager_.SetStreamMute(stream.first, false, stream.second);
    }
    streamsWhenRingDualOnPrimarySpeaker_.clear();
    audioPolicyManager_.SetStreamMute(STREAM_MUSIC, false, STREAM_USAGE_MUSIC);
}

bool AudioDeviceCommon::SelectRingerOrAlarmDevices(const vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo)
{
    CHECK_AND_RETURN_RET_LOG(descs.size() > 0 && descs.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT, false,
        "audio devices not in range for ringer or alarmer.");
    const int32_t sessionId = rendererChangeInfo->sessionId;
    const StreamUsage streamUsage = rendererChangeInfo->rendererInfo.streamUsage;
    bool allDevicesInDualDevicesRange = true;
    std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
    for (size_t i = 0; i < descs.size(); i++) {
        if (IsRingerOrAlarmerDualDevicesRange(descs[i]->deviceType_)) {
            activeDevices.push_back(make_pair(descs[i]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG));
            AUDIO_INFO_LOG("select ringer/alarm devices devicetype[%{public}zu]:%{public}d", i, descs[i]->deviceType_);
        } else {
            allDevicesInDualDevicesRange = false;
            break;
        }
    }

    AUDIO_INFO_LOG("select ringer/alarm sessionId:%{public}d, streamUsage:%{public}d", sessionId, streamUsage);
    if (!descs.empty() && allDevicesInDualDevicesRange) {
        if (descs.size() == AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT &&
            AudioPolicyUtils::GetInstance().GetSinkName(*descs.front(), sessionId) !=
            AudioPolicyUtils::GetInstance().GetSinkName(*descs.back(), sessionId)) {
            AUDIO_INFO_LOG("set dual hal tone, reset primary sink to default before.");
            audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG);
            if (enableDualHalToneState_ && enableDualHalToneSessionId_ != sessionId) {
                AUDIO_INFO_LOG("session changed, disable old dual hal tone.");
                UpdateDualToneState(false, enableDualHalToneSessionId_);
            }

            if ((audioPolicyManager_.GetRingerMode() != RINGER_MODE_NORMAL && streamUsage != STREAM_USAGE_ALARM) ||
                (VolumeUtils::IsPCVolumeEnable() && audioVolumeManager_.GetStreamMute(STREAM_MUSIC))) {
                AUDIO_INFO_LOG("no normal ringer mode and no alarm, dont dual hal tone.");
                return false;
            }
            UpdateDualToneState(true, sessionId);
        } else {
            if (enableDualHalToneState_ && enableDualHalToneSessionId_ == sessionId) {
                AUDIO_INFO_LOG("device unavailable, disable dual hal tone.");
                UpdateDualToneState(false, enableDualHalToneSessionId_);
            }
            bool pre = isRingDualToneOnPrimarySpeaker_;
            isRingDualToneOnPrimarySpeaker_ = IsRingDualToneOnPrimarySpeaker(descs, sessionId);
            ClearRingMuteWhenCallStart(pre, isRingDualToneOnPrimarySpeaker_);
            audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
        }
        return true;
    }
    return false;
}

int32_t AudioDeviceCommon::HandleDeviceChangeForFetchInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo)
{
    if (desc->deviceType_ == DEVICE_TYPE_NONE ||
        (IsSameDevice(desc, capturerChangeInfo->inputDeviceInfo) && desc->connectState_ != DEACTIVE_CONNECTED)) {
        AUDIO_WARNING_LOG("stream %{public}d device not change, no need move device", capturerChangeInfo->sessionId);
        std::shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetCapturerDevice(capturerChangeInfo->clientUID);
        if (!IsSameDevice(desc, audioActiveDevice_.GetCurrentInputDevice()) &&
            (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) && desc->deviceType_ != preferredDesc->deviceType_) ||
            IsSameDevice(desc, capturerChangeInfo->inputDeviceInfo))) {
            audioActiveDevice_.SetCurrentInputDevice(*desc);
            // networkId is not used.
            OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(), "");
            audioActiveDevice_.UpdateActiveDeviceRoute(audioActiveDevice_.GetCurrentInputDeviceType(),
                DeviceFlag::INPUT_DEVICES_FLAG, audioActiveDevice_.GetCurrentInputDevice().deviceName_);
        }
        return ERR_NEED_NOT_SWITCH_DEVICE;
    }
    return SUCCESS;
}

void AudioDeviceCommon::FetchInputDeviceInner(
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos,
    const AudioStreamDeviceChangeReasonExt reason, bool& needUpdateActiveDevice, bool& isUpdateActiveDevice,
    int32_t& runningStreamCount)
{
    for (auto &capturerChangeInfo : capturerChangeInfos) {
        SourceType sourceType = capturerChangeInfo->capturerInfo.sourceType;
        int32_t clientUID = capturerChangeInfo->clientUID;
        if ((sourceType == SOURCE_TYPE_VIRTUAL_CAPTURE &&
            audioSceneManager_.GetAudioScene(true) != AUDIO_SCENE_PHONE_CALL) ||
            (sourceType != SOURCE_TYPE_VIRTUAL_CAPTURE && capturerChangeInfo->capturerState != CAPTURER_RUNNING &&
                !capturerChangeInfo->prerunningState)) {
            AUDIO_WARNING_LOG("stream %{public}d not running, no need fetch device", capturerChangeInfo->sessionId);
            continue;
        }
        runningStreamCount++;
        std::shared_ptr<AudioDeviceDescriptor> desc = audioRouterCenter_.FetchInputDevice(sourceType, clientUID,
            capturerChangeInfo->sessionId);
        AudioDeviceDescriptor inputDeviceInfo = capturerChangeInfo->inputDeviceInfo;
        if (HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo) == ERR_NEED_NOT_SWITCH_DEVICE) {
            continue;
        }
        HandleBluetoothInputDeviceFetched(desc, capturerChangeInfos, sourceType);
        CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
        if (desc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
            audioEcManager_.ActivateArmDevice(desc->macAddress_, desc->deviceRole_);
        }
        if (needUpdateActiveDevice) {
            std::shared_ptr<AudioDeviceDescriptor> preferredDesc = audioAffinityManager_.GetCapturerDevice(clientUID);
            if (((preferredDesc->deviceType_ != DEVICE_TYPE_NONE) &&
                !IsSameDevice(desc, audioActiveDevice_.GetCurrentInputDevice())
                && desc->deviceType_ != preferredDesc->deviceType_)
                || ((preferredDesc->deviceType_ == DEVICE_TYPE_NONE)
                && !IsSameDevice(desc, audioActiveDevice_.GetCurrentInputDevice()))) {
                WriteInputRouteChangeEvent(desc, reason);
                audioActiveDevice_.SetCurrentInputDevice(*desc);
                AUDIO_DEBUG_LOG("currentActiveInputDevice update %{public}d",
                    audioActiveDevice_.GetCurrentInputDeviceType());
                isUpdateActiveDevice = true;
            }
            needUpdateActiveDevice = false;
        }
        if (NotifyRecreateCapturerStream(isUpdateActiveDevice, capturerChangeInfo, reason)) {continue;}
        // move sourceoutput to target device
        MoveToNewInputDevice(capturerChangeInfo, desc);
        audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(capturerChangeInfo->sessionId,
            desc->deviceType_);
    }
}

void AudioDeviceCommon::FetchInputDevice(std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::FetchInputDevice");
    AUDIO_PRERELEASE_LOGI("Start for %{public}zu stream, connected %{public}s",
        capturerChangeInfos.size(), audioDeviceManager_.GetConnDevicesStr().c_str());
    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    int32_t runningStreamCount = 0;

    FetchInputDeviceInner(capturerChangeInfos, reason, needUpdateActiveDevice, isUpdateActiveDevice,
        runningStreamCount);

    FetchInputEnd(isUpdateActiveDevice, runningStreamCount);
}

void AudioDeviceCommon::FetchInputEnd(const bool isUpdateActiveDevice, const int32_t runningStreamCount)
{
    if (isUpdateActiveDevice) {
        OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(), ""); // networkId is not used.
    }
    if (runningStreamCount == 0) {
        FetchInputDeviceWhenNoRunningStream();
    }
}

void AudioDeviceCommon::HandleBluetoothInputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos, SourceType sourceType)
{
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        BluetoothScoFetch(desc, capturerChangeInfos, sourceType);
    } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        HandleA2dpInputDeviceFetched(desc, sourceType);
    }
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

bool AudioDeviceCommon::NotifyRecreateCapturerStream(bool isUpdateActiveDevice,
    const std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Is update active device: %{public}d, current capturerFlag: %{public}d, origianl flag: %{public}d",
        isUpdateActiveDevice, capturerChangeInfo->capturerInfo.capturerFlags,
        capturerChangeInfo->capturerInfo.originalFlag);
    CHECK_AND_RETURN_RET_LOG(isUpdateActiveDevice, false, "isUpdateActiveDevice is false");
    CHECK_AND_RETURN_RET_LOG(capturerChangeInfo->capturerInfo.originalFlag == AUDIO_FLAG_MMAP, false,
        "original flag is false");
    // Switch between old and new stream as they have different hals
    std::string oldDevicePortName = AudioPolicyUtils::GetInstance().GetSourcePortName(
        capturerChangeInfo->inputDeviceInfo.deviceType_);
    if ((strcmp(oldDevicePortName.c_str(),
        AudioPolicyUtils::GetInstance().GetSourcePortName(audioActiveDevice_.GetCurrentInputDeviceType()).c_str())) ||
        ((capturerChangeInfo->inputDeviceInfo.networkId_ == LOCAL_NETWORK_ID) ^
        (audioActiveDevice_.GetCurrentInputDevice().networkId_ == LOCAL_NETWORK_ID))) {
        int32_t streamClass = GetPreferredInputStreamTypeInner(capturerChangeInfo->capturerInfo.sourceType,
            audioActiveDevice_.GetCurrentInputDeviceType(), capturerChangeInfo->capturerInfo.originalFlag,
            audioActiveDevice_.GetCurrentInputDevice().networkId_, capturerChangeInfo->capturerInfo.samplingRate);
        TriggerRecreateCapturerStreamCallback(capturerChangeInfo, streamClass, reason);
        return true;
    }
    return false;
}

void AudioDeviceCommon::MoveToNewInputDevice(std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
    std::shared_ptr<AudioDeviceDescriptor> &inputDevice)
{
    std::vector<SourceOutput> targetSourceOutputs = FilterSourceOutputs(capturerChangeInfo->sessionId);

    // MoveSourceOuputByIndexName
    auto ret = (inputDevice->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalInputDevice(targetSourceOutputs, std::make_shared<AudioDeviceDescriptor>(*inputDevice))
        : MoveToRemoteInputDevice(targetSourceOutputs, std::make_shared<AudioDeviceDescriptor>(*inputDevice));
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "Move source output %{public}d to device %{public}d failed!",
        capturerChangeInfo->sessionId, inputDevice->deviceType_);
    AUDIO_WARNING_LOG("move session %{public}d [%{public}d][%{public}s]-->[%{public}d][%{public}s]",
        capturerChangeInfo->sessionId, capturerChangeInfo->inputDeviceInfo.deviceType_,
        GetEncryptAddr(capturerChangeInfo->inputDeviceInfo.macAddress_).c_str(),
        inputDevice->deviceType_, GetEncryptAddr(inputDevice->macAddress_).c_str());

    if (audioConfigManager_.GetUpdateRouteSupport() && inputDevice->networkId_ == LOCAL_NETWORK_ID) {
        audioActiveDevice_.UpdateActiveDeviceRoute(inputDevice->deviceType_, DeviceFlag::INPUT_DEVICES_FLAG,
            inputDevice->deviceName_);
    }
    UpdateDeviceInfo(capturerChangeInfo->inputDeviceInfo, std::make_shared<AudioDeviceDescriptor>(*inputDevice),
        true, true);
    streamCollector_.UpdateCapturerDeviceInfo(capturerChangeInfo->clientUID, capturerChangeInfo->sessionId,
        capturerChangeInfo->inputDeviceInfo);
}

void AudioDeviceCommon::FetchInputDeviceWhenNoRunningStream()
{
    std::shared_ptr<AudioDeviceDescriptor> desc;
    AudioDeviceDescriptor tempDesc = audioActiveDevice_.GetCurrentInputDevice();
    if (tempDesc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && Bluetooth::AudioHfpManager::IsRecognitionStatus()) {
        desc = audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_VOICE_RECOGNITION, -1);
    } else {
        desc = audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_MIC, -1);
    }
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
    if (desc->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(desc, tempDesc)) {
        AUDIO_DEBUG_LOG("input device is not change");
        return;
    }
    audioActiveDevice_.SetCurrentInputDevice(*desc);
    if (desc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioEcManager_.PresetArmIdleInput(desc->macAddress_);
    }
    DeviceType deviceType = audioActiveDevice_.GetCurrentInputDeviceType();
    AUDIO_DEBUG_LOG("currentActiveInputDevice update %{public}d", deviceType);
    OnPreferredInputDeviceUpdated(deviceType, ""); // networkId is not used
}

void AudioDeviceCommon::BluetoothScoFetch(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos, SourceType sourceType)
{
    Trace trace("AudioDeviceCommon::BluetoothScoFetch");
    int32_t ret;
    if (Util::IsScoSupportSource(sourceType)) {
        int32_t activeRet = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
        if (activeRet != SUCCESS) {
            AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch input device");
            desc->exceptionFlag_ = true;
            audioDeviceManager_.UpdateDevicesListInfo(
                std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
            FetchInputDevice(capturerChangeInfos);
        }
        ret = ScoInputDeviceFetchedForRecongnition(true, desc->macAddress_, desc->connectState_);
    } else {
        ret = HandleScoInputDeviceFetched(desc, capturerChangeInfos);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sco [%{public}s] is not connected yet", GetEncryptAddr(desc->macAddress_).c_str());
    }
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

void AudioDeviceCommon::HandleA2dpInputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
    SourceType sourceType)
{
    audioActiveDevice_.SetActiveBtInDeviceMac(desc->macAddress_);
    AudioStreamInfo audioStreamInfo = {};
    audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP_IN, audioStreamInfo);

    std::string networkId = audioActiveDevice_.GetCurrentOutputDeviceNetworkId();
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(
        audioActiveDevice_.GetCurrentOutputDeviceType());

    int32_t ret = LoadA2dpModule(DEVICE_TYPE_BLUETOOTH_A2DP_IN, audioStreamInfo, networkId, sinkName,
        sourceType);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "load a2dp input module failed");
}

void AudioDeviceCommon::TriggerRecreateCapturerStreamCallback(
    const std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
    int32_t streamFlag, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioDeviceCommon::TriggerRecreateCapturerStreamCallback");

    AUDIO_WARNING_LOG("Trigger recreate capturer stream, pid: %{public}d, sessionId: %{public}d, flag: %{public}d",
        capturerChangeInfo->callerPid, capturerChangeInfo->sessionId, streamFlag);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRecreateCapturerStreamEvent(capturerChangeInfo->callerPid,
            capturerChangeInfo->sessionId, streamFlag, reason);
    } else {
        AUDIO_WARNING_LOG("No audio policy server handler");
    }
}

int32_t AudioDeviceCommon::MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor)
{
    AUDIO_DEBUG_LOG("Start");
    // check
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_, ERR_INVALID_OPERATION,
        "failed: not a local device.");
    // start move.
    uint32_t sourceId = -1; // invalid source id, use source name instead.
    std::string sourceName = AudioPolicyUtils::GetInstance().GetSourcePortName(localDeviceDescriptor->deviceType_);
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, sourceName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sourceOutputs[i].paStreamId);
    }

    return SUCCESS;
}

int32_t AudioDeviceCommon::MoveToRemoteInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start");

    std::string networkId = remoteDeviceDescriptor->networkId_;
    DeviceRole deviceRole = remoteDeviceDescriptor->deviceRole_;
    DeviceType deviceType = remoteDeviceDescriptor->deviceType_;

    // check: networkid
    CHECK_AND_RETURN_RET_LOG(networkId != LOCAL_NETWORK_ID, ERR_INVALID_OPERATION,
        "failed: not a remote device.");

    uint32_t sourceId = -1; // invalid sink id, use sink name instead.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);

    AudioIOHandle moduleId;
    if (audioIOHandleMap_.GetModuleIdByKey(moduleName, moduleId)) {
        (void)moduleId; // mIOHandle is module id, not equal to sink id.
    } else {
        AUDIO_ERR_LOG("no such device.");
        if (!isOpenRemoteDevice) {
            return ERR_INVALID_PARAM;
        } else {
            return OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
        }
    }
    int32_t res = AudioServerProxy::GetInstance().CheckRemoteDeviceStateProxy(networkId, deviceRole, true);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "remote device state is invalid!");

    // start move.
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, moduleName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] failed", sourceOutputs[i].paStreamId);
    }

    if (deviceType != DeviceType::DEVICE_TYPE_DEFAULT) {
        AUDIO_DEBUG_LOG("Not defult type[%{public}d] on device:[%{public}s]",
            deviceType, GetEncryptStr(networkId).c_str());
    }
    return SUCCESS;
}

int32_t AudioDeviceCommon::ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
    ConnectState connectState)
{
    if (handleFlag && connectState != DEACTIVE_CONNECTED) {
        return SUCCESS;
    }
    return Bluetooth::AudioHfpManager::HandleScoWithRecongnition(handleFlag);
}

int32_t AudioDeviceCommon::HandleScoInputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos)
{
#ifdef BLUETOOTH_ENABLE
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch input device");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        FetchInputDevice(capturerChangeInfos);
        return ERROR;
    }
    Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true));
#endif
    return SUCCESS;
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

int32_t AudioDeviceCommon::MoveToRemoteOutputDevice(std::vector<SinkInput> sinkInputIds,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start for [%{public}zu] sink-inputs", sinkInputIds.size());

    std::string networkId = remoteDeviceDescriptor->networkId_;
    DeviceRole deviceRole = remoteDeviceDescriptor->deviceRole_;
    DeviceType deviceType = remoteDeviceDescriptor->deviceType_;

    // check: networkid
    CHECK_AND_RETURN_RET_LOG(networkId != LOCAL_NETWORK_ID, ERR_INVALID_OPERATION,
        "failed: not a remote device.");

    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);
    AudioIOHandle moduleId;
    if (audioIOHandleMap_.GetModuleIdByKey(moduleName, moduleId)) {
        (void)moduleId; // mIOHandle is module id, not equal to sink id.
    } else {
        AUDIO_ERR_LOG("no such device.");
        if (!isOpenRemoteDevice) {
            return ERR_INVALID_PARAM;
        } else {
            return OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
        }
    }
    int32_t res = AudioServerProxy::GetInstance().CheckRemoteDeviceStateProxy(networkId, deviceRole, true);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "remote device state is invalid!");

    // start move.
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, moduleName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "move [%{public}d] failed", sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, moduleName, sinkInputIds[i].pid);
    }

    if (deviceType != DeviceType::DEVICE_TYPE_DEFAULT) {
        AUDIO_WARNING_LOG("Not defult type[%{public}d] on device:[%{public}s]",
            deviceType, GetEncryptStr(networkId).c_str());
    }
    isCurrentRemoteRenderer = true;
    return SUCCESS;
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
    bool hasRunningRecognitionCapturerStream = streamCollector_.HasRunningRecognitionCapturerStream();
    if (hasRunningRecognitionCapturerStream) {
        return;
    }
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

int32_t AudioDeviceCommon::SwitchActiveA2dpDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(audioA2dpDevice_.CheckA2dpDeviceExist(deviceDescriptor->macAddress_),
        ERR_INVALID_PARAM, "the target A2DP device doesn't exist.");
    int32_t result = ERROR;
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("a2dp device name [%{public}s]", (deviceDescriptor->deviceName_).c_str());
    std::string lastActiveA2dpDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    audioActiveDevice_.SetActiveBtDeviceMac(deviceDescriptor->macAddress_);
    AudioDeviceDescriptor lastDevice = audioPolicyManager_.GetActiveDeviceDescriptor();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    if (Bluetooth::AudioA2dpManager::GetActiveA2dpDevice() == deviceDescriptor->macAddress_ &&
        audioIOHandleMap_.CheckIOHandleExist(BLUETOOTH_SPEAKER)) {
        AUDIO_WARNING_LOG("a2dp device [%{public}s] is already active",
            GetEncryptAddr(deviceDescriptor->macAddress_).c_str());
        return SUCCESS;
    }

    result = Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(deviceDescriptor->macAddress_);
    if (result != SUCCESS) {
        audioActiveDevice_.SetActiveBtDeviceMac(lastActiveA2dpDevice);
        AUDIO_ERR_LOG("Active [%{public}s] failed, using original [%{public}s] device",
            GetEncryptAddr(audioActiveDevice_.GetActiveBtDeviceMac()).c_str(),
            GetEncryptAddr(lastActiveA2dpDevice).c_str());
        return result;
    }

    AudioStreamInfo audioStreamInfo = {};
    audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo);
    std::string networkId = audioActiveDevice_.GetCurrentOutputDeviceNetworkId();
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(
        audioActiveDevice_.GetCurrentOutputDeviceType());
    result = LoadA2dpModule(DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, networkId, sinkName, SOURCE_TYPE_INVALID);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "LoadA2dpModule failed %{public}d", result);
#endif
    return result;
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
    if (uid == 5523) { /* 5523 call manager */
        return Bluetooth::AudioHfpManager::SetVirtualCall("meetimeservice", isVirtual);
    }
    std::string bundleName = AudioBundleManager::GetBundleNameFromUid(uid);
    return Bluetooth::AudioHfpManager::SetVirtualCall(bundleName, isVirtual);
}

void AudioDeviceCommon::SetHeadsetUnpluggedToSpkOrEpFlag(DeviceType oldDeviceType, DeviceType newDeviceType)
{
    if ((oldDeviceType == DEVICE_TYPE_USB_HEADSET || oldDeviceType == DEVICE_TYPE_USB_ARM_HEADSET) &&
        (newDeviceType == DEVICE_TYPE_SPEAKER || newDeviceType == DEVICE_TYPE_EARPIECE)) {
        isHeadsetUnpluggedToSpkOrEpFlag_ = true;
    }
}
}
}
