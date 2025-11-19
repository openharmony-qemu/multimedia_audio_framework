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
#ifndef AUDIO_RENDERER_MOCK_H
#define AUDIO_RENDERER_MOCK_H
#include <gmock/gmock.h>
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class MockAudioRenderer : public AudioRenderer {
public:
    MOCK_METHOD(void, SetAudioPrivacyType, (AudioPrivacyType privacyType), (override));
    MOCK_METHOD(AudioPrivacyType, GetAudioPrivacyType, (), (override));
    MOCK_METHOD(int32_t, SetParams, (const AudioRendererParams params), (override));
    MOCK_METHOD(int32_t, SetRendererCallback, (const std::shared_ptr<AudioRendererCallback> &callback), (override));
    MOCK_METHOD(int32_t, GetParams, (AudioRendererParams &params), (const, override));
    MOCK_METHOD(int32_t, GetRendererInfo, (AudioRendererInfo &rendererInfo), (const, override));
    MOCK_METHOD(int32_t, GetStreamInfo, (AudioStreamInfo &streamInfo), (const, override));
    MOCK_METHOD(bool, Start, (StateChangeCmdType cmdType), (override));
    MOCK_METHOD(int32_t, Write, (uint8_t *buffer, size_t bufferSize), (override));
    MOCK_METHOD(int32_t, Write,
        (uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize), (override));
    MOCK_METHOD(RendererState, GetStatus, (), (const, override));
    MOCK_METHOD(bool, GetAudioTime, (Timestamp &timestamp, Timestamp::Timestampbase base), (const, override));
    MOCK_METHOD(bool, GetAudioPosition, (Timestamp &timestamp, Timestamp::Timestampbase base), (override));
    MOCK_METHOD(int32_t, GetLatency, (uint64_t &latency), (const, override));
    MOCK_METHOD(bool, Drain, (), (const, override));
    MOCK_METHOD(bool, Flush, (), (const, override));
    MOCK_METHOD(bool, PauseTransitent, (StateChangeCmdType cmdType), (override));
    MOCK_METHOD(bool, Pause, (StateChangeCmdType cmdType), (override));
    MOCK_METHOD(bool, Stop, (), (override));
    MOCK_METHOD(bool, Release, (), (override));
    MOCK_METHOD(int32_t, GetBufferSize, (size_t &bufferSize), (const, override));
    MOCK_METHOD(int32_t, GetAudioStreamId, (uint32_t &sessionID), (const, override));
    MOCK_METHOD(int32_t, GetFrameCount, (uint32_t &frameCount), (const, override));
    MOCK_METHOD(int32_t, SetAudioRendererDesc, (AudioRendererDesc audioRendererDesc), (override));
    MOCK_METHOD(int32_t, SetStreamType, (AudioStreamType audioStreamType), (override));
    MOCK_METHOD(int32_t, SetVolume, (float volume), (const, override));
    MOCK_METHOD(int32_t, SetVolumeMode, (int32_t mode), (override));
    MOCK_METHOD(float, GetVolume, (), (const, override));
    MOCK_METHOD(int32_t, SetLoudnessGain, (float loudnessGain), (const, override));
    MOCK_METHOD(float, GetLoudnessGain, (), (const, override));
    MOCK_METHOD(int32_t, SetRenderRate, (AudioRendererRate renderRate), (const, override));
    MOCK_METHOD(AudioRendererRate, GetRenderRate, (), (const, override));
    MOCK_METHOD(int32_t, SetRendererSamplingRate, (uint32_t sampleRate), (const, override));
    MOCK_METHOD(uint32_t, GetRendererSamplingRate, (), (const, override));
    MOCK_METHOD(int32_t, SetRendererPositionCallback,
        (int64_t markPosition, const std::shared_ptr<RendererPositionCallback> &callback), (override));
    MOCK_METHOD(void, UnsetRendererPositionCallback, (), (override));
    MOCK_METHOD(int32_t, SetRendererPeriodPositionCallback,
        (int64_t frameNumber, const std::shared_ptr<RendererPeriodPositionCallback> &callback), (override));
    MOCK_METHOD(void, SetFastStatusChangeCallback,
        (const std::shared_ptr<AudioRendererFastStatusChangeCallback> &callback), (override));
    MOCK_METHOD(void, UnsetRendererPeriodPositionCallback, (), (override));
    MOCK_METHOD(int32_t, SetBufferDuration, (uint64_t bufferDuration), (const, override));
    MOCK_METHOD(int32_t, SetRenderMode, (AudioRenderMode renderMode), (override));
    MOCK_METHOD(AudioRenderMode, GetRenderMode, (), (const, override));
    MOCK_METHOD(int32_t, SetRendererWriteCallback,
        (const std::shared_ptr<AudioRendererWriteCallback> &callback), (override));
    MOCK_METHOD(int32_t, SetRendererFirstFrameWritingCallback,
        (const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback), (override));
    MOCK_METHOD(int32_t, GetBufferDesc, (BufferDesc &bufDesc), (override));
    MOCK_METHOD(int32_t, Enqueue, (const BufferDesc &bufDesc), (override));
    MOCK_METHOD(int32_t, Clear, (), (const, override));
    MOCK_METHOD(int32_t, GetBufQueueState, (BufferQueueState &bufState), (const, override));
    MOCK_METHOD(void, SetInterruptMode, (InterruptMode mode), (override));
    MOCK_METHOD(int32_t, SetParallelPlayFlag, (bool parallelPlayFlag), (override));
    MOCK_METHOD(int32_t, SetLowPowerVolume, (float volume), (const, override));
    MOCK_METHOD(float, GetLowPowerVolume, (), (const, override));
    MOCK_METHOD(int32_t, SetOffloadAllowed, (bool isAllowed), (override));
    MOCK_METHOD(int32_t, SetOffloadMode, (int32_t state, bool isAppBack), (const, override));
    MOCK_METHOD(int32_t, UnsetOffloadMode, (), (const, override));
    MOCK_METHOD(float, GetSingleStreamVolume, (), (const, override));
    MOCK_METHOD(float, GetMinStreamVolume, (), (const, override));
    MOCK_METHOD(float, GetMaxStreamVolume, (), (const, override));
    MOCK_METHOD(uint32_t, GetUnderflowCount, (), (const, override));
    MOCK_METHOD(int32_t, GetCurrentOutputDevices, (AudioDeviceDescriptor &deviceInfo), (const, override));
    MOCK_METHOD(AudioEffectMode, GetAudioEffectMode, (), (const, override));
    MOCK_METHOD(int64_t, GetFramesWritten, (), (const, override));
    MOCK_METHOD(int32_t, SetAudioEffectMode, (AudioEffectMode effectMode), (const, override));
    MOCK_METHOD(void, SetAudioRendererErrorCallback,
        (std::shared_ptr<AudioRendererErrorCallback> errorCallback), (override));
    MOCK_METHOD(int32_t, RegisterOutputDeviceChangeWithInfoCallback,
        (const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterOutputDeviceChangeWithInfoCallback, (), (override));
    MOCK_METHOD(int32_t, UnregisterOutputDeviceChangeWithInfoCallback,
        (const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, RegisterAudioPolicyServerDiedCb,
        (const int32_t clientPid, const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterAudioPolicyServerDiedCb, (const int32_t clientPid), (override));
    MOCK_METHOD(int32_t, SetChannelBlendMode, (ChannelBlendMode blendMode), (override));
    MOCK_METHOD(int32_t, SetVolumeWithRamp, (float volume, int32_t duration), (override));
    MOCK_METHOD(void, SetPreferredFrameSize, (int32_t frameSize), (override));
    MOCK_METHOD(int32_t, SetSpeed, (float speed), (override));
    MOCK_METHOD(float, GetSpeed, (), (override));
    MOCK_METHOD(bool, IsOffloadEnable, (), (override));
    MOCK_METHOD(bool, IsFastRenderer, (), (override));
    MOCK_METHOD(void, SetSilentModeAndMixWithOthers, (bool on), (override));
    MOCK_METHOD(bool, GetSilentModeAndMixWithOthers, (), (override));
    MOCK_METHOD(void, EnableVoiceModemCommunicationStartStream, (bool enable), (override));
    MOCK_METHOD(bool, IsNoStreamRenderer, (), (const, override));
    MOCK_METHOD(int64_t, GetSourceDuration, (), (const, override));
    MOCK_METHOD(void, SetSourceDuration, (int64_t duration), (override));
    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (DeviceType deviceType), (override));
    MOCK_METHOD(FastStatus, GetFastStatus, (), (override));
    MOCK_METHOD(bool, Mute, (StateChangeCmdType cmdType), (const, override));
    MOCK_METHOD(bool, Unmute, (StateChangeCmdType cmdType), (const, override));
    MOCK_METHOD(int32_t, GetAudioTimestampInfo,
        (Timestamp &timestamp, Timestamp::Timestampbase base), (const, override));
    MOCK_METHOD(int32_t, StartDataCallback, (), (override));
    MOCK_METHOD(int32_t, StopDataCallback, (), (override));
    MOCK_METHOD(void, SetInterruptEventCallbackType, (InterruptEventCallbackType callbackType), (override));
    MOCK_METHOD(void, SetAudioHapticsSyncId, (int32_t audioHapticsSyncId), (override));
    MOCK_METHOD(void, ResetFirstFrameState, (), (override));
    MOCK_METHOD(int32_t, SetTarget, (RenderTarget target), (override));
    MOCK_METHOD(RenderTarget, GetTarget, (), (const, override));
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RENDERER_MOCK_H