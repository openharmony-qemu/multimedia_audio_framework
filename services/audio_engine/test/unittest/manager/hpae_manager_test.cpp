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
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_audio_service_dump_callback_unit_test.h"
#include "hpae_manager_unit_test.h"
#include "hpae_manager.h"
#include "hpae_fast_renderer_manager.h"
#include "hpae_fast_capturer_manager.h"
#include "hpae_offload_renderer_manager.h"

using namespace OHOS;
using namespace AudioStandard;

namespace {
constexpr int32_t TEST_SYNC_ERROR = -999;
}
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
static std::string g_rootPath = "/data/";
const std::string ROOT_PATH = "/data/source_file_io_48000_2_s16le.pcm";
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_CAPTURE_SESSION_ID = 123455;
constexpr int32_t TEST_STREAM_UID = 111111;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t SESSION_ID_NOEXIST = 100000;

class HpaeManagerUnitTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<HpaeManager> hpaeManager_ = nullptr;
};
void HpaeManagerUnitTest::SetUp()
{
    hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
}

void HpaeManagerUnitTest::TearDown()
{
    hpaeManager_->DeInit();
    hpaeManager_ = nullptr;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeManager> &hpaeManager)
{
    int waitCount = 0;
    const int waitCountThd = 100;
    while (hpaeManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    EXPECT_EQ(hpaeManager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}

void WaitForMsgProcessingStable(std::shared_ptr<HpaeManager> &hpaeManager)
{
    int waitCount = 0;
    int stableIdleCount = 0;
    const int waitCountThd = 150;
    const int stableIdleCountThd = 5;
    while (waitCount < waitCountThd) {
        if (hpaeManager->IsMsgProcessing()) {
            stableIdleCount = 0;
        } else {
            stableIdleCount++;
            if (stableIdleCount >= stableIdleCountThd) {
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
    }
    EXPECT_EQ(hpaeManager->IsMsgProcessing(), false);
    EXPECT_GE(stableIdleCount, stableIdleCountThd);
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
    audioModuleInfo.needEmptyChunk = true;
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
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

static HpaeSinkInfo GetSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    return sinkInfo;
}

static void InitSourceInfo(HpaeSourceInfo &sourceInfo)
{
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;
}

HpaeStreamInfo GetRenderStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.uid = TEST_STREAM_UID;
    return streamInfo;
}

HpaeStreamInfo GetCaptureStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.uid = TEST_STREAM_UID;
    return streamInfo;
}

static std::shared_ptr<HpaeSinkInputNode> CreateTestNode(OHOS::AudioStandard::HPAE::HpaeSessionState state)
{
    HpaeNodeInfo nodeinfo;
    nodeinfo.sessionId = TEST_STREAM_SESSION_ID;
    nodeinfo.streamType = STREAM_MUSIC;
    std::shared_ptr<HpaeSinkInputNode> node = std::make_shared<HpaeSinkInputNode>(nodeinfo);
    node->SetState(state);
    return node;
}

HWTEST_F(HpaeManagerUnitTest, constructHpaeManagerTest001, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);
    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    EXPECT_EQ(hpaeManager_->IsRunning(), false);
}


HWTEST_F(HpaeManagerUnitTest, constructHpaeManagerTest002, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);
    hpaeManager_->DeInit();
    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    EXPECT_EQ(hpaeManager_->IsRunning(), false);
}

HWTEST_F(HpaeManagerUnitTest, GetHpaeRenderManagerTest, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();

    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetCloseAudioPortResult(), SUCCESS);

    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    EXPECT_EQ(hpaeManager_->IsRunning(), false);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderManagerTest, TestSize.Level1)
{
    IHpaeManager::GetHpaeManager().Init();
    EXPECT_EQ(IHpaeManager::GetHpaeManager().IsInit(), true);
    sleep(1);
    EXPECT_EQ(IHpaeManager::GetHpaeManager().IsRunning(), true);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(IHpaeManager::GetHpaeManager().OpenAudioPort(audioModuleInfo), SUCCESS);
    IHpaeManager::GetHpaeManager().DeInit();
    EXPECT_EQ(IHpaeManager::GetHpaeManager().IsInit(), false);
    EXPECT_EQ(IHpaeManager::GetHpaeManager().IsRunning(), false);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    int32_t result = hpaeManager_->RegisterSerivceCallback(callback);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetSinkMute(audioModuleInfo.name, true, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSinkMuteResult(), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSinkMute(audioModuleInfo.name, false, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSinkMuteResult(), SUCCESS);

    EXPECT_EQ(hpaeManager_->SuspendAudioDevice(audioModuleInfo.name, true), SUCCESS);
    EXPECT_EQ(hpaeManager_->SuspendAudioDevice(audioModuleInfo.name, false), SUCCESS);

    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetGetAllSinkInputsResult(), SUCCESS);
    std::vector<SinkInput> sinkInputs = callback->GetSinkInputs();
    EXPECT_EQ(sinkInputs.size(), 1);
    for (const auto &it : sinkInputs) {
        std::cout << "sinkInputs.sinkName:" << it.sinkName << std::endl;
        EXPECT_EQ(it.paStreamId, streamInfo.sessionId);
        EXPECT_EQ(it.sinkName, audioModuleInfo.name);
    }
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetGetAllSinkInputsResult(), SUCCESS);
    sinkInputs = callback->GetSinkInputs();
    EXPECT_EQ(sinkInputs.size(), 0);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    int32_t result = hpaeManager_->RegisterSerivceCallback(callback);
    EXPECT_EQ(result, SUCCESS);

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_UID, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_UID, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_UID, false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);

    EXPECT_EQ(hpaeManager_->GetAllSourceOutputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetGetAllSourceOutputsResult(), SUCCESS);
    std::vector<SourceOutput> sourceOutputs = callback->GetSourceOutputs();
    EXPECT_EQ(sourceOutputs.size(), 1);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->TriggerAppsUidUpdate(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    int32_t syncId = 123;
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);

    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    int32_t syncId = 123;
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);

    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);

    int32_t syncId = 123;
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSED);

    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPED);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest005, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    HpaeStreamInfo streamInfo2 = GetRenderStreamInfo();
    streamInfo2.sessionId = TEST_STREAM_SESSION_ID + 1;
    streamInfo2.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo2);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb2 = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo2.sessionId, writeFixedValueCb2);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb2 = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo2.sessionId, statusChangeCb2);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Start(streamInfo2.streamClassType, streamInfo2.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);
    hpaeManager_->GetSessionInfo(streamInfo2.streamClassType, streamInfo2.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb2->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Pause(streamInfo2.streamClassType, streamInfo2.sessionId);
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Stop(streamInfo2.streamClassType, streamInfo2.sessionId);
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo2.streamClassType, streamInfo2.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(statusChangeCb2->GetStatus(), I_STATUS_STOPPED);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Release(streamInfo2.streamClassType, streamInfo2.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo2.streamClassType, streamInfo2.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerMoveTest001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerMoveTest002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerMoveTest003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    int32_t syncId = 123;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 0, "Speaker_File");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    int32_t result = hpaeManager_->RegisterSerivceCallback(callback);
    EXPECT_EQ(result, SUCCESS);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->TriggerAppsUidUpdate(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_UID, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_UID, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_UID, false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);

    EXPECT_EQ(hpaeManager_->SetSourceOutputMuteByStreamId(streamInfo.sessionId, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMuteByStreamId(streamInfo.sessionId, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMuteByStreamId(streamInfo.sessionId, false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    int32_t result = hpaeManager_->RegisterSerivceCallback(callback);
    EXPECT_EQ(result, SUCCESS);

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    audioModuleInfo.sourceType = "17";
    hpaeManager_->effectLiveState_ = "Nosupport";
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    int32_t result = hpaeManager_->RegisterSerivceCallback(callback);
    EXPECT_EQ(result, SUCCESS);

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    audioModuleInfo.sourceType = "17";
    hpaeManager_->effectLiveState_ = "NRON";
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest005, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    int32_t result = hpaeManager_->RegisterSerivceCallback(callback);
    EXPECT_EQ(result, SUCCESS);

    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    audioModuleInfo.sourceType = "17";
    hpaeManager_->effectLiveState_ = "NROFF";
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerMoveTest001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSourceAudioModeInfo("mic1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    sleep(1);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1");
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 0, "mic");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_PAUSING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_STOPPING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 0, "mic");
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerMoveTest002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSourceAudioModeInfo("mic1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    sleep(1);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1");
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 0, "mic");
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1");
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);
    
    hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 0, "mic");
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IsAcousticEchoCancelerSupported001, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    bool result = hpaeManager_->IsAcousticEchoCancelerSupported(SOURCE_TYPE_VOICE_COMMUNICATION);
    EXPECT_EQ(result, true);
}

HWTEST_F(HpaeManagerUnitTest, IsAcousticEchoCancelerSupported002, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    bool result = hpaeManager_->IsAcousticEchoCancelerSupported(SOURCE_TYPE_VOICE_COMMUNICATION);
    EXPECT_EQ(result, true);
    result = hpaeManager_->IsAcousticEchoCancelerSupported(SOURCE_TYPE_VOICE_TRANSCRIPTION);
    EXPECT_EQ(result, true);
    result = hpaeManager_->IsAcousticEchoCancelerSupported(SOURCE_TYPE_MIC);
    EXPECT_EQ(result, false);
}

HWTEST_F(HpaeManagerUnitTest, SetEffectLiveParameter001, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::vector<std::pair<std::string, std::string>> params;
    bool result = hpaeManager_->SetEffectLiveParameter(params);
    EXPECT_EQ(result, false);
}

HWTEST_F(HpaeManagerUnitTest, SetEffectLiveParameter002, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::vector<std::pair<std::string, std::string>> params;
    params.push_back({"invalidKey", "invalidValue"});
    bool result = hpaeManager_->SetEffectLiveParameter(params);
    EXPECT_EQ(result, false);
    params.clear();
    params.push_back({"live_effect_enable", "invalidValue"});
    result = hpaeManager_->SetEffectLiveParameter(params);
    EXPECT_EQ(result, false);
}

HWTEST_F(HpaeManagerUnitTest, SetEffectLiveParameter003, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::vector<std::pair<std::string, std::string>> params;
    params.push_back({"live_effect_enable", "NRON"});
    hpaeManager_->effectLiveState_ = "NoSupport";
    bool result = hpaeManager_->SetEffectLiveParameter(params);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test SetEffectLiveParameter supported path
 * @tc.type  : FUNC
 * @tc.number: SetEffectLiveParameter004
 * @tc.desc  : With a supported preloaded state the locked write updates effectLiveState_.
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectLiveParameter004, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::vector<std::pair<std::string, std::string>> params;
    params.push_back({"live_effect_enable", "NRON"});
    // Preload a supported state so the call passes the NoSupport gate and reaches the
    // locked write; the write happens before persistence, so the state is updated even
    // if the setting provider later fails.
    hpaeManager_->effectLiveState_ = "NROFF";
    EXPECT_NO_FATAL_FAILURE(hpaeManager_->SetEffectLiveParameter(params));
    EXPECT_EQ(hpaeManager_->effectLiveState_, "NRON");
}

