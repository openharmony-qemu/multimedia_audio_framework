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

#ifndef REMOTE_FAST_AUDIO_CAPTURE_SOURCE_H
#define REMOTE_FAST_AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <v1_0/iaudio_manager.h>
#include "ashmem.h"
#include "adapter/i_device_manager.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCategory RemoteAudioCategory;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture RemoteIAudioCapture;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat RemoteAudioFormat;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes RemoteAudioSampleAttributes;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor RemoteAudioDeviceDescriptor;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor RemoteAudioSceneDescriptor;

class RemoteFastAudioCaptureSource : public IAudioCaptureSource, public IDeviceManagerCallback {
public:
    RemoteFastAudioCaptureSource(const std::string &deviceNetworkId);
    ~RemoteFastAudioCaptureSource();

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
    void RegistCallback(uint32_t type, IAudioSourceCallback *callback) override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void DumpInfo(std::string &dumpString) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;

private:
    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame, uint32_t &syncInfoSize) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;

    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static RemoteAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static RemoteAudioCategory GetAudioCategory(AudioScene audioScene);
    inline std::string PrintAttr(void)
    {
        std::stringstream attrStr;
        attrStr << "adapterName: " << attr_.adapterName << ", openMicSpeaker: " << attr_.openMicSpeaker;
        attrStr << ", format: " << static_cast<int32_t>(attr_.format) << ", sampleRate: " << attr_.sampleRate;
        attrStr << ", channel: " << attr_.channel << ", volume: " << attr_.volume << ", filePath: " << attr_.filePath;
        attrStr << ", deviceNetworkId: " << attr_.deviceNetworkId << ", deviceType: " << attr_.deviceType;
        return attrStr.str();
    }
    void InitAudioSampleAttr(RemoteAudioSampleAttributes &param);
    void InitDeviceDesc(RemoteAudioDeviceDescriptor &deviceDesc);
    void InitSceneDesc(RemoteAudioSceneDescriptor &sceneDesc, AudioScene audioScene);
    int32_t CreateCapture(void);

    // low latency
    int32_t PrepareMmapBuffer(const RemoteAudioSampleAttributes &param);
    int32_t CheckPositionTime(void);

private:
    static constexpr uint32_t DEEP_BUFFER_CAPTURE_PERIOD_SIZE = 3840;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr int32_t MAX_GET_POSITION_TRY_COUNT = 10;
    static constexpr int32_t MAX_GET_POSITION_HANDLE_TIME = 10000000; // 10000000us
    static constexpr int32_t MAX_GET_POSITION_WAIT_TIME = 10000000; // 10ms
    static constexpr int32_t INVALID_FD = -1;
    static constexpr uint32_t CAPTURE_INTERLEAVED = 1;

    const std::string deviceNetworkId_ = "";
    IAudioSourceAttr attr_ = {};
    SourceCallbackWrapper callback_ = {};
    std::atomic<bool> sourceInited_ = false;
    std::atomic<bool> captureInited_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    float leftVolume_ = 0;
    float rightVolume_ = 0;
    uint32_t hdiCaptureId_ = 0;
    sptr<RemoteIAudioCapture> audioCapture_ = nullptr;
#ifdef DEBUG_DIRECT_USE_HDI
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "/data/local/tmp/remote_fast_audio_capture.pcm";
#endif
    std::atomic<bool> muteState_ = false;

    // low latency
    int32_t bufferFd_ = INVALID_FD;
    uint32_t bufferTotalFrameSize_ = 0;
    uint32_t eachReadFrameSize_ = 0;
    uint32_t syncInfoSize_ = 0;
#ifdef DEBUG_DIRECT_USE_HDI
    sptr<Ashmem> ashmemSource_ = nullptr;
    size_t bufferSize_ = 0;
#endif
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_FAST_AUDIO_CAPTURE_SOURCE_H
