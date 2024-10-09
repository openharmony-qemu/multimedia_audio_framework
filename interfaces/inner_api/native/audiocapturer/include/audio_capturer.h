/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_CAPTURER_H
#define AUDIO_CAPTURER_H

#include <memory>

#include "audio_info.h"
#include "microphone_descriptor.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Defines information about audio capturer parameters
 * @since 8
 */
struct AudioCapturerParams {
    /** Audio source type */
    AudioSourceType inputSource = AUDIO_MIC;
    /** Audio codec format */
    AudioEncodingType audioEncoding = ENCODING_PCM;
    /** Sampling rate */
    AudioSamplingRate samplingRate = SAMPLE_RATE_44100;
    /** Number of audio channels */
    AudioChannel audioChannel = MONO;
    /** Audio stream type */
    AudioStreamType streamType = STREAM_MEDIA;
    /** audioSampleFormat */
    AudioSampleFormat audioSampleFormat = SAMPLE_S16LE;
    /** Audio Channel Layout */
    AudioChannelLayout channelLayout = CH_LAYOUT_UNKNOWN;
};

class AudioCapturerCallback {
public:
    virtual ~AudioCapturerCallback() = default;

    /**
     * Called when an interrupt is received.
     *
     * @param interruptEvent Indicates the InterruptEvent information needed by client.
     * For details, refer InterruptEvent struct in audio_info.h
     * @since 10
     */
    virtual void OnInterrupt(const InterruptEvent &interruptEvent) = 0;

    /**
    * Called when renderer state is updated.
     *
     * @param state Indicates updated state of the capturer.
     * For details, refer enum CapturerState.
     * @since 8
     */
    virtual void OnStateChange(const CapturerState state) = 0;
};

class CapturerPositionCallback {
public:
    virtual ~CapturerPositionCallback() = default;

    /**
     * Called when the requested frame number is read.
     *
     * @param framePosition requested frame position.
     * @since 8
     */
    virtual void OnMarkReached(const int64_t &framePosition) = 0;
};

class CapturerPeriodPositionCallback {
public:
    virtual ~CapturerPeriodPositionCallback() = default;

    /**
     * Called when the requested frame count is read.
     *
     * @param frameCount requested frame frame count for callback.
     * @since 8
     */
    virtual void OnPeriodReached(const int64_t &frameNumber) = 0;
};

class AudioCapturerReadCallback {
public:
    virtual ~AudioCapturerReadCallback() = default;

    /**
     * Called when buffer to be enqueued.
     *
     * @param length Indicates requested buffer length.
     * @since 9
     */
    virtual void OnReadData(size_t length) = 0;
};

class AudioCapturerDeviceChangeCallback {
public:
    virtual ~AudioCapturerDeviceChangeCallback() = default;

    /**
     * Called when capturer device is updated.
     *
     * @param state Indicates updated device of the capturer.
     * since 11
     */
    virtual void OnStateChange(const DeviceInfo &deviceInfo) = 0;
};

class AudioCapturerInfoChangeCallback {
public:
    virtual ~AudioCapturerInfoChangeCallback() = default;

    /**
     * Called when capturer info is updated.
     *
     * @param state Indicates info of the capturer.
     * since 11
     */
    virtual void OnStateChange(const AudioCapturerChangeInfo &capturerChangeInfo) = 0;
};

/**
 * @brief Provides functions for applications to implement audio capturing.
 */
class AudioCapturer {
public:
    /**
     * @brief create capturer instance.
     *
     * @param options The audio capturer configuration to be used while creating capturer instance.
     * refer AudioCapturerOptions in audio_info.h.
     * @return Returns unique pointer to the AudioCapturer object
     * @since 8
     */
    static std::unique_ptr<AudioCapturer> Create(AudioStreamType audioStreamType);

    /**
     * @brief create capturer instance.
     *
     * @param options The audio capturer configuration to be used while creating capturer instance.
     * refer AudioCapturerOptions in audio_info.h.
     * @param appInfo Originating application's uid and token id can be passed here
     * @return Returns unique pointer to the AudioCapturer object
     * @since 9
     */
    static std::unique_ptr<AudioCapturer> Create(AudioStreamType audioStreamType, const AppInfo &appInfo);

    /**
     * @brief create capturer instance.
     *
     * @param options The audio capturer configuration to be used while creating capturer instance.
     * refer AudioCapturerOptions in audio_info.h.
     * @return Returns unique pointer to the AudioCapturer object
     * @since 8
     */
    static std::unique_ptr<AudioCapturer> Create(const AudioCapturerOptions &options);

