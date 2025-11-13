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

#include "hpae_offload_sinkoutput_node.h"
#include "hpae_mocks.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;
using ::testing::_;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int32_t OFFLOAD_FULL = -1;
constexpr int32_t OFFLOAD_WRITE_FAILED = -2;
constexpr size_t DATA_SIZE = 1024;
constexpr uint32_t OFFLOAD_SET_BUFFER_SIZE_NUM = 5;
class HpaeOffloadSinkOutputNodeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<HpaeOffloadSinkOutputNode> offloadNode_;
    std::shared_ptr<MockAudioRenderSink> mockSink_;
};

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    size_t frameLen = 960;
    uint32_t nodeId = 1243;
    nodeInfo.nodeId = nodeId;
    nodeInfo.frameLen = frameLen;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
}

void HpaeOffloadSinkOutputNodeTest::SetUp()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    offloadNode_ = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    mockSink_ = std::make_shared<NiceMock<MockAudioRenderSink>>();
    offloadNode_->audioRendererSink_ = mockSink_;
    ::testing::DefaultValue<int32_t>::Set(0);
}

void HpaeOffloadSinkOutputNodeTest::TearDown()
{
    offloadNode_ = nullptr;
    mockSink_ = nullptr;
    ::testing::DefaultValue<int32_t>::Clear();
}

// Test OFFLOAD_FULL with background inactive condition
HWTEST_F(HpaeOffloadSinkOutputNodeTest, OffloadNeedSleep_FullInBackground_ShouldUnlock, TestSize.Level0)
{
    // Set background inactive state
    offloadNode_->hdiPolicyState_ = OFFLOAD_INACTIVE_BACKGROUND;

    // Expect unlock method called
    EXPECT_CALL(*mockSink_, UnLockOffloadRunningLock()).Times(1);
    offloadNode_->OffloadNeedSleep(OFFLOAD_FULL);
    // Verify state changes
    EXPECT_TRUE(offloadNode_->isHdiFull_.load());
}

// Test OFFLOAD_FULL with movie stream type
HWTEST_F(HpaeOffloadSinkOutputNodeTest, OffloadNeedSleep_FullMovieStream_ShouldUnlock, TestSize.Level0)
{
    // Set stream type to movie
    offloadNode_->nodeInfo_.streamType = STREAM_MOVIE;

    // Expect unlock method called
    EXPECT_CALL(*mockSink_, UnLockOffloadRunningLock()).Times(1);
    offloadNode_->OffloadNeedSleep(OFFLOAD_FULL);
    // Verify state changes
    EXPECT_TRUE(offloadNode_->isHdiFull_.load());
}

// Test OFFLOAD_FULL without matching conditions
HWTEST_F(HpaeOffloadSinkOutputNodeTest, OffloadNeedSleep_FullNoCondition_ShouldNotUnlock, TestSize.Level0)
{
    // Set non-matching conditions
    offloadNode_->hdiPolicyState_ = OFFLOAD_ACTIVE_FOREGROUND;

    // Unlock method should not be called
    EXPECT_CALL(*mockSink_, UnLockOffloadRunningLock()).Times(0);
    offloadNode_->OffloadNeedSleep(OFFLOAD_FULL);
    // Verify state changes
    EXPECT_TRUE(offloadNode_->isHdiFull_.load());
}

// Test error type with retry count below max
HWTEST_F(HpaeOffloadSinkOutputNodeTest, OffloadNeedSleep_ErrorBelowMaxRetry_ShouldIncreaseRetry, TestSize.Level0)
{
    // Set initial retry count
    offloadNode_->backoffController_.delay_ = 1;

    // Unlock method should not be called
    EXPECT_CALL(*mockSink_, UnLockOffloadRunningLock()).Times(0);
    offloadNode_->OffloadNeedSleep(OFFLOAD_WRITE_FAILED);
    // Verify retry count increased
    EXPECT_EQ(offloadNode_->backoffController_.delay_, 2);
    EXPECT_FALSE(offloadNode_->isHdiFull_.load());
}

// Test error type with retry count at max
HWTEST_F(HpaeOffloadSinkOutputNodeTest, OffloadNeedSleep_ErrorAtMaxRetry_ShouldNotIncrease, TestSize.Level0)
{
    // Set retry count to max value
    offloadNode_->backoffController_.delay_ = 20; // 20ms limit

    // Unlock method should not be called
    EXPECT_CALL(*mockSink_, UnLockOffloadRunningLock()).Times(0);

    offloadNode_->OffloadNeedSleep(OFFLOAD_WRITE_FAILED);

    // Verify retry count unchanged
    EXPECT_EQ(offloadNode_->backoffController_.delay_, 20); // 20ms limit
    EXPECT_FALSE(offloadNode_->isHdiFull_.load());
}

// Test SUCCESS type resets retry count
HWTEST_F(HpaeOffloadSinkOutputNodeTest, OffloadNeedSleep_Success_ShouldResetRetry, TestSize.Level0)
{
    // Set initial retry count
    offloadNode_->backoffController_.delay_ = 5;

    // Unlock method should not be called
    EXPECT_CALL(*mockSink_, UnLockOffloadRunningLock()).Times(0);

    offloadNode_->OffloadNeedSleep(SUCCESS);
    // Verify retry count reset
    EXPECT_EQ(offloadNode_->backoffController_.delay_, 0);
    EXPECT_FALSE(offloadNode_->isHdiFull_.load());
}

