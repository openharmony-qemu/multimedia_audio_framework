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

#include "audio_spatialization_state_change_listener_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <memory>
#include <thread>
#include <vector>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class MockSpatializationStateChangeCallback : public AudioSpatializationStateChangeCallback {
public:
    MOCK_METHOD(void, OnSpatializationStateChange, (const AudioSpatializationState&), (override));
};

void AudioSpatializationStateChangeListenerUnitTest::SetUpTestCase(void) {}
void AudioSpatializationStateChangeListenerUnitTest::TearDownTestCase(void) {}
void AudioSpatializationStateChangeListenerUnitTest::SetUp(void) {}
void AudioSpatializationStateChangeListenerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioSpatializationStateChangeListener.
 * @tc.number: AudioSpatializationStateChangeListener_001
 * @tc.desc  : Test AudioSpatializationStateChangeListener OnSpatializationStateChange interface.
 */
HWTEST(AudioSpatializationStateChangeListenerUnitTest, AudioSpatializationStateChangeListener_001, TestSize.Level4)
{
    AudioSpatializationStateChangeListener listener;
    AudioSpatializationState state{};
    int32_t ret = listener.OnSpatializationStateChange(state);
    EXPECT_EQ(ret, AUDIO_ERR);
}

/**
 * @tc.name  : Test AudioSpatializationStateChangeListener.
 * @tc.number: AudioSpatializationStateChangeListener_002
 * @tc.desc  : Test AudioSpatializationStateChangeListener OnSpatializationStateChange interface.
 */
HWTEST(AudioSpatializationStateChangeListenerUnitTest, AudioSpatializationStateChangeListener_002, TestSize.Level4)
{
    AudioSpatializationStateChangeListener listener;
    auto mock = std::make_shared<MockSpatializationStateChangeCallback>();
    listener.SetCallback(std::weak_ptr<AudioSpatializationStateChangeCallback>(mock));
    AudioSpatializationState state{};
    EXPECT_CALL(*mock, OnSpatializationStateChange(::testing::Ref(state)))
        .Times(1)
        .WillOnce([](const AudioSpatializationState&) { Return(); });
    int32_t ret = listener.OnSpatializationStateChange(state);
    EXPECT_EQ(ret, AUDIO_OK);
}

} // namespace AudioStandard
} // namespace OHOS
