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

#include "oh_audio_buffer.h"
#include "aiut_log.h"
#include "audio_service_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_TOTAL_SIZE_FRAME = 1024;
constexpr uint32_t TEST_BYTE_SIZE_PER_FRAME = 4;
constexpr uint32_t TEST_SPAN_SIZE_FRAME = 256;
constexpr uint32_t TEST_SPAN_COUNT = 4;
constexpr uint64_t TEST_FRAME_POS_100 = 100;
constexpr uint64_t TEST_FRAME_POS_256 = 256;
constexpr uint64_t TEST_FRAME_POS_512 = 512;
constexpr uint64_t TEST_FRAME_POS_INVALID = 3000;
constexpr float TEST_VOLUME_HALF = 0.5f;
constexpr uint32_t TEST_SESSION_ID = 42;
constexpr int64_t TEST_WRITTEN_TIME = 123456789;
constexpr uint32_t TEST_UNDERRUN_COUNT = 5;
}  // namespace

class OHAudioBufferAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

static std::shared_ptr<OHAudioBuffer> CreateTestBuffer()
{
    return OHAudioBuffer::CreateFromLocal(TEST_TOTAL_SIZE_FRAME, TEST_SPAN_SIZE_FRAME,
        TEST_BYTE_SIZE_PER_FRAME);
}

// ==================== CheckWriteOrReadFrame Tests ====================

