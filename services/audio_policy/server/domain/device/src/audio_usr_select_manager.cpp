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

    RecordDeviceInfo info {.uid_ = uid, .selectedDevice_ = desc};
    int index = GetIdFromRecordDeviceInfoList(uid);
    UpdateRecordDeviceInfoForSelectInner(index, info);
    return !isVirtualDevice;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetSelectedInputDeviceByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t index = GetIdFromRecordDeviceInfoList(uid);
    return index > -1 ? recordDeviceInfoList_[index].selectedDevice_ : invalidDesc;
}

BluetoothAndNearlinkPreferredRecordCategory AudioUsrSelectManager::GetPreferBluetoothAndNearlinkRecordByUid(
    int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int index = GetIdFromRecordDeviceInfoList(uid);
    CHECK_AND_RETURN_RET(index >= 0, BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE);
    return recordDeviceInfoList_[index].appPreferredCategory_;
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetCapturerDevice(int32_t uid, SourceType sourceType)
{
    std::lock_guard<std::mutex> lock(mutex_);

    int index = GetIdFromRecordDeviceInfoList(uid);
    std::shared_ptr<AudioDeviceDescriptor> capturerDevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN_RET(index >= 0 && !recordDeviceInfoList_.empty(), capturerDevice);
    CHECK_AND_RETURN_RET(recordDeviceInfoList_[0].sourceType_ != SourceType::SOURCE_TYPE_INVALID,
        recordDeviceInfoList_[index].activeSelectedDevice_);

    std::shared_ptr<AudioDeviceDescriptor> appPreferredDevice = GetPreferDevice();
    capturerDevice = recordDeviceInfoList_[0].activeSelectedDevice_->deviceType_ == DEVICE_TYPE_NONE ?
        appPreferredDevice : recordDeviceInfoList_[0].activeSelectedDevice_;
    return JudgeFinalSelectDevice(capturerDevice, sourceType, recordDeviceInfoList_[0].appPreferredCategory_);
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
    CHECK_AND_RETURN_RET(recordDeviceInfoList_[0].appPreferredCategory_ !=
        BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE, std::make_shared<AudioDeviceDescriptor>());
    std::vector<DeviceType> types = {
        DEVICE_TYPE_NEARLINK,
        DEVICE_TYPE_BLUETOOTH_SCO,
    };
    auto audioDeviceDescriptors =
        AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevicesByTypesAndRole(types, INPUT_DEVICE);
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() > 0,
        std::make_shared<AudioDeviceDescriptor>(), "no bluetooth or nearlink devices");
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

void AudioUsrSelectManager::UpdateRecordDeviceInfo(UpdateType updateType, RecordDeviceInfo info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t index = GetIdFromRecordDeviceInfoList(info.uid_);
    AUDIO_INFO_LOG("UpdateRecordDeviceInfo updateType:%{public}d", updateType);
    switch (updateType) {
        case UpdateType::START_CLIENT:
            UpdateRecordDeviceInfoForStartInner(index, info);
            break;
        case UpdateType::APP_SELECT:
            UpdateRecordDeviceInfoForSelectInner(index, info);
            break;
        case UpdateType::SYSTEM_SELECT:
            if (info.activeSelectedDevice_->deviceType_ != DEVICE_TYPE_NONE) {
                for (auto &recordDeviceInfo : recordDeviceInfoList_) {
                    recordDeviceInfo.activeSelectedDevice_ = info.activeSelectedDevice_;
                }
            } else {
                for (auto &recordDeviceInfo : recordDeviceInfoList_) {
                    recordDeviceInfo.activeSelectedDevice_ = recordDeviceInfo.selectedDevice_;
                }
            }
            break;
        case UpdateType::APP_PREFER:
            UpdateRecordDeviceInfoForPreferInner(index, info);
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

void AudioUsrSelectManager::UpdateRecordDeviceInfoForStartInner(int32_t index, RecordDeviceInfo info)
{
    if (index < 0) {
        RecordDeviceInfo recordDeviceInfo {
            .uid_ = info.uid_,
            .sourceType_ = info.sourceType_,
            .activeSelectedDevice_ = info.activeSelectedDevice_
        };
        recordDeviceInfoList_.emplace(recordDeviceInfoList_.begin(), recordDeviceInfo);
    } else {
        recordDeviceInfoList_[index].sourceType_ = info.sourceType_;
        recordDeviceInfoList_[index].activeSelectedDevice_ = info.activeSelectedDevice_->deviceType_ !=
            DEVICE_TYPE_NONE ? info.activeSelectedDevice_ : recordDeviceInfoList_[index].selectedDevice_;
        if (index > 0 && index < static_cast<int32_t>(recordDeviceInfoList_.size())) {
            std::rotate(recordDeviceInfoList_.begin(), recordDeviceInfoList_.begin() + index,
                recordDeviceInfoList_.begin() + index + 1);
        }
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForSelectInner(int32_t index, RecordDeviceInfo info)
{
    if (index < 0) {
        if (info.selectedDevice_->deviceType_ != DEVICE_TYPE_NONE) {
            RecordDeviceInfo recordDeviceInfo {
                .uid_ = info.uid_,
                .selectedDevice_ = info.selectedDevice_,
                .activeSelectedDevice_ = std::make_shared<AudioDeviceDescriptor>()
            };
            recordDeviceInfoList_.push_back(recordDeviceInfo);
        }
    } else {
        recordDeviceInfoList_[index].selectedDevice_ = info.selectedDevice_;
        recordDeviceInfoList_[index].activeSelectedDevice_ = info.selectedDevice_;
        if (recordDeviceInfoList_[index].sourceType_ != SourceType::SOURCE_TYPE_INVALID &&
            appIsBackStatesMap_.find(info.uid_) != appIsBackStatesMap_.end() &&
            appIsBackStatesMap_[info.uid_] == AppIsBackState::STATE_FOREGROUND) {
            if (index > 0 && index < static_cast<int32_t>(recordDeviceInfoList_.size())) {
                std::rotate(recordDeviceInfoList_.begin(), recordDeviceInfoList_.begin() + index,
                    recordDeviceInfoList_.begin() + index + 1);
            }
        }
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForPreferInner(int32_t index, RecordDeviceInfo info)
{
    if (index < 0) {
        RecordDeviceInfo recordDeviceInfo {
            .uid_ = info.uid_,
            .appPreferredCategory_ = info.appPreferredCategory_
        };
        recordDeviceInfoList_.push_back(recordDeviceInfo);
    } else {
        recordDeviceInfoList_[index].appPreferredCategory_ = info.appPreferredCategory_;
    }
}

void AudioUsrSelectManager::UpdateRecordDeviceInfoForStopInner(int32_t index)
{
    if (index >= 0) {
        if (recordDeviceInfoList_[index].appPreferredCategory_ !=
                BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE ||
            recordDeviceInfoList_[index].selectedDevice_->deviceType_ != DEVICE_TYPE_NONE) {
            recordDeviceInfoList_[index].sourceType_ = SourceType::SOURCE_TYPE_INVALID;
            if (index >= 0 && index < static_cast<int32_t>(recordDeviceInfoList_.size()) - 1) {
                std::rotate(recordDeviceInfoList_.begin() + index, recordDeviceInfoList_.begin() + index + 1,
                    recordDeviceInfoList_.end());
            }
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
