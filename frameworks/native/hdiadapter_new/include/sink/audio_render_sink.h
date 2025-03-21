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

#ifndef AUDIO_RENDER_SINK_H
#define AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include "ctime"
#include "v4_0/iaudio_manager.h"
#include "audio_utils.h"
#include "audio_performance_monitor.h"
#include "adapter/i_device_manager.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
class AudioRenderSink : public IAudioRenderSink {
public:
    explicit AudioRenderSink(const uint32_t renderId, const std::string &halName = "primary");
    ~AudioRenderSink();

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
    int32_t SetDeviceConnectedFlag(bool flag) override;

    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;
    int32_t GetAudioScene(void) override;

    int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) override;
    void RegistCallback(uint32_t type, IAudioSinkCallback *callback) override;
    void ResetActiveDeviceForDisconnect(DeviceType device) override;

    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower(void) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void SetAddress(const std::string &address) override;

    void DumpInfo(std::string &dumpString) override;

private:
    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static AudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static int32_t ConvertByteToAudioFormat(int32_t format);
    static std::string ParseAudioFormatToStr(int32_t format);
    static AudioSampleFormat ParseAudioFormat(const std::string &format);
    static AudioCategory GetAudioCategory(AudioScene audioScene);
    AudioPortPin GetAudioPortPin(void) const noexcept;
    uint32_t GetUniqueId(void) const;
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    void InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc);
    void InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene);
    void SetAudioRouteInfoForEnhanceChain(void);
    int32_t CreateRender(void);
    int32_t DoSetOutputRoute(std::vector<DeviceType> &outputDevices);
    int32_t InitRender(void);
    void InitLatencyMeasurement(void);
    void DeInitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *data, size_t len);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    void CheckUpdateState(char *data, uint64_t len);
    std::string GetAttrInfoStr(const struct AudioSampleAttributes &attrInfo);
    int32_t UpdateDPAttr(const std::string &dpInfo);
    std::string GetDPDeviceInfo(const std::string &condition);
    std::string GetRouting(void) const;
    void WriteSmartPAStatusSysEvent(int32_t status);
    void UpdateSinkState(bool started);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr unsigned int BUFFER_CALC_20MS = 20;
    static constexpr unsigned int BUFFER_CALC_1000MS = 1000;
    static constexpr unsigned int FORMAT_1_BYTE = 1;
    static constexpr unsigned int FORMAT_2_BYTE = 2;
    static constexpr unsigned int FORMAT_3_BYTE = 3;
    static constexpr unsigned int FORMAT_4_BYTE = 4;
    static constexpr double INTERVAL = 3.0;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME_BASE = "AudioPrimaryBackgroundPlay";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    uint32_t renderId_ = HDI_INVALID_ID;
    const std::string halName_ = "";
    IAudioSinkAttr attr_ = {};
    SinkCallbackWrapper callback_ = {};
    bool sinkInited_ = false;
    bool renderInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    time_t startTime_ = time(nullptr);
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    uint32_t openSpeaker_ = 0;
    uint32_t hdiRenderId_ = 0;
    std::string adapterNameCase_ = "";
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
    int32_t logMode_ = 0;
    std::string logUtilsTag_ = "";
    mutable int64_t volumeDataCount_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    DeviceType currentActiveDevice_ = DEVICE_TYPE_NONE;
    AudioScene currentAudioScene_ = AUDIO_SCENE_INVALID;
    int32_t currentDevicesSize_ = 0;
    bool forceSetRouteFlag_ = false;
    int32_t paStatus_ = 0;
    std::string address_ = "";
    AdapterType sinkType_ = ADAPTER_TYPE_PRIMARY;
    std::mutex sinkMutex_;
    // for setdeviceconnect flag
    bool deviceConnectedFlag_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RENDER_SINK_H
