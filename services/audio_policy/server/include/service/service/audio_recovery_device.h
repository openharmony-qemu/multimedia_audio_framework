
/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_RECOVERY_DEVICE_H
#define ST_AUDIO_RECOVERY_DEVICE_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_stream_collector.h"
#include "audio_device_manager.h"
#include "audio_affinity_manager.h"

#include "audio_connected_device.h"
#include "audio_router_map.h"
#include "audio_device_common.h"
#include "audio_active_device.h"
#include "audio_scene_manager.h"
#include "audio_a2dp_offload_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioRecoveryDevice {
public:
    static AudioRecoveryDevice& GetInstance()
    {
        static AudioRecoveryDevice instance;
        return instance;
    }
    void Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager);
    void RecoveryPreferredDevices();

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
private:
    AudioRecoveryDevice() :
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioAffinityManager_(AudioAffinityManager::GetAudioAffinityManager()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioRouteMap_(AudioRouteMap::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioDeviceCommon_(AudioDeviceCommon::GetInstance()) {}
    ~AudioRecoveryDevice() {}
    int32_t HandleRecoveryPreferredDevices(int32_t preferredType, int32_t deviceType,
        int32_t usageOrSourceType);

    // selectoutputdevice
    int32_t SelectOutputDeviceForFastInner(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc);
    int32_t SetRenderDeviceForUsage(StreamUsage streamUsage, std::shared_ptr<AudioDeviceDescriptor> desc);
    int32_t ConnectVirtualDevice(std::shared_ptr<AudioDeviceDescriptor> &desc);
    void WriteSelectOutputSysEvents(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &selectedDesc,
        StreamUsage strUsage);
    int32_t SelectFastOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    int32_t SelectOutputDeviceByFilterInner(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc);

    // selectinputdevice
    void SetCaptureDeviceForUsage(AudioScene scene, SourceType srcType, std::shared_ptr<AudioDeviceDescriptor> desc);
    int32_t SelectFastInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    void WriteSelectInputSysEvents(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &selectedDesc,
        SourceType srcType, AudioScene scene);
private:
    AudioStreamCollector& streamCollector_;
    AudioDeviceManager &audioDeviceManager_;
    AudioAffinityManager &audioAffinityManager_;
    AudioActiveDevice& audioActiveDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioRouteMap& audioRouteMap_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioDeviceCommon& audioDeviceCommon_;
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
};
}
}
#endif