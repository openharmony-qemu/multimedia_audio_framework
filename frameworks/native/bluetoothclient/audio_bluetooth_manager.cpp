/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioBluetoothManager"
#endif

#include <shared_mutex>
#include "audio_bluetooth_manager.h"
#include "bluetooth_def.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_device_utils.h"
#include "bluetooth_hfp_interface.h"
#include "hisysevent.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

const int32_t BT_VIRTUAL_DEVICE_ADD = 0;
const int32_t BT_VIRTUAL_DEVICE_REMOVE = 1;
constexpr const uint8_t CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG = 0x03;
A2dpSource *AudioA2dpManager::a2dpInstance_ = nullptr;
std::shared_ptr<AudioA2dpListener> AudioA2dpManager::a2dpListener_ = std::make_shared<AudioA2dpListener>();
int AudioA2dpManager::connectionState_ = static_cast<int>(BTConnectState::DISCONNECTED);
int32_t AudioA2dpManager::captureConnectionState_ = static_cast<int32_t>(BTHdapConnectState::DISCONNECTED);
BluetoothRemoteDevice AudioA2dpManager::activeA2dpDevice_;
std::shared_mutex g_a2dpInstanceLock;
std::shared_ptr<AudioHfpListener> AudioHfpManager::hfpListener_ = std::make_shared<AudioHfpListener>();
std::atomic<AudioScene> AudioHfpManager::scene_ = AUDIO_SCENE_DEFAULT;
BluetoothRemoteDevice AudioHfpManager::activeHfpDevice_;
std::atomic<bool> AudioHfpManager::isRecognitionScene_ = false;
std::atomic<bool> AudioHfpManager::isRecordScene_ = false;
std::map<pid_t, bool> AudioHfpManager::virtualCalls_;
std::mutex AudioHfpManager::virtualCallMutex_;
std::vector<std::shared_ptr<AudioA2dpPlayingStateChangedListener>> AudioA2dpManager::a2dpPlayingStateChangedListeners_;
std::mutex g_activehfpDeviceLock;
std::mutex g_a2dpPlayingStateChangedLock;
static const int32_t BT_SET_ACTIVE_DEVICE_TIMEOUT = 8; //BtService SetActiveDevice 8s timeout

static bool GetAudioStreamInfo(A2dpCodecInfo codecInfo, AudioStreamInfo &audioStreamInfo)
{
    AUDIO_DEBUG_LOG("codec info rate[%{public}d]  format[%{public}d]  channel[%{public}d]",
        codecInfo.sampleRate, codecInfo.bitsPerSample, codecInfo.channelMode);
    switch (codecInfo.sampleRate) {
        case A2DP_SBC_SAMPLE_RATE_48000_USER:
        case A2DP_L2HCV2_SAMPLE_RATE_48000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
            break;
        case A2DP_SBC_SAMPLE_RATE_44100_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_44100;
            break;
        case A2DP_SBC_SAMPLE_RATE_32000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_32000;
            break;
        case A2DP_SBC_SAMPLE_RATE_16000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_16000;
            break;
        case A2DP_L2HCV2_SAMPLE_RATE_96000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_96000;
            break;
        default:
            return false;
    }
    switch (codecInfo.bitsPerSample) {
        case A2DP_SAMPLE_BITS_16_USER:
            audioStreamInfo.format = SAMPLE_S16LE;
            break;
        case A2DP_SAMPLE_BITS_24_USER:
            audioStreamInfo.format = SAMPLE_S24LE;
            break;
        case A2DP_SAMPLE_BITS_32_USER:
            audioStreamInfo.format = SAMPLE_S32LE;
            break;
        default:
            return false;
    }
    switch (codecInfo.channelMode) {
        case A2DP_SBC_CHANNEL_MODE_STEREO_USER:
            audioStreamInfo.channels = STEREO;
            break;
        case A2DP_SBC_CHANNEL_MODE_MONO_USER:
            audioStreamInfo.channels = MONO;
            break;
        default:
            return false;
    }
    audioStreamInfo.encoding = ENCODING_PCM;
    return true;
}

// LCOV_EXCL_START
void AudioA2dpManager::RegisterBluetoothA2dpListener()
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "Failed to obtain A2DP profile instance");
    a2dpInstance_->RegisterObserver(a2dpListener_);
}

void AudioA2dpManager::UnregisterBluetoothA2dpListener()
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "A2DP profile instance unavailable");

    a2dpInstance_->DeregisterObserver(a2dpListener_);
    a2dpInstance_ = nullptr;
}

