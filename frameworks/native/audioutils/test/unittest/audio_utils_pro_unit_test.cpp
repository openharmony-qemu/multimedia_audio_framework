/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <thread>
#include <gtest/gtest.h>
#include "audio_utils.h"
#include "parameter.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {

class AudioUtilsProUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioUtilsProUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioUtilsProUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioUtilsProUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioUtilsProUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test SolePipe API
 * @tc.type  : FUNC
 * @tc.number: SolePipe_001
 * @tc.desc  : Test SolePipe API.
 */
HWTEST(AudioUtilsProUnitTest, SolePipe_001, TestSize.Level1)
{
    int32_t sourceType = 10086;
    uint32_t routeFlag = 20192;
    std::string pipeName = "pipeName";
    SolePipe::SetSolePipeSourceInfo(sourceType, routeFlag, pipeName);

    int32_t testSource = 10010;
    uint32_t testFlag = 20020;
    std::string testPipe = "testPipe";

    bool ret = SolePipe::IsSolePipeSource(testSource);
    EXPECT_EQ(ret, false);
    ret = SolePipe::GetSolePipeBySourceType(testSource, testFlag, testPipe);
    EXPECT_EQ(ret, false);

    testSource = 10086;
    ret = SolePipe::IsSolePipeSource(testSource);
    EXPECT_EQ(ret, true);
    ret = SolePipe::GetSolePipeBySourceType(testSource, testFlag, testPipe);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(testFlag, routeFlag);
    EXPECT_EQ(testPipe, pipeName);
}

} // namespace AudioStandard
} // namespace OHOS
