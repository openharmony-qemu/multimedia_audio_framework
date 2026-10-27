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

#include <chrono>
#include "audio_rollback_manager.h"
#include "audio_policy_log.h"
#include "audio_device_common.h"
#include "audio_device_manager.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr uint32_t CONTINUE_TIMEOUT_MS = 2000;
constexpr uint32_t ROLLBACK_TIMEOUT_MS = 1000;

static bool IsConnected(const AudioDeviceDescriptor &desc)
{
    return desc.isEnable_ && desc.connectState_ == CONNECTED;
}

static bool IsNearlinkBtOutput(const AudioDeviceDescriptor &desc)
{
    return desc.deviceType_ == DEVICE_TYPE_NEARLINK || desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
        (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc.deviceRole_ == OUTPUT_DEVICE);
}

AudioRollbackManager& AudioRollbackManager::GetInstance()
{
    static AudioRollbackManager instance;
    return instance;
}

void AudioRollbackManager::OnDeviceNewConnect(const AudioDeviceDescriptor &desc)
{
    CHECK_AND_RETURN(IsNearlinkBtOutput(desc));
    auto deviceId = desc.deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, type=%{public}d, role=%{public}d, connState=%{public}d, enable=%{public}d",
        deviceId, desc.deviceType_, desc.deviceRole_, desc.connectState_, desc.isEnable_);
    std::lock_guard lg(mutex_);
    if (IsConnected(desc)) {
        connectedSet_.insert(deviceId);
        SetConnectLostRollback(desc);
        return;
    }
    selectionRollbacks_.clear();
    if (!rollbackInfo_) {
        rollbackInfo_ = std::make_unique<RollbackInfo>();
    }
    rollbackInfo_->type = NEW_CONNECT_ROLLBACK;
    rollbackInfo_->deviceId = deviceId;
    rollbackInfo_->selections = AudioRouterInfra::GetInstance().GetSelections(
        {SelectDeviceType::CALL_OUTPUT, SelectDeviceType::MEDIA_OUTPUT});
    CHECK_AND_CALL_FUNC_RETURN(!rollbackInfo_->selections.empty(), rollbackInfo_.reset());
    timer_.SetTimeout(CONTINUE_TIMEOUT_MS, [deviceId, this]() {
        CHECK_AND_RETURN(RollbackNewConnect(deviceId, NEW_CONNECT_ROLLBACK));
        AudioDeviceCommon::FetchAndRoute("RollbackNewConnect");
    });
}

void AudioRollbackManager::SetConnectLostRollback(const AudioDeviceDescriptor &desc)
{
    auto deviceId = desc.deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, type=%{public}d", deviceId, desc.deviceType_);
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
    CHECK_AND_CALL_FUNC_RETURN(!rollbackInfo_->selections.empty(), rollbackInfo_.reset());
    timer_.SetTimeout(ROLLBACK_TIMEOUT_MS, [deviceId, this]() {
        std::lock_guard lg(mutex_);
        if (rollbackInfo_ && rollbackInfo_->deviceId == deviceId && rollbackInfo_->type == CONNECT_LOST_ROLLBACK) {
            rollbackInfo_.reset();
        }
    });
}

void AudioRollbackManager::ContinueSetDevice(const AudioDeviceDescriptor &desc)
{
    auto deviceId = desc.deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, type=%{public}d", deviceId, desc.deviceType_);
    auto pred = [deviceId] (Selection &item) {
        return item.deviceId == deviceId;
    };
    auto rmIt = remove_if(virtualSelectDevices_.begin(), virtualSelectDevices_.end(), pred);
    for (auto it = rmIt; it < virtualSelectDevices_.end(); it++) {
        RollbackSelection(*it);
    }
    virtualSelectDevices_.erase(rmIt, virtualSelectDevices_.end());
}

