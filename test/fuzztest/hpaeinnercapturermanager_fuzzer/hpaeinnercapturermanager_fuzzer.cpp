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
#include "audio_stream_info.h"
#include "audio_ec_info.h"
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
static std::string g_rootPath = "/data/";
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const char* DEFAULT_TEST_DEVICE_CLASS = "file_io";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_SESSION_ID = 123456;
constexpr int32_t MAX_MSG_PROCESSING_RETRY_COUNT = 50;
constexpr int32_t MSG_PROCESSING_SLEEP_MS = 20;
constexpr int32_t MSG_PROCESSING_EXTRA_SLEEP_MS = 40;
typedef void (*TestPtr)(const uint8_t *, size_t);
const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6,
    CHANNEL_7,
    CHANNEL_8,
    CHANNEL_9,
    CHANNEL_10,
    CHANNEL_11,
    CHANNEL_12,
    CHANNEL_13,
    CHANNEL_14,
    CHANNEL_15,
    CHANNEL_16,
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

template<class T>
void RoundVal(T &roundVal, const std::vector<T>& list)
{
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>()%list.size()];
    }
}

void RoundSinkInfo(HpaeSinkInfo &sinkInfo)
{
    RoundVal(sinkInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(sinkInfo.format, AUDIO_SUPPORTED_FORMATS);
}

void RoundStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(streamInfo.format, AUDIO_SUPPORTED_FORMATS);
}

// Fix Bug 1: Add deviceName = "InnerCapturerSink" to both GetInCapSinkInfo and GetInCapFuzzSinkInfo
HpaeSinkInfo GetInCapSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "InnerCapturerSink";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeInnerCapturerManagerTest.pcm";
    RoundSinkInfo(sinkInfo);
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    return sinkInfo;
}

HpaeSinkInfo GetInCapFuzzSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "InnerCapturerSink";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeInnerCapturerManagerTest.pcm";
    RoundSinkInfo(sinkInfo);
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    return sinkInfo;
}

// Helper: init a play stream info with DEFAULT_SESSION_ID
void InitPlayStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
}

// Helper: init a record stream info with DEFAULT_SESSION_ID + 1
void InitRecordStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID + 1;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_MIC;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
}

HpaeStreamInfo GetInCapPlayStreamInfo()
{
    HpaeStreamInfo streamInfo;
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    return streamInfo;
}

HpaeStreamInfo GetInCapPlayFuzzStreamInfo()
{
    HpaeStreamInfo streamInfo;
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    return streamInfo;
}

static HpaeStreamInfo GetInCapRecordStreamInfo()
{
    HpaeStreamInfo streamInfo;
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID + 1;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    return streamInfo;
}

static HpaeStreamInfo GetInCapRecordFuzzStreamInfo()
{
    HpaeStreamInfo streamInfo;
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    return streamInfo;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeInnerCapturerManager>& hpaeInnerCapturerManager)
{
    int retry = 0;
    while (hpaeInnerCapturerManager->IsMsgProcessing() && retry < MAX_MSG_PROCESSING_RETRY_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MSG_PROCESSING_SLEEP_MS));
        retry++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(MSG_PROCESSING_EXTRA_SLEEP_MS));
}

