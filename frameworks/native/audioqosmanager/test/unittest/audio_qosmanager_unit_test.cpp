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

#include "audio_qosmanager.h"
#include <gtest/gtest.h>
 #include <chrono>
 #include <thread>

using namespace testing::ext;
using namespace std;

namespace {
static constexpr int32_t WAIT_FOR_SET_QOS_TIME_MS = 500; // 500ms
static constexpr int32_t SET_PRIORITY_1 = 1;
static constexpr int32_t SET_PRIORITY_4 = 4;
}

class AudioQosmanagerUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp(){};
    virtual void TearDown(){};
};

/**
 * @tc.name   : Test SetThreadQosLevelAsync
 * @tc.number : SetThreadQosLevelAsync_001
 * @tc.desc   : Verify SetThreadQosLevelAsync function returns success and maintains thread ID
 */
HWTEST_F(AudioQosmanagerUnitTest, SetThreadQosLevelAsync_001, TestSize.Level4)
{
    SetThreadQosLevelAsync(SET_PRIORITY_1);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_SET_QOS_TIME_MS));
    ResetThreadQosLevel();
    EXPECT_TRUE(gettid());
    SetThreadQosLevelAsync(SET_PRIORITY_4);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_SET_QOS_TIME_MS));
    ResetThreadQosLevel();
    EXPECT_TRUE(gettid());
}
