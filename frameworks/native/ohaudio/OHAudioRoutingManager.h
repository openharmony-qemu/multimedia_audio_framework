/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OH_AUDIO_ROUTING_MANAGER_H
#define OH_AUDIO_ROUTING_MANAGER_H

#include "audio_info.h"
#include "audio_common_log.h"
#include "native_audio_routing_manager.h"
#include "native_audio_common.h"
#include "native_audio_device_base.h"
#include "audio_system_manager.h"
#include "OHAudioDeviceDescriptor.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioDeviceChangedCallback : public AudioManagerDeviceChangeCallback {
public:
    explicit OHAudioDeviceChangedCallback(OH_AudioRoutingManager_OnDeviceChangedCallback callback)
        : callback_(callback)
    {
    }
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;

    OH_AudioRoutingManager_OnDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioDeviceChangedCallback()
    {
        AUDIO_INFO_LOG("~OHAudioDeviceChangedCallback called.");
        if (callback_ != nullptr) {
            callback_ = nullptr;
        }
    }

private:
    OH_AudioRoutingManager_OnDeviceChangedCallback callback_;
};

class OHAudioRoutingManager {
public:
    ~OHAudioRoutingManager();

    static OHAudioRoutingManager* GetInstance()
    {
        if (!ohAudioRoutingManager_) {
            ohAudioRoutingManager_ = new OHAudioRoutingManager();
        }
        return ohAudioRoutingManager_;
    }
    OH_AudioDeviceDescriptorArray* GetDevices(DeviceFlag deviceFlag);

    OH_AudioDeviceDescriptorArray *ConvertDesc(std::vector<sptr<AudioDeviceDescriptor>> &desc);
    OH_AudioDeviceDescriptorArray *GetAvailableDevices(AudioDeviceUsage deviceUsage);
    OH_AudioDeviceDescriptorArray *GetPreferredOutputDevice(StreamUsage streamUsage);
    OH_AudioDeviceDescriptorArray *GetPreferredInputDevice(SourceType sourceType);

    OH_AudioCommon_Result SetDeviceChangeCallback(const DeviceFlag flag,
        OH_AudioRoutingManager_OnDeviceChangedCallback callback);
    OH_AudioCommon_Result UnsetDeviceChangeCallback(DeviceFlag flag,
        OH_AudioRoutingManager_OnDeviceChangedCallback ohOnDeviceChangedcallback);

private:
    OHAudioRoutingManager();
    static OHAudioRoutingManager *ohAudioRoutingManager_;
    AudioSystemManager *audioSystemManager_ = AudioSystemManager::GetInstance();
    std::vector<std::shared_ptr<OHAudioDeviceChangedCallback>> ohAudioOnDeviceChangedCallbackArray_;
};
OHAudioRoutingManager* OHAudioRoutingManager::ohAudioRoutingManager_ = nullptr;

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_ROUTING_MANAGER_H