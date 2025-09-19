/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_CAPTURER_PRIVATE_H
#define AUDIO_CAPTURER_PRIVATE_H

#include <mutex>
#include <shared_mutex>
#include "audio_utils.h"
#include "audio_interrupt_callback.h"
#include "i_audio_stream.h"
#include "audio_stream_descriptor.h"
#include "audio_capturer_proxy_obj.h"
#include "audio_task_loop.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t INVALID_SESSION_ID = static_cast<uint32_t>(-1);
class AudioCapturerStateChangeCallbackImpl;
class CapturerPolicyServiceDiedCallback;
class InputDeviceChangeWithInfoCallbackImpl;

class AudioCapturerPrivate : public AudioCapturer, public std::enable_shared_from_this<AudioCapturerPrivate> {
public:
    int32_t GetFrameCount(uint32_t &frameCount) const override;
    int32_t SetParams(const AudioCapturerParams params) override;
    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override;
    int32_t SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback) override;
    int32_t GetParams(AudioCapturerParams &params) const override;
    int32_t GetCapturerInfo(AudioCapturerInfo &capturerInfo) const override;
    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override;
    bool Start() override;
    int32_t  Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override;
    CapturerState GetStatus() const override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool GetFirstPkgTimeStampInfo(int64_t &firstTs) const override;
    bool Pause() const override;
    bool Stop() const override;
    bool Flush() const override;
    bool Release() override;
    int32_t GetBufferSize(size_t &bufferSize) const override;
    int32_t GetAudioStreamId(uint32_t &sessionID) const override;
    int32_t SetCapturerPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPositionCallback> &callback) override;
    void UnsetCapturerPositionCallback() override;
    int32_t SetCapturerPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override;
    void UnsetCapturerPeriodPositionCallback() override;
    int32_t SetBufferDuration(uint64_t bufferDuration) const override;
    int32_t SetCaptureMode(AudioCaptureMode renderMode) override;
    AudioCaptureMode GetCaptureMode()const override;
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    int32_t Clear()const override;
    int32_t GetBufQueueState(BufferQueueState &bufState)const override;
    void SetValid(bool valid) override;
    int64_t GetFramesRead() const override;
    int32_t GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const override;
    int32_t GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const override;
    int32_t SetAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) override;
    int32_t RemoveAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) override;
    int32_t SetAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) override;
    int32_t RemoveAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) override;
    int32_t RegisterAudioCapturerEventListener() override;
    int32_t UnregisterAudioCapturerEventListener() override;
    void SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback) override;
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback) override;
    void SetFastStatusChangeCallback(const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback) override;

    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool GetTimeStampInfo(Timestamp &timestampNs, Timestamp::Timestampbase base) const override;
    int32_t RegisterCapturerPolicyServiceDiedCallback();
    int32_t RemoveCapturerPolicyServiceDiedCallback();

    bool IsDeviceChanged(AudioDeviceDescriptor &newDeviceInfo);
    std::vector<sptr<MicrophoneDescriptor>> GetCurrentMicrophones() const override;

    void GetAudioInterrupt(AudioInterrupt &audioInterrupt);
    int32_t SetInputDevice(DeviceType deviceType) const override;
    void SetAudioInterrupt(const AudioInterrupt &audioInterrupt);
    FastStatus GetFastStatus() override;

    uint32_t GetOverflowCount() const override;

    int32_t SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources) override;
    int32_t SetInterruptStrategy(InterruptStrategy strategy) override;

    void SetInterruptEventCallbackType(InterruptEventCallbackType callbackType) override;

    void RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter);
    void HandleSetCapturerInfoByOptions(const AudioCapturerOptions &capturerOptions, const AppInfo &appInfo);

    std::shared_ptr<IAudioStream> audioStream_;
    AudioCapturerInfo capturerInfo_ = {};
    AudioPlaybackCaptureConfig filterConfig_ = {};
    AudioStreamType audioStreamType_;
    bool abortRestore_ = false;
    AudioSessionStrategy strategy_ = { AudioConcurrencyMode::INVALID };

    AudioCapturerPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo, bool createStream = true);
    virtual ~AudioCapturerPrivate();
    bool isChannelChange_ = false;
    static inline AudioStreamParams ConvertToAudioStreamParams(const AudioCapturerParams params)
    {
        AudioStreamParams audioStreamParams;

        audioStreamParams.format = params.audioSampleFormat;
        audioStreamParams.samplingRate = params.samplingRate;
        audioStreamParams.channels = params.audioChannel;
        audioStreamParams.encoding = params.audioEncoding;
        audioStreamParams.channelLayout = params.channelLayout;

        return audioStreamParams;
    }