// Test empty data returns failure
HWTEST_F(HpaeOffloadSinkOutputNodeTest, ProcessRenderFrame_EmptyData_ReturnsFailure, TestSize.Level0)
{
    // Set empty data
    offloadNode_->renderFrameData_.clear();
    
    // Execute function
    int32_t result = offloadNode_->ProcessRenderFrame();
    // Verify failure returned
    EXPECT_EQ(result, OFFLOAD_WRITE_FAILED);
}

// Test RenderFrame returns success but writes 0 bytes (non-first write)
HWTEST_F(HpaeOffloadSinkOutputNodeTest, ProcessRenderFrame_WriteZero_ReturnsOffloadFull, TestSize.Level0)
{
    // Set non-empty data
    offloadNode_->renderFrameData_ = std::vector<char>(1024, 0);
    offloadNode_->firstWriteHdi_ = false; // Not first write
    
    // Mock RenderFrame: success but 0 bytes written
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _))
        .WillOnce([](char &data, size_t size, uint64_t &written) {
            written = 0;
            return SUCCESS;
        });
    // Execute function
    int32_t result = offloadNode_->ProcessRenderFrame();
    // Verify OFFLOAD_FULL returned
    EXPECT_EQ(result, OFFLOAD_FULL);
}

// Test RenderFrame failure returns OFFLOAD_WRITE_FAILED
HWTEST_F(HpaeOffloadSinkOutputNodeTest, ProcessRenderFrame_RenderFailure_ReturnsFailure, TestSize.Level0)
{
    // Set non-empty data
    offloadNode_->renderFrameData_ = std::vector<char>(1024, 0);

    // Mock RenderFrame failure
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _))
        .WillOnce(Return(OFFLOAD_WRITE_FAILED));
    // Execute function
    int32_t result = offloadNode_->ProcessRenderFrame();
    // Verify failure returned
    EXPECT_EQ(result, OFFLOAD_WRITE_FAILED);
}

// Test partial write returns failure
HWTEST_F(HpaeOffloadSinkOutputNodeTest, ProcessRenderFrame_PartialWrite_ReturnsFailure, TestSize.Level0)
{
    // Set non-empty data
    offloadNode_->renderFrameData_ = std::vector<char>(DATA_SIZE, 0);
    
    // Mock partial write
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _))
        .WillOnce([](char &data, size_t size, uint64_t &written) {
            written = DATA_SIZE / 2; // Half data written
            return SUCCESS;
        });
    // Execute function
    int32_t result = offloadNode_->ProcessRenderFrame();
    // Verify failure returned
    EXPECT_EQ(result, OFFLOAD_WRITE_FAILED);
}

// Test first successful write initializes state
HWTEST_F(HpaeOffloadSinkOutputNodeTest, ProcessRenderFrame_FirstWrite_InitializesState, TestSize.Level0)
{
    // Set non-empty data and first write flag
    offloadNode_->renderFrameData_ = std::vector<char>(DATA_SIZE, 0);
    offloadNode_->firstWriteHdi_ = true; // First write
    offloadNode_->writePos_ = 0;
    
    // Mock successful write
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _))
        .WillOnce([](char &data, size_t size, uint64_t &written) {
            written = DATA_SIZE;
            return SUCCESS;
        });
    // Execute function
    int32_t result = offloadNode_->ProcessRenderFrame();
    // Verify success returned
    EXPECT_EQ(result, SUCCESS);
    // Verify first write state initialized
    EXPECT_FALSE(offloadNode_->firstWriteHdi_);
    EXPECT_NE(offloadNode_->hdiPos_.second.time_since_epoch().count(), 0);
    EXPECT_EQ(offloadNode_->setHdiBufferSizeNum_, OFFLOAD_SET_BUFFER_SIZE_NUM - 1);
    EXPECT_GT(offloadNode_->writePos_, 0);
}

// Test subsequent successful write updates state
HWTEST_F(HpaeOffloadSinkOutputNodeTest, ProcessRenderFrame_SubsequentWrite_UpdatesState, TestSize.Level0)
{
    // Set non-empty data and non-first write
    offloadNode_->renderFrameData_ = std::vector<char>(DATA_SIZE, 0);
    offloadNode_->firstWriteHdi_ = false;
    offloadNode_->writePos_ = 1000; // Initial write position
    
    // Mock successful write
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _))
        .WillOnce([](char &data, size_t size, uint64_t &written) {
            written = DATA_SIZE;
            return SUCCESS;
        });
    // Execute function
    int32_t result = offloadNode_->ProcessRenderFrame();
    // Verify success returned
    EXPECT_EQ(result, SUCCESS);
    // Verify state updated
    EXPECT_GT(offloadNode_->writePos_, 1000); // Write position increased
    EXPECT_TRUE(offloadNode_->renderFrameData_.empty()); // Data cleared
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS