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

#ifndef LOCAL_DEVICE_MANAGER_H
#define LOCAL_DEVICE_MANAGER_H

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include "v4_0/iaudio_manager.h"
#include "hdf_remote_service.h"
#include "adapter/i_device_manager.h"

namespace OHOS {
namespace AudioStandard {
typedef struct LocalAdapterWrapper {
    struct IAudioAdapter *adapter_ = nullptr;
    struct AudioAdapterDescriptor adapterDesc_ = {};
    std::unordered_set<uint32_t> hdiRenderIds_;
    std::unordered_set<uint32_t> hdiCaptureIds_;
    std::mutex renderMtx_;
    std::mutex captureMtx_;
    int32_t routeHandle_ = -1;
} LocalAdapterWrapper;

typedef struct LocalParameter {
    std::string adapterName_ = "";
    AudioParamKey key_ = AudioParamKey::NONE;
    std::string condition_ = "";
    std::string value_ = "";
} LocalParameter;

class LocalDeviceManager : public IDeviceManager {
public:
    LocalDeviceManager() = default;
    ~LocalDeviceManager() = default;

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

    void *CreateRender(const std::string &adapterName, void *param, void *deviceDesc, uint32_t &hdiRenderId) override;
    void DestroyRender(const std::string &adapterName, uint32_t hdiRenderId) override;
    void *CreateCapture(const std::string &adapterName, void *param, void *deviceDesc, uint32_t &hdiCaptureId) override;
    void DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId) override;

    void DumpInfo(std::string &dumpString) override;

private:
    void InitAudioManager(void);
    std::shared_ptr<LocalAdapterWrapper> GetAdapter(const std::string &adapterName, bool tryCreate = false);
    int32_t SwitchAdapterDesc(struct AudioAdapterDescriptor *descs, const std::string &adapterName, uint32_t size);
    uint32_t GetPortId(const std::string &adapterName, enum AudioPortDirection portFlag);
    int32_t SetOutputPortPin(DeviceType outputDevice, AudioRouteNode &sink);
    int32_t SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source);
    void SaveSetParameter(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value);

private:
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr uid_t UID_BLUETOOTH_SA = 1002;

    struct IAudioManager *audioManager_ = nullptr;
    struct HdfRemoteService *hdfRemoteService_ = nullptr;
    struct HdfDeathRecipient *hdfDeathRecipient_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<LocalAdapterWrapper> > adapters_;
    std::mutex adapterMtx_;
    std::vector<LocalParameter> reSetParams_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // LOCAL_DEVICE_MANAGER_H