bool AudioRollbackManager::RollbackNewConnect(int32_t deviceId, RollbackType type)
{
    std::lock_guard lg(mutex_);
    CHECK_AND_RETURN_RET(rollbackInfo_ && rollbackInfo_->deviceId == deviceId && rollbackInfo_->type == type, false);
    AUDIO_INFO_LOG("id=%{public}d, rbType=%{public}d", deviceId, type);
    auto result = false;
    for (auto &item : rollbackInfo_->selections) {
        result = RollbackSelection(item) || result;
    }
    rollbackInfo_.reset();
    return result;
}

bool AudioRollbackManager::RollbackSetDevice(int32_t uid, SelectDeviceType type, uint32_t streamId, int32_t deviceId)
{
    bool result = false;
    auto pred = [deviceId, uid, type, streamId] (SelectionRollback &item) {
        return item.targetId == deviceId && item.selection.uid == uid &&
            item.selection.type == type && item.selection.streamId == streamId;
    };
    std::lock_guard lg(mutex_);
    auto rmIt = remove_if(selectionRollbacks_.begin(), selectionRollbacks_.end(), pred);
    for (auto it = rmIt; it < selectionRollbacks_.end(); it++) {
        result = RollbackSelection(it->selection) || result;
    }
    selectionRollbacks_.erase(rmIt, selectionRollbacks_.end());
    return result;
}

void AudioRollbackManager::OnConnectStateUpdate(
    const AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    CHECK_AND_RETURN(command == CONNECTSTATE_UPDATE || command == ENABLE_UPDATE);
    auto devices = AudioDeviceManager::GetAudioDeviceManager()
        .GetAvailableBluetoothDevice(desc.deviceType_, desc.macAddress_);
    CHECK_AND_RETURN_LOG(!devices.empty(), "Absent desc");
    auto it = find_if(devices.begin(), devices.end(),
        [](auto &item) { return item && IsNearlinkBtOutput(*item); });
    CHECK_AND_RETURN(it != devices.end() && *it);
    auto &device = **it;
    AUDIO_INFO_LOG("command=%{public}d, id=%{public}d, type=%{public}d, role=%{public}d, "
        "connState=%{public}d, enable=%{public}d", command, device.deviceId_,
        device.deviceType_, device.deviceRole_, device.connectState_, device.isEnable_);

    std::lock_guard lg(mutex_);
    auto it2 = connectedSet_.find(device.deviceId_);
    if (IsConnected(device)) {
        CHECK_AND_RETURN(it2 == connectedSet_.end());
        connectedSet_.insert(it2, device.deviceId_);
        SetConnectLostRollback(device);
        ContinueSetDevice(device);
    } else {
        CHECK_AND_RETURN(it2 != connectedSet_.end());
        connectedSet_.erase(it2);
        HandleConnectLost(device);
    }
}

void AudioRollbackManager::OnDeviceDisconnect(const AudioDeviceDescriptor &desc)
{
    CHECK_AND_RETURN(IsNearlinkBtOutput(desc));
    std::lock_guard lg(mutex_);
    connectedSet_.erase(desc.deviceId_);
    HandleConnectLost(desc);
}

void AudioRollbackManager::HandleConnectLost(const AudioDeviceDescriptor &desc)
{
    auto deviceId = desc.deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, type=%{public}d", deviceId, desc.deviceType_);
    auto pred = [deviceId] (SelectionRollback &item) {
        return item.targetId == deviceId;
    };
    RemoveSelection([deviceId](auto &s) { return s.deviceId == deviceId; });
    auto rmIt = remove_if(selectionRollbacks_.begin(), selectionRollbacks_.end(), pred);
    for (auto it = rmIt; it < selectionRollbacks_.end(); it++) {
        RollbackSelection(it->selection);
    }
    selectionRollbacks_.erase(rmIt, selectionRollbacks_.end());

    CHECK_AND_RETURN(rollbackInfo_ && rollbackInfo_->deviceId == deviceId);
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
    CHECK_AND_RETURN(FilterSetDevice(uid, SelectDeviceType::MEDIA_OUTPUT, streamId, device));
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
    CHECK_AND_RETURN(FilterSetDevice(uid, SelectDeviceType::CALL_OUTPUT, streamId, device));
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(uid, streamId, device, caller);
}

bool AudioRollbackManager::FilterSetDevice(int32_t uid, SelectDeviceType type, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device && device->deviceId_ > 0, true);
    auto desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(device->deviceId_);
    CHECK_AND_RETURN_RET(desc, true);
    CHECK_AND_RETURN_RET(!HandleVirtualConnected(uid, type, streamId, *desc), false);
    SetSelectionRollback(uid, type, streamId, *desc);
    return true;
}

bool AudioRollbackManager::HandleVirtualConnected(int32_t uid, SelectDeviceType type,
    uint32_t streamId, const AudioDeviceDescriptor &desc)
{
    if (uid < SYSTEM_UID) {
        uid = SYSTEM_UID;
    }
    auto deviceId = desc.deviceId_;
    auto pred = [uid, type, streamId] (Selection &item) {
        return item.uid == uid && item.type == type && item.streamId == streamId;
    };
    std::lock_guard lg(mutex_);
    rollbackInfo_.reset();
    auto it = remove_if(virtualSelectDevices_.begin(), virtualSelectDevices_.end(), pred);
    if (desc.connectState_ != VIRTUAL_CONNECTED) {
        virtualSelectDevices_.erase(it, virtualSelectDevices_.end());
        return false;
    }
    if (it != virtualSelectDevices_.end()) {
        CHECK_AND_RETURN_RET(it->deviceId != deviceId, true);
        it->deviceId = deviceId;
    } else {
        virtualSelectDevices_.push_back({uid, type, deviceId, streamId});
    }
    AUDIO_INFO_LOG("uid=%{public}d, sType=%{public}d, stream=%{public}d, id=%{public}d, type=%{public}d",
        uid, type, streamId, deviceId, desc.deviceType_);
    timer_.SetTimeout(CONTINUE_TIMEOUT_MS, [uid, type, streamId, deviceId, this]() {
        std::lock_guard lg(mutex_);
        virtualSelectDevices_.erase(remove_if(virtualSelectDevices_.begin(), virtualSelectDevices_.end(),
            [uid, type, streamId, deviceId] (Selection &e) {
                return e.uid == uid && e.type == type && e.streamId == streamId && e.deviceId == deviceId;
            }), virtualSelectDevices_.end());
    });
    return true;
}

void AudioRollbackManager::SetSelectionRollback(int32_t uid, SelectDeviceType type,
    uint32_t streamId, const AudioDeviceDescriptor &desc)
{
    if (uid < SYSTEM_UID) {
        uid = SYSTEM_UID;
    }
    auto deviceId = desc.deviceId_;
    auto pred = [uid, type, streamId] (SelectionRollback &item) {
        return item.selection.uid == uid && item.selection.type == type && item.selection.streamId == streamId;
    };
    std::lock_guard lg(mutex_);
    rollbackInfo_.reset();
    selectionRollbacks_.erase(remove_if(
        selectionRollbacks_.begin(), selectionRollbacks_.end(), pred), selectionRollbacks_.end());
    CHECK_AND_RETURN(!connectedSet_.contains(deviceId) && IsNearlinkBtOutput(desc));
    if (type == SelectDeviceType::MEDIA_OUTPUT && desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        CHECK_AND_RETURN(!AudioDeviceManager::GetAudioDeviceManager().IsScoConnected(desc.macAddress_));
    }
    AUDIO_INFO_LOG("uid=%{public}d, sType=%{public}d, stream=%{public}d, id=%{public}d, type=%{public}d",
        uid, type, streamId, deviceId, desc.deviceType_);
    bool needRollback = false;
    for (auto &item : AudioRouterInfra::GetInstance().GetSelections({type})) {
        CHECK_AND_CONTINUE(item.uid == uid && item.streamId == streamId && item.deviceId != deviceId);
        selectionRollbacks_.push_back({deviceId, item});
        needRollback = true;
    }
    CHECK_AND_RETURN(needRollback);
    timer_.SetTimeout(CONTINUE_TIMEOUT_MS, [deviceId, uid, type, streamId, this]() {
        CHECK_AND_RETURN(RollbackSetDevice(uid, type, streamId, deviceId));
        AudioDeviceCommon::FetchAndRoute("RollbackSetDevice");
    });
}

