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

#ifndef AUDIO_ROUTER_SELECT_STRATEGY_H
#define AUDIO_ROUTER_SELECT_STRATEGY_H

#include <memory>
#include "audio_device_descriptor.h"
#include "audio_device_simple_descriptor.h"
#include "audio_select_device_info.h"
#include "audio_source_type.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterSelectStrategy {
public:
    virtual ~AudioRouterSelectStrategy() = default;

    static AudioRouterSelectStrategy& GetInstance();

    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaInputDevice(int32_t uid, uint32_t streamId,
        SourceType sourceType) = 0;
    virtual void SetMediaInputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaOutputDevice(int32_t uid, uint32_t streamId) = 0;
    virtual void SetMediaOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallInputDevice(int32_t uid, uint32_t streamId) = 0;
    virtual void SetCallInputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallOutputDevice(int32_t uid, uint32_t streamId) = 0;
    virtual void SetCallOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller = "") = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetRecognitionInputDevice();
    virtual void SetRecognitionInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &device);

    bool GetEnhancedRoutingSupported() const;

    virtual int32_t UpdateDefaultOutputDevice(DeviceType deviceType, int32_t uid, uint32_t streamId,
        StreamUsage streamUsage);
    virtual bool IsStreamSetDefaultOutputDevice(int32_t uid, uint32_t streamId);
    virtual void UpdateMediaDefaultOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaDefaultOutputDevice(int32_t uid,
        uint32_t streamId);
    virtual void UpdateCallDefaultOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallDefaultOutputDevice(int32_t uid,
        uint32_t streamId);

    virtual void UpdateCurrentOutputDevice(int32_t uid,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices) = 0;
    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetCurrentOutputDevice(int32_t uid) = 0;
    virtual void UpdateCurrentInputDevice(int32_t uid,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices) = 0;
    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetCurrentInputDevice(int32_t uid) = 0;

    void ExcludeDevices(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
        AudioDeviceUsage usage);
    void UnexcludeDevices(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
        AudioDeviceUsage usage);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(AudioDeviceUsage audioDevUsage);
    bool IsDeviceExcluded(const std::shared_ptr<AudioDeviceDescriptor> &device,
        AudioDeviceUsage usage);
    void SetScoExcluded(bool scoExcluded);
    bool GetScoExcluded();
    bool IsPreferredDevice(const AudioDeviceDescriptor &desc);
    bool HasValidSelectDevice(AudioSelectDeviceInfo selectInfo);
    AudioDeviceDescriptor Get1stCurrentInputDevice(int32_t uid = SYSTEM_UID);
    AudioDeviceDescriptor Get1stCurrentOutputDevice(int32_t uid = SYSTEM_UID);
    bool IsCurrentInputDevice(const int32_t deviceId, int32_t uid = SYSTEM_UID);
    bool IsCurrentOutputDevice(const int32_t deviceId, int32_t uid = SYSTEM_UID);
    std::set<std::shared_ptr<AudioDeviceDescriptor>> FindCurrentOutputDevice(std::set<DeviceType> &&types);
    std::set<std::shared_ptr<AudioDeviceDescriptor>> FindCurrentInputDevice(std::set<DeviceType> &&types);

protected:
    std::shared_ptr<AudioDeviceDescriptor> ConvertSimpleToDevice(
        const std::shared_ptr<AudioDeviceSimpleDescriptor> &simpleDesc);
    std::shared_ptr<AudioDeviceSimpleDescriptor> ConvertDeviceToSimple(
        const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ConvertSimpleToDevice(
        const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> &simpleDescs);
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> ConvertDeviceToSimple(
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);

    std::shared_ptr<AudioDeviceDescriptor> JudgeFinalSelectDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &desc, SourceType sourceType,
        BluetoothAndNearlinkPreferredRecordCategory category);
    std::shared_ptr<AudioDeviceDescriptor> GetPreferDevice(
        BluetoothAndNearlinkPreferredRecordCategory category);

    std::shared_ptr<AudioDeviceDescriptor> GetFinalOutputDeviceForMedia(
        const std::shared_ptr<AudioDeviceDescriptor> &desc);
    std::shared_ptr<AudioDeviceDescriptor> GetFinalOutputDeviceForCall(
        const std::shared_ptr<AudioDeviceDescriptor> &desc);

    bool IsSameDeviceDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> desc1,
        std::shared_ptr<AudioDeviceDescriptor> desc2);

    void ExcludePairDevice(const std::shared_ptr<AudioDeviceDescriptor> &device,
        AudioDeviceUsage usage);
    void UnexcludePairDevice(const std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioDeviceUsage usage);
};

}
}
#endif
