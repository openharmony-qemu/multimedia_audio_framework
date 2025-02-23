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

#ifndef REMOTE_AUDIO_CAPTURE_SOURCE_H
#define REMOTE_AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>
#include <v1_0/iaudio_manager.h>
#include "adapter/i_device_manager.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCategory RemoteAudioCategory;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture RemoteIAudioCapture;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat RemoteAudioFormat;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes RemoteAudioSampleAttributes;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor RemoteAudioDeviceDescriptor;

class RemoteAudioCaptureSource : public IAudioCaptureSource, public IDeviceManagerCallback {
public:
    RemoteAudioCaptureSource(const std::string &deviceNetworkId);
    ~RemoteAudioCaptureSource();

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

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void DumpInfo(std::string &dumpString) override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;

private:
    static RemoteAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static RemoteAudioCategory GetAudioCategory(AudioScene audioScene);
    void InitAudioSampleAttr(RemoteAudioSampleAttributes &param);
    void InitDeviceDesc(RemoteAudioDeviceDescriptor &deviceDesc);
    int32_t CreateCapture(void);
    void DestroyCapture(void);
    void CheckUpdateState(char *frame, size_t replyBytes);

private:
    static constexpr uint32_t DEEP_BUFFER_CAPTURE_PERIOD_SIZE = 4096;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t AUDIO_BUFFER_SIZE = 16 * 1024;
    static constexpr const char *DUMP_REMOTE_CAPTURE_SOURCE_FILENAME = "dump_remote_audiosource.pcm";

    const std::string deviceNetworkId_ = "";
    IAudioSourceAttr attr_ = {};
    SourceCallbackWrapper callback_ = {};
    std::atomic<bool> sourceInited_ = false;
    std::atomic<bool> captureInited_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    uint32_t hdiCaptureId_ = 0;
    std::mutex createCaptureMutex_;
    sptr<RemoteIAudioCapture> audioCapture_ = nullptr;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int captureFrameNum_ = 0;
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    bool muteState_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_AUDIO_CAPTURE_SOURCE_H
