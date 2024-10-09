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
#include <gtest/gtest.h>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
std::unique_ptr<LinearPosTimeModel> linearPosTimeModel;
std::unique_ptr<AudioSharedMemory> audioSharedMemory;
std::shared_ptr<OHAudioBuffer> oHAudioBuffer;
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
    AudioPlaybackCaptureConfig config = {{{STREAM_USAGE_MUSIC}, FilterMode::INCLUDE, {0}, FilterMode::INCLUDE}, false};
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
    linearPosTimeModel = std::make_unique<LinearPosTimeModel>();

    uint64_t posInFrame = 20;
    int64_t invalidTime = -1;
    int64_t retPos = linearPosTimeModel->GetTimeOfPos(posInFrame);
    EXPECT_EQ(invalidTime, retPos);

    int32_t sampleRate = -1;
    bool isConfig = linearPosTimeModel->ConfigSampleRate(sampleRate);
    EXPECT_EQ(false, isConfig);

    sampleRate = (int32_t)AudioSamplingRate::SAMPLE_RATE_44100;
    isConfig = linearPosTimeModel->ConfigSampleRate(sampleRate);
    EXPECT_EQ(true, isConfig);

    isConfig = linearPosTimeModel->ConfigSampleRate(sampleRate);
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
    linearPosTimeModel->ResetFrameStamp(frame, nanoTime);

    uint64_t spanCountInFrame = 2;
    linearPosTimeModel->SetSpanCount(spanCountInFrame);

    uint64_t posInFrame = 20;
    int64_t retPos = linearPosTimeModel->GetTimeOfPos(posInFrame);

    deltaFrame = posInFrame - frame;
    int64_t retPosCal1 = nanoTime + deltaFrame * NANO_COUNT_PER_SECOND / (int64_t)AudioSamplingRate::SAMPLE_RATE_44100;
    EXPECT_EQ(retPos, retPosCal1);

    frame = 40;
    nanoTime = 50;
    linearPosTimeModel->UpdataFrameStamp(frame, nanoTime);

    retPos = linearPosTimeModel->GetTimeOfPos(posInFrame);
    deltaFrame = frame - posInFrame;
    int64_t retPosCal2 = nanoTime + deltaFrame * NANO_COUNT_PER_SECOND / (int64_t)AudioSamplingRate::SAMPLE_RATE_44100;
    EXPECT_NE(retPos, retPosCal2);
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
    oHAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_EQ(nullptr, oHAudioBuffer);
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
    oHAudioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    EXPECT_NE(nullptr, oHAudioBuffer);

    uint32_t totalSizeInFrameRet;
    uint32_t spanSizeInFrameRet;
    uint32_t byteSizePerFrameRet;

    int32_t ret = oHAudioBuffer->GetSizeParameter(totalSizeInFrameRet, spanSizeInFrameRet, byteSizePerFrameRet);
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
    oHAudioBuffer->SetHandleInfo(frames, nanoTime);
    bool ret = oHAudioBuffer->GetHandleInfo(frames, nanoTime);
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

    ret = oHAudioBuffer->ResetCurReadWritePos(readFrame, writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = oHAudioBuffer->GetAvailableDataFrames();
    EXPECT_EQ(TEST_NUM, ret);

    writeFrame = 1001;
    readFrame = 1000;

    ret = oHAudioBuffer->ResetCurReadWritePos(readFrame, writeFrame);
    EXPECT_EQ(SUCCESS, ret);

    ret = oHAudioBuffer->GetAvailableDataFrames();
    EXPECT_EQ(TEST_NUM - 1, ret);

    uint64_t writeFrameRet = oHAudioBuffer->GetCurWriteFrame();
    uint64_t readFrameRet = oHAudioBuffer->GetCurReadFrame();
    EXPECT_EQ(writeFrame, writeFrameRet);
    EXPECT_EQ(readFrame, readFrameRet);

    writeFrame = 5000;
    ret = oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    writeFrame = readFrame - 1;
    ret = oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    writeFrame = 1000;
    ret = oHAudioBuffer->SetCurWriteFrame(writeFrame);
    EXPECT_LT(ret, TEST_RET_NUM);

    writeFrame = 3000 + 2;
    ret = oHAudioBuffer->SetCurWriteFrame(writeFrame);
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
    ret = oHAudioBuffer->SetCurReadFrame(writeFrame);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    uint64_t readFrame = 1000;
    ret = oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_EQ(SUCCESS, ret);

    readFrame = 1000;
    ret = oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_EQ(SUCCESS, ret);

    readFrame = 2000;
    ret = oHAudioBuffer->SetCurReadFrame(readFrame);
    EXPECT_LT(ret, TEST_RET_NUM);

    readFrame = 3000 + 2;
    ret = oHAudioBuffer->SetCurReadFrame(readFrame);
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

    ret = oHAudioBuffer->GetBufferByFrame(posInFrame, bufferDesc);
    EXPECT_EQ(SUCCESS, ret);

    posInFrame = 3000 + 1;
    ret = oHAudioBuffer->GetBufferByFrame(posInFrame, bufferDesc);
    EXPECT_LT(ret, TEST_RET_NUM);

    uint64_t writePosInFrame = 1000;
    ret = oHAudioBuffer->GetWriteBuffer(writePosInFrame, bufferDesc);
    EXPECT_EQ(SUCCESS, ret);

    writePosInFrame = 3000 +1;
    ret = oHAudioBuffer->GetWriteBuffer(writePosInFrame, bufferDesc);
    EXPECT_LT(ret, TEST_RET_NUM);

    uint64_t readPosInFrame = 1000;
    ret = oHAudioBuffer->GetReadbuffer(readPosInFrame, bufferDesc);
    EXPECT_EQ(SUCCESS, ret);

    readPosInFrame = 3000;
    ret = oHAudioBuffer->GetReadbuffer(readPosInFrame, bufferDesc);
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
    SpanInfo *spanInfo = oHAudioBuffer->GetSpanInfo(posInFrame);
    EXPECT_EQ(NULL, spanInfo);

    uint32_t spanIndex = 2;
    SpanInfo *spanInfoFromIndex = oHAudioBuffer->GetSpanInfoByIndex(spanIndex);
    EXPECT_EQ(NULL, spanInfoFromIndex);

    uint32_t spanCount = oHAudioBuffer->GetSpanCount();
    uint32_t spanCountExpect = 1;
    EXPECT_EQ(spanCountExpect, spanCount);


    size_t totalSize = oHAudioBuffer->GetDataSize();
    EXPECT_EQ(totalSize > TEST_RET_NUM, true);

    uint8_t * dataBase = oHAudioBuffer->GetDataBase();
    EXPECT_NE(nullptr, dataBase);
}

/**
* @tc.name  : Test OHAudioBuffer API
* @tc.type  : FUNC
* @tc.number: OHAudioBuffer_008
* @tc.desc  : Test OHAudioBuffer interface.
*/
HWTEST(AudioServiceCommonUnitTest, OHAudioBuffer_008, TestSize.Level1)
{
    MessageParcel parcel;

    int32_t ret = oHAudioBuffer->WriteToParcel(oHAudioBuffer, parcel);
    EXPECT_EQ(SUCCESS, ret);

    oHAudioBuffer = oHAudioBuffer->ReadFromParcel(parcel);
    EXPECT_NE(nullptr, oHAudioBuffer);
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
} // namespace AudioStandard
} // namespace OHOS