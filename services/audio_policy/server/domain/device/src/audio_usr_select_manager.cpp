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
#define LOG_TAG "AudioUsrSelectManager"
#endif

#include "audio_usr_select_manager.h"

#include "audio_policy_log.h"
#include "audio_recovery_device.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static const int32_t MEDIA_SERVICE_UID = 1013;
bool AudioUsrSelectManager::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
    int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(deviceDescriptor->deviceId_);
    if (desc == nullptr) {
        AUDIO_ERR_LOG("AudioUsrSelectManager::SelectInputDeviceByUid no device found, deviceId: %{public}d",
            deviceDescriptor->deviceId_);
        return false;
    }

    // If the selected device is virtual device, connect it.
    bool isVirtualDevice = AudioDeviceManager::GetAudioDeviceManager().IsVirtualConnectedDevice(desc);
    if (isVirtualDevice) {
        int32_t ret = AudioRecoveryDevice::GetInstance().ConnectVirtualDevice(desc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Connect device [%{public}s] failed",
            GetEncryptStr(desc->macAddress_).c_str());
    }

    std::pair<int32_t, AudioDevicePtr> devicePair{uid, desc};
    auto it = findDevice(uid);
    if (it != selectedDevices_.end()) {
        selectedDevices_.erase(it);
    }
    selectedDevices_.push_front(devicePair);

    return !isVirtualDevice;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetSelectedInputDeviceByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    auto it = findDevice(uid);
    if (it == selectedDevices_.end()) {
        AUDIO_ERR_LOG("AudioUsrSelectManager::GetSelectedInputDeviceByUid no selected device. uid:%{public}d", uid);
        return invalidDesc;
    }

    auto desc = AudioDeviceManager::GetAudioDeviceManager().GetExistedDevice(it->second);
    if (desc == nullptr) {
        AUDIO_ERR_LOG("AudioUsrSelectManager::GetSelectedInputDeviceByUid device does not exist. uid:%{public}d", uid);
        return invalidDesc;
    }
    return desc;
}

void AudioUsrSelectManager::ClearSelectedInputDeviceByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = findDevice(uid);
    if (it != selectedDevices_.end()) {
        selectedDevices_.erase(it);
    }
}

void AudioUsrSelectManager::PreferBluetoothAndNearlinkRecordByUid(int32_t uid, bool isPreferred)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it =
        std::find(isPreferredBluetoothAndNearlinkRecord_.begin(), isPreferredBluetoothAndNearlinkRecord_.end(), uid);
    if (it != isPreferredBluetoothAndNearlinkRecord_.end()) {
        isPreferredBluetoothAndNearlinkRecord_.erase(it);
    }
    
    if (isPreferred) {
        isPreferredBluetoothAndNearlinkRecord_.push_front(uid);
    }
}

bool AudioUsrSelectManager::GetPreferBluetoothAndNearlinkRecordByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it =
        std::find(isPreferredBluetoothAndNearlinkRecord_.begin(), isPreferredBluetoothAndNearlinkRecord_.end(), uid);
    if (it != isPreferredBluetoothAndNearlinkRecord_.end()) {
        return true;
    }
    return false;
}

void AudioUsrSelectManager::EnableSelectInputDevice(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &inputStreamDescs)
{
    std::lock_guard<std::mutex> lock(mutex_);

    isEnabled_ = true;
    std::unordered_map<int32_t, int32_t> uidMap;
    for (size_t i = 0; i < inputStreamDescs.size(); ++i) {
        auto &streamDesc = inputStreamDescs[i];
        if (streamDesc->streamStatus_ != STREAM_STATUS_STARTED) {
            continue;
        }

        int32_t uid = GetRealUid(streamDesc);
        uidMap[uid] = i;
    }

    // use selected rules first
    for (const auto &device : selectedDevices_) {
        CHECK_AND_CONTINUE(uidMap.find(device.first) != uidMap.end());
        auto desc = device.second;
        
        int32_t index = uidMap[device.first];
        auto &streamDesc = inputStreamDescs[index];
        capturerDevice_ = JudgeFinalSelectDevice(desc, streamDesc->capturerInfo_.sourceType);
        CHECK_AND_CONTINUE(capturerDevice_ != nullptr);
        return;
    }

    if (isPreferredBluetoothAndNearlinkRecord_.size() == 0) {
        AUDIO_WARNING_LOG("AudioUsrSelectManager::EnableSelectInputDevice no prefer settings");
        return;
    }

    // then use prefer rules
    // According to the device connection time, obtain the most recently connected Bluetooth/Nearlink device
    auto preferDevice = GetPreferDevice();
    CHECK_AND_RETURN(preferDevice != nullptr);
    for (const int32_t uid : isPreferredBluetoothAndNearlinkRecord_) {
        CHECK_AND_CONTINUE(uidMap.find(uid) != uidMap.end());

        int32_t index = uidMap[uid];
        auto &streamDesc = inputStreamDescs[index];
        capturerDevice_ = JudgeFinalSelectDevice(preferDevice, streamDesc->capturerInfo_.sourceType);
        CHECK_AND_CONTINUE(capturerDevice_ != nullptr);
        break;
    }
}

