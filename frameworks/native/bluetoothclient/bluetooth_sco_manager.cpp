/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "BluetoothScoManager"
#endif

#include "bluetooth_sco_manager.h"
#include "bluetooth_errorcode.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "bluetooth_device_utils.h"
#include "bluetooth_hfp_interface.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

BluetoothScoManager &BluetoothScoManager::GetInstance()
{
    static BluetoothScoManager scoManager;
    return scoManager;
}

BluetoothScoManager::BluetoothScoManager()
{
    currentScoState_ = AudioScoState::DISCONNECTED;
}

void BluetoothScoManager::UpdateScoState(HfpScoConnectState scoState, const BluetoothRemoteDevice &device)
{
    {
        std::unique_lock<std::mutex> stateLock(scoLock_);
        AUDIO_INFO_LOG("recv sco %{public}s state with %{public}s device and current sco state %{public}d",
            scoState == HfpScoConnectState::SCO_DISCONNECTED ? "diconnect" : "connect",
            IsSameHfpDevice(activeHfpDevice_, device) ? "same" : "not same", currentScoState_);
        switch (currentScoState_) {
            case AudioScoState::DISCONNECTED:
                UpdateScoStateWhenDisconnected(scoState, device);
                break;
            case AudioScoState::CONNECTED:
                UpdateScoStateWhenConnected(scoState, device);
                break;
            case AudioScoState::CONNECTING:
                UpdateScoStateWhenConnecting(scoState, device);
                break;
            case AudioScoState::DISCONNECTING:
                UpdateScoStateWhenDisconnecting(scoState, device);
                break;
            default:
                break;
        }
    }
}

void BluetoothScoManager::UpdateScoStateWhenDisconnected(HfpScoConnectState scoState, const BluetoothRemoteDevice &device)
{
    if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        ForceUpdateScoState();
    }
}

void BluetoothScoManager::UpdateScoStateWhenConnected(HfpScoConnectState scoState, const BluetoothRemoteDevice &device)
{
    if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        if (IsSameHfpDevice(activeHfpDevice_, device)) {
            currentScoState_ = AudioScoState::DISCONNECTED;
        }
    } else if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        ForceUpdateScoState();
    }
}

void BluetoothScoManager::UpdateScoStateWhenConnecting(HfpScoConnectState scoState, const BluetoothRemoteDevice &device)
{
    if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        if (IsSameHfpDevice(activeHfpDevice_, device)) {
            currentScoState_ = AudioScoState::DISCONNECTED;
        }
    } else if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        if (IsSameHfpDevice(activeHfpDevice_, device)) {
            currentScoState_ = AudioScoState::CONNECTED;
        } else {
            ForceUpdateScoState();
        }
    }
    ProcCacheRequest();
}

void BluetoothScoManager::UpdateScoStateWhenDisconnecting(HfpScoConnectState scoState, const BluetoothRemoteDevice &device)
{
    if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        if (IsSameHfpDevice(activeHfpDevice_, device)) {
            currentScoState_ = AudioScoState::DISCONNECTED;
        }
    } else if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        ForceUpdateScoState();
    }
    ProcCacheRequest();
}

void BluetoothScoManager::ForceUpdateScoState()
{
    // need query sco type from bluetooth to refresh local
}

void BluetoothScoManager::ProcCacheRequest()
{
    std::shared_ptr<ScoCacheRequest> req = nullptr;
    {
        req = cacheReq_;
        cacheReq_ = nullptr;
    }
    if (req == nullptr) {
        return;
    }

    AUDIO_INFO_LOG("proc cache %{public}s request category %{public}d and current sco state %{public}d",
        req->connectReq ? "connect" : "disconnect", req->category, currentScoState_);
    if (req->connectReq) {
        HandleScoConnectNoLock(req->category, req->device);
    } else {
        HandleScoDisconnectNoLock(req->device);
    }
}

int32_t BluetoothScoManager::HandleScoConnect(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return HandleScoConnectNoLock(scoCategory, device);
}

int32_t BluetoothScoManager::HandleScoConnectNoLock(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = SUCCESS;
    switch (currentScoState_) {
        case AudioScoState::DISCONNECTED:
            ret = ProcConnectReqWhenDisconnected(scoCategory, device);
            break;
        case AudioScoState::CONNECTED:
            ret = ProcConnectReqWhenConnected(scoCategory, device);
            break;
        case AudioScoState::CONNECTING:
            ret = ProcConnectReqWhenConnecting(scoCategory, device);
            break;
        case AudioScoState::DISCONNECTING:
            ret = SaveRequestToCache(true, scoCategory, device);
            break;
        default:
            ret = ERROR;
            break;
    }
    return ret;
}

int32_t BluetoothScoManager::ProcConnectReqWhenDisconnected(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = ConnectSco(scoCategory, device);
    if (ret == BT_ERR_SCO_HAS_BEEN_CONNECTED) {
        AUDIO_WARNING_LOG("category %{public}d has been connected", scoCategory);
        currentScoState_ = AudioScoState::CONNECTED;
    } else if (ret != 0) {
        AUDIO_ERR_LOG("connect scoCategory: %{public}d ret: %{public}d ", scoCategory, ret);
        int32_t restoreRet = TryRestoreHfpDevice(scoCategory, device);
        CHECK_AND_RETURN_RET_LOG(restoreRet == 0, restoreRet, "try restore hfp device failed");
        currentScoState_ = AudioScoState::CONNECTING;
    } else {
        AUDIO_INFO_LOG("connect scoCategory: %{public}d success ", scoCategory);
        currentScoState_ = AudioScoState::CONNECTING;
    }
    currentScoCategory_ = scoCategory;
    activeHfpDevice_ = device;
    return SUCCESS;
}

int32_t BluetoothScoManager::ProcConnectReqWhenConnected(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    bool isSameDevice = IsSameHfpDevice(activeHfpDevice_, device);
    if (IsNeedSwitchScoCategory(scoCategory) && isSameDevice) {
        AUDIO_INFO_LOG("bypass connect category %{public}d current category %{public}d for %{public}s device",
            scoCategory, currentScoCategory_, isSameDevice ? "same" : "not same");
        return SUCCESS;
    }

    AUDIO_INFO_LOG("connect category %{public}d current category %{public}d for %{public}s device",
        scoCategory, currentScoCategory_, isSameDevice ? "same" : "not same");
    int32_t ret = DisconnectScoReliable(currentScoCategory_, activeHfpDevice_);
    if (ret != 0) {
        currentScoState_ = AudioScoState::DISCONNECTED;
        return ProcConnectReqWhenDisconnected(scoCategory, device);
    }

    currentScoState_ = AudioScoState::DISCONNECTING;
    return SaveRequestToCache(true, scoCategory, device);
}

int32_t BluetoothScoManager::ProcConnectReqWhenConnecting(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    bool isSameDevice = IsSameHfpDevice(activeHfpDevice_, device);
    if (IsNeedSwitchScoCategory(scoCategory) && isSameDevice) {
        AUDIO_INFO_LOG("connect category %{public}d current category %{public}d for %{public}s device",
            scoCategory, currentScoCategory_, isSameDevice ? "same" : "not same");
        return SUCCESS;
    }
    return SaveRequestToCache(true, scoCategory, device);
}

bool BluetoothScoManager::IsNeedSwitchScoCategory(ScoCategory scoCategory)
{
    if (scoCategory == currentScoCategory_ ) {
        return false;
    }

    if ((currentScoCategory_ == ScoCategory::SCO_DEFAULT &&
        scoCategory == ScoCategory::SCO_VIRTUAL) ||
        (currentScoCategory_ == ScoCategory::SCO_VIRTUAL &&
        scoCategory == ScoCategory::SCO_DEFAULT)) {
        return false;
    }

    if ((currentScoCategory_ == ScoCategory::SCO_RECOGNITION) &&
        (scoCategory == ScoCategory::SCO_CALLULAR ||
        scoCategory == ScoCategory::SCO_VIRTUAL)) {
        return false;
    }

    if ((scoCategory == ScoCategory::SCO_RECOGNITION) &&
        (currentScoCategory_ == ScoCategory::SCO_CALLULAR ||
        currentScoCategory_ == ScoCategory::SCO_VIRTUAL)) {
        return false;
    }
    return true;
}

int32_t BluetoothScoManager::HandleScoDisconnect(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return HandleScoDisconnectNoLock(device);
}

int32_t BluetoothScoManager::HandleScoDisconnectNoLock(const BluetoothRemoteDevice &device)
{
    int32_t ret = SUCCESS;
    switch (currentScoState_) {
        case AudioScoState::DISCONNECTED:
        case AudioScoState::DISCONNECTING:
            cacheReq_ = nullptr;
            break;
        case AudioScoState::CONNECTED:
            ret = ProcDisconnectReqWhenConnected(device);
            break;
        case AudioScoState::CONNECTING:
            ret = ProcDisconnectReqWhenConnecting(device);
            break;
        default:
            ret = ERROR;
            break;
    }
    return ret;
}

int32_t BluetoothScoManager::ProcDisconnectReqWhenConnected(const BluetoothRemoteDevice &device)
{
    if (!IsSameHfpDevice(activeHfpDevice_, device)) {
        AUDIO_WARNING_LOG("disconnect device not meeting expectations");
    }
    cacheReq_ = nullptr;
    int32_t ret = DisconnectSco(currentScoCategory_, activeHfpDevice_);
    if (ret != 0) {
        currentScoState_ = AudioScoState::DISCONNECTED;
    } else {
        currentScoState_ = AudioScoState::DISCONNECTING;
    }
    return SUCCESS;
}

int32_t BluetoothScoManager::ProcDisconnectReqWhenConnecting(const BluetoothRemoteDevice &device)
{
    if (!IsSameHfpDevice(activeHfpDevice_, device)) {
        AUDIO_WARNING_LOG("disconnect device not meeting expectations");
    }
    cacheReq_ = nullptr;
    return SaveRequestToCache(false, currentScoCategory_, device);
}

int32_t BluetoothScoManager::SaveRequestToCache(bool isConnect, ScoCategory scoCategory,
    const BluetoothRemoteDevice &device)
{
    if (cacheReq_ == nullptr) {
        cacheReq_ = std::make_shared<ScoCacheRequest>();
    }
    CHECK_AND_RETURN_RET_LOG(cacheReq_ != nullptr, ERROR, "request cache is nullptr");
    cacheReq_->connectReq = isConnect;
    cacheReq_->category = scoCategory;
    cacheReq_->device = device;
    AUDIO_INFO_LOG("cache request, scoCategory: %{public}d isConnect: %{public}d ", scoCategory, isConnect);
    return SUCCESS;
}

bool BluetoothScoManager::IsSameHfpDevice(const BluetoothRemoteDevice &device1, const BluetoothRemoteDevice &device2)
{
    return device1.GetDeviceAddr() == device2.GetDeviceAddr();
}

int32_t BluetoothScoManager::ConnectSco(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = ERROR;
    if (scoCategory == ScoCategory::SCO_RECOGNITION) {
        ret = BluetoothHfpInterface::GetInstance().OpenVoiceRecognition(device);
    } else {
        if (scoCategory == ScoCategory::SCO_DEFAULT) {
            scoCategory = ScoCategory::SCO_VIRTUAL;
        }
        ret = BluetoothHfpInterface::GetInstance().ConnectSco(static_cast<uint8_t> (scoCategory));
    }
    return ret;
}

int32_t BluetoothScoManager::TryRestoreHfpDevice(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = BluetoothHfpInterface::GetInstance().SetActiveDevice(device);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "set active hfp device failed");
    return ConnectSco(scoCategory, device);
}

int32_t BluetoothScoManager::DisconnectSco(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = ERROR;
    if (scoCategory == ScoCategory::SCO_RECOGNITION) {
        ret = BluetoothHfpInterface::GetInstance().CloseVoiceRecognition(device);
    } else {
        if (scoCategory == ScoCategory::SCO_DEFAULT) {
            scoCategory = SCO_VIRTUAL;
        }
        ret = BluetoothHfpInterface::GetInstance().DisconnectSco(static_cast<uint8_t> (scoCategory));
    }

    return ret;
}

int32_t BluetoothScoManager::DisconnectScoReliable(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int ret = DisconnectSco(scoCategory, device);
    if (ret == BT_ERR_VIRTUAL_CALL_NOT_STARTED) {
        // try to get current category form bluetooth
        AUDIO_WARNING_LOG("DisconnectSco, scoCategory: %{public}d failed", scoCategory);
    } else if (ret != 0) {
        AUDIO_ERR_LOG("DisconnectSco, scoCategory: %{public}d ret: %{public}d ", scoCategory, ret);
    } else {
        AUDIO_INFO_LOG("DisconnectSco, scoCategory: %{public}d success", scoCategory);
    }
    return ret;
}

AudioScoState BluetoothScoManager::GetAudioScoState()
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return currentScoState_;
}

bool BluetoothScoManager::IsInScoCategory(ScoCategory scoCategory)
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return (currentScoCategory_ == scoCategory) &&
        (currentScoState_ == AudioScoState::CONNECTING ||
        currentScoState_ == AudioScoState::CONNECTED);
}
}
}