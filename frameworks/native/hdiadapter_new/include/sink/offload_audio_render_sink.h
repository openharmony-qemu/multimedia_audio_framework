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

#ifndef OFFLOAD_AUDIO_RENDER_SINK_H
#define OFFLOAD_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include "v5_0/iaudio_manager.h"
#include "audio_utils.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
struct OffloadHdiCallback {
    struct IAudioCallback callback_;
    std::function<void(const RenderCallbackType type)> serviceCallback_;
    void *sink_;
};

class OffloadAudioRenderSink : public IAudioRenderSink {
public:
    OffloadAudioRenderSink() = default;
    ~OffloadAudioRenderSink();

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
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;
    void SetSpeed(float speed) override;

    int32_t SetAudioScene(AudioScene audioScene, bool scoExcludeFlag = false) override;
    int32_t GetAudioScene(void) override;

    int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) override;
    void RegistCallback(uint32_t type, IAudioSinkCallback *callback) override;
    void ResetActiveDeviceForDisconnect(DeviceType device) override;

    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower(void) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t Drain(AudioDrainType type) override;
    void RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> callback) override;
    int32_t SetBufferSize(uint32_t sizeMs) override;
    int32_t SetOffloadRenderCallbackType(RenderCallbackType type) override;
    int32_t LockOffloadRunningLock(void) override;
    int32_t UnLockOffloadRunningLock(void) override;

    void DumpInfo(std::string &dumpString) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static AudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static int32_t OffloadRenderCallback(struct IAudioCallback *self, enum AudioCallbackType type, int8_t *reserved,
        int8_t *cookie);
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    void InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(void);
    void InitLatencyMeasurement(void);
    void DeInitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *data, size_t len);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    void CheckUpdateState(char *data, uint64_t len);
    int32_t SetVolumeInner(float left, float right);
    void UpdateSinkState(bool started);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr size_t OFFLOAD_DFX_SPLIT = 2;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioOffloadBackgroundPlay";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif
    static constexpr uint32_t AUDIO_SPEED_BASE = 1000;

    IAudioSinkAttr attr_ = {};
    SinkCallbackWrapper callback_ = {};
    struct OffloadHdiCallback hdiCallback_ = {};
    bool sinkInited_ = false;
    bool started_ = false;
    bool isFlushing_ = false;
    bool isNeedRestart_ = false;
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    uint32_t hdiRenderId_ = 0;
    struct IAudioRender *audioRender_ = nullptr;
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
    // for signal detect
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    bool signalDetected_ = false;
    size_t signalDetectedTime_ = 0;
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
    std::string logUtilsTag_ = "OffloadSink";
    mutable int64_t volumeDataCount_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
    bool runningLocked_ = false;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    std::atomic<uint64_t> renderPos_ = 0;
    std::mutex sinkMutex_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // OFFLOAD_AUDIO_RENDER_SINK_H
