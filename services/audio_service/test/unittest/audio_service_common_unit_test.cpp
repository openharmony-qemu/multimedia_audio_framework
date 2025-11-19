/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_info.h"
#include "audio_ring_cache.h"
#include "audio_process_config.h"
#include "linear_pos_time_model.h"
#include "oh_audio_buffer.h"
#include "va_shared_buffer.h"
#include "va_shared_buffer_operator.h"
#include <thread>
#include <gtest/gtest.h>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
std::unique_ptr<LinearPosTimeModel> g_linearPosTimeModel;
std::shared_ptr<OHAudioBuffer> g_oHAudioBuffer;
const int32_t TEST_NUM = 1000;
const int32_t TEST_RET_NUM = 0;
const int64_t NANO_COUNT_PER_SECOND = 1000000000;
class AudioServiceCommonUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioServiceCommonUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioServiceCommonUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioServiceCommonUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioServiceCommonUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test ProcessConfig API
 * @tc.type  : FUNC
 * @tc.number: ProcessConfigTest_001
 * @tc.desc  : Test ProcessConfig test.
 */
HWTEST(AudioServiceCommonUnitTest, ProcessConfigTest_001, TestSize.Level1)
{
    CaptureFilterOptions filterOptions = {{STREAM_USAGE_MUSIC}, FilterMode::INCLUDE, {0}, FilterMode::INCLUDE};
    AudioPlaybackCaptureConfig config = {filterOptions, false};
    std::string dumpStr = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_NE(dumpStr, "");
}

/**
* @tc.name  : Test LinearPosTimeModel API
* @tc.type  : FUNC
* @tc.number: LinearPosTimeModel_001
* @tc.desc  : Test LinearPosTimeModel interface.
*/
HWTEST(AudioServiceCommonUnitTest, LinearPosTimeModel_001, TestSize.Level1)
{
    g_linearPosTimeModel = std::make_unique<LinearPosTimeModel>();

    uint64_t posInFrame = 20;
    int64_t invalidTime = -1;
    int64_t retPos = g_linearPosTimeModel->GetTimeOfPos(posInFrame);
    EXPECT_EQ(invalidTime, retPos);

    int32_t sampleRate = -1;
    bool isConfig = g_linearPosTimeModel->ConfigSampleRate(sampleRate);
    EXPECT_EQ(false, isConfig);

    sampleRate = (int32_t)AudioSamplingRate::SAMPLE_RATE_44100;
    isConfig = g_linearPosTimeModel->ConfigSampleRate(sampleRate);
    EXPECT_EQ(true, isConfig);

    isConfig = g_linearPosTimeModel->ConfigSampleRate(sampleRate);
    EXPECT_EQ(false, isConfig);
}

/**
* @tc.name  : Test LinearPosTimeModel API
* @tc.type  : FUNC
* @tc.number: LinearPosTimeModel_002
* @tc.desc  : Test LinearPosTimeModel interface.
*/
HWTEST(AudioServiceCommonUnitTest, LinearPosTimeModel_002, TestSize.Level1)
{
    int64_t deltaFrame = 0;
    uint64_t frame = 0;
    int64_t nanoTime = 0;
    g_linearPosTimeModel->ResetFrameStamp(frame, nanoTime);

    uint64_t spanCountInFrame = 2;
    g_linearPosTimeModel->SetSpanCount(spanCountInFrame);

    uint64_t posInFrame = 20;
    int64_t retPos = g_linearPosTimeModel->GetTimeOfPos(posInFrame);

    deltaFrame = posInFrame - frame;
    int64_t retPosCal1 = nanoTime + deltaFrame * NANO_COUNT_PER_SECOND / (int64_t)AudioSamplingRate::SAMPLE_RATE_44100;
    EXPECT_EQ(retPos, retPosCal1);

    frame = 40;
    nanoTime = 50;
    g_linearPosTimeModel->UpdataFrameStamp(frame, nanoTime);

    retPos = g_linearPosTimeModel->GetTimeOfPos(posInFrame);
    deltaFrame = frame - posInFrame;
    int64_t retPosCal2 = nanoTime + deltaFrame * NANO_COUNT_PER_SECOND / (int64_t)AudioSamplingRate::SAMPLE_RATE_44100;
    EXPECT_NE(retPos, retPosCal2);
}

/**
* @tc.name  : Test CheckPosTimeReasonable API
* @tc.type  : FUNC
* @tc.number: CheckPosTimeReasonable
* @tc.desc  : Test CheckPosTimeReasonable interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckPosTimeReasonable_001, TestSize.Level1)
{
    std::pair<uint64_t, int64_t> pre = std::make_pair(10, 100);
    std::pair<uint64_t, int64_t> next = std::make_pair(5, 50);
    bool ret = g_linearPosTimeModel->CheckPosTimeReasonable(pre, next);

    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test CheckPosTimeReasonable API
* @tc.type  : FUNC
* @tc.number: CheckPosTimeReasonable
* @tc.desc  : Test CheckPosTimeReasonable interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckPosTimeReasonable_002, TestSize.Level1)
{
    std::pair<uint64_t, int64_t> pre = std::make_pair(10, 100);
    std::pair<uint64_t, int64_t> next = std::make_pair(11, 50);
    bool ret = g_linearPosTimeModel->CheckPosTimeReasonable(pre, next);

    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test CheckPosTimeReasonable API
* @tc.type  : FUNC
* @tc.number: CheckPosTimeReasonable
* @tc.desc  : Test CheckPosTimeReasonable interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckPosTimeReasonablel_003, TestSize.Level1)
{
    std::pair<uint64_t, int64_t> pre = std::make_pair(10, 100);
    std::pair<uint64_t, int64_t> next = std::make_pair(10, 50);
    bool ret = g_linearPosTimeModel->CheckPosTimeReasonable(pre, next);

    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test CheckReasonable API
* @tc.type  : FUNC
* @tc.number: CheckReasonable
* @tc.desc  : Test CheckReasonable interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckReasonable_001, TestSize.Level1)
{
    uint64_t frame = 100;
    int64_t nanoTime = 1000000;
    static constexpr int32_t maxCount = 5;
    for (int i = 0; i < maxCount - 1; ++i) {
        g_linearPosTimeModel->posTimeVec_.push_back(std::make_pair(frame + i, nanoTime + i));
    }
    CheckPosTimeRes result = g_linearPosTimeModel->CheckReasonable(frame + maxCount - 1, nanoTime + maxCount - 1);
    EXPECT_EQ(result, NEED_MODIFY);

    for (int i = 0; i < maxCount; ++i) {
        g_linearPosTimeModel->posTimeVec_.push_back(std::make_pair(frame + i, nanoTime + i));
    }
    result = g_linearPosTimeModel->CheckReasonable(frame + maxCount, nanoTime + maxCount);
    EXPECT_EQ(result, CHECK_FAILED);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_001
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame - 1;
    uint32_t byteSizePerFrame = 1000;
    g_oHAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_EQ(nullptr, g_oHAudioBuffer);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_002
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_002, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 1000;
    g_oHAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_NE(nullptr, g_oHAudioBuffer);

    uint32_t totalSizeInFrameRet;
    uint32_t spanSizeInFrameRet;
    uint32_t byteSizePerFrameRet;

    int32_t ret = g_oHAudioBuffer->GetSizeParameter(totalSizeInFrameRet, spanSizeInFrameRet, byteSizePerFrameRet);
    EXPECT_EQ(spanSizeInFrame, spanSizeInFrameRet);
    EXPECT_EQ(totalSizeInFrame, totalSizeInFrameRet);
    EXPECT_EQ(byteSizePerFrame, byteSizePerFrameRet);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_003
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_003, TestSize.Level1)
{
    uint64_t frames = 1000;
    int64_t nanoTime = NANO_COUNT_PER_SECOND;
    g_oHAudioBuffer->SetHandleInfo(frames, nanoTime);
    bool ret = g_oHAudioBuffer->GetHandleInfo(frames, nanoTime);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_004
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_004, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t writeFrame = 3000;
    uint64_t readFrame = writeFrame - 1001;

    ret = g_oHAudioBuffer->ResetCurReadWritePos(readFrame, writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = g_oHAudioBuffer->GetWritableDataFrames();
    EXPECT_EQ(TEST_NUM, ret);

    writeFrame = 1001;
    readFrame = 1000;

    ret = g_oHAudioBuffer->ResetCurReadWritePos(readFrame, writeFrame);
    EXPECT_EQ(SUCCESS, ret);

    ret = g_oHAudioBuffer->GetWritableDataFrames();
    EXPECT_EQ(TEST_NUM - 1, ret);

    uint64_t writeFrameRet = g_oHAudioBuffer->GetCurWriteFrame();
    uint64_t readFrameRet = g_oHAudioBuffer->GetCurReadFrame();
    EXPECT_EQ(writeFrame, writeFrameRet);
    EXPECT_EQ(readFrame, readFrameRet);

    writeFrame = 5000;
    ret = g_oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    writeFrame = readFrame - 1;
    ret = g_oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    writeFrame = 1000;
    ret = g_oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_LT(ret, TEST_RET_NUM);

    writeFrame = 3000 + 2;
    ret = g_oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_LT(ret, TEST_RET_NUM);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_005
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_005, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t writeFrame = 5000;
    ret = g_oHAudioBuffer->SetCurReadFrame(writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    uint64_t readFrame = 1000;
    ret = g_oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_EQ(SUCCESS, ret);

    readFrame = 1000;
    ret = g_oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_EQ(SUCCESS, ret);

    readFrame = 2000;
    ret = g_oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_LT(ret, TEST_RET_NUM);

    readFrame = 3000 + 2;
    ret = g_oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_LT(ret, TEST_RET_NUM);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_006
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_006, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDesc bufferDesc;
    uint64_t posInFrame = 1000;
    uint64_t spanSizeInFrame = 1000;

    RingBufferWrapper ringbufferWrapper;
    ret = g_oHAudioBuffer->ohAudioBufferBase_.GetBufferByFrame(posInFrame, spanSizeInFrame, ringbufferWrapper);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(true, ringbufferWrapper.IsLegal());

    posInFrame = 3000 + 1;
    ret = g_oHAudioBuffer->ohAudioBufferBase_.GetBufferByFrame(posInFrame, spanSizeInFrame, ringbufferWrapper);
    EXPECT_LT(ret, TEST_RET_NUM);

    uint64_t writePosInFrame = 1000;
    ret = g_oHAudioBuffer->GetWriteBuffer(writePosInFrame, bufferDesc);
    EXPECT_EQ(SUCCESS, ret);

    writePosInFrame = 3000 + 1;
    ret = g_oHAudioBuffer->GetWriteBuffer(writePosInFrame, bufferDesc);
    EXPECT_LT(ret, TEST_RET_NUM);

    uint64_t readPosInFrame = 1000;
    ret = g_oHAudioBuffer->GetReadbuffer(readPosInFrame, bufferDesc);
    EXPECT_EQ(SUCCESS, ret);

    readPosInFrame = 3000;
    ret = g_oHAudioBuffer->GetReadbuffer(readPosInFrame, bufferDesc);
    EXPECT_LT(ret, TEST_RET_NUM);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_007
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_007, TestSize.Level1)
{
    uint64_t posInFrame = 4000;
    SpanInfo *spanInfo = g_oHAudioBuffer->GetSpanInfo(posInFrame);
    EXPECT_EQ(NULL, spanInfo);

    uint32_t spanIndex = 2;
    SpanInfo *spanInfoFromIndex = g_oHAudioBuffer->GetSpanInfoByIndex(spanIndex);
    EXPECT_EQ(NULL, spanInfoFromIndex);

    uint32_t spanCount = g_oHAudioBuffer->GetSpanCount();
    uint32_t spanCountExpect = 1;
    EXPECT_EQ(spanCountExpect, spanCount);


    size_t totalSize = g_oHAudioBuffer->GetDataSize();
    EXPECT_EQ(totalSize > TEST_RET_NUM, true);

    uint8_t * dataBase = g_oHAudioBuffer->GetDataBase();
    EXPECT_NE(nullptr, dataBase);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_009
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_009, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_NE(nullptr, ohAudioBuffer);

    int32_t ret = ohAudioBuffer->SetCurWriteFrame(totalSizeInFrame);
    EXPECT_EQ(ret, SUCCESS);

    // 200ms
    FutexCode futexCode = ohAudioBuffer->WaitFor(200000000, [&ohAudioBuffer] () {
        return ohAudioBuffer->GetWritableDataFrames() > 0;
    });
    EXPECT_EQ(futexCode, FUTEX_TIMEOUT);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_010
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_010, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_NE(nullptr, ohAudioBuffer);

    int32_t ret = ohAudioBuffer->SetCurWriteFrame(totalSizeInFrame);
    EXPECT_EQ(ret, SUCCESS);

    std::thread threadSetReadIndex([ohAudioBuffer, totalSizeInFrame] () {
        ohAudioBuffer->SetCurReadFrame(totalSizeInFrame);
    });

    // 200ms
    FutexCode futexCode = ohAudioBuffer->WaitFor(200000000, [&ohAudioBuffer] () {
        return ohAudioBuffer->GetWritableDataFrames() > 0;
    });
    EXPECT_EQ(futexCode, FUTEX_SUCCESS);
    threadSetReadIndex.join();
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBufferBase_001
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1000;
    uint32_t byteSizePerFrame = 100;
    size_t totalSizeInBytes = totalSizeInFrame * byteSizePerFrame;
    auto ohAudioBufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    EXPECT_NE(nullptr, ohAudioBufferBase);

    int32_t ret = ohAudioBufferBase->SetCurWriteFrame(totalSizeInFrame);
    EXPECT_EQ(ret, SUCCESS);

    RingBufferWrapper buffer;
    ret = ohAudioBufferBase->GetAllReadableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, totalSizeInBytes);
    EXPECT_NE(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, totalSizeInBytes);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);

    ret = ohAudioBufferBase->GetAllWritableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, 0);
    EXPECT_EQ(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, 0);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);

    std::thread threadSetReadIndex([ohAudioBufferBase, totalSizeInFrame] () {
        ohAudioBufferBase->SetCurReadFrame(totalSizeInFrame);
    });

    // 200ms
    FutexCode futexCode = ohAudioBufferBase->WaitFor(200000000, [&ohAudioBufferBase] () {
        return ohAudioBufferBase->GetWritableDataFrames() > 0;
    });
    EXPECT_EQ(futexCode, FUTEX_SUCCESS);
    threadSetReadIndex.join();

    ret = ohAudioBufferBase->GetAllReadableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, 0);
    EXPECT_EQ(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, 0);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);

    ret = ohAudioBufferBase->GetAllWritableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, totalSizeInBytes);
    EXPECT_NE(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, totalSizeInBytes);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBufferBase_002
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1000;
    uint32_t byteSizePerFrame = 100;
    size_t totalSizeInBytes = totalSizeInFrame * byteSizePerFrame;
    auto ohAudioBufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    EXPECT_NE(nullptr, ohAudioBufferBase);

    int32_t ret = ohAudioBufferBase->SetCurWriteFrame(totalSizeInFrame - 1);
    EXPECT_EQ(ret, SUCCESS);

    RingBufferWrapper buffer;
    ret = ohAudioBufferBase->GetAllReadableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(true, buffer.IsLegal());
    EXPECT_EQ(buffer.dataLength, totalSizeInBytes - byteSizePerFrame);
    EXPECT_NE(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, totalSizeInBytes - byteSizePerFrame);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);

    ret = ohAudioBufferBase->GetAllWritableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, byteSizePerFrame);
    EXPECT_NE(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, byteSizePerFrame);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);

    std::thread threadSetReadIndex([ohAudioBufferBase, totalSizeInFrame] () {
        ohAudioBufferBase->SetCurReadFrame(totalSizeInFrame - 1);
    });

    // 200ms
    FutexCode futexCode = ohAudioBufferBase->WaitFor(200000000, [&ohAudioBufferBase] () {
        return ohAudioBufferBase->GetWritableDataFrames() > 0;
    });
    EXPECT_EQ(futexCode, FUTEX_SUCCESS);
    threadSetReadIndex.join();

    ret = ohAudioBufferBase->GetAllReadableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, 0);
    EXPECT_EQ(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, 0);
    EXPECT_EQ(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, 0);

    ret = ohAudioBufferBase->GetAllWritableBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(buffer.dataLength, totalSizeInBytes);
    EXPECT_NE(buffer.basicBufferDescs[0].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[0].bufLength, byteSizePerFrame);
    EXPECT_NE(buffer.basicBufferDescs[1].buffer, nullptr);
    EXPECT_EQ(buffer.basicBufferDescs[1].bufLength, totalSizeInBytes - byteSizePerFrame);
}

/**
* @tc.name  : Test GetSyncWriteFrame API
* @tc.type  : FUNC
* @tc.number: GetSyncWriteFrame
* @tc.desc  : Test GetSyncWriteFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, GetSyncWriteFrame_003, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    int32_t ret = ohAudioBuffer->GetSyncWriteFrame();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test GetSynSetSyncWriteFramecWriteFrame API
* @tc.type  : FUNC
* @tc.number: GetSyncWrSetSyncWriteFrameiteFrame
* @tc.desc  : Test SetSyncWriteFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, SetSyncWriteFrame_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    uint32_t writeFrame = 10;
    int32_t ret = ohAudioBuffer->SetSyncWriteFrame(writeFrame);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test GetSyncWriteFrame API
* @tc.type  : FUNC
* @tc.number: GetSyncWriteFrame
* @tc.desc  : Test GetSyncWriteFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, SetMuteFactor_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    float invalidMuteFactor = 0.5f;

    bool result = ohAudioBuffer->SetMuteFactor(invalidMuteFactor);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test GetDuckFactor API
* @tc.type  : FUNC
* @tc.number: GetDuckFactor
* @tc.desc  : Test GetDuckFactor interface.
*/
HWTEST(AudioServiceCommonUnitTest, GetDuckFactor_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    float result = ohAudioBuffer->GetDuckFactor();
    EXPECT_FLOAT_EQ(result, MAX_FLOAT_VOLUME);
}

