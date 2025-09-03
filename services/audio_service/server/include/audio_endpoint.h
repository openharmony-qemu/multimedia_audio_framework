/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_ENDPOINT_H
#define AUDIO_ENDPOINT_H

#include <sstream>
#include <memory>
#include <thread>

#include "common/hdi_adapter_info.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "i_process_status_listener.h"
#include "linear_pos_time_model.h"
#include "audio_device_descriptor.h"
#include "i_stream_manager.h"
#include "i_renderer_stream.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
// When AudioEndpoint is offline, notify the owner.
class IAudioEndpointStatusListener {
public:
    enum HdiDeviceStatus : uint32_t {
        STATUS_ONLINE = 0,
        STATUS_OFFLINE,
        STATUS_INVALID,
    };

    /**
     * When AudioEndpoint changed status, we need to notify AudioProcessStream.
    */
    virtual int32_t OnEndpointStatusChange(HdiDeviceStatus status) = 0;
};

class AudioEndpoint : public IProcessStatusListener {
public:
    static constexpr int32_t MAX_LINKED_PROCESS = 6; // 6
    enum EndpointType : uint32_t {
        TYPE_MMAP = 0,
        TYPE_INVALID,
        TYPE_VOIP_MMAP
    };

    enum EndpointStatus : uint32_t {
        INVALID = 0,
        UNLINKED, // no process linked
        IDEL,     // no running process
        STARTING, // calling start sink
        RUNNING,  // at least one process is running
        STOPPING, // calling stop sink
        STOPPED   // sink stoped
    };

    static std::shared_ptr<AudioEndpoint> CreateEndpoint(EndpointType type, uint64_t id,
        const AudioProcessConfig &clientConfig, const AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo);
    static std::string GenerateEndpointKey(AudioDeviceDescriptor &deviceInfo, int32_t endpointFlag);

    virtual std::string GetEndpointName() = 0;

    virtual EndpointType GetEndpointType() = 0;
    virtual int32_t SetVolume(AudioStreamType streamType, float volume) = 0;

    virtual std::shared_ptr<OHAudioBufferBase> GetBuffer() = 0;

    virtual EndpointStatus GetStatus() = 0;

    virtual void Release() = 0;

    virtual bool ShouldInnerCap(int32_t innerCapId) = 0;
    virtual int32_t EnableFastInnerCap(int32_t innerCapId,
        const std::optional<std::string> &dualDeviceName = std::nullopt) = 0;
    virtual int32_t DisableFastInnerCap() = 0;
    virtual int32_t DisableFastInnerCap(int32_t innerCapId) = 0;

    virtual int32_t LinkProcessStream(IAudioProcessStream *processStream, bool startWhenLinking = true) = 0;
    virtual int32_t UnlinkProcessStream(IAudioProcessStream *processStream) = 0;

    virtual int32_t GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe) = 0;

    virtual void Dump(std::string &dumpString) = 0;

    virtual DeviceRole GetDeviceRole();
    virtual AudioDeviceDescriptor &GetDeviceInfo();
    virtual AudioStreamInfo &GetAudioStreamInfo();
    virtual float GetMaxAmplitude() = 0;
    virtual uint32_t GetLinkedProcessCount() = 0;

    virtual AudioMode GetAudioMode() const = 0;

    virtual int32_t AddCaptureInjector(const uint32_t &sinkPortIndex, const SourceType &sourceType) = 0;
    virtual int32_t RemoveCaptureInjector(const uint32_t &sinkPortIndex, const SourceType &sourceType) = 0;

    virtual ~AudioEndpoint() = default;

protected:
    // SamplingRate EncodingType SampleFormat Channel
    AudioStreamInfo dstStreamInfo_;
    AudioDeviceDescriptor deviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);

private:
    virtual bool Config(const AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo,
                        AudioStreamType streamType) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ENDPOINT_H
