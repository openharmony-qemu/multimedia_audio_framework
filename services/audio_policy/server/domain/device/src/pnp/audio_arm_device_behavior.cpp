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
#include "audio_definition_policy_utils.h"
#include "audio_policy_utils.h"
#include "audio_pipe_manager.h"
#include "audio_scene_manager.h"

#include "audio_arm_device_behavior.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static constexpr int32_t MS_PER_S = 1000;
static constexpr uint64_t ROUNDING_HALF_DIVISOR = 2ULL;
static const unsigned int BUFFER_CALC_20MS = (20 * SOURCE_PERIOD_SIZE);
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
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is nullptr");
    bool isEcFeatureEnable = coreService->GetEcFeatureEnable();
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != role) {continue;}
        AUDIO_INFO_LOG("[module_reload]: module[%{public}s], role[%{public}d]", moduleInfo.name.c_str(), role);
        int32_t result = ProcessModuleActivation(deviceDesc, moduleInfo, role, isEcFeatureEnable);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
            "ProcessModuleActivation %{public}s failed %{public}d", moduleInfo.role.c_str(), result);
    }
    activeArmAddr = address;
    return SUCCESS;
}

int32_t AudioArmDeviceBehavior::ReloadArmAudioPort(const AudioModuleInfo &moduleInfo)
{
    AUDIO_INFO_LOG("Switch device from arm to another arm, reload arm module");
    auto currentOutputDevice = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice();
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort(currentOutputDevice.networkId_,
        AudioPolicyUtils::GetInstance().GetSinkPortName(currentOutputDevice.deviceType_));

    AudioIOHandle activateDeviceIOHandle = HDI_INVALID_ID;
    bool result = AudioIOHandleMap::GetInstance().GetModuleIdByKey(moduleInfo.name, activateDeviceIOHandle);
    CHECK_AND_RETURN_RET_LOG(result, ERR_INVALID_PARAM,
        "can not find %{public}s in io map", moduleInfo.name.c_str());

    uint32_t paIndex = 0;
    AudioIOHandle ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager()
        .ReloadA2dpAudioPort(moduleInfo, paIndex);
    CHECK_AND_CALL_FUNC_RETURN_RET(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        HILOG_COMM_ERROR("ReloadArmAudioPort failed for ioHandle[%{public}u]", ioHandle));
    CHECK_AND_CALL_FUNC_RETURN_RET(paIndex != 0, ERR_INVALID_PARAM,
        HILOG_COMM_ERROR("ReloadArmAudioPort failed for paId[%{public}u]", paIndex));
    AudioIOHandleMap::GetInstance().AddIOHandleInfo(moduleInfo.name, ioHandle);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");

    auto pipeInfo = pipeManager->GetPipeinfoById(activateDeviceIOHandle);
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, ERR_NULL_POINTER, "pipeInfo is nullptr");
    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->InitAudioStreamInfo();

    AudioAdapterManager::GetInstance().RemoveAudioPipeVolume(activateDeviceIOHandle);
    auto pipeDeviceVolumeGroup = pipeManager->GetPipeDeviceVolumeInfoForPipe(pipeInfo);
    AudioAdapterManager::GetInstance().UpdateAudioPipeVolume(pipeDeviceVolumeGroup);

    AUDIO_INFO_LOG("open paIndex: %{public}u", paIndex);
    return SUCCESS;
}

SelectedDeviceFormat AudioArmDeviceBehavior::DetermineArmUsbFormat(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    const DeviceStreamInfo &deviceStreamInfo)
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    if (pipeManager != nullptr) {
        auto allStreams = pipeManager->GetAllOutputStreamDescs();
        std::string macAddr = deviceDesc->GetMacAddress();
        if (AudioSceneManager::GetInstance().GetAudioScene() != AUDIO_SCENE_DEFAULT) {
            AUDIO_INFO_LOG("ArmUsb call scene active, use 48k fixed format");
            return {AudioSamplingRate::SAMPLE_RATE_48000, AudioSampleFormat::SAMPLE_S16LE,
                AudioChannelLayout::CH_LAYOUT_STEREO};
        }
        AudioStreamInfo highestInfo;
        if (AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(macAddr, allStreams, nullptr, highestInfo)) {
            auto sel = AudioDefinitionPolicyUtils::SelectBestOutputFormat(highestInfo,
                deviceDesc->GetAudioStreamInfo());
            AUDIO_INFO_LOG("ArmUsb highest stream: rate=%{public}u, format=%{public}u",
                static_cast<uint32_t>(sel.rate), static_cast<uint32_t>(sel.format));
            return sel;
        }
    }
    SelectedDeviceFormat selected;
    selected.rate = static_cast<AudioSamplingRate>(*deviceStreamInfo.samplingRate.rbegin());
    selected.format = deviceStreamInfo.format;
    selected.channelLayout = deviceStreamInfo.GetMainChannelLayout();
    AUDIO_INFO_LOG("ArmUsb no running stream, use HAL default: rate=%{public}u",
        static_cast<uint32_t>(selected.rate));
    return selected;
}

void AudioArmDeviceBehavior::ApplyFormatToModuleInfo(AudioModuleInfo &moduleInfo,
    const SelectedDeviceFormat &selected, uint32_t buffCalcMs,
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    moduleInfo.rate = std::to_string(static_cast<uint32_t>(selected.rate));
    if (selected.format != AudioSampleFormat::INVALID_WIDTH) {
        moduleInfo.format = formatEnumToStr[selected.format];
    }
    if (selected.channelLayout != CH_LAYOUT_UNKNOWN) {
        moduleInfo.channels = std::to_string(ConvertLayoutToAudioChannel(selected.channelLayout));
        moduleInfo.channelLayout = std::to_string(static_cast<uint64_t>(selected.channelLayout));
    }
    if (!moduleInfo.rate.empty() && !moduleInfo.format.empty() && !moduleInfo.channels.empty()) {
        uint32_t rateValue = 0;
        uint32_t channelValue = 0;
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.rate, rateValue), "invalid rate");
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.channels, channelValue), "invalid channels");
        moduleInfo.bufferSize = std::to_string(rateValue * channelValue *
            AudioPolicyUtils::GetInstance().PcmFormatToBytes(formatStrToEnum[moduleInfo.format]) *
            buffCalcMs / static_cast<uint32_t>(MS_PER_S));
    }
    moduleInfo.macAddress = deviceDesc->GetMacAddress();
    moduleInfo.deviceType = std::to_string(static_cast<int32_t>(deviceDesc->deviceType_));
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

    auto selected = DetermineArmUsbFormat(deviceDesc, deviceStreamInfo);
    ApplyFormatToModuleInfo(moduleInfo, selected, buffCalcMs, deviceDesc);
}

int32_t AudioArmDeviceBehavior::ProcessModuleActivation(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    AudioModuleInfo &moduleInfo, DeviceRole role, bool isEcFeatureEnable)
{
    UpdateArmModuleInfo(deviceDesc, moduleInfo);
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is nullptr");

    if (role == INPUT_DEVICE && isEcFeatureEnable) {
        AUDIO_INFO_LOG("just save arm usb source module info, rate=%{public}s", moduleInfo.rate.c_str());
        coreService->SetUsbSourceModuleInfo(moduleInfo);
        auto pipeManager = AudioPipeManager::GetPipeManager();
        CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");
        pipeManager->UpdateDynamicCapturerConfig(ClassType::TYPE_USB, moduleInfo);
        return SUCCESS;
    }

    if (AudioIOHandleMap::GetInstance().CheckIOHandleExist(moduleInfo.name)) {
        AudioIOHandle existingHandle = HDI_INVALID_ID;
        AudioIOHandleMap::GetInstance().GetModuleIdByKey(moduleInfo.name, existingHandle);
        auto pipeManager = AudioPipeManager::GetPipeManager();
        auto existingPipe = (pipeManager != nullptr) ? pipeManager->GetPipeinfoById(existingHandle) : nullptr;
        if (existingPipe != nullptr &&
            existingPipe->moduleInfo_.rate == moduleInfo.rate &&
            existingPipe->moduleInfo_.format == moduleInfo.format &&
            existingPipe->moduleInfo_.channels == moduleInfo.channels) {
            AUDIO_INFO_LOG("ArmUsb format unchanged, skip reload: rate=%{public}s", moduleInfo.rate.c_str());
        } else {
            int32_t result = ReloadArmAudioPort(moduleInfo);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_INVALID_PARAM,
                "Reload usb %{public}s failed %{public}d", moduleInfo.role.c_str(), result);
        }
    } else {
        int32_t result = AudioIOHandleMap::GetInstance()
            .OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_INVALID_PARAM,
            "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), result);
    }

    if (isEcFeatureEnable) {
        coreService->SetUsbSinkModuleInfo(moduleInfo);
    }
    return SUCCESS;
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