/**
 * @tc.name  : Test CheckWriteOrReadFrame with valid aligned frame
 * @tc.number: OHAudioBuffer_CheckWriteOrReadFrame_Valid
 * @tc.desc  : Test CheckWriteOrReadFrame returns true with span-aligned frame.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_CheckWriteOrReadFrame_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->CheckWriteOrReadFrame(TEST_FRAME_POS_256);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "CheckWriteOrReadFrame should return true for aligned frame");
    bool ret2 = buffer->CheckWriteOrReadFrame(TEST_FRAME_POS_512);
    AIUT_EXPECT_TRUE_INFO_LOG(ret2, "CheckWriteOrReadFrame should return true for 512");
}

/**
 * @tc.name  : Test CheckWriteOrReadFrame with unaligned frame
 * @tc.number: OHAudioBuffer_CheckWriteOrReadFrame_Unaligned
 * @tc.desc  : Test CheckWriteOrReadFrame returns false with non-span-aligned frame.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_CheckWriteOrReadFrame_Unaligned, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->CheckWriteOrReadFrame(TEST_FRAME_POS_100);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "CheckWriteOrReadFrame should return false for unaligned frame");
}

/**
 * @tc.name  : Test CheckWriteOrReadFrame with zero spanSize
 * @tc.number: OHAudioBuffer_CheckWriteOrReadFrame_ZeroSpan
 * @tc.desc  : Test CheckWriteOrReadFrame returns false when spanSizeInFrame is 0.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_CheckWriteOrReadFrame_ZeroSpan, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->UpdateSpanBasicInfo(0, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    bool ret = buffer->CheckWriteOrReadFrame(TEST_FRAME_POS_256);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "CheckWriteOrReadFrame should return false with zero span");
}

// ==================== GetWriteBuffer / GetReadbuffer Tests ====================

/**
 * @tc.name  : Test GetWriteBuffer with valid position
 * @tc.number: OHAudioBuffer_GetWriteBuffer_Valid
 * @tc.desc  : Test GetWriteBuffer returns SUCCESS with valid span-aligned position.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetWriteBuffer_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    int32_t ret = buffer->GetWriteBuffer(0, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetWriteBuffer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(desc.buffer, nullptr, "desc.buffer should not be null");
}

/**
 * @tc.name  : Test GetWriteBuffer with unaligned position
 * @tc.number: OHAudioBuffer_GetWriteBuffer_Unaligned
 * @tc.desc  : Test GetWriteBuffer returns ERR_INVALID_PARAM with unaligned position.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetWriteBuffer_Unaligned, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    int32_t ret = buffer->GetWriteBuffer(TEST_FRAME_POS_100, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetWriteBuffer unaligned should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetReadbuffer with valid position
 * @tc.number: OHAudioBuffer_GetReadbuffer_Valid
 * @tc.desc  : Test GetReadbuffer returns SUCCESS with valid span-aligned position.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetReadbuffer_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_256, false);
    BufferDesc desc;
    int32_t ret = buffer->GetReadbuffer(0, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetReadbuffer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(desc.buffer, nullptr, "desc.buffer should not be null");
}

/**
 * @tc.name  : Test GetReadbuffer with out of range position
 * @tc.number: OHAudioBuffer_GetReadbuffer_OutOfRange
 * @tc.desc  : Test GetReadbuffer returns ERR_INVALID_PARAM with out of range position.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetReadbuffer_OutOfRange, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    int32_t ret = buffer->GetReadbuffer(TEST_FRAME_POS_INVALID, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetReadbuffer out of range should fail, ret=%d", ret);
}

// ==================== GetSpanInfo Tests ====================

/**
 * @tc.name  : Test GetSpanInfo with valid position
 * @tc.number: OHAudioBuffer_GetSpanInfo_Valid
 * @tc.desc  : Test GetSpanInfo returns non-null with valid position.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanInfo_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    SpanInfo *info = buffer->GetSpanInfo(0);
    AIUT_EXPECT_NE_INFO_LOG(info, nullptr, "GetSpanInfo should return non-null");
    info = buffer->GetSpanInfo(TEST_FRAME_POS_256);
    AIUT_EXPECT_NE_INFO_LOG(info, nullptr, "GetSpanInfo at 256 should return non-null");
}

/**
 * @tc.name  : Test GetSpanInfo with out of range position
 * @tc.number: OHAudioBuffer_GetSpanInfo_OutOfRange
 * @tc.desc  : Test GetSpanInfo returns nullptr with out of range position.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanInfo_OutOfRange, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    SpanInfo *info = buffer->GetSpanInfo(TEST_FRAME_POS_INVALID);
    AIUT_EXPECT_EQ_INFO_LOG(info, nullptr, "GetSpanInfo out of range should return nullptr");
}

/**
 * @tc.name  : Test GetSpanInfo with wrap-around position
 * @tc.number: OHAudioBuffer_GetSpanInfo_WrapAround
 * @tc.desc  : Test GetSpanInfo handles wrap-around when deltaToBase >= totalSizeInFrame.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanInfo_WrapAround, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    uint64_t wrapPos = TEST_TOTAL_SIZE_FRAME + TEST_FRAME_POS_256;
    SpanInfo *info = buffer->GetSpanInfo(wrapPos);
    AIUT_EXPECT_NE_INFO_LOG(info, nullptr, "GetSpanInfo wrap should return non-null");
}

/**
 * @tc.name  : Test GetSpanInfo with zero spanSize
 * @tc.number: OHAudioBuffer_GetSpanInfo_ZeroSpan
 * @tc.desc  : Test GetSpanInfo returns nullptr when spanSizeInFrame is 0.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanInfo_ZeroSpan, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->UpdateSpanBasicInfo(0, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    SpanInfo *info = buffer->GetSpanInfo(0);
    AIUT_EXPECT_EQ_INFO_LOG(info, nullptr, "GetSpanInfo with zero span should return nullptr");
}

// ==================== GetSpanInfoByIndex Tests ====================

/**
 * @tc.name  : Test GetSpanInfoByIndex with valid index
 * @tc.number: OHAudioBuffer_GetSpanInfoByIndex_Valid
 * @tc.desc  : Test GetSpanInfoByIndex returns non-null with valid index.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanInfoByIndex_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    SpanInfo *info = buffer->GetSpanInfoByIndex(0);
    AIUT_EXPECT_NE_INFO_LOG(info, nullptr, "GetSpanInfoByIndex(0) should return non-null");
    info = buffer->GetSpanInfoByIndex(TEST_SPAN_COUNT - 1);
    AIUT_EXPECT_NE_INFO_LOG(info, nullptr, "GetSpanInfoByIndex(3) should return non-null");
}

/**
 * @tc.name  : Test GetSpanInfoByIndex with invalid index
 * @tc.number: OHAudioBuffer_GetSpanInfoByIndex_Invalid
 * @tc.desc  : Test GetSpanInfoByIndex returns nullptr with index >= spanCount.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanInfoByIndex_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    SpanInfo *info = buffer->GetSpanInfoByIndex(TEST_SPAN_COUNT);
    AIUT_EXPECT_EQ_INFO_LOG(info, nullptr, "GetSpanInfoByIndex(4) should return nullptr");
    info = buffer->GetSpanInfoByIndex(TEST_FRAME_POS_INVALID);
    AIUT_EXPECT_EQ_INFO_LOG(info, nullptr, "GetSpanInfoByIndex(invalid) should return nullptr");
}

// ==================== GetSpanCount Tests ====================

/**
 * @tc.name  : Test GetSpanCount after init
 * @tc.number: OHAudioBuffer_GetSpanCount_Init
 * @tc.desc  : Test GetSpanCount returns correct value after init.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSpanCount_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetSpanCount(), TEST_SPAN_COUNT,
        "spanCount should be 4");
}

// ==================== Delegate Method Tests ====================

/**
 * @tc.name  : Test GetStreamVolume and SetStreamVolume via OHAudioBuffer
 * @tc.number: OHAudioBuffer_StreamVolume
 * @tc.desc  : Test GetStreamVolume and SetStreamVolume through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_StreamVolume, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetStreamVolume(TEST_VOLUME_HALF);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetStreamVolume failed");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetStreamVolume(), TEST_VOLUME_HALF,
        "volume should be 0.5");
}

/**
 * @tc.name  : Test GetMuteFactor and SetMuteFactor via OHAudioBuffer
 * @tc.number: OHAudioBuffer_MuteFactor
 * @tc.desc  : Test GetMuteFactor and SetMuteFactor through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_MuteFactor, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetMuteFactor(MIN_FLOAT_VOLUME);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetMuteFactor failed");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetMuteFactor(), MIN_FLOAT_VOLUME,
        "muteFactor should be 0.0");
}

/**
 * @tc.name  : Test GetDuckFactor and SetDuckFactor via OHAudioBuffer
 * @tc.number: OHAudioBuffer_DuckFactor
 * @tc.desc  : Test GetDuckFactor and SetDuckFactor through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_DuckFactor, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetDuckFactor(TEST_VOLUME_HALF, 1000);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetDuckFactor failed");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetDuckFactor(), TEST_VOLUME_HALF,
        "duckFactor should be 0.5");
}

/**
 * @tc.name  : Test GetUnderrunCount and SetUnderrunCount via OHAudioBuffer
 * @tc.number: OHAudioBuffer_UnderrunCount
 * @tc.desc  : Test GetUnderrunCount and SetUnderrunCount through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_UnderrunCount, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetUnderrunCount(TEST_UNDERRUN_COUNT);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetUnderrunCount failed");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetUnderrunCount(), TEST_UNDERRUN_COUNT,
        "underrunCount should be 5");
}

/**
 * @tc.name  : Test GetHandleInfo and SetHandleInfo via OHAudioBuffer
 * @tc.number: OHAudioBuffer_HandleInfo
 * @tc.desc  : Test GetHandleInfo and SetHandleInfo through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_HandleInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetHandleInfo(TEST_FRAME_POS_100, 1000000);
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    bool ret = buffer->GetHandleInfo(frames, nanoTime);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "GetHandleInfo failed");
    AIUT_EXPECT_EQ_INFO_LOG(frames, TEST_FRAME_POS_100, "frames should be 100");
}

/**
 * @tc.name  : Test GetSessionId and SetSessionId via OHAudioBuffer
 * @tc.number: OHAudioBuffer_SessionId
 * @tc.desc  : Test GetSessionId and SetSessionId through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_SessionId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->SetSessionId(TEST_SESSION_ID);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetSessionId failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetSessionId(), TEST_SESSION_ID,
        "sessionId should be 42");
}

/**
 * @tc.name  : Test GetLastWrittenTime and SetLastWrittenTime via OHAudioBuffer
 * @tc.number: OHAudioBuffer_LastWrittenTime
 * @tc.desc  : Test GetLastWrittenTime and SetLastWrittenTime through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_LastWrittenTime, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetLastWrittenTime(TEST_WRITTEN_TIME);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetLastWrittenTime(), TEST_WRITTEN_TIME,
        "lastWrittenTime should match");
}

/**
 * @tc.name  : Test GetBufferHolder via OHAudioBuffer
 * @tc.number: OHAudioBuffer_GetBufferHolder
 * @tc.desc  : Test GetBufferHolder returns AUDIO_SERVER_SHARED after CreateFromLocal.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetBufferHolder, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetBufferHolder(), AUDIO_SERVER_SHARED,
        "bufferHolder should be AUDIO_SERVER_SHARED");
}

/**
 * @tc.name  : Test GetSizeParameter via OHAudioBuffer
 * @tc.number: OHAudioBuffer_GetSizeParameter
 * @tc.desc  : Test GetSizeParameter returns correct values through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetSizeParameter, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    uint32_t totalFrame = 0;
    uint32_t spanFrame = 0;
    uint32_t bytePerFrame = 0;
    int32_t ret = buffer->GetSizeParameter(totalFrame, spanFrame, bytePerFrame);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetSizeParameter failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(totalFrame, TEST_TOTAL_SIZE_FRAME, "totalFrame should be 1024");
    AIUT_EXPECT_EQ_INFO_LOG(spanFrame, TEST_SPAN_SIZE_FRAME, "spanFrame should be 256");
    AIUT_EXPECT_EQ_INFO_LOG(bytePerFrame, TEST_BYTE_SIZE_PER_FRAME, "bytePerFrame should be 4");
}

/**
 * @tc.name  : Test GetStreamStatus via OHAudioBuffer
 * @tc.number: OHAudioBuffer_GetStreamStatus
 * @tc.desc  : Test GetStreamStatus returns non-null through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetStreamStatus, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    auto *status = buffer->GetStreamStatus();
    AIUT_EXPECT_NE_INFO_LOG(status, nullptr, "GetStreamStatus should return non-null");
}

/**
 * @tc.name  : Test GetFutex via OHAudioBuffer
 * @tc.number: OHAudioBuffer_GetFutex
 * @tc.desc  : Test GetFutex returns non-null through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetFutex, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    auto *futex = buffer->GetFutex();
    AIUT_EXPECT_NE_INFO_LOG(futex, nullptr, "GetFutex should return non-null");
}

/**
 * @tc.name  : Test GetDataBase and GetDataSize via OHAudioBuffer
 * @tc.number: OHAudioBuffer_GetDataBaseSize
 * @tc.desc  : Test GetDataBase and GetDataSize through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetDataBaseSize, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_NE_INFO_LOG(buffer->GetDataBase(), nullptr, "GetDataBase should return non-null");
    size_t expectedSize = TEST_TOTAL_SIZE_FRAME * TEST_BYTE_SIZE_PER_FRAME;
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetDataSize(), expectedSize,
        "GetDataSize should be %zu", expectedSize);
}

/**
 * @tc.name  : Test RestoreStatus via OHAudioBuffer
 * @tc.number: OHAudioBuffer_RestoreStatus
 * @tc.desc  : Test GetRestoreStatus, CheckRestoreStatus, SetRestoreStatus through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_RestoreStatus, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetRestoreStatus(), NO_NEED_FOR_RESTORE,
        "initial status should be NO_NEED_FOR_RESTORE");
    buffer->SetRestoreStatus(NEED_RESTORE);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetRestoreStatus(), NEED_RESTORE,
        "status should be NEED_RESTORE");
    buffer->CheckRestoreStatus();
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetRestoreStatus(), RESTORING,
        "status should be RESTORING");
}

/**
 * @tc.name  : Test StopFlag via OHAudioBuffer
 * @tc.number: OHAudioBuffer_StopFlag
 * @tc.desc  : Test SetStopFlag and GetStopFlag through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_StopFlag, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStopFlag(true);
    AIUT_EXPECT_TRUE_INFO_LOG(buffer->GetStopFlag(), "GetStopFlag should return true");
}

/**
 * @tc.name  : Test SetCurWriteFrame and SetCurReadFrame via OHAudioBuffer
 * @tc.number: OHAudioBuffer_SetCurReadWriteFrame
 * @tc.desc  : Test SetCurWriteFrame and SetCurReadFrame through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_SetCurReadWriteFrame, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t retW = buffer->SetCurWriteFrame(TEST_FRAME_POS_256, false);
    AIUT_EXPECT_EQ_INFO_LOG(retW, SUCCESS, "SetCurWriteFrame failed, ret=%d", retW);
    int32_t retR = buffer->SetCurReadFrame(0, false);
    AIUT_EXPECT_EQ_INFO_LOG(retR, SUCCESS, "SetCurReadFrame failed, ret=%d", retR);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurWriteFrame(), TEST_FRAME_POS_256,
        "curWriteFrame should be 256");
}

/**
 * @tc.name  : Test SetCurWriteFrame with unaligned frame via OHAudioBuffer
 * @tc.number: OHAudioBuffer_SetCurWriteFrame_Unaligned
 * @tc.desc  : Test SetCurWriteFrame returns ERR_INVALID_PARAM with unaligned frame.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_SetCurWriteFrame_Unaligned, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->SetCurWriteFrame(TEST_FRAME_POS_100, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "SetCurWriteFrame unaligned should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test ResetCurReadWritePos via OHAudioBuffer
 * @tc.number: OHAudioBuffer_ResetCurReadWritePos
 * @tc.desc  : Test ResetCurReadWritePos through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_ResetCurReadWritePos, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->ResetCurReadWritePos(0, TEST_FRAME_POS_256, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "ResetCurReadWritePos failed, ret=%d", ret);
}

/**
 * @tc.name  : Test GetWritableDataFrames via OHAudioBuffer
 * @tc.number: OHAudioBuffer_GetWritableDataFrames
 * @tc.desc  : Test GetWritableDataFrames through OHAudioBuffer.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_GetWritableDataFrames, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t frames = buffer->GetWritableDataFrames();
    AIUT_EXPECT_EQ_INFO_LOG(frames, static_cast<int32_t>(TEST_TOTAL_SIZE_FRAME),
        "writable frames should be 1024, got frames=%d", frames);
}

/**
 * @tc.name  : Test WakeFutex via OHAudioBuffer
 * @tc.number: OHAudioBuffer_WakeFutex
 * @tc.desc  : Test WakeFutex through OHAudioBuffer. WakeFutex returns void, verify futex state after call.
 */
HWTEST_F(OHAudioBufferAiut, OHAudioBuffer_WakeFutex, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->WakeFutex(IS_READY);
    buffer->WakeFutex(IS_NOT_READY);
    auto *futex = buffer->GetFutex();
    AIUT_EXPECT_NE_INFO_LOG(futex, nullptr, "GetFutex should return non-null after WakeFutex");
}

}  // namespace AudioStandard
}  // namespace OHOS
