/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "audio_errors.h"

#include "fast_audio_renderer_sink.h"
#include "audio_renderer_sink.h"
#include "bluetooth_renderer_sink.h"
#include "remote_audio_renderer_sink.h"
#include "audio_renderer_file_sink.h"
#include "offload_audio_renderer_sink.h"
#include "multichannel_audio_renderer_sink.h"


using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRenderSinkUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AudioRenderSinkUnitTest::SetUpTestCase()
{}

void AudioRenderSinkUnitTest::TearDownTestCase()
{}

void AudioRenderSinkUnitTest::SetUp()
{}

void AudioRenderSinkUnitTest::TearDown()
{}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_001
 * @tc.desc  : Test Template AudioRendererSink call GetInstance And Actions
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_001, TestSize.Level1)
{
    IMmapAudioRendererSink *renderer = FastAudioRendererSink::GetInstance();
    ASSERT_NE(renderer, nullptr);

    bool isInit = renderer->IsInited();
    EXPECT_NE(isInit, true);

    auto ret = renderer->Start();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    renderer->ResetOutputRouteForDisconnect(DEVICE_TYPE_SPEAKER);

    float left = 0;
    float right = 0;
    ret = renderer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetVoiceVolume(left);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Stop();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Reset();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->Flush();
    EXPECT_EQ(ret, SUCCESS);

    renderer->RegisterAudioSinkCallback(nullptr);
    renderer->DeInit();
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_002
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_002, TestSize.Level1)
{
    IMmapAudioRendererSink *renderer = FastAudioRendererSink::GetInstance();
    ASSERT_NE(renderer, nullptr);

    string usbInfo;
    auto ret = renderer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t latency = 0;
    ret = renderer->GetLatency(&latency);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    std::vector<DeviceType> activeDevices;
    ret = renderer->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevices);
    EXPECT_NE(ret, SUCCESS);

    std::vector<DeviceType> outputDevices;
    ret = renderer->SetOutputRoutes(outputDevices);
    EXPECT_NE(ret, SUCCESS);

    std::string condition;
    std::string value;
    renderer->SetAudioParameter(VOLUME, condition, value);

    std::vector<int32_t> appsUid;
    ret = renderer->UpdateAppsUid(appsUid);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPaPower(1);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPriPaPower();
    EXPECT_NE(ret, SUCCESS);

    uint32_t renderId = 0;
    ret = renderer->GetRenderId(renderId);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    renderer->SetAddress(address);

    uint64_t transactionId = 0;
    ret = renderer->GetTransactionId(&transactionId);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    renderer->SetAudioMonoState(true);
    renderer->SetAudioBalanceValue(1);

    float amolitude = renderer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    ret = renderer->SetRenderEmpty(20000);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_003
 * @tc.desc  : Test Template AudioRendererSink call GetInstance And Actions
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_003, TestSize.Level1)
{
    AudioRendererSink *renderer = AudioRendererSink::GetInstance("usb");
    ASSERT_NE(renderer, nullptr);

    bool isInit = renderer->IsInited();
    EXPECT_NE(isInit, true);

    auto ret = renderer->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    renderer->ResetOutputRouteForDisconnect(DEVICE_TYPE_SPEAKER);

    float left = 0;
    float right = 0;
    ret = renderer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetVoiceVolume(left);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Stop();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Reset();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Flush();
    EXPECT_NE(ret, SUCCESS);

    renderer->RegisterAudioSinkCallback(nullptr);
    renderer->DeInit();
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_004
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_004, TestSize.Level1)
{
    AudioRendererSink *renderer = AudioRendererSink::GetInstance("usb");
    ASSERT_NE(renderer, nullptr);

    string usbInfo;
    auto ret = renderer->Preload(usbInfo);
    EXPECT_NE(ret, SUCCESS);

    uint32_t latency = 0;
    ret = renderer->GetLatency(&latency);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, AUDIO_SCENE_INVALID);

    std::vector<DeviceType> activeDevices;
    ret = renderer->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevices);
    EXPECT_NE(ret, SUCCESS);

    std::vector<DeviceType> outputDevices;
    ret = renderer->SetOutputRoutes(outputDevices);
    EXPECT_NE(ret, SUCCESS);

    std::string condition;
    std::string value;
    renderer->SetAudioParameter(VOLUME, condition, value);

    std::vector<int32_t> appsUid;
    ret = renderer->UpdateAppsUid(appsUid);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPaPower(1);
    EXPECT_EQ(ret, SUCCESS);

    ret  = renderer->SetPriPaPower();
    EXPECT_NE(ret, SUCCESS);

    uint32_t renderId = 0;
    ret = renderer->GetRenderId(renderId);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    renderer->SetAddress(address);

    uint64_t transactionId = 0;
    ret = renderer->GetTransactionId(&transactionId);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);

    renderer->SetAudioMonoState(true);
    renderer->SetAudioBalanceValue(1);

    float amolitude = renderer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    ret = renderer->SetRenderEmpty(20000);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_005
 * @tc.desc  : Test Template AudioRendererSink call GetInstance And Actions
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_005, TestSize.Level1)
{
    BluetoothRendererSink *renderer = BluetoothRendererSink::GetInstance();
    ASSERT_NE(renderer, nullptr);

    bool isInit = renderer->IsInited();
    EXPECT_NE(isInit, true);

    auto ret = renderer->Start();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    renderer->ResetOutputRouteForDisconnect(DEVICE_TYPE_SPEAKER);

    float left = 0;
    float right = 0;
    ret = renderer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetVoiceVolume(left);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Stop();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Reset();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Flush();
    EXPECT_NE(ret, SUCCESS);

    renderer->RegisterAudioSinkCallback(nullptr);
    renderer->DeInit();
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_006
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_006, TestSize.Level1)
{
    BluetoothRendererSink *renderer = BluetoothRendererSink::GetInstance();
    ASSERT_NE(renderer, nullptr);

    string usbInfo;
    auto ret = renderer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t latency = 0;
    ret = renderer->GetLatency(&latency);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    std::vector<DeviceType> activeDevices;
    ret = renderer->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevices);
    EXPECT_NE(ret, SUCCESS);

    std::vector<DeviceType> outputDevices;
    ret = renderer->SetOutputRoutes(outputDevices);
    EXPECT_NE(ret, SUCCESS);

    std::string condition;
    std::string value;
    renderer->SetAudioParameter(VOLUME, condition, value);

    std::vector<int32_t> appsUid;
    ret = renderer->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    ret  = renderer->SetPaPower(1);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPriPaPower();
    EXPECT_NE(ret, SUCCESS);

    uint32_t renderId = 0;
    ret = renderer->GetRenderId(renderId);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    renderer->SetAddress(address);

    uint64_t transactionId = 0;
    ret = renderer->GetTransactionId(&transactionId);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);

    renderer->SetAudioMonoState(true);
    renderer->SetAudioBalanceValue(1);

    float amolitude = renderer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    ret = renderer->SetRenderEmpty(20000);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_007
 * @tc.desc  : Test Template AudioRendererSink call GetInstance And Actions
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_007, TestSize.Level1)
{
    IMmapAudioRendererSink *renderer = BluetoothRendererSink::GetMmapInstance();
    ASSERT_NE(renderer, nullptr);

    bool isInit = renderer->IsInited();
    EXPECT_NE(isInit, true);

    auto ret = renderer->Start();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    renderer->ResetOutputRouteForDisconnect(DEVICE_TYPE_SPEAKER);

    float left = 0;
    float right = 0;
    ret = renderer->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetVoiceVolume(left);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Stop();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Reset();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Flush();
    EXPECT_NE(ret, SUCCESS);

    renderer->RegisterAudioSinkCallback(nullptr);
    renderer->DeInit();
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_008
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_008, TestSize.Level1)
{
    IMmapAudioRendererSink *renderer = BluetoothRendererSink::GetMmapInstance();
    ASSERT_NE(renderer, nullptr);

    string usbInfo;
    auto ret = renderer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t latency = 0;
    ret = renderer->GetLatency(&latency);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    std::vector<DeviceType> activeDevices;
    ret = renderer->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevices);
    EXPECT_NE(ret, SUCCESS);

    std::vector<DeviceType> outputDevices;
    ret = renderer->SetOutputRoutes(outputDevices);
    EXPECT_NE(ret, SUCCESS);

    std::string condition;
    std::string value;
    renderer->SetAudioParameter(VOLUME, condition, value);

    std::vector<int32_t> appsUid;
    ret = renderer->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    ret  = renderer->SetPaPower(1);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPriPaPower();
    EXPECT_NE(ret, SUCCESS);

    uint32_t renderId = 0;
    ret = renderer->GetRenderId(renderId);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    renderer->SetAddress(address);

    uint64_t transactionId = 0;
    ret = renderer->GetTransactionId(&transactionId);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);

    renderer->SetAudioMonoState(true);
    renderer->SetAudioBalanceValue(1);

    float amolitude = renderer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    ret = renderer->SetRenderEmpty(20000);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_009
 * @tc.desc  : Test Template AudioRendererSink call GetInstance And Actions
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_009, TestSize.Level1)
{
    RemoteAudioRendererSink *renderer = RemoteAudioRendererSink::GetInstance("12345");
    ASSERT_NE(renderer, nullptr);

    bool isInit = renderer->IsInited();
    EXPECT_NE(isInit, true);

    auto ret = renderer->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    renderer->ResetOutputRouteForDisconnect(DEVICE_TYPE_SPEAKER);

    float left = 0;
    float right = 0;
    ret = renderer->SetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetVoiceVolume(left);
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Pause();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->Resume();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->Reset();
    EXPECT_NE(ret, SUCCESS);

    ret = renderer->Flush();
    EXPECT_NE(ret, SUCCESS);

    renderer->RegisterAudioSinkCallback(nullptr);
    renderer->DeInit();
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_010
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_010, TestSize.Level1)
{
    RemoteAudioRendererSink *renderer = RemoteAudioRendererSink::GetInstance("12345");
    ASSERT_NE(renderer, nullptr);

    string usbInfo;
    auto ret = renderer->Preload(usbInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t latency = 0;
    ret = renderer->GetLatency(&latency);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    std::vector<DeviceType> activeDevices;
    ret = renderer->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevices);
    EXPECT_NE(ret, SUCCESS);

    std::vector<DeviceType> outputDevices;
    ret = renderer->SetOutputRoutes(outputDevices);
    EXPECT_NE(ret, SUCCESS);

    std::string condition;
    std::string value;
    renderer->SetAudioParameter(VOLUME, condition, value);

    std::vector<int32_t> appsUid;
    ret = renderer->UpdateAppsUid(appsUid);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPaPower(1);
    EXPECT_NE(ret, SUCCESS);

    ret  = renderer->SetPriPaPower();
    EXPECT_NE(ret, SUCCESS);

    uint32_t renderId = 0;
    ret = renderer->GetRenderId(renderId);
    EXPECT_EQ(ret, SUCCESS);

    std::string address;
    renderer->SetAddress(address);

    uint64_t transactionId = 0;
    ret = renderer->GetTransactionId(&transactionId);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    renderer->SetAudioMonoState(true);
    renderer->SetAudioBalanceValue(1);

    float amolitude = renderer->GetMaxAmplitude();
    EXPECT_EQ(amolitude, 0.0);

    ret = renderer->SetRenderEmpty(20000);
    EXPECT_EQ(ret, SUCCESS);

    ret = renderer->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_011
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_011, TestSize.Level1)
{
    MultiChannelRendererSink *renderer = MultiChannelRendererSink::GetInstance("multichannel");
    ASSERT_NE(renderer, nullptr);

    auto ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, AUDIO_SCENE_INVALID);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_012
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_012, TestSize.Level1)
{
    OffloadRendererSink *renderer = OffloadRendererSink::GetInstance();
    ASSERT_NE(renderer, nullptr);

    auto ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test Template AudioRendererSink
 * @tc.number: AudioRendererSink_013
 * @tc.desc  : Test Template AudioRendererSink call Create Then Get Or Set Value
 */
HWTEST(AudioRenderSinkUnitTest, AudioRenderSinkUnitTest_013, TestSize.Level1)
{
    AudioRendererFileSink *renderer = AudioRendererFileSink::GetInstance();
    ASSERT_NE(renderer, nullptr);

    auto ret = renderer->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
} // namespace AudioStandard
} // namespace OHOS