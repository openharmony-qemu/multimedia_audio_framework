/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_info.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
class AudioServiceAdapterCallback {
public:
    /**
     * @brief computes the volume to be set in audioserver
     *
     * @param streamType streamType for which volume will be computed
     * @return Returns volume level in float
     */
    virtual std::pair<float, int32_t> OnGetVolumeDbCb(AudioStreamType streamType) = 0;

    virtual void OnAudioStreamRemoved(const uint64_t sessionID) = 0;

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
    static std::unique_ptr<AudioServiceAdapter> CreateAudioAdapter(std::unique_ptr<AudioServiceAdapterCallback> cb);

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
     * @brief sets audio volume db
     *
     * @param streamType the streamType for which volume will be set, streamType defined in{@link audio_info.h}
     * @param volume the volume level to be set
     * @return Returns {@link SUCCESS} if volume is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetVolumeDb(AudioStreamType streamType, float volume) = 0;

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

    virtual ~AudioServiceAdapter();
};
} // namespace AudioStandard
} // namespace OHOS
#endif  // ST_AUDIO_SERVICE_ADAPTER_H
