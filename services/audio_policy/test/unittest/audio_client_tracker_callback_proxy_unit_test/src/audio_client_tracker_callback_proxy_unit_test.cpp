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

#include "audio_client_tracker_callback_proxy_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {


void AudioClientTrackerCallbackProxyUnitTest::SetUpTestCase(void) {}
void AudioClientTrackerCallbackProxyUnitTest::TearDownTestCase(void) {}
void AudioClientTrackerCallbackProxyUnitTest::SetUp(void) {}
void AudioClientTrackerCallbackProxyUnitTest::TearDown(void) {}


/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_001
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_001, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();
    ASSERT_TRUE(listener != nullptr);
    std::weak_ptr<AudioClientTrackerTest> callback = std::make_shared<AudioClientTrackerTest>();
    listener->SetClientTrackerCallback(callback);

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    ASSERT_TRUE(clientTrackerCallbackListener != nullptr);

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->MuteStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_002
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_002, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->MuteStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_003
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_003, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->UnmuteStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_004
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_004, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->UnmuteStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_005
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_005, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->PausedStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_006
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_006, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->PausedStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_007
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_007, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->ResumeStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_008
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_008, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    StreamSetStateEventInternal streamSetStateEventInternal;
    clientTrackerCallbackListener->ResumeStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_009
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_009, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    float volume = 0.0f;
    clientTrackerCallbackListener->SetLowPowerVolumeImpl(volume);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_010
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_010, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    float volume = 0.0f;
    clientTrackerCallbackListener->SetLowPowerVolumeImpl(volume);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_011
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_011, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    float volume = 0.0f;
    clientTrackerCallbackListener->GetSingleStreamVolumeImpl(volume);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_012
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_012, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    float volume = 0.0f;
    clientTrackerCallbackListener->GetSingleStreamVolumeImpl(volume);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_013
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_013, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    int32_t state = 0;
    bool isAppBack = true;
    clientTrackerCallbackListener->SetOffloadModeImpl(state, isAppBack);
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_014
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_014, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    int32_t state = 0;
    bool isAppBack = true;
    clientTrackerCallbackListener->SetOffloadModeImpl(state, isAppBack);
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_015
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_015, TestSize.Level1)
{
    //std::make_shared<AudioClientTrackerCallbackStub>();
    sptr<AudioClientTrackerCallbackStub> listener = new AudioClientTrackerCallbackStub();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);

    clientTrackerCallbackListener->UnsetOffloadModeImpl();
    EXPECT_NE(clientTrackerCallbackListener->listener_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackProxy.
 * @tc.number: AudioClientTrackerCallbackProxy_016
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackProxyUnitTest, AudioClientTrackerCallbackProxy_016, TestSize.Level1)
{
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    clientTrackerCallbackListener->listener_ = nullptr;

    clientTrackerCallbackListener->UnsetOffloadModeImpl();
    EXPECT_NE(clientTrackerCallbackListener, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS