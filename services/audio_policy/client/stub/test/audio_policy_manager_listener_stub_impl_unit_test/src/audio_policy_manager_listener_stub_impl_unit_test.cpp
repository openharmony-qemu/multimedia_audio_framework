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

#include "audio_policy_manager_listener_stub_impl_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <memory>
#include <thread>
#include <vector>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class MockAudioInterruptCallback : public AudioInterruptCallback {
public:
    MOCK_METHOD(void, OnInterrupt, (const InterruptEventInternal&), (override));
};

void AudioPolicyManagerListenerStubImplUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerListenerStubImplUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerListenerStubImplUnitTest::SetUp(void) {}
void AudioPolicyManagerListenerStubImplUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioPolicyManagerListenerStubImpl.
 * @tc.number: AudioPolicyManagerListenerStubImpl_001
 * @tc.desc  : Test AudioPolicyManagerListenerStubImpl OnInterrupt interface.
 */
HWTEST(AudioPolicyManagerListenerStubImplUnitTest, AudioPolicyManagerListenerStubImpl_001, TestSize.Level4)
{
    AudioPolicyManagerListenerStubImpl stub;
    InterruptEventInternal event{};
    int32_t ret = stub.OnInterrupt(event);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerListenerStubImpl.
 * @tc.number: AudioPolicyManagerListenerStubImpl_002
 * @tc.desc  : Test AudioPolicyManagerListenerStubImpl OnInterrupt interface.
 */
HWTEST(AudioPolicyManagerListenerStubImplUnitTest, AudioPolicyManagerListenerStubImpl_002, TestSize.Level4)
{
    AudioPolicyManagerListenerStubImpl stub;
    auto mockCb = std::make_shared<MockAudioInterruptCallback>();
    stub.SetInterruptCallback(std::weak_ptr<AudioInterruptCallback>(mockCb));
    InterruptEventInternal event{};
    EXPECT_CALL(*mockCb, OnInterrupt(_)).Times(1).WillOnce([&](const InterruptEventInternal& arg) { Return(); });
    int32_t ret = stub.OnInterrupt(event);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