HWTEST_F(HpaeManagerUnitTest, GetEffectLiveParameter001, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::vector<std::string> subKeys;
    std::vector<std::pair<std::string, std::string>> result;
    bool res = hpaeManager_->GetEffectLiveParameter(subKeys, result);
    EXPECT_EQ(res, true);
}

HWTEST_F(HpaeManagerUnitTest, GetEffectLiveParameter002, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::vector<std::string> subKeys;
    std::vector<std::pair<std::string, std::string>> result;
    subKeys.push_back("invalidKey");
    bool res = hpaeManager_->GetEffectLiveParameter(subKeys, result);
    EXPECT_EQ(res, false);
}

HWTEST_F(HpaeManagerUnitTest, GetEffectLiveParameter003, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::vector<std::string> subKeys;
    std::vector<std::pair<std::string, std::string>> result;
    subKeys.push_back("live_effect_supported");
    hpaeManager_->effectLiveState_ = "NoSupport";
    bool res = hpaeManager_->GetEffectLiveParameter(subKeys, result);
    EXPECT_EQ(res, true);
    EXPECT_EQ(subKeys[0], result[0].first);
    EXPECT_EQ("NoSupport", result[0].second);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerMoveTest004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    EXPECT_EQ(hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, ""), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "virtual1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->MoveSinkInputByIndexOrName(SESSION_ID_NOEXIST, 1, "Speaker_File1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId].streamInfo.isMoveAble = false;
    EXPECT_EQ(hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId].streamInfo.isMoveAble = true;
    EXPECT_EQ(hpaeManager_->MoveSinkInputByIndexOrName(streamInfo.sessionId, 1, "Speaker_File1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    int32_t syncId = 123;
    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId].state = HPAE_SESSION_RELEASED;
    EXPECT_EQ(hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId].state = HPAE_SESSION_RELEASED;
    EXPECT_EQ(hpaeManager_->StartWithSyncId(streamInfo.streamClassType, streamInfo.sessionId, syncId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Drain(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Flush(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->Start(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Pause(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Stop(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Drain(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Flush(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId].state = HPAE_SESSION_PREPARED;
    EXPECT_EQ(hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);

    EXPECT_EQ(hpaeManager_->SetClientVolume(SESSION_ID_NOEXIST, 1.0f), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetClientVolume(streamInfo.sessionId, 1.0f), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetOffloadPolicy(SESSION_ID_NOEXIST, 1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->movingIds_.emplace(streamInfo.sessionId, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeManager_->SetOffloadPolicy(streamInfo.sessionId, 1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->movingIds_.erase(streamInfo.sessionId);
    EXPECT_EQ(hpaeManager_->SetOffloadPolicy(streamInfo.sessionId, 1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->SetSpeed(SESSION_ID_NOEXIST, 1.0f);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->movingIds_.emplace(streamInfo.sessionId, HPAE_SESSION_RUNNING);
    hpaeManager_->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->movingIds_.erase(streamInfo.sessionId);
    hpaeManager_->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->Drain(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Flush(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerMoveTest003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSourceAudioModeInfo("mic1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->SetDefaultSource(audioModuleInfo.name);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);

    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>(ROOT_PATH);
    EXPECT_EQ(hpaeManager_->RegisterReadCallback(SESSION_ID_NOEXIST, readDataCb), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->RegisterReadCallback(streamInfo.sessionId, readDataCb), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, ""), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "virtual1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->MoveSourceOutputByIndexOrName(SESSION_ID_NOEXIST, 1, "mic1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->capturerIdStreamInfoMap_[streamInfo.sessionId].streamInfo.isMoveAble = false;
    EXPECT_EQ(hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->capturerIdStreamInfoMap_[streamInfo.sessionId].streamInfo.isMoveAble = true;
    EXPECT_EQ(hpaeManager_->MoveSourceOutputByIndexOrName(streamInfo.sessionId, 1, "mic1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->capturerIdStreamInfoMap_[streamInfo.sessionId].state = HPAE_SESSION_RELEASED;
    EXPECT_EQ(hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Drain(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Flush(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->Start(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Pause(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Stop(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Drain(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Flush(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, SESSION_ID_NOEXIST), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->capturerIdStreamInfoMap_[streamInfo.sessionId].state = HPAE_SESSION_PREPARED;
    EXPECT_EQ(hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);

    EXPECT_EQ(hpaeManager_->Drain(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Flush(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->CloseInAudioPort("mic1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

HWTEST_F(HpaeManagerUnitTest, GetAllSinks003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    int32_t ret = hpaeManager_->ReloadRenderManager(audioModuleInfo);
    EXPECT_EQ(ret, SUCCESS);

    hpaeManager_->DumpSinkInfo(audioModuleInfo.name);
    hpaeManager_->DumpSinkInfo("virtual1");
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->OpenVirtualAudioPort(audioModuleInfo, TEST_STREAM_SESSION_ID), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenVirtualAudioPort(audioModuleInfo1, TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    
    ret = hpaeManager_->GetAllSinks();
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(ret, SUCCESS);

    audioModuleInfo1 = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DumpSourceInfo(audioModuleInfo1.name);
    hpaeManager_->DumpSourceInfo("virtual1");
    
    hpaeManager_->DumpAllAvailableDevice();
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerOpenCloseAudioPort001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File2"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File1"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File"), SUCCESS);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerSetDefaultSource001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->SetDefaultSource(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSource("virtual1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSource(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSource(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo1 = GetSourceAudioModeInfo("mic1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSource("Speaker_File1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseInAudioPort("Speaker_File2"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("mic1"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("mic"), SUCCESS);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerEffectTest001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // empty would fall to coreSink_
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t fixedNum = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, fixedNum);
    hpaeManager_->RegisterWriteCallback(streamInfo.sessionId, writeFixedValueCb);
    std::shared_ptr<StatusChangeCb> statusChangeCb = std::make_shared<StatusChangeCb>();
    hpaeManager_->RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, statusChangeCb);
    WaitForMsgProcessing(hpaeManager_);
    HpaeSessionInfo sessionInfo;
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.streamInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sessionInfo.streamInfo.streamType, streamInfo.streamType);
    EXPECT_EQ(sessionInfo.streamInfo.frameLen, streamInfo.frameLen);
    EXPECT_EQ(sessionInfo.streamInfo.format, streamInfo.format);
    EXPECT_EQ(sessionInfo.streamInfo.samplingRate, streamInfo.samplingRate);
    EXPECT_EQ(sessionInfo.streamInfo.channels, streamInfo.channels);
    EXPECT_EQ(sessionInfo.streamInfo.streamClassType, streamInfo.streamClassType);
    EXPECT_EQ(sessionInfo.state, HPAE_SESSION_NEW);
    EXPECT_EQ(hpaeManager_->SetRate(streamInfo.sessionId, RENDER_RATE_DOUBLE), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t effectMode = 0;
    EXPECT_EQ(hpaeManager_->GetAudioEffectMode(streamInfo.sessionId, effectMode), SUCCESS);
    int32_t privacyType = 0;
    EXPECT_EQ(hpaeManager_->SetPrivacyType(streamInfo.sessionId, privacyType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetPrivacyType(streamInfo.sessionId, privacyType), SUCCESS);
    EXPECT_EQ(hpaeManager_->GetWritableSize(streamInfo.sessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->UpdateSpatializationState(streamInfo.sessionId + 1, true, false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->UpdateSpatializationState(streamInfo.sessionId, true, false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->UpdateMaxLength(streamInfo.sessionId, TEST_SLEEP_TIME_20), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetOffloadRenderCallbackType(streamInfo.sessionId, CB_FLUSH_COMPLETED), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File1"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File"), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerEffectTest002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioSpatializationState stateInfo;
    stateInfo.headTrackingEnabled = false;
    stateInfo.spatializationEnabled = false;
    EXPECT_EQ(hpaeManager_->UpdateSpatializationState(stateInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->UpdateSpatialDeviceType(EARPHONE_TYPE_INEAR), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    
    AudioEffectPropertyArray property;
    EXPECT_EQ(hpaeManager_->GetAudioEffectProperty(property), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetAudioEffectProperty(property), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->UpdateEffectBtOffloadSupported(true);
    EXPECT_EQ(hpaeManager_->SetOutputDevice(TEST_STREAM_SESSION_ID, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetMicrophoneMuteInfo(false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->GetAudioEnhanceProperty(property, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetAudioEnhanceProperty(property, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerSuspend002, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::string deviceName = "virtual1";
    EXPECT_EQ(hpaeManager_->SuspendAudioDevice(deviceName, true), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSinkMute(deviceName, true), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(TEST_STREAM_SESSION_ID, true), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerMoveFailed002, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RUNNING);
    hpaeManager_->HandleMoveSessionFailed(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_STREAM_SESSION_ID,
        MOVE_SINGLE, "Speaker_File1");
    EXPECT_EQ(hpaeManager_->movingIds_.size(), 0);

    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RUNNING);
    hpaeManager_->HandleMoveSessionFailed(HPAE_STREAM_CLASS_TYPE_RECORD, TEST_STREAM_SESSION_ID,
        MOVE_SINGLE, "Speaker_File1");
    EXPECT_EQ(hpaeManager_->movingIds_.size(), 0);

    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RUNNING);
    hpaeManager_->HandleMoveSessionFailed(HPAE_STREAM_CLASS_TYPE_RECORD, TEST_STREAM_SESSION_ID,
        MOVE_PREFER, "Speaker_File1");
    EXPECT_EQ(hpaeManager_->movingIds_.size(), 0);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerMarkClosingSinkPrefer001, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->rendererIdSinkNameMap_.emplace(TEST_STREAM_SESSION_ID, "speaker_file");
    EXPECT_EQ(hpaeManager_->idPreferSinkNameMap_.size() == 0, true);
    hpaeManager_->MarkClosingSinkPrefer("speaker_file");
    EXPECT_EQ(hpaeManager_->idPreferSinkNameMap_.size() == 1, true);
}

HWTEST_F(HpaeManagerUnitTest, HpaeRenderManagerReloadTest001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();

    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();

    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetCloseAudioPortResult(), SUCCESS);

    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();

    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    EXPECT_EQ(hpaeManager_->IsRunning(), false);
}

HWTEST_F(HpaeManagerUnitTest, HpaeRenderManagerReloadTest002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    audioModuleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, HpaeRenderManagerReloadTest003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();

    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();

    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetCloseAudioPortResult(), SUCCESS);

    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();

    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    EXPECT_EQ(hpaeManager_->IsRunning(), false);
}

/**
 * @tc.name  : Test UpdateCollaborativeState
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_001
 * @tc.desc  : Test UpdateCollaborativeState when config in vaild.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    int32_t ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_CLOSE);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerDumpStreamInfoTest, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);

    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sinkAudioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sinkPortId = callback->GetPortId();
    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sourceAudioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sourcePortId = callback->GetPortId();

    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterHpaeDumpCallback(dumpCallback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    HpaeStreamInfo rendererStreamInfo = GetRenderStreamInfo();
    rendererStreamInfo.deviceName = sinkAudioModuleInfo.name;  // keep on real sink, matching original routing
    hpaeManager_->CreateStream(rendererStreamInfo);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DumpSinkInputsInfo();
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(dumpCallback->GetSinkInputsSize(), 1);
    EXPECT_EQ(
        hpaeManager_->ShouldNotSkipProcess(rendererStreamInfo.streamClassType, rendererStreamInfo.sessionId), true);
    

    HpaeStreamInfo capturerStreamInfo = GetCaptureStreamInfo();
    capturerStreamInfo.deviceName = sourceAudioModuleInfo.name;
    hpaeManager_->CreateStream(capturerStreamInfo);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DumpSourceOutputsInfo();
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(dumpCallback->GetSourceOutputsSize(), 1);
    EXPECT_EQ(
        hpaeManager_->ShouldNotSkipProcess(capturerStreamInfo.streamClassType, capturerStreamInfo.sessionId), true);
    
    EXPECT_EQ(hpaeManager_->ShouldNotSkipProcess(HPAE_STREAM_CLASS_TYPE_INVALID, TEST_STREAM_SESSION_ID), false);
    hpaeManager_->CloseAudioPort(sinkPortId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->CloseAudioPort(sourcePortId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerTestHidumperWithoutCallback, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);

    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sinkAudioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sinkPortId = callback->GetPortId();
    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sourceAudioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t sourcePortId = callback->GetPortId();

    hpaeManager_->DumpSinkInfo(sinkAudioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DumpSourceInfo(sourceAudioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DumpAllAvailableDevice();
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DumpSinkInputsInfo();
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DumpSourceOutputsInfo();
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->CloseAudioPort(sinkPortId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->CloseAudioPort(sourcePortId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerGetSinkAndSourceInfoTest_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    HpaeSinkInfo sinkInfo;
    HpaeSourceInfo sourceInfo;

    int32_t ret = -1;
    EXPECT_EQ(hpaeManager_->GetSinkInfoByIdx(0,
        [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
            sinkInfo = sinkInfoRet;
            ret = result;
    }), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(ret, ERROR);

    ret = -1;
    EXPECT_EQ(hpaeManager_->GetSourceInfoByIdx(0,
        [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
            sourceInfo  = sourceInfoRet;
            ret = result;
    }), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerGetSinkAndSourceInfoTest_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    HpaeSinkInfo sinkInfo;
    HpaeSourceInfo sourceInfo;

    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();
    int32_t ret = -1;
    EXPECT_EQ(hpaeManager_->GetSinkInfoByIdx(portId,
        [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
            sinkInfo = sinkInfoRet;
            ret = result;
    }), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(std::to_string(sinkInfo.channels) == audioModuleInfo1.channels, true);
    EXPECT_EQ(std::to_string(sinkInfo.samplingRate) == audioModuleInfo1.rate, true);
    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo2 = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo2), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();
    ret = -1;
    EXPECT_EQ(hpaeManager_->GetSourceInfoByIdx(portId,
        [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
            sourceInfo  = sourceInfoRet;
            ret = result;
    }), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(std::to_string(sourceInfo.channels) == audioModuleInfo2.channels, true);
    EXPECT_EQ(std::to_string(sourceInfo.samplingRate) == audioModuleInfo2.rate, true);
    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, CloseInCoreAudioPort_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::string sourceName = "mic";
    HpaeSourceInfo sourceInfo;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_.insert_or_assign(sourceName, capturerManager);
    hpaeManager_->defaultSource_ = "mic";
    hpaeManager_->coreSource_ = "virtual";
    EXPECT_EQ(hpaeManager_->CloseInAudioPort(sourceName), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, CloseInCoreAudioPort_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::string sourceName = "mic1";
    HpaeSourceInfo sourceInfo;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_.insert_or_assign(sourceName, capturerManager);
    hpaeManager_->defaultSource_ = "mic";
    hpaeManager_->coreSource_ = "virtual";
    EXPECT_EQ(hpaeManager_->CloseInAudioPort(sourceName), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, CloseInCoreAudioPort_003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::string sourceName = "mic";
    HpaeSourceInfo sourceInfo;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_.insert_or_assign(sourceName, capturerManager);
    hpaeManager_->defaultSource_ = "mic";
    hpaeManager_->coreSource_ = "mic";
    EXPECT_EQ(hpaeManager_->CloseInAudioPort(sourceName), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, CloseInCoreAudioPort_004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::string sourceName = "mic1";
    HpaeSourceInfo sourceInfo;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_.insert_or_assign(sourceName, capturerManager);
    hpaeManager_->defaultSource_ = "mic";
    hpaeManager_->coreSource_ = "mic";
    EXPECT_EQ(hpaeManager_->CloseInAudioPort(sourceName), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, CloseInCoreAudioPort_005, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    std::string sourceName = "mic1";
    std::string coreSourceName = "mic";
    HpaeSourceInfo sourceInfo;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_.insert_or_assign(sourceName, capturerManager);
    hpaeManager_->capturerManagerMap_.insert_or_assign(coreSourceName, capturerManager);
    hpaeManager_->defaultSource_ = "mic";
    hpaeManager_->coreSource_ = "mic";
    EXPECT_EQ(hpaeManager_->CloseInAudioPort(sourceName), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, InjectorToPrimaryCapturer, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo moduleInfo = GetSinkAudioModeInfo(VIRTUAL_INJECTOR);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(moduleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t injectorPortId = callback->GetPortId();

    auto it = hpaeManager_->sinkVirtualOutputNodeMap_.find(injectorPortId);
    EXPECT_EQ(it != hpaeManager_->sinkVirtualOutputNodeMap_.end(), true);
    auto sinkOutputNode = it->second;
    EXPECT_EQ(sinkOutputNode != nullptr, true);
    HpaeNodeInfo nodeInfo = sinkOutputNode->GetNodeInfo();
    EXPECT_EQ(moduleInfo.channels, std::to_string(nodeInfo.channels));
    EXPECT_EQ(moduleInfo.rate, std::to_string(nodeInfo.samplingRate));

    AudioModuleInfo moduleInfo2 = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(moduleInfo2), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t capturerPortId = callback->GetPortId();

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = moduleInfo.name;
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);

    HpaeStreamInfo streamInfo2 = GetCaptureStreamInfo();
    streamInfo2.deviceName = moduleInfo2.name;
    streamInfo2.sourceType = SOURCE_TYPE_MIC;
    hpaeManager_->CreateStream(streamInfo2);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->AddCaptureInjector(injectorPortId, capturerPortId, streamInfo2.sourceType);
    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo2.sessionId);
    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId);
    sleep(3); // 3s for sleep
    hpaeManager_->Stop(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->Stop(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo2.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->RemoveCaptureInjector(injectorPortId, capturerPortId, streamInfo2.sourceType);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->CloseAudioPort(injectorPortId);
    hpaeManager_->CloseAudioPort(capturerPortId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, InjectorToEndPointCapturer, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo moduleInfo = GetSinkAudioModeInfo(VIRTUAL_INJECTOR);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(moduleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t injectorPortId = callback->GetPortId();

    auto it = hpaeManager_->sinkVirtualOutputNodeMap_.find(injectorPortId);
    EXPECT_EQ(it != hpaeManager_->sinkVirtualOutputNodeMap_.end(), true);
    auto sinkOutputNode = it->second;
    ASSERT_EQ(sinkOutputNode != nullptr, true);
    HpaeNodeInfo nodeInfo = sinkOutputNode->GetNodeInfo();
    EXPECT_EQ(moduleInfo.channels, std::to_string(nodeInfo.channels));
    EXPECT_EQ(moduleInfo.rate, std::to_string(nodeInfo.samplingRate));

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = moduleInfo.name;
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->Stop(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->CloseAudioPort(injectorPortId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, InjectorUpdataAudioPortInfoAndReloadTest, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);

    AudioModuleInfo moduleInfo = GetSinkAudioModeInfo(VIRTUAL_INJECTOR);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(moduleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t injectorPortId = callback->GetPortId();

    auto it = hpaeManager_->sinkVirtualOutputNodeMap_.find(injectorPortId);
    ASSERT_EQ(it != hpaeManager_->sinkVirtualOutputNodeMap_.end(), true);
    auto sinkOutputNode = it->second;
    ASSERT_EQ(sinkOutputNode != nullptr, true);
    HpaeNodeInfo &nodeInfo = sinkOutputNode->GetNodeInfo();
    EXPECT_EQ(moduleInfo.channels, std::to_string(nodeInfo.channels));
    EXPECT_EQ(moduleInfo.rate, std::to_string(nodeInfo.samplingRate));

    moduleInfo.channels = "4";
    moduleInfo.rate = "16000";
    hpaeManager_->UpdateAudioPortInfo(injectorPortId, moduleInfo);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(moduleInfo.channels, std::to_string(nodeInfo.channels));
    EXPECT_EQ(moduleInfo.rate, std::to_string(nodeInfo.samplingRate));

    moduleInfo.channels = "0";
    hpaeManager_->UpdateAudioPortInfo(injectorPortId, moduleInfo);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_NE(moduleInfo.channels, std::to_string(nodeInfo.channels));

    moduleInfo.channels = "2";
    moduleInfo.rate = "44100";
    hpaeManager_->ReloadRenderManager(moduleInfo, true);
    EXPECT_EQ(moduleInfo.channels, std::to_string(nodeInfo.channels));
    EXPECT_EQ(moduleInfo.rate, std::to_string(nodeInfo.samplingRate));

    hpaeManager_->CloseAudioPort(injectorPortId);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, DeleteAudioPort_TEST_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager_ = std::make_shared<HpaeRendererManager>(sinkInfo);
    hpaeManager_->sinkNameSinkIdMap_["Speaker_File"] = 1;
    hpaeManager_->rendererManagerMap_["Speaker_File"] = hpaeRendererManager_;
    hpaeManager_->sinkIdSinkNameMap_[1] = "Speaker_File";
    hpaeManager_->DeleteAudioport("Speaker_File");
    EXPECT_EQ(hpaeManager_->sinkNameSinkIdMap_.size(), 0);  // raw removal shrinks the map
    // Virtual_Render is lazy: absent before the first real port
    EXPECT_EQ(hpaeManager_->sinkNameSinkIdMap_.count("Virtual_Render"), 0);

    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->sourceNameSourceIdMap_["mic"] = 2;
    hpaeManager_->capturerManagerMap_["mic"] = capturerManager;
    hpaeManager_->DeleteAudioport("mic");
    EXPECT_EQ(hpaeManager_->sourceNameSourceIdMap_.size(), 0);
    hpaeManager_->DeleteAudioport("mic");
    EXPECT_EQ(hpaeManager_->sourceNameSourceIdMap_.size(), 0);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerGetPreferSInk001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode = CreateTestNode(HPAE_SESSION_RUNNING);
    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RUNNING);
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    sinkInputs.emplace_back(sinkInputNode);
    sinkInputs.emplace_back(nullptr);
    std::vector<std::shared_ptr<HpaeSinkInputNode>> results = hpaeManager_->GetPerferSinkInputs(sinkInputs);
    EXPECT_EQ(results.size(), 1);
    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RELEASED);
    results = hpaeManager_->GetPerferSinkInputs(sinkInputs);
    EXPECT_EQ(results.size(), 0);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerGetUsedMoveInfos001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = TEST_STREAM_SESSION_ID;
    moveInfo.sessionInfo.state = HPAE_SESSION_RUNNING;
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    moveInfos.emplace_back(moveInfo);
    HpaeCaptureMoveInfo moveInfo1;
    moveInfo1.sessionId = TEST_STREAM_UID;
    moveInfo1.sessionInfo.state = HPAE_SESSION_RUNNING;
    moveInfos.emplace_back(moveInfo1);
    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RUNNING);
    std::vector<HpaeCaptureMoveInfo> results = hpaeManager_->GetUsedMoveInfos(moveInfos);
    EXPECT_EQ(results.size(), 2);
    hpaeManager_->movingIds_.emplace(TEST_STREAM_SESSION_ID, HPAE_SESSION_RELEASED);
    results = hpaeManager_->GetUsedMoveInfos(moveInfos);
    EXPECT_EQ(results.size(), 1);
}

HWTEST_F(HpaeManagerUnitTest, FlushDrainCoveredRoutedPlayAndRecordBranches001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    const uint32_t playSessionId = TEST_STREAM_SESSION_ID + 100;
    const uint32_t recordSessionId = TEST_STREAM_SESSION_ID + 101;
    const uint32_t innerRecordSessionId = TEST_STREAM_SESSION_ID + 102;
    const std::string sinkName = "Speaker_File";
    const std::string sourceName = "mic";
    const std::string innerSourceName = "inner_capture";

    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto rendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    hpaeManager_->rendererManagerMap_[sinkName] = rendererManager;
    hpaeManager_->rendererManagerMap_[innerSourceName] = rendererManager;

    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_[sourceName] = capturerManager;

    HpaeSessionInfo playSessionInfo;
    playSessionInfo.streamInfo = GetRenderStreamInfo();
    playSessionInfo.streamInfo.sessionId = playSessionId;
    playSessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeManager_->rendererIdSinkNameMap_[playSessionId] = sinkName;
    hpaeManager_->rendererIdStreamInfoMap_[playSessionId] = playSessionInfo;

    HpaeSessionInfo recordSessionInfo;
    recordSessionInfo.streamInfo = GetCaptureStreamInfo();
    recordSessionInfo.streamInfo.sessionId = recordSessionId;
    recordSessionInfo.streamInfo.sourceType = SOURCE_TYPE_MIC;
    recordSessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeManager_->capturerIdSourceNameMap_[recordSessionId] = sourceName;
    hpaeManager_->capturerIdStreamInfoMap_[recordSessionId] = recordSessionInfo;

    HpaeSessionInfo innerRecordSessionInfo = recordSessionInfo;
    innerRecordSessionInfo.streamInfo.sessionId = innerRecordSessionId;
    innerRecordSessionInfo.streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    hpaeManager_->capturerIdSourceNameMap_[innerRecordSessionId] = innerSourceName;
    hpaeManager_->capturerIdStreamInfoMap_[innerRecordSessionId] = innerRecordSessionInfo;

    EXPECT_EQ(hpaeManager_->Flush(HPAE_STREAM_CLASS_TYPE_PLAY, playSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Drain(HPAE_STREAM_CLASS_TYPE_PLAY, playSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Flush(HPAE_STREAM_CLASS_TYPE_RECORD, recordSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Drain(HPAE_STREAM_CLASS_TYPE_RECORD, recordSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Flush(HPAE_STREAM_CLASS_TYPE_RECORD, innerRecordSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Drain(HPAE_STREAM_CLASS_TYPE_RECORD, innerRecordSessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, InnerRecordStartPauseStopRoutesToRendererManager001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    const uint32_t innerStartSessionId = TEST_STREAM_SESSION_ID + 110;
    const uint32_t innerPauseSessionId = TEST_STREAM_SESSION_ID + 111;
    const uint32_t innerStopSessionId = TEST_STREAM_SESSION_ID + 112;
    const std::string innerSourceName = "inner_capture";

    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto rendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    hpaeManager_->rendererManagerMap_[innerSourceName] = rendererManager;

    HpaeSessionInfo innerStartSessionInfo;
    innerStartSessionInfo.streamInfo = GetCaptureStreamInfo();
    innerStartSessionInfo.streamInfo.sessionId = innerStartSessionId;
    innerStartSessionInfo.streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    innerStartSessionInfo.state = HPAE_SESSION_PREPARED;
    hpaeManager_->capturerIdSourceNameMap_[innerStartSessionId] = innerSourceName;
    hpaeManager_->capturerIdStreamInfoMap_[innerStartSessionId] = innerStartSessionInfo;

    HpaeSessionInfo innerPauseSessionInfo = innerStartSessionInfo;
    innerPauseSessionInfo.streamInfo.sessionId = innerPauseSessionId;
    innerPauseSessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeManager_->capturerIdSourceNameMap_[innerPauseSessionId] = innerSourceName;
    hpaeManager_->capturerIdStreamInfoMap_[innerPauseSessionId] = innerPauseSessionInfo;

    HpaeSessionInfo innerStopSessionInfo = innerStartSessionInfo;
    innerStopSessionInfo.streamInfo.sessionId = innerStopSessionId;
    innerStopSessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeManager_->capturerIdSourceNameMap_[innerStopSessionId] = innerSourceName;
    hpaeManager_->capturerIdStreamInfoMap_[innerStopSessionId] = innerStopSessionInfo;

    EXPECT_EQ(hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_RECORD, innerStartSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Pause(HPAE_STREAM_CLASS_TYPE_RECORD, innerPauseSessionId), SUCCESS);
    EXPECT_EQ(hpaeManager_->Stop(HPAE_STREAM_CLASS_TYPE_RECORD, innerStopSessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

static void PrepareCoverageManager(std::shared_ptr<HpaeManager> &hpaeManager)
{
    EXPECT_NE(hpaeManager, nullptr);
    hpaeManager->Init();
    EXPECT_EQ(hpaeManager->IsInit(), true);
    sleep(1);
}

struct CoverageRouteInfo {
    HpaeSinkInfo sinkInfo;
    HpaeSourceInfo sourceInfo;
};

static CoverageRouteInfo AddCoverageRouteManagers(std::shared_ptr<HpaeManager> &hpaeManager,
    const std::string &sinkName, const std::string &sourceName, const std::string &innerSourceName)
{
    CoverageRouteInfo routeInfo;
    routeInfo.sinkInfo = GetSinkInfo();
    routeInfo.sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_FAST;
    auto rendererManager = std::make_shared<HpaeRendererManager>(routeInfo.sinkInfo);
    hpaeManager->rendererManagerMap_[sinkName] = rendererManager;
    hpaeManager->rendererManagerMap_[innerSourceName] = rendererManager;

    InitSourceInfo(routeInfo.sourceInfo);
    routeInfo.sourceInfo.routeFlag = AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(routeInfo.sourceInfo);
    hpaeManager->capturerManagerMap_[sourceName] = capturerManager;
    return routeInfo;
}

static HpaeSessionInfo AddCoveragePlaySession(std::shared_ptr<HpaeManager> &hpaeManager,
    uint32_t sessionId, const std::string &sinkName)
{
    HpaeSessionInfo sessionInfo;
    sessionInfo.streamInfo = GetRenderStreamInfo();
    sessionInfo.streamInfo.sessionId = sessionId;
    sessionInfo.streamInfo.frameLen = FRAME_LENGTH;
    sessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeManager->rendererIdSinkNameMap_[sessionId] = sinkName;
    hpaeManager->rendererIdStreamInfoMap_[sessionId] = sessionInfo;
    return sessionInfo;
}

static HpaeSessionInfo AddCoverageRecordSession(std::shared_ptr<HpaeManager> &hpaeManager,
    uint32_t sessionId, const std::string &sourceName, SourceType sourceType)
{
    HpaeSessionInfo sessionInfo;
    sessionInfo.streamInfo = GetCaptureStreamInfo();
    sessionInfo.streamInfo.sessionId = sessionId;
    sessionInfo.streamInfo.frameLen = FRAME_LENGTH + 1;
    sessionInfo.streamInfo.sourceType = sourceType;
    sessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeManager->capturerIdSourceNameMap_[sessionId] = sourceName;
    hpaeManager->capturerIdStreamInfoMap_[sessionId] = sessionInfo;
    return sessionInfo;
}

HWTEST_F(HpaeManagerUnitTest, StreamSpanRouteAndInnerReadCallbackBranches001, TestSize.Level1)
{
    PrepareCoverageManager(hpaeManager_);

    const uint32_t playSessionId = TEST_STREAM_SESSION_ID + 120;
    const uint32_t recordSessionId = TEST_STREAM_SESSION_ID + 121;
    const uint32_t innerRecordSessionId = TEST_STREAM_SESSION_ID + 122;
    const std::string sinkName = "Speaker_File";
    const std::string sourceName = "mic";
    const std::string innerSourceName = "inner_capture";

    CoverageRouteInfo routeInfo = AddCoverageRouteManagers(hpaeManager_, sinkName, sourceName, innerSourceName);
    HpaeSessionInfo playSessionInfo = AddCoveragePlaySession(hpaeManager_, playSessionId, sinkName);
    HpaeSessionInfo recordSessionInfo = AddCoverageRecordSession(
        hpaeManager_, recordSessionId, sourceName, SOURCE_TYPE_MIC);
    AddCoverageRecordSession(hpaeManager_, innerRecordSessionId, innerSourceName, SOURCE_TYPE_PLAYBACK_CAPTURE);

    uint32_t spanSizeInFrame = 0;
    EXPECT_EQ(hpaeManager_->GetStreamSpanSize(HPAE_STREAM_CLASS_TYPE_PLAY, playSessionId, spanSizeInFrame), SUCCESS);
    EXPECT_EQ(spanSizeInFrame, static_cast<uint32_t>(playSessionInfo.streamInfo.frameLen));
    EXPECT_EQ(hpaeManager_->GetStreamSpanSize(
        HPAE_STREAM_CLASS_TYPE_RECORD, recordSessionId, spanSizeInFrame), SUCCESS);
    EXPECT_EQ(spanSizeInFrame, static_cast<uint32_t>(recordSessionInfo.streamInfo.frameLen));
    EXPECT_EQ(hpaeManager_->GetStreamSpanSize(HPAE_STREAM_CLASS_TYPE_INVALID, recordSessionId, spanSizeInFrame),
        ERR_INVALID_OPERATION);

    uint32_t routeFlag = 0;
    EXPECT_EQ(hpaeManager_->GetStreamRouteFlag(HPAE_STREAM_CLASS_TYPE_PLAY, playSessionId, routeFlag), SUCCESS);
    EXPECT_EQ(routeFlag, routeInfo.sinkInfo.routeFlag);
    EXPECT_EQ(hpaeManager_->GetStreamRouteFlag(HPAE_STREAM_CLASS_TYPE_RECORD, recordSessionId, routeFlag), SUCCESS);
    EXPECT_EQ(routeFlag, routeInfo.sourceInfo.routeFlag);
    hpaeManager_->rendererManagerMap_.erase(sinkName);
    EXPECT_EQ(hpaeManager_->GetStreamRouteFlag(HPAE_STREAM_CLASS_TYPE_PLAY, playSessionId, routeFlag),
        ERR_INVALID_OPERATION);
    hpaeManager_->rendererManagerMap_[sinkName] = hpaeManager_->rendererManagerMap_[innerSourceName];

    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>(ROOT_PATH);
    EXPECT_EQ(hpaeManager_->RegisterReadCallback(innerRecordSessionId, readDataCb), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : Test IsSessionHdiSpeedSupported query branches
 * @tc.type  : FUNC
 * @tc.number: IsSessionHdiSpeedSupported_001
 * @tc.desc  : Per-session HDI speed capability: unknown session, normal chain and missing sink.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(HpaeManagerUnitTest, IsSessionHdiSpeedSupported_001, TestSize.Level1)
{
    PrepareCoverageManager(hpaeManager_);

    const uint32_t playSessionId = TEST_STREAM_SESSION_ID + 123;
    const std::string sinkName = "Speaker_File";
    AddCoverageRouteManagers(hpaeManager_, sinkName, "mic", "inner_capture");
    AddCoveragePlaySession(hpaeManager_, playSessionId, sinkName);

    // Unknown session and normal-manager chain both report no HDI speed capability.
    EXPECT_FALSE(hpaeManager_->IsSessionHdiSpeedSupported(TEST_STREAM_SESSION_ID + 999));
    EXPECT_FALSE(hpaeManager_->IsSessionHdiSpeedSupported(playSessionId));

    // Sink manager missing from the map keeps the conservative false answer.
    hpaeManager_->rendererManagerMap_.erase(sinkName);
    EXPECT_FALSE(hpaeManager_->IsSessionHdiSpeedSupported(playSessionId));
}

/**
 * @tc.name  : Test IsSessionHdiSpeedSupported true path via the sync result chain
 * @tc.type  : FUNC
 * @tc.number: IsSessionHdiSpeedSupported_002
 * @tc.desc  : Offload manager chain reports HDI speed capability through SendRequestSync.
 */
HWTEST_F(HpaeManagerUnitTest, IsSessionHdiSpeedSupported_002, TestSize.Level1)
{
    PrepareCoverageManager(hpaeManager_);

    const uint32_t playSessionId = TEST_STREAM_SESSION_ID + 124;
    const std::string sinkName = "Speaker_Offload";
    HpaeSinkInfo sinkInfo;
    hpaeManager_->rendererManagerMap_[sinkName] = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    hpaeManager_->rendererIdSinkNameMap_[playSessionId] = sinkName;

    // Only the promise/future result path can deliver true (the timeout fallback returns
    // false), so this also proves the request was executed and answered on the manager thread.
    EXPECT_TRUE(hpaeManager_->IsSessionHdiSpeedSupported(playSessionId));
}

HWTEST_F(HpaeManagerUnitTest, StopAudioPort001, TestSize.Level4)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    std::string sourceName = "test";
    HpaeSourceInfo sourceInfo;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerManagerMap_.insert_or_assign(sourceName, capturerManager);
    hpaeManager_->rendererManagerMap_["test"] = nullptr;
    EXPECT_EQ(hpaeManager_->StopAudioPort("test"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, StopAudioPort002, TestSize.Level4)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    std::string sourceName = "test";
    HpaeSinkInfo sinkInfo;
    auto rendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    hpaeManager_->rendererManagerMap_.insert_or_assign(sourceName, rendererManager);
    EXPECT_EQ(hpaeManager_->StopAudioPort("test"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, StopAudioPort003, TestSize.Level4)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    hpaeManager_->capturerManagerMap_["test"] = nullptr;
    hpaeManager_->rendererManagerMap_["test"] = nullptr;
    EXPECT_EQ(hpaeManager_->StopAudioPort("test"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, TriggerAppsUidUpdate001, TestSize.Level4)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    hpaeManager_->capturerManagerMap_["test"] = nullptr;
    hpaeManager_->rendererManagerMap_["test"] = nullptr;
    hpaeManager_->TriggerAppsUidUpdate(HPAE_STREAM_CLASS_TYPE_INVALID, 1);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_NE(hpaeManager_->capturerManagerMap_.size(), 0);
}

HWTEST_F(HpaeManagerUnitTest, CreateRendererManager_Test_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo("Bt_Speaker");
    hpaeManager_->CreateRendererManager(audioModuleInfo, 1, false);
    std::shared_ptr<IHpaeRendererManager> rendererManager = hpaeManager_->GetRendererManagerByName("Bt_Speaker");
    EXPECT_NE(rendererManager, nullptr);
}

/**
 * @tc.name  : Test HpaeManager SetAppsEnhanceMuteState API
 * @tc.type  : FUNC
 * @tc.number: HandleAppsEnhanceMuteState_001
 * @tc.desc  : Test HpaeManager SetAppsEnhanceMuteState basic functionality
 */
HWTEST_F(HpaeManagerUnitTest, HandleAppsEnhanceMuteState_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);

    uint32_t sessionId = 123456;
    // Test set apps mute state
    EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, true));
    EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, false));
}

/**
 * @tc.name  : Test HpaeManager SetAppsEnhanceMuteState API
 * @tc.type  : FUNC
 * @tc.number: HandleAppsEnhanceMuteState_002
 * @tc.desc  : Test HpaeManager SetAppsEnhanceMuteState with multiple calls
 */
HWTEST_F(HpaeManagerUnitTest, HandleAppsEnhanceMuteState_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);

    // Test multiple calls with different states
    uint32_t sessionId = 111111;
    EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, true));
    EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, false));
    EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, true));
    EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, false));
}

/**
 * @tc.name  : Test HpaeManager SetAppsEnhanceMuteState API
 * @tc.type  : FUNC
 * @tc.number: HandleAppsEnhanceMuteState_003
 * @tc.desc  : Test HpaeManager SetAppsEnhanceMuteState with different session ids
 */
HWTEST_F(HpaeManagerUnitTest, HandleAppsEnhanceMuteState_003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsInit(), true);

    // Test with different session ids
    std::vector<uint32_t> sessionIds = {123, 456, 789, 123456789};
    for (auto sessionId : sessionIds) {
        EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, true));
        EXPECT_EQ(SUCCESS, hpaeManager_->SetAppsEnhanceMuteState(sessionId, false));
    }
}

// ==================== SendRequestSync Tests ====================

/**
 * @brief SendRequestSync returns actual result from lambda executed on HpaeManagerThread
 */
HWTEST_F(HpaeManagerUnitTest, sendRequestSyncReturnsActualResult, TestSize.Level0)
{
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    auto result = hpaeManager_->SendRequestSync<int32_t>(
        []() -> int32_t { return 42; },
        "TestReturn42",
        -1);

    EXPECT_EQ(result, 42);
    hpaeManager_->DeInit();
}

/**
 * @brief SendRequestSync returns default value when lambda returns error
 */
HWTEST_F(HpaeManagerUnitTest, sendRequestSyncReturnsErrorFromLambda, TestSize.Level0)
{
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    auto result = hpaeManager_->SendRequestSync<int32_t>(
        []() -> int32_t { return TEST_SYNC_ERROR; },
        "TestReturnError",
        SUCCESS);

    EXPECT_EQ(result, TEST_SYNC_ERROR);
    hpaeManager_->DeInit();
}

/**
 * @brief SendRequestSync returns default value on timeout when thread is not running
 */
HWTEST_F(HpaeManagerUnitTest, sendRequestSyncReturnsDefaultOnTimeout, TestSize.Level0)
{
    // Don't Init - no HpaeManagerThread running, so the request won't be processed
    EXPECT_EQ(hpaeManager_->IsRunning(), false);

    auto result = hpaeManager_->SendRequestSync<int32_t>(
        []() -> int32_t { return 42; },
        "TestTimeout",
        -1,
        100);  // 100ms timeout for fast test

    EXPECT_EQ(result, -1);  // Should return default value due to timeout
}

/**
 * @brief SendRequestSync works with multiple sequential calls
 */
HWTEST_F(HpaeManagerUnitTest, sendRequestSyncSequentialCalls, TestSize.Level0)
{
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    int32_t counter = 0;
    for (int i = 0; i < 5; i++) {
        std::string funcName = "TestSequential_" + std::to_string(i);
        auto result = hpaeManager_->SendRequestSync<int32_t>(
            [&counter]() -> int32_t { return ++counter; },
            funcName,
            -1);
        EXPECT_EQ(result, i + 1);
    }
    EXPECT_EQ(counter, 5);
    hpaeManager_->DeInit();
}

/**
 * @brief SetAissEnabledSync returns error when sessionEffectManager_ is null (before Init)
 */
HWTEST_F(HpaeManagerUnitTest, SetAissEnabled_NullSessionEffectManager, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    // Do NOT Init — sessionEffectManager_ stays null
    EXPECT_EQ(hpaeManager_->SetAissEnabledSync(12345, true), ERR_OPERATION_FAILED);
    EXPECT_EQ(hpaeManager_->SetAissEnabledSync(12345, false), ERR_OPERATION_FAILED);
}

/**
 * @brief SetAissEnabledSync returns error when rendererManager not found for sessionId
 */
HWTEST_F(HpaeManagerUnitTest, SetAissEnabled_NoRendererManager, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);
    // Session 99999 has no rendererManager — should return ERR_SESSION_NOT_FOUND
    EXPECT_EQ(hpaeManager_->SetAissEnabledSync(99999, true),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
    hpaeManager_->DeInit();
}

/**
 * @brief PostCreateSessionEffect / PostDestroySessionEffect exist on IHpaeRendererManager interface.
 * Verifies the default (base-class) implementation does not crash.
 */
HWTEST_F(HpaeManagerUnitTest, PostSessionEffect_DefaultImplNoop, TestSize.Level0)
{
    // IHpaeRendererManager base-class default impl is no-op — calling them should not crash
    // We test this indirectly: CreateRendererManager gives us a concrete subclass,
    // but the base-class defaults are what non-RendererManager implementations get.
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceName = "test";
    auto rm = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    // The concrete renderer manager implements these — but if it's a nullptr sink,
    // it still shouldn't crash. The method exists on the interface.
    // We can't easily test the base-class default without a mock, but we verify it compiles.
    AudioEffectConfig config;
    auto dummyInstance = std::make_shared<EffectInstance>();
    // Just verifying the interface exists and is callable
    if (rm != nullptr) {
        rm->PostCreateSessionEffect(12345, "test_effect", dummyInstance, config);
        rm->PostDestroySessionEffect(12345);
    }
}

/**
 * @brief SetAissEnabledSync enable/disable round-trip with audio port and stream.
 * Verifies the async Post* path does not crash on a full pipeline.
 */
HWTEST_F(HpaeManagerUnitTest, SetAissEnabled_EnableDisableRoundTrip, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    // Setup audio port and stream
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessingStable(hpaeManager_);

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = audioModuleInfo.name;  // keep stream on real sink for the AISS chain
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessingStable(hpaeManager_);

    // Start the stream so SignalProcessThread is active and cluster has converter/gain nodes
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessingStable(hpaeManager_);

    // Enable AISS — should succeed (Bind + async PostCreateSessionEffect)
    int32_t ret = hpaeManager_->SetAissEnabledSync(streamInfo.sessionId, true);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessingStable(hpaeManager_);
    // Give async PostCreateSessionEffect time to execute on SignalProcessThread
    usleep(100000); // 100ms

    // Disable AISS — should succeed (async PostDestroySessionEffect + Unbind)
    ret = hpaeManager_->SetAissEnabledSync(streamInfo.sessionId, false);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessingStable(hpaeManager_);
    usleep(100000); // 100ms

    // Cleanup
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessingStable(hpaeManager_);
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessingStable(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File"), SUCCESS);
    hpaeManager_->DeInit();
}

// ==================== Upstream Tests ====================


HWTEST_F(HpaeManagerUnitTest, GetStreamSpanSize_001, TestSize.Level1)
{
    hpaeManager_->Init();
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback =
        std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);

    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sinkAudioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = sinkAudioModuleInfo.name;  // keep on real sink, matching original routing
    EXPECT_EQ(hpaeManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    uint32_t spanSizeInFrame = 0;
    int32_t ret = hpaeManager_->GetStreamSpanSize(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, spanSizeInFrame);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_GT(spanSizeInFrame, 0);

    EXPECT_EQ(hpaeManager_->DestroyStream(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseAudioPort(callback->GetPortId()), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

HWTEST_F(HpaeManagerUnitTest, GetStreamRouteFlag_Record_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.routeFlag = AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    hpaeManager_->capturerIdSourceNameMap_[TEST_CAPTURE_SESSION_ID] = "mic";
    hpaeManager_->capturerManagerMap_["mic"] = capturerManager;

    uint32_t routeFlag = 0;
    int32_t ret = hpaeManager_->GetStreamRouteFlag(HPAE_STREAM_CLASS_TYPE_RECORD, TEST_CAPTURE_SESSION_ID, routeFlag);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(routeFlag, sourceInfo.routeFlag);

    hpaeManager_->capturerManagerMap_.clear();
    ret = hpaeManager_->GetStreamRouteFlag(HPAE_STREAM_CLASS_TYPE_RECORD, TEST_CAPTURE_SESSION_ID, routeFlag);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

HWTEST_F(HpaeManagerUnitTest, GetRendererManagerByDeviceClass_EmptyString_003, TestSize.Level1)
{
    hpaeManager_->Init();
    sleep(1);

    auto rendererManager = hpaeManager_->GetRendererManagerByDeviceClass("");
    EXPECT_EQ(rendererManager, nullptr);

    hpaeManager_->DeInit();
}

HWTEST_F(HpaeManagerUnitTest, CoverageForwardingApis_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    std::vector<EffectChain> effectChains;
    EffectChainManagerParam effectChainManagerParam;
    std::vector<std::shared_ptr<AudioEffectLibEntry>> effectLibraryList;
    hpaeManager_->InitAudioEffectChainManager(effectChains, effectChainManagerParam, effectLibraryList);
    hpaeManager_->SetOutputDeviceSink(DEVICE_TYPE_SPEAKER, audioModuleInfo.name);
    EXPECT_EQ(hpaeManager_->SetSpatializationSceneType(SPATIALIZATION_SCENE_TYPE_DEFAULT), SUCCESS);
    EXPECT_EQ(hpaeManager_->EffectRotationUpdate(0), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetEffectSystemVolume(STREAM_MUSIC, 0.5f), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetAbsVolumeStateToEffect(true), SUCCESS);
    AudioEffectPropertyArray propertyArray;
    EXPECT_EQ(hpaeManager_->GetAudioEffectProperty(propertyArray), SUCCESS);
    hpaeManager_->InitHdiState();
    hpaeManager_->SetPersonalizedSpatializationStateResultCallback([](int32_t) {});
    hpaeManager_->UpdateParamExtra("test_main", "test_sub", "test_value");
    hpaeManager_->InitAudioEnhanceChainManager(effectChains, effectChainManagerParam, effectLibraryList);
    EXPECT_EQ(hpaeManager_->SetVolumeInfo(STREAM_MUSIC, 0.5f), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetStreamVolumeInfo(TEST_STREAM_SESSION_ID, 0.5f), SUCCESS);
    hpaeManager_->UpdateExtraSceneType("test_main", "test_sub", "test_scene");
    hpaeManager_->AddStreamVolumeToEffect("123456", 0.5f);
    hpaeManager_->DeleteStreamVolumeToEffect("123456");
    hpaeManager_->UpdatePersonalizedSpatializationState(true);
    EXPECT_EQ(hpaeManager_->UpdateCollaborativeProductId("test_product"), SUCCESS);
    EXPECT_EQ(hpaeManager_->UpdatePersonalizedHRTFBin(-1, 0), SUCCESS);
    hpaeManager_->LoadCollaborationConfig();
    EXPECT_EQ(hpaeManager_->SetAuxiliarySinkEnable(true), SUCCESS);
    hpaeManager_->HandleBypassSpatializationForStereo(true);
    hpaeManager_->SetSinkMuteForSwitchDevice("file_io", true);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(callback->GetGetAudioEffectPropertyResult(), SUCCESS);
    EXPECT_GE(hpaeManager_->GetMaxAmplitude(), 0.0f);
    hpaeManager_->NotifySettingsDataReady();
    hpaeManager_->NotifyAccountsChanged();
}

// ==================== SetEffectSystemVolumeWithDeviceType Tests ====================

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_001
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_001
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for SPEAKER device.
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.6f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_002
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_002
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for BLUETOOTH_A2DP device.
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.7f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_003
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_003
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for WIRED_HEADSET device.
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const int32_t systemVolumeType = STREAM_RING;
    const float systemVolume = 0.8f;
    const DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_MaxVolume_001
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_MaxVolume_001
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType with maximum volume (1.0f).
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_MaxVolume_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 1.0f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_MinVolume_001
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_MinVolume_001
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType with minimum volume (0.0f).
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_MinVolume_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.0f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_Multiple_001
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_Multiple_001
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for multiple device types sequentially.
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_Multiple_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const int32_t systemVolumeType = STREAM_MUSIC;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, 0.5f,
        DEVICE_TYPE_SPEAKER), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, 0.6f,
        DEVICE_TYPE_BLUETOOTH_A2DP), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, 0.7f,
        DEVICE_TYPE_WIRED_HEADSET), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_DifferentStreams_001
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_DifferentStreams_001
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for different stream types.
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_DifferentStreams_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(STREAM_MUSIC, 0.5f, deviceType), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(STREAM_RING, 0.6f, deviceType), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(STREAM_NOTIFICATION, 0.7f, deviceType), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DeInit();
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_WithoutInit_001
 * @tc.type  : FUNC
 * @tc.number: SetEffectSystemVolumeWithDeviceType_WithoutInit_001
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType without Init (should still succeed).
 */
HWTEST_F(HpaeManagerUnitTest, SetEffectSystemVolumeWithDeviceType_WithoutInit_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    // Do NOT call Init - test that the method handles this gracefully
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.5f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    // The method should still succeed as it sends a request asynchronously
    EXPECT_EQ(hpaeManager_->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType), SUCCESS);
}

HWTEST_F(HpaeManagerUnitTest, CoverageCallbackFailurePaths_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_GE(hpaeManager_->GetMsgCount(), 0);
    hpaeManager_->Invoke(static_cast<HpaeMsgCode>(-1), std::tuple<>());
    hpaeManager_->InvokeSync(static_cast<HpaeMsgCode>(-1), std::tuple<>());
    hpaeManager_->OnCallbackOpenOrReloadFailed(false);
    EXPECT_EQ(callback->GetPortId(), -1);
    hpaeManager_->HandleReloadDeviceResult("missing_device", SUCCESS);
    EXPECT_EQ(callback->GetPortId(), -1);
    hpaeManager_->HandleInitDeviceResult("missing_device", SUCCESS);
    EXPECT_EQ(callback->GetPortId(), -1);

    hpaeManager_->HandleMoveSessionFailed(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_STREAM_SESSION_ID, MOVE_SINGLE,
        "missing_sink");
    EXPECT_EQ(callback->GetMoveSinkInputByIndexOrNameResult(), ERROR_INVALID_PARAM);
    hpaeManager_->HandleMoveSessionFailed(HPAE_STREAM_CLASS_TYPE_RECORD, TEST_CAPTURE_SESSION_ID, MOVE_SINGLE,
        "missing_source");
    EXPECT_EQ(callback->GetMoveSourceOutputByIndexOrNameResult(), ERROR_INVALID_PARAM);

    std::shared_ptr<HpaeSinkInputNode> sinkInput = CreateTestNode(HPAE_SESSION_NEW);
    hpaeManager_->HandleMoveSinkInput(sinkInput, "missing_sink");
    EXPECT_EQ(callback->GetMoveSinkInputByIndexOrNameResult(), ERROR_INVALID_PARAM);
    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = TEST_CAPTURE_SESSION_ID;
    hpaeManager_->HandleMoveSourceOutput(moveInfo, "missing_source");
    EXPECT_EQ(callback->GetMoveSourceOutputByIndexOrNameResult(), ERROR_INVALID_PARAM);
}

HWTEST_F(HpaeManagerUnitTest, CoverageMovingPlaybackRelease_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->movingIds_[streamInfo.sessionId] = HPAE_SESSION_RUNNING;
    EXPECT_EQ(hpaeManager_->SetMovingStreamState(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId,
        HPAE_SESSION_RUNNING, HPAE_SESSION_RUNNING, OPERATION_STARTED), false);

    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId] = {streamInfo, HPAE_SESSION_RUNNING};
    hpaeManager_->sinkInputs_[streamInfo.sessionId].streamId = streamInfo.sessionId;
    hpaeManager_->idPreferSinkNameMap_[streamInfo.sessionId] = "Speaker_File";
    hpaeManager_->movingIds_[streamInfo.sessionId] = HPAE_SESSION_RUNNING;
    EXPECT_EQ(hpaeManager_->SetMovingStreamState(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId,
        HPAE_SESSION_RELEASED, HPAE_SESSION_RELEASED, OPERATION_RELEASED), true);
    EXPECT_EQ(hpaeManager_->sinkInputs_.count(streamInfo.sessionId), 0);
    EXPECT_EQ(hpaeManager_->idPreferSinkNameMap_.count(streamInfo.sessionId), 0);
}

HWTEST_F(HpaeManagerUnitTest, CoverageMovingRecordAndPending_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    HpaeStreamInfo captureInfo = GetCaptureStreamInfo();
    captureInfo.sessionId = TEST_CAPTURE_SESSION_ID;
    hpaeManager_->movingIds_[captureInfo.sessionId] = HPAE_SESSION_RUNNING;
    EXPECT_EQ(hpaeManager_->SetMovingStreamState(HPAE_STREAM_CLASS_TYPE_RECORD, captureInfo.sessionId,
        HPAE_SESSION_RUNNING, HPAE_SESSION_RUNNING, OPERATION_STARTED), false);

    hpaeManager_->capturerIdStreamInfoMap_[captureInfo.sessionId] = {captureInfo, HPAE_SESSION_RUNNING};
    hpaeManager_->sourceOutputs_[captureInfo.sessionId].streamId = captureInfo.sessionId;
    hpaeManager_->movingIds_[captureInfo.sessionId] = HPAE_SESSION_RUNNING;
    EXPECT_EQ(hpaeManager_->SetMovingStreamState(HPAE_STREAM_CLASS_TYPE_RECORD, captureInfo.sessionId,
        HPAE_SESSION_RELEASED, HPAE_SESSION_RELEASED, OPERATION_RELEASED), true);
    EXPECT_EQ(hpaeManager_->sourceOutputs_.count(captureInfo.sessionId), 0);

    captureInfo.sessionId = TEST_CAPTURE_SESSION_ID + 1;
    captureInfo.sourceType = SOURCE_TYPE_LIVE;
    hpaeManager_->effectLiveState_ = "NROFF";
    hpaeManager_->capturerIdStreamInfoMap_[captureInfo.sessionId] = {captureInfo, HPAE_SESSION_NEW};
    hpaeManager_->movingIds_[captureInfo.sessionId] = HPAE_SESSION_NEW;
    EXPECT_EQ(hpaeManager_->SetMovingStreamState(HPAE_STREAM_CLASS_TYPE_RECORD, captureInfo.sessionId,
        HPAE_SESSION_RUNNING, HPAE_SESSION_RUNNING, OPERATION_STARTED), true);

    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->pendingTransitionsTracker_.push_back({streamInfo.sessionId, HPAE_SESSION_PAUSED,
        OPERATION_PAUSED, std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(100)});
    hpaeManager_->rendererIdStreamInfoMap_[streamInfo.sessionId] = {streamInfo, HPAE_SESSION_PAUSING};
    EXPECT_EQ(hpaeManager_->ProcessPendingTransitionsAndGetNextDelay(), 0);
}

HWTEST_F(HpaeManagerUnitTest, CoverageAudioPortEdges_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo sinkInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sinkInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sinkInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_GE(callback->GetPortId(), 0);

    AudioModuleInfo mchSinkInfo = GetSinkAudioModeInfo("MCH_Speaker");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(mchSinkInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo sourceInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sourceInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(sourceInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    std::string sourceName = sourceInfo.name;
    EXPECT_EQ(hpaeManager_->SuspendAudioDevice(sourceName, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo innerCapturerInfo = GetSinkAudioModeInfo("Inner_Capture_Sink");
    innerCapturerInfo.lib = "libmodule-inner-capturer-sink.z.so";
    EXPECT_EQ(hpaeManager_->OpenAudioPort(innerCapturerInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : Test UpdateCollaborativeState with NEARLINK_OPEN
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_NearLink_002
 * @tc.desc  : Test UpdateCollaborativeState with COLLABORATION_NEARLINK_OPEN state.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_NearLink_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    int32_t ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_NEARLINK_OPEN);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_CLOSE);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

/**
 * @tc.name  : Test UpdateCollaborativeState switch BT to NearLink
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_SwitchBtToNl_003
 * @tc.desc  : Test switching from BLUETOOTH_OPEN to NEARLINK_OPEN.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_SwitchBtToNl_003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    int32_t ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_NEARLINK_OPEN);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_CLOSE);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

/**
 * @tc.name  : Test UpdateCollaborativeState same state no-op
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_NoChange_004
 * @tc.desc  : Test UpdateCollaborativeState with same state should log and return.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_NoChange_004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    int32_t ret = hpaeManager_->UpdateCollaborativeState(COLLABORATION_CLOSE);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

// Verify: target source not in capturerManagerMap_ → defaultSource_ must NOT change.
HWTEST_F(HpaeManagerUnitTest, IHpaeManagerSetDefaultSource002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    // Case 1: defaultSource_ starts empty, target not in map → must stay empty.
    hpaeManager_->defaultSource_ = "";
    EXPECT_EQ(hpaeManager_->SetDefaultSource("NotInMapSource"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->defaultSource_, "");

    // Case 2: defaultSource_ points to an opened & init'd source, target not in map
    //         → must remain unchanged.
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->defaultSource_ = audioModuleInfo.name;
    std::string before = hpaeManager_->defaultSource_;
    EXPECT_EQ(hpaeManager_->SetDefaultSource("AnotherMissingSource"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->defaultSource_, before);

    EXPECT_EQ(hpaeManager_->CloseInAudioPort(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

// Verify: target source exists and IsInit() == true → defaultSource_ switches.
HWTEST_F(HpaeManagerUnitTest, IHpaeManagerSetDefaultSource003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    AudioModuleInfo srcA = GetSourceAudioModeInfo("mic");
    AudioModuleInfo srcB = GetSourceAudioModeInfo("mic1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(srcA), SUCCESS);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(srcB), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    // Prime defaultSource_ to srcA, then switch to init'd srcB.
    hpaeManager_->defaultSource_ = srcA.name;
    EXPECT_EQ(hpaeManager_->SetDefaultSource(srcB.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->defaultSource_, srcB.name);

    EXPECT_EQ(hpaeManager_->CloseInAudioPort(srcA.name), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseInAudioPort(srcB.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : UpdateBypassSpatializationForStereo_StateTransition_001
 * @tc.desc  : UpdateBypassSpatializationForStereo flips bypassSpatializationForStereo_ when the
 *             incoming value differs from the cached one.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateBypassSpatializationForStereo_StateTransition_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    // Fresh manager defaults to false.
    hpaeManager_->bypassSpatializationForStereo_ = false;
    hpaeManager_->adaptiveSpatialRenderingEnabled_ = false;

    hpaeManager_->UpdateBypassSpatializationForStereo(true);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);

    hpaeManager_->UpdateBypassSpatializationForStereo(false);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, false);
}

/**
 * @tc.name  : UpdateBypassSpatializationForStereo_SameValueNoFlip_002
 * @tc.desc  : Calling UpdateBypassSpatializationForStereo with the same value as the cached one
 *             keeps bypassSpatializationForStereo_ unchanged.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateBypassSpatializationForStereo_SameValueNoFlip_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    hpaeManager_->bypassSpatializationForStereo_ = true;
    hpaeManager_->adaptiveSpatialRenderingEnabled_ = false;

    hpaeManager_->UpdateBypassSpatializationForStereo(true);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);

    hpaeManager_->bypassSpatializationForStereo_ = false;
    hpaeManager_->UpdateBypassSpatializationForStereo(false);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, false);
}

/**
 * @tc.name  : UpdateBypassSpatializationForStereo_AdaptiveRenderingGates_003
 * @tc.desc  : bypassSpatializationForStereo_ always caches the raw bypass flag, but downstream value
 *             passed to HpaePolicyManager is (bypass && adaptiveSpatialRenderingEnabled_). Toggling
 *             adaptiveSpatialRenderingEnabled_ changes only the downstream propagation, not the cached
 *             flag once it has been settled.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateBypassSpatializationForStereo_AdaptiveRenderingGates_003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    // adaptiveSpatialRenderingEnabled_ == false: cached flag still tracks raw bypass.
    hpaeManager_->bypassSpatializationForStereo_ = false;
    hpaeManager_->adaptiveSpatialRenderingEnabled_ = false;
    hpaeManager_->UpdateBypassSpatializationForStereo(true);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);

    // Enable adaptive rendering afterwards; cached flag must remain true without a new call.
    hpaeManager_->adaptiveSpatialRenderingEnabled_ = true;
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);

    // Disabling bypass again flips the cached flag regardless of adaptive state.
    hpaeManager_->UpdateBypassSpatializationForStereo(false);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, false);
}

/**
 * @tc.name  : HandleBypassSpatializationForStereo_AsyncDispatch_004
 * @tc.desc  : HandleBypassSpatializationForStereo posts UpdateBypassSpatializationForStereo onto the
 *             manager thread; after WaitForMsgProcessing the cached flag reflects the request.
 */
HWTEST_F(HpaeManagerUnitTest, HandleBypassSpatializationForStereo_AsyncDispatch_004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    hpaeManager_->bypassSpatializationForStereo_ = false;
    hpaeManager_->adaptiveSpatialRenderingEnabled_ = false;

    hpaeManager_->HandleBypassSpatializationForStereo(true);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);

    hpaeManager_->HandleBypassSpatializationForStereo(false);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, false);
}

/**
 * @tc.name  : UpdateSpatializationState_RefreshesAdaptiveThenBypass_005
 * @tc.desc  : UpdateSpatializationState copies adaptiveSpatialRenderingEnabled from the incoming
 *             state and re-runs UpdateBypassSpatializationForStereo with the cached bypass flag, so
 *             the cached flag survives while adaptive state tracks the request.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateSpatializationState_RefreshesAdaptiveThenBypass_005, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    hpaeManager_->bypassSpatializationForStereo_ = true;
    hpaeManager_->adaptiveSpatialRenderingEnabled_ = false;

    AudioSpatializationState stateInfo;
    stateInfo.spatializationEnabled = false;
    stateInfo.headTrackingEnabled = false;
    stateInfo.adaptiveSpatialRenderingEnabled = true;
    ASSERT_EQ(hpaeManager_->UpdateSpatializationState(stateInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->adaptiveSpatialRenderingEnabled_, true);
    // Cached bypass flag must survive the UpdateSpatializationState-triggered re-run.
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);

    // Now flip adaptive off via the same path.
    stateInfo.adaptiveSpatialRenderingEnabled = false;
    ASSERT_EQ(hpaeManager_->UpdateSpatializationState(stateInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->adaptiveSpatialRenderingEnabled_, false);
    EXPECT_EQ(hpaeManager_->bypassSpatializationForStereo_, true);
}

// ==================== IsChannelLayoutSupportedForDspEffect with DeviceType Tests ====================

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_001
 * @tc.number: IsChannelLayoutSupportedForDspEffectWithDeviceType_001
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for SPEAKER device.
 */
HWTEST_F(HpaeManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    bool ret = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_6POINT0_FRONT, DEVICE_TYPE_SPEAKER);
    // Without proper HDI setup, this should return false
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_002
 * @tc.number: IsChannelLayoutSupportedForDspEffectWithDeviceType_002
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for BLUETOOTH_A2DP device.
 */
HWTEST_F(HpaeManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    bool ret = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_STEREO, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_003
 * @tc.number: IsChannelLayoutSupportedForDspEffectWithDeviceType_003
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for WIRED_HEADSET device.
 */
HWTEST_F(HpaeManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_003, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    bool ret = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_MONO, DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_004
 * @tc.number: IsChannelLayoutSupportedForDspEffectWithDeviceType_004
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for NEARLINK device.
 */
HWTEST_F(HpaeManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_004, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    bool ret = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_2POINT1, DEVICE_TYPE_NEARLINK);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001
 * @tc.number: IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for multiple channel layouts.
 */
HWTEST_F(HpaeManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret1 = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_MONO, deviceType);
    bool ret2 = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_STEREO, deviceType);
    bool ret3 = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_5POINT1, deviceType);
    bool ret4 = hpaeManager_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_7POINT1, deviceType);

    EXPECT_EQ(ret1, false);
    EXPECT_EQ(ret2, true);
    EXPECT_EQ(ret3, true);
    EXPECT_EQ(ret4, true);
}

// ==================== UpdateCollaborativeState Enhanced Routing Tests ====================

/**
 * @tc.name  : UpdateCollaborativeState_NearlinkEnhancedRouting_001
 * @tc.number: UpdateCollaborativeState_NearlinkEnhancedRouting_001
 * @tc.desc  : Test UpdateCollaborativeState with NEARLINK when enhanced routing is supported.
 *            This tests the branch where IsEnhancedRoutingSupported() returns true,
 *            causing NEARLINK_SINK_NAME to be used.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_NearlinkEnhancedRouting_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    // Test NEARLINK_OPEN with enhanced routing (uses NEARLINK_SINK_NAME)
    CollaborationState isCollaborationEnabled = COLLABORATION_NEARLINK_OPEN;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    // Verify collaboration state was updated
    EXPECT_EQ(hpaeManager_->isCollaborationEnabled_, COLLABORATION_NEARLINK_OPEN);

    // Reset state
    isCollaborationEnabled = COLLABORATION_CLOSE;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : UpdateCollaborativeState_BluetoothEnhancedRouting_001
 * @tc.number: UpdateCollaborativeState_BluetoothEnhancedRouting_001
 * @tc.desc  : Test UpdateCollaborativeState with BLUETOOTH when enhanced routing is supported.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_BluetoothEnhancedRouting_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    // Test BLUETOOTH_OPEN (uses BT_SINK_NAME)
    CollaborationState isCollaborationEnabled = COLLABORATION_BLUETOOTH_OPEN;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    // Verify collaboration state was updated
    EXPECT_EQ(hpaeManager_->isCollaborationEnabled_, COLLABORATION_BLUETOOTH_OPEN);
    EXPECT_EQ(hpaeManager_->collaborationRouteMode_, COLLABORATION_DEFAULT_ROUTE_MODE);

    // Reset state
    isCollaborationEnabled = COLLABORATION_CLOSE;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : UpdateCollaborativeState_SwitchBetweenModes_001
 * @tc.number: UpdateCollaborativeState_SwitchBetweenModes_001
 * @tc.desc  : Test switching between different collaboration states.
 */
HWTEST_F(HpaeManagerUnitTest, UpdateCollaborativeState_SwitchBetweenModes_001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);

    // Start with BLUETOOTH_OPEN
    CollaborationState isCollaborationEnabled = COLLABORATION_BLUETOOTH_OPEN;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->isCollaborationEnabled_, COLLABORATION_BLUETOOTH_OPEN);

    // Switch to NEARLINK_OPEN
    isCollaborationEnabled = COLLABORATION_NEARLINK_OPEN;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->isCollaborationEnabled_, COLLABORATION_NEARLINK_OPEN);

    // Switch back to CLOSE
    isCollaborationEnabled = COLLABORATION_CLOSE;
    ASSERT_EQ(hpaeManager_->UpdateCollaborativeState(isCollaborationEnabled), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->isCollaborationEnabled_, COLLABORATION_CLOSE);
}

/**
 * @tc.name  : SetStreamSilentMode_UnknownSession_002
 * @tc.type  : FUNC
 * @tc.number: SetStreamSilentMode_UnknownSession_002
 * @tc.desc  : SetStreamSilentMode on unknown session is a safe no-op.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(HpaeManagerUnitTest, SetStreamSilentMode_UnknownSession_002, TestSize.Level1)
{
    PrepareCoverageManager(hpaeManager_);
    hpaeManager_->SetStreamSilentMode(HPAE_STREAM_CLASS_TYPE_PLAY, 999, true);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : SetStreamSilentMode_NonPlayType_003
 * @tc.type  : FUNC
 * @tc.number: SetStreamSilentMode_NonPlayType_003
 * @tc.desc  : SetStreamSilentMode with non-PLAY stream class type is a safe no-op.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(HpaeManagerUnitTest, SetStreamSilentMode_NonPlayType_003, TestSize.Level1)
{
    PrepareCoverageManager(hpaeManager_);
    hpaeManager_->SetStreamSilentMode(HPAE_STREAM_CLASS_TYPE_RECORD, 1, true);
    WaitForMsgProcessing(hpaeManager_);
}

/**
 * @tc.name  : EmptyDeviceNameFallsToCoreSink
 * @tc.desc  : A PLAY stream with empty deviceName lands on coreSink_, the first real sink opened.
 */
HWTEST_F(HpaeManagerUnitTest, EmptyDeviceNameFallsToCoreSink, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback =
        std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();  // deviceName empty -> coreSink_
    EXPECT_EQ(hpaeManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    std::vector<SinkInput> sinkInputs = callback->GetSinkInputs();
    EXPECT_EQ(sinkInputs.size(), 1);  // fresh manager per test: this stream is the only sink input
    EXPECT_EQ(sinkInputs[0].sinkName == audioModuleInfo.name, true);  // coreSink_ is the first real sink
    EXPECT_EQ(sinkInputs[0].paStreamId, streamInfo.sessionId);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

/**
 * @tc.name  : EmptyDeviceNameWithoutRealSinkFails
 * @tc.desc  : With no sink ever opened, an empty deviceName stream is skipped, not created.
 */
HWTEST_F(HpaeManagerUnitTest, EmptyDeviceNameWithoutRealSinkFails, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();  // deviceName empty, no sink ever opened
    EXPECT_EQ(hpaeManager_->CreateStream(streamInfo), SUCCESS);  // async, lambda logs and skips
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback =
        std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSinkInputs().empty(), true);  // coreSink_ empty: creation skipped
    hpaeManager_->DeInit();
}

/**
 * @tc.name  : NonCoreSinkCloseFallsBackToVirtualRender
 * @tc.desc  : Closing a non-core sink moves its streams to the Virtual_Render fallback sink.
 */
HWTEST_F(HpaeManagerUnitTest, NonCoreSinkCloseFallsBackToVirtualRender, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback =
        std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    AudioModuleInfo coreSink = GetSinkAudioModeInfo();  // first open -> coreSink_ + core manager
    EXPECT_EQ(hpaeManager_->OpenAudioPort(coreSink), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    AudioModuleInfo secondSink = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(secondSink), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    streamInfo.deviceName = secondSink.name;
    EXPECT_EQ(hpaeManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);  // RUNNING path moves back
    WaitForMsgProcessing(hpaeManager_);
    // close the non-core sink: its streams fall back to Virtual_Render
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort(secondSink.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    std::vector<SinkInput> sinkInputs = callback->GetSinkInputs();
    EXPECT_EQ(sinkInputs.size(), 1);  // stream survives the close
    EXPECT_EQ(sinkInputs[0].sinkName == "Virtual_Render", true);
    EXPECT_EQ(hpaeManager_->idPreferSinkNameMap_[streamInfo.sessionId] == secondSink.name, true);
    // re-open the closed sink: the RUNNING stream migrates back from Virtual_Render
    EXPECT_EQ(hpaeManager_->OpenAudioPort(secondSink), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    sinkInputs = callback->GetSinkInputs();
    EXPECT_EQ(sinkInputs.size(), 1);
    EXPECT_EQ(sinkInputs[0].sinkName == secondSink.name, true);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

/**
 * @tc.name  : StopAudioPortOnVirtualSinkIsSkipped
 * @tc.desc  : StopAudioPort on Virtual_Render returns SUCCESS and sinks keep serving streams.
 */
HWTEST_F(HpaeManagerUnitTest, StopAudioPortOnVirtualSinkIsSkipped, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);  // first real sink -> coreSink_
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->StopAudioPort("Virtual_Render"), SUCCESS);  // guard hit, skip stop
    WaitForMsgProcessing(hpaeManager_);
    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback =
        std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    EXPECT_EQ(hpaeManager_->RegisterSerivceCallback(callback), SUCCESS);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();  // empty deviceName -> coreSink_
    EXPECT_EQ(hpaeManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetAllSinkInputs(), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSinkInputs().empty(), false);  // core sink still serves streams
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

/**
 * @tc.name  : CloseOutAudioPortOnVirtualSinkIsSkipped
 * @tc.desc  : CloseOutAudioPort on Virtual_Render returns SUCCESS and keeps the entry.
 */
HWTEST_F(HpaeManagerUnitTest, CloseOutAudioPortOnVirtualSinkIsSkipped, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);  // lazy-creates Virtual_Render
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Virtual_Render"), SUCCESS);  // guard hit, skip close
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->sinkNameSinkIdMap_.count("Virtual_Render"), 1);  // entry survives
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();  // empty deviceName -> coreSink_
    EXPECT_EQ(hpaeManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
}

}  // namespace
