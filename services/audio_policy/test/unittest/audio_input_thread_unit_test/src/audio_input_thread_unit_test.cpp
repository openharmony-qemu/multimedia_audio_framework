/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_input_thread_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInputThreadUnitTest::SetUpTestCase(void) {}
void AudioInputThreadUnitTest::TearDownTestCase(void) {}
void AudioInputThreadUnitTest::SetUp(void) {}
void AudioInputThreadUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioRecoveryDeviceUnitTest_001.
* @tc.desc  : Test AudioAnalogHeadsetDeviceCheck.
*/
HWTEST_F(AudioInputThreadUnitTest, AudioInputThreadUnitTest_001, TestSize.Level1)
{
    input_event evt;
    evt.code = SW_HEADPHONE_INSERT;
    auto audioInputThread = std::make_shared<AudioInputThread>();
    auto result = audioInputThread->AudioAnalogHeadsetDeviceCheck(evt);
    EXPECT_EQ(result, SUCCESS);

    evt.code = SW_MICROPHONE_INSERT;
    result = audioInputThread->AudioAnalogHeadsetDeviceCheck(evt);
    EXPECT_EQ(result, SUCCESS);

    evt.code = SW_LINEOUT_INSERT;
    result = audioInputThread->AudioAnalogHeadsetDeviceCheck(evt);
    EXPECT_EQ(result, SUCCESS);

    evt.code = SW_LINEIN_INSERT;
    result = audioInputThread->AudioAnalogHeadsetDeviceCheck(evt);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioRecoveryDeviceUnitTest_002.
* @tc.desc  : Test AudioPnpInputPollAndRead.
*/
HWTEST_F(AudioInputThreadUnitTest, AudioInputThreadUnitTest_002, TestSize.Level1)
{
    auto audioInputThread = std::make_shared<AudioInputThread>();
    auto result = audioInputThread->AudioPnpInputOpen();
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS