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

    AUDIO_INFO_LOG("select input device, uid: %{public}d, deviceId: %{public}d", uid, deviceDescriptor->deviceId_);
    auto desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(deviceDescriptor->deviceId_);
    if (desc == nullptr) {
        AUDIO_ERR_LOG("AudioUsrSelectManager::SelectInputDeviceByUid no device found, deviceId: %{public}d",
            deviceDescriptor->deviceId_);
        return false;
    }

    // If the selected device is virtual device, connect it.
    bool isVirtualDevice = AudioDeviceManager::GetAudioDeviceManager().IsVirtualConnectedDevice(desc);
    if (isVirtualDevice) {
        AUDIO_INFO_LOG("is a virtual device, deviceId: %{public}d", desc->deviceId_);
        int32_t ret = AudioRecoveryDevice::GetInstance().ConnectVirtualDevice(desc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Connect device [%{public}s] failed",
            GetEncryptStr(desc->macAddress_).c_str());
    }

    UpdateRecordDeviceInfo(UpdateType::APP_SELECT, uid, -1, SourceType::SOURCE_TYPE_INVALID, desc);
    return !isVirtualDevice;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetSelectedInputDeviceByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t index = GetIdFromRecordDeviceInfoList(uid);
    return index > -1 ? recordDeviceInfoList_[index].selectedDevice_ : invalidDesc;
}

void AudioUsrSelectManager::PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("prefer to use bluetooth and nearlink to record, uid: %{public}d, category: %{public}d",
        uid, category);
    auto it =
        std::find(isPreferredBluetoothAndNearlinkRecord_.begin(), isPreferredBluetoothAndNearlinkRecord_.end(), uid);
    if (it != isPreferredBluetoothAndNearlinkRecord_.end()) {
        isPreferredBluetoothAndNearlinkRecord_.erase(it);
        categoryMap_.erase(uid);
    }

    if (category != BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE) {
        isPreferredBluetoothAndNearlinkRecord_.push_front(uid);
        categoryMap_[uid] = category;
    }
}

BluetoothAndNearlinkPreferredRecordCategory AudioUsrSelectManager::GetPreferBluetoothAndNearlinkRecordByUid(
    int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it =
        std::find(isPreferredBluetoothAndNearlinkRecord_.begin(), isPreferredBluetoothAndNearlinkRecord_.end(), uid);
    if (it != isPreferredBluetoothAndNearlinkRecord_.end()) {
        return categoryMap_[uid];
    }
    return BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetCapturerDevice(
    int32_t uid, int32_t sessionId, SourceType sourceType)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::shared_ptr<AudioDeviceDescriptor> capturerDevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN_RET(!recordDeviceInfoList_.empty(), capturerDevice);
    std::shared_ptr<AudioDeviceDescriptor> appPreferredDevice = std::make_shared<AudioDeviceDescriptor>();
    for (auto preferredDevice : recordDeviceInfoList_[0].appPreferredDevices_) {
        auto it = preferredDevice.find(sessionId);
        if (it != preferredDevice.end()) {
            appPreferredDevice = it->second;
        }
    }

    capturerDevice = recordDeviceInfoList_[0].activeSelectedDevice_->deviceType_ == DEVICE_TYPE_NONE ?
        appPreferredDevice : recordDeviceInfoList_[0].activeSelectedDevice_;
    return JudgeFinalSelectDevice(capturerDevice, sourceType, categoryMap_[uid]);
}

