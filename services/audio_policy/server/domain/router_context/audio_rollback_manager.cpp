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
#define LOG_TAG "AudioRollbackManager"
#endif

#include <thread>
#include "audio_rollback_manager.h"
#include "audio_policy_log.h"
#include "audio_device_manager.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"
#include "audio_active_device.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr int32_t CONTINUE_TIMEOUT_SEC = 2;
constexpr int32_t ROLLBACK_TIMEOUT_SEC = 1;
const set<ConnectState> NOT_CONNECTED{SUSPEND_CONNECTED, VIRTUAL_CONNECTED, DEACTIVE_CONNECTED};

AudioRollbackManager& AudioRollbackManager::GetInstance()
{
    static AudioRollbackManager instance;
    return instance;
}

void AudioRollbackManager::OnDeviceNewConnect(const AudioDeviceDescriptor &desc)
{
    CHECK_AND_RETURN(NOT_CONNECTED.contains(desc.connectState_) && desc.deviceRole_ == OUTPUT_DEVICE);
    auto deviceId = desc.deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d, connectState=%{public}d",
        deviceId, desc.deviceType_, desc.connectState_);
    std::lock_guard lg(mutex_);
    selectionRollbacks_.clear();
    if (!rollbackInfo_) {
        rollbackInfo_ = std::make_unique<RollbackInfo>();
    }
    rollbackInfo_->type = NEW_CONNECT_ROLLBACK;
    rollbackInfo_->deviceId = deviceId;
    rollbackInfo_->selections = AudioRouterInfra::GetInstance().GetSelections(
        {SelectDeviceType::CALL_OUTPUT, SelectDeviceType::MEDIA_OUTPUT});
    thread([deviceId, this]() {
        this_thread::sleep_for(chrono::seconds(CONTINUE_TIMEOUT_SEC));
        RollbackSelections(deviceId, NEW_CONNECT_ROLLBACK);
    }).detach();
}

void AudioRollbackManager::SetConnectLostRollback(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(device && device->connectState_ == CONNECTED);
    auto deviceId = device->deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d", deviceId, device->deviceType_);
    std::lock_guard lg(mutex_);
    selectionRollbacks_.clear();
    if (!rollbackInfo_) {
        rollbackInfo_ = std::make_unique<RollbackInfo>();
    }
    if (rollbackInfo_->deviceId == deviceId) {
        CHECK_AND_RETURN(rollbackInfo_->type != CONNECT_LOST_ROLLBACK);
    } else {
        rollbackInfo_->deviceId = deviceId;
        rollbackInfo_->selections = AudioRouterInfra::GetInstance().GetSelections(
            {SelectDeviceType::CALL_OUTPUT, SelectDeviceType::MEDIA_OUTPUT});
    }
    rollbackInfo_->type = CONNECT_LOST_ROLLBACK;
    thread([deviceId, this]() {
        this_thread::sleep_for(chrono::seconds(ROLLBACK_TIMEOUT_SEC));
        std::lock_guard lg(mutex_);
        if (rollbackInfo_ && rollbackInfo_->deviceId == deviceId && rollbackInfo_->type == CONNECT_LOST_ROLLBACK) {
            rollbackInfo_.reset();
        }
    }).detach();
}

void AudioRollbackManager::RollbackSelections(int32_t deviceId, RollbackType type)
{
    std::lock_guard lg(mutex_);
    CHECK_AND_RETURN(rollbackInfo_ && rollbackInfo_->deviceId == deviceId && rollbackInfo_->type == type);
    AUDIO_INFO_LOG("id=%{public}d, rollbackType=%{public}d", deviceId, type);
    for (auto &item : rollbackInfo_->selections) {
        RollbackSelection(item);
    }
    rollbackInfo_.reset();
}

void AudioRollbackManager::OnConnectStateUpdate(
    const AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    CHECK_AND_RETURN(command == CONNECTSTATE_UPDATE);
    auto devices = AudioDeviceManager::GetAudioDeviceManager()
        .GetAvailableBluetoothDevice(desc.deviceType_, desc.macAddress_);
    CHECK_AND_RETURN_LOG(!devices.empty(), "Absent desc");
    auto it = find_if(devices.begin(), devices.end(),
        [](auto &item) { return item && item->deviceRole_ == OUTPUT_DEVICE; });
    CHECK_AND_RETURN(it != devices.end() && *it);
    auto &device = *it;
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d, role=%{public}d, connectState=%{public}d",
        device->deviceId_, device->deviceType_, device->deviceRole_, device->connectState_);
    if (device->connectState_ == CONNECTED) {
        SetConnectLostRollback(device);
    } else {
        RollbackSelections(device->deviceId_, CONNECT_LOST_ROLLBACK);
    }
}

