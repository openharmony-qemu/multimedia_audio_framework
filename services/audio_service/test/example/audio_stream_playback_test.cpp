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
#include <cstdio>
#include <iostream>

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_service_log.h"
#include "audio_renderer.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "parameter.h"
#include "pcm2wav.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
enum AudioOptCode : int32_t {
    INVALID_OPERATION = -1,
    INIT_LOCAL_SPK = 0,
    INIT_REMOTE_SPK = 1,
    START_SPK = 2,
    STOP_SPK = 3,
    SWITCH_SPK = 4,
    RELEASE_SPK = 5,
    INIT_LOCAL_MIC = 6,
    INIT_REMOTE_MIC = 7,
    START_MIC = 8,
    STOP_MIC = 9,
    SWITCH_MIC = 10,
    RELEASE_MIC = 11,
    EXIT_DEMO = 12,
};

std::map<int32_t, std::string> g_OptStrMap = {
    {INIT_LOCAL_SPK, "call local spk init process"},
    {INIT_REMOTE_SPK, "call remote spk init process"},
    {START_SPK, "call start spk process"},
    {STOP_SPK, "call stop spk process"},
    {SWITCH_SPK, "call switch spk device process"},
    {RELEASE_SPK, "release spk process"},

    {INIT_LOCAL_MIC, "call local mic init process"},
    {INIT_REMOTE_MIC, "call remote mic init process"},
    {START_MIC, "call start mic process"},
    {STOP_MIC, "call stop mic process"},
    {SWITCH_MIC, "call switch mic device process"},
    {RELEASE_MIC, "release mic process"},

    {EXIT_DEMO, "exit interactive run test"},
};

class PlaybackTest : public AudioRendererWriteCallback,
    public AudioCapturerReadCallback,
    public std::enable_shared_from_this<PlaybackTest> {
public:
    int32_t InitRenderer(bool isFast);
    int32_t StartPlay();
    int32_t StopPlay();
    int32_t ReleaseRenderer();
    int32_t InitCapturer(bool isFast);
    int32_t StartCapture();
    int32_t StopCapture();
    int32_t ReleaseCapture();
    void OnWriteData(size_t length) override;
    void OnReadData(size_t length) override;
    bool OpenSpkFile(const std::string &spkFilePath);
    bool OpenMicFile(const std::string &micFilePath);
    void CloseSpkFile();
    void CloseMicFile();
    int32_t SwitchDevice(DeviceRole deviceRole);
    void SetSpkRemote(bool isRemote);
    bool GetSpkRemote();
    void SetMicRemote(bool isRemote);
    bool GetMicRemote();

private:
    int32_t SwitchOutputDevice();
    int32_t SwitchInputDevice();

private:
    std::unique_ptr<AudioStandard::AudioRenderer> audioRenderer_ = nullptr;
    std::unique_ptr<AudioStandard::AudioCapturer> audioCapturer_ = nullptr;
    static constexpr long WAV_HEADER_SIZE = 44;
    bool needSkipWavHeader_ = true;
    FILE *spkWavFile_ = nullptr;
    FILE *micWavFile_ = nullptr;
    bool isSpkFast_ = false;
    bool isMicFast_ = false;
    bool isSpkRemote_ = false;
    bool isMicRemote_ = false;
};

void PlaybackTest::OnWriteData(size_t length)
{
    BufferDesc bufDesc;
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("audioRenderer is nullptr.");
        return;
    }
    audioRenderer_->GetBufferDesc(bufDesc);

    if (spkWavFile_ == nullptr) {
        AUDIO_ERR_LOG("spkWavFile_ is nullptr.");
        return;
    }
    if (needSkipWavHeader_) {
        fseek(spkWavFile_, WAV_HEADER_SIZE, SEEK_SET);
        needSkipWavHeader_ = false;
    }
    if (feof(spkWavFile_)) {
        fseek(spkWavFile_, WAV_HEADER_SIZE, SEEK_SET); // infinite loop
    }
    fread(bufDesc.buffer, 1, bufDesc.bufLength, spkWavFile_);
    AUDIO_INFO_LOG("%{public}s OnWriteData data length: %{public}zu.", __func__, bufDesc.bufLength);
    audioRenderer_->Enqueue(bufDesc);
}

bool PlaybackTest::OpenSpkFile(const std::string &spkFilePath)
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

void PlaybackTest::CloseSpkFile()
{
    if (spkWavFile_ != nullptr) {
        fclose(spkWavFile_);
        spkWavFile_ = nullptr;
    }
}

