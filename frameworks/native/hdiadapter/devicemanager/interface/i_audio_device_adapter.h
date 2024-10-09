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

#ifndef I_AUDIO_DEVICE_ADAPTER_H
#define I_AUDIO_DEVICE_ADAPTER_H

#include <v1_0/iaudio_manager.h>
#include <v1_0/iaudio_callback.h>
#include <v1_0/audio_types.h>

using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioManager;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRoute;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey;

namespace OHOS {
namespace AudioStandard {
class IAudioDeviceAdapterCallback {
public:
    IAudioDeviceAdapterCallback() = default;
    virtual ~IAudioDeviceAdapterCallback() = default;

    virtual void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition, const std::string &value) = 0;
};

class IAudioDeviceAdapter {
public:
    IAudioDeviceAdapter() = default;
    virtual ~IAudioDeviceAdapter() = default;

    virtual int32_t Init() = 0;
    virtual int32_t RegExtraParamObserver() = 0;
    virtual int32_t CreateRender(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioRender> &audioRender, IAudioDeviceAdapterCallback *renderCb, uint32_t &renderId) = 0;
    virtual void DestroyRender(sptr<IAudioRender> audioRender, uint32_t &renderId) = 0;
    virtual int32_t CreateCapture(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioCapture> &audioCapture, IAudioDeviceAdapterCallback *captureCb, uint32_t &captureId) = 0;
    virtual void DestroyCapture(sptr<IAudioCapture> audioCapture, uint32_t &captureId) = 0;
    virtual void SetAudioParameter(const AudioParamKey key, const std::string &condition,
        const std::string &value) = 0;
    virtual std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) = 0;
    virtual int32_t UpdateAudioRoute(const AudioRoute &route) = 0;
    virtual int32_t Release() = 0;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // I_AUDIO_DEVICE_ADAPTER_H