void AudioA2dpManager::DisconnectBluetoothA2dpSink()
{
    int connectionState = static_cast<int>(BTConnectState::DISCONNECTED);
    auto a2dpList = MediaBluetoothDeviceManager::GetAllA2dpBluetoothDevice();
    for (const auto &device : a2dpList) {
        if (a2dpListener_ != nullptr) {
            a2dpListener_->OnConnectionStateChanged(device, connectionState,
                static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
        }
    }

    auto virtualDevices = MediaBluetoothDeviceManager::GetA2dpVirtualDeviceList();
    for (const auto &virtualDevice : virtualDevices) {
        if (a2dpListener_ != nullptr) {
            a2dpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_REMOVE),
                virtualDevice.GetDeviceAddr());
        }
    }

    MediaBluetoothDeviceManager::ClearAllA2dpBluetoothDevice();
}

void AudioA2dpManager::DisconnectBluetoothA2dpSource()
{
    CHECK_AND_RETURN_LOG(a2dpListener_ != nullptr, "a2dpListener_ is nullptr");
    int captureConnectionState = static_cast<int>(BTHdapConnectState::DISCONNECTED);
    auto a2dpInList = A2dpInBluetoothDeviceManager::GetAllA2dpInBluetoothDevice();
    A2dpCodecInfo defaultCodecInfo = {};
    for (const auto &device : a2dpInList) {
        a2dpListener_->OnCaptureConnectionStateChanged(device, captureConnectionState, defaultCodecInfo);
    }
    A2dpInBluetoothDeviceManager::ClearAllA2dpInBluetoothDevice();
    A2dpInBluetoothDeviceManager::ClearAllA2dpInStreamInfo();
}

int32_t AudioA2dpManager::SetActiveA2dpDevice(const std::string& macAddress)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    AUDIO_WARNING_LOG("incoming device:%{public}s, current device:%{public}s",
        GetEncryptAddr(macAddress).c_str(), GetEncryptAddr(activeA2dpDevice_.GetDeviceAddr()).c_str());
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance is null");
    BluetoothRemoteDevice device;
    if (macAddress != "") {
        int32_t tmp = MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device);
        CHECK_AND_RETURN_RET_LOG(tmp == SUCCESS, ERROR, "the configuring A2DP device doesn't exist.");
    } else {
        AUDIO_INFO_LOG("Deactive A2DP device");
    }
    int32_t ret = a2dpInstance_->SetActiveSinkDevice(device);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "SetActiveA2dpDevice failed. result: %{public}d", ret);
    activeA2dpDevice_ = device;
    return SUCCESS;
}

std::string AudioA2dpManager::GetActiveA2dpDevice()
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, "", "A2DP profile instance is null");
    BluetoothRemoteDevice device = a2dpInstance_->GetActiveSinkDevice();
    return device.GetDeviceAddr();
}

int32_t AudioA2dpManager::SetDeviceAbsVolume(const std::string& macAddress, int32_t volume)
{
    BluetoothRemoteDevice device;
    int32_t ret = MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetDeviceAbsVolume: the configuring A2DP device doesn't exist.");
    return AvrcpTarget::GetProfile()->SetDeviceAbsoluteVolume(device, volume);
}

int32_t AudioA2dpManager::GetA2dpDeviceStreamInfo(const std::string& macAddress,
    AudioStreamInfo &streamInfo)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance is null");
    BluetoothRemoteDevice device;
    int32_t ret = MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        "GetA2dpDeviceStreamInfo: the configuring A2DP device doesn't exist.");
    A2dpCodecStatus codecStatus = a2dpInstance_->GetCodecStatus(device);
    bool result = GetAudioStreamInfo(codecStatus.codecInfo, streamInfo);
    CHECK_AND_RETURN_RET_LOG(result, ERROR, "GetA2dpDeviceStreamInfo: Unsupported a2dp codec info");
    return SUCCESS;
}

int32_t AudioA2dpManager::GetA2dpInDeviceStreamInfo(const std::string &macAddress,
    AudioStreamInfo &streamInfo)
{
    bool ret = A2dpInBluetoothDeviceManager::GetA2dpInDeviceStreamInfo(macAddress, streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == true, ERROR, "the StreamInfo of the a2dp input device doesn't exist.");
    return SUCCESS;
}

bool AudioA2dpManager::HasA2dpDeviceConnected()
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, false, "A2DP profile instance is null");
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices;
    a2dpInstance_->GetDevicesByStates(states, devices);

    return !devices.empty();
}

int32_t AudioA2dpManager::A2dpOffloadSessionRequest(const std::vector<A2dpStreamInfo> &info)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance is null");
    CHECK_AND_RETURN_RET_LOG(activeA2dpDevice_.GetDeviceAddr() != "00:00:00:00:00:00", A2DP_NOT_OFFLOAD,
        "Invalid mac address, not request, return A2DP_NOT_OFFLOAD.");
    int32_t ret = a2dpInstance_->A2dpOffloadSessionRequest(activeA2dpDevice_, info);
    AUDIO_DEBUG_LOG("Request %{public}zu stream and return a2dp offload state %{public}d", info.size(), ret);
    return ret;
}

int32_t AudioA2dpManager::OffloadStartPlaying(const std::vector<int32_t> &sessionsID)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance is null");
    CHECK_AND_RETURN_RET_LOG(activeA2dpDevice_.GetDeviceAddr() != "00:00:00:00:00:00", ERROR,
        "Invalid mac address, not start, return error.");
    AUDIO_DEBUG_LOG("Start playing %{public}zu stream", sessionsID.size());
    return a2dpInstance_->OffloadStartPlaying(activeA2dpDevice_, sessionsID);
}

int32_t AudioA2dpManager::OffloadStopPlaying(const std::vector<int32_t> &sessionsID)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance is null");
    if (activeA2dpDevice_.GetDeviceAddr() == "00:00:00:00:00:00") {
        AUDIO_DEBUG_LOG("Invalid mac address, not stop, return error.");
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Stop playing %{public}zu stream", sessionsID.size());
    return a2dpInstance_->OffloadStopPlaying(activeA2dpDevice_, sessionsID);
}

int32_t AudioA2dpManager::GetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance is null");
    if (activeA2dpDevice_.GetDeviceAddr() == "00:00:00:00:00:00") {
        AUDIO_DEBUG_LOG("Invalid mac address, return error.");
        return ERROR;
    }
    return a2dpInstance_->GetRenderPosition(activeA2dpDevice_, delayValue, sendDataSize, timeStamp);
}

int32_t AudioA2dpManager::RegisterA2dpPlayingStateChangedListener(
    std::shared_ptr<AudioA2dpPlayingStateChangedListener> listener)
{
    std::lock_guard<std::mutex> lock(g_a2dpPlayingStateChangedLock);
    a2dpPlayingStateChangedListeners_.push_back(listener);
    return SUCCESS;
}

void AudioA2dpManager::OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState)
{
    std::lock_guard<std::mutex> lock(g_a2dpPlayingStateChangedLock);
    for (auto listener : a2dpPlayingStateChangedListeners_) {
        listener->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    }
}

void AudioA2dpManager::CheckA2dpDeviceReconnect()
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "A2DP profile instance is null");
    CHECK_AND_RETURN_LOG(a2dpListener_ != nullptr, "a2dpListener_ is nullptr");
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices;
    a2dpInstance_->GetDevicesByStates(states, devices);

    for (auto &device : devices) {
        a2dpListener_->OnConnectionStateChanged(device, static_cast<int32_t>(BTConnectState::CONNECTED),
            static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));

        int32_t wearState = 0; // 0 unwear state
        if (IsBTWearDetectionEnable(device)) {
            wearState = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
            if (wearState == 1) MediaBluetoothDeviceManager::SetMediaStack(device, WEAR_ACTION); // 1 wear state
        }
        AUDIO_WARNING_LOG("reconnect a2dp device:%{public}s, wear state:%{public}d",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(), wearState);
    }

    std::vector<std::string> virtualDevices;
    a2dpInstance_->GetVirtualDeviceList(virtualDevices);
    for (auto &macAddress : virtualDevices) {
        AUDIO_WARNING_LOG("reconnect virtual a2dp device:%{public}s", GetEncryptAddr(macAddress).c_str());
        a2dpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_ADD), macAddress);
    }
}

int32_t AudioA2dpManager::Connect(const std::string &macAddress)
{
    std::shared_lock<std::shared_mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "A2DP profile instance unavailable");
    BluetoothRemoteDevice virtualDevice = BluetoothRemoteDevice(macAddress);
    if (MediaBluetoothDeviceManager::IsA2dpBluetoothDeviceConnecting(macAddress)) {
        AUDIO_WARNING_LOG("A2dp device %{public}s is connecting, ignore connect request",
            GetEncryptAddr(macAddress).c_str());
        virtualDevice.SetVirtualAutoConnectType(CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG, 0);
        return SUCCESS;
    }
    std::vector<std::string> virtualDevices;
    a2dpInstance_->GetVirtualDeviceList(virtualDevices);
    if (std::find(virtualDevices.begin(), virtualDevices.end(), macAddress) == virtualDevices.end()) {
        AUDIO_WARNING_LOG("A2dp device %{public}s is not virtual device, ignore connect request",
            GetEncryptAddr(macAddress).c_str());
        return SUCCESS;
    }
    int32_t ret = a2dpInstance_->Connect(virtualDevice);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "A2dp Connect Failed");
    virtualDevice.SetVirtualAutoConnectType(CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG, 0);
    return SUCCESS;
}

void AudioA2dpListener::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
{
    AUDIO_WARNING_LOG("state: %{public}d, macAddress: %{public}s", state,
        GetEncryptAddr(device.GetDeviceAddr()).c_str());
    // Record connection state and device for hdi start time to check
    AudioA2dpManager::SetConnectionState(state);
    if (state == static_cast<int>(BTConnectState::CONNECTING)) {
        MediaBluetoothDeviceManager::SetMediaStack(device, BluetoothDeviceAction::CONNECTING_ACTION);
    }
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        MediaBluetoothDeviceManager::SetMediaStack(device, BluetoothDeviceAction::CONNECT_ACTION);
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        MediaBluetoothDeviceManager::SetMediaStack(device, BluetoothDeviceAction::DISCONNECT_ACTION);
    }
}

void AudioA2dpListener::OnConfigurationChanged(const BluetoothRemoteDevice &device, const A2dpCodecInfo &codecInfo,
    int error)
{
    AUDIO_INFO_LOG("OnConfigurationChanged: sampleRate: %{public}d, channels: %{public}d, format: %{public}d",
        codecInfo.sampleRate, codecInfo.channelMode, codecInfo.bitsPerSample);
    AudioStreamInfo streamInfo = {};
    bool result = GetAudioStreamInfo(codecInfo, streamInfo);
    CHECK_AND_RETURN_LOG(result, "OnConfigurationChanged: Unsupported a2dp codec info");
    MediaBluetoothDeviceManager::UpdateA2dpDeviceConfiguration(device, streamInfo);
}

void AudioA2dpListener::OnPlayingStatusChanged(const BluetoothRemoteDevice &device, int playingState, int error)
{
    AUDIO_INFO_LOG("OnPlayingStatusChanged, state: %{public}d, error: %{public}d", playingState, error);
    if (error == SUCCESS) {
        AudioA2dpManager::OnA2dpPlayingStateChanged(device.GetDeviceAddr(), playingState);
    }
}

void AudioA2dpListener::OnMediaStackChanged(const BluetoothRemoteDevice &device, int action)
{
    AUDIO_WARNING_LOG("action: %{public}d, macAddress: %{public}s", action,
        GetEncryptAddr(device.GetDeviceAddr()).c_str());
    MediaBluetoothDeviceManager::SetMediaStack(device, action);
}

void AudioA2dpListener::OnVirtualDeviceChanged(int32_t action, std::string macAddress)
{
    AUDIO_WARNING_LOG("action: %{public}d, macAddress: %{public}s", action, GetEncryptAddr(macAddress).c_str());
    if (action == static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_ADD)) {
        MediaBluetoothDeviceManager::SetMediaStack(BluetoothRemoteDevice(macAddress),
            BluetoothDeviceAction::VIRTUAL_DEVICE_ADD_ACTION);
    }
    if (action == static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_REMOVE)) {
        MediaBluetoothDeviceManager::SetMediaStack(BluetoothRemoteDevice(macAddress),
            BluetoothDeviceAction::VIRTUAL_DEVICE_REMOVE_ACTION);
    }
}

void AudioA2dpListener::OnCaptureConnectionStateChanged(const BluetoothRemoteDevice &device, int state,
    const A2dpCodecInfo &codecInfo)
{
    AUDIO_INFO_LOG("capture connection state: %{public}d", state);
    AudioA2dpManager::SetCaptureConnectionState(static_cast<int32_t>(state));
    AudioStreamInfo streamInfo = {};
    if (state == static_cast<int>(BTHdapConnectState::CONNECTED)) {
        AUDIO_INFO_LOG("A2dpInCodecInfo: sampleRate: %{public}d, channels: %{public}d, format: %{public}d",
            codecInfo.sampleRate, codecInfo.channelMode, codecInfo.bitsPerSample);
        bool result = GetAudioStreamInfo(codecInfo, streamInfo);
        CHECK_AND_RETURN_LOG(result == true, "Unsupported a2dpIn codec info");
        A2dpInBluetoothDeviceManager::SetA2dpInStack(device, streamInfo, BluetoothDeviceAction::CONNECT_ACTION);
    } else if (state == static_cast<int>(BTHdapConnectState::DISCONNECTED)) {
        A2dpInBluetoothDeviceManager::SetA2dpInStack(device, streamInfo, BluetoothDeviceAction::DISCONNECT_ACTION);
    }
}

