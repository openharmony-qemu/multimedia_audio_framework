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
} // namespace AudioStandard
} // namespace OHOS