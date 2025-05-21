/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "audio_dump_pcm.h"
#include "audio_dump_pcm_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class AudioDumpPcmUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioDumpPcmUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioDumpPcmUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioDumpPcmUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioDumpPcmUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test Init API
* @tc.type  : FUNC
* @tc.number: Init_001
* @tc.desc  : Test Init interface.
*/
HWTEST(AudioDumpPcmUnitTest, Init_001, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.isInited_ = true;
    bool ret = audioCacheMgrInner.Init();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test Init API
* @tc.type  : FUNC
* @tc.number: Init_002
* @tc.desc  : Test Init interface.
*/
HWTEST(AudioDumpPcmUnitTest, Init_002, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.isInited_ = false;
    bool ret = audioCacheMgrInner.Init();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DeInit API
* @tc.type  : FUNC
* @tc.number: DeInit_001
* @tc.desc  : Test DeInit interface.
*/
HWTEST(AudioDumpPcmUnitTest, DeInit_001, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    bool ret = audioCacheMgrInner.DeInit();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DeInit API
* @tc.type  : FUNC
* @tc.number: DeInit_002
* @tc.desc  : Test DeInit interface.
*/
HWTEST(AudioDumpPcmUnitTest, DeInit_002, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.InitCallbackHandler();
    bool ret = audioCacheMgrInner.DeInit();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test DumpAllMemBlock API
* @tc.type  : FUNC
* @tc.number: DumpAllMemBlock_001
* @tc.desc  : Test DumpAllMemBlock interface.
*/
HWTEST(AudioDumpPcmUnitTest, DumpAllMemBlock_001, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.isInited_ = false;
    int32_t ret = audioCacheMgrInner.DumpAllMemBlock();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test DumpAllMemBlock API
* @tc.type  : FUNC
* @tc.number: DumpAllMemBlock_002
* @tc.desc  : Test DumpAllMemBlock interface.
*/
HWTEST(AudioDumpPcmUnitTest, DumpAllMemBlock_002, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.isInited_ = true;
    int32_t ret = audioCacheMgrInner.DumpAllMemBlock();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test DumpAllMemBlock API
* @tc.type  : FUNC
* @tc.number: DumpAllMemBlock_003
* @tc.desc  : Test DumpAllMemBlock interface.
*/
HWTEST(AudioDumpPcmUnitTest, DumpAllMemBlock_003, TestSize.Level1)
{
    AudioCacheMgrInner audioCacheMgrInner;
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    audioCacheMgrInner.isInited_ = true;

    audioCacheMgrInner.memChunkDeque_.push_back(memChunk);
    int32_t ret = audioCacheMgrInner.DumpAllMemBlock();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test CacheData API
* @tc.type  : FUNC
* @tc.number: CacheData_001
* @tc.desc  : Test CacheData interface.
*/
HWTEST(AudioDumpPcmUnitTest, CacheData_001, TestSize.Level1)
{
    std::string dumpFileName;
    void* srcDataPointer;
    size_t dataLength;
    uint8_t srcBuffer[16] = {0};
    auto audioCacheMgrInner = std::make_shared<AudioCacheMgrInner>();
    ASSERT_TRUE(audioCacheMgrInner != nullptr);

    dumpFileName = "test.txt";
    srcDataPointer = static_cast<void *>(srcBuffer);
    dataLength = 8;
    audioCacheMgrInner->isInited_ = false;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);

    audioCacheMgrInner->isInited_ = true;
    audioCacheMgrInner->isDumpingData_ = true;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);

    audioCacheMgrInner->isDumpingData_ = false;
    audioCacheMgrInner->totalMemChunkNums_ = -1;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);

    audioCacheMgrInner->isDumpingData_ = false;
    audioCacheMgrInner->totalMemChunkNums_ = 1;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);
}

/**
* @tc.name  : Test GetCachedDuration API
* @tc.type  : FUNC
* @tc.number: GetCachedDuration_001
* @tc.desc  : Test GetCachedDuration interface.
*/
HWTEST(AudioDumpPcmUnitTest, GetCachedDuration_001, TestSize.Level1)
{
    int64_t startTime = 0;
    int64_t endTime = 0;
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    ASSERT_TRUE(memChunk != nullptr);
    AudioCacheMgrInner audioCacheMgrInner;

    audioCacheMgrInner.isInited_ = false;
    audioCacheMgrInner.GetCachedDuration(startTime, endTime);

    audioCacheMgrInner.isInited_ = true;
    audioCacheMgrInner.GetCachedDuration(startTime, endTime);
    EXPECT_EQ(audioCacheMgrInner.memChunkDeque_.size(), 0);

    audioCacheMgrInner.memChunkDeque_.push_back(memChunk);
    audioCacheMgrInner.memChunkDeque_.push_back(memChunk);
    audioCacheMgrInner.GetCachedDuration(startTime, endTime);
    EXPECT_NE(audioCacheMgrInner.memChunkDeque_.size(), 0);
}

/**
* @tc.name  : Test GetDumpParameter API
* @tc.type  : FUNC
* @tc.number: GetDumpParameter_001
* @tc.desc  : Test GetDumpParameter interface.
*/
HWTEST(AudioDumpPcmUnitTest, GetDumpParameter_001, TestSize.Level1)
{
    std::vector<std::string> subKeys;
    std::vector<std::pair<std::string, std::string>> result;
    bool ret;
    AudioCacheMgrInner audioCacheMgrInner;

    subKeys.push_back(GET_STATUS_KEY);
    ret = audioCacheMgrInner.GetDumpParameter(subKeys, result);
    EXPECT_EQ(ret, true);

    subKeys.clear();
    subKeys.push_back(GET_TIME_KEY);
    ret = audioCacheMgrInner.GetDumpParameter(subKeys, result);
    EXPECT_EQ(ret, true);

    subKeys.clear();
    subKeys.push_back(GET_MEMORY_KEY);
    ret = audioCacheMgrInner.GetDumpParameter(subKeys, result);
    EXPECT_EQ(ret, true);

    subKeys.clear();
    subKeys.push_back("test");
    ret = audioCacheMgrInner.GetDumpParameter(subKeys, result);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SetDumpParameter API
* @tc.type  : FUNC
* @tc.number: SetDumpParameter_001
* @tc.desc  : Test SetDumpParameter interface.
*/
HWTEST(AudioDumpPcmUnitTest, SetDumpParameter_001, TestSize.Level1)
{
    std::vector<std::pair<std::string, std::string>> params;
    bool ret;
    AudioCacheMgrInner audioCacheMgrInner;

    params.push_back(std::make_pair(SET_OPEN_KEY, "test"));
    ret = audioCacheMgrInner.SetDumpParameter(params);
    EXPECT_EQ(ret, true);

    params.clear();
    params.push_back(std::make_pair(SET_CLOSE_KEY, "test"));
    ret = audioCacheMgrInner.SetDumpParameter(params);
    EXPECT_EQ(ret, true);

    params.clear();
    params.push_back(std::make_pair(SET_UPLOAD_KEY, "test"));
    ret = audioCacheMgrInner.SetDumpParameter(params);
    EXPECT_NE(ret, true);

    params.clear();
    params.push_back(std::make_pair("test1", "test"));
    ret = audioCacheMgrInner.SetDumpParameter(params);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test OnHandle API
* @tc.type  : FUNC
* @tc.number: OnHandle_001
* @tc.desc  : Test OnHandle interface.
*/
HWTEST(AudioDumpPcmUnitTest, OnHandle_001, TestSize.Level1)
{
    uint32_t code;
    int64_t data = 0;
    auto audioCacheMgrInner = std::make_shared<AudioCacheMgrInner>();
    ASSERT_TRUE(audioCacheMgrInner != nullptr);

    code = AudioCacheMgrInner::RELEASE_OVERTIME_MEMBLOCK;
    audioCacheMgrInner->OnHandle(code, data);

    code = AudioCacheMgrInner::PRINT_MEMORY_CONDITION;
    audioCacheMgrInner->OnHandle(code, data);

    code = AudioCacheMgrInner::RAISE_PRIORITY;
    audioCacheMgrInner->OnHandle(code, data);

    code = 5;
    audioCacheMgrInner->OnHandle(code, data);
}
} // namespace AudioStandard
} // namespace OHOS
