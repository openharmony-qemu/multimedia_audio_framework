/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hdi_utils_ringbuffer_unit_test.h"


using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void HdiUtilsRingBufferUnitTest::SetUpTestCase(void) { }

void HdiUtilsRingBufferUnitTest::TearDownTestCase(void) { }

void HdiUtilsRingBufferUnitTest::SetUp(void) { }

void HdiUtilsRingBufferUnitTest::TearDown(void) { }

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_001
* @tc.desc  : Test HdiRingBuffer::~HdiRingBuffer() When ringBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_001, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->ringBuffer_.data = new uint8_t[1];
    EXPECT_NE(ptrHdiRingBuffer->ringBuffer_.data, nullptr);

    ptrHdiRingBuffer->~HdiRingBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->ringBuffer_.data, nullptr);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_002
* @tc.desc  : Test HdiRingBuffer::~HdiRingBuffer() When ringBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_002, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->ringBuffer_.data = nullptr;
    EXPECT_EQ(ptrHdiRingBuffer->ringBuffer_.data, nullptr);

    ptrHdiRingBuffer->~HdiRingBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->ringBuffer_.data, nullptr);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_003
* @tc.desc  : Test HdiRingBuffer::~HdiRingBuffer() When outputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_003, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->outputBuffer_.data = new uint8_t[1];
    EXPECT_NE(ptrHdiRingBuffer->outputBuffer_.data, nullptr);

    ptrHdiRingBuffer->~HdiRingBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->outputBuffer_.data, nullptr);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_004
* @tc.desc  : Test HdiRingBuffer::~HdiRingBuffer() When outputBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_004, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->outputBuffer_.data = nullptr;
    EXPECT_EQ(ptrHdiRingBuffer->outputBuffer_.data, nullptr);

    ptrHdiRingBuffer->~HdiRingBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->outputBuffer_.data, nullptr);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_005
* @tc.desc  : Test HdiRingBuffer::~HdiRingBuffer() When inputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_005, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->inputBuffer_.data = new uint8_t[1];
    EXPECT_NE(ptrHdiRingBuffer->inputBuffer_.data, nullptr);

    ptrHdiRingBuffer->~HdiRingBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->inputBuffer_.data, nullptr);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_006
* @tc.desc  : Test HdiRingBuffer::~HdiRingBuffer() When inputBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_006, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->inputBuffer_.data = nullptr;
    EXPECT_EQ(ptrHdiRingBuffer->inputBuffer_.data, nullptr);

    ptrHdiRingBuffer->~HdiRingBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->inputBuffer_.data, nullptr);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_007
* @tc.desc  : Test HdiRingBuffer::Init() When ringBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_007, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t sampleRate = 1;
    const int32_t channelCount = 1;
    const int32_t formatBytes = 1;
    const int32_t onceFrameNum = 1;
    const int32_t maxFrameNum = 1;
    ptrHdiRingBuffer->ringBuffer_.data = new uint8_t[1];
    EXPECT_NE(ptrHdiRingBuffer->ringBuffer_.data, nullptr);

    ptrHdiRingBuffer->Init(sampleRate, channelCount, formatBytes, onceFrameNum, maxFrameNum);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_008
* @tc.desc  : Test HdiRingBuffer::Init() When ringBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_008, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t sampleRate = 1;
    const int32_t channelCount = 1;
    const int32_t formatBytes = 1;
    const int32_t onceFrameNum = 1;
    const int32_t maxFrameNum = 1;
    ptrHdiRingBuffer->ringBuffer_.data = nullptr;

    ptrHdiRingBuffer->Init(sampleRate, channelCount, formatBytes, onceFrameNum, maxFrameNum);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_009
* @tc.desc  : Test HdiRingBuffer::Init() When outputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_009, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t sampleRate = 1;
    const int32_t channelCount = 1;
    const int32_t formatBytes = 1;
    const int32_t onceFrameNum = 1;
    const int32_t maxFrameNum = 1;
    ptrHdiRingBuffer->outputBuffer_.data = new uint8_t[1];
    EXPECT_NE(ptrHdiRingBuffer->outputBuffer_.data, nullptr);

    ptrHdiRingBuffer->Init(sampleRate, channelCount, formatBytes, onceFrameNum, maxFrameNum);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_010
* @tc.desc  : Test HdiRingBuffer::Init() When outputBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_010, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t sampleRate = 1;
    const int32_t channelCount = 1;
    const int32_t formatBytes = 1;
    const int32_t onceFrameNum = 1;
    const int32_t maxFrameNum = 1;
    ptrHdiRingBuffer->outputBuffer_.data = nullptr;

    ptrHdiRingBuffer->Init(sampleRate, channelCount, formatBytes, onceFrameNum, maxFrameNum);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_011
