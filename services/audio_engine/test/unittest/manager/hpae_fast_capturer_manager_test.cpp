/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "audio_errors.h"
#include "audio_stream_enum.h"
#include "hpae_fast_capturer_manager.h"
#include "hpae_mocks.h"
#include "hpae_node_common.h"
#include "test_case_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t FAST_FRAME_LENGTH = 960;
constexpr uint32_t FAST_SESSION_ID = 10001;
constexpr uint32_t FAST_SESSION_ID_SECOND = 10002;
constexpr uint32_t FAST_CAPTURE_ID = 19;
constexpr uint32_t FAST_APP_UID = 20001;
constexpr uint32_t FAST_APP_UID_SECOND = 20002;

HpaeSourceInfo CreateFastSourceInfo(uint32_t routeFlag = 0)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.adapterName = "primary";
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = "file_io";
    sourceInfo.deviceName = "Built_in_mic";
    sourceInfo.sourceName = "Built_in_mic";
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.deviceType = DEVICE_TYPE_MIC;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.channelLayout = CH_LAYOUT_STEREO;
    sourceInfo.frameLen = FAST_FRAME_LENGTH;
    sourceInfo.volume = 1.0f;
    sourceInfo.routeFlag = routeFlag;
    return sourceInfo;
}

HpaeStreamInfo CreateFastStreamInfo(uint32_t sessionId = FAST_SESSION_ID, uint32_t uid = FAST_APP_UID)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = sessionId;
    streamInfo.uid = uid;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.frameLen = FAST_FRAME_LENGTH;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_MIC;
    streamInfo.deviceName = "Built_in_mic";
    streamInfo.isMoveAble = true;
    return streamInfo;
}

HpaeNodeInfo CreateSourceNodeInfo(const HpaeSourceInfo &sourceInfo)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.deviceClass = sourceInfo.deviceClass;
    nodeInfo.deviceNetId = sourceInfo.deviceNetId;
    nodeInfo.deviceName = sourceInfo.sourceName;
    nodeInfo.channels = sourceInfo.channels;
    nodeInfo.channelLayout = static_cast<AudioChannelLayout>(sourceInfo.channelLayout);
    nodeInfo.format = sourceInfo.format;
    nodeInfo.frameLen = sourceInfo.frameLen;
    nodeInfo.samplingRate = sourceInfo.samplingRate;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceType = sourceInfo.sourceType;
    nodeInfo.routeFlag = sourceInfo.routeFlag;
    return nodeInfo;
}

std::shared_ptr<HpaeFastSourceInputNode> CreateFastSourceInputNode(const HpaeSourceInfo &sourceInfo,
    const std::shared_ptr<NiceMock<MockAudioCaptureSource>> &mockSource = nullptr)
{
    HpaeNodeInfo nodeInfo = CreateSourceNodeInfo(sourceInfo);
    auto sourceInputNode = std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
    if (mockSource != nullptr) {
        sourceInputNode->audioCapturerSource_ = mockSource;
        sourceInputNode->captureId_ = FAST_CAPTURE_ID;
    }
    return sourceInputNode;
}

std::shared_ptr<HpaeFastCapturerManager> CreateInitializedManager(
    const std::shared_ptr<NiceMock<MockAudioCaptureSource>> &mockSource)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    manager->sourceInputNode_ = CreateFastSourceInputNode(sourceInfo, mockSource);
    manager->isInit_.store(true);
    return manager;
}

std::shared_ptr<HpaeFastCapturerManager> CreatePreparedManager(
    const std::shared_ptr<NiceMock<MockAudioCaptureSource>> &mockSource,
    HpaeStreamInfo streamInfo = CreateFastStreamInfo())
{
    auto manager = CreateInitializedManager(mockSource);
    EXPECT_EQ(manager->CreateOutputSession(streamInfo), SUCCESS);
    manager->sourceOutputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PREPARED);
    manager->sessionNodeMap_[streamInfo.sessionId].state = HPAE_SESSION_PREPARED;
    return manager;
}
} // namespace

class HpaeFastCapturerManagerTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : ConstructAndSimpleApis
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_001
 * @tc.desc  : Test constructor and APIs that do not require a real HDI source.
 */
HWTEST_F(HpaeFastCapturerManagerTest, ConstructAndSimpleApis, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    ASSERT_NE(manager, nullptr);

    HpaeSourceInfo retInfo = manager->GetSourceInfo();
    EXPECT_EQ(retInfo.sourceName, sourceInfo.sourceName);
    EXPECT_EQ(retInfo.deviceClass, sourceInfo.deviceClass);
    EXPECT_EQ(retInfo.frameLen, sourceInfo.frameLen);
    EXPECT_EQ(manager->IsInit(), false);
    EXPECT_EQ(manager->IsRunning(), false);
    EXPECT_EQ(manager->IsMsgProcessing(), false);
    EXPECT_EQ(manager->GetThreadName(), sourceInfo.deviceName);
    EXPECT_EQ(manager->GetAllSourceOutputsInfo().empty(), true);

    uint64_t latency = 100;
    manager->OnRequestLatency(FAST_SESSION_ID, latency);
    EXPECT_EQ(latency, 0);
    manager->OnNotifyQueue();
    EXPECT_EQ(manager->DeactivateThread(), true);
    EXPECT_EQ(manager->StopManager(), SUCCESS);
    EXPECT_EQ(manager->DumpSourceInfo(), ERR_ILLEGAL_STATE);
    EXPECT_NE(manager->GetDeviceHDFDumpInfo().empty(), true);
    EXPECT_EQ(manager->AddCaptureInjector(nullptr, SOURCE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager->RemoveCaptureInjector(nullptr, SOURCE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager->SetAppsEnhanceMuteState(FAST_SESSION_ID, true), SUCCESS);
    EXPECT_EQ(manager->CapturerSourceStart(), ERR_ILLEGAL_STATE);
    EXPECT_EQ(manager->CapturerSourceStop(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : SendRequestAndThreadBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_012
 * @tc.desc  : Test SendRequest init guard and signal-thread null/non-null branches.
 */
HWTEST_F(HpaeFastCapturerManagerTest, SendRequestAndThreadBranches, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    bool requestHandled = false;
    manager->SendRequest([&requestHandled] { requestHandled = true; }, "test");
    manager->HandleMsg();
    EXPECT_EQ(requestHandled, false);

    manager->SendRequest([&requestHandled] { requestHandled = true; }, "test", true);
    manager->HandleMsg();
    EXPECT_EQ(requestHandled, true);

    requestHandled = false;
    manager->isInit_.store(true);
    manager->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    manager->SendRequest([&requestHandled] { requestHandled = true; }, "test");
    manager->HandleMsg();
    EXPECT_EQ(requestHandled, true);
    EXPECT_EQ(manager->DeactivateThread(), true);
}

/**
 * @tc.name  : CreateSourceAttrFlagBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_002
 * @tc.desc  : Test attr creation for mmap, voip fast and fast+voip route flags.
 */
HWTEST_F(HpaeFastCapturerManagerTest, CreateSourceAttrFlagBranches, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    IAudioSourceAttr attr;
    manager->CreateSourceAttr(attr);
    EXPECT_EQ(attr.adapterName, sourceInfo.adapterName);
    EXPECT_EQ(attr.sampleRate, sourceInfo.samplingRate);
    EXPECT_EQ(attr.channel, sourceInfo.channels);
    EXPECT_EQ(attr.format, sourceInfo.format);
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_MMAP);

    sourceInfo = CreateFastSourceInfo(AUDIO_INPUT_FLAG_VOIP_FAST);
    manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    manager->CreateSourceAttr(attr);
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_VOIP_FAST);

    sourceInfo = CreateFastSourceInfo(AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP);
    manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    manager->CreateSourceAttr(attr);
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_VOIP_FAST);

    sourceInfo = CreateFastSourceInfo(AUDIO_INPUT_FLAG_FAST);
    manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    manager->CreateSourceAttr(attr);
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_MMAP);

    sourceInfo = CreateFastSourceInfo(AUDIO_INPUT_FLAG_VOIP);
    manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    manager->CreateSourceAttr(attr);
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_MMAP);
}

/**
 * @tc.name  : InitCapturerManagerInvalidFrameLen
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_003
 * @tc.desc  : Test init manager exits before HDI access on invalid source frame length.
 */
HWTEST_F(HpaeFastCapturerManagerTest, InitCapturerManagerInvalidFrameLen, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    sourceInfo.frameLen = 0;
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    EXPECT_NE(manager->InitCapturerManager(), SUCCESS);

    sourceInfo = CreateFastSourceInfo();
    sourceInfo.frameLen = 50000;
    manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    EXPECT_NE(manager->InitCapturerManager(), SUCCESS);
}

/**
 * @tc.name  : CreateStreamAndQuery
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_004
 * @tc.desc  : Test create stream guards, queued creation and source output query.
 */
HWTEST_F(HpaeFastCapturerManagerTest, CreateStreamAndQuery, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    HpaeStreamInfo streamInfo = CreateFastStreamInfo();
    EXPECT_EQ(manager->CreateStream(streamInfo), ERR_INVALID_OPERATION);

    manager->isInit_.store(true);
    HpaeStreamInfo invalidStreamInfo;
    EXPECT_NE(manager->CreateStream(invalidStreamInfo), SUCCESS);

    manager->sourceInputNode_ = CreateFastSourceInputNode(sourceInfo);
    EXPECT_EQ(manager->CreateStream(streamInfo), SUCCESS);
    EXPECT_EQ(manager->IsMsgProcessing(), true);
    manager->HandleMsg();
    EXPECT_EQ(manager->IsMsgProcessing(), false);
    ASSERT_EQ(manager->sourceOutputNodeMap_.size(), 1);
    EXPECT_EQ(manager->sessionNodeMap_[streamInfo.sessionId].state, HPAE_SESSION_PREPARED);

    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(manager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo), SUCCESS);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.sessionId, streamInfo.sessionId);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.channels, streamInfo.channels);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(manager->GetSourceOutputInfo(FAST_SESSION_ID_SECOND, sourceOutputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(manager->RegisterReadCallback(streamInfo.sessionId, std::weak_ptr<ICapturerStreamCallback>()), SUCCESS);
    manager->HandleMsg();

    manager->sourceInputNode_ = nullptr;
    EXPECT_EQ(manager->Flush(streamInfo.sessionId), SUCCESS);
    manager->HandleMsg();
}

/**
 * @tc.name  : StateControlAndMute
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_005
 * @tc.desc  : Test start, flush, drain, mute, pause, stop and destroy queued paths.
 */
HWTEST_F(HpaeFastCapturerManagerTest, StateControlAndMute, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreatePreparedManager(mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Start()).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockSource, Stop()).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockSource, Flush()).WillByDefault(Return(SUCCESS));

    EXPECT_EQ(manager->Start(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    ASSERT_NE(manager->sourceOutputNodeMap_[FAST_SESSION_ID], nullptr);
    EXPECT_EQ(manager->sourceOutputNodeMap_[FAST_SESSION_ID]->GetState(), HPAE_SESSION_RUNNING);
    EXPECT_EQ(manager->sessionNodeMap_[FAST_SESSION_ID].state, HPAE_SESSION_RUNNING);

    EXPECT_EQ(manager->Flush(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->Drain(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();

    EXPECT_EQ(manager->SetStreamMute(FAST_SESSION_ID, true), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_[FAST_SESSION_ID]->GetMute(), true);

    EXPECT_EQ(manager->Pause(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_[FAST_SESSION_ID]->GetState(), HPAE_SESSION_PAUSED);
    EXPECT_EQ(manager->sessionNodeMap_[FAST_SESSION_ID].state, HPAE_SESSION_PAUSED);

    EXPECT_EQ(manager->Stop(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_[FAST_SESSION_ID]->GetState(), HPAE_SESSION_STOPPED);
    EXPECT_EQ(manager->sessionNodeMap_[FAST_SESSION_ID].state, HPAE_SESSION_STOPPED);

    EXPECT_EQ(manager->Release(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(FAST_SESSION_ID), 0);
    EXPECT_EQ(manager->sessionNodeMap_.count(FAST_SESSION_ID), 0);

    manager = CreatePreparedManager(mockSource);
    manager->sourceOutputNodeMap_[FAST_SESSION_ID]->SetState(HPAE_SESSION_RUNNING);
    manager->sessionNodeMap_[FAST_SESSION_ID].state = HPAE_SESSION_RUNNING;
    EXPECT_EQ(manager->DestroyStream(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(FAST_SESSION_ID), 0);
}

/**
 * @tc.name  : MissingSessionOperations
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_006
 * @tc.desc  : Test queued operations when the target session is absent.
 */
HWTEST_F(HpaeFastCapturerManagerTest, MissingSessionOperations, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreateInitializedManager(mockSource);

    EXPECT_EQ(manager->Start(FAST_SESSION_ID), SUCCESS);
    EXPECT_EQ(manager->Pause(FAST_SESSION_ID), SUCCESS);
    EXPECT_EQ(manager->Flush(FAST_SESSION_ID), SUCCESS);
    EXPECT_EQ(manager->Drain(FAST_SESSION_ID), SUCCESS);
    EXPECT_EQ(manager->Stop(FAST_SESSION_ID), SUCCESS);
    EXPECT_EQ(manager->SetStreamMute(FAST_SESSION_ID, true), SUCCESS);
    EXPECT_EQ(manager->RegisterReadCallback(FAST_SESSION_ID, std::weak_ptr<ICapturerStreamCallback>()), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.empty(), true);

    EXPECT_EQ(manager->DestroyStream(FAST_SESSION_ID), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.empty(), true);
    EXPECT_EQ(manager->DeleteOutputSession(FAST_SESSION_ID), SUCCESS);
    EXPECT_NE(manager->ConnectOutputSession(FAST_SESSION_ID), SUCCESS);
    EXPECT_NE(manager->DisConnectOutputSession(FAST_SESSION_ID), SUCCESS);

    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto notInitManager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    EXPECT_EQ(notInitManager->DestroyStream(FAST_SESSION_ID), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : ConnectDisconnectConditionBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_014
 * @tc.desc  : Test Connect/DisConnect compound-condition success and individual failure paths.
 */
HWTEST_F(HpaeFastCapturerManagerTest, ConnectDisconnectConditionBranches, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreateInitializedManager(mockSource);
    HpaeStreamInfo streamInfo = CreateFastStreamInfo();
    EXPECT_EQ(manager->CreateOutputSession(streamInfo), SUCCESS);

    EXPECT_EQ(manager->ConnectOutputSession(streamInfo.sessionId), SUCCESS);
    EXPECT_EQ(manager->DisConnectOutputSession(streamInfo.sessionId), SUCCESS);

    manager->converterNodeMap_.erase(streamInfo.sessionId);
    EXPECT_NE(manager->ConnectOutputSession(streamInfo.sessionId), SUCCESS);
    EXPECT_NE(manager->DisConnectOutputSession(streamInfo.sessionId), SUCCESS);

    HpaeStreamInfo secondStreamInfo = CreateFastStreamInfo(FAST_SESSION_ID_SECOND, FAST_APP_UID_SECOND);
    EXPECT_EQ(manager->CreateOutputSession(secondStreamInfo), SUCCESS);
    manager->sourceInputNode_ = nullptr;
    EXPECT_NE(manager->ConnectOutputSession(secondStreamInfo.sessionId), SUCCESS);
    EXPECT_NE(manager->DisConnectOutputSession(secondStreamInfo.sessionId), SUCCESS);
}

/**
 * @tc.name  : MoveStreamCallbacks
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_007
 * @tc.desc  : Test move stream failure, empty-name guard and successful move callback.
 */
HWTEST_F(HpaeFastCapturerManagerTest, MoveStreamCallbacks, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreateInitializedManager(mockSource);
    auto callback = std::make_shared<NiceMock<MockSendMsgCallback>>();
    manager->RegisterSendMsgCallback(callback);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Stop()).WillByDefault(Return(SUCCESS));

    EXPECT_CALL(*callback, Invoke(MOVE_SESSION_FAILED, _)).Times(1);
    EXPECT_EQ(manager->MoveStream(FAST_SESSION_ID, "new_source"), SUCCESS);
    manager->HandleMsg();

    HpaeStreamInfo streamInfo = CreateFastStreamInfo();
    EXPECT_EQ(manager->CreateOutputSession(streamInfo), SUCCESS);
    manager->sourceOutputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PREPARED);
    manager->sessionNodeMap_[streamInfo.sessionId].state = HPAE_SESSION_PREPARED;
    EXPECT_EQ(manager->MoveStream(streamInfo.sessionId, ""), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(streamInfo.sessionId), 1);

    EXPECT_CALL(*callback, Invoke(MOVE_SOURCE_OUTPUT, _)).Times(1);
    EXPECT_EQ(manager->MoveStream(streamInfo.sessionId, "new_source"), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(streamInfo.sessionId), 0);
}

/**
 * @tc.name  : MoveAllStreamCallbacks
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_008
 * @tc.desc  : Test move-all sync callback and selected-session async callback.
 */
HWTEST_F(HpaeFastCapturerManagerTest, MoveAllStreamCallbacks, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    auto manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    auto callback = std::make_shared<NiceMock<MockSendMsgCallback>>();
    manager->RegisterSendMsgCallback(callback);

    EXPECT_CALL(*callback, InvokeSync(MOVE_ALL_SOURCE_OUTPUT, _)).Times(1);
    EXPECT_EQ(manager->MoveAllStream("default", {}, MOVE_ALL), SUCCESS);

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    manager = CreatePreparedManager(mockSource, CreateFastStreamInfo());
    manager->RegisterSendMsgCallback(callback);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Stop()).WillByDefault(Return(SUCCESS));
    EXPECT_CALL(*callback, Invoke(MOVE_ALL_SOURCE_OUTPUT, _)).Times(1);
    EXPECT_EQ(manager->MoveAllStream("selected", {FAST_SESSION_ID}, MOVE_PREFER), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(FAST_SESSION_ID), 0);
}

/**
 * @tc.name  : AddNodesToSource
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_009
 * @tc.desc  : Test add-node null guard, disconnected add and connected running add.
 */
HWTEST_F(HpaeFastCapturerManagerTest, AddNodesToSource, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreateInitializedManager(mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Start()).WillByDefault(Return(SUCCESS));

    HpaeCaptureMoveInfo nullMoveInfo;
    nullMoveInfo.sessionId = FAST_SESSION_ID;
    EXPECT_EQ(manager->AddNodeToSource(nullMoveInfo), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.empty(), true);

    HpaeStreamInfo streamInfo = CreateFastStreamInfo();
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    auto stoppedOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    stoppedOutputNode->SetAppUid(FAST_APP_UID);
    stoppedOutputNode->SetState(HPAE_SESSION_STOPPED);
    HpaeCaptureMoveInfo stoppedMoveInfo;
    stoppedMoveInfo.sessionId = streamInfo.sessionId;
    stoppedMoveInfo.sourceOutputNode = stoppedOutputNode;
    stoppedMoveInfo.sessionInfo.state = HPAE_SESSION_STOPPED;
    EXPECT_EQ(manager->AddAllNodesToSource({stoppedMoveInfo}, false), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(streamInfo.sessionId), 1);
    EXPECT_EQ(manager->converterNodeMap_.count(streamInfo.sessionId), 1);

    HpaeStreamInfo preparedStreamInfo = CreateFastStreamInfo(FAST_SESSION_ID + 10, FAST_APP_UID);
    HpaeNodeInfo preparedNodeInfo;
    ConfigNodeInfo(preparedNodeInfo, preparedStreamInfo);
    auto preparedOutputNode = std::make_shared<HpaeSourceOutputNode>(preparedNodeInfo);
    preparedOutputNode->SetState(HPAE_SESSION_PREPARED);
    HpaeCaptureMoveInfo preparedMoveInfo;
    preparedMoveInfo.sessionId = preparedStreamInfo.sessionId;
    preparedMoveInfo.sourceOutputNode = preparedOutputNode;
    preparedMoveInfo.sessionInfo.state = HPAE_SESSION_PREPARED;
    EXPECT_EQ(manager->AddAllNodesToSource({preparedMoveInfo}, true), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(preparedStreamInfo.sessionId), 1);

    HpaeStreamInfo runningStreamInfo = CreateFastStreamInfo(FAST_SESSION_ID_SECOND, FAST_APP_UID_SECOND);
    HpaeNodeInfo runningNodeInfo;
    ConfigNodeInfo(runningNodeInfo, runningStreamInfo);
    auto runningOutputNode = std::make_shared<HpaeSourceOutputNode>(runningNodeInfo);
    runningOutputNode->SetAppUid(FAST_APP_UID_SECOND);
    runningOutputNode->SetState(HPAE_SESSION_RUNNING);
    HpaeCaptureMoveInfo runningMoveInfo;
    runningMoveInfo.sessionId = runningStreamInfo.sessionId;
    runningMoveInfo.sourceOutputNode = runningOutputNode;
    runningMoveInfo.sessionInfo.state = HPAE_SESSION_RUNNING;
    EXPECT_EQ(manager->AddAllNodesToSource({runningMoveInfo}, true), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceOutputNodeMap_.count(runningStreamInfo.sessionId), 1);
    EXPECT_EQ(manager->converterNodeMap_.count(runningStreamInfo.sessionId), 1);
    EXPECT_EQ(manager->sourceInputNode_->GetSourceState(), STREAM_MANAGER_RUNNING);

    manager->AddSingleNodeToSource(runningMoveInfo, true);
    EXPECT_EQ(manager->converterNodeMap_.count(runningStreamInfo.sessionId), 1);
}

/**
 * @tc.name  : ProcessAndAppsUid
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_010
 * @tc.desc  : Test apps uid collection, process early return and stop manager path.
 */
HWTEST_F(HpaeFastCapturerManagerTest, ProcessAndAppsUid, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreatePreparedManager(mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Stop()).WillByDefault(Return(SUCCESS));

    manager->sourceOutputNodeMap_[FAST_SESSION_ID]->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(manager->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(manager->CapturerSourceStart(), SUCCESS);
    manager->TriggerAppsUidUpdate(FAST_SESSION_ID);
    manager->HandleMsg();
    EXPECT_EQ(manager->appsUid_.size(), 1);
    EXPECT_EQ(manager->sessionsId_.size(), 1);

    manager->sourceOutputNodeMap_[FAST_SESSION_ID]->SetState(HPAE_SESSION_STOPPED);
    manager->TriggerAppsUidUpdate(FAST_SESSION_ID);
    manager->HandleMsg();
    EXPECT_EQ(manager->appsUid_.size(), 1);
    EXPECT_EQ(manager->sessionsId_.size(), 1);

    manager->sourceInputNode_->SetSourceState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(manager->CapturerSourceStop(), SUCCESS);
    manager->TriggerAppsUidUpdate(0);
    manager->HandleMsg();
    EXPECT_EQ(manager->appsUid_.empty(), true);
    EXPECT_EQ(manager->sessionsId_.empty(), true);

    EXPECT_EQ(manager->SetMute(true), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->isMute_.load(), true);
    EXPECT_EQ(manager->SetMute(true), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->isMute_.load(), true);

    manager->sourceOutputNodeMap_[FAST_SESSION_ID]->SetState(HPAE_SESSION_RUNNING);
    manager->UpdateAppsUidAndSessionId();
    EXPECT_EQ(manager->appsUid_.size(), 1);
    manager->sourceInputNode_ = nullptr;
    manager->UpdateAppsUidAndSessionId();
    EXPECT_EQ(manager->appsUid_.size(), 1);

    manager->Process();
    EXPECT_EQ(manager->StopManager(), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceInputNode_, nullptr);
}

/**
 * @tc.name  : NotifyAndCheckRunningBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_013
 * @tc.desc  : Test notify source-map branches and CheckIfAnyStreamRunning empty/non-running/running paths.
 */
HWTEST_F(HpaeFastCapturerManagerTest, NotifyAndCheckRunningBranches, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreateInitializedManager(mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Start()).WillByDefault(Return(SUCCESS));

    manager->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, FAST_SESSION_ID, CAPTURER_RUNNING);
    manager->CheckIfAnyStreamRunning();

    HpaeStreamInfo streamInfo = CreateFastStreamInfo();
    EXPECT_EQ(manager->CreateOutputSession(streamInfo), SUCCESS);
    manager->sessionNodeMap_[streamInfo.sessionId].state = HPAE_SESSION_PREPARED;
    manager->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, CAPTURER_PREPARED);
    manager->CheckIfAnyStreamRunning();

    manager->sessionNodeMap_[streamInfo.sessionId].state = HPAE_SESSION_RUNNING;
    manager->CheckIfAnyStreamRunning();
    EXPECT_EQ(manager->sourceInputNode_->GetSourceState(), STREAM_MANAGER_RUNNING);

    manager->sourceInputNode_ = nullptr;
    manager->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, CAPTURER_RUNNING);
    EXPECT_EQ(manager->sourceInputNode_, nullptr);
}

/**
 * @tc.name  : DeInitAndReloadFailure
 * @tc.type  : FUNC
 * @tc.number: HpaeFastCapturerManagerTest_011
 * @tc.desc  : Test deinit move-default branch and reload failure callback path.
 */
HWTEST_F(HpaeFastCapturerManagerTest, DeInitAndReloadFailure, TestSize.Level1)
{
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    auto manager = CreatePreparedManager(mockSource);
    auto callback = std::make_shared<NiceMock<MockSendMsgCallback>>();
    manager->RegisterSendMsgCallback(callback);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    ON_CALL(*mockSource, Stop()).WillByDefault(Return(SUCCESS));

    EXPECT_CALL(*callback, InvokeSync(MOVE_ALL_SOURCE_OUTPUT, _)).Times(1);
    EXPECT_EQ(manager->DeInit(true), SUCCESS);
    EXPECT_EQ(manager->IsInit(), false);
    EXPECT_EQ(manager->sourceInputNode_, nullptr);

    manager = CreatePreparedManager(mockSource);
    manager->RegisterSendMsgCallback(callback);
    HpaeSourceInfo invalidSourceInfo = CreateFastSourceInfo();
    invalidSourceInfo.frameLen = 0;
    EXPECT_CALL(*callback, Invoke(RELOAD_AUDIO_SINK_RESULT, _)).Times(1);
    EXPECT_EQ(manager->ReloadCaptureManager(invalidSourceInfo, true), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->sourceInputNode_, nullptr);

    HpaeSourceInfo sourceInfo = CreateFastSourceInfo();
    manager = std::make_shared<HpaeFastCapturerManager>(sourceInfo);
    manager->RegisterSendMsgCallback(callback);
    EXPECT_CALL(*callback, Invoke(INIT_DEVICE_RESULT, _)).Times(1);
    EXPECT_EQ(manager->ReloadCaptureManager(invalidSourceInfo, false), SUCCESS);
    manager->HandleMsg();
    EXPECT_EQ(manager->DeactivateThread(), true);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