void HpaeInnerCapturerManagerFuzzTest1()
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
    hpaeInnerCapturerManager->GetThreadName();
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    hpaeInnerCapturerManager->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Pause(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Pause(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Flush(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Drain(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeInnerCapturerManager->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo);
    hpaeInnerCapturerManager->GetSourceOutputInfo(recordStreamInfo.sessionId, sourceOutoputInfo);
    hpaeInnerCapturerManager->Stop(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Stop(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 2: Use recordStreamInfo.sessionId instead of new GetData<uint32_t>() for session IDs
void HpaeInnerCapturerManagerFuzzTest2()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->GetThreadName();
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    hpaeInnerCapturerManager->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb);

    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Pause(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Pause(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Flush(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Drain(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeInnerCapturerManager->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo);
    hpaeInnerCapturerManager->GetSourceOutputInfo(recordStreamInfo.sessionId, sourceOutoputInfo);
    hpaeInnerCapturerManager->Stop(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Stop(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 2: Use recordStreamInfo.sessionId instead of new GetData<uint32_t>() for session IDs
void HpaeInnerCapturerManagerFuzzTest3()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->GetThreadName();
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    hpaeInnerCapturerManager->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb);

    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Pause(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Pause(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Flush(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Drain(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeInnerCapturerManager->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo);
    hpaeInnerCapturerManager->GetSourceOutputInfo(recordStreamInfo.sessionId, sourceOutoputInfo);
    hpaeInnerCapturerManager->Stop(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Stop(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
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
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayStreamInfo();
    ++playSencondStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playSencondStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(playSencondStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = GetData<uint32_t>();
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
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
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->SuspendStreamManager(false);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(playSencondNodeInfo.sessionId);
    hpaeInnerCapturerManager->Release(playSencondStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 2: Use recordStreamInfo.sessionId and playStreamInfo.sessionId instead of GetData<uint32_t>()
void HpaeInnerCapturerManagerAddNodeToSinkFuzzTest2()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayFuzzStreamInfo();
    ++playSencondStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playSencondStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(playSencondStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = GetData<uint32_t>();
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    playSencondNodeInfo.samplingRate = SAMPLE_RATE_44100;
    playSencondNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    playSencondNodeInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    playSencondNodeInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    std::shared_ptr<HpaeSinkInputNode> HpaeSinkInputSencondNode =
        std::make_shared<HpaeSinkInputNode>(playSencondNodeInfo);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isConnect = GetData<bool>();
    hpaeInnerCapturerManager->AddSingleNodeToSinkInner(HpaeSinkInputSencondNode, isConnect);
    bool isSuspend1 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend1);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->AddNodeToSink(HpaeSinkInputSencondNode);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isSuspend2 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend2);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(playSencondNodeInfo.sessionId);
    hpaeInnerCapturerManager->Release(playSencondStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 2: Use recordStreamInfo.sessionId and playStreamInfo.sessionId instead of GetData<uint32_t>()
void HpaeInnerCapturerManagerAddNodeToSinkFuzzTest3()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(recordStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
    hpaeInnerCapturerManager->CreateStream(playStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayFuzzStreamInfo();
    ++playSencondStreamInfo.sessionId;
    hpaeInnerCapturerManager->CreateStream(playSencondStreamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(playSencondStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = GetData<uint32_t>();
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    playSencondNodeInfo.samplingRate = SAMPLE_RATE_44100;
    playSencondNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    playSencondNodeInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    playSencondNodeInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    std::shared_ptr<HpaeSinkInputNode> HpaeSinkInputSencondNode =
        std::make_shared<HpaeSinkInputNode>(playSencondNodeInfo);
    hpaeInnerCapturerManager->Release(playStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isConnect = GetData<bool>();
    hpaeInnerCapturerManager->AddSingleNodeToSinkInner(HpaeSinkInputSencondNode, isConnect);
    bool isSuspend1 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend1);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->AddNodeToSink(HpaeSinkInputSencondNode);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    bool isSuspend2 = GetData<bool>();
    hpaeInnerCapturerManager->SuspendStreamManager(isSuspend2);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Release(playSencondNodeInfo.sessionId);
    hpaeInnerCapturerManager->Release(playSencondStreamInfo.sessionId);
    hpaeInnerCapturerManager->Release(recordStreamInfo.sessionId);
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
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
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
    int32_t testVariable = GetData<int32_t>();
    float volume = GetData<float>();
    hpaeInnerCapturerManager->SetClientVolume(playStreamInfo.sessionId, volume);
    hpaeInnerCapturerManager->SetRate(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetAudioEffectMode(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetAudioEffectMode(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetPrivacyType(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetPrivacyType(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetWritableSize(playStreamInfo.sessionId);
    hpaeInnerCapturerManager->UpdateSpatializationState(playStreamInfo.sessionId, true, true);
    hpaeInnerCapturerManager->UpdateMaxLength(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetClientVolume(playStreamInfo.sessionId, volume);
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
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->Start(recordStreamInfo.sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);

    HpaeStreamInfo playStreamInfo = GetInCapPlayFuzzStreamInfo();
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
    int32_t testVariable = GetData<int32_t>();
    float volume = GetData<float>();
    hpaeInnerCapturerManager->SetClientVolume(playStreamInfo.sessionId, volume);
    hpaeInnerCapturerManager->SetRate(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetAudioEffectMode(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetAudioEffectMode(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetPrivacyType(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetPrivacyType(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->GetWritableSize(playStreamInfo.sessionId);
    hpaeInnerCapturerManager->UpdateSpatializationState(playStreamInfo.sessionId, true, true);
    hpaeInnerCapturerManager->UpdateMaxLength(playStreamInfo.sessionId, testVariable);
    hpaeInnerCapturerManager->SetClientVolume(playStreamInfo.sessionId, volume);
    bool isMute = GetData<bool>();
    hpaeInnerCapturerManager->SetMute(isMute);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

void HpaeInnerCapturerManagerReloadFuzzTest1()
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
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    isReload = GetData<bool>();
    hpaeInnerCapturerManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 4: Use GetInCapSinkInfo() instead of default HpaeSinkInfo()
void MoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    uint32_t sessionId = GetData<uint32_t>();
    std::string sinkName = "13222";
    hpaeInnerCapturerManager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 4: Use GetInCapSinkInfo() instead of default HpaeSinkInfo()
void MoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    std::vector<uint32_t> sessionId = {GetData<uint32_t>(), GetData<uint32_t>(), GetData<uint32_t>()};
    std::string sinkName = "13222";
    hpaeInnerCapturerManager->MoveAllStream(sinkName, sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 4: Use GetInCapSinkInfo() instead of default HpaeSinkInfo()
void OnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = IOperation::OPERATION_INVALID;
    hpaeInnerCapturerManager->OnNodeStatusUpdate(sessionId, operation);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// Fix Bug 4: Use GetInCapSinkInfo() instead of default HpaeSinkInfo()
void OnFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto hpaeInnerCapturerManager = std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    hpaeInnerCapturerManager->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    uint32_t sessionId = GetData<uint32_t>();
    hpaeInnerCapturerManager->OnFadeDone(sessionId);
    WaitForMsgProcessing(hpaeInnerCapturerManager);
    hpaeInnerCapturerManager->DeInit();
}

// --- New dispatch functions (Bug 5: missing API tests) ---

void InitDeInitCorrectPathFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    mgr->IsInit();
    mgr->IsRunning();
    mgr->DeInit();
}

void FullPlayLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    HpaeStreamInfo playInfo;
    InitPlayStreamInfo(playInfo);
    mgr->CreateStream(playInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(playInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Pause(playInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Start(playInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Flush(playInfo.sessionId);
    mgr->Drain(playInfo.sessionId);
    mgr->Stop(playInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Release(playInfo.sessionId);
    mgr->DestroyStream(playInfo.sessionId);
    mgr->DeInit();
}

void FullRecordLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    HpaeStreamInfo recInfo;
    InitRecordStreamInfo(recInfo);
    mgr->CreateStream(recInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(recInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(recInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Release(recInfo.sessionId);
    mgr->DestroyStream(recInfo.sessionId);
    mgr->DeInit();
}

void DualPathLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    HpaeStreamInfo playInfo;
    HpaeStreamInfo recInfo;
    InitPlayStreamInfo(playInfo);
    InitRecordStreamInfo(recInfo);
    mgr->CreateStream(playInfo);
    mgr->CreateStream(recInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(playInfo.sessionId);
    mgr->Start(recInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(playInfo.sessionId);
    mgr->Stop(recInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Release(playInfo.sessionId);
    mgr->Release(recInfo.sessionId);
    mgr->DestroyStream(playInfo.sessionId);
    mgr->DestroyStream(recInfo.sessionId);
    mgr->DeInit();
}

void SetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    uint32_t sessionId = GetData<uint32_t>();
    float gain = GetData<float>();
    mgr->SetLoudnessGain(sessionId, gain);
    mgr->DeInit();
}

void DumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    mgr->DumpSinkInfo();
    mgr->GetDeviceHDFDumpInfo();
    mgr->DeInit();
}

void SetOffloadPolicyFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t state = GetData<int32_t>();
    mgr->SetOffloadPolicy(sessionId, state);
    mgr->DeInit();
}

void SetSpeedFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    mgr->SetSpeed(sessionId, speed);
    mgr->DeInit();
}

void RegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    uint32_t sessionId = GetData<uint32_t>();
    std::weak_ptr<ICapturerStreamCallback> callback;
    mgr->RegisterReadCallback(sessionId, callback);
    mgr->DeInit();
}

void DeInitWithMoveDefaultFuzzTest()
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    auto mgr = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    mgr->Init();
    WaitForMsgProcessing(mgr);
    HpaeStreamInfo playInfo;
    InitPlayStreamInfo(playInfo);
    mgr->CreateStream(playInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(playInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DeInit(true);
}

typedef void (*TestFuncs[24])();

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
    MoveStreamFuzzTest,
    MoveAllStreamFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    OnFadeDoneFuzzTest,
    InitDeInitCorrectPathFuzzTest,
    FullPlayLifecycleFuzzTest,
    FullRecordLifecycleFuzzTest,
    DualPathLifecycleFuzzTest,
    SetLoudnessGainFuzzTest,
    DumpSinkInfoFuzzTest,
    SetOffloadPolicyFuzzTest,
    SetSpeedFuzzTest,
    RegisterReadCallbackFuzzTest,
    DeInitWithMoveDefaultFuzzTest,
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
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
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