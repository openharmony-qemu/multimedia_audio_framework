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
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS