/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "AudioCollaborationManagerUnitTest"
#endif

#include "audio_collaboration_manager_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_rotation.h"
#include "audio_errors.h"
#include "audio_effect_chain.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
static constexpr int32_t DEFAULT_LATENCY_TEST = 205;
}

void AudioCollaborationManagerUnitTest::SetUpTestCase(void) {}
void AudioCollaborationManagerUnitTest::TearDownTestCase(void) {}
void AudioCollaborationManagerUnitTest::SetUp(void) {}
void AudioCollaborationManagerUnitTest::TearDown(void) {}

/**
* @tc.name   : Test updateCollaborativeProductId API
* @tc.number : updateCollaborativeProductId_001
* @tc.desc   : Test updateCollaborativeProductId interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, updateCollaborativeProductId_001, TestSize.Level1)
{
    std::string productId = "11_123456";

    AudioCollaborationManagerUnitTest::GetInstance()->updateCollaborativeProductId(productId);
    EXPECT_EQ(AudioCollaborationManagerUnitTest::GetInstance()->productId_, "11");

    productId = "00014B_07_4113";
    AudioCollaborationManagerUnitTest::GetInstance()->updateCollaborativeProductId(productId);
    EXPECT_EQ(AudioCollaborationManagerUnitTest::GetInstance()->productId_, "00014B");
}

/**
* @tc.name   : Test updateLatencyInner API
* @tc.number : updateLatencyInner_001
* @tc.desc   : Test updateLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, updateLatencyInner_001, TestSize.Level1)
{
    AudioCollaborationManagerUnitTest::GetInstance()->LoadCollaborationConfig();


    AudioCollaborationManagerUnitTest::GetInstance()->productId_ = "00014B0";
    AudioCollaborationManagerUnitTest::GetInstance()->updateLatencyInner();
    EXPECT_EQ(AudioCollaborationManagerUnitTest::GetInstance()->latencyMs_, DEFAULT_LATENCY_TEST);

    AudioCollaborationManagerUnitTest::GetInstance()->twsMode_ = TWS_MODE_OTHERS;
    AudioCollaborationManagerUnitTest::GetInstance()->updateLatencyInner();
    EXPECT_EQ(AudioCollaborationManagerUnitTest::GetInstance()->latencyMs_, DEFAULT_LATENCY_TEST);

    AudioCollaborationManagerUnitTest::GetInstance()->productId_ = "00014B";
    AudioCollaborationManagerUnitTest::GetInstance()->twsMode_ = TWS_MODE_DEFAULT;
    AudioCollaborationManagerUnitTest::GetInstance()->updateLatencyInner();
}

} // namespace AudioStandard
} // namespace OHOS