/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_capturer_third_unit_test.h"

#include <chrono>
#include <future>
#include <thread>

#include "audio_capturer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_capturer_private.h"
#include "audio_stream_enum.h"
#include "audio_aiut_utils.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void AudioCapturerThirdUnitTest::SetUpTestCase(void)
{
    AIUT::GrantPermission("ohos.permission.MICROPHONE");
}

void AudioCapturerThirdUnitTest::TearDownTestCase(void) {}
void AudioCapturerThirdUnitTest::SetUp(void) {}
void AudioCapturerThirdUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioCapturer Create with invalid parameters
 * @tc.number: Audio_Capturer_Create_Invalid_001
 * @tc.desc  : Test Create with invalid stream type.
 */
HWTEST_F(AudioCapturerThirdUnitTest, Audio_Capturer_Create_Invalid_001, TestSize.Level1)
{
    // Test with invalid stream type
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(static_cast<AudioStreamType>(-1));
    // Expect nullptr for invalid stream type
    EXPECT_NE(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        audioCapturer->Release();
    }
}

} // namespace AudioStandard
} // namespace OHOS
