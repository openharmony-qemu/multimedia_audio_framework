/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef I_POLICY_PROVIDER_H
#define I_POLICY_PROVIDER_H

#include <memory>
#include <vector>

#include "audio_info.h"
#include "audio_shared_memory.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const std::vector<std::pair<AudioVolumeType, DeviceGroup>> g_volumeIndexVector = {
        {STREAM_VOICE_CALL, DEVICE_GROUP_BUILT_IN},
        {STREAM_VOICE_CALL, DEVICE_GROUP_WIRELESS},
        {STREAM_VOICE_CALL, DEVICE_GROUP_WIRED},
        {STREAM_VOICE_CALL, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_RING, DEVICE_GROUP_BUILT_IN},
        {STREAM_RING, DEVICE_GROUP_WIRELESS},
        {STREAM_RING, DEVICE_GROUP_WIRED},
        {STREAM_RING, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_MUSIC, DEVICE_GROUP_BUILT_IN},
        {STREAM_MUSIC, DEVICE_GROUP_WIRELESS},
        {STREAM_MUSIC, DEVICE_GROUP_WIRED},
        {STREAM_MUSIC, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_VOICE_ASSISTANT, DEVICE_GROUP_BUILT_IN},
        {STREAM_VOICE_ASSISTANT, DEVICE_GROUP_WIRELESS},
        {STREAM_VOICE_ASSISTANT, DEVICE_GROUP_WIRED},
        {STREAM_VOICE_ASSISTANT, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_ALARM, DEVICE_GROUP_BUILT_IN},
        {STREAM_ALARM, DEVICE_GROUP_WIRELESS},
        {STREAM_ALARM, DEVICE_GROUP_WIRED},
        {STREAM_ALARM, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_ACCESSIBILITY, DEVICE_GROUP_BUILT_IN},
        {STREAM_ACCESSIBILITY, DEVICE_GROUP_WIRELESS},
        {STREAM_ACCESSIBILITY, DEVICE_GROUP_WIRED},
        {STREAM_ACCESSIBILITY, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_ULTRASONIC, DEVICE_GROUP_BUILT_IN},
        {STREAM_ULTRASONIC, DEVICE_GROUP_WIRELESS},
        {STREAM_ULTRASONIC, DEVICE_GROUP_WIRED},
        {STREAM_ULTRASONIC, DEVICE_GROUP_REMOTE_CAST},
        {STREAM_ALL, DEVICE_GROUP_BUILT_IN},
        {STREAM_ALL, DEVICE_GROUP_WIRELESS},
        {STREAM_ALL, DEVICE_GROUP_WIRED},
        {STREAM_ALL, DEVICE_GROUP_REMOTE_CAST},
    };
}
class IPolicyProvider {
public:
    virtual int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, DeviceInfo &deviceInfo) = 0;

    virtual int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer) = 0;

    virtual int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config) = 0;

    virtual int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) = 0;

    virtual int32_t NotifyWakeUpCapturerRemoved() = 0;

    virtual bool IsAbsVolumeSupported() = 0;

    virtual int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp) = 0;

    virtual int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName) = 0;

    virtual ~IPolicyProvider() = default;

    static bool GetVolumeIndex(AudioVolumeType streamType, DeviceGroup deviceGroup, size_t &index)
    {
        bool isFind = false;
        for (size_t tempIndex = 0; tempIndex < g_volumeIndexVector.size(); tempIndex++) {
            if (g_volumeIndexVector[tempIndex].first == streamType &&
                g_volumeIndexVector[tempIndex].second == deviceGroup) {
                isFind = true;
                index = tempIndex;
                break;
            }
        }
        return isFind;
    };
    static size_t GetVolumeVectorSize()
    {
        return g_volumeIndexVector.size();
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_POLICY_PROVIDER_H
