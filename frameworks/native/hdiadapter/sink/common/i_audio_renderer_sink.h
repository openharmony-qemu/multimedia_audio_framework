/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef I_AUDIO_RENDERER_SINK_H
#define I_AUDIO_RENDERER_SINK_H

#include <string>
#include "audio_info.h"
#include "audio_hdiadapter_info.h"

namespace OHOS {
namespace AudioStandard {

typedef struct IAudioSinkAttr {
    const char *adapterName = NULL;
    uint32_t openMicSpeaker = 0;
    HdiAdapterFormat format = HdiAdapterFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    float volume = 0.0f;
    const char *filePath = NULL;
    const char *deviceNetworkId = NULL;
    int32_t deviceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    std::string address;
} IAudioSinkAttr;

class IAudioSinkCallback {
public:
    virtual void OnAudioSinkParamChange(const std::string &netWorkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) = 0;
};

typedef void OnRenderCallback(const RenderCallbackType type, int8_t *userdata);
class IAudioRendererSink {
public:
    static IAudioRendererSink *GetInstance(const char *devceClass, const char *deviceNetworkId);

    virtual ~IAudioRendererSink() = default;

    virtual int32_t Init(const IAudioSinkAttr &attr) = 0;
    virtual bool IsInited(void) = 0;
    virtual void DeInit(void) = 0;

    virtual int32_t Flush(void) = 0;
    virtual int32_t Pause(void) = 0;
    virtual int32_t Reset(void) = 0;
    virtual int32_t Resume(void) = 0;
    virtual int32_t Start(void) = 0;
    virtual int32_t Stop(void) = 0;

    virtual int32_t SuspendRenderSink(void) = 0;
    virtual int32_t RestoreRenderSink(void) = 0;

    virtual int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) = 0;
    virtual int32_t GetLatency(uint32_t *latency) = 0;

    virtual int32_t SetVolume(float left, float right) = 0;
    virtual int32_t GetVolume(float &left, float &right) = 0;
    virtual int32_t SetVoiceVolume(float volume) = 0;

    virtual int32_t GetTransactionId(uint64_t *transactionId) = 0;

    virtual int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) = 0;
    virtual int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) = 0;

    virtual void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) = 0;
    virtual std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) = 0;
    virtual void RegisterParameterCallback(IAudioSinkCallback* callback) = 0;

    virtual void SetAudioMonoState(bool audioMono) = 0;
    virtual void SetAudioBalanceValue(float audioBalance) = 0;

    virtual int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) = 0;
    virtual float GetMaxAmplitude() = 0;

    virtual int32_t Preload(const std::string &usbInfoStr)
    {
        return 0;
    }

    virtual void ResetOutputRouteForDisconnect(DeviceType device) = 0;
    virtual int32_t SetPaPower(int32_t flag) = 0;
    virtual int32_t SetPriPaPower() = 0;

    virtual int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS],
        const size_t size) = 0;

    virtual int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) = 0;

    virtual int32_t SetRenderEmpty(int32_t durationUs)
    {
        // Only operate on primary for now
        return 0;
    }

    virtual int32_t SetSinkMuteForSwitchDevice(bool mute)
    {
        // Only operate on offload for now
        return 0;
    }
};

class IMmapAudioRendererSink : public IAudioRendererSink {
public:
    IMmapAudioRendererSink() = default;
    virtual ~IMmapAudioRendererSink() = default;
    virtual int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) = 0;
    virtual int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) = 0;
};

enum AudioDrainType {
    AUDIO_DRAIN_EARLY_NOTIFY,
    AUDIO_DRAIN_ALL
};

class IOffloadAudioRendererSink : public IAudioRendererSink {
public:
    IOffloadAudioRendererSink() = default;
    virtual ~IOffloadAudioRendererSink() = default;
    virtual int32_t RegisterRenderCallback(OnRenderCallback (*callback), int8_t *userdata) = 0;
    virtual int32_t Drain(AudioDrainType type) = 0;
    virtual int32_t SetBufferSize(uint32_t sizeMs) = 0;

    virtual int32_t OffloadRunningLockInit(void) = 0;
    virtual int32_t OffloadRunningLockLock(void) = 0;
    virtual int32_t OffloadRunningLockUnlock(void) = 0;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // I_AUDIO_RENDERER_SINK_H
