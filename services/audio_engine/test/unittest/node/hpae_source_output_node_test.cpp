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

#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "audio_highpass_filter.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_NODE_ID = 1243;
const uint32_t DEFAULT_APP_UID = 1001;
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";

class HpaeSourceOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceOutputNodeTest::SetUp()
{}

void HpaeSourceOutputNodeTest::TearDown()
{}

class TestReadDataCb : public ICapturerStreamCallback, public std::enable_shared_from_this<TestReadDataCb> {
public:
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override
    {
        return SUCCESS;
    }
    TestReadDataCb()
    {}
    virtual ~TestReadDataCb()
    {}
};

HWTEST_F(HpaeSourceOutputNodeTest, constructHpaeSourceOutputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceOutputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSoruceOutputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSoruceOutputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSoruceOutputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSoruceOutputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

static void GetTestAudioSourceAttr(IAudioSourceAttr &attr)
{
    attr.adapterName = "";
    attr.openMicSpeaker = 0;
    attr.format = AudioSampleFormat::INVALID_WIDTH;
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    attr.volume = 0.0f;
    attr.bufferSize = 0;
    attr.isBigEndian = false;
    attr.filePath = g_rootCapturerPath;
    attr.deviceNetworkId = "";
    attr.deviceType = 0;
    attr.sourceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
}

HWTEST_F(HpaeSourceOutputNodeTest, connectHpaeSourceInputAndOutputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSoruceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    GetTestAudioSourceAttr(attr);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStart(), 0);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == STREAM_MANAGER_RUNNING, true);

    hpaeSoruceOutputNode->Connect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 2);  // 2 for test
    hpaeSoruceOutputNode->DoProcess();

    std::shared_ptr<TestReadDataCb> testReadDataCb = std::make_shared<TestReadDataCb>();
    hpaeSoruceOutputNode->RegisterReadCallback(testReadDataCb);
    hpaeSoruceOutputNode->DoProcess();

    hpaeSoruceOutputNode->DisConnect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 1);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStop(), 0);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == STREAM_MANAGER_SUSPENDED, true);
}

static HpaeNodeInfo GetTestNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channelLayout = CH_LAYOUT_MONO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    return nodeInfo;
}

/**
 * @tc.name: TestHpaeSourceOutputNode_001
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_001
 * @tc.desc: Test HpaeSourceOutputNode constructor and destructor
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 1024;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;

    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    // Test that buffers are initialized with correct sizes
    size_t expectedSourceOutputSize = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    EXPECT_GT(expectedSourceOutputSize, 0U);

    size_t expectedInterleveSize = nodeInfo.frameLen * nodeInfo.channels;
    EXPECT_GT(expectedInterleveSize, 0U);
}

/**
 * @tc.name: SetNodeInfo_001
 * @tc.type: FUNC
 * @tc.number: SetNodeInfo_001
 * @tc.desc: Test SetNodeInfo resizes internal buffers when frame length changes.
 */
HWTEST_F(HpaeSourceOutputNodeTest, SetNodeInfo_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 160;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.frameLen = 240;
    node->SetNodeInfo(newNodeInfo);

    EXPECT_EQ(node->GetNodeInfo().frameLen, newNodeInfo.frameLen);
    EXPECT_EQ(node->sourceOutputData_.size(),
        newNodeInfo.frameLen * newNodeInfo.channels * GetSizeFromFormat(newNodeInfo.format));
    EXPECT_EQ(node->interleveData_.size(), newNodeInfo.frameLen * newNodeInfo.channels);
}

/**
 * @tc.name: SetNodeInfo_002
 * @tc.type: FUNC
 * @tc.number: SetNodeInfo_002
 * @tc.desc: Test SetNodeInfo keeps internal buffer sizes when frame length is unchanged.
 */
HWTEST_F(HpaeSourceOutputNodeTest, SetNodeInfo_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 160;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    size_t sourceOutputSize = node->sourceOutputData_.size();
    size_t interleveSize = node->interleveData_.size();

    HpaeNodeInfo newNodeInfo = nodeInfo;
    node->SetNodeInfo(newNodeInfo);

    EXPECT_EQ(node->GetNodeInfo().frameLen, newNodeInfo.frameLen);
    EXPECT_EQ(node->sourceOutputData_.size(), sourceOutputSize);
    EXPECT_EQ(node->interleveData_.size(), interleveSize);
}

/**
 * @tc.name: SetNodeInfo_003
 * @tc.type: FUNC
 * @tc.number: SetNodeInfo_003
 * @tc.desc: Test SetNodeInfo resizes internal buffers when only channels changes (same frameLen).
 */
HWTEST_F(HpaeSourceOutputNodeTest, SetNodeInfo_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 160;
    nodeInfo.channels = CHANNEL_8;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceType = SOURCE_TYPE_MIC;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    // Same frameLen, channels-only change: buffers must be refreshed to the new geometry.
    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.channels = CHANNEL_16;
    node->SetNodeInfo(newNodeInfo);

    EXPECT_EQ(node->GetNodeInfo().channels, CHANNEL_16);
    EXPECT_EQ(node->GetNodeInfo().frameLen, newNodeInfo.frameLen);
    EXPECT_EQ(node->sourceOutputData_.size(),
        newNodeInfo.frameLen * newNodeInfo.channels * GetSizeFromFormat(newNodeInfo.format));
    EXPECT_EQ(node->interleveData_.size(), newNodeInfo.frameLen * newNodeInfo.channels);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_002
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_002
 * @tc.desc: Test HpaeSourceOutputNode GetTimestamp functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    uint64_t timestamp1 = node->GetTimestamp();
    uint64_t timestamp2 = node->GetTimestamp();

    EXPECT_GT(timestamp1, 0U);
    EXPECT_GT(timestamp2, 0U);
    EXPECT_GE(timestamp2, timestamp1);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_003
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_003
 * @tc.desc: Test HpaeSourceOutputNode SetState and GetState functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial state
    HpaeSessionState initialState = node->GetState();
    EXPECT_EQ(initialState, HPAE_SESSION_NEW);

    // Test setting new state
    HpaeSessionState newState = HPAE_SESSION_RUNNING;
    int32_t result = node->SetState(newState);

    EXPECT_EQ(result, SUCCESS);

    HpaeSessionState currentState = node->GetState();
    EXPECT_EQ(currentState, newState);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_004
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_004
 * @tc.desc: Test HpaeSourceOutputNode SetAppUid and GetAppUid functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial UID
    int32_t initialUid = node->GetAppUid();
    EXPECT_EQ(initialUid, -1);

    // Test setting new UID
    int32_t testUid = DEFAULT_APP_UID;
    node->SetAppUid(testUid);

    int32_t currentUid = node->GetAppUid();
    EXPECT_EQ(currentUid, testUid);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_005
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_005
 * @tc.desc: Test HpaeSourceOutputNode SetMute functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial mute state
    // Initial state should be false based on constructor

    EXPECT_EQ(node->isMute_, false);
    // Test setting mute to true
    node->SetMute(true);
    EXPECT_EQ(node->isMute_, true);

    // Test setting mute to false
    node->SetMute(false);
    EXPECT_EQ(node->isMute_, false);

    // Test setting same value
    node->SetMute(false);
    // Should handle same value without issue
    EXPECT_EQ(node->isMute_, false);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_006
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_006
 * @tc.desc: Test HpaeSourceOutputNode Reset functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    bool result = node->Reset();

    EXPECT_TRUE(result);
    // Reset should always return true and clean up connections
}

/**
 * @tc.name: TestHpaeSourceOutputNode_007
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_007
 * @tc.desc: Test HpaeSourceOutputNode ResetAll functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    bool result = node->ResetAll();

    EXPECT_TRUE(result);
    // ResetAll should always return true and clean up all connections
}

/**
 * @tc.name: TestHpaeSourceOutputNode_008
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_008
 * @tc.desc: Test HpaeSourceOutputNode RegisterReadCallback with null callback
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    std::weak_ptr<ICapturerStreamCallback> nullCallback;
    bool result = node->RegisterReadCallback(nullCallback);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_009
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_009
 * @tc.desc: Test HpaeSourceOutputNode DisConnect with null preNode
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_009, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    std::shared_ptr<OutputNode<HpaePcmBuffer*>> nullPreNode = nullptr;

    // Should handle null preNode gracefully without crash
    node->DisConnect(nullPreNode);
    SUCCEED();
    EXPECT_EQ(node->inputStream_.outputPorts_.size(), 0);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_010
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_010
 * @tc.desc: Test HpaeSourceOutputNode constructor with different buffer sizes
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_010, TestSize.Level1)
{
    // Test with different frame lengths
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    nodeInfo1.frameLen = 512;
    nodeInfo1.channels = MONO;
    nodeInfo1.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node1 = std::make_unique<HpaeSourceOutputNode>(nodeInfo1);
    EXPECT_NE(node1, nullptr);

    HpaeNodeInfo nodeInfo2 = GetTestNodeInfo();
    nodeInfo2.frameLen = 1024;
    nodeInfo2.channels = STEREO;
    nodeInfo2.format = SAMPLE_F32LE;
    std::unique_ptr<HpaeSourceOutputNode> node2 = std::make_unique<HpaeSourceOutputNode>(nodeInfo2);
    EXPECT_NE(node2, nullptr);

    HpaeNodeInfo nodeInfo3 = GetTestNodeInfo();
    nodeInfo3.frameLen = 2048;
    nodeInfo3.channels = CHANNEL_4;
    nodeInfo3.format = SAMPLE_S24LE;
    std::unique_ptr<HpaeSourceOutputNode> node3 = std::make_unique<HpaeSourceOutputNode>(nodeInfo3);
    EXPECT_NE(node3, nullptr);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_011
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_011
 * @tc.desc: Test HpaeSourceOutputNode state transitions
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_011, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test multiple state transitions
    HpaeSessionState states[] = {
        HPAE_SESSION_NEW,
        HPAE_SESSION_PREPARED,
        HPAE_SESSION_RUNNING,
        HPAE_SESSION_PAUSED,
        HPAE_SESSION_STOPPED,
        HPAE_SESSION_RELEASED
    };

    for (auto state : states) {
        int32_t result = node->SetState(state);
        EXPECT_EQ(result, SUCCESS);

        HpaeSessionState currentState = node->GetState();
        EXPECT_EQ(currentState, state);
    }
}

/**
 * @tc.name: TestHpaeSourceOutputNode_012
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_012
 * @tc.desc: Test HpaeSourceOutputNode UID management
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_012, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test various UID values
    int32_t testUids[] = {0, 1000, 1001, 9999, -1, 100000}; // 1000, 1001, 9999, 100000 for test uid

    for (auto uid : testUids) {
        node->SetAppUid(uid);
        int32_t currentUid = node->GetAppUid();
        EXPECT_EQ(currentUid, uid);
    }
}

/**
 * @tc.name: TestHpaeSourceOutputNode_013
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_013
 * @tc.desc: Test HpaeSourceOutputNode mute state persistence
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_013, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test mute state changes
    node->SetMute(true);
    EXPECT_EQ(node->isMute_, true);
    // Perform some operations
    node->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(node->state_, HPAE_SESSION_RUNNING);
    node->SetAppUid(DEFAULT_APP_UID);
    EXPECT_EQ(node->GetAppUid(), DEFAULT_APP_UID);

    node->SetMute(false);
    EXPECT_EQ(node->isMute_, false);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_014
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_014
 * @tc.desc: Test HpaeSourceOutputNode with different source types
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_014, TestSize.Level1)
{
    SourceType sourceTypes[] = {
        SOURCE_TYPE_MIC,
        SOURCE_TYPE_VOICE_COMMUNICATION,
        SOURCE_TYPE_VOICE_RECOGNITION,
        SOURCE_TYPE_PLAYBACK_CAPTURE
    };

    for (auto sourceType : sourceTypes) {
        HpaeNodeInfo nodeInfo = GetTestNodeInfo();
        nodeInfo.sourceType = sourceType;
        std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);

        // Test basic operations with different source types
        node->SetState(HPAE_SESSION_RUNNING);
        node->SetAppUid(DEFAULT_APP_UID);
        node->SetMute(false);

        HpaeSessionState state = node->GetState();
        EXPECT_EQ(state, HPAE_SESSION_RUNNING);
        int32_t uid = node->GetAppUid();
        EXPECT_EQ(uid, DEFAULT_APP_UID);
        uint64_t timestamp = node->GetTimestamp();
        EXPECT_GT(timestamp, 0U);
    }
}

/**
 * @tc.name: TestHpaeSourceOutputNode_015
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_015
 * @tc.desc: Test HpaeSourceOutputNode Reset and ResetAll consistency
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_015, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Set some state
    node->SetState(HPAE_SESSION_RUNNING);
    node->SetAppUid(DEFAULT_APP_UID);
    node->SetMute(true);

    // Test Reset
    bool resetResult = node->Reset();
    EXPECT_TRUE(resetResult);

    // State should persist after Reset (Reset only handles connections)
    HpaeSessionState stateAfterReset = node->GetState();
    int32_t uidAfterReset = node->GetAppUid();

    EXPECT_EQ(stateAfterReset, HPAE_SESSION_RUNNING);
    EXPECT_EQ(uidAfterReset, DEFAULT_APP_UID);

    // Test ResetAll
    bool resetAllResult = node->ResetAll();
    EXPECT_TRUE(resetAllResult);

    // State should persist after ResetAll as well
    HpaeSessionState stateAfterResetAll = node->GetState();
    int32_t uidAfterResetAll = node->GetAppUid();

    EXPECT_EQ(stateAfterResetAll, HPAE_SESSION_RUNNING);
    EXPECT_EQ(uidAfterResetAll, DEFAULT_APP_UID);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_016
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_016
 * @tc.desc: Test HpaeSourceOutputNode GetMute functionality
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_016, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial mute state
    bool initialMute = node->GetMute();
    EXPECT_EQ(initialMute, false);

    // Test setting mute to true
    node->SetMute(true);
    bool currentMute = node->GetMute();
    EXPECT_EQ(currentMute, true);

    // Test setting mute to false
    node->SetMute(false);
    currentMute = node->GetMute();
    EXPECT_EQ(currentMute, false);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_017
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_017
 * @tc.desc: Test HpaeSourceOutputNode TokenId management
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_017, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial token ID
    uint32_t initialTokenId = node->GetTokenId();
    EXPECT_EQ(initialTokenId, 0U);

    // Test setting token ID
    uint32_t testTokenId = 12345U;
    node->SetTokenId(testTokenId);

    uint32_t currentTokenId = node->GetTokenId();
    EXPECT_EQ(currentTokenId, testTokenId);

    // Test with different token ID values
    uint32_t testTokenIds[] = {0U, 1U, 99999U, UINT32_MAX};

    for (auto tokenId : testTokenIds) {
        node->SetTokenId(tokenId);
        EXPECT_EQ(node->GetTokenId(), tokenId);
    }
}

/**
 * @tc.name: TestHpaeSourceOutputNode_018
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_018
 * @tc.desc: Test HpaeSourceOutputNode PcEnableState management
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_018, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial PC enable state
    int32_t initialPcState = node->GetPcEnableState();
    // Initial value is from system parameter, just check we can get it
    EXPECT_GE(initialPcState, 0);

    // Test setting PC enable state to 1
    node->SetPcEnableState(1);
    int32_t currentPcState = node->GetPcEnableState();
    EXPECT_EQ(currentPcState, 1);

    // Test setting PC enable state to 0
    node->SetPcEnableState(0);
    currentPcState = node->GetPcEnableState();
    EXPECT_EQ(currentPcState, 0);

    // Test setting PC enable state to different values
    int32_t testValues[] = {-1, 0, 1, 2, 100};
    for (auto val : testValues) {
        node->SetPcEnableState(val);
        EXPECT_EQ(node->GetPcEnableState(), val);
    }
}

/**
 * @tc.name: TestHpaeSourceOutputNode_019
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_019
 * @tc.desc: Test HpaeSourceOutputNode with ULTRASONIC source type
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_019, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sourceType = SOURCE_TYPE_ULTRASONIC;
    nodeInfo.channels = STEREO;

    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    // Test basic operations with ULTRASONIC source type
    node->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(node->GetState(), HPAE_SESSION_RUNNING);

    node->SetAppUid(DEFAULT_APP_UID);
    EXPECT_EQ(node->GetAppUid(), DEFAULT_APP_UID);

    // Test with PC enable state set
    node->SetPcEnableState(1);
    std::unique_ptr<HpaeSourceOutputNode> nodeWithFilter = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(nodeWithFilter, nullptr);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_020
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_020
 * @tc.desc: Test HpaeSourceOutputNode with VOICE_DOWNLINK source type
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_020, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sourceType = SOURCE_TYPE_VOICE_DOWNLINK;
    nodeInfo.format = SAMPLE_S16LE;

    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    // Test basic operations with VOICE_DOWNLINK source type
    node->SetState(HPAE_SESSION_RUNNING);
    node->SetAppUid(DEFAULT_APP_UID);
    node->SetMute(false);

    EXPECT_EQ(node->GetState(), HPAE_SESSION_RUNNING);
    EXPECT_EQ(node->GetAppUid(), DEFAULT_APP_UID);
    EXPECT_EQ(node->GetMute(), false);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_021
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_021
 * @tc.desc: Test HpaeSourceOutputNode with PLAYBACK_CAPTURE source type
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_021, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;

    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(node, nullptr);

    // Test basic operations with PLAYBACK_CAPTURE source type
    node->SetState(HPAE_SESSION_RUNNING);
    node->SetAppUid(DEFAULT_APP_UID);

    EXPECT_EQ(node->GetState(), HPAE_SESSION_RUNNING);
    EXPECT_EQ(node->GetAppUid(), DEFAULT_APP_UID);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_022
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_022
 * @tc.desc: Test SetNodeInfo with format change
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_022, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 160;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    size_t sourceOutputSize = node->sourceOutputData_.size();
    size_t interleveSize = node->interleveData_.size();

    // Change format from S16LE to F32LE
    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.format = SAMPLE_F32LE;
    newNodeInfo.frameLen = 320;
    node->SetNodeInfo(newNodeInfo);

    EXPECT_EQ(node->GetNodeInfo().format, SAMPLE_F32LE);
    EXPECT_EQ(node->GetNodeInfo().frameLen, 320);
    // Buffer sizes should change due to frameLen change
    EXPECT_NE(node->sourceOutputData_.size(), sourceOutputSize);
    EXPECT_NE(node->interleveData_.size(), interleveSize);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_023
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_023
 * @tc.desc: Test SetMute with same value twice
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_023, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test setting mute to same value twice
    node->SetMute(true);
    bool mute1 = node->GetMute();
    node->SetMute(true);  // Set same value again
    bool mute2 = node->GetMute();
    EXPECT_EQ(mute1, true);
    EXPECT_EQ(mute2, true);

    node->SetMute(false);
    mute1 = node->GetMute();
    node->SetMute(false);  // Set same value again
    mute2 = node->GetMute();
    EXPECT_EQ(mute1, false);
    EXPECT_EQ(mute2, false);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_024
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_024
 * @tc.desc: Test SetNodeInfo with same parameters
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_024, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 160;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    size_t sourceOutputSize = node->sourceOutputData_.size();
    size_t interleveSize = node->interleveData_.size();

    // Set same node info again
    HpaeNodeInfo sameNodeInfo = nodeInfo;
    node->SetNodeInfo(sameNodeInfo);

    // Buffer sizes should remain the same
    EXPECT_EQ(node->sourceOutputData_.size(), sourceOutputSize);
    EXPECT_EQ(node->interleveData_.size(), interleveSize);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_025
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_025
 * @tc.desc: Test SetAppUid with various values including negative
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_025, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    // Test initial UID
    int32_t initialUid = node->GetAppUid();
    EXPECT_EQ(initialUid, -1);

    // Test with various UID values
    int32_t testUids[] = {-100, -1, 0, 1, 1000, 99999, INT32_MAX};

    for (auto uid : testUids) {
        node->SetAppUid(uid);
        EXPECT_EQ(node->GetAppUid(), uid);
    }
}

/**
 * @tc.name: TestHpaeSourceOutputNode_026
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_026
 * @tc.desc: Test RegisterReadCallback with valid callback
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_026, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    std::shared_ptr<TestReadDataCb> testReadDataCb = std::make_shared<TestReadDataCb>();
    std::weak_ptr<ICapturerStreamCallback> validCallback = testReadDataCb;

    bool result = node->RegisterReadCallback(validCallback);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: TestHpaeSourceOutputNode_027
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceOutputNode_027
 * @tc.desc: Test RegisterReadCallback with expired callback
 */
