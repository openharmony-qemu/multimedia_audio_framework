/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_CAPTURER_FILE_SOURCE_H
#define AUDIO_CAPTURER_FILE_SOURCE_H

#include <cstdint>
#include <cstdio>
#include "i_audio_capturer_source.h"

namespace OHOS {
namespace AudioStandard {
class AudioCapturerFileSource : public IAudioCapturerSource {
public:
    AudioCapturerFileSource();
    ~AudioCapturerFileSource();
    int32_t Init(const IAudioSourceAttr &attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t CaptureFrameWithEc(
        FrameDesc *fdesc, uint64_t &replyBytes,
        FrameDesc *fdescEc, uint64_t &replyBytesEc) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;
    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice, const std::string &deviceName = "") override;
    int32_t SetInputRoute(DeviceType deviceType, const std::string &deviceName = "") override;
    uint64_t GetTransactionId() override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;
    float GetMaxAmplitude() override;
    int32_t GetCaptureId(uint32_t &captureId) const override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;
private:
    bool capturerInited_ = false;
    FILE *filePtr = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_CAPTURER_FILE_SOURCE_H
