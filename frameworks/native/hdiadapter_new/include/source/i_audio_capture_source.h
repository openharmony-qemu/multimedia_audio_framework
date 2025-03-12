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

#ifndef I_AUDIO_CAPTURE_SOURCE_H
#define I_AUDIO_CAPTURE_SOURCE_H

#include <iostream>
#include <string>
#include "audio_info.h"
#include "audio_errors.h"
#include "common/hdi_adapter_info.h"

#define SUCCESS_RET { return SUCCESS; }
#define NOT_SUPPORT_RET { return ERR_NOT_SUPPORTED; }

namespace OHOS {
namespace AudioStandard {
typedef struct IAudioSourceAttr {
    const char *adapterName = "";
    uint32_t openMicSpeaker = 0;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    float volume = 0.0f;
    uint32_t bufferSize = 0;
    bool isBigEndian = false;
    const char *filePath = nullptr;
    const char *deviceNetworkId = nullptr;
    int32_t deviceType = 0;
    int32_t sourceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    bool hasEcConfig = false;
    AudioSampleFormat formatEc = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRateEc = 0;
    uint32_t channelEc = 0;
} IAudioSourceAttr;

typedef struct FrameDesc {
    char *frame;
    uint64_t frameLen;
} FrameDesc;

class IAudioSourceCallback {
public:
    virtual ~IAudioSourceCallback() = default;
    virtual void OnCaptureSourceParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) {}
    virtual void OnCaptureState(bool isActive) {}
    virtual void OnWakeupClose(void) {}
};

class IAudioCaptureSource {
public:
    virtual ~IAudioCaptureSource() = default;

    virtual int32_t Init(const IAudioSourceAttr &attr) = 0;
    virtual void DeInit(void) = 0;
    virtual bool IsInited(void) = 0;

    virtual int32_t Start(void) = 0;
    virtual int32_t Stop(void) = 0;
    virtual int32_t Resume(void) = 0;
    virtual int32_t Pause(void) = 0;
    virtual int32_t Flush(void) = 0;
    virtual int32_t Reset(void) = 0;
    virtual int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) = 0;
    virtual int32_t CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
        uint64_t &replyBytesEc) = 0;

    virtual std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) = 0;

    virtual int32_t SetVolume(float left, float right) = 0;
    virtual int32_t GetVolume(float &left, float &right) = 0;
    virtual int32_t SetMute(bool isMute) = 0;
    virtual int32_t GetMute(bool &isMute) = 0;

    virtual uint64_t GetTransactionId(void) = 0;
    virtual int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) = 0;
    virtual float GetMaxAmplitude(void) = 0;

    virtual int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice) = 0;

    virtual int32_t UpdateActiveDevice(DeviceType inputDevice) = 0;
    virtual int32_t UpdateSourceType(SourceType sourceType) SUCCESS_RET
    virtual void RegistCallback(uint32_t type, IAudioSourceCallback *callback) {}
    virtual void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback) {}

    virtual int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) = 0;
    virtual int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) = 0;

    virtual void SetAddress(const std::string &address) {}

    virtual void DumpInfo(std::string &dumpString) {}

    // mmap extend function
    virtual int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) NOT_SUPPORT_RET
    virtual int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) NOT_SUPPORT_RET
};

} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_CAPTURE_SOURCE_H
