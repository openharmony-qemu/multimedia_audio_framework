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

#include "audio_policy_state_monitor.h"
#include "gtest/gtest.h"
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const std::time_t AUDIO_SESSION_TIME_OUT_DURATION_TEST_S = 3; // 3s

static const std::shared_ptr<AudioPolicyStateMonitor> g_audioPolicyStateMonitor =
    DelayedSingleton<AudioPolicyStateMonitor>::GetInstance();

class AudioPolicyStateMonitorTest : public testing::Test {
public:
    // SetUp: Called before each test cases
    void SetUp(void) override
    {
        ASSERT_NE(g_audioPolicyStateMonitor, nullptr);
        ASSERT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), 0);
        for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
            ASSERT_FALSE(g_audioPolicyStateMonitor->idAllocator_[i]);
        }
    }
};

class AudioPolicyStateMonitorCallbackMocker : public AudioPolicyStateMonitorCallback {
public:
    void OnTimeOut() override
    {}
};

/**
 * @tc.name  : OneTimeStateMonitorTest.
 * @tc.desc  : Test for one time state monitor regsitor.
 */
HWTEST_F(AudioPolicyStateMonitorTest, OneTimeStateMonitorTest, TestSize.Level1)
{
    auto cb = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    int32_t cbId = g_audioPolicyStateMonitor->RegisterCallback(
        cb,
        AUDIO_SESSION_TIME_OUT_DURATION_TEST_S,
        CallbackType::ONE_TIME);
    EXPECT_NE(cbId, INVALID_CB_ID);
    g_audioPolicyStateMonitor->UnRegisterCallback(cbId);
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), 0);
}

/**
 * @tc.name  : OneTimeStateMonitorTest.
 * @tc.desc  : Test for repeat state monitor regsitor.
 */
HWTEST_F(AudioPolicyStateMonitorTest, RepeatStateMonitorTest, TestSize.Level1)
{
    auto cb = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    int32_t cbId = g_audioPolicyStateMonitor->RegisterCallback(
        cb,
        AUDIO_SESSION_TIME_OUT_DURATION_TEST_S,
        CallbackType::REPEAT);
    EXPECT_NE(cbId, INVALID_CB_ID);
    g_audioPolicyStateMonitor->UnRegisterCallback(cbId);
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), 0);
}

/**
 * @tc.name  : OneTimeStateMonitorTest.
 * @tc.desc  : Test for repeat state monitor regsitor.
 */
HWTEST_F(AudioPolicyStateMonitorTest, StopStateMonitorSuccessTest, TestSize.Level1)
{
    auto cb = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        int32_t cbId = g_audioPolicyStateMonitor->RegisterCallback(
            cb,
            AUDIO_SESSION_TIME_OUT_DURATION_TEST_S,
            CallbackType::REPEAT);
        EXPECT_NE(cbId, INVALID_CB_ID);
    }

    int32_t targetCbId = 50;
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.count(targetCbId), 1);
    g_audioPolicyStateMonitor->UnRegisterCallback(targetCbId);
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), MAX_CB_ID_NUM - 1);
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.count(targetCbId), 0);


    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        g_audioPolicyStateMonitor->UnRegisterCallback(i);
    }
}

/**
 * @tc.name  : RegisterMaxNumOfStateMonitorTest.
 * @tc.desc  : Test for max num of state monitor regsitor.
 */
HWTEST_F(AudioPolicyStateMonitorTest, RegisterMaxNumOfStateMonitorTest, TestSize.Level1)
{
    auto cb = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        int32_t cbId = g_audioPolicyStateMonitor->RegisterCallback(
            cb,
            AUDIO_SESSION_TIME_OUT_DURATION_TEST_S,
            CallbackType::REPEAT);
        EXPECT_NE(cbId, INVALID_CB_ID);
        EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), i + 1);
    }

    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        g_audioPolicyStateMonitor->UnRegisterCallback(i);
        EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), MAX_CB_ID_NUM - i - 1);
    }
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), 0);
}

/**
 * @tc.name  : RegisterStateMonitorFailWithNullThreadTest.
 * @tc.desc  : Test for Register state monitor failed when no thread.
 */
HWTEST_F(AudioPolicyStateMonitorTest, RegisterStateMonitorFailWithNullThreadTest, TestSize.Level1)
{
    auto cb = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    std::shared_ptr<std::thread> stateMonitorThreadTemp = g_audioPolicyStateMonitor->stateMonitorThread_;
    g_audioPolicyStateMonitor->stateMonitorThread_ = nullptr;
    int32_t cbId = g_audioPolicyStateMonitor->RegisterCallback(
        cb,
        AUDIO_SESSION_TIME_OUT_DURATION_TEST_S,
        CallbackType::ONE_TIME);
    EXPECT_EQ(cbId, INVALID_CB_ID);
    g_audioPolicyStateMonitor->UnRegisterCallback(cbId);
    EXPECT_EQ(g_audioPolicyStateMonitor->monitoredObj_.size(), 0);
    g_audioPolicyStateMonitor->stateMonitorThread_ = stateMonitorThreadTemp;
}

/**
 * @tc.name  : RegisterStateMonitorFailWithNullCbTest.
 * @tc.desc  : Test for Register state monitor failed with null callback.
 */
HWTEST_F(AudioPolicyStateMonitorTest, RegisterStateMonitorFailWithNullCbTest, TestSize.Level1)
{
    int32_t cbId = g_audioPolicyStateMonitor->RegisterCallback(
        nullptr,
        AUDIO_SESSION_TIME_OUT_DURATION_TEST_S,
        CallbackType::ONE_TIME);
    EXPECT_EQ(cbId, INVALID_CB_ID);
}

} // AudioStandardnamespace
} // OHOSnamespace
