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

#include "audio_policy_state_monitor_unit_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyStateMonitorUnitTest::SetUpTestCase(void) {}
void AudioPolicyStateMonitorUnitTest::TearDownTestCase(void) {}
void AudioPolicyStateMonitorUnitTest::SetUp(void) {}
void AudioPolicyStateMonitorUnitTest::TearDown(void) {}

class AudioPolicyStateMonitorCallbackMocker : public AudioPolicyStateMonitorCallback {
public:
    void OnTimeOut() override {}
};

/**
 * @tc.name  : Test AudioPolicyStateMonitor.
 * @tc.number: AudioPolicyStateMonitorUnitTest_001
 * @tc.desc  : Test AudioPolicyStateMonitor interface.
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_001, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);
    audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);
}

/**
 * @tc.name  : Test AudioPolicyStateMonitor.
 * @tc.number: AudioPolicyStateMonitorUnitTest_002
 * @tc.desc  : Test ~AudioPolicyStateMonitor interface.
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_002, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);
    audioPolicyStateMonitor_->stateMonitorThread_->detach();
    EXPECT_EQ(audioPolicyStateMonitor_->stateMonitorThread_->joinable(), false);
}

/**
 * @tc.name  : Test AudioPolicyStateMonitor.
 * @tc.number: AudioPolicyStateMonitorUnitTest_003
 * @tc.desc  : Test RegisterCallback interface.
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_003, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);

    auto callback1 = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();

    std::time_t delayTime_ = 0;
    CallbackType callbackType = ONE_TIME;

    for (int i = 0; i < MAX_CB_ID_NUM; ++i) {
        audioPolicyStateMonitor_->idAllocator_[i] = true;
    }

    int32_t ret = audioPolicyStateMonitor_->RegisterCallback(callback1, delayTime_, callbackType);
    EXPECT_EQ(ret, INVALID_CB_ID);
}

/**
 * @tc.name  : Test AudioPolicyStateMonitor.
 * @tc.number: AudioPolicyStateMonitorUnitTest_004
 * @tc.desc  : Test TraverseAndInvokeTimeoutCallbacks interface. cb == nullptr
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_004, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);

    audioPolicyStateMonitor_->monitoredObj_.clear();

    std::shared_ptr<AudioPolicyStateMonitorCallback> callback1 = nullptr;
    audioPolicyStateMonitor_->monitoredObj_.insert({1, callback1});

    audioPolicyStateMonitor_->TraverseAndInvokeTimeoutCallbacks();
    EXPECT_EQ(audioPolicyStateMonitor_->monitoredObj_.size(), 1);
}

/**
 * @tc.name  : Test AudioPolicyStateMonitor.
 * @tc.number: AudioPolicyStateMonitorUnitTest_005
 * @tc.desc  : Test TraverseAndInvokeTimeoutCallbacks interface. now - cb->startTimeStamp_ >= cb->delayTime_
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_005, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);

    audioPolicyStateMonitor_->monitoredObj_.clear();

    auto callback1 = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    ASSERT_TRUE(callback1 != nullptr);

    callback1->startTimeStamp_ = 0;
    callback1->delayTime_ = 0;
    callback1->callbackType_ = CallbackType::REPEAT;

    audioPolicyStateMonitor_->monitoredObj_.insert({1, callback1});

    audioPolicyStateMonitor_->TraverseAndInvokeTimeoutCallbacks();
    EXPECT_EQ(audioPolicyStateMonitor_->monitoredObj_.size(), 1);
}

/**
 * @tc.name  : Test TraverseAndInvokeTimeoutCallbacks.
 * @tc.number: AudioPolicyStateMonitorUnitTest_006
 * @tc.desc  : Test TraverseAndInvokeTimeoutCallbacks interface. cb->callbackType_ != CallbackType::ONE_TIME
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_006, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);

    audioPolicyStateMonitor_->monitoredObj_.clear();

    auto callback1 = std::make_shared<AudioPolicyStateMonitorCallbackMocker>();
    ASSERT_TRUE(callback1 != nullptr);
    callback1->startTimeStamp_ = 0;
    callback1->delayTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + 1000;
    callback1->callbackType_ = CallbackType::REPEAT;
    audioPolicyStateMonitor_->monitoredObj_.insert({1, callback1});

    audioPolicyStateMonitor_->TraverseAndInvokeTimeoutCallbacks();
    EXPECT_EQ(audioPolicyStateMonitor_->monitoredObj_.size(), 1);
}

/**
 * @tc.name  : Test AllocateCbId.
 * @tc.number: AudioPolicyStateMonitorUnitTest_007
 * @tc.desc  : Test AllocateCbId interface.
 */
HWTEST_F(AudioPolicyStateMonitorUnitTest, AudioPolicyStateMonitorUnitTest_007, TestSize.Level4)
{
    std::shared_ptr<AudioPolicyStateMonitor> audioPolicyStateMonitor_ = std::make_shared<AudioPolicyStateMonitor>();
    ASSERT_TRUE(audioPolicyStateMonitor_ != nullptr);

    for (int i = 0; i < MAX_CB_ID_NUM; ++i) {
        audioPolicyStateMonitor_->idAllocator_[i] = true;
    }

    int32_t ret = audioPolicyStateMonitor_->AllocateCbId();
    EXPECT_EQ(ret, INVALID_CB_ID);
}
} // namespace AudioStandard
} // namespace OHOS