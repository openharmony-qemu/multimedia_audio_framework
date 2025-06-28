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
#define LOG_TAG "BluetoothHfpWrapInterface"
#endif

#include "bluetooth_hfp_interface.h"
#include <mutex>
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

static HandsFreeAudioGateway *g_hfpInstance = nullptr;

class BluetoothHfpWrapInterface : public BluetoothHfpInterface {
public:
    BluetoothHfpWrapInterface();
    ~BluetoothHfpWrapInterface() override;

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) override;
    AudioScoState GetScoState(const BluetoothRemoteDevice &device) override;
    int32_t GetCurrentCategory(ScoCategory &category) override;
    int32_t ConnectSco(uint8_t callType) override;
    int32_t DisconnectSco(uint8_t callType) override;
    int32_t OpenVoiceRecognition(const BluetoothRemoteDevice &device) override;
    int32_t CloseVoiceRecognition(const BluetoothRemoteDevice &device) override;
    int32_t SetActiveDevice(const BluetoothRemoteDevice &device) override;

    void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer) override;
    void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer) override;
    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states) override;
    void GetVirtualDeviceList(std::vector<std::string> &devices) override;
    BluetoothRemoteDevice GetActiveDevice() override;
    int32_t Connect(const BluetoothRemoteDevice &device) override;
    int32_t IsInbandRingingEnabled(bool &isEnabled) override;

    int32_t GetLastError() override;
    std::string GetLastOpration() override;

private:
    int32_t lastErrno_ = 0;
    std::string lastOpr_;
    std::mutex oprLock_;

    void SetLastOprInfo(int32_t error, const std::string &func);
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

BluetoothHfpWrapInterface::~BluetoothHfpWrapInterface()
{
    g_hfpInstance = nullptr;
}

int32_t BluetoothHfpWrapInterface::GetDeviceState(const BluetoothRemoteDevice &device,
    int32_t &state)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->GetDeviceState(device, state);
    SetLastOprInfo(error, __func__);
    return error;
}

AudioScoState BluetoothHfpWrapInterface::GetScoState(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, AudioScoState::DISCONNECTED, "HFP AG profile unavailable");
    SetLastOprInfo(0, __func__);
    HfpScoConnectState state = static_cast<HfpScoConnectState>(g_hfpInstance->GetScoState(device));
    if (state == HfpScoConnectState::SCO_CONNECTED) {
        return AudioScoState::CONNECTED;
    }
    return AudioScoState::DISCONNECTED;
}

int32_t BluetoothHfpWrapInterface::GetCurrentCategory(ScoCategory &category)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int callType = 0;
    int32_t error = g_hfpInstance->GetCurrentCallType(callType);
    SetLastOprInfo(error, __func__);
    CHECK_AND_RETURN_RET_LOG(error == SUCCESS, ERROR, "GetCurrentCallType failed ret is %{public}d", error);
    category = static_cast<ScoCategory>(callType);
    return SUCCESS;
}

int32_t BluetoothHfpWrapInterface::ConnectSco(uint8_t callType)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->ConnectSco(callType);
    SetLastOprInfo(error, __func__);
    return error;
}

int32_t BluetoothHfpWrapInterface::DisconnectSco(uint8_t callType)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->DisconnectSco(callType);
    SetLastOprInfo(error, __func__);
    return error;
}

int32_t BluetoothHfpWrapInterface::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->OpenVoiceRecognition(device) ? SUCCESS : ERROR;
    SetLastOprInfo(error, __func__);
    return error;
}

int32_t BluetoothHfpWrapInterface::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->CloseVoiceRecognition(device) ? SUCCESS : ERROR;
    SetLastOprInfo(error, __func__);
    return error;
}

int32_t BluetoothHfpWrapInterface::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->SetActiveDevice(device) ? SUCCESS : ERROR;
    SetLastOprInfo(error, __func__);
    return error;
}

void BluetoothHfpWrapInterface::RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
{
    CHECK_AND_RETURN_LOG(g_hfpInstance != nullptr, "HFP AG profile unavailable");
    g_hfpInstance->RegisterObserver(observer);
    SetLastOprInfo(0, __func__);
}

void BluetoothHfpWrapInterface::DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
{
    CHECK_AND_RETURN_LOG(g_hfpInstance != nullptr, "HFP AG profile unavailable");
    g_hfpInstance->DeregisterObserver(observer);
    SetLastOprInfo(0, __func__);
}

std::vector<BluetoothRemoteDevice> BluetoothHfpWrapInterface::GetDevicesByStates(std::vector<int> states)
{
    std::vector<BluetoothRemoteDevice> devices;
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, devices, "HFP AG profile unavailable");
    SetLastOprInfo(0, __func__);
    return g_hfpInstance->GetDevicesByStates(states);
}

void BluetoothHfpWrapInterface::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    CHECK_AND_RETURN_LOG(g_hfpInstance != nullptr, "HFP AG profile unavailable");
    g_hfpInstance->GetVirtualDeviceList(devices);
    SetLastOprInfo(0, __func__);
}

BluetoothRemoteDevice BluetoothHfpWrapInterface::GetActiveDevice()
{
    BluetoothRemoteDevice device;
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, device, "HFP AG profile unavailable");
    SetLastOprInfo(0, __func__);
    return g_hfpInstance->GetActiveDevice();
}

int32_t BluetoothHfpWrapInterface::Connect(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    int32_t error = g_hfpInstance->Connect(device);
    SetLastOprInfo(error, __func__);
    return error;
}

int32_t BluetoothHfpWrapInterface::IsInbandRingingEnabled(bool &isEnabled)
{
    CHECK_AND_RETURN_RET_LOG(g_hfpInstance != nullptr, ERROR, "HFP AG profile unavailable");
    return g_hfpInstance->IsInbandRingingEnabled(isEnabled);
}

void BluetoothHfpWrapInterface::SetLastOprInfo(int32_t error, const std::string &func)
{
    std::unique_lock<std::mutex> lock(oprLock_);
    lastOpr_ = func;
    lastErrno_ = error;
}

int32_t BluetoothHfpWrapInterface::GetLastError()
{
    std::unique_lock<std::mutex> lock(oprLock_);
    return lastErrno_;
}

std::string BluetoothHfpWrapInterface::GetLastOpration()
{
    std::unique_lock<std::mutex> lock(oprLock_);
    return lastOpr_;
}
} // Bluetooth
} // OHOS