/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDeviceManager"
#endif

#include "audio_device_manager.h"

#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_device_parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
constexpr int32_t MS_PER_S = 1000;
constexpr int32_t NS_PER_MS = 1000000;
const int32_t ADDRESS_STR_LEN = 17;
const int32_t START_POS = 6;
const int32_t END_POS = 13;

// LCOV_EXCL_START
std::string GetEncryptAddr(const std::string &addr)
{
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

AudioDeviceManager::AudioDeviceManager()
{
}

static int64_t GetCurrentTimeMS()
{
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

void AudioDeviceManager::ParseDeviceXml()
{
    unique_ptr<AudioDeviceParser> audioDeviceParser = make_unique<AudioDeviceParser>(this);
    if (audioDeviceParser->LoadConfiguration()) {
        AUDIO_INFO_LOG("Audio device manager load configuration successfully.");
        audioDeviceParser->Parse();
    }
}

void AudioDeviceManager::OnXmlParsingCompleted(
    const unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> &xmlData)
{
    CHECK_AND_RETURN_LOG(!xmlData.empty(), "Failed to parse xml file.");

    devicePrivacyMaps_ = xmlData;

    auto privacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (privacyDevices != devicePrivacyMaps_.end()) {
        privacyDeviceList_ = privacyDevices->second;
    }

    auto publicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (publicDevices != devicePrivacyMaps_.end()) {
        publicDeviceList_ = publicDevices->second;
    }
}

bool AudioDeviceManager::DeviceAttrMatch(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    AudioDevicePrivacyType &privacyType, DeviceRole &devRole, DeviceUsage &devUsage)
{
    list<DevicePrivacyInfo> deviceList;
    if (privacyType == TYPE_PRIVACY) {
        deviceList = privacyDeviceList_;
    } else if (privacyType == TYPE_PUBLIC) {
        deviceList = publicDeviceList_;
    } else {
        return false;
    }

    if (devDesc->connectState_ == VIRTUAL_CONNECTED) {
        return false;
    }

    for (auto &devInfo : deviceList) {
        if ((devInfo.deviceType == devDesc->deviceType_) &&
            (devRole == devDesc->deviceRole_) &&
            ((devInfo.deviceUsage & devUsage) != 0) &&
            ((devInfo.deviceCategory == devDesc->deviceCategory_) ||
            ((devInfo.deviceCategory & devDesc->deviceCategory_) != 0))) {
            return true;
        }
    }

    return false;
}

void AudioDeviceManager::FillArrayWhenDeviceAttrMatch(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    AudioDevicePrivacyType privacyType, DeviceRole devRole, DeviceUsage devUsage, string logName,
    vector<shared_ptr<AudioDeviceDescriptor>> &descArray)
{
    bool result = DeviceAttrMatch(devDesc, privacyType, devRole, devUsage);
    if (result) {
        descArray.push_back(devDesc);
        AUDIO_INFO_LOG("Add to %{public}s list, and then %{public}s",
            logName.c_str(), GetConnDevicesStr(descArray).c_str());
    }
}

void AudioDeviceManager::AddRemoteRenderDev(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    if ((devDesc->networkId_ != LOCAL_NETWORK_ID || devDesc->deviceType_ == DEVICE_TYPE_REMOTE_CAST) &&
        devDesc->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        remoteRenderDevices_.push_back(devDesc);
    }
}

void AudioDeviceManager::AddRemoteCaptureDev(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    if (devDesc->networkId_ != LOCAL_NETWORK_ID && devDesc->deviceRole_ == DeviceRole::INPUT_DEVICE) {
        remoteCaptureDevices_.push_back(devDesc);
    }
}

void AudioDeviceManager::MakePairedDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    DeviceRole devRole)
{
    auto isPresent = [&devDesc, &devRole] (const shared_ptr<AudioDeviceDescriptor> &desc) {
        if (desc->networkId_ != devDesc->networkId_ || desc->deviceRole_ != devRole) {
            return false;
        }
        if (devDesc->macAddress_ != "" && devDesc->macAddress_ == desc->macAddress_) {
            return true;
        } else {
            return (desc->deviceType_ == devDesc->deviceType_);
        }
    };

    auto it = find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (it != connectedDevices_.end()) {
        devDesc->pairDeviceDescriptor_ = *it;
        (*it)->pairDeviceDescriptor_ = devDesc;
    }

    MakePairedDefaultDeviceDescriptor(devDesc, devRole);
}

void AudioDeviceManager::MakePairedDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    if (devDesc->deviceRole_ == INPUT_DEVICE) {
        MakePairedDeviceDescriptor(devDesc, OUTPUT_DEVICE);
    } else if (devDesc->deviceRole_ == OUTPUT_DEVICE) {
        MakePairedDeviceDescriptor(devDesc, INPUT_DEVICE);
    }
}

void AudioDeviceManager::MakePairedDefaultDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    DeviceRole devRole)
{
    // EARPIECE -> MIC ; SPEAKER -> MIC ; MIC -> SPEAKER
    auto isPresent = [&devDesc, &devRole] (const shared_ptr<AudioDeviceDescriptor> &desc) {
        if ((devDesc->deviceType_ == DEVICE_TYPE_EARPIECE || devDesc->deviceType_ == DEVICE_TYPE_SPEAKER) &&
            devRole == INPUT_DEVICE && desc->deviceType_ == DEVICE_TYPE_MIC &&
            desc->networkId_ == devDesc->networkId_) {
            return true;
        } else if (devDesc->deviceType_ == DEVICE_TYPE_MIC && devRole == OUTPUT_DEVICE &&
            desc->deviceType_ == DEVICE_TYPE_SPEAKER && desc->networkId_ == devDesc->networkId_) {
            return true;
        }
        return false;
    };

    auto it = find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (it != connectedDevices_.end()) {
        MakePairedDefaultDeviceImpl(devDesc, *it);
    }
}

void AudioDeviceManager::MakePairedDefaultDeviceImpl(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    const shared_ptr<AudioDeviceDescriptor> &connectedDesc)
{
    devDesc->pairDeviceDescriptor_ = connectedDesc;
    if (devDesc->deviceType_ == DEVICE_TYPE_EARPIECE && earpiece_ != NULL &&
        earpiece_->networkId_ == connectedDesc->networkId_) {
        earpiece_->pairDeviceDescriptor_ = connectedDesc;
    } else if (devDesc->deviceType_ == DEVICE_TYPE_SPEAKER && speaker_ != NULL && defalutMic_ != NULL) {
        if (speaker_->networkId_ == connectedDesc->networkId_) {
            speaker_->pairDeviceDescriptor_ = connectedDesc;
        }
        if (defalutMic_->networkId_ == devDesc->networkId_) {
            defalutMic_->pairDeviceDescriptor_ = devDesc;
        }
        connectedDesc->pairDeviceDescriptor_ = devDesc;
    } else if (devDesc->deviceType_ == DEVICE_TYPE_MIC && defalutMic_ != NULL && speaker_ != NULL) {
        if (defalutMic_->networkId_ == connectedDesc->networkId_) {
            defalutMic_->pairDeviceDescriptor_ = connectedDesc;
        }
        if (speaker_->networkId_ == devDesc->networkId_) {
            speaker_->pairDeviceDescriptor_ = devDesc;
        }
        connectedDesc->pairDeviceDescriptor_ = devDesc;
    }
}

bool AudioDeviceManager::IsArmUsbDevice(const AudioDeviceDescriptor &desc)
{
    auto isPresent = [&desc] (const auto &connDesc) {
        return connDesc->deviceId_ == desc.deviceId_;
    };
    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (itr == connectedDevices_.end()) {
        return false;
    }

    return (*itr)->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET;
}

void AudioDeviceManager::AddConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    connectedDevices_.insert(connectedDevices_.begin(), devDesc);
    AUDIO_INFO_LOG("Connected list %{public}s", GetConnDevicesStr(connectedDevices_).c_str());
}

void AudioDeviceManager::RemoveConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    auto isPresent = [&devDesc](const shared_ptr<AudioDeviceDescriptor> &descriptor) {
        if (descriptor->deviceType_ == devDesc->deviceType_ &&
            descriptor->networkId_ == devDesc->networkId_) {
            if (descriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP &&
                descriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
                return true;
            } else {
                // if the disconnecting device is A2DP, need to compare mac address in addition.
                return descriptor->macAddress_ == devDesc->macAddress_;
            }
        }
        return false;
    };

    for (auto it = connectedDevices_.begin(); it != connectedDevices_.end();) {
        it = find_if(it, connectedDevices_.end(), isPresent);
        if (it != connectedDevices_.end()) {
            if ((*it)->pairDeviceDescriptor_ != nullptr) {
                (*it)->pairDeviceDescriptor_->pairDeviceDescriptor_ = nullptr;
            }
            it = connectedDevices_.erase(it);
        }
    }
    AUDIO_INFO_LOG("Connected list %{public}s", GetConnDevicesStr(connectedDevices_).c_str());
}

void AudioDeviceManager::AddDefaultDevices(const sptr<AudioDeviceDescriptor> &devDesc)
{
    DeviceType devType = devDesc->deviceType_;
    if (devType == DEVICE_TYPE_EARPIECE) {
        earpiece_ = devDesc;
    } else if (devType == DEVICE_TYPE_SPEAKER) {
        speaker_ = devDesc;
    } else if (devType == DEVICE_TYPE_MIC) {
        defalutMic_ = devDesc;
    }
}

void AudioDeviceManager::UpdateDeviceInfo(shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    if (deviceDesc->connectTimeStamp_ == 0) {
        deviceDesc->connectTimeStamp_ = GetCurrentTimeMS();
    }
    MakePairedDeviceDescriptor(deviceDesc);
}

void AudioDeviceManager::AddCommunicationDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, OUTPUT_DEVICE, VOICE, "communication render privacy device",
        commRenderPrivacyDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PUBLIC, OUTPUT_DEVICE, VOICE, "communication render public device",
        commRenderPublicDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, INPUT_DEVICE, VOICE, "communication capture privacy device",
        commCapturePrivacyDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PUBLIC, INPUT_DEVICE, VOICE, "communication capture public device",
        commCapturePublicDevices_);
}

void AudioDeviceManager::AddMediaDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, OUTPUT_DEVICE, MEDIA, "media render privacy device",
        mediaRenderPrivacyDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PUBLIC, OUTPUT_DEVICE, MEDIA, "media render public device",
        mediaRenderPublicDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, INPUT_DEVICE, MEDIA, "media capture privacy device",
        mediaCapturePrivacyDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PUBLIC, INPUT_DEVICE, MEDIA, "media capture public device",
        mediaCapturePublicDevices_);
}

void AudioDeviceManager::AddCaptureDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, INPUT_DEVICE, ALL_USAGE, "capture privacy device",
        capturePrivacyDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PUBLIC, INPUT_DEVICE, ALL_USAGE, "capture public device",
        capturePublicDevices_);
    FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, INPUT_DEVICE, RECONGNITION, "capture recon privacy device",
        reconCapturePrivacyDevices_);
}

void AudioDeviceManager::HandleScoWithDefaultCategory(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    if (devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && devDesc->deviceCategory_ == CATEGORY_DEFAULT &&
        devDesc->isEnable_) {
        if (devDesc->deviceRole_ == INPUT_DEVICE) {
            commCapturePrivacyDevices_.push_back(devDesc);
        } else if (devDesc->deviceRole_ == OUTPUT_DEVICE) {
            commRenderPrivacyDevices_.push_back(devDesc);
        }
    }
}

bool AudioDeviceManager::UpdateExistDeviceDescriptor(const sptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    auto isPresent = [&deviceDescriptor](const shared_ptr<AudioDeviceDescriptor> &descriptor) {
        if (descriptor->deviceType_ == deviceDescriptor->deviceType_ &&
            descriptor->networkId_ == deviceDescriptor->networkId_ &&
            descriptor->deviceRole_ == deviceDescriptor->deviceRole_) {
            if (descriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP &&
                descriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
                return true;
            } else {
                // if the disconnecting device is A2DP, need to compare mac address in addition.
                return descriptor->macAddress_ == deviceDescriptor->macAddress_;
            }
        }
        return false;
    };

    auto iter = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (iter != connectedDevices_.end()) {
        **iter = deviceDescriptor;
        UpdateDeviceInfo(*iter);
        return true;
    }
    return false;
}

void AudioDeviceManager::RemoveVirtualConnectedDevice(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    auto isPresent = [&devDesc](const shared_ptr<AudioDeviceDescriptor> &descriptor) {
        return descriptor->deviceType_ == devDesc->deviceType_
            && descriptor->deviceRole_ == devDesc->deviceRole_
            && descriptor->networkId_ == devDesc->networkId_
            && descriptor->macAddress_ == devDesc->macAddress_
            && descriptor->connectState_ == VIRTUAL_CONNECTED;
    };
    connectedDevices_.erase(std::remove_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent),
        connectedDevices_.end());
}

void AudioDeviceManager::AddNewDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(deviceDescriptor);
    CHECK_AND_RETURN_LOG(devDesc != nullptr, "Memory allocation failed");

    int32_t audioId = deviceDescriptor->deviceId_;
    AUDIO_INFO_LOG("add type:id %{public}d:%{public}d", deviceDescriptor->getType(), audioId);

    RemoveVirtualConnectedDevice(devDesc);
    if (UpdateExistDeviceDescriptor(deviceDescriptor)) {
        AUDIO_INFO_LOG("The device has been added and will not be added again.");
        return;
    }
    AddConnectedDevices(devDesc);

    if (devDesc->networkId_ != LOCAL_NETWORK_ID || devDesc->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        AddRemoteRenderDev(devDesc);
        AddRemoteCaptureDev(devDesc);
    } else {
        HandleScoWithDefaultCategory(devDesc);
        AddDefaultDevices(deviceDescriptor);
        AddCommunicationDevices(devDesc);
        AddMediaDevices(devDesc);
        AddCaptureDevices(devDesc);
    }
    UpdateDeviceInfo(devDesc);
}

std::string AudioDeviceManager::GetConnDevicesStr()
{
    return GetConnDevicesStr(connectedDevices_);
}

std::string AudioDeviceManager::GetConnDevicesStr(const vector<shared_ptr<AudioDeviceDescriptor>> &descs)
{
    std::string devices;
    devices.append("device type:id:(category:constate) ");
    for (auto iter : descs) {
        CHECK_AND_CONTINUE_LOG(iter != nullptr, "iter is nullptr");
        devices.append(std::to_string(static_cast<uint32_t>(iter->getType())));
        devices.append(":" + std::to_string(static_cast<uint32_t>(iter->deviceId_)));
        if (iter->getType() == DEVICE_TYPE_BLUETOOTH_A2DP ||
            iter->getType() == DEVICE_TYPE_BLUETOOTH_SCO) {
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->deviceCategory_)));
            devices.append(":" + std::to_string(static_cast<uint32_t>(iter->connectState_)));
        }
        devices.append(" ");
    }
    return devices;
}

void AudioDeviceManager::RemoveMatchDeviceInArray(const AudioDeviceDescriptor &devDesc, string logName,
    vector<shared_ptr<AudioDeviceDescriptor>> &descArray)
{
    auto isPresent = [&devDesc] (const shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return devDesc.deviceType_ == desc->deviceType_ && devDesc.macAddress_ == desc->macAddress_ &&
            devDesc.networkId_ == desc->networkId_;
    };

    auto removeBeginIt = std::remove_if(descArray.begin(), descArray.end(), isPresent);
    size_t deleteNum = static_cast<uint32_t>(descArray.end() - removeBeginIt);
    descArray.erase(removeBeginIt, descArray.end());

    AUDIO_INFO_LOG("Remove %{public}zu desc from %{public}s list, and then %{public}s", deleteNum,
        logName.c_str(), GetConnDevicesStr(descArray).c_str());
}

void AudioDeviceManager::RemoveNewDevice(const sptr<AudioDeviceDescriptor> &devDesc)
{
    int32_t audioId = devDesc->deviceId_;
    AUDIO_INFO_LOG("remove type:id %{public}d:%{public}d ", devDesc->getType(), audioId);

    RemoveConnectedDevices(make_shared<AudioDeviceDescriptor>(devDesc));
    RemoveRemoteDevices(devDesc);
    RemoveCommunicationDevices(devDesc);
    RemoveMediaDevices(devDesc);
    RemoveCaptureDevices(devDesc);
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRemoteRenderDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : remoteRenderDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRemoteCaptureDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : remoteCaptureDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderPrivacyDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commRenderPrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderPublicDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commRenderPublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderBTCarDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> carDescs;
    for (const auto &desc : commRenderPublicDevices_) {
        if (desc == nullptr || desc->deviceCategory_ != BT_CAR) {
            continue;
        }
        carDescs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return carDescs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommCapturePrivacyDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commCapturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommCapturePublicDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commCapturePublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaRenderPrivacyDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaRenderPrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaRenderPublicDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaRenderPublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaCapturePrivacyDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaCapturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaCapturePublicDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaCapturePublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCapturePrivacyDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : capturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCapturePublicDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : capturePublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRecongnitionCapturePrivacyDevices()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : reconCapturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}
// LCOV_EXCL_STOP
unique_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetCommRenderDefaultDevice(StreamUsage streamUsage)
{
    if (streamUsage < STREAM_USAGE_UNKNOWN || streamUsage > STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        AUDIO_DEBUG_LOG("Invalid stream usage");
    }

    unique_ptr<AudioDeviceDescriptor> devDesc;
    if (hasEarpiece_ && streamUsage != STREAM_USAGE_VIDEO_COMMUNICATION) {
        devDesc = make_unique<AudioDeviceDescriptor>(earpiece_);
    } else {
        devDesc = make_unique<AudioDeviceDescriptor>(speaker_);
    }
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetRenderDefaultDevice()
{
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(speaker_);
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetCaptureDefaultDevice()
{
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(defalutMic_);
    return devDesc;
}

// LCOV_EXCL_START
void AudioDeviceManager::AddAvailableDevicesByUsage(const AudioDeviceUsage usage,
    const DevicePrivacyInfo &deviceInfo, const sptr<AudioDeviceDescriptor> &dev,
    std::vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    switch (usage) {
        case MEDIA_OUTPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & OUTPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & MEDIA)) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(dev));
            }
            break;
        case MEDIA_INPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & INPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & MEDIA)) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(dev));
            }
            break;
        case ALL_MEDIA_DEVICES:
            if (static_cast<uint32_t>(deviceInfo.deviceUsage) & MEDIA) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(dev));
            }
            break;
        case CALL_OUTPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & OUTPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & VOICE)) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(dev));
            }
            break;
        case CALL_INPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & INPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & VOICE)) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(dev));
            }
            break;
        case ALL_CALL_DEVICES:
            if (static_cast<uint32_t>(deviceInfo.deviceUsage) & VOICE) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(dev));
            }
            break;
        default:
            break;
    }
}

bool AudioDeviceManager::IsExistedDevice(const sptr<AudioDeviceDescriptor> &device,
    const vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    bool isExistedDev = false;
    for (const auto &dev : audioDeviceDescriptors) {
        if (device->deviceType_ == dev->deviceType_ &&
            device->networkId_ == dev->networkId_ &&
            device->deviceRole_ == dev->deviceRole_ &&
            device->macAddress_ == dev->macAddress_) {
            isExistedDev = true;
        }
    }
    return isExistedDev;
}

void AudioDeviceManager::GetAvailableDevicesWithUsage(const AudioDeviceUsage usage,
    const list<DevicePrivacyInfo> &deviceInfos, const sptr<AudioDeviceDescriptor> &dev,
    vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    for (auto &deviceInfo : deviceInfos) {
        if (dev->deviceType_ != deviceInfo.deviceType ||
            IsExistedDevice(dev, audioDeviceDescriptors)) {
            continue;
        }
        AddAvailableDevicesByUsage(usage, deviceInfo, dev, audioDeviceDescriptors);
    }
}

void AudioDeviceManager::GetDefaultAvailableDevicesByUsage(AudioDeviceUsage usage,
    vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    if (((usage & MEDIA_OUTPUT_DEVICES) != 0) || ((usage & CALL_OUTPUT_DEVICES) != 0)) {
        if (speaker_ != nullptr) {
            audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(speaker_));
        }
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_SPEAKER && desc->networkId_ != LOCAL_NETWORK_ID) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(*desc));
            }
        }
    }

    if (((usage & MEDIA_INPUT_DEVICES) != 0) || ((usage & CALL_INPUT_DEVICES) != 0)) {
        if (defalutMic_ != nullptr) {
            audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(defalutMic_));
        }
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_MIC && desc->networkId_ != LOCAL_NETWORK_ID) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(*desc));
            }
        }
    }

    if ((usage & CALL_OUTPUT_DEVICES) != 0) {
        if (earpiece_ != nullptr) {
            audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(earpiece_));
        }
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_EARPIECE && desc->networkId_ != LOCAL_NETWORK_ID) {
                audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(*desc));
            }
        }
    }
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableDevicesByUsage(AudioDeviceUsage usage)
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    GetDefaultAvailableDevicesByUsage(usage, audioDeviceDescriptors);

    for (const auto &dev : connectedDevices_) {
        for (const auto &devicePrivacy : devicePrivacyMaps_) {
            list<DevicePrivacyInfo> deviceInfos = devicePrivacy.second;
            sptr<AudioDeviceDescriptor> desc = new (std::nothrow) AudioDeviceDescriptor(*dev);
            GetAvailableDevicesWithUsage(usage, deviceInfos, desc, audioDeviceDescriptors);
        }
    }
    return audioDeviceDescriptors;
}

unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> AudioDeviceManager::GetDevicePrivacyMaps()
{
    return devicePrivacyMaps_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableBluetoothDevice(DeviceType devType,
    const std::string &macAddress)
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == devType && desc->macAddress_ == macAddress) {
            audioDeviceDescriptors.push_back(make_unique<AudioDeviceDescriptor>(*desc));
        }
    }
    return audioDeviceDescriptors;
}

void AudioDeviceManager::UpdateScoState(const std::string &macAddress, bool isConnnected)
{
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->macAddress_ == macAddress) {
            desc->isScoRealConnected_ = isConnnected;
        }
    }
}

bool AudioDeviceManager::GetScoState()
{
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->connectState_ == CONNECTED) {
            return true;
        }
    }
    return false;
}

void AudioDeviceManager::UpdateDevicesListInfo(const sptr<AudioDeviceDescriptor> &d,
    const DeviceInfoUpdateCommand updateCommand)
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(d);
    bool ret = false;
    switch (updateCommand) {
        case CATEGORY_UPDATE:
            ret = UpdateDeviceCategory(d);
            break;
        case CONNECTSTATE_UPDATE:
            ret = UpdateConnectState(devDesc);
            break;
        case ENABLE_UPDATE:
            ret = UpdateEnableState(devDesc);
            break;
        case EXCEPTION_FLAG_UPDATE:
            ret = UpdateExceptionFlag(devDesc);
            break;
        default:
            break;
    }
    if (!ret) {
        int32_t audioId = d->deviceId_;
        AUDIO_ERR_LOG("cant find type:id %{public}d:%{public}d mac:%{public}s networkid:%{public}s in connected list",
            d->deviceType_, audioId, GetEncryptStr(d->macAddress_).c_str(), GetEncryptStr(d->networkId_).c_str());
    }
}

bool AudioDeviceManager::UpdateDeviceCategory(const sptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    bool updateFlag = false;
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(deviceDescriptor);

    for (auto &desc : connectedDevices_) {
        if ((devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) &&
            desc->deviceType_ == devDesc->deviceType_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->macAddress_ == devDesc->macAddress_ &&
            desc->deviceCategory_ != devDesc->deviceCategory_) {
            desc->deviceCategory_ = devDesc->deviceCategory_;
            if (devDesc->deviceCategory_ == BT_UNWEAR_HEADPHONE) {
                RemoveBtFromOtherList(deviceDescriptor);
            } else {
                // Update connectTimeStamp_ when wearing headphones that support wear detection
                desc->connectTimeStamp_ = GetCurrentTimeMS();
                AddBtToOtherList(desc);
            }
        }
        updateFlag = true;
    }
    return updateFlag;
}

bool AudioDeviceManager::UpdateConnectState(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    bool isScoDevice = devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO;

    for (const auto &desc : connectedDevices_) {
        if (desc->networkId_ != devDesc->networkId_ ||
            desc->macAddress_ != devDesc->macAddress_) {
            continue;
        }
        if (desc->deviceType_ == devDesc->deviceType_) {
            desc->connectState_ = devDesc->connectState_;
            updateFlag = true;
            continue;
        }
        // a2dp connectState needs to be updated simultaneously when connectState of sco is updated
        if (isScoDevice) {
            if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                devDesc->connectState_ == CONNECTED) {
                // sco connected, suspend a2dp
                desc->connectState_ = SUSPEND_CONNECTED;
                updateFlag = true;
                AUDIO_INFO_LOG("sco connectState is connected, update a2dp to suspend");
            } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc->connectState_ == SUSPEND_CONNECTED &&
                (devDesc->connectState_ == DEACTIVE_CONNECTED || devDesc->connectState_ == SUSPEND_CONNECTED)) {
                // sco deactive or suspend, a2dp CONNECTED
                desc->connectState_ = CONNECTED;
                updateFlag = true;
                AUDIO_INFO_LOG("sco connectState %{public}d, update a2dp to connected", devDesc->connectState_);
            }
        }
    }
    return updateFlag;
}

bool AudioDeviceManager::UpdateEnableState(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    for (const auto &desc : connectedDevices_) {
        if (devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            if (desc->deviceType_ == devDesc->deviceType_ &&
                desc->macAddress_ == devDesc->macAddress_) {
                desc->isEnable_ = devDesc->isEnable_;
                updateFlag = true;
            }
        } else if (desc->deviceType_ == devDesc->deviceType_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->isEnable_ != devDesc->isEnable_) {
                desc->isEnable_ = devDesc->isEnable_;
                updateFlag = true;
        }
    }
    return updateFlag;
}

bool AudioDeviceManager::UpdateExceptionFlag(const shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    bool updateFlag = false;
    for (const auto &desc : connectedDevices_) {
        if (deviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            deviceDescriptor->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            if (desc->deviceType_ == deviceDescriptor->deviceType_ &&
                desc->macAddress_ == deviceDescriptor->macAddress_) {
                desc->exceptionFlag_ = deviceDescriptor->exceptionFlag_;
                updateFlag = true;
            }
        } else if (desc->deviceType_ == deviceDescriptor->deviceType_ &&
            desc->networkId_ == deviceDescriptor->networkId_ &&
            desc->exceptionFlag_ != deviceDescriptor->exceptionFlag_) {
                desc->exceptionFlag_ = deviceDescriptor->exceptionFlag_;
                updateFlag = true;
        }
    }
    return updateFlag;
}

void AudioDeviceManager::UpdateEarpieceStatus(const bool hasEarPiece)
{
    hasEarpiece_ = hasEarPiece;
}

void AudioDeviceManager::AddBtToOtherList(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    if (devDesc->networkId_ != LOCAL_NETWORK_ID) {
        AddRemoteRenderDev(devDesc);
        AddRemoteCaptureDev(devDesc);
    } else {
        HandleScoWithDefaultCategory(devDesc);
        AddCommunicationDevices(devDesc);
        AddMediaDevices(devDesc);
        AddCaptureDevices(devDesc);
    }
}

void AudioDeviceManager::RemoveBtFromOtherList(const AudioDeviceDescriptor &devDesc)
{
    if (devDesc.networkId_ != LOCAL_NETWORK_ID) {
        RemoveRemoteDevices(devDesc);
    } else {
        RemoveCommunicationDevices(devDesc);
        RemoveMediaDevices(devDesc);
        RemoveCaptureDevices(devDesc);
    }
}

void AudioDeviceManager::RemoveRemoteDevices(const AudioDeviceDescriptor &devDesc)
{
    RemoveMatchDeviceInArray(devDesc, "remote render device", remoteRenderDevices_);
    RemoveMatchDeviceInArray(devDesc, "remote capture device", remoteCaptureDevices_);
}

void AudioDeviceManager::RemoveCommunicationDevices(const AudioDeviceDescriptor &devDesc)
{
    RemoveMatchDeviceInArray(devDesc, "communication render privacy device", commRenderPrivacyDevices_);
    RemoveMatchDeviceInArray(devDesc, "communication render public device", commRenderPublicDevices_);
    RemoveMatchDeviceInArray(devDesc, "communication capture privacy device", commCapturePrivacyDevices_);
    RemoveMatchDeviceInArray(devDesc, "communication capture public device", commCapturePublicDevices_);
}

void AudioDeviceManager::RemoveMediaDevices(const AudioDeviceDescriptor &devDesc)
{
    RemoveMatchDeviceInArray(devDesc, "media render privacy device", mediaRenderPrivacyDevices_);
    RemoveMatchDeviceInArray(devDesc, "media render public device", mediaRenderPublicDevices_);
    RemoveMatchDeviceInArray(devDesc, "media capture privacy device", mediaCapturePrivacyDevices_);
    RemoveMatchDeviceInArray(devDesc, "media capture public device", mediaCapturePublicDevices_);
}

void AudioDeviceManager::RemoveCaptureDevices(const AudioDeviceDescriptor &devDesc)
{
    RemoveMatchDeviceInArray(devDesc, "capture privacy device", capturePrivacyDevices_);
    RemoveMatchDeviceInArray(devDesc, "capture public device", capturePublicDevices_);
    RemoveMatchDeviceInArray(devDesc, "capture recon privacy device", reconCapturePrivacyDevices_);
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetDevicesByFilter(DeviceType devType, DeviceRole devRole,
    const string &macAddress, const string &networkId, ConnectState connectState)
{
    vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    for (const auto &desc : connectedDevices_) {
        if ((devType == DEVICE_TYPE_NONE || devType == desc->deviceType_) &&
            (devRole == DEVICE_ROLE_NONE || devRole == desc->deviceRole_) &&
            (macAddress == "" || macAddress == desc->macAddress_) &&
            (networkId == "" || networkId == desc->networkId_) && (connectState == desc->connectState_)) {
            audioDeviceDescriptors.push_back(desc);
        }
    }
    AUDIO_DEBUG_LOG("Filter device size %{public}zu", audioDeviceDescriptors.size());
    return audioDeviceDescriptors;
}

DeviceUsage AudioDeviceManager::GetDeviceUsage(const AudioDeviceDescriptor &desc)
{
    AUDIO_DEBUG_LOG("device type [%{public}d] category [%{public}d]", desc.deviceType_, desc.deviceCategory_);
    DeviceUsage usage = MEDIA;
    for (auto &devInfo : privacyDeviceList_) {
        if ((devInfo.deviceType == desc.deviceType_) &&
            ((devInfo.deviceCategory & desc.deviceCategory_) || devInfo.deviceCategory == 0)) {
            return devInfo.deviceUsage;
        }
    }

    for (auto &devInfo : publicDeviceList_) {
        if ((devInfo.deviceType == desc.deviceType_) &&
            ((devInfo.deviceCategory & desc.deviceCategory_) || devInfo.deviceCategory == 0)) {
            return devInfo.deviceUsage;
        }
    }

    if (DEVICE_TYPE_BLUETOOTH_A2DP == desc.deviceType_) {
        usage = MEDIA;
    }

    if (DEVICE_TYPE_BLUETOOTH_SCO == desc.deviceType_) {
        usage = VOICE;
    }

    return usage;
}

void AudioDeviceManager::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    for (auto device : connectedDevices_) {
        if (device->macAddress_ == macAddress) {
            device->deviceName_ = deviceName;
        }
    }
}

bool AudioDeviceManager::IsDeviceConnected(sptr<AudioDeviceDescriptor> &audioDeviceDescriptors)
{
    size_t connectedDevicesNum = connectedDevices_.size();
    for (size_t i = 0; i < connectedDevicesNum; i++) {
        if (connectedDevices_[i] != nullptr) {
            if (connectedDevices_[i]->deviceRole_ == audioDeviceDescriptors->deviceRole_
                && connectedDevices_[i]->deviceType_ == audioDeviceDescriptors->deviceType_
                && connectedDevices_[i]->networkId_ == audioDeviceDescriptors->networkId_
                && connectedDevices_[i]->macAddress_ == audioDeviceDescriptors->macAddress_
                && connectedDevices_[i]->volumeGroupId_ == audioDeviceDescriptors->volumeGroupId_) {
                return true;
            }
        }
    }
    AUDIO_WARNING_LOG("Role:%{public}d networkId:%{public}s Type:%{public}d macAddress:%{public}s device not found",
        audioDeviceDescriptors->deviceRole_, GetEncryptStr(audioDeviceDescriptors->networkId_).c_str(),
        audioDeviceDescriptors->deviceType_, GetEncryptAddr(audioDeviceDescriptors->macAddress_).c_str());
    return false;
}

bool AudioDeviceManager::IsVirtualConnectedDevice(const sptr<AudioDeviceDescriptor> &selectedDesc)
{
    CHECK_AND_RETURN_RET_LOG(selectedDesc != nullptr, false, "Invalid device descriptor");
    auto isVirtual = [&selectedDesc](const shared_ptr<AudioDeviceDescriptor>& desc) {
        return desc->connectState_ == VIRTUAL_CONNECTED
            && desc->deviceRole_ == selectedDesc->deviceRole_
            && desc->deviceType_ == selectedDesc->deviceType_
            && desc->networkId_ == selectedDesc->networkId_
            && desc->macAddress_ == selectedDesc->macAddress_;
    };
    bool isVirtualDevice = false;
    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isVirtual);
    if (itr != connectedDevices_.end()) {
        isVirtualDevice = true;
        AUDIO_INFO_LOG("Device[%{public}s] is virtual connection",
            GetEncryptAddr(selectedDesc->macAddress_).c_str());
    }
    return isVirtualDevice;
}

int32_t AudioDeviceManager::UpdateDeviceDescDeviceId(sptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, ERROR, "Invalid device descriptor");
    auto isPresent = [&deviceDescriptor](const shared_ptr<AudioDeviceDescriptor> &desc) {
        return desc->deviceRole_ == deviceDescriptor->deviceRole_
            && desc->deviceType_ == deviceDescriptor->deviceType_
            && desc->networkId_ == deviceDescriptor->networkId_
            && desc->macAddress_ == deviceDescriptor->macAddress_;
    };
    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    CHECK_AND_RETURN_RET_LOG(itr != connectedDevices_.end(), ERROR, "Device not found");
    deviceDescriptor->deviceId_ = (*itr)->deviceId_;
    return SUCCESS;
}

int32_t AudioDeviceManager::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    selectedDefaultOutputDeviceInfo_[sessionID] = std::make_pair(deviceType, streamUsage);
    if (!isRunning) {
        AUDIO_INFO_LOG("no need to set default output device since current stream has not started");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("stream %{public}u with usage %{public}d selects output device %{public}d",
        sessionID, streamUsage, deviceType);
    if (streamUsage == STREAM_USAGE_VOICE_MESSAGE) {
        // select media default output device
        auto it = std::find_if(mediaDefaultOutputDevices_.begin(), mediaDefaultOutputDevices_.end(),
            [&sessionID](const std::pair<uint32_t, DeviceType> &mediaDefaultOutputDevice) {
                return mediaDefaultOutputDevice.first == sessionID;
            });
        if (it != mediaDefaultOutputDevices_.end()) {
            mediaDefaultOutputDevices_.erase(it);
        }
        mediaDefaultOutputDevices_.push_back(std::make_pair(sessionID, deviceType));
        if (selectedMediaDefaultOutputDevice_ != deviceType) {
            AUDIO_INFO_LOG("media default output device changes from %{public}d to %{public}d",
                selectedMediaDefaultOutputDevice_, deviceType);
            selectedMediaDefaultOutputDevice_ = deviceType;
            return NEED_TO_FETCH;
        }
    } else if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION || streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        // select call default output device
        auto it = std::find_if(callDefaultOutputDevices_.begin(), callDefaultOutputDevices_.end(),
            [&sessionID](const std::pair<uint32_t, DeviceType> &callDefaultOutputDevice) {
                return callDefaultOutputDevice.first == sessionID;
            });
        if (it != callDefaultOutputDevices_.end()) {
            callDefaultOutputDevices_.erase(it);
        }
        callDefaultOutputDevices_.push_back(std::make_pair(sessionID, deviceType));
        if (selectedCallDefaultOutputDevice_ != deviceType) {
            AUDIO_INFO_LOG("call default output device changes from %{public}d to %{public}d",
                selectedCallDefaultOutputDevice_, deviceType);
            selectedCallDefaultOutputDevice_ = deviceType;
            return NEED_TO_FETCH;
        }
    } else {
        AUDIO_ERR_LOG("Invalid stream usage %{public}d", streamUsage);
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioDeviceManager::UpdateDefaultOutputDeviceWhenStarting(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    if (!selectedDefaultOutputDeviceInfo_.count(sessionID)) {
        AUDIO_DEBUG_LOG("no need to update default output device since current stream has not set");
        return SUCCESS;
    }
    DeviceType deviceType = selectedDefaultOutputDeviceInfo_[sessionID].first;
    StreamUsage streamUsage = selectedDefaultOutputDeviceInfo_[sessionID].second;
    if (streamUsage == STREAM_USAGE_VOICE_MESSAGE) {
        // select media default output device
        auto it = std::find_if(mediaDefaultOutputDevices_.begin(), mediaDefaultOutputDevices_.end(),
            [&sessionID](const std::pair<uint32_t, DeviceType> &mediaDefaultOutputDevice) {
                return mediaDefaultOutputDevice.first == sessionID;
            });
        if (it != mediaDefaultOutputDevices_.end()) {
            mediaDefaultOutputDevices_.erase(it);
        }
        mediaDefaultOutputDevices_.push_back(std::make_pair(sessionID, deviceType));
        AUDIO_INFO_LOG("changes from %{public}d to %{public}d because media stream %{public}u starts",
            selectedMediaDefaultOutputDevice_, deviceType, sessionID);
        selectedMediaDefaultOutputDevice_ = deviceType;
    } else if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION || streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        // select call default output device
        auto it = std::find_if(callDefaultOutputDevices_.begin(), callDefaultOutputDevices_.end(),
            [&sessionID](const std::pair<uint32_t, DeviceType> &callDefaultOutputDevice) {
                return callDefaultOutputDevice.first == sessionID;
            });
        if (it != callDefaultOutputDevices_.end()) {
            callDefaultOutputDevices_.erase(it);
        }
        callDefaultOutputDevices_.push_back(std::make_pair(sessionID, deviceType));
        AUDIO_INFO_LOG("changes from %{public}d to %{public}d because call stream %{public}u starts",
            selectedCallDefaultOutputDevice_, deviceType, sessionID);
        selectedCallDefaultOutputDevice_ = deviceType;
    }
    return SUCCESS;
}

