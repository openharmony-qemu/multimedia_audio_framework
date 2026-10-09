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

#ifndef VA_DEVICE_MANAGER_H
#define VA_DEVICE_MANAGER_H

#include <functional>
#include <iremote_broker.h>
#include <unordered_map>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_device_descriptor.h"
#include "va_device.h"
#include "va_device_info.h"
#include "audio_core_service.h"

#include "ashmem.h"
#include <sys/mman.h>
#include <unistd.h>
#include "message_parcel.h"

#include "va_shared_buffer_operator.h"
#include "va_shared_buffer.h"

#include "audio_pipe_manager.h"

#include "audio_definition_adapter_info.h"

#include "iv_a_device_controller.h"
#include "iremote_object.h"


namespace OHOS {
namespace AudioStandard {

class VADeviceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit VADeviceDeathRecipient(std::function<void()> callback) : callback_(std::move(callback)) {}
    ~VADeviceDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        if (callback_) {
            callback_();
        }
    }
private:
    std::function<void()> callback_;
};

class VADeviceManager {
public:
    static VADeviceManager &GetInstance();

    void OnDevicesConnected(
        const std::shared_ptr<VADevice> &vaDevice, const sptr<IVADeviceController> &controller);

    void OnDevicesDisconnected(const std::shared_ptr<VADevice> &vaDevice);

    void SetAssociatedMacAddresses(const std::string &macAddress, bool isEnable);

    void SetNoiseReductionMode(const std::shared_ptr<VADevice> &vaDevice, int32_t noiseReductionMode);

    void GetDeviceController(const std::string macAddr, sptr<IRemoteObject> &controller);
private:
    VADeviceManager() = default;
    virtual ~VADeviceManager() = default;

    AudioPolicyConfigData &config_ = AudioPolicyConfigData::GetInstance();

    std::unordered_map<std::string, sptr<IVADeviceController>> connectedVADeviceMap_;

    std::mutex statusMutex_;

    void AddVADeviceToAdapter(const std::shared_ptr<VADevice> &vaDevice);

    void CreateVAPipeAndDevice(const std::shared_ptr<PolicyAdapterInfo> &adapterInfoPtr,
        const std::string &pipeName, const std::string &deviceName, DeviceType deviceType);

    void GetVAPipeAndDeviceName(const VADeviceConfiguration &config,
        std::string &pipeName, std::string &deviceName, DeviceType &deviceType);

    void AddVAStreamPropToMap(std::list<VAAudioStreamProperty> properties,
        const std::string &pipeName, const std::string &deviceName);

    std::shared_ptr<AudioDeviceDescriptor> ConvertVADeviceToDescriptor(const std::shared_ptr<VADevice> &vaDevice);

    std::shared_ptr<DeviceStreamInfo> ConvertVAStreamPropertyToInfo(const VAAudioStreamProperty &vaStreamProperty);

    std::shared_ptr<PipeStreamPropInfo> ConvertVADeviceStreamPropertyToPipeStreamPropInfo(
        const VAAudioStreamProperty &vaStreamProperty);

    uint32_t CalculateBufferSize(const VAAudioStreamProperty &vaStreamProperty);

    void UpdateNoiseReductionCapability(const std::shared_ptr<VADevice> &vaDevice,
        const std::shared_ptr<AudioDeviceDescriptor> &descriptor);
};
} //namespace AudioStandard
} //namespace OHOS
#endif //VA_DEVICE_MANAGER_H