/**
* @tc.name  : Test SetDuckFactor API
* @tc.type  : FUNC
* @tc.number: SetDuckFactor
* @tc.desc  : Test SetDuckFactor interface.
*/
HWTEST(AudioServiceCommonUnitTest, SetDuckFactor_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    float invalidDuckFactor = -0.1f;
    bool result = ohAudioBuffer->SetDuckFactor(invalidDuckFactor);
    EXPECT_FALSE(result);

    invalidDuckFactor = 1.1f;
    result = ohAudioBuffer->SetDuckFactor(invalidDuckFactor);
    EXPECT_FALSE(result);

    float validDuckFactor = 0.5f;
    result = ohAudioBuffer->SetDuckFactor(validDuckFactor);
    EXPECT_TRUE(result);

    validDuckFactor = 0.0f;
    result = ohAudioBuffer->SetDuckFactor(validDuckFactor);
    EXPECT_TRUE(result);

    float maxDuckFactor = 1.0f;
    result = ohAudioBuffer->SetDuckFactor(maxDuckFactor);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test GetStreamVolume API
* @tc.type  : FUNC
* @tc.number: GetStreamVolume
* @tc.desc  : Test GetStreamVolume interface.
*/
HWTEST(AudioServiceCommonUnitTest, GetStreamVolume_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    float result = ohAudioBuffer->GetStreamVolume();
    EXPECT_FLOAT_EQ(result, MAX_FLOAT_VOLUME);
}

/**
* @tc.name  : Test SetStreamVolume API
* @tc.type  : FUNC
* @tc.number: SetStreamVolume
* @tc.desc  : Test SetStreamVolume interface.
*/
HWTEST(AudioServiceCommonUnitTest, SetStreamVolume_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    bool result = ohAudioBuffer->SetStreamVolume(0.5f);
    EXPECT_TRUE(result);

    float invalidVolume = 1.1f;
    result = ohAudioBuffer->SetStreamVolume(invalidVolume);
    EXPECT_FALSE(result);

    invalidVolume = -0.1f;
    result = ohAudioBuffer->SetStreamVolume(invalidVolume);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test GetSyncReadFrame API
* @tc.type  : FUNC
* @tc.number: GetSyncReadFrame
* @tc.desc  : Test GetSyncReadFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, GetSyncReadFrame_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    bool result = ohAudioBuffer->GetSyncReadFrame();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test SetSyncReadFrame API
* @tc.type  : FUNC
* @tc.number: SetSyncReadFrame
* @tc.desc  : Test SetSyncReadFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, SetSyncReadFrame_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    uint32_t testValue = 123;
    EXPECT_FALSE(ohAudioBuffer->SetSyncReadFrame(testValue));
}

 /**
 * @tc.name  : Test GetMuteFactor API
 * @tc.type  : FUNC
 * @tc.number: GetMuteFactor
 * @tc.desc  : Test GetMuteFactor interface.
 */
HWTEST(AudioServiceCommonUnitTest, GetMuteFactor_001, TestSize.Level4)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_EQ(ohAudioBuffer->GetMuteFactor(), 1);
}

 /**
 * @tc.name  : Test SetRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: SetRestoreStatus
 * @tc.desc  : Test SetRestoreStatus interface.
 */
