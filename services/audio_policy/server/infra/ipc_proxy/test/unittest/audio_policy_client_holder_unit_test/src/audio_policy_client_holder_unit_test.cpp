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
#include "audio_policy_client_holder_unit_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const int32_t  SUCCESS = 0;

void AudioPolicyClientHolderUnitTest::SetUpTestCase(void) {}
void AudioPolicyClientHolderUnitTest::TearDownTestCase(void) {}
void AudioPolicyClientHolderUnitTest::SetUp(void)
{
    mockClient_ = new MockAudioPolicyClient();
    clientHolder_ = std::make_shared<AudioPolicyClientHolder>(nullptr);
}

void AudioPolicyClientHolderUnitTest::TearDown(void)
{
    mockClient_ = nullptr;
    clientHolder_ = nullptr;
}

/**
 * @tc.name  : Test OnSpatializationEnabledChange.
 * @tc.number: AudioPolicyClientHolderTest_OnSpatializationEnabledChange_001
 * @tc.desc  : Verify that hasSystemPermission_ == true.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnSpatializationEnabledChange_001, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnSpatializationEnabledChange(_)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = true;
    bool enabled = true;
    clientHolder_->OnSpatializationEnabledChange(enabled);
}

/**
 * @tc.name  : Test OnSpatializationEnabledChange.
 * @tc.number: AudioPolicyClientHolderTest_OnSpatializationEnabledChange_002
 * @tc.desc  : Verify that hasSystemPermission_ == false.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnSpatializationEnabledChange_002, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnSpatializationEnabledChange(_)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = false;
    bool enabled = true;
    clientHolder_->OnSpatializationEnabledChange(enabled);
}

/**
 * @tc.name  : Test OnSpatializationEnabledChangeForAnyDevice.
 * @tc.number: AudioPolicyClientHolderTest_OnSpatializationEnabledChangeForAnyDevice_001
 * @tc.desc  : Verify that hasSystemPermission_ == true.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnSpatializationEnabledChangeForAnyDevice_001, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnSpatializationEnabledChangeForAnyDevice(_, _)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = true;
    bool enabled = true;
    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    clientHolder_->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

/**
 * @tc.name  : Test OnSpatializationEnabledChangeForAnyDevice.
 * @tc.number: AudioPolicyClientHolderTest_OnSpatializationEnabledChangeForAnyDevice_002
 * @tc.desc  : Verify that hasSystemPermission_ == false.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnSpatializationEnabledChangeForAnyDevice_002, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnSpatializationEnabledChangeForAnyDevice(_, _)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = false;
    bool enabled = true;
    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    clientHolder_->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

/**
 * @tc.name  : Test OnHeadTrackingEnabledChang.
 * @tc.number: AudioPolicyClientHolderTest_OnHeadTrackingEnabledChange_001
 * @tc.desc  : Verify that hasSystemPermission_ == true.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnHeadTrackingEnabledChange_001, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnHeadTrackingEnabledChange(_)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = true;
    bool enabled = true;
    clientHolder_->OnHeadTrackingEnabledChange(enabled);
}

/**
 * @tc.name  : Test OnHeadTrackingEnabledChange.
 * @tc.number: AudioPolicyClientHolderTest_OnHeadTrackingEnabledChange_002
 * @tc.desc  : Verify that hasSystemPermission_ == false.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnHeadTrackingEnabledChange_002, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnHeadTrackingEnabledChange(_)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = false;
    bool enabled = true;
    clientHolder_->OnHeadTrackingEnabledChange(enabled);
}

/**
 * @tc.name  : Test OnHeadTrackingEnabledChangeForAnyDevice.
 * @tc.number: AudioPolicyClientHolderTest_OnHeadTrackingEnabledChangeForAnyDevice_001
 * @tc.desc  : Verify that hasSystemPermission_ == true.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnHeadTrackingEnabledChangeForAnyDevice_001, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnHeadTrackingEnabledChangeForAnyDevice(_, _)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = true;
    bool enabled = true;
    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    clientHolder_->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

/**
 * @tc.name  : Test OnHeadTrackingEnabledChangeForAnyDevice.
 * @tc.number: AudioPolicyClientHolderTest_OnHeadTrackingEnabledChangeForAnyDevice_002
 * @tc.desc  : Verify that hasSystemPermission_ == false.
 */
HWTEST_F(AudioPolicyClientHolderUnitTest, OnHeadTrackingEnabledChangeForAnyDevice_002, TestSize.Level4)
{
    EXPECT_CALL(*mockClient_, OnHeadTrackingEnabledChangeForAnyDevice(_, _)).WillOnce(Return(SUCCESS));
    clientHolder_->audioPolicyClient_ = mockClient_;
    clientHolder_->hasSystemPermission_ = false;
    bool enabled = true;
    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    clientHolder_->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

} // namespace AudioStandard
} // namespace OHOS