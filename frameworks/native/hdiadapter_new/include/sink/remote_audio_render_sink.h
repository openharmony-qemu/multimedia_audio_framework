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

#ifndef REMOTE_AUDIO_RENDER_SINK_H
#define REMOTE_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <v1_0/iaudio_manager.h>
#include <thread>
#include <shared_mutex>
#include "adapter/i_device_manager.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCategory RemoteAudioCategory;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender RemoteIAudioRender;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat RemoteAudioFormat;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes RemoteAudioSampleAttributes;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor RemoteAudioDeviceDescriptor;

class RemoteAudioRenderSink : public IAudioRenderSink, public IDeviceManagerCallback {
public:
    struct RenderWrapper {
        uint32_t hdiRenderId_ = 0;
        sptr<RemoteIAudioRender> audioRender_ = nullptr;
        FILE *dumpFile_ = nullptr;
        std::string dumpFileName_ = "";
    };

public:
    explicit RemoteAudioRenderSink(const std::string &deviceNetworkId);
    ~RemoteAudioRenderSink();

    int32_t Init(const IAudioSinkAttr &attr) override;
    void DeInit(void) override;
    bool IsInited(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Resume(void) override;
    int32_t Pause(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int64_t GetVolumeDataCount() override;

    int32_t SuspendRenderSink(void) override;
    int32_t RestoreRenderSink(void) override;

    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;

    int32_t GetLatency(uint32_t &latency) override;
    int32_t GetTransactionId(uint64_t &transactionId) override;
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude(void) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;

    int32_t SetAudioScene(AudioScene audioScene, bool scoExcludeFlag = false) override;
    int32_t GetAudioScene(void) override;

    int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) override;
    void RegistCallback(uint32_t type, IAudioSinkCallback *callback) override;
    void ResetActiveDeviceForDisconnect(DeviceType device) override;

    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower(void) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t SplitRenderFrame(char &data, uint64_t len, uint64_t &writeLen, const char *streamType) override;

    void DumpInfo(std::string &dumpString) override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    static RemoteAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static RemoteAudioCategory GetAudioCategory(AudioScene audioScene);
    void InitSplitStream(const char *splitStreamStr, std::vector<RemoteAudioCategory> &splitStreamVector);
    void InitAudioSampleAttr(RemoteAudioSampleAttributes &param, RemoteAudioCategory type);
    void InitDeviceDesc(RemoteAudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(RemoteAudioCategory type);
    int32_t DoSetOutputRoute(void);
    void CheckUpdateState(char *data, uint64_t len);
    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen, RemoteAudioCategory type);

    void JoinStartThread();

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr const char *MEDIA_STREAM_TYPE = "1";
    static constexpr const char *COMMUNICATION_STREAM_TYPE = "2";
    static constexpr const char *NAVIGATION_STREAM_TYPE = "13";
    static constexpr const char *DUMP_REMOTE_RENDER_SINK_FILENAME = "dump_remote_audiosink";
    static const std::unordered_map<std::string, RemoteAudioCategory> SPLIT_STREAM_MAP;

    const std::string deviceNetworkId_ = "";
    IAudioSinkAttr attr_ = {};
    SinkCallbackWrapper callback_ = {};
    std::atomic<bool> sinkInited_ = false;
    std::atomic<bool> renderInited_ = false;
    std::atomic<bool> isThreadRunning_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;

    std::shared_ptr<std::thread> startThread_ = nullptr;

    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    std::mutex sinkMutex_;
    std::shared_mutex renderWrapperMutex_;
    std::mutex createRenderMutex_;
    std::mutex threadMutex_;
    std::unordered_map<RemoteAudioCategory, struct RenderWrapper> audioRenderWrapperMap_;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
    // for dfx log
    std::string logUtilsTag_ = "RemoteSink";
    mutable int64_t volumeDataCount_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_AUDIO_RENDER_SINK_H
