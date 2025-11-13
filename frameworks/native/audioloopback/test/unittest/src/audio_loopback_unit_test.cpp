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
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "audio_renderer_mock.h"
#include "audio_capturer_mock.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
bool g_hasPermission = false;

void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audio_loopback_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void AudioLoopbackUnitTest::SetUpTestCase(void) {}
void AudioLoopbackUnitTest::TearDownTestCase(void) {}

void AudioLoopbackUnitTest::SetUp(void)
{
    GetPermission();
}

void AudioLoopbackUnitTest::TearDown(void) {}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_001, TestSize.Level0)
{
#ifdef TEMP_DISABLE
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, AppInfo());
    EXPECT_NE(audioLoopback, nullptr);
#endif
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_002, TestSize.Level0)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_IDLE;
    EXPECT_EQ(audioLoopback->Enable(true), false);
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    EXPECT_EQ(audioLoopback->Enable(true), false);
    EXPECT_EQ(audioLoopback->Enable(false), false);
    EXPECT_EQ(audioLoopback->Enable(false), false);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_003, TestSize.Level1)
{
#ifdef TEMP_DISABLE
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    auto audioLoopback2 = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback2->CreateAudioLoopback();
    EXPECT_NE(audioLoopback2->capturerState_, CAPTURER_RUNNING);
    audioLoopback->DestroyAudioLoopback();
    audioLoopback2->DestroyAudioLoopback();
#endif
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_004, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_UNAVAILABLE_DEVICE);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_005, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_UNAVAILABLE_DEVICE);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_006, TestSize.Level1)
{
#ifdef TEMP_DISABLE
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->CreateAudioLoopback();
    audioLoopback->currentState_ = LOOPBACK_STATE_PREPARED;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_AVAILABLE_RUNNING);
    audioLoopback->isRendererUsb_ = false;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_UNAVAILABLE_DEVICE);
    audioLoopback->DestroyAudioLoopback();
#endif
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_007, TestSize.Level1)
{
#ifdef TEMP_DISABLE
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    ASSERT_NE(audioLoopback->audioCapturer_, nullptr);
    audioLoopback->audioCapturer_->Release();
    audioLoopback->audioCapturer_ = nullptr;
    audioLoopback->DestroyAudioLoopback();
#endif
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_008, TestSize.Level1)
{
#ifdef TEMP_DISABLE
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
    ASSERT_NE(audioLoopback->audioRenderer_, nullptr);
    audioLoopback->audioRenderer_->Release();
    audioLoopback->audioRenderer_ = nullptr;
    audioLoopback->DestroyAudioLoopback();
#endif
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_009, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->rendererOptions_.rendererInfo.contentType = ContentType::CONTENT_TYPE_ULTRASONIC;
    audioLoopback->rendererOptions_.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_SYSTEM;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->audioRenderer_, nullptr);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_010, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->rendererOptions_.rendererInfo.rendererFlags = 0;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->rendererFastStatus_, FASTSTATUS_NORMAL);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_011, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->capturerOptions_.capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->audioCapturer_, nullptr);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_012, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->capturerOptions_.capturerInfo.capturerFlags = 0;
    audioLoopback->CreateAudioLoopback();
    EXPECT_EQ(audioLoopback->capturerFastStatus_, FASTSTATUS_NORMAL);
    audioLoopback->DestroyAudioLoopback();
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_CreateAudioLoopback_013, TestSize.Level1)
{
    AppInfo appInfo = AppInfo();
    appInfo.appPid = -1;
    appInfo.appUid = 1;
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, appInfo);
    EXPECT_NE(audioLoopback, nullptr);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    int32_t ret = audioLoopback->SetVolume(1);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioLoopback->karaokeParams_["Karaoke_volume"], "100");
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    int32_t ret = audioLoopback->SetVolume(1);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioLoopback->karaokeParams_["Karaoke_volume"], "100");
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_003, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    int32_t ret = audioLoopback->SetVolume(10);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetVolume_004, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    int32_t ret = audioLoopback->SetVolume(-1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_GetStatus_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    EXPECT_EQ(audioLoopback->GetStatus(), LOOPBACK_AVAILABLE_RUNNING);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_UpdateStatus_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->currentState_, LOOPBACK_STATE_DESTROYED);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_UpdateStatus_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    audioLoopback->rendererState_ = RENDERER_RUNNING;
    audioLoopback->isRendererUsb_ = true;
    audioLoopback->rendererFastStatus_ = FASTSTATUS_FAST;

    audioLoopback->capturerState_ = CAPTURER_RUNNING;
    audioLoopback->isCapturerUsb_ = true;
    audioLoopback->capturerFastStatus_ = FASTSTATUS_FAST;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->currentState_, LOOPBACK_STATE_RUNNING);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_UpdateStatus_003, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    audioLoopback->UpdateStatus();
    EXPECT_EQ(audioLoopback->currentState_, LOOPBACK_STATE_DESTROYED);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetReverbPreset_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    bool ret = audioLoopback->SetReverbPreset(REVERB_PRESET_THEATER);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetReverbPreset_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    bool ret = audioLoopback->SetReverbPreset(REVERB_PRESET_THEATER);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetEqualizerPreset_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->currentState_ = LOOPBACK_STATE_RUNNING;
    bool ret = audioLoopback->SetEqualizerPreset(EQUALIZER_PRESET_FLAT);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_SetEqualizerPreset_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    bool ret = audioLoopback->SetEqualizerPreset(EQUALIZER_PRESET_FLAT);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_StartAudioLoopback_001, TestSize.Level1)
{
    std::shared_ptr<MockAudioRenderer> mockRenderer = std::make_shared<NiceMock<MockAudioRenderer>>();
    std::shared_ptr<MockAudioCapturer> mockCapturer = std::make_shared<NiceMock<MockAudioCapturer>>();
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());

    audioLoopback->audioRenderer_ = mockRenderer;
    audioLoopback->audioCapturer_ = mockCapturer;
    EXPECT_CALL(*mockRenderer, Start(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockCapturer, Start()).WillOnce(Return(true));

    audioLoopback->StartAudioLoopback();

    EXPECT_EQ(audioLoopback->rendererState_, RENDERER_RUNNING);
    EXPECT_EQ(audioLoopback->capturerState_, CAPTURER_RUNNING);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_StartAudioLoopback_002, TestSize.Level1)
{
    std::shared_ptr<MockAudioRenderer> mockRenderer = std::make_shared<NiceMock<MockAudioRenderer>>();
    std::shared_ptr<MockAudioCapturer> mockCapturer = std::make_shared<NiceMock<MockAudioCapturer>>();
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());

    audioLoopback->audioRenderer_ = mockRenderer;
    audioLoopback->audioCapturer_ = mockCapturer;
    EXPECT_CALL(*mockRenderer, Start(_)).WillOnce(Return(false));
    EXPECT_CALL(*mockCapturer, Start()).Times(0);

    audioLoopback->StartAudioLoopback();

    EXPECT_NE(audioLoopback->rendererState_, RENDERER_RUNNING);
    EXPECT_NE(audioLoopback->capturerState_, CAPTURER_RUNNING);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_StartAudioLoopback_003, TestSize.Level1)
{
    std::shared_ptr<MockAudioRenderer> mockRenderer = std::make_shared<NiceMock<MockAudioRenderer>>();
    std::shared_ptr<MockAudioCapturer> mockCapturer = std::make_shared<NiceMock<MockAudioCapturer>>();
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());

    audioLoopback->audioRenderer_ = mockRenderer;
    audioLoopback->audioCapturer_ = mockCapturer;
    EXPECT_CALL(*mockRenderer, Start(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockCapturer, Start()).WillOnce(Return(false));

    audioLoopback->StartAudioLoopback();

    EXPECT_EQ(audioLoopback->rendererState_, RENDERER_RUNNING);
    EXPECT_NE(audioLoopback->capturerState_, CAPTURER_RUNNING);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_DestroyAudioLoopback_002, TestSize.Level1)
{
    std::shared_ptr<MockAudioRenderer> mockRenderer = std::make_shared<NiceMock<MockAudioRenderer>>();
    std::shared_ptr<MockAudioCapturer> mockCapturer = std::make_shared<NiceMock<MockAudioCapturer>>();
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());

    audioLoopback->audioRenderer_ = mockRenderer;
    audioLoopback->audioCapturer_ = mockCapturer;
    EXPECT_CALL(*mockCapturer, Stop()).Times(1);
    EXPECT_CALL(*mockRenderer, Stop()).Times(1);

    audioLoopback->DestroyAudioLoopback();

    EXPECT_EQ(audioLoopback->audioRenderer_, nullptr);
    EXPECT_EQ(audioLoopback->audioCapturer_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
