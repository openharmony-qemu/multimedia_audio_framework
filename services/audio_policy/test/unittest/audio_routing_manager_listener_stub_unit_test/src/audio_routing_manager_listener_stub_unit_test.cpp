/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_routing_manager_listener_stub_unit_test.h"
#include "i_standard_audio_routing_manager_listener.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {


void AudioRoutingManagerListenerStubUnitTest::SetUpTestCase(void) {}
void AudioRoutingManagerListenerStubUnitTest::TearDownTestCase(void) {}
void AudioRoutingManagerListenerStubUnitTest::SetUp(void) {}
void AudioRoutingManagerListenerStubUnitTest::TearDown(void) {}


/**
 * @tc.name  : Test AudioRoutingManagerListenerStub.
 * @tc.number: AudioRoutingManagerListenerStub_001
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioRoutingManagerListenerStubUnitTest, AudioRoutingManagerListenerStub_001, TestSize.Level1)
{
    auto audioRoutingManagerListenerStub_ = std::make_shared<AudioRoutingManagerListenerStub>();
    uint32_t code =
        IStandardAudioRoutingManagerListener::AudioRingerModeUpdateListenerMsg::ON_DISTRIBUTED_ROUTING_ROLE_CHANGE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioRoutingManagerListenerStub::GetDescriptor());
    int32_t result = audioRoutingManagerListenerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);

    code =
        IStandardAudioRoutingManagerListener::AudioRingerModeUpdateListenerMsg::ON_AUDIO_OUTPUT_DEVICE_REFINERD;
    result = audioRoutingManagerListenerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioRoutingManagerListenerStub.
 * @tc.number: AudioRoutingManagerListenerStub_002
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioRoutingManagerListenerStubUnitTest, AudioRoutingManagerListenerStub_002, TestSize.Level1)
{
    auto audioRoutingManagerListenerStub_ = std::make_shared<AudioRoutingManagerListenerStub>();
    uint32_t code =
        IStandardAudioRoutingManagerListener::AudioRingerModeUpdateListenerMsg::ON_AUDIO_OUTPUT_DEVICE_REFINERD;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioRoutingManagerListenerStub::GetDescriptor());
    int32_t result = audioRoutingManagerListenerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioRoutingManagerListenerStub.
 * @tc.number: AudioRoutingManagerListenerStub_003
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioRoutingManagerListenerStubUnitTest, AudioRoutingManagerListenerStub_003, TestSize.Level1)
{
    auto audioRoutingManagerListenerStub_ = std::make_shared<AudioRoutingManagerListenerStub>();
    uint32_t code =
        IStandardAudioRoutingManagerListener::AudioRingerModeUpdateListenerMsg::ON_AUDIO_INPUT_DEVICE_REFINERD;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioRoutingManagerListenerStub::GetDescriptor());
    int32_t result = audioRoutingManagerListenerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioRoutingManagerListenerStub.
 * @tc.number: AudioRoutingManagerListenerStub_005
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioRoutingManagerListenerStubUnitTest, AudioRoutingManagerListenerStub_005, TestSize.Level1)
{
    auto audioRoutingManagerListenerStub_ = std::make_shared<AudioRoutingManagerListenerStub>();
    uint32_t code = IStandardAudioRoutingManagerListener::AudioRingerModeUpdateListenerMsg::ON_ERROR;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioRoutingManagerListenerStub::GetDescriptor());
    int32_t result = audioRoutingManagerListenerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioRoutingManagerListenerStub.
 * @tc.number: AudioRoutingManagerListenerStub_006
 * @tc.desc  : Test OnAudioInputDeviceRefinedInternal.
 */
HWTEST(AudioRoutingManagerListenerStubUnitTest, AudioRoutingManagerListenerStub_006, TestSize.Level1)
{
    auto audioRoutingManagerListenerStub_ = std::make_shared<AudioRoutingManagerListenerStub>();
    MessageParcel data;
    data.WriteInt32(1);
    MessageParcel reply;
    audioRoutingManagerListenerStub_->OnAudioInputDeviceRefinedInternal(data, reply);
    EXPECT_NE(audioRoutingManagerListenerStub_, nullptr);
}

/**
 * @tc.name  : Test AudioRoutingManagerListenerStub.
 * @tc.number: AudioRoutingManagerListenerStub_007
 * @tc.desc  : Test OnAudioOutputDeviceRefinedInternal.
 */
HWTEST(AudioRoutingManagerListenerStubUnitTest, AudioRoutingManagerListenerStub_007, TestSize.Level1)
{
    auto audioRoutingManagerListenerStub_ = std::make_shared<AudioRoutingManagerListenerStub>();
    MessageParcel data;
    data.WriteInt32(1);
    MessageParcel reply;
    audioRoutingManagerListenerStub_->OnAudioOutputDeviceRefinedInternal(data, reply);
    EXPECT_NE(audioRoutingManagerListenerStub_, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