void AudioHfpManager::RegisterBluetoothScoListener()
{
    HfpBluetoothDeviceManager::RegisterDisconnectScoFunc(&DisconnectScoForDevice);
    AUDIO_INFO_LOG("in");
    BluetoothHfpInterface::GetInstance().RegisterObserver(hfpListener_);
}

void AudioHfpManager::UnregisterBluetoothScoListener()
{
    AUDIO_INFO_LOG("in");
    BluetoothHfpInterface::GetInstance().DeregisterObserver(hfpListener_);
}

void AudioHfpManager::CheckHfpDeviceReconnect()
{
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices = BluetoothHfpInterface::GetInstance().GetDevicesByStates(states);
    for (auto &device : devices) {
        if (hfpListener_ != nullptr) {
            hfpListener_->OnConnectionStateChanged(device,
                static_cast<int32_t>(BTConnectState::CONNECTED),
                static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
        }

        int32_t wearState = 0; // 0 unwear state
        if (IsBTWearDetectionEnable(device)) {
            wearState = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
            if (wearState == 1) HfpBluetoothDeviceManager::SetHfpStack(device, WEAR_ACTION); // 1 wear state
        }
        AUDIO_INFO_LOG("reconnect hfp device:%{public}s, wear state:%{public}d",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(), wearState);
    }

    if (hfpListener_ != nullptr) {
        std::vector<std::string> virtualDevices;
        BluetoothHfpInterface::GetInstance().GetVirtualDeviceList(virtualDevices);
        for (auto &macAddress : virtualDevices) {
            AUDIO_PRERELEASE_LOGI("reconnect virtual hfp device:%{public}s",
                GetEncryptAddr(macAddress).c_str());
            hfpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(
                Bluetooth::BT_VIRTUAL_DEVICE_ADD), macAddress);
        }
    }
}

int32_t AudioHfpManager::SetActiveHfpDevice(const std::string &macAddress)
{
    AudioXCollie audioXCollie("AudioHfpManager::SetActiveHfpDevice", BT_SET_ACTIVE_DEVICE_TIMEOUT,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    BluetoothRemoteDevice device;
    if (macAddress != "" && HfpBluetoothDeviceManager::GetConnectedHfpBluetoothDevice(macAddress, device) != SUCCESS) {
        AUDIO_ERR_LOG("SetActiveHfpDevice failed for the HFP device, %{public}s does not exist.",
            GetEncryptAddr(macAddress).c_str());
        return ERROR;
    }
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    AUDIO_INFO_LOG("incoming device:%{public}s, current device:%{public}s",
        GetEncryptAddr(macAddress).c_str(), GetEncryptAddr(activeHfpDevice_.GetDeviceAddr()).c_str());
    if (macAddress != activeHfpDevice_.GetDeviceAddr()) {
        AUDIO_WARNING_LOG("Active hfp device is changed, need to DisconnectSco for current activeHfpDevice.");
        int32_t ret = DisconnectScoWrapper();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "DisconnectSco failed, result: %{public}d", ret);
    }
    int32_t res = BluetoothHfpInterface::GetInstance().SetActiveDevice(device);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERROR, "SetActiveHfpDevice failed, result: %{public}d", res);
    activeHfpDevice_ = device;
    return SUCCESS;
}

int32_t AudioHfpManager::ClearActiveHfpDevice(const std::string &macAddress)
{
    BluetoothRemoteDevice device;
    if (macAddress != "" && HfpBluetoothDeviceManager::GetConnectedHfpBluetoothDevice(macAddress, device) != SUCCESS) {
        AUDIO_ERR_LOG("ClearActiveHfpDevice failed for the HFP device, %{public}s does not exist.",
            GetEncryptAddr(macAddress).c_str());
        return ERROR;
    }
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    AUDIO_DEBUG_LOG("clearing device:%{public}s, current device:%{public}s",
        GetEncryptAddr(macAddress).c_str(), GetEncryptAddr(activeHfpDevice_.GetDeviceAddr()).c_str());
    if (macAddress != activeHfpDevice_.GetDeviceAddr()) {
        return SUCCESS;
    }
    AUDIO_WARNING_LOG("Current hfp device is cleared, need to DisconnectSco for current activeHfpDevice.");
    int32_t ret = DisconnectScoWrapper();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "DisconnectSco failed, result: %{public}d", ret);
    activeHfpDevice_ = BluetoothRemoteDevice();
    return SUCCESS;
}

