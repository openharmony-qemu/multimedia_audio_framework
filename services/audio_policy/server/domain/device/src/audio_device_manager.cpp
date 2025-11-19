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
#include "audio_policy_utils.h"
#include "audio_bluetooth_manager.h"
#include "audio_adapter_manager.h"
#include "audio_device_status.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
constexpr int32_t MS_PER_S = 1000;
constexpr int32_t NS_PER_MS = 1000000;
const int32_t ADDRESS_STR_LEN = 17;
const int32_t START_POS = 6;
const int32_t END_POS = 13;

// LCOV_EXCL_START
static std::string GetEncryptAddr(const std::string &addr)
{
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        string macHead("card=");
        if (addr.find(macHead, 0) == 0) {
            auto pos = macHead.length();
            auto end = addr.find(';', macHead.length());
            auto num = end == string::npos ? addr.substr(pos) : addr.substr(pos, end - pos);
            return string("c**") + num + "**";
        }
        return "";
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
    AUDIO_INFO_LOG("In");
    unique_ptr<AudioDeviceParser> audioDeviceParser = make_unique<AudioDeviceParser>(this);
    if (audioDeviceParser->LoadConfiguration()) {
        AUDIO_INFO_LOG("Audio device manager load configuration successfully.");
    }
    AUDIO_INFO_LOG("Out");
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
    AudioDevicePrivacyType privacyType, DeviceRole devRole, DeviceUsage devUsage)
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

    if (devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        devUsage == MEDIA &&
        devRole == OUTPUT_DEVICE) {
        AUDIO_INFO_LOG("bluetooth sco not support in media output scene");
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

bool AudioDeviceManager::IsDescMatchedInVector(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    list<DevicePrivacyInfo> &deviceList)
{
    for (auto &devInfo : deviceList) {
        if ((devInfo.deviceType == devDesc->deviceType_) &&
            (devInfo.deviceUsage & devDesc->deviceUsage_) &&
            ((devInfo.deviceCategory == devDesc->deviceCategory_) ||
            (devInfo.deviceCategory & devDesc->deviceCategory_) != 0)) {
            return true;
        }
    }

    return false;
}

AudioDevicePrivacyType AudioDeviceManager::GetDevicePrivacyType(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, AudioDevicePrivacyType::TYPE_NEGATIVE, "devDesc is nullptr");

    if (IsDescMatchedInVector(devDesc, privacyDeviceList_)) {
        return AudioDevicePrivacyType::TYPE_PRIVACY;
    }

    if (IsDescMatchedInVector(devDesc, publicDeviceList_)) {
        return AudioDevicePrivacyType::TYPE_PUBLIC;
    }

    return AudioDevicePrivacyType::TYPE_NEGATIVE;
}

void AudioDeviceManager::FillArrayWhenDeviceAttrMatch(const shared_ptr<AudioDeviceDescriptor> &devDesc,
    AudioDevicePrivacyType privacyType, DeviceRole devRole, DeviceUsage devUsage, string logName,
    vector<shared_ptr<AudioDeviceDescriptor>> &descArray)
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    bool result = DeviceAttrMatch(devDesc, privacyType, devRole, devUsage);
    if (result) {
        descArray.push_back(devDesc);
        AUDIO_WARNING_LOG("Add to %{public}s list, and then %{public}s",
            logName.c_str(), AudioPolicyUtils::GetInstance().GetDevicesStr(descArray).c_str());
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

void AudioDeviceManager::MakePairedDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    auto isPresent = [&devDesc] (const shared_ptr<AudioDeviceDescriptor> &desc) {
        return devDesc->IsPairedDeviceDesc(*desc);
    };

    if (devDesc->deviceRole_ == INPUT_DEVICE || devDesc->deviceRole_ == OUTPUT_DEVICE) {
        auto it = find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
        if (it != connectedDevices_.end()) {
            devDesc->pairDeviceDescriptor_ = *it;
            (*it)->pairDeviceDescriptor_ = devDesc;
        }
        if (devDesc->deviceRole_ == INPUT_DEVICE) {
            MakePairedDefaultDeviceDescriptor(devDesc, OUTPUT_DEVICE);
        } else {
            MakePairedDefaultDeviceDescriptor(devDesc, INPUT_DEVICE);
        }
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
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (itr == connectedDevices_.end()) {
        return false;
    }

    return (*itr)->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET;
}

void AudioDeviceManager::AddConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    connectedDevices_.insert(connectedDevices_.begin(), devDesc);
    AUDIO_INFO_LOG("Connected list %{public}s",
        AudioPolicyUtils::GetInstance().GetDevicesStr(connectedDevices_).c_str());
}

void AudioDeviceManager::RemoveConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    Trace trace("AudioDeviceManager::RemoveConnectedDevices");
    auto isPresent = [&devDesc](const shared_ptr<AudioDeviceDescriptor> &descriptor) {
        if (descriptor->deviceType_ == devDesc->deviceType_ &&
            descriptor->networkId_ == devDesc->networkId_) {
            if (IsUsb(descriptor->deviceType_)) {
                return descriptor->macAddress_ == devDesc->macAddress_ &&
                    descriptor->deviceRole_ == devDesc->deviceRole_;
            }
            // if the disconnecting device, need to compare mac address in addition.
            return descriptor->macAddress_ == devDesc->macAddress_;
        }
        return false;
    };

    for (auto it = connectedDevices_.begin(); it != connectedDevices_.end();) {
        Trace traceSec("AudioDeviceManager::RemoveConnectedDevices:erase");
        it = find_if(it, connectedDevices_.end(), isPresent);
        if (it != connectedDevices_.end()) {
            if (devDesc->connectState_ != VIRTUAL_CONNECTED && IsVirtualDevicesExist(devDesc)) {
                (*it)->connectState_ = VIRTUAL_CONNECTED;
                ++it;
                continue;
            }
            if ((*it)->pairDeviceDescriptor_ != nullptr) {
                (*it)->pairDeviceDescriptor_->pairDeviceDescriptor_ = nullptr;
            }
            it = connectedDevices_.erase(it);
        }
    }
    AUDIO_INFO_LOG("Connected list %{public}s",
        AudioPolicyUtils::GetInstance().GetDevicesStr(connectedDevices_).c_str());
}

bool AudioDeviceManager::IsConnectedDevices(const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, false, "Invalid device descriptor");
    auto isPresent = [&devDesc](const shared_ptr<AudioDeviceDescriptor> &desc) {
        return desc->connectState_ != VIRTUAL_CONNECTED &&
            desc->deviceType_ == devDesc->deviceType_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->macAddress_ == devDesc->macAddress_;
    };
    bool isConnectedDevice = false;
    auto itr = find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (itr != connectedDevices_.end()) {
        isConnectedDevice = true;
    }
    AUDIO_INFO_LOG("isConnectedDevice %{public}d, connected list %{public}s", isConnectedDevice,
        AudioPolicyUtils::GetInstance().GetDevicesStr(connectedDevices_).c_str());
    return isConnectedDevice;
}

bool AudioDeviceManager::HasConnectedA2dp()
{
    auto isPresent = [](const shared_ptr<AudioDeviceDescriptor> &desc) {
        return desc != nullptr &&
            desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
            desc->connectState_ != VIRTUAL_CONNECTED;
    };
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    auto it = find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    return it != connectedDevices_.end();
}

void AudioDeviceManager::UpdateVirtualDevices(const std::shared_ptr<AudioDeviceDescriptor> &devDesc, bool isConnected)
{
    CHECK_AND_RETURN_LOG(devDesc != nullptr, "Invalid device descriptor");
    auto isPresent = [&devDesc](const shared_ptr<AudioDeviceDescriptor> &desc) {
        return desc->deviceType_ == devDesc->deviceType_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->macAddress_ == devDesc->macAddress_;
    };
    if (isConnected) {
        std::lock_guard<std::mutex> lock(virtualDevicesMutex_);
        auto it = find_if(virtualDevices_.begin(), virtualDevices_.end(), isPresent);
        if (it == virtualDevices_.end()) {
            virtualDevices_.push_back(devDesc);
        }
    } else {
        std::lock_guard<std::mutex> lock(virtualDevicesMutex_);
        virtualDevices_.erase(std::remove_if(virtualDevices_.begin(), virtualDevices_.end(), isPresent),
            virtualDevices_.end());
    }
    AUDIO_INFO_LOG("VirtualDevices list %{public}s",
        AudioPolicyUtils::GetInstance().GetDevicesStr(virtualDevices_).c_str());
}

bool AudioDeviceManager::IsVirtualDevicesExist(const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, false, "Invalid device descriptor");
    auto isPresent = [&devDesc](const shared_ptr<AudioDeviceDescriptor> &desc) {
        return desc->deviceType_ == devDesc->deviceType_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->macAddress_ == devDesc->macAddress_;
    };

    std::lock_guard<std::mutex> lock(virtualDevicesMutex_);
    auto it = find_if(virtualDevices_.begin(), virtualDevices_.end(), isPresent);
    bool isExist = it != virtualDevices_.end();
    AUDIO_INFO_LOG("isVirtualDevicesExist %{public}d, virtualDevices list %{public}s", isExist,
        AudioPolicyUtils::GetInstance().GetDevicesStr(virtualDevices_).c_str());
    return isExist;
}

void AudioDeviceManager::AddDefaultDevices(const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
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
    if (devDesc->isVrSupported_ == true) {
        AUDIO_INFO_LOG("Device supports recognition in");
        FillArrayWhenDeviceAttrMatch(devDesc, TYPE_PRIVACY, INPUT_DEVICE, RECOGNITION,
            "capture recognition privacy device", reconCapturePrivacyDevices_);
    }
}

void AudioDeviceManager::HandleScoWithDefaultCategory(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    if (devDesc->connectState_ != VIRTUAL_CONNECTED &&
        devDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        devDesc->deviceCategory_ == CATEGORY_DEFAULT &&
        devDesc->isEnable_) {
        if (devDesc->deviceRole_ == INPUT_DEVICE) {
            commCapturePrivacyDevices_.push_back(devDesc);
        } else if (devDesc->deviceRole_ == OUTPUT_DEVICE) {
            commRenderPrivacyDevices_.push_back(devDesc);
        }
    }
}

bool AudioDeviceManager::UpdateExistDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    auto isPresent = [&deviceDescriptor](const shared_ptr<AudioDeviceDescriptor> &descriptor) {
        if (descriptor->deviceType_ == deviceDescriptor->deviceType_ &&
            descriptor->networkId_ == deviceDescriptor->networkId_ &&
            descriptor->deviceRole_ == deviceDescriptor->deviceRole_) {
            if (IsUsb(descriptor->deviceType_)) {
                return descriptor->macAddress_ == deviceDescriptor->macAddress_;
            }
            if (descriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP &&
                descriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO &&
                descriptor->deviceType_ != DEVICE_TYPE_NEARLINK &&
                descriptor->deviceType_ != DEVICE_TYPE_NEARLINK_IN) {
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

void AudioDeviceManager::AddNewDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(deviceDescriptor);
    CHECK_AND_RETURN_LOG(devDesc != nullptr, "Memory allocation failed");

    int32_t audioId = deviceDescriptor->deviceId_;
    AUDIO_INFO_LOG("add type:id %{public}d:%{public}d", deviceDescriptor->getType(), audioId);

    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    RemoveVirtualConnectedDevice(devDesc);
    if (UpdateExistDeviceDescriptor(deviceDescriptor)) {
        AUDIO_WARNING_LOG("The device has been added and will not be added again.");
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
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    return AudioPolicyUtils::GetInstance().GetDevicesStr(connectedDevices_);
}

void AudioDeviceManager::RemoveMatchDeviceInArray(const AudioDeviceDescriptor &devDesc, string logName,
    vector<shared_ptr<AudioDeviceDescriptor>> &descArray)
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    auto isPresent = [&devDesc] (const shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return devDesc.deviceType_ == desc->deviceType_ && devDesc.macAddress_ == desc->macAddress_ &&
            devDesc.networkId_ == desc->networkId_ &&
            (!IsUsb(desc->deviceType_) || devDesc.deviceRole_ == desc->deviceRole_);
    };

    auto removeBeginIt = std::remove_if(descArray.begin(), descArray.end(), isPresent);
    size_t deleteNum = static_cast<uint32_t>(descArray.end() - removeBeginIt);
    descArray.erase(removeBeginIt, descArray.end());

    AUDIO_WARNING_LOG("Remove %{public}zu desc from %{public}s list, and then %{public}s", deleteNum,
        logName.c_str(), AudioPolicyUtils::GetInstance().GetDevicesStr(descArray).c_str());
}

void AudioDeviceManager::RemoveNewDevice(const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    Trace trace("AudioDeviceManager::RemoveNewDevice");
    int32_t audioId = devDesc->deviceId_;
    AUDIO_INFO_LOG("remove type:id %{public}d:%{public}d ", devDesc->getType(), audioId);

    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    RemoveConnectedDevices(devDesc);
    UpdateVirtualDevices(devDesc, false);
    RemoveRemoteDevices(devDesc);
    RemoveCommunicationDevices(devDesc);
    RemoveMediaDevices(devDesc);
    RemoveCaptureDevices(devDesc);
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRemoteRenderDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : remoteRenderDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRemoteCaptureDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : remoteCaptureDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderPrivacyDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commRenderPrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderPublicDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commRenderPublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderBTCarDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> carDescs;
    for (const auto &desc : commRenderPublicDevices_) {
        if (desc == nullptr || desc->deviceCategory_ != BT_CAR) {
            continue;
        }
        carDescs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return carDescs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommCapturePrivacyDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commCapturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommCapturePublicDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : commCapturePublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaRenderPrivacyDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaRenderPrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaRenderPublicDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaRenderPublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaCapturePrivacyDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaCapturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaCapturePublicDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : mediaCapturePublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCapturePrivacyDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : capturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCapturePublicDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : capturePublicDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRecongnitionCapturePrivacyDevices()
{
    std::lock_guard<std::mutex> lock(descArrayMutex_);
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    for (const auto &desc : reconCapturePrivacyDevices_) {
        if (desc == nullptr) {
            continue;
        }
        descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }
    return descs;
}
// LCOV_EXCL_STOP
shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetCommRenderDefaultDevice(StreamUsage streamUsage)
{
    if (streamUsage < STREAM_USAGE_UNKNOWN || streamUsage > STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        AUDIO_DEBUG_LOG("Invalid stream usage");
    }

    shared_ptr<AudioDeviceDescriptor> devDesc;
    if (hasEarpiece_ && streamUsage != STREAM_USAGE_VIDEO_COMMUNICATION) {
        devDesc = make_shared<AudioDeviceDescriptor>(earpiece_);
    } else {
        devDesc = make_shared<AudioDeviceDescriptor>(speaker_);
    }
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetRenderDefaultDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(speaker_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetCaptureDefaultDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(defalutMic_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::FindConnectedDeviceById(const int32_t deviceId)
{
    auto it = std::find_if(connectedDevices_.cbegin(), connectedDevices_.cend(), [deviceId](auto &item) {
        return item->deviceId_ == deviceId;
    });
    return it == connectedDevices_.cend() ? nullptr : *it;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetActiveScoDevice(std::string scoMac, DeviceRole role)
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    for (auto &dev : connectedDevices_) {
        CHECK_AND_RETURN_RET_LOG(dev != nullptr, make_shared<AudioDeviceDescriptor>(),
            "Device is nullptr");
        if (dev->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && dev->macAddress_ == scoMac
            && dev->deviceRole_ == role) {
            return dev;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}


// LCOV_EXCL_START
void AudioDeviceManager::AddAvailableDevicesByUsage(const AudioDeviceUsage usage,
    const DevicePrivacyInfo &deviceInfo, const std::shared_ptr<AudioDeviceDescriptor> &dev,
    std::vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    CHECK_AND_RETURN_LOG(dev != nullptr, "nullptr dev");
    switch (usage) {
        case MEDIA_OUTPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & OUTPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & MEDIA) &&
                (dev->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO)) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            }
            break;
        case MEDIA_INPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & INPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & MEDIA)) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            }
            break;
        case ALL_MEDIA_DEVICES:
            if (static_cast<uint32_t>(deviceInfo.deviceUsage) & MEDIA) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            }
            break;
        case CALL_OUTPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & OUTPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & VOICE)) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            }
            break;
        case CALL_INPUT_DEVICES:
            if ((static_cast<uint32_t>(dev->deviceRole_) & INPUT_DEVICE) &&
                (static_cast<uint32_t>(deviceInfo.deviceUsage) & VOICE)) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            }
            break;
        case ALL_CALL_DEVICES:
            if (static_cast<uint32_t>(deviceInfo.deviceUsage) & VOICE) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            }
            break;
        default:
            break;
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetExistedDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET_LOG(device->deviceType_ != DEVICE_TYPE_NONE, device, "device is nullptr");
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    for (const auto &dev : connectedDevices_) {
        if (dev->IsSameDeviceInfo(*device)) {
            return make_shared<AudioDeviceDescriptor>(dev);
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

bool AudioDeviceManager::IsExistedDevice(const std::shared_ptr<AudioDeviceDescriptor> &device,
    const vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
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
    const list<DevicePrivacyInfo> &deviceInfos, const std::shared_ptr<AudioDeviceDescriptor> &dev,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    for (auto &deviceInfo : deviceInfos) {
        CHECK_AND_BREAK_LOG(dev != nullptr, "dev is nullptr");
        if (dev->deviceType_ != deviceInfo.deviceType ||
            IsExistedDevice(dev, audioDeviceDescriptors)) {
            continue;
        }
        AddAvailableDevicesByUsage(usage, deviceInfo, dev, audioDeviceDescriptors);
    }
}

void AudioDeviceManager::GetDefaultAvailableDevicesByUsage(AudioDeviceUsage usage,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    if (((usage & MEDIA_OUTPUT_DEVICES) != 0) || ((usage & CALL_OUTPUT_DEVICES) != 0)) {
        if (speaker_ != nullptr) {
            audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(speaker_));
        }
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_SPEAKER && desc->networkId_ != LOCAL_NETWORK_ID) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            }
        }
    }

    if (((usage & MEDIA_INPUT_DEVICES) != 0) || ((usage & CALL_INPUT_DEVICES) != 0)) {
        if (defalutMic_ != nullptr) {
            audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(defalutMic_));
        }
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_MIC && desc->networkId_ != LOCAL_NETWORK_ID) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            }
        }
    }

    if ((usage & CALL_OUTPUT_DEVICES) != 0) {
        if (earpiece_ != nullptr) {
            audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(earpiece_));
        }
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_EARPIECE && desc->networkId_ != LOCAL_NETWORK_ID) {
                audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            }
        }
    }
}

// GetRemoteAvailableDevicesByUsage must be called with AudioDeviceManager::currentActiveDevicesMutex_lock
void AudioDeviceManager::GetRemoteAvailableDevicesByUsage(AudioDeviceUsage usage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptor)
{
    if ((usage & MEDIA_OUTPUT_DEVICES) != 0) {
        for (const auto &desc : connectedDevices_) {
            if (desc->deviceType_ == DEVICE_TYPE_REMOTE_CAST && desc->networkId_ == LOCAL_NETWORK_ID) {
                audioDeviceDescriptor.push_back(make_shared<AudioDeviceDescriptor>(*desc));
            }
        }
    }
}

VolumeBehavior AudioDeviceManager::GetDeviceVolumeBehavior(const std::string &networkId, DeviceType deviceType)
{
    AUDIO_INFO_LOG("GetDeviceVolumeBehavior: networkId [%{public}s], deviceType [%{public}d]",
        networkId.c_str(), deviceType);
    VolumeBehavior volumeBehavior;
    for (auto &desc : connectedDevices_) {
        if (desc->deviceType_ != deviceType || desc->networkId_ != networkId) {
            continue;
        }
        // Find the target device.
        if (desc->volumeBehavior_.isReady) {
            volumeBehavior.isReady = true;
            volumeBehavior.isVolumeControlDisabled = desc->volumeBehavior_.isVolumeControlDisabled;
            volumeBehavior.databaseVolumeName = desc->volumeBehavior_.databaseVolumeName;
            AUDIO_INFO_LOG("isVolumeControlDisabled [%{public}d], databaseVolumeName [%{public}s]",
                volumeBehavior.isVolumeControlDisabled, volumeBehavior.databaseVolumeName.c_str());
        } else {
            AUDIO_WARNING_LOG("The target device is found. But the isReady is false!");
        }
    }
    return volumeBehavior;
}

