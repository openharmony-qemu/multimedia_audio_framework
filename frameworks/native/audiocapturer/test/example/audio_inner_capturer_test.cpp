/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "InnerCapTest"
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
#include <map>

#include "audio_capturer_log.h"
#include "audio_capturer.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
enum CapturerMode : int32_t {
    DIRECTLY_READ = 0,
    READ_AFTER_CALLBACK = 1,
};

enum OperationCode : int32_t {
    CODE_INVALID = -1,
    CAPTURER_CODE_INIT = 0,
    CAPTURER_CODE_START = 1,
    CAPTURER_CODE_PAUSE = 2,
    CAPTURER_CODE_FLUSH = 3,
    CAPTURER_CODE_STOP = 4,
    CAPTURER_CODE_RELEASE = 5,
    EXIT_DEMO = 1000,
    CAPTURER_CODE_MAX
};

class PaCapturerTest : public AudioCapturerReadCallback, public enable_shared_from_this<PaCapturerTest> {
public:
    virtual ~PaCapturerTest() {};

    int32_t InitCapturer(CapturerMode capturerMode);
    int32_t StartRecorder();
    int32_t PauseRecorder();
    int32_t FlushRecorder();
    int32_t StopRecorder();
    int32_t ReleaseRecorder();
    void OnReadData(size_t length) override;

private:
    std::unique_ptr<AudioCapturer> audioCapturer_ = nullptr;
    std::condition_variable enableReadCv_;
    bool enableRead_ = false;
    FILE *pfd_ = nullptr;
    CapturerMode capturerMode_ = READ_AFTER_CALLBACK;
};

void PaCapturerTest::OnReadData(size_t length)
{
    int32_t ret = -1;
    AUDIO_INFO_LOG("PaCapturerTest::OnReadData, length: %{public}zu", length);
    BufferDesc bufferDesc = { nullptr, 0, 0 };
    audioCapturer_->GetBufferDesc(bufferDesc);

    ret = fwrite(reinterpret_cast<void *>(bufferDesc.buffer), 1, bufferDesc.bufLength, pfd_);
    if (ret != 0) { AUDIO_ERR_LOG(" something wrong when writing pcm! "); }

    audioCapturer_->Enqueue(bufferDesc);
}

int32_t PaCapturerTest::InitCapturer(CapturerMode capturerMode)
{
    AUDIO_INFO_LOG("Start InitCapturer");
    capturerMode_ = capturerMode;
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE;
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
    int32_t ret = -1;
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
    ret = fclose(pfd_);
    if (ret != 0) { AUDIO_ERR_LOG(" something wrong when fclose! "); }
    pfd_ = nullptr;
    return 0;
}

int32_t GetUserInput()
{
    int32_t res = -1; // result
    size_t count = 3; // try three time
    int32_t ret = -1;
    cout << ">> >> >>";

    ret = scanf_s("%d", &res);
    if (ret != 0) { AUDIO_ERR_LOG(" something wrong when scanf_s! "); }
    while (res < 0 && count-- > 0) {
        cout << "invalid input, not a number! Please retry with a number." << endl;
        cout << ">> >> >>";
        ret = scanf_s("%d", &res);
        if (ret != 0) { AUDIO_ERR_LOG(" something wrong when scanf_s! "); }
    }

    return res;
}

void PrintUsage()
{
    cout << "[inner capturer Test App]" << endl << endl;
    cout << "Supported Functionalities:" << endl;
    cout << "================================Usage=======================================" << endl << endl;
    cout << "  0: Init Capturer." << endl;
    cout << "  1: Start read." << endl;
    cout << "  2: Pause read." << endl;
    cout << "  3: Flush read." << endl;
    cout << "  4: Stop read." << endl;
    cout << "  5: Release read." << endl;

    cout << "  1000: exit demo." << endl;
    cout << " Please input your choice: " << endl;
    cout << "================================Usage=======================================" << endl << endl;
}

int32_t InitRecorder(std::shared_ptr<PaCapturerTest> capturerTest, CapturerMode capturerMode)
{
    if (capturerTest == nullptr) {
        cout << "PaCapturerTest obj is nullptr, init recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->InitCapturer(capturerMode);
    if (ret != 0) {
        cout << "Init capturer error!" << endl;
        return -1;
    }
    return 0;
}

int32_t StartRecorder(std::shared_ptr<PaCapturerTest> capturerTest)
{
    if (capturerTest == nullptr) {
        cout << "PaCapturerTest obj is nullptr, start recorder error." << endl;
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
        cout << "PaCapturerTest obj is nullptr, pause recorder error." << endl;
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
        cout << "PaCapturerTest obj is nullptr, Flush recorder error." << endl;
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
        cout << "PaCapturerTest obj is nullptr, stop recorder error." << endl;
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
        cout << "PaCapturerTest obj is nullptr, stop recorder error." << endl;
        return -1;
    }
    int32_t ret = capturerTest->ReleaseRecorder();
    if (ret != 0) {
        cout << "Stop recorder error!" << endl;
        return -1;
    }
    return 0;
}

void HandleCapturerCode(OperationCode optCode, std::shared_ptr<PaCapturerTest> capturerTest)
{
    switch (optCode) {
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
        default:
            cout << "Invalid input: " << optCode << endl;
            break;
    }
}

void Loop(std::shared_ptr<PaCapturerTest> capturerTest)
{
    bool isProcTestRun = true;
    while (isProcTestRun) {
        PrintUsage();
        OperationCode optCode = CODE_INVALID;
        int32_t res = GetUserInput();
        int32_t capturerMode = 1; // default callback

        if (res < 0 && res >= OperationCode::CAPTURER_CODE_MAX) {
            cout << "OperationCode is INVALID!" << endl;
            return;
        }
        optCode = static_cast<OperationCode>(res);
        switch (optCode) {
            // Capturer
            case CAPTURER_CODE_INIT:
                cout << " ==== Recorder has been inited, use callback mode ==== " << endl;
                InitRecorder(capturerTest, static_cast<CapturerMode>(capturerMode));
                break;
            case EXIT_DEMO:
                isProcTestRun = false;
                break;
            default:
                HandleCapturerCode(optCode, capturerTest);
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
    cout << "oh inner capturer test." << endl;
    std::shared_ptr<PaCapturerTest> capturerTest = std::make_shared<PaCapturerTest>();
    Loop(capturerTest);
    return 0;
}
