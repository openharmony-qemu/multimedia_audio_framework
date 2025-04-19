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

#ifndef ST_AUDIO_POLICY_INTERFACE_H
#define ST_AUDIO_POLICY_INTERFACE_H

#include <list>
#include <mutex>

#include "audio_device_descriptor.h"
#include "audio_device_info.h"
#include "audio_info.h"
#include "audio_interrupt_info.h"
#include "audio_stream_change_info.h"

namespace OHOS {
namespace AudioStandard {
/**
 * Describes the device change type and device information.
 *
 * @since 7
 */
struct DeviceChangeAction {
    DeviceChangeType type;
    DeviceFlag flag;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors;
};

class AudioFocusInfoChangeCallback {
public:
    virtual ~AudioFocusInfoChangeCallback() = default;
    /**
     * Called when focus info change.
     *
     * @param focusInfoList Indicates the focusInfoList information needed by client.
     * For details, refer audioFocusInfoList_ struct in audio_policy_server.h
     * @since 9
     */
    virtual void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) = 0;

    virtual void OnAudioFocusRequested(const AudioInterrupt &) {}

    virtual void OnAudioFocusAbandoned(const AudioInterrupt &) {}
};

class AudioDeviceRefiner {
public:
    virtual ~AudioDeviceRefiner() = default;

    virtual int32_t OnAudioOutputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, StreamUsage streamUsage, int32_t clientUid, AudioPipeType audioPipeType) = 0;
    virtual int32_t OnAudioInputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, SourceType sourceType, int32_t clientUid, AudioPipeType audioPipeType) = 0;
};

class AudioClientInfoMgrCallback {
public:
    virtual ~AudioClientInfoMgrCallback() = default;
    virtual bool OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid) = 0;
};

class AudioPreferredOutputDeviceChangeCallback {
public:
    virtual ~AudioPreferredOutputDeviceChangeCallback() = default;
    /**
     * Called when the prefer output device changes
     *
     * @param vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptor.
     */
    virtual void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) = 0;
};

class AudioPreferredInputDeviceChangeCallback {
    public:
    virtual ~AudioPreferredInputDeviceChangeCallback() = default;
    /**
     * Called when the prefer input device changes
     *
     * @param vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptor.
     */
    virtual void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) = 0;
};

class AudioManagerDeviceChangeCallback {
public:
    virtual ~AudioManagerDeviceChangeCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param deviceChangeAction Indicates the DeviceChangeAction information needed by client.
     * For details, refer DeviceChangeAction struct
     * @since 8
     */
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) = 0;
};

class AudioDistribuitedOutputChangeCallback {
public:
    virtual ~AudioDistribuitedOutputChangeCallback() = default;
    /**
     * Called when Distribuited output device changed
     *
     * @param deviceDesc selected device
     * @param isRemote is cast direction remote
     * @since 16
     */
    virtual void OnDistribuitedOutputChange(const AudioDeviceDescriptor &deviceDesc, bool isRemote) = 0;
};

class AudioQueryClientTypeCallback {
public:
    virtual ~AudioQueryClientTypeCallback() = default;
    virtual bool OnQueryClientType(const std::string &bundleName, uint32_t uid) = 0;
};

class VolumeKeyEventCallback {
public:
    virtual ~VolumeKeyEventCallback() = default;
    /**
     * @brief VolumeKeyEventCallback will be executed when hard volume key is pressed up/down
     *
     * @param volumeEvent the volume event info.
     * @since 8
     */
    virtual void OnVolumeKeyEvent(VolumeEvent volumeEvent) = 0;
};

class AudioCapturerStateChangeCallback {
public:
    virtual ~AudioCapturerStateChangeCallback() = default;
    /**
     * Called when the capturer state changes
     *
     * @param capturerChangeInfo Contains the renderer state information.
     */
    virtual void OnCapturerStateChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) = 0;
    std::mutex cbMutex_;
};

class AudioRendererStateChangeCallback {
public:
    virtual ~AudioRendererStateChangeCallback() = default;
    /**
     * Called when the renderer state changes
     *
     * @param rendererChangeInfo Contains the renderer state information.
     */
    virtual void OnRendererStateChange(
        const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) = 0;
};

class AudioQueryAllowedPlaybackCallback {
    public:
        virtual ~AudioQueryAllowedPlaybackCallback() = default;
        virtual bool OnQueryAllowedPlayback(int32_t uid, int32_t pid) = 0;
};

class AudioManagerAudioSceneChangedCallback {
public:
    virtual ~AudioManagerAudioSceneChangedCallback() = default;
    /**
     * Called when AudioScene changed.
     *
     * @param AudioScene audio scene
     * @since 16
     */
    virtual void OnAudioSceneChange(const AudioScene audioScene) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_INTERFACE_H

