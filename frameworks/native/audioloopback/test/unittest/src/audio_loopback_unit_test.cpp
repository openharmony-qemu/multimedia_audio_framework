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
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
void AudioLoopbackUnitTest::SetUpTestCase(void) {}
void AudioLoopbackUnitTest::TearDownTestCase(void) {}
void AudioLoopbackUnitTest::SetUp(void) {}
void AudioLoopbackUnitTest::TearDown(void) {}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_001, TestSize.Level0)
{
    AppInfo appInfo = AppInfo();
    appInfo.appTokenId = 1;
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, AppInfo());
    EXPECT_EQ(audioLoopback, nullptr);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_002, TestSize.Level0)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    EXPECT_EQ(audioLoopback->Enable(true), false);
    EXPECT_EQ(audioLoopback->Enable(false), true);
    EXPECT_EQ(audioLoopback->Enable(false), true);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_003, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    auto audioLoopback2 = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback2->CreateAudioLoopback();
    EXPECT_NE(audioLoopback2->capturerState_, CAPTURER_RUNNING);
    audioLoopback->DestroyAudioLoopback();
    audioLoopback2->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_004, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_UNAVAILABLE_DEVICE);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_005, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_UNAVAILABLE_DEVICE);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_006, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->CreateAudioLoopback();
    audioLoopback->isStarted_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_AVAILABLE_RUNNING);
    audioLoopback->isRendererUsb_ = false;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_UNAVAILABLE_DEVICE);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_007, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    ASSERT_NE(audioLoopback->audioCapturer_, nullptr);
    audioLoopback->audioCapturer_->Release();
    audioLoopback->audioCapturer_ = nullptr;
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_008, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    ASSERT_NE(audioLoopback->audioRenderer_, nullptr);
    audioLoopback->audioRenderer_->Release();
    audioLoopback->audioRenderer_ = nullptr;
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_009, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->rendererOptions_.rendererInfo.contentType = ContentType::CONTENT_TYPE_ULTRASONIC;
    audioLoopback->rendererOptions_.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_SYSTEM;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->audioRenderer_, nullptr);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_010, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->rendererOptions_.rendererInfo.rendererFlags = 0;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->rendererFastStatus_, FASTSTATUS_NORMAL);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_011, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->capturerOptions_.capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->audioCapturer_, nullptr);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_012, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->capturerOptions_.capturerInfo.capturerFlags = 0;
    audioLoopback->CreateAudioLoopback();
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    EXPECT_EQ(audioLoopback->capturerFastStatus_, FASTSTATUS_NORMAL);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_013, TestSize.Level1)
{
    AppInfo appInfo = AppInfo();
    appInfo.appPid = -1;
    appInfo.appUid = 1;
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, appInfo);
    EXPECT_NE(audioLoopback, nullptr);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    int32_t ret = audioLoopback->SetVolume(1);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioLoopback->karaokeParams_["Karaoke_volume"], "100");
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    int32_t ret = audioLoopback->SetVolume(1);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioLoopback->karaokeParams_["Karaoke_volume"], "100");
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_003, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    int32_t ret = audioLoopback->SetVolume(10);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_004, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    int32_t ret = audioLoopback->SetVolume(-1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_GetStatus_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_AVAILABLE_RUNNING);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_UpdateStatus_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->isStarted_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->currentStatus_, LOOPBACK_UNAVAILABLE_SCENE);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_UpdateStatus_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentStatus_ = LOOPBACK_AVAILABLE_RUNNING;
    audioLoopback->rendererState_ = RENDERER_RUNNING;
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->rendererFastStatus_ = FASTSTATUS_FAST;

    audioLoopback->capturerState_ = CAPTURER_RUNNING;
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->capturerFastStatus_ = FASTSTATUS_FAST;
    audioLoopback->isStarted_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->currentStatus_, LOOPBACK_AVAILABLE_RUNNING);
}

HWTEST(AudioLoopbackUnitTest, Audio_Loopback_UpdateStatus_003, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());

    audioLoopback->currentStatus_ = LOOPBACK_UNAVAILABLE_DEVICE;
    audioLoopback->isStarted_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->currentStatus_, LOOPBACK_UNAVAILABLE_DEVICE);
}
} // namespace AudioStandard
} // namespace OHOS
