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

#ifndef SHARED_AUDIO_RENDERER_WRAPPER_H
#define SHARED_AUDIO_RENDERER_WRAPPER_H

#include <vector>
#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <cstring>
#include <timestamp.h>
#include <mutex>
#include "audio_effect.h"
#include "audio_stream_change_info.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class SharedAudioRendererWrapper : public AudioRenderer {
public:
    void SetAudioPrivacyType(AudioPrivacyType privacyType) override
    {
        sharedAudioRenderer_->SetAudioPrivacyType(privacyType);
    }

    AudioPrivacyType GetAudioPrivacyType() override
    {
        return sharedAudioRenderer_->GetAudioPrivacyType();
    }

    int32_t SetParams(const AudioRendererParams params) override
    {
        return sharedAudioRenderer_->SetParams(params);
    }

    int32_t SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback) override
    {
        return sharedAudioRenderer_->SetRendererCallback(callback);
    }

    int32_t GetParams(AudioRendererParams &params) const override
    {
        return sharedAudioRenderer_->GetParams(params);
    }

    int32_t GetRendererInfo(AudioRendererInfo &rendererInfo) const override
    {
        return sharedAudioRenderer_->GetRendererInfo(rendererInfo);
    }

    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override
    {
        return sharedAudioRenderer_->GetStreamInfo(streamInfo);
    }

    bool Start(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override
    {
        return sharedAudioRenderer_->Start(cmdType);
    }

    int32_t Write(uint8_t *buffer, size_t bufferSize) override
    {
        return sharedAudioRenderer_->Write(buffer, bufferSize);
    }

    int32_t Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize) override
    {
        return sharedAudioRenderer_->Write(pcmBuffer, pcmBufferSize, metaBuffer, metaBufferSize);
    }

    RendererState GetStatus() const override
    {
        return sharedAudioRenderer_->GetStatus();
    }

    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override
    {
        return sharedAudioRenderer_->GetAudioTime(timestamp, base);
    }

    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override
    {
        return sharedAudioRenderer_->GetAudioPosition(timestamp, base);
    }

    int32_t GetLatency(uint64_t &latency) const override
    {
        return sharedAudioRenderer_->GetLatency(latency);
    }

    bool Drain() const override
    {
        return sharedAudioRenderer_->Drain();
    }

    bool Flush() const override
    {
        return sharedAudioRenderer_->Flush();
    }

    bool PauseTransitent(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override
    {
        return sharedAudioRenderer_->PauseTransitent(cmdType);
    }

    bool Pause(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override
    {
        return sharedAudioRenderer_->Pause(cmdType);
    }

    bool Stop() override
    {
        return sharedAudioRenderer_->Stop();
    }

    bool Release() override
    {
        return sharedAudioRenderer_->Release();
    }

    int32_t GetBufferSize(size_t &bufferSize) const override
    {
        return sharedAudioRenderer_->GetBufferSize(bufferSize);
    }

    int32_t GetAudioStreamId(uint32_t &sessionID) const override
    {
        return sharedAudioRenderer_->GetAudioStreamId(sessionID);
    }

    int32_t GetFrameCount(uint32_t &frameCount) const override
    {
        return sharedAudioRenderer_->GetFrameCount(frameCount);
    }

    int32_t SetAudioRendererDesc(AudioRendererDesc audioRendererDesc) override
    {
        return sharedAudioRenderer_->SetAudioRendererDesc(audioRendererDesc);
    }

    int32_t SetStreamType(AudioStreamType audioStreamType) override
    {
        return sharedAudioRenderer_->SetStreamType(audioStreamType);
    }

    int32_t SetVolume(float volume) const override
    {
        return sharedAudioRenderer_->SetVolume(volume);
    }

    float GetVolume() const override
    {
        return sharedAudioRenderer_->GetVolume();
    }

    int32_t SetRenderRate(AudioRendererRate renderRate) const override
    {
        return sharedAudioRenderer_->SetRenderRate(renderRate);
    }

    AudioRendererRate GetRenderRate() const override
    {
        return sharedAudioRenderer_->GetRenderRate();
    }

    int32_t SetRendererSamplingRate(uint32_t sampleRate) const override
    {
        return sharedAudioRenderer_->SetRendererSamplingRate(sampleRate);
    }

    uint32_t GetRendererSamplingRate() const override
    {
        return sharedAudioRenderer_->GetRendererSamplingRate();
    }

    int32_t SetRendererPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPositionCallback> &callback) override
    {
        return sharedAudioRenderer_->SetRendererPositionCallback(markPosition, callback);
    }

    void UnsetRendererPositionCallback() override
    {
        sharedAudioRenderer_->UnsetRendererPositionCallback();
    }

    int32_t SetRendererPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override
    {
        return sharedAudioRenderer_->SetRendererPeriodPositionCallback(frameNumber, callback);
    }

    void UnsetRendererPeriodPositionCallback() override
    {
        sharedAudioRenderer_->UnsetRendererPeriodPositionCallback();
    }

    int32_t SetBufferDuration(uint64_t bufferDuration) const override
    {
        return sharedAudioRenderer_->SetBufferDuration(bufferDuration);
    }

    int32_t SetRenderMode(AudioRenderMode renderMode) override
    {
        return sharedAudioRenderer_->SetRenderMode(renderMode);
    }

    AudioRenderMode GetRenderMode() const override
    {
        return sharedAudioRenderer_->GetRenderMode();
    }

    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override
    {
        return sharedAudioRenderer_->SetRendererWriteCallback(callback);
    }

    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override
    {
        return sharedAudioRenderer_->SetRendererFirstFrameWritingCallback(callback);
    }

    int32_t GetBufferDesc(BufferDesc &bufDesc) override
    {
        return sharedAudioRenderer_->GetBufferDesc(bufDesc);
    }

    int32_t Enqueue(const BufferDesc &bufDesc) override
    {
        return sharedAudioRenderer_->Enqueue(bufDesc);
    }

    int32_t Clear() const override
    {
        return sharedAudioRenderer_->Clear();
    }

    int32_t GetBufQueueState(BufferQueueState &bufState) const override
    {
        return sharedAudioRenderer_->GetBufQueueState(bufState);
    }

    void SetInterruptMode(InterruptMode mode) override
    {
        sharedAudioRenderer_->SetInterruptMode(mode);
    }

    int32_t SetParallelPlayFlag(bool parallelPlayFlag) override
    {
        return sharedAudioRenderer_->SetParallelPlayFlag(parallelPlayFlag);
    }

    int32_t SetLowPowerVolume(float volume) const override
    {
        return sharedAudioRenderer_->SetLowPowerVolume(volume);
    }

    float GetLowPowerVolume() const override
    {
        return sharedAudioRenderer_->GetLowPowerVolume();
    }

    int32_t SetOffloadAllowed(bool isAllowed) override
    {
        return sharedAudioRenderer_->SetOffloadAllowed(isAllowed);
    }

    int32_t SetOffloadMode(int32_t state, bool isAppBack) const override
    {
        return sharedAudioRenderer_->SetOffloadMode(state, isAppBack);
    }

    int32_t UnsetOffloadMode() const override
    {
        return sharedAudioRenderer_->UnsetOffloadMode();
    }

    float GetSingleStreamVolume() const override
    {
        return sharedAudioRenderer_->GetSingleStreamVolume();
    }

    float GetMinStreamVolume() const override
    {
        return sharedAudioRenderer_->GetMinStreamVolume();
    }

    float GetMaxStreamVolume() const override
    {
        return sharedAudioRenderer_->GetMaxStreamVolume();
    }

    uint32_t GetUnderflowCount() const override
    {
        return sharedAudioRenderer_->GetUnderflowCount();
    }

    int32_t GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const override
    {
        return sharedAudioRenderer_->GetCurrentOutputDevices(deviceInfo);
    }

    AudioEffectMode GetAudioEffectMode() const override
    {
        return sharedAudioRenderer_->GetAudioEffectMode();
    }

    int64_t GetFramesWritten() const override
    {
        return sharedAudioRenderer_->GetFramesWritten();
    }

    int32_t SetAudioEffectMode(AudioEffectMode effectMode) const override
    {
        return sharedAudioRenderer_->SetAudioEffectMode(effectMode);
    }

    void SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback) override
    {
        sharedAudioRenderer_->SetAudioRendererErrorCallback(errorCallback);
    }

    int32_t RegisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override
    {
        return sharedAudioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(callback);
    }

    int32_t UnregisterOutputDeviceChangeWithInfoCallback() override
    {
        return sharedAudioRenderer_->UnregisterOutputDeviceChangeWithInfoCallback();
    }

    int32_t UnregisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override
    {
        return sharedAudioRenderer_->UnregisterOutputDeviceChangeWithInfoCallback(callback);
    }

    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback) override
    {
        return sharedAudioRenderer_->RegisterAudioPolicyServerDiedCb(clientPid, callback);
    }

    int32_t UnregisterAudioPolicyServerDiedCb(const int32_t clientPid) override
    {
        return sharedAudioRenderer_->UnregisterAudioPolicyServerDiedCb(clientPid);
    }

    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override
    {
        return sharedAudioRenderer_->SetChannelBlendMode(blendMode);
    }

    int32_t SetVolumeWithRamp(float volume, int32_t duration) override
    {
        return sharedAudioRenderer_->SetVolumeWithRamp(volume, duration);
    }

    void SetPreferredFrameSize(int32_t frameSize) override
    {
        sharedAudioRenderer_->SetPreferredFrameSize(frameSize);
    }

    int32_t SetSpeed(float speed) override
    {
        return sharedAudioRenderer_->SetSpeed(speed);
    }

    float GetSpeed() override
    {
        return sharedAudioRenderer_->GetSpeed();
    }

    bool IsOffloadEnable() override
    {
        return sharedAudioRenderer_->IsOffloadEnable();
    }

    bool IsFastRenderer() override
    {
        return sharedAudioRenderer_->IsFastRenderer();
    }

    void SetSilentModeAndMixWithOthers(bool on) override
    {
        sharedAudioRenderer_->SetSilentModeAndMixWithOthers(on);
    }

    bool GetSilentModeAndMixWithOthers() override
    {
        return sharedAudioRenderer_->GetSilentModeAndMixWithOthers();
    }

    void EnableVoiceModemCommunicationStartStream(bool enable) override
    {
        sharedAudioRenderer_->EnableVoiceModemCommunicationStartStream(enable);
    }

    bool IsNoStreamRenderer() const override
    {
        return sharedAudioRenderer_->IsNoStreamRenderer();
    }

    int32_t SetDefaultOutputDevice(DeviceType deviceType) override
    {
        return sharedAudioRenderer_->SetDefaultOutputDevice(deviceType);
    }

    bool Mute(StateChangeCmdType cmdType = CMD_FROM_CLIENT) const override
    {
        return sharedAudioRenderer_->Mute(cmdType);
    }

    bool Unmute(StateChangeCmdType cmdType = CMD_FROM_CLIENT) const override
    {
        return sharedAudioRenderer_->Unmute(cmdType);
    }

    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override
    {
        return sharedAudioRenderer_->GetAudioTimestampInfo(timestamp, base);
    }

    explicit SharedAudioRendererWrapper(std::shared_ptr<AudioRenderer> renderer) : sharedAudioRenderer_(renderer)
   
    {
    }

    ~SharedAudioRendererWrapper() override = default;

    SharedAudioRendererWrapper(const SharedAudioRendererWrapper &) = delete;
    SharedAudioRendererWrapper &operator=(const SharedAudioRendererWrapper &) = delete;
    SharedAudioRendererWrapper(SharedAudioRendererWrapper &&) = delete;
    SharedAudioRendererWrapper &operator=(SharedAudioRendererWrapper &&) = delete;
private:
    std::shared_ptr<AudioRenderer> sharedAudioRenderer_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // SHARED_AUDIO_RENDERER_WRAPPER_H