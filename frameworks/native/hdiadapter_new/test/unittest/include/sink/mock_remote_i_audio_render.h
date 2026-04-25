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

#ifndef MOCK_REMOTE_I_AUDIO_RENDER_H
#define MOCK_REMOTE_I_AUDIO_RENDER_H

#include <gmock/gmock.h>
#include <v2_0/iaudio_manager.h>

namespace OHOS {
namespace AudioStandard {

typedef OHOS::HDI::DistributedAudio::Audio::V2_0::IAudioRender RemoteIAudioRender;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::AudioTimeStamp RemoteAudioTimeStamp;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::IAudioCallback RemoteIAudioCallback;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::AudioChannelMode RemoteAudioChannelMode;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::AudioDrainNotifyType RemoteAudioDrainNotifyType;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::AudioSceneDescriptor RemoteAudioSceneDescriptor;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::AudioSampleAttributes RemoteAudioSampleAttributes;
typedef OHOS::HDI::DistributedAudio::Audio::V2_0::AudioMmapBufferDescriptor RemoteAudioMmapBufferDescriptor;

class MockRemoteIAudioRender : public RemoteIAudioRender {
public:
    MockRemoteIAudioRender() = default;
    virtual ~MockRemoteIAudioRender() = default;

    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Stop, (), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));
    MOCK_METHOD(int32_t, Pause, (), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
    MOCK_METHOD(int32_t, RenderFrame, (const std::vector<int8_t> &frame, uint64_t &replyBytes), (override));
    MOCK_METHOD(int32_t, SetExtraParams, (const std::string &params), (override));
    MOCK_METHOD(int32_t, GetLatency, (uint32_t &ms), (override));
    MOCK_METHOD(int32_t, GetRenderPosition, (uint64_t &frames, RemoteAudioTimeStamp &stamp), (override));
    MOCK_METHOD(int32_t, SetRenderSpeed, (float speed), (override));
    MOCK_METHOD(int32_t, RegCallback, (const sptr<RemoteIAudioCallback> &callback, int8_t cookie), (override));
    MOCK_METHOD(int32_t, SetVolume, (float volume), (override));
    MOCK_METHOD(int32_t, GetRenderSpeed, (float &speed), (override));
    MOCK_METHOD(int32_t, SetChannelMode, (RemoteAudioChannelMode mode), (override));
    MOCK_METHOD(int32_t, GetChannelMode, (RemoteAudioChannelMode &mode), (override));
    MOCK_METHOD(int32_t, DrainBuffer, (RemoteAudioDrainNotifyType &type), (override));
    MOCK_METHOD(int32_t, IsSupportsDrain, (bool &support), (override));
    MOCK_METHOD(int32_t, CheckSceneCapability, (const RemoteAudioSceneDescriptor &scene, bool &supported), (override));
    MOCK_METHOD(int32_t, SelectScene, (const RemoteAudioSceneDescriptor &scene), (override));
    MOCK_METHOD(int32_t, SetMute, (bool mute), (override));
    MOCK_METHOD(int32_t, GetMute, (bool &mute), (override));
    MOCK_METHOD(int32_t, SetVolumeWithRamp, (float volume, uint32_t duration), (override));
    MOCK_METHOD(int32_t, GetVolume, (float &volume), (override));
    MOCK_METHOD(int32_t, GetGainThreshold, (float &min, float &max), (override));
    MOCK_METHOD(int32_t, GetGain, (float &gain), (override));
    MOCK_METHOD(int32_t, SetGain, (float gain), (override));
    MOCK_METHOD(int32_t, GetFrameSize, (uint64_t &size), (override));
    MOCK_METHOD(int32_t, GetFrameCount, (uint64_t &count), (override));
    MOCK_METHOD(int32_t, SetSampleAttributes, (const RemoteAudioSampleAttributes &attrs), (override));
    MOCK_METHOD(int32_t, GetSampleAttributes, (RemoteAudioSampleAttributes &attrs), (override));
    MOCK_METHOD(int32_t, GetCurrentChannelId, (uint32_t &channelId), (override));
    MOCK_METHOD(int32_t, GetExtraParams, (std::string &keyValueList), (override));
    MOCK_METHOD(int32_t, ReqMmapBuffer, (int32_t reqSize, RemoteAudioMmapBufferDescriptor &desc), (override));
    MOCK_METHOD(int32_t, GetMmapPosition, (uint64_t &frames, RemoteAudioTimeStamp &timestamp), (override));
    MOCK_METHOD(int32_t, AddAudioEffect, (uint64_t effectid), (override));
    MOCK_METHOD(int32_t, RemoveAudioEffect, (uint64_t effectid), (override));
    MOCK_METHOD(int32_t, GetFrameBufferSize, (uint64_t &bufferSize), (override));
    MOCK_METHOD(int32_t, TurnStandbyMode, (), (override));
    MOCK_METHOD(int32_t, AudioDevDump, (int32_t range, int32_t fd), (override));
    MOCK_METHOD(int32_t, IsSupportsPauseAndResume, (bool &supportPause, bool &supportResume), (override));
    MOCK_METHOD(int32_t, SetBufferSize, (uint32_t size), (override));
};

} // namespace AudioStandard
} // namespace OHOS

#endif // MOCK_REMOTE_I_AUDIO_RENDER_H
