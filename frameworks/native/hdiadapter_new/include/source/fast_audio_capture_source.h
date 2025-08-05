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

#ifndef FAST_AUDIO_CAPTURE_SOURCE_H
#define FAST_AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>
#include "v5_0/iaudio_manager.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
class FastAudioCaptureSource : public IAudioCaptureSource {
public:
    FastAudioCaptureSource() = default;
    ~FastAudioCaptureSource();

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

    void DumpInfo(std::string &dumpString) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame, uint32_t &syncInfoSize) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;

    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static AudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static enum AudioInputType ConvertToHDIAudioInputType(int32_t sourceType);
    static AudioCategory GetAudioCategory(AudioScene audioScene);
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    void InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc);
    void InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene);
    int32_t CreateCapture(void);
    int32_t DoSetInputRoute(DeviceType inputDevice);

    // low latency
    int32_t PrepareMmapBuffer(void);
    int32_t CheckPositionTime(void);

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr int32_t MAX_GET_POSITION_TRY_COUNT = 50;
    static constexpr int64_t GENERAL_MAX_GET_POSITION_HANDLE_TIME = 10000000; // 10ms = 10ns * 1000 * 1000
    static constexpr int64_t VOIP_MAX_GET_POSITION_HANDLE_TIME = 20000000; // 20ms = 20ns * 1000 * 1000
    static constexpr int32_t MAX_GET_POSITION_WAIT_TIME = 2000000; // 2000000us
    static constexpr int32_t INVALID_FD = -1;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioFastCapture";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    IAudioSourceAttr attr_ = {};
    SourceCallbackWrapper callback_ = {};
    bool sourceInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    std::mutex statusMutex_;
    uint32_t hdiCaptureId_ = 0;
    struct IAudioCapture *audioCapture_ = nullptr;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
#endif
    AudioScene currentAudioScene_ = AUDIO_SCENE_INVALID;
    std::atomic<bool> isCheckPositionSuccess_ = true;

    // low latency
    int32_t bufferFd_ = INVALID_FD;
    uint32_t bufferTotalFrameSize_ = 0;
    uint32_t eachReadFrameSize_ = 0;
    size_t bufferSize_ = 0;
    uint32_t syncInfoSize_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // FAST_AUDIO_CAPTURE_SOURCE_H