    /**
     * @brief create capturer instance.
     *
     * @param options The audio capturer configuration to be used while creating capturer instance.
     * refer AudioCapturerOptions in audio_info.h.
     * @param appInfo Originating application's uid and token id can be passed here
     * @return Returns unique pointer to the AudioCapturer object
     * @since 9
     */
    static std::unique_ptr<AudioCapturer> Create(const AudioCapturerOptions &options, const AppInfo &appInfo);

    /**
     * @brief create capturer instance.
     *
     * @param options The audio capturer configuration to be used while creating capturer instance.
     * refer AudioCapturerOptions in audio_info.h.
     * @param cachePath Application cache path
     * @return Returns unique pointer to the AudioCapturer object
     * @since 9
     */
    static std::unique_ptr<AudioCapturer> Create(const AudioCapturerOptions &options, const std::string cachePath);

    /**
     * @brief create capturer instance.
     *
     * @param capturerOptions The audio capturer configuration to be used while creating capturer instance.
     * refer AudioCapturerOptions in audio_info.h.
     * @param cachePath Application cache path
     * @param appInfo Originating application's uid and token id can be passed here
     * @return Returns unique pointer to the AudioCapturer object
     * @since 9
     */
    static std::unique_ptr<AudioCapturer> Create(const AudioCapturerOptions &options, const std::string cachePath,
        const AppInfo &appInfo);

    /**
     * @brief Sets audio capture parameters.
     *
     * @param params Indicates information about audio capture parameters to set. For details, see
     * {@link AudioCapturerParams}.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t SetParams(const AudioCapturerParams params) = 0;

    /**
     * @brief Update AudioPlaybackCaptureConfig, only for Inner-Cap records.
     *
     * @param config Indicates information about audio capture parameters to set. For details, see
     * {@link CaptureFilterOptions}.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 12
     */
    virtual int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) = 0;

    /**
     * @brief Registers the capturer callback listener.
     * (1)If old SetParams(const AudioCapturerParams params) API,
     *    this API must be called immediately after SetParams.
     * (2) Else if using Create(const AudioCapturerOptions &capturerOptions),
     *    this API must be called immediately  after Create.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback) = 0;

    /**
     * @brief Obtains audio capturer parameters.
     *
     * This function can be called after {@link SetParams} is successful.
     *
     * @param params Indicates information about audio capturer parameters.For details,see
     * {@link AudioCapturerParams}.
     * @return Returns {@link SUCCESS} if the parameter information is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t GetParams(AudioCapturerParams &params) const = 0;

    /**
     * @brief Obtains audio capturer information.
     *
     * This function can be called after {@link SetParams} is successful.
     *
     * @param capturerInfo Indicates information about audio capturer information.For details,see
     * {@link AudioCapturerInfo}.
     * @return Returns {@link SUCCESS} if the parameter information is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t GetCapturerInfo(AudioCapturerInfo &capturerInfo) const = 0;

    /**
     * @brief Obtains audio stream information.
     *
     * This function can be called after {@link Create} is successful.
     *
     * @param streamInfo Indicates information about audio stream information.For details,see
     * {@link AudioStreamInfo}.
     * @return Returns {@link SUCCESS} if the parameter information is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const = 0;

    /**
     * @brief Starts audio capturing.
     *
     * @return Returns <b>true</b> if the capturing is successfully started; returns <b>false</b> otherwise.
     * @since 8
     */
    virtual bool Start() const = 0;

    /**
     * @brief capture audio data.
     *
     * @param buffer Indicates the pointer to the buffer into which the audio data is to be written.
     * @param userSize Indicates the size of the buffer into which the audio data is to be written, in bytes.
     * <b>userSize >= frameCount * channelCount * BytesPerSample</b> must evaluate to <b>true</b>. You can call
     * {@link GetFrameCount} to obtain the <b>frameCount</b> value.
     * @param isBlockingRead Specifies whether data reading will be blocked.
     * @return Returns the size of the audio data read from the device. The value ranges from <b>0</b> to
     * <b>userSize</b>. If the reading fails, one of the following error codes is returned.
     * <b>ERR_INVALID_PARAM</b>: The input parameter is incorrect.
     * <b>ERR_ILLEGAL_STATE</b>: The <b>AudioCapturer</b> instance is not initialized.
     * <b>ERR_INVALID_READ</b>: The read size < 0.
     * @since 8
     */
    virtual int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) const = 0;

    /**
     * @brief Obtains the audio capture state.
     *
     * @return Returns the audio capture state defined in {@link CapturerState}.
     * @since 9
     */
    virtual CapturerState GetStatus() const = 0;

    /**
     * @brief Obtains the Timestamp.
     *
     * @param timestamp Indicates a {@link Timestamp} instance reference provided by the caller.
     * @param base Indicates the time base, which can be {@link Timestamp.Timestampbase#BOOTTIME} or
     * {@link Timestamp.Timestampbase#MONOTONIC}.
     * @return Returns <b>true</b> if the timestamp is successfully obtained; returns <b>false</b> otherwise.
     * @since 8
     */
    virtual bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const = 0;

    /**
     * @brief Pause audio capturing.
     *
     * @return Returns <b>true</b> if the capturing is successfully Paused; returns <b>false</b> otherwise.
     * @since 9
     */
    virtual bool Pause() const = 0;

    /**
     * @brief Stops audio capturing.
     *
     * @return Returns <b>true</b> if the capturing is successfully stopped; returns <b>false</b> otherwise.
     * @since 8
     */
    virtual bool Stop() const = 0;
    /**
     * @brief flush capture stream.
     *
     * @return Returns <b>true</b> if the object is successfully flushed; returns <b>false</b> otherwise.
     * @since 8
     */
    virtual bool Flush() const = 0;

    /**
     * @brief Releases a local <b>AudioCapturer</b> object.
     *
     * @return Returns <b>true</b> if the object is successfully released; returns <b>false</b> otherwise.
     * @since 8
     */
    virtual bool Release() = 0;

    /**
     * @brief Obtains a reasonable minimum buffer size for capturer, however, the capturer can
     *        accept other read sizes as well.
     *
     * @param bufferSize Indicates a buffersize pointer value that wil be written.
     * @return Returns {@link SUCCESS} if bufferSize is successfully obtained; returns an error code
     * @since 8
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetBufferSize(size_t &bufferSize) const = 0;

    /**
     * @brief Obtains the capturer stream id.
     *
     * @param sessionId Indicates the reference variable into which stream id value will be written.
     * @return Returns {@link SUCCESS} if stream id is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 10
     */
    virtual int32_t GetAudioStreamId(uint32_t &sessionID) const = 0;

    /* @brief Obtains the number of frames required in the current condition, in bytes per sample.
     *
     * @param frameCount Indicates the pointer in which framecount will be written
     * @return Returns {@link SUCCESS} if frameCount is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */

    virtual int32_t GetFrameCount(uint32_t &frameCount) const = 0;
    /**
     * @brief Registers the capturer position callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */

    virtual int32_t SetCapturerPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPositionCallback> &callback) = 0;

    /**
     * @brief Unregisters the capturer position callback listener
     *
     * @since 8
     */
    virtual void UnsetCapturerPositionCallback() = 0;

    /**
     * @brief Registers the capturer period position callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t SetCapturerPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) = 0;

    /**
     * @brief Unregisters the capturer period position callback listener
     *
     * @since 8
     */
    virtual void UnsetCapturerPeriodPositionCallback() = 0;

    /**
     * @brief set the buffer duration for capturer, minimum buffer duration is 5msec
     *         maximum is 20msec
     *
     * @param bufferDuration  Indicates a buffer duration to be set for capturer
     * @return Returns {@link SUCCESS} if bufferDuration is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    virtual int32_t SetBufferDuration(uint64_t bufferDuration) const = 0;

    /**
     * @brief Set the application cache path to access the application resources
     *
     * @param cachePath Indicates application cache path.
     * @return none
     * @since 8
     */
    virtual void SetApplicationCachePath(const std::string cachePath) = 0;

    /**
     * @brief Obtains the capturer supported formats.
     *
     * @return vector with capturer supported formats.
     * @since 8
     */
    static std::vector<AudioSampleFormat> GetSupportedFormats();

    /**
     * @brief Obtains the capturer supported channels.
     *
     * @return vector with capturer supported channels.
     * @since 8
     */
    static std::vector<AudioChannel> GetSupportedChannels();

    /**
     * @brief Obtains the capturer supported encoding types.
     *
     * @return vector with capturer supported encoding types.
     * @since 8
     */
    static std::vector<AudioEncodingType> GetSupportedEncodingTypes();

    /**
     * @brief Obtains the capturer supported SupportedSamplingRates.
     *
     * @return vector with capturer supported SupportedSamplingRates.
     * @since 8
     */
    static std::vector<AudioSamplingRate> GetSupportedSamplingRates();

    /**
     * @brief Sets the capture mode. By default the mode is CAPTURE_MODE_NORMAL.
     * This API is needs to be used only if CAPTURE_MODE_CALLBACK is required.
     *
     * * @param captureMode The mode of capture.
     * @return  Returns {@link SUCCESS} if capture mode is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    virtual int32_t SetCaptureMode(AudioCaptureMode captureMode) = 0;

    /**
     * @brief Obtains the capture mode.
     *
     * @return  Returns current capture mode.
     * @since 9
     */
    virtual AudioCaptureMode GetCaptureMode() const = 0;

    /**
     * @brief Registers the capturer read callback listener.
     * This API should only be used if CAPTURE_MODE_CALLBACK is needed.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    virtual int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) = 0;

    /**
     * @brief Gets the BufferDesc to read the data.
     * This API should only be used if CAPTURE_MODE_CALLBACK is needed.
     *
     * @param bufDesc Indicates the buffer descriptor from which data will be read.
     * refer BufferQueueState in audio_info.h.
     * @return Returns {@link SUCCESS} if bufDesc is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    virtual int32_t GetBufferDesc(BufferDesc &bufDesc) const = 0;

    /**
     * @brief Enqueues used buffer to the bufferQueue for recording new data.
     * This API should only be used if CAPTURE_MODE_CALLBACK is needed.
     *
     * @return Returns {@link SUCCESS} if bufDesc is successfully enqueued; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    virtual int32_t Enqueue(const BufferDesc &bufDesc) const = 0;

    /**
     * @brief Clears the bufferQueue.
     * This API should only be used if CAPTURE_MODE_CALLBACK is needed.
     *
     * @return Returns {@link SUCCESS} if successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    virtual int32_t Clear() const = 0;

    /**
     * @brief Obtains the current state of bufferQueue.
     * This API should only be used if CAPTURE_MODE_CALLBACK is needed.
     *
     * @param bufDesc Indicates the bufState reference in which state will be obtained.
     * refer BufferQueueState in audio_info.h.
     * @return Returns {@link SUCCESS} if bufState is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    virtual int32_t GetBufQueueState(BufferQueueState &bufState) const = 0;

    /**
     * @brief Set audiocapture valid state.
     *
     * @param valid Valid or not.
     * @since 10
     */
    virtual void SetValid(bool valid) = 0;

    /**
     * @brief Gets the audio frame size that has been read.
     *
     * @return Returns the audio frame size that has been read.
     * @since 10
     */
    virtual int64_t GetFramesRead() const = 0;

    /**
     * @brief Sets the audio device change callback.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t SetAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) = 0;

    /**
     * @brief Unsets the audio device change callback.
     *
     * @return Returns {@link SUCCESS} if callback remove is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t RemoveAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) = 0;

    /**
     * @brief Sets the audio capturer info change callback.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t SetAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) = 0;

    /**
     * @brief Removes the audio capturer info change callback.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t RemoveAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) = 0;

    /**
     * @brief Register the audio capturer event change callback.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t RegisterAudioCapturerEventListener() = 0;

    /**
     * @brief Unregister the audio capturer event change callback.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t UnregisterAudioCapturerEventListener() = 0;

    /**
     * @brief Gets current input devices.
     *
     * @return Returns current input device info.
     * @since 11
     */
    virtual int32_t GetCurrentInputDevices(DeviceInfo &deviceInfo) const = 0;

    /**
     * @brief Gets the current audio capturer change info.
     *
     * @return Returns the audio capturer change info.
     * @since 11
     */
    virtual int32_t GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const = 0;

    /**
     * @brief Obtains microphones this capturer used currently.
     *
     * @return Returns Microphone descriptors.
     * @since 11
     */
    virtual std::vector<sptr<MicrophoneDescriptor>> GetCurrentMicrophones() const = 0;

    /**
     * @brief Set Capturer Silent State.
     *
     * @return Returns {@link SUCCESS} if state setting is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    virtual int32_t SetCaptureSilentState(bool state) = 0;

    virtual uint32_t GetOverflowCount() const = 0;

    virtual int32_t SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources) = 0;

    virtual ~AudioCapturer();

protected:
    static AudioStreamType FindStreamTypeBySourceType(SourceType sourceType);

private:
    static void SendCapturerCreateError(const SourceType &sourceType,
        const int32_t &errorCode);
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_H
