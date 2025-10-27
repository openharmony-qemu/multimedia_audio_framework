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

#ifndef REMOTE_FAST_AUDIO_RENDER_SINK_H
#define REMOTE_FAST_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include <v1_0/iaudio_manager.h>
#include "ashmem.h"
#include "adapter/i_device_manager.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender RemoteIAudioRender;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat RemoteAudioFormat;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes RemoteAudioSampleAttributes;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor RemoteAudioDeviceDescriptor;

class RemoteFastAudioRenderSink : public IAudioRenderSink, public IDeviceManagerCallback {
public:
    explicit RemoteFastAudioRenderSink(const std::string &deviceNetworkId);
    ~RemoteFastAudioRenderSink();

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

    void DumpInfo(std::string &dumpString) override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame, uint32_t &syncInfoSize) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;

    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static RemoteAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    void InitAudioSampleAttr(RemoteAudioSampleAttributes &param);
    void InitDeviceDesc(RemoteAudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(void);

    // low latency
    int32_t PrepareMmapBuffer(void);
    int32_t CheckPositionTime(void);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 3840;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr int32_t MAX_GET_POSITION_TRY_COUNT = 10;
    static constexpr int32_t MAX_GET_POSITION_HANDLE_TIME = 10000000; // 10000000us
    static constexpr int32_t MAX_GET_POSITION_WAIT_TIME = 2000000; // 2000000us
    static constexpr int32_t INVALID_FD = -1;

    const std::string deviceNetworkId_ = "";
    IAudioSinkAttr attr_ = {};
    SinkCallbackWrapper callback_ = {};
    std::atomic<bool> sinkInited_ = false;
    std::atomic<bool> renderInited_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    float leftVolume_ = 0;
    float rightVolume_ = 0;
    uint32_t hdiRenderId_ = HDI_INVALID_ID;
    sptr<RemoteIAudioRender> audioRender_ = nullptr;

    // low latency
    int32_t bufferFd_ = INVALID_FD;
    uint32_t bufferTotalFrameSize_ = 0;
    uint32_t eachReadFrameSize_ = 0;
    uint32_t syncInfoSize_ = 0;
#ifdef DEBUG_DIRECT_USE_HDI
    sptr<Ashmem> ashmemSink_ = nullptr;
    size_t bufferSize_ = 0;
#endif
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_FAST_AUDIO_RENDER_SINK_H
