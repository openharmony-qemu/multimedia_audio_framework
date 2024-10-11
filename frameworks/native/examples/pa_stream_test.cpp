/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "PaStreamTest"
#endif

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <unistd.h>
#include <thread>
#include <random>
#include <iostream>
#include "securec.h"

#include "audio_log.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "pcm2wav.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
constexpr int32_t SAMPLE_FORMAT_U8 = 8;
constexpr int32_t SAMPLE_FORMAT_S16LE = 16;
constexpr int32_t SAMPLE_FORMAT_S24LE = 24;
constexpr int32_t SAMPLE_FORMAT_S32LE = 32;
constexpr size_t ONE_READ_FRAME = 3840;

enum RendererMode : int32_t {
    DIRECTLY_WRITE = 0,
    AFTER_CALLBACK = 1,
};

enum CapturerMode : int32_t {
    DIRECTLY_READ = 0,
    READ_AFTER_CALLBACK = 1,
};

enum OperationCode : int32_t {
    CODE_INVALID = -1,
    RENDERER_CODE_INIT = 0,
    RENDERER_CODE_START = 1,
    RENDERER_CODE_PAUSE = 2,
    RENDERER_CODE_FLUSH = 3,
    RENDERER_CODE_DRAIN = 4,
    RENDERER_CODE_STOP = 5,
    RENDERER_CODE_RELEASE = 6,
    RENDERER_CODE_WRITE = 7,
    CAPTURER_CODE_INIT = 100,
    CAPTURER_CODE_START = 101,
    CAPTURER_CODE_PAUSE = 102,
    CAPTURER_CODE_FLUSH = 103,
    CAPTURER_CODE_STOP = 105,
    CAPTURER_CODE_RELEASE = 106,
    CAPTURER_CODE_READ = 107,
    EXIT_DEMO = 1000,
};

std::map<int32_t, std::string> g_OptStrMap = {
    {RENDERER_CODE_INIT, "call spk init process"},
    {RENDERER_CODE_START, "call start spk process"},
    {RENDERER_CODE_PAUSE, "call pause spk process"},
    {RENDERER_CODE_FLUSH, "call flush spk process"},
    {RENDERER_CODE_DRAIN, "call drain spk process"},
    {RENDERER_CODE_STOP, "call stop spk process"},
    {RENDERER_CODE_RELEASE, "release spk process"},
    {RENDERER_CODE_WRITE, "write data"},
    {CAPTURER_CODE_INIT, "call capturer init process"},
    {CAPTURER_CODE_START, "call start capturer process"},
    {CAPTURER_CODE_PAUSE, "call pause capturer process"},
    {CAPTURER_CODE_FLUSH, "call flush capturer process"},
    {CAPTURER_CODE_STOP, "call stop capturer process"},
    {CAPTURER_CODE_RELEASE, "call release capturer process"},
    {CAPTURER_CODE_READ, "read data"},
    {EXIT_DEMO, "exit interactive run test"},
};

class PaRendererTest : public AudioRendererWriteCallback, public enable_shared_from_this<PaRendererTest> {
public:
    virtual ~PaRendererTest() {};
    int32_t InitRenderer(RendererMode rendererMode, int32_t fileIndex);
    int32_t StartPlay();
    int32_t PausePlay();
    int32_t FlushPlay();
    int32_t DrainPlay();
    int32_t StopPlay();
    int32_t ReleasePlay();
    int32_t WriteData();
    void WriteDataWorker();
    void OnWriteData(size_t length) override;
    AudioSampleFormat GetSampleFormat(int32_t wavSampleFormat) const;
    bool OpenSpkFile(const std::string &spkFilePath);
    void CloseSpkFile();

private:
    std::unique_ptr<AudioRenderer> audioRenderer_ = nullptr;
    static constexpr long WAV_HEADER_SIZE = 44;
    FILE *spkWavFile_ = nullptr;
    size_t bytesAlreadyWrite_ = 0;

    std::condition_variable enableWriteCv_;
    std::mutex enableWriteThreadLock_;
    bool enableWrite_ = false;
    int32_t fast_ = 1000;
    int32_t slow_ = 30000;
    size_t bufferLength_ = 0;
    bool isFileOpened_ = false;
    wav_hdr wavHeader_;
    RendererMode rendererMode_ = DIRECTLY_WRITE;

    std::map<int32_t, std::string> filePathMap_ = {
        {0, "/data/test.wav"},
        {1, "/data/test2.wav"},
    };
};

AudioSampleFormat PaRendererTest::GetSampleFormat(int32_t wavSampleFormat) const
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

bool PaRendererTest::OpenSpkFile(const std::string &spkFilePath)
{
    if (spkWavFile_ != nullptr) {
        AUDIO_ERR_LOG("Spk file has been opened, spkFilePath %{public}s", spkFilePath.c_str());
        return true;
    }

    char path[PATH_MAX] = { 0x00 };
    if ((strlen(spkFilePath.c_str()) > PATH_MAX) || (realpath(spkFilePath.c_str(), path) == nullptr)) {
        return false;
    }
    AUDIO_INFO_LOG("spk path = %{public}s", path);
    spkWavFile_ = fopen(path, "rb");
    if (spkWavFile_ == nullptr) {
        AUDIO_ERR_LOG("Unable to open wave file");
        return false;
    }
    return true;
}

void PaRendererTest::CloseSpkFile()
{
    if (spkWavFile_ != nullptr) {
        fclose(spkWavFile_);
        spkWavFile_ = nullptr;
    }
}

int32_t PaRendererTest::InitRenderer(RendererMode rendererMode, int32_t fileIndex)
{
    rendererMode_ = rendererMode;
    AUDIO_INFO_LOG("Start OpenSpkFile, isFileOpened_: %{public}d", isFileOpened_);
    if (isFileOpened_ == false) {
        AUDIO_INFO_LOG("Start OpenSpkFile, fileIndex: %{public}d", fileIndex);
        std::string path = filePathMap_[fileIndex];
        OpenSpkFile(path);
        
        size_t headerSize = sizeof(wav_hdr);
        size_t bytesRead = fread(&wavHeader_, 1, headerSize, spkWavFile_);
        AUDIO_DEBUG_LOG("Init renderer, bytesRead: %{public}zu", bytesRead);
        isFileOpened_ = true;
    }
    ContentType contentType = ContentType::CONTENT_TYPE_MUSIC;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_MEDIA;

    AudioRendererOptions rendererOptions = {};
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(wavHeader_.SamplesPerSec);
    rendererOptions.streamInfo.format = GetSampleFormat(wavHeader_.bitsPerSample);
    rendererOptions.streamInfo.channels = static_cast<AudioChannel>(wavHeader_.NumOfChan);
    rendererOptions.rendererInfo.contentType = contentType;
    rendererOptions.rendererInfo.streamUsage = streamUsage;
    rendererOptions.rendererInfo.rendererFlags = 0;
    AUDIO_ERR_LOG("samplingRate %{public}d, format %{public}d, channels %{public}d",
        rendererOptions.streamInfo.samplingRate, rendererOptions.streamInfo.format,
        rendererOptions.streamInfo.channels);
    audioRenderer_ = AudioRenderer::Create(rendererOptions);
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("AudioRendererTest: Create failed");
        return -1;
    }

    if (rendererMode_ == AFTER_CALLBACK) {
        if (audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK)) {
            AUDIO_ERR_LOG("SetRenderMode failed");
            return false;
        }

        if (audioRenderer_->SetRendererWriteCallback(shared_from_this())) {
            AUDIO_ERR_LOG("SetRendererWriteCallback failed");
            return false;
        }
    }

    if (audioRenderer_->GetBufferSize(bufferLength_)) {
        return -1;
    }
    AUDIO_INFO_LOG("Audio renderer create success.");
    return 0;
}

int32_t PaRendererTest::StartPlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    if (!audioRenderer_->Start()) {
        AUDIO_ERR_LOG("Audio renderer start failed.");
        return -1;
    }
    enableWrite_ = true;
    enableWriteCv_.notify_all();
    return 0;
}

int32_t PaRendererTest::PausePlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    enableWrite_ = false;
    if (!audioRenderer_->Pause()) {
        AUDIO_ERR_LOG("Audio renderer start failed.");
        return -1;
    }
    return 0;
}