void PlaybackTest::SetSpkRemote(bool isRemote)
{
    isSpkRemote_ = isRemote;
}

bool PlaybackTest::GetSpkRemote()
{
    return isSpkRemote_;
}

void PlaybackTest::SetMicRemote(bool isRemote)
{
    isMicRemote_ = isRemote;
}

bool PlaybackTest::GetMicRemote()
{
    return isMicRemote_;
}

int32_t PlaybackTest::InitRenderer(bool isFast)
{
    AudioStandard::AudioRendererOptions rendererOptions = {
        {
            AudioStandard::AudioSamplingRate::SAMPLE_RATE_48000,
            AudioStandard::AudioEncodingType::ENCODING_PCM,
            AudioStandard::AudioSampleFormat::SAMPLE_S16LE,
            AudioStandard::AudioChannel::STEREO,
        },
        {
            AudioStandard::ContentType::CONTENT_TYPE_UNKNOWN,
            AudioStandard::StreamUsage::STREAM_USAGE_GAME,
            isFast ? AudioStandard::STREAM_FLAG_FAST : AudioStandard::STREAM_FLAG_NORMAL, // fast audio stream
        }
    };
    audioRenderer_ = AudioStandard::AudioRenderer::Create(rendererOptions);
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audio renderer create failed.");
        return -1;
    }
    std::string path = "/data/test.wav";
    OpenSpkFile(path);
    int32_t ret = audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set render mode callback fail, ret %{public}d.", ret);
    ret = audioRenderer_->SetRendererWriteCallback(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Client test save data callback fail, ret %{public}d.", ret);
    AUDIO_INFO_LOG("Audio renderer create success.");
    isSpkFast_ = isFast;
    return 0;
}

int32_t PlaybackTest::SwitchDevice(DeviceRole deviceRole)
{
    if (deviceRole == OUTPUT_DEVICE) {
        return SwitchOutputDevice();
    } else {
        return SwitchInputDevice();
    }
}

int32_t PlaybackTest::SwitchOutputDevice()
{
    cout << "Select output device. current play device: " << isSpkRemote_ << "(0 : local, 1 : remote)" << endl;
    AudioSystemManager *manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        cout << "Get AudioSystemManager failed" << std::endl;
        return ERR_INVALID_OPERATION;
    }

    vector<sptr<AudioDeviceDescriptor>> devices;
    if (isSpkRemote_) {
        devices = manager->GetDevices(OUTPUT_DEVICES_FLAG);
        vector<sptr<AudioDeviceDescriptor>>::iterator it;
        for (it = devices.begin(); it != devices.end();) {
            if ((*it)->deviceType_ != DEVICE_TYPE_SPEAKER) {
                it = devices.erase(it);
            } else {
                it++;
            }
        }
    } else {
        devices = manager->GetDevices(DISTRIBUTED_OUTPUT_DEVICES_FLAG);
    }
    if (devices.size() != 1) {
        cout << "GetDevices failed, find no device, unsupported size:" << devices.size() << endl;
        return ERR_INVALID_OPERATION;
    }

    cout << "using device:" << devices[0]->networkId_ << endl;

    int32_t ret = -1;

    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = getuid();
    if (isSpkFast_) {
        filter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    }
    ret = manager->SelectOutputDevice(filter, devices);
    if (ret == SUCCESS) {
        isSpkRemote_ = !isSpkRemote_;
        cout << "Select output device success. current play device:" <<
            isSpkRemote_ << "(0 : local, 1 : remote)" << endl;
    } else {
        cout << "SelectOutputDevice failed, ret:" << ret << endl;
    }
    return ret;
}

int32_t PlaybackTest::SwitchInputDevice()
{
    AudioSystemManager *manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        cout << "Get AudioSystemManager failed" << std::endl;
        return ERR_INVALID_OPERATION;
    }

    vector<sptr<AudioDeviceDescriptor>> devices;
    if (isMicRemote_) {
        devices = manager->GetDevices(INPUT_DEVICES_FLAG);
    } else {
        devices = manager->GetDevices(DISTRIBUTED_INPUT_DEVICES_FLAG);
    }
    if (devices.size() != 1) {
        cout << "GetDevices failed, find no device, unsupported size:" << devices.size() << endl;
        return ERR_INVALID_OPERATION;
    }

    cout << "using device:" << devices[0]->networkId_ << endl;

    int32_t ret = -1;

    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = getuid();
    if (isMicFast_) {
        filter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
        filter->capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    }
    ret = manager->SelectInputDevice(filter, devices);
    if (ret == SUCCESS) {
        isMicRemote_ = !isMicRemote_;
        cout << "SelectInputDevice success" << endl;
    } else {
        cout << "SelectInputDevice failed, ret:" << ret << endl;
    }
    return ret;
}

