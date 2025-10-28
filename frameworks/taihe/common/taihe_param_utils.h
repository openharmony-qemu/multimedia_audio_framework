/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_PARAM_UTILS_H
#define TAIHE_PARAM_UTILS_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
#include <securec.h>
#else
#include "ability.h"
#endif
#include "audio_stream_manager.h"
#include "audio_capturer_options.h"
#include "audio_session_device_info.h"
#include "timestamp.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class TaiheParamUtils {
public:
    static void ConvertDeviceInfoToAudioDeviceDescriptor(
        std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor,
        const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo);
    static int32_t GetRendererInfo(OHOS::AudioStandard::AudioRendererInfo &rendererInfo, AudioRendererInfo const &in);
    static int32_t GetStreamInfo(OHOS::AudioStandard::AudioStreamInfo &audioStreamInfo,
        AudioCapturerOptions const &options);
    static int32_t GetStreamInfo(OHOS::AudioStandard::AudioStreamInfo &audioStreamInfo,
        AudioRendererOptions const &options);
    static int32_t GetCapturerInfo(OHOS::AudioStandard::AudioCapturerInfo &audioCapturerInfo,
        AudioCapturerInfo const &in);
    static int32_t GetCapturerOptions(OHOS::AudioStandard::AudioCapturerOptions *opts,
        AudioCapturerOptions const &options);
    static int32_t GetRendererOptions(OHOS::AudioStandard::AudioRendererOptions *opts,
        AudioRendererOptions const &options);
    static int32_t GetSpatialDeviceState(OHOS::AudioStandard::AudioSpatialDeviceState *spatialDeviceState,
        AudioSpatialDeviceState in);
    static int32_t GetAudioDeviceDescriptor(
        std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> &selectedAudioDevice,
        bool &argTransFlag, AudioDeviceDescriptor in);
    static int32_t GetAudioDeviceDescriptorVector(
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &deviceDescriptorsVector,
        bool &argTransFlag, array_view<AudioDeviceDescriptor> in);
    static int32_t GetAudioCapturerInfo(OHOS::AudioStandard::AudioCapturerInfo &capturerInfo,
        AudioCapturerInfo const &in);
    static int32_t GetAudioCapturerFilter(OHOS::sptr<OHOS::AudioStandard::AudioCapturerFilter> &audioCapturerFilter,
        AudioCapturerFilter const &in);
    static int32_t GetAudioRendererFilter(OHOS::sptr<OHOS::AudioStandard::AudioRendererFilter> &audioRendererFilter,
        bool &argTransFlag, AudioRendererFilter const &in);
    static int32_t GetAudioSessionStrategy(OHOS::AudioStandard::AudioSessionStrategy &audioSessionStrategy,
        AudioSessionStrategy const &in);
    static int32_t UniqueEffectPropertyData(OHOS::AudioStandard::AudioEffectPropertyArrayV3 &propertyArray);
    static int32_t GetEffectPropertyArray(OHOS::AudioStandard::AudioEffectPropertyArrayV3 &propertyArray,
        array_view<AudioEffectProperty> in);
    static int32_t GetExtraParametersSubKV(std::vector<std::pair<std::string, std::string>> &subKV,
        map_view<string, string> in);

    static MicStateChangeEvent SetValueMicStateChange(
        const OHOS::AudioStandard::MicStateChangeEvent &micStateChangeEvent);
    static VolumeEvent SetValueVolumeEvent(const OHOS::AudioStandard::VolumeEvent &volumeEvent);
    static StreamVolumeEvent SetValueStreamVolumeEvent(const OHOS::AudioStandard::StreamVolumeEvent &volumeEvent);
    static AudioCapturerChangeInfo SetAudioCapturerChangeInfoDescriptors(
        const OHOS::AudioStandard::AudioCapturerChangeInfo &changeInfo);
    static AudioDeviceDescriptor SetDeviceDescriptor(const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo);
    static DeviceChangeAction SetValueDeviceChangeAction(const OHOS::AudioStandard::DeviceChangeAction &action);
    static AudioSessionStateChangedEvent SetValueAudioSessionStateChangedEvent(
        const OHOS::AudioStandard::AudioSessionStateChangedEvent &event);
    static CurrentOutputDeviceChangedEvent SetValueCurrentOutputDeviceChangedEvent(
        const OHOS::AudioStandard::CurrentOutputDeviceChangedEvent &event);
    static taihe::array<AudioDeviceDescriptor> SetDeviceDescriptors(
        const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &deviceDescriptors);
    static taihe::array<AudioDeviceDescriptor> SetValueDeviceInfo(
        const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo);
    static taihe::array<AudioRendererChangeInfo> SetRendererChangeInfos(
        const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> &changeInfos);
    static taihe::array<AudioCapturerChangeInfo> SetCapturerChangeInfos(
        const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> &changeInfos);
    static taihe::array<AudioEffectMode> SetEffectInfo(
        const OHOS::AudioStandard::AudioSceneEffectInfo &audioSceneEffectInfo);
    static DeviceBlockStatusInfo SetValueBlockedDeviceAction(
        const OHOS::AudioStandard::MicrophoneBlockedInfo &microphoneBlockedInfo);
    static taihe::array<VolumeGroupInfo> SetVolumeGroupInfos(
        const std::vector<OHOS::sptr<OHOS::AudioStandard::VolumeGroupInfo>> &volumeGroupInfos);
    static taihe::array<StreamUsage> SetValueStreamUsageArray(
        const std::vector<OHOS::AudioStandard::StreamUsage> &streamUsageArray);
    static taihe::array<AudioVolumeType> SetValueAudioVolumeTypeArray(
        const std::vector<OHOS::AudioStandard::AudioVolumeType> &volumeTypeArray);
    static string ToTaiheString(const std::string &src);
    static AudioRendererInfo ToTaiheRendererInfo(const OHOS::AudioStandard::AudioRendererInfo &rendererInfo);
    static AudioCapturerInfo ToTaiheCapturerInfo(const OHOS::AudioStandard::AudioCapturerInfo &capturerInfo);
    static AudioSamplingRate ToTaiheAudioSamplingRate(OHOS::AudioStandard::AudioSamplingRate audioSamplingRate);
    static AudioChannel ToTaiheAudioChannel(OHOS::AudioStandard::AudioChannel audioChannel);
    static AudioSampleFormat ToTaiheAudioSampleFormat(OHOS::AudioStandard::AudioSampleFormat audioSampleFormat);
    static AudioStreamInfo ToTaiheAudioStreamInfo(std::shared_ptr<OHOS::AudioStandard::AudioStreamInfo> &src);
    static AudioTimestampInfo ToTaiheAudioTimestampInfo(OHOS::AudioStandard::Timestamp &src);
    static AudioRendererChangeInfo ToTaiheAudioRendererChangeInfo(
        const std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo> &src);
    static AudioCapturerChangeInfo ToTaiheAudioCapturerChangeInfo(
        const std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo> &src);
    static VolumeGroupInfo ToTaiheVolumeGroupInfo(const OHOS::sptr<OHOS::AudioStandard::VolumeGroupInfo> &src);
    static taihe::array<AudioEffectProperty> ToTaiheEffectPropertyArray(
        const OHOS::AudioStandard::AudioEffectPropertyArrayV3 &propertyArray);
    static taihe::array<taihe::string> ToTaiheArrayString(const std::vector<std::string> &src);
    static taihe::array<uint8_t> ToTaiheArrayBuffer(uint8_t *src, size_t srcLen);
    static AudioSpatialEnabledStateForDevice ToTaiheAudioSpatialEnabledStateForDevice(
        const OHOS::AudioStandard::AudioSpatialEnabledStateForDevice &audioSpatialEnabledStateForDevice);
    static AudioSessionDeactivatedEvent ToTaiheSessionDeactivatedEvent(
        const OHOS::AudioStandard::AudioSessionDeactiveEvent &audioSessionDeactiveEvent);
    static bool IsSameRef(std::shared_ptr<uintptr_t> src, std::shared_ptr<uintptr_t> dst);

    template<typename E>
    static std::shared_ptr<uintptr_t> TypeCallback(callback_view<void(E)> callback)
    {
        std::shared_ptr<taihe::callback<void(E)>> taiheCallback =
            std::make_shared<taihe::callback<void(E)>>(callback);
        return std::reinterpret_pointer_cast<uintptr_t>(taiheCallback);
    }

    template<typename E>
    static std::shared_ptr<uintptr_t> TypeCallback(callback_view<AudioDataCallbackResult(E)> callback)
    {
        std::shared_ptr<taihe::callback<AudioDataCallbackResult(E)>> taiheCallback =
            std::make_shared<taihe::callback<AudioDataCallbackResult(E)>>(callback);
        return std::reinterpret_pointer_cast<uintptr_t>(taiheCallback);
    }
};
} // namespace ANI::Audio

#endif // TAIHE_PARAM_UTILS_H