int32_t AudioDeviceManager::SetDeviceVolumeBehavior(const std::string &networkId, DeviceType deviceType,
    VolumeBehavior volumeBehavior)
{
    AUDIO_INFO_LOG("SetDeviceVolumeBehavior: networkId [%{public}s], deviceType [%{public}d]",
        networkId.c_str(), deviceType);
    remoteInfoNetworkId_ = networkId;
    remoteInfoDeviceType_ = deviceType;
    for (auto &desc : connectedDevices_) {
        if (desc->deviceType_ != remoteInfoDeviceType_ || desc->networkId_ != remoteInfoNetworkId_) {
            continue;
        }
        // Find the target device.
        desc->volumeBehavior_ = volumeBehavior;
        AUDIO_INFO_LOG("isVolumeControlDisabled [%{public}d], databaseVolumeName [%{public}s]",
            volumeBehavior.isVolumeControlDisabled, volumeBehavior.databaseVolumeName.c_str());
        AudioAdapterManager::GetInstance().UpdateVolumeWhenDeviceConnect(desc);
    }
    return SUCCESS;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableDevicesByUsage(AudioDeviceUsage usage)
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    std::vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    GetDefaultAvailableDevicesByUsage(usage, audioDeviceDescriptors);
    GetRemoteAvailableDevicesByUsage(usage, audioDeviceDescriptors);
    for (const auto &dev : connectedDevices_) {
        if (dev == nullptr) {
            AUDIO_INFO_LOG("dev is null from connectedDevices_");
            continue;
        }
        for (const auto &devicePrivacy : devicePrivacyMaps_) {
            list<DevicePrivacyInfo> deviceInfos = devicePrivacy.second;
            std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(*dev);
            GetAvailableDevicesWithUsage(usage, deviceInfos, desc, audioDeviceDescriptors);
        }
    }
    // If there are distributed devices, place them at a higher priority in the sorting order.
    if (remoteInfoNetworkId_ != "" && remoteInfoDeviceType_ != DEVICE_TYPE_DEFAULT) {
        ReorderAudioDevices(audioDeviceDescriptors, remoteInfoNetworkId_, remoteInfoDeviceType_);
    }
    return audioDeviceDescriptors;
}

void AudioDeviceManager::ReorderAudioDevices(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors,
    const std::string &remoteInfoNetworkId, DeviceType remoteInfoDeviceType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> nonLocalSpeakerDevices;
    for (auto &desc : audioDeviceDescriptors) {
        if (desc->deviceType_ == DEVICE_TYPE_SPEAKER && desc->networkId_ != LOCAL_NETWORK_ID) {
            nonLocalSpeakerDevices.push_back(std::move(desc));
        }
    }
    audioDeviceDescriptors.erase(std::remove_if(audioDeviceDescriptors.begin(), audioDeviceDescriptors.end(),
        [](const auto &device) {return device == nullptr;}), audioDeviceDescriptors.end());
    std::sort(nonLocalSpeakerDevices.begin(), nonLocalSpeakerDevices.end(),
        [](const auto &a, const auto &b) {return a->deviceId_ < b->deviceId_;});
    audioDeviceDescriptors.insert(audioDeviceDescriptors.end(),
        std::make_move_iterator(nonLocalSpeakerDevices.begin()),
        std::make_move_iterator(nonLocalSpeakerDevices.end()));

    if (remoteInfoNetworkId != "" && remoteInfoDeviceType == DEVICE_TYPE_REMOTE_CAST) {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> remoteCastDevices;
        for (auto &desc : audioDeviceDescriptors) {
            if (desc->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
                remoteCastDevices.push_back(std::move(desc));
            }
        }
        audioDeviceDescriptors.erase(std::remove_if(audioDeviceDescriptors.begin(), audioDeviceDescriptors.end(),
            [](const auto &device) {return device == nullptr;}), audioDeviceDescriptors.end());
        std::sort(remoteCastDevices.begin(), remoteCastDevices.end(),
            [](const auto &a, const auto &b) {return a->deviceId_ < b->deviceId_;});

        audioDeviceDescriptors.insert(audioDeviceDescriptors.end(),
            std::make_move_iterator(remoteCastDevices.begin()),
            std::make_move_iterator(remoteCastDevices.end()));
    }
}

unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> AudioDeviceManager::GetDevicePrivacyMaps()
{
    return devicePrivacyMaps_;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableBluetoothDevice(DeviceType devType,
    const std::string &macAddress)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == devType && desc->macAddress_ == macAddress) {
            audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
        }
    }
    return audioDeviceDescriptors;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetConnectedDevicesByTypesAndRole(
    const std::vector<DeviceType> &types, DeviceRole role)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    for (const auto &desc : connectedDevices_) {
        if (std::find(types.begin(), types.end(), desc->deviceType_) != types.end() &&
            desc->deviceRole_ == role &&
            desc->connectState_ != VIRTUAL_CONNECTED) {
            audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(desc));
        }
    }
    return audioDeviceDescriptors;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetConnectedDevices()
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    for (const auto &desc : connectedDevices_) {
        audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(desc));
    }
    return audioDeviceDescriptors;
}

bool AudioDeviceManager::GetScoState()
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    bool isScoStateConnect = Bluetooth::AudioHfpManager::IsAudioScoStateConnect();
    for (const auto &desc : connectedDevices_) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "Device is nullptr, continue");
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && isScoStateConnect) {
            return true;
        }
    }
    return false;
}

AudioStreamDeviceChangeReasonExt AudioDeviceManager::UpdateDevicesListInfo(
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc, const DeviceInfoUpdateCommand updateCommand)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, reason, "desc is nullptr");

    bool ret = false;
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    switch (updateCommand) {
        case CATEGORY_UPDATE:
            ret = UpdateDeviceCategory(devDesc);
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
        case USAGE_UPDATE:
            reason = UpdateDeviceUsage(devDesc);
            break;
        default:
            break;
    }
    if (!ret) {
        int32_t audioId = devDesc->deviceId_;
        AUDIO_ERR_LOG("cant update type:id %{public}d:%{public}d mac:%{public}s networkid:%{public}s in connected list",
            devDesc->deviceType_, audioId, GetEncryptStr(devDesc->macAddress_).c_str(),
            GetEncryptStr(devDesc->networkId_).c_str());
    }
    return reason;
}

bool AudioDeviceManager::UpdateDeviceCategory(const std::shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    for (auto &desc : connectedDevices_) {
        if (desc->deviceType_ == devDesc->deviceType_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->macAddress_ == devDesc->macAddress_ &&
            desc->deviceCategory_ != devDesc->deviceCategory_) {
            descForCb.push_back(std::make_shared<AudioDeviceDescriptor>(*desc));
            desc->deviceCategory_ = devDesc->deviceCategory_;
            descForCb.push_back(std::make_shared<AudioDeviceDescriptor>(*desc));
            if (devDesc->deviceCategory_ == BT_UNWEAR_HEADPHONE) {
                RemoveBtFromOtherList(devDesc);
            } else {
                // Update connectTimeStamp_ when wearing headphones that support wear detection
                desc->connectTimeStamp_ = GetCurrentTimeMS();
                AddBtToOtherList(desc);
            }
            AudioDeviceStatus::GetInstance().TriggerDeviceInfoUpdatedCallback(descForCb);
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
                desc->connectState_ != VIRTUAL_CONNECTED &&
                devDesc->connectState_ == CONNECTED) {
                // sco connected, suspend a2dp
                desc->connectState_ = SUSPEND_CONNECTED;
                updateFlag = true;
                AUDIO_WARNING_LOG("sco connectState is connected, update a2dp to suspend");
            } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
                desc->connectState_ == SUSPEND_CONNECTED &&
                (devDesc->connectState_ == DEACTIVE_CONNECTED || devDesc->connectState_ == SUSPEND_CONNECTED)) {
                // sco deactive or suspend, a2dp CONNECTED
                desc->connectState_ = CONNECTED;
                updateFlag = true;
                AUDIO_WARNING_LOG("sco connectState %{public}d, update a2dp to connected", devDesc->connectState_);
            }
        }
    }
    return updateFlag;
}

bool AudioDeviceManager::UpdateEnableState(const shared_ptr<AudioDeviceDescriptor> &devDesc)
{
    bool updateFlag = false;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == devDesc->deviceType_ &&
            desc->macAddress_ == devDesc->macAddress_ &&
            desc->networkId_ == devDesc->networkId_ &&
            desc->isEnable_ != devDesc->isEnable_) {
            descForCb.push_back(std::make_shared<AudioDeviceDescriptor>(*desc));
            desc->isEnable_ = devDesc->isEnable_;
            descForCb.push_back(std::make_shared<AudioDeviceDescriptor>(*desc));
            updateFlag = true;
            AudioDeviceStatus::GetInstance().TriggerDeviceInfoUpdatedCallback(descForCb);
        }
    }
    return updateFlag;
}

bool AudioDeviceManager::UpdateExceptionFlag(const shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    bool updateFlag = false;
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == deviceDescriptor->deviceType_ &&
            desc->macAddress_ == deviceDescriptor->macAddress_ &&
            desc->networkId_ == deviceDescriptor->networkId_ &&
            desc->exceptionFlag_ != deviceDescriptor->exceptionFlag_) {
            desc->exceptionFlag_ = deviceDescriptor->exceptionFlag_;
            updateFlag = true;
        }
    }
    return updateFlag;
}

AudioStreamDeviceChangeReasonExt AudioDeviceManager::UpdateDeviceUsage(
    const shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, reason, "device is nullptr");
    bool updateFlag = false;
    for (const auto &desc : connectedDevices_) {
        CHECK_AND_CONTINUE(desc != nullptr);
        if (desc->deviceType_ == deviceDesc->deviceType_ &&
            desc->macAddress_ == deviceDesc->macAddress_ &&
            desc->networkId_ == deviceDesc->networkId_ &&
            desc->deviceUsage_ != deviceDesc->deviceUsage_) {
            reason = (desc->deviceUsage_ > deviceDesc->deviceUsage_) ?
                AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE :
                AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
            desc->deviceUsage_ = deviceDesc->deviceUsage_;
            updateFlag = true;
        }
    }
    CHECK_AND_RETURN_RET_LOG(updateFlag == true, reason, "Update usage fails");
    return reason;
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
    RemoveMatchDeviceInArray(devDesc, "capture recognition privacy device", reconCapturePrivacyDevices_);
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetDevicesByFilter(DeviceType devType, DeviceRole devRole,
    const string &macAddress, const string &networkId, ConnectState connectState)
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
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

void AudioDeviceManager::OnReceiveUpdateDeviceNameEvent(const std::string macAddress, const std::string deviceName)
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    for (auto device : connectedDevices_) {
        if (device->macAddress_ == macAddress) {
            device->deviceName_ = deviceName;
        }
    }
}

