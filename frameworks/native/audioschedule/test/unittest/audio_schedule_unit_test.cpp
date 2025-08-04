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

#include "audio_schedule.h"
#include "audio_schedule_guard.h"

#include <pthread.h>
#include <sched.h>
#include <chrono>
#include <cstdint>

#include <gtest/gtest.h>
#include "parameter.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

const int32_t HIGH_LEVEL_THREAD_PRIORITY = 4;
const int32_t AUDIO_DEFAULT_PRIORITY = 1;

int32_t GetIntParameter(const char* key, int32_t defaultValue)
{
    return defaultValue;
}

void AudioInfoLog(const char* format, ...) {}
void AudioErrLog(const char* format, ...) {}

class AudioScheduleUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp(){};
    virtual void TearDown() {}
};

/**
 * @tc.name   : Test AudioScheduleUnit
 * @tc.number : AudioScheduleUnitTest_001
 * @tc.desc   : Test AudioScheduleUnitTest_001
 */
HWTEST_F(AudioScheduleUnitTest, AudioScheduleUnitTest_001, TestSize.Level1)
{
    SetProcessDataThreadPriority(1);
    GetIntParameter("const.multimedia.audio_setPriority", AUDIO_DEFAULT_PRIORITY);
    EXPECT_TRUE(SetEndpointThreadPriority());
}

/**
 * @tc.name   : Test AudioScheduleUnit
 * @tc.number : AudioScheduleUnitTest_002
 * @tc.desc   : Test AudioScheduleUnitTest_002
 */
HWTEST_F(AudioScheduleUnitTest, AudioScheduleUnitTest_002, TestSize.Level1)
{
    SetProcessDataThreadPriority(4);
    GetIntParameter("const.multimedia.audio_setPriority", HIGH_LEVEL_THREAD_PRIORITY);
    EXPECT_TRUE(SetEndpointThreadPriority());
}

/**
 * @tc.name   : Test AudioScheduleUnit
 * @tc.number : AudioScheduleUnitTest_003
 * @tc.desc   : Test AudioScheduleUnitTest_003
 */
 HWTEST_F(AudioScheduleUnitTest, AudioScheduleUnitTest_003, TestSize.Level3)
 {
     SetProcessDataThreadPriority(-1);
     GetIntParameter("const.multimedia.audio_setPriority", HIGH_LEVEL_THREAD_PRIORITY);
     EXPECT_TRUE(SetEndpointThreadPriority());
 }

/**
 * @tc.name   : Test ResetProcessDataThreadPriority
 * @tc.number : ResetProcessDataThreadPriorityTest_001
 * @tc.desc   : Test ResetProcessDataThreadPriority
 */
HWTEST_F(AudioScheduleUnitTest, ResetProcessDataThreadPriorityTest_002, TestSize.Level1)
{
    ResetProcessDataThreadPriority();
    GetIntParameter("const.multimedia.audio_setPriority", HIGH_LEVEL_THREAD_PRIORITY);
    EXPECT_TRUE(SetEndpointThreadPriority());
}

/**
 * @tc.name   : Test ResetEndpointThreadPriority
 * @tc.number : ResetEndpointThreadPriorityTest_001
 * @tc.desc   : Test ResetEndpointThreadPriority
 */
HWTEST_F(AudioScheduleUnitTest, ResetEndpointThreadPriorityTest_002, TestSize.Level1)
{
    EXPECT_TRUE(ResetEndpointThreadPriority());
}
} // namespace AudioStandard
} // namespace OHOS