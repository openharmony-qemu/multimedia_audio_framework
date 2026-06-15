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

#include "ring_buffer_handler_aiut.h"
#include "aiut_log.h"
#include "util/ring_buffer_handler.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void RingBufferHandlerAiut::SetUpTestCase(void) {}
void RingBufferHandlerAiut::TearDownTestCase(void) {}

void RingBufferHandlerAiut::SetUp(void)
{
    handler_ = std::make_unique<RingBufferHandler>();
}

void RingBufferHandlerAiut::TearDown(void)
{
    handler_.reset();
}

// ==================== Init Tests ====================

/**
 * @tc.name  : Test RingBufferHandler Init with valid params
 * @tc.number: RingBufferHandler_Init_001
 * @tc.desc  : Test RingBufferHandler Init with standard sample rate/channel/format.
 */
HWTEST_F(RingBufferHandlerAiut, RingBufferHandler_Init_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    handler_->Init(TEST_SAMPLE_RATE, TEST_CHANNEL_COUNT, TEST_FORMAT_BYTES);
}

// ==================== WriteDataToRingBuffer Tests ====================

/**
 * @tc.name  : Test WriteDataToRingBuffer with null data
 * @tc.number: RingBufferHandler_WriteData_001
 * @tc.desc  : Test WriteDataToRingBuffer returns error when data is nullptr.
 */
HWTEST_F(RingBufferHandlerAiut, RingBufferHandler_WriteData_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    handler_->Init(TEST_SAMPLE_RATE, TEST_CHANNEL_COUNT, TEST_FORMAT_BYTES);
    int32_t ret = handler_->WriteDataToRingBuffer(nullptr, 0);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "WriteDataToRingBuffer should fail with null data");
}

/**
 * @tc.name  : Test WriteDataToRingBuffer with mismatched dataLen
 * @tc.number: RingBufferHandler_WriteData_002
 * @tc.desc  : Test WriteDataToRingBuffer returns error when dataLen doesn't match perFrameLength.
 */
HWTEST_F(RingBufferHandlerAiut, RingBufferHandler_WriteData_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    handler_->Init(TEST_SAMPLE_RATE, TEST_CHANNEL_COUNT, TEST_FORMAT_BYTES);
    uint8_t data[128] = {0};
    int32_t ret = handler_->WriteDataToRingBuffer(data, sizeof(data));
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "WriteDataToRingBuffer should fail with mismatched dataLen");
}

// ==================== ReadDataFromRingBuffer Tests ====================

/**
 * @tc.name  : Test ReadDataFromRingBuffer with null data
 * @tc.number: RingBufferHandler_ReadData_001
 * @tc.desc  : Test ReadDataFromRingBuffer returns error when data is nullptr.
 */
HWTEST_F(RingBufferHandlerAiut, RingBufferHandler_ReadData_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    handler_->Init(TEST_SAMPLE_RATE, TEST_CHANNEL_COUNT, TEST_FORMAT_BYTES);
    int32_t ret = handler_->ReadDataFromRingBuffer(nullptr, 0);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "ReadDataFromRingBuffer should fail with null data");
}

/**
 * @tc.name  : Test ReadDataFromRingBuffer returns zeros when no data written
 * @tc.number: RingBufferHandler_ReadData_002
 * @tc.desc  : Test ReadDataFromRingBuffer returns zeros when buffer is empty.
 */
HWTEST_F(RingBufferHandlerAiut, RingBufferHandler_ReadData_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    handler_->Init(TEST_SAMPLE_RATE, TEST_CHANNEL_COUNT, TEST_FORMAT_BYTES);
    // perFrameLength = (48000 * 2 / 100) * 2 * 2 = 3840
    uint32_t perFrameLength = (TEST_SAMPLE_RATE * 2 / 100) * TEST_CHANNEL_COUNT * TEST_FORMAT_BYTES;
    std::vector<uint8_t> data(perFrameLength, 0xFF);
    int32_t ret = handler_->ReadDataFromRingBuffer(data.data(), perFrameLength);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "ReadDataFromRingBuffer should succeed");
    // When buffer is empty, data should be zeroed out
    bool allZero = true;
    for (uint32_t i = 0; i < perFrameLength; ++i) {
        if (data[i] != 0) {
            allZero = false;
            break;
        }
    }
    AIUT_EXPECT_TRUE_INFO_LOG(allZero, "ReadDataFromRingBuffer should return zeros when buffer is empty");
}

// ==================== Write and Read Cycle Tests ====================

/**
 * @tc.name  : Test write and read cycle
 * @tc.number: RingBufferHandler_WriteReadCycle_001
 * @tc.desc  : Test write then read returns the same data.
 */
HWTEST_F(RingBufferHandlerAiut, RingBufferHandler_WriteReadCycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    handler_->Init(TEST_SAMPLE_RATE, TEST_CHANNEL_COUNT, TEST_FORMAT_BYTES);
    uint32_t perFrameLength = (TEST_SAMPLE_RATE * 2 / 100) * TEST_CHANNEL_COUNT * TEST_FORMAT_BYTES;
    std::vector<uint8_t> writeData(perFrameLength, 0xAB);
    std::vector<uint8_t> readData(perFrameLength, 0x00);

    int32_t writeRet = handler_->WriteDataToRingBuffer(writeData.data(), perFrameLength);
    AIUT_EXPECT_EQ_INFO_LOG(writeRet, SUCCESS, "WriteDataToRingBuffer should succeed");

    int32_t readRet = handler_->ReadDataFromRingBuffer(readData.data(), perFrameLength);
    AIUT_EXPECT_EQ_INFO_LOG(readRet, SUCCESS, "ReadDataFromRingBuffer should succeed");

    bool dataMatch = (memcmp(writeData.data(), readData.data(), perFrameLength) == 0);
    AIUT_EXPECT_TRUE_INFO_LOG(dataMatch, "Read data should match written data");
}

} // namespace AudioStandard
} // namespace OHOS
