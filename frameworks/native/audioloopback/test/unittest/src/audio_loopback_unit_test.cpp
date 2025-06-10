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

#include "audio_loopback_unit_test.h"
#include "audio_loopback_private.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
void AudioLoopbackUnitTest::SetUpTestCase(void) {}
void AudioLoopbackUnitTest::TearDownTestCase(void) {}
void AudioLoopbackUnitTest::SetUp(void) {}
void AudioLoopbackUnitTest::TearDown(void) {}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    auto audioLoopback2 = std::make_shared<AudioLoopbackPrivate>(HARDWARE, AppInfo());
    audioLoopback2->CreateAudioLoopback();
    EXPECT_NE(audioLoopback2->capturerState_, CAPTURER_RUNNING);
    audioLoopback->DestroyAudioLoopback();
    audioLoopback2->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_SetKaraokeParameters_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    audioLoopback->karaokeParams_["Karaoke_enable"] = "enable";
    bool ret = audioLoopback->SetKaraokeParameters();
    EXPECT_EQ(ret, true);
    audioLoopback->DestroyAudioLoopback();
}
} // namespace AudioStandard
} // namespace OHOS
