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
#ifndef AUDIO_CAPTURER_MOCK_H
#define AUDIO_CAPTURER_MOCK_H
#include <gmock/gmock.h>
#include "audio_capturer.h"

namespace OHOS {
namespace AudioStandard {
class MockAudioCapturer : public AudioCapturer {
public:
    MOCK_METHOD(int32_t, SetParams, (const AudioCapturerParams params), (override));
    MOCK_METHOD(int32_t, UpdatePlaybackCaptureConfig, (const AudioPlaybackCaptureConfig &config), (override));
    MOCK_METHOD(int32_t, SetCapturerCallback, (const std::shared_ptr<AudioCapturerCallback> &callback), (override));
    MOCK_METHOD(void, SetAudioCapturerErrorCallback,
        (std::shared_ptr<AudioCapturerErrorCallback> errorCallback), (override));
    MOCK_METHOD(void, SetFastStatusChangeCallback,
        (const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, GetParams, (AudioCapturerParams &params), (const, override));
    MOCK_METHOD(int32_t, GetCapturerInfo, (AudioCapturerInfo &capturerInfo), (const, override));
    MOCK_METHOD(int32_t, GetStreamInfo, (AudioStreamInfo &streamInfo), (const, override));
    MOCK_METHOD(bool, Start, (), (override));
    MOCK_METHOD(int32_t, Read, (uint8_t &buffer, size_t userSize, bool isBlockingRead), (override));
    MOCK_METHOD(CapturerState, GetStatus, (), (const, override));
    MOCK_METHOD(bool, GetTimeStampInfo, (Timestamp &timestamp, Timestamp::Timestampbase base), (const, override));
    MOCK_METHOD(bool, GetAudioTime, (Timestamp &timestamp, Timestamp::Timestampbase base), (const, override));
    MOCK_METHOD(bool, GetFirstPkgTimeStampInfo, (int64_t &firstTs), (const, override));
    MOCK_METHOD(bool, Pause, (), (const, override));
    MOCK_METHOD(bool, Stop, (), (const, override));
    MOCK_METHOD(bool, Flush, (), (const, override));
    MOCK_METHOD(bool, Release, (), (override));
    MOCK_METHOD(int32_t, GetBufferSize, (size_t &bufferSize), (const, override));
    MOCK_METHOD(int32_t, GetAudioStreamId, (uint32_t &sessionID), (const, override));
    MOCK_METHOD(int32_t, GetFrameCount, (uint32_t &frameCount), (const, override));
    MOCK_METHOD(int32_t, SetCapturerPositionCallback,
        (int64_t markPosition, const std::shared_ptr<CapturerPositionCallback> &callback), (override));
    MOCK_METHOD(void, UnsetCapturerPositionCallback, (), (override));
    MOCK_METHOD(int32_t, SetCapturerPeriodPositionCallback,
        (int64_t frameNumber, const std::shared_ptr<CapturerPeriodPositionCallback> &callback), (override));
    MOCK_METHOD(void, UnsetCapturerPeriodPositionCallback, (), (override));
    MOCK_METHOD(int32_t, RegisterAudioPolicyServerDiedCb,
        (const int32_t clientPid, const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback), (override));
    MOCK_METHOD(int32_t, SetBufferDuration, (uint64_t bufferDuration), (const, override));
    MOCK_METHOD(int32_t, SetCaptureMode, (AudioCaptureMode captureMode), (override));
    MOCK_METHOD(AudioCaptureMode, GetCaptureMode, (), (const, override));
    MOCK_METHOD(int32_t, SetCapturerReadCallback,
        (const std::shared_ptr<AudioCapturerReadCallback> &callback), (override));
    MOCK_METHOD(int32_t, GetBufferDesc, (BufferDesc &bufDesc), (override));
    MOCK_METHOD(int32_t, Enqueue, (const BufferDesc &bufDesc), (override));
    MOCK_METHOD(int32_t, Clear, (), (const, override));
    MOCK_METHOD(int32_t, GetBufQueueState, (BufferQueueState &bufState), (const, override));
    MOCK_METHOD(void, SetValid, (bool valid), (override));
    MOCK_METHOD(int64_t, GetFramesRead, (), (const, override));
    MOCK_METHOD(int32_t, SetAudioCapturerDeviceChangeCallback,
        (const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, RemoveAudioCapturerDeviceChangeCallback,
        (const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, SetAudioCapturerInfoChangeCallback,
        (const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, RemoveAudioCapturerInfoChangeCallback,
        (const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback), (override));
    MOCK_METHOD(int32_t, RegisterAudioCapturerEventListener, (), (override));
    MOCK_METHOD(int32_t, UnregisterAudioCapturerEventListener, (), (override));
    MOCK_METHOD(int32_t, GetCurrentInputDevices, (AudioDeviceDescriptor &deviceInfo), (const, override));
    MOCK_METHOD(int32_t, GetCurrentCapturerChangeInfo, (AudioCapturerChangeInfo &changeInfo), (const, override));
    MOCK_METHOD(std::vector<sptr<MicrophoneDescriptor>>, GetCurrentMicrophones, (), (const, override));
    MOCK_METHOD(int32_t, GetAudioTimestampInfo,
        (Timestamp &timestamp, Timestamp::Timestampbase base), (const, override));
    MOCK_METHOD(uint32_t, GetOverflowCount, (), (const, override));
    MOCK_METHOD(int32_t, SetInputDevice, (DeviceType deviceType), (const, override));
    MOCK_METHOD(FastStatus, GetFastStatus, (), (override));
    MOCK_METHOD(int32_t, SetAudioSourceConcurrency, (const std::vector<SourceType> &targetSources), (override));
    MOCK_METHOD(int32_t, SetInterruptStrategy, (InterruptStrategy strategy), (override));
    MOCK_METHOD(void, SetInterruptEventCallbackType, (InterruptEventCallbackType callbackType), (override));
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CAPTURER_MOCK_H