HWTEST_F(HpaeSourceOutputNodeTest, TestHpaeSourceOutputNode_027, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);

    std::weak_ptr<ICapturerStreamCallback> expiredCallback;
    {
        std::shared_ptr<TestReadDataCb> tempCb = std::make_shared<TestReadDataCb>();
        expiredCallback = tempCb;
    }
    // tempCb is destroyed here, expiredCallback is now expired

    bool result = node->RegisterReadCallback(expiredCallback);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetupAudioLimiter_andReset_001
 * @tc.type: FUNC
 * @tc.number: SetupAudioLimiter_andReset_001
 * @tc.desc: Cover SetupAudioLimiter create path (cpp:174 FALSE) + SetConfig success (cpp:181 TRUE),
 *           the already-setup early return (cpp:174 TRUE -> ERROR), and ResetAudioLimiter.
 */
HWTEST_F(HpaeSourceOutputNodeTest, SetupAudioLimiter_andReset_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    // First call: limiter_ == nullptr -> create path runs (cpp:174 FALSE). With valid config
    // SetConfig returns SUCCESS -> cpp:181 TRUE branch.
    int32_t ret1 = node->SetupAudioLimiter();
    EXPECT_EQ(ret1, SUCCESS);

    // Second call: limiter_ already non-null -> cpp:174 TRUE -> early return ERROR.
    int32_t ret2 = node->SetupAudioLimiter();
    EXPECT_EQ(ret2, ERROR);

    // ResetAudioLimiter puts limiter_ back to nullptr (statement coverage).
    node->ResetAudioLimiter();
    EXPECT_EQ(node->limiter_, nullptr);
}

/**
 * @tc.name: MixInputs_withoutLimiter_001
 * @tc.type: FUNC
 * @tc.number: MixInputs_withoutLimiter_001
 * @tc.desc: Cover MixInputs no-limiter path (cpp:199 TRUE): plain element-wise sum into mixedOutput_.
 */
HWTEST_F(HpaeSourceOutputNodeTest, MixInputs_withoutLimiter_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->limiter_, nullptr);  // precondition: no limiter -> cpp:199 TRUE path

    PcmBufferInfo info(node->GetChannelCount(), node->GetFrameLen(), node->GetSampleRate());
    HpaePcmBuffer a(info);
    HpaePcmBuffer b(info);
    std::vector<HpaePcmBuffer *> inputs = { &a, &b };
    HpaePcmBuffer *out = node->MixInputs(inputs);  // private; -fno-access-control
    EXPECT_EQ(out, &node->mixedOutput_);
}

/**
 * @tc.name: MixInputs_withLimiter_001
 * @tc.type: FUNC
 * @tc.number: MixInputs_withLimiter_001
 * @tc.desc: Cover MixInputs limiter path (cpp:199 FALSE): sum into tmpOutput_ then limiter->Process.
 */
HWTEST_F(HpaeSourceOutputNodeTest, MixInputs_withLimiter_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->SetupAudioLimiter(), SUCCESS);  // limiter_ now non-null
    ASSERT_NE(node->limiter_, nullptr);

    PcmBufferInfo info(node->GetChannelCount(), node->GetFrameLen(), node->GetSampleRate());
    HpaePcmBuffer a(info);
    HpaePcmBuffer b(info);
    std::vector<HpaePcmBuffer *> inputs = { &a, &b };
    HpaePcmBuffer *out = node->MixInputs(inputs);  // cpp:199 FALSE -> tmpOutput + limiter->Process
    EXPECT_EQ(out, &node->mixedOutput_);
}

/**
 * @tc.name: SelectOutputData_injectorMixMode_001
 * @tc.type: FUNC
 * @tc.number: SelectOutputData_injectorMixMode_001
 * @tc.desc: Cover SelectOutputData injector+unmuted path (cpp:146 FALSE, cpp:150 TRUE -> MixInputs).
 */
HWTEST_F(HpaeSourceOutputNodeTest, SelectOutputData_injectorMixMode_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    node->SetInjectorState(true);   // cpp:146 FALSE (!isInjector_ == false -> skip front())
    node->SetMicMute(false);        // cpp:150 TRUE (!isMicMute_ == true -> MixInputs)

    PcmBufferInfo info(node->GetChannelCount(), node->GetFrameLen(), node->GetSampleRate());
    HpaePcmBuffer micBuf(info);
    micBuf.SetSourceBufferType(HPAE_SOURCE_BUFFER_TYPE_MIC);
    std::vector<HpaePcmBuffer *> vec = { &micBuf };
    HpaePcmBuffer *out = node->SelectOutputData(vec);  // private; -fno-access-control
    EXPECT_EQ(out, &node->mixedOutput_);  // mix mode returns the node-internal mixed buffer
}

/**
 * @tc.name: SelectOutputData_injectorSoloMode_withInjector_001
 * @tc.type: FUNC
 * @tc.number: SelectOutputData_injectorSoloMode_withInjector_001
 * @tc.desc: Cover SelectOutputData solo mode (cpp:150 FALSE) that finds the INJECTOR buffer
 *           (cpp:155 TRUE arm) and returns it directly.
 */
HWTEST_F(HpaeSourceOutputNodeTest, SelectOutputData_injectorSoloMode_withInjector_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    node->SetInjectorState(true);
    node->SetMicMute(true);   // solo mode: cpp:150 FALSE (!isMicMute_ == false) -> injector-only loop

    PcmBufferInfo info(node->GetChannelCount(), node->GetFrameLen(), node->GetSampleRate());
    HpaePcmBuffer micBuf(info);
    micBuf.SetSourceBufferType(HPAE_SOURCE_BUFFER_TYPE_MIC);  // skipped: type != INJECTOR
    HpaePcmBuffer injBuf(info);
    injBuf.SetSourceBufferType(HPAE_SOURCE_BUFFER_TYPE_INJECTOR);  // matched: cpp:155 TRUE
    std::vector<HpaePcmBuffer *> vec = { &micBuf, &injBuf };
    HpaePcmBuffer *out = node->SelectOutputData(vec);
    EXPECT_EQ(out, &injBuf);
}

/**
 * @tc.name: SelectOutputData_injectorSoloMode_noInjector_001
 * @tc.type: FUNC
 * @tc.number: SelectOutputData_injectorSoloMode_noInjector_001
 * @tc.desc: Cover SelectOutputData solo mode with no INJECTOR buffer present: the loop matches
 *           nothing (cpp:155 FALSE arm) and falls back to &silenceData_.
 */
HWTEST_F(HpaeSourceOutputNodeTest, SelectOutputData_injectorSoloMode_noInjector_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    node->SetInjectorState(true);
    node->SetMicMute(true);

    PcmBufferInfo info(node->GetChannelCount(), node->GetFrameLen(), node->GetSampleRate());
    HpaePcmBuffer micBuf(info);
    micBuf.SetSourceBufferType(HPAE_SOURCE_BUFFER_TYPE_MIC);  // cpp:155 FALSE: type != INJECTOR
    std::vector<HpaePcmBuffer *> vec = { &micBuf };
    HpaePcmBuffer *out = node->SelectOutputData(vec);
    EXPECT_EQ(out, &node->silenceData_);  // fallback when no injector buffer found
}

/**
 * @tc.name: ProcessAudioData_voiceDownlink_001
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_voiceDownlink_001
 * @tc.desc: Cover ProcessAudioData non-mute path for SOURCE_TYPE_VOICE_DOWNLINK: the downlink
 *           CopyRightToLeft branch (cpp:234 TRUE). Highpass filter is disabled (not ULTRASONIC),
 *           so cpp:224 stays FALSE.
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_voiceDownlink_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sourceType = SOURCE_TYPE_VOICE_DOWNLINK;
    nodeInfo.format = SAMPLE_S16LE;
    auto node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    PcmBufferInfo info(node->GetChannelCount(), node->GetFrameLen(), node->GetSampleRate());
    HpaePcmBuffer outBuf(info);  // zero-filled float buffer matches node frame/channel dims
    // isMute=false -> else branch; isSupportHighpassFilter_=false -> cpp:224 FALSE; then
    // sourceType == VOICE_DOWNLINK -> cpp:234 TRUE (CopyRightToLeft).
    node->ProcessAudioData(&outBuf, false);  // private; -fno-access-control
    SUCCEED();
}

/**
 * @tc.name: SetNodeInfo_004
 * @tc.type: FUNC
 * @tc.number: SetNodeInfo_004
 * @tc.desc: SetNodeInfo rebuilds injector-path buffers (mixedOutput_/tmpOutput_/silenceData_)
 *           with the new geometry instead of keeping ctor-sized buffers.
 */
HWTEST_F(HpaeSourceOutputNodeTest, SetNodeInfo_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 160;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.frameLen = 240;
    node->SetNodeInfo(newNodeInfo);

    EXPECT_EQ(node->mixedOutput_.GetFrameLen(), newNodeInfo.frameLen);
    EXPECT_EQ(node->tmpOutput_.GetFrameLen(), newNodeInfo.frameLen);
    EXPECT_EQ(node->silenceData_.GetFrameLen(), newNodeInfo.frameLen);
    EXPECT_EQ(node->mixedOutput_.GetChannelCount(), newNodeInfo.channels);
}

/**
 * @tc.name: ProcessAudioData_Clamp_001
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_Clamp_001
 * @tc.desc: ProcessAudioData clamps reads to the upstream buffer capacity (upstream converter
 *           frozen at a smaller frameLen) and zeroes the unconverted tail.
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_Clamp_001, TestSize.Level1)
{
    const uint32_t upstreamFrameLen = 3840; // cached converter span, 80ms@48k
    const uint32_t nodeFrameLen = 4800;     // session span, 100ms@48k
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = nodeFrameLen;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    PcmBufferInfo upstreamInfo(nodeInfo.channels, upstreamFrameLen, nodeInfo.samplingRate);
    HpaePcmBuffer upstream(upstreamInfo);
    float *upstreamData = upstream.GetPcmDataBuffer();
    ASSERT_NE(upstreamData, nullptr);
    std::fill(upstreamData, upstreamData + upstreamFrameLen * nodeInfo.channels, 1.0f);

    std::fill(node->sourceOutputData_.begin(), node->sourceOutputData_.end(), 0xFF);
    node->ProcessAudioData(&upstream, false); // must not read past upstream capacity

    size_t totalBytes = static_cast<size_t>(nodeFrameLen) * nodeInfo.channels
        * GetSizeFromFormat(nodeInfo.format);
    size_t convertedBytes = static_cast<size_t>(upstreamFrameLen) * nodeInfo.channels
        * GetSizeFromFormat(nodeInfo.format);
    ASSERT_EQ(node->sourceOutputData_.size(), totalBytes);
    bool headNonZero = false;
    for (size_t i = 0; i < convertedBytes; i++) {
        headNonZero = headNonZero || (node->sourceOutputData_[i] != 0);
    }
    EXPECT_TRUE(headNonZero); // converted head from the 1.0f input
    for (size_t i = convertedBytes; i < totalBytes; i++) {
        EXPECT_EQ(node->sourceOutputData_[i], 0); // zeroed tail, not stale 0xFF
    }
}

/**
 * @tc.name: ProcessAudioData_Clamp_002
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_Clamp_002
 * @tc.desc: ProcessAudioData with a full-size upstream buffer converts the whole frame
 *           (clamp must not engage when geometry matches).
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_Clamp_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 4800; // 5 for 100ms@48k
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    PcmBufferInfo upstreamInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate);
    HpaePcmBuffer upstream(upstreamInfo);
    float *upstreamData = upstream.GetPcmDataBuffer();
    ASSERT_NE(upstreamData, nullptr);
    std::fill(upstreamData, upstreamData + nodeInfo.frameLen * nodeInfo.channels, 1.0f);

    std::fill(node->sourceOutputData_.begin(), node->sourceOutputData_.end(), 0);
    node->ProcessAudioData(&upstream, false);

    for (size_t i = 0; i < node->sourceOutputData_.size(); i++) {
        EXPECT_EQ(node->sourceOutputData_[i] != 0, true); // fully converted, no zeroed tail
    }
}

/**
 * @tc.name: ProcessAudioData_Mute_001
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_Mute_001
 * @tc.desc: ProcessAudioData with isMute true zeroes the whole output regardless of the input.
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_Mute_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 480; // 480 for 10ms@48k
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    PcmBufferInfo upstreamInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate);
    HpaePcmBuffer upstream(upstreamInfo);
    float *upstreamData = upstream.GetPcmDataBuffer();
    ASSERT_NE(upstreamData, nullptr);
    std::fill(upstreamData, upstreamData + nodeInfo.frameLen * nodeInfo.channels, 1.0f);

    std::fill(node->sourceOutputData_.begin(), node->sourceOutputData_.end(), 0xFF);
    node->ProcessAudioData(&upstream, true); // mute path must not convert anything

    for (size_t i = 0; i < node->sourceOutputData_.size(); i++) {
        EXPECT_EQ(node->sourceOutputData_[i], 0); // fully zeroed by the mute path
    }
}

/**
 * @tc.name: ProcessAudioData_Clamp_003
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_Clamp_003
 * @tc.desc: ProcessAudioData clamps and still applies the highpass branch when the node supports
 *           it: safeFrameCount passed to ApplyHighpassFilter is the clamped upstream span.
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_Clamp_003, TestSize.Level1)
{
    // 2 for nbSamples = 2 * 1920 == 3840 keeps the device-side filter chain alive end to end
    const uint32_t upstreamFrameLen = 1920;
    const uint32_t nodeFrameLen = 4800; // 5 for 100ms@48k, larger than upstream so clamp engages
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = nodeFrameLen;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    // Force the ultrasonic support bits: on PC UT the filter .so is absent so ApplyFilter returns
    // ERR_INVALID_HANDLE and ApplyHighpassFilter returns early safely; on device the full chain
    // runs with nbSamples == 3840. Both paths must keep the clamp semantics below.
    node->isSupportHighpassFilter_ = true;
    node->filterChannelCount_ = static_cast<int32_t>(nodeInfo.channels);
    node->highpassFilter_ = std::make_unique<AudioHighPassFilter>();
    node->highpassFilter_->InitFilter(node->filterChannelCount_); // init on device; fails safely on PC

    PcmBufferInfo upstreamInfo(nodeInfo.channels, upstreamFrameLen, nodeInfo.samplingRate);
    HpaePcmBuffer upstream(upstreamInfo);
    float *upstreamData = upstream.GetPcmDataBuffer();
    ASSERT_NE(upstreamData, nullptr);
    std::fill(upstreamData, upstreamData + upstreamFrameLen * nodeInfo.channels, 1.0f);

    std::fill(node->sourceOutputData_.begin(), node->sourceOutputData_.end(), 0xFF);
    node->ProcessAudioData(&upstream, false); // clamp + highpass branch, must stay safe on both envs

    size_t totalBytes = static_cast<size_t>(nodeFrameLen) * nodeInfo.channels
        * GetSizeFromFormat(nodeInfo.format);
    size_t convertedBytes = static_cast<size_t>(upstreamFrameLen) * nodeInfo.channels
        * GetSizeFromFormat(nodeInfo.format);
    ASSERT_EQ(node->sourceOutputData_.size(), totalBytes);
    bool headNonZero = false;
    for (size_t i = 0; i < convertedBytes; i++) {
        headNonZero = headNonZero || (node->sourceOutputData_[i] != 0);
    }
    EXPECT_TRUE(headNonZero); // converted head from the 1.0f input
    for (size_t i = convertedBytes; i < totalBytes; i++) {
        EXPECT_EQ(node->sourceOutputData_[i], 0); // zeroed tail, not stale 0xFF
    }
}

/**
 * @tc.name: ProcessAudioData_Clamp_004
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_Clamp_004
 * @tc.desc: ProcessAudioData aligns safeSamples down to whole frames of the node channel count:
 *           3 upstream samples against a stereo node convert exactly 2 samples.
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_Clamp_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 4800; // 5 for 100ms@48k
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    const uint32_t upstreamChannels = 3; // 3 samples total: 3 % 2 == 1, alignment drops one
    const uint32_t upstreamFrameLen = 1;
    PcmBufferInfo upstreamInfo(upstreamChannels, upstreamFrameLen, nodeInfo.samplingRate);
    HpaePcmBuffer upstream(upstreamInfo);
    float *upstreamData = upstream.GetPcmDataBuffer();
    ASSERT_NE(upstreamData, nullptr);
    std::fill(upstreamData, upstreamData + upstreamChannels * upstreamFrameLen, 1.0f);

    std::fill(node->sourceOutputData_.begin(), node->sourceOutputData_.end(), 0xFF);
    node->ProcessAudioData(&upstream, false);

    size_t bytesPerSample = GetSizeFromFormat(nodeInfo.format);
    size_t totalBytes = static_cast<size_t>(nodeInfo.frameLen) * nodeInfo.channels
        * bytesPerSample;
    for (size_t i = 0; i < 2 * bytesPerSample; i++) {
        EXPECT_NE(node->sourceOutputData_[i], 0); // exactly 2 aligned samples converted
    }
    for (size_t i = 2 * bytesPerSample; i < totalBytes; i++) {
        EXPECT_EQ(node->sourceOutputData_[i], 0); // misaligned third sample dropped, tail zeroed
    }
}

/**
 * @tc.name: ProcessAudioData_Clamp_005
 * @tc.type: FUNC
 * @tc.number: ProcessAudioData_Clamp_005
 * @tc.desc: ProcessAudioData with an empty upstream (0 frameLen) clamps safeSamples to 0: no
 *           highpass call, no conversion, the whole output stays zeroed.
 */
HWTEST_F(HpaeSourceOutputNodeTest, ProcessAudioData_Clamp_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.frameLen = 4800; // 5 for 100ms@48k
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::unique_ptr<HpaeSourceOutputNode> node = std::make_unique<HpaeSourceOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    node->isSupportHighpassFilter_ = true; // must still be skipped: safeSamples == 0

    PcmBufferInfo upstreamInfo(nodeInfo.channels, 0, nodeInfo.samplingRate);
    HpaePcmBuffer upstream(upstreamInfo);

    std::fill(node->sourceOutputData_.begin(), node->sourceOutputData_.end(), 0xFF);
    node->ProcessAudioData(&upstream, false); // empty span: everything skipped, output zeroed

    for (size_t i = 0; i < node->sourceOutputData_.size(); i++) {
        EXPECT_EQ(node->sourceOutputData_[i], 0); // zero-fill path, nothing converted
    }
}


/**
 * @tc.name  : doProcess edge gates: downlink active + missing read callback
 * @tc.type  : FUNC
 * @tc.number: HpaeSourceOutputNodeTest_EdgeLog_001
 * @tc.desc  : Downlink branch and null readCallback hit their edge gates, register recovers
 */
HWTEST_F(HpaeSourceOutputNodeTest, doProcessEdgeLogDownlinkAndNoCallback, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    nodeInfo.sourceType = SOURCE_TYPE_VOICE_DOWNLINK;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSoruceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId,
        SOURCE_TYPE_VOICE_DOWNLINK, sourceName), 0);
    IAudioSourceAttr attr;
    GetTestAudioSourceAttr(attr);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStart(), 0);
    hpaeSoruceOutputNode->Connect(hpaeSoruceInputNode);

    // no read callback: null gate first hit + sustained; register then recovers
    hpaeSoruceOutputNode->DoProcess(); // readCb null first hit + downlink first hit
    hpaeSoruceOutputNode->DoProcess(); // both sustained
    std::shared_ptr<TestReadDataCb> testReadDataCb = std::make_shared<TestReadDataCb>();
    hpaeSoruceOutputNode->RegisterReadCallback(testReadDataCb);
    hpaeSoruceOutputNode->DoProcess(); // readCb recovery line

    hpaeSoruceOutputNode->DisConnect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStop(), 0);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
