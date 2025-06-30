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
#define LOG_TAG "BluetoothDeviceManager"
#endif

#include "bluetooth_device_manager.h"

#include <thread>

#include "bluetooth_audio_manager.h"
#include "audio_bluetooth_manager.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

const int WEAR_ENABLED = 1;
const int DEFAULT_COD = -1;
const int DEFAULT_MAJOR_CLASS = -1;
const int DEFAULT_MAJOR_MINOR_CLASS = -1;
const int A2DP_DEFAULT_SELECTION = -1;
const int HFP_DEFAULT_SELECTION = -1;
const int USER_SELECTION = 1;
const std::map<std::pair<int, int>, DeviceCategory> bluetoothDeviceCategoryMap_ = {
    {std::make_pair(BluetoothDevice::MAJOR_AUDIO_VIDEO, BluetoothDevice::AUDIO_VIDEO_HEADPHONES), BT_HEADPHONE},
    {std::make_pair(BluetoothDevice::MAJOR_AUDIO_VIDEO, BluetoothDevice::AUDIO_VIDEO_WEARABLE_HEADSET), BT_HEADPHONE},
    {std::make_pair(BluetoothDevice::MAJOR_AUDIO_VIDEO, BluetoothDevice::AUDIO_VIDEO_LOUDSPEAKER), BT_SOUNDBOX},
    {std::make_pair(BluetoothDevice::MAJOR_AUDIO_VIDEO, BluetoothDevice::AUDIO_VIDEO_HANDSFREE), BT_CAR},
    {std::make_pair(BluetoothDevice::MAJOR_AUDIO_VIDEO, BluetoothDevice::AUDIO_VIDEO_CAR_AUDIO), BT_CAR},
    {std::make_pair(BluetoothDevice::MAJOR_WEARABLE, BluetoothDevice::WEARABLE_GLASSES), BT_GLASSES},
    {std::make_pair(BluetoothDevice::MAJOR_WEARABLE, BluetoothDevice::WEARABLE_WRIST_WATCH), BT_WATCH},
};
IDeviceStatusObserver *g_deviceObserver = nullptr;
std::mutex g_observerLock;
std::mutex g_a2dpDeviceLock;
std::mutex g_a2dpDeviceMapLock;
std::mutex g_a2dpWearStateMapLock;
std::map<std::string, BluetoothRemoteDevice> MediaBluetoothDeviceManager::a2dpBluetoothDeviceMap_;
std::map<std::string, BluetoothDeviceAction> MediaBluetoothDeviceManager::wearDetectionStateMap_;
std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::privacyDevices_;
std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::commonDevices_;
std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::negativeDevices_;
std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::connectingDevices_;
std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::virtualDevices_;
std::mutex g_a2dpInDeviceMapLock;
std::mutex g_a2dpInStreamInfoMapLock;
std::map<std::string, BluetoothRemoteDevice> A2dpInBluetoothDeviceManager::a2dpInBluetoothDeviceMap_;
std::map<std::string, AudioStreamInfo> A2dpInBluetoothDeviceManager::a2dpInStreamInfoMap_;
std::mutex g_hfpDeviceLock;
std::mutex g_hfpDeviceMapLock;
std::mutex g_hfpWearStateMapLock;
std::map<std::string, BluetoothRemoteDevice> HfpBluetoothDeviceManager::hfpBluetoothDeviceMap_;
std::map<std::string, BluetoothDeviceAction> HfpBluetoothDeviceManager::wearDetectionStateMap_;
std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::privacyDevices_;
std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::commonDevices_;
std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::negativeDevices_;
std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::connectingDevices_;
std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::virtualDevices_;
std::mutex HfpBluetoothDeviceManager::stopVirtualCallHandleLock_;
HfpBluetoothDeviceManager::DisconnectScoForDevice HfpBluetoothDeviceManager::disconnectScoFun_ = nullptr;
BluetoothStopVirtualCallHandle HfpBluetoothDeviceManager::stopVirtualCallHandle_ = { BluetoothRemoteDevice(), false};

int32_t RegisterDeviceObserver(IDeviceStatusObserver &observer)
{
    std::lock_guard<std::mutex> deviceLock(g_observerLock);
    g_deviceObserver = &observer;
    return SUCCESS;
}

void UnregisterDeviceObserver()
{
    std::lock_guard<std::mutex> deviceLock(g_observerLock);
    g_deviceObserver = nullptr;
}

void SendUserSelectionEvent(AudioStandard::DeviceType devType, const std::string &macAddress, int32_t eventType)
{
    AUDIO_INFO_LOG("devType is %{public}d, eventType is%{public}d.", devType, eventType);
    BluetoothRemoteDevice device;
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        if (MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device) != SUCCESS) {
            AUDIO_ERR_LOG("failed for the device is not connected.");
            return;
        }
        BluetoothAudioManager::GetInstance().SendDeviceSelection(device, eventType,
            HFP_DEFAULT_SELECTION, USER_SELECTION);
    } else if (devType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        if (A2dpInBluetoothDeviceManager::IsA2dpInBluetoothDeviceExist(macAddress) != SUCCESS) {
            AUDIO_ERR_LOG("failed for the device is not connected.");
            return;
        }
        BluetoothAudioManager::GetInstance().SendDeviceSelection(device, eventType,
            HFP_DEFAULT_SELECTION, USER_SELECTION);
    } else if (devType == DEVICE_TYPE_BLUETOOTH_SCO) {
        if (HfpBluetoothDeviceManager::GetConnectedHfpBluetoothDevice(macAddress, device) != SUCCESS) {
            AUDIO_ERR_LOG("failed for the device is not connected.");
            return;
        }
        BluetoothAudioManager::GetInstance().SendDeviceSelection(device, A2DP_DEFAULT_SELECTION,
            eventType, USER_SELECTION);
    } else {
        AUDIO_ERR_LOG("failed for the devType is not Bluetooth type.");
    }
}

bool IsBTWearDetectionEnable(const BluetoothRemoteDevice &device)
{
    int32_t wearEnabledAbility = 0;
    bool isWearSupported = false;
    BluetoothAudioManager::GetInstance().GetWearDetectionState(device.GetDeviceAddr(), wearEnabledAbility);
    BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device.GetDeviceAddr(), isWearSupported);
    AUDIO_INFO_LOG("wear detection on-off state: %{public}d, wear detection support state: %{public}d",
        wearEnabledAbility, isWearSupported);
    return (wearEnabledAbility == WEAR_ENABLED && isWearSupported);
}

DeviceCategory GetDeviceCategory(const BluetoothRemoteDevice &device)
{
    int cod = DEFAULT_COD;
    int majorClass = DEFAULT_MAJOR_CLASS;
    int majorMinorClass = DEFAULT_MAJOR_MINOR_CLASS;
    device.GetDeviceProductType(cod, majorClass, majorMinorClass);
    AUDIO_WARNING_LOG("Device type majorClass: %{public}d, majorMinorClass: %{public}d.", majorClass, majorMinorClass);
    DeviceCategory bluetoothCategory = CATEGORY_DEFAULT;
    auto pos = bluetoothDeviceCategoryMap_.find(std::make_pair(majorClass, majorMinorClass));
    if (pos != bluetoothDeviceCategoryMap_.end()) {
        bluetoothCategory = pos->second;
        AUDIO_WARNING_LOG("Bluetooth category is: %{public}d", bluetoothCategory);
    }
    return bluetoothCategory;
}

void MediaBluetoothDeviceManager::SetMediaStack(const BluetoothRemoteDevice &device, int action)
{
    switch (action) {
        case BluetoothDeviceAction::CONNECTING_ACTION:
            HandleConnectingDevice(device);
            break;
        case BluetoothDeviceAction::CONNECT_ACTION:
            HandleConnectDevice(device);
            break;
        case BluetoothDeviceAction::DISCONNECT_ACTION:
            HandleDisconnectDevice(device);
            break;
        case BluetoothDeviceAction::WEAR_ACTION:
            HandleWearDevice(device);
            break;
        case BluetoothDeviceAction::UNWEAR_ACTION:
            HandleUnwearDevice(device);
            break;
        case BluetoothDeviceAction::ENABLEFROMREMOTE_ACTION:
            HandleEnableDevice(device);
            break;
        case BluetoothDeviceAction::DISABLEFROMREMOTE_ACTION:
            HandleDisableDevice(device);
            break;
        case BluetoothDeviceAction::ENABLE_WEAR_DETECTION_ACTION:
            HandleWearEnable(device);
            break;
        case BluetoothDeviceAction::DISABLE_WEAR_DETECTION_ACTION:
            HandleWearDisable(device);
            break;
        case BluetoothDeviceAction::USER_SELECTION_ACTION:
            HandleUserSelection(device);
            break;
        case BluetoothDeviceAction::VIRTUAL_DEVICE_ADD_ACTION:
            HandleVirtualConnectDevice(device);
            break;
        case BluetoothDeviceAction::VIRTUAL_DEVICE_REMOVE_ACTION:
            HandleRemoveVirtualConnectDevice(device);
            break;
        case BluetoothDeviceAction::CATEGORY_UPDATE_ACTION:
            HandleUpdateDeviceCategory(device);
            break;
        default:
            AUDIO_ERR_LOG("SetMediaStack failed due to the unknow action: %{public}d", action);
            break;
    }
}

void MediaBluetoothDeviceManager::HandleConnectingDevice(const BluetoothRemoteDevice &device)
{
    if (IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        return;
    }
    AddDeviceInConfigVector(device, connectingDevices_);
}

void MediaBluetoothDeviceManager::HandleConnectDevice(const BluetoothRemoteDevice &device)
{
    if (IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        return;
    }
    AudioDeviceDescriptor desc = HandleConnectDeviceInner(device);
    NotifyToUpdateAudioDevice(device, desc, DeviceStatus::ADD);
}

AudioDeviceDescriptor MediaBluetoothDeviceManager::HandleConnectDeviceInner(const BluetoothRemoteDevice &device)
{
    RemoveDeviceInConfigVector(device, connectingDevices_);
    // If the device was virtual connected, remove it from the negativeDevices_ list.
    RemoveDeviceInConfigVector(device, negativeDevices_);
    DeviceCategory bluetoothCategory = GetDeviceCategory(device);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = bluetoothCategory;
    switch (bluetoothCategory) {
        case BT_GLASSES:
        case BT_HEADPHONE:
            if (IsBTWearDetectionEnable(device)) {
                AddDeviceInConfigVector(device, negativeDevices_);
                desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
            } else {
                AddDeviceInConfigVector(device, privacyDevices_);
            }
            break;
        case BT_SOUNDBOX:
        case BT_CAR:
            AddDeviceInConfigVector(device, commonDevices_);
            break;
        case BT_WATCH:
            AddDeviceInConfigVector(device, negativeDevices_);
            break;
        default:
            AUDIO_WARNING_LOG("Unknow BT category, regard as bluetooth headset.");
            AddDeviceInConfigVector(device, privacyDevices_);
            desc.deviceCategory_ = BT_HEADPHONE;
            break;
    }
    return desc;
}

void MediaBluetoothDeviceManager::HandleDisconnectDevice(const BluetoothRemoteDevice &device)
{
    RemoveDeviceInConfigVector(device, connectingDevices_);
    if (!IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_INFO_LOG("The device is already disconnected, ignore disconnect action.");
        return;
    }
    RemoveDeviceInConfigVector(device, privacyDevices_);
    RemoveDeviceInConfigVector(device, commonDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_a2dpWearStateMapLock);
        wearDetectionStateMap_.erase(device.GetDeviceAddr());
    }
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    NotifyToUpdateAudioDevice(device, desc, DeviceStatus::REMOVE);
}

void MediaBluetoothDeviceManager::HandleWearDevice(const BluetoothRemoteDevice &device)
{
    bool isDeviceExist = IsA2dpBluetoothDeviceExist(device.GetDeviceAddr());
    CHECK_AND_RETURN_LOG(isDeviceExist,
        "HandleWearDevice failed for the device has not be reported the connected action.");
    RemoveDeviceInConfigVector(device, negativeDevices_);
    RemoveDeviceInConfigVector(device, privacyDevices_);
    AddDeviceInConfigVector(device, privacyDevices_);
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_a2dpWearStateMapLock);
        wearDetectionStateMap_[device.GetDeviceAddr()] = BluetoothDeviceAction::WEAR_ACTION;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = BT_HEADPHONE;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    }
}

void MediaBluetoothDeviceManager::HandleUnwearDevice(const BluetoothRemoteDevice &device)
{
    bool isDeviceExist = IsA2dpBluetoothDeviceExist(device.GetDeviceAddr());
    CHECK_AND_RETURN_LOG(isDeviceExist, "HandleWearDevice failed for the device has not worn.");
    RemoveDeviceInConfigVector(device, privacyDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    AddDeviceInConfigVector(device, negativeDevices_);
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_a2dpWearStateMapLock);
        wearDetectionStateMap_[device.GetDeviceAddr()] = BluetoothDeviceAction::UNWEAR_ACTION;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    }
}

void MediaBluetoothDeviceManager::HandleEnableDevice(const BluetoothRemoteDevice &device)
{
    if (!IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleEnableDevice failed for the device has not connected.");
        return;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.isEnable_ = true;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::ENABLE_UPDATE);
    }
}

void MediaBluetoothDeviceManager::HandleDisableDevice(const BluetoothRemoteDevice &device)
{
    if (!IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleDisableDevice failed for the device has not connected.");
        return;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.isEnable_ = false;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::ENABLE_UPDATE);
    }
}

void MediaBluetoothDeviceManager::HandleWearEnable(const BluetoothRemoteDevice &device)
{
    if (!IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleWearEnable failed for the device has not connected.");
        return;
    }
    RemoveDeviceInConfigVector(device, negativeDevices_);
    RemoveDeviceInConfigVector(device, privacyDevices_);
    std::lock_guard<std::mutex> wearStateMapLock(g_a2dpWearStateMapLock);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    auto wearStateIter = wearDetectionStateMap_.find(device.GetDeviceAddr());
    if (wearStateIter != wearDetectionStateMap_.end() &&
        wearStateIter->second == BluetoothDeviceAction::WEAR_ACTION) {
        AddDeviceInConfigVector(device, privacyDevices_);
        desc.deviceCategory_ = BT_HEADPHONE;
    } else {
        AddDeviceInConfigVector(device, negativeDevices_);
        desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    }
}

void MediaBluetoothDeviceManager::HandleWearDisable(const BluetoothRemoteDevice &device)
{
    if (!IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleWearDisable failed for the device has not connected.");
        return;
    }
    RemoveDeviceInConfigVector(device, privacyDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    AddDeviceInConfigVector(device, privacyDevices_);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = BT_HEADPHONE;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    }
}

void MediaBluetoothDeviceManager::HandleUserSelection(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnForcedDeviceSelected(DEVICE_TYPE_BLUETOOTH_A2DP, device.GetDeviceAddr());
    }
}

void MediaBluetoothDeviceManager::HandleVirtualConnectDevice(const BluetoothRemoteDevice &device)
{
    AddDeviceInConfigVector(device, virtualDevices_);
    DeviceCategory bluetoothCategory = GetDeviceCategory(device);
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = bluetoothCategory;
    AddDeviceInConfigVector(device, negativeDevices_);
    NotifyToUpdateVirtualDevice(device, desc, DeviceStatus::VIRTUAL_ADD);
}

void MediaBluetoothDeviceManager::HandleRemoveVirtualConnectDevice(const BluetoothRemoteDevice &device)
{
    RemoveDeviceInConfigVector(device, virtualDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    NotifyToUpdateVirtualDevice(device, desc, DeviceStatus::VIRTUAL_REMOVE);
}

void MediaBluetoothDeviceManager::AddDeviceInConfigVector(const BluetoothRemoteDevice &device,
    std::vector<BluetoothRemoteDevice> &deviceVector)
{
    std::lock_guard<std::mutex> a2dpDeviceLock(g_a2dpDeviceLock);
    auto isPresent = [&device] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
        return device.GetDeviceAddr() == bluetoothRemoteDevice.GetDeviceAddr();
    };
    auto deviceIter = std::find_if(deviceVector.begin(), deviceVector.end(), isPresent);
    if (deviceIter == deviceVector.end()) {
        deviceVector.push_back(device);
    }
}

void MediaBluetoothDeviceManager::RemoveDeviceInConfigVector(const BluetoothRemoteDevice &device,
    std::vector<BluetoothRemoteDevice> &deviceVector)
{
    std::lock_guard<std::mutex> a2dpDeviceLock(g_a2dpDeviceLock);
    auto isPresent = [&device] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
        return device.GetDeviceAddr() == bluetoothRemoteDevice.GetDeviceAddr();
    };
    auto deviceIter = std::find_if(deviceVector.begin(), deviceVector.end(), isPresent);
    if (deviceIter != deviceVector.end()) {
        deviceVector.erase(deviceIter);
    }
}

void MediaBluetoothDeviceManager::NotifyToUpdateAudioDevice(const BluetoothRemoteDevice &device,
    AudioDeviceDescriptor &desc, DeviceStatus deviceStatus)
{
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceName_ = device.GetDeviceName();
    desc.connectState_ = ConnectState::CONNECTED;
    AUDIO_WARNING_LOG("a2dpBluetoothDeviceMap_ operation: %{public}d new bluetooth device, device address\
        is %{public}s, category is %{public}d", deviceStatus,
        GetEncryptAddr(device.GetDeviceAddr()).c_str(), desc.deviceCategory_);
    {
        std::lock_guard<std::mutex> deviceMapLock(g_a2dpDeviceMapLock);
        if (deviceStatus == DeviceStatus::ADD) {
            a2dpBluetoothDeviceMap_[device.GetDeviceAddr()] = device;
        } else if (deviceStatus == DeviceStatus::REMOVE) {
            if (a2dpBluetoothDeviceMap_.find(device.GetDeviceAddr()) != a2dpBluetoothDeviceMap_.end()) {
                a2dpBluetoothDeviceMap_.erase(device.GetDeviceAddr());
            }
        }
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    CHECK_AND_RETURN_LOG(g_deviceObserver != nullptr, "NotifyToUpdateAudioDevice, device observer is null");
    bool isConnected = deviceStatus == DeviceStatus::ADD;
    g_deviceObserver->OnDeviceStatusUpdated(desc, isConnected);
}

void MediaBluetoothDeviceManager::HandleUpdateDeviceCategory(const BluetoothRemoteDevice &device)
{
    if (!IsA2dpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_WARNING_LOG("HandleUpdateDeviceCategory failed for the device has not be reported the connected action.");
        return;
    }
    AudioDeviceDescriptor desc = HandleConnectDeviceInner(device);
    int32_t wearState = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
    if (wearState == 1 && desc.deviceCategory_ == BT_UNWEAR_HEADPHONE) { // 1 wear state
        desc.deviceCategory_ = BT_HEADPHONE;
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    }
}

void MediaBluetoothDeviceManager::NotifyToUpdateVirtualDevice(const BluetoothRemoteDevice &device,
    AudioDeviceDescriptor &desc, DeviceStatus deviceStatus)
{
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceName_ = device.GetDeviceName();
    desc.connectState_ = ConnectState::VIRTUAL_CONNECTED;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    CHECK_AND_RETURN_LOG(g_deviceObserver != nullptr, "NotifyToUpdateVirtualDevice, device observer is null");
    bool isConnected = deviceStatus == DeviceStatus::VIRTUAL_ADD;
    g_deviceObserver->OnDeviceStatusUpdated(desc, isConnected);
}

bool MediaBluetoothDeviceManager::IsA2dpBluetoothDeviceExist(const std::string& macAddress)
{
    std::lock_guard<std::mutex> deviceMapLock(g_a2dpDeviceMapLock);
    if (a2dpBluetoothDeviceMap_.find(macAddress) != a2dpBluetoothDeviceMap_.end()) {
        return true;
    }
    return false;
}

bool MediaBluetoothDeviceManager::IsA2dpBluetoothDeviceConnecting(const std::string& macAddress)
{
    std::lock_guard<std::mutex> a2dpDeviceLock(g_a2dpDeviceLock);
    auto deviceIter = std::find_if(connectingDevices_.begin(), connectingDevices_.end(),
        [&macAddress] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
            return macAddress == bluetoothRemoteDevice.GetDeviceAddr();
        });
    if (deviceIter != connectingDevices_.end()) {
        return true;
    }
    return false;
}

int32_t MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(const std::string& macAddress,
    BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> deviceMapLock(g_a2dpDeviceMapLock);
    auto deviceIter = a2dpBluetoothDeviceMap_.find(macAddress);
    if (deviceIter != a2dpBluetoothDeviceMap_.end()) {
        device = deviceIter->second;
        return SUCCESS;
    }
    return ERROR;
}

std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::GetAllA2dpBluetoothDevice()
{
    std::lock_guard<std::mutex> deviceMapLock(g_a2dpDeviceMapLock);
    std::vector<BluetoothRemoteDevice> a2dpList = {};
    a2dpList.reserve(a2dpBluetoothDeviceMap_.size());
    for (const auto &[macaddr, device] : a2dpBluetoothDeviceMap_) {
        a2dpList.emplace_back(device);
    }
    return a2dpList;
}

void MediaBluetoothDeviceManager::UpdateA2dpDeviceConfiguration(const BluetoothRemoteDevice &device,
    const AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    CHECK_AND_RETURN_LOG(g_deviceObserver != nullptr, "UpdateA2dpDeviceConfiguration, device observer is null");
    g_deviceObserver->OnDeviceConfigurationChanged(DEVICE_TYPE_BLUETOOTH_A2DP, device.GetDeviceAddr(),
        device.GetDeviceName(), streamInfo);
}

void MediaBluetoothDeviceManager::ClearAllA2dpBluetoothDevice()
{
    AUDIO_INFO_LOG("Bluetooth service crashed and enter the ClearAllA2dpBluetoothDevice.");
    {
        std::lock_guard<std::mutex> a2dpDeviceLock(g_a2dpDeviceLock);
        privacyDevices_.clear();
        commonDevices_.clear();
        negativeDevices_.clear();
        connectingDevices_.clear();
        virtualDevices_.clear();
    }
    std::lock_guard<std::mutex> deviceMapLock(g_a2dpDeviceMapLock);
    std::lock_guard<std::mutex> wearStateMapLock(g_a2dpWearStateMapLock);
    a2dpBluetoothDeviceMap_.clear();
    wearDetectionStateMap_.clear();
}

std::vector<BluetoothRemoteDevice> MediaBluetoothDeviceManager::GetA2dpVirtualDeviceList()
{
    std::lock_guard<std::mutex> a2dpDeviceLock(g_hfpDeviceLock);
    return virtualDevices_;
}

void A2dpInBluetoothDeviceManager::SetA2dpInStack(const BluetoothRemoteDevice &device,
    const AudioStreamInfo &streamInfo, int32_t action)
{
    switch (action) {
        case BluetoothDeviceAction::CONNECT_ACTION:
            HandleConnectDevice(device, streamInfo);
            break;
        case BluetoothDeviceAction::DISCONNECT_ACTION:
            HandleDisconnectDevice(device, streamInfo);
            break;
        default:
            AUDIO_ERR_LOG("SetA2dpInStack failed due to the unknow action: %{public}d", action);
            break;
    }
}

void A2dpInBluetoothDeviceManager::HandleConnectDevice(const BluetoothRemoteDevice &device,
    const AudioStreamInfo &streamInfo)
{
    if (IsA2dpInBluetoothDeviceExist(device.GetDeviceAddr())) {
        return;
    }
    DeviceCategory bluetoothCategory = GetDeviceCategory(device);
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = bluetoothCategory;
    switch (bluetoothCategory) {
        case BT_HEADPHONE:
        case BT_GLASSES:
        case BT_SOUNDBOX:
        case BT_CAR:
        case BT_WATCH:
            break;
        default:
            AUDIO_INFO_LOG("Unknow BT category, regard as bluetooth headset.");
            desc.deviceCategory_ = BT_HEADPHONE;
            break;
    }
    NotifyToUpdateAudioDevice(device, streamInfo, desc, DeviceStatus::ADD);
}

void A2dpInBluetoothDeviceManager::HandleDisconnectDevice(const BluetoothRemoteDevice &device,
    const AudioStreamInfo &streamInfo)
{
    if (!IsA2dpInBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_WARNING_LOG("The device is already disconnected, ignore disconnect action.");
        return;
    }
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    NotifyToUpdateAudioDevice(device, streamInfo, desc, DeviceStatus::REMOVE);
}