HWTEST(AudioServiceCommonUnitTest, SetRestoreStatus_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    RestoreStatus result = ohAudioBuffer->SetRestoreStatus(NEED_RESTORE);
    EXPECT_NE(RESTORE_ERROR, result);
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_001
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_001, TestSize.Level1)
{
    size_t testSize = 3840;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(testSize);
    EXPECT_NE(nullptr, ringCache);

    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(testSize);
    std::unique_ptr<uint8_t[]> readBuffer = std::make_unique<uint8_t[]>(testSize);

    BufferWrap writeWrap = {writeBuffer.get(), testSize};
    BufferWrap readWrap = {readBuffer.get(), testSize};

    int32_t tryCount = 200;
    while (tryCount-- > 0) {
        OptResult result1 = ringCache->Enqueue(writeWrap);
        EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

        OptResult result2 = ringCache->Dequeue(readWrap);
        EXPECT_EQ(result2.ret, OPERATION_SUCCESS);
    }
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_002
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_002, TestSize.Level1)
{
    size_t testSize = 3840;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(testSize);
    EXPECT_NE(nullptr, ringCache);

    size_t tempSize = 1920;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);
    std::unique_ptr<uint8_t[]> readBuffer = std::make_unique<uint8_t[]>(tempSize);

    BufferWrap writeWrap = {writeBuffer.get(), tempSize};
    BufferWrap readWrap = {readBuffer.get(), tempSize};

    int32_t tryCount = 200;
    while (tryCount-- > 0) {
        OptResult result1 = ringCache->GetWritableSize();
        EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
        EXPECT_EQ(result1.size, testSize);

        result1 = ringCache->Enqueue(writeWrap); // write 1920
        EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

        result1 = ringCache->GetWritableSize();
        EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
        EXPECT_EQ(result1.size, testSize - tempSize); // left 1920


        OptResult result2 = ringCache->GetReadableSize();
        EXPECT_EQ(result2.ret, OPERATION_SUCCESS);
        EXPECT_EQ(result2.size, tempSize); // can read 1920

        result2 = ringCache->Dequeue(readWrap);
        EXPECT_EQ(result2.ret, OPERATION_SUCCESS);

        result2 = ringCache->GetReadableSize();
        EXPECT_EQ(result2.ret, OPERATION_SUCCESS);
        EXPECT_EQ(result2.size, 0); // can read 0
    }
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_003
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_003, TestSize.Level1)
{
    size_t cacheSize = 960;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(cacheSize);

    size_t tempSize = 19200;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);
    std::unique_ptr<uint8_t[]> readBuffer = std::make_unique<uint8_t[]>(tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        writeBuffer[index] = index;
    }

    int32_t totalCount = tempSize / cacheSize;
    size_t offset = 0;
    while (totalCount-- > 0) {
        uint8_t *writePtr = writeBuffer.get() + offset;
        BufferWrap spanWrap = {writePtr, cacheSize};
        OptResult result1 = ringCache->Enqueue(spanWrap);
        EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

        uint8_t *readPtr = readBuffer.get() + offset;
        BufferWrap readWrap = {readPtr, cacheSize};
        OptResult result2 = ringCache->Dequeue(readWrap);
        EXPECT_EQ(result2.ret, OPERATION_SUCCESS);
        offset += cacheSize;
    }

    for (size_t index = 0; index < tempSize;index++) {
        EXPECT_EQ(writeBuffer[index], readBuffer[index]);
    }
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_004
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_004, TestSize.Level1)
{
    size_t cacheSize = 960;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(cacheSize);

    size_t tempSize = 480;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);
    std::unique_ptr<uint8_t[]> readBuffer = std::make_unique<uint8_t[]>(tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        writeBuffer[index] = index;
    }

    BufferWrap writeWrap = {writeBuffer.get(), tempSize};
    BufferWrap readWrap = {readBuffer.get(), tempSize};

    OptResult result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

    result1 = ringCache->ReConfig(tempSize, true); // test copyRemained is true
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->Dequeue(readWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        EXPECT_EQ(writeBuffer[index], readBuffer[index]);
    }
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_005
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_005, TestSize.Level1)
{
    size_t cacheSize = 960;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(cacheSize);

    size_t tempSize = 480;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);
    std::unique_ptr<uint8_t[]> readBuffer = std::make_unique<uint8_t[]>(tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        writeBuffer[index] = index;
    }

    BufferWrap writeWrap = {writeBuffer.get(), tempSize};
    BufferWrap readWrap = {readBuffer.get(), tempSize};

    OptResult result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

    result1 = ringCache->Dequeue(readWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

    result1 = ringCache->ReConfig(tempSize, true); // test copyRemained is true
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->Dequeue(readWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        EXPECT_EQ(writeBuffer[index], readBuffer[index]);
    }
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_006
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_006, TestSize.Level1)
{
    size_t cacheSize = 960;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(cacheSize);

    size_t tempSize = 480;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        writeBuffer[index] = index;
    }

    BufferWrap writeWrap = {writeBuffer.get(), tempSize};

    OptResult result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

    result1 = ringCache->Dequeue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

    result1 = ringCache->ReConfig(tempSize, false); // test copyRemained is false
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->GetReadableSize();
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, 0);
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_007
* @tc.desc  : Test AudioRingCache interface.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_007, TestSize.Level1)
{
    size_t cacheSize = 480;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(cacheSize);

    size_t tempSize = 480;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        writeBuffer[index] = index;
    }

    BufferWrap writeWrap = {writeBuffer.get(), tempSize};

    OptResult result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

    size_t reSize = tempSize + tempSize;
    result1 = ringCache->ReConfig(reSize, false); // test copyRemained is false
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, reSize);

    result1 = ringCache->GetReadableSize();
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, 0);

    result1 = ringCache->GetWritableSize();
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, reSize);

    result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->GetWritableSize();
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->Enqueue(writeWrap);
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, tempSize);

    result1 = ringCache->GetWritableSize();
    EXPECT_EQ(result1.ret, OPERATION_SUCCESS);
    EXPECT_EQ(result1.size, 0);
}

/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: AudioRingCache_008
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, AudioRingCache_008, TestSize.Level1)
{
    size_t cacheSize = 480;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(cacheSize);

    size_t tempSize = 1920;
    std::unique_ptr<uint8_t[]> writeBuffer = std::make_unique<uint8_t[]>(tempSize);
    std::unique_ptr<uint8_t[]> readBuffer = std::make_unique<uint8_t[]>(tempSize);

    for (size_t index = 0; index < tempSize;index++) {
        writeBuffer[index] = index % UINT8_MAX;
    }

    size_t offset = 0;
    size_t spanSize = 320; // 480 * 2 /3
    int32_t totalCount = tempSize / spanSize;
    while (totalCount-- > 0) {
        uint8_t *writePtr = writeBuffer.get() + offset;
        BufferWrap spanWrap = {writePtr, spanSize};
        OptResult result1 = ringCache->Enqueue(spanWrap);
        EXPECT_EQ(result1.ret, OPERATION_SUCCESS);

        uint8_t *readPtr = readBuffer.get() + offset;
        BufferWrap readWrap = {readPtr, spanSize};
        OptResult result2 = ringCache->Dequeue(readWrap);
        EXPECT_EQ(result2.ret, OPERATION_SUCCESS);
        offset += spanSize;
    }

    for (size_t index = 0; index < tempSize;index++) {
        EXPECT_EQ(writeBuffer[index], readBuffer[index]);
    }
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: DumpInnerCapConfig_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, DumpInnerCapConfig_001, TestSize.Level1)
{
    CaptureFilterOptions filterOptions = {{STREAM_USAGE_MUSIC}, FilterMode::EXCLUDE, {0}, FilterMode::EXCLUDE};
    AudioPlaybackCaptureConfig config = {filterOptions, false};
    std::string dumpStr = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_NE(dumpStr, "");
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: DumpInnerCapConfig_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, DumpInnerCapConfig_002, TestSize.Level1)
{
    CaptureFilterOptions filterOptions = {{STREAM_USAGE_MUSIC},
        FilterMode::MAX_FILTER_MODE, {0}, FilterMode::MAX_FILTER_MODE};
    AudioPlaybackCaptureConfig config = {filterOptions, false};
    std::string dumpStr = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_NE(dumpStr, "");
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: ReadInnerCapConfigFromParcel_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, ReadInnerCapConfigFromParcel_001, TestSize.Level1)
{
    MessageParcel parcel;
    AudioPlaybackCaptureConfig config;

    for (int i = 0; i < 31; i++) {
        config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_MEDIA);
    }
    int ret = config.Marshalling(parcel);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: ReadInnerCapConfigFromParcel_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, ReadInnerCapConfigFromParcel_002, TestSize.Level1)
{
    MessageParcel parcel;
    AudioPlaybackCaptureConfig config;

    for (int i = 0; i < 29; i++) {
        config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT);
    }
    int ret = config.Marshalling(parcel);
    EXPECT_NE(ret, SUCCESS);
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: ReadInnerCapConfigFromParcel_003
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, ReadInnerCapConfigFromParcel_003, TestSize.Level1)
{
    MessageParcel parcel;
    AudioPlaybackCaptureConfig config;

    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_ALARM);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_VOICE_RINGTONE);

    int ret = config.Marshalling(parcel);
    EXPECT_NE(ret, SUCCESS);
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: ReadInnerCapConfigFromParcel_004
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, ReadInnerCapConfigFromParcel_004, TestSize.Level1)
{
    MessageParcel parcel;
    AudioPlaybackCaptureConfig config;

    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_ALARM);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_ENFORCED_TONE);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_INVALID);

    int ret = config.Marshalling(parcel);
    EXPECT_NE(ret, SUCCESS);
}
/**
* @tc.name  : Test AudioRingCache API
* @tc.type  : FUNC
* @tc.number: ReadInnerCapConfigFromParcel_005
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioServiceCommonUnitTest, ReadInnerCapConfigFromParcel_005, TestSize.Level1)
{
    MessageParcel parcel;
    AudioPlaybackCaptureConfig config;

    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_ALARM);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_MEDIA);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_MEDIA);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_MEDIA);
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_ALARM);

    int ret = 0;
    ret = config.Marshalling(parcel);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test LinearPosTimeModel API
* @tc.type  : FUNC
* @tc.number: LinearPosTimeModel_003
* @tc.desc  : Test LinearPosTimeModel interface.
*/
HWTEST(AudioServiceCommonUnitTest, LinearPosTimeModel_003, TestSize.Level1)
{
    auto linearPos = std::make_unique<LinearPosTimeModel>();
    ASSERT_TRUE(linearPos != nullptr);

    uint64_t frame = 0;
    int64_t nanoTime = 0;
    for (int i = 0; i < 10; i++) {
        linearPos->posTimeVec_.push_back(std::make_pair(frame, nanoTime));
        if (frame < 5) {
            frame++;
        }
        nanoTime++;
    }
    linearPos->sampleRate_ = 1;
    auto ret = linearPos->CheckReasonable(frame, nanoTime);
    EXPECT_EQ(ret, CHECK_FAILED);
}

/**
* @tc.name  : Test LinearPosTimeModel API
* @tc.type  : FUNC
* @tc.number: LinearPosTimeModel_004
* @tc.desc  : Test LinearPosTimeModel interface.
*/
HWTEST(AudioServiceCommonUnitTest, LinearPosTimeModel_004, TestSize.Level1)
{
    auto linearPos = std::make_unique<LinearPosTimeModel>();
    ASSERT_TRUE(linearPos != nullptr);

    uint64_t posInFrame = 20;
    linearPos->isConfiged = true;
    linearPos->sampleRate_ = 1;
    auto ret = linearPos->GetTimeOfPos(posInFrame);
    EXPECT_NE(ret, -1);
}

/**
* @tc.name  : Test LinearPosTimeModel API
* @tc.type  : FUNC
* @tc.number: LinearPosTimeModel_005
* @tc.desc  : Test LinearPosTimeModel interface.
*/
HWTEST(AudioServiceCommonUnitTest, LinearPosTimeModel_005, TestSize.Level1)
{
    auto linearPos = std::make_unique<LinearPosTimeModel>();
    ASSERT_TRUE(linearPos != nullptr);

    uint64_t posInFrame = 0;
    linearPos->stampFrame_ = 5;
    linearPos->isConfiged = true;
    linearPos->sampleRate_ = 1;
    auto ret = linearPos->GetTimeOfPos(posInFrame);
    EXPECT_NE(ret, -1);
}

