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
#define LOG_TAG "OhAudioCapturerLowlatencyTest"
#endif

#include <ostream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "native_audiostreambuilder.h"
#include <native_audiocapturer.h>
#include <thread>
#include <chrono>
#include <securec.h>
#include "audio_info.h"
#include "audio_system_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
enum OperationCode : int32_t {
    INVALID_OPERATION = -1,
    RELEASE_SPK_OH = 1,
    START_SPK_OH = 2,
    STOP_SPK_OH  = 3,
    START_LOOP_TEST = 4,
    END_LOOP_TEST = 5,
    INIT_LOCAL_MIC_OH = 6,
    INIT_REMOTE_MIC_OH = 7,
    START_MIC_OH = 8,
    PAUSE_MIC_OH = 9,
    STOP_MIC_OH = 10,
    RELEASE_MIC_OH = 11,
    EXIT_INTERACTIVE_TEST = 20,
};

enum AudioOHTestType : int32_t {
    INVALID_OH_TEST = 0,
    INTERACTIVE_RUN_MIC_TEST = 1,
    INTERACTIVE_RUN_LOOP = 2,
    EXIT_OH_TEST = 7,
};
static constexpr size_t CACHE_BUFFER_SIZE = 960;
int64_t g_stampTime = 0;
OperationCode g_optCode = INVALID_OPERATION;
}

class AudioNdkTest;

shared_ptr<AudioNdkTest> g_audioNdkTest = nullptr;
string g_spkfilePath = "";
const string MIC_FILE_PATH = "/data/data/mic_oh.pcm";
FILE *g_spkWavFile = nullptr;
FILE *g_micPcmFile = nullptr;
OH_AudioStreamBuilder* builder;
OH_AudioCapturer* audioCapturer;
OH_AudioRenderer* audioRenderer;
OH_AudioStream_Result ret;
int32_t g_latencyMode = 1;
static constexpr int64_t SECOND_TO_NANOSECOND = 1000000000;
int32_t g_loopCount = -1; // for loop
unique_ptr<uint8_t[]> g_byteBuffer = nullptr;
BufferDesc g_cacheBuffer = {nullptr, 0, 0};

string CallStartSpk();
string CallStopSpk();
string CallReleaseSpk();
string ConfigMicTest(bool isRemote);
string CallStartMic();
string CallPauseMic();
string CallStopMic();
string CallReleaseMic();
using CallTestOperationFunc = string (*)();

map<int32_t, string> g_audioNdkTestType = {
    {INTERACTIVE_RUN_MIC_TEST, "Interactive run mic oh test"},
    {INTERACTIVE_RUN_LOOP, "Roundtrip latency test"},
    {EXIT_OH_TEST, "Exit audio oh test"},
};

map<int32_t, string> g_interactiveOptStrMap = {
    {START_SPK_OH, "call start spk oh"},
    {STOP_SPK_OH, "call stop spk oh"},
    {RELEASE_SPK_OH, "release spk oh"},
    {START_LOOP_TEST, "start loop"},
    {END_LOOP_TEST, "end loop"},
    {INIT_LOCAL_MIC_OH, "call local mic init oh"},
    {INIT_REMOTE_MIC_OH, "call remote mic init oh"},
    {START_MIC_OH, "call start mic oh"},
    {PAUSE_MIC_OH, "call pause mic oh"},
    {STOP_MIC_OH, "call stop mic oh"},
    {RELEASE_MIC_OH, "release mic oh"},
    {EXIT_INTERACTIVE_TEST, "exit interactive run test"},
};

map<int32_t, CallTestOperationFunc> g_interactiveOptFuncMap = {
    {START_SPK_OH, CallStartSpk},
    {STOP_SPK_OH, CallStopSpk},
    {RELEASE_SPK_OH, CallReleaseSpk},
    {START_MIC_OH, CallStartMic},
    {PAUSE_MIC_OH, CallPauseMic},
    {STOP_MIC_OH, CallStopMic},
    {RELEASE_MIC_OH, CallReleaseMic}
};

class AudioNdkTest {
public:
    AudioNdkTest() = default;
    ~AudioNdkTest() = default;

    int32_t InitSpk(int32_t loopCount, bool isRemote);
    bool StartSpk();
    bool StopSpk();
    bool ReleaseSpk();
    int32_t InitMic(bool isRemote);
    bool StartMic();
    bool PauseMic();
    bool StopMic();
    bool ReleaseMic();
    void SelectDevice(DeviceRole deviceRole);
};

void InitSignalBuffer(void* signalSoundBuffer, int32_t bufferLen)
{
    int ret = memset_s(signalSoundBuffer, bufferLen, 0, bufferLen);
    if (ret != EOK) {
        return;
    }
    const int channels = 2; // 2 channels
    const int samplePerChannel = 96 / channels; // 96 for 1ms
    int16_t *signalData = static_cast<int16_t *>(signalSoundBuffer);
    int16_t bound = 10;
    for (int idx = 0; idx < samplePerChannel; idx++) {
        signalData[channels * idx] = bound + static_cast<int16_t>(sinf(2.0f * static_cast<float>(M_PI) * idx /
            samplePerChannel) * (SHRT_MAX - bound));
        for (int c = 1; c < channels; c++) {
            signalData[channels * idx + c] = signalData[channels * idx];
        }
    }
}

static int32_t AudioRendererOnWriteData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    g_loopCount++;
    int32_t periodCount = g_loopCount % 400; // 400 * 0.005 = 2s
    if (periodCount == 0) {
        InitSignalBuffer(buffer, bufferLen); // set signal data
        int64_t temp = ClockTime::GetCurNano() - g_stampTime;
        std::cout << "client read-write latency:" << (temp / AUDIO_MS_PER_SECOND) << " us" << std::endl;
        return 0;
    }
    int32_t keepQuiteHold = 50;
    if (periodCount > keepQuiteHold) {
        return 0;
    }
    // copy mic data in the cache buffer
    int ret = memcpy_s(buffer, bufferLen, static_cast<void *>(g_cacheBuffer.buffer), g_cacheBuffer.bufLength);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memcpy_s failed.");
    }
    return 0;
}

static int32_t AudioCapturerOnReadData(OH_AudioCapturer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    if (g_optCode == START_LOOP_TEST) {
        int32_t cnt = fwrite(buffer, 1, bufferLen, g_micPcmFile);
        CHECK_AND_RETURN_RET_LOG(cnt == bufferLen, ERR_WRITE_FAILED,
            "%{public}s fwrite fail, cnt %{public}d, bufLength %{public}d.", __func__, cnt, bufferLen);
        int ret = memcpy_s(static_cast<void *>(g_cacheBuffer.buffer), bufferLen, buffer, bufferLen);
        if (ret != EOK) {
            AUDIO_WARNING_LOG("memcpy_s failed.");
        }
        g_stampTime = ClockTime::GetCurNano();
        return 0;
    }

    size_t count = 1;
    if (fwrite(buffer, bufferLen, count, g_micPcmFile) != count) {
        cout <<"buffer fwrite err";
    }
    return 0;
}


int32_t AudioNdkTest::InitSpk(int32_t loopCount, bool isRemote)
{
    if (loopCount < 0) {
        g_loopCount = 1; // loop once
    } else if (loopCount == 0) {
        g_loopCount = -1; // infinite loop
    } else {
        g_loopCount = loopCount;
    }

    if (isRemote) {
        SelectDevice(OUTPUT_DEVICE);
    }

    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    ret = OH_AudioStreamBuilder_Create(&builder, type);
    cout << "createcallback ret:  " << ret << endl;
    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, STEREO);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)g_latencyMode);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    ret = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    cout << "setcallback ret: " << ret << endl;
    // 3. create OH_AudioRenderer
    ret = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    cout << "create renderer client, ret: " << ret << endl;
    return SUCCESS;
}

bool AudioNdkTest::ReleaseSpk()
{
    if (!audioRenderer) {
        ret = OH_AudioRenderer_Release(audioRenderer);
        cout << "Renderer release ret: " << ret << endl;
    }

    if (!builder) {
        ret = OH_AudioStreamBuilder_Destroy(builder);
        cout << "Renderer destroy builder ret: " << ret << endl;
    }
    return true;
}

bool AudioNdkTest::StartSpk()
{
    ret = OH_AudioRenderer_Start(audioRenderer);
    cout << "Renderer start ret: " << ret << endl;
    return true;
}

bool AudioNdkTest::StopSpk()
{
    ret = OH_AudioRenderer_Stop(audioRenderer);
    cout << "Renderer stop ret: " << ret << endl;
    return true;
}

int32_t AudioNdkTest::InitMic(bool isRemote)
{
    if (isRemote) {
        SelectDevice(INPUT_DEVICE);
    }
    // 1. create
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    ret = OH_AudioStreamBuilder_Create(&builder, type);
    cout <<"create capturer builder: " << ret << endl;

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, STEREO);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)g_latencyMode);

    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    ret = OH_AudioStreamBuilder_SetCapturerCallback(builder, callbacks, nullptr);
    cout << "setcallback: " << ret << endl;

    // 3. create OH_AudioCapturer
    ret = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    cout << "create capturer client, ret: " << ret << endl;

    return SUCCESS;
}

bool AudioNdkTest::StartMic()
{
    ret = OH_AudioCapturer_Start(audioCapturer);
    cout << "start ret: " << ret << endl;
    return true;
}

bool AudioNdkTest::PauseMic()
{
    ret = OH_AudioCapturer_Pause(audioCapturer);
    cout << "pause ret: " << ret << endl;
    return true;
}

bool AudioNdkTest::StopMic()
{
    ret = OH_AudioCapturer_Stop(audioCapturer);
    cout << "stop ret: " << ret << endl;
    return true;
}

bool AudioNdkTest::ReleaseMic()
{
    if (!audioCapturer) {
        ret = OH_AudioCapturer_Release(audioCapturer);
        cout << "release ret: " << ret << endl;
    }

    if (!builder) {
        ret = OH_AudioStreamBuilder_Destroy(builder);
        cout << "destroy builder ret: " << ret << endl;
    }
    return true;
}

void AudioNdkTest::SelectDevice(DeviceRole deviceRole)
{
    AudioSystemManager *manager = AudioSystemManager::GetInstance();
    if (manager == nullptr) {
        cout << "Get AudioSystemManager failed" << endl;
        return;
    }

    vector<sptr<AudioDeviceDescriptor>> devices;
    if (deviceRole == OUTPUT_DEVICE) {
        devices = manager->GetDevices(DISTRIBUTED_OUTPUT_DEVICES_FLAG);
    } else {
        devices = manager->GetDevices(DISTRIBUTED_INPUT_DEVICES_FLAG);
    }
    if (devices.size() != 1) {
        cout << "GetDevices failed, unsupported size:" << devices.size() << endl;
        return;
    }
    cout << "using device:" << devices[0]->networkId_ << endl;

    int32_t ret = 0;
    if (deviceRole == OUTPUT_DEVICE) {
        sptr<AudioRendererFilter> filter = new AudioRendererFilter();
        filter->uid = getuid();
        filter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
        ret = manager->SelectOutputDevice(filter, devices);
    } else {
        sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
        filter->uid = getuid();
        filter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
        filter->capturerInfo.capturerFlags = STREAM_FLAG_FAST;
        ret = manager->SelectInputDevice(filter, devices);
    }

    if (ret == SUCCESS) {
        cout << "SelectDevice seccess" << endl;
    } else {
        cout << "SelectDevice failed, ret:" << ret << endl;
    }
}

bool OpenSpkFile()
{
    if (g_spkWavFile != nullptr) {
        AUDIO_ERR_LOG("Spk file has been opened, g_spkfilePath %{public}s", g_spkfilePath.c_str());
        return true;
    }

    char path[PATH_MAX] = { 0x00 };
    if ((strlen(g_spkfilePath.c_str()) > PATH_MAX) || (realpath(g_spkfilePath.c_str(), path) == nullptr)) {
        return false;
    }
    AUDIO_INFO_LOG("spk path = %{public}s", path);
    g_spkWavFile = fopen(path, "rb");
    if (g_spkWavFile == nullptr) {
        AUDIO_ERR_LOG("Unable to open wave file");
        return false;
    }
    return true;
}

void CloseSpkFile()
{
    if (g_spkWavFile != nullptr) {
        fclose(g_spkWavFile);
        g_spkWavFile = nullptr;
    }
}

bool OpenMicFile()
{
    if (g_micPcmFile != nullptr) {
        AUDIO_ERR_LOG("Mic file has been opened, MIC_FILE_PATH %{public}s", MIC_FILE_PATH.c_str());
        return true;
    }

    AUDIO_INFO_LOG("mic path = %{public}s", MIC_FILE_PATH.c_str());
    g_micPcmFile = fopen(MIC_FILE_PATH.c_str(), "ab+");
    if (g_micPcmFile == nullptr) {
        AUDIO_ERR_LOG("Unable to open wave file");
        return false;
    }
    return true;
}

void CloseMicFile()
{
    if (g_micPcmFile != nullptr) {
        fclose(g_micPcmFile);
        g_micPcmFile = nullptr;
    }
}

void PrintInteractiveUsage()
{
    cout << endl << "======================= InteractiveRunTestSelect ============================" << endl;
    cout << "You can respond to instructions for corresponding option:" << endl;
    for (auto it = g_interactiveOptStrMap.begin(); it != g_interactiveOptStrMap.end(); it ++) {
        cout << "\t enter " << it->first << " : " << it->second << endl;
    }
}

void PrintNdkTestUsage()
{
    cout << endl << "========================== NdkTestSelect ================================" << endl;
    cout << "You can respond to instructions for corresponding test:" << endl;
    for (auto it = g_audioNdkTestType.begin(); it != g_audioNdkTestType.end(); it ++) {
        cout << it->first << ". " << it->second << endl;
    }
}

