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

#ifndef SLE_AUDIO_DEVICE_MANAGER_H
#define SLE_AUDIO_DEVICE_MANAGER_H

#include <unordered_set>
#include "audio_general_manager.h"
#include "audio_stream_descriptor.h"
#include "istandard_sle_audio_operation_callback.h"

namespace OHOS {
namespace AudioStandard {
struct SleVolumeConfigInfo {
    AudioVolumeType volumeType = STREAM_DEFAULT;
    int32_t volumeLevel = -1;
    bool isMute = false;

    SleVolumeConfigInfo() = default;
    SleVolumeConfigInfo(AudioVolumeType volumeType, int32_t volume) : volumeType(volumeType), volumeLevel(volume) {}
};

enum SleAudioStreamType : uint32_t {
    SLE_AUDIO_STREAM_NONE = 0x00000000,
    SLE_AUDIO_STREAM_UNDEFINED = 0x00000001,
    SLE_AUDIO_STREAM_MUSIC = 0x00000002,
    SLE_AUDIO_STREAM_VOICE_CALL = 0x00000004,
    SLE_AUDIO_STREAM_VOICE_ASSISTANT = 0x00000008,
    SLE_AUDIO_STREAM_RING = 0x00000010,
    SLE_AUDIO_STREAM_VOIP = 0x00000020,
    SLE_AUDIO_STREAM_GAME = 0x00000040,
    SLE_AUDIO_STREAM_RECORD = 0x00000080,
    SLE_AUDIO_STREAM_ALERT = 0x00000100,
    SLE_AUDIO_STREAM_VIDEO = 0x00000200,
    SLE_AUDIO_STREAM_GUID = 0x00000400,
};

class SleAudioDeviceManager : public SleAudioOperationCallback {
public:
    static SleAudioDeviceManager &GetInstance()
    {
        static SleAudioDeviceManager instance;
        return instance;
    }

    int32_t SetSleAudioOperationCallback(const sptr<IStandardSleAudioOperationCallback> &callback);

    // Callback Interface Implementations
    void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override;
    void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override;
    bool IsInBandRingOpen(const std::string &device) const override;
    uint32_t GetSupportStreamType(const std::string &device) const override;
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) override;
    int32_t StartPlaying(const std::string &device, uint32_t streamType) override;
    int32_t StopPlaying(const std::string &device, uint32_t streamType) override;
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) const override;
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType) override;
    int32_t SendUserSelection(const std::string &device, uint32_t streamType) override;
    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override;

    // Parameter Conversion Interface
    int32_t SetActiveDevice(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage);
    int32_t SetActiveDevice(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType);
    int32_t StartPlaying(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage);
    int32_t StopPlaying(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage);
    int32_t StartPlaying(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType);
    int32_t StopPlaying(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType);
    int32_t SendUserSelection(const AudioDeviceDescriptor &deviceDesc, StreamUsage streamUsage);
    int32_t SendUserSelection(const AudioDeviceDescriptor &deviceDesc, SourceType sourceType);
    int32_t SetDeviceAbsVolume(const std::string &device, AudioStreamType streamType, int32_t volume);

    // Core Device Management Methods
    int32_t AddNearlinkDevice(const AudioDeviceDescriptor &deviceDesc);
    int32_t RemoveNearlinkDevice(const AudioDeviceDescriptor &deviceDesc);
    void UpdateSleStreamTypeCount(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isRemoved = false);
    void ResetSleStreamTypeCount(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> GetNearlinkStreamTypeMapByDevice(
        const std::string &deviceAddr);

    // Devcice Volume Manager
    int32_t SetNearlinkDeviceMute(const std::string &device, AudioStreamType streamType, bool isMute);
    int32_t SetNearlinkDeviceVolumeLevel(const std::string &device, AudioStreamType streamType,
        const int32_t volumeLevel);
    int32_t GetVolumeLevelByVolumeType(AudioVolumeType volumeType, const AudioDeviceDescriptor &deviceDesc);

    uint32_t GetSleStreamTypeByStreamUsage(StreamUsage streamUsage) const;
    uint32_t GetSleStreamTypeBySourceType(SourceType sourceType) const;
    std::set<StreamUsage> GetStreamUsagesBySleStreamType(uint32_t streamType) const;
    std::set<SourceType> GetSourceTypesBySleStreamType(uint32_t streamType) const;
private:
    SleAudioDeviceManager() = default;
    virtual ~SleAudioDeviceManager() = default;

    void UpdateStreamTypeMap(const std::string &deviceAddr, uint32_t streamType, uint32_t sessionId, bool isAdd);
    bool IsNearlinkDevice(DeviceType deviceType);
    bool IsMoveToNearlinkDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsNearlinkMoveToOtherDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc);

    sptr<IStandardSleAudioOperationCallback> callback_ = nullptr;

    std::mutex deviceVolumeConfigMutex_;
    std::unordered_map<std::string, std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo>> deviceVolumeConfigInfo_;

    std::mutex startedSleStreamTypeMutex_;
    // Maps device MAC -> (stream type ->set of session IDs)
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::unordered_set<uint32_t>>> startedSleStreamType_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // SLE_AUDIO_DEVICE_MANAGER_H