int32_t AudioHfpManager::UpdateActiveHfpDevice(const BluetoothRemoteDevice &device)
{
    AUDIO_INFO_LOG("update active device:%{public}s, current device:%{public}s",
        GetEncryptAddr(device.GetDeviceAddr()).c_str(),
        GetEncryptAddr(activeHfpDevice_.GetDeviceAddr()).c_str());
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    int32_t res = BluetoothHfpInterface::GetInstance().SetActiveDevice(device);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERROR, "SetActiveDevice failed, result: %{public}d", res);
    activeHfpDevice_ = device;
    return TryUpdateScoCategoryNoLock();
}

std::string AudioHfpManager::GetActiveHfpDevice()
{
    BluetoothRemoteDevice device = BluetoothHfpInterface::GetInstance().GetActiveDevice();
    return device.GetDeviceAddr();
}

int32_t AudioHfpManager::DisconnectSco()
{
    AUDIO_INFO_LOG("disconnect sco from outer");
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    return DisconnectScoWrapper();
}

void AudioHfpManager::DisconnectBluetoothHfpSink()
{
    int32_t connectionState = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    if (hfpListener_ != nullptr) {
        auto hfpList = HfpBluetoothDeviceManager::GetAllHfpBluetoothDevice();
        for (const auto &device : hfpList) {
            hfpListener_->OnConnectionStateChanged(device, connectionState,
                static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
        }

        auto virtualDevices = HfpBluetoothDeviceManager::GetHfpVirtualDeviceList();
        for (const auto &virtualDevice : virtualDevices) {
            hfpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(
                Bluetooth::BT_VIRTUAL_DEVICE_REMOVE),
                virtualDevice.GetDeviceAddr());
        }
    }
    HfpBluetoothDeviceManager::ClearAllHfpBluetoothDevice();
}

void AudioHfpManager::ClearCurrentActiveHfpDevice(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    AUDIO_INFO_LOG("clear current active hfp device:%{public}s",
        GetEncryptAddr(device.GetDeviceAddr()).c_str());
    BluetoothScoManager::GetInstance().ResetScoState(device);
    if (device.GetDeviceAddr() != activeHfpDevice_.GetDeviceAddr()) {
        return;
    }
    activeHfpDevice_ = BluetoothRemoteDevice();
}

int32_t AudioHfpManager::Connect(const std::string &macAddress)
{
    BluetoothRemoteDevice virtualDevice = BluetoothRemoteDevice(macAddress);
    if (HfpBluetoothDeviceManager::IsHfpBluetoothDeviceConnecting(macAddress)) {
        AUDIO_WARNING_LOG("Hfp device %{public}s is connecting, ignore connect request",
            GetEncryptAddr(macAddress).c_str());
        virtualDevice.SetVirtualAutoConnectType(CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG, 0);
        return SUCCESS;
    }
    std::vector<std::string> virtualDevices;
    BluetoothHfpInterface::GetInstance().GetVirtualDeviceList(virtualDevices);
    if (std::find(virtualDevices.begin(), virtualDevices.end(), macAddress) == virtualDevices.end()) {
        AUDIO_WARNING_LOG("Hfp device %{public}s is not virtual device, ignore connect request",
            GetEncryptAddr(macAddress).c_str());
        return SUCCESS;
    }
    int32_t ret = BluetoothHfpInterface::GetInstance().Connect(virtualDevice);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Hfp Connect Failed");
    virtualDevice.SetVirtualAutoConnectType(CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG, 0);
    return SUCCESS;
}

int32_t AudioHfpManager::UpdateAudioScene(AudioScene scene, bool isRecordScene)
{
    if (scene_.load() != scene) {
        AUDIO_INFO_LOG("update audio scene from %{public}d to %{public}d with recordscene",
            scene_.load(), scene);
    }
    if (isRecordScene_.load() != isRecordScene) {
        AUDIO_INFO_LOG("%{public}s record scene", isRecordScene ? "is" : "not");
    }
    scene_.store(scene);
    isRecordScene_.store(isRecordScene);
    return TryUpdateScoCategory();
}

int32_t AudioHfpManager::UpdateAudioScene(AudioScene scene)
{
    if (scene_.load() != scene) {
        AUDIO_INFO_LOG("update audio scene from %{public}d to %{public}d", scene_.load(), scene);
    }
    scene_.store(scene);
    return TryUpdateScoCategory();
}

