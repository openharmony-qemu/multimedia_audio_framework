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

#ifndef SHARED_AUDIO_CAPTURER_WRAPPER_H
#define SHARED_AUDIO_CAPTURER_WRAPPER_H

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"

namespace OHOS {
namespace AudioStandard {

class SharedCapturerWrapper : public AudioCapturer {
public:
    explicit SharedCapturerWrapper(std::shared_ptr<AudioCapturer> sharedAudioCapturer)
        : sharedAudioCapturer_(sharedAudioCapturer)
    {
    }

    int32_t SetParams(const AudioCapturerParams params) override
    {
        return sharedAudioCapturer_->SetParams(params);
    }

    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override
    {
        return sharedAudioCapturer_->UpdatePlaybackCaptureConfig(config);
    }

    int32_t SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback) override
    {
        return sharedAudioCapturer_->SetCapturerCallback(callback);
    }

    int32_t GetParams(AudioCapturerParams &params) const override
    {
        return sharedAudioCapturer_->GetParams(params);
    }

    int32_t GetCapturerInfo(AudioCapturerInfo &capturerInfo) const override
    {
        return sharedAudioCapturer_->GetCapturerInfo(capturerInfo);
    }

    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override
    {
        return sharedAudioCapturer_->GetStreamInfo(streamInfo);
    }

    bool Start() override
    {
        return sharedAudioCapturer_->Start();
    }

    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override
    {
        return sharedAudioCapturer_->Read(buffer, userSize, isBlockingRead);
    }

    CapturerState GetStatus() const override
    {
        return sharedAudioCapturer_->GetStatus();
    }

    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override
    {
        return sharedAudioCapturer_->GetAudioTime(timestamp, base);
    }

    bool GetFirstPkgTimeStampInfo(int64_t &firstTs) const override
    {
        return sharedAudioCapturer_->GetFirstPkgTimeStampInfo(firstTs);
    }

    bool Pause() const override
    {
        return sharedAudioCapturer_->Pause();
    }

    bool Stop() const override
    {
        return sharedAudioCapturer_->Stop();
    }

    bool Flush() const override
    {
        return sharedAudioCapturer_->Flush();
    }

    bool Release() override
    {
        return sharedAudioCapturer_->Release();
    }

    int32_t GetBufferSize(size_t &bufferSize) const override
    {
        return sharedAudioCapturer_->GetBufferSize(bufferSize);
    }

    int32_t GetAudioStreamId(uint32_t &sessionID) const override
    {
        return sharedAudioCapturer_->GetAudioStreamId(sessionID);
    }

    int32_t GetFrameCount(uint32_t &frameCount) const override
    {
        return sharedAudioCapturer_->GetFrameCount(frameCount);
    }

    int32_t SetCapturerPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPositionCallback> &callback) override
    {
        return sharedAudioCapturer_->SetCapturerPositionCallback(markPosition, callback);
    }

    void UnsetCapturerPositionCallback() override
    {
        sharedAudioCapturer_->UnsetCapturerPositionCallback();
    }

    int32_t SetCapturerPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override
    {
        return sharedAudioCapturer_->SetCapturerPeriodPositionCallback(frameNumber, callback);
    }

    void UnsetCapturerPeriodPositionCallback() override
    {
        sharedAudioCapturer_->UnsetCapturerPeriodPositionCallback();
    }

    int32_t SetBufferDuration(uint64_t bufferDuration) const override
    {
        return sharedAudioCapturer_->SetBufferDuration(bufferDuration);
    }

    int32_t SetCaptureMode(AudioCaptureMode captureMode) override
    {
        return sharedAudioCapturer_->SetCaptureMode(captureMode);
    }

    AudioCaptureMode GetCaptureMode() const override
    {
        return sharedAudioCapturer_->GetCaptureMode();
    }

    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) override
    {
        return sharedAudioCapturer_->SetCapturerReadCallback(callback);
    }

    int32_t GetBufferDesc(BufferDesc &bufDesc) override
    {
        return sharedAudioCapturer_->GetBufferDesc(bufDesc);
    }

    int32_t Enqueue(const BufferDesc &bufDesc) override
    {
        return sharedAudioCapturer_->Enqueue(bufDesc);
    }

    int32_t Clear() const override
    {
        return sharedAudioCapturer_->Clear();
    }

    int32_t GetBufQueueState(BufferQueueState &bufState) const override
    {
        return sharedAudioCapturer_->GetBufQueueState(bufState);
    }

    void SetValid(bool valid) override
    {
        sharedAudioCapturer_->SetValid(valid);
    }

    int64_t GetFramesRead() const override
    {
        return sharedAudioCapturer_->GetFramesRead();
    }

    int32_t SetAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) override
    {
        return sharedAudioCapturer_->SetAudioCapturerDeviceChangeCallback(callback);
    }

    int32_t RemoveAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) override
    {
        return sharedAudioCapturer_->RemoveAudioCapturerDeviceChangeCallback(callback);
    }

    int32_t SetAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) override
    {
        return sharedAudioCapturer_->SetAudioCapturerInfoChangeCallback(callback);
    }

    int32_t RemoveAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) override
    {
        return sharedAudioCapturer_->RemoveAudioCapturerInfoChangeCallback(callback);
    }

    int32_t RegisterAudioCapturerEventListener() override
    {
        return sharedAudioCapturer_->RegisterAudioCapturerEventListener();
    }

    int32_t UnregisterAudioCapturerEventListener() override
    {
        return sharedAudioCapturer_->UnregisterAudioCapturerEventListener();
    }

    int32_t GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const override
    {
        return sharedAudioCapturer_->GetCurrentInputDevices(deviceInfo);
    }

    int32_t GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const override
    {
        return sharedAudioCapturer_->GetCurrentCapturerChangeInfo(changeInfo);
    }

    std::vector<sptr<MicrophoneDescriptor>> GetCurrentMicrophones() const override
    {
        return sharedAudioCapturer_->GetCurrentMicrophones();
    }

    uint32_t GetOverflowCount() const override
    {
        return sharedAudioCapturer_->GetOverflowCount();
    }

    int32_t SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources) override
    {
        return sharedAudioCapturer_->SetAudioSourceConcurrency(targetSources);
    }

    int32_t SetInputDevice(DeviceType deviceType) const override
    {
        return sharedAudioCapturer_->SetInputDevice(deviceType);
    }

    int32_t SetInterruptStrategy(InterruptStrategy strategy) override
    {
        CHECK_AND_RETURN_RET_LOG(sharedAudioCapturer_ != nullptr,
            ERR_MEMORY_ALLOC_FAILED, "sharedAudioCapturer_ is nullptr");
        return sharedAudioCapturer_->SetInterruptStrategy(strategy);
    }

    FastStatus GetFastStatus() override
    {
        return FASTSTATUS_NORMAL;
    }

    void SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback) override
    {
        return sharedAudioCapturer_->SetAudioCapturerErrorCallback(errorCallback);
    }

    void SetFastStatusChangeCallback(
        const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback) override
    {
        return;
    }

    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback) override
    {
        return sharedAudioCapturer_->RegisterAudioPolicyServerDiedCb(clientPid, callback);
    }

    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override
    {
        return sharedAudioCapturer_->GetAudioTimestampInfo(timestamp, base);
    }

    bool GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override
    {
        return sharedAudioCapturer_->GetTimeStampInfo(timestamp, base);
    }

    void SetInterruptEventCallbackType(InterruptEventCallbackType callbackType) override
    {
        return sharedAudioCapturer_->SetInterruptEventCallbackType(callbackType);
    }

    ~SharedCapturerWrapper() override = default;

    SharedCapturerWrapper(const SharedCapturerWrapper&) = delete;
    SharedCapturerWrapper(SharedCapturerWrapper&&) = delete;
    SharedCapturerWrapper& operator=(const SharedCapturerWrapper&) = delete;
    SharedCapturerWrapper& operator=(SharedCapturerWrapper&&) = delete;
private:
    std::shared_ptr<AudioCapturer> sharedAudioCapturer_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // SHARED_AUDIO_CAPTURER_WRAPPER_H