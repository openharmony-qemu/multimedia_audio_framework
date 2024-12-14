/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "audio_capturer_source_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_capturer_source.h"
#include "fast_audio_capturer_source.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void AudioCapturerSourceUnitTest::SetUpTestCase(void) {}
void AudioCapturerSourceUnitTest::TearDownTestCase(void) {}
void AudioCapturerSourceUnitTest::SetUp(void) {}
void AudioCapturerSourceUnitTest::TearDown(void) {}


/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_001
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance And Actions
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_001, TestSize.Level1)
{
    AudioCapturerSource *capturer = AudioCapturerSource::GetInstance("usb");
    ASSERT_NE(capturer, nullptr);

    auto ret = capturer->Start();
    EXPECT_NE(ret, SUCCESS);

    float left = 0;
    float right = 0;
    ret = capturer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->GetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->Pause();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Resume();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Stop();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_002
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_002, TestSize.Level1)
{
    CaptureAttr *attr = new CaptureAttr{};
    AudioCapturerSource *capturer = AudioCapturerSource::Create(attr);
    ASSERT_NE(capturer, nullptr);
    
    auto ret = capturer->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);
    // attr will delete by ~AudioCapturerSource
    delete capturer;
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_003
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance And Actions
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_003, TestSize.Level1)
{
    FastAudioCapturerSource *capturer = FastAudioCapturerSource::GetInstance();
    ASSERT_NE(capturer, nullptr);

    auto ret = capturer->InitWithoutAttr();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Start();
    EXPECT_NE(ret, SUCCESS);

    float left = 0;
    float right = 0;
    ret = capturer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->GetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->Pause();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Resume();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Stop();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_004
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_004, TestSize.Level1)
{
    FastAudioCapturerSource *capturer = FastAudioCapturerSource::GetInstance();
    ASSERT_NE(capturer, nullptr);
    
    auto ret = capturer->SetMute(true);
    EXPECT_NE(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);
}
} // namespace AudioStandard
} // namespace OHOS
