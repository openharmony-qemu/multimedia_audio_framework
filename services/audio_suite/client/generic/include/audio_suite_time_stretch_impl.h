/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SUITE_TIME_STRETCH_IMPL_H
#define AUDIO_SUITE_TIME_STRETCH_IMPL_H

#include <cstdint>
#include <mutex>
#include <vector>
#include "audio_time_stretch_api.h"
#include "audio_suite_time_stretch.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

using CreateFunc = TimeStretchHandle (*)(uint32_t, uint32_t);
using DestroyFunc = void (*)(TimeStretchHandle);
using SetSpeedFunc = int32_t (*)(TimeStretchHandle, float);
using GetSpeedFunc = float (*)(TimeStretchHandle);
using SetPitchFunc = int32_t (*)(TimeStretchHandle, float);
using GetPitchFunc = float (*)(TimeStretchHandle);
using ChangeSpeedFloatFunc = int32_t (*)(TimeStretchHandle, const float *, float *, int32_t, int32_t);
using FlushFunc = int32_t (*)(TimeStretchHandle, int32_t *);
using GetSpecFunc = AudioTimeStretchSpec (*)(TimeStretchHandle);
using SamplesAvailableFunc = int32_t (*)(TimeStretchHandle);

struct TimeStretchAlgoApi {
    CreateFunc create{nullptr};
    DestroyFunc destroy{nullptr};
    SetSpeedFunc setSpeed{nullptr};
    GetSpeedFunc getSpeed{nullptr};
    SetPitchFunc setPitch{nullptr};
    GetPitchFunc getPitch{nullptr};
    ChangeSpeedFloatFunc changeSpeedFloat{nullptr};
    FlushFunc flush{nullptr};
    GetSpecFunc getSpec{nullptr};
    SamplesAvailableFunc samplesAvailable{nullptr};
};

class AudioSuiteTimeStretchImpl : public AudioSuiteTimeStretch {
public:
    AudioSuiteTimeStretchImpl(uint32_t sampleRate, uint32_t channels, AudioSampleFormat format);
    ~AudioSuiteTimeStretchImpl() override;

    int32_t Initialize();
    int32_t SetSpeed(float speed) override;
    float GetSpeed() const override;
    int32_t SetPitch(float pitch) override;
    float GetPitch() const override;
    int32_t GetDelayFrames() const override;
    int32_t Process(const uint8_t *inputData, int32_t inputSize,
                    uint8_t *outputData, int32_t outputCapacity, int32_t &outSize) override;
    int32_t Flush(uint8_t *outputData, int32_t outputCapacity, int32_t &outSize,
                  int32_t &residualFrames) override;

private:
    void *soHandle_ = nullptr;
    TimeStretchHandle algoHandle_ = nullptr;
    uint32_t sampleRate_ = 0;
    uint32_t channels_ = 0;
    uint32_t formatBytes_ = 0;
    AudioSampleFormat format_ = SAMPLE_S16LE;
    bool needFormatConvert_ = false;
    float speed_ = 1.0f;
    TimeStretchAlgoApi algoApi_{};
    std::vector<float> floatInputBuffer_;
    std::vector<float> floatOutputBuffer_;
    mutable std::mutex mutex_;

    int32_t LoadAlgorithm();
    int32_t MapFormat(AudioSampleFormat format);
    int32_t ValidateConfig() const;
    int32_t LoadAlgoSymbols();
    uint32_t GetBytesPerFrame() const;
    void EnsureBuffers(int32_t inFrames, int32_t maxOutFrames);
    int32_t RunChangeSpeed(const uint8_t *inputData, int32_t inFrames,
        int32_t maxOutFrames, uint8_t *outputData);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SUITE_TIME_STRETCH_IMPL_H
