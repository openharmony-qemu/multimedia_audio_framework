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

#include <iostream>
#include "audio_group_handle_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioGroupHandleUnitTest::SetUpTestCase(void) {}
void AudioGroupHandleUnitTest::TearDownTestCase(void) {}
void AudioGroupHandleUnitTest::SetUp(void) {}
void AudioGroupHandleUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test CheckId.
 * @tc.number: AudioGroupHandleUnitTest_001
 * @tc.desc  : Test CheckId.
 */
HWTEST(AudioGroupHandleUnitTest, AudioGroupHandleUnitTest_001, TestSize.Level4)
{
    AudioGroupHandle audioGroupHandleTest = AudioGroupHandle::GetInstance();
    GroupType type = GroupType::VOLUME_TYPE;
    audioGroupHandleTest.currentVolumeId_ = MAX_ID;
    audioGroupHandleTest.currentInterruptId_ = MAX_ID;

    audioGroupHandleTest.CheckId(type);
    EXPECT_EQ(0, audioGroupHandleTest.currentVolumeId_);
    EXPECT_EQ(MAX_ID, audioGroupHandleTest.currentInterruptId_);
}

/**
 * @tc.name  : Test CheckId.
 * @tc.number: AudioGroupHandleUnitTest_002
 * @tc.desc  : Test CheckId.
 */
HWTEST(AudioGroupHandleUnitTest, AudioGroupHandleUnitTest_002, TestSize.Level4)
{
    AudioGroupHandle audioGroupHandleTest = AudioGroupHandle::GetInstance();
    GroupType type = GroupType::INTERRUPT_TYPE;
    audioGroupHandleTest.currentVolumeId_ = 1;
    audioGroupHandleTest.currentInterruptId_ = MAX_ID;

    audioGroupHandleTest.CheckId(type);
    EXPECT_EQ(1, audioGroupHandleTest.currentVolumeId_);
    EXPECT_EQ(0, audioGroupHandleTest.currentInterruptId_);
}

/**
 * @tc.name  : Test CheckId.
 * @tc.number: AudioGroupHandleUnitTest_003
 * @tc.desc  : Test CheckId.
 */
HWTEST(AudioGroupHandleUnitTest, AudioGroupHandleUnitTest_003, TestSize.Level4)
{
    AudioGroupHandle audioGroupHandleTest = AudioGroupHandle::GetInstance();
    GroupType type = GroupType::INTERRUPT_TYPE;
    audioGroupHandleTest.currentVolumeId_ = 1;
    audioGroupHandleTest.currentInterruptId_ = 1;

    audioGroupHandleTest.CheckId(type);
    EXPECT_EQ(1, audioGroupHandleTest.currentVolumeId_);
    EXPECT_EQ(1, audioGroupHandleTest.currentInterruptId_);
}
} // namespace AudioStandard
} // namespace OHOS
