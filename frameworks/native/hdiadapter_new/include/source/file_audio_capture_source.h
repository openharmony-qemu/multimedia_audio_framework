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

#ifndef FILE_AUDIO_CAPTURE_SOURCE_H
#define FILE_AUDIO_CAPTURE_SOURCE_H

#include "source/i_audio_capture_source.h"
#include <iostream>
#include <cstring>

namespace OHOS {
namespace AudioStandard {
class FileAudioCaptureSource : public IAudioCaptureSource {
public:
    FileAudioCaptureSource() = default;
    ~FileAudioCaptureSource();

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

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    void DumpInfo(std::string &dumpString) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    bool sourceInited_ = false;
    FILE *file_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // FILE_AUDIO_CAPTURE_SOURCE_H