void AudioUsrSelectManager::DisableSelectInputDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    isEnabled_ = false;
    capturerDevice_ = nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetCapturerDevice(int32_t uid, SourceType sourceType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // If marked, directly return the selection result.
    CHECK_AND_RETURN_RET(!isEnabled_, capturerDevice_);

    std::shared_ptr<AudioDeviceDescriptor> capturerDevice = nullptr;
    // If not marked, first find the selection of the current UID
    auto deviceIt = findDevice(uid);
    if (deviceIt != selectedDevices_.end()) {
        // Based on the sourceType, determine the final input device
        auto desc = JudgeFinalSelectDevice(deviceIt->second, sourceType);
        CHECK_AND_RETURN_RET_LOG(desc == nullptr, desc,
            "AudioUsrSelectManager::GetCapturerDevice has selected device.");
    }

    // If the current UID has no selection, then apply the preference setting
    auto it =
        std::find(isPreferredBluetoothAndNearlinkRecord_.begin(), isPreferredBluetoothAndNearlinkRecord_.end(), uid);
    // If the current UID has no preference setting
    CHECK_AND_RETURN_RET_LOG(it != isPreferredBluetoothAndNearlinkRecord_.end(), nullptr,
        "AudioUsrSelectManager::GetCapturerDevice no prefer data");
    // According to the device connection time, obtain the most recently connected Bluetooth/Nearlink device
    auto preferDevice = GetPreferDevice();
    CHECK_AND_RETURN_RET(preferDevice != nullptr, nullptr);
    return JudgeFinalSelectDevice(preferDevice, sourceType);
}

std::list<std::pair<int32_t, AudioDevicePtr>>::iterator AudioUsrSelectManager::findDevice(int32_t uid)
{
    return std::find_if(selectedDevices_.begin(), selectedDevices_.end(),
        [uid](const std::pair<int32_t, AudioDevicePtr>& device) {
        return device.first == uid;
    });
}

int32_t AudioUsrSelectManager::GetRealUid(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamDesc->callerUid_ == MEDIA_SERVICE_UID) {
        return streamDesc->appInfo_.appUid;
    }
    return streamDesc->callerUid_;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::JudgeFinalSelectDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &desc, SourceType sourceType)
{
    // 如果是直播或录像且设备为sco，需要判断是否存在可用的高清设备
    if ((sourceType == SOURCE_TYPE_CAMCORDER || sourceType == SOURCE_TYPE_LIVE) &&
        desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        auto a2dpin = std::make_shared<AudioDeviceDescriptor>(desc);
        a2dpin->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
        bool isA2dpinConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(a2dpin);
        CHECK_AND_RETURN_RET(!isA2dpinConnected, a2dpin);
    }

    // 判断设备是不是存在且处于连接状态
    bool isConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(desc);
    return isConnected ? desc : nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetPreferDevice()
{
    std::vector<DeviceType> types = {
        DEVICE_TYPE_NEARLINK,
        DEVICE_TYPE_BLUETOOTH_SCO,
    };
    auto audioDeviceDescriptors =
        AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevicesByTypesAndRole(types, INPUT_DEVICE);
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() > 0, nullptr, "no bluetooth or nearlink devices");
    std::sort(audioDeviceDescriptors.begin(), audioDeviceDescriptors.end(), [](const auto &desc1, const auto desc2) {
        return desc1->connectTimeStamp_ < desc2->connectTimeStamp_;
    });
    return audioDeviceDescriptors.back();
}
} // namespace AudioStandard
} // namespace OHOS