bool AudioDeviceManager::IsDeviceConnected(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptors)
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
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

bool AudioDeviceManager::IsVirtualConnectedDevice(const std::shared_ptr<AudioDeviceDescriptor> &selectedDesc)
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

int32_t AudioDeviceManager::UpdateDeviceDescDeviceId(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
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
    AUDIO_INFO_LOG("stream %{public}u with usage %{public}d selects output device %{public}d",
        sessionID, streamUsage, deviceType);
    if (!isRunning) {
        AUDIO_WARNING_LOG("current stream has not started");
        return SUCCESS;
    }
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
            AUDIO_WARNING_LOG("media default output device changes from %{public}d to %{public}d",
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
            AUDIO_WARNING_LOG("call default output device changes from %{public}d to %{public}d",
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
        AUDIO_WARNING_LOG("changes from %{public}d to %{public}d because media stream %{public}u starts",
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
        AUDIO_WARNING_LOG("changes from %{public}d to %{public}d because call stream %{public}u starts",
            selectedCallDefaultOutputDevice_, deviceType, sessionID);
        selectedCallDefaultOutputDevice_ = deviceType;
    }
    return SUCCESS;
}

int32_t AudioDeviceManager::UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    if (!selectedDefaultOutputDeviceInfo_.count(sessionID)) {
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
        AUDIO_WARNING_LOG("changes from %{public}d to %{public}d because media stream %{public}u stops",
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
        AUDIO_WARNING_LOG("changes from %{public}d to %{public}d because call stream %{public}u stops",
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

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetSelectedMediaRenderDevice()
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = nullptr;
    if (selectedMediaDefaultOutputDevice_ == DEVICE_TYPE_EARPIECE) {
        devDesc = make_shared<AudioDeviceDescriptor>(earpiece_);
    } else if (selectedMediaDefaultOutputDevice_ == DEVICE_TYPE_SPEAKER) {
        devDesc = make_shared<AudioDeviceDescriptor>(speaker_);
    }
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetSelectedCallRenderDevice()
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = nullptr;
    if (selectedCallDefaultOutputDevice_ == DEVICE_TYPE_EARPIECE) {
        devDesc = make_shared<AudioDeviceDescriptor>(earpiece_);
    } else if (selectedCallDefaultOutputDevice_ == DEVICE_TYPE_SPEAKER) {
        devDesc = make_shared<AudioDeviceDescriptor>(speaker_);
    }
    return devDesc;
}

int32_t AudioDeviceManager::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const SourceType sourceType, bool isRunning)
{
    std::lock_guard<std::mutex> lock(selectInputDeviceMutex_);
    selectedInputDeviceInfo_[sessionID] = std::make_pair(deviceType, sourceType);
    AUDIO_INFO_LOG("stream %{public}u run %{public}d with usage %{public}d selects input device %{public}d",
        sessionID, isRunning, sourceType, deviceType);
    return NEED_TO_FETCH;
}

int32_t AudioDeviceManager::RemoveSelectedInputDevice(const uint32_t sessionID)
{
    AUDIO_INFO_LOG("AudioDeviceManager::RemoveSelectedInputDevice %{public}d", sessionID);
    std::lock_guard<std::mutex> lock(selectInputDeviceMutex_);
    selectedInputDeviceInfo_.erase(sessionID);
    return SUCCESS;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetSelectedCaptureDevice(const uint32_t sessionID)
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>();
    if (sessionID == 0 || !selectedInputDeviceInfo_.count(sessionID)) {
        return devDesc;
    }
    for (const auto &desc : connectedDevices_) {
        if (desc->deviceType_ == selectedInputDeviceInfo_[sessionID].first) {
            AUDIO_WARNING_LOG("sessionid %{public}d has selected device", sessionID);
            return make_shared<AudioDeviceDescriptor>(*desc);
        }
    }
    return devDesc;
}

int32_t AudioDeviceManager::SetPreferredInputDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &devDesc, const uint32_t sessionID, const SourceType sourceType)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(),
        ERR_PERMISSION_DENIED, "set preferred input device denied: no system permission");
    CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter");
    std::lock_guard<std::mutex> lock(preferredInputDeviceMutex_);
    preferredInputDeviceInfo_[sessionID] = std::make_pair(devDesc, sourceType);
    AUDIO_INFO_LOG("set preferred input device successful, sessionID = %{public}d, "
        "deviceType = %{public}d, sourceType = %{public}d", sessionID, devDesc->deviceType_, sourceType);
    return SUCCESS;
}

