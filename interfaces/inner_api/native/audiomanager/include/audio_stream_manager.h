/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_STREAM_MANAGER_H
#define ST_AUDIO_STREAM_MANAGER_H

#include <iostream>
#include <map>
#include "audio_effect.h"
#include "audio_system_manager.h"
#include "audio_stream_change_info.h"

namespace OHOS {
namespace AudioStandard {
class DeviceChangeWithInfoCallback {
public:
    virtual ~DeviceChangeWithInfoCallback() = default;

    virtual void OnDeviceChangeWithInfo(const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReasonExt reason) = 0;

    virtual void OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) = 0;
};

class AudioFormatUnsupportedErrorCallback {
public:
    virtual ~AudioFormatUnsupportedErrorCallback() = default;

    /**
     * Called when format unsupported error occurs
     *
     * @param errorCode ErrorCode information.
     */
    virtual void OnFormatUnsupportedError(const AudioErrors &errorCode) = 0;
};

class AudioClientTracker {
public:
    virtual ~AudioClientTracker() = default;

    /**
     * Mute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;

    /**
     * Unmute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;

    /**
     * Paused Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;

     /**
     * Resumed Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;

    /**
     * Set low power volume was controlled by system application
     *
     * @param volume volume value.
     */
    virtual void SetLowPowerVolumeImpl(float volume) = 0;

    /**
     * Get low power volume was controlled by system application
     *
     * @param volume volume value.
     */
    virtual void GetLowPowerVolumeImpl(float &volume) = 0;

    /**
     * Set Stream into a specified Offload state
     *
     * @param state power state.
     * @param isAppBack app state.
     */
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) = 0;

    /**
     * Unset Stream out of Offload state
     *
     */
    virtual void UnsetOffloadModeImpl() = 0;

    /**
     * Get single stream was controlled by system application
     *
     * @param volume volume value.
     */
    virtual void GetSingleStreamVolumeImpl(float &volume) = 0;
};

class AudioStreamManager {
public:
    AudioStreamManager() = default;
    virtual ~AudioStreamManager() = default;

    static AudioStreamManager *GetInstance();

    /**
     * @brief Registers the renderer event callback listener.
     *
     * @param clientPid client PID
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     * @deprecated since 12
     */
    int32_t RegisterAudioRendererEventListener(const int32_t clientPid,
                                              const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    /**
     * @brief Unregisters the renderer event callback listener.
     *
     * @param clientPid client PID
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     * @deprecated since 12
     */
    int32_t UnregisterAudioRendererEventListener(const int32_t clientPid);

    /**
     * @brief Registers the renderer event callback listener.
     *
     * @param callback
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t RegisterAudioRendererEventListener(const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    /**
     * @brief Unregisters the renderer event callback listener.
     *
     * @param callback need to unregister.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t UnregisterAudioRendererEventListener(const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    /**
     * @brief Registers the capturer event callback listener.
     *
     * @param clientPid client PID
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t RegisterAudioCapturerEventListener(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerStateChangeCallback> &callback);

    /**
     * @brief Unregisters the capturer event callback listener.
     *
     * @param clientPid client PID
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t UnregisterAudioCapturerEventListener(const int32_t clientPid);

    /**
     * @brief Get current renderer change Infos.
     *
     * @param audioRendererChangeInfos  audioRendererChangeInfos
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);

    /**
     * @brief Get current capturer change Infos.
     *
     * @param audioRendererChangeInfos  audioRendererChangeInfos
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);

    /**
     * @brief Get Audio Effect Infos.
     *
     * @param AudioSceneEffectInfo  AudioSceneEffectInfo
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t GetEffectInfoArray(AudioSceneEffectInfo &audioSceneEffectInfo, StreamUsage streamUsage);

    /**
     * @brief Get Audio render Effect param.
     *
     * @param AudioSceneEffectInfo  AudioSceneEffectInfo
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray);

    /**
     * @brief Get Audio Capture Effect param.
     *
     * @param AudioSceneEffectInfo  AudioSceneEffectInfo
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);

    /**
     * @brief Sets the audio effect Param.
     *
     * * @param effectParam The audio effect Param at which the stream needs to be rendered.
     * @return  Returns {@link SUCCESS} if audio effect Param is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray);

    /**
     * @brief Gets the audio effect Param.
     *
     * * @param effectParam The audio effect moParamde at which the stream needs to be rendered.
     * @return  Returns {@link SUCCESS} if audio effect Param is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray);

    /**
     * @brief Sets the audio effect Param.
     *
     * * @param effectParam The audio effect Param at which the stream needs to be rendered.
     * @return  Returns {@link SUCCESS} if audio effect Param is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray);

    /**
     * @brief Gets the audio effect Param.
     *
     * * @param effectParam The audio effect moParamde at which the stream needs to be rendered.
     * @return  Returns {@link SUCCESS} if audio effect Param is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 13
     */
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);

    /**
     * @brief Is stream active.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsStreamActive(AudioVolumeType volumeType) const;

    /**
     * @brief Is stream active.
     *
     * @param streamUsage stream usage.
     * @return Returns <b>true</b> if the stream is active; returns <b>false</b> otherwise.
     * @since 20
     */
    bool IsStreamActiveByStreamUsage(StreamUsage streamUsage) const;

    /**
     * @brief Is fast playback supported.
     *
     * @param streamInfo audio stream info.
     * @param usage  StreamUsage.
     * @return Returns <b>true</b> if the stream is support fast playback; returns <b>false</b> otherwise.
     * @since 20
     */
    bool IsFastPlaybackSupported(AudioStreamInfo &streamInfo, StreamUsage usage);

    /**
     * @brief Is fast recording supported.
     *
     * @param streamInfo audio stream info.
     * @param source  SourceType.
     * @return Returns <b>true</b> if the stream is support fast recording; returns <b>false</b> otherwise.
     * @since 20
     */
    bool IsFastRecordingSupported(AudioStreamInfo &streamInfo, SourceType source);

    /**
     * @brief Gets sampling rate for hardware output.
     *
     * @param AudioDeviceDescriptor Target output device.
     * @return The sampling rate for output.
     * @since 11
     */
    int32_t GetHardwareOutputSamplingRate(std::shared_ptr<AudioDeviceDescriptor> &desc);

    /**
     * @brief Judges whether the playback is supported by the renderer.
     *
     * @param streamInfo AudioStreamInfo
     * @param streamUsage StreamUsage
     * @return Returns direct playback mode.
     * @since 19
     */
    DirectPlaybackMode GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo, const StreamUsage &streamUsage);

    /**
     * @brief Sets format unsupported error callback.
     *
     * @param callback The format unsupported error callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 19
     */
    int32_t SetAudioFormatUnsupportedErrorCallback(
        const std::shared_ptr<AudioFormatUnsupportedErrorCallback> &callback);

    /**
     * @brief Unsets format unsupported error callback.
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 19
     */
    int32_t UnsetAudioFormatUnsupportedErrorCallback();

    /**
     * Query whether Acoustic Echo Canceler is supported on input SourceType.
     * @param { SourceType } sourceType - Audio source type.
     * @returns { bool } Promise used to return the support status of Acoustic Echo Canceler.
     * The value true means that Acoustic Echo Canceler is supported, and false means the opposite.
     * @since 20
     */
    bool IsAcousticEchoCancelerSupported(SourceType sourceType);

    /**
     * @brief Force Stop the audio stream.
     *
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t ForceStopAudioStream(StopAudioType audioType);

    /**
     * Checks whether it is possible to obtain the recording focus.
     * @param { AudioCapturerInfo } AudioCapturerInfo.
     * @returns { bool } Indicating whether obtaining the recording focus is possible.
     *
     * @since 20
     */
    bool IsCapturerFocusAvailable(const AudioCapturerInfo &capturerInfo);

    /**
     * Checks whether the audio loopback is supported.
     * @param   { AudioLoopbackMode } mode - The audio loopback mode.
     * @returns { bool } The value true means that the audio loopback is supported,
     *          and false means the opposite.
     * @since 20
     */
    bool IsAudioLoopbackSupported(AudioLoopbackMode mode);
private:
    std::mutex rendererStateChangeCallbacksMutex_;
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> rendererStateChangeCallbacks_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_STREAM_MANAGER_H
