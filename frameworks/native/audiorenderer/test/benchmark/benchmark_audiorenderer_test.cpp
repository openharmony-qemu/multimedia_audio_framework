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
#include <chrono>
#include <thread>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace {
    class BenchmarkAudiorendererTest : public benchmark::Fixture {
    public:
        BenchmarkAudiorendererTest()
        {
            Iterations(iterations);
            Repetitions(repetitions);
            ReportAggregatesOnly();
        }

        ~BenchmarkAudiorendererTest() override = default;

        void SetUp(const ::benchmark::State &state) override
        {
            sleep(1);
            AudioRendererOptions rendererOptions;
            rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
            rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
            rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
            rendererOptions.streamInfo.channels = AudioChannel::STEREO;
            rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
            rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
            rendererOptions.rendererInfo.rendererFlags = RENDERER_FLAG;
            audioRenderer = AudioRenderer::Create(rendererOptions);
            sleep(1);
        }

        void TearDown(const ::benchmark::State &state) override
        {
            audioRenderer->Release();
        }

    protected:
        unique_ptr<AudioRenderer> audioRenderer;
        const int32_t RENDERER_FLAG = 0;
        const int32_t repetitions = 3;
        const int32_t iterations = 300;
        const string AUDIORENDER_TEST_FILE_PATH = "/data/test_44100_2.wav";
    };

    // StartAbility
    BENCHMARK_F(BenchmarkAudiorendererTest, StartAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            bool isStarted = audioRenderer->Start();
            if (isStarted == false)
            {
                state.SkipWithError("StartAbilityTestCase audioRenderer start failed.");
            }

            state.PauseTiming();
            bool isStopped = audioRenderer->Stop();
            if (isStopped == false)
            {
                state.SkipWithError("StartAbilityTestCase audioRenderer start failed.");
            }
            state.ResumeTiming();
        }
    }

    // PauseAbility
    BENCHMARK_F(BenchmarkAudiorendererTest, PauseAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            state.PauseTiming();
            bool isStarted = audioRenderer->Start();
            if (isStarted == false)
            {
                state.SkipWithError("PauseAbilityTestCase audioRenderer start failed.");
            }

            bool isPaused = audioRenderer->Pause();
            if (isPaused == false)
            {
                state.SkipWithError("PauseAbilityTestCase audioRenderer pause failed.");
            }

            state.PauseTiming();
            bool isStopped = audioRenderer->Stop();
            if (isStopped == false)
            {
                state.SkipWithError("PauseAbilityTestCase audioRenderer start failed.");
            }
            state.ResumeTiming();
        }
    }

    // StopAbility
    BENCHMARK_F(BenchmarkAudiorendererTest, StopAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            state.PauseTiming();
            bool isStarted = audioRenderer->Start();
            if (isStarted == false)
            {
                state.SkipWithError("StopAbilityTestCase audioRenderer start failed.");
            }
            state.ResumeTiming();

            bool isStopped = audioRenderer->Stop();
            if (isStopped == false)
            {
                state.SkipWithError("StopAbilityTestCase audioRenderer start failed.");
            }
        }
    }

    // SetVolumeAbility
    BENCHMARK_F(BenchmarkAudiorendererTest, SetVolumeAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            state.PauseTiming();
            int32_t ret = -1;
            ret = audioRenderer->SetVolume(0);
            if (ret != SUCCESS)
            {
                state.SkipWithError("SetVolumeAbilityTestCase 0 audioRenderer failed.");
            }
            state.ResumeTiming();

            ret = audioRenderer->SetVolume(1.0);
            if (ret != SUCCESS)
            {
                state.SkipWithError("SetVolumeAbilityTestCase 1.0 audioRenderer failed.");
            }
        }
    }

    // WriteAbility  ****************** write
    BENCHMARK_F(BenchmarkAudiorendererTest, WriteAbilityTestCase)
    (
        benchmark::State &state)
    {
        while (state.KeepRunning())
        {
            state.PauseTiming();
            int32_t ret = -1;

            FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
            if (wavFile == nullptr)
            {
                state.SkipWithError("WriteAbilityTestCase fopen fail");
            }

            bool isStarted = audioRenderer->Start();
            if (isStarted == false)
            {
                state.SkipWithError("WriteAbilityTestCase audioRenderer start fail");
            }

            size_t bufferLen;
            ret = audioRenderer->GetBufferSize(bufferLen);

            uint8_t *buffer = reinterpret_cast<uint8_t *>(malloc(bufferLen));
            if (buffer == nullptr)
            {
                state.SkipWithError("WriteAbilityTestCase getbuffer fail");
            }
            size_t bytesToWrite = 0;
            int32_t bytesWritten = 0;

            bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
            state.ResumeTiming();

            bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
            if (bytesWritten <= 0)
            {
                state.SkipWithError("WriteAbilityTestCase Write fail");
            }
            state.PauseTiming();
            audioRenderer->Stop();
            free(buffer);
            fclose(wavFile);
            state.ResumeTiming();
        }
    }
}

// Run the benchmark
BENCHMARK_MAIN();