* @tc.desc  : Test HdiRingBuffer::Init() When inputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_011, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t sampleRate = 1;
    const int32_t channelCount = 1;
    const int32_t formatBytes = 1;
    const int32_t onceFrameNum = 1;
    const int32_t maxFrameNum = 1;
    ptrHdiRingBuffer->inputBuffer_.data = new uint8_t[1];
    EXPECT_NE(ptrHdiRingBuffer->inputBuffer_.data, nullptr);

    ptrHdiRingBuffer->Init(sampleRate, channelCount, formatBytes, onceFrameNum, maxFrameNum);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_012
* @tc.desc  : Test HdiRingBuffer::Init() When inputBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_012, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t sampleRate = 1;
    const int32_t channelCount = 1;
    const int32_t formatBytes = 1;
    const int32_t onceFrameNum = 1;
    const int32_t maxFrameNum = 1;
    ptrHdiRingBuffer->inputBuffer_.data = nullptr;

    ptrHdiRingBuffer->Init(sampleRate, channelCount, formatBytes, onceFrameNum, maxFrameNum);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_013
* @tc.desc  : Test HdiRingBuffer::GetRingBufferStatus() When readFull_ Is True
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_013, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = true;

    auto result = ptrHdiRingBuffer->GetRingBufferStatus();
    EXPECT_EQ(result, RingBufferState::RINGBUFFER_EMPTY);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_014
* @tc.desc  : Test HdiRingBuffer::GetRingBufferStatus() When  readFull_ Is False And writeFull_ Is True
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_014, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = false;
    ptrHdiRingBuffer->writeFull_ = true;

    auto result = ptrHdiRingBuffer->GetRingBufferStatus();
    EXPECT_EQ(result, RingBufferState::RINGBUFFER_FULL);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_015
* @tc.desc  : Test HdiRingBuffer::GetRingBufferStatus() When  readFull_ Is False And writeFull_ Is False
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_015, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = false;
    ptrHdiRingBuffer->writeFull_ = false;

    auto result = ptrHdiRingBuffer->GetRingBufferStatus();
    EXPECT_EQ(result, RingBufferState::RINGBUFFER_HALFFULL);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_016
* @tc.desc  : Test HdiRingBuffer::GetRingBufferDataLen() When case Is RINGBUFFER_EMPTY
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_016, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = true;
    ptrHdiRingBuffer->writeFull_ = false;

    auto result = ptrHdiRingBuffer->GetRingBufferDataLen();
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_017
* @tc.desc  : Test HdiRingBuffer::GetRingBufferDataLen() When case Is RINGBUFFER_FULL
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_017, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = false;
    ptrHdiRingBuffer->writeFull_ = true;
    ptrHdiRingBuffer->maxBufferSize_ = 2;

    auto result = ptrHdiRingBuffer->GetRingBufferDataLen();
    EXPECT_EQ(result, ptrHdiRingBuffer->maxBufferSize_);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_018
* @tc.desc  : Test HdiRingBuffer::GetRingBufferDataLen() When case Is RINGBUFFER_HALFFULL
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_018, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = false;
    ptrHdiRingBuffer->writeFull_ = false;
    ptrHdiRingBuffer->maxBufferSize_ = 2;
    ptrHdiRingBuffer->writeIndex_ = 6;
    ptrHdiRingBuffer->readIndex_ = 3;

    auto result = ptrHdiRingBuffer->GetRingBufferDataLen();
    EXPECT_EQ(result, ptrHdiRingBuffer->writeIndex_ - ptrHdiRingBuffer->readIndex_);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_019
* @tc.desc  : Test HdiRingBuffer::AddWriteIndex() When (writeIndex_ + length) == maxBufferSize_
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_019, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t length = 0;
    ptrHdiRingBuffer->writeIndex_ = 5;
    ptrHdiRingBuffer->maxBufferSize_ = 5;

    ptrHdiRingBuffer->AddWriteIndex(length);
    EXPECT_EQ(ptrHdiRingBuffer->writeFull_, true);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_020
* @tc.desc  : Test HdiRingBuffer::AddWriteIndex() When (writeIndex_ + length) != maxBufferSize_
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_020, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t length = 0;
    ptrHdiRingBuffer->writeIndex_ = 5;
    ptrHdiRingBuffer->maxBufferSize_ = 0;

    ptrHdiRingBuffer->AddWriteIndex(length);
    EXPECT_EQ(ptrHdiRingBuffer->writeFull_, false);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_021
* @tc.desc  : Test HdiRingBuffer::AddReadIndex() When (readIndex_ + length) == maxBufferSize_
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_021, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t length = 0;
    ptrHdiRingBuffer->readIndex_ = 5;
    ptrHdiRingBuffer->maxBufferSize_ = 5;

    ptrHdiRingBuffer->AddReadIndex(length);
    EXPECT_EQ(ptrHdiRingBuffer->readFull_, true);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_022
