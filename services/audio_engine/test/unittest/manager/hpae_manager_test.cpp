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

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
static std::string g_rootPath = "/data/";
const std::string ROOT_PATH = "/data/source_file_io_48000_2_s16le.pcm";
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
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
    const int waitCountThd = 5;
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

HWTEST_F(HpaeManagerUnitTest, constructHpaeManagerTest, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);
    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    sleep(1);
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    int32_t portId = callback->GetPortId();
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
    for (const auto &it : sourceOutputs) {
        std::cout << "deviceSourceId:" << it.deviceSourceId << std::endl;
        EXPECT_EQ(it.paStreamId, streamInfo.sessionId);
        EXPECT_EQ(it.deviceSourceId, portId);
    }

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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    HpaeStreamInfo streamInfo2 = GetRenderStreamInfo();
    streamInfo2.sessionId = TEST_STREAM_SESSION_ID + 1;
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
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
    
    HpaeDeviceInfo devicesInfo_;
    hpaeManager_->DumpAllAvailableDevice(devicesInfo_);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(devicesInfo_.sinkInfos.size() > 0, true);
    EXPECT_EQ(devicesInfo_.sourceInfos.size() > 0, true);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerSetDefaultSink001, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->SetDefaultSink(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSink("virtual1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSink(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->SetDefaultSink("Speaker_File1"), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File2"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File1"), SUCCESS);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File"), SUCCESS);
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
}

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerSetDefaultSink002, TestSize.Level1)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->defaultSink_ = "virtual1";
    EXPECT_EQ(hpaeManager_->SetDefaultSink(audioModuleInfo.name), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->CloseOutAudioPort("Speaker_File"), SUCCESS);
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo1), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
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
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
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
    
    AudioEffectPropertyArrayV3 propertyV3;
    EXPECT_EQ(hpaeManager_->GetAudioEffectProperty(propertyV3), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetAudioEffectProperty(propertyV3), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    AudioEffectPropertyArray property;
    EXPECT_EQ(hpaeManager_->GetAudioEffectProperty(property), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->UpdateEffectBtOffloadSupported(true);
    EXPECT_EQ(hpaeManager_->SetOutputDevice(TEST_STREAM_SESSION_ID, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetMicrophoneMuteInfo(false), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->GetAudioEnhanceProperty(propertyV3, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetAudioEnhanceProperty(propertyV3, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    
    AudioEnhancePropertyArray propertyEn;
    EXPECT_EQ(hpaeManager_->GetAudioEnhanceProperty(propertyEn, DEVICE_TYPE_SPEAKER), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);

    EXPECT_EQ(hpaeManager_->SetAudioEnhanceProperty(propertyEn, DEVICE_TYPE_SPEAKER), SUCCESS);
    hpaeManager_->UpdateExtraSceneType("123", "456", "789");
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

HWTEST_F(HpaeManagerUnitTest, IHpaeManagerAddPreferSink001, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->rendererIdSinkNameMap_.emplace(TEST_STREAM_SESSION_ID, "speaker_file");
    hpaeManager_->AddPreferSinkForDefaultChange(false, "speaker_file");
    EXPECT_EQ(hpaeManager_->idPreferSinkNameMap_.size() == 0, true);
    hpaeManager_->AddPreferSinkForDefaultChange(true, "speaker_file");
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
    EXPECT_EQ(hpaeManager_->ReloadAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
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
    int32_t ret = hpaeManager_->UpdateCollaborativeState(true);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    ret = hpaeManager_->UpdateCollaborativeState(false);
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
    HpaeDeviceInfo deviceInfo;
    hpaeManager_->DumpAllAvailableDevice(deviceInfo);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(deviceInfo.sinkInfos.size() > 0, true);
    EXPECT_EQ(deviceInfo.sourceInfos.size() > 0, true);
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
}  // namespace