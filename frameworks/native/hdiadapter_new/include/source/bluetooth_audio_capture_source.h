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

#ifndef BLUETOOTH_AUDIO_CAPTURE_SOURCE_H
#define BLUETOOTH_AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include <thread>
#include "audio_proxy_manager.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"
#include "capturer_clock_manager.h"

namespace OHOS {
namespace AudioStandard {
typedef struct OHOS::HDI::Audio_Bluetooth::AudioSampleAttributes BtAudioSampleAttributes;
typedef struct OHOS::HDI::Audio_Bluetooth::AudioDeviceDescriptor BtAudioDeviceDescriptor;
typedef enum OHOS::HDI::Audio_Bluetooth::AudioFormat BtAudioFormat;
typedef struct OHOS::HDI::Audio_Bluetooth::AudioCapture BtAudioCapture;

class BluetoothAudioCaptureSource : public IAudioCaptureSource {
public:
    explicit BluetoothAudioCaptureSource(const uint32_t captureId);
    ~BluetoothAudioCaptureSource();

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

    int32_t SetAudioScene(AudioScene audioScene, bool scoExcludeFlag = false) override;

    int32_t UpdateActiveDevice(DeviceType inputDevice) override;
    void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback) override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void SetInvalidState(void) override;

    void DumpInfo(std::string &dumpString) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    static BtAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    void InitAudioSampleAttr(BtAudioSampleAttributes &param);
    void InitDeviceDesc(BtAudioDeviceDescriptor &deviceDesc);
    void SetAudioRouteInfoForEnhanceChain(void);
    int32_t CreateCapture(void);
    void InitLatencyMeasurement(void);
    void DeInitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *frame, size_t replyBytes);
    void CheckUpdateState(char *frame, size_t replyBytes);
    int32_t DoStop(void);
    bool IsValidState(void);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_CAPTURE_PERIOD_SIZE = 4096;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t AUDIO_BUFFER_SIZE = 16 * 1024;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioBluetoothCapture";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    uint32_t captureId_ = HDI_INVALID_ID;
    std::string halName_ = "";
    IAudioSourceAttr attr_ = {};
    SourceCallbackWrapper callback_ = {};
    bool sourceInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    bool validState_ = true;
    float leftVolume_ = 0.0;
    float rightVolume_ = 0.0;
    std::mutex statusMutex_;
    uint32_t hdiCaptureId_ = 0;
    std::string adapterNameCase_ = "bt_hdap";
    BtAudioCapture *audioCapture_ = nullptr;
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
    std::string logUtilsTag_ = "A2dpSource";
    mutable int64_t volumeDataCount_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    DeviceType currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    bool muteState_ = false;

    std::shared_ptr<AudioSourceClock> audioSrcClock_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // BLUETOOTH_AUDIO_CAPTURE_SOURCE_H