/**
* @tc.name  : Test CheckWriteOrReadFrame API
* @tc.type  : FUNC
* @tc.number: CheckWriteOrReadFrame_001
* @tc.desc  : Test CheckWriteOrReadFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckWriteOrReadFrame_001, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 0;
    EXPECT_FALSE(g_oHAudioBuffer->CheckWriteOrReadFrame(100));
}

/**
* @tc.name  : Test CheckWriteOrReadFrame API
* @tc.type  : FUNC
* @tc.number: CheckWriteOrReadFrame_002
* @tc.desc  : Test CheckWriteOrReadFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckWriteOrReadFrame_002, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 10;
    EXPECT_FALSE(g_oHAudioBuffer->CheckWriteOrReadFrame(15));
}

/**
* @tc.name  : Test CheckWriteOrReadFrame API
* @tc.type  : FUNC
* @tc.number: CheckWriteOrReadFrame_003
* @tc.desc  : Test CheckWriteOrReadFrame interface.
*/
HWTEST(AudioServiceCommonUnitTest, CheckWriteOrReadFrame_003, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 10;
    EXPECT_FALSE(g_oHAudioBuffer->CheckWriteOrReadFrame(20));
}

/**
* @tc.name  : Test SizeCheck API
* @tc.type  : FUNC
* @tc.number: SizeCheck_001
* @tc.desc  : Test SizeCheck interface.
*/
HWTEST(AudioServiceCommonUnitTest, SizeCheck_001, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 0;
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInByte_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanConut_ = 10;
    uint32_t totalSizeFrame = 1000;
    int32_t result = g_oHAudioBuffer->spanBasicInfo_.SizeCheck(totalSizeFrame);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test SizeCheck API
* @tc.type  : FUNC
* @tc.number: SizeCheck_002
* @tc.desc  : Test SizeCheck interface.
*/
HWTEST(AudioServiceCommonUnitTest, SizeCheck_002, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInByte_ = 0;
    g_oHAudioBuffer->spanBasicInfo_.spanConut_ = 10;
    uint32_t totalSizeFrame = 1000;
    int32_t result = g_oHAudioBuffer->spanBasicInfo_.SizeCheck(totalSizeFrame);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test SizeCheck API
* @tc.type  : FUNC
* @tc.number: SizeCheck_003
* @tc.desc  : Test SizeCheck interface.
*/
HWTEST(AudioServiceCommonUnitTest, SizeCheck_003, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInByte_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanConut_ = 10;
    uint32_t totalSizeFrame = 1000;
    int32_t result = g_oHAudioBuffer->spanBasicInfo_.SizeCheck(totalSizeFrame);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test SizeCheck API
* @tc.type  : FUNC
* @tc.number: SizeCheck_004
* @tc.desc  : Test SizeCheck interface.
*/
HWTEST(AudioServiceCommonUnitTest, SizeCheck_004, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInByte_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanConut_ = 10;
    uint32_t totalSizeFrame = 50;
    int32_t result = g_oHAudioBuffer->spanBasicInfo_.SizeCheck(totalSizeFrame);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    uint32_t totalSizeFrame1 = 50;
    int32_t ret = g_oHAudioBuffer->spanBasicInfo_.SizeCheck(totalSizeFrame1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test SizeCheck API
* @tc.type  : FUNC
* @tc.number: SizeCheck_005
* @tc.desc  : Test SizeCheck interface.
*/
HWTEST(AudioServiceCommonUnitTest, SizeCheck_005, TestSize.Level1)
{
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInFrame_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanSizeInByte_ = 100;
    g_oHAudioBuffer->spanBasicInfo_.spanConut_ = 10;
    uint32_t totalSizeFrame = 1000;
    int32_t result = g_oHAudioBuffer->spanBasicInfo_.SizeCheck(totalSizeFrame);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test Unmarshalling API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_Unmarshalling_001
 * @tc.desc  : Test OHAudioBufferBase::Unmarshalling interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_Unmarshalling_001, TestSize.Level4)
{
    Parcel parcel;
    MessageParcel &messageParcel = static_cast<MessageParcel &>(parcel);
    messageParcel.WriteUint32(static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_SHARED) + 1);
    messageParcel.WriteUint32(10);
    messageParcel.WriteUint32(10);
    messageParcel.WriteFileDescriptor(3);
    messageParcel.WriteFileDescriptor(4);

    auto buffer = OHAudioBufferBase::Unmarshalling(parcel);
    EXPECT_EQ(buffer, nullptr);
}

/**
 * @tc.name  : Test CreateFromRemote API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_CreateFromRemote _001
 * @tc.desc  : Test OHAudioBufferBase::CreateFromRemote  interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_CreateFromRemote_001, TestSize.Level4)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioBufferHolder bufferHolder = AUDIO_CLIENT;
    int dataFd = 3;
    int infoFd = 1;

    std::shared_ptr<OHAudioBufferBase> buffer = OHAudioBufferBase::CreateFromRemote(totalSizeInFrame,
        byteSizePerFrame, bufferHolder, dataFd, infoFd);
    EXPECT_EQ(buffer, nullptr);
}

/**
 * @tc.name  : Test GetSyncWriteFrame API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetSyncWriteFrame_001
 * @tc.desc  : Test GetSyncWriteFrame interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetSyncWriteFrame_001, TestSize.Level1)
{
    OHAudioBufferBase audioBufferBase(AUDIO_SERVER_ONLY_WITH_SYNC, 100, 10);
    audioBufferBase.bufferHolder_ = AUDIO_SERVER_ONLY_WITH_SYNC;
    audioBufferBase.syncWriteFrame_ = nullptr;
    EXPECT_EQ(audioBufferBase.GetSyncWriteFrame(), 0);
}

/**
 * @tc.name  : Test GetSyncWriteFrame API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetSyncWriteFrame_002
 * @tc.desc  : Test GetSyncWriteFrame interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetSyncWriteFrame_002, TestSize.Level1)
{
    OHAudioBufferBase audioBufferBase(AUDIO_SERVER_ONLY_WITH_SYNC, 100, 10);
    audioBufferBase.bufferHolder_ = AUDIO_SERVER_ONLY_WITH_SYNC;
    uint32_t syncWriteFrame = 50;
    audioBufferBase.syncWriteFrame_ = &syncWriteFrame;
    EXPECT_EQ(audioBufferBase.GetSyncWriteFrame(), syncWriteFrame);
}

/**
 * @tc.name  : Test SetSyncReadFrame API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_SetSyncReadFrame_001
 * @tc.desc  : Test SetSyncReadFrame interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_SetSyncReadFrame_001, TestSize.Level1)
{
    uint32_t syncWriteFrame = 50;
    OHAudioBufferBase audioBufferBase(AUDIO_SERVER_ONLY_WITH_SYNC, 100, 10);
    audioBufferBase.bufferHolder_ = AUDIO_SERVER_ONLY_WITH_SYNC;
    audioBufferBase.syncReadFrame_ = &syncWriteFrame;
    uint32_t readFrame = 10;
    EXPECT_TRUE(audioBufferBase.SetSyncReadFrame(readFrame));
}

/**
 * @tc.name  : Test GetFutex API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetFutex_001
 * @tc.desc  : Test OHAudioBufferBase::GetFutex() interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetFutex_001, TestSize.Level1)
{
    OHAudioBufferBase audioBufferBase(AUDIO_SERVER_ONLY_WITH_SYNC, 100, 10);
    audioBufferBase.basicBufferInfo_ = nullptr;
    EXPECT_EQ(audioBufferBase.GetFutex(), nullptr);
}

/**
 * @tc.name  : Test SetRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_SetRestoreStatus_001
 * @tc.desc  : Test SetRestoreStatus interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_SetRestoreStatus_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    RestoreStatus result = ohAudioBuffer->SetRestoreStatus(NO_NEED_FOR_RESTORE);
    EXPECT_NE(RESTORING, result);
}

/**
 * @tc.name  : Test GetStreamVolume API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetStreamVolume_001
 * @tc.desc  : Test GetStreamVolume interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetStreamVolume_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ohAudioBuffer->basicBufferInfo_->streamVolume.store(MAX_FLOAT_VOLUME + 0.1);
    float result = ohAudioBuffer->GetStreamVolume();
    EXPECT_FLOAT_EQ(result, MAX_FLOAT_VOLUME);
}

/**
 * @tc.name  : Test GetStreamVolume API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetStreamVolume_002
 * @tc.desc  : Test GetStreamVolume interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetStreamVolume_002, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ohAudioBuffer->basicBufferInfo_->streamVolume.store(MIN_FLOAT_VOLUME - 0.1);
    float result = ohAudioBuffer->GetStreamVolume();
    EXPECT_FLOAT_EQ(result, MIN_FLOAT_VOLUME);
}

/**
 * @tc.name  : Test GetMuteFactor API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetMuteFactor_001
 * @tc.desc  : Test GetMuteFactor interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetMuteFactor_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ohAudioBuffer->basicBufferInfo_->muteFactor.store(MAX_FLOAT_VOLUME + 0.1);
    float result = ohAudioBuffer->GetMuteFactor();
    EXPECT_FLOAT_EQ(result, MAX_FLOAT_VOLUME);
}

/**
 * @tc.name  : Test GetMuteFactor API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetMuteFactor_002
 * @tc.desc  : Test GetMuteFactor interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetMuteFactor_002, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ohAudioBuffer->basicBufferInfo_->muteFactor.store(MIN_FLOAT_VOLUME - 0.1);
    float result = ohAudioBuffer->GetMuteFactor();
    EXPECT_FLOAT_EQ(result, MIN_FLOAT_VOLUME);
}

/**
 * @tc.name  : Test GetDuckFactor API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetDuckFactor_001
 * @tc.desc  : Test GetDuckFactor interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetDuckFactor_001, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ohAudioBuffer->basicBufferInfo_->duckFactor.store(MAX_FLOAT_VOLUME + 0.1);
    float result = ohAudioBuffer->GetDuckFactor();
    EXPECT_FLOAT_EQ(result, MAX_FLOAT_VOLUME);
}

/**
 * @tc.name  : Test GetDuckFactor API
 * @tc.type  : FUNC
 * @tc.number: OHAudioBufferBase_GetDuckFactor_002
 * @tc.desc  : Test GetDuckFactor interface.
 */
HWTEST(AudioServiceCommonUnitTest, OHAudioBufferBase_GetDuckFactor_002, TestSize.Level1)
{
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = 100;
    auto ohAudioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ohAudioBuffer->basicBufferInfo_->duckFactor.store(MIN_FLOAT_VOLUME - 0.1);
    float result = ohAudioBuffer->GetDuckFactor();
    EXPECT_FLOAT_EQ(result, MIN_FLOAT_VOLUME);
}

/**
* @tc.name  : Test GetTimeOfPos API
* @tc.type  : FUNC
* @tc.number: GetTimeOfPos_001
* @tc.desc  : Test GetTimeOfPos interface.
*/
HWTEST(AudioServiceCommonUnitTest, GetTimeOfPos_001, TestSize.Level1)
{
    g_linearPosTimeModel = std::make_unique<LinearPosTimeModel>();

    uint64_t posInFrame = 20;
    int64_t invalidTime = -1;
    g_linearPosTimeModel->stampFrame_ = 0;
    g_linearPosTimeModel->sampleRate_ = 0;
    int64_t retPos = g_linearPosTimeModel->GetTimeOfPos(posInFrame);
    EXPECT_EQ(invalidTime, retPos);
}

/**
* @tc.name  : Test GetTimeOfPos API
* @tc.type  : FUNC
* @tc.number: GetTimeOfPos_002
* @tc.desc  : Test GetTimeOfPos interface.
*/
HWTEST(AudioServiceCommonUnitTest, GetTimeOfPos_002, TestSize.Level1)
{
    g_linearPosTimeModel = std::make_unique<LinearPosTimeModel>();

    uint64_t posInFrame = 1;
    int64_t invalidTime = -1;
    g_linearPosTimeModel->stampFrame_ = 10;
    g_linearPosTimeModel->sampleRate_ = 0;
    int64_t retPos = g_linearPosTimeModel->GetTimeOfPos(posInFrame);
    EXPECT_EQ(invalidTime, retPos);
}

/**
* @tc.name  : Test Init API
* @tc.type  : FUNC
* @tc.number: Init_001
* @tc.desc  : Test Init interface.
*/
HWTEST(AudioServiceCommonUnitTest, Init_001, TestSize.Level1)
{
    const size_t testMaxSize = 16 * 1024 * 1024 + 1;
    size_t testSize = 3840;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(testSize);
    EXPECT_NE(nullptr, ringCache);
    ringCache->cacheTotalSize_ = testMaxSize;
    bool result = ringCache->Init();
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test Init API
* @tc.type  : FUNC
* @tc.number: Init_002
* @tc.desc  : Test Init interface.
*/
HWTEST(AudioServiceCommonUnitTest, Init_002, TestSize.Level1)
{
    size_t testSize = 3840;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(testSize);
    EXPECT_NE(nullptr, ringCache);
    ringCache->cacheTotalSize_ = -1;
    bool result = ringCache->Init();
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test Create API
* @tc.type  : FUNC
* @tc.number: Create_001
* @tc.desc  : Test Create interface.
*/
HWTEST(AudioServiceCommonUnitTest, Create_001, TestSize.Level1)
{
    size_t testSize = 3840;
    size_t cacheSize = 16 * 1024 * 1024 + 1;
    std::unique_ptr<AudioRingCache> ringCache = AudioRingCache::Create(testSize);
    EXPECT_NE(nullptr, ringCache);
    std::unique_ptr<AudioRingCache> result = ringCache->Create(cacheSize);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_001
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_001, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    buffer->GetVASharedMemInfo(memInfo);
    EXPECT_EQ(memInfo.dataMemCapacity_, 1024);
    EXPECT_NE(memInfo.dataFd_, INVALID_FD);
    EXPECT_NE(memInfo.statusMemCapacity_, 0);
    EXPECT_NE(memInfo.statusFd_, INVALID_FD);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_002
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_002, TestSize.Level1)
{
    VASharedMemInfo memInfoInvalid;
    memInfoInvalid.dataFd_ = -1;
    memInfoInvalid.dataMemCapacity_ = -1;
    memInfoInvalid.statusMemCapacity_ = 0;
    memInfoInvalid.statusFd_ = INVALID_FD;
    
    std::shared_ptr<VASharedBuffer> bufferInvalid = VASharedBuffer::CreateFromRemote(memInfoInvalid);
    EXPECT_EQ(nullptr, bufferInvalid);
     
    const uint32_t bufferCapacity = 1024;
    std::shared_ptr<VASharedBuffer> bufferLocal = VASharedBuffer::CreateFromLocal(bufferCapacity);
    EXPECT_NE(nullptr, bufferLocal);

    VASharedMemInfo memInfo;
    bufferLocal->GetVASharedMemInfo(memInfo);

    std::shared_ptr<VASharedBuffer> bufferValid = VASharedBuffer::CreateFromRemote(memInfo);
    EXPECT_NE(nullptr, bufferValid);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_003
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(VAAudioSharedMemoryTest, VASharedBuffer_003, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    std::shared_ptr<VAAudioSharedMemory> sharedMemory_ = VAAudioSharedMemory::CreateFromLocal(1024, "test_memory");
    EXPECT_NE(sharedMemory_, nullptr);

    EXPECT_NE(sharedMemory_->GetBase(), nullptr);
    EXPECT_EQ(sharedMemory_->GetSize(), 1024);
    EXPECT_EQ(sharedMemory_->GetName(), "test_memory");
    EXPECT_NE(sharedMemory_->GetFd(), INVALID_FD);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_004
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_004, TestSize.Level1)
{
    VASharedBuffer sharedBuffer;
    int32_t result = sharedBuffer.SizeCheck();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_005
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_005, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);
    uint8_t *database = buffer->GetDataBase();
    EXPECT_NE(database, nullptr);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_006
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_006, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);

    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    size_t dataSize = buffer->GetDataSize();
    EXPECT_EQ(dataSize, 1024);
}

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_007
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_007, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);
    sptr<Ashmem> ashmem = buffer->GetDataAshmem();
    EXPECT_NE(ashmem, nullptr);
}

/* *
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_008
* @tc.desc  : Test VASharedBuffer interface.

HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_008, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = 0;
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);
    uint8_t *statusInfoBase = buffer->GetStatusInfoBase();
    EXPECT_EQ(statusInfoBase, nullptr);
} */

/**
* @tc.name  : Test VASharedBuffer API
* @tc.type  : FUNC
* @tc.number: VASharedBuffer_009
* @tc.desc  : Test VASharedBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBuffer_009, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedMemInfo retrievedMemInfo;
    buffer->GetVASharedMemInfo(retrievedMemInfo);

    EXPECT_EQ(retrievedMemInfo.dataMemCapacity_, 1024);
    EXPECT_NE(retrievedMemInfo.statusMemCapacity_, 0);
    EXPECT_NE(retrievedMemInfo.dataFd_, INVALID_FD);
    EXPECT_NE(retrievedMemInfo.statusFd_, INVALID_FD);
}

/**
* @tc.name  : Test VASharedBufferOperator API
* @tc.type  : FUNC
* @tc.number: VASharedBufferOperator_001
* @tc.desc  : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_001, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);

    EXPECT_NE(operator_->dataAshmem_, nullptr);
    EXPECT_EQ(operator_->capacity, 1024);
    EXPECT_NE(operator_->statusInfo_, nullptr);

    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_002
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_002, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator *operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);

    operator_->SetMinReadSize(100);
    EXPECT_NE(operator_->minReadSize_, 100);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_003
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_003, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);

    EXPECT_EQ(operator_->GetReadableSize(), 0);
    operator_->Reset();
    EXPECT_EQ(operator_->GetReadableSize(), 0);

    uint8_t testData[50] = {0};
    size_t writeSize = operator_->Write(testData, 50);
    EXPECT_EQ(writeSize, 50);
    EXPECT_EQ(operator_->GetReadableSize(), 50);
    operator_->Reset();
    EXPECT_EQ(operator_->GetReadableSize(), 0);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_004
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_004, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;


    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);
    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);
    size_t readableSize = operator_->GetReadableSize();
    EXPECT_EQ(readableSize, 0);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_005
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_005, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;

    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);
    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);

    uint8_t testData[100] = {0};
    for (int i = 0; i < 100; ++i) {
        testData[i] = static_cast<uint8_t>(i);
    }
    size_t writeSize = operator_->Write(testData, 100);
    EXPECT_EQ(writeSize, 100);

    uint8_t readData[100] = {0};
    size_t readSize = operator_->Read(readData, 100);
    EXPECT_EQ(readSize, 100);

    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(readData[i], static_cast<uint8_t>(i));
    }
    EXPECT_EQ(operator_->GetReadableSize(), 0);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_006
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_006, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);

    EXPECT_EQ(operator_->GetReadableSize(), 0);

    uint8_t testData[50] = {0};
    size_t writeSize = operator_->Write(testData, 50);
    EXPECT_EQ(writeSize, 50);
    EXPECT_EQ(operator_->GetReadableSize(), 50);

    uint8_t readData[30] = {0};
    size_t readSize = operator_->Read(readData, 30);
    EXPECT_EQ(readSize, 30);
    EXPECT_EQ(operator_->GetReadableSize(), 20);
    
    operator_->SetReadPosToWritePos();
    EXPECT_EQ(operator_->GetReadableSize(), 0);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_007
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_007, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);

    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);

    auto futex = operator_->GetFutex();
    EXPECT_NE(futex, nullptr);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_009
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_009, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator* operator_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(operator_, nullptr);
   
    operator_->SetMinReadSize(100);
    bool enoughData = operator_->HasEnoughReadableData();
    EXPECT_FALSE(enoughData);
    delete operator_;
}

/**
* @tc.name   : Test VASharedBufferOperator API
* @tc.type   : FUNC
* @tc.number : VASharedBufferOperator_0010
* @tc.desc   : Test VASharedBufferOperator interface.
*/
HWTEST(AudioServiceCommonUnitTest, VASharedBufferOperator_0010, TestSize.Level1)
{
    std::shared_ptr<VASharedBuffer> buffer = VASharedBuffer::CreateFromLocal(1024);
    EXPECT_NE(buffer, nullptr);
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = 1024;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = sizeof(VASharedStatusInfo);
    memInfo.statusFd_ = INVALID_FD;
    EXPECT_EQ(buffer->Init(memInfo), SUCCESS);

    VASharedBufferOperator* SharedStatusInfo_ = new VASharedBufferOperator(*buffer);
    EXPECT_NE(SharedStatusInfo_, nullptr);
    SharedStatusInfo_->InitVASharedStatusInfo();
    EXPECT_NE(SharedStatusInfo_->statusInfo_, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS