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

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_event_utils.h"
#include "audio_core_service.h"
#include "audio_select_interface_service.h"
#include "audio_router_select_strategy.h"

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t RECOVERY_ATTEMPT_LIMIT = 5;
constexpr uint32_t INITIAL_STREAM_RESTORATION_WAIT_US = 1000000;
constexpr uint32_t RETRY_INTERVAL_US = 300000;
constexpr int32_t BLUETOOTH_UID = 1002;
} // namespace

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

static bool IsBtRecognition(const sptr<AudioCapturerFilter> &filter)
{
    CHECK_AND_RETURN_RET(filter, false);
    return filter->uid == BLUETOOTH_UID && filter->capturerInfo.sourceType == SOURCE_TYPE_VOICE_RECOGNITION;
}

static void SetPreferredDevice(AudioScene scene, SourceType srcType,
    const sptr<AudioCapturerFilter> &audioCapturerFilter, const shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (scene == AUDIO_SCENE_PHONE_CALL || scene == AUDIO_SCENE_PHONE_CHAT ||
        srcType == SOURCE_TYPE_VOICE_COMMUNICATION || srcType == SOURCE_TYPE_INTERPHONE) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE, desc);
    } else if (IsBtRecognition(audioCapturerFilter)) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECOGNITION_CAPTURE, desc);
    } else {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_RECORD_CAPTURE, desc);
    }
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
    AUDIO_DEBUG_LOG("Start recovery preferred devices.");
    int32_t tryCounter = RECOVERY_ATTEMPT_LIMIT;
    // Waiting for 1000000 μs. Ensure that the playback/recording stream is restored first
    uint32_t firstSleepTime = INITIAL_STREAM_RESTORATION_WAIT_US;
    // Retry interval
    uint32_t sleepTime = RETRY_INTERVAL_US;
    int32_t result = -1;
    std::map<Media::MediaMonitor::PreferredType,
        std::shared_ptr<Media::MediaMonitor::MonitorDeviceInfo>> preferredDevices;
    usleep(firstSleepTime);
    while (result != SUCCESS && tryCounter > 0) {
        tryCounter--;
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
            CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, result, "audioRendererFilter is nullptr.");
            audioRendererFilter->uid = -1;
            audioRendererFilter->rendererInfo.streamUsage =
                static_cast<StreamUsage>(usageOrSourceType);
            result = AudioSelectInterfaceService::GetInstance().SelectOutputDevice(
                audioRendererFilter, deviceDescriptorVector);
        } else if (preferredType == Media::MediaMonitor::CALL_CAPTURE ||
                    preferredType == Media::MediaMonitor::RECORD_CAPTURE) {
            sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
            CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr, result, "audioCapturerFilter is nullptr.");
            audioCapturerFilter->uid = -1;
            audioCapturerFilter->capturerInfo.sourceType =
                static_cast<SourceType>(usageOrSourceType);
            result = SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
        }
    }
    return result;
}

void AudioRecoveryDevice::RecoverExcludedOutputDevices()
{
    AUDIO_INFO_LOG("[ADeviceEvent] Start recover excluded output devices");
    int32_t tryCounter = RECOVERY_ATTEMPT_LIMIT;
    // Waiting for 1000000 μs. Ensure that the playback/recording stream is restored first
    uint32_t firstSleepTime = INITIAL_STREAM_RESTORATION_WAIT_US;
    // Retry interval
    uint32_t sleepTime = RETRY_INTERVAL_US;
    int32_t result = -1;
    map<Media::MediaMonitor::AudioDeviceUsage,
        vector<shared_ptr<Media::MediaMonitor::MonitorDeviceInfo>>> excludedDevicesMap;
    usleep(firstSleepTime);
    while (result != SUCCESS && tryCounter > 0) {
        tryCounter--;
        Media::MediaMonitor::MediaMonitorManager::GetInstance().GetAudioExcludedDevicesMsg(excludedDevicesMap);
        for (auto iter = excludedDevicesMap.begin(); iter != excludedDevicesMap.end(); ++iter) {
            result = HandleExcludedOutputDevicesRecovery(static_cast<AudioDeviceUsage>(iter->first), iter->second);
            CHECK_AND_CONTINUE_LOG(result == SUCCESS, "Handle usage[%{public}d] excluded devices recovery failed",
                iter->first);
        }
        if (result != SUCCESS) {
            usleep(sleepTime);
        }
    }
}

int32_t AudioRecoveryDevice::HandleExcludedOutputDevicesRecovery(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<Media::MediaMonitor::MonitorDeviceInfo>> &excludedDevices)
{
    vector<shared_ptr<AudioDeviceDescriptor>> excludedOutputDevices;
    for (auto &device : excludedDevices) {
        auto it = audioConnectedDevice_.GetConnectedDeviceByType(device->networkId_,
            static_cast<DeviceType>(device->deviceType_), device->address_);
        if (it != nullptr) {
            excludedOutputDevices.push_back(it);
        }
    }
    if (!excludedOutputDevices.empty()) {
        return AudioSelectInterfaceService::GetInstance().ExcludeOutputDevices(audioDevUsage, excludedOutputDevices);
    }
    return ERROR;
}

