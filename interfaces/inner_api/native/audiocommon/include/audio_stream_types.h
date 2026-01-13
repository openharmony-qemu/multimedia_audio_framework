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

#ifndef AUDIO_STREAM_TYPES_H
#define AUDIO_STREAM_TYPES_H

#include "audio_stutter.h"
#include "audio_stream_info.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief AudioRendererFilter is used for select speficed AudioRenderer.
 */
class AudioRendererFilter : public Parcelable {
public:
    AudioRendererFilter();
    virtual ~AudioRendererFilter();

    int32_t uid = -1;
    AudioRendererInfo rendererInfo = {};
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    int32_t streamId = -1;

    bool Marshalling(Parcel &parcel) const override;
    static AudioRendererFilter* Unmarshalling(Parcel &parcel);
};

/**
 * @brief AudioCapturerFilter is used for select speficed audiocapturer.
 */
class AudioCapturerFilter : public Parcelable {
public:
    AudioCapturerFilter();
    virtual ~AudioCapturerFilter();

    int32_t uid = -1;
    AudioCapturerInfo capturerInfo = {SOURCE_TYPE_INVALID, 0};

    bool Marshalling(Parcel &parcel) const override;
    static AudioCapturerFilter *Unmarshalling(Parcel &in);
};

class AudioParameterCallback {
public:
    virtual ~AudioParameterCallback() = default;
    /**
     * @brief AudioParameterCallback will be executed when parameter change.
     *
     * @param networkId networkId
     * @param key  Audio paramKey
     * @param condition condition
     * @param value value
     * @since 9
     */
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) = 0;

    virtual void OnHdiRouteStateChange(const std::string &networkId, bool enable) = 0;
};

class AudioCollaborationEnabledChangeForCurrentDeviceCallback {
public:
    virtual ~AudioCollaborationEnabledChangeForCurrentDeviceCallback() = default;
    /**
     * @brief AudioCollaborationEnabledChangeForCurrentDeviceCallback will be executed
     *  when collboration enabled state changes
     *
     * @param enabled the collboration enabled state for current device.
     * @since 20
     */
    virtual void OnCollaborationEnabledChangeForCurrentDevice(const bool &enabled) {}
};

class AudioNnStateChangeCallback {
public:
    virtual ~AudioNnStateChangeCallback() = default;

    virtual void OnNnStateChange(const int32_t &nnState) = 0;
};

class AudioCapturerSourceCallback {
public:
    virtual ~AudioCapturerSourceCallback() = default;
    virtual void OnCapturerState(bool isActive) = 0;
};

class WakeUpSourceCloseCallback {
public:
    virtual ~WakeUpSourceCloseCallback() = default;
    virtual void OnWakeupClose() = 0;
};

class WakeUpSourceCallback : public AudioCapturerSourceCallback, public WakeUpSourceCloseCallback {
public:
    virtual ~WakeUpSourceCallback() = default;
    // Stop all listening capturers from sending false callbacks;
    // when all capturers have stopped, allow one capturer to start sending true callbacks
    virtual void OnCapturerState(bool isActive) = 0;
    virtual void OnWakeupClose() = 0;
};

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
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_TYPES_H
