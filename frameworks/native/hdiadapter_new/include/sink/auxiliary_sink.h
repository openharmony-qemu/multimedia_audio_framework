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

#ifndef AUXILIARY_SINK_H
#define AUXILIARY_SINK_H

#include "sink/i_audio_render_sink.h"
#include "adapter/i_device_manager.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include "v6_0/iaudio_manager.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
class AuxiliarySink : public IAudioRenderSink {
public:
    AuxiliarySink() = default;
    ~AuxiliarySink();

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

    int32_t GetVolumeDataCount(int64_t &volumeData) override;

    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;

    int32_t GetLatency(uint32_t &latency) override;
    int32_t GetTransactionId(uint64_t &transactionId) override;
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude(void) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void DumpInfo(std::string &dumpString) override;

private:
    int32_t PrepareMmapBuffer(void);
    void ReleaseMmapBuffer(void);

    static AudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    static uint32_t PcmFormatToBit(AudioSampleFormat format);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 3840;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr int32_t MAX_GET_POSITION_TRY_COUNT = 50;
    static constexpr int32_t GENERAL_MAX_GET_POSITION_HANDLE_TIME = 10000000; // 10ms = 10ns * 1000 * 1000
    static constexpr int32_t VOIP_MAX_GET_POSITION_HANDLE_TIME = 20000000; // 20ms = 20ns * 1000 * 1000
    static constexpr int32_t MAX_GET_POSITION_WAIT_TIME = 2000000; // 2000000us
    static constexpr int32_t INVALID_FD = -1;
    static constexpr const char *AUXILIARY_SINK_FILENAME = "dump_auxiliary_sink";
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    std::string logTag_ = "AuxSink";
    mutable int64_t volumeDataCount_ = 0;
    std::string halName_ = "AuxSink";
    int32_t sinkId_ = HDI_INVALID_ID;
    IAudioSinkAttr attr_ = {};
    std::shared_ptr<IDeviceManager> deviceManager_ = nullptr;
    bool sinkInited_ = false;

    struct AudioMmapBufferDescriptor buffer_ = {};
    int32_t bufferFd_ = INVALID_FD;
    uint32_t frameSizeInByte_ = 4; // 16bit * 2ch / 8bit
    uint32_t totalBufferFrames_ = 1764; // 44.1K / 1K * 20ms *2
    uint32_t eachSpanFrames_ = 882; // 44.1K / 1K * 20ms
    uint32_t eachSpanFramesSize_ = 3528;
    uint32_t syncInfoSize_ = 0;
    size_t bufferSize_ = 7056;
    int32_t dupBufferFd_ = INVALID_FD;
    char *bufferAddress_ = nullptr;
    uint32_t curWritePos_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUXILIARY_SINK_H
