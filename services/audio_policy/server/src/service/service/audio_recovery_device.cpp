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
#define LOG_TAG "AudioRecoveryDevice"
#endif

#include "audio_recovery_device.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "media_monitor_manager.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {

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

void AudioRecoveryDevice::Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager)
{
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
}

void AudioRecoveryDevice::DeInit()
{
    audioA2dpOffloadManager_ = nullptr;
}

void AudioRecoveryDevice::RecoveryPreferredDevices()
{
    AUDIO_DEBUG_LOG("Start recovery peferred devices.");
    int32_t tryCounter = 5;
    // Waiting for 1000000 μs. Ensure that the playback/recording stream is restored first
    uint32_t firstSleepTime = 1000000;
    // Retry interval
    uint32_t sleepTime = 300000;
    int32_t result = -1;
    std::map<Media::MediaMonitor::PerferredType,
        std::shared_ptr<Media::MediaMonitor::MonitorDeviceInfo>> preferredDevices;
    usleep(firstSleepTime);
    while (result != SUCCESS && tryCounter-- > 0) {
        Media::MediaMonitor::MediaMonitorManager::GetInstance().GetAudioRouteMsg(preferredDevices);
        if (preferredDevices.size() == 0) {
            continue;
        }
        for (auto iter = preferredDevices.begin(); iter != preferredDevices.end(); ++iter) {
            result = HandleRecoveryPreferredDevices(static_cast<int32_t>(iter->first), iter->second->deviceType_,
                iter->second->usageOrSourceType_);
            if (result != SUCCESS) {
                AUDIO_ERR_LOG("Handle recovery preferred devices failed"
		    ", deviceType:%{public}d, usageOrSourceType:%{public}d, tryCounter:%{public}d",
                    iter->second->deviceType_, iter->second->usageOrSourceType_, tryCounter);
            }
        }
        if (result != SUCCESS) {
            usleep(sleepTime);
        }
    }
}

int32_t AudioRecoveryDevice::HandleRecoveryPreferredDevices(int32_t preferredType, int32_t deviceType,
    int32_t usageOrSourceType)
{
    int32_t result = -1;
    auto it = audioConnectedDevice_.GetConnectedDeviceByType(deviceType);
    if (it != nullptr) {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
        deviceDescriptorVector.push_back(it);
        if (preferredType == Media::MediaMonitor::MEDIA_RENDER ||
            preferredType == Media::MediaMonitor::CALL_RENDER ||
            preferredType == Media::MediaMonitor::RING_RENDER ||
            preferredType == Media::MediaMonitor::TONE_RENDER) {
            sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
            audioRendererFilter->uid = -1;
            audioRendererFilter->rendererInfo.streamUsage =
                static_cast<StreamUsage>(usageOrSourceType);
            result = SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
        } else if (preferredType == Media::MediaMonitor::CALL_CAPTURE ||
                    preferredType == Media::MediaMonitor::RECORD_CAPTURE) {
            sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
            audioCapturerFilter->uid = -1;
            audioCapturerFilter->capturerInfo.sourceType =
                static_cast<SourceType>(usageOrSourceType);
            result = SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
        }
    }
    return result;
}

int32_t AudioRecoveryDevice::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    AUDIO_WARNING_LOG("uid[%{public}d] type[%{public}d] mac[%{public}s] streamUsage[%{public}d] pid[%{public}d]",
        audioRendererFilter->uid, selectedDesc[0]->deviceType_, GetEncryptAddr(selectedDesc[0]->macAddress_).c_str(),
        audioRendererFilter->rendererInfo.streamUsage, IPCSkeleton::GetCallingPid());

    CHECK_AND_RETURN_RET_LOG((selectedDesc[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE) &&
        (selectedDesc.size() == 1), ERR_INVALID_OPERATION, "DeviceCheck no success");
    if (audioRendererFilter->uid != -1) { return SelectOutputDeviceByFilterInner(audioRendererFilter, selectedDesc); }
    if (audioRendererFilter->rendererInfo.rendererFlags == STREAM_FLAG_FAST) {
        return SelectOutputDeviceForFastInner(audioRendererFilter, selectedDesc);
    }
    bool isVirtualDevice = false;
    if (selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
        selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        selectedDesc[0]->isEnable_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(selectedDesc[0], ENABLE_UPDATE);
        isVirtualDevice = audioDeviceManager_.IsVirtualConnectedDevice(selectedDesc[0]);
        if (isVirtualDevice == true) {
            selectedDesc[0]->connectState_ = VIRTUAL_CONNECTED;
        }
    }
    if (selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(selectedDesc[0]->macAddress_);
    }
    StreamUsage strUsage = audioRendererFilter->rendererInfo.streamUsage;
    int32_t res = SetRenderDeviceForUsage(strUsage, selectedDesc[0]);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "SetRenderDeviceForUsage fail");

    // If the selected device is virtual device, connect it.
    if (isVirtualDevice) {
        int32_t ret = ConnectVirtualDevice(selectedDesc[0]);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Connect device [%{public}s] failed",
            GetEncryptStr(selectedDesc[0]->macAddress_).c_str());
        return SUCCESS;
    }

    audioActiveDevice_.NotifyUserSelectionEventToBt(selectedDesc[0]);
    audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    audioDeviceCommon_.FetchDevice(false);
    audioCapturerSession_.ReloadSourceForDeviceChange(
        audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SelectOutputDevice");
    if ((selectedDesc[0]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
        (selectedDesc[0]->networkId_ != LOCAL_NETWORK_ID)) {
        audioA2dpOffloadManager_->UpdateOffloadWhenActiveDeviceSwitchFromA2dp();
    } else {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(selectedDesc[0]->deviceType_);
    }
    audioDeviceCommon_.OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice());
    WriteSelectOutputSysEvents(selectedDesc, strUsage);
    return SUCCESS;
}