void A2dpInBluetoothDeviceManager::NotifyToUpdateAudioDevice(const BluetoothRemoteDevice &device,
    const AudioStreamInfo &streamInfo, AudioDeviceDescriptor &desc, DeviceStatus deviceStatus)
{
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    desc.deviceRole_ = DeviceRole::INPUT_DEVICE;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceName_ = device.GetDeviceName();
    desc.connectState_ = ConnectState::CONNECTED;
    AUDIO_INFO_LOG("a2dpInBluetoothDeviceMap_ operation: %{public}d new bluetooth device, device address is %{public}s,\
        category is %{public}d", deviceStatus, GetEncryptAddr(device.GetDeviceAddr()).c_str(), desc.deviceCategory_);
    {
        std::lock_guard<std::mutex> a2dpInDeviceMapLock(g_a2dpInDeviceMapLock);
        if (deviceStatus == DeviceStatus::ADD) {
            a2dpInBluetoothDeviceMap_[device.GetDeviceAddr()] = device;
        } else if (deviceStatus == DeviceStatus::REMOVE) {
            if (a2dpInBluetoothDeviceMap_.find(device.GetDeviceAddr()) != a2dpInBluetoothDeviceMap_.end()) {
                a2dpInBluetoothDeviceMap_.erase(device.GetDeviceAddr());
            }
        }
    }
    {
        std::lock_guard<std::mutex> a2dpInStreamInfoMapLock(g_a2dpInStreamInfoMapLock);
        if (deviceStatus == DeviceStatus::ADD) {
            a2dpInStreamInfoMap_[device.GetDeviceAddr()] = streamInfo;
        } else if (deviceStatus == DeviceStatus::REMOVE) {
            if (a2dpInStreamInfoMap_.find(device.GetDeviceAddr()) != a2dpInStreamInfoMap_.end()) {
                a2dpInStreamInfoMap_.erase(device.GetDeviceAddr());
            }
        }
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    CHECK_AND_RETURN_LOG(g_deviceObserver != nullptr, "NotifyToUpdateAudioDevice, device observer is null");
    bool isConnected = deviceStatus == DeviceStatus::ADD;
    g_deviceObserver->OnDeviceStatusUpdated(desc, isConnected);
}

bool A2dpInBluetoothDeviceManager::GetA2dpInDeviceStreamInfo(const std::string& macAddress, AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::mutex> a2dpInStreamInfoMapLock(g_a2dpInStreamInfoMapLock);
    auto it = a2dpInStreamInfoMap_.find(macAddress);
    if (it != a2dpInStreamInfoMap_.end()) {
        streamInfo = it->second;
        return true;
    }
    return false;
}

bool A2dpInBluetoothDeviceManager::IsA2dpInBluetoothDeviceExist(const std::string& macAddress)
{
    std::lock_guard<std::mutex> a2dpInDeviceMapLock(g_a2dpInDeviceMapLock);
    if (a2dpInBluetoothDeviceMap_.find(macAddress) != a2dpInBluetoothDeviceMap_.end()) {
        return true;
    }
    return false;
}

std::vector<BluetoothRemoteDevice> A2dpInBluetoothDeviceManager::GetAllA2dpInBluetoothDevice()
{
    std::lock_guard<std::mutex> a2dpInDeviceMapLock(g_a2dpInDeviceMapLock);
    std::vector<BluetoothRemoteDevice> a2dpInList = {};
    a2dpInList.reserve(a2dpInBluetoothDeviceMap_.size());
    for (const auto &[macaddr, device] : a2dpInBluetoothDeviceMap_) {
        a2dpInList.emplace_back(device);
    }
    return a2dpInList;
}

void A2dpInBluetoothDeviceManager::ClearAllA2dpInBluetoothDevice()
{
    AUDIO_INFO_LOG("Bluetooth service crashed and enter the ClearAllA2dpInBluetoothDevice.");
    std::lock_guard<std::mutex> a2dpInDeviceMapLock(g_a2dpInDeviceMapLock);
    a2dpInBluetoothDeviceMap_.clear();
}

void A2dpInBluetoothDeviceManager::ClearAllA2dpInStreamInfo()
{
    std::lock_guard<std::mutex> a2dpInStreamMapLock(g_a2dpInStreamInfoMapLock);
    a2dpInStreamInfoMap_.clear();
}

void HfpBluetoothDeviceManager::SetHfpStack(const BluetoothRemoteDevice &device, int action)
{
    switch (action) {
        case BluetoothDeviceAction::CONNECTING_ACTION:
            HandleConnectingDevice(device);
            break;
        case BluetoothDeviceAction::CONNECT_ACTION:
            HandleConnectDevice(device);
            break;
        case BluetoothDeviceAction::DISCONNECT_ACTION:
            HandleDisconnectDevice(device);
            break;
        case BluetoothDeviceAction::WEAR_ACTION:
            HandleWearDevice(device);
            break;
        case BluetoothDeviceAction::UNWEAR_ACTION:
            HandleUnwearDevice(device);
            break;
        case BluetoothDeviceAction::ENABLEFROMREMOTE_ACTION:
            HandleEnableDevice(device);
            break;
        case BluetoothDeviceAction::DISABLEFROMREMOTE_ACTION:
            HandleDisableDevice(device);
            break;
        case BluetoothDeviceAction::ENABLE_WEAR_DETECTION_ACTION:
            HandleWearEnable(device);
            break;
        case BluetoothDeviceAction::DISABLE_WEAR_DETECTION_ACTION:
            HandleWearDisable(device);
            break;
        case BluetoothDeviceAction::USER_SELECTION_ACTION:
            HandleUserSelection(device);
            break;
        case BluetoothDeviceAction::STOP_VIRTUAL_CALL:
            HandleStopVirtualCall(device);
            break;
        case BluetoothDeviceAction::VIRTUAL_DEVICE_ADD_ACTION:
            HandleVirtualConnectDevice(device);
            break;
        case BluetoothDeviceAction::VIRTUAL_DEVICE_REMOVE_ACTION:
            HandleRemoveVirtualConnectDevice(device);
            break;
        case BluetoothDeviceAction::CATEGORY_UPDATE_ACTION:
            HandleUpdateDeviceCategory(device);
            break;
        default:
            AUDIO_ERR_LOG("SetHfpStack failed due to the unknow action: %{public}d", action);
            break;
    }
}

void HfpBluetoothDeviceManager::HandleConnectingDevice(const BluetoothRemoteDevice &device)
{
    if (IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        return;
    }
    AddDeviceInConfigVector(device, connectingDevices_);
}

void HfpBluetoothDeviceManager::HandleConnectDevice(const BluetoothRemoteDevice &device)
{
    if (IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        return;
    }
    AudioDeviceDescriptor desc = HandleConnectDeviceInner(device);
    NotifyToUpdateAudioDevice(device, desc, DeviceStatus::ADD);
}

AudioDeviceDescriptor HfpBluetoothDeviceManager::HandleConnectDeviceInner(const BluetoothRemoteDevice &device)
{
    RemoveDeviceInConfigVector(device, connectingDevices_);
    // If the device was virtual connected, remove it from the negativeDevices_ list.
    RemoveDeviceInConfigVector(device, negativeDevices_);
    DeviceCategory bluetoothCategory = GetDeviceCategory(device);
    HandsFreeAudioGateway *hfpInstance = HandsFreeAudioGateway::GetProfile();
    CHECK_AND_RETURN_RET_LOG(hfpInstance != nullptr, ERROR, "HFP AG profile instance unavailable");
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = bluetoothCategory;
    hfpInstance->IsVoiceRecognitionSupported(device, desc.isVrSupported_);
    AUDIO_INFO_LOG("Device supports recognition = %{public}d", desc.isVrSupported_);
    switch (bluetoothCategory) {
        case BT_GLASSES:
        case BT_HEADPHONE:
            if (IsBTWearDetectionEnable(device)) {
                AddDeviceInConfigVector(device, negativeDevices_);
                desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
            } else {
                AddDeviceInConfigVector(device, privacyDevices_);
            }
            break;
        case BT_SOUNDBOX:
        case BT_CAR:
            AddDeviceInConfigVector(device, commonDevices_);
            break;
        case BT_WATCH:
            AddDeviceInConfigVector(device, negativeDevices_);
            break;
        default:
            AUDIO_WARNING_LOG("Unknow BT category, regard as bluetooth headset.");
            AddDeviceInConfigVector(device, privacyDevices_);
            desc.deviceCategory_ = BT_HEADPHONE;
            break;
    }
    return desc;
}

void HfpBluetoothDeviceManager::HandleDisconnectDevice(const BluetoothRemoteDevice &device)
{
    RemoveDeviceInConfigVector(device, connectingDevices_);
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_INFO_LOG("The device is already disconnected, ignore disconnect action.");
        return;
    }
    RemoveDeviceInConfigVector(device, privacyDevices_);
    RemoveDeviceInConfigVector(device, commonDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_hfpWearStateMapLock);
        wearDetectionStateMap_.erase(device.GetDeviceAddr());
    }
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    NotifyToUpdateAudioDevice(device, desc, DeviceStatus::REMOVE);
}

void HfpBluetoothDeviceManager::HandleWearDevice(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleWearDevice failed for the device has not be reported the connected action.");
        return;
    }
    RemoveDeviceInConfigVector(device, negativeDevices_);
    RemoveDeviceInConfigVector(device, privacyDevices_);
    AddDeviceInConfigVector(device, privacyDevices_);
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_hfpWearStateMapLock);
        wearDetectionStateMap_[device.GetDeviceAddr()] = BluetoothDeviceAction::WEAR_ACTION;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = BT_HEADPHONE;
    OnDeviceCategoryUpdated(device, desc);
}

void HfpBluetoothDeviceManager::HandleUnwearDevice(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleWearDevice failed for the device has not worn.");
        return;
    }
    RemoveDeviceInConfigVector(device, privacyDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    AddDeviceInConfigVector(device, negativeDevices_);
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_hfpWearStateMapLock);
        wearDetectionStateMap_[device.GetDeviceAddr()] = BluetoothDeviceAction::UNWEAR_ACTION;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    OnDeviceCategoryUpdated(device, desc);
}

void HfpBluetoothDeviceManager::HandleEnableDevice(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleEnableDevice failed for the device has not connected.");
        return;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.isEnable_ = true;
    OnDeviceEnableUpdated(device, desc);
}

void HfpBluetoothDeviceManager::HandleDisableDevice(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleDisableDevice failed for the device has not connected.");
        return;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.isEnable_ = false;
    OnDeviceEnableUpdated(device, desc);
}

void HfpBluetoothDeviceManager::HandleWearEnable(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleWearEnable failed for the device has not connected.");
        return;
    }
    RemoveDeviceInConfigVector(device, negativeDevices_);
    RemoveDeviceInConfigVector(device, privacyDevices_);
    AudioDeviceDescriptor desc;
    {
        std::lock_guard<std::mutex> wearStateMapLock(g_hfpWearStateMapLock);
        desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
        desc.macAddress_ = device.GetDeviceAddr();
        auto wearStateIter = wearDetectionStateMap_.find(device.GetDeviceAddr());
        if (wearStateIter != wearDetectionStateMap_.end() &&
            wearStateIter->second == BluetoothDeviceAction::WEAR_ACTION) {
            AddDeviceInConfigVector(device, privacyDevices_);
            desc.deviceCategory_ = BT_HEADPHONE;
        } else {
            AddDeviceInConfigVector(device, negativeDevices_);
            desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
        }
    }
    OnDeviceCategoryUpdated(device, desc);
}

void HfpBluetoothDeviceManager::HandleWearDisable(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_ERR_LOG("HandleWearDisable failed for the device has not connected.");
        return;
    }
    RemoveDeviceInConfigVector(device, privacyDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    AddDeviceInConfigVector(device, privacyDevices_);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceCategory_ = BT_HEADPHONE;
    OnDeviceCategoryUpdated(device, desc);
}

void HfpBluetoothDeviceManager::HandleUserSelection(const BluetoothRemoteDevice &device)
{
    std::string deviceAddr = device.GetDeviceAddr();
    DeviceCategory bluetoothCategory = GetDeviceCategory(device);
    if (bluetoothCategory == BT_WATCH) {
        std::lock_guard<std::mutex> wearStateMapLock(g_hfpWearStateMapLock);
        std::lock_guard<std::mutex> hfpDeviceLock(g_hfpDeviceLock);
        auto isPresent = [] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
            return wearDetectionStateMap_[bluetoothRemoteDevice.GetDeviceAddr()] == WEAR_ACTION;
        };
        auto deviceIter = std::find_if(privacyDevices_.rbegin(), privacyDevices_.rend(), isPresent);
        if (deviceIter != privacyDevices_.rend()) {
            deviceAddr = deviceIter->GetDeviceAddr();
            AUDIO_WARNING_LOG("Change user select device from watch %{public}s to wear headphone %{public}s",
                GetEncryptAddr(device.GetDeviceAddr()).c_str(), GetEncryptAddr(deviceAddr).c_str());
        }
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnForcedDeviceSelected(DEVICE_TYPE_BLUETOOTH_SCO, deviceAddr);
    }
}

void HfpBluetoothDeviceManager::HandleStopVirtualCall(const BluetoothRemoteDevice &device)
{
    {
        std::lock_guard<std::mutex> handleLock(stopVirtualCallHandleLock_);
        stopVirtualCallHandle_.device = device;
        stopVirtualCallHandle_.isWaitingForStoppingVirtualCall = true;
    }
    TryDisconnectScoAsync(device);
}

void HfpBluetoothDeviceManager::HandleVirtualConnectDevice(const BluetoothRemoteDevice &device)
{
    AddDeviceInConfigVector(device, virtualDevices_);
    DeviceCategory bluetoothCategory = GetDeviceCategory(device);
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = bluetoothCategory;
    AddDeviceInConfigVector(device, negativeDevices_);
    NotifyToUpdateVirtualDevice(device, desc, DeviceStatus::VIRTUAL_ADD);
}

void HfpBluetoothDeviceManager::HandleRemoveVirtualConnectDevice(const BluetoothRemoteDevice &device)
{
    RemoveDeviceInConfigVector(device, virtualDevices_);
    RemoveDeviceInConfigVector(device, negativeDevices_);
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    NotifyToUpdateVirtualDevice(device, desc, DeviceStatus::VIRTUAL_REMOVE);
}

void HfpBluetoothDeviceManager::HandleUpdateDeviceCategory(const BluetoothRemoteDevice &device)
{
    if (!IsHfpBluetoothDeviceExist(device.GetDeviceAddr())) {
        AUDIO_WARNING_LOG("HandleUpdateDeviceCategory failed for the device has not be reported the connected action.");
        return;
    }
    AudioDeviceDescriptor desc = HandleConnectDeviceInner(device);
    int32_t wearState = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
    if (wearState == 1 && desc.deviceCategory_ == BT_UNWEAR_HEADPHONE) { // 1 wear state
        desc.deviceCategory_ = BT_HEADPHONE;
    }
    OnDeviceCategoryUpdated(device, desc);
}

void HfpBluetoothDeviceManager::AddDeviceInConfigVector(const BluetoothRemoteDevice &device,
    std::vector<BluetoothRemoteDevice> &deviceVector)
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_hfpDeviceLock);
    auto isPresent = [&device] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
        return device.GetDeviceAddr() == bluetoothRemoteDevice.GetDeviceAddr();
    };
    auto deviceIter = std::find_if(deviceVector.begin(), deviceVector.end(), isPresent);
    if (deviceIter == deviceVector.end()) {
        deviceVector.push_back(device);
    }
}

void HfpBluetoothDeviceManager::RemoveDeviceInConfigVector(const BluetoothRemoteDevice &device,
    std::vector<BluetoothRemoteDevice> &deviceVector)
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_hfpDeviceLock);
    auto isPresent = [&device] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
        return device.GetDeviceAddr() == bluetoothRemoteDevice.GetDeviceAddr();
    };
    auto deviceIter = std::find_if(deviceVector.begin(), deviceVector.end(), isPresent);
    if (deviceIter != deviceVector.end()) {
        deviceVector.erase(deviceIter);
    }
}

void HfpBluetoothDeviceManager::NotifyToUpdateAudioDevice(const BluetoothRemoteDevice &device,
    AudioDeviceDescriptor &desc, DeviceStatus deviceStatus)
{
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceName_ = device.GetDeviceName();
    desc.connectState_ = ConnectState::DEACTIVE_CONNECTED;
    AUDIO_WARNING_LOG("hfpBluetoothDeviceMap_ operation: %{public}d new bluetooth device, device address is %{public}s,\
        category is %{public}d", deviceStatus, GetEncryptAddr(device.GetDeviceAddr()).c_str(), desc.deviceCategory_);
    {
        std::lock_guard<std::mutex> deviceMapLock(g_hfpDeviceMapLock);
        if (deviceStatus == DeviceStatus::ADD) {
            hfpBluetoothDeviceMap_[device.GetDeviceAddr()] = device;
        } else if (deviceStatus == DeviceStatus::REMOVE) {
            if (hfpBluetoothDeviceMap_.find(device.GetDeviceAddr()) != hfpBluetoothDeviceMap_.end()) {
                hfpBluetoothDeviceMap_.erase(device.GetDeviceAddr());
            }
        }
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver == nullptr) {
        AUDIO_ERR_LOG("NotifyToUpdateAudioDevice, device observer is null");
        return;
    }
    bool isConnected = deviceStatus == DeviceStatus::ADD;
    g_deviceObserver->OnDeviceStatusUpdated(desc, isConnected);
}

void HfpBluetoothDeviceManager::NotifyToUpdateVirtualDevice(const BluetoothRemoteDevice &device,
    AudioDeviceDescriptor &desc, DeviceStatus deviceStatus)
{
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    desc.deviceName_ = device.GetDeviceName();
    desc.connectState_ = ConnectState::VIRTUAL_CONNECTED;
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    CHECK_AND_RETURN_LOG(g_deviceObserver != nullptr, "NotifyToUpdateVirtualDevice, device observer is null");
    bool isConnected = deviceStatus == DeviceStatus::VIRTUAL_ADD;
    g_deviceObserver->OnDeviceStatusUpdated(desc, isConnected);
}

bool HfpBluetoothDeviceManager::IsHfpBluetoothDeviceExist(const std::string& macAddress)
{
    std::lock_guard<std::mutex> deviceMapLock(g_hfpDeviceMapLock);
    if (hfpBluetoothDeviceMap_.find(macAddress) != hfpBluetoothDeviceMap_.end()) {
        return true;
    }
    return false;
}

bool HfpBluetoothDeviceManager::IsHfpBluetoothDeviceConnecting(const std::string& macAddress)
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_hfpDeviceLock);
    auto deviceIter = std::find_if(connectingDevices_.begin(), connectingDevices_.end(),
        [&macAddress] (BluetoothRemoteDevice &bluetoothRemoteDevice) {
            return macAddress == bluetoothRemoteDevice.GetDeviceAddr();
        });
    if (deviceIter != connectingDevices_.end()) {
        return true;
    }
    return false;
}

int32_t HfpBluetoothDeviceManager::GetConnectedHfpBluetoothDevice(const std::string& macAddress,
    BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> deviceMapLock(g_hfpDeviceMapLock);
    auto deviceIter = hfpBluetoothDeviceMap_.find(macAddress);
    if (deviceIter != hfpBluetoothDeviceMap_.end()) {
        device = deviceIter->second;
        return SUCCESS;
    }
    return ERROR;
}

std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::GetAllHfpBluetoothDevice()
{
    std::lock_guard<std::mutex> deviceMapLock(g_hfpDeviceMapLock);
    std::vector<BluetoothRemoteDevice> hfpList = {};
    hfpList.reserve(hfpBluetoothDeviceMap_.size());
    for (const auto &[macaddr, device] : hfpBluetoothDeviceMap_) {
        hfpList.emplace_back(device);
    }
    return hfpList;
}

void HfpBluetoothDeviceManager::ClearAllHfpBluetoothDevice()
{
    AUDIO_WARNING_LOG("Bluetooth service crashed and enter the ClearAllhfpBluetoothDevice.");
    {
        std::lock_guard<std::mutex> hfpDeviceLock(g_hfpDeviceLock);
        privacyDevices_.clear();
        commonDevices_.clear();
        negativeDevices_.clear();
        connectingDevices_.clear();
        virtualDevices_.clear();
    }
    std::lock_guard<std::mutex> deviceMapLock(g_hfpDeviceMapLock);
    std::lock_guard<std::mutex> wearStateMapLock(g_hfpWearStateMapLock);
    hfpBluetoothDeviceMap_.clear();
    wearDetectionStateMap_.clear();
}

void HfpBluetoothDeviceManager::OnScoStateChanged(const BluetoothRemoteDevice &device, bool isConnected, int reason)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = device.GetDeviceAddr();
    if (isConnected) {
        desc.connectState_ = ConnectState::CONNECTED;
    } else {
        {
            std::lock_guard<std::mutex> handleLock(stopVirtualCallHandleLock_);
            if (device.GetDeviceAddr() == stopVirtualCallHandle_.device.GetDeviceAddr() &&
                stopVirtualCallHandle_.isWaitingForStoppingVirtualCall) {
                AUDIO_WARNING_LOG("reason change to %{public}d", HFP_AG_SCO_REMOTE_USER_TERMINATED);
                reason = HFP_AG_SCO_REMOTE_USER_TERMINATED;
                stopVirtualCallHandle_.device = BluetoothRemoteDevice();
                stopVirtualCallHandle_.isWaitingForStoppingVirtualCall = false;
            }
        }
        desc.connectState_ = reason == HFP_AG_SCO_REMOTE_USER_TERMINATED ?  ConnectState::SUSPEND_CONNECTED
                                                                         :  ConnectState::DEACTIVE_CONNECTED;
    }
    std::lock_guard<std::mutex> observerLock(g_observerLock);
    if (g_deviceObserver != nullptr) {
        g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CONNECTSTATE_UPDATE);
    }
}

std::vector<BluetoothRemoteDevice> HfpBluetoothDeviceManager::GetHfpVirtualDeviceList()
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_hfpDeviceLock);
    return virtualDevices_;
}

void HfpBluetoothDeviceManager::RegisterDisconnectScoFunc(DisconnectScoForDevice func)
{
    disconnectScoFun_ = func;
}

void HfpBluetoothDeviceManager::TryDisconnectScoAsync(const BluetoothRemoteDevice &device)
{
    std::thread disconnectScoThread = std::thread([device]() {
        if (HfpBluetoothDeviceManager::disconnectScoFun_ != nullptr) {
            AUDIO_INFO_LOG("bluetooth service trigger disconnect sco async");
            HfpBluetoothDeviceManager::disconnectScoFun_(device);
        }
    });
    disconnectScoThread.detach();
}

void HfpBluetoothDeviceManager::TryDisconnectScoSync(const BluetoothRemoteDevice &device, const std::string &reason)
{
    if (disconnectScoFun_ != nullptr) {
        AUDIO_INFO_LOG("bluetooth service trigger disconnect %{public}s sco sync with reason %{public}s",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(), reason.c_str());
        disconnectScoFun_(device);
    }
}

void HfpBluetoothDeviceManager::OnDeviceCategoryUpdated(const BluetoothRemoteDevice &device,
    AudioDeviceDescriptor &desc)
{
    {
        std::lock_guard<std::mutex> observerLock(g_observerLock);
        if (g_deviceObserver != nullptr) {
            g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE);
        }
    }
    if (desc.deviceCategory_ == BT_UNWEAR_HEADPHONE) {
        TryDisconnectScoSync(device, "BT_UNWEAR_HEADPHONE");
    }
}

void HfpBluetoothDeviceManager::OnDeviceEnableUpdated(const BluetoothRemoteDevice &device,
    AudioDeviceDescriptor &desc)
{
    {
        std::lock_guard<std::mutex> observerLock(g_observerLock);
        if (g_deviceObserver != nullptr) {
            g_deviceObserver->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::ENABLE_UPDATE);
        }
    }
    if (!desc.isEnable_) {
        TryDisconnectScoSync(device, "Device Disable");
    }
}
// LCOV_EXCL_STOP
} // namespace Bluetooth
} // namespace OHOS