private:
    int32_t CheckAndRestoreAudioCapturer(std::string callingFunc);
    int32_t AsyncCheckAudioCapturer(std::string callingFunc);
    int32_t CheckAudioCapturer(std::string callingFunc);
    int32_t CheckAndStopAudioCapturer(std::string callingFunc);
    int32_t InitAudioInterruptCallback();
    std::shared_ptr<AudioStreamDescriptor> ConvertToStreamDescriptor(const AudioStreamParams &audioStreamParams);
    IAudioStream::StreamClass DecideStreamClassAndUpdateCapturerInfo(uint32_t flag);
    int32_t InitInputDeviceChangeCallback();
    int32_t SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream);
    void InitSwitchInfo(IAudioStream::StreamClass targetClass, IAudioStream::SwitchInfo &info);
    bool ContinueAfterSplit(RestoreInfo restoreInfo);
    bool SwitchToTargetStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo);
    bool FinishOldStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo, CapturerState previousState,
        IAudioStream::SwitchInfo &info);
    bool GenerateNewStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo, CapturerState previousState,
        IAudioStream::SwitchInfo &info);
    void HandleAudioInterruptWhenServerDied();
    void InitLatencyMeasurement(const AudioStreamParams &audioStreamParams);
    int32_t InitAudioStream(const AudioStreamParams &AudioStreamParams);
    FastStatus GetFastStatusInner();
    void FastStatusChangeCallback(FastStatus status);
    void CheckSignalData(uint8_t *buffer, size_t bufferSize) const;
    void WriteOverflowEvent() const;
    int32_t GetCurrentInputDevicesInner(AudioDeviceDescriptor &deviceInfo) const;
    int32_t GetAudioStreamIdInner(uint32_t &sessionID) const;
    uint32_t GetOverflowCountInner() const;
    CapturerState GetStatusInner() const;
    std::shared_ptr<IAudioStream> GetInnerStream() const;
    IAudioStream::StreamClass GetPreferredStreamClass(AudioStreamParams audioStreamParams);
    IAudioStream::StreamClass SetCaptureInfo(AudioStreamParams &audioStreamParams);
    std::shared_ptr<AudioStreamDescriptor> GenerateStreamDesc(
        const IAudioStream::SwitchInfo &switchInfo, const RestoreInfo &restoreInfo);
    int32_t HandleCreateFastStreamError(AudioStreamParams &audioStreamParams);
    bool IsRestoreOrStopNeeded();
    void SetInSwitchingFlag(bool inSwitchingFlag);

    std::shared_ptr<InputDeviceChangeWithInfoCallbackImpl> inputDeviceChangeCallback_ = nullptr;
    bool isSwitching_ = false;
    mutable std::shared_mutex switchStreamMutex_;
    std::shared_ptr<AudioStreamCallback> audioStreamCallback_ = nullptr;
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;
    std::shared_ptr<AudioCapturerErrorCallback> audioCapturerErrorCallback_ = nullptr;
    AppInfo appInfo_ = {};
    AudioInterrupt audioInterrupt_ = {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, false}, 0};
    bool isVoiceCallCapturer_ = false;
    uint32_t sessionID_ = INVALID_SESSION_ID;
    std::shared_ptr<AudioCapturerProxyObj> capturerProxyObj_;
    static std::map<AudioStreamType, SourceType> streamToSource_;
    std::mutex lock_;
    bool isValid_ = true;
    std::shared_ptr<AudioCapturerStateChangeCallbackImpl> audioStateChangeCallback_ = nullptr;
    std::shared_ptr<CapturerPolicyServiceDiedCallback> audioPolicyServiceDiedCallback_ = nullptr;
    std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> policyServiceDiedCallback_ = nullptr;
    AudioDeviceDescriptor currentDeviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    bool latencyMeasEnabled_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    mutable std::mutex signalDetectAgentMutex_;
    FILE *dumpFile_ = nullptr;
    AudioCaptureMode audioCaptureMode_ = CAPTURE_MODE_NORMAL;
    bool isFastVoipSupported_ = false;
    std::mutex setCapturerCbMutex_;
    mutable std::shared_mutex capturerMutex_;
    std::mutex capturerPolicyServiceDiedCbMutex_;
    std::mutex audioCapturerErrCallbackMutex_;
    std::mutex policyServiceDiedCallbackMutex_;
    std::mutex audioInterruptMutex_;
    int32_t callbackLoopTid_ = -1;
    std::shared_ptr<AudioCapturerFastStatusChangeCallback> fastStatusChangeCallback_ = nullptr;
    std::mutex fastStatusChangeCallbackMutex_;
    std::atomic<uint32_t> switchStreamInNewThreadTaskCount_ = 0;

    AudioLoopThread taskLoop_ = AudioLoopThread("OS_Recreate");
    std::condition_variable taskLoopCv_;
    std::mutex inSwitchingMtx_;
    bool inSwitchingFlag_ = false;
};

class AudioCapturerInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioCapturerInterruptCallbackImpl(const std::shared_ptr<IAudioStream> &audioStream);
    virtual ~AudioCapturerInterruptCallbackImpl();

    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback);
    void UpdateAudioStream(const std::shared_ptr<IAudioStream> &audioStream);
    void StartSwitch();
    void FinishSwitch();
private:
    void NotifyEvent(const InterruptEvent &interruptEvent);
    std::shared_ptr<IAudioStream> audioStream_;
    std::weak_ptr<AudioCapturerCallback> callback_;
    bool isForcePaused_ = false;
    std::shared_ptr<AudioCapturerCallback> cb_;
    std::mutex mutex_;
    bool switching_ = false;
    std::condition_variable switchStreamCv_;
};

class AudioStreamCallbackCapturer : public AudioStreamCallback {
public:
    AudioStreamCallbackCapturer(std::weak_ptr<AudioCapturerPrivate> capturer);
    virtual ~AudioStreamCallbackCapturer() = default;

    void OnStateChange(const State state, const StateChangeCmdType __attribute__((unused)) cmdType) override;
    void SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback);
private:
    std::weak_ptr<AudioCapturerCallback> callback_;
    std::weak_ptr<AudioCapturerPrivate> capturer_;
};

class AudioCapturerStateChangeCallbackImpl : public AudioCapturerStateChangeCallback {
public:
    AudioCapturerStateChangeCallbackImpl();
    virtual ~AudioCapturerStateChangeCallbackImpl();

    void OnCapturerStateChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;
    void SaveDeviceChangeCallback(const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback);
    void RemoveDeviceChangeCallback(const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback);
    int32_t DeviceChangeCallbackArraySize();
    void SaveCapturerInfoChangeCallback(const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback);
    void RemoveCapturerInfoChangeCallback(const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback);
    int32_t GetCapturerInfoChangeCallbackArraySize();
    void SetAudioCapturerObj(std::weak_ptr<AudioCapturerPrivate> capturerObj);
    void NotifyAudioCapturerDeviceChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    void NotifyAudioCapturerInfoChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    void HandleCapturerDestructor();
private:
    std::vector<std::shared_ptr<AudioCapturerDeviceChangeCallback>> deviceChangeCallbacklist_;
    std::vector<std::shared_ptr<AudioCapturerInfoChangeCallback>> capturerInfoChangeCallbacklist_;
    std::mutex capturerMutex_;
    std::weak_ptr<AudioCapturerPrivate> capturer_;
    std::mutex deviceChangeCallbackMutex_;
};

class InputDeviceChangeWithInfoCallbackImpl : public DeviceChangeWithInfoCallback {
public:
    InputDeviceChangeWithInfoCallbackImpl() = default;

    virtual ~InputDeviceChangeWithInfoCallbackImpl() = default;

    void OnDeviceChangeWithInfo(const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReasonExt reason) override;

    void OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override;

    void SetAudioCapturerObj(std::weak_ptr<AudioCapturerPrivate> capturerObj)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        capturer_ = capturerObj;
    }

    void UnsetAudioCapturerObj()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        capturer_.reset();
    }
private:
    std::weak_ptr<AudioCapturerPrivate> capturer_;
    std::mutex mutex_;
};

class CapturerPolicyServiceDiedCallback : public AudioStreamPolicyServiceDiedCallback,
    public std::enable_shared_from_this<CapturerPolicyServiceDiedCallback> {
public:
    CapturerPolicyServiceDiedCallback();
    virtual ~CapturerPolicyServiceDiedCallback();
    void SetAudioCapturerObj(std::weak_ptr<AudioCapturerPrivate>);
    void SetAudioInterrupt(AudioInterrupt &audioInterrupt);
    void OnAudioPolicyServiceDied() override;

private:
    std::weak_ptr<AudioCapturerPrivate> capturer_;
    AudioInterrupt audioInterrupt_;
    void RestoreTheadLoop();
    std::atomic<int32_t> taskCount_ = 0;
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif // AUDIO_CAPTURER_PRIVATE_H
