/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_ACCESSORY_H
#define AUDIO_ACCESSORY_H

#include "native_audio_accessory_common.h"
#include "native_audio_accessory_manager.h"
#include "va_device.h"
#include "virtual_audio_interface.h"
#include "audio_policy_interface.h"
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace OHOS {
namespace AudioStandard {

class AudioAccessoryInputStream;
class AudioAccessory;
// AudioAccessory: Accessory instance with actual logic
// Wraps Inner API VADevice and manages streams
// Implements VADeviceControllerCallback for stream control
class AudioAccessory : public VADevice, public VADeviceControllerCallback {
public:
    // VADeviceControllerCallback implementation
    int32_t OpenInputStream(const VAAudioStreamProperty &prop, const VAInputStreamAttribute &attr,
                            std::shared_ptr<VAInputStreamCallback> &inputStream) override;
    int32_t GetParameters(const std::string& key, std::string& value) override;
    int32_t SetParameters(const std::string& key, const std::string& value) override;
    AudioAccessory(
        const OH_AudioAccessoryInfo& info,
        OH_AudioAccessory_SetNoiseReductionCallback onNoiseReduction,
        OH_AudioAccessory_OpenInputStreamCallback onOpenInputStream);
    ~AudioAccessory() override = default;

    std::shared_ptr<VAAudioStreamProperty> GetStreamProperty() const;
    std::shared_ptr<VAInputStreamAttribute> GetInputStreamAttribute() const;
    std::shared_ptr<VAInputStreamCallback> GetInputStreamCallback();

    int32_t OpenInputStreamInternal(std::shared_ptr<VAInputStreamCallback>& inputStream,
                                    OH_AudioAccessory* accessoryHandle,
                                    OH_AudioAccessoryInputStream* streamHandle);
    int32_t CloseInputStream(std::shared_ptr<VAInputStreamCallback> inputStream);
    VADevice MakeVADevice() const;

    // Capabilities
    void SetCapabilities(const OH_AudioAccessoryCapabilities& caps);

    // Noise Reduction
    void SetNoiseReductionCapability(
        const std::vector<OH_AudioNoiseReductionMode>& modes,
        OH_AudioNoiseReductionMode currentMode,
        OH_AudioAccessory_SetNoiseReductionCallback callback);
    int32_t SetNoiseReductionMode(OH_AudioNoiseReductionMode mode);
    OH_AudioNoiseReductionMode GetNoiseReductionMode() const;

    // Connection
    int32_t NotifyConnected();
    int32_t NotifyDisconnected();

    // Stream notification
    void NotifyOpenInputStream(std::shared_ptr<VAInputStreamCallback> callback);
    void NotifyStartStreamFailed();

    // Getters
    std::string GetMacAddress() const { return macAddress_; }
    bool IsConnected() const { return isConnected_; }

private:
    std::string accessoryName_;
    std::string manufacturer_;
    std::string modelNumber_;
    std::string macAddress_;
    OH_AudioAccessoryType type_;
    bool isUnidirectional_;

    OH_AudioAccessory_SetNoiseReductionCallback onNoiseReduction_;
    OH_AudioAccessory_OpenInputStreamCallback onOpenInputStream_;

    std::vector<OH_AudioNoiseReductionMode> supportedModes_;
    std::atomic<OH_AudioNoiseReductionMode> currentMode_;

    std::vector<OH_AudioStreamInfo> streamProperties_;

    std::mutex mutex_;
    bool isConnected_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_ACCESSORY_H
