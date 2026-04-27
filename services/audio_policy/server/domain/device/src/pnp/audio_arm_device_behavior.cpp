/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioArmDeviceBehavior"
#endif

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_active_device.h"
#include "audio_core_service.h"
#include "audio_iohandle_map.h"
#include "audio_core_config_manager.h"
#include "audio_policy_utils.h"
#include "audio_pipe_manager.h"

#include "audio_arm_device_behavior.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static constexpr int32_t MS_PER_S = 1000;
static constexpr uint64_t ROUNDING_HALF_DIVISOR = 2ULL;
static const unsigned int BUFFER_CALC_20MS = 20;
static std::map<AudioSampleFormat, std::string> formatEnumToStr = {
    {SAMPLE_U8, "s16le"},
    {SAMPLE_S16LE, "s16le"},
    {SAMPLE_S24LE, "s24le"},
    {SAMPLE_S32LE, "s32le"},
    {SAMPLE_F32LE, "s16le"},
};

static std::map<std::string, AudioSampleFormat> formatStrToEnum = {
    {"s8", SAMPLE_U8},
    {"s16", SAMPLE_S16LE},
    {"s24", SAMPLE_S24LE},
    {"s32", SAMPLE_S32LE},
    {"s16le", SAMPLE_S16LE},
    {"s24le", SAMPLE_S24LE},
    {"s32le", SAMPLE_S32LE},
};

static bool CalculateArmBufferDurationMs(const AudioModuleInfo &moduleInfo, uint32_t &buffCalcMs)
{
    CHECK_AND_RETURN_RET(!(moduleInfo.rate.empty() || moduleInfo.format.empty() || moduleInfo.channels.empty() ||
        moduleInfo.bufferSize.empty()), true);

    uint32_t rateValue = 0;
    uint32_t channelValue = 0;
    uint32_t bufferSizeValue = 0;
    CHECK_AND_RETURN_RET_LOG(StringConverter(moduleInfo.rate, rateValue), false,
        "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
    CHECK_AND_RETURN_RET_LOG(StringConverter(moduleInfo.channels, channelValue), false,
        "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());
    CHECK_AND_RETURN_RET_LOG(StringConverter(moduleInfo.bufferSize, bufferSizeValue), false,
        "convert invalid original bufferSize: %{public}s", moduleInfo.bufferSize.c_str());

    const uint32_t bytesPerSample =
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(formatStrToEnum[moduleInfo.format]);
    CHECK_AND_RETURN_RET_LOG(bytesPerSample > 0, false, "bytesPerSample is 0");

    const uint64_t denom = static_cast<uint64_t>(rateValue) *
                           static_cast<uint64_t>(channelValue) *
                           static_cast<uint64_t>(bytesPerSample);
    CHECK_AND_RETURN_RET_LOG(denom > 0, false, "denom is 0");
    const uint64_t number = static_cast<uint64_t>(bufferSizeValue) *
                            static_cast<uint64_t>(MS_PER_S);
    buffCalcMs = static_cast<uint32_t>((number + denom / ROUNDING_HALF_DIVISOR) / denom);
    return true;
}
} // namespace

int32_t AudioArmDeviceBehavior::ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && !streamDesc->newDeviceDescs_.empty() &&
        streamDesc->newDeviceDescs_.front() != nullptr, ERR_NULL_POINTER, "param is invalid");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();

    std::string address = deviceDesc->GetMacAddress();
    DeviceRole role = deviceDesc->getRole();
    AUDIO_INFO_LOG("address=%{public}s, role=%{public}d",
        AudioPolicyUtils::GetInstance().EncUsbAddr(address).c_str(), role);
    string &activeArmAddr = role == INPUT_DEVICE ? activeArmInputAddr_ : activeArmOutputAddr_;
    CHECK_AND_RETURN_RET_LOG(address != activeArmAddr, SUCCESS, "usb device addr already active");
    std::list<AudioModuleInfo> moduleInfoList;
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_INVALID_PARAM, "pipeManager is nullptr");
    bool ret = pipeManager->GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "GetModuleListByType empty");
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != role) {continue;}
        AUDIO_INFO_LOG("[module_reload]: module[%{public}s], role[%{public}d]", moduleInfo.name.c_str(), role);
        auto coreService = AudioCoreService::GetCoreService();
        CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is nullptr");
        bool isEcFeatureEnable = coreService->GetEcFeatureEnable();
        auto currentOutputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice();
        if (!(isEcFeatureEnable && role == INPUT_DEVICE) &&
            AudioIOHandleMap::GetInstance().CheckIOHandleExist(moduleInfo.name)) {
            AudioIOHandleMap::GetInstance().MuteDefaultSinkPort(currentOutputDevice.networkId_,
                AudioPolicyUtils::GetInstance().GetSinkPortName(currentOutputDevice.deviceType_));
            AudioIOHandleMap::GetInstance().ClosePortAndEraseIOHandle(moduleInfo.name);
        }
        UpdateArmModuleInfo(deviceDesc, moduleInfo);
        if (isEcFeatureEnable) {
            if (role == OUTPUT_DEVICE) {
                int32_t ret = AudioIOHandleMap::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
                CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM,
                    "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), ret);
                coreService->SetUsbSinkModuleInfo(moduleInfo);
            } else {
                AUDIO_INFO_LOG("just save arm usb source module info, rate=%{public}s", moduleInfo.rate.c_str());
                coreService->SetUsbSourceModuleInfo(moduleInfo);
                pipeManager->UpdateDynamicCapturerConfig(ClassType::TYPE_USB, moduleInfo);
            }
        } else {
            int32_t ret = AudioIOHandleMap::GetInstance().OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM,
                "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), ret);
        }
    }
    activeArmAddr = address;
    return SUCCESS;
}

