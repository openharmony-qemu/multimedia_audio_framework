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

#ifndef REMOTE_DEVICE_MANAGER_H
#define REMOTE_DEVICE_MANAGER_H

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <v1_0/iaudio_manager.h>
#include <v1_0/iaudio_callback.h>
#include <v1_0/audio_types.h>
#include "audio_info.h"
#include "adapter/i_device_manager.h"

namespace OHOS {
namespace AudioStandard {
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter RemoteIAudioAdapter;
typedef struct OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptor RemoteAudioAdapterDescriptor;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback RemoteIAudioCallback;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCallbackType RemoteAudioCallbackType;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey RemoteAudioExtParamKey;
typedef enum OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortDirection RemoteAudioPortDirection;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioManager RemoteIAudioManager;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRouteNode RemoteAudioRouteNode;

class RemoteAdapterHdiCallback final : public RemoteIAudioCallback {
public:
    RemoteAdapterHdiCallback(const std::string &adapterName);
    ~RemoteAdapterHdiCallback() override {}

    int32_t RenderCallback(RemoteAudioCallbackType type, int8_t &reserved, int8_t &cookie) override;
    int32_t ParamCallback(RemoteAudioExtParamKey key, const std::string &condition, const std::string &value,
        int8_t &reserved, int8_t cookie) override;

private:
    const std::string adapterName_;
};

typedef struct RemoteAdapterWrapper {
    RemoteAdapterWrapper(const std::string &adapterName) : hdiCallback_(adapterName) {}

    sptr<RemoteIAudioAdapter> adapter_ = nullptr;
    RemoteAudioAdapterDescriptor adapterDesc_ = {};
    std::unordered_set<uint32_t> hdiRenderIds_;
    std::unordered_set<uint32_t> hdiCaptureIds_;
    std::mutex renderMtx_;
    std::mutex captureMtx_;
    int32_t routeHandle_ = -1;
    std::unordered_map<uint32_t, IDeviceManagerCallback *> renderCallbacks_;
    std::unordered_map<uint32_t, IDeviceManagerCallback *> captureCallbacks_;
    std::mutex renderCallbackMtx_;
    std::mutex captureCallbackMtx_;
    RemoteAdapterHdiCallback hdiCallback_;
} RemoteAdapterWrapper;

class RemoteDeviceManager : public IDeviceManager {
public:
    RemoteDeviceManager() = default;
    ~RemoteDeviceManager() = default;

    int32_t LoadAdapter(const std::string &adapterName) override;
    void UnloadAdapter(const std::string &adapterName, bool force = false) override;

    void AllAdapterSetMicMute(bool isMute) override;

    void SetAudioParameter(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;
    std::string GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition) override;
    int32_t SetVoiceVolume(const std::string &adapterName, float volume) override;
    int32_t SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
        int32_t streamId) override;
    int32_t SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
        int32_t inputType) override;
    void SetMicMute(const std::string &adapterName, bool isMute) override;
    int32_t HandleEvent(const std::string &adapterName, const AudioParamKey key, const char *condition,
        const char *value, void *reserved) override;
    void RegistRenderSinkCallback(const std::string &adapterName, uint32_t hdiRenderId,
        IDeviceManagerCallback *callback) override;
    void RegistCaptureSourceCallback(const std::string &adapterName, uint32_t hdiCaptureId,
        IDeviceManagerCallback *callback) override;
    void UnRegistRenderSinkCallback(const std::string &adapterName, uint32_t hdiRenderId) override;
    void UnRegistCaptureSourceCallback(const std::string &adapterName, uint32_t hdiCaptureId) override;

    void *CreateRender(const std::string &adapterName, void *param, void *deviceDesc, uint32_t &hdiRenderId) override;
    void DestroyRender(const std::string &adapterName, uint32_t hdiRenderId) override;
    void *CreateCapture(const std::string &adapterName, void *param, void *deviceDesc, uint32_t &hdiCaptureId) override;
    void DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId) override;

    void DumpInfo(std::string &dumpString) override;

private:
    void InitAudioManager(void);
    std::shared_ptr<RemoteAdapterWrapper> GetAdapter(const std::string &adapterName, bool tryCreate = false);
    int32_t SwitchAdapterDesc(const std::vector<RemoteAudioAdapterDescriptor> &descs, const std::string &adapterName);
    uint32_t GetPortId(RemoteAudioPortDirection portFlag);
    int32_t HandleStateChangeEvent(const std::string &adapterName, const AudioParamKey key, const char *condition,
        const char *value);
    int32_t HandleRenderParamEvent(const std::string &adapterName, const AudioParamKey key, const char *condition,
        const char *value);
    int32_t HandleCaptureParamEvent(const std::string &adapterName, const AudioParamKey key, const char *condition,
        const char *value);
    int32_t SetOutputPortPin(DeviceType outputDevice, RemoteAudioRouteNode &sink);
    int32_t SetInputPortPin(DeviceType inputDevice, RemoteAudioRouteNode &source);

private:
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr int32_t EVENT_DES_SIZE = 60;
    static constexpr int32_t ADAPTER_STATE_CONTENT_DES_SIZE = 60;
    static constexpr int32_t PARAMS_STATE_NUM = 2;
    static constexpr char DAUDIO_DEV_TYPE_SPK = '1';
    static constexpr char DAUDIO_DEV_TYPE_MIC = '2';

    sptr<RemoteIAudioManager> audioManager_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<RemoteAdapterWrapper> > adapters_;
    std::mutex adapterMtx_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_DEVICE_MANAGER_H
