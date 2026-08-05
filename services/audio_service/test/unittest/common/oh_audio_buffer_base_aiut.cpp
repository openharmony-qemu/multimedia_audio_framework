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
constexpr float TEST_VOLUME_HALF = 0.5f;
constexpr float TEST_VOLUME_INVALID_LOW = -0.1f;
constexpr float TEST_VOLUME_INVALID_HIGH = 1.1f;
constexpr uint32_t TEST_DUCK_DURATION_MS = 1000;
constexpr uint32_t TEST_UNDERRUN_COUNT = 5;
constexpr uint64_t TEST_FRAME_POS_100 = 100;
constexpr uint64_t TEST_FRAME_POS_200 = 200;
constexpr uint64_t TEST_FRAME_POS_INVALID = 3000;
constexpr int64_t TEST_NANO_TIME = 1000000;
constexpr int64_t TEST_WRITTEN_TIME = 123456789;
constexpr uint32_t TEST_RAW_BUFFER_SIZE = 1024;
constexpr uint32_t TEST_SESSION_ID = 42;
}  // namespace

class OHAudioBufferBaseAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

static std::shared_ptr<OHAudioBufferBase> CreateTestBuffer()
{
    return OHAudioBufferBase::CreateFromLocal(TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
}

// ==================== Volume / Mute / Duck Factor Tests ====================

/**
 * @tc.name  : Test GetStreamVolume with default value
 * @tc.number: OHAudioBufferBase_GetStreamVolume_Default
 * @tc.desc  : Test GetStreamVolume returns MAX_FLOAT_VOLUME after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetStreamVolume_Default, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    float vol = buffer->GetStreamVolume();
    AIUT_EXPECT_EQ_INFO_LOG(vol, MAX_FLOAT_VOLUME, "expected MAX_FLOAT_VOLUME, got vol=%f", vol);
}

/**
 * @tc.name  : Test SetStreamVolume with valid volume
 * @tc.number: OHAudioBufferBase_SetStreamVolume_Valid
 * @tc.desc  : Test SetStreamVolume returns true with valid volume.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetStreamVolume_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetStreamVolume(TEST_VOLUME_HALF);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetStreamVolume failed, ret=%d", ret);
    float vol = buffer->GetStreamVolume();
    AIUT_EXPECT_EQ_INFO_LOG(vol, TEST_VOLUME_HALF, "expected %f, got vol=%f", TEST_VOLUME_HALF, vol);
}

/**
 * @tc.name  : Test SetStreamVolume with invalid volume
 * @tc.number: OHAudioBufferBase_SetStreamVolume_Invalid
 * @tc.desc  : Test SetStreamVolume returns false with invalid volume.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetStreamVolume_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool retLow = buffer->SetStreamVolume(TEST_VOLUME_INVALID_LOW);
    AIUT_EXPECT_FALSE_INFO_LOG(retLow, "SetStreamVolume should fail with negative volume");
    bool retHigh = buffer->SetStreamVolume(TEST_VOLUME_INVALID_HIGH);
    AIUT_EXPECT_FALSE_INFO_LOG(retHigh, "SetStreamVolume should fail with volume > 1.0");
}

/**
 * @tc.name  : Test GetStreamVolume with null basicBufferInfo
 * @tc.number: OHAudioBufferBase_GetStreamVolume_NullBuffer
 * @tc.desc  : Test GetStreamVolume returns MAX_FLOAT_VOLUME when not inited.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetStreamVolume_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OHAudioBufferBase buffer(AUDIO_SERVER_SHARED, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    float vol = buffer.GetStreamVolume();
    AIUT_EXPECT_EQ_INFO_LOG(vol, MAX_FLOAT_VOLUME, "expected MAX_FLOAT_VOLUME, got vol=%f", vol);
}

/**
 * @tc.name  : Test SetStreamVolume with null basicBufferInfo
 * @tc.number: OHAudioBufferBase_SetStreamVolume_NullBuffer
 * @tc.desc  : Test SetStreamVolume returns false when not inited.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetStreamVolume_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OHAudioBufferBase buffer(AUDIO_SERVER_SHARED, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    bool ret = buffer.SetStreamVolume(TEST_VOLUME_HALF);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "SetStreamVolume should fail when not inited");
}

/**
 * @tc.name  : Test GetMuteFactor and SetMuteFactor
 * @tc.number: OHAudioBufferBase_MuteFactor_Normal
 * @tc.desc  : Test GetMuteFactor and SetMuteFactor with valid values.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_MuteFactor_Normal, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool retZero = buffer->SetMuteFactor(MIN_FLOAT_VOLUME);
    AIUT_EXPECT_TRUE_INFO_LOG(retZero, "SetMuteFactor(0.0) failed");
    float factor0 = buffer->GetMuteFactor();
    AIUT_EXPECT_EQ_INFO_LOG(factor0, MIN_FLOAT_VOLUME, "expected 0.0, got factor=%f", factor0);
    bool retOne = buffer->SetMuteFactor(MAX_FLOAT_VOLUME);
    AIUT_EXPECT_TRUE_INFO_LOG(retOne, "SetMuteFactor(1.0) failed");
    float factor1 = buffer->GetMuteFactor();
    AIUT_EXPECT_EQ_INFO_LOG(factor1, MAX_FLOAT_VOLUME, "expected 1.0, got factor=%f", factor1);
}

/**
 * @tc.name  : Test SetMuteFactor with invalid value
 * @tc.number: OHAudioBufferBase_SetMuteFactor_Invalid
 * @tc.desc  : Test SetMuteFactor returns false with value not 0.0 or 1.0.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetMuteFactor_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetMuteFactor(TEST_VOLUME_HALF);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "SetMuteFactor should fail with 0.5");
}

/**
 * @tc.name  : Test GetMuteFactor with null basicBufferInfo
 * @tc.number: OHAudioBufferBase_GetMuteFactor_NullBuffer
 * @tc.desc  : Test GetMuteFactor returns MAX_FLOAT_VOLUME when not inited.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetMuteFactor_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OHAudioBufferBase buffer(AUDIO_SERVER_SHARED, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    float factor = buffer.GetMuteFactor();
    AIUT_EXPECT_EQ_INFO_LOG(factor, MAX_FLOAT_VOLUME, "expected MAX_FLOAT_VOLUME, got factor=%f", factor);
}

/**
 * @tc.name  : Test GetDuckFactor and SetDuckFactor
 * @tc.number: OHAudioBufferBase_DuckFactor_Normal
 * @tc.desc  : Test GetDuckFactor and SetDuckFactor with valid values.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_DuckFactor_Normal, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->SetDuckFactor(TEST_VOLUME_HALF, TEST_DUCK_DURATION_MS);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetDuckFactor failed");
    float factor = buffer->GetDuckFactor();
    AIUT_EXPECT_EQ_INFO_LOG(factor, TEST_VOLUME_HALF, "expected %f, got factor=%f", TEST_VOLUME_HALF, factor);
}

/**
 * @tc.name  : Test SetDuckFactor with invalid value
 * @tc.number: OHAudioBufferBase_SetDuckFactor_Invalid
 * @tc.desc  : Test SetDuckFactor returns false with invalid value.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetDuckFactor_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool retLow = buffer->SetDuckFactor(TEST_VOLUME_INVALID_LOW, TEST_DUCK_DURATION_MS);
    AIUT_EXPECT_FALSE_INFO_LOG(retLow, "SetDuckFactor should fail with negative value");
    bool retHigh = buffer->SetDuckFactor(TEST_VOLUME_INVALID_HIGH, TEST_DUCK_DURATION_MS);
    AIUT_EXPECT_FALSE_INFO_LOG(retHigh, "SetDuckFactor should fail with value > 1.0");
}

/**
 * @tc.name  : Test GetDuckFactor with null basicBufferInfo
 * @tc.number: OHAudioBufferBase_GetDuckFactor_NullBuffer
 * @tc.desc  : Test GetDuckFactor returns MAX_FLOAT_VOLUME when not inited.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetDuckFactor_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OHAudioBufferBase buffer(AUDIO_SERVER_SHARED, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    float factor = buffer.GetDuckFactor();
    AIUT_EXPECT_EQ_INFO_LOG(factor, MAX_FLOAT_VOLUME, "expected MAX_FLOAT_VOLUME, got factor=%f", factor);
}

// ==================== Position / Frame Tests ====================

/**
 * @tc.name  : Test GetCurWriteFrame and GetCurReadFrame after init
 * @tc.number: OHAudioBufferBase_GetCurReadWriteFrame_Init
 * @tc.desc  : Test GetCurWriteFrame and GetCurReadFrame return 0 after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetCurReadWriteFrame_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurWriteFrame(), 0ULL, "curWriteFrame should be 0");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurReadFrame(), 0ULL, "curReadFrame should be 0");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetBasePosInFrame(), 0ULL, "basePosInFrame should be 0");
}

/**
 * @tc.name  : Test SetCurWriteFrame with same position
 * @tc.number: OHAudioBufferBase_SetCurWriteFrame_SamePos
 * @tc.desc  : Test SetCurWriteFrame returns SUCCESS when writeFrame equals oldWritePos.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurWriteFrame_SamePos, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->SetCurWriteFrame(0, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetCurWriteFrame same pos failed, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCurWriteFrame with valid forward position
 * @tc.number: OHAudioBufferBase_SetCurWriteFrame_Valid
 * @tc.desc  : Test SetCurWriteFrame returns SUCCESS with valid forward position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurWriteFrame_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->SetCurWriteFrame(TEST_FRAME_POS_100, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetCurWriteFrame failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurWriteFrame(), TEST_FRAME_POS_100,
        "curWriteFrame should be 100");
}

/**
 * @tc.name  : Test SetCurWriteFrame with backward position
 * @tc.number: OHAudioBufferBase_SetCurWriteFrame_Backward
 * @tc.desc  : Test SetCurWriteFrame returns ERR_INVALID_PARAM with backward position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurWriteFrame_Backward, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_200, false);
    int32_t ret = buffer->SetCurWriteFrame(TEST_FRAME_POS_100, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "SetCurWriteFrame backward should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCurWriteFrame with out of range position
 * @tc.number: OHAudioBufferBase_SetCurWriteFrame_OutOfRange
 * @tc.desc  : Test SetCurWriteFrame returns ERR_INVALID_PARAM with position out of range.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurWriteFrame_OutOfRange, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->SetCurWriteFrame(TEST_FRAME_POS_INVALID, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "SetCurWriteFrame out of range should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCurReadFrame with same position
 * @tc.number: OHAudioBufferBase_SetCurReadFrame_SamePos
 * @tc.desc  : Test SetCurReadFrame returns SUCCESS when readFrame equals oldReadPos.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurReadFrame_SamePos, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->SetCurReadFrame(0, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetCurReadFrame same pos failed, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCurReadFrame with valid forward position
 * @tc.number: OHAudioBufferBase_SetCurReadFrame_Valid
 * @tc.desc  : Test SetCurReadFrame returns SUCCESS with valid forward position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurReadFrame_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_200, false);
    int32_t ret = buffer->SetCurReadFrame(TEST_FRAME_POS_100, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetCurReadFrame failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurReadFrame(), TEST_FRAME_POS_100,
        "curReadFrame should be 100");
}

/**
 * @tc.name  : Test SetCurReadFrame with backward position
 * @tc.number: OHAudioBufferBase_SetCurReadFrame_Backward
 * @tc.desc  : Test SetCurReadFrame returns ERR_INVALID_PARAM with backward position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurReadFrame_Backward, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_200, false);
    buffer->SetCurReadFrame(TEST_FRAME_POS_100, false);
    int32_t ret = buffer->SetCurReadFrame(0, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "SetCurReadFrame backward should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCurReadFrame with position beyond writeFrame
 * @tc.number: OHAudioBufferBase_SetCurReadFrame_BeyondWrite
 * @tc.desc  : Test SetCurReadFrame returns ERR_INVALID_PARAM when readFrame > writeFrame.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetCurReadFrame_BeyondWrite, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_100, false);
    int32_t ret = buffer->SetCurReadFrame(TEST_FRAME_POS_200, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "SetCurReadFrame beyond write should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test ResetCurReadWritePos with valid params
 * @tc.number: OHAudioBufferBase_ResetCurReadWritePos_Valid
 * @tc.desc  : Test ResetCurReadWritePos returns SUCCESS with valid params.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_ResetCurReadWritePos_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->ResetCurReadWritePos(0, TEST_FRAME_POS_100, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "ResetCurReadWritePos failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurReadFrame(), 0ULL, "curReadFrame should be 0");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetCurWriteFrame(), TEST_FRAME_POS_100,
        "curWriteFrame should be 100");
}

/**
 * @tc.name  : Test ResetCurReadWritePos with invalid params
 * @tc.number: OHAudioBufferBase_ResetCurReadWritePos_Invalid
 * @tc.desc  : Test ResetCurReadWritePos returns ERR_INVALID_PARAM with read > write.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_ResetCurReadWritePos_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t ret = buffer->ResetCurReadWritePos(TEST_FRAME_POS_200, TEST_FRAME_POS_100, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "ResetCurReadWritePos read>write should fail, ret=%d", ret);
    int32_t ret2 = buffer->ResetCurReadWritePos(0, TEST_FRAME_POS_INVALID, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret2, ERR_INVALID_PARAM,
        "ResetCurReadWritePos too large should fail, ret=%d", ret2);
}

// ==================== Offset / Buffer Access Tests ====================

/**
 * @tc.name  : Test GetOffsetByFrame with valid position
 * @tc.number: OHAudioBufferBase_GetOffsetByFrame_Valid
 * @tc.desc  : Test GetOffsetByFrame returns SUCCESS with valid position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrame_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    size_t offset = 0;
    int32_t ret = buffer->GetOffsetByFrame(TEST_FRAME_POS_100, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetOffsetByFrame failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(offset, TEST_FRAME_POS_100 * TEST_BYTE_SIZE_PER_FRAME,
        "offset should be 400, got offset=%zu", offset);
}

/**
 * @tc.name  : Test GetOffsetByFrame with position beyond totalSize
 * @tc.number: OHAudioBufferBase_GetOffsetByFrame_WrapAround
 * @tc.desc  : Test GetOffsetByFrame handles wrap-around when deltaToBase >= totalSizeInFrame.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrame_WrapAround, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->ResetCurReadWritePos(0, 0, false);
    size_t offset = 0;
    uint64_t posBeyondTotal = TEST_TOTAL_SIZE_FRAME + TEST_FRAME_POS_100;
    int32_t ret = buffer->GetOffsetByFrame(posBeyondTotal, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetOffsetByFrame wrap failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(offset, TEST_FRAME_POS_100 * TEST_BYTE_SIZE_PER_FRAME,
        "offset should be 400 after wrap, got offset=%zu", offset);
}

/**
 * @tc.name  : Test GetOffsetByFrame with invalid position
 * @tc.number: OHAudioBufferBase_GetOffsetByFrame_Invalid
 * @tc.desc  : Test GetOffsetByFrame returns ERR_INVALID_PARAM with out of range position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrame_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    size_t offset = 0;
    int32_t ret = buffer->GetOffsetByFrame(TEST_FRAME_POS_INVALID, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetOffsetByFrame out of range should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetOffsetByFrameForWrite with valid position
 * @tc.number: OHAudioBufferBase_GetOffsetByFrameForWrite_Valid
 * @tc.desc  : Test GetOffsetByFrameForWrite returns SUCCESS with valid position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrameForWrite_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    size_t offset = 0;
    int32_t ret = buffer->GetOffsetByFrameForWrite(TEST_FRAME_POS_100, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetOffsetByFrameForWrite failed, ret=%d", ret);
}

/**
 * @tc.name  : Test GetOffsetByFrameForWrite with position before read
 * @tc.number: OHAudioBufferBase_GetOffsetByFrameForWrite_BeforeRead
 * @tc.desc  : Test GetOffsetByFrameForWrite fails when writePos < readPos.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrameForWrite_BeforeRead, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->ResetCurReadWritePos(TEST_FRAME_POS_200, TEST_FRAME_POS_200, false);
    size_t offset = 0;
    int32_t ret = buffer->GetOffsetByFrameForWrite(TEST_FRAME_POS_100, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetOffsetByFrameForWrite before read should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetOffsetByFrameForRead with valid position
 * @tc.number: OHAudioBufferBase_GetOffsetByFrameForRead_Valid
 * @tc.desc  : Test GetOffsetByFrameForRead returns SUCCESS with valid position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrameForRead_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    size_t offset = 0;
    int32_t ret = buffer->GetOffsetByFrameForRead(TEST_FRAME_POS_100, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetOffsetByFrameForRead failed, ret=%d", ret);
}

/**
 * @tc.name  : Test GetOffsetByFrameForRead with out of range position
 * @tc.number: OHAudioBufferBase_GetOffsetByFrameForRead_OutOfRange
 * @tc.desc  : Test GetOffsetByFrameForRead fails with position beyond totalSizeInFrame.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetOffsetByFrameForRead_OutOfRange, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    size_t offset = 0;
    int32_t ret = buffer->GetOffsetByFrameForRead(TEST_FRAME_POS_INVALID, offset);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetOffsetByFrameForRead out of range should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetBufferByOffset with valid params
 * @tc.number: OHAudioBufferBase_GetBufferByOffset_Valid
 * @tc.desc  : Test GetBufferByOffset returns SUCCESS with valid offset and dataLength.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferByOffset_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    int32_t ret = buffer->GetBufferByOffset(0, 100, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetBufferByOffset failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(ringBuffer.dataLength, 100U,
        "dataLength should be 100");
}

/**
 * @tc.name  : Test GetBufferByOffset with invalid offset
 * @tc.number: OHAudioBufferBase_GetBufferByOffset_InvalidOffset
 * @tc.desc  : Test GetBufferByOffset returns ERR_INVALID_PARAM with offset >= totalSizeInByte.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferByOffset_InvalidOffset, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    size_t bigOffset = TEST_TOTAL_SIZE_FRAME * TEST_BYTE_SIZE_PER_FRAME + 1;
    int32_t ret = buffer->GetBufferByOffset(bigOffset, 100, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetBufferByOffset invalid offset should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetBufferByOffset with invalid dataLength
 * @tc.number: OHAudioBufferBase_GetBufferByOffset_InvalidDataLength
 * @tc.desc  : Test GetBufferByOffset returns ERR_INVALID_PARAM with dataLength 0 or too large.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferByOffset_InvalidDataLength, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    int32_t retZero = buffer->GetBufferByOffset(0, 0, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(retZero, ERR_INVALID_PARAM,
        "GetBufferByOffset dataLength=0 should fail, ret=%d", retZero);
    size_t bigLen = TEST_TOTAL_SIZE_FRAME * TEST_BYTE_SIZE_PER_FRAME + 1;
    int32_t retBig = buffer->GetBufferByOffset(0, bigLen, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(retBig, ERR_INVALID_PARAM,
        "GetBufferByOffset dataLength too large should fail, ret=%d", retBig);
}

/**
 * @tc.name  : Test GetBufferByOffset with cross-boundary data
 * @tc.number: OHAudioBufferBase_GetBufferByOffset_CrossBoundary
 * @tc.desc  : Test GetBufferByOffset handles data that wraps around the buffer end.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferByOffset_CrossBoundary, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    size_t totalBytes = TEST_TOTAL_SIZE_FRAME * TEST_BYTE_SIZE_PER_FRAME;
    size_t offset = totalBytes - 100;
    int32_t ret = buffer->GetBufferByOffset(offset, 200, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetBufferByOffset cross failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(ringBuffer.dataLength, 200U, "dataLength should be 200");
    AIUT_EXPECT_NE_INFO_LOG(ringBuffer.basicBufferDescs[1].buffer, nullptr,
        "second desc buffer should not be null for cross-boundary");
}

/**
 * @tc.name  : Test TryGetContinuousBufferByOffset with valid params
 * @tc.number: OHAudioBufferBase_TryGetContinuousBufferByOffset_Valid
 * @tc.desc  : Test TryGetContinuousBufferByOffset returns SUCCESS with non-crossing data.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_TryGetContinuousBufferByOffset_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    int32_t ret = buffer->TryGetContinuousBufferByOffset(0, 100, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "TryGetContinuousBufferByOffset failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(desc.buffer, nullptr, "desc.buffer should not be null");
    AIUT_EXPECT_EQ_INFO_LOG(desc.bufLength, 100U, "bufLength should be 100");
}

/**
 * @tc.name  : Test TryGetContinuousBufferByOffset with crossing data
 * @tc.number: OHAudioBufferBase_TryGetContinuousBufferByOffset_Crossing
 * @tc.desc  : Test TryGetContinuousBufferByOffset fails when data crosses boundary.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_TryGetContinuousBufferByOffset_Crossing, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    size_t totalBytes = TEST_TOTAL_SIZE_FRAME * TEST_BYTE_SIZE_PER_FRAME;
    size_t offset = totalBytes - 100;
    int32_t ret = buffer->TryGetContinuousBufferByOffset(offset, 200, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "TryGetContinuousBufferByOffset crossing should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetBufferByFrame with valid params
 * @tc.number: OHAudioBufferBase_GetBufferByFrame_Valid
 * @tc.desc  : Test GetBufferByFrame returns SUCCESS with valid frame and size.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferByFrame_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    int32_t ret = buffer->GetBufferByFrame(0, 100, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetBufferByFrame failed, ret=%d", ret);
}

/**
 * @tc.name  : Test GetBufferByFrame with invalid size
 * @tc.number: OHAudioBufferBase_GetBufferByFrame_InvalidSize
 * @tc.desc  : Test GetBufferByFrame returns ERR_INVALID_PARAM with size 0 or too large.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferByFrame_InvalidSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    int32_t retZero = buffer->GetBufferByFrame(0, 0, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(retZero, ERR_INVALID_PARAM,
        "GetBufferByFrame size=0 should fail, ret=%d", retZero);
    int32_t retBig = buffer->GetBufferByFrame(0, TEST_FRAME_POS_INVALID, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(retBig, ERR_INVALID_PARAM,
        "GetBufferByFrame size too large should fail, ret=%d", retBig);
}

// ==================== Data Frames / Buffer Access Tests ====================

/**
 * @tc.name  : Test GetWritableDataFrames after init
 * @tc.number: OHAudioBufferBase_GetWritableDataFrames_Init
 * @tc.desc  : Test GetWritableDataFrames returns totalSizeInFrame after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetWritableDataFrames_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    int32_t frames = buffer->GetWritableDataFrames();
    AIUT_EXPECT_EQ_INFO_LOG(frames, static_cast<int32_t>(TEST_TOTAL_SIZE_FRAME),
        "writable frames should be 1024, got frames=%d", frames);
}

/**
 * @tc.name  : Test GetReadableDataFrames after setting write frame
 * @tc.number: OHAudioBufferBase_GetReadableDataFrames_AfterWrite
 * @tc.desc  : Test GetReadableDataFrames returns correct value after SetCurWriteFrame.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetReadableDataFrames_AfterWrite, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_100, false);
    int32_t frames = buffer->GetReadableDataFrames();
    AIUT_EXPECT_EQ_INFO_LOG(frames, static_cast<int32_t>(TEST_FRAME_POS_100),
        "readable frames should be 100, got frames=%d", frames);
}

/**
 * @tc.name  : Test GetAllWritableBuffer after init
 * @tc.number: OHAudioBufferBase_GetAllWritableBuffer_Init
 * @tc.desc  : Test GetAllWritableBuffer returns SUCCESS after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetAllWritableBuffer_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    int32_t ret = buffer->GetAllWritableBuffer(ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetAllWritableBuffer failed, ret=%d", ret);
}

/**
 * @tc.name  : Test GetAllReadableBuffer after write
 * @tc.number: OHAudioBufferBase_GetAllReadableBuffer_AfterWrite
 * @tc.desc  : Test GetAllReadableBuffer returns SUCCESS after setting write frame.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetAllReadableBuffer_AfterWrite, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetCurWriteFrame(TEST_FRAME_POS_100, false);
    RingBufferWrapper ringBuffer;
    int32_t ret = buffer->GetAllReadableBuffer(ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetAllReadableBuffer failed, ret=%d", ret);
}

/**
 * @tc.name  : Test GetAllReadableBufferFromPosFrame with invalid position
 * @tc.number: OHAudioBufferBase_GetAllReadableBufferFromPosFrame_Invalid
 * @tc.desc  : Test GetAllReadableBufferFromPosFrame fails with invalid position.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetAllReadableBufferFromPosFrame_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RingBufferWrapper ringBuffer;
    int32_t ret = buffer->GetAllReadableBufferFromPosFrame(TEST_FRAME_POS_INVALID, ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "GetAllReadableBufferFromPosFrame invalid should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetRawBuffer with valid size
 * @tc.number: OHAudioBufferBase_GetRawBuffer_Valid
 * @tc.desc  : Test GetRawBuffer returns SUCCESS with valid size.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetRawBuffer_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    int32_t ret = buffer->GetRawBuffer(TEST_RAW_BUFFER_SIZE, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetRawBuffer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(desc.buffer, nullptr, "desc.buffer should not be null");
    AIUT_EXPECT_EQ_INFO_LOG(desc.bufLength, TEST_RAW_BUFFER_SIZE,
        "bufLength should be 1024");
}

/**
 * @tc.name  : Test GetRawBuffer with invalid size
 * @tc.number: OHAudioBufferBase_GetRawBuffer_InvalidSize
 * @tc.desc  : Test GetRawBuffer returns error with size > totalSizeInByte.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetRawBuffer_InvalidSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    BufferDesc desc;
    uint32_t bigSize = TEST_TOTAL_SIZE_FRAME * TEST_BYTE_SIZE_PER_FRAME + 1;
    int32_t ret = buffer->GetRawBuffer(bigSize, desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIO_INVALID_PARAM,
        "GetRawBuffer too large should fail, ret=%d", ret);
}

// ==================== Sync Frame / Misc Tests ====================

/**
 * @tc.name  : Test GetSyncWriteFrame and SetSyncWriteFrame without sync support
 * @tc.number: OHAudioBufferBase_SyncWriteFrame_NotSupported
 * @tc.desc  : Test GetSyncWriteFrame returns 0 and SetSyncWriteFrame returns false
 *             when bufferHolder is not AUDIO_SERVER_ONLY_WITH_SYNC.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SyncWriteFrame_NotSupported, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    uint32_t frame = buffer->GetSyncWriteFrame();
    AIUT_EXPECT_EQ_INFO_LOG(frame, 0U, "GetSyncWriteFrame should return 0");
    bool ret = buffer->SetSyncWriteFrame(100);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "SetSyncWriteFrame should fail without sync support");
}

/**
 * @tc.name  : Test GetSyncReadFrame and SetSyncReadFrame without sync support
 * @tc.number: OHAudioBufferBase_SyncReadFrame_NotSupported
 * @tc.desc  : Test GetSyncReadFrame returns 0 and SetSyncReadFrame returns false
 *             when bufferHolder is not AUDIO_SERVER_ONLY_WITH_SYNC.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SyncReadFrame_NotSupported, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    uint32_t frame = buffer->GetSyncReadFrame();
    AIUT_EXPECT_EQ_INFO_LOG(frame, 0U, "GetSyncReadFrame should return 0");
    bool ret = buffer->SetSyncReadFrame(100);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "SetSyncReadFrame should fail without sync support");
}

/**
 * @tc.name  : Test GetFutex after init
 * @tc.number: OHAudioBufferBase_GetFutex_Init
 * @tc.desc  : Test GetFutex returns non-null after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetFutex_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    auto *futex = buffer->GetFutex();
    AIUT_EXPECT_NE_INFO_LOG(futex, nullptr, "GetFutex should return non-null");
}

/**
 * @tc.name  : Test GetDataBase and GetDataSize after init
 * @tc.number: OHAudioBufferBase_GetDataBaseSize_Init
 * @tc.desc  : Test GetDataBase returns non-null and GetDataSize returns correct value.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetDataBaseSize_Init, TestSize.Level0)
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
 * @tc.name  : Test GetLastWrittenTime and SetLastWrittenTime
 * @tc.number: OHAudioBufferBase_LastWrittenTime
 * @tc.desc  : Test GetLastWrittenTime and SetLastWrittenTime.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_LastWrittenTime, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetLastWrittenTime(TEST_WRITTEN_TIME);
    int64_t time = buffer->GetLastWrittenTime();
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_WRITTEN_TIME,
        "expected %lld, got time=%lld", TEST_WRITTEN_TIME, time);
}

// ==================== Restore / Status Tests ====================

/**
 * @tc.name  : Test GetRestoreInfo and SetRestoreInfo
 * @tc.number: OHAudioBufferBase_RestoreInfo
 * @tc.desc  : Test GetRestoreInfo and SetRestoreInfo.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_RestoreInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RestoreInfo setInfo;
    setInfo.restoreReason = DEVICE_CHANGED;
    setInfo.deviceChangeReason = 1;
    setInfo.targetStreamFlag = AUDIO_FLAG_NORMAL;
    setInfo.routeFlag = 2;
    buffer->SetRestoreInfo(setInfo);
    RestoreInfo getInfo;
    buffer->GetRestoreInfo(getInfo);
    AIUT_EXPECT_EQ_INFO_LOG(getInfo.restoreReason, DEVICE_CHANGED,
        "restoreReason mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(getInfo.routeFlag, 2U, "routeFlag mismatch");
}

/**
 * @tc.name  : Test GetTimeStampInfo and SetTimeStampInfo
 * @tc.number: OHAudioBufferBase_TimeStampInfo
 * @tc.desc  : Test GetTimeStampInfo and SetTimeStampInfo.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_TimeStampInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetTimeStampInfo(100, 200);
    uint64_t position = 0;
    uint64_t timeStamp = 0;
    buffer->GetTimeStampInfo(position, timeStamp);
    AIUT_EXPECT_EQ_INFO_LOG(position, 100ULL, "position should be 100");
    AIUT_EXPECT_EQ_INFO_LOG(timeStamp, 200ULL, "timeStamp should be 200");
}

/**
 * @tc.name  : Test GetRestoreStatus after init
 * @tc.number: OHAudioBufferBase_GetRestoreStatus_Init
 * @tc.desc  : Test GetRestoreStatus returns NO_NEED_FOR_RESTORE after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetRestoreStatus_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RestoreStatus status = buffer->GetRestoreStatus();
    AIUT_EXPECT_EQ_INFO_LOG(status, NO_NEED_FOR_RESTORE,
        "expected NO_NEED_FOR_RESTORE");
}

/**
 * @tc.name  : Test SetRestoreStatus with NEED_RESTORE
 * @tc.number: OHAudioBufferBase_SetRestoreStatus_NeedRestore
 * @tc.desc  : Test SetRestoreStatus transitions from NO_NEED_FOR_RESTORE to NEED_RESTORE.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetRestoreStatus_NeedRestore, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    RestoreStatus expected = buffer->SetRestoreStatus(NEED_RESTORE);
    AIUT_EXPECT_EQ_INFO_LOG(expected, NO_NEED_FOR_RESTORE,
        "expected NO_NEED_FOR_RESTORE, got status=%d", expected);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetRestoreStatus(), NEED_RESTORE,
        "status should be NEED_RESTORE");
}

/**
 * @tc.name  : Test CheckRestoreStatus with NEED_RESTORE
 * @tc.number: OHAudioBufferBase_CheckRestoreStatus_NeedRestore
 * @tc.desc  : Test CheckRestoreStatus transitions NEED_RESTORE to RESTORING.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_CheckRestoreStatus_NeedRestore, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetRestoreStatus(NEED_RESTORE);
    RestoreStatus expected = buffer->CheckRestoreStatus();
    AIUT_EXPECT_EQ_INFO_LOG(expected, NEED_RESTORE,
        "expected NEED_RESTORE, got status=%d", expected);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetRestoreStatus(), RESTORING,
        "status should be RESTORING");
}

/**
 * @tc.name  : Test SetRestoreStatus with NO_NEED_FOR_RESTORE from RESTORING
 * @tc.number: OHAudioBufferBase_SetRestoreStatus_NoNeedFromRestoring
 * @tc.desc  : Test SetRestoreStatus transitions from RESTORING to NO_NEED_FOR_RESTORE.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SetRestoreStatus_NoNeedFromRestoring, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetRestoreStatus(NEED_RESTORE);
    buffer->CheckRestoreStatus();
    RestoreStatus expected = buffer->SetRestoreStatus(NO_NEED_FOR_RESTORE);
    AIUT_EXPECT_EQ_INFO_LOG(expected, RESTORING,
        "expected RESTORING, got status=%d", expected);
}

/**
 * @tc.name  : Test SetStopFlag and GetStopFlag
 * @tc.number: OHAudioBufferBase_StopFlag
 * @tc.desc  : Test SetStopFlag and GetStopFlag.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_StopFlag, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStopFlag(true);
    bool flag = buffer->GetStopFlag();
    AIUT_EXPECT_TRUE_INFO_LOG(flag, "GetStopFlag should return true");
    bool flag2 = buffer->GetStopFlag();
    AIUT_EXPECT_FALSE_INFO_LOG(flag2, "GetStopFlag should return false after exchange");
}

// ==================== Static Mode Tests ====================

/**
 * @tc.name  : Test SetStaticMode and GetStaticMode
 * @tc.number: OHAudioBufferBase_StaticMode
 * @tc.desc  : Test SetStaticMode and GetStaticMode.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_StaticMode, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStaticMode(true);
    AIUT_EXPECT_TRUE_INFO_LOG(buffer->GetStaticMode(), "GetStaticMode should return true");
    buffer->SetStaticMode(false);
    AIUT_EXPECT_FALSE_INFO_LOG(buffer->GetStaticMode(), "GetStaticMode should return false");
}

/**
 * @tc.name  : Test BufferEndCallbackSendTimes in static mode
 * @tc.number: OHAudioBufferBase_BufferEndCallbackSendTimes
 * @tc.desc  : Test Increase/Decrease/Reset BufferEndCallbackSendTimes in static mode.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_BufferEndCallbackSendTimes, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStaticMode(true);
    buffer->IncreaseBufferEndCallbackSendTimes();
    AIUT_EXPECT_TRUE_INFO_LOG(buffer->IsNeedSendBufferEndCallback(),
        "IsNeedSendBufferEndCallback should return true");
    buffer->DecreaseBufferEndCallbackSendTimes();
    AIUT_EXPECT_FALSE_INFO_LOG(buffer->IsNeedSendBufferEndCallback(),
        "IsNeedSendBufferEndCallback should return false");
    buffer->IncreaseBufferEndCallbackSendTimes();
    buffer->ResetBufferEndCallbackSendTimes();
    AIUT_EXPECT_FALSE_INFO_LOG(buffer->IsNeedSendBufferEndCallback(),
        "IsNeedSendBufferEndCallback should return false after reset");
}

/**
 * @tc.name  : Test LoopEndCallback in static mode
 * @tc.number: OHAudioBufferBase_LoopEndCallback
 * @tc.desc  : Test SetIsNeedSendLoopEndCallback and IsNeedSendLoopEndCallback in static mode.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_LoopEndCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStaticMode(true);
    buffer->SetIsNeedSendLoopEndCallback(true);
    AIUT_EXPECT_TRUE_INFO_LOG(buffer->IsNeedSendLoopEndCallback(),
        "IsNeedSendLoopEndCallback should return true");
    buffer->SetIsNeedSendLoopEndCallback(false);
    AIUT_EXPECT_FALSE_INFO_LOG(buffer->IsNeedSendLoopEndCallback(),
        "IsNeedSendLoopEndCallback should return false");
}

/**
 * @tc.name  : Test IsFirstFrame in static mode
 * @tc.number: OHAudioBufferBase_IsFirstFrame
 * @tc.desc  : Test SetIsFirstFrame and IsFirstFrame in static mode.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_IsFirstFrame, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStaticMode(true);
    AIUT_EXPECT_TRUE_INFO_LOG(buffer->IsFirstFrame(), "IsFirstFrame should return true after init");
    buffer->SetIsFirstFrame(false);
    AIUT_EXPECT_FALSE_INFO_LOG(buffer->IsFirstFrame(), "IsFirstFrame should return false");
}

/**
 * @tc.name  : Test StaticPlayPosition in static mode
 * @tc.number: OHAudioBufferBase_StaticPlayPosition
 * @tc.desc  : Test SetStaticPlayPosition and GetStaticPlayPosition in static mode.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_StaticPlayPosition, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStaticMode(true);
    buffer->SetStaticPlayPosition(3, 256);
    int64_t loopTimes = 0;
    size_t dataPos = 0;
    buffer->GetStaticPlayPosition(loopTimes, dataPos);
    AIUT_EXPECT_EQ_INFO_LOG(loopTimes, 3, "loopTimes should be 3");
    AIUT_EXPECT_EQ_INFO_LOG(dataPos, 256U, "dataPos should be 256");
}

/**
 * @tc.name  : Test CheckFrozenAndSetLastProcessTime in static mode
 * @tc.number: OHAudioBufferBase_CheckFrozen_NotFrozen
 * @tc.desc  : Test CheckFrozenAndSetLastProcessTime returns false when not frozen.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_CheckFrozen_NotFrozen, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetStaticMode(true);
    bool ret = buffer->CheckFrozenAndSetLastProcessTime(BUFFER_IN_CLIENT);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "CheckFrozen should return false when not frozen");
}

/**
 * @tc.name  : Test CheckFrozenAndSetLastProcessTime not in static mode
 * @tc.number: OHAudioBufferBase_CheckFrozen_NotStatic
 * @tc.desc  : Test CheckFrozenAndSetLastProcessTime returns false when not in static mode.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_CheckFrozen_NotStatic, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    bool ret = buffer->CheckFrozenAndSetLastProcessTime(BUFFER_IN_CLIENT);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "CheckFrozen should return false when not in static mode");
}

// ==================== Session / Holder / Size Tests ====================

/**
 * @tc.name  : Test GetSessionId and SetSessionId
 * @tc.number: OHAudioBufferBase_SessionId
 * @tc.desc  : Test GetSessionId and SetSessionId.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_SessionId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetSessionId(), 0U, "sessionId should be 0");
    int32_t ret = buffer->SetSessionId(TEST_SESSION_ID);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetSessionId failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetSessionId(), TEST_SESSION_ID,
        "sessionId should be 42");
}

/**
 * @tc.name  : Test GetBufferHolder after CreateFromLocal
 * @tc.number: OHAudioBufferBase_GetBufferHolder
 * @tc.desc  : Test GetBufferHolder returns AUDIO_SERVER_SHARED after CreateFromLocal.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetBufferHolder, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetBufferHolder(), AUDIO_SERVER_SHARED,
        "bufferHolder should be AUDIO_SERVER_SHARED");
}

/**
 * @tc.name  : Test GetSizeParameter and GetTotalSizeInFrame
 * @tc.number: OHAudioBufferBase_GetSizeParameter
 * @tc.desc  : Test GetSizeParameter and GetTotalSizeInFrame return correct values.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetSizeParameter, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    uint32_t totalFrame = 0;
    uint32_t bytePerFrame = 0;
    int32_t ret = buffer->GetSizeParameter(totalFrame, bytePerFrame);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetSizeParameter failed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(totalFrame, TEST_TOTAL_SIZE_FRAME, "totalFrame should be 1024");
    AIUT_EXPECT_EQ_INFO_LOG(bytePerFrame, TEST_BYTE_SIZE_PER_FRAME, "bytePerFrame should be 4");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetTotalSizeInFrame(), TEST_TOTAL_SIZE_FRAME,
        "GetTotalSizeInFrame should be 1024");
}

/**
 * @tc.name  : Test GetStreamStatus after init
 * @tc.number: OHAudioBufferBase_GetStreamStatus_Init
 * @tc.desc  : Test GetStreamStatus returns non-null after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetStreamStatus_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    auto *status = buffer->GetStreamStatus();
    AIUT_EXPECT_NE_INFO_LOG(status, nullptr, "GetStreamStatus should return non-null");
}

/**
 * @tc.name  : Test GetStreamStatus with null basicBufferInfo
 * @tc.number: OHAudioBufferBase_GetStreamStatus_Null
 * @tc.desc  : Test GetStreamStatus returns nullptr when not inited.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetStreamStatus_Null, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OHAudioBufferBase buffer(AUDIO_SERVER_SHARED, TEST_TOTAL_SIZE_FRAME, TEST_BYTE_SIZE_PER_FRAME);
    auto *status = buffer.GetStreamStatus();
    AIUT_EXPECT_EQ_INFO_LOG(status, nullptr, "GetStreamStatus should return nullptr");
}

/**
 * @tc.name  : Test GetUnderrunCount and SetUnderrunCount
 * @tc.number: OHAudioBufferBase_UnderrunCount
 * @tc.desc  : Test GetUnderrunCount and SetUnderrunCount.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_UnderrunCount, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetUnderrunCount(), 0U, "underrunCount should be 0");
    bool ret = buffer->SetUnderrunCount(TEST_UNDERRUN_COUNT);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "SetUnderrunCount failed");
    AIUT_EXPECT_EQ_INFO_LOG(buffer->GetUnderrunCount(), TEST_UNDERRUN_COUNT,
        "underrunCount should be 5");
}

/**
 * @tc.name  : Test GetHandleInfo and SetHandleInfo
 * @tc.number: OHAudioBufferBase_HandleInfo
 * @tc.desc  : Test GetHandleInfo and SetHandleInfo.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_HandleInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    buffer->SetHandleInfo(TEST_FRAME_POS_100, TEST_NANO_TIME);
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    bool ret = buffer->GetHandleInfo(frames, nanoTime);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "GetHandleInfo failed");
    AIUT_EXPECT_EQ_INFO_LOG(frames, TEST_FRAME_POS_100, "frames should be 100");
    AIUT_EXPECT_EQ_INFO_LOG(nanoTime, TEST_NANO_TIME, "nanoTime should be 1000000");
}

/**
 * @tc.name  : Test GetInitializationInfo after init
 * @tc.number: OHAudioBufferBase_GetInitializationInfo
 * @tc.desc  : Test GetInitializationInfo returns correct values after init.
 */
HWTEST_F(OHAudioBufferBaseAiut, OHAudioBufferBase_GetInitializationInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto buffer = CreateTestBuffer();
    AIUT_ASSERT_NE_INFO_LOG(buffer, nullptr, "buffer is null");
    auto info = buffer->GetInitializationInfo();
    AIUT_EXPECT_EQ_INFO_LOG(info.bufferHolder, AUDIO_SERVER_SHARED,
        "bufferHolder should be AUDIO_SERVER_SHARED");
    AIUT_EXPECT_EQ_INFO_LOG(info.totalSizeInFrame, TEST_TOTAL_SIZE_FRAME,
        "totalSizeInFrame should be 1024");
    AIUT_EXPECT_EQ_INFO_LOG(info.byteSizePerFrame, TEST_BYTE_SIZE_PER_FRAME,
        "byteSizePerFrame should be 4");
}

}  // namespace AudioStandard
}  // namespace OHOS
