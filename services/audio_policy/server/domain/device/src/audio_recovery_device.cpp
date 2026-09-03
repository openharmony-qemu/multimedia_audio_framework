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
#include "audio_utils.h"

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
            result = AudioSelectInterfaceService::GetInstance().SelectInputDevice(audioCapturerFilter,
                deviceDescriptorVector);
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

} // namespace AudioStandard
} // namespace OHOS