int32_t AudioHfpManager::HandleScoWithRecongnition(bool handleFlag)
{
    if (isRecognitionScene_.load() != handleFlag) {
        AUDIO_INFO_LOG("%{public}s recognition scene", handleFlag ? "is" : "not");
    }
    isRecognitionScene_.store(handleFlag);
    return TryUpdateScoCategory();
}

bool AudioHfpManager::IsRecognitionStatus()
{
    return BluetoothScoManager::GetInstance().IsInScoCategory(ScoCategory::SCO_RECOGNITION);
}

int32_t AudioHfpManager::SetVirtualCall(pid_t uid, const bool isVirtual)
{
    auto scene = scene_.load();
    CHECK_AND_RETURN_RET_LOG(scene == AUDIO_SCENE_DEFAULT, ERROR, "only support no call");
    {
        std::lock_guard<std::mutex> hfpDeviceLock(virtualCallMutex_);
        if (isVirtual) {
            if (virtualCalls_.count(uid) > 0) {
                virtualCalls_.erase(uid);
            }
        } else {
            virtualCalls_[uid] = isVirtual;
        }
    }

    AUDIO_INFO_LOG("set virtual call %{public}d by service %{public}d", isVirtual, uid);
    return TryUpdateScoCategory();
}

bool AudioHfpManager::IsVirtualCall()
{
    std::lock_guard<std::mutex> hfpDeviceLock(virtualCallMutex_);
    for (const auto &it : virtualCalls_) {
        if (!it.second) {
            AUDIO_INFO_LOG("not virtual call for service %{public}d", it.first);
            return false;
        }
    }
    AUDIO_INFO_LOG("is virtual call");
    return true;
}

bool AudioHfpManager::IsAudioScoStateConnect()
{
    AudioScoState scoState = BluetoothScoManager::GetInstance().GetAudioScoState();
    return (scoState == AudioScoState::CONNECTED || scoState == AudioScoState::CONNECTING);
}

ScoCategory AudioHfpManager::JudgeScoCategory()
{
    bool isInbardingEnabled = false;
    BluetoothHfpInterface::GetInstance().IsInbandRingingEnabled(isInbardingEnabled);

    auto scene = scene_.load();
    if ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) && !isInbardingEnabled) {
        AUDIO_WARNING_LOG("The inbarding switch is off, ignore the ring scene.");
        return isRecognitionScene_.load() ? ScoCategory::SCO_RECOGNITION : ScoCategory::SCO_DEFAULT;
    }

    if (scene == AUDIO_SCENE_VOICE_RINGING || scene == AUDIO_SCENE_PHONE_CALL) {
        return ScoCategory::SCO_CALLULAR;
    } else if (scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_PHONE_CHAT) {
        return !IsVirtualCall() ? ScoCategory::SCO_CALLULAR : ScoCategory::SCO_VIRTUAL;
    }

    return isRecognitionScene_.load() ? ScoCategory::SCO_RECOGNITION : ScoCategory::SCO_DEFAULT;
}

int32_t AudioHfpManager::TryUpdateScoCategory()
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    return TryUpdateScoCategoryNoLock();
}

int32_t AudioHfpManager::TryUpdateScoCategoryNoLock()
{
    BluetoothRemoteDevice defaultDevice;
    if (!activeHfpDevice_.IsValidBluetoothRemoteDevice() ||
        activeHfpDevice_.GetDeviceAddr() == defaultDevice.GetDeviceAddr()) {
        AUDIO_INFO_LOG("current device: %{public}s is invalid",
            GetEncryptAddr(activeHfpDevice_.GetDeviceAddr()).c_str());
        return DisconnectScoWrapper();
    }

    auto category = JudgeScoCategory();
    if (category == ScoCategory::SCO_DEFAULT && !isRecordScene_.load()) {
        return DisconnectScoWrapper();
    }

    int32_t ret = BluetoothScoManager::GetInstance().HandleScoConnect(category, activeHfpDevice_);
    if (ret != SUCCESS) {
        WriteScoOprFaultEvent();
    }
    return ret;
}

void AudioHfpManager::DisconnectScoForDevice(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);

    if (device.GetDeviceAddr() != activeHfpDevice_.GetDeviceAddr()) {
        AUDIO_WARNING_LOG("disconnect sco for device %{public}s but active device %{public}s",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(),
            GetEncryptAddr(activeHfpDevice_.GetDeviceAddr()).c_str());
        return;
    }
    DisconnectScoWrapper();
}

int32_t AudioHfpManager::DisconnectScoWrapper()
{
    int32_t ret = BluetoothScoManager::GetInstance().HandleScoDisconnect(activeHfpDevice_);
    if (ret != SUCCESS) {
        WriteScoOprFaultEvent();
    }
    return ret;
}

void AudioHfpManager::WriteScoOprFaultEvent()
{
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "SCO_STATE_AUDIO",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "PKG_NAME", "",
        "SCO_ADDRESS", BluetoothScoManager::GetInstance().GetAudioScoDevice().GetDeviceAddr(),
        "SCENE", static_cast<uint8_t>(scene_.load()),
        "SCO_MODE", static_cast<uint8_t>(BluetoothScoManager::GetInstance().GetAudioScoCategory()),
        "AUDIO_SCO_STATE", static_cast<uint8_t>(BluetoothScoManager::GetInstance().GetAudioScoState()),
        "RET", static_cast<uint8_t>(BluetoothHfpInterface::GetInstance().GetLastError()));
    if (ret) {
        AUDIO_ERR_LOG("write event fail: SCO_STATE_AUDIO, ret = %{public}d", ret);
    }
}

std::string AudioHfpManager::GetAudioScoDeviceMac()
{
    return BluetoothScoManager::GetInstance().GetAudioScoDevice().GetDeviceAddr().c_str();
}

void AudioHfpListener::OnScoStateChanged(const BluetoothRemoteDevice &device, int state, int reason)
{
    HILOG_COMM_INFO("state:[%{public}d] reason:[%{public}d] device:[%{public}s]",
        state, reason, GetEncryptAddr(device.GetDeviceAddr()).c_str());
    // SCO_DISCONNECTED = 3, SCO_CONNECTING = 4, SCO_DISCONNECTING = 5, SCO_CONNECTED = 6
    HfpScoConnectState scoState = static_cast<HfpScoConnectState>(state);
    if (scoState == HfpScoConnectState::SCO_CONNECTED || scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        if (scoState == HfpScoConnectState::SCO_CONNECTED && reason == HFP_AG_SCO_REMOTE_USER_SET_UP) {
            AudioHfpManager::UpdateActiveHfpDevice(device);
        } else {
            bool isConnected = (scoState == HfpScoConnectState::SCO_CONNECTED) ? true : false;
            BluetoothScoManager::GetInstance().UpdateScoState(scoState, device, reason);
            HfpBluetoothDeviceManager::OnScoStateChanged(device, isConnected, reason);
        }
    }
}

ScoCategory AudioHfpManager::GetScoCategory()
{
    return JudgeScoCategory();
}

void AudioHfpListener::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
{
    AUDIO_WARNING_LOG("state: %{public}d device: %{public}s", state, GetEncryptAddr(device.GetDeviceAddr()).c_str());
    if (state == static_cast<int>(BTConnectState::CONNECTING)) {
        HfpBluetoothDeviceManager::SetHfpStack(device, BluetoothDeviceAction::CONNECTING_ACTION);
    }
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HfpBluetoothDeviceManager::SetHfpStack(device, BluetoothDeviceAction::CONNECT_ACTION);
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        AudioHfpManager::ClearCurrentActiveHfpDevice(device);
        HfpBluetoothDeviceManager::SetHfpStack(device, BluetoothDeviceAction::DISCONNECT_ACTION);
    }
}

void AudioHfpListener::OnHfpStackChanged(const BluetoothRemoteDevice &device, int action)
{
    AUDIO_WARNING_LOG("action: %{public}d device: %{public}s", action, GetEncryptAddr(device.GetDeviceAddr()).c_str());
    HfpBluetoothDeviceManager::SetHfpStack(device, action);
}

void AudioHfpListener::OnVirtualDeviceChanged(int32_t action, std::string macAddress)
{
    AUDIO_WARNING_LOG("action: %{public}d device: %{public}s", action, GetEncryptAddr(macAddress).c_str());
    if (action == static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_ADD)) {
        HfpBluetoothDeviceManager::SetHfpStack(BluetoothRemoteDevice(macAddress),
            BluetoothDeviceAction::VIRTUAL_DEVICE_ADD_ACTION);
    }
    if (action == static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_REMOVE)) {
        HfpBluetoothDeviceManager::SetHfpStack(BluetoothRemoteDevice(macAddress),
            BluetoothDeviceAction::VIRTUAL_DEVICE_REMOVE_ACTION);
    }
}
// LCOV_EXCL_STOP
} // namespace Bluetooth
} // namespace OHOS
