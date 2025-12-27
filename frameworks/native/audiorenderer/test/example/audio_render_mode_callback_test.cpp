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
#ifndef LOG_TAG
#define LOG_TAG "AudioRenderModeCallbackTest"
#endif

#include <chrono>
#include <cstdio>
#include <thread>
#include <climits>
#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include "audio_errors.h"
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
    constexpr int32_t PARAM2 = 2;
    constexpr int32_t PARAM3 = 3;
    constexpr size_t FIXED_SIZE = 1024;
    const uint64_t SYNC_FRAME_PTS_SPAN_US = 20000; // 20ms for test
}
class AudioRenderModeCallbackTest : public AudioRendererWriteCallback,
    public enable_shared_from_this<AudioRenderModeCallbackTest> {
public:
    void TestPeriodCall()
    {
        Timestamp stamp;
        audioRenderer_->GetAudioPosition(stamp, Timestamp::MONOTONIC);
        audioRenderer_->SetSpeed(1.0);
        std::cout << "writing data..." << std::endl;
    }

    void OnWriteData(size_t length) override
    {
        if (isEnd_) {
            return;
        }
        if (feof(wavFile_)) {
            CallExit();
            return;
        }

        BufferDesc bufferDesc;
        int32_t ret = audioRenderer_->GetBufferDesc(bufferDesc);
        if (ret != SUCCESS || bufferDesc.buffer == nullptr) {
            std::cout << "GetBufferDesc failed" << std::endl;
            CallExit();
            return;
        }

        if (bufferDesc.bufLength < FIXED_SIZE) {
            std::cout << "bufferDesc.bufLength is invalid:" << bufferDesc.bufLength << std::endl;
            CallExit();
            return;
        }

        bufferDesc.dataLength = FIXED_SIZE;
        bufferDesc.syncFramePts = mockPts_;
        mockPts_ += SYNC_FRAME_PTS_SPAN_US;
        if (mockPts_ % AUDIO_US_PER_S == 0) {
            TestPeriodCall();
        }

        bufferDesc.dataLength = fread(bufferDesc.buffer, 1, bufferDesc.dataLength, wavFile_);
        ret = audioRenderer_->Enqueue(bufferDesc);
        if (ret != SUCCESS) {
            std::cout << "Enqueue failed" << std::endl;
            CallExit();
            return;
        }
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

    bool InitEAC3Render()
    {
        AudioRendererOptions rendererOptions = {};
        rendererOptions.streamInfo.encoding = encodingType_;
        rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_48000;
        rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
        rendererOptions.streamInfo.channels = STEREO;
        rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
        rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MOVIE;
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

        return true;
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
        rendererOptions.streamInfo.encoding = encodingType_;
        rendererOptions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(wavHeader.SamplesPerSec);
        rendererOptions.streamInfo.format = GetSampleFormat(wavHeader.bitsPerSample);
        rendererOptions.streamInfo.channels = static_cast<AudioChannel>(wavHeader.NumOfChan);
        rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
        rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MOVIE;
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

        return true;
    }

    int32_t TestPlayback(int argc, char *argv[])
    {
        AUDIO_INFO_LOG("RenderCallbackTest: TestPlayback start");
        if (!InitEAC3Render()) {
            std::cout << "InitEAC3Render failed" << std::endl;
            return -1;
        }

        std::cout << "TestPlayback Start" << std::endl;
        if (!audioRenderer_->Start()) {
            AUDIO_ERR_LOG("RenderCallbackTest: Start failed");
            audioRenderer_->Release();
            return -1;
        }
        std::unique_lock<std::mutex> lock(endMutex_);
        endCV_.wait(lock, [this] {
            return isEnd_;
        });
        std::cout << "TestPlayback Stop" << std::endl;
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
    }

    FILE *wavFile_ = nullptr;
    AudioEncodingType encodingType_ = ENCODING_EAC3; // default to EAC3
private:
    void CallExit()
    {
        std::unique_lock<std::mutex> lock(endMutex_);
        std::cout << "TestPlayback reach file end" << std::endl;
        isEnd_ = true;
        endCV_.notify_all();
    }

private:
    unique_ptr<AudioRenderer> audioRenderer_ = nullptr;
    std::mutex endMutex_;
    bool isEnd_ = false;
    std::condition_variable endCV_;
    uint64_t mockPts_ = 0;
};

std::map<std::string, AudioEncodingType> g_typeToStr = {
    {"EAC3", ENCODING_EAC3},
    {"AC3", ENCODING_AC3},
    {"TRUE_HD", ENCODING_TRUE_HD},
    {"DTS_HD", ENCODING_DTS_HD},
    {"DTS_X", ENCODING_DTS_X},
    {"VIVID", ENCODING_AUDIOVIVID_DIRECT}};

AudioEncodingType GetEncodingType(std::string type)
{
    if (g_typeToStr.count(type)) {
        return g_typeToStr[type];
    }
    std::cout << "invalid encoding type:" << type << " will use EAC3 instead!" << std::endl;
    return ENCODING_EAC3;
}

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

    if (argc > PARAM3) {
        std::cout << "call test with file path and format" << std::endl;
        return -1;
    }

    if (argc == PARAM3) {
        std::cout << "Test with format:" << argv[PARAM2] << std::endl;
        testObj->encodingType_ = GetEncodingType(argv[PARAM2]);
    } else {
        std::cout << "Test with EAC3" << std::endl;
    }

    testObj->wavFile_ = fopen(path, "rb");
    if (testObj->wavFile_ == nullptr) {
        AUDIO_ERR_LOG("AudioRendererTest: Unable to open wave file");
        return -1;
    }

    return testObj->TestPlayback(argc, argv);
}