void AudioArmDeviceBehavior::UpdateArmModuleInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "AudioDeviceDescriptor is nullptr");
    uint32_t buffCalcMs = BUFFER_CALC_20MS;
    CHECK_AND_RETURN_LOG(CalculateArmBufferDurationMs(moduleInfo, buffCalcMs),
        "calculate arm buffer duration ms failed");
    CHECK_AND_RETURN_LOG(deviceDesc->GetAudioStreamInfo().size() != 0, "AudioStreamInfo is empty");
    DeviceStreamInfo deviceStreamInfo = deviceDesc->GetAudioStreamInfo().back();
    CHECK_AND_RETURN_LOG(!deviceStreamInfo.samplingRate.empty(), "samplingRate is empty");
    std::string halRate = std::to_string(*deviceStreamInfo.samplingRate.begin());
    std::string halFormat = formatEnumToStr[deviceStreamInfo.format];
    AUDIO_INFO_LOG("RATE from hal is: %{public}s, format from hal is: %{public}s, format ori: %{public}u",
        halRate.c_str(), halFormat.c_str(), deviceStreamInfo.format);
    if (!halRate.empty()) {
        moduleInfo.rate = halRate;
    }
    if (!halFormat.empty()) {
        moduleInfo.format = halFormat;
    }

    if (!moduleInfo.rate.empty() && !moduleInfo.format.empty() && !moduleInfo.channels.empty()) {
        uint32_t rateValue = 0;
        uint32_t channelValue = 0;
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.rate, rateValue),
            "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.channels, channelValue),
            "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());

        uint32_t bufferSize = rateValue * channelValue *
            AudioPolicyUtils::GetInstance().PcmFormatToBytes(formatStrToEnum[moduleInfo.format]) *
            buffCalcMs / static_cast<uint32_t>(MS_PER_S);
        moduleInfo.bufferSize = std::to_string(bufferSize);
        AUDIO_INFO_LOG("update arm usb buffer size: %{public}s", moduleInfo.bufferSize.c_str());
    }
    
    moduleInfo.macAddress = deviceDesc->GetMacAddress();
}

int32_t AudioArmDeviceBehavior::DeactivateDevice()
{
    return SUCCESS;
}

int32_t AudioArmDeviceBehavior::DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "deviceDesc is nullptr");
    AUDIO_INFO_LOG("address=%{public}s, role=%{public}d",
        AudioPolicyUtils::GetInstance().EncUsbAddr(deviceDesc->GetMacAddress()).c_str(), deviceDesc->getRole());
    string &activeArmAddr = deviceDesc->getRole() == INPUT_DEVICE ? activeArmInputAddr_ : activeArmOutputAddr_;
    CHECK_AND_RETURN_RET_LOG(deviceDesc->GetMacAddress() == activeArmAddr, SUCCESS, "usb device addr not match");
    std::list<AudioModuleInfo> moduleInfoList;
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_INVALID_PARAM, "pipeManager is nullptr");
    bool ret = pipeManager->GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "GetModuleListByType Failed");
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != deviceDesc->getRole()) {continue;}
        if (AudioIOHandleMap::GetInstance().CheckIOHandleExist(moduleInfo.name)) {
            AudioIOHandleMap::GetInstance().ClosePortAndEraseIOHandle(moduleInfo.name);
        }
    }
    activeArmAddr = "";
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