bool AudioRollbackManager::RollbackSelection(const Selection &selection)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto device = deviceManager.FindConnectedDeviceById(selection.deviceId);
    CHECK_AND_RETURN_RET_LOG(device && device->isEnable_ && (device->connectState_ == CONNECTED ||
        device->connectState_ == DEACTIVE_CONNECTED), false, "NotConnect[%{public}d]", selection.deviceId);
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
    return true;
}

void AudioRollbackManager::ExcludeSelection(SelectDeviceType type, const AudioDeviceDescriptor &desc)
{
    CHECK_AND_RETURN(IsNearlinkBtOutput(desc));
    auto deviceId = desc.deviceId_;
    std::lock_guard lg(mutex_);
    RemoveSelection([deviceId, type](Selection &s) { return s.deviceId == deviceId && s.type == type; });
}

void AudioRollbackManager::OnAppDied(int32_t uid)
{
    std::lock_guard lg(mutex_);
    RemoveSelection([uid](auto &s) { return s.uid == uid; });
}

void AudioRollbackManager::RemoveSelection(std::function<bool(Selection&)> &&pred)
{
    virtualSelectDevices_.erase(remove_if(virtualSelectDevices_.begin(),
        virtualSelectDevices_.end(), pred), virtualSelectDevices_.end());

    selectionRollbacks_.erase(remove_if(selectionRollbacks_.begin(),
        selectionRollbacks_.end(), [&pred](auto &e) {
            return pred(e.selection);
        }), selectionRollbacks_.end());

    CHECK_AND_RETURN(rollbackInfo_);
    rollbackInfo_->selections.erase(remove_if(rollbackInfo_->selections.begin(),
        rollbackInfo_->selections.end(), pred), rollbackInfo_->selections.end());
}

StepTimer::~StepTimer()
{
    {
        unique_lock lk(mutex_);
        tasks_.clear();
        cv_.notify_all();
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

void StepTimer::SetTimeout(uint32_t timeoutMs, std::function<void()> fn)
{
    int64_t now = chrono::duration_cast<chrono::milliseconds>(
        chrono::steady_clock::now().time_since_epoch()).count();
    unique_lock lk(mutex_);
    tasks_.push_back({now + timeoutMs, std::move(fn)});
    if (running_) {
        cv_.notify_one();
        return;
    }
    running_ = true;
    if (worker_.joinable()) {
        lk.unlock();
        worker_.join();
        lk.lock();
    }
    worker_ = std::thread(&StepTimer::WorkLoop, this);
}

std::vector<std::function<void()>> StepTimer::PickTasks(int64_t &now)
{
    now = chrono::duration_cast<chrono::milliseconds>(
        chrono::steady_clock::now().time_since_epoch()).count();
    std::vector<std::function<void()>> funcs;
    unique_lock lk(mutex_);
    for (auto it = tasks_.begin(); it != tasks_.end();) {
        if (it->first <= now) {
            funcs.push_back(std::move(it->second));
            it = tasks_.erase(it);
        } else {
            it++;
        }
    }
    return funcs;
}

void StepTimer::WorkLoop()
{
    while (true) {
        int64_t now = 0;
        for (auto tasks = PickTasks(now); !tasks.empty(); tasks = PickTasks(now)) {
            for (auto &task : tasks) {
                task();
            }
        }
        unique_lock lk(mutex_);
        if (tasks_.empty()) {
            running_ = false;
            return;
        }
        int64_t next = INT64_MAX;
        for (auto &t : tasks_) {
            int64_t delta = t.first - now;
            next = (delta < next) ? ((delta > 0) ? delta : 0) : next;
        }
        cv_.wait_for(lk, chrono::milliseconds(next));
    }
}

}
}
