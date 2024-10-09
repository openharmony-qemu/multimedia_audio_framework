/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "InterruptMultiRendererTest"
#endif

#include <chrono>
#include <thread>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "audio_renderer_log.h"
#include "interrupt_multi_renderer_test.h"
#include "pcm2wav.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr int32_t FIVE_SEC = 5;
    constexpr int32_t MIN_NO_OF_ARGS = 3;
    constexpr int32_t ARG_INDEX_1 = 1;
    constexpr int32_t ARG_INDEX_2 = 2;
    constexpr int32_t ARG_INDEX_3 = 3;
    constexpr int32_t ARG_INDEX_4 = 4;
    constexpr int32_t NUM_BASE = 10;
    
    constexpr int32_t SAMPLE_FORMAT_U8 = 8;
    constexpr int32_t SAMPLE_FORMAT_S16LE = 16;
    constexpr int32_t SAMPLE_FORMAT_S24LE = 24;
    constexpr int32_t SAMPLE_FORMAT_S32LE = 32;
}

void AudioRendererCallbackTestImpl::OnStateChange(const RendererState state,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl:: OnStateChange");

    switch (state) {
        case RENDERER_PREPARED:
            AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_PREPARED");
            break;
        case RENDERER_RUNNING:
            AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_RUNNING");
            break;
        case RENDERER_STOPPED:
            AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_STOPPED");
            break;
        case RENDERER_PAUSED:
            AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_PAUSED");
            break;
        case RENDERER_RELEASED:
            AUDIO_DEBUG_LOG("AudioRendererCallbackTestImpl: OnStateChange RENDERER_RELEASED");
            break;
        default:
            AUDIO_ERR_LOG("AudioRendererCallbackTestImpl: OnStateChange NOT A VALID state");
            break;
    }
}

void AudioRendererCallbackTestImpl::OnInterrupt(const InterruptEvent &interruptEvent)
{
    AUDIO_DEBUG_LOG("InterruptMultiRendererTest:  OnInterrupt");
    AUDIO_DEBUG_LOG("InterruptMultiRendererTest: interrupt hintType: %{public}d", interruptEvent.hintType);

    if (interruptEvent.forceType == INTERRUPT_FORCE) {
        switch (interruptEvent.hintType) {
            case INTERRUPT_HINT_PAUSE:
                AUDIO_DEBUG_LOG("InterruptMultiRendererTest: ForcePaused Pause Writing");
                isRendererPaused_ = true;
                break;
            case INTERRUPT_HINT_STOP:
                AUDIO_DEBUG_LOG("InterruptMultiRendererTest: ForceStopped Stop Writing");
                isRendererStopped_ = true;
                break;
            case INTERRUPT_HINT_DUCK:
                AUDIO_INFO_LOG("InterruptMultiRendererTest: force INTERRUPT_HINT_DUCK received");
                break;
            case INTERRUPT_HINT_UNDUCK:
                AUDIO_INFO_LOG("InterruptMultiRendererTest: force INTERRUPT_HINT_UNDUCK received");
                break;
            default:
                AUDIO_ERR_LOG("InterruptMultiRendererTest: OnInterrupt NOT A VALID force HINT");
                break;
        }
    } else if  (interruptEvent.forceType == INTERRUPT_SHARE) {
        switch (interruptEvent.hintType) {
            case INTERRUPT_HINT_PAUSE:
                AUDIO_DEBUG_LOG("InterruptMultiRendererTest: SharePause Hint received, Do pause if required");
                break;
            case INTERRUPT_HINT_RESUME:
                AUDIO_DEBUG_LOG("InterruptMultiRendererTest: Do ShareResume");
                isRendererResumed_ = true;
                break;
            default:
                AUDIO_ERR_LOG("InterruptMultiRendererTest: OnInterrupt default share hint case");
                break;
        }
    }
}

void InterruptMultiRendererTest::WriteBuffer(AudioRenderer* audioRenderer, FILE* wavFile,
                                             const shared_ptr<AudioRendererCallbackTestImpl> &cb) const
{
    size_t bufferLen = 4096;
    audioRenderer->GetBufferSize(bufferLen);

    int32_t n = 2;
    auto buffer = make_unique<uint8_t[]>(n * bufferLen);

    size_t bytesToWrite = 0;
    size_t bytesWritten = 0;
    size_t minBytes = 4;
    while (true) {
        if (cb->isRendererResumed_) {
            if (audioRenderer->Start()) {
                cb->isRendererPaused_ = false;
            }
        }

        while (!feof(wavFile) &&
               !cb->isRendererPaused_ && !cb->isRendererStopped_ && !cb->isStopInProgress_) {
            bytesToWrite = fread(buffer.get(), 1, bufferLen, wavFile);
            bytesWritten = 0;

            while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
                int32_t retBytes = audioRenderer->Write(buffer.get() + bytesWritten,
                                                        bytesToWrite - bytesWritten);
                if (retBytes < 0) {
                    AUDIO_ERR_LOG("InterruptMultiRendererTest: Error occurred in writing buffer: %{public}d", retBytes);
                    if (audioRenderer->GetStatus() == RENDERER_PAUSED) {
                        cb->isRendererPaused_ = true;
                        int32_t seekPos = bytesWritten - bytesToWrite;
                        if (fseek(wavFile, seekPos, SEEK_CUR)) {
                            AUDIO_INFO_LOG("InterruptMultiRendererTest: fseek failed");
                        }
                        AUDIO_INFO_LOG("InterruptMultiRendererTest: fseek success");
                    } else if (audioRenderer->GetStatus() == RENDERER_STOPPED) {
                        AUDIO_INFO_LOG("InterruptMultiRendererTest: Renderer Stopped");
                        cb->isRendererStopped_ = true;
                    }
                    break;
                }
                bytesWritten += retBytes;
            }
        }

        if (feof(wavFile) || cb->isRendererStopped_) {
            if (feof(wavFile)) {
                AUDIO_INFO_LOG("InterruptMultiRendererTest: EOF reached. Complete rendering ");
            }
            if (audioRenderer->GetStatus() == RENDERER_RUNNING) {
                audioRenderer->Stop();
            }
            audioRenderer->Release();
            break;
        }
    }
}

bool InterruptMultiRendererTest::StartRender(const unique_ptr<AudioRenderer> &audioRenderer) const
{
    AUDIO_INFO_LOG("InterruptMultiRendererTest: Starting renderer");
    if (!audioRenderer->Start()) {
        AUDIO_ERR_LOG("InterruptMultiRendererTest: Start rejected or failed");
        if (!audioRenderer->Release()) {
            AUDIO_ERR_LOG("InterruptMultiRendererTest: Release failed");
        }
        return false;
    }
    AUDIO_INFO_LOG("InterruptMultiRendererTest: Playback started");
    return true;
}

AudioSampleFormat InterruptMultiRendererTest::GetSampleFormat(int32_t wavSampleFormat) const
{
    switch (wavSampleFormat) {
        case SAMPLE_FORMAT_U8:
            return AudioSampleFormat::SAMPLE_U8;
        case SAMPLE_FORMAT_S16LE:
            return AudioSampleFormat::SAMPLE_S16LE;
        case SAMPLE_FORMAT_S24LE:
            return AudioSampleFormat::SAMPLE_S24LE;
        case SAMPLE_FORMAT_S32LE:
            return AudioSampleFormat::SAMPLE_S32LE;
        default:
            return AudioSampleFormat::INVALID_WIDTH;
    }
}

bool InterruptMultiRendererTest::InitRender(const unique_ptr<AudioRenderer> &audioRenderer, FILE* &wavFile) const
{
    wav_hdr wavHeader;
    size_t headerSize = sizeof(wav_hdr);
    size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
    if (bytesRead != headerSize) {
        AUDIO_ERR_LOG("InterruptMultiRendererTest: File header reading error");
        return false;
    }

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = GetSampleFormat(wavHeader.bitsPerSample);
    rendererParams.sampleRate = static_cast<AudioSamplingRate>(wavHeader.SamplesPerSec);
    rendererParams.channelCount = static_cast<AudioChannel>(wavHeader.NumOfChan);
    rendererParams.encodingType = static_cast<AudioEncodingType>(ENCODING_PCM);

    if (audioRenderer->SetParams(rendererParams)) {
        AUDIO_ERR_LOG("InterruptMultiRendererTest: Set audio renderer parameters failed");
        if (!audioRenderer->Release()) {
            AUDIO_ERR_LOG("InterruptMultiRendererTest: Release failed");
        }
        return false;
    }
    AUDIO_INFO_LOG("InterruptMultiRendererTest: Playback renderer created");

    return true;
}

int32_t InterruptMultiRendererTest::ValidateFile(char *filePath, char path[]) const
{
    if ((strlen(filePath) > PATH_MAX) || (realpath(filePath, path) == nullptr)) {
        AUDIO_ERR_LOG("InterruptMultiRendererTest: Invalid input filepath");
        return -1;
    }

    return 0;
}

int32_t InterruptMultiRendererTest::TestPlayback(int argc, char *argv[]) const
{
    char *file1Path = argv[ARG_INDEX_1];
    char path1[PATH_MAX + 1] = {0x00};
    char *file2Path = argv[ARG_INDEX_2];
    char path2[PATH_MAX + 1] = {0x00};
    FILE *wavFile1 = nullptr;
    FILE *wavFile2 = nullptr;

    if (!ValidateFile(file1Path, path1)) {
        wavFile1 = fopen(path1, "rb");
        if (wavFile1 == nullptr) {
            return -1;
        }
    }

    if (!ValidateFile(file2Path, path2)) {
        wavFile2 = fopen(path2, "rb");
        if (wavFile2 == nullptr) {
            fclose(wavFile1);
            return -1;
        }
    }

    AudioStreamType streamType1 = STREAM_MUSIC;
    AudioStreamType streamType2 = STREAM_VOICE_CALL;
    if (argc > MIN_NO_OF_ARGS) {
        streamType1 = static_cast<AudioStreamType>(strtol(argv[ARG_INDEX_3], NULL, NUM_BASE));
    }
    if (argc > MIN_NO_OF_ARGS + 1) {
        streamType2 = static_cast<AudioStreamType>(strtol(argv[ARG_INDEX_4], NULL, NUM_BASE));
    }

    unique_ptr<AudioRenderer> audioRenderer1 = AudioRenderer::Create(streamType1);
    unique_ptr<AudioRenderer> audioRenderer2 = AudioRenderer::Create(streamType2);

    shared_ptr<AudioRendererCallback> cb1 = make_shared<AudioRendererCallbackTestImpl>();
    if (InitRender(audioRenderer1, wavFile1)) {
        audioRenderer1->SetRendererCallback(cb1);
    }

    shared_ptr<AudioRendererCallback> cb2 = make_shared<AudioRendererCallbackTestImpl>();
    if (InitRender(audioRenderer2, wavFile2)) {
        audioRenderer2->SetRendererCallback(cb2);
    }

    std::shared_ptr<AudioRendererCallbackTestImpl> cb1Impl =
        std::static_pointer_cast<AudioRendererCallbackTestImpl>(cb1);
    unique_ptr<thread> writeThread1 = nullptr;
    if (audioRenderer1->GetStatus() == RENDERER_PREPARED && StartRender(audioRenderer1)) {
        writeThread1 = make_unique<thread>(&InterruptMultiRendererTest::WriteBuffer, this,
                                           audioRenderer1.get(), wavFile1, cb1Impl);
        this_thread::sleep_for(chrono::seconds(FIVE_SEC));
    }

    std::shared_ptr<AudioRendererCallbackTestImpl> cb2Impl =
        std::static_pointer_cast<AudioRendererCallbackTestImpl>(cb2);
    unique_ptr<thread> writeThread2 = nullptr;
    if (audioRenderer2->GetStatus() == RENDERER_PREPARED && StartRender(audioRenderer2)) {
        writeThread2 = make_unique<thread>(&InterruptMultiRendererTest::WriteBuffer, this,
                                           audioRenderer2.get(), wavFile2, cb2Impl);
    }

    if (writeThread1 && writeThread1->joinable()) {
        writeThread1->join();
    }
    if (writeThread2 && writeThread2->joinable()) {
        writeThread2->join();
    }
    fclose(wavFile1);
    fclose(wavFile2);

    return 0;
}
} // namespace AudioStandard
} // namespace OHOS

using namespace OHOS;
using namespace OHOS::AudioStandard;

int main(int argc, char *argv[])
{
    AUDIO_INFO_LOG("InterruptMultiRendererTest: Render test in");

    if ((argv == nullptr) || (argc < MIN_NO_OF_ARGS)) {
        AUDIO_ERR_LOG("InterruptMultiRendererTest: argv / argc incorrect");
        return 0;
    }

    AUDIO_INFO_LOG("InterruptMultiRendererTest: argc=%d", argc);
    AUDIO_INFO_LOG("InterruptMultiRendererTest: argv[1]=%{public}s", argv[ARG_INDEX_1]);
    AUDIO_INFO_LOG("InterruptMultiRendererTest: argv[2]=%{public}s", argv[ARG_INDEX_2]);

    auto interruptMultiRendererTest = make_unique<InterruptMultiRendererTest>();
    AUDIO_INFO_LOG("InterruptMultiRendererTest: TestPlayback start ");
    int32_t ret = interruptMultiRendererTest->TestPlayback(argc, argv);
    AUDIO_INFO_LOG("InterruptMultiRendererTest: TestPlayback end");

    return ret;
}
