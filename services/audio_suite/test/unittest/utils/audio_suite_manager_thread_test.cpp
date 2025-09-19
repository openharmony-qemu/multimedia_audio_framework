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
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_suite_manager_thread.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

class AudioSuiteUtilsTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(AudioSuiteUtilsTest, SuiteManagerThread_001, TestSize.Level0)
{
    AudioSuiteManagerThread thread;
    EXPECT_FALSE(thread.running_.load());
    EXPECT_EQ(thread.m_audioSuiteManager, nullptr);
    EXPECT_FALSE(thread.recvSignal_.load());
    EXPECT_FALSE(thread.IsMsgProcessing());
}

}  // namespace