int32_t PlaybackTest::StartPlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    if (!audioRenderer_->Start()) {
        AUDIO_ERR_LOG("Audio renderer start failed.");
        return -1;
    }
    cout << "Start play. current play device: " << isSpkRemote_ << "(0 : local, 1 : remote)" << endl;
    return 0;
}

int32_t PlaybackTest::StopPlay()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Audiorenderer init failed.");
        return -1;
    }
    if (!audioRenderer_->Stop()) {
        AUDIO_ERR_LOG("Audio renderer stop failed.");
        return -1;
    }
    return 0;
}

int32_t PlaybackTest::ReleaseRenderer()
{
    StopPlay();
    isSpkFast_ = false;
    if (audioRenderer_ != nullptr && !audioRenderer_->Release()) {
        AUDIO_ERR_LOG("Audio renderer release failed.");
        cout << "Audio render release failed" << endl;
    }
    audioRenderer_ = nullptr;
    CloseSpkFile();
    AUDIO_INFO_LOG("Audio renderer release success.");
    return 0;
}

void PlaybackTest::OnReadData(size_t length)
{
    BufferDesc bufDesc;
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer is nullptr.");
        return;
    }
    int32_t ret = audioCapturer_->GetBufferDesc(bufDesc);
    if (ret != 0 || bufDesc.buffer == nullptr || bufDesc.bufLength == 0) {
        AUDIO_ERR_LOG("Get buffer desc failed. On read data.");
        return;
    }
    if (micWavFile_ == nullptr) {
        AUDIO_ERR_LOG("micWavFile_ is nullptr.");
        return;
    }
    size_t cnt = fwrite(bufDesc.buffer, 1, bufDesc.bufLength, micWavFile_);
    if (cnt != bufDesc.bufLength) {
        AUDIO_ERR_LOG("fwrite fail, cnt %{public}zu, bufLength %{public}zu.", cnt, bufDesc.bufLength);
        return;
    }
    audioCapturer_->Enqueue(bufDesc);
}

bool PlaybackTest::OpenMicFile(const std::string &micFilePath)
{
    if (micWavFile_ != nullptr) {
        AUDIO_ERR_LOG("Mic file has been opened, micFilePath %{public}s.", micFilePath.c_str());
        return true;
    }

    char path[PATH_MAX] = { 0x00 };
    if ((strlen(micFilePath.c_str()) > PATH_MAX) || (realpath(micFilePath.c_str(), path) == nullptr)) {
        AUDIO_ERR_LOG("micFilePath is not valid.");
        return false;
    }
    AUDIO_INFO_LOG("mic path = %{public}s.", path);
    micWavFile_ = fopen(path, "ab+");
    if (micWavFile_ == nullptr) {
        AUDIO_ERR_LOG("Unable to open wave file");
        return false;
    }
    return true;
}

void PlaybackTest::CloseMicFile()
{
    if (micWavFile_ != nullptr) {
        fclose(micWavFile_);
        micWavFile_ = nullptr;
    }
}

int32_t PlaybackTest::InitCapturer(bool isFast)
{
    AudioStandard::AudioCapturerOptions capturerOptions = {
        {
            AudioStandard::AudioSamplingRate::SAMPLE_RATE_48000,
            AudioStandard::AudioEncodingType::ENCODING_PCM,
            AudioStandard::AudioSampleFormat::SAMPLE_S16LE,
            AudioStandard::AudioChannel::STEREO,
        },
        {
            AudioStandard::SourceType::SOURCE_TYPE_MIC,
            isFast ? AudioStandard::STREAM_FLAG_FAST : AudioStandard::STREAM_FLAG_NORMAL,
        }
    };
    audioCapturer_ = AudioStandard::AudioCapturer::Create(capturerOptions);
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("Audio capturer create failed.");
        return -1;
    }
    std::string path = "/data/mic.pcm";
    OpenMicFile(path);
    int32_t ret = audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set capture mode callback fail, ret %{public}d.", ret);
    ret = audioCapturer_->SetCapturerReadCallback(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Client test save data callback fail, ret %{public}d.", ret);
    AUDIO_INFO_LOG("Audio capturer create success.");
    isMicFast_ = isFast;
    return 0;
}

int32_t PlaybackTest::StartCapture()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer init failed.");
        return -1;
    }
    if (!audioCapturer_->Start()) {
        AUDIO_ERR_LOG("Audio capture start failed.");
        return -1;
    }
    return 0;
}

int32_t PlaybackTest::StopCapture()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("audioCapturer init failed.");
        return -1;
    }
    if (!audioCapturer_->Stop()) {
        AUDIO_ERR_LOG("Audio capture stop failed.");
        return -1;
    }
    return 0;
}

int32_t PlaybackTest::ReleaseCapture()
{
    isMicFast_ = false;
    if (audioCapturer_ != nullptr && !audioCapturer_->Release()) {
        AUDIO_ERR_LOG("Audio capturer release failed.");
        cout << "Audio capturer release failed" << endl;
    }
    audioCapturer_ = nullptr;
    CloseMicFile();
    AUDIO_INFO_LOG("Audio capturer release success.");
    return 0;
}
using CallTestOptCodeFunc = int32_t (*)(std::shared_ptr<PlaybackTest> playTest);

bool SetSysPara(const std::string &key, int32_t &value)
{
    auto res = SetParameter(key.c_str(), std::to_string(value).c_str());
    if (res < 0) {
        AUDIO_WARNING_LOG("SetSysPara fail, key:%{public}s res:%{public}d", key.c_str(), res);
        return false;
    }
    AUDIO_INFO_LOG("SetSysPara success.");
    return true;
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
    cout << "Supported Functionalities:" << endl;
    cout << "================================Usage=======================================" << endl << endl;
    cout << "  0: Init local spk." << endl;
    cout << "  1: Init remote spk." << endl;
    cout << "  2: Start play." << endl;
    cout << "  3: Stop play." << endl;
    cout << "  4: Switch play device between local and remote." << endl;
    cout << "  5: Release spk." << endl;
    cout << "  6: Init local mic." << endl;
    cout << "  7: Init remote mic." << endl;
    cout << "  8: Start record." << endl;
    cout << "  9: Stop record." << endl;
    cout << "  10: Switch record device between local and remote." << endl;
    cout << "  11: Release mic." << endl;
    cout << "  12: exit demo." << endl;
    cout << " Please input your choice: " << endl;
}

int32_t InitPlayback(std::shared_ptr<PlaybackTest> playTest, bool isRemote, bool isFast)
{
    cout << "Init renderer." << endl;
    cout << "--isRemote: " << isRemote << "-- --isSpkFast: " << isFast << " --" <<endl;
    if (playTest == nullptr) {
        cout << "Play test is nullptr, init spk error." << endl;
        return -1;
    }
    if (isRemote) {
        cout << "Use remote device, select remote spk." << endl;
        int32_t ret = playTest->SwitchDevice(OUTPUT_DEVICE);
        if (ret != 0) {
            cout << "find no remote device." << endl;
            return -1;
        }
    }
    int32_t ret = playTest->InitRenderer(isFast);
    if (ret != 0) {
        cout << "Init renderer error!" << endl;
        return -1;
    }
    if (isRemote) {
        playTest->SetSpkRemote(true);
    }
    cout << "Init renderer success." << endl << endl;
    return 0;
}

int32_t ReleasePlayback(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, release spk error." << endl;
        return -1;
    }
    int32_t ret = playTest->ReleaseRenderer();
    if (ret != 0) {
        cout << "Release renderer error!" << endl;
        return -1;
    }
    playTest->SetSpkRemote(false);
    cout << "Release renderer success." << endl << endl;
    return 0;
}

int32_t StartPlay(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, start play error." << endl;
        return -1;
    }
    int32_t ret = playTest->StartPlay();
    if (ret != 0) {
        cout << "Start play error!" << endl;
        return -1;
    }
    cout << "Start play success." << endl << endl;
    return 0;
}

int32_t StopPlay(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, stop play error." << endl;
        return -1;
    }
    int32_t ret = playTest->StopPlay();
    if (ret != 0) {
        cout << "Stop play error!" << endl;
        return -1;
    }
    cout << "Stop play success." << endl << endl;
    return 0;
}

int32_t SwitchPlayDevice(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, switch play device error." << endl;
        return -1;
    }
    int32_t ret = playTest->SwitchDevice(OUTPUT_DEVICE);
    if (ret != 0) {
        cout << "Switch play device error!" << endl;
        return -1;
    }
    cout << "Switch play device success." << endl << endl;
    return 0;
}

