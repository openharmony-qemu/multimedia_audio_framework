/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "hpae_manager_stream_fuzzer.h"

#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include "audio_errors.h"
#include "test_case_common.h"
#include "hpae_audio_service_dump_callback_unit_test.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 14;
static int32_t NUM_2 = 2;
static std::string g_rootPath = "/data/";
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;

typedef void (*TestFuncs)();

vector<AudioSpatializationSceneType> AudioSpatializationSceneTypeVec {
    SPATIALIZATION_SCENE_TYPE_DEFAULT,
    SPATIALIZATION_SCENE_TYPE_MUSIC,
    SPATIALIZATION_SCENE_TYPE_MOVIE,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
    SPATIALIZATION_SCENE_TYPE_MAX,
};

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

vector<HpaeStreamClassType> HpaeStreamClassTypeVec = {
    HPAE_STREAM_CLASS_TYPE_INVALID,
    HPAE_STREAM_CLASS_TYPE_PLAY,
    HPAE_STREAM_CLASS_TYPE_RECORD,
};

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

void WaitForMsgProcessing(std::shared_ptr<HPAE::HpaeManager> &hpaeManager)
{
    int waitCount = 0;
    while (hpaeManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
}

HPAE::HpaeStreamInfo GetRenderStreamInfo()
{
    HPAE::HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

AudioModuleInfo GetSourceAudioModeInfo(std::string name = "mic")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + "source_" + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_FILE_SOURCE);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

HPAE::HpaeStreamInfo GetCaptureStreamInfo()
{
    HPAE::HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_RECORD;
    return streamInfo;
}

void InitFuzzTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();
    sleep(1);
    hpaeManager_->IsRunning();
    hpaeManager_->DeInit();
    hpaeManager_->IsInit();
    sleep(1);
    hpaeManager_->IsRunning();
    hpaeManager_->DeInit();
    hpaeManager_ = nullptr;
}

void HpaeRenderStreamManagerFuzzTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    bool mute = GetData<bool>();
    bool isSync = GetData<bool>();
    hpaeManager_->SetSinkMute(audioModuleInfo.name, true, true);
    hpaeManager_->SetSinkMute(audioModuleInfo.name, false, true);
    bool isSuspend = GetData<bool>();
    hpaeManager_->SuspendAudioDevice(audioModuleInfo.name, true);
    hpaeManager_->SuspendAudioDevice(audioModuleInfo.name, false);
    hpaeManager_->GetAllSinkInputs();
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);

    hpaeManager_->GetAllSinkInputs();
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeRenderStreamManagerFuzzTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    bool mute = GetData<bool>();
    bool isSync = GetData<bool>();
    hpaeManager_->SetSinkMute(audioModuleInfo.name, mute, isSync);
    bool isSuspend = GetData<bool>();
    hpaeManager_->SuspendAudioDevice(audioModuleInfo.name, isSuspend);

    hpaeManager_->GetAllSinkInputs();
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    hpaeManager_->Release(streamClassType, sessionId);

    hpaeManager_->GetAllSinkInputs();
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeRenderStreamManagerFuzzTest3()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];

    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    hpaeManager_->Start(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    int32_t syncId = GetData<int32_t>();
    hpaeManager_->StartWithSyncId(streamClassType, sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    hpaeManager_->Pause(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    hpaeManager_->Stop(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    hpaeManager_->Release(streamClassType, sessionId);
    hpaeManager_->Release(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeRenderStreamManagerFuzzTest4()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);

    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    int32_t syncId = GetData<int32_t>();
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeCaptureStreamManagerFuzzTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fuzzPortId = GetData<int32_t>();
    bool mute = GetData<bool>();
    hpaeManager_->SetSourceOutputMute(fuzzPortId, mute);
    hpaeManager_->GetAllSourceOutputs();
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeCaptureStreamManagerFuzzTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);

    hpaeManager_->SetSourceOutputMute(portId, true);

    callback->GetSetSourceOutputMuteResult();
    hpaeManager_->SetSourceOutputMute(portId, false);

    hpaeManager_->GetAllSourceOutputs();

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeCaptureStreamManagerFuzzTest3()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);

    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeRenderStreamManagerMoveFuzzTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    float volume = GetData<float>();
    hpaeManager_->SetClientVolume(streamInfo.sessionId, volume);
    float loudnessGain = GetData<float>();
    hpaeManager_->SetLoudnessGain(streamInfo.sessionId, loudnessGain);
    int32_t rate = GetData<int32_t>();
    hpaeManager_->SetRate(streamInfo.sessionId, rate);
    WaitForMsgProcessing(hpaeManager_);

    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);

    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeRenderStreamManagerMoveFuzzTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    hpaeManager_->SetClientVolume(sessionId, volume);
    float loudnessGain = GetData<float>();
    hpaeManager_->SetLoudnessGain(sessionId, loudnessGain);
    int32_t rate = GetData<int32_t>();
    hpaeManager_->SetRate(sessionId, rate);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    uint32_t sinkIndex = GetData<uint32_t>();
    hpaeManager_->MoveSinkInputByIndexOrName(sessionId, sinkIndex, "Speaker_File1");
    hpaeManager_->Start(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    sinkIndex = GetData<uint32_t>();
    hpaeManager_->MoveSinkInputByIndexOrName(sessionId, sinkIndex, "Speaker_File");
    hpaeManager_->Pause(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    uint32_t sinkIndex2 = GetData<uint32_t>();
    sinkIndex = GetData<uint32_t>();
    hpaeManager_->MoveSinkInputByIndexOrName(sessionId, sinkIndex, "Speaker_File1");
    hpaeManager_->Stop(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    sinkIndex = GetData<uint32_t>();
    hpaeManager_->MoveSinkInputByIndexOrName(sessionId, sinkIndex, "Speaker_File");
    hpaeManager_->Release(streamClassType, sessionId);

    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeCaptureStreamManagerMoveTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    AudioModuleInfo audioModuleInfo1 = GetSourceAudioModeInfo("mic1");
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1");
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 0, "mic");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);

    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 0, "mic");
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeCaptureStreamManagerMoveTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    AudioModuleInfo audioModuleInfo1 = GetSourceAudioModeInfo("mic1");
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    uint32_t sessionId = GetData<uint32_t>();
    HpaeSessionInfo sessionInfo;
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    uint32_t sourceIndex = GetData<uint32_t>();
    hpaeManager_->MoveSourceOutputByIndexOrName(sessionId, sourceIndex, "mic1");
    hpaeManager_->Start(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);

    sourceIndex = GetData<uint32_t>();
    hpaeManager_->MoveSourceOutputByIndexOrName(sessionId, sourceIndex, "mic");
    hpaeManager_->Pause(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    sourceIndex = GetData<uint32_t>();
    hpaeManager_->MoveSourceOutputByIndexOrName(sessionId, sourceIndex, "mic1");
    hpaeManager_->Stop(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    sourceIndex = GetData<uint32_t>();
    hpaeManager_->MoveSourceOutputByIndexOrName(sessionId, sourceIndex, "mic");
    hpaeManager_->Release(streamClassType, sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamClassType, sessionId, sessionInfo);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerDumpStreamInfoTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();

    hpaeManager_->IsInit();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);

    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(sinkAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sinkPortId = callback->GetPortId();
    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(sourceAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sourcePortId = callback->GetPortId();

    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);

    HpaeStreamInfo rendererStreamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(rendererStreamInfo);
    hpaeManager_->DumpSinkInputsInfo();
    WaitForMsgProcessing(hpaeManager_);
    dumpCallback->GetSinkInputsSize();
    hpaeManager_->ShouldNotSkipProcess(rendererStreamInfo.streamClassType, rendererStreamInfo.sessionId);

    HpaeStreamInfo capturerStreamInfo = GetCaptureStreamInfo();
    capturerStreamInfo.deviceName = sourceAudioModuleInfo.name;
    hpaeManager_->CreateStream(capturerStreamInfo);
    hpaeManager_->DumpSourceOutputsInfo();
    WaitForMsgProcessing(hpaeManager_);
    dumpCallback->GetSourceOutputsSize();

    hpaeManager_->ShouldNotSkipProcess(capturerStreamInfo.streamClassType, capturerStreamInfo.sessionId);

    hpaeManager_->ShouldNotSkipProcess(HPAE_STREAM_CLASS_TYPE_INVALID, TEST_STREAM_SESSION_ID);
    hpaeManager_->CloseAudioPort(sinkPortId);
    hpaeManager_->CloseAudioPort(sourcePortId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerDumpStreamInfoTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();

    hpaeManager_->IsInit();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);

    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(sinkAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sinkPortId = GetData<int32_t>();
    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(sourceAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sourcePortId = GetData<int32_t>();

    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);
    HpaeStreamInfo rendererStreamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(rendererStreamInfo);

    hpaeManager_->DumpSinkInputsInfo();
    WaitForMsgProcessing(hpaeManager_);
    dumpCallback->GetSinkInputsSize();
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];

    hpaeManager_->ShouldNotSkipProcess(streamClassType, sessionId);
    HpaeStreamInfo capturerStreamInfo = GetCaptureStreamInfo();
    capturerStreamInfo.deviceName = sourceAudioModuleInfo.name;
    hpaeManager_->CreateStream(capturerStreamInfo);
    hpaeManager_->DumpSourceOutputsInfo();
    dumpCallback->GetSourceOutputsSize();
    sessionId = GetData<uint32_t>();
    index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    streamClassType = HpaeStreamClassTypeVec[index];
    hpaeManager_->ShouldNotSkipProcess(streamClassType, sessionId);

    hpaeManager_->CloseAudioPort(sinkPortId);
    hpaeManager_->CloseAudioPort(sourcePortId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    InitFuzzTest,
    HpaeRenderStreamManagerFuzzTest,
    HpaeRenderStreamManagerFuzzTest2,
    HpaeRenderStreamManagerFuzzTest3,
    HpaeRenderStreamManagerFuzzTest4,
    HpaeCaptureStreamManagerFuzzTest,
    HpaeCaptureStreamManagerFuzzTest2,
    HpaeCaptureStreamManagerFuzzTest3,
    HpaeRenderStreamManagerMoveFuzzTest,
    HpaeRenderStreamManagerMoveFuzzTest2,
    HpaeCaptureStreamManagerMoveTest,
    HpaeCaptureStreamManagerMoveTest2,
    HpaeManagerDumpStreamInfoTest,
    HpaeManagerDumpStreamInfoTest2,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
