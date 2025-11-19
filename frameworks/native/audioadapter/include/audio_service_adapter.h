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

#ifndef ST_AUDIO_SERVICE_ADAPTER_H
#define ST_AUDIO_SERVICE_ADAPTER_H

#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_module_info.h"

#define NOT_SUPPORT_RET { return ERR_NOT_SUPPORTED; }
namespace OHOS {
namespace AudioStandard {
class AudioServiceAdapterCallback {
public:
    virtual void OnAudioStreamRemoved(const uint64_t sessionID) = 0;

    virtual void OnSetVolumeDbCb() = 0;

    virtual ~AudioServiceAdapterCallback() {}
};

class AudioServiceAdapter {
public:
    /**
     * @brief create audioserviceadapter instance
     *
     * @param cb callback reference for AudioServiceAdapterCallback class
     * @return Returns instance of class that extends AudioServiceAdapter
    */
    static std::shared_ptr<AudioServiceAdapter> CreateAudioAdapter(std::unique_ptr<AudioServiceAdapterCallback> cb,
        bool isAudioEngine = false);

    /**
     * @brief Connect to underlining audio server
     *
     * @return Returns true if connection is success, else return false
     * @since 1.0
     * @version 1.0
     */
    virtual bool Connect() = 0;

    /**
     * @brief Opens the audio port while loading the audio modules source and sink.
     *
     * @param audioPortName name of the audio modules to be loaded
     * @param moduleArgs audio module info like rate, channel etc
     * @return Returns module index if module loaded successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual uint32_t OpenAudioPort(std::string audioPortName, std::string moduleArgs) = 0;
    virtual int32_t OpenAudioPort(std::string audioPortName, const AudioModuleInfo& audioModuleInfo) = 0;

    /**
     * @brief Reload the audio port while loading the audio modules sink.
     *
     * @param audioPortName name of the audio modules to be loaded
     * @param audioModuleInfo audio module info like rate, channel etc
     * @return Returns module index if module loaded successfully; returns an error code
     * defined in {@link audio_errors .h} otherwise.
     */
    virtual int32_t ReloadAudioPort(const std::string &audioPortName, const AudioModuleInfo& audioModuleInfo) = 0;

    /**
     * @brief closes/unloads the audio modules loaded.
     *
     * @param audioHandleIndex the index of the loaded audio module
     * @return Returns {@link SUCCESS} if module/port is closed successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t CloseAudioPort(int32_t audioHandleIndex) = 0;

    /**
     * @brief sets default audio sink.
     *
     * @param name name of default audio sink to be set
     * @return Returns {@link SUCCESS} if default audio sink is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetDefaultSink(std::string name) = 0;

    /**
     * @brief sets default audio source.
     *
     * @param name name of default audio source to be set
     * @return Returns {@link SUCCESS} if default audio source is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetDefaultSource(std::string name) = 0;

    /**
     * @brief sets all sink-input connect to one default dink
     *
     * @param name name of default audio sink to be set
     * @return Returns {@link SUCCESS} if default audio sink is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetLocalDefaultSink(std::string name) = 0;

    /**
     * @brief get sinks by adapter name
     *
     * @param adapterName name of default audio sink to be set
     * @return Returns sink ids.
     */
    virtual std::vector<uint32_t> GetTargetSinks(std::string adapterName) = 0;

    /**
     * @brief get all sinks
     *
     * @return Returns sink infos.
     */
    virtual std::vector<SinkInfo> GetAllSinks() = 0;

    /**
     * @brief set mute for give output streamType
     *
     * @param streamType the output streamType for which mute will be set, streamType defined in{@link audio_info.h}
     * @param mute boolean value, true: Set mute; false: Set unmute
     * @return Returns {@link SUCCESS} if mute/unmute is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetSourceOutputMute(int32_t uid, bool setMute) = 0;

    /**
     * @brief suspends the current active device
     *
     * @param audioPortName Name of the default audio sink to be suspended
     * @return Returns {@link SUCCESS} if suspend is success; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SuspendAudioDevice(std::string &audioPortName, bool isSuspend) = 0;

    /**
     * @brief mute the device or unmute
     *
     * @param sinkName Name of the audio sink
     * @return Returns {@link true} if mute is success; returns false otherwise.
     */
    virtual bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) = 0;

    /**
     * @brief returns the list of all sink inputs
     *
     * @return Returns : List of all sink inputs
     */
    virtual std::vector<SinkInput> GetAllSinkInputs() = 0;

    /**
     * @brief returns the list of all source outputs
     *
     * @return Returns : List of all source outputs
     */
    virtual std::vector<SourceOutput> GetAllSourceOutputs() = 0;

    /**
     * @brief Disconnects the connected audio server
     *
     * @return void
     */
    virtual void Disconnect() = 0;

    /**
     * @brief Move one stream to target source.
     *
     * @return int32_t the result.
     */
    virtual int32_t MoveSourceOutputByIndexOrName(uint32_t sourceOutputId,
        uint32_t sourceIndex, std::string sourceName) = 0;

    /**
     * @brief Move one stream to target sink.
     *
     * @return int32_t the result.
     */
    virtual int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) = 0;

    /**
     * @brief Get current effect property.
     *
     * @return int32_t the result.
     */
    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) = 0;

    /**
     * @brief Get current effect property.
     *
     * @return int32_t the result.
     */
    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) = 0;

    /**
     * @brief Get current enhance property.
     *
     * @return int32_t the result.
     */
    virtual int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    /**
     * @brief Get current enhance property.
     *
     * @return int32_t the result.
     */
    virtual int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    /**
     * @brief Set collaborative service enabled
     *
     * @return int32_t the result, only supports proaudio for now.
     */
    virtual int32_t UpdateCollaborativeState(bool isCollaborationEnabled) NOT_SUPPORT_RET

    /**
     * @brief Set SetAbsVolumeStateToEffect service enabled
     *
     * @return int32_t the result, only supports proaudio for now.
     */
    virtual int32_t SetAbsVolumeStateToEffect(const bool absVolumeState) NOT_SUPPORT_RET

    /**
     * @brief Set SetSystemVolumeToEffect service enabled
     *
     * @return int32_t the result, only supports proaudio for now.
     */
    virtual int32_t SetSystemVolumeToEffect(AudioStreamType streamType, float volume) NOT_SUPPORT_RET

    virtual void AddCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType) = 0;
    virtual void RemoveCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType) = 0;
    virtual void UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo) = 0;
    /**
     * @brief Check is channelLayout support for multichannel render manager
     *
     * @return {@link true} if support, {@link false} otherwise
     */
    virtual bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout) NOT_SUPPORT_RET

    virtual ~AudioServiceAdapter();
};
} // namespace AudioStandard
} // namespace OHOS
#endif  // ST_AUDIO_SERVICE_ADAPTER_H
