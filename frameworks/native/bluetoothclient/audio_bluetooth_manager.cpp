/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "audio_bluetooth_manager.h"
#include "bluetooth_def.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_device_utils.h"
#include "bluetooth_hfp_ag.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

A2dpSource *AudioA2dpManager::a2dpInstance_ = nullptr;
std::shared_ptr<AudioA2dpListener> AudioA2dpManager::a2dpListener_ = std::make_shared<AudioA2dpListener>();
int AudioA2dpManager::connectionState_ = static_cast<int>(BTConnectState::DISCONNECTED);
BluetoothRemoteDevice AudioA2dpManager::activeA2dpDevice_;
std::mutex g_a2dpInstanceLock;
HandsFreeAudioGateway *AudioHfpManager::hfpInstance_ = nullptr;
std::shared_ptr<AudioHfpListener> AudioHfpManager::hfpListener_ = std::make_shared<AudioHfpListener>();
AudioScene AudioHfpManager::scene_ = AUDIO_SCENE_DEFAULT;
AudioScene AudioHfpManager::sceneFromPolicy_ = AUDIO_SCENE_DEFAULT;
OHOS::Bluetooth::ScoCategory AudioHfpManager::scoCategory = OHOS::Bluetooth::ScoCategory::SCO_DEFAULT;
OHOS::Bluetooth::RecognitionStatus AudioHfpManager::recognitionStatus =
    OHOS::Bluetooth::RecognitionStatus::RECOGNITION_DISCONNECTED;
BluetoothRemoteDevice AudioHfpManager::activeHfpDevice_;
std::vector<std::shared_ptr<AudioA2dpPlayingStateChangedListener>> AudioA2dpManager::a2dpPlayingStateChangedListeners_;
std::mutex g_activehfpDeviceLock;
std::mutex g_audioSceneLock;
std::mutex g_hfpInstanceLock;
std::mutex g_a2dpPlayingStateChangedLock;

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
    AUDIO_INFO_LOG("AudioA2dpManager::RegisterBluetoothA2dpListener");
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "Failed to obtain A2DP profile instance");
    a2dpInstance_->RegisterObserver(a2dpListener_);
}

void AudioA2dpManager::UnregisterBluetoothA2dpListener()
{
    AUDIO_INFO_LOG("AudioA2dpManager::UnregisterBluetoothA2dpListener");
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "A2DP profile instance unavailable");
    a2dpInstance_->DeregisterObserver(a2dpListener_);
    a2dpInstance_ = nullptr;
}

void AudioA2dpManager::DisconnectBluetoothA2dpSink()
{
    int connectionState = static_cast<int>(BTConnectState::DISCONNECTED);
    auto a2dpList = MediaBluetoothDeviceManager::GetAllA2dpBluetoothDevice();
    for (const auto &device : a2dpList) {
        a2dpListener_->OnConnectionStateChanged(device, connectionState,
            static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
    }

    auto virtualDevices = MediaBluetoothDeviceManager::GetA2dpVirtualDeviceList();
    for (const auto &virtualDevice : virtualDevices) {
        a2dpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_REMOVE),
            virtualDevice.GetDeviceAddr());
    }

    MediaBluetoothDeviceManager::ClearAllA2dpBluetoothDevice();
}

int32_t AudioA2dpManager::SetActiveA2dpDevice(const std::string& macAddress)
{
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    AUDIO_WARNING_LOG("incoming device:%{public}s, current device:%{public}s",
        GetEncryptAddr(macAddress).c_str(), GetEncryptAddr(activeA2dpDevice_.GetDeviceAddr()).c_str());
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "Failed to obtain A2DP profile instance");
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
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, "", "Failed to obtain A2DP profile instance");
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
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "Failed to obtain A2DP profile instance");
    BluetoothRemoteDevice device;
    int32_t ret = MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        "GetA2dpDeviceStreamInfo: the configuring A2DP device doesn't exist.");
    A2dpCodecStatus codecStatus = a2dpInstance_->GetCodecStatus(device);
    bool result = GetAudioStreamInfo(codecStatus.codecInfo, streamInfo);
    CHECK_AND_RETURN_RET_LOG(result, ERROR, "GetA2dpDeviceStreamInfo: Unsupported a2dp codec info");
    return SUCCESS;
}

bool AudioA2dpManager::HasA2dpDeviceConnected()
{
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_RET(a2dpInstance_, false);
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices;
    a2dpInstance_->GetDevicesByStates(states, devices);

    return !devices.empty();
}

int32_t AudioA2dpManager::A2dpOffloadSessionRequest(const std::vector<A2dpStreamInfo> &info)
{
    CHECK_AND_RETURN_RET_LOG(activeA2dpDevice_.GetDeviceAddr() != "00:00:00:00:00:00", A2DP_NOT_OFFLOAD,
        "Invalid mac address, not request, return A2DP_NOT_OFFLOAD.");
    int32_t ret = a2dpInstance_->A2dpOffloadSessionRequest(activeA2dpDevice_, info);
    AUDIO_DEBUG_LOG("Request %{public}zu stream and return a2dp offload state %{public}d", info.size(), ret);
    return ret;
}

int32_t AudioA2dpManager::OffloadStartPlaying(const std::vector<int32_t> &sessionsID)
{
    CHECK_AND_RETURN_RET_LOG(activeA2dpDevice_.GetDeviceAddr() != "00:00:00:00:00:00", ERROR,
        "Invalid mac address, not start, return error.");
    AUDIO_DEBUG_LOG("Start playing %{public}zu stream", sessionsID.size());
    return a2dpInstance_->OffloadStartPlaying(activeA2dpDevice_, sessionsID);
}

int32_t AudioA2dpManager::OffloadStopPlaying(const std::vector<int32_t> &sessionsID)
{
    if (activeA2dpDevice_.GetDeviceAddr() == "00:00:00:00:00:00") {
        AUDIO_DEBUG_LOG("Invalid mac address, not stop, return error.");
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Stop playing %{public}zu stream", sessionsID.size());
    return a2dpInstance_->OffloadStopPlaying(activeA2dpDevice_, sessionsID);
}

int32_t AudioA2dpManager::GetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    if (activeA2dpDevice_.GetDeviceAddr() == "00:00:00:00:00:00") {
        AUDIO_DEBUG_LOG("Invalid mac address, return error.");
        return ERROR;
    }
    return ERROR;
    // a2dpInstance_->GetRenderPosition(activeA2dpDevice_, delayValue, sendDataSize, timeStamp);
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
    if (a2dpInstance_ == nullptr) {
        a2dpInstance_ = A2dpSource::GetProfile();
    }
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "A2DP profile instance unavailable");
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
    AUDIO_INFO_LOG("OnMediaStackChanged, action: %{public}d", action);
    MediaBluetoothDeviceManager::SetMediaStack(device, action);
}

void AudioA2dpListener::OnVirtualDeviceChanged(int32_t action, std::string address)
{
    AUDIO_WARNING_LOG("action: %{public}d, macAddress: %{public}s", action,
        GetEncryptAddr(address).c_str());
    if (action == static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_ADD)) {
        MediaBluetoothDeviceManager::SetMediaStack(BluetoothRemoteDevice(address),
            BluetoothDeviceAction::VIRTUAL_DEVICE_ADD_ACTION);
    }
    if (action == static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_REMOVE)) {
        MediaBluetoothDeviceManager::SetMediaStack(BluetoothRemoteDevice(address),
            BluetoothDeviceAction::VIRTUAL_DEVICE_REMOVE_ACTION);
    }
}

void AudioHfpManager::RegisterBluetoothScoListener()
{
    AUDIO_INFO_LOG("AudioHfpManager::RegisterBluetoothScoListener");
    std::lock_guard<std::mutex> hfpLock(g_hfpInstanceLock);
    hfpInstance_ = HandsFreeAudioGateway::GetProfile();
    CHECK_AND_RETURN_LOG(hfpInstance_ != nullptr, "Failed to obtain HFP AG profile instance");

    hfpInstance_->RegisterObserver(hfpListener_);
}

void AudioHfpManager::UnregisterBluetoothScoListener()
{
    AUDIO_INFO_LOG("AudioHfpManager::UnregisterBluetoothScoListene");
    std::lock_guard<std::mutex> hfpLock(g_hfpInstanceLock);
    CHECK_AND_RETURN_LOG(hfpInstance_ != nullptr, "HFP AG profile instance unavailable");

    hfpInstance_->DeregisterObserver(hfpListener_);
    hfpInstance_ = nullptr;
}

void AudioHfpManager::CheckHfpDeviceReconnect()
{
    if (hfpInstance_ == nullptr) {
        hfpInstance_ = HandsFreeAudioGateway::GetProfile();
    }
    CHECK_AND_RETURN_LOG(hfpInstance_ != nullptr, "HFP profile instance unavailable");
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices = hfpInstance_->GetDevicesByStates(states);
    for (auto &device : devices) {
        hfpListener_->OnConnectionStateChanged(device, static_cast<int32_t>(BTConnectState::CONNECTED),
            static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));

        int32_t wearState = 0; // 0 unwear state
        if (IsBTWearDetectionEnable(device)) {
            wearState = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
            if (wearState == 1) HfpBluetoothDeviceManager::SetHfpStack(device, WEAR_ACTION); // 1 wear state
        }
        AUDIO_INFO_LOG("reconnect hfp device:%{public}s, wear state:%{public}d",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(), wearState);
    }

    std::vector<std::string> virtualDevices;
    hfpInstance_->GetVirtualDeviceList(virtualDevices);
    for (auto &macAddress : virtualDevices) {
        AUDIO_PRERELEASE_LOGI("reconnect virtual hfp device:%{public}s", GetEncryptAddr(macAddress).c_str());
        hfpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_ADD), macAddress);
    }
}

int32_t AudioHfpManager::HandleScoWithRecongnition(bool handleFlag, BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    bool ret = true;
    if (handleFlag) {
        int8_t scoCategory = GetScoCategoryFromScene(scene_);
        if (scoCategory == ScoCategory::SCO_DEFAULT &&
            AudioHfpManager::scoCategory != ScoCategory::SCO_RECOGNITION) {
            AUDIO_INFO_LOG("Recongnition sco connect");
            AudioHfpManager::recognitionStatus = RecognitionStatus::RECOGNITION_CONNECTING;
            ret = hfpInstance_->OpenVoiceRecognition(device);
            if (ret) {
                AudioHfpManager::scoCategory = ScoCategory::SCO_RECOGNITION;
                AudioHfpManager::recognitionStatus = RecognitionStatus::RECOGNITION_CONNECTED;
            }
        } else {
            AUDIO_WARNING_LOG("Sco Connected OR Connecting, No Need to Create");
        }
    } else {
        if (AudioHfpManager::scoCategory == ScoCategory::SCO_RECOGNITION) {
            AUDIO_INFO_LOG("Recongnition sco close");
            AudioHfpManager::recognitionStatus = RecognitionStatus::RECOGNITION_DISCONNECTING;
            ret = hfpInstance_->CloseVoiceRecognition(device);
            if (ret) {
                AudioHfpManager::scoCategory = ScoCategory::SCO_DEFAULT;
                AudioHfpManager::recognitionStatus = RecognitionStatus::RECOGNITION_DISCONNECTED;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(ret == true, ERROR, "HandleScoWithRecongnition failed, result: %{public}d", ret);
    return SUCCESS;
}

void AudioHfpManager::ClearRecongnitionStatus()
{
    if (AudioHfpManager::scoCategory == ScoCategory::SCO_RECOGNITION) {
        AudioHfpManager::scoCategory = ScoCategory::SCO_DEFAULT;
        AudioHfpManager::recognitionStatus = RecognitionStatus::RECOGNITION_DISCONNECTED;
    }
}

ScoCategory AudioHfpManager::GetScoCategory()
{
    return scoCategory;
}

RecognitionStatus AudioHfpManager::GetRecognitionStatus()
{
    return recognitionStatus;
}

int32_t AudioHfpManager::SetActiveHfpDevice(const std::string &macAddress)
{
    BluetoothRemoteDevice device;
    if (HfpBluetoothDeviceManager::GetConnectedHfpBluetoothDevice(macAddress, device) != SUCCESS) {
        AUDIO_ERR_LOG("SetActiveHfpDevice failed for the HFP device %{public}s does not exist.",
            GetEncryptAddr(macAddress).c_str());
        return ERROR;
    }
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    AUDIO_INFO_LOG("incoming device:%{public}s, current device:%{public}s",
        GetEncryptAddr(macAddress).c_str(), GetEncryptAddr(activeHfpDevice_.GetDeviceAddr()).c_str());
    if (macAddress != activeHfpDevice_.GetDeviceAddr()) {
        AUDIO_WARNING_LOG("Active hfp device is changed, need to DisconnectSco for current activeHfpDevice.");
        int32_t ret = DisconnectSco();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "DisconnectSco failed, result: %{public}d", ret);
    }
    std::lock_guard<std::mutex> hfpLock(g_hfpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    bool res = hfpInstance_->SetActiveDevice(device);
    CHECK_AND_RETURN_RET_LOG(res == true, ERROR, "SetActiveHfpDevice failed, result: %{public}d", res);
    activeHfpDevice_ = device;
    return SUCCESS;
}

std::string AudioHfpManager::GetActiveHfpDevice()
{
    std::lock_guard<std::mutex> hfpLock(g_hfpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, "", "HFP AG profile instance unavailable");
    BluetoothRemoteDevice device = hfpInstance_->GetActiveDevice();
    return device.GetDeviceAddr();
}

int32_t AudioHfpManager::ConnectScoWithAudioScene(AudioScene scene)
{
    if (scoCategory == ScoCategory::SCO_RECOGNITION) {
        AUDIO_WARNING_LOG("Recognition Sco Connected");
        return SUCCESS;
    }
    std::lock_guard<std::mutex> sceneLock(g_audioSceneLock);
    int8_t lastScoCategory = GetScoCategoryFromScene(scene_);
    int8_t newScoCategory = GetScoCategoryFromScene(scene);
    AUDIO_INFO_LOG("new sco category is %{public}d, last sco category is %{public}d", newScoCategory, lastScoCategory);

    if (lastScoCategory == newScoCategory) {
        AUDIO_INFO_LOG("sco category %{public}d not change", newScoCategory);
        return SUCCESS;
    }
    std::lock_guard<std::mutex> hfpLock(g_hfpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    bool isInbardingEnabled = false;
    hfpInstance_->IsInbandRingingEnabled(isInbardingEnabled);
    if ((scene == AUDIO_SCENE_RINGING || scene == AUDIO_SCENE_VOICE_RINGING) && !isInbardingEnabled) {
        AUDIO_WARNING_LOG("The inbarding switch is off, ignore the ring scene.");
        return SUCCESS;
    }
    int32_t ret;
    if (lastScoCategory != ScoCategory::SCO_DEFAULT) {
        AUDIO_INFO_LOG("Entered to disConnectSco for last audioScene category.");
        ret = hfpInstance_->DisconnectSco(static_cast<uint8_t>(lastScoCategory));
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR,
            "ConnectScoWithAudioScene failed as the last SCO failed to be disconnected, result: %{public}d", ret);
    }
    if (newScoCategory != ScoCategory::SCO_DEFAULT) {
        AUDIO_INFO_LOG("Entered to connectSco for new audioScene category.");
        ret = hfpInstance_->ConnectSco(static_cast<uint8_t>(newScoCategory));
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "ConnectScoWithAudioScene failed, result: %{public}d", ret);
    }
    scene_ = scene;
    return SUCCESS;
}

int32_t AudioHfpManager::DisconnectSco()
{
    std::lock_guard<std::mutex> sceneLock(g_audioSceneLock);
    int8_t currentScoCategory = GetScoCategoryFromScene(scene_);
    if (currentScoCategory == ScoCategory::SCO_DEFAULT) {
        AUDIO_WARNING_LOG("Current sco category is DEFAULT, not need to disconnect sco.");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("current sco category %{public}d", currentScoCategory);

    std::lock_guard<std::mutex> hfpLock(g_hfpInstanceLock);
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    int32_t ret = hfpInstance_->DisconnectSco(static_cast<uint8_t>(currentScoCategory));
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "DisconnectSco failed, result: %{public}d", ret);
    scene_ = AUDIO_SCENE_DEFAULT;
    return SUCCESS;
}

int8_t AudioHfpManager::GetScoCategoryFromScene(AudioScene scene)
{
    switch (scene) {
        case AUDIO_SCENE_VOICE_RINGING:
        case AUDIO_SCENE_PHONE_CALL:
            return ScoCategory::SCO_CALLULAR;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_PHONE_CHAT:
            return ScoCategory::SCO_VIRTUAL;
        default:
            return ScoCategory::SCO_DEFAULT;
    }
}

void AudioHfpManager::DisconnectBluetoothHfpSink()
{
    int connectionState = static_cast<int>(BTConnectState::DISCONNECTED);
    auto hfpList = HfpBluetoothDeviceManager::GetAllHfpBluetoothDevice();
    for (const auto &device : hfpList) {
        hfpListener_->OnConnectionStateChanged(device, connectionState,
            static_cast<uint32_t>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
    }

    auto virtualDevices = HfpBluetoothDeviceManager::GetHfpVirtualDeviceList();
    for (const auto &virtualDevice : virtualDevices) {
        hfpListener_->OnVirtualDeviceChanged(static_cast<int32_t>(Bluetooth::BT_VIRTUAL_DEVICE_REMOVE),
            virtualDevice.GetDeviceAddr());
    }
    HfpBluetoothDeviceManager::ClearAllHfpBluetoothDevice();
}

void AudioHfpManager::UpdateCurrentActiveHfpDevice(const BluetoothRemoteDevice &device)
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    activeHfpDevice_ = device;
}

std::string AudioHfpManager::GetCurrentActiveHfpDevice()
{
    std::lock_guard<std::mutex> hfpDeviceLock(g_activehfpDeviceLock);
    return activeHfpDevice_.GetDeviceAddr();
}

void AudioHfpManager::UpdateAudioScene(AudioScene scene)
{
    std::lock_guard<std::mutex> sceneLock(g_audioSceneLock);
    scene_ = scene;
}

AudioStandard::AudioScene AudioHfpManager::GetCurrentAudioScene()
{
    std::lock_guard<std::mutex> sceneLock(g_audioSceneLock);
    return scene_;
}

void AudioHfpManager::SetAudioSceneFromPolicy(AudioScene scene)
{
    sceneFromPolicy_ = scene;
}

AudioStandard::AudioScene AudioHfpManager::GetPolicyAudioScene()
{
    return sceneFromPolicy_;
}

int32_t AudioHfpManager::Connect(const std::string &macAddress)
{
    CHECK_AND_RETURN_RET_LOG(hfpInstance_ != nullptr, ERROR, "HFP AG profile instance unavailable");
    BluetoothRemoteDevice virtualDevice = BluetoothRemoteDevice(macAddress);
    if (HfpBluetoothDeviceManager::IsHfpBluetoothDeviceConnecting(macAddress)) {
        AUDIO_WARNING_LOG("Hfp device %{public}s is connecting, ignore connect request",
            GetEncryptAddr(macAddress).c_str());
        virtualDevice.SetVirtualAutoConnectType(CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG, 0);
        return SUCCESS;
    }
    std::vector<std::string> virtualDevices;
    hfpInstance_->GetVirtualDeviceList(virtualDevices);
    if (std::find(virtualDevices.begin(), virtualDevices.end(), macAddress) == virtualDevices.end()) {
        AUDIO_WARNING_LOG("Hfp device %{public}s is not virtual device, ignore connect request",
            GetEncryptAddr(macAddress).c_str());
        return SUCCESS;
    }
    int32_t ret = hfpInstance_->Connect(virtualDevice);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Hfp Connect Failed");
    virtualDevice.SetVirtualAutoConnectType(CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG, 0);
    return SUCCESS;
}

void AudioHfpListener::OnScoStateChanged(const BluetoothRemoteDevice &device, int state, int reason)
{
    AUDIO_WARNING_LOG("state:[%{public}d] reason:[%{public}d] device:[%{public}s]",
        state, reason, GetEncryptAddr(device.GetDeviceAddr()).c_str());
    // SCO_DISCONNECTED = 3, SCO_CONNECTING = 4, SCO_DISCONNECTING = 5, SCO_CONNECTED = 6
    HfpScoConnectState scoState = static_cast<HfpScoConnectState>(state);
    if (scoState == HfpScoConnectState::SCO_CONNECTED || scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        if (device.GetDeviceAddr() == AudioHfpManager::GetCurrentActiveHfpDevice() &&
            scoState == HfpScoConnectState::SCO_DISCONNECTED) {
            BluetoothRemoteDevice defaultDevice;
            AudioHfpManager::UpdateCurrentActiveHfpDevice(defaultDevice);
            AUDIO_WARNING_LOG("Sco disconnect, need set audio scene as default.");
            AudioHfpManager::UpdateAudioScene(AUDIO_SCENE_DEFAULT);
        } else if (scoState == HfpScoConnectState::SCO_CONNECTED && reason == HFP_AG_SCO_REMOTE_USER_SET_UP) {
            AudioScene audioScene = AudioHfpManager::GetPolicyAudioScene();
            if (audioScene != AudioHfpManager::GetCurrentAudioScene()) {
                AUDIO_WARNING_LOG("Sco connect by peripheral device, update scene_ %{public}d", audioScene);
                AudioHfpManager::UpdateAudioScene(audioScene);
            }
            AudioHfpManager::UpdateCurrentActiveHfpDevice(device);
        }
        bool isConnected = (scoState == HfpScoConnectState::SCO_CONNECTED) ? true : false;

        // VGS feature
        bool isVgsSupported = false;
        if (isConnected) {
            HandsFreeAudioGateway *hfpInstance = HandsFreeAudioGateway::GetProfile();
            CHECK_AND_RETURN_LOG(hfpInstance != nullptr, "Failed to obtain HFP AG profile instance");
            hfpInstance->IsVgsSupported(device, isVgsSupported);
        }
        AUDIO_INFO_LOG("AudioHfpListener::OnScoStateChanged: isVgsSupported: [%{public}d]", isVgsSupported);
        HfpBluetoothDeviceManager::OnScoStateChanged(device, isVgsSupported, reason);
    }
}

void AudioHfpListener::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
{
    AUDIO_WARNING_LOG("state: %{public}d device: %{public}s", state,
        GetEncryptAddr(device.GetDeviceAddr()).c_str());
    if (state == static_cast<int>(BTConnectState::CONNECTING)) {
        HfpBluetoothDeviceManager::SetHfpStack(device, BluetoothDeviceAction::CONNECTING_ACTION);
    }
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HfpBluetoothDeviceManager::SetHfpStack(device, BluetoothDeviceAction::CONNECT_ACTION);
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        if (device.GetDeviceAddr() == AudioHfpManager::GetCurrentActiveHfpDevice()) {
            BluetoothRemoteDevice defaultDevice;
            AudioHfpManager::UpdateCurrentActiveHfpDevice(defaultDevice);
            AUDIO_WARNING_LOG("Current active hfp device diconnect, need set audio scene as default.");
            AudioHfpManager::UpdateAudioScene(AUDIO_SCENE_DEFAULT);
        }
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
