/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "VADeviceManager"
#endif

#include "va_device_manager.h"
#include "audio_policy_log.h"
#include "audio_module_info.h"

namespace OHOS {
namespace AudioStandard {

static const std::string VA_DEVICE_INFO_NAME = "Virtual Audio";
static const std::string VA_INPUT_PIPE_INFO_NAME = "va_input";

VADeviceManager &VADeviceManager::GetInstance()
{
    static VADeviceManager instance;
    return instance;
}

std::shared_ptr<AudioDeviceDescriptor> VADeviceManager::ConvertVADeviceToDescriptor(
    const std::shared_ptr<VADevice> &vaDevice)
{
    if (vaDevice == nullptr) {
        return nullptr;
    }
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    auto config = vaDevice->configuration_;
    desc->deviceName_ = config.name_;
    desc->displayName_ = config.name_;
    switch (config.type_) {
        case VA_DEVICE_TYPE_NONE:
            desc->deviceType_ = DEVICE_TYPE_NONE;
            break;
        case VA_DEVICE_TYPE_BT_SPP:
            desc->deviceType_ = DEVICE_TYPE_BT_SPP;
            break;
        default:
            desc->deviceType_ = DEVICE_TYPE_NONE;
            break;
    }
    switch (config.role_) {
        case VA_DEVICE_ROLE_IN:
            desc->deviceRole_ = INPUT_DEVICE;
            break;
        case VA_DEVICE_ROLE_OUT:
            desc->deviceRole_ = OUTPUT_DEVICE;
            break;
        default:
            desc->deviceRole_ = DEVICE_ROLE_NONE;
            break;
    }
    desc->macAddress_ = config.address_;

    for (const auto &streamProp : config.properties_) {
        std::shared_ptr<DeviceStreamInfo> streamInfo = ConvertVAStreamPropertyToInfo(streamProp);
        desc->audioStreamInfo_.push_back(*streamInfo);
    }
    return desc;
}

std::shared_ptr<DeviceStreamInfo> VADeviceManager::ConvertVAStreamPropertyToInfo(
    const VAAudioStreamProperty &vaStreamProperty)
{
    std::shared_ptr<DeviceStreamInfo> streamInfo = std::make_shared<DeviceStreamInfo>();
    streamInfo->encoding = vaStreamProperty.encoding_;
    streamInfo->format = vaStreamProperty.sampleFormat_;
    streamInfo->channelLayout.insert(vaStreamProperty.channelLayout_);
    streamInfo->samplingRate.insert(static_cast<AudioSamplingRate>(vaStreamProperty.sampleRate_));
    return streamInfo;
}

void VADeviceManager::OnDevicesConnected(
    const std::shared_ptr<VADevice> &vaDevice,
    const sptr<IVADeviceController> &controller)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    CHECK_AND_RETURN_LOG(vaDevice != nullptr && controller != nullptr, "invalid parameter: null pointer detected");
    AUDIO_INFO_LOG("va device manager connecting to device: {\"name\":\"%{public}s\", \"type\":\"%{public}d\"}",
                   vaDevice->configuration_.name_.c_str(), vaDevice->configuration_.type_);
    std::shared_ptr<AudioDeviceDescriptor> descriptor = ConvertVADeviceToDescriptor(vaDevice);
    connectedVADeviceMap_[vaDevice->configuration_.address_] = controller;

    if (!config_.GetAdapterInfo(AudioAdapterType::TYPE_VA)) {
        RegisterVAAdapterToMap();
    }
    AddVAStreamPropToMap(vaDevice->configuration_.properties_);
    AudioCoreService::GetCoreService()->GetEventEntry()->OnDeviceStatusUpdated(*descriptor, true);
}

void VADeviceManager::OnDevicesDisconnected(const std::shared_ptr<VADevice> &vaDevice)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    CHECK_AND_RETURN_LOG(vaDevice != nullptr, "invalid parameter: null pointer detected");
    std::shared_ptr<AudioDeviceDescriptor> descriptor = ConvertVADeviceToDescriptor(vaDevice);
    AudioCoreService::GetCoreService()->GetEventEntry()->OnDeviceStatusUpdated(*descriptor, false);
    connectedVADeviceMap_.erase(vaDevice->configuration_.address_);
    if (connectedVADeviceMap_.size() <= 0) {
        config_.UnregisterAdapter(AudioAdapterType::TYPE_VA);
    }
}

void VADeviceManager::GetDeviceController(const std::string macAddr, sptr<IRemoteObject> &controller)
{
    CHECK_AND_RETURN_LOG(
        PermissionUtil::VerifySystemPermission(), "get device controller denied: no system permission");
    std::lock_guard<std::mutex> lock(statusMutex_);
    auto it = connectedVADeviceMap_.find(macAddr);
    if (it == connectedVADeviceMap_.end()) {
        controller = nullptr;
    } else {
        sptr<IVADeviceController> vaController = it->second;
        if (vaController == nullptr) {
            controller = nullptr;
        } else {
            controller = vaController->AsObject();
        }
    }
}

void VADeviceManager::RegisterVAAdapterToMap()
{
    PolicyAdapterInfo adapterInfo{};
    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = std::make_shared<PolicyAdapterInfo>(adapterInfo);
    adapterInfoPtr->adapterName = ADAPTER_TYPE_VA;

    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeInfo->adapterInfo_ = adapterInfoPtr;
    pipeInfo->name_ = VA_INPUT_PIPE_INFO_NAME;
    pipeInfo->role_ = PIPE_ROLE_INPUT;
    pipeInfo->supportDevices_.push_back(VA_DEVICE_INFO_NAME);

    AdapterDeviceInfo deviceInfo{};
    deviceInfo.adapterInfo_ = adapterInfoPtr;
    deviceInfo.name_ = VA_DEVICE_INFO_NAME;
    deviceInfo.type_ = DEVICE_TYPE_BT_SPP;
    deviceInfo.role_ = INPUT_DEVICE;
    deviceInfo.supportPipes_.push_back(VA_INPUT_PIPE_INFO_NAME);
    deviceInfo.supportPipeMap_[AudioFlag::AUDIO_INPUT_FLAG_NORMAL] = pipeInfo;

    adapterInfoPtr->deviceInfos.push_back(make_shared<AdapterDeviceInfo>(deviceInfo));

    PaPropInfo paProp = {};
    paProp.lib_ = "libmodule-hdi-source.z.so";
    paProp.role_ = "source";
    paProp.moduleName_ = VIRTUAL_AUDIO;

    pipeInfo->paProp_ = std::move(paProp);

    adapterInfoPtr->pipeInfos.push_back(pipeInfo);

    config_.RegisterAdapter(adapterInfoPtr);
}

void VADeviceManager::AddVAStreamPropToMap(std::list<VAAudioStreamProperty> properties)
{
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps;
    for (auto vaStreamProp : properties) {
        std::shared_ptr<PipeStreamPropInfo> pipeStreamPropInfo =
            ConvertVADeviceStreamPropertyToPipeStreamPropInfo(vaStreamProp);
        CHECK_AND_RETURN_LOG(pipeStreamPropInfo != nullptr, "pipeStreamPropInfo is null");
        pipeStreamPropInfo->supportDevices_.push_back(VA_DEVICE_INFO_NAME);
        streamProps.push_back(pipeStreamPropInfo);
    }
    CHECK_AND_RETURN_LOG(streamProps.size() != 0, "streamProps is empty");
    config_.RegisterStreamProperty(AudioAdapterType::TYPE_VA, VA_INPUT_PIPE_INFO_NAME, streamProps);
}

std::shared_ptr<PipeStreamPropInfo> VADeviceManager::ConvertVADeviceStreamPropertyToPipeStreamPropInfo(
    const VAAudioStreamProperty &vaStreamProperty)
{
    std::shared_ptr<PipeStreamPropInfo> pipeStreamInfo = std::make_shared<PipeStreamPropInfo>();
    pipeStreamInfo->format_ = vaStreamProperty.sampleFormat_;
    pipeStreamInfo->sampleRate_ = vaStreamProperty.sampleRate_;
    pipeStreamInfo->channelLayout_ = vaStreamProperty.channelLayout_;
    pipeStreamInfo->channels_ = AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(pipeStreamInfo->channelLayout_);
    pipeStreamInfo->bufferSize_ = CalculateBufferSize(vaStreamProperty);
    return pipeStreamInfo;
}

uint32_t VADeviceManager::CalculateBufferSize(const VAAudioStreamProperty &vaStreamProperty)
{
    const uint32_t cyclesPerSecond = 50;
    if (vaStreamProperty.samplesPerCycle_ >
            static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_192000) / cyclesPerSecond) {
        AUDIO_ERR_LOG("invalid samplesPerCycle");
        return 0;
    }
    uint8_t channels = AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(vaStreamProperty.channelLayout_);
    uint32_t samplesPerCycle_ = vaStreamProperty.samplesPerCycle_;
    uint32_t bitWidth = 2;
    uint32_t bufferSize = channels * samplesPerCycle_ * bitWidth;
    AUDIO_INFO_LOG("calculate buffer size: %{public}d", bufferSize);
    return bufferSize;
}
}  // namespace VirtualAudioDevice
}  // namespace OHOS