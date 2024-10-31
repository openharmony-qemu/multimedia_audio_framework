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

#include "audio_client_tracker_callback_stub_unit_test.h"
#include "i_standard_client_tracker.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_info.h"
#include "audio_stream_info.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {


void AudioClientTrackerCallbackStubUnitTest::SetUpTestCase(void) {}
void AudioClientTrackerCallbackStubUnitTest::TearDownTestCase(void) {}
void AudioClientTrackerCallbackStubUnitTest::SetUp(void) {}
void AudioClientTrackerCallbackStubUnitTest::TearDown(void) {}


/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_001
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_001, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    StreamSetStateEventInternal streamSetStateEventInternal = {};
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::MUTESTREAM;
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_MUTE;
    streamSetStateEventInternal.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioClientTrackerCallbackStub_->SelectCodeCase(code, streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_002
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_002, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    StreamSetStateEventInternal streamSetStateEventInternal = {};
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::PAUSEDSTREAM;
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_PAUSE;
    streamSetStateEventInternal.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioClientTrackerCallbackStub_->SelectCodeCase(code, streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_003
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_003, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    StreamSetStateEventInternal streamSetStateEventInternal = {};
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::RESUMESTREAM;
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_RESUME;
    streamSetStateEventInternal.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioClientTrackerCallbackStub_->SelectCodeCase(code, streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_004
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_004, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    StreamSetStateEventInternal streamSetStateEventInternal = {};
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::UNMUTESTREAM;
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_UNMUTE;
    streamSetStateEventInternal.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioClientTrackerCallbackStub_->SelectCodeCase(code, streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_005
 * @tc.desc  : Test SelectCodeCase.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_005, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    StreamSetStateEventInternal streamSetStateEventInternal = {};
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::ON_ERROR;
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_MUTE;
    streamSetStateEventInternal.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioClientTrackerCallbackStub_->SelectCodeCase(code, streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_006
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_006, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::PAUSEDSTREAM;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);

    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);

    code = IStandardClientTracker::AudioClientTrackerMsg::RESUMESTREAM;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);

    code = IStandardClientTracker::AudioClientTrackerMsg::MUTESTREAM;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);

    code = IStandardClientTracker::AudioClientTrackerMsg::UNMUTESTREAM;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_007
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_007, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::SETLOWPOWERVOL;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    int result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_008
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_008, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::GETLOWPOWERVOL;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    int result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_009
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_009, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::GETSINGLESTREAMVOL;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    int result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_010
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_010, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::SETOFFLOADMODE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    int result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);

    code = IStandardClientTracker::AudioClientTrackerMsg::UNSETOFFLOADMODE;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_011
 * @tc.desc  : Test OnRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_011, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::ON_INIT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    int result = audioClientTrackerCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_012
 * @tc.desc  : Test OffloadRemoteRequest.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_012, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    std::shared_ptr<AudioClientTrackerTest> callback_1 = std::make_shared<AudioClientTrackerTest>();
    std::weak_ptr<AudioClientTrackerTest> callback(callback_1);
    audioClientTrackerCallbackStub_->SetClientTrackerCallback(callback);
    uint32_t code = IStandardClientTracker::AudioClientTrackerMsg::ON_INIT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AudioClientTrackerCallbackStub::GetDescriptor());
    int result = audioClientTrackerCallbackStub_->OffloadRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, 1);
}

/**
 * @tc.name  : Test AudioClientTrackerCallbackStub.
 * @tc.number: AudioClientTrackerCallbackStub_013
 * @tc.desc  : Test callback.
 */
HWTEST(AudioClientTrackerCallbackStubUnitTest, AudioClientTrackerCallbackStub_013, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub_ =
        std::make_shared<AudioClientTrackerCallbackStub>();
    StreamSetStateEventInternal streamSetStateEventInternal = {};
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_MUTE;
    streamSetStateEventInternal.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    float volume = 1;
    audioClientTrackerCallbackStub_->GetLowPowerVolumeImpl(volume);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->GetSingleStreamVolumeImpl(volume);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->MuteStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->PausedStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->ResumeStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->SetLowPowerVolumeImpl(volume);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->SetOffloadModeImpl(1, true);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->UnmuteStreamImpl(streamSetStateEventInternal);
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);

    audioClientTrackerCallbackStub_->UnsetOffloadModeImpl();
    EXPECT_NE(audioClientTrackerCallbackStub_, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