std::list<std::pair<int32_t, AudioDevicePtr>>::iterator AudioUsrSelectManager::findDevice(int32_t uid)
{
    return std::find_if(selectedDevices_.begin(), selectedDevices_.end(),
        [uid](const std::pair<int32_t, AudioDevicePtr>& device) {
        return device.first == uid;
    });
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::JudgeFinalSelectDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &desc, SourceType sourceType,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    // 判断设备是不是存在且处于连接状态
    bool isConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(desc);

    if (desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO || category == PREFERRED_LOW_LATENCY) {
        return isConnected ? desc : std::make_shared<AudioDeviceDescriptor>();
    }

    // 如果是直播或录像且设备为sco，需要判断是否存在可用的高清设备
    if (sourceType == SOURCE_TYPE_CAMCORDER || sourceType == SOURCE_TYPE_LIVE || category == PREFERRED_HIGH_QUALITY) {
        auto a2dpin = std::make_shared<AudioDeviceDescriptor>(desc);
        a2dpin->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
        bool isA2dpinConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(a2dpin);
        CHECK_AND_RETURN_RET(!isA2dpinConnected, a2dpin);
    }

    return isConnected ? desc : std::make_shared<AudioDeviceDescriptor>();
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

int32_t AudioUsrSelectManager::GetIdFromRecordDeviceInfoList(int32_t uid)
{
    int32_t index = 0;
    for (auto recordDeviceInfo : recordDeviceInfoList_) {
        if (recordDeviceInfo.uid_ == uid) {
            return index;
        }
        index++;
    }
    return -1;
}

void AudioUsrSelectManager::UpdateRecordDeviceInfo(UpdateType updateType, int32_t uid, int32_t sessionId,
    SourceType sourceType, const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t index = GetIdFromRecordDeviceInfoList(uid);
    AUDIO_INFO_LOG("UpdateRecordDeviceInfo updateType:%{public}d", updateType);
    switch (updateType) {
        case UpdateType::START_CLIENT:
            UpdateRecordDeviceInfoForStartInner(index, uid, sourceType, sessionId, desc);
            break;
        case UpdateType::APP_SELECT:
            UpdateRecordDeviceInfoForSelectInner(index, uid, desc);
            break;
        case UpdateType::SYSTEM_SELECT:
            if (desc->deviceType_ != DEVICE_TYPE_NONE) {
                for (auto &recordDeviceInfo : recordDeviceInfoList_) {
                    recordDeviceInfo.activeSelectedDevice_ = desc;
                }
            } else {
                for (auto &recordDeviceInfo : recordDeviceInfoList_) {
                    recordDeviceInfo.activeSelectedDevice_ = recordDeviceInfo.selectedDevice_;
                }
            }
            break;
        case UpdateType::APP_PREFER:
            UpdateRecordDeviceInfoForPreferInner(index, uid, sessionId, desc);
            break;
        case UpdateType::STOP_CLIENT:
            UpdateRecordDeviceInfoForStopInner(index);
            break;
        case UpdateType::RELEASE_CLIENT:
            if (index >= 0) {
                recordDeviceInfoList_.erase(recordDeviceInfoList_.begin() + index);
            }
            break;
        default:
            return;
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForStartInner(int32_t index, int32_t uid, int32_t sessionId,
    SourceType sourceType, const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (index < 0) {
        RecordDeviceInfo recordDeviceInfo {
            .uid_ = uid,
            .sourceType_ = sourceType,
            .activeSelectedDevice_ =
                AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice(),
            .appPreferredDevices_ = {{{sessionId, desc}}}
        };
        recordDeviceInfoList_.emplace(recordDeviceInfoList_.begin(), recordDeviceInfo);
    } else {
        for (auto &recordInfo : recordDeviceInfoList_) {
            if (recordInfo.uid_ == uid) {
                recordInfo.sourceType_ = sourceType;
            }
        }
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForSelectInner(int32_t index, int32_t uid,
    const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (index < 0) {
        if (desc->deviceType_ != DEVICE_TYPE_NONE) {
            RecordDeviceInfo recordDeviceInfo {
                .uid_ = uid,
                .sourceType_ = SourceType::SOURCE_TYPE_INVALID,
                .selectedDevice_ = desc,
                .activeSelectedDevice_ = std::make_shared<AudioDeviceDescriptor>()
            };
            recordDeviceInfoList_.push_back(recordDeviceInfo);
        }
    } else {
        recordDeviceInfoList_[index].selectedDevice_ = desc;
        recordDeviceInfoList_[index].activeSelectedDevice_ = desc;
        if (recordDeviceInfoList_[index].sourceType_ != SourceType::SOURCE_TYPE_INVALID &&
            appIsBackStatesMap_.find(uid) != appIsBackStatesMap_.end() &&
            appIsBackStatesMap_[uid] == AppIsBackState::STATE_FOREGROUND) {
            std::rotate(recordDeviceInfoList_.begin(), recordDeviceInfoList_.begin() + index,
                recordDeviceInfoList_.begin() + index + 1);
        }
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForPreferInner(int32_t index, int32_t uid, int32_t sessionId,
    const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (index < 0) {
        RecordDeviceInfo recordDeviceInfo {
            .uid_ = uid,
            .appPreferredDevices_ = {{{sessionId, desc}}}
        };
        recordDeviceInfoList_.push_back(recordDeviceInfo);
    } else {
        for (auto &appPreferredDevices : recordDeviceInfoList_[index].appPreferredDevices_) {
            auto it = appPreferredDevices.find(sessionId);
            if (it != appPreferredDevices.end()) {
                it->second = desc;
            }
        }
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForStopInner(int32_t index)
{
    if (index >= 0) {
        if (recordDeviceInfoList_[index].appPreferredDevices_.size() > 0 ||
            recordDeviceInfoList_[index].selectedDevice_->deviceType_ != DEVICE_TYPE_NONE) {
            recordDeviceInfoList_[index].sourceType_ = SourceType::SOURCE_TYPE_INVALID;
            std::rotate(recordDeviceInfoList_.begin() + index, recordDeviceInfoList_.begin() + index + 1,
                recordDeviceInfoList_.end());
        } else {
            recordDeviceInfoList_.erase(recordDeviceInfoList_.begin() + index);
        }
    }
}

void AudioUsrSelectManager::UpdateAppIsBackState(int32_t uid, AppIsBackState appState)
{
    std::lock_guard<std::mutex> lock(mutex_);
    switch (appState) {
        case AppIsBackState::STATE_END:
            appIsBackStatesMap_.erase(uid);
        case AppIsBackState::STATE_FOREGROUND:
        case AppIsBackState::STATE_BACKGROUND:
            appIsBackStatesMap_[uid] = appState;
            break;
        default:
            return;
    }
}
} // namespace AudioStandard
} // namespace OHOS
