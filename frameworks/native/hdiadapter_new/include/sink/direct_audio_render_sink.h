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

#ifndef DIRECT_AUDIO_RENDER_SINK_H
#define DIRECT_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include <thread>
#include "v6_0/iaudio_manager.h"
#include "audio_utils.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
struct DirectHdiCallback {
    struct IAudioCallback callback_;
    std::function<void(const RenderCallbackType type)> serviceCallback_;
    void *sink_;
};

class DirectAudioRenderSink : public IAudioRenderSink {
public:
    DirectAudioRenderSink() = default;
    ~DirectAudioRenderSink();

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

    void SetSpeed(float speed) override;
    int32_t SetVolume(float left, float right) override;
    int32_t SetVolumeWithRamp(float left, float right, uint32_t durationMs) override;
    int32_t GetVolume(float &left, float &right) override;

    int32_t GetLatency(uint32_t &latency) override;
    int32_t GetTransactionId(uint64_t &transactionId) override;
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude(void) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t RegistDirectHdiCallback(std::function<void(const RenderCallbackType type)> callback) override;
    void DumpInfo(std::string &dumpString) override;

private:
    static int32_t DirectRenderCallback(struct IAudioCallback *self, enum AudioCallbackType type, int8_t *reserved,
        int8_t *cookie);
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    void InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(void);
    void StartTestThread(void);

private:
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr uint32_t TEST_CALLBACK_TIME = 20;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioDirectBackgroundPlay";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    IAudioSinkAttr attr_ = {};
    struct DirectHdiCallback hdiCallback_ = {};
    bool sinkInited_ = false;
    bool started_ = false;
    int32_t testFlag_ = 0;
    uint64_t mockPts_ = 0;
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    uint32_t hdiRenderId_ = HDI_INVALID_ID;
    struct IAudioRender *audioRender_ = nullptr;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
    // for dfx log
    std::string logUtilsTag_ = "DirectSink";

#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    std::thread testThread_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // DIRECT_AUDIO_RENDER_SINK_H