int32_t GetUserInput()
{
    int32_t res = -1;
    size_t count = 3;
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

void AutoRunMic()
{
    cout << "Auto run mic oh test enter, please input recordTimeS:" << endl;
    int32_t recordTimeS = GetUserInput();
    if (!OpenMicFile()) {
        cout << "open mic file path failed!" << endl;
        return;
    }

    if (g_audioNdkTest->InitMic(false) != SUCCESS) {
        cout << "Mic init failed!" << endl;
        return;
    }

    do {
        if (!g_audioNdkTest->StartMic()) {
            cout << "Mic start failed!" << endl;
            break;
        }

        cout << "wait " << recordTimeS << "s for capture frame..." << endl;
        ClockTime::RelativeSleep(recordTimeS * SECOND_TO_NANOSECOND);
        cout << "AutoRunMic end" << endl;

        if (!g_audioNdkTest->StopMic()) {
            cout << "Mic stop failed!" << endl;
            break;
        }
    } while (false);

    if (!g_audioNdkTest->ReleaseMic()) {
        cout << "Mic release failed!" << endl;
    }
    CloseMicFile();
}

string ConfigMicTest(bool isRemote)
{
    if (!OpenMicFile()) {
        cout << "Open mic file path failed!" << endl;
        return "Open mic pcm file fail";
    }

    int32_t ret = g_audioNdkTest->InitMic(isRemote);
    if (ret != SUCCESS) {
        return "Mic init failed";
    }
    return "Mic init SUCCESS";
}

string CallStartSpk()
{
    if (!g_audioNdkTest->StartSpk()) {
        return "Spk start failed";
    }
    return "Spk start SUCCESS";
}

string CallStopSpk()
{
    if (!g_audioNdkTest->StopSpk()) {
        return "Spk stop failed";
    }
    return "Spk stop SUCCESS";
}

string CallReleaseSpk()
{
    if (!g_audioNdkTest->ReleaseSpk()) {
        return "Spk release failed";
    }
    CloseSpkFile();
    return "Spk release SUCCESS";
}

string CallStartMic()
{
    if (!g_audioNdkTest->StartMic()) {
        return "Mic start failed";
    }
    return "Mic start SUCCESS";
}

string CallPauseMic()
{
    if (!g_audioNdkTest->PauseMic()) {
        return "Mic pause failed";
    }
    return "Mic pause SUCCESS";
}

string CallStopMic()
{
    if (!g_audioNdkTest->StopMic()) {
        return "Mic stop failed";
    }
    return "Mic stop SUCCESS";
}

string CallReleaseMic()
{
    if (!g_audioNdkTest->ReleaseMic()) {
        return "Mic release failed";
    }
    CloseMicFile();
    return "Mic release SUCCESS";
}

string StartLoopTest()
{
    cout << ConfigMicTest(false) << endl;
    cout << CallStartMic() << endl;

    int32_t ret = g_audioNdkTest->InitSpk(0, false);
    if (ret != SUCCESS) {
        CallReleaseMic();
        return "init spk failed";
    } else {
        std::cout << "init spk success" << endl;
    }

    cout << CallStartSpk() << endl;
    return "StartLoopTest success!";
}

string EndLoopTest()
{
    cout << CallStopSpk() << endl;
    cout << CallReleaseSpk() << endl;
    cout << CallStopMic() << endl;
    cout << CallReleaseMic() << endl;
    return "EndLooptest";
}

void InitCachebuffer()
{
    g_byteBuffer = make_unique<uint8_t []>(CACHE_BUFFER_SIZE);
    g_cacheBuffer.buffer = g_byteBuffer.get();
    g_cacheBuffer.bufLength = CACHE_BUFFER_SIZE;
    g_cacheBuffer.dataLength = CACHE_BUFFER_SIZE;
}

void InteractiveRun()
{
    InitCachebuffer();
    cout << "Interactive run oh test enter." << endl;
    bool isInteractiveRun = true;
    while (isInteractiveRun) {
        PrintInteractiveUsage();
        int32_t res = GetUserInput();
        if (g_interactiveOptStrMap.count(res)) {
            g_optCode = static_cast<OperationCode>(res);
        }
        switch (g_optCode) {
            case EXIT_INTERACTIVE_TEST:
                isInteractiveRun = false;
                break;
            case START_LOOP_TEST:
                cout << StartLoopTest() << endl;
                break;
            case END_LOOP_TEST:
                cout << EndLoopTest() << endl;
                break;
            case INIT_LOCAL_MIC_OH:
                cout << ConfigMicTest(false) << endl;
                break;
            case INIT_REMOTE_MIC_OH:
                cout << ConfigMicTest(true) << endl;
                break;
            default:
                auto it = g_interactiveOptFuncMap.find(g_optCode);
                if (it != g_interactiveOptFuncMap.end() && it->second != nullptr) {
                    CallTestOperationFunc &func = it->second;
                    cout << (*func)() << endl;
                    break;
                }
                cout << "Invalid input :" << g_optCode << endl;
                break;
        }
    }
    cout << "Interactive run oh test end." << endl;
}

} // namespace AudioStandard
} // namespace OHOS

using namespace OHOS::AudioStandard;
int main()
{
    cout << "Please input 1 or 0 (1 means low latency, 0 means ordinary): " << endl;
    cin >> g_latencyMode;

    bool isNdkTestRun = true;
    g_audioNdkTest = make_shared<AudioNdkTest>();

    while (isNdkTestRun) {
        PrintNdkTestUsage();
        AudioOHTestType ohTestType = INVALID_OH_TEST;
        int32_t res = GetUserInput();
        if (g_audioNdkTestType.count(res)) {
            ohTestType = static_cast<AudioOHTestType>(res);
        }
        switch (ohTestType) {
            case INTERACTIVE_RUN_MIC_TEST:
                cout << "OH run mic test, ohTestType: " << ohTestType << endl;
                InteractiveRun();
                break;
            case INTERACTIVE_RUN_LOOP:
                cout << "OH run loop test, ohTestType: " << ohTestType << endl;
                InteractiveRun();
                break;
            case EXIT_OH_TEST:
                isNdkTestRun = false;
                cout << "exit OH test, ohTestType: " << ohTestType << endl;
                break;
            default:
                cout << "invalid input, ohTestType: " << ohTestType << endl;
                break;
        }
    }
    cout << "Test end!" << endl;
    return 0;
}
