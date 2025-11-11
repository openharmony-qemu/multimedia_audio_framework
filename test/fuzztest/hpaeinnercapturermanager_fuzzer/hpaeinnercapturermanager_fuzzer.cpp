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
#include "hpaeinnercapturermanager_fuzzer.h"
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <queue>
#include <string>
#undef private
#include "audio_info.h"
#include "hpae_inner_capturer_manager.h"
#include "i_hpae_renderer_manager.h"
#include "audio_engine_log.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;


namespace OHOS {
namespace AudioStandard {
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH1 = 960;
const uint32_t DEFAULT_FRAME_LENGTH2 = 882;
static std::string g_rootPath = "/data/";
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const char* DEFAULT_TEST_DEVICE_CLASS = "file_io";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
const uint32_t DEFAULT_SESSION_ID = 123456;
typedef void (*TestPtr)(const uint8_t *, size_t);

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

int32_t WriteFixedDataCb::OnStreamData(AudioCallBackStreamInfo& callBackStremInfo)
{
    return SUCCESS;
}

ReadDataCb::ReadDataCb(const std::string &fileName)
{
    testFile_ = fopen(fileName.c_str(), "ab");
    if (testFile_ == nullptr) {
        AUDIO_ERR_LOG("Open file failed");
    }
}

ReadDataCb::~ReadDataCb()
{
    if (testFile_) {
        fclose(testFile_);
        testFile_ = nullptr;
    }
}

int32_t ReadDataCb::OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo)
{
    return SUCCESS;
}

HpaeSinkInfo GetInCapSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeInnerCapturerManagerTest.pcm";
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.frameLen = DEFAULT_FRAME_LENGTH1;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    return sinkInfo;
}

HpaeSinkInfo GetInCapFuzzSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeInnerCapturerManagerTest.pcm";
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.frameLen = GetData<size_t>();
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    return sinkInfo;
}

HpaeStreamInfo GetInCapPlayStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.frameLen = GetData<size_t>();
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    return streamInfo;
}

HpaeStreamInfo GetInCapPlayFuzzStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.frameLen = GetData<size_t>();
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.sessionId = GetData<uint32_t>();
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    return streamInfo;
}

static HpaeStreamInfo GetInCapRecordStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH2;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.sessionId = DEFAULT_SESSION_ID + 1;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    return streamInfo;
}

static HpaeStreamInfo GetInCapRecordFuzzStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH2;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.sessionId = GetData<uint32_t>();
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    return streamInfo;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeInnerCapturerManager>& hpaeInnerCapturerManager)
{
    while (hpaeInnerCapturerManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 20ms frameLen, need optimize
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));  // 40ms wait time, need optimize
}

void HpaeInnerCapturerManagerFuzzTest1()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->GetThreadName();
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    hpaeInnerCapturerManager->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    hpaeInnerCapturerManager->Pause(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Pause(playStreamInfo.sessionId);
    hpaeInnerCapturerManager->Flush(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Drain(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeInnerCapturerManager->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo);
    hpaeInnerCapturerManager->GetSourceOutputInfo(recordStreamInfo.sessionId, sourceOutoputInfo);
    hpaeInnerCapturerManager->Stop(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerFuzzTest2()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    hpaeInnerCapturerManager->GetThreadName();
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    uint32_t recordSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->Start(recordSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t playSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->RegisterWriteCallback(playSessionId, writeInPlayDataCb);
    
    hpaeInnerCapturerManager->Start(playSessionId);
    hpaeInnerCapturerManager->Pause(recordSessionId);
    hpaeInnerCapturerManager->Pause(playSessionId);
    hpaeInnerCapturerManager->Flush(recordSessionId);
    hpaeInnerCapturerManager->Drain(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeInnerCapturerManager->GetSinkInputInfo(playSessionId, sinkInputInfo);
    hpaeInnerCapturerManager->GetSourceOutputInfo(recordSessionId, sourceOutoputInfo);
    hpaeInnerCapturerManager->Stop(recordSessionId);
    hpaeInnerCapturerManager->Release(recordSessionId);
    hpaeInnerCapturerManager->Release(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerFuzzTest3()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->GetThreadName();
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    uint32_t recordSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->Start(recordSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t playSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->RegisterWriteCallback(playSessionId, writeInPlayDataCb);
    
    hpaeInnerCapturerManager->Start(playSessionId);
    hpaeInnerCapturerManager->Pause(recordSessionId);
    hpaeInnerCapturerManager->Pause(playSessionId);
    hpaeInnerCapturerManager->Flush(recordSessionId);
    hpaeInnerCapturerManager->Drain(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeInnerCapturerManager->GetSinkInputInfo(playSessionId, sinkInputInfo);
    hpaeInnerCapturerManager->GetSourceOutputInfo(recordSessionId, sourceOutoputInfo);
    hpaeInnerCapturerManager->Stop(recordSessionId);
    hpaeInnerCapturerManager->Release(recordSessionId);
    hpaeInnerCapturerManager->Release(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerAddNodeToSinkFuzzTest1()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayStreamInfo();
    ++playSencondStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playSencondStreamInfo);
    hpaeInnerCapturerManager->Start(playSencondStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = playSencondStreamInfo.sessionId + 1;
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = DEFAULT_FRAME_LENGTH2;
    playSencondNodeInfo.samplingRate = SAMPLE_RATE_44100;
    playSencondNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    playSencondNodeInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    playSencondNodeInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    std::shared_ptr<HpaeSinkInputNode> HpaeSinkInputSencondNode =
        std::make_shared<HpaeSinkInputNode>(playSencondNodeInfo);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->AddSingleNodeToSinkInner(HpaeSinkInputSencondNode, false);
    hpaeInnerCapturerManager->SuspendStreamManager(true);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->AddNodeToSink(HpaeSinkInputSencondNode);
    hpaeInnerCapturerManager->SuspendStreamManager(false);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(playSencondNodeInfo.sessionId);
    hpaeInnerCapturerManager->Release(playSencondStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerAddNodeToSinkFuzzTest2()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    uint32_t recordSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->Start(recordSessionId);
    
    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    uint32_t playSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    hpaeInnerCapturerManager->Start(playSessionId);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayFuzzStreamInfo();
    ++playSencondStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playSencondStreamInfo);
    hpaeInnerCapturerManager->Start(playSencondStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = GetData<uint32_t>();
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = GetData<uint64_t>();
    playSencondNodeInfo.samplingRate = SAMPLE_RATE_44100;
    playSencondNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    playSencondNodeInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    playSencondNodeInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    std::shared_ptr<HpaeSinkInputNode> HpaeSinkInputSencondNode =
        std::make_shared<HpaeSinkInputNode>(playSencondNodeInfo);
    hpaeInnerCapturerManager->Release(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isConnect = GetData<bool>();
    hpaeInnerCapturerManager->AddSingleNodeToSinkInner(HpaeSinkInputSencondNode, isConnect);
    bool isSuspend1 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend1);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->AddNodeToSink(HpaeSinkInputSencondNode);
    bool isSuspend2 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend2);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(playSencondNodeInfo.sessionId);
    hpaeInnerCapturerManager->Release(playSencondStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerAddNodeToSinkFuzzTest3()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    uint32_t recordSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->Start(recordSessionId);
    
    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    uint32_t playSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    hpaeInnerCapturerManager->Start(playSessionId);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayFuzzStreamInfo();
    ++playSencondStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playSencondStreamInfo);
    hpaeInnerCapturerManager->Start(playSencondStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = GetData<uint32_t>();
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = GetData<uint64_t>();
    playSencondNodeInfo.samplingRate = SAMPLE_RATE_44100;
    playSencondNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    playSencondNodeInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    playSencondNodeInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    std::shared_ptr<HpaeSinkInputNode> HpaeSinkInputSencondNode =
        std::make_shared<HpaeSinkInputNode>(playSencondNodeInfo);
    hpaeInnerCapturerManager->Release(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isConnect = GetData<bool>();
    hpaeInnerCapturerManager->AddSingleNodeToSinkInner(HpaeSinkInputSencondNode, isConnect);
    bool isSuspend1 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend1);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->AddNodeToSink(HpaeSinkInputSencondNode);
    bool isSuspend2 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend2);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(playSencondNodeInfo.sessionId);
    hpaeInnerCapturerManager->Release(playSencondStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerOtherFuzzTest1()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    hpaeInnerCapturerManager->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    hpaeInnerCapturerManager->GetAllSinkInputsInfo();
    hpaeInnerCapturerManager->GetAllSourceOutputsInfo();
    hpaeInnerCapturerManager->GetSinkInfo();
    hpaeInnerCapturerManager->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    int32_t testVariable = 1;
    hpaeInnerCapturerManager->SetClientVolume(playStreamInfo.sessionId, 1.0f);
    hpaeInnerCapturerManager->SetRate(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetAudioEffectMode(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetAudioEffectMode(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetPrivacyType(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetPrivacyType(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetWritableSize(playStreamInfo.sessionId);
    hpaeInnerCapturerManager->UpdateSpatializationState(playStreamInfo.sessionId, true, true);
    hpaeInnerCapturerManager->UpdateMaxLength(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetClientVolume(playStreamInfo.sessionId, 1.0f);
    bool isMute = GetData<bool>();
    hpaeInnerCapturerManager->SetMute(isMute);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerOtherFuzzTest2()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    uint32_t recordSessionId = GetData<uint32_t>();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t playSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->RegisterWriteCallback(playSessionId, writeInPlayDataCb);
    hpaeInnerCapturerManager->Start(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    hpaeInnerCapturerManager->GetAllSinkInputsInfo();
    hpaeInnerCapturerManager->GetAllSourceOutputsInfo();
    hpaeInnerCapturerManager->GetSinkInfo();
    hpaeInnerCapturerManager->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    int32_t testVariable = GetData<int32_t>();
    float volume = GetData<float>();
    hpaeInnerCapturerManager->SetClientVolume(playSessionId, volume);
    hpaeInnerCapturerManager->SetRate(playSessionId, testVariable);
    hpaeInnerCapturerManager->SetAudioEffectMode(playSessionId, testVariable);
    hpaeInnerCapturerManager->GetAudioEffectMode(playSessionId, testVariable);
    hpaeInnerCapturerManager->SetPrivacyType(playSessionId, testVariable);
    hpaeInnerCapturerManager->GetPrivacyType(playSessionId, testVariable);
    hpaeInnerCapturerManager->GetWritableSize(playSessionId);
    hpaeInnerCapturerManager->UpdateSpatializationState(playSessionId, true, true);
    hpaeInnerCapturerManager->UpdateMaxLength(playSessionId, testVariable);
    hpaeInnerCapturerManager->SetClientVolume(playSessionId, volume);
    bool isMute = GetData<bool>();
    hpaeInnerCapturerManager->SetMute(isMute);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerOtherFuzzTest3()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    uint32_t recordSessionId = GetData<uint32_t>();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t playSessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->RegisterWriteCallback(playSessionId, writeInPlayDataCb);
    hpaeInnerCapturerManager->Start(playSessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    hpaeInnerCapturerManager->GetAllSinkInputsInfo();
    hpaeInnerCapturerManager->GetAllSourceOutputsInfo();
    hpaeInnerCapturerManager->GetSinkInfo();
    hpaeInnerCapturerManager->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    int32_t testVariable = GetData<int32_t>();
    float volume = GetData<float>();
    hpaeInnerCapturerManager->SetClientVolume(playSessionId, volume);
    hpaeInnerCapturerManager->SetRate(playSessionId, testVariable);
    hpaeInnerCapturerManager->SetAudioEffectMode(playSessionId, testVariable);
    hpaeInnerCapturerManager->GetAudioEffectMode(playSessionId, testVariable);
    hpaeInnerCapturerManager->SetPrivacyType(playSessionId, testVariable);
    hpaeInnerCapturerManager->GetPrivacyType(playSessionId, testVariable);
    hpaeInnerCapturerManager->GetWritableSize(playSessionId);
    hpaeInnerCapturerManager->UpdateSpatializationState(playSessionId, true, true);
    hpaeInnerCapturerManager->UpdateMaxLength(playSessionId, testVariable);
    hpaeInnerCapturerManager->SetClientVolume(playSessionId, volume);
    bool isMute = GetData<bool>();
    hpaeInnerCapturerManager->SetMute(isMute);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerReloadFuzzTest1()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    ++playStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, false);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerReloadFuzzTest2()
{
    HpaeSinkInfo sinkInfo = GetInCapFuzzSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    ++playStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
    isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerReloadFuzzTest3()
{
    HpaeSinkInfo sinkInfo = GetInCapFuzzSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    ++playStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
    isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void MoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    std::string sinkName = "13222";
    hpaeInnerCapturerManager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void MoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    std::vector<uint32_t> sessionId = {GetData<uint32_t>(), GetData<uint32_t>(), GetData<uint32_t>()};
    std::string sinkName = "13222";
    hpaeInnerCapturerManager->MoveAllStream(sinkName, sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void OnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = IOperation::OPERATION_INVALID;
    hpaeInnerCapturerManager->OnNodeStatusUpdate(sessionId, operation);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void OnFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->OnFadeDone(sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

typedef void (*TestFuncs[16])();

TestFuncs g_testFuncs = {
    HpaeInnerCapturerManagerFuzzTest1,
    HpaeInnerCapturerManagerFuzzTest2,
    HpaeInnerCapturerManagerFuzzTest3,
    HpaeInnerCapturerManagerAddNodeToSinkFuzzTest1,
    HpaeInnerCapturerManagerAddNodeToSinkFuzzTest2,
    HpaeInnerCapturerManagerAddNodeToSinkFuzzTest3,
    HpaeInnerCapturerManagerOtherFuzzTest1,
    HpaeInnerCapturerManagerOtherFuzzTest2,
    HpaeInnerCapturerManagerOtherFuzzTest3,
    HpaeInnerCapturerManagerReloadFuzzTest1,
    HpaeInnerCapturerManagerReloadFuzzTest2,
    HpaeInnerCapturerManagerReloadFuzzTest3,
    MoveStreamFuzzTest,
    MoveAllStreamFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    OnFadeDoneFuzzTest,
};

bool FuzzTest(const uint8_t *rawData, size_t size)
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
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}