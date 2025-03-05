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

#ifndef WAKEUP_AUDIO_CAPTURE_SOURCE_H
#define WAKEUP_AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include "source/audio_capture_source.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include "v4_0/iaudio_manager.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
class WakeupBuffer {
public:
    explicit WakeupBuffer(IAudioCaptureSource *source);
    ~WakeupBuffer() = default;

    int32_t Poll(char *frame, uint64_t requestBytes, uint64_t &replyBytes, uint64_t &curWritePos);

private:
    static inline void MemcpyAndCheck(void *dest, size_t destMax, const void *src, size_t count)
    {
        CHECK_AND_RETURN_LOG(memcpy_s(dest, destMax, src, count) == EOK, "copy fail");
    }
    void Offer(const char *frame, const uint64_t bufferBytes);

private:
    static constexpr size_t MAX_BUFFER_SIZE = 32000; // 2 seconds

    size_t size_ = 0;
    std::unique_ptr<char[]> buffer_ = nullptr;
    IAudioCaptureSource *source_ = nullptr;
    std::mutex mutex_;
    uint64_t head_ = 0;
    uint64_t headNum_ = 0;
};

class WakeupAudioCaptureSource : public IAudioCaptureSource {
public:
    WakeupAudioCaptureSource(const uint32_t captureId);
    ~WakeupAudioCaptureSource() = default;

    int32_t Init(const IAudioSourceAttr &attr) override;
    void DeInit(void) override;
    bool IsInited(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Resume(void) override;
    int32_t Pause(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
        uint64_t &replyBytesEc) override;

    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;

    uint64_t GetTransactionId(void) override;
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude(void) override;

    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice) override;

    int32_t UpdateActiveDevice(DeviceType inputDevice) override;
    void RegistCallback(uint32_t type, IAudioSourceCallback *callback) override;
    void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback) override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void DumpInfo(std::string &dumpString) override;

private:
    AudioCaptureSource audioCaptureSource_;
    static inline std::unique_ptr<WakeupBuffer> wakeupBuffer_ = nullptr;
    static inline std::mutex wakeupMutex_;
    static inline int sourceInitCount_ = 0;
    static inline int startCount_ = 0;

    std::atomic<bool> sourceInited_ = false;
    std::atomic<bool> started_ = false;
    uint64_t curWritePos_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // WAKEUP_AUDIO_CAPTURE_SOURCE_H
