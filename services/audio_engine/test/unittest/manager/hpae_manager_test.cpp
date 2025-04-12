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
#include "hpae_manager_unit_test.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
namespace {
static std::string g_rootPath = "/data/data/.pulse_dir/";
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;

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
    const int WAIT_COUNT_THD = 5;
    while (hpaeManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= WAIT_COUNT_THD) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    EXPECT_EQ(hpaeManager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < WAIT_COUNT_THD, true);
}

AudioModuleInfo GetSinkAudioModeInfo()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = "Speaker_File";
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

AudioModuleInfo GetSourceAudioModeInfo()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = "mic";
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
    return streamInfo;
}

TEST_F(HpaeManagerUnitTest, constructHpaeManagerTest)
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

TEST_F(HpaeManagerUnitTest, GetHpaeRenderManagerTest)
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

TEST_F(HpaeManagerUnitTest, IHpaeRenderManagerTest)
{
    IHpaeManager::GetHpaeManager()->Init();
    EXPECT_EQ(IHpaeManager::GetHpaeManager()->IsInit(), true);
    sleep(1);
    EXPECT_EQ(IHpaeManager::GetHpaeManager()->IsRunning(), true);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    EXPECT_EQ(IHpaeManager::GetHpaeManager()->OpenAudioPort(audioModuleInfo), SUCCESS);
    IHpaeManager::GetHpaeManager()->DeInit();
    EXPECT_EQ(IHpaeManager::GetHpaeManager()->IsInit(), false);
    EXPECT_EQ(IHpaeManager::GetHpaeManager()->IsRunning(), false);
}

TEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest)
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

TEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest)
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

    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(portId, true), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetSetSourceOutputMuteResult(), SUCCESS);
    EXPECT_EQ(hpaeManager_->SetSourceOutputMute(portId, false), SUCCESS);
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

TEST_F(HpaeManagerUnitTest, IHpaeRenderStreamManagerTest002)
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
    EXPECT_EQ(sessionInfo.state, I_STATUS_IDLE);

    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, I_STATUS_STARTING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);

    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, I_STATUS_PAUSING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);

    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, I_STATUS_STOPPING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}

TEST_F(HpaeManagerUnitTest, IHpaeCaptureStreamManagerTest002)
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
    EXPECT_EQ(sessionInfo.state, I_STATUS_IDLE);
    hpaeManager_->Start(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    EXPECT_EQ(sessionInfo.state, I_STATUS_STARTING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STARTED);
    hpaeManager_->Pause(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, I_STATUS_PAUSING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_PAUSED);
    hpaeManager_->Stop(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), SUCCESS);
    EXPECT_EQ(sessionInfo.state, I_STATUS_STOPPING);
    EXPECT_EQ(statusChangeCb->GetStatus(), I_STATUS_STOPPED);
    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo), ERROR);
}
}  // namespace