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
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "bluetooth_device_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

AudioScoState BluetoothScoManager::currentScoState_ = AudioScoState::DISCONNECTED;
AudioScoMode BluetoothScoManager::currentScoMode_ = AudioScoMode::DEFAULT_MODE;
AudioScoMode BluetoothScoManager::lastScoMode_ = AudioScoMode::DEFAULT_MODE;
HandsFreeAudioGateway *BluetoothScoManager::hfpInstance_ = HandsFreeAudioGateway::GetProfile();
std::mutex g_scoStateLock;

void BluetoothScoManager::UpdateScoState(HfpScoConnectState scoState, const BluetoothRemoteDevice *device)
{
    std::unique_lock<std::mutex> stateLock(g_scoStateLock);
    AudioScoState lastScoState = currentScoState_;
    AudioScoMode tmpMode = currentScoMode_;
    AUDIO_INFO_LOG("Before UpdateScoState, lastScoState: %{public}d", lastScoState);
    if (scoState == HfpScoConnectState::SCO_CONNECTED) {
        currentScoState_ = AudioScoState::CONNECTED;
        stateLock.unlock();
        if (lastScoState == AudioScoState::DISCONNECT_AFTER_CONNECTED) {
            BluetoothScoManager::HandleScoDisconnect(GetScoCategeryFromMode(currentScoMode_), device);
        } else if (lastScoState == AudioScoState::CONNECT_AFTER_DISCONNECTED) {
            BluetoothScoManager::HandleScoDisconnect(GetScoCategeryFromMode(lastScoMode_), device);
            BluetoothScoManager::HandleScoConnect(GetScoCategeryFromMode(tmpMode), device);
        }
    } else if (scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        currentScoState_ = AudioScoState::DISCONNECTED;
        stateLock.unlock();
        if (lastScoState == AudioScoState::CONNECT_AFTER_DISCONNECTED) {
            BluetoothScoManager::HandleScoConnect(GetScoCategeryFromMode(currentScoMode_), device);
        } else if (lastScoState == AudioScoState::DISCONNECT_AFTER_CONNECTED) {
            BluetoothScoManager::HandleScoConnect(GetScoCategeryFromMode(lastScoMode_), device);
            BluetoothScoManager::HandleScoDisconnect(GetScoCategeryFromMode(tmpMode), device);
        }
    }
}

int32_t BluetoothScoManager::HandleScoConnect(ScoCategory scoCategory, const BluetoothRemoteDevice *device)
{
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    std::lock_guard<std::mutex> stateLock(g_scoStateLock);
    AudioScoState lastScoState = currentScoState_;
    lastScoMode_ = currentScoMode_;
    currentScoMode_ = BluetoothScoManager::GetScoModeFromCategery(scoCategory);
    AUDIO_INFO_LOG("HandleScoConnect, lastScoState: %{public}d, lastScoMode: %{public}d, currentScoMode: %{public}d",
        lastScoState, lastScoMode_, currentScoMode_);
    int32_t ret = ERROR;
    if (lastScoState == AudioScoState::DISCONNECTED) {
        if (currentScoMode_ == AudioScoMode::REC_MODE) {
            CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "HandleScoConnect failed, device is nullptr");
            ret = ((hfpInstance_->OpenVoiceRecognition(*device) == true) ? SUCCESS : ERROR);
        } else {
            ret = hfpInstance_->ConnectSco(static_cast<uint8_t> (scoCategory));
        }
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "HandleScoConnect failed, result: %{public}d", ret);
        currentScoState_ = AudioScoState::CONNECTING;
    } else if (lastScoState == AudioScoState::DISCONNECT_AFTER_CONNECTED && lastScoMode_ == currentScoMode_) {
        currentScoState_ = AudioScoState::CONNECTING;
    } else if (lastScoState == AudioScoState::DISCONNECT_AFTER_CONNECTED && lastScoMode_ != currentScoMode_) {
        currentScoState_ = AudioScoState::CONNECT_AFTER_DISCONNECTED;
    } else if (lastScoState == AudioScoState::DISCONNECTING) {
        currentScoState_ = AudioScoState::CONNECT_AFTER_DISCONNECTED;
    }
    return SUCCESS;
}

int32_t BluetoothScoManager::HandleScoDisconnect(ScoCategory scoCategory, const BluetoothRemoteDevice *device)
{
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    std::lock_guard<std::mutex> stateLock(g_scoStateLock);
    AudioScoState lastScoState = currentScoState_;
    lastScoMode_ = currentScoMode_;
    currentScoMode_ = BluetoothScoManager::GetScoModeFromCategery(scoCategory);
    AUDIO_INFO_LOG("HandleScoDisconnect, lastScoState: %{public}d, lastScoMode: %{public}d, currentScoMode: %{public}d",
        lastScoState, lastScoMode_, currentScoMode_);
    int32_t ret = ERROR;
    if (lastScoState == AudioScoState::CONNECTED) {
        if (currentScoMode_ == AudioScoMode::REC_MODE) {
            CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "HandleScoDisconnect failed, device is nullptr");
            ret = ((hfpInstance_->CloseVoiceRecognition(*device) == true) ? SUCCESS : ERROR);
        } else {
            ret = hfpInstance_->DisconnectSco(static_cast<uint8_t> (scoCategory));
        }
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "HandleScoDisconnect failed, result: %{public}d", ret);
        currentScoState_ = AudioScoState::DISCONNECTING;
    } else if (lastScoState == AudioScoState::CONNECT_AFTER_DISCONNECTED) {
        currentScoState_ = AudioScoState::DISCONNECTING;
    } else if (lastScoState == AudioScoState::CONNECTING) {
        currentScoState_ = AudioScoState::DISCONNECT_AFTER_CONNECTED;
    }
    return SUCCESS;
}

AudioScoState BluetoothScoManager::GetAudioScoState()
{
    return currentScoState_;
}

AudioScoMode BluetoothScoManager::GetScoModeFromCategery(ScoCategory scoCategory)
{
    switch (scoCategory) {
        case ScoCategory::SCO_CALLULAR:
            return AudioScoMode::CALL_MODE;
        case ScoCategory::SCO_VIRTUAL:
            return AudioScoMode::VOIP_MODE;
        case ScoCategory::SCO_RECOGNITION:
            return AudioScoMode::REC_MODE;
        default:
            return AudioScoMode::DEFAULT_MODE;
    }
}

ScoCategory BluetoothScoManager::GetScoCategeryFromMode(AudioScoMode scoMode)
{
    switch (scoMode) {
        case AudioScoMode::CALL_MODE:
            return ScoCategory::SCO_CALLULAR;
        case AudioScoMode::VOIP_MODE:
            return ScoCategory::SCO_VIRTUAL;
        case AudioScoMode::REC_MODE:
            return ScoCategory::SCO_RECOGNITION;
        default:
            return ScoCategory::SCO_DEFAULT;
    }
}

}
}