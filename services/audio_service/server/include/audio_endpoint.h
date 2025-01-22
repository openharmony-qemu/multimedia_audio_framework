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

#include "i_audio_renderer_sink.h"
#include "i_process_status_listener.h"
#include "linear_pos_time_model.h"
#include "audio_device_descriptor.h"

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
        TYPE_INDEPENDENT,
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
        const AudioProcessConfig &clientConfig, const AudioDeviceDescriptor &deviceInfo);
    static std::string GenerateEndpointKey(AudioDeviceDescriptor &deviceInfo, int32_t endpointFlag);

    virtual std::string GetEndpointName() = 0;

    virtual EndpointType GetEndpointType() = 0;
    virtual int32_t SetVolume(AudioStreamType streamType, float volume) = 0;
    virtual int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) = 0;
    virtual std::shared_ptr<OHAudioBuffer> GetBuffer() = 0;

    virtual EndpointStatus GetStatus() = 0;

    virtual void Release() = 0;

    virtual bool ShouldInnerCap() = 0;
    virtual int32_t EnableFastInnerCap() = 0;
    virtual int32_t DisableFastInnerCap() = 0;

    virtual int32_t LinkProcessStream(IAudioProcessStream *processStream) = 0;
    virtual int32_t UnlinkProcessStream(IAudioProcessStream *processStream) = 0;

    virtual int32_t GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe) = 0;

    virtual void Dump(std::string &dumpString) = 0;

    virtual DeviceRole GetDeviceRole() = 0;
    virtual AudioDeviceDescriptor &GetDeviceInfo() = 0;
    virtual float GetMaxAmplitude() = 0;
    virtual uint32_t GetLinkedProcessCount() = 0;

    virtual ~AudioEndpoint() = default;
private:
    virtual bool Config(const AudioDeviceDescriptor &deviceInfo) = 0;
};

class AudioEndpointSeparate : public AudioEndpoint {
public:
    explicit AudioEndpointSeparate(EndpointType type, uint64_t id, AudioStreamType streamType);
    ~AudioEndpointSeparate();

    bool Config(const AudioDeviceDescriptor &deviceInfo) override;
    bool StartDevice();
    bool StopDevice();

    // when audio process start.
    int32_t OnStart(IAudioProcessStream *processStream) override;
    // when audio process pause.
    int32_t OnPause(IAudioProcessStream *processStream) override;
    // when audio process request update handle info.
    int32_t OnUpdateHandleInfo(IAudioProcessStream *processStream) override;
    int32_t LinkProcessStream(IAudioProcessStream *processStream) override;
    int32_t UnlinkProcessStream(IAudioProcessStream *processStream) override;
    int32_t GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe) override;

    void Dump(std::string &dumpString) override;

    std::string GetEndpointName() override;

    inline EndpointType GetEndpointType() override
    {
        return endpointType_;
    }

    // for inner-cap
    bool ShouldInnerCap() override;
    int32_t EnableFastInnerCap() override;
    int32_t DisableFastInnerCap() override;

    int32_t SetVolume(AudioStreamType streamType, float volume) override;

    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) override;

    std::shared_ptr<OHAudioBuffer> GetBuffer() override;

    EndpointStatus GetStatus() override;

    void Release() override;

    AudioDeviceDescriptor &GetDeviceInfo() override
    {
        return deviceInfo_;
    }

    DeviceRole GetDeviceRole() override
    {
        return deviceInfo_.deviceRole_;
    }

    float GetMaxAmplitude() override;
    uint32_t GetLinkedProcessCount() override;
private:
    int32_t PrepareDeviceBuffer(const AudioDeviceDescriptor &deviceInfo);
    int32_t GetAdapterBufferInfo(const AudioDeviceDescriptor &deviceInfo);
    void ResyncPosition();
    void InitAudiobuffer(bool resetReadWritePos);
    void ProcessData(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData);

    bool GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime);
    int32_t GetProcLastWriteDoneInfo(const std::shared_ptr<OHAudioBuffer> processBuffer, uint64_t curWriteFrame,
        uint64_t &proHandleFrame, int64_t &proHandleTime);

    bool IsAnyProcessRunning();

    std::string GetStatusStr(EndpointStatus status);

    int32_t WriteToSpecialProcBuf(const std::shared_ptr<OHAudioBuffer> &procBuf, const BufferDesc &readBuf);
    void WriteToProcessBuffers(const BufferDesc &readBuf);

private:
    static constexpr int64_t ONE_MILLISECOND_DURATION = 1000000; // 1ms
    // SamplingRate EncodingType SampleFormat Channel
    AudioDeviceDescriptor deviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo dstStreamInfo_;
    EndpointType endpointType_;
    uint64_t id_ = 0;
    AudioStreamType streamType_ = STREAM_DEFAULT;
    std::mutex listLock_;
    std::vector<IAudioProcessStream *> processList_;
    std::vector<std::shared_ptr<OHAudioBuffer>> processBufferList_;

    std::atomic<bool> isInited_ = false;
    std::shared_ptr<IMmapAudioRendererSink> fastSink_ = nullptr;
    int64_t spanDuration_ = 0; // nano second
    int64_t serverAheadReadTime_ = 0;
    int dstBufferFd_ = -1; // -1: invalid fd.
    uint32_t dstTotalSizeInframe_ = 0;
    uint32_t dstSpanSizeInframe_ = 0;
    uint32_t dstByteSizePerFrame_ = 0;
    std::shared_ptr<OHAudioBuffer> dstAudioBuffer_ = nullptr;
    std::atomic<EndpointStatus> endpointStatus_ = INVALID;

    std::mutex loopThreadLock_;
    std::condition_variable workThreadCV_;

    bool isDeviceRunningInIdel_ = true; // will call start sink when linked.
    bool needResyncPosition_ = true;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ENDPOINT_H
