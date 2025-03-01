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

#ifndef BLUETOOTH_AUDIO_RENDER_SINK_H
#define BLUETOOTH_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include "audio_proxy_manager.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
typedef struct OHOS::HDI::Audio_Bluetooth::AudioSampleAttributes BtAudioSampleAttributes;
typedef struct OHOS::HDI::Audio_Bluetooth::AudioDeviceDescriptor BtAudioDeviceDescriptor;
typedef enum OHOS::HDI::Audio_Bluetooth::AudioFormat BtAudioFormat;
typedef struct OHOS::HDI::Audio_Bluetooth::AudioRender BtAudioRender;

class BluetoothAudioRenderSink : public IAudioRenderSink {
public:
    explicit BluetoothAudioRenderSink(bool isBluetoothLowLatency = false);
    ~BluetoothAudioRenderSink();

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
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;

    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;
    int32_t GetAudioScene(void) override;

    int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) override;
    void RegistCallback(uint32_t type, IAudioSinkCallback *callback) override;
    void ResetActiveDeviceForDisconnect(DeviceType device) override;

    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower(void) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void DumpInfo(std::string &dumpString) override;

private:
    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;

    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static BtAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    void InitAudioSampleAttr(BtAudioSampleAttributes &param);
    void InitDeviceDesc(BtAudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(void);
    int32_t InitRender(void);
    void InitLatencyMeasurement(void);
    void DeInitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *data, size_t len);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    void CheckUpdateState(char *data, uint64_t len);
    int32_t DoRenderFrame(char &data, uint64_t len, uint64_t &writeLen);
    void UpdateSinkState(bool started);

    // low latency
    int32_t PrepareMmapBuffer(void);
    int32_t CheckPositionTime(void);
    int32_t CheckBluetoothScenario(void);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
    static constexpr uint32_t WAIT_TIME_FOR_RETRY_IN_MICROSECOND = 50000;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr int32_t MAX_GET_POSITION_TRY_COUNT = 50;
    static constexpr int32_t MAX_GET_POSITION_HANDLE_TIME = 10000000; // 10000000us
    static constexpr int32_t MAX_GET_POSITION_WAIT_TIME = 2000000; // 2000000us
    static constexpr int32_t INVALID_FD = -1;
    static constexpr int64_t STAMP_THRESHOLD_MS = 20;
    static constexpr int32_t RENDER_FRAME_NUM = -4;
    static constexpr uint32_t RENDER_FRAME_INTERVAL_IN_MICROSECONDS = 10000;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioBluetoothBackgroundPlay";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    bool isBluetoothLowLatency_ = false;
    IAudioSinkAttr attr_ = {};
    SinkCallbackWrapper callback_ = {};
    bool sinkInited_ = false;
    int32_t sinkInitCount_ = 0;
    bool started_ = false;
    bool paused_ = false;
    std::atomic<bool> suspend_ = false;
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    uint32_t hdiRenderId_ = 0;
    BtAudioRender *audioRender_ = nullptr;
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
    // for signal detect
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    bool signalDetected_ = false;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
    // for device switch
    std::mutex switchDeviceMutex_;
    int32_t muteCount_ = 0;
    std::atomic<bool> switchDeviceMute_ = false;
    // for dfx log
    int32_t logMode_ = 0;
    std::string logUtilsTag_ = "";
    mutable int64_t volumeDataCount_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    AudioSampleFormat audioSampleFormat_ = SAMPLE_S16LE;
    std::mutex sinkMutex_;

    // low latency
    int32_t bufferFd_ = INVALID_FD;
    uint32_t bufferTotalFrameSize_ = 0;
    uint32_t eachReadFrameSize_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // BLUETOOTH_AUDIO_RENDER_SINK_H
