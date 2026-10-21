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
#ifndef AUDIO_DEFINITION_POLICY_UTILS_H
#define AUDIO_DEFINITION_POLICY_UTILS_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <vector>
#include "singleton.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_stream_descriptor.h"
#include "audio_pipe_info.h"

namespace OHOS {
namespace AudioStandard {

struct SelectedDeviceFormat {
    AudioSamplingRate rate = AudioSamplingRate::SAMPLE_RATE_48000;
    AudioSampleFormat format = AudioSampleFormat::SAMPLE_S16LE;
    AudioChannelLayout channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
};

class AudioDefinitionPolicyUtils {
public:
    static uint32_t PcmFormatToBytes(AudioSampleFormat format);
    static DeviceType GetDeviceTypeFromStr(const std::string &deviceTypeStr);
    static SelectedDeviceFormat SelectBestOutputFormat(const AudioStreamInfo &appStreamInfo,
        const std::list<DeviceStreamInfo> &deviceCaps);
    static bool FindHighestFormatStreamOnDevice(const std::string &macAddress,
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams,
        const std::shared_ptr<AudioStreamDescriptor> &extra, AudioStreamInfo &highestInfo);
    static bool HasVoipStreamOnDevice(const std::string &macAddress,
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams,
        const std::shared_ptr<AudioPipeInfo> &pipeInfo);
private:
    AudioDefinitionPolicyUtils() {}
    ~AudioDefinitionPolicyUtils() {}
public:
    static std::unordered_map<std::string, DeviceRole> deviceRoleStrToEnum;
    static std::unordered_map<std::string, AudioPipeRole> pipeRoleStrToEnum;
    static std::unordered_map<std::string, DeviceType> deviceTypeStrToEnum;
    static std::unordered_map<std::string, DeviceType> deviceNameToTypeEnum;
    static std::unordered_map<std::string, AudioPin> pinStrToEnum;
    static std::unordered_map<std::string, AudioSampleFormat> formatStrToEnum;
    static std::unordered_map<AudioSampleFormat, std::string> enumToFormatStr;
    static std::unordered_map<std::string, AudioChannelLayout> layoutStrToEnum;
    static std::unordered_map<std::string, AudioFlag> flagStrToEnum;
    static std::unordered_map<std::string, uint32_t> usageStrToEnum;
    static std::unordered_map<std::string, AudioPreloadType> preloadStrToEnum;
    static std::unordered_map<std::string, bool> supModeStrToBool;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEFINITION_POLICY_UTILS_H