int32_t AudioRecoveryDevice::SelectOutputDeviceForFastInner(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    int32_t res = SetRenderDeviceForUsage(audioRendererFilter->rendererInfo.streamUsage, selectedDesc[0]);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "SetRenderDeviceForUsage fail");
    SetRenderDeviceForUsage(audioRendererFilter->rendererInfo.streamUsage, selectedDesc[0]);
    res = SelectFastOutputDevice(audioRendererFilter, selectedDesc[0]);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res,
        "AddFastRouteMapInfo failed! fastRouteMap is too large!");
    audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    return true;
}

int32_t AudioRecoveryDevice::SetRenderDeviceForUsage(StreamUsage streamUsage,
    std::shared_ptr<AudioDeviceDescriptor> desc)
{
    // get deviceUsage and perferedType
    auto deviceUsage = MEDIA_OUTPUT_DEVICES;
    auto perferedType = AUDIO_MEDIA_RENDER;
    auto tempId = desc->deviceId_;
    if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION || streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        deviceUsage = CALL_OUTPUT_DEVICES;
        perferedType = AUDIO_CALL_RENDER;
    }
    // find device
    auto devices = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(deviceUsage);
    auto itr = std::find_if(devices.begin(), devices.end(), [&desc](const auto &device) {
        return (desc->deviceType_ == device->deviceType_) &&
            (desc->macAddress_ == device->macAddress_) &&
            (desc->networkId_ == device->networkId_) &&
            (!IsUsb(desc->deviceType_) || desc->deviceRole_ == device->deviceRole_);
    });
    CHECK_AND_RETURN_RET_LOG(itr != devices.end(), ERR_INVALID_OPERATION,
        "device not available type:%{public}d macAddress:%{public}s id:%{public}d networkId:%{public}s",
        desc->deviceType_, GetEncryptAddr(desc->macAddress_).c_str(),
        tempId, GetEncryptStr(desc->networkId_).c_str());
    // set preferred device
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>(**itr);
    CHECK_AND_RETURN_RET_LOG(descriptor != nullptr, ERR_INVALID_OPERATION, "Create device descriptor failed");
    AudioPolicyUtils::GetInstance().SetPreferredDevice(perferedType, descriptor);
    return SUCCESS;
}

int32_t AudioRecoveryDevice::ConnectVirtualDevice(std::shared_ptr<AudioDeviceDescriptor> &selectedDesc)
{
    int32_t ret = Bluetooth::AudioA2dpManager::Connect(selectedDesc->macAddress_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "A2dp connect failed");
    ret = Bluetooth::AudioHfpManager::Connect(selectedDesc->macAddress_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Hfp connect failed");
    AUDIO_INFO_LOG("Connect virtual device[%{public}s]", GetEncryptAddr(selectedDesc->macAddress_).c_str());
    return SUCCESS;
}

void AudioRecoveryDevice::WriteSelectOutputSysEvents(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &selectedDesc,
    StreamUsage strUsage)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::SET_FORCE_USE_AUDIO_DEVICE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    AudioDeviceDescriptor curOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    bean->Add("CLIENT_UID", static_cast<int32_t>(IPCSkeleton::GetCallingUid()));
    bean->Add("DEVICE_TYPE", curOutputDeviceDesc.deviceType_);
    bean->Add("STREAM_TYPE", strUsage);
    bean->Add("BT_TYPE", curOutputDeviceDesc.deviceCategory_);
    bean->Add("DEVICE_NAME", curOutputDeviceDesc.deviceName_);
    bean->Add("ADDRESS", curOutputDeviceDesc.macAddress_);
    bean->Add("IS_PLAYBACK", 1);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioRecoveryDevice::SelectFastOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    AUDIO_INFO_LOG("Start for uid[%{public}d] device[%{public}s]", audioRendererFilter->uid,
        GetEncryptStr(deviceDescriptor->networkId_).c_str());
    // note: check if stream is already running
    // if is running, call moveProcessToEndpoint.

    // otherwises, keep router info in the map
    int32_t res = audioRouteMap_.AddFastRouteMapInfo(audioRendererFilter->uid, deviceDescriptor->networkId_,
        OUTPUT_DEVICE);
    return res;
}

int32_t AudioRecoveryDevice::SelectOutputDeviceByFilterInner(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    if (selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
        selectedDesc[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        selectedDesc[0]->isEnable_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(selectedDesc[0], ENABLE_UPDATE);
        bool isVirtualDevice = audioDeviceManager_.IsVirtualConnectedDevice(selectedDesc[0]);
        if (isVirtualDevice == true) {
            selectedDesc[0]->connectState_ = VIRTUAL_CONNECTED;
        }
    }
    audioAffinityManager_.AddSelectRendererDevice(audioRendererFilter->uid, selectedDesc[0]);
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);
    for (auto &changeInfo : rendererChangeInfos) {
        if (changeInfo->clientUID == audioRendererFilter->uid && changeInfo->sessionId != 0) {
            AudioServerProxy::GetInstance().RestoreSessionProxy(changeInfo->sessionId, true);
        }
    }
    return SUCCESS;
}

int32_t AudioRecoveryDevice::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    AUDIO_WARNING_LOG("uid[%{public}d] type[%{public}d] mac[%{public}s] pid[%{public}d]",
        audioCapturerFilter->uid, selectedDesc[0]->deviceType_,
        GetEncryptAddr(selectedDesc[0]->macAddress_).c_str(), IPCSkeleton::GetCallingPid());
    // check size == 1 && input device
    int32_t res = audioDeviceCommon_.DeviceParamsCheck(DeviceRole::INPUT_DEVICE, selectedDesc);
    CHECK_AND_RETURN_RET(res == SUCCESS, res);
    if (audioCapturerFilter->uid != -1) {
        audioAffinityManager_.AddSelectCapturerDevice(audioCapturerFilter->uid, selectedDesc[0]);
        vector<shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
        streamCollector_.GetCurrentCapturerChangeInfos(capturerChangeInfos);
        for (auto &changeInfo : capturerChangeInfos) {
            if (changeInfo->clientUID == audioCapturerFilter->uid && changeInfo->sessionId != 0) {
                AudioServerProxy::GetInstance().RestoreSessionProxy(changeInfo->sessionId, false);
            }
        }
        return SUCCESS;
    }

    SourceType srcType = audioCapturerFilter->capturerInfo.sourceType;

    if (audioCapturerFilter->capturerInfo.capturerFlags == STREAM_FLAG_FAST && selectedDesc.size() == 1) {
        SetCaptureDeviceForUsage(audioSceneManager_.GetAudioScene(true), srcType, selectedDesc[0]);
        int32_t result = SelectFastInputDevice(audioCapturerFilter, selectedDesc[0]);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
            "AddFastRouteMapInfo failed! fastRouteMap is too large!");
        AUDIO_INFO_LOG("Success for uid[%{public}d] device[%{public}s]",
            audioCapturerFilter->uid, GetEncryptStr(selectedDesc[0]->networkId_).c_str());
        audioDeviceCommon_.FetchDevice(false);
        audioCapturerSession_.ReloadSourceForDeviceChange(
            audioActiveDevice_.GetCurrentInputDevice(),
            audioActiveDevice_.GetCurrentOutputDevice(), "SelectInputDevice fast");
        return SUCCESS;
    }

    AudioScene scene = audioSceneManager_.GetAudioScene(true);
    if (scene == AUDIO_SCENE_PHONE_CALL || scene == AUDIO_SCENE_PHONE_CHAT ||
        srcType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE, selectedDesc[0]);
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE, selectedDesc[0]);
    }
    audioDeviceCommon_.FetchDevice(false);

    WriteSelectInputSysEvents(selectedDesc, srcType, scene);
    audioCapturerSession_.ReloadSourceForDeviceChange(
        audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SelectInputDevice");
    return SUCCESS;
}

void AudioRecoveryDevice::SetCaptureDeviceForUsage(AudioScene scene, SourceType srcType,
    std::shared_ptr<AudioDeviceDescriptor> desc)
{
    AUDIO_INFO_LOG("Scene: %{public}d, srcType: %{public}d", scene, srcType);
    if (scene == AUDIO_SCENE_PHONE_CALL || scene == AUDIO_SCENE_PHONE_CHAT ||
        srcType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE, desc);
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE, desc);
    }
}

int32_t AudioRecoveryDevice::SelectFastInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    // note: check if stream is already running
    // if is running, call moveProcessToEndpoint.

    // otherwises, keep router info in the map
    int32_t res = audioRouteMap_.AddFastRouteMapInfo(audioCapturerFilter->uid,
        deviceDescriptor->networkId_, INPUT_DEVICE);
    return res;
}

void AudioRecoveryDevice::WriteSelectInputSysEvents(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &selectedDesc,
    SourceType srcType, AudioScene scene)
{
    auto uid = IPCSkeleton::GetCallingUid();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::SET_FORCE_USE_AUDIO_DEVICE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(uid));
    bean->Add("DEVICE_TYPE", selectedDesc[0]->deviceType_);
    bean->Add("STREAM_TYPE", srcType);
    bean->Add("BT_TYPE", selectedDesc[0]->deviceCategory_);
    bean->Add("DEVICE_NAME", selectedDesc[0]->deviceName_);
    bean->Add("ADDRESS", selectedDesc[0]->macAddress_);
    bean->Add("AUDIO_SCENE", scene);
    bean->Add("IS_PLAYBACK", 0);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

}
}
