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

#ifndef I_AUDIO_RENDER_SINK_H
#define I_AUDIO_RENDER_SINK_H

#include <iostream>
#include <string>
#include <functional>
#include "audio_info.h"
#include "audio_errors.h"
#include "common/hdi_adapter_info.h"

#define SUCCESS_RET { return SUCCESS; }
#define NOT_SUPPORT_RET { return ERR_NOT_SUPPORTED; }

namespace OHOS {
namespace AudioStandard {
typedef struct IAudioSinkAttr {
    const char *adapterName = "";
    uint32_t openMicSpeaker = 0;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    float volume = 0.0f;
    const char *filePath = nullptr;
    const char *deviceNetworkId = nullptr;
    int32_t deviceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    std::string address;
    const char *aux;
} IAudioSinkAttr;

class IAudioSinkCallback {
public:
    virtual ~IAudioSinkCallback() = default;

    virtual void OnRenderSinkParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) {}
    virtual void OnRenderSinkStateChange(uint32_t uniqueId, bool started) {}
};

class IAudioRenderSink {
public:
    virtual ~IAudioRenderSink() = default;

    virtual int32_t Init(const IAudioSinkAttr &attr) = 0;
    virtual void DeInit(void) = 0;
    virtual bool IsInited(void) = 0;

    virtual int32_t Start(void) = 0;
    virtual int32_t Stop(void) = 0;
    virtual int32_t Resume(void) = 0;
    virtual int32_t Pause(void) = 0;
    virtual int32_t Flush(void) = 0;
    virtual int32_t Reset(void) = 0;
    virtual int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) = 0;

    virtual int32_t SuspendRenderSink(void) = 0;
    virtual int32_t RestoreRenderSink(void) = 0;

    virtual void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) = 0;
    virtual std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) = 0;

    virtual int32_t SetVolume(float left, float right) = 0;
    virtual int32_t GetVolume(float &left, float &right) = 0;

    virtual int32_t GetLatency(uint32_t &latency) = 0;
    virtual int32_t GetTransactionId(uint64_t &transactionId) = 0;
    virtual int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) = 0;
    virtual float GetMaxAmplitude(void) = 0;
    virtual void SetAudioMonoState(bool audioMono) = 0;
    virtual void SetAudioBalanceValue(float audioBalance) = 0;
    virtual int32_t SetSinkMuteForSwitchDevice(bool mute) SUCCESS_RET

    virtual int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) = 0;
    virtual int32_t GetAudioScene(void) = 0;

    virtual int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) = 0;
    virtual void RegistCallback(uint32_t type, IAudioSinkCallback *callback) {}
    virtual void RegistCallback(uint32_t type, std::shared_ptr<IAudioSinkCallback> callback) {}
    virtual void ResetActiveDeviceForDisconnect(DeviceType device) = 0;

    virtual int32_t SetPaPower(int32_t flag) = 0;
    virtual int32_t SetPriPaPower(void) = 0;

    virtual int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) = 0;
    virtual int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) = 0;

    virtual int32_t SetRenderEmpty(int32_t durationUs) SUCCESS_RET
    virtual void SetAddress(const std::string &address) {}

    virtual void DumpInfo(std::string &dumpString) = 0;

    // mmap extend function
    virtual int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) NOT_SUPPORT_RET
    virtual int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) NOT_SUPPORT_RET

    // offload extend function
    virtual int32_t Drain(AudioDrainType type) NOT_SUPPORT_RET
    virtual void RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> callback) {}
    virtual int32_t SetBufferSize(uint32_t sizeMs) NOT_SUPPORT_RET
    virtual int32_t LockOffloadRunningLock(void) NOT_SUPPORT_RET
    virtual int32_t UnLockOffloadRunningLock(void) NOT_SUPPORT_RET

    // remote extend function
    virtual int32_t SplitRenderFrame(char &data, uint64_t len, uint64_t &writeLen, const char *streamType) \
        NOT_SUPPORT_RET
        
    // primary extend function
    virtual int32_t SetDeviceConnectedFlag(bool flag) NOT_SUPPORT_RET
};

} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_RENDER_SINK_H
