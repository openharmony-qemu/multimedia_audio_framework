/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef I_AUDIO_PROCESS_STREAM_H
#define I_AUDIO_PROCESS_STREAM_H

#include <memory>

#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {

struct AudioCaptureDataProcParams {
    AudioCaptureDataProcParams(
        const BufferDesc &readBuf,
        std::vector<uint8_t> &captureConvBuffer,
        std::vector<uint8_t> &rendererConvBuffer
    ) : readBuf_(readBuf),
        captureConvBuffer_(captureConvBuffer),
        rendererConvBuffer_(rendererConvBuffer)
    {
    }

    bool isConvertReadFormat_ = false;
    AudioSamplingRate srcSamplingRate = SAMPLE_RATE_48000;

    const BufferDesc &readBuf_;
    std::vector<uint8_t> &captureConvBuffer_;
    std::vector<uint8_t> &rendererConvBuffer_;
};

class IAudioProcessStream {
public:
    /**
     * Get buffer of client for AudioEndpoint.
    */
    virtual std::shared_ptr<OHAudioBufferBase> GetStreamBuffer() = 0;

    virtual AudioStreamInfo GetStreamInfo() = 0;

    virtual uint32_t GetAudioSessionId() = 0;

    virtual AudioStreamType GetAudioStreamType() = 0;

    virtual void SetInnerCapState(bool isInnerCapped, int32_t innerCapId) = 0;
    virtual bool GetInnerCapState(int32_t innerCapId) = 0;
    virtual std::unordered_map<int32_t, bool> GetInnerCapState() = 0;

    virtual AppInfo GetAppInfo() = 0;

    virtual BufferDesc &GetConvertedBuffer() = 0;

    virtual bool GetMuteState() = 0;

    virtual AudioProcessConfig GetAudioProcessConfig() = 0;

    virtual void WriteDumpFile(void *buffer, size_t bufferSize) = 0;

    virtual int32_t SetDefaultOutputDevice(int32_t defaultOutputDevice, bool skipForce = false) = 0;

    virtual int32_t SetSilentModeAndMixWithOthers(bool on) = 0;

    virtual uint32_t GetSpanSizeInFrame() = 0;

    virtual uint32_t GetByteSizePerFrame() = 0;

    virtual StreamStatus GetStreamInServerStatus() = 0;

    virtual void EnableStandby();

    virtual ~IAudioProcessStream() = default;
 
    virtual std::time_t GetStartMuteTime() = 0;
    virtual void SetStartMuteTime(std::time_t time) = 0;
 
    virtual bool GetSilentState() = 0;
    virtual void SetSilentState(bool state) = 0;
    virtual void AddMuteWriteFrameCnt(int64_t muteFrameCnt) = 0;
    virtual void AddMuteFrameSize(int64_t muteFrameCnt) {}
    virtual void AddNormalFrameSize() {}
    virtual void AddNoDataFrameSize() {}
    virtual StreamStatus GetStreamStatus() {return STREAM_IDEL;}
    virtual int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) = 0;
    virtual int32_t GetAudioHapticsSyncId() = 0;
    virtual bool PrepareRingBuffer(uint64_t curRead, RingBufferWrapper& ringBuffer) = 0;
    virtual void PrepareStreamDataBuffer(size_t spanSizeInByte,
        RingBufferWrapper &ringBuffer, AudioStreamData &streamData) = 0;
    virtual void UpdateStreamInfo() {}

    virtual int32_t WriteToSpecialProcBuf(AudioCaptureDataProcParams &procParams)
    {
        return SUCCESS;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_PROCESS_STREAM_H