int32_t AudioDeviceManager::UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    if (!selectedDefaultOutputDeviceInfo_.count(sessionID)) {
        AUDIO_DEBUG_LOG("no need to update default output device since current stream has not set");
        return SUCCESS;
    }
    StreamUsage streamUsage = selectedDefaultOutputDeviceInfo_[sessionID].second;
    if (streamUsage == STREAM_USAGE_VOICE_MESSAGE) {
        // select media default output device
        auto it = std::find_if(mediaDefaultOutputDevices_.begin(), mediaDefaultOutputDevices_.end(),
            [&sessionID](const std::pair<uint32_t, DeviceType> &mediaDefaultOutputDevice) {
                return mediaDefaultOutputDevice.first == sessionID;
            });
        if (it == mediaDefaultOutputDevices_.end()) {
            return SUCCESS;
        }
        mediaDefaultOutputDevices_.erase(it);
        DeviceType currDeviceType;
        if (mediaDefaultOutputDevices_.empty()) {
            currDeviceType = DEVICE_TYPE_DEFAULT;
        } else {
            currDeviceType = mediaDefaultOutputDevices_.back().second;
        }
        AUDIO_INFO_LOG("changes from %{public}d to %{public}d because media stream %{public}u stops",
            selectedMediaDefaultOutputDevice_, currDeviceType, sessionID);
        selectedMediaDefaultOutputDevice_ = currDeviceType;
    } else if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION || streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        // select call default output device
        auto it = std::find_if(callDefaultOutputDevices_.begin(), callDefaultOutputDevices_.end(),
            [&sessionID](const std::pair<uint32_t, DeviceType> &callDefaultOutputDevice) {
                return callDefaultOutputDevice.first == sessionID;
            });
        if (it == callDefaultOutputDevices_.end()) {
            return SUCCESS;
        }
        callDefaultOutputDevices_.erase(it);
        DeviceType currDeviceType;
        if (callDefaultOutputDevices_.empty()) {
            currDeviceType = DEVICE_TYPE_DEFAULT;
        } else {
            currDeviceType = callDefaultOutputDevices_.back().second;
        }
        AUDIO_INFO_LOG("changes from %{public}d to %{public}d because call stream %{public}u stops",
            selectedCallDefaultOutputDevice_, currDeviceType, sessionID);
        selectedCallDefaultOutputDevice_ = currDeviceType;
    }
    return SUCCESS;
}

int32_t AudioDeviceManager::RemoveSelectedDefaultOutputDevice(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    selectedDefaultOutputDeviceInfo_.erase(sessionID);
    return SUCCESS;
}

unique_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetSelectedMediaRenderDevice()
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    unique_ptr<AudioDeviceDescriptor> devDesc = nullptr;
    if (selectedMediaDefaultOutputDevice_ == DEVICE_TYPE_EARPIECE) {
        devDesc = make_unique<AudioDeviceDescriptor>(earpiece_);
    } else if (selectedMediaDefaultOutputDevice_ == DEVICE_TYPE_SPEAKER) {
        devDesc = make_unique<AudioDeviceDescriptor>(speaker_);
    }
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetSelectedCallRenderDevice()
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    unique_ptr<AudioDeviceDescriptor> devDesc = nullptr;
    if (selectedCallDefaultOutputDevice_ == DEVICE_TYPE_EARPIECE) {
        devDesc = make_unique<AudioDeviceDescriptor>(earpiece_);
    } else if (selectedCallDefaultOutputDevice_ == DEVICE_TYPE_SPEAKER) {
        devDesc = make_unique<AudioDeviceDescriptor>(speaker_);
    }
    return devDesc;
}
// LCOV_EXCL_STOP
}
}