int32_t PaRendererTest::FlushPlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    if (!audioRenderer_->Flush()) {
        AUDIO_ERR_LOG("Audio renderer start failed.");
        return -1;
    }
    return 0;
}

int32_t PaRendererTest::DrainPlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    if (!audioRenderer_->Drain()) {
        AUDIO_ERR_LOG("Audio renderer start failed.");
        return -1;
    }
    return 0;
}

int32_t PaRendererTest::StopPlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    enableWrite_ = false;
    if (!audioRenderer_->Stop()) {
        AUDIO_ERR_LOG("Audio renderer stop failed.");
        return -1;
    }
    return 0;
}

int32_t PaRendererTest::ReleasePlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    enableWrite_ = false;
    if (!audioRenderer_->Release()) {
        AUDIO_ERR_LOG("Audio renderer stop failed.");
        return -1;
    }
    audioRenderer_ = nullptr;
    return 0;
}

int32_t PaRendererTest::WriteData()
{
    enableWrite_ = true;
    std::thread writeDataThread = std::thread(&PaRendererTest::WriteDataWorker, this);
    writeDataThread.detach();
    return 0;
}

void PaRendererTest::WriteDataWorker()
{
    while (true) {
        std::unique_lock<std::mutex> threadLock(enableWriteThreadLock_);
        enableWriteCv_.wait(threadLock, [this] {
            AUDIO_INFO_LOG("enable write state: %{public}d", enableWrite_);
            return enableWrite_;
        });
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(fast_, slow_);
        int32_t randomNum = dis(gen);
        AUDIO_INFO_LOG("recorder sleepTime %{public}d", randomNum);
        usleep(randomNum);
        if (audioRenderer_ == nullptr) {
            AUDIO_ERR_LOG("Audiorenderer init failed.");
            enableWrite_ = false;
            return ;
        }
        if (spkWavFile_ == nullptr) {
            AUDIO_ERR_LOG("wavFile is nullptr");
            enableWrite_ = false;
            return ;
        }
        if (feof(spkWavFile_)) {
            fseek(spkWavFile_, WAV_HEADER_SIZE, SEEK_SET);
        }

        if (rendererMode_ == DIRECTLY_WRITE) {
            auto buffer = std::make_unique<uint8_t[]>(bufferLength_);
            AUDIO_ERR_LOG("WriteDataWorker: bufferLength_ %{public}zu", bufferLength_);
            fread(buffer.get(), 1, bufferLength_, spkWavFile_);
            bytesAlreadyWrite_ += audioRenderer_->Write(buffer.get(), bufferLength_);
            AUDIO_INFO_LOG("bytesAlreadyWrite_: %{public}zu, bufferLength_: %{public}zu",
                bytesAlreadyWrite_, bufferLength_);
        }
    }
}

void PaRendererTest::OnWriteData(size_t length)
{
    AUDIO_INFO_LOG("On write data callback, length %{public}zu", length);
    BufferDesc currentWriteBuffer = { nullptr, 0, 0};
    audioRenderer_->GetBufferDesc(currentWriteBuffer);
    if (currentWriteBuffer.buffer == nullptr) {
        return  ;
    }
    if (length > currentWriteBuffer.bufLength) {
        currentWriteBuffer.dataLength = currentWriteBuffer.bufLength;
    } else {
        currentWriteBuffer.dataLength = length;
    }
    fread(currentWriteBuffer.buffer, 1, currentWriteBuffer.dataLength, spkWavFile_);
    bytesAlreadyWrite_ += currentWriteBuffer.dataLength;
    audioRenderer_->Enqueue(currentWriteBuffer);
    AUDIO_INFO_LOG("Callback mode, bytesAlreadyWrite_: %{public}zu, length: %{public}zu",
        bytesAlreadyWrite_, length);
}

class PaCapturerTest : public AudioCapturerReadCallback, public enable_shared_from_this<PaCapturerTest> {
public:
    virtual ~PaCapturerTest() {};

    int32_t InitCapturer(bool isBlocking, CapturerMode capturerMode);
    int32_t StartRecorder();
    int32_t PauseRecorder();
    int32_t FlushRecorder();
    int32_t StopRecorder();
    int32_t ReleaseRecorder();
    int32_t ReadData();
    void ReadDataWorker();
    void OnReadData(size_t length) override;

private:
    std::unique_ptr<AudioCapturer> audioCapturer_ = nullptr;
    bool isBlocking_ = true;

    std::condition_variable enableReadCv_;
    std::mutex enableReadThreadLock_;
    bool enableRead_ = false;
    int32_t fast_ = 1; // min sleep time
    int32_t slow_ = 2; // max sleep time
    FILE *pfd_ = nullptr;
    CapturerMode capturerMode_ = DIRECTLY_READ;
};

void PaCapturerTest::OnReadData(size_t length)
{
    AUDIO_INFO_LOG("PaCapturerTest::OnReadData, length: %{public}zu", length);
    BufferDesc bufferDesc = { nullptr, 0, 0 };
    audioCapturer_->GetBufferDesc(bufferDesc);
    fwrite(reinterpret_cast<void *>(bufferDesc.buffer), 1, bufferDesc.bufLength, pfd_);
    audioCapturer_->Enqueue(bufferDesc);
}

int32_t PaCapturerTest::InitCapturer(bool isBlocking, CapturerMode capturerMode)
{
    AUDIO_INFO_LOG("Start InitCapturer");
    isBlocking_ = isBlocking;
    capturerMode_ = capturerMode;
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_8000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = 0;

    audioCapturer_ = AudioCapturer::Create(capturerOptions);
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("Create audioCapturer failed");
        return -1;
    }
    if (capturerMode_ == READ_AFTER_CALLBACK) {
        if (audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK)) {
            AUDIO_ERR_LOG("SetCaptureMode failed");
            return -1;
        }
        if (audioCapturer_->SetCapturerReadCallback(shared_from_this())) {
            AUDIO_ERR_LOG("SetCapturerReadCallback failed");
            return -1;
        }
    }
    AUDIO_INFO_LOG("Audio capturer create success.");
    pfd_ = fopen("/data/data/.pulse_dir/capturer.pcm", "wb+");
    return 0;
}

int32_t PaCapturerTest::StartRecorder()
{
    AUDIO_INFO_LOG("StartRecorder");
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer_ init failed.");
        return -1;
    }
    enableRead_ = true;
    enableReadCv_.notify_all();
    if (!audioCapturer_->Start()) {
        AUDIO_ERR_LOG("Audio capturer start failed.");
        return -1;
    }
    return 0;
}

int32_t PaCapturerTest::PauseRecorder()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer_ init failed.");
        return -1;
    }
    enableRead_ = false;
    if (!audioCapturer_->Pause()) {
        AUDIO_ERR_LOG("Audio capturer start failed.");
        return -1;
    }
    return 0;
}

int32_t PaCapturerTest::FlushRecorder()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer_ init failed.");
        return -1;
    }
    if (!audioCapturer_->Flush()) {
        AUDIO_ERR_LOG("Audio capturer start failed.");
        return -1;
    }
    return 0;
}

int32_t PaCapturerTest::StopRecorder()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer_ init failed.");
        return -1;
    }
    if (!audioCapturer_->Stop()) {
        AUDIO_ERR_LOG("Audio capturer stop failed.");
        return -1;
    }
    return 0;
}

int32_t PaCapturerTest::ReleaseRecorder()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer_ init failed.");
        return -1;
    }
    enableRead_ = false;
    if (!audioCapturer_->Release()) {
        AUDIO_ERR_LOG("Audio capturer stop failed.");
        return -1;
    }
    audioCapturer_ = nullptr;
    fclose(pfd_);
    pfd_ = nullptr;
    return 0;
}

int32_t PaCapturerTest::ReadData()
{
    std::thread readDataThread = std::thread(&PaCapturerTest::ReadDataWorker, this);
    readDataThread.detach();
    return 0;
}

void PaCapturerTest::ReadDataWorker()
{
    while (true) {
        std::unique_lock<std::mutex> threadLock(enableReadThreadLock_);
        enableReadCv_.wait(threadLock, [this] {
            AUDIO_INFO_LOG("enable read state: %{public}d", enableRead_);
            return enableRead_;
        });
        AUDIO_INFO_LOG("ReadDataWorker");
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(fast_, slow_);

        uint8_t *buffer = reinterpret_cast<uint8_t *>(malloc(ONE_READ_FRAME));
        memset_s(buffer, ONE_READ_FRAME, 0, ONE_READ_FRAME);
        int32_t currentReadIndex = 0;
        while (currentReadIndex < ONE_READ_FRAME) {
            int32_t len = audioCapturer_->Read(*(buffer + currentReadIndex),
                ONE_READ_FRAME - currentReadIndex, isBlocking_);
            currentReadIndex += len;
        }
        fwrite(reinterpret_cast<void *>(buffer), 1, ONE_READ_FRAME, pfd_);
    }
}

int32_t GetUserInput()
{
    int32_t res = -1; // result
    size_t count = 3; // try three time
    cout << ">>";
    cin >> res;
    while (cin.fail() && count-- > 0) {
        cin.clear();
        cin.ignore();
        cout << "invalid input, not a number! Please retry with a number." << endl;
        cout << ">>";
        cin >> res;
    }
    return res;
}

void PrintUsage()
{
    cout << "[Pa Stream Test App]" << endl << endl;
    cout << "Supported Functionalities:" << endl;
    cout << "================================Usage=======================================" << endl << endl;
    cout << "  0: Init renderer." << endl;
    cout << "  1: Start play." << endl;
    cout << "  2: Pause play." << endl;
    cout << "  3: Flush play." << endl;
    cout << "  4: Drain play." << endl;
    cout << "  5: Stop play." << endl;
    cout << "  6: Release play." << endl;
    cout << "  7: Write data run." << endl;

    cout << "  100: Init Capturer." << endl;
    cout << "  101: Start read." << endl;
    cout << "  102: Pause read." << endl;
    cout << "  103: Flush read." << endl;
    cout << "  105: Stop read." << endl;
    cout << "  106: Release read." << endl;
    cout << "  107: Read data run." << endl;

    cout << "  1000: exit demo." << endl;
    cout << " Please input your choice: " << endl;
}

int32_t InitPlayback(std::shared_ptr<PaRendererTest> streamTest, RendererMode rendererMode, int32_t fileIndex)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, init spk error." << endl;
        return -1;
    }
    int32_t ret = streamTest->InitRenderer(rendererMode, fileIndex);
    if (ret != 0) {
        cout << "Init renderer error!" << endl;
        return -1;
    }
    return 0;
}

int32_t StartPlay(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, start play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->StartPlay();
    if (ret != 0) {
        cout << "Start play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t PausePlay(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, pause play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->PausePlay();
    if (ret != 0) {
        cout << "Pause play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t FlushPlay(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, Flush play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->FlushPlay();
    if (ret != 0) {
        cout << "Flush play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t DrainPlay(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, Drain play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->DrainPlay();
    if (ret != 0) {
        cout << "Drain play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t StopPlay(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, stop play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->StopPlay();
    if (ret != 0) {
        cout << "Stop play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t ReleasePlay(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, stop play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->ReleasePlay();
    if (ret != 0) {
        cout << "Stop play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t WriteData(std::shared_ptr<PaRendererTest> streamTest)
{
    if (streamTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, stop play error." << endl;
        return -1;
    }
    int32_t ret = streamTest->WriteData();
    if (ret != 0) {
        cout << "Stop play error!" << endl;
        return -1;
    }
    return 0;
}

int32_t InitRecorder(std::shared_ptr<PaCapturerTest> capturerTest, bool isBlocking, CapturerMode capturerMode)
{
    if (capturerTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, init recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->InitCapturer(isBlocking, capturerMode);
    if (ret != 0) {
        cout << "Init capturer error!" << endl;
        return -1;
    }
    return 0;
}

int32_t StartRecorder(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, start recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->StartRecorder();
    if (ret != 0) {
        cout << "Start recorder error!" << endl;
        return -1;
    }
    return 0;
}

int32_t PauseRecorder(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, pause recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->PauseRecorder();
    if (ret != 0) {
        cout << "Pause recorder error!" << endl;
        return -1;
    }
    return 0;
}

int32_t FlushRecorder(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, Flush recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->FlushRecorder();
    if (ret != 0) {
        cout << "Flush recorder error!" << endl;
        return -1;
    }
    return 0;
}

int32_t StopRecorder(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, stop recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->StopRecorder();
    if (ret != 0) {
        cout << "Stop recorder error!" << endl;
        return -1;
    }
    return 0;
}

int32_t ReleaseRecorder(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaRendererTest obj is nullptr, stop recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->ReleaseRecorder();
    if (ret != 0) {
        cout << "Stop recorder error!" << endl;
        return -1;
    }
    return 0;
}

int32_t ReadData(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaCapturerTest obj is nullptr, read data error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->ReadData();
    if (ret != 0) {
        cout << "Read data error!" << endl;
        return -1;
    }
    return 0;
}

void HandleCapturerCode(OperationCode optCode, std::shared_ptr<PaRendererTest> streamTest,
    std::shared_ptr<PaCapturerTest> capturerTest)
{
    switch (optCode) {
        case RENDERER_CODE_START:
            StartPlay(streamTest);
            break;
        case RENDERER_CODE_PAUSE:
            PausePlay(streamTest);
            break;
        case RENDERER_CODE_FLUSH:
            FlushPlay(streamTest);
            break;
        case RENDERER_CODE_DRAIN:
            DrainPlay(streamTest);
            break;
        case RENDERER_CODE_STOP:
            StopPlay(streamTest);
            break;
        case RENDERER_CODE_RELEASE:
            ReleasePlay(streamTest);
            break;
        case RENDERER_CODE_WRITE:
            WriteData(streamTest);
            break;
        case CAPTURER_CODE_START:
            StartRecorder(capturerTest);
            break;
        case CAPTURER_CODE_PAUSE:
            PauseRecorder(capturerTest);
            break;
        case CAPTURER_CODE_FLUSH:
            FlushRecorder(capturerTest);
            break;
        case CAPTURER_CODE_STOP:
            StopRecorder(capturerTest);
            break;
        case CAPTURER_CODE_RELEASE:
            ReleaseRecorder(capturerTest);
            break;
        case CAPTURER_CODE_READ:
            ReadData(capturerTest);
            break;
        default:
            cout << "Invalid input: " << optCode << endl;
            break;
    }
}

void Loop(std::shared_ptr<PaRendererTest> streamTest, std::shared_ptr<PaCapturerTest> capturerTest)
{
    bool isProcTestRun = true;
    while (isProcTestRun) {
        PrintUsage();
        OperationCode optCode = CODE_INVALID;
        int32_t res = GetUserInput();
        int32_t fileIndex = -1;
        int32_t rendererMode = 0;
        int32_t isBlocking = 0;
        int32_t capturerMode = 0;

        if (g_OptStrMap.count(res)) {
            optCode = static_cast<OperationCode>(res);
        }
        switch (optCode) {
            case RENDERER_CODE_INIT:
                rendererMode = GetUserInput();
                fileIndex = GetUserInput();
                InitPlayback(streamTest, static_cast<RendererMode>(rendererMode), fileIndex);
                break;
            // Capturer
            case CAPTURER_CODE_INIT:
                isBlocking = GetUserInput();
                capturerMode = GetUserInput();
                InitRecorder(capturerTest, isBlocking, static_cast<CapturerMode>(capturerMode));
                break;
            case EXIT_DEMO:
                isProcTestRun = false;
                break;
            default:
                HandleCapturerCode(optCode, streamTest, capturerTest);
                break;
        }
    }
}
}
}

using namespace OHOS::AudioStandard;
using namespace std;
int main(int argc, char* argv[])
{
    cout << "oh pa stream test." << endl;
    std::shared_ptr<PaRendererTest> streamTest = std::make_shared<PaRendererTest>();
    std::shared_ptr<PaCapturerTest> capturerTest = std::make_shared<PaCapturerTest>();

    Loop(streamTest, capturerTest);
    streamTest->CloseSpkFile();
    return 0;
}