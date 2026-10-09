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
#include "audio_effect_service.h"
#include "audio_noise_reduction_types.h"
#include "audio_pipe_manager.h"

#include <algorithm>

namespace OHOS {
namespace AudioStandard {

static const std::string VA_DEVICE_INFO_NAME_SPP = "Virtual Audio Spp";
static const std::string VA_DEVICE_INFO_NAME_NEARLINK = "Virtual Audio Nearlink";
static const std::string VA_DEVICE_INFO_NAME_UNIDIRECTIONAL_SPP = "Virtual Audio Unidirectional Spp";

static const std::string VA_INPUT_PIPE_INFO_NAME_SPP = VA_INPUT_SPP;
static const std::string VA_INPUT_PIPE_INFO_NAME_NEARLINK = VA_INPUT_NEARLINK;
static const std::string VA_INPUT_PIPE_INFO_NAME_UNIDIRECTIONAL_SPP = VA_INPUT_UNIDIRECTIONAL_SPP;

namespace {
constexpr uint32_t CYCLES_PER_SECOND = 50;

uint32_t GetSampleBytes(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return 1;
        case SAMPLE_S16LE:
            return 2;
        case SAMPLE_S24LE:
            return 3;
        case SAMPLE_S32LE:
        case SAMPLE_F32LE:
            return 4;
        default:
            return 0;
    }
}
}

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
        case VA_DEVICE_TYPE_BT_SPP:
            desc->deviceType_ = DEVICE_TYPE_BT_SPP;
            break;
        case VA_DEVICE_TYPE_NEARLINK_PORT:
            desc->deviceType_ = DEVICE_TYPE_NEARLINK_PORT;
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
    desc->isUnidirectional_ = config.isUnidirectional_;

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
    CHECK_AND_RETURN_LOG(vaDevice != nullptr && controller != nullptr, "invalid parameter: null pointer detected");
    AUDIO_INFO_LOG("va device manager connecting to device: {\"name\":\"%{public}s\", \"type\":\"%{public}d\"}",
        vaDevice->configuration_.name_.c_str(), vaDevice->configuration_.type_);
    std::shared_ptr<AudioDeviceDescriptor> descriptor = ConvertVADeviceToDescriptor(vaDevice);
    CHECK_AND_RETURN_LOG(descriptor != nullptr, "descriptor is nullptr");
    {
        std::lock_guard<std::mutex> lock(statusMutex_);
        connectedVADeviceMap_[vaDevice->configuration_.address_] = controller;

        std::string pipeName;
        std::string deviceName;
        DeviceType deviceType;
        GetVAPipeAndDeviceName(vaDevice->configuration_, pipeName, deviceName, deviceType);
        AddVADeviceToAdapter(vaDevice);
        AddVAStreamPropToMap(vaDevice->configuration_.properties_, pipeName, deviceName);
    }
    sptr<IRemoteObject> controllerObj = controller->AsObject();
    if (controllerObj != nullptr) {
        std::shared_ptr<VADevice> vaDeviceForCb = vaDevice;
        sptr<VADeviceDeathRecipient> recipient = new (std::nothrow) VADeviceDeathRecipient(
            [vaDeviceForCb]() { VADeviceManager::GetInstance().OnDevicesDisconnected(vaDeviceForCb); });
        if (recipient != nullptr && !controllerObj->AddDeathRecipient(recipient)) {
            AUDIO_ERR_LOG("AddDeathRecipient failed for va device: %{public}s",
                vaDevice->configuration_.name_.c_str());
        }
    }
    AudioCoreService::GetCoreService()->GetEventEntry()->OnDeviceStatusUpdated(*descriptor, true);
    CHECK_AND_RETURN(descriptor->isUnidirectional_);
    SetAssociatedMacAddresses(descriptor->macAddress_, false);
    UpdateNoiseReductionCapability(vaDevice, descriptor);
}

void VADeviceManager::OnDevicesDisconnected(const std::shared_ptr<VADevice> &vaDevice)
{
    CHECK_AND_RETURN_LOG(vaDevice != nullptr, "invalid parameter: null pointer detected");
    AUDIO_INFO_LOG("disconnecting va device: {\"name\":\"%{public}s\", \"type\":\"%{public}d\"}",
        vaDevice->configuration_.name_.c_str(),
        vaDevice->configuration_.type_);
    std::shared_ptr<AudioDeviceDescriptor> descriptor = ConvertVADeviceToDescriptor(vaDevice);
    bool needUnregisterAdapter = false;
    {
        std::lock_guard<std::mutex> lock(statusMutex_);
        auto it = connectedVADeviceMap_.find(vaDevice->configuration_.address_);
        if (it == connectedVADeviceMap_.end()) {
            AUDIO_INFO_LOG("va device already disconnected, addressLen=%{public}zu",
                vaDevice->configuration_.address_.size());
            return;
        }
        connectedVADeviceMap_.erase(it);
        needUnregisterAdapter = connectedVADeviceMap_.empty();
    }
    AudioCoreService::GetCoreService()->GetEventEntry()->OnDeviceStatusUpdated(*descriptor, false);
    if (needUnregisterAdapter) {
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

void VADeviceManager::GetVAPipeAndDeviceName(const VADeviceConfiguration &config,
    std::string &pipeName, std::string &deviceName, DeviceType &deviceType)
{
    const bool isNearlink = !config.isUnidirectional_ &&
        config.type_ == VA_DEVICE_TYPE_NEARLINK_PORT;
    pipeName = config.isUnidirectional_ ? VA_INPUT_PIPE_INFO_NAME_UNIDIRECTIONAL_SPP
        : (isNearlink ? VA_INPUT_PIPE_INFO_NAME_NEARLINK : VA_INPUT_PIPE_INFO_NAME_SPP);
    deviceName = config.isUnidirectional_ ? VA_DEVICE_INFO_NAME_UNIDIRECTIONAL_SPP
        : (isNearlink ? VA_DEVICE_INFO_NAME_NEARLINK : VA_DEVICE_INFO_NAME_SPP);
    deviceType = isNearlink ? DEVICE_TYPE_NEARLINK_PORT : DEVICE_TYPE_BT_SPP;
}

void VADeviceManager::CreateVAPipeAndDevice(const std::shared_ptr<PolicyAdapterInfo> &adapterInfoPtr,
    const std::string &pipeName, const std::string &deviceName, DeviceType deviceType)
{
    CHECK_AND_RETURN_LOG(adapterInfoPtr != nullptr, "adapterInfoPtr is nullptr");

    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeInfo->adapterInfo_ = adapterInfoPtr;
    pipeInfo->name_ = pipeName;
    pipeInfo->role_ = PIPE_ROLE_INPUT;
    pipeInfo->supportDevices_.push_back(deviceName);

    AdapterDeviceInfo deviceInfo{};
    deviceInfo.adapterInfo_ = adapterInfoPtr;
    deviceInfo.name_ = deviceName;
    deviceInfo.type_ = deviceType;
    deviceInfo.role_ = INPUT_DEVICE;
    deviceInfo.supportPipes_.push_back(pipeName);
    deviceInfo.supportPipeMap_[AudioFlag::AUDIO_INPUT_FLAG_NORMAL] = pipeInfo;

    PaPropInfo paProp = {};
    paProp.lib_ = "libmodule-hdi-source.z.so";
    paProp.role_ = "source";
    paProp.moduleName_ = pipeName;
    pipeInfo->paProp_ = std::move(paProp);

    adapterInfoPtr->pipeInfos.push_back(pipeInfo);
    adapterInfoPtr->deviceInfos.push_back(make_shared<AdapterDeviceInfo>(deviceInfo));
}

void VADeviceManager::AddVADeviceToAdapter(const std::shared_ptr<VADevice> &vaDevice)
{
    CHECK_AND_RETURN_LOG(vaDevice != nullptr, "vaDevice is nullptr");
    std::string pipeName;
    std::string deviceName;
    DeviceType deviceType;
    GetVAPipeAndDeviceName(vaDevice->configuration_, pipeName, deviceName, deviceType);

    std::shared_ptr<PolicyAdapterInfo> existingAdapter = config_.GetAdapterInfo(AudioAdapterType::TYPE_VA);
    if (existingAdapter == nullptr) {
        auto adapterInfoPtr = std::make_shared<PolicyAdapterInfo>();
        adapterInfoPtr->adapterName = ADAPTER_TYPE_VA;
        CreateVAPipeAndDevice(adapterInfoPtr, pipeName, deviceName, deviceType);
        config_.RegisterAdapter(adapterInfoPtr);
        return;
    }

    const bool pipeExists = std::any_of(existingAdapter->pipeInfos.begin(),
        existingAdapter->pipeInfos.end(), [&pipeName](const auto &pipe) {
            return pipe != nullptr && pipe->name_ == pipeName;
        });
    if (!pipeExists) {
        CreateVAPipeAndDevice(existingAdapter, pipeName, deviceName, deviceType);
    }
    config_.Reorganize();
}

void VADeviceManager::AddVAStreamPropToMap(std::list<VAAudioStreamProperty> properties,
    const std::string &pipeName, const std::string &deviceName)
{
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps;
    for (auto vaStreamProp : properties) {
        std::shared_ptr<PipeStreamPropInfo> pipeStreamPropInfo =
            ConvertVADeviceStreamPropertyToPipeStreamPropInfo(vaStreamProp);
        CHECK_AND_RETURN_LOG(pipeStreamPropInfo != nullptr, "pipeStreamPropInfo is null");
        pipeStreamPropInfo->supportDevices_.push_back(deviceName);
        streamProps.push_back(pipeStreamPropInfo);
    }
    CHECK_AND_RETURN_LOG(streamProps.size() != 0, "streamProps is empty");
    config_.RegisterStreamProperty(AudioAdapterType::TYPE_VA, pipeName, streamProps);
}

std::shared_ptr<PipeStreamPropInfo> VADeviceManager::ConvertVADeviceStreamPropertyToPipeStreamPropInfo(
    const VAAudioStreamProperty &vaStreamProperty)
{
    std::shared_ptr<PipeStreamPropInfo> pipeStreamInfo = std::make_shared<PipeStreamPropInfo>();
    pipeStreamInfo->format_ = vaStreamProperty.sampleFormat_;
    pipeStreamInfo->sampleRate_ = vaStreamProperty.sampleRate_;
    pipeStreamInfo->channelLayout_ = vaStreamProperty.channelLayout_;
    pipeStreamInfo->channels_ = ConvertLayoutToAudioChannel(pipeStreamInfo->channelLayout_);
    pipeStreamInfo->bufferSize_ = CalculateBufferSize(vaStreamProperty);
    return pipeStreamInfo;
}

uint32_t VADeviceManager::CalculateBufferSize(const VAAudioStreamProperty &vaStreamProperty)
{
    uint32_t samplesPerCycle = vaStreamProperty.samplesPerCycle_ != 0 ? vaStreamProperty.samplesPerCycle_ :
        vaStreamProperty.sampleRate_ / CYCLES_PER_SECOND;
    if (samplesPerCycle > static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_192000) / CYCLES_PER_SECOND) {
        AUDIO_ERR_LOG("invalid samplesPerCycle");
        return 0;
    }
    uint8_t channels = ConvertLayoutToAudioChannel(vaStreamProperty.channelLayout_);
    uint32_t sampleBytes = GetSampleBytes(vaStreamProperty.sampleFormat_);
    if (channels == CHANNEL_UNKNOW || sampleBytes == 0 || samplesPerCycle == 0) {
        AUDIO_ERR_LOG("invalid stream property");
        return 0;
    }
    uint32_t bufferSize = channels * samplesPerCycle * sampleBytes;
    AUDIO_INFO_LOG("calculate buffer size: %{public}d", bufferSize);
    return bufferSize;
}

void VADeviceManager::SetAssociatedMacAddresses(const std::string &macAddress, bool isEnable)
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE);
    deviceDesc->macAddress_ = macAddress;
    deviceDesc->isEnable_ = isEnable;
    deviceDesc->isExclusiveMicScoUnavailable_ = !isEnable;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
 
    AudioCoreService::GetCoreService()->GetEventEntry()->OnDeviceInfoUpdated(
        *deviceDesc, DeviceInfoUpdateCommand::ENABLE_UPDATE);
}

void VADeviceManager::SetNoiseReductionMode(
    const std::shared_ptr<VADevice> &vaDevice, int32_t noiseReductionMode)
{
    CHECK_AND_RETURN_LOG(vaDevice != nullptr, "vaDevice is nullptr");
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_LOG(pipeManager != nullptr, "pipeManager is nullptr");
    std::vector<int32_t> clientUids;
    std::string targetMacAddress = vaDevice->configuration_.address_;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> inputStreamDescs =
        pipeManager->GetAllInputStreamDescs();
    for (const auto &streamDesc : inputStreamDescs) {
        CHECK_AND_CONTINUE(streamDesc != nullptr && streamDesc->streamStatus_ == STREAM_STATUS_STARTED);
        SourceType sourceType = streamDesc->capturerInfo_.sourceType;
        CHECK_AND_CONTINUE(sourceType == SOURCE_TYPE_MIC || sourceType == SOURCE_TYPE_CAMCORDER ||
            sourceType == SOURCE_TYPE_LIVE);
        bool hasSppDevice = false;
        for (const auto &deviceDesc : streamDesc->newDeviceDescs_) {
            if (deviceDesc != nullptr && deviceDesc->deviceType_ == DEVICE_TYPE_BT_SPP &&
                deviceDesc->macAddress_ == targetMacAddress) {
                hasSppDevice = true;
                break;
            }
        }
        if (hasSppDevice) {
            clientUids.push_back(streamDesc->callerUid_);
        }
    }
    CHECK_AND_RETURN_LOG(!clientUids.empty(),
        "No running recording stream with SPP device found for noise reduction mode setting");
    auto deviceDesc = ConvertVADeviceToDescriptor(vaDevice);
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
    NoiseReductionMode mode = static_cast<NoiseReductionMode>(noiseReductionMode);
    for (int32_t clientUid : clientUids) {
    }
}
 
void VADeviceManager::UpdateNoiseReductionCapability(const std::shared_ptr<VADevice> &vaDevice,
    const std::shared_ptr<AudioDeviceDescriptor> &descriptor)
{
    CHECK_AND_RETURN_LOG(vaDevice != nullptr, "vaDevice is nullptr");
    CHECK_AND_RETURN_LOG(descriptor != nullptr, "descriptor is nullptr");
    CHECK_AND_RETURN_LOG(!vaDevice->configuration_.noiseReductionCapability_.supportedModes_.empty(),
        "noiseReductionCapability supportedModes is empty");
}
}  // namespace VirtualAudioDevice
}  // namespace OHOS