int32_t AudioRecoveryDevice::ConnectVirtualDevice(std::shared_ptr<AudioDeviceDescriptor> &selectedDesc)
{
    CHECK_AND_RETURN_RET_LOG(selectedDesc != nullptr, ERROR_INVALID_PARAM, "selectedDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(selectedDesc->networkId_ == LOCAL_NETWORK_ID, SUCCESS,
        "Virtual remote device, not process");

    AUDIO_INFO_LOG("Connect virtual device[%{public}s]", GetEncryptAddr(selectedDesc->macAddress_).c_str());
    if (selectedDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
        selectedDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        Bluetooth::AudioA2dpManager::Connect(selectedDesc->macAddress_);
        Bluetooth::AudioHfpManager::Connect(selectedDesc->macAddress_);
    } else {
        int32_t result = SleAudioDeviceManager::GetInstance().ConnectAllowedProfiles(selectedDesc->macAddress_);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Nearlink connect failed");
    }
    return SUCCESS;
}

int32_t AudioRecoveryDevice::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr && !selectedDesc.empty() && selectedDesc[0] != nullptr,
        ERROR, "ptr exception");
    AUDIO_WARNING_LOG("uid[%{public}d] type[%{public}d] mac[%{public}s] pid[%{public}d]",
        audioCapturerFilter->uid, selectedDesc[0]->deviceType_,
        GetEncryptAddr(selectedDesc[0]->macAddress_).c_str(), IPCSkeleton::GetCallingPid());
    // check size == 1 && input device
    int32_t res = audioDeviceCommon_.DeviceParamsCheck(DeviceRole::INPUT_DEVICE, selectedDesc);
    CHECK_AND_RETURN_RET(res == SUCCESS, res);

    SourceType srcType = audioCapturerFilter->capturerInfo.sourceType;

    if (audioCapturerFilter->capturerInfo.capturerFlags == STREAM_FLAG_FAST && selectedDesc.size() == 1) {
        SetCaptureDeviceForUsage(audioSceneManager_.GetAudioScene(true), srcType, selectedDesc[0]);
        int32_t result = SelectFastInputDevice(audioCapturerFilter, selectedDesc[0]);
        CheckAndWriteDeviceChangeExceptionEvent(result == SUCCESS, AudioStreamDeviceChangeReason::OVERRODE,
            selectedDesc[0]->deviceType_, selectedDesc[0]->deviceRole_, result, "AddFastRouteMapInfo failed!");
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
            "AddFastRouteMapInfo failed! fastRouteMap is too large!");
        AUDIO_INFO_LOG("Success for uid[%{public}d] device[%{public}s]",
            audioCapturerFilter->uid, GetEncryptStr(selectedDesc[0]->networkId_).c_str());
        AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("SelectInputDevice_1");
        AudioCoreService::GetCoreService()->ReloadSourceForDeviceChange(
            AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice(),
            AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(), "SelectInputDevice fast");
        return SUCCESS;
    }
    auto ret = RefreshVirtualDevice(selectedDesc[0]);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AudioScene scene = audioSceneManager_.GetAudioScene(true);
    SetPreferredDevice(scene, srcType, audioCapturerFilter, selectedDesc[0]);
    audioActiveDevice_.NotifyUserSelectionEventForInput(selectedDesc[0], srcType);
    AudioCoreService::GetCoreService()->FetchInputDeviceAndRoute("SelectInputDevice_2");

    WriteSelectInputSysEvents(selectedDesc, srcType, scene);
    AudioCoreService::GetCoreService()->ReloadSourceForDeviceChange(
        AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice(),
        AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(), "SelectInputDevice");
    return SUCCESS;
}

int32_t AudioRecoveryDevice::RefreshVirtualDevice(shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_RET_LOG(desc, ERR_NULL_POINTER, "desc is nullptr");
    bool isVirtualDevice = audioDeviceManager_.IsVirtualConnectedDevice(desc);
    if (isVirtualDevice == true) {
        desc->connectState_ = VIRTUAL_CONNECTED;
    }
    AudioSelectInterfaceService::GetInstance().SetDeviceEnableAndUsage(desc);
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState(desc->macAddress_);
    }
    // If the selected device is virtual device, connect it.
    if (isVirtualDevice) {
        int32_t ret = ConnectVirtualDevice(desc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Connect device [%{public}s] failed",
            GetEncryptStr(desc->macAddress_).c_str());
    }
    return SUCCESS;
}

void AudioRecoveryDevice::SetCaptureDeviceForUsage(AudioScene scene, SourceType srcType,
    std::shared_ptr<AudioDeviceDescriptor> desc)
{
    AUDIO_INFO_LOG("Scene: %{public}d, srcType: %{public}d", scene, srcType);
    if (scene == AUDIO_SCENE_PHONE_CALL || scene == AUDIO_SCENE_PHONE_CHAT ||
        srcType == SOURCE_TYPE_VOICE_COMMUNICATION || srcType == SOURCE_TYPE_INTERPHONE) {
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

int32_t AudioRecoveryDevice::ClearActiveHfpDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        return Bluetooth::AudioHfpManager::ClearActiveHfpDevice(desc->macAddress_);
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
