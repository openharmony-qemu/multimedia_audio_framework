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
#include "audio_device_enhance_manager.h"
#include "audio_errors.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "audio_renderer_mock.h"
#include "audio_capturer_mock.h"
#include "audio_session_info.h"
#include "audio_debug_manager.h"
#include "audio_debug_callback.h"
#include <unistd.h>

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
bool g_hasPermission = false;

class TestLoopbackDebugCallback : public AudioLoopbackDebugCallback {
public:
    int32_t GetLoopbackDebugInfo(AudioLoopbackDebugInfo &debugInfo) override
    {
        debugInfo = testInfo_;
        return retCode_;
    }
    AudioLoopbackDebugInfo testInfo_;
    int32_t retCode_ = SUCCESS;
};

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
    EXPECT_EQ(audioLoopback->Enable(true), true);
    EXPECT_EQ(audioLoopback->Enable(false), true);
    EXPECT_EQ(audioLoopback->Enable(false), true);
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

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_SelectOutputDeviceForAudioRenderer_001, TestSize.Level0)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRenderer> audioRenderer = nullptr;
    auto ret = AudioDeviceEnhanceManager::GetInstance().SelectOutputDeviceForAudioRenderer(audioRenderer, desc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_SelectOutputDeviceForAudioRenderer_002, TestSize.Level0)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    auto renderer = std::make_shared<NiceMock<MockAudioRenderer>>();
    std::shared_ptr<AudioRenderer> audioRenderer = renderer;
    EXPECT_CALL(*renderer, SelectOutputDevice(desc)).WillOnce(Return(SUCCESS));

    auto ret = AudioDeviceEnhanceManager::GetInstance().SelectOutputDeviceForAudioRenderer(audioRenderer, desc);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_SelectOutputDeviceForAudioRenderer_003, TestSize.Level0)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    auto renderer = std::make_shared<NiceMock<MockAudioRenderer>>();
    std::shared_ptr<AudioRenderer> audioRenderer = renderer;
    EXPECT_CALL(*renderer, SelectOutputDevice(desc)).WillOnce(Return(ERR_INVALID_PARAM));

    auto ret = AudioDeviceEnhanceManager::GetInstance().SelectOutputDeviceForAudioRenderer(audioRenderer, desc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_SelectInputDeviceForAudioCapturer_001, TestSize.Level0)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioCapturer> audioCapturer = nullptr;
    auto ret = AudioDeviceEnhanceManager::GetInstance().SelectInputDeviceForAudioCapturer(audioCapturer, desc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_SelectInputDeviceForAudioCapturer_002, TestSize.Level0)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    auto capturer = std::make_shared<NiceMock<MockAudioCapturer>>();
    std::shared_ptr<AudioCapturer> audioCapturer = capturer;
    EXPECT_CALL(*capturer, SelectInputDevice(desc)).WillOnce(Return(SUCCESS));

    auto ret = AudioDeviceEnhanceManager::GetInstance().SelectInputDeviceForAudioCapturer(audioCapturer, desc);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_SelectInputDeviceForAudioCapturer_003, TestSize.Level0)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    auto capturer = std::make_shared<NiceMock<MockAudioCapturer>>();
    std::shared_ptr<AudioCapturer> audioCapturer = capturer;
    EXPECT_CALL(*capturer, SelectInputDevice(desc)).WillOnce(Return(ERR_INVALID_PARAM));

    auto ret = AudioDeviceEnhanceManager::GetInstance().SelectInputDeviceForAudioCapturer(audioCapturer, desc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_RendererBaseSelectOutputDevice_001, TestSize.Level0)
{
    MockAudioRenderer renderer;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    
    auto ret = renderer.AudioRenderer::SelectOutputDevice(desc);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Device_Enhance_Manager_CapturerBaseSelectInputDevice_001, TestSize.Level0)
{
    MockAudioCapturer capturer;
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto ret = capturer.AudioCapturer::SelectInputDevice(desc);
    EXPECT_EQ(ret, -1);
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

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_OnStateChange, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    auto renderCallback = std::make_shared<AudioLoopbackPrivate::RendererCallbackImpl>(*audioLoopback);
    auto captureCallback = std::make_shared<AudioLoopbackPrivate::CapturerCallbackImpl>(*audioLoopback);

    audioLoopback->activeOutputDevice_ = DEVICE_TYPE_USB_HEADSET;
    audioLoopback->activeInputDevice_ = DEVICE_TYPE_USB_HEADSET;

    audioLoopback->isRendererUsb_ = true;
    audioLoopback->isCapturerUsb_ = true;

    AudioDeviceDescriptor deviceInfo;
    AudioStreamDeviceChangeReason reson = AudioStreamDeviceChangeReason::UNKNOWN;
    renderCallback->OnOutputDeviceChange(deviceInfo, reson);
    captureCallback->OnStateChange(deviceInfo);
    EXPECT_FALSE(audioLoopback->isRendererUsb_);
    EXPECT_FALSE(audioLoopback->isCapturerUsb_);

    renderCallback->OnFastStatusChange(FASTSTATUS_NORMAL);
    captureCallback->OnFastStatusChange(FASTSTATUS_NORMAL);
    EXPECT_EQ(audioLoopback->rendererFastStatus_, FASTSTATUS_NORMAL);
    EXPECT_EQ(audioLoopback->capturerFastStatus_, FASTSTATUS_NORMAL);

    renderCallback->OnFastStatusChange(FASTSTATUS_FAST);
    captureCallback->OnFastStatusChange(FASTSTATUS_FAST);
    EXPECT_EQ(audioLoopback->rendererFastStatus_, FASTSTATUS_FAST);
    EXPECT_EQ(audioLoopback->capturerFastStatus_, FASTSTATUS_FAST);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_GlobalMode_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isGlobal_ = true;
    auto capturerConfig = audioLoopback->GenerateCapturerConfig();
    EXPECT_EQ(capturerConfig.strategy.concurrencyMode, AudioConcurrencyMode::SILENT);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_GlobalMode_002, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->isGlobal_ = false;
    auto capturerConfig = audioLoopback->GenerateCapturerConfig();
    EXPECT_EQ(capturerConfig.strategy.concurrencyMode, AudioConcurrencyMode::INVALID);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_OnOutputDeviceChange_Normal_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    auto renderCallback = std::make_shared<AudioLoopbackPrivate::RendererCallbackImpl>(*audioLoopback);
    audioLoopback->activeOutputDevice_ = DEVICE_TYPE_SPEAKER;
    audioLoopback->isRendererUsb_ = false;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN;
    renderCallback->OnOutputDeviceChange(deviceInfo, reason);
    EXPECT_FALSE(audioLoopback->isRendererUsb_);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_OnOutputDeviceChange_SameDevice_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    auto renderCallback = std::make_shared<AudioLoopbackPrivate::RendererCallbackImpl>(*audioLoopback);
    audioLoopback->activeOutputDevice_ = DEVICE_TYPE_USB_HEADSET;
    audioLoopback->isRendererUsb_ = false;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN;
    renderCallback->OnOutputDeviceChange(deviceInfo, reason);
    EXPECT_TRUE(audioLoopback->isRendererUsb_);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_GetLoopbackDebugInfo_001, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    AudioLoopbackDebugInfo debugInfo;
    int32_t ret = audioLoopback->GetLoopbackDebugInfo(debugInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(debugInfo.mode, LOOPBACK_HARDWARE);
    EXPECT_EQ(debugInfo.currentState, LOOPBACK_STATE_IDLE);
    EXPECT_EQ(debugInfo.activeOutputDevice, DEVICE_TYPE_NONE);
    EXPECT_EQ(debugInfo.activeInputDevice, DEVICE_TYPE_NONE);
    EXPECT_EQ(debugInfo.reverbPreset, REVERB_PRESET_THEATER);
    EXPECT_EQ(debugInfo.equalizerPreset, EQUALIZER_PRESET_FULL);
    EXPECT_EQ(debugInfo.volume, 50);
    EXPECT_EQ(debugInfo.uplinkStreamState, CAPTURER_INVALID);
    EXPECT_EQ(debugInfo.downlinkStreamState, RENDERER_INVALID);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_GetLoopbackDebugInfo_003, TestSize.Level1)
{
    auto audioLoopback = std::make_shared<AudioLoopbackPrivate>(LOOPBACK_HARDWARE, AppInfo());
    audioLoopback->karaokeParams_["Karaoke_volume"] = "invalid";
    AudioLoopbackDebugInfo debugInfo;
    int32_t ret = audioLoopback->GetLoopbackDebugInfo(debugInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(debugInfo.volume, 0);
}

namespace {
    constexpr int32_t TEST_APP_UID = 1234;
    constexpr int32_t TEST_VOLUME = 75;
    constexpr size_t TEST_BUF_SIZE = 4096;
    constexpr uint32_t TEST_APP_UID_CB1 = 100;
    constexpr uint32_t TEST_APP_UID_CB2 = 200;
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_PrintDebugInfo_001, TestSize.Level1)
{
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(0, -1);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_PrintDebugInfo_002, TestSize.Level1)
{
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_PrintDebugInfo_003, TestSize.Level1)
{
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    callback->testInfo_.appUid = TEST_APP_UID;
    callback->testInfo_.appName = "TestApp";
    callback->testInfo_.mode = LOOPBACK_HARDWARE;
    callback->testInfo_.currentState = LOOPBACK_STATE_RUNNING;
    callback->testInfo_.activeOutputDevice = DEVICE_TYPE_USB_HEADSET;
    callback->testInfo_.activeInputDevice = DEVICE_TYPE_USB_HEADSET;
    callback->testInfo_.reverbPreset = REVERB_PRESET_KTV;
    callback->testInfo_.equalizerPreset = EQUALIZER_PRESET_BRIGHT;
    callback->testInfo_.volume = TEST_VOLUME;
    callback->testInfo_.uplinkStreamState = CAPTURER_RUNNING;
    callback->testInfo_.downlinkStreamState = RENDERER_RUNNING;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(key, 0u);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    ret = AudioDebugManager::GetInstance().PrintAllAudioLoopbacksDebugInfo(pipefd[1]);
    EXPECT_EQ(ret, SUCCESS);
    close(pipefd[1]);

    char buf[TEST_BUF_SIZE] = {0};
    ssize_t bytesRead = read(pipefd[0], buf, sizeof(buf) - 1);
    close(pipefd[0]);
    ASSERT_GT(bytesRead, 0);

    std::string output(buf, bytesRead);
    EXPECT_NE(output.find("audioLoopback"), std::string::npos);
    EXPECT_NE(output.find("appUid: " + std::to_string(TEST_APP_UID)), std::string::npos);
    EXPECT_NE(output.find("TestApp"), std::string::npos);
    EXPECT_NE(output.find("volume: " + std::to_string(TEST_VOLUME)), std::string::npos);
    EXPECT_NE(output.find("CAPTURER_RUNNING"), std::string::npos);
    EXPECT_NE(output.find("RENDERER_RUNNING"), std::string::npos);

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_PrintDebugInfo_004, TestSize.Level1)
{
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    callback->retCode_ = ERR_ILLEGAL_STATE;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    EXPECT_EQ(ret, SUCCESS);

    ret = AudioDebugManager::GetInstance().PrintAllAudioLoopbacksDebugInfo(-1);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_PrintDebugInfo_005, TestSize.Level1)
{
    auto callback = std::make_shared<TestLoopbackDebugCallback>();

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    EXPECT_EQ(ret, SUCCESS);

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);

    ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, -1);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioLoopbackUnitTest, Audio_Loopback_PrintDebugInfo_006, TestSize.Level1)
{
    auto callback1 = std::make_shared<TestLoopbackDebugCallback>();
    callback1->testInfo_.appUid = TEST_APP_UID_CB1;
    callback1->testInfo_.appName = "App1";
    auto callback2 = std::make_shared<TestLoopbackDebugCallback>();
    callback2->testInfo_.appUid = TEST_APP_UID_CB2;
    callback2->testInfo_.appName = "App2";

    uint32_t key1 = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key1, callback1);
    EXPECT_EQ(ret, SUCCESS);
    uint32_t key2 = 0;
    ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key2, callback2);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(key1, key2);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    ret = AudioDebugManager::GetInstance().PrintAllAudioLoopbacksDebugInfo(pipefd[1]);
    EXPECT_EQ(ret, SUCCESS);
    close(pipefd[1]);

    char buf[TEST_BUF_SIZE] = {0};
    ssize_t bytesRead = read(pipefd[0], buf, sizeof(buf) - 1);
    close(pipefd[0]);
    ASSERT_GT(bytesRead, 0);

    std::string output(buf, bytesRead);
    EXPECT_NE(output.find("appUid: " + std::to_string(TEST_APP_UID_CB1)), std::string::npos);
    EXPECT_NE(output.find("App1"), std::string::npos);
    EXPECT_NE(output.find("appUid: " + std::to_string(TEST_APP_UID_CB2)), std::string::npos);
    EXPECT_NE(output.find("App2"), std::string::npos);

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key1);
    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key2);
}
} // namespace AudioStandard
} // namespace OHOS