void AudioRollbackManager::OnDeviceDisconnect(const AudioDeviceDescriptor &desc)
{
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d, connectState=%{public}d",
        desc.deviceId_, desc.deviceType_, desc.connectState_);
    auto deviceId = desc.deviceId_;
    auto pred = [deviceId] (SelectionRollback &item) {
        return item.targetId == deviceId;
    };
    std::lock_guard lg(mutex_);
    auto rmIt = remove_if(selectionRollbacks_.begin(), selectionRollbacks_.end(), pred);
    for (auto it = rmIt; it < selectionRollbacks_.end(); it++) {
        RollbackSelection(it->selection);
    }
    selectionRollbacks_.erase(rmIt, selectionRollbacks_.end());

    CHECK_AND_RETURN(rollbackInfo_ && rollbackInfo_->deviceId == deviceId &&
        rollbackInfo_->type == CONNECT_LOST_ROLLBACK);
    for (auto &item : rollbackInfo_->selections) {
        RollbackSelection(item);
    }
    rollbackInfo_.reset();
}

void AudioRollbackManager::SetMediaInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(uid, streamId, device);
}

void AudioRollbackManager::SetMediaOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    SetSelectionRollback(uid, SelectDeviceType::MEDIA_OUTPUT, streamId, device);
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(uid, streamId, device);
}

void AudioRollbackManager::SetCallInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(uid, streamId, device);
}

void AudioRollbackManager::SetCallOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller)
{
    SetSelectionRollback(uid, SelectDeviceType::CALL_OUTPUT, streamId, device);
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(uid, streamId, device, caller);
}

void AudioRollbackManager::SetSelectionRollback(int32_t uid, SelectDeviceType type,
    uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(device && device->deviceId_ > 0);
    if (uid < SYSTEM_UID) {
        uid = SYSTEM_UID;
    }
    auto deviceId = device->deviceId_;
    auto pred = [uid, type, streamId] (SelectionRollback &item) {
        return item.selection.uid == uid && item.selection.type == type && item.selection.streamId == streamId;
    };
    std::lock_guard lg(mutex_);
    rollbackInfo_.reset();
    selectionRollbacks_.erase(remove_if(
        selectionRollbacks_.begin(), selectionRollbacks_.end(), pred), selectionRollbacks_.end());
    CHECK_AND_RETURN(NOT_CONNECTED.contains(device->connectState_));
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d, connectState=%{public}d",
        deviceId, device->deviceType_, device->connectState_);
    for (auto &item : AudioRouterInfra::GetInstance().GetSelections({type})) {
        CHECK_AND_CONTINUE(item.uid == uid && item.streamId == streamId && item.deviceId != deviceId);
        selectionRollbacks_.push_back({deviceId, item});
    }
    thread([deviceId, uid, type, streamId, this]() {
        this_thread::sleep_for(chrono::seconds(CONTINUE_TIMEOUT_SEC));
        auto pred = [deviceId, uid, type, streamId] (SelectionRollback &item) {
            return item.targetId == deviceId && item.selection.uid == uid &&
                item.selection.type == type && item.selection.streamId == streamId;
        };
        std::lock_guard lg(mutex_);
        auto rmIt = remove_if(selectionRollbacks_.begin(), selectionRollbacks_.end(), pred);
        for (auto it = rmIt; it < selectionRollbacks_.end(); it++) {
            RollbackSelection(it->selection);
        }
        selectionRollbacks_.erase(rmIt, selectionRollbacks_.end());
    }).detach();
}

void AudioRollbackManager::RollbackSelection(const Selection &selection)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto device = deviceManager.FindConnectedDeviceById(selection.deviceId);
    CHECK_AND_RETURN_LOG(device && (device->connectState_ == CONNECTED ||
        device->connectState_ == DEACTIVE_CONNECTED), "NotConnect[%{public}d]", selection.deviceId);
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d, connectState=%{public}d",
        selection.deviceId, device->deviceType_, device->connectState_);
    switch (selection.type) {
        case SelectDeviceType::MEDIA_INPUT:
            strategy.SetMediaInputDevice(selection.uid, selection.streamId, device);
            break;
        case SelectDeviceType::MEDIA_OUTPUT:
            strategy.SetMediaOutputDevice(selection.uid, selection.streamId, device);
            break;
        case SelectDeviceType::CALL_INPUT:
            strategy.SetCallInputDevice(selection.uid, selection.streamId, device);
            break;
        case SelectDeviceType::CALL_OUTPUT:
            strategy.SetCallOutputDevice(selection.uid, selection.streamId, device);
            break;
        default:
            AUDIO_ERR_LOG("Error: invalid type");
    }
    if (device->connectState_ == DEACTIVE_CONNECTED) {
        AudioActiveDevice::GetInstance().NotifyUserSelectionEventToBt(device);
    }
}

}
}
