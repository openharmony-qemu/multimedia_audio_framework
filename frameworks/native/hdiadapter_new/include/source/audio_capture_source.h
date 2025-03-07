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

#ifndef AUDIO_CAPTURE_SOURCE_H
#define AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include <thread>
#include "v4_0/iaudio_manager.h"
#include "audio_utils.h"
#include "util/audio_running_lock.h"
#include "util/ring_buffer_handler.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
class AudioCaptureSource : public IAudioCaptureSource {
public:
    explicit AudioCaptureSource(const uint32_t captureId, const std::string &halName = "primary");
    ~AudioCaptureSource();

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
    int32_t UpdateSourceType(SourceType sourceType) override;
    void RegistCallback(uint32_t type, IAudioSourceCallback *callback) override;
    void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback) override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void SetAddress(const std::string &address) override;

    void DumpInfo(std::string &dumpString) override;

private:
    static AudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static uint64_t GetChannelLayoutByChannelCount(uint32_t channelCount);
    static enum AudioInputType ConvertToHDIAudioInputType(int32_t sourceType);
    static AudioSampleFormat ParseAudioFormat(const std::string &format);
    static AudioCategory GetAudioCategory(AudioScene audioScene);
    static int32_t GetByteSizeByFormat(AudioSampleFormat format);
    static bool IsFormalSourceType(int32_t sourceType);
    uint32_t GetUniqueId(void) const;
    uint32_t GetUniqueIdBySourceType(void) const;
    void InitEcOrMicRefAttr(const IAudioSourceAttr &attr);
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    void InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc);
    void InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene);
    void SetAudioRouteInfoForEnhanceChain(void);
    int32_t CreateCapture(void);
    int32_t DoSetInputRoute(DeviceType inputDevice);
    int32_t InitCapture(void);
    void InitLatencyMeasurement(void);
    void DeInitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *frame, size_t replyBytes);
    void CheckUpdateState(char *frame, size_t replyBytes);
    bool IsNonblockingSource(const std::string &adapterName);
    int32_t NonblockingStart(void);
    int32_t NonblockingStop(void);
    int32_t NonblockingCaptureFrameWithEc(FrameDesc *fdescEc, uint64_t &replyBytesEc);
    void CaptureFrameOnlyEc(std::vector<uint8_t> &ecData);
    void CaptureThreadLoop(void);
    int32_t UpdateActiveDeviceWithoutLock(DeviceType inputDevice);
    int32_t DoStop(void);
    void DumpData(char *frame, uint64_t &replyBytes);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_CAPTURE_PERIOD_SIZE = 4096;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;
    static constexpr float MAX_VOLUME_LEVEL = 15.0f;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t AUDIO_BUFFER_SIZE = 16 * 1024;
    static constexpr uint32_t USB_DEFAULT_BUFFER_SIZE = 3840;
    static constexpr uint32_t FRAME_TIME_LEN_MS = 20; // 20ms
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioPrimaryCapture";
    static constexpr const char *RUNNING_LOCK_NAME_WAKEUP = "AudioWakeupCapture";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    uint32_t captureId_ = HDI_INVALID_ID;
    const std::string halName_ = "";
    IAudioSourceAttr attr_ = {};
    std::mutex callbackMutex_;
    SourceCallbackWrapper callback_ = {};
    bool sourceInited_ = false;
    bool captureInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    float leftVolume_ = MAX_VOLUME_LEVEL;
    float rightVolume_ = MAX_VOLUME_LEVEL;
    std::mutex statusMutex_;
    uint32_t openMic_ = 0;
    uint32_t hdiCaptureId_ = 0;
    std::string adapterNameCase_ = "";
    struct IAudioCapture *audioCapture_ = nullptr;
    // for signal detect
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    bool signalDetected_ = false;
    std::mutex signalDetectMutex_;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int captureFrameNum_ = 0;
    // for dfx log
    int32_t logMode_ = 0;
    std::string logUtilsTag_ = "";
    mutable int64_t volumeDataCount_ = 0;
    // for ec and mic_ref
    std::unique_ptr<std::thread> captureThread_ = nullptr;
    bool isCaptureThreadRunning_ = false;
    std::shared_ptr<RingBufferHandler> ringBufferHandler_ = nullptr;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    DeviceType currentActiveDevice_ = DEVICE_TYPE_INVALID;
    AudioScene currentAudioScene_ = AUDIO_SCENE_INVALID;
    std::atomic<bool> muteState_ = false;
    std::string address_ = "";
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_CAPTURE_SOURCE_H
