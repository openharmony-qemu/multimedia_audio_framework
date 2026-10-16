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
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "audio_errors.h"
#include "audio_shared_memory.h"
#include "audio_stream_enum.h"
#include "audio_utils.h"
#include "hpae_fast_source_input_node.h"
#include "hpae_format_convert.h"
#include "hpae_mocks.h"
#include "hpae_source_output_node.h"
#include "oh_audio_buffer.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t FAST_NODE_ID = 1001;
constexpr uint32_t FAST_SESSION_ID = 1002;
constexpr uint32_t FAST_FRAME_LENGTH = 16;
constexpr uint32_t FAST_TOTAL_FRAME = 32;
constexpr uint32_t FAST_SPAN_FRAME = 8;
constexpr uint32_t FAST_BYTE_PER_FRAME = 4;
constexpr uint32_t FAST_CAPTURE_ID = 7;
constexpr uint32_t FAST_TEST_UID = 1008;

HpaeNodeInfo CreateFastNodeInfo(uint32_t routeFlag = 0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = FAST_NODE_ID;
    nodeInfo.sessionId = FAST_SESSION_ID;
    nodeInfo.frameLen = FAST_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceType = SOURCE_TYPE_MIC;
    nodeInfo.deviceClass = "file_io";
    nodeInfo.deviceNetId = "LocalDevice";
    nodeInfo.deviceName = "Built_in_mic";
    nodeInfo.routeFlag = routeFlag;
    return nodeInfo;
}

std::shared_ptr<HpaeFastSourceInputNode> CreateFastNode(uint32_t routeFlag = 0)
{
    HpaeNodeInfo nodeInfo = CreateFastNodeInfo(routeFlag);
    return std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
}

void InitAudioSourceAttr(IAudioSourceAttr &attr)
{
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = CH_LAYOUT_STEREO;
    attr.sourceType = static_cast<int32_t>(SOURCE_TYPE_MIC);
    attr.audioStreamFlag = AUDIO_FLAG_MMAP;
}

std::shared_ptr<AudioSharedMemory> CreateMmapMemory(uint32_t totalFrame, uint32_t bytePerFrame, uint32_t syncInfoSize)
{
    size_t bufferSize = static_cast<size_t>(totalFrame) * bytePerFrame + syncInfoSize;
    return AudioSharedMemory::CreateFromLocal(bufferSize, "hpae_fast_source_input_node_test");
}

void ExpectMmapBufferInfo(const std::shared_ptr<NiceMock<MockAudioCaptureSource>> &mockSource,
    int fd, uint32_t totalFrame, uint32_t spanFrame, uint32_t bytePerFrame, uint32_t syncInfoSize)
{
    EXPECT_CALL(*mockSource, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(fd), SetArgReferee<1>(totalFrame),
            SetArgReferee<2>(spanFrame), SetArgReferee<3>(bytePerFrame),
            SetArgReferee<4>(syncInfoSize), Return(SUCCESS)));
}

void AttachMockSource(const std::shared_ptr<HpaeFastSourceInputNode> &node,
    const std::shared_ptr<NiceMock<MockAudioCaptureSource>> &mockSource)
{
    node->audioCapturerSource_ = mockSource;
    node->captureId_ = FAST_CAPTURE_ID;
}
} // namespace

class HpaeFastSourceInputNodeTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : ConstructAndPortApis
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_001
 * @tc.desc  : Test constructor, port accessors and simple state APIs.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, ConstructAndPortApis, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = CreateFastNodeInfo();
    auto fastNode = std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
    ASSERT_NE(fastNode, nullptr);

    EXPECT_EQ(fastNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(fastNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(fastNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(fastNode->GetBitWidth(), nodeInfo.format);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_NEW);
    EXPECT_EQ(fastNode->GetCaptureId(), HDI_INVALID_ID);
    EXPECT_EQ(fastNode->GetOutputPortNum(), 0);
    EXPECT_EQ(fastNode->Reset(), true);
    EXPECT_EQ(fastNode->ResetAll(), true);

    std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = fastNode;
    std::shared_ptr<HpaeNode> sharedNode = outputNode->GetSharedInstance();
    ASSERT_NE(sharedNode, nullptr);
    EXPECT_EQ(sharedNode->GetNodeId(), fastNode->GetNodeId());

    HpaeNodeInfo otherNodeInfo = CreateFastNodeInfo();
    EXPECT_EQ(fastNode->GetOutputPort(), fastNode->GetOutputPort(otherNodeInfo, true));
    EXPECT_EQ(fastNode->GetOutputPortBufferType(otherNodeInfo), HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_EQ(fastNode->SetSourceState(STREAM_MANAGER_IDLE), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : ConstructVariantsAndDfxCallback
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_012
 * @tc.desc  : Test constructor ternary branches and DFX callback lock branches.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, ConstructVariantsAndDfxCallback, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = CreateFastNodeInfo();
    nodeInfo.customSampleRate = SAMPLE_RATE_44100;
    nodeInfo.deviceClass = "";
    auto callback = std::make_shared<NiceMock<MockNodeCallback>>();
    nodeInfo.statusCallback = callback;

    EXPECT_CALL(*callback, OnNotifyDfxNodeAdmin(true, _)).Times(1);
    EXPECT_CALL(*callback, OnNotifyDfxNodeAdmin(false, _)).Times(1);
    {
        auto fastNode = std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
        ASSERT_NE(fastNode, nullptr);
        EXPECT_EQ(fastNode->logUtilsTag_, "HpaeFastSourceInputNode::Fast");
        EXPECT_EQ(fastNode->writeTimeModel_.sampleRate_, SAMPLE_RATE_44100);
    }
}

/**
 * @tc.name  : SourceStateGuards
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_002
 * @tc.desc  : Test invalid source/capture id guards and stop-without-source branch.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, SourceStateGuards, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    ASSERT_NE(fastNode, nullptr);

    EXPECT_NE(fastNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceReset(), SUCCESS);
    EXPECT_EQ(fastNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(fastNode->CapturerSourceDeInit(), SUCCESS);

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    fastNode->audioCapturerSource_ = mockSource;
    fastNode->captureId_ = HDI_INVALID_ID;
    EXPECT_NE(fastNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceReset(), SUCCESS);
}

/**
 * @tc.name  : SourceStateOperations
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_003
 * @tc.desc  : Test source state transitions and passthrough operation return paths.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, SourceStateOperations, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);

    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    EXPECT_CALL(*mockSource, Start()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastNode->needReSyncPosition_, true);

    EXPECT_CALL(*mockSource, Flush()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_EQ(fastNode->needReSyncPosition_, true);

    EXPECT_CALL(*mockSource, Pause()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode->CapturerSourcePause(), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSource, Resume()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode->CapturerSourceResume(), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_RUNNING);

    EXPECT_CALL(*mockSource, Reset()).WillOnce(Return(ERROR));
    EXPECT_EQ(fastNode->CapturerSourceReset(), ERROR);

    EXPECT_CALL(*mockSource, Stop()).WillOnce(Return(ERROR));
    EXPECT_EQ(fastNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(false));
    EXPECT_EQ(fastNode->CapturerSourceDeInit(), SUCCESS);
    EXPECT_EQ(fastNode->audioCapturerSource_, nullptr);
}

/**
 * @tc.name  : SourceOperationFailures
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_004
 * @tc.desc  : Test operation failure branches after source and capture id are valid.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, SourceOperationFailures, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);

    EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(false)).WillRepeatedly(Return(false));
    EXPECT_NE(fastNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(fastNode->CapturerSourceStop(), SUCCESS);

    fastNode = CreateFastNode();
    mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    EXPECT_CALL(*mockSource, Start()).WillOnce(Return(ERROR));
    EXPECT_NE(fastNode->CapturerSourceStart(), SUCCESS);

    EXPECT_CALL(*mockSource, Flush()).WillOnce(Return(ERROR));
    EXPECT_NE(fastNode->CapturerSourceFlush(), SUCCESS);

    EXPECT_CALL(*mockSource, Pause()).WillOnce(Return(ERROR));
    EXPECT_NE(fastNode->CapturerSourcePause(), SUCCESS);

    EXPECT_CALL(*mockSource, Resume()).WillOnce(Return(ERROR));
    EXPECT_NE(fastNode->CapturerSourceResume(), SUCCESS);
}

/**
 * @tc.name  : CapturerSourceInitRejectsInvalidInputs
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_005
 * @tc.desc  : Test init rejection for missing source, invalid id, init failure and bad mmap info.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, CapturerSourceInitRejectsInvalidInputs, TestSize.Level1)
{
    IAudioSourceAttr attr;
    InitAudioSourceAttr(attr);

    auto fastNode = CreateFastNode();
    EXPECT_NE(fastNode->CapturerSourceInit(attr), SUCCESS);

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    fastNode->audioCapturerSource_ = mockSource;
    fastNode->captureId_ = HDI_INVALID_ID;
    EXPECT_NE(fastNode->CapturerSourceInit(attr), SUCCESS);

    fastNode = CreateFastNode();
    mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(false)).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockSource, Init(_)).WillOnce(Return(ERROR));
    EXPECT_NE(fastNode->CapturerSourceInit(attr), SUCCESS);

    fastNode = CreateFastNode();
    mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(false)).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockSource, Init(_)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSource, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(INVALID_FD), SetArgReferee<1>(0),
            SetArgReferee<2>(0), SetArgReferee<3>(0), SetArgReferee<4>(0), Return(SUCCESS)));
    EXPECT_NE(fastNode->CapturerSourceInit(attr), SUCCESS);
}

/**
 * @tc.name  : CapturerSourceInitReleasesSourceOnFailure
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_032
 * @tc.desc  : Test CapturerSourceInit releases source and captureId_ when Init fails.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, CapturerSourceInitReleasesSourceOnFailure, TestSize.Level1)
{
    IAudioSourceAttr attr;
    InitAudioSourceAttr(attr);

    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(false)).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockSource, Init(_)).WillOnce(Return(ERROR));

    EXPECT_NE(fastNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(fastNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(fastNode->captureId_, HDI_INVALID_ID);
    EXPECT_EQ(fastNode->CapturerSourceDeInit(), SUCCESS);
}

/**
 * @tc.name  : CapturerSourceInitAndDeInit
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_006
 * @tc.desc  : Test successful mmap init for normal and sync-info buffers, then deinit cleanup.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, CapturerSourceInitAndDeInit, TestSize.Level1)
{
    IAudioSourceAttr attr;
    InitAudioSourceAttr(attr);

    for (uint32_t syncInfoSize : {0U, static_cast<uint32_t>(BASIC_SYNC_INFO_SIZE)}) {
        auto fastNode = CreateFastNode();
        auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
        AttachMockSource(fastNode, mockSource);
        auto memory = CreateMmapMemory(FAST_TOTAL_FRAME, FAST_BYTE_PER_FRAME, syncInfoSize);
        ASSERT_NE(memory, nullptr);

        EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(false)).WillRepeatedly(Return(true));
        EXPECT_CALL(*mockSource, Init(_)).WillOnce(Return(SUCCESS));
        ExpectMmapBufferInfo(mockSource, memory->GetFd(), FAST_TOTAL_FRAME, FAST_SPAN_FRAME,
            FAST_BYTE_PER_FRAME, syncInfoSize);
        EXPECT_EQ(fastNode->CapturerSourceInit(attr), SUCCESS);
        EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_IDLE);
        EXPECT_NE(fastNode->srcAudioBuffer_, nullptr);
        EXPECT_EQ(fastNode->srcSpanSizeInframe_, FAST_SPAN_FRAME);
        EXPECT_EQ(fastNode->GetNodeInfo().frameLen, FAST_SPAN_FRAME);

        EXPECT_CALL(*mockSource, DeInit()).Times(1);
        EXPECT_EQ(fastNode->CapturerSourceDeInit(), SUCCESS);
        EXPECT_EQ(fastNode->audioCapturerSource_, nullptr);
        EXPECT_EQ(fastNode->captureId_, HDI_INVALID_ID);
        EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_RELEASED);
    }

    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    auto memory = CreateMmapMemory(FAST_TOTAL_FRAME, FAST_BYTE_PER_FRAME, 0);
    ASSERT_NE(memory, nullptr);
    EXPECT_CALL(*mockSource, IsInited()).WillOnce(Return(true)).WillRepeatedly(Return(true));
    ExpectMmapBufferInfo(mockSource, memory->GetFd(), FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0);
    EXPECT_EQ(fastNode->CapturerSourceInit(attr), SUCCESS);
}

/**
 * @tc.name  : GetCapturerSourceInstanceBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_007
 * @tc.desc  : Test source instance selector branches for bus address, wakeup, EC and network id.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, GetCapturerSourceInstanceBranches, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    (void)fastNode->GetCapturerSourceInstance("invalid_device", "net_id", SOURCE_TYPE_MIC, "mic", "bus_address");
    (void)fastNode->GetCapturerSourceInstance("invalid_device", "net_id", SOURCE_TYPE_WAKEUP, "mic");
    (void)fastNode->GetCapturerSourceInstance("invalid_device", "net_id", SOURCE_TYPE_MIC, HDI_ID_INFO_EC);
    (void)fastNode->GetCapturerSourceInstance("invalid_device", "net_id", SOURCE_TYPE_MIC, "mic");
    EXPECT_NE(fastNode, nullptr);
}

/**
 * @tc.name  : AdapterBufferInfoConditionBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_013
 * @tc.desc  : Test each GetAdapterBufferInfo OR condition independently.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, AdapterBufferInfoConditionBranches, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    EXPECT_NE(fastNode->GetAdapterBufferInfo(), SUCCESS);

    struct MmapCase {
        int32_t ret;
        int fd;
        uint32_t totalFrame;
        uint32_t spanFrame;
        uint32_t bytePerFrame;
        uint32_t syncInfoSize;
        bool success;
    };
    std::vector<MmapCase> cases = {
        {ERROR, 10, FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0, false},
        {SUCCESS, INVALID_FD, FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0, false},
        {SUCCESS, 10, 0, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0, false},
        {SUCCESS, 10, FAST_TOTAL_FRAME, 0, FAST_BYTE_PER_FRAME, 0, false},
        {SUCCESS, 10, FAST_TOTAL_FRAME, FAST_SPAN_FRAME, 0, 0, false},
        {SUCCESS, 10, FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0, true},
    };

    for (const auto &testCase : cases) {
        fastNode = CreateFastNode();
        auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
        AttachMockSource(fastNode, mockSource);
        EXPECT_CALL(*mockSource, GetMmapBufferInfo(_, _, _, _, _))
            .WillOnce(DoAll(SetArgReferee<0>(testCase.fd), SetArgReferee<1>(testCase.totalFrame),
                SetArgReferee<2>(testCase.spanFrame), SetArgReferee<3>(testCase.bytePerFrame),
                SetArgReferee<4>(testCase.syncInfoSize), Return(testCase.ret)));
        int32_t ret = fastNode->GetAdapterBufferInfo();
        EXPECT_EQ(ret == SUCCESS, testCase.success);
    }
}

/**
 * @tc.name  : PrepareDeviceBufferConditionBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_014
 * @tc.desc  : Test sample-rate ternary and invalid sample rate path in PrepareDeviceBuffer.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, PrepareDeviceBufferConditionBranches, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    auto memory = CreateMmapMemory(FAST_TOTAL_FRAME, FAST_BYTE_PER_FRAME, 0);
    ASSERT_NE(memory, nullptr);
    fastNode->audioSourceAttr_.sampleRate = 0;
    ExpectMmapBufferInfo(mockSource, memory->GetFd(), FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0);
    EXPECT_EQ(fastNode->PrepareDeviceBuffer(), SUCCESS);
    EXPECT_GT(fastNode->spanDuration_, 0);

    HpaeNodeInfo nodeInfo = CreateFastNodeInfo();
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(0);
    fastNode = std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
    mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->audioSourceAttr_.sampleRate = 0;
    memory = CreateMmapMemory(FAST_TOTAL_FRAME, FAST_BYTE_PER_FRAME, 0);
    ASSERT_NE(memory, nullptr);
    ExpectMmapBufferInfo(mockSource, memory->GetFd(), FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME, 0);
    EXPECT_EQ(fastNode->PrepareDeviceBuffer(), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : BufferLoopHelpers
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_008
 * @tc.desc  : Test local buffer initialization, one-span read and next loop wakeup preparation.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, BufferLoopHelpers, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;

    fastNode->InitAudiobuffer(true);
    fastNode->InitAudiobuffer(false);
    ASSERT_GT(fastNode->srcAudioBuffer_->GetSpanCount(), 0);
    SpanInfo *spanInfo = fastNode->srcAudioBuffer_->GetSpanInfoByIndex(0);
    ASSERT_NE(spanInfo, nullptr);
    EXPECT_EQ(spanInfo->spanStatus.load(), SPAN_WRITE_DONE);
    EXPECT_EQ(spanInfo->volumeStart, 1 << 16);
    EXPECT_EQ(spanInfo->volumeEnd, 1 << 16);

    fastNode->curReadPos_ = FAST_SPAN_FRAME;
    EXPECT_EQ(fastNode->TryReadOneSpan(0), false);

    fastNode->curReadPos_ = 0;
    EXPECT_EQ(fastNode->TryReadOneSpan(0), true);
    EXPECT_EQ(fastNode->curReadPos_, FAST_SPAN_FRAME);

    fastNode->curReadPos_ = FAST_TOTAL_FRAME * 3;
    EXPECT_EQ(fastNode->TryReadOneSpan(fastNode->curReadPos_), false);

    fastNode->curReadPos_ = FAST_SPAN_FRAME;
    int64_t wakeUpTime = 0;
    EXPECT_EQ(fastNode->PrepareNextLoop(wakeUpTime), true);
    EXPECT_GT(wakeUpTime, 0);

    wakeUpTime = ClockTime::GetCurNano() + AUDIO_NS_PER_SECOND * 3;
    fastNode->CheckWakeUpTime(wakeUpTime);
    EXPECT_LT(wakeUpTime - ClockTime::GetCurNano(), AUDIO_NS_PER_SECOND);
}

/**
 * @tc.name  : TimingAndVoipBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_015
 * @tc.desc  : Test IsVoipFast OR/AND branches and timing helper fallbacks.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, TimingAndVoipBranches, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    EXPECT_EQ(fastNode->IsVoipFast(), false);
    EXPECT_EQ(CreateFastNode(AUDIO_INPUT_FLAG_VOIP_FAST)->IsVoipFast(), true);
    EXPECT_EQ(CreateFastNode(AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP)->IsVoipFast(), true);
    EXPECT_EQ(CreateFastNode(AUDIO_INPUT_FLAG_FAST)->IsVoipFast(), false);
    EXPECT_EQ(CreateFastNode(AUDIO_INPUT_FLAG_VOIP)->IsVoipFast(), false);

    int64_t wakeUpTime = 0;
    EXPECT_EQ(fastNode->PrepareNextLoop(wakeUpTime), false);

    fastNode->spanDuration_ = HpaeFastSourceInputNode::ONE_MILLISECOND_DURATION_NS / 2;
    EXPECT_EQ(fastNode->GetPredictNextWriteTime(0), 0);
    fastNode->spanDuration_ = HpaeFastSourceInputNode::ONE_MILLISECOND_DURATION_NS * 2;
    EXPECT_EQ(fastNode->GetPredictNextWriteTime(0), 0);

    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->curReadPos_ = 0;
    int64_t beforePrepare = ClockTime::GetCurNano();
    fastNode->writeTimeModel_.ResetFrameStamp(0, beforePrepare - AUDIO_NS_PER_SECOND);
    EXPECT_EQ(fastNode->PrepareNextLoop(wakeUpTime), true);
    EXPECT_GE(wakeUpTime, beforePrepare);
    EXPECT_LT(wakeUpTime - beforePrepare, AUDIO_NS_PER_SECOND);

    fastNode = CreateFastNode(AUDIO_INPUT_FLAG_VOIP_FAST);
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    fastNode->curReadPos_ = 0;
    EXPECT_EQ(fastNode->PrepareNextLoop(wakeUpTime), true);
    EXPECT_GT(wakeUpTime, 0);

    fastNode->curReadPos_ = 1;
    EXPECT_EQ(fastNode->PrepareNextLoop(wakeUpTime), false);
}

/**
 * @tc.name  : DeviceHandleInfoBranches
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_009
 * @tc.desc  : Test mmap handle position failure and success paths.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, DeviceHandleInfoBranches, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    EXPECT_EQ(fastNode->GetDeviceHandleInfo(frames, nanoTime), false);

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(0), SetArgReferee<1>(1), SetArgReferee<2>(2), Return(SUCCESS)));
    EXPECT_EQ(fastNode->GetDeviceHandleInfo(frames, nanoTime), true);
    EXPECT_EQ(nanoTime, AUDIO_NS_PER_SECOND + 2);

    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _)).WillOnce(Return(ERROR));
    EXPECT_EQ(fastNode->GetDeviceHandleInfo(frames, nanoTime), false);

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(FAST_SPAN_FRAME), SetArgReferee<1>(2),
            SetArgReferee<2>(3), Return(SUCCESS)));
    EXPECT_EQ(fastNode->GetDeviceHandleInfo(frames, nanoTime), true);
    EXPECT_EQ(frames, FAST_SPAN_FRAME);
    EXPECT_EQ(nanoTime, 2 * AUDIO_NS_PER_SECOND + 3);
}

/**
 * @tc.name  : ResetReadPositionAndSyncGuards
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_016
 * @tc.desc  : Test reset read position guards and sync read frame guard branches.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, ResetReadPositionAndSyncGuards, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->ResetReadPosition();
    fastNode->RecordCheckSyncInfo(0);
    fastNode->srcSpanSizeInframe_ = 0;
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->RecordCheckSyncInfo(0);

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(0), SetArgReferee<1>(0), SetArgReferee<2>(0), Return(SUCCESS)));
    fastNode->ResetReadPosition();
    EXPECT_EQ(fastNode->curReadPos_, 0);
    fastNode->RecordCheckSyncInfo(FAST_SPAN_FRAME);
    EXPECT_EQ(fastNode->TryReadOneSpan(FAST_SPAN_FRAME), true);

    fastNode->srcAudioBuffer_ = nullptr;
    EXPECT_EQ(fastNode->TryReadOneSpan(FAST_SPAN_FRAME), false);
}

/**
 * @tc.name  : UpdateAppsAndNotify
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_010
 * @tc.desc  : Test apps uid update and stream change notification forwarding.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, UpdateAppsAndNotify, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    std::vector<int32_t> appsUid = {1, 2};
    std::vector<int32_t> sessionsId = {3, 4};
    fastNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);
    fastNode->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, FAST_SESSION_ID, SOURCE_TYPE_MIC, CAPTURER_RUNNING);

    auto guardNode = CreateFastNode();
    auto guardSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(guardNode, guardSource);
    ON_CALL(*guardSource, IsInited()).WillByDefault(Return(false));
    EXPECT_CALL(*guardSource, UpdateAppsUid(_)).Times(0);
    guardNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);
    guardNode->audioCapturerSource_ = nullptr;

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    EXPECT_CALL(*mockSource, UpdateAppsUid(appsUid)).WillOnce(Return(SUCCESS));
    fastNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);

    EXPECT_CALL(*mockSource, NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, FAST_SESSION_ID, SOURCE_TYPE_MIC,
        CAPTURER_RUNNING, FAST_TEST_UID, true, false)).Times(1);
    fastNode->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, FAST_SESSION_ID, SOURCE_TYPE_MIC,
        CAPTURER_RUNNING, FAST_TEST_UID, true);
}

/**
 * @tc.name  : DoProcessGuards
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_011
 * @tc.desc  : Test early returns for missing source, missing buffer and non-running state.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, DoProcessGuards, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->DoProcess();
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_NEW);

    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->DoProcess();
    EXPECT_EQ(fastNode->srcAudioBuffer_, nullptr);

    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->SetSourceState(STREAM_MANAGER_IDLE);
    fastNode->DoProcess();
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : DoProcessOneSpan
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_017
 * @tc.desc  : Test one-span processing path including resync true branch.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, DoProcessOneSpan, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    fastNode->InitAudiobuffer(true);
    fastNode->SetSourceState(STREAM_MANAGER_RUNNING);
    fastNode->needReSyncPosition_ = true;

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(FAST_SPAN_FRAME), SetArgReferee<1>(0), SetArgReferee<2>(0),
        Return(SUCCESS)));
    fastNode->DoProcess();
    EXPECT_EQ(fastNode->needReSyncPosition_, false);
}

// ==================== GetPredictNextWriteTime Tests ====================

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_StartPeriodNotify, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    uint64_t posInFrame = FAST_SPAN_FRAME * 10;
    int64_t result = fastNode->GetPredictNextWriteTime(posInFrame);
    EXPECT_NE(result, 0);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_BigPeriodNotify, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    uint64_t posInFrame = FAST_SPAN_FRAME * 40;
    int64_t result = fastNode->GetPredictNextWriteTime(posInFrame);
    EXPECT_NE(result, 0);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_NoNotify, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    uint64_t posInFrame = FAST_SPAN_FRAME * 21;
    int64_t result = fastNode->GetPredictNextWriteTime(posInFrame);
    EXPECT_NE(result, 0);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_FrameStampNotConfiged, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    int64_t result = fastNode->GetPredictNextWriteTime(FAST_SPAN_FRAME);
    EXPECT_NE(result, -1);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_WriteFrameEqualsPos, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->posInFrame_ = FAST_SPAN_FRAME * 10;
    fastNode->timeInNano_ = 1666667;
    fastNode->writeTimeModel_.ResetFrameStamp(fastNode->posInFrame_.load(), fastNode->timeInNano_.load());
    fastNode->needReSyncPosition_ = false;
    int64_t result = fastNode->GetPredictNextWriteTime(FAST_SPAN_FRAME * 10);
    EXPECT_NE(result, 0);
    EXPECT_EQ(fastNode->needReSyncPosition_, false);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_UpdataCheckSuccess, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->posInFrame_ = 480;
    fastNode->timeInNano_ = 10000000;
    fastNode->needReSyncPosition_ = false;
    int64_t result = fastNode->GetPredictNextWriteTime(FAST_SPAN_FRAME * 10);
    EXPECT_NE(result, 0);
    EXPECT_EQ(fastNode->needReSyncPosition_, false);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_UpdataCheckFailed, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->posInFrame_ = 480;
    fastNode->timeInNano_ = 100000000000LL;
    int64_t result = fastNode->GetPredictNextWriteTime(FAST_SPAN_FRAME);
    EXPECT_NE(result, 0);
}

HWTEST_F(HpaeFastSourceInputNodeTest, GetPredictNextWriteTime_UpdataNeedModify, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    int64_t baseTime = 1000000000LL;
    int64_t frameDuration = static_cast<int64_t>(480) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    for (int i = 1; i <= 4; i++) {
        fastNode->writeTimeModel_.UpdateFrameStamp(i * 480, baseTime + i * frameDuration);
    }
    fastNode->posInFrame_ = 5 * 480;
    fastNode->timeInNano_ = baseTime + 5 * frameDuration;
    fastNode->needReSyncPosition_ = false;
    int64_t result = fastNode->GetPredictNextWriteTime(FAST_SPAN_FRAME * 10);
    EXPECT_EQ(fastNode->needReSyncPosition_, true);
    EXPECT_NE(result, 0);
}

// ==================== PrepareNextLoop WakeUpTime Abnormal Tests ====================

HWTEST_F(HpaeFastSourceInputNodeTest, PrepareNextLoop_WakeUpTimeAbnormal, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->curReadPos_ = 0;
    fastNode->writeTimeModel_.ConfigSampleRate(SAMPLE_RATE_48000);
    fastNode->writeTimeModel_.ResetFrameStamp(0, 0);
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    fastNode->posInFrame_ = 0;
    fastNode->timeInNano_ = ClockTime::GetCurNano() - 100000000000LL;
    int64_t wakeUpTime = 0;
    EXPECT_EQ(fastNode->PrepareNextLoop(wakeUpTime), true);
    EXPECT_GT(wakeUpTime, ClockTime::GetCurNano());
}

// ==================== AsyncGetPosTime Branch Tests ====================

HWTEST_F(HpaeFastSourceInputNodeTest, AsyncGetPosTime_GetDeviceHandleInfo, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    fastNode->audioCapturerSource_ = mockSource;
    fastNode->captureId_ = FAST_CAPTURE_ID;
    fastNode->stopUpdateThread_.store(false);
    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(480), SetArgReferee<1>(0), SetArgReferee<2>(20000000), Return(SUCCESS)));
    fastNode->AsyncGetPosTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(fastNode->posInFrame_, 0);
    fastNode->StopUpdateThread();
}

HWTEST_F(HpaeFastSourceInputNodeTest, AsyncGetPosTime_StopThreadBreak, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    fastNode->audioCapturerSource_ = mockSource;
    fastNode->captureId_ = FAST_CAPTURE_ID;
    fastNode->stopUpdateThread_.store(false);
    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(480), SetArgReferee<1>(0), SetArgReferee<2>(20000000), Return(SUCCESS)));
    fastNode->AsyncGetPosTime();
    fastNode->StopUpdateThread();
    EXPECT_EQ(fastNode->stopUpdateThread_.load(), true);
    EXPECT_EQ(fastNode->updatePosTimeThread_.joinable(), false);
}

// ==================== Branch Coverage Supplement Tests ====================

/**
 * @tc.name  : DoProcessNormalLoop
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_018
 * @tc.desc  : Test DoProcess needReSyncPosition_ false branch and TryReadOneSpan success exit path.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, DoProcessNormalLoop, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    fastNode->InitAudiobuffer(true);
    fastNode->SetSourceState(STREAM_MANAGER_RUNNING);
    fastNode->needReSyncPosition_ = false;
    fastNode->curReadPos_ = 0;

    fastNode->DoProcess();
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastNode->curReadPos_, FAST_SPAN_FRAME);
    EXPECT_EQ(fastNode->needReSyncPosition_, false);
}

/**
 * @tc.name  : DoProcessReSyncPositionFail
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_019
 * @tc.desc  : Test DoProcess needReSyncPosition_ true branch with ResetReadPosition failure early return.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, DoProcessReSyncPositionFail, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    fastNode->InitAudiobuffer(true);
    fastNode->SetSourceState(STREAM_MANAGER_RUNNING);
    fastNode->needReSyncPosition_ = true;

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(0), SetArgReferee<1>(0), SetArgReferee<2>(0), Return(ERROR)));
    fastNode->DoProcess();
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastNode->needReSyncPosition_, true);
    EXPECT_EQ(fastNode->curReadPos_, 0);
}

/**
 * @tc.name  : DoProcessLoopBodySleep
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_020
 * @tc.desc  : Test DoProcess while-loop body: TryReadOneSpan false then PrepareNextLoop sleep then retry success.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, DoProcessLoopBodySleep, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->srcByteSizePerFrame_ = FAST_BYTE_PER_FRAME;
    fastNode->spanDuration_ = static_cast<int64_t>(FAST_SPAN_FRAME) * AUDIO_NS_PER_SECOND / SAMPLE_RATE_48000;
    fastNode->InitAudiobuffer(true);
    fastNode->SetSourceState(STREAM_MANAGER_RUNNING);
    fastNode->needReSyncPosition_ = false;
    fastNode->curReadPos_ = FAST_SPAN_FRAME;

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(0), SetArgReferee<1>(0), SetArgReferee<2>(0), Return(SUCCESS)));
    fastNode->DoProcess();
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastNode->curReadPos_, FAST_SPAN_FRAME * 2);
}

/**
 * @tc.name  : CapturerSourceStopSuccess
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_021
 * @tc.desc  : Test CapturerSourceStop when Stop() returns SUCCESS (the != SUCCESS false branch).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, CapturerSourceStopSuccess, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    ON_CALL(*mockSource, IsInited()).WillByDefault(Return(true));
    EXPECT_CALL(*mockSource, Stop()).WillOnce(Return(SUCCESS));

    EXPECT_EQ(fastNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(fastNode->GetSourceState(), STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(fastNode->needReSyncPosition_, true);
}

/**
 * @tc.name  : GetCapturerSourceInstanceMicRef
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_022
 * @tc.desc  : Test GetCapturerSourceInstance sourceName == HDI_ID_INFO_MIC_REF OR sub-branch.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, GetCapturerSourceInstanceMicRef, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    int32_t ret = fastNode->GetCapturerSourceInstance("invalid_device", "net_id", SOURCE_TYPE_MIC,
        HDI_ID_INFO_MIC_REF);
    EXPECT_NE(ret, SUCCESS);
    EXPECT_EQ(fastNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(fastNode->captureId_, HDI_INVALID_ID);
}

/**
 * @tc.name  : GetCapturerSourceInstanceEmptyNetId
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_023
 * @tc.desc  : Test GetCapturerSourceInstance else branch with empty deviceNetId (info.empty() true path).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, GetCapturerSourceInstanceEmptyNetId, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    int32_t ret = fastNode->GetCapturerSourceInstance("invalid_device", "", SOURCE_TYPE_MIC, "mic");
    EXPECT_NE(ret, SUCCESS);
    EXPECT_EQ(fastNode->audioCapturerSource_, nullptr);
    EXPECT_EQ(fastNode->captureId_, HDI_INVALID_ID);
}

/**
 * @tc.name  : NotifyStreamChangeToSourceNullSource
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_024
 * @tc.desc  : Test NotifyStreamChangeToSource null source guard (delegation skipped).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, NotifyStreamChangeToSourceNullSource, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    EXPECT_CALL(*mockSource, NotifyStreamChangeToSource(_, _, _, _, _, _, _)).Times(0);

    fastNode->audioCapturerSource_ = nullptr;
    fastNode->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, FAST_SESSION_ID, SOURCE_TYPE_MIC,
        CAPTURER_RUNNING, FAST_TEST_UID, true);
    EXPECT_EQ(fastNode->audioCapturerSource_, nullptr);
    fastNode->audioCapturerSource_ = mockSource;
}

/**
 * @tc.name  : InitAudiobufferNullBuffer
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_025
 * @tc.desc  : Test InitAudiobuffer srcAudioBuffer_ nullptr guard (early return).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, InitAudiobufferNullBuffer, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    fastNode->srcAudioBuffer_ = nullptr;
    fastNode->InitAudiobuffer(true);
    fastNode->InitAudiobuffer(false);
    EXPECT_EQ(fastNode->srcAudioBuffer_, nullptr);
}

/**
 * @tc.name  : AsyncGetPosTimeAlreadyJoinable
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_026
 * @tc.desc  : Test AsyncGetPosTime when thread already joinable (guard returns early).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, AsyncGetPosTimeAlreadyJoinable, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->stopUpdateThread_.store(false);
    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(0), SetArgReferee<1>(0), SetArgReferee<2>(0), Return(SUCCESS)));
    fastNode->AsyncGetPosTime();
    ASSERT_EQ(fastNode->updatePosTimeThread_.joinable(), true);
    fastNode->AsyncGetPosTime();
    EXPECT_EQ(fastNode->updatePosTimeThread_.joinable(), true);
    fastNode->StopUpdateThread();
    EXPECT_EQ(fastNode->updatePosTimeThread_.joinable(), false);
}

/**
 * @tc.name  : ResetReadPositionGetDeviceHandleInfoFail
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_027
 * @tc.desc  : Test ResetReadPosition when GetDeviceHandleInfo fails (returns false).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, ResetReadPositionGetDeviceHandleInfoFail, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    fastNode->srcAudioBuffer_ =
        OHAudioBuffer::CreateFromLocal(FAST_TOTAL_FRAME, FAST_SPAN_FRAME, FAST_BYTE_PER_FRAME);
    ASSERT_NE(fastNode->srcAudioBuffer_, nullptr);
    fastNode->srcSpanSizeInframe_ = FAST_SPAN_FRAME;
    fastNode->curReadPos_ = 0;

    EXPECT_CALL(*mockSource, GetMmapHandlePosition(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(0), SetArgReferee<1>(0), SetArgReferee<2>(0), Return(ERROR)));
    EXPECT_EQ(fastNode->ResetReadPosition(), false);
    EXPECT_EQ(fastNode->curReadPos_, 0);
}

/**
 * @tc.name  : GetDfxStreamInfoReturnsNodeInfo
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_028
 * @tc.desc  : Test GetDfxStreamInfo returns fields sourced from node info.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, GetDfxStreamInfoReturnsNodeInfo, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = CreateFastNodeInfo();
    auto fastNode = std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
    ASSERT_NE(fastNode, nullptr);
    AudioStreamInfo streamInfo = fastNode->GetDfxStreamInfo();
    EXPECT_EQ(streamInfo.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(streamInfo.format, nodeInfo.format);
    EXPECT_EQ(streamInfo.channels, nodeInfo.channels);
    EXPECT_EQ(streamInfo.channelLayout, nodeInfo.channelLayout);
}

/**
 * @tc.name  : InitDumpFileSetsDumpName
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_029
 * @tc.desc  : Test InitDumpFile builds dumpHdiName_ from source attr fields.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, InitDumpFileSetsDumpName, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    IAudioSourceAttr attr;
    InitAudioSourceAttr(attr);
    fastNode->audioSourceAttr_ = attr;

    fastNode->InitDumpFile();
    EXPECT_NE(fastNode->dumpHdiName_.find("hpae_fast_source_hdi_audio_"), std::string::npos);
    EXPECT_NE(fastNode->dumpHdiName_.find(std::to_string(attr.sampleRate)), std::string::npos);
    EXPECT_NE(fastNode->dumpHdiName_.find(std::to_string(attr.channel)), std::string::npos);
    EXPECT_NE(fastNode->dumpHdiName_.find(std::to_string(attr.format)), std::string::npos);
    EXPECT_NE(fastNode->dumpHdiName_.find(".pcm"), std::string::npos);
}

/**
 * @tc.name  : CheckRecordSignalLatencyPaths
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_030
 * @tc.desc  : Test CheckRecordSignal latency-disabled early return and null-agent guard paths.
 */
HWTEST_F(HpaeFastSourceInputNodeTest, CheckRecordSignalLatencyPaths, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    std::vector<uint8_t> buffer(FAST_BYTE_PER_FRAME * FAST_SPAN_FRAME, 0);

    fastNode->latencyMeasEnabled_ = false;
    fastNode->CheckRecordSignal(buffer.data(), buffer.size());
    EXPECT_EQ(fastNode->signalDetected_, false);

    fastNode->latencyMeasEnabled_ = true;
    fastNode->signalDetectAgent_ = nullptr;
    fastNode->CheckRecordSignal(buffer.data(), buffer.size());
    EXPECT_EQ(fastNode->signalDetected_, false);
    EXPECT_EQ(fastNode->latencyMeasEnabled_, true);
}

/**
 * @tc.name  : GetAdapterBufferInfoAllZeroFields
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSourceInputNodeTest_031
 * @tc.desc  : Test GetAdapterBufferInfo with all mmap fields zero (combined condition failure).
 */
HWTEST_F(HpaeFastSourceInputNodeTest, GetAdapterBufferInfoAllZeroFields, TestSize.Level1)
{
    auto fastNode = CreateFastNode();
    auto mockSource = std::make_shared<NiceMock<MockAudioCaptureSource>>();
    AttachMockSource(fastNode, mockSource);
    EXPECT_CALL(*mockSource, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(-1), SetArgReferee<1>(0), SetArgReferee<2>(0),
            SetArgReferee<3>(0), SetArgReferee<4>(0), Return(SUCCESS)));
    EXPECT_EQ(fastNode->GetAdapterBufferInfo(), ERR_ILLEGAL_STATE);
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
