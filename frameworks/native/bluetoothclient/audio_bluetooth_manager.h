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

#ifndef AUDIO_BLUETOOTH_MANAGERI_H
#define AUDIO_BLUETOOTH_MANAGERI_H

#include "bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_codec.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_hfp_ag.h"
#include "audio_info.h"
#include "bluetooth_device_utils.h"

namespace OHOS {
namespace Bluetooth {
class AudioA2dpPlayingStateChangedListener {
public:
    virtual void OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState) = 0;
};

// Audio bluetooth a2dp feature support
class AudioA2dpListener : public A2dpSourceObserver {
public:
    AudioA2dpListener() = default;
    virtual ~AudioA2dpListener() = default;

    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause);
    virtual void OnConfigurationChanged(const BluetoothRemoteDevice &device, const A2dpCodecInfo &info, int error);
    virtual void OnPlayingStatusChanged(const BluetoothRemoteDevice &device, int playingState, int error);
    virtual void OnMediaStackChanged(const BluetoothRemoteDevice &device, int action);
    virtual void OnVirtualDeviceChanged(int32_t action, std::string macAddress);

private:
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AudioA2dpListener);
};

class AudioA2dpManager {
public:
    AudioA2dpManager() = default;
    virtual ~AudioA2dpManager() = default;
    static void RegisterBluetoothA2dpListener();
    static void UnregisterBluetoothA2dpListener();
    static void DisconnectBluetoothA2dpSink();
    static int32_t SetActiveA2dpDevice(const std::string& macAddress);
    static std::string GetActiveA2dpDevice();
    static int32_t SetDeviceAbsVolume(const std::string& macAddress, int32_t volume);
    static int32_t GetA2dpDeviceStreamInfo(const std::string& macAddress,
        AudioStandard::AudioStreamInfo &streamInfo);
    static bool HasA2dpDeviceConnected();
    static void CheckA2dpDeviceReconnect();
    static int32_t A2dpOffloadSessionRequest(const std::vector<A2dpStreamInfo> &info);
    static int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionsID);
    static int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionsID);
    static int32_t GetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);
    static int32_t RegisterA2dpPlayingStateChangedListener(
        std::shared_ptr<AudioA2dpPlayingStateChangedListener> listener);
    static void OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState);
    static int32_t Connect(const std::string &macAddress);

    static void SetConnectionState(int state)
    {
        connectionState_ = state;
    }
    static int GetConnectionState()
    {
        return connectionState_;
    }
    static BluetoothRemoteDevice GetCurrentActiveA2dpDevice()
    {
        return activeA2dpDevice_;
    }

private:
    static A2dpSource *a2dpInstance_;
    static std::shared_ptr<AudioA2dpListener> a2dpListener_;
    static int connectionState_;
    static BluetoothRemoteDevice activeA2dpDevice_;
    static std::vector<std::shared_ptr<AudioA2dpPlayingStateChangedListener>> a2dpPlayingStateChangedListeners_;
};

// Audio bluetooth sco feature support
class AudioHfpListener : public HandsFreeAudioGatewayObserver {
public:
    AudioHfpListener() = default;
    virtual ~AudioHfpListener() = default;

    void OnScoStateChanged(const BluetoothRemoteDevice &device, int state, int reason);
    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause);
    void OnActiveDeviceChanged(const BluetoothRemoteDevice &device) {}
    void OnHfEnhancedDriverSafetyChanged(const BluetoothRemoteDevice &device, int indValue) {}
    void OnVirtualDeviceChanged(int32_t action, std::string macAddress);
    virtual void OnHfpStackChanged(const BluetoothRemoteDevice &device, int action);

private:
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AudioHfpListener);
};

class AudioHfpManager {
public:
    AudioHfpManager() = default;
    virtual ~AudioHfpManager() = default;
    static void RegisterBluetoothScoListener();
    static void UnregisterBluetoothScoListener();
    static int32_t SetActiveHfpDevice(const std::string &macAddress);
    static std::string GetActiveHfpDevice();
    static int32_t ConnectScoWithAudioScene(AudioStandard::AudioScene scene);
    static int32_t DisconnectSco();
    static int8_t GetScoCategoryFromScene(AudioStandard::AudioScene scene);
    static void DisconnectBluetoothHfpSink();
    static void UpdateCurrentActiveHfpDevice(const BluetoothRemoteDevice &device);
    static std::string GetCurrentActiveHfpDevice();
    static void UpdateAudioScene(AudioStandard::AudioScene scene);
    static void CheckHfpDeviceReconnect();
    static AudioStandard::AudioScene GetCurrentAudioScene();
    static AudioStandard::AudioScene GetPolicyAudioScene();
    static void SetAudioSceneFromPolicy(AudioStandard::AudioScene scene);
    static int32_t HandleScoWithRecongnition(bool handleFlag, BluetoothRemoteDevice &device);
    static ScoCategory GetScoCategory();
    static int32_t Connect(const std::string &macAddress);

private:
    static HandsFreeAudioGateway *hfpInstance_;
    static std::shared_ptr<AudioHfpListener> hfpListener_;
    static AudioStandard::AudioScene scene_;
    static AudioStandard::AudioScene sceneFromPolicy_;
    static BluetoothRemoteDevice activeHfpDevice_;
    static ScoCategory scoCategory;
};
}
}
#endif  // AUDIO_BLUETOOTH_MANAGERI_H
