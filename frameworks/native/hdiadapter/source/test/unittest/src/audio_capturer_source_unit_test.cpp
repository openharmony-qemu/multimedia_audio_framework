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

#include "audio_capturer_source.cpp"
#include "fast_audio_capturer_source.h"
#include "bluetooth_capturer_source.h"

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

    bool isInit = capturer->IsInited();
    EXPECT_NE(isInit, true);

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

    ret = capturer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Flush();
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

    string usbInfo;
    auto ret = capturer->Preload(usbInfo);
    EXPECT_NE(ret, SUCCESS);
    
    ret = capturer->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    AudioParamKey key = USB_DEVICE;
    std::string condition;
    std::string param = capturer->GetAudioParameter(key, condition);
    EXPECT_EQ(param, "");

    float amolitude = capturer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    std::vector<int32_t> appsUid;
    ret = capturer->UpdateAppsUid(appsUid);
    EXPECT_NE(ret, ERROR);

    uint32_t captureId = 0;
    ret = capturer->GetCaptureId(captureId);
    EXPECT_EQ(ret, SUCCESS);

    ret  = capturer->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    capturer->SetAddress(address);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);

    capturer->RegisterParameterCallback(nullptr);
    capturer->RegisterAudioCapturerSourceCallback(nullptr);
    capturer->RegisterWakeupCloseCallback(nullptr);

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
    AudioCapturerSource *capturer = AudioCapturerSource::GetInstance("Built_in_wakeup", SOURCE_TYPE_WAKEUP);
    ASSERT_NE(capturer, nullptr);

    bool isInit = capturer->IsInited();
    EXPECT_NE(isInit, true);

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

    ret = capturer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Flush();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_004
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_004, TestSize.Level1)
{
    AudioCapturerSource *capturer = AudioCapturerSource::GetInstance("Built_in_wakeup", SOURCE_TYPE_WAKEUP);
    ASSERT_NE(capturer, nullptr);

    string usbInfo;
    auto ret = capturer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);
    
    ret = capturer->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    AudioParamKey key = USB_DEVICE;
    std::string condition;
    std::string param = capturer->GetAudioParameter(key, condition);
    EXPECT_EQ(param, "");

    float amolitude = capturer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    std::vector<int32_t> appsUid;
    ret = capturer->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t captureId = 0;
    ret = capturer->GetCaptureId(captureId);
    EXPECT_EQ(ret, SUCCESS);

    ret  = capturer->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    capturer->SetAddress(address);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);

    capturer->RegisterParameterCallback(nullptr);
    capturer->RegisterAudioCapturerSourceCallback(nullptr);
    capturer->RegisterWakeupCloseCallback(nullptr);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_005
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance And Actions
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_005, TestSize.Level1)
{
    AudioCapturerSource *capturer = AudioCapturerSource::GetInstance("primary", SOURCE_TYPE_MIC);
    ASSERT_NE(capturer, nullptr);

    bool isInit = capturer->IsInited();
    EXPECT_NE(isInit, true);

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

    ret = capturer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Flush();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_006
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_006, TestSize.Level1)
{
    AudioCapturerSource *capturer = AudioCapturerSource::GetInstance("primary", SOURCE_TYPE_MIC);
    ASSERT_NE(capturer, nullptr);

    string usbInfo;
    auto ret = capturer->Preload(usbInfo);
    EXPECT_NE(ret, SUCCESS);
    
    ret = capturer->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    AudioParamKey key = USB_DEVICE;
    std::string condition;
    std::string param = capturer->GetAudioParameter(key, condition);
    EXPECT_EQ(param, "");

    float amolitude = capturer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    std::vector<int32_t> appsUid;
    ret = capturer->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t captureId = 0;
    ret = capturer->GetCaptureId(captureId);
    EXPECT_EQ(ret, SUCCESS);

    ret  = capturer->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    capturer->SetAddress(address);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);

    capturer->RegisterParameterCallback(nullptr);
    capturer->RegisterAudioCapturerSourceCallback(nullptr);
    capturer->RegisterWakeupCloseCallback(nullptr);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_007
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance And Actions
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_007, TestSize.Level1)
{
    FastAudioCapturerSource *capturer = FastAudioCapturerSource::GetInstance();
    ASSERT_NE(capturer, nullptr);

    auto ret = capturer->InitWithoutAttr();
    EXPECT_EQ(ret, SUCCESS);

    bool isInit = capturer->IsInited();
    EXPECT_NE(isInit, true);

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

    ret = capturer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Flush();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_008
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_008, TestSize.Level1)
{
    FastAudioCapturerSource *capturer = FastAudioCapturerSource::GetInstance();
    ASSERT_NE(capturer, nullptr);

    string usbInfo;
    auto ret = capturer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->SetMute(true);
    EXPECT_NE(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    AudioParamKey key = USB_DEVICE;
    std::string condition;
    std::string param = capturer->GetAudioParameter(key, condition);
    EXPECT_EQ(param, "");

    float amolitude = capturer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    std::vector<int32_t> appsUid;
    ret = capturer->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t captureId = 0;
    ret = capturer->GetCaptureId(captureId);
    EXPECT_EQ(ret, SUCCESS);

    ret  = capturer->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    capturer->SetAddress(address);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);

    capturer->RegisterParameterCallback(nullptr);
    capturer->RegisterAudioCapturerSourceCallback(nullptr);
    capturer->RegisterWakeupCloseCallback(nullptr);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_009
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance And Actions
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_009, TestSize.Level1)
{
    FastAudioCapturerSource *capturer = FastAudioCapturerSource::GetVoipInstance();
    ASSERT_NE(capturer, nullptr);

    auto ret = capturer->InitWithoutAttr();
    EXPECT_EQ(ret, SUCCESS);

    bool isInit = capturer->IsInited();
    EXPECT_NE(isInit, true);

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

    ret = capturer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Flush();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_010
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_010, TestSize.Level1)
{
    FastAudioCapturerSource *capturer = FastAudioCapturerSource::GetVoipInstance();
    ASSERT_NE(capturer, nullptr);

    string usbInfo;
    auto ret = capturer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->SetMute(true);
    EXPECT_NE(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    AudioParamKey key = USB_DEVICE;
    std::string condition;
    std::string param = capturer->GetAudioParameter(key, condition);
    EXPECT_EQ(param, "");

    float amolitude = capturer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    std::vector<int32_t> appsUid;
    ret = capturer->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t captureId = 0;
    ret = capturer->GetCaptureId(captureId);
    EXPECT_EQ(ret, SUCCESS);

    ret  = capturer->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    capturer->SetAddress(address);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);

    capturer->RegisterParameterCallback(nullptr);
    capturer->RegisterAudioCapturerSourceCallback(nullptr);
    capturer->RegisterWakeupCloseCallback(nullptr);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_011
 * @tc.desc  : Test Template AudioCapturerSource call GetInstance And Actions
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_011, TestSize.Level1)
{
    BluetoothCapturerSource *capturer = BluetoothCapturerSource::GetInstance();
    ASSERT_NE(capturer, nullptr);

    bool isInit = capturer->IsInited();
    EXPECT_NE(isInit, true);

    float left = 0;
    float right = 0;
    auto ret = capturer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->GetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturer->Flush();
    EXPECT_EQ(ret, SUCCESS);

    capturer->DeInit();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_012
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_012, TestSize.Level1)
{
    BluetoothCapturerSource *capturer = BluetoothCapturerSource::GetInstance();
    ASSERT_NE(capturer, nullptr);

    string usbInfo;
    auto ret = capturer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);
    
    ret = capturer->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);

    bool isMute = false;
    ret = capturer->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetInputRoute(DEVICE_TYPE_MIC, "primary");
    EXPECT_NE(ret, SUCCESS);

    ret = capturer->SetAudioScene(AUDIO_SCENE_PHONE_CALL, DEVICE_TYPE_MIC);
    EXPECT_NE(ret, SUCCESS);

    AudioParamKey key = USB_DEVICE;
    std::string condition;
    std::string param = capturer->GetAudioParameter(key, condition);
    EXPECT_EQ(param, "");

    float amolitude = capturer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    std::vector<int32_t> appsUid;
    ret = capturer->UpdateAppsUid(appsUid);
    EXPECT_NE(ret, SUCCESS);

    uint32_t captureId = 0;
    ret = capturer->GetCaptureId(captureId);
    EXPECT_EQ(ret, SUCCESS);

    ret  = capturer->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    capturer->SetAddress(address);

    auto id = capturer->GetTransactionId();
    EXPECT_EQ(id, 0);

    capturer->RegisterParameterCallback(nullptr);
    capturer->RegisterAudioCapturerSourceCallback(nullptr);
    capturer->RegisterWakeupCloseCallback(nullptr);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_013
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_013, TestSize.Level1)
{
    AudioCapturerSourceInner capturer;
    AudioFormat format = capturer.ConvertToHdiFormat(HdiAdapterFormat::SAMPLE_U8);
    EXPECT_EQ(format, AUDIO_FORMAT_TYPE_PCM_8_BIT);
    format = capturer.ConvertToHdiFormat(HdiAdapterFormat::SAMPLE_S16);
    EXPECT_EQ(format, AUDIO_FORMAT_TYPE_PCM_16_BIT);
    format = capturer.ConvertToHdiFormat(HdiAdapterFormat::SAMPLE_S24);
    EXPECT_EQ(format, AUDIO_FORMAT_TYPE_PCM_24_BIT);
    format =capturer.ConvertToHdiFormat(HdiAdapterFormat::SAMPLE_S32);
    EXPECT_EQ(format, AUDIO_FORMAT_TYPE_PCM_32_BIT);
    format =capturer.ConvertToHdiFormat(HdiAdapterFormat::SAMPLE_F32);
    EXPECT_EQ(format, AUDIO_FORMAT_TYPE_PCM_16_BIT);

    int32_t ret = GetByteSizeByFormat(HdiAdapterFormat::SAMPLE_U8);
    EXPECT_EQ(ret, BYTE_SIZE_SAMPLE_U8);
    ret = GetByteSizeByFormat(HdiAdapterFormat::SAMPLE_S16);
    EXPECT_EQ(ret, BYTE_SIZE_SAMPLE_S16);
    ret = GetByteSizeByFormat(HdiAdapterFormat::SAMPLE_S24);
    EXPECT_EQ(ret, BYTE_SIZE_SAMPLE_S24);
    ret = GetByteSizeByFormat(HdiAdapterFormat::SAMPLE_S32);
    EXPECT_EQ(ret, BYTE_SIZE_SAMPLE_S32);
    ret = GetByteSizeByFormat(HdiAdapterFormat::SAMPLE_F32);
    EXPECT_EQ(ret, BYTE_SIZE_SAMPLE_S16);

    uint32_t id = GenerateUniqueIDBySource(SOURCE_TYPE_EC);
    EXPECT_NE(id, 0);
    id = GenerateUniqueIDBySource(SOURCE_TYPE_MIC_REF);
    EXPECT_NE(id, 0);
    id = GenerateUniqueIDBySource(SOURCE_TYPE_WAKEUP);
    EXPECT_NE(id, 0);

    uint64_t layout = GetChannelLayoutByCount(MONO);
    EXPECT_EQ(layout, CH_LAYOUT_MONO);
    layout = GetChannelLayoutByCount(STEREO);
    EXPECT_EQ(layout, CH_LAYOUT_STEREO);
    layout = GetChannelLayoutByCount(CHANNEL_4);
    EXPECT_EQ(layout, CH_LAYOUT_QUAD);
    layout = GetChannelLayoutByCount(CHANNEL_8);
    EXPECT_EQ(layout, CH_LAYOUT_7POINT1);
    layout = GetChannelLayoutByCount(CHANNEL_16);
    EXPECT_EQ(layout, CH_LAYOUT_STEREO);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_014
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Get Or Set Value
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_014, TestSize.Level1)
{
    auto res = ConvertToHDIAudioInputType(SOURCE_TYPE_INVALID);
    EXPECT_EQ(res, AUDIO_INPUT_DEFAULT_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_MIC);
    EXPECT_EQ(res, AUDIO_INPUT_MIC_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_WAKEUP);
    EXPECT_EQ(res, AUDIO_INPUT_SPEECH_WAKEUP_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_VOICE_COMMUNICATION);
    EXPECT_EQ(res, AUDIO_INPUT_VOICE_COMMUNICATION_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_VOICE_CALL);
    EXPECT_EQ(res, AUDIO_INPUT_VOICE_CALL_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_VOICE_RECOGNITION);
    EXPECT_EQ(res, AUDIO_INPUT_VOICE_RECOGNITION_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_EC);
    EXPECT_EQ(res, AUDIO_INPUT_EC_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_MIC_REF);
    EXPECT_EQ(res, AUDIO_INPUT_NOISE_REDUCTION_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_CAMCORDER);
    EXPECT_EQ(res, AUDIO_INPUT_CAMCORDER_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_UNPROCESSED);
    EXPECT_EQ(res, AUDIO_INPUT_RAW_TYPE);
    res = ConvertToHDIAudioInputType(SOURCE_TYPE_MAX);
    EXPECT_EQ(res, AUDIO_INPUT_NOISE_REDUCTION_TYPE);

    AudioCategory audioCategory = GetAudioCategory(AUDIO_SCENE_PHONE_CALL);
    EXPECT_EQ(audioCategory, AUDIO_IN_CALL);
    audioCategory = GetAudioCategory(AUDIO_SCENE_PHONE_CHAT);
    EXPECT_EQ(audioCategory, AUDIO_IN_COMMUNICATION);
    audioCategory = GetAudioCategory(AUDIO_SCENE_RINGING);
    EXPECT_EQ(audioCategory, AUDIO_IN_RINGTONE);
    audioCategory = GetAudioCategory(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(audioCategory, AUDIO_IN_MEDIA);
    audioCategory = GetAudioCategory(AUDIO_SCENE_MAX);
    EXPECT_EQ(audioCategory, AUDIO_IN_MEDIA);

    AudioRouteNode source;
    auto ret = SetInputPortPin(DEVICE_TYPE_MIC, source);
    EXPECT_EQ(ret, SUCCESS);
    ret = SetInputPortPin(DEVICE_TYPE_WIRED_HEADSET, source);
    EXPECT_EQ(ret, SUCCESS);
    ret = SetInputPortPin(DEVICE_TYPE_USB_ARM_HEADSET, source);
    EXPECT_EQ(ret, SUCCESS);
    ret = SetInputPortPin(DEVICE_TYPE_USB_HEADSET, source);
    EXPECT_EQ(ret, SUCCESS);
    ret = SetInputPortPin(DEVICE_TYPE_BLUETOOTH_SCO, source);
    EXPECT_EQ(ret, SUCCESS);
    ret = SetInputPortPin(DEVICE_TYPE_MAX, source);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_015
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Init
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_015, TestSize.Level1)
{
    AudioCapturerSourceInner capturerSourceInner("primary");

    auto ret = capturerSourceInner.InitAdapterAndCapture();
    EXPECT_NE(ret, SUCCESS);
    
    ret = capturerSourceInner.StartNonblockingCapture();
    EXPECT_NE(ret, SUCCESS);

    ret = capturerSourceInner.StopNonblockingCapture();
    EXPECT_EQ(ret, SUCCESS);

    AudioSampleAttributes attrs;
    capturerSourceInner.InitAttrsCapture(attrs);
    capturerSourceInner.SetEcSampleAttributes(attrs);
    capturerSourceInner.CaptureThreadLoop();
}

/**
 * @tc.name  : Test Template AudioCapturerSource
 * @tc.number: Audio_Capturer_Source_016
 * @tc.desc  : Test Template AudioCapturerSource call Create Then Update Usb Attrs
 */
HWTEST(AudioCapturerSourceUnitTest, AudioCapturerSourceUnitTest_016, TestSize.Level1)
{
    AudioCapturerSourceInner capturerSourceInner("usb");

    std::string usbInfo = "source_rate:16000;source_format:AUDIO_FORMAT_PCM_16_BIT";
    auto ret = capturerSourceInner.UpdateUsbAttrs(usbInfo);
    EXPECT_EQ(ret, SUCCESS);
    
    usbInfo = "source_rate:24000;source_format:AUDIO_FORMAT_PCM_24_BIT";
    ret = capturerSourceInner.UpdateUsbAttrs(usbInfo);
    EXPECT_EQ(ret, SUCCESS);
    
    usbInfo = "source_rate:32000;source_format:AUDIO_FORMAT_PCM_32_BIT";
    ret = capturerSourceInner.UpdateUsbAttrs(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    usbInfo = "source_rate:48000;source_format:AUDIO_FORMAT_PCM_48_BIT";
    ret = capturerSourceInner.UpdateUsbAttrs(usbInfo);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
