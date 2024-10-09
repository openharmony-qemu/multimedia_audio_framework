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
#define LOG_TAG "AudioRenderModeCallbackTest"
#endif

#include <chrono>
#include <cstdio>
#include <thread>
#include <climits>
#include <cstdlib>
#include "audio_renderer_log.h"
#include "audio_renderer.h"
#include "pcm2wav.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace {
    constexpr int32_t SAMPLE_FORMAT_U8 = 8;
    constexpr int32_t SAMPLE_FORMAT_S16LE = 16;
    constexpr int32_t SAMPLE_FORMAT_S24LE = 24;
    constexpr int32_t SAMPLE_FORMAT_S32LE = 32;
    constexpr int32_t ARGS_COUNT_THREE = 3;
    constexpr int32_t PARAM2 = 2;
}
class AudioRenderModeCallbackTest : public AudioRendererWriteCallback,
    public enable_shared_from_this<AudioRenderModeCallbackTest> {
public:
    void OnWriteData(size_t length) override
    {
        AUDIO_INFO_LOG("RenderCallbackTest: OnWriteData is called");
        reqBufLen_ = length;
        isEnqueue_ = true;
    }
    
    AudioSampleFormat GetSampleFormat(int32_t wavSampleFormat)
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

    bool InitRender()
    {
        wav_hdr wavHeader;
        size_t headerSize = sizeof(wav_hdr);
        size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile_);
        if (bytesRead != headerSize) {
            AUDIO_ERR_LOG("RenderCallbackTest: File header reading error");
            return false;
        }

        AudioRendererOptions rendererOptions = {};
        rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
        rendererOptions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(wavHeader.SamplesPerSec);
        rendererOptions.streamInfo.format = GetSampleFormat(wavHeader.bitsPerSample);
        rendererOptions.streamInfo.channels = static_cast<AudioChannel>(wavHeader.NumOfChan);
        rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
        rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
        rendererOptions.rendererInfo.rendererFlags = 0;

        audioRenderer_ = AudioRenderer::Create(rendererOptions);
        if (audioRenderer_== nullptr) {
            AUDIO_ERR_LOG("RenderCallbackTest: Renderer create failed");
            return false;
        }

        AUDIO_INFO_LOG("RenderCallbackTest: Playback renderer created");
        if (audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK)) {
            AUDIO_ERR_LOG("RenderCallbackTest: SetRenderMode failed");
            return false;
        }

        if (audioRenderer_->SetRendererWriteCallback(shared_from_this())) {
            AUDIO_ERR_LOG("RenderCallbackTest: SetRendererWriteCallback failed");
            return false;
        }

        if (blendMode_ != 0) {
            AUDIO_INFO_LOG("RenderCallbackTest: set blendmode to %{public}d", blendMode_);
            audioRenderer_->SetChannelBlendMode((ChannelBlendMode)blendMode_);
        }

        audioRenderer_->GetBufferSize(reqBufLen_);

        return true;
    }

    int32_t TestPlayback(int argc, char *argv[])
    {
        AUDIO_INFO_LOG("RenderCallbackTest: TestPlayback start");
        if (!InitRender()) {
            return -1;
        }

        if (!audioRenderer_->Start()) {
            AUDIO_ERR_LOG("RenderCallbackTest: Start failed");
            audioRenderer_->Release();
            return -1;
        }

        enqueueThread_ = make_unique<thread>(&AudioRenderModeCallbackTest::EnqueueBuffer, this);
        enqueueThread_ ->join();

        audioRenderer_->Clear();
        audioRenderer_->Stop();
        audioRenderer_->Release();
        AUDIO_INFO_LOG("RenderCallbackTest: TestPlayback end");

        return 0;
    }

    ~AudioRenderModeCallbackTest()
    {
        AUDIO_INFO_LOG("RenderCallbackTest: Inside ~AudioRenderModeCallbackTest");
        if (fclose(wavFile_)) {
            AUDIO_INFO_LOG("RenderCallbackTest: wavFile_ failed");
        } else {
            AUDIO_INFO_LOG("RenderCallbackTest: fclose(wavFile_) success");
        }
        wavFile_ = nullptr;

        if (enqueueThread_ && enqueueThread_->joinable()) {
            enqueueThread_->join();
            enqueueThread_ = nullptr;
        }
    }

    void SetBlendMode(int32_t blendMode)
    {
        blendMode_ = blendMode;
    }

    FILE *wavFile_ = nullptr;
private:
    void EnqueueBuffer()
    {
        AUDIO_INFO_LOG("RenderCallbackTest: EnqueueBuffer thread");
        while (!feof(wavFile_)) {
            if (isEnqueue_) {
                // Requested length received in callback
                size_t reqLen = reqBufLen_;
                bufDesc_.buffer = nullptr;
                audioRenderer_->GetBufferDesc(bufDesc_);
                if (bufDesc_.buffer == nullptr) {
                    continue;
                }
                // requested len in callback will never be greater than allocated buf length
                // This is just a fail-safe
                if (reqLen > bufDesc_.bufLength) {
                    bufDesc_.dataLength = bufDesc_.bufLength;
                } else {
                    bufDesc_.dataLength = reqLen;
                }

                fread(bufDesc_.buffer, 1, bufDesc_.dataLength, wavFile_);
                audioRenderer_->Enqueue(bufDesc_);
                isEnqueue_ = false;
            }
        }
    }

    unique_ptr<AudioRenderer> audioRenderer_ = nullptr;
    unique_ptr<thread> enqueueThread_ = nullptr;
    bool isEnqueue_ = true;
    BufferDesc bufDesc_ {};
    size_t reqBufLen_;
    int32_t blendMode_ = 0;
};

int main(int argc, char *argv[])
{
    char *inputPath = argv[1];
    char path[PATH_MAX + 1] = {0x00};
    if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
        AUDIO_ERR_LOG("RenderCallbackTest: Invalid input filepath");
        return -1;
    }
    AUDIO_INFO_LOG("RenderCallbackTest: path = %{public}s", path);
    auto testObj = std::make_shared<AudioRenderModeCallbackTest>();

    if (argc == ARGS_COUNT_THREE) {
        testObj->SetBlendMode(atoi(argv[PARAM2]));
    }

    testObj->wavFile_ = fopen(path, "rb");
    if (testObj->wavFile_ == nullptr) {
        AUDIO_ERR_LOG("AudioRendererTest: Unable to open wave file");
        return -1;
    }

    return testObj->TestPlayback(argc, argv);
}
