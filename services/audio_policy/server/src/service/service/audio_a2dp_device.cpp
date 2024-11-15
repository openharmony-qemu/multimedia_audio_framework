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
#define LOG_TAG "AudioA2dpDevice"
#endif

#include "audio_a2dp_device.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_policy_manager_factory.h"

#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

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

void AudioA2dpDevice::GetA2dpModuleInfo(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo)
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
}

int32_t AudioA2dpDevice::LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
    std::string networkID, std::string sinkName)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED,
        "A2dp module is not exist in the configuration file");

    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "source" ? INPUT_DEVICE : OUTPUT_DEVICE;
        DeviceRole deviceRole = deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load a2dp module [%{public}s], load role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            // a2dp device connects for the first time
            GetA2dpModuleInfo(moduleInfo, audioStreamInfo);
            AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo);
            CHECK_AND_RETURN_RET_LOG(ioHandle != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
                "OpenAudioPort failed %{public}d", ioHandle);
            audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);
        } else {
            // At least one a2dp device is already connected. A new a2dp device is connecting.
            // Need to reload a2dp module when switching to a2dp device.
            int32_t result = ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkID, sinkName);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "ReloadA2dpAudioPort failed %{public}d", result);
        }
    }

    return SUCCESS;
}

int32_t AudioA2dpDevice::ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, DeviceType deviceType,
    const AudioStreamInfo& audioStreamInfo, std::string networkID, std::string sinkName)
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
    GetA2dpModuleInfo(moduleInfo, audioStreamInfo);
    AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo);
    CHECK_AND_RETURN_RET_LOG(ioHandle != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
        "OpenAudioPort failed %{public}d", ioHandle);
    audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);
    return SUCCESS;
}

bool AudioA2dpDevice::GetA2dpDeviceInfo(const std::string& device, A2dpDeviceConfigInfo& info)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end() && configInfoPos->second.absVolumeSupport) {
        info.streamInfo = configInfoPos->second.streamInfo;
        info.absVolumeSupport = configInfoPos->second.absVolumeSupport;
        info.volumeLevel = configInfoPos->second.volumeLevel;
        info.mute = configInfoPos->second.mute;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::GetA2dpInDeviceInfo(const std::string& device, A2dpDeviceConfigInfo& info)
{
    std::lock_guard<std::mutex> lock(a2dpInDeviceMapMutex_);
    auto configInfoPos = connectedA2dpInDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpInDeviceMap_.end() && configInfoPos->second.absVolumeSupport) {
        info.streamInfo = configInfoPos->second.streamInfo;
        info.absVolumeSupport = configInfoPos->second.absVolumeSupport;
        info.volumeLevel = configInfoPos->second.volumeLevel;
        info.mute = configInfoPos->second.mute;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::GetA2dpDeviceVolumeLevel(const std::string& device, int32_t& volumeLevel)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        volumeLevel = configInfoPos->second.volumeLevel;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::CheckA2dpDeviceExist(const std::string& device)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        return true;
    }
    return false;
}

bool AudioA2dpDevice::SetA2dpDeviceMute(const std::string& device, bool mute)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos == connectedA2dpDeviceMap_.end() || !configInfoPos->second.absVolumeSupport) {
        AUDIO_WARNING_LOG("Set Mute failed for macAddress:[%{public}s]", GetEncryptAddr(device).c_str());
        return false;
    }
    configInfoPos->second.mute = mute;
    return true;
}

bool AudioA2dpDevice::GetA2dpDeviceMute(const std::string& device, bool& isMute)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos == connectedA2dpDeviceMap_.end()) {
        return false;
    }
    isMute = configInfoPos->second.mute;
    return true;
}

void AudioA2dpDevice::SetA2dpDeviceStreamInfo(const std::string& device, const DeviceStreamInfo& streamInfo)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    connectedA2dpDeviceMap_[device].streamInfo = streamInfo;
}

void AudioA2dpDevice::AddA2dpDevice(const std::string& device, const A2dpDeviceConfigInfo& config)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    connectedA2dpDeviceMap_.insert(make_pair(device, config));
}

void AudioA2dpDevice::AddA2dpInDevice(const std::string& device, const A2dpDeviceConfigInfo& config)
{
    std::lock_guard<std::mutex> lock(a2dpInDeviceMapMutex_);
    connectedA2dpInDeviceMap_.insert(make_pair(device, config));
}

size_t AudioA2dpDevice::DelA2dpInDevice(const std::string& device)
{
    std::lock_guard<std::mutex> lock(a2dpInDeviceMapMutex_);
    connectedA2dpInDeviceMap_.erase(device);
    return connectedA2dpInDeviceMap_.size();
}

size_t AudioA2dpDevice::DelA2dpDevice(const std::string& device)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    connectedA2dpDeviceMap_.erase(device);
    return connectedA2dpDeviceMap_.size();
}

bool AudioA2dpDevice::SetA2dpDeviceAbsVolumeSupport(const std::string& device, const bool support)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        configInfoPos->second.absVolumeSupport = support;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::SetA2dpDeviceVolumeLevel(const std::string& device, const int32_t volumeLevel)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos == connectedA2dpDeviceMap_.end() || !configInfoPos->second.absVolumeSupport) {
        AUDIO_WARNING_LOG("Set VolumeLevel failed for macAddress:[%{public}s]", GetEncryptAddr(device).c_str());
        return false;
    }
    configInfoPos->second.volumeLevel = volumeLevel;
    return true;
}

}
}