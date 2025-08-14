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

#include "audio_client_tracker_callback_listener_unit_test.h"

namespace OHOS {
namespace AudioStandard {
void AudioClientTrackerCallbackListenerUnitTest::SetUpTestCase(void) {}

void AudioClientTrackerCallbackListenerUnitTest::TearDownTestCase(void) {}

void AudioClientTrackerCallbackListenerUnitTest::SetUp(void)
{
    listener_ = new AudioClientTrackerCallbackService();
    clientTracker_ = std::make_shared<ClientTrackerCallbackListener>(listener_);
    streamSetStateEventInternal_ = {};
}

void AudioClientTrackerCallbackListenerUnitTest::TearDown(void)
{
    if (listener_ != nullptr) {
        delete listener_;
        listener_ = nullptr;
    }
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: MuteStreamImpl_01
* @tc.desc  : Test MuteStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, MuteStreamImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    ASSERT_NE(clientTracker_, nullptr);
    EXPECT_NO_THROW(
        clientTracker_->MuteStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: MuteStreamImpl_02
* @tc.desc  : Test MuteStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, MuteStreamImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    EXPECT_NO_THROW(
        clientTracker->MuteStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: UnmuteStreamImpl_01
* @tc.desc  : Test UnmuteStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, UnmuteStreamImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    EXPECT_NO_THROW(
        clientTracker_->UnmuteStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: UnmuteStreamImpl_02
* @tc.desc  : Test UnmuteStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, UnmuteStreamImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    EXPECT_NO_THROW(
        clientTracker->UnmuteStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: PausedStreamImpl_01
* @tc.desc  : Test PausedStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, PausedStreamImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    EXPECT_NO_THROW(
        clientTracker_->PausedStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: PausedStreamImpl_02
* @tc.desc  : Test PausedStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, PausedStreamImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    EXPECT_NO_THROW(
        clientTracker->PausedStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: ResumeStreamImpl_01
* @tc.desc  : Test ResumeStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, ResumeStreamImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    EXPECT_NO_THROW(
        clientTracker_->ResumeStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: ResumeStreamImpl_02
* @tc.desc  : Test ResumeStreamImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, ResumeStreamImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    EXPECT_NO_THROW(
        clientTracker->ResumeStreamImpl(streamSetStateEventInternal_);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: SetLowPowerVolumeImpl_01
* @tc.desc  : Test SetLowPowerVolumeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, SetLowPowerVolumeImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    float volume = 0.5f;
    EXPECT_NO_THROW(
        clientTracker_->SetLowPowerVolumeImpl(volume);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: SetLowPowerVolumeImpl_02
* @tc.desc  : Test SetLowPowerVolumeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, SetLowPowerVolumeImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    float volume = 0.5f;
    EXPECT_NO_THROW(
        clientTracker->SetLowPowerVolumeImpl(volume);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: GetLowPowerVolumeImpl_01
* @tc.desc  : Test GetLowPowerVolumeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, GetLowPowerVolumeImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    float volume = 0.5f;
    EXPECT_NO_THROW(
        clientTracker_->GetLowPowerVolumeImpl(volume);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: GetLowPowerVolumeImpl_02
* @tc.desc  : Test GetLowPowerVolumeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, GetLowPowerVolumeImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    float volume = 0.5f;
    EXPECT_NO_THROW(
        clientTracker->GetLowPowerVolumeImpl(volume);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: GetSingleStreamVolumeImpl_01
* @tc.desc  : Test GetSingleStreamVolumeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, GetSingleStreamVolumeImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    float volume = 0.5f;
    EXPECT_NO_THROW(
        clientTracker_->GetSingleStreamVolumeImpl(volume);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: GetSingleStreamVolumeImpl_02
* @tc.desc  : Test GetSingleStreamVolumeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, GetSingleStreamVolumeImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    float volume = 0.5f;
    EXPECT_NO_THROW(
        clientTracker->GetSingleStreamVolumeImpl(volume);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: SetOffloadModeImpl_01
* @tc.desc  : Test SetOffloadModeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, SetOffloadModeImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    int32_t state = 1;
    bool isAppBack = false;
    EXPECT_NO_THROW(
        clientTracker_->SetOffloadModeImpl(state, isAppBack);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: SetOffloadModeImpl_02
* @tc.desc  : Test SetOffloadModeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, SetOffloadModeImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    int32_t state = 1;
    bool isAppBack = false;
    EXPECT_NO_THROW(
        clientTracker->SetOffloadModeImpl(state, isAppBack);
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: UnsetOffloadModeImpl_01
* @tc.desc  : Test UnsetOffloadModeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, UnsetOffloadModeImpl_01, TestSize.Level1)
{
    ASSERT_NE(listener_, nullptr);
    EXPECT_NO_THROW(
        clientTracker_->UnsetOffloadModeImpl();
    );
}

/**
* @tc.name  : Test ClientTrackerCallbackListener.
* @tc.number: UnsetOffloadModeImpl_02
* @tc.desc  : Test UnsetOffloadModeImpl.
*/
HWTEST_F(AudioClientTrackerCallbackListenerUnitTest, UnsetOffloadModeImpl_02, TestSize.Level1)
{
    std::shared_ptr<AudioClientTracker> clientTracker = std::make_shared<ClientTrackerCallbackListener>(nullptr);
    EXPECT_NO_THROW(
        clientTracker->UnsetOffloadModeImpl();
    );
}

} // AudioStandardnamespace
} // OHOSnamespace
