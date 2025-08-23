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

#include "audio_policy_manager_listener_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <memory>
#include <thread>
#include <vector>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class MockStandardAudioPolicyManagerListener : public IStandardAudioPolicyManagerListener {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    MOCK_METHOD(ErrCode, OnInterrupt, (const InterruptEventInternal&), (override));

    ErrCode OnRouteUpdate(uint32_t, const std::string&) override
    {
        return 0;
    }

    ErrCode OnAvailableDeviceChange(uint32_t, const DeviceChangeAction&) override
    {
        return 0;
    }

    ErrCode OnQueryClientType(const std::string&, uint32_t, bool& ret) override
    {
        ret = false;
        return 0;
    }

    ErrCode OnCheckClientInfo(const std::string&, int32_t&, int32_t, bool& ret) override
    {
        ret = false;
        return 0;
    }

    ErrCode OnCheckVKBInfo(const std::string&, bool& isValid) override
    {
        isValid = false;
        return 0;
    }

    ErrCode OnQueryAllowedPlayback(int32_t, int32_t, bool& ret) override
    {
        ret = false;
        return 0;
    }

    ErrCode OnBackgroundMute(int32_t) override
    {
        return 0;
    }

    ErrCode OnQueryBundleNameIsInList(const std::string&, const std::string&, bool& ret) override
    {
        ret = false;
        return 0;
    }

    ErrCode OnQueryDeviceVolumeBehavior(VolumeBehavior&) override
    {
        return 0;
    }
};

void AudioPolicyManagerListenerUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerListenerUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerListenerUnitTest::SetUp(void) {}
void AudioPolicyManagerListenerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioPolicyManagerListener.
 * @tc.number: AudioPolicyManagerListener_001
 * @tc.desc  : Test AudioPolicyManagerListener interface.
 */
HWTEST(AudioPolicyManagerListenerUnitTest, AudioPolicyManagerListener_001, TestSize.Level1)
{
    sptr<IStandardAudioPolicyManagerListener> nullListener = nullptr;
    AudioPolicyManagerListenerCallback cb(nullListener);
    InterruptEventInternal evt{};
    EXPECT_NO_THROW(cb.OnInterrupt(evt));
}

/**
 * @tc.name  : Test AudioPolicyManagerListener.
 * @tc.number: AudioPolicyManagerListener_002
 * @tc.desc  : Test AudioPolicyManagerListener interface.
 */
HWTEST(AudioPolicyManagerListenerUnitTest, AudioPolicyManagerListener_002, TestSize.Level1)
{
    sptr<MockStandardAudioPolicyManagerListener> mock = new MockStandardAudioPolicyManagerListener();
    AudioPolicyManagerListenerCallback cb(mock);
    InterruptEventInternal evt{};
    EXPECT_CALL(*mock, OnInterrupt(::testing::Ref(evt))).Times(1).WillOnce(Return(0));
    cb.OnInterrupt(evt);
}

} // namespace AudioStandard
} // namespace OHOS
