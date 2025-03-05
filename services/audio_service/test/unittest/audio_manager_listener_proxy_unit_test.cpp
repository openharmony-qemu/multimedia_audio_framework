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

#include <gtest/gtest.h>
#include "audio_manager_listener_proxy.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

sptr<IRemoteObject> impl = new RemoteObjectTestStub();

class AudioManagerListenerProxyUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioManagerListenerProxyUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioManagerListenerProxyUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioManagerListenerProxyUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioManagerListenerProxyUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_001.
 * @tc.desc  : Test OnAudioParameterChange.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_001, TestSize.Level1)
{
    std::string networkId = "test";
    std::string condition = "condition";
    std::string value = 0;

    sptr<IStandardAudioServerManagerListener> listener = new AudioManagerListenerProxy(impl);
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(listener);
    audioManagerListenerCallback->OnAudioParameterChange(networkId, AudioParamKey::A2DP_OFFLOAD_STATE,
        condition, value);
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_002.
 * @tc.desc  : Test OnAudioParameterChange.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_002, TestSize.Level1)
{
    std::string networkId = "test";
    std::string condition = "condition";
    std::string value = 0;

    sptr<IStandardAudioServerManagerListener> listener = nullptr;
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(nullptr);
    audioManagerListenerCallback->OnAudioParameterChange(networkId, AudioParamKey::A2DP_OFFLOAD_STATE,
        condition, value);
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_003.
 * @tc.desc  : Test OnCapturerState.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_003, TestSize.Level1)
{
    sptr<IStandardAudioServerManagerListener> listener = new AudioManagerListenerProxy(impl);
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(listener);
    audioManagerListenerCallback->OnCapturerState(true);
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_004.
 * @tc.desc  : Test OnCapturerState.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_004, TestSize.Level1)
{
    sptr<IStandardAudioServerManagerListener> listener = nullptr;
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(listener);
    audioManagerListenerCallback->OnCapturerState(true);
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_005.
 * @tc.desc  : Test OnWakeupClose.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_005, TestSize.Level1)
{
    sptr<IStandardAudioServerManagerListener> listener = new AudioManagerListenerProxy(impl);
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(listener);
    audioManagerListenerCallback->OnWakeupClose();
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_006.
 * @tc.desc  : Test OnWakeupClose.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_006, TestSize.Level1)
{
    sptr<IStandardAudioServerManagerListener> listener = nullptr;
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(listener);
    audioManagerListenerCallback->OnWakeupClose();
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}

/**
 * @tc.name  : Test AudioManagerListenerProxy
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerProxy_007.
 * @tc.desc  : Test TrigerFirstOnCapturerStateCallback.
 */
HWTEST(AudioManagerListenerProxyUnitTest, AudioManagerListenerProxy_007, TestSize.Level1)
{
    sptr<IStandardAudioServerManagerListener> listener = new AudioManagerListenerProxy(impl);
    auto audioManagerListenerCallback = std::make_shared<AudioManagerListenerCallback>(listener);
    audioManagerListenerCallback->TrigerFirstOnCapturerStateCallback(true);
    ASSERT_TRUE(listener != nullptr);

    audioManagerListenerCallback->isFirstOnCapturerStateCallbackSent_ = true;
    audioManagerListenerCallback->TrigerFirstOnCapturerStateCallback(true);
    ASSERT_TRUE(audioManagerListenerCallback != nullptr);
}
} // namespace AudioStandard
} // namespace OHOS