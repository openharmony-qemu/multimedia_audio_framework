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

#include "bluetooth_hfp_interface.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

static HandsFreeAudioGateway *g_hfpInstance = nullptr;

class BluetoothHfpWrapInterface : public BluetoothHfpInterface {
public:
    BluetoothHfpWrapInterface();
    ~BluetoothHfpWrapInterface() = default;

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) override;
    int32_t GetScoState(const BluetoothRemoteDevice &device) override;
    int32_t ConnectSco(uint8_t callType) override;
    int32_t DisconnectSco(uint8_t callType) override;
    int32_t OpenVoiceRecognition(const BluetoothRemoteDevice &device) override;
    int32_t CloseVoiceRecognition(const BluetoothRemoteDevice &device) override;
    int32_t SetActiveDevice(const BluetoothRemoteDevice &device) override;
};

BluetoothHfpInterface &BluetoothHfpInterface::GetInstance()
{
    static BluetoothHfpWrapInterface interface;
    return interface;
}

BluetoothHfpWrapInterface::BluetoothHfpWrapInterface()
{
    if (g_hfpInstance == nullptr) {
        g_hfpInstance = HandsFreeAudioGateway::GetProfile();
    }
}

int32_t BluetoothHfpWrapInterface::GetDeviceState(const BluetoothRemoteDevice &device,
    int32_t &state)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->GetDeviceState(device, state);
}

int32_t BluetoothHfpWrapInterface::GetScoState(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->GetScoState(device);
}

int32_t BluetoothHfpWrapInterface::ConnectSco(uint8_t callType)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->ConnectSco(callType);
}

int32_t BluetoothHfpWrapInterface::DisconnectSco(uint8_t callType)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->DisconnectSco(callType);
}

int32_t BluetoothHfpWrapInterface::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->OpenVoiceRecognition(device) ? SUCCESS : ERROR;
}

int32_t BluetoothHfpWrapInterface::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->CloseVoiceRecognition(device) ? SUCCESS : ERROR;
}

int32_t BluetoothHfpWrapInterface::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->SetActiveDevice(device) ? SUCCESS : ERROR;
}
}
}