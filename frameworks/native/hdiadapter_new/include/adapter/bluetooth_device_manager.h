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

#ifndef BLUETOOTH_DEVICE_MANAGER_H
#define BLUETOOTH_DEVICE_MANAGER_H

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <set>
#include <mutex>
#include "audio_proxy_manager.h"
#include "adapter/i_device_manager.h"

namespace OHOS {
namespace AudioStandard {
typedef struct OHOS::HDI::Audio_Bluetooth::AudioAdapter BtAudioAdapter;
typedef struct OHOS::HDI::Audio_Bluetooth::AudioAdapterDescriptor BtAudioAdapterDescriptor;
typedef OHOS::HDI::Audio_Bluetooth::AudioRender BtAudioRender;
typedef OHOS::HDI::Audio_Bluetooth::AudioCapture BtAudioCapture;
typedef enum OHOS::HDI::Audio_Bluetooth::AudioPortDirection BtAudioPortDirection;
typedef struct OHOS::HDI::Audio_Bluetooth::AudioProxyManager BtAudioProxyManager;

typedef struct BluetoothAdapterWrapper {
    BtAudioAdapter *adapter_ = nullptr;
    std::mutex adapterMtx_;
    BtAudioAdapterDescriptor adapterDesc_ = {};
    std::unordered_map<uint32_t, BtAudioRender *> renders_;
    std::unordered_map<uint32_t, BtAudioCapture *> captures_;
    std::set<uint32_t> freeHdiRenderIdSet_;
    std::set<uint32_t> freeHdiCaptureIdSet_;
    std::mutex renderMtx_;
    std::mutex captureMtx_;
} BluetoothAdapterWrapper;

class BluetoothDeviceManager : public IDeviceManager {
public:
    BluetoothDeviceManager() = default;
    ~BluetoothDeviceManager();

    int32_t LoadAdapter(const std::string &adapterName) override;
    void UnloadAdapter(const std::string &adapterName, bool force = false) override;

    void AllAdapterSetMicMute(bool isMute) override;

    int32_t SetAudioParameter(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
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

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

    void SetAudioScene(const AudioScene scene) override;

private:
    void InitAudioManager(void);
    std::shared_ptr<BluetoothAdapterWrapper> GetAdapter(const std::string &adapterName, bool tryCreate = false);
    int32_t SwitchAdapterDesc(BtAudioAdapterDescriptor *descs, const std::string &adapterName, uint32_t size);
    uint32_t GetPortId(const std::string &adapterName, BtAudioPortDirection portFlag);
    uint32_t GetHdiRenderId(const std::string &adapterName);
    uint32_t GetHdiCaptureId(const std::string &adapterName);

private:
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 8;

    BtAudioProxyManager *audioManager_ = nullptr;
    void *handle_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<BluetoothAdapterWrapper> > adapters_;
    std::mutex adapterMtx_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // BLUETOOTH_DEVICE_MANAGER_H
