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
#define LOG_TAG "BluetoothA2dpWrapInterface"
#endif

#include "bluetooth_a2dp_interface.h"
#include <mutex>
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

static A2dpSource *g_a2dpInstance = nullptr;

class BluetoothA2dpWrapInterface : public BluetoothA2dpInterface {
public:
    BluetoothA2dpWrapInterface();
    ~BluetoothA2dpWrapInterface() override;

    int32_t GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) override;
    int32_t GetRenderPosition(const BluetoothRemoteDevice &device, uint32_t &delayValue, uint64_t &sendDataSize,
        uint32_t &timestamp) override;
    int32_t OffloadStopPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionId) override;
    int32_t OffloadStartPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionId) override;
    int32_t A2dpOffloadSessionRequest(const BluetoothRemoteDevice &device,
        const std::vector<A2dpStreamInfo> &info) override;
    int32_t SetActiveSinkDevice(const BluetoothRemoteDevice &device) override;
    int32_t Connect(const BluetoothRemoteDevice &device) override;
    void RegisterObserver(std::shared_ptr<A2dpSourceObserver> observer) override;
    void DeregisterObserver(std::shared_ptr<A2dpSourceObserver> observer) override;
    void GetVirtualDeviceList(std::vector<std::string> &devices) override;
    A2dpCodecStatus GetCodecStatus(const BluetoothRemoteDevice &device) override;
    std::string GetActiveA2dpDevice() override;

    int32_t GetLastError() override;
    std::string GetLastOperation() override;

private:
    int32_t lastErrno_ = 0;
    std::string lastOpr_;
    std::mutex oprLock_;

    void SetLastOprInfo(int32_t error, const std::string &func);
};

BluetoothA2dpInterface &BluetoothA2dpInterface::GetInstance()
{
    static BluetoothA2dpWrapInterface interface;
    return interface;
}

BluetoothA2dpWrapInterface::BluetoothA2dpWrapInterface()
{
    if (g_a2dpInstance == nullptr) {
        g_a2dpInstance = A2dpSource::GetProfile();
    }
}

BluetoothA2dpWrapInterface::~BluetoothA2dpWrapInterface()
{
    g_a2dpInstance = nullptr;
}

int32_t BluetoothA2dpWrapInterface::GetDevicesByStates(const std::vector<int> &states,
    std::vector<BluetoothRemoteDevice> &devices)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->GetDevicesByStates(states, devices);
    SetLastOprInfo(ret, __func__);
    return ret;
}

int32_t BluetoothA2dpWrapInterface::GetRenderPosition(const BluetoothRemoteDevice &device, uint32_t &delayValue,
    uint64_t &sendDataSize, uint32_t &timestamp)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->GetRenderPosition(device, delayValue, sendDataSize, timestamp);
    SetLastOprInfo(ret, __func__);
    return ret;
}

int32_t BluetoothA2dpWrapInterface::OffloadStopPlaying(const BluetoothRemoteDevice &device,
    const std::vector<int32_t> &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->OffloadStopPlaying(device, sessionId);
    SetLastOprInfo(ret, __func__);
    return ret;
}

int32_t BluetoothA2dpWrapInterface::OffloadStartPlaying(const BluetoothRemoteDevice &device,
    const std::vector<int32_t> &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->OffloadStartPlaying(device, sessionId);
    SetLastOprInfo(ret, __func__);
    return ret;
}

int32_t BluetoothA2dpWrapInterface::A2dpOffloadSessionRequest(const BluetoothRemoteDevice &device,
    const std::vector<A2dpStreamInfo> &info)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->A2dpOffloadSessionRequest(device, info);
    SetLastOprInfo(ret, __func__);
    return ret;
}

int32_t BluetoothA2dpWrapInterface::SetActiveSinkDevice(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->SetActiveSinkDevice(device);
    SetLastOprInfo(ret, __func__);
    return ret;
}

int32_t BluetoothA2dpWrapInterface::Connect(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, ERROR, "A2DP profile unavailable");
    int32_t ret = g_a2dpInstance->Connect(device);
    SetLastOprInfo(ret, __func__);
    return ret;
}

void BluetoothA2dpWrapInterface::RegisterObserver(std::shared_ptr<A2dpSourceObserver> observer)
{
    CHECK_AND_RETURN_LOG(g_a2dpInstance != nullptr, "A2DP profile unavailable");
    g_a2dpInstance->RegisterObserver(observer);
    SetLastOprInfo(0, __func__);
}

void BluetoothA2dpWrapInterface::DeregisterObserver(std::shared_ptr<A2dpSourceObserver> observer)
{
    CHECK_AND_RETURN_LOG(g_a2dpInstance != nullptr, "A2DP profile unavailable");
    g_a2dpInstance->DeregisterObserver(observer);
    SetLastOprInfo(0, __func__);
}

void BluetoothA2dpWrapInterface::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    CHECK_AND_RETURN_LOG(g_a2dpInstance != nullptr, "A2DP profile unavailable");
    g_a2dpInstance->GetVirtualDeviceList(devices);
    SetLastOprInfo(0, __func__);
}

A2dpCodecStatus BluetoothA2dpWrapInterface::GetCodecStatus(const BluetoothRemoteDevice &device)
{
    A2dpCodecStatus codecStatus;
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, codecStatus, "A2DP profile unavailable");
    SetLastOprInfo(0, __func__);
    return g_a2dpInstance->GetCodecStatus(device);
}

std::string BluetoothA2dpWrapInterface::GetActiveA2dpDevice()
{
    CHECK_AND_RETURN_RET_LOG(g_a2dpInstance != nullptr, "", "A2DP profile unavailable");
    SetLastOprInfo(0, __func__);
    return g_a2dpInstance->GetActiveSinkDevice().GetDeviceAddr();
}

void BluetoothA2dpWrapInterface::SetLastOprInfo(int32_t error, const std::string &func)
{
    std::unique_lock<std::mutex> lock(oprLock_);
    lastOpr_ = func;
    lastErrno_ = error;
}

int32_t BluetoothA2dpWrapInterface::GetLastError()
{
    std::unique_lock<std::mutex> lock(oprLock_);
    return lastErrno_;
}

std::string BluetoothA2dpWrapInterface::GetLastOperation()
{
    std::unique_lock<std::mutex> lock(oprLock_);
    return lastOpr_;
}
} // Bluetooth
} // OHOS