* @tc.desc  : Test HdiRingBuffer::AddReadIndex() When (readIndex_ + length) != maxBufferSize_
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_022, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    const int32_t length = 0;
    ptrHdiRingBuffer->readIndex_ = 5;
    ptrHdiRingBuffer->maxBufferSize_ = 0;

    ptrHdiRingBuffer->AddReadIndex(length);
    EXPECT_EQ(ptrHdiRingBuffer->readFull_, false);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_023
* @tc.desc  : Test HdiRingBuffer::AcquireOutputBuffer() When outputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_023, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->outputBuffer_.data = new uint8_t[1]{3};
    ptrHdiRingBuffer->perFrameLength_ = 1;
    EXPECT_NE(ptrHdiRingBuffer->outputBuffer_.data, nullptr);

    ptrHdiRingBuffer->AcquireOutputBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->outputBuffer_.data[0], 0);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_024
* @tc.desc  : Test HdiRingBuffer::AcquireOutputBuffer() When outputBuffer_.data Is Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_024, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->outputBuffer_.data = nullptr;

    ptrHdiRingBuffer->AcquireOutputBuffer();
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_025
* @tc.desc  : Test HdiRingBuffer::DequeueInputBuffer() When inputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_025, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->inputBuffer_.data = nullptr;

    ptrHdiRingBuffer->DequeueInputBuffer();
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_026
* @tc.desc  : Test HdiRingBuffer::ReleaseOutputBuffer()
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_026, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->maxBufferSize_ = 5;
    ptrHdiRingBuffer->readIndex_ = 0;
    ptrHdiRingBuffer->perFrameLength_ = 0;
    ptrHdiRingBuffer->readFull_ = false;
    RingBuffer item;

    auto result = ptrHdiRingBuffer->ReleaseOutputBuffer(item);
    EXPECT_EQ(result, ERR_READ_BUFFER);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_027
* @tc.desc  : Test HdiRingBuffer::ReleaseOutputBuffer()
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_027, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->maxBufferSize_ = 5;
    ptrHdiRingBuffer->readIndex_ = 0;
    ptrHdiRingBuffer->perFrameLength_ = 0;
    ptrHdiRingBuffer->readFull_ = true;
    RingBuffer item;

    auto result = ptrHdiRingBuffer->ReleaseOutputBuffer(item);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_028
* @tc.desc  : Test HdiRingBuffer::ReleaseOutputBuffer()
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_028, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->maxBufferSize_ = 0;
    ptrHdiRingBuffer->readIndex_ = 5;
    ptrHdiRingBuffer->perFrameLength_ = 0;
    ptrHdiRingBuffer->readFull_ = true;
    RingBuffer item;

    auto result = ptrHdiRingBuffer->ReleaseOutputBuffer(item);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_029
* @tc.desc  : Test HdiRingBuffer::EnqueueInputBuffer()
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_029, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = false;
    ptrHdiRingBuffer->writeFull_ = true;
    ptrHdiRingBuffer->maxBufferSize_ = 5;
    RingBuffer item;

    auto result = ptrHdiRingBuffer->EnqueueInputBuffer(item);
    EXPECT_EQ(result, ERR_WRITE_BUFFER);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_030
* @tc.desc  : Test HdiRingBuffer::EnqueueInputBuffer()
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_030, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->readFull_ = true;
    ptrHdiRingBuffer->maxBufferSize_ = 5;
    RingBuffer item;

    auto result = ptrHdiRingBuffer->EnqueueInputBuffer(item);
    EXPECT_EQ(result, ERR_WRITE_BUFFER);
}

/**
* @tc.name  : Test HdiRingBuffer API
* @tc.number: HdiRingBuffer_031
* @tc.desc  : Test HdiRingBuffer::DequeueInputBuffer() When inputBuffer_.data Is Not Null
*/
HWTEST(HdiUtilsRingBufferUnitTest, HdiRingBuffer_031, TestSize.Level0)
{
    std::shared_ptr<HdiRingBuffer> ptrHdiRingBuffer = std::make_shared<HdiRingBuffer>();
    ASSERT_NE(ptrHdiRingBuffer, nullptr);

    ptrHdiRingBuffer->inputBuffer_.data = new uint8_t[1]{3};
    ptrHdiRingBuffer->perFrameLength_ = 1;
    EXPECT_NE(ptrHdiRingBuffer->inputBuffer_.data, nullptr);

    ptrHdiRingBuffer->DequeueInputBuffer();
    EXPECT_EQ(ptrHdiRingBuffer->inputBuffer_.data[0], 0);
}
} // namespace AudioStandard
} // namespace OHOS