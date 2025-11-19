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
#include "hisysevent.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

static const uint32_t WAIT_A2DP_OFFLOAD_CLOSE_DELAY_US = 30000; // 30ms

BluetoothScoManager &BluetoothScoManager::GetInstance()
{
    static BluetoothScoManager scoManager;
    return scoManager;
}

BluetoothScoManager::BluetoothScoManager()
{
    currentScoDevice_ = BluetoothHfpInterface::GetInstance().GetActiveDevice();
    currentScoState_ = BluetoothHfpInterface::GetInstance().GetScoState(currentScoDevice_);
    CHECK_AND_RETURN_LOG(currentScoState_ != AudioScoState::DISCONNECTED, "sco state is disconnected");
    ScoCategory category = SCO_DEFAULT;
    int32_t ret = BluetoothHfpInterface::GetInstance().GetCurrentCategory(category);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "get sco category failed");
    currentScoCategory_ = category;
    AUDIO_INFO_LOG("current sco category %{public}d state %{public}d of device %{public}s",
        currentScoCategory_, currentScoState_,
        GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
}

void BluetoothScoManager::UpdateScoState(HfpScoConnectState scoState,
    const BluetoothRemoteDevice &device, int reason)
{
    std::unique_lock<std::mutex> stateLock(scoLock_);
    AUDIO_INFO_LOG("recv sco %{public}s state of %{public}s device and current sco state %{public}d of %{public}s",
        scoState == HfpScoConnectState::SCO_DISCONNECTED ? "diconnect" : "connect",
        GetEncryptAddr(device.GetDeviceAddr()).c_str(), currentScoState_,
        GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());

    if (!IsSameHfpDevice(currentScoDevice_, device)) {
        WriteScoStateFaultEvent(scoState, device, reason);
        return;
    }
    switch (currentScoState_) {
        case AudioScoState::DISCONNECTED:
            UpdateScoStateWhenDisconnected(scoState, device, reason);
            break;
        case AudioScoState::CONNECTED:
            UpdateScoStateWhenConnected(scoState, device, reason);
            break;
        case AudioScoState::CONNECTING:
            UpdateScoStateWhenConnecting(scoState, device, reason);
            break;
        case AudioScoState::DISCONNECTING:
            UpdateScoStateWhenDisconnecting(scoState, device, reason);
            break;
        default:
            break;
    }
}

void BluetoothScoManager::UpdateScoStateWhenDisconnected(HfpScoConnectState scoState,
    const BluetoothRemoteDevice &device, int reason)
{
    WriteScoStateFaultEvent(scoState, device, reason);
    if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        ForceUpdateScoCategory();
    }
}

void BluetoothScoManager::UpdateScoStateWhenConnected(HfpScoConnectState scoState,
    const BluetoothRemoteDevice &device, int reason)
{
    WriteScoStateFaultEvent(scoState, device, reason);
    if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        SetAudioScoState(AudioScoState::DISCONNECTED);
    } else if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        ScoCategory category = SCO_DEFAULT;
        int32_t ret = BluetoothHfpInterface::GetInstance().GetCurrentCategory(category);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get sco category failed");
        if (currentScoCategory_ != category) {
            AUDIO_INFO_LOG("update sco category from %{public}d to %{public}d of device %{public}s",
                currentScoCategory_, category,
                GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
            currentScoCategory_ = category;
        }
    }
}

void BluetoothScoManager::UpdateScoStateWhenConnecting(HfpScoConnectState scoState,
    const BluetoothRemoteDevice &device, int reason)
{
    if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        WriteScoStateFaultEvent(scoState, device, reason);
        SetAudioScoState(AudioScoState::DISCONNECTED);
    } else if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        SetAudioScoState(AudioScoState::CONNECTED);
    }
    ProcCacheRequest();
}

void BluetoothScoManager::UpdateScoStateWhenDisconnecting(HfpScoConnectState scoState,
    const BluetoothRemoteDevice &device, int reason)
{
    if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        SetAudioScoState(AudioScoState::DISCONNECTED);
    } else if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        WriteScoStateFaultEvent(scoState, device, reason);
        ForceUpdateScoCategory();
    }
    ProcCacheRequest();
}

void BluetoothScoManager::WriteScoStateFaultEvent(HfpScoConnectState scoState,
    const BluetoothRemoteDevice &device, int reason)
{
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "SCO_STATE_BT",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "SCO_STATE", static_cast<uint8_t>(currentScoState_),
        "REASON", static_cast<uint8_t>(reason),
        "SCO_ADDRESS", device.GetDeviceAddr());
    if (ret) {
        AUDIO_ERR_LOG("write event fail: SCO_STATE_BT, ret = %{public}d", ret);
    }
}

void BluetoothScoManager::ForceUpdateScoCategory()
{
    // need query sco type from bluetooth to refresh local
    ScoCategory category = SCO_DEFAULT;
    int32_t ret = BluetoothHfpInterface::GetInstance().GetCurrentCategory(category);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "get sco category failed");
    AUDIO_INFO_LOG("force update sco category from %{public}d to %{public}d of device %{public}s",
        currentScoCategory_, category,
        GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
    currentScoCategory_ = category;
    SetAudioScoState(AudioScoState::CONNECTED);
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

int32_t BluetoothScoManager::HandleScoConnect(ScoCategory scoCategory,
    const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return HandleScoConnectNoLock(scoCategory, device);
}

int32_t BluetoothScoManager::HandleScoConnectNoLock(ScoCategory scoCategory,
    const BluetoothRemoteDevice &device)
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
            ret = SaveRequestToCache(true, scoCategory, device, "connect request when disconnecting");
            break;
        default:
            ret = ERROR;
            break;
    }
    return ret;
}

int32_t BluetoothScoManager::ProcConnectReqWhenDisconnected(ScoCategory scoCategory,
    const BluetoothRemoteDevice &device)
{
    int32_t ret = ConnectSco(scoCategory, device);
    if (ret == BT_ERR_SCO_HAS_BEEN_CONNECTED) {
        AUDIO_WARNING_LOG("category %{public}d has been connected", scoCategory);
        SetAudioScoState(AudioScoState::CONNECTED);
    } else if (ret != 0) {
        AUDIO_ERR_LOG("connect scoCategory: %{public}d ret: %{public}d ", scoCategory, ret);
        int32_t restoreRet = TryRestoreHfpDevice(scoCategory, device);
        CHECK_AND_RETURN_RET_LOG(restoreRet == 0, restoreRet, "try restore hfp device failed");
        SetAudioScoState(AudioScoState::CONNECTING);
    } else {
        AUDIO_INFO_LOG("connect scoCategory: %{public}d success ", scoCategory);
        SetAudioScoState(AudioScoState::CONNECTING);
    }
    currentScoCategory_ = scoCategory;
    currentScoDevice_ = device;
    return SUCCESS;
}

int32_t BluetoothScoManager::ProcConnectReqWhenConnected(ScoCategory scoCategory,
    const BluetoothRemoteDevice &device)
{
    bool isSameDevice = IsSameHfpDevice(currentScoDevice_, device);
    if (!IsNeedSwitchScoCategory(scoCategory) && isSameDevice) {
        AUDIO_INFO_LOG("bypass connect category %{public}d current category %{public}d for device %{public}s",
            scoCategory, currentScoCategory_, GetEncryptAddr(device.GetDeviceAddr()).c_str());
        return SUCCESS;
    }

    AUDIO_INFO_LOG("connect category %{public}d current %{public}d for device %{public}s current %{public}s",
        scoCategory, currentScoCategory_, GetEncryptAddr(device.GetDeviceAddr()).c_str(),
        GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
    int32_t ret = DisconnectScoReliable(currentScoCategory_, currentScoDevice_);
    if (ret != 0) {
        SetAudioScoState(AudioScoState::DISCONNECTED);
        return ProcConnectReqWhenDisconnected(scoCategory, device);
    }

    SetAudioScoState(AudioScoState::DISCONNECTING);
    return SaveRequestToCache(true, scoCategory, device, "ProcConnectReqWhenConnected");
}

int32_t BluetoothScoManager::ProcConnectReqWhenConnecting(ScoCategory scoCategory,
    const BluetoothRemoteDevice &device)
{
    bool isSameDevice = IsSameHfpDevice(currentScoDevice_, device);
    if (!IsNeedSwitchScoCategory(scoCategory) && isSameDevice) {
        AUDIO_INFO_LOG("connect category %{public}d current category %{public}d for device %{public}s",
            scoCategory, currentScoCategory_, GetEncryptAddr(device.GetDeviceAddr()).c_str());
        return SUCCESS;
    }
    return SaveRequestToCache(true, scoCategory, device, "ProcConnectReqWhenConnecting");
}

bool BluetoothScoManager::IsNeedSwitchScoCategory(ScoCategory scoCategory)
{
    if (scoCategory == currentScoCategory_) {
        return false;
    }

    if ((currentScoCategory_ == ScoCategory::SCO_DEFAULT &&
        scoCategory == ScoCategory::SCO_VIRTUAL) ||
        (currentScoCategory_ == ScoCategory::SCO_VIRTUAL &&
        scoCategory == ScoCategory::SCO_DEFAULT)) {
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
    if (!IsSameHfpDevice(currentScoDevice_, device)) {
        AUDIO_WARNING_LOG("disconnect device %{public}s but current is %{public}s",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(),
            GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
    }
    cacheReq_ = nullptr;
    int32_t ret = DisconnectScoReliable(currentScoCategory_, currentScoDevice_);
    if (ret != 0) {
        SetAudioScoState(AudioScoState::DISCONNECTED);
    } else {
        SetAudioScoState(AudioScoState::DISCONNECTING);
    }
    return SUCCESS;
}

int32_t BluetoothScoManager::ProcDisconnectReqWhenConnecting(const BluetoothRemoteDevice &device)
{
    if (!IsSameHfpDevice(currentScoDevice_, device)) {
        AUDIO_WARNING_LOG("disconnect device %{public}s but current is %{public}s",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(),
            GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
    }
    cacheReq_ = nullptr;
    return SaveRequestToCache(false, currentScoCategory_, device, "ProcDisconnectReqWhenConnecting");
}

int32_t BluetoothScoManager::SaveRequestToCache(bool isConnect, ScoCategory scoCategory,
    const BluetoothRemoteDevice &device, const std::string &reason)
{
    if (cacheReq_ == nullptr) {
        cacheReq_ = std::make_shared<ScoCacheRequest>();
    } else {
        if (cacheReq_->connectReq == isConnect && cacheReq_->category == scoCategory &&
            IsSameHfpDevice(cacheReq_->device, device)) {
            return SUCCESS;
        }
    }
    CHECK_AND_RETURN_RET_LOG(cacheReq_ != nullptr, ERROR, "request cache is nullptr");
    cacheReq_->connectReq = isConnect;
    cacheReq_->category = scoCategory;
    cacheReq_->device = device;
    AUDIO_INFO_LOG("%{public}s cache request, scoCategory: %{public}d isConnect: %{public}d",
        reason.c_str(), scoCategory, isConnect);
    return SUCCESS;
}

bool BluetoothScoManager::IsSameHfpDevice(const BluetoothRemoteDevice &device1,
    const BluetoothRemoteDevice &device2)
{
    return device1.GetDeviceAddr() == device2.GetDeviceAddr();
}

int32_t BluetoothScoManager::ConnectSco(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = ERROR;
    if (scoCategory == ScoCategory::SCO_RECOGNITION) {
        ret = BluetoothHfpInterface::GetInstance().OpenVoiceRecognition(device);
        // Ensure A2DP offload route is closed before enabling BT SCO route.
        // Previously, inconsistency between uplink and downlink caused headset noise.
        usleep(WAIT_A2DP_OFFLOAD_CLOSE_DELAY_US);
    } else {
        if (scoCategory == ScoCategory::SCO_DEFAULT) {
            scoCategory = ScoCategory::SCO_VIRTUAL;
        }
        ret = BluetoothHfpInterface::GetInstance().ConnectSco(static_cast<uint8_t>(scoCategory));
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
        ret = BluetoothHfpInterface::GetInstance().DisconnectSco(static_cast<uint8_t>(scoCategory));
    }

    return ret;
}

int32_t BluetoothScoManager::DisconnectScoReliable(ScoCategory scoCategory, const BluetoothRemoteDevice &device)
{
    int32_t ret = DisconnectSco(scoCategory, device);
    if (ret == BT_ERR_VIRTUAL_CALL_NOT_STARTED) {
        // try to get current category form bluetooth
        AUDIO_WARNING_LOG("DisconnectSco, scoCategory: %{public}d failed", scoCategory);
        ScoCategory tmp = SCO_DEFAULT;
        ret = BluetoothHfpInterface::GetInstance().GetCurrentCategory(tmp);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get sco category failed");
        ret = DisconnectSco(tmp, device);
        AUDIO_INFO_LOG("DisconnectSco, scoCategory: %{public}d ret: %{public}d", tmp, ret);
    } else if (ret != 0) {
        AUDIO_ERR_LOG("DisconnectSco, scoCategory: %{public}d ret: %{public}d", scoCategory, ret);
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

ScoCategory BluetoothScoManager::GetAudioScoCategory()
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return currentScoCategory_;
}

void BluetoothScoManager::ResetScoState(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    if (!IsSameHfpDevice(currentScoDevice_, device)) {
        AUDIO_WARNING_LOG("reset device %{public}s but current is %{public}s",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(),
            GetEncryptAddr(currentScoDevice_.GetDeviceAddr()).c_str());
        return;
    }

    cacheReq_ = nullptr;
    SetAudioScoState(AudioScoState::DISCONNECTED);
    currentScoDevice_ = BluetoothRemoteDevice();
}

void BluetoothScoManager::SetAudioScoState(AudioScoState state)
{
    if (currentScoState_ == state) {
        return;
    }
    currentScoState_ = state;
    if (currentScoState_ == AudioScoState::CONNECTING ||
        currentScoState_ == AudioScoState::DISCONNECTING) {
        if (scoTimer_ != nullptr) {
            scoTimer_->StopTimer();
        } else {
            scoTimer_ = std::make_shared<AudioTimer>(std::bind(
                &BluetoothScoManager::OnScoStateTimeOut, this));
            if (scoTimer_ == nullptr) {
                AUDIO_ERR_LOG("create audio timer failed");
                return;
            }
        }
        scoTimer_->StartTimer(scoStateDuration_);
        AUDIO_DEBUG_LOG("start timer for state %{public}d", currentScoState_);
    } else {
        scoTimer_ = nullptr;
        AUDIO_DEBUG_LOG("delete timer for state %{public}d", currentScoState_);
    }
}

void BluetoothScoManager::OnScoStateTimeOut()
{
    AUDIO_ERR_LOG("scoCategory: %{public}d state: %{public}d time out",
        currentScoCategory_, currentScoState_);
}

BluetoothRemoteDevice BluetoothScoManager::GetAudioScoDevice()
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return currentScoDevice_;
}

bool BluetoothScoManager::IsInScoCategory(ScoCategory scoCategory)
{
    std::lock_guard<std::mutex> stateLock(scoLock_);
    return (currentScoCategory_ == scoCategory) &&
        (currentScoState_ == AudioScoState::CONNECTING ||
        currentScoState_ == AudioScoState::CONNECTED);
}
} // Bluetooth
} // OHOS