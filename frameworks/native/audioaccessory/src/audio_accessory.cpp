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

#include "audio_accessory.h"
#include "audio_accessory_input_stream.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_log_utils.h"
#include "va_device_broker_wrapper_impl.h"
#include "virtual_audio_interface.h"
#include "audio_noise_reduction_types.h"

namespace OHOS {
namespace AudioStandard {
AudioAccessory::AudioAccessory(
    const OH_AudioAccessoryInfo& info,
    OH_AudioAccessory_SetNoiseReductionCallback onNoiseReduction,
    OH_AudioAccessory_OpenInputStreamCallback onOpenInputStream)
    : accessoryName_(info.accessoryName ? info.accessoryName : ""),
      manufacturer_(info.manufacturer ? info.manufacturer : ""),
      modelNumber_(info.modelNumber ? info.modelNumber : ""),
      macAddress_(info.macAddress ? info.macAddress : ""),
      type_(info.type),
      isUnidirectional_(info.isUnidirectional),
      onNoiseReduction_(onNoiseReduction),
      onOpenInputStream_(onOpenInputStream),
      currentMode_(AUDIO_NOISE_REDUCTION_MODE_FIDELITY)
{
    configuration_.name_ = accessoryName_;
    configuration_.type_ = type_ == AUDIO_ACCESSORY_TYPE_BT_SPP ? VA_DEVICE_TYPE_BT_SPP : VA_DEVICE_TYPE_NONE;
    configuration_.role_ = VA_DEVICE_ROLE_IN;
    configuration_.address_ = macAddress_;
    configuration_.manufacturer_ = manufacturer_;
    configuration_.modelNumber_ = modelNumber_;
    configuration_.isUnidirectional_ = isUnidirectional_;
}

std::shared_ptr<VAAudioStreamProperty> AudioAccessory::GetStreamProperty() const
{
    CHECK_AND_RETURN_RET(!streamProperties_.empty(), nullptr);
    auto prop = std::make_shared<VAAudioStreamProperty>();
    const auto& info = streamProperties_[0];
    prop->sampleRate_ = static_cast<uint32_t>(info.samplingRate);
    prop->sampleFormat_ = static_cast<AudioSampleFormat>(info.sampleFormat);
    prop->channelLayout_ = static_cast<AudioChannelLayout>(info.channelLayout);
    return prop;
}

std::shared_ptr<VAInputStreamAttribute> AudioAccessory::GetInputStreamAttribute() const
{
    auto attr = std::make_shared<VAInputStreamAttribute>();
    attr->type = static_cast<SourceType>(type_);
    return attr;
}

std::shared_ptr<VAInputStreamCallback> AudioAccessory::GetInputStreamCallback()
{
    return nullptr;
}

int32_t AudioAccessory::OpenInputStreamInternal(std::shared_ptr<VAInputStreamCallback>& inputStream,
    OH_AudioAccessory* accessoryHandle, OH_AudioAccessoryInputStream* streamHandle)
{
    CHECK_AND_RETURN_RET(isConnected_, ERR_ILLEGAL_STATE);

    auto prop = GetStreamProperty();
    CHECK_AND_RETURN_RET_LOG(prop != nullptr, ERR_ILLEGAL_STATE, "Stream property not set");

    auto selfShared = std::shared_ptr<AudioAccessory>(this, [](AudioAccessory*) {});
    auto streamImpl = std::make_shared<AudioAccessoryInputStream>(*selfShared, *prop);
    CHECK_AND_RETURN_RET_LOG(streamImpl != nullptr, ERR_OPERATION_FAILED, "Create input stream failed");

    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = static_cast<int32_t>(prop->sampleRate_);
    streamInfo.channelLayout = static_cast<OH_AudioChannelLayout>(prop->channelLayout_);
    streamInfo.encodingType = OH_AudioStream_EncodingType::AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = static_cast<OH_AudioStream_SampleFormat>(prop->sampleFormat_);

    auto actualStreamHandle = reinterpret_cast<OH_AudioAccessoryInputStream *>(streamImpl.get());
    if (onOpenInputStream_) {
        streamImpl->SetRegistering(true);
        bool accepted = onOpenInputStream_(accessoryHandle, actualStreamHandle, &streamInfo);
        streamImpl->SetRegistering(false);
        if (!accepted) {
            AUDIO_ERR_LOG("onOpenInputStream rejected by plugin");
            streamImpl->ForceRelease();
            return ERR_ILLEGAL_STATE;
        }
    }

    if (!streamImpl->AreAllCallbacksRegistered()) {
        std::string missing = streamImpl->GetMissingCallbacksInfo();
        AUDIO_ERR_LOG("Stream creation failed: Missing mandatory callbacks [%{public}s]. "
                      "Plugin must register onStart, onStop, and onRelease.", missing.c_str());
        streamImpl->ForceRelease();
        return ERR_ILLEGAL_STATE;
    }
    inputStream = streamImpl;
    return SUCCESS;
}

int32_t AudioAccessory::CloseInputStream(std::shared_ptr<VAInputStreamCallback> inputStream)
{
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, ERR_INVALID_PARAM, "inputStream is nullptr");
    return inputStream->Close();
}

void AudioAccessory::SetCapabilities(const OH_AudioAccessoryCapabilities& caps)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (caps.streamProperties != nullptr && caps.streamPropertyCount > 0) {
        streamProperties_.assign(caps.streamProperties, caps.streamProperties + caps.streamPropertyCount);
    }
    for (const auto& info : streamProperties_) {
        VAAudioStreamProperty prop;
        prop.sampleRate_ = static_cast<uint32_t>(info.samplingRate);
        prop.sampleFormat_ = static_cast<AudioSampleFormat>(info.sampleFormat);
        prop.channelLayout_ = static_cast<AudioChannelLayout>(info.channelLayout);
        configuration_.properties_.push_back(prop);
    }
}

void AudioAccessory::SetNoiseReductionCapability(
    const std::vector<OH_AudioNoiseReductionMode>& modes,
    OH_AudioNoiseReductionMode currentMode,
    OH_AudioAccessory_SetNoiseReductionCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    supportedModes_ = modes;
    currentMode_.store(currentMode);
    CHECK_AND_RETURN_LOG(callback != nullptr, "noiseReductionCallback is nullptr");
    onNoiseReduction_ = callback;
}

int32_t AudioAccessory::SetNoiseReductionMode(OH_AudioNoiseReductionMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET(isConnected_, ERR_ILLEGAL_STATE);

    for (auto m : supportedModes_) {
        if (m == mode) {
            currentMode_.store(mode);
            return SUCCESS;
        }
    }
    return ERR_NOT_SUPPORTED;
}

OH_AudioNoiseReductionMode AudioAccessory::GetNoiseReductionMode() const
{
    return currentMode_.load();
}

int32_t AudioAccessory::OpenInputStream(const VAAudioStreamProperty &prop, const VAInputStreamAttribute &attr,
                                        std::shared_ptr<VAInputStreamCallback> &inputStream)
{
    return OpenInputStreamInternal(inputStream, reinterpret_cast<OH_AudioAccessory*>(this), nullptr);
}

int32_t AudioAccessory::GetParameters(const std::string& key, std::string& value)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioAccessory::SetParameters(const std::string& key, const std::string& value)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioAccessory::NotifyConnected()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isConnected_) {
        return ERR_ILLEGAL_STATE;
    }

    isConnected_ = true;
    std::shared_ptr<VADeviceBrokerWrapper> vaBroker;
    int32_t result = AudioPolicyManager::GetInstance().GetVADeviceBroker(vaBroker);
    if (result != SUCCESS || vaBroker == nullptr) {
        isConnected_ = false;
        AUDIO_ERR_LOG("AudioAccessory::NotifyConnected failed to get VA broker");
        return result;
    }

    auto selfShared = std::shared_ptr<AudioAccessory>(this, [](AudioAccessory*) {});
    result = vaBroker->OnDevicesConnected(*this, selfShared);
    if (result != SUCCESS) {
        isConnected_ = false;
        AUDIO_ERR_LOG("AAudioAccessory::NotifyConnected failed to connect device");
        return result;
    }
    CHECK_AND_RETURN_RET(onNoiseReduction_ != nullptr, SUCCESS);
    return SUCCESS;
}

int32_t AudioAccessory::NotifyDisconnected()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET(isConnected_, ERR_ILLEGAL_STATE);

    std::shared_ptr<VADeviceBrokerWrapper> vaBroker;
    int32_t result = AudioPolicyManager::GetInstance().GetVADeviceBroker(vaBroker);
    if (result != SUCCESS || vaBroker == nullptr) {
        AUDIO_ERR_LOG("AudioAccessory::NotifyDisconnected failed to get VA broker");
        return result;
    }

    result = vaBroker->OnDevicesDisconnected(*this);
    if (result == SUCCESS) {
        isConnected_ = false;
    }
    return result;
}

void AudioAccessory::NotifyOpenInputStream(std::shared_ptr<VAInputStreamCallback> callback)
{
    // Handled by stream implementation
}

void AudioAccessory::NotifyStartStreamFailed()
{
    AudioPolicyManager::GetInstance().SetVADeviceExceptionFlag(macAddress_, true);
}

VADevice AudioAccessory::MakeVADevice() const
{
    VADevice device;
    device.implementor_ = manufacturer_;
    device.modelNumber_ = modelNumber_;
    device.configuration_.name_ = accessoryName_;
    device.configuration_.address_ = macAddress_;
    device.configuration_.manufacturer_ = manufacturer_;
    device.configuration_.modelNumber_ = modelNumber_;
    device.configuration_.isUnidirectional_ = isUnidirectional_;
    device.configuration_.role_ = VA_DEVICE_ROLE_IN;
    device.configuration_.type_ = type_ == AUDIO_ACCESSORY_TYPE_BT_SPP ? VA_DEVICE_TYPE_BT_SPP :
        VA_DEVICE_TYPE_NONE;
    for (const auto& prop : streamProperties_) {
        VAAudioStreamProperty vaProp;
        vaProp.sampleRate_ = static_cast<uint32_t>(prop.samplingRate);
        vaProp.sampleFormat_ = static_cast<AudioSampleFormat>(prop.sampleFormat);
        vaProp.channelLayout_ = static_cast<AudioChannelLayout>(prop.channelLayout);
        vaProp.encoding_ = AudioEncodingType::ENCODING_PCM;
        device.configuration_.properties_.push_back(vaProp);
    }
    device.configuration_.noiseReductionCapability_.supportedModes_.clear();
    for (const auto& mode : supportedModes_) {
        device.configuration_.noiseReductionCapability_.supportedModes_.push_back(
            static_cast<VANoiseReductionMode>(mode));
    }
    device.configuration_.noiseReductionCapability_.defaultMode_ =
        static_cast<VANoiseReductionMode>(currentMode_.load());
    return device;
}

} // namespace AudioStandard
} // namespace OHOS