shared_ptr<AudioDeviceDescriptor> AudioDeviceManager::GetOnlinePreferredInputDevice(const uint32_t sessionID)
{
    CHECK_AND_RETURN_RET_LOG(preferredInputDeviceInfo_.count(sessionID), nullptr,
        "sessionID %{public}d: preferredInputDevice is empty", sessionID);
    shared_ptr<AudioDeviceDescriptor> preferredInputDevice = preferredInputDeviceInfo_[sessionID].first;
    shared_ptr<AudioDeviceDescriptor> onlinePreferredInputDevice = nullptr;

    DeviceType deviceType = preferredInputDevice->deviceType_;
    string mac = preferredInputDevice->macAddress_;

    if (mac.empty()) {
        AUDIO_WARNING_LOG("preferred input device has no mac address");
        for (const auto &dev : connectedDevices_) {
            if (dev->deviceType_ == deviceType) {
                onlinePreferredInputDevice = dev;
            }
        }
    } else {
        for (const auto &dev : connectedDevices_) {
            if (dev->deviceType_ == deviceType && dev->macAddress_ == mac) {
                onlinePreferredInputDevice = dev;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(onlinePreferredInputDevice != nullptr, nullptr, "cannot find online preferredInputDevice");
    CHECK_AND_RETURN_RET_LOG(onlinePreferredInputDevice->deviceRole_ == INPUT_DEVICE, nullptr, "wrong deviceRole");
    AUDIO_INFO_LOG("find online preferredInputDevice deviceType: %{public}d", onlinePreferredInputDevice->deviceType_);
    return onlinePreferredInputDevice;
}

int32_t AudioDeviceManager::RemovePreferredInputDevice(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(preferredInputDeviceMutex_);
    preferredInputDeviceInfo_.erase(sessionID);
    AUDIO_INFO_LOG("remove preferred input device for session %{public}d", sessionID);
    return SUCCESS;
}

void AudioDeviceManager::Dump(std::string &dumpString)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    AppendFormat(dumpString, " MediaDefaultOutputDevices:\n");
    for (auto it : mediaDefaultOutputDevices_) {
        AppendFormat(dumpString, "  sessionId: %u, device type: %s\n", it.first,
            AudioInfoDumpUtils::GetDeviceTypeName(it.second).c_str());
    }
    AppendFormat(dumpString, "current media default output device: %s\n",
        AudioInfoDumpUtils::GetDeviceTypeName(selectedMediaDefaultOutputDevice_).c_str());

    AppendFormat(dumpString, " CallDefaultOutputDevices:\n");
    for (auto it : callDefaultOutputDevices_) {
        AppendFormat(dumpString, "  sessionId: %u, device type: %s\n", it.first,
            AudioInfoDumpUtils::GetDeviceTypeName(it.second).c_str());
    }
    AppendFormat(dumpString, "current call default output device: %s\n",
        AudioInfoDumpUtils::GetDeviceTypeName(selectedCallDefaultOutputDevice_).c_str());
}

void AudioDeviceManager::GetAllConnectedDeviceByType(std::string networkId, DeviceType deviceType,
    std::string macAddress, DeviceRole deviceRole, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    auto isPresent =
        [&networkId, &deviceType, &macAddress, &deviceRole](const shared_ptr<AudioDeviceDescriptor> &desc) {
        return networkId == desc->networkId_ && deviceType == desc->deviceType_ &&
            macAddress == desc->macAddress_ && (!IsUsb(deviceType) || deviceRole == desc->deviceRole_);
    };
    auto it = find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    while (it != connectedDevices_.end()) {
        descForCb.push_back(*it);
        it = find_if(std::next(it), connectedDevices_.end(), isPresent);
    }
    return;
}

bool AudioDeviceManager::IsSessionSetDefaultDevice(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(selectDefaultOutputDeviceMutex_);
    return selectedDefaultOutputDeviceInfo_.find(sessionId) != selectedDefaultOutputDeviceInfo_.end();
}

bool AudioDeviceManager::ExistsByType(DeviceType devType)
{
    std::lock_guard<std::mutex> lg(currentActiveDevicesMutex_);
    auto it = find_if(connectedDevices_.cbegin(), connectedDevices_.cend(), [devType](auto &item) {
        return item->deviceType_ == devType;
    });
    return it != connectedDevices_.cend();
}

bool AudioDeviceManager::ExistsByTypeAndAddress(DeviceType devType, const string &address)
{
    std::lock_guard<std::mutex> lg(currentActiveDevicesMutex_);
    auto it = find_if(connectedDevices_.cbegin(), connectedDevices_.cend(), [devType, &address](auto &item) {
        return item->deviceType_ == devType && item->macAddress_ == address;
    });
    return it != connectedDevices_.cend();
}

bool AudioDeviceManager::ExistSameRemoteDeviceByMacAddress(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    std::lock_guard<std::mutex> currentActiveDevicesLock(currentActiveDevicesMutex_);
    std::vector<shared_ptr<AudioDeviceDescriptor>> descs;
    GetDefaultAvailableDevicesByUsage(MEDIA_OUTPUT_DEVICES, descs);
    for (const auto &ds : descs) {
        CHECK_AND_CONTINUE(ds->networkId_ != LOCAL_NETWORK_ID && !ds->macAddress_.empty());
        if (ds->macAddress_ == desc->macAddress_) {
            AUDIO_INFO_LOG("same mac remote hicar exist, skip a2dp");
            return true;
        }
    }
    return false;
}

int32_t AudioDeviceManager::GetDevicePriority(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (DeviceAttrMatch(desc, TYPE_PRIVACY, OUTPUT_DEVICE, ALL_USAGE)) {
        return PRIORITY_PRIVACY;
    }
    if (DeviceAttrMatch(desc, TYPE_PUBLIC, OUTPUT_DEVICE, ALL_USAGE)) {
        return PRIORITY_PUBLIC;
    }
    if (desc->deviceType_ == DEVICE_TYPE_SPEAKER && desc->networkId_ != LOCAL_NETWORK_ID) {
        return PRIORITY_DISTRIBUTED;
    }
    if (desc->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        return PRIORITY_DISTRIBUTED;
    }
    return PRIORITY_BASE;
}
}
}
