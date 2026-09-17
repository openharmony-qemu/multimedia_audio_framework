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
#include "audio_log.h"
#include "audio_device_manager.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr int32_t CONTINUE_TIMEOUT_SEC = 2;
constexpr int32_t ROLLBACK_TIMEOUT_SEC = 1;
const set<ConnectState> NOT_CONNECTED{VIRTUAL_CONNECTED, DEACTIVE_CONNECTED};

AudioRollbackManager& AudioRollbackManager::GetInstance()
{
    static AudioRollbackManager instance;
    return instance;
}

bool AudioRollbackManager::HandleNonConnected(int32_t uid, SelectDeviceType type,
    uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET_LOG(device && device->deviceId_ > 0, false, "Invalid desc");
    if (uid < SYSTEM_UID) {
        uid = SYSTEM_UID;
    }
    auto deviceId = device->deviceId_;
    auto pred = [uid, type, streamId] (Selection &item) {
        return item.uid == uid && item.type == type && item.streamId == streamId;
    };
    std::lock_guard lg(mutex_);
    rollbackInfo_.reset();
    auto it = remove_if(continueSelections_.begin(), continueSelections_.end(), pred);
    if (!NOT_CONNECTED.contains(device->connectState_)) {
        continueSelections_.erase(it, continueSelections_.end());
        return false;
    }
    AUDIO_INFO_LOG("uid=%{public}d, id=%{public}d, type=%{public}d", uid, deviceId, type);
    if (it != continueSelections_.end()) {
        if (it->deviceId == deviceId) {
            return true;
        }
        *it = {uid, type, deviceId, streamId};
    } else {
        continueSelections_.push_back({uid, type, deviceId, streamId});
    }
    thread([deviceId, uid, type, streamId, this]() {
        this_thread::sleep_for(chrono::seconds(CONTINUE_TIMEOUT_SEC));
        auto pred = [deviceId, uid, type, streamId] (Selection &item) {
            return item.deviceId == deviceId && item.uid == uid && item.type == type && item.streamId == streamId;
        };
        std::lock_guard lg(mutex_);
        auto it = remove_if(continueSelections_.begin(), continueSelections_.end(), pred);
        continueSelections_.erase(it, continueSelections_.end());
    }).detach();
    return true;
}

void AudioRollbackManager::ContinueSelection(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(device && device->connectState_ == CONNECTED);
    auto id = device->deviceId_;
    auto pred = [id] (Selection &item) {
        return item.deviceId == id;
    };
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d", id, device->deviceType_);
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    std::lock_guard lg(mutex_);
    auto rmIt = remove_if(continueSelections_.begin(), continueSelections_.end(), pred);
    CHECK_AND_RETURN(rmIt != continueSelections_.end());
    for (auto it = rmIt; it < continueSelections_.end(); it++) {
        switch (it->type) {
            case SelectDeviceType::MEDIA_INPUT:
                strategy.SetMediaInputDevice(it->uid, it->streamId, device);
                break;
            case SelectDeviceType::MEDIA_OUTPUT:
                strategy.SetMediaOutputDevice(it->uid, it->streamId, device);
                break;
            case SelectDeviceType::CALL_INPUT:
                strategy.SetCallInputDevice(it->uid, it->streamId, device);
                break;
            case SelectDeviceType::CALL_OUTPUT:
                strategy.SetCallOutputDevice(it->uid, it->streamId, device);
                break;
            default:
                AUDIO_ERR_LOG("Error: invalid type");
        }
    }
    continueSelections_.erase(rmIt, continueSelections_.end());
}

void AudioRollbackManager::SetNewConnectRollback(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(device && NOT_CONNECTED.contains(device->connectState_));
    auto deviceId = device->deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d", deviceId, device->deviceType_);
    std::lock_guard lg(mutex_);
    continueSelections_.clear();
    if (!rollbackInfo_) {
        rollbackInfo_ = std::make_unique<RollbackInfo>();
    }
    rollbackInfo_->type = NEW_CONNECT_ROLLBACK;
    rollbackInfo_->deviceId = deviceId;
    rollbackInfo_->selections = AudioRouterInfra::GetInstance().GetSelections(
        {SelectDeviceType::CALL_OUTPUT, SelectDeviceType::MEDIA_OUTPUT});
    thread([deviceId, this]() {
        this_thread::sleep_for(chrono::seconds(CONTINUE_TIMEOUT_SEC));
        RollbackSelection(deviceId, NEW_CONNECT_ROLLBACK);
    }).detach();
}

void AudioRollbackManager::SetConnectLostRollback(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(device && device->connectState_ == CONNECTED);
    auto deviceId = device->deviceId_;
    AUDIO_INFO_LOG("id=%{public}d, devType=%{public}d", deviceId, device->deviceType_);
    std::lock_guard lg(mutex_);
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

void AudioRollbackManager::RollbackSelection(int32_t deviceId, RollbackType type)
{
    std::lock_guard lg(mutex_);
    CHECK_AND_RETURN(rollbackInfo_ && rollbackInfo_->deviceId == deviceId && rollbackInfo_->type == type);
    AUDIO_INFO_LOG("id=%{public}d, rollbackType=%{public}d", deviceId, type);
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    for (auto &item : rollbackInfo_->selections) {
        auto device = deviceManager.FindConnectedDeviceById(item.deviceId);
        CHECK_AND_CONTINUE_LOG(device && device->connectState_ == CONNECTED, "Not connect[%{public}d]", item.deviceId);
        switch (item.type) {
            case SelectDeviceType::MEDIA_INPUT:
                strategy.SetMediaInputDevice(item.uid, item.streamId, device);
                break;
            case SelectDeviceType::MEDIA_OUTPUT:
                strategy.SetMediaOutputDevice(item.uid, item.streamId, device);
                break;
            case SelectDeviceType::CALL_INPUT:
                strategy.SetCallInputDevice(item.uid, item.streamId, device);
                break;
            case SelectDeviceType::CALL_OUTPUT:
                strategy.SetCallOutputDevice(item.uid, item.streamId, device);
                break;
            default:
                AUDIO_ERR_LOG("Error: invalid type");
        }
    }
    rollbackInfo_.reset();
}

void AudioRollbackManager::OnConnectStateUpdate(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    CHECK_AND_RETURN(command == CONNECTSTATE_UPDATE);
    auto devices = AudioDeviceManager::GetAudioDeviceManager()
        .GetAvailableBluetoothDevice(desc.deviceType_, desc.macAddress_);
    CHECK_AND_RETURN_LOG(!devices.empty() && devices[0] && devices[0]->deviceId_ > 0, "Invalid desc");
    auto device = devices[0];
    if (device->connectState_ == CONNECTED) {
        SetConnectLostRollback(device);
        ContinueSelection(device);
    } else {
        RollbackSelection(device->deviceId_, CONNECT_LOST_ROLLBACK);
    }
}

void AudioRollbackManager::SetMediaInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(!HandleNonConnected(uid, SelectDeviceType::MEDIA_INPUT, streamId, device));
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(uid, streamId, device);
}

void AudioRollbackManager::SetMediaOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(!HandleNonConnected(uid, SelectDeviceType::MEDIA_OUTPUT, streamId, device));
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(uid, streamId, device);
}

void AudioRollbackManager::SetCallInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN(!HandleNonConnected(uid, SelectDeviceType::CALL_INPUT, streamId, device));
    AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(uid, streamId, device);
}

void AudioRollbackManager::SetCallOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller)
{
    CHECK_AND_RETURN(!HandleNonConnected(uid, SelectDeviceType::CALL_OUTPUT, streamId, device));
    AudioRouterSelectStrategy::GetInstance().SetCallOutputDevice(uid, streamId, device, caller);
}

}
}
