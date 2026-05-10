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

class HpaeOffloadCallbackInfo : public IOffloadCallback {
public:
    void OnNotifyHdiData(const std::pair<uint64_t, TimePoint> &hdiPos) override {}
    ~HpaeOffloadCallbackInfo() override {}
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
    offloadNode_->SetBufferSize(); // setbuffersize to realy set unlock_

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
    offloadNode_->SetSpeed(1.0f);
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

HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetPolicyState_TaskExsist_StateForeground, TestSize.Level0)
{
    offloadNode_->setPolicyStateTask_.flag = true;
    offloadNode_->hdiPolicyState_ = OFFLOAD_INACTIVE_BACKGROUND;
    offloadNode_->SetPolicyState(0);
    EXPECT_EQ(offloadNode_->hdiPolicyState_, OFFLOAD_ACTIVE_FOREGROUND);
    EXPECT_FALSE(offloadNode_->setPolicyStateTask_.flag);
}

HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetPolicyState_TaskExsist_StateBackground, TestSize.Level0)
{
    offloadNode_->setPolicyStateTask_.flag = true;
    offloadNode_->SetPolicyState(3);
    EXPECT_EQ(offloadNode_->hdiPolicyState_, OFFLOAD_INACTIVE_BACKGROUND);
    EXPECT_TRUE(offloadNode_->setPolicyStateTask_.flag);
}

HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetPolicyState_TaskNotExsist_StateForeground, TestSize.Level0)
{
    offloadNode_->hdiPolicyState_ = OFFLOAD_INACTIVE_BACKGROUND;
    offloadNode_->SetPolicyState(0);
    EXPECT_EQ(offloadNode_->hdiPolicyState_, OFFLOAD_ACTIVE_FOREGROUND);
    EXPECT_FALSE(offloadNode_->setPolicyStateTask_.flag);
    offloadNode_->NotifyHdiPos();
    std::shared_ptr<IOffloadCallback> callback = std::make_shared<HpaeOffloadCallbackInfo>();
    offloadNode_->offloadCallback_ = callback.get();
    offloadNode_->NotifyHdiPos();
    EXPECT_NE(offloadNode_->offloadCallback_, nullptr);
}

HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetPolicyState_TaskNotExsist_StateBackground, TestSize.Level0)
{
    offloadNode_->hdiPolicyState_ = OFFLOAD_ACTIVE_FOREGROUND;
    offloadNode_->SetPolicyState(3);
    EXPECT_EQ(offloadNode_->hdiPolicyState_, OFFLOAD_INACTIVE_BACKGROUND);
    EXPECT_TRUE(offloadNode_->setPolicyStateTask_.flag);
}

// Test SetBufferSize with small buffer and running state should lock
HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetBufferSize_SmallBufferRunning_ShouldLock, TestSize.Level0)
{
    // Set running state and foreground (small buffer 200ms)
    offloadNode_->state_ = STREAM_MANAGER_RUNNING;
    offloadNode_->hdiPolicyState_ = OFFLOAD_ACTIVE_FOREGROUND;

    // Expect lock method called once, SetBufferSize called once
    EXPECT_CALL(*mockSink_, LockOffloadRunningLock()).Times(1);
    EXPECT_CALL(*mockSink_, SetBufferSize(_)).Times(1);

    offloadNode_->SetBufferSize();
}

// Test SetBufferSize with small buffer but not running state should not lock
HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetBufferSize_SmallBufferNotRunning_ShouldNotLock, TestSize.Level0)
{
    // Set IDLE state (not running)
    offloadNode_->state_ = STREAM_MANAGER_IDLE;
    offloadNode_->hdiPolicyState_ = OFFLOAD_ACTIVE_FOREGROUND;

    // Expect lock method NOT called, only SetBufferSize
    EXPECT_CALL(*mockSink_, LockOffloadRunningLock()).Times(0);
    EXPECT_CALL(*mockSink_, SetBufferSize(_)).Times(1);

    offloadNode_->SetBufferSize();
}

// Test SetBufferSize with large buffer (background) and running state should not lock
HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetBufferSize_LargeBufferRunning_ShouldNotLock, TestSize.Level0)
{
    // Set running state but BACKGROUND (large buffer 7000ms)
    offloadNode_->state_ = STREAM_MANAGER_RUNNING;
    offloadNode_->hdiPolicyState_ = OFFLOAD_INACTIVE_BACKGROUND;

    // Expect lock method NOT called (buffer is large)
    EXPECT_CALL(*mockSink_, LockOffloadRunningLock()).Times(0);
    EXPECT_CALL(*mockSink_, SetBufferSize(_)).Times(1);

    offloadNode_->SetBufferSize();
}

// Test SetBufferSize with MOVIE stream type (500ms buffer) should not lock
HWTEST_F(HpaeOffloadSinkOutputNodeTest, SetBufferSize_MovieStream_ShouldNotLock, TestSize.Level0)
{
    // Set running state with MOVIE stream (500ms buffer, not small)
    offloadNode_->state_ = STREAM_MANAGER_RUNNING;
    offloadNode_->nodeInfo_.streamType = STREAM_MOVIE;

    // Expect lock method NOT called (buffer is 500ms > 200ms)
    EXPECT_CALL(*mockSink_, LockOffloadRunningLock()).Times(0);
    EXPECT_CALL(*mockSink_, SetBufferSize(_)).Times(1);

    offloadNode_->SetBufferSize();
}

/**
 * @tc.name  : FaultCode_RenderSinkInit_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkInit returns ERR_ILLEGAL_STATE when audioRendererSink_ is nullptr,
 *             covering PLAY_CREATE_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeOffloadSinkOutputNodeTest, FaultCode_RenderSinkInit_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    IAudioSinkAttr attr;
    attr.adapterName = "offload";
    int32_t result = node->RenderSinkInit(attr);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : FaultCode_RenderSinkDeInit_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkDeInit returns ERR_ILLEGAL_STATE when audioRendererSink_ is nullptr,
 *             covering PLAY_RELEASE_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeOffloadSinkOutputNodeTest, FaultCode_RenderSinkDeInit_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = node->RenderSinkDeInit();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : FaultCode_RenderSinkStart_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStart returns ERR_ILLEGAL_STATE when audioRendererSink_ is nullptr,
 *             covering PLAY_START_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeOffloadSinkOutputNodeTest, FaultCode_RenderSinkStart_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = node->RenderSinkStart();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : FaultCode_RenderSinkFlush_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkFlush returns ERR_ILLEGAL_STATE when audioRendererSink_ is nullptr,
 *             covering PLAY_FLUSH_INSTANCE_NULL fault code path.
 */
HWTEST_F(HpaeOffloadSinkOutputNodeTest, FaultCode_RenderSinkFlush_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    int32_t result = node->RenderSinkFlush();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

// FaultCode test constants
static constexpr int32_t TEST_APP_UID = 1001;

/**
 * @tc.name  : FaultCode_UpdateAppsUid_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAppsUid returns ERROR when audioRendererSink_ is nullptr,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeOffloadSinkOutputNodeTest, FaultCode_UpdateAppsUid_NullSink, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);

    // audioRendererSink_ is nullptr by default
    std::vector<int32_t> appsUid = {TEST_APP_UID};
    int32_t result = node->UpdateAppsUid(appsUid);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_UpdateAppsUid_SinkNotInited
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAppsUid returns ERR_ILLEGAL_STATE when sink is not initialized,
 *             covering PLAY_SEND_STATE_ILLEGAL fault code path.
 */
HWTEST_F(HpaeOffloadSinkOutputNodeTest, FaultCode_UpdateAppsUid_SinkNotInited, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    node->audioRendererSink_ = mockSink;

    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(false));

    std::vector<int32_t> appsUid = {TEST_APP_UID};
    int32_t result = node->UpdateAppsUid(appsUid);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS