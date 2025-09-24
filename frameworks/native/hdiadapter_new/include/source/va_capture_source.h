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

#ifndef VA_CAPTURE_SOURCE_H
#define VA_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "util/callback_wrapper.h"
#include "util/audio_running_lock.h"
#include "capturer_clock_manager.h"

#include "audio_primary_source_clock.h"

#include "va_device_info.h"
#include "iv_a_input_stream.h"
#include "iv_a_device_controller.h"
#include "va_shared_buffer.h"
#include "va_shared_buffer_operator.h"


namespace OHOS {
namespace AudioStandard {
class VACaptureSource : public IAudioCaptureSource {
public:
    explicit VACaptureSource(const uint32_t captureId);
    ~VACaptureSource();

    int32_t Init(const IAudioSourceAttr& attr) override;

    void DeInit(void) override;
    bool IsInited(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Resume(void) override;
    int32_t Pause(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t CaptureFrame(char* frame, uint64_t requestBytes, uint64_t& replyBytes) override;
    int32_t CaptureFrameWithEc(
        FrameDesc* fdesc, uint64_t& replyBytes, FrameDesc* fdescEc, uint64_t& replyBytesEc) override;

    std::string GetAudioParameter(const AudioParamKey key, const std::string& condition) override;
    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;

    int32_t SetVolume(float left, float right)override;
    int32_t GetVolume(float &left, float &right)override;
    int32_t SetMute(bool isMute)override;
    int32_t GetMute(bool &isMute)override;

    uint64_t GetTransactionId(void) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    float GetMaxAmplitude(void) override;

    int32_t SetAudioScene(AudioScene audioScene, bool scoExcludeFlag = false) override;

    int32_t UpdateActiveDevice(DeviceType inputDevice) override;
    void RegistCallback(uint32_t type, IAudioSourceCallback* callback) override;
    void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback) override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t>& appsUid) final;

    void SetAddress(const std::string& address) override;
    void SetInvalidState(void) override;

    void DumpInfo(std::string& dumpString) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

    void CheckUpdateState(char* frame, size_t replyBytes);

private:
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    sptr<IVADeviceController> deviceController_;
    sptr<IVAInputStream> inputStream_;
    std::shared_ptr<VASharedBufferOperator> bufferOperator_;
    uint32_t captureId_ = HDI_INVALID_ID;
    IAudioSourceAttr attr_ = {};
    SourceCallbackWrapper callback_ = {};
    std::atomic<bool> sourceInited_ = false;
    std::atomic<bool> started_ = false;
    std::mutex statusMutex_;

    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int captureFrameNum_ = 0;

    int32_t logMode_ = 0;
    FILE* dumpFile_ = nullptr;
    std::string dumpFileName_ = "";

    std::shared_ptr<AudioCapturerSourceClock> audioSrcClock_ = nullptr;

    int64_t startTimestamp = 0;

    int32_t CreateCapture();
    int32_t InitOperator();
    void PrintUsageTimeDfx(int64_t useTime);

    std::shared_ptr<VAAudioStreamProperty> MakeVAStreamPropertyFromIAudioSourceAttr();
    std::shared_ptr<VAInputStreamAttribute> MakeVAStreamAttributeFromIAudioSourceAttr();
};

}  //namespace AudioStandard
}  //namespace OHOS
#endif //VA_CAPTURE_SOURCE_H