int32_t InitMic(std::shared_ptr<PlaybackTest> playTest, bool isRemote, bool isFast)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, init mic error." << endl;
        return -1;
    }
    if (isRemote) {
        cout << "Use remote device, select remote mic." << endl;
        int32_t ret = playTest->SwitchDevice(INPUT_DEVICE);
        if (ret != 0) {
            cout << "find no remote device." << endl;
            return -1;
        }
    }
    int32_t ret = playTest->InitCapturer(isFast);
    if (ret != 0) {
        cout << "Init capturer error!" << endl;
        return -1;
    }
    if (isRemote) {
        playTest->SetMicRemote(true);
    }
    cout << "Init capturer success." << endl << endl;
    return 0;
}

int32_t ReleaseMic(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, release capturer error." << endl;
        return -1;
    }
    int32_t ret = playTest->ReleaseCapture();
    if (ret != 0) {
        cout << "Release capturer error!" << endl;
        return -1;
    }
    playTest->SetMicRemote(false);
    cout << "Release capturer success." << endl << endl;
    return 0;
}

int32_t StartCapture(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, start capturer error." << endl;
        return -1;
    }
    int32_t ret = playTest->StartCapture();
    if (ret != 0) {
        cout << "Start capturer error!" << endl;
        return -1;
    }
    cout << "Start capturer success." << endl << endl;
    return 0;
}

int32_t StopCapture(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, stop capturer error." << endl;
        return -1;
    }
    int32_t ret = playTest->StopCapture();
    if (ret != 0) {
        cout << "Stop capturer error!" << endl;
        return -1;
    }
    cout << "Stop capturer success." << endl << endl;
    return 0;
}

int32_t SwitchCaptureDevice(std::shared_ptr<PlaybackTest> playTest)
{
    if (playTest == nullptr) {
        cout << "Play test is nullptr, switch capture device error." << endl;
        return -1;
    }
    int32_t ret = playTest->SwitchDevice(INPUT_DEVICE);
    if (ret != 0) {
        cout << "Switch capture device error!" << endl;
        return -1;
    }
    cout << "Switch capture device success." << endl << endl;
    return 0;
}

std::map<int32_t, CallTestOptCodeFunc> g_optFuncMap = {
    {START_SPK, StartPlay},
    {STOP_SPK, StopPlay},
    {SWITCH_SPK, SwitchPlayDevice},
    {RELEASE_SPK, ReleasePlayback},
    {START_MIC, StartCapture},
    {STOP_MIC, StopCapture},
    {SWITCH_MIC, SwitchCaptureDevice},
    {RELEASE_MIC, ReleaseMic},
};

void Loop(std::shared_ptr<PlaybackTest> playTest)
{
    bool isProcTestRun = true;
    while (isProcTestRun) {
        PrintUsage();
        AudioOptCode optCode = INVALID_OPERATION;
        int32_t res = GetUserInput();
        if (g_OptStrMap.count(res)) {
            optCode = static_cast<AudioOptCode>(res);
        }
        switch (optCode) {
            case INIT_LOCAL_SPK:
                InitPlayback(playTest, false, false);
                break;
            case INIT_REMOTE_SPK:
                InitPlayback(playTest, true, false);
                break;
            case INIT_LOCAL_MIC:
                InitMic(playTest, false, false);
                break;
            case INIT_REMOTE_MIC:
                InitMic(playTest, true, false);
                break;
            case EXIT_DEMO:
                ReleasePlayback(playTest);
                ReleaseMic(playTest);
                isProcTestRun = false;
                cout << "exit demo..." << endl;
                break;
            default:
                auto it = g_optFuncMap.find(optCode);
                if (it != g_optFuncMap.end() && it->second != nullptr) {
                    CallTestOptCodeFunc &func = it->second;
                    cout << (*func)(playTest) << endl;
                    break;
                }
                cout << "Invalid input: " << optCode << endl;
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
    cout << "[Fast Audio Stream Test App]" << endl << endl;
    AUDIO_INFO_LOG("oh fast audio stream test.");
    std::shared_ptr<PlaybackTest> playTest = std::make_shared<PlaybackTest>();
    int32_t val = 1;
    std::string key = "persist.multimedia.audio.mmap.enable";
    SetSysPara(key, val);
    Loop(playTest);
    return 0;
}