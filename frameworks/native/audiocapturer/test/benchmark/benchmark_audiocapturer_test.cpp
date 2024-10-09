/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_capturer.h"
using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace {
    class BenchmarkAudiocapturerTest : public benchmark::Fixture {
    public:
        BenchmarkAudiocapturerTest()
        {
            Iterations(iterations);
            Repetitions(repetitions);
            ReportAggregatesOnly();
        }

        ~BenchmarkAudiocapturerTest() override = default;

        void SetUp(const ::benchmark::State &state) override
        {
            sleep(1);
            AudioCapturerOptions capturerOptions;
            capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
            capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
            capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
            capturerOptions.streamInfo.channels = AudioChannel::MONO;
            capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
            capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
            audioCapturer = AudioCapturer::Create(capturerOptions);
            sleep(1);
        }

        void TearDown(const ::benchmark::State &state) override
        {
            audioCapturer->Release();
        }

    protected:
        unique_ptr<AudioCapturer> audioCapturer;
        const int32_t CAPTURER_FLAG = 0;
        const int32_t repetitions = 3;
        const int32_t iterations = 300;
    };

    // StartAbility
    BENCHMARK_F(BenchmarkAudiocapturerTest, StartAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            bool isStarted = audioCapturer->Start();
            if (isStarted == false)
            {
                state.SkipWithError("StartAbilityTestCase audioCapturer start failed.");
            }

            state.PauseTiming();
            bool isStopped = audioCapturer->Stop();
            if (isStopped == false)
            {
                state.SkipWithError("StartAbilityTestCase audioCapturer start failed.");
            }
            state.ResumeTiming();
        }
    }

    // StopAbility
    BENCHMARK_F(BenchmarkAudiocapturerTest, StopAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            state.PauseTiming();
            bool isStarted = audioCapturer->Start();
            if (isStarted == false)
            {
                state.SkipWithError("StopAbilityTestCase audioCapturer start failed.");
            }
            state.ResumeTiming();

            bool isStopped = audioCapturer->Stop();
            if (isStopped == false)
            {
                state.SkipWithError("StopAbilityTestCase audioCapturer start failed.");
            }
        }
    }
}

// Run the benchmark
BENCHMARK_MAIN();