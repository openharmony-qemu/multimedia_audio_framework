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

#include <iostream>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "audio_errors.h"
#include "audio_pnp_server_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPnpServerTest::SetUpTestCase(void) {}
void AudioPnpServerTest::TearDownTestCase(void) {}
void AudioPnpServerTest::SetUp(void)
{
    audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();
    mockCallback_ = std::make_shared<MockAudioPnpDeviceChangeCallback>();
}
void AudioPnpServerTest::TearDown(void)
{
    if (mockCallback_ != nullptr) {
        mockCallback_.reset();
    }
    if (audioPnpServer_ != nullptr) {
        audioPnpServer_ = nullptr;
    }
}

/**
 * @tc.name  : RegisterPnpStatusListener_Success
 * @tc.number: AudioPnpServerTest_001
 * @tc.desc  : Test RegisterPnpStatusListener function when callback is successfully registered.
 */
HWTEST_F(AudioPnpServerTest, RegisterPnpStatusListener_Success, testing::ext::TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    EXPECT_NE(mockCallback_, nullptr);
    int32_t ret = audioPnpServer_->RegisterPnpStatusListener(mockCallback_);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RegisterPnpStatusListener_NullCallback
 * @tc.number: AudioPnpServerTest_002
 * @tc.desc  : Test RegisterPnpStatusListener function when callback is null.
 */
HWTEST_F(AudioPnpServerTest, RegisterPnpStatusListener_NullCallback, testing::ext::TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    int32_t ret = audioPnpServer_->RegisterPnpStatusListener(nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : UnRegisterPnpStatusListener_ShouldReturnSuccess_WhenCalled
 * @tc.number: AudioPnpServerTest_001
 * @tc.desc  : Test if UnRegisterPnpStatusListener returns success when called.
 */
HWTEST_F(AudioPnpServerTest, UnRegisterPnpStatusListener_ShouldReturnSuccess_WhenCalled, TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    int32_t result = audioPnpServer_->UnRegisterPnpStatusListener();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : OnPnpDeviceStatusChanged_ShouldCallCallback_WhenCallbackIsNotNull
 * @tc.number: AudioPnpServerTest_002
 * @tc.desc  : Test if OnPnpDeviceStatusChanged calls the callback when it is not null.
 */
HWTEST_F(AudioPnpServerTest, OnPnpDeviceStatusChanged_ShouldCallCallback_WhenCallbackIsNotNull, TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    EXPECT_NE(mockCallback_, nullptr);
    audioPnpServer_->RegisterPnpStatusListener(mockCallback_);

    std::string info = "test_info";

    audioPnpServer_->OnPnpDeviceStatusChanged(info);
    EXPECT_NE(audioPnpServer_->pnpCallback_, nullptr);
}

/**
 * @tc.name  : OnPnpDeviceStatusChanged_ShouldNotCallCallback_WhenCallbackIsNull
 * @tc.number: AudioPnpServerTest_003
 * @tc.desc  : Test if OnPnpDeviceStatusChanged does not call the callback when it is null.
 */
HWTEST_F(AudioPnpServerTest, OnPnpDeviceStatusChanged_ShouldNotCallCallback_WhenCallbackIsNull, TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    audioPnpServer_->UnRegisterPnpStatusListener();

    std::string info = "test_info";
    audioPnpServer_->OnPnpDeviceStatusChanged(info);
    EXPECT_EQ(audioPnpServer_->pnpCallback_, nullptr);
}

/**
 * @tc.name  : DetectAudioDevice_AnalogHeadsetStateAdded_Success
 * @tc.number: AudioPnpServerTest_001
 * @tc.desc  : Test DetectAudioDevice when AnalogHeadsetState is added and DetectAnalogHeadsetState returns SUCCESS.
 */
HWTEST_F(AudioPnpServerTest, DetectAudioDevice_AnalogHeadsetStateAdded_Success, TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    audioPnpServer_->DetectAudioDevice();
    EXPECT_EQ(audioPnpServer_->pnpCallback_, nullptr);
}

/**
 * @tc.name  : DetectAudioDpDevice_Success_Add
 * @tc.number: AudioPnpServerTest_001
 * @tc.desc  : Test DetectAudioDpDevice function when DetectDPState
 */
HWTEST_F(AudioPnpServerTest, DetectAudioDpDevice_Success_Add, TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);
    // Arrange
    AudioEvent audioEvent = {0};
    // Act
    audioPnpServer_->DetectAudioDpDevice();

    // Assert
    EXPECT_EQ(audioEvent.eventType, 0);
}

/**
 * @tc.name  : StopPnpServer
 * @tc.number: AudioPnpServerTest_003
 * @tc.desc  : Test StopPnpServer function.
 */
HWTEST_F(AudioPnpServerTest, StopPnpServer, TestSize.Level0)
{
    EXPECT_NE(audioPnpServer_, nullptr);

    // Act
    audioPnpServer_->StopPnpServer();

    EXPECT_EQ(audioPnpServer_->socketThread_, nullptr);
    EXPECT_EQ(audioPnpServer_->inputThread_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS