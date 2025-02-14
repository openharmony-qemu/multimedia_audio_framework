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
     * @brief Gets sampling rate for hardware output.
     *
     * @param AudioDeviceDescriptor Target output device.
     * @return The sampling rate for output.
     * @since 11
     */
    int32_t GetHardwareOutputSamplingRate(std::shared_ptr<AudioDeviceDescriptor> &desc);

private:
    std::mutex rendererStateChangeCallbacksMutex_;
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> rendererStateChangeCallbacks_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_STREAM_MANAGER_H
