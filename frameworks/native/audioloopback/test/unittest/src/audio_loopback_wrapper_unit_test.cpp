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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_loopback_wrapper_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void LoopbackCallbackUnitTest::SetUpTestCase(void) {}
void LoopbackCallbackUnitTest::TearDownTestCase(void) {}
void LoopbackCallbackUnitTest::SetUp() {}
void LoopbackCallbackUnitTest::TearDown() {}

HWTEST_F(LoopbackCallbackUnitTest, OnCommandResult_CallbackValid_001, TestSize.Level1)
{
    class TestILoopbackCB : public ILoopbackCB {
    public:
        int commandReceived = 0;
        bool successReceived = false;
        int errorCodeReceived = 0;
        void OnCommandResult(int command, bool success, int errorCode) override {
            commandReceived = command;
            successReceived = success;
            errorCodeReceived = errorCode;
        }
        void OnStatusChange(int status, int reason) override {}
    };

    auto cb = std::make_shared<TestILoopbackCB>();
    auto callback = new LoopbackCallback(cb);

    callback->OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS);
    EXPECT_EQ(cb->commandReceived, LOOPBACK_CMD_ENABLE);
    EXPECT_EQ(cb->successReceived, true);
    EXPECT_EQ(cb->errorCodeReceived, SUCCESS);
}

HWTEST_F(LoopbackCallbackUnitTest, OnCommandResult_CallbackExpired_001, TestSize.Level1)
{
    class TestILoopbackCB : public ILoopbackCB {
    public:
        int callCount = 0;
        void OnCommandResult(int command, bool success, int errorCode) override { callCount++; }
        void OnStatusChange(int status, int reason) override {}
    };

    auto cb = std::make_shared<TestILoopbackCB>();
    auto callback = new LoopbackCallback(cb);
    cb = nullptr;

    int32_t result = callback->OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(LoopbackCallbackUnitTest, OnStatusChange_CallbackValid_001, TestSize.Level1)
{
    class TestILoopbackCB : public ILoopbackCB {
    public:
        int statusReceived = 0;
        int reasonReceived = 0;
        void OnCommandResult(int command, bool success, int errorCode) override {}
        void OnStatusChange(int status, int reason) override {
            statusReceived = status;
            reasonReceived = reason;
        }
    };

    auto cb = std::make_shared<TestILoopbackCB>();
    auto callback = new LoopbackCallback(cb);

    callback->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM);
    EXPECT_EQ(cb->statusReceived, LOOPBACK_AVAILABLE_RUNNING);
    EXPECT_EQ(cb->reasonReceived, CMD_FROM_SYSTEM);
}

HWTEST_F(LoopbackCallbackUnitTest, OnStatusChange_CallbackExpired_001, TestSize.Level1)
{
    class TestILoopbackCB : public ILoopbackCB {
    public:
        int callCount = 0;
        void OnCommandResult(int command, bool success, int errorCode) override {}
        void OnStatusChange(int status, int reason) override { callCount++; }
    };

    auto cb = std::make_shared<TestILoopbackCB>();
    auto callback = new LoopbackCallback(cb);
    cb = nullptr;

    int32_t result = callback->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM);
    EXPECT_EQ(result, SUCCESS);
}

void AudioLoopbackWrapperUnitTest::SetUpTestCase(void) {}
void AudioLoopbackWrapperUnitTest::TearDownTestCase(void) {}

void AudioLoopbackWrapperUnitTest::SetUp()
{
    appInfo_.appPid = 1000; // test pid
    appInfo_.appUid = 1000; // test uid
    mockLoopback_ = new MockILoopback();
    mockPrivate_ = std::make_shared<MockAudioLoopbackPrivate>(LOOPBACK_HARDWARE, appInfo_);
}

void AudioLoopbackWrapperUnitTest::TearDown()
{
    mockLoopback_ = nullptr;
    mockPrivate_ = nullptr;
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Constructor_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    EXPECT_NE(wrapper, nullptr);
    EXPECT_EQ(wrapper->GetType(), LOOPBACK_TYPE_NORMAL);
    EXPECT_EQ(wrapper->GetMode(), LOOPBACK_HARDWARE);
    EXPECT_NE(wrapper->GetPrivate(), nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Constructor_GlobalHandler_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    EXPECT_NE(wrapper, nullptr);
    EXPECT_EQ(wrapper->GetType(), LOOPBACK_TYPE_GLOBAL_HANDLER);
    EXPECT_NE(wrapper->GetPrivate(), nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Constructor_GlobalControl_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    EXPECT_NE(wrapper, nullptr);
    EXPECT_EQ(wrapper->GetType(), LOOPBACK_TYPE_GLOBAL_CONTROL);
    EXPECT_EQ(wrapper->GetPrivate(), nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Normal_Success_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(true));

    bool result = wrapper->Enable(true);
    EXPECT_TRUE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Normal_IpcFailed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(ERR_OPERATION_FAILED),
        Return(SUCCESS)));

    bool result = wrapper->Enable(true);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Normal_NoProxy_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(true));

    bool result = wrapper->Enable(true);
    EXPECT_TRUE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Control_NoProxy_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    bool result = wrapper->Enable(true);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_UnknownType_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, static_cast<LoopbackType>(100),
        appInfo_);

    bool result = wrapper->Enable(true);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Handler_Success_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(true));

    bool result = wrapper->Enable(true);
    EXPECT_TRUE(result);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Handler_NoProxy_Failed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);
    EXPECT_CALL(*mockPrivate_, Enable(_)).Times(0);

    bool result = wrapper->Enable(true);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Handler_ProxyFailed_PrivateStillEnabled_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(ERR_OPERATION_FAILED),
        Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(true));

    bool result = wrapper->Enable(true);
    EXPECT_TRUE(result);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Handler_PrivateFailed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(false));

    bool result = wrapper->Enable(true);
    EXPECT_FALSE(result);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Handler_PrivateNull_Failed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    bool result = wrapper->Enable(true);
    EXPECT_FALSE(result);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_Control_NoProxy_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, static_cast<AudioLoopbackStatus>(-1));
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_Control_IpcFailed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetStatus(_, _)).WillOnce(DoAll(SetArgReferee<0>(LOOPBACK_AVAILABLE_IDLE),
        SetArgReferee<1>(ERR_OPERATION_FAILED), Return(SUCCESS)));

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, static_cast<AudioLoopbackStatus>(-1));
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, GetStatus()).WillOnce(Return(LOOPBACK_AVAILABLE_RUNNING));

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, LOOPBACK_AVAILABLE_RUNNING);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_NoPrivate_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, LOOPBACK_AVAILABLE_IDLE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetVolume_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    int32_t result = wrapper->SetVolume(0.5f);
    EXPECT_EQ(result, ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetVolume_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, SetVolume(0.5f)).WillOnce(Return(SUCCESS));

    int32_t result = wrapper->SetVolume(0.5f);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetVolume_NoPrivate_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);

    int32_t result = wrapper->SetVolume(0.5f);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetVolume_Control_Success_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetVolume(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(0.5f), SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    float volume = wrapper->GetVolume();
    EXPECT_EQ(volume, 0.5f);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetVolume_Control_NoProxy_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    float volume = wrapper->GetVolume();
    EXPECT_EQ(volume, 0.0f);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetVolume_Control_IpcFailed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetVolume(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(0.3f), SetArgReferee<1>(ERR_OPERATION_FAILED), Return(SUCCESS)));

    float volume = wrapper->GetVolume();
    EXPECT_EQ(volume, 0.0f);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetVolume_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, GetVolume()).WillOnce(Return(0.8f));

    float volume = wrapper->GetVolume();
    EXPECT_EQ(volume, 0.8f);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetVolume_NoPrivate_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);

    float volume = wrapper->GetVolume();
    EXPECT_EQ(volume, 0.0f);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetReverbPreset_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    bool result = wrapper->SetReverbPreset(REVERB_PRESET_KTV);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetReverbPreset_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, SetReverbPreset(REVERB_PRESET_KTV)).WillOnce(Return(true));

    bool result = wrapper->SetReverbPreset(REVERB_PRESET_KTV);
    EXPECT_TRUE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetReverbPreset_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    AudioLoopbackReverbPreset preset = wrapper->GetReverbPreset();
    EXPECT_EQ(preset, REVERB_PRESET_ORIGINAL);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetReverbPreset_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, GetReverbPreset()).WillOnce(Return(REVERB_PRESET_CONCERT));

    AudioLoopbackReverbPreset preset = wrapper->GetReverbPreset();
    EXPECT_EQ(preset, REVERB_PRESET_CONCERT);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetEqualizerPreset_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    bool result = wrapper->SetEqualizerPreset(EQUALIZER_PRESET_FULL);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetEqualizerPreset_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, SetEqualizerPreset(EQUALIZER_PRESET_FULL)).WillOnce(Return(true));

    bool result = wrapper->SetEqualizerPreset(EQUALIZER_PRESET_FULL);
    EXPECT_TRUE(result);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetEqualizerPreset_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    AudioLoopbackEqualizerPreset preset = wrapper->GetEqualizerPreset();
    EXPECT_EQ(preset, EQUALIZER_PRESET_FLAT);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetEqualizerPreset_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, GetEqualizerPreset()).WillOnce(Return(EQUALIZER_PRESET_BRIGHT));

    AudioLoopbackEqualizerPreset preset = wrapper->GetEqualizerPreset();
    EXPECT_EQ(preset, EQUALIZER_PRESET_BRIGHT);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetSupportedDevicePairs_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    auto pairs = wrapper->GetSupportedDevicePairs();
    EXPECT_TRUE(pairs.empty());
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetSupportedDevicePairs_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    std::vector<AudioDevicePair> expectedPairs = {};
    EXPECT_CALL(*mockPrivate_, GetSupportedDevicePairs()).WillOnce(Return(expectedPairs));

    auto pairs = wrapper->GetSupportedDevicePairs();
    EXPECT_TRUE(pairs.empty());
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetPreferredDevicePair_Control_NotSupported_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    auto pair = wrapper->GetPreferredDevicePair();
    EXPECT_TRUE(pair.first == nullptr || pair.first->deviceType_ == DEVICE_TYPE_NONE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetPreferredDevicePair_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    AudioDevicePair expectedPair;
    EXPECT_CALL(*mockPrivate_, GetPreferredDevicePair()).WillOnce(Return(expectedPair));

    auto pair = wrapper->GetPreferredDevicePair();
    EXPECT_TRUE(pair.first == nullptr || pair.first->deviceType_ == DEVICE_TYPE_NONE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetAudioLoopbackCallback_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    EXPECT_CALL(*mockPrivate_, SetAudioLoopbackCallback(_)).WillOnce(Return(SUCCESS));

    int32_t result = wrapper->SetAudioLoopbackCallback(callback);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, SetAudioLoopbackCallback_NoPrivate_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    auto callback = std::make_shared<MockAudioLoopbackCallback>();

    int32_t result = wrapper->SetAudioLoopbackCallback(callback);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, RemoveAudioLoopbackCallback_Normal_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, RemoveAudioLoopbackCallback()).WillOnce(Return(SUCCESS));

    int32_t result = wrapper->RemoveAudioLoopbackCallback();
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, RemoveAudioLoopbackCallback_NoPrivate_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    int32_t result = wrapper->RemoveAudioLoopbackCallback();
    EXPECT_EQ(result, SUCCESS);
}

// Init/CreateLoopbackIpc talk to the real audio policy service, same as the
// on-device scenario tests in audio_loopback_unit_test.cpp.
HWTEST_F(AudioLoopbackWrapperUnitTest, Init_CreateLoopbackIpc_Success_001, TestSize.Level1)
{
    // LOOPBACK_HARDWARE + LOOPBACK_TYPE_NORMAL needs no system permission on the server side,
    // so Init succeeds while audio_server is running and yields a usable ILoopback proxy.
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);

    int32_t result = wrapper->Init();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_NE(wrapper->GetLoopbackProxy(), nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Init_CreateLoopbackIpc_Failed_001, TestSize.Level1)
{
    // Server-side AudioLoopbackManager rejects mode != LOOPBACK_HARDWARE with ERR_NOT_SUPPORTED
    // (NORMAL type skips the system-permission check, so the rejection is stable regardless of
    // the caller token); when the policy service is unreachable the client returns ERR_NULL_POINTER.
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(static_cast<AudioLoopbackMode>(100),
        LOOPBACK_TYPE_NORMAL, appInfo_);

    int32_t result = wrapper->Init();

    EXPECT_NE(result, SUCCESS);
    EXPECT_EQ(wrapper->GetLoopbackProxy(), nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, CreateAudioLoopback_FourParam_InvalidMode_001, TestSize.Level1)
{
    int32_t result = SUCCESS;
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(static_cast<AudioLoopbackMode>(100),
        LOOPBACK_TYPE_NORMAL, result, appInfo_);

    EXPECT_EQ(result, ERROR_PLATFORM_NOT_SUPPORT);
    EXPECT_EQ(audioLoopback, nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, CreateAudioLoopback_TwoParam_InvalidMode_001, TestSize.Level1)
{
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(static_cast<AudioLoopbackMode>(100), appInfo_);

    EXPECT_EQ(audioLoopback, nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, CreateAudioLoopback_TwoParam_Success_001, TestSize.Level1)
{
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, appInfo_);

    ASSERT_NE(audioLoopback, nullptr);
    EXPECT_GT(audioLoopback->GetDebugKey(), 0u);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, CreateAudioLoopback_FourParam_Success_001, TestSize.Level1)
{
    int32_t result = ERROR;
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL,
        result, appInfo_);
    ASSERT_NE(audioLoopback, nullptr);
    EXPECT_EQ(result, SUCCESS);

    uint32_t debugKeyAfterFirst = audioLoopback->GetDebugKey();
    EXPECT_GT(debugKeyAfterFirst, 0u);

    auto audioLoopbackAgain = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, appInfo_);
    ASSERT_NE(audioLoopbackAgain, nullptr);
    EXPECT_EQ(audioLoopback.get(), audioLoopbackAgain.get());
    EXPECT_EQ(audioLoopbackAgain->GetDebugKey(), debugKeyAfterFirst);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, CreateAudioLoopback_FourParam_InitFailed_001, TestSize.Level1)
{
    auto audioLoopback = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, appInfo_);
    ASSERT_NE(audioLoopback, nullptr);

    auto wrapper = static_cast<AudioLoopbackWrapper *>(audioLoopback.get());
    wrapper->type_ = LOOPBACK_TYPE_GLOBAL_CONTROL;

    int32_t result = SUCCESS;
    auto failed = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        result, appInfo_);

    EXPECT_NE(result, SUCCESS);
    EXPECT_EQ(failed, nullptr);

    wrapper->type_ = LOOPBACK_TYPE_NORMAL;
    result = ERROR;
    auto restored = AudioLoopback::CreateAudioLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, result, appInfo_);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_NE(restored, nullptr);
    EXPECT_EQ(restored.get(), audioLoopback.get());
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusChange_Handler_NotifyProxyAndCallback_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_RUNNING, _)).Times(1);
    EXPECT_CALL(*callback, OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM)).Times(1);

    wrapper->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_CLIENT);

    wrapper->RemoveAudioLoopbackCallback();
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusChange_Handler_NoProxy_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*mockLoopback_, NotifyStatus(_, _)).Times(0);
    EXPECT_CALL(*callback, OnStatusChange(_, _)).Times(0);

    wrapper->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM);

    wrapper->RemoveAudioLoopbackCallback();
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusChange_Normal_ForwardCallbackOnly_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*callback, OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM)).Times(1);
    EXPECT_CALL(*mockLoopback_, NotifyStatus(_, _)).Times(0);

    wrapper->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_CLIENT);

    wrapper->RemoveAudioLoopbackCallback();
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusChange_Handler_NoCallback_NotifyProxyOnly_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_RUNNING, _)).Times(1);

    wrapper->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_NotHandler_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    EXPECT_CALL(*mockPrivate_, SetAudioLoopbackCallback(_)).WillOnce(Return(SUCCESS));
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*mockPrivate_, Enable(_)).Times(0);
    EXPECT_CALL(*mockPrivate_, GetStatus()).Times(0);
    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);
    EXPECT_CALL(*mockLoopback_, NotifyStatus(_, _)).Times(0);
    EXPECT_CALL(*callback, OnStatusChange(_, _)).Times(0);

    wrapper->OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_GetStatus_NotifyOnce_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);

    constexpr int32_t testErrorCode = 42;
    int32_t notifiedRet = 0;
    EXPECT_CALL(*mockPrivate_, GetStatus()).WillOnce(Return(LOOPBACK_AVAILABLE_RUNNING));
    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_RUNNING, _))
        .Times(1).WillOnce(DoAll(SaveArg<1>(&notifiedRet), Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, Enable(_)).Times(0);
    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);

    wrapper->OnCommandResult(LOOPBACK_CMD_GETSTATUS, true, testErrorCode);
    EXPECT_EQ(notifiedRet, testErrorCode);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_GetStatus_NoPrivate_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(nullptr);

    EXPECT_CALL(*mockLoopback_, NotifyStatus(_, _)).Times(0);
    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);

    wrapper->OnCommandResult(LOOPBACK_CMD_GETSTATUS, true, SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_GetStatus_NoProxy_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, GetStatus()).Times(0);
    EXPECT_CALL(*mockPrivate_, Enable(_)).Times(0);
    EXPECT_CALL(*mockLoopback_, NotifyStatus(_, _)).Times(0);

    wrapper->OnCommandResult(LOOPBACK_CMD_GETSTATUS, true, SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_Enable_Success_NotifyOnce_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    EXPECT_CALL(*mockPrivate_, SetAudioLoopbackCallback(_)).WillOnce(Return(SUCCESS));
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(true));
    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, GetStatus()).WillOnce(Return(LOOPBACK_AVAILABLE_RUNNING));
    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_RUNNING, _)).Times(1);
    EXPECT_CALL(*callback, OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM)).Times(1);

    wrapper->OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_Enable_Failed_NotifyTwice_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    EXPECT_CALL(*mockPrivate_, SetAudioLoopbackCallback(_)).WillOnce(Return(SUCCESS));
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);
    EXPECT_CALL(*mockPrivate_, GetStatus()).WillOnce(Return(LOOPBACK_AVAILABLE_IDLE));
    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_IDLE, _)).Times(2);
    EXPECT_CALL(*callback, OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM)).Times(1);

    wrapper->OnCommandResult(LOOPBACK_CMD_ENABLE, false, SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_Disable_Success_NotifyTwice_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(mockPrivate_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    EXPECT_CALL(*mockPrivate_, SetAudioLoopbackCallback(_)).WillOnce(Return(SUCCESS));
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*mockPrivate_, Enable(false)).WillOnce(Return(true));
    EXPECT_CALL(*mockLoopback_, Enable(false, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_CALL(*mockPrivate_, GetStatus()).WillOnce(Return(LOOPBACK_AVAILABLE_IDLE));
    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_IDLE, _)).Times(2);
    EXPECT_CALL(*callback, OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM)).Times(1);

    wrapper->OnCommandResult(LOOPBACK_CMD_DISABLE, true, SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_NoPrivate_NotifyIdleOnce_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);
    wrapper->SetPrivate(nullptr);

    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);
    EXPECT_CALL(*mockLoopback_, NotifyStatus(LOOPBACK_AVAILABLE_IDLE, _)).Times(1);

    wrapper->OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnCommandResult_Enable_Failed_NoProxy_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        appInfo_);
    wrapper->SetPrivate(mockPrivate_);

    EXPECT_CALL(*mockPrivate_, Enable(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockPrivate_, GetStatus()).WillOnce(Return(LOOPBACK_AVAILABLE_IDLE));
    EXPECT_CALL(*mockLoopback_, NotifyStatus(_, _)).Times(0);
    EXPECT_CALL(*mockLoopback_, Enable(_, _)).Times(0);

    wrapper->OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusNotify_Control_NotifyCallback_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*callback, OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM)).Times(1);

    wrapper->OnStatusNotify(LOOPBACK_AVAILABLE_RUNNING, LOOPBACK_REASON_HANDLER_DIED);

    EXPECT_EQ(wrapper->notifyStatus_, LOOPBACK_AVAILABLE_RUNNING);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusNotify_Control_NoCallback_NoNotify_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    wrapper->OnStatusNotify(LOOPBACK_AVAILABLE_RUNNING, LOOPBACK_REASON_HANDLER_DIED);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnStatusNotify_NotControl_NoNotify_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*callback, OnStatusChange(_, _)).Times(0);

    wrapper->OnStatusNotify(LOOPBACK_AVAILABLE_RUNNING, LOOPBACK_REASON_HANDLER_DIED);

    wrapper->RemoveAudioLoopbackCallback();
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnAvailableStatusChangeFromServer_Control_NotifyCallback_001,
    TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    constexpr int32_t testReason = 1;
    AudioLoopbackAvailableStatus received = {};
    EXPECT_CALL(*callback, OnAvailableStatusChange(_)).WillOnce(SaveArg<0>(&received));

    wrapper->OnAvailableStatusChangeFromServer(true, testReason);

    EXPECT_TRUE(received.available);
    EXPECT_EQ(received.unavailableReason, static_cast<AudioLoopbackUnavailableReason>(testReason));
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnAvailableStatusChangeFromServer_Control_NoCallback_NoNotify_001,
    TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    wrapper->OnAvailableStatusChangeFromServer(true, 0);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, OnAvailableStatusChangeFromServer_NotControl_NoNotify_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, appInfo_);
    auto callback = std::make_shared<MockAudioLoopbackCallback>();
    wrapper->SetAudioLoopbackCallback(callback);

    EXPECT_CALL(*callback, OnAvailableStatusChange(_)).Times(0);

    wrapper->OnAvailableStatusChangeFromServer(true, 0);

    wrapper->RemoveAudioLoopbackCallback();
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetAvailableStatus_NoProxy_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    EXPECT_CALL(*mockLoopback_, GetAvailableStatus(_, _)).Times(0);

    AudioLoopbackAvailableStatus status {};
    int32_t result = wrapper->GetAvailableStatus(status);

    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetAvailableStatus_IpcFailed_ReturnsIpcResult_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetAvailableStatus(_, _)).Times(1).WillOnce(Return(ERR_OPERATION_FAILED));

    AudioLoopbackAvailableStatus status {};
    int32_t result = wrapper->GetAvailableStatus(status);

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetAvailableStatus_BusinessFailed_ReturnsResult_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetAvailableStatus(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ERR_INVALID_PARAM), Return(SUCCESS)));

    AudioLoopbackAvailableStatus status {};
    int32_t result = wrapper->GetAvailableStatus(status);

    EXPECT_EQ(result, ERR_INVALID_PARAM);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetAvailableStatus_Success_ForwardsFields_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    AudioLoopbackAvailableStatus expected {};
    expected.available = true;
    expected.unavailableReason = static_cast<AudioLoopbackUnavailableReason>(1);
    EXPECT_CALL(*mockLoopback_, GetAvailableStatus(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(expected), SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    AudioLoopbackAvailableStatus status {};
    int32_t result = wrapper->GetAvailableStatus(status);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(status.available);
    EXPECT_EQ(status.unavailableReason, static_cast<AudioLoopbackUnavailableReason>(1));
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetAvailableStatus_ValueOverload_ForwardsFields_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    AudioLoopbackAvailableStatus expected {};
    expected.available = true;
    expected.unavailableReason = static_cast<AudioLoopbackUnavailableReason>(1);
    EXPECT_CALL(*mockLoopback_, GetAvailableStatus(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(expected), SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    AudioLoopbackAvailableStatus status = wrapper->GetAvailableStatus();

    EXPECT_TRUE(status.available);
    EXPECT_EQ(status.unavailableReason, static_cast<AudioLoopbackUnavailableReason>(1));
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetAvailableStatus_ValueOverload_IpcFailed_ReturnsDefault_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetAvailableStatus(_, _)).Times(1).WillOnce(Return(ERR_OPERATION_FAILED));

    AudioLoopbackAvailableStatus status = wrapper->GetAvailableStatus();

    EXPECT_FALSE(status.available);
    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, RegisterAvailableStatusCallback_NoProxy_EarlyReturn_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(0);

    int32_t result = wrapper->RegisterAvailableStatusCallback();

    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, RegisterAvailableStatusCallback_Success_RegisterOnce_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);
    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, RegisterAvailableStatusCallback_IpcFailed_ReRegisterAllowed_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(2)
        .WillOnce(Return(ERR_OPERATION_FAILED))
        .WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), ERR_OPERATION_FAILED);
    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, RegisterAvailableStatusCallback_BusinessFailed_ReRegisterAllowed_001,
    TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(2)
        .WillOnce(DoAll(SetArgReferee<1>(ERR_INVALID_PARAM), Return(SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), ERR_INVALID_PARAM);
    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, UnregisterAvailableStatusCallback_NotRegistered_EarlyReturn_001,
    TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    EXPECT_CALL(*mockLoopback_, UnregisterAvailableStatusCallback(_)).Times(0);

    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), SUCCESS);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, UnregisterAvailableStatusCallback_Registered_NoProxy_EarlyReturn_001,
    TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);

    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, UnregisterAvailableStatusCallback_IpcFailed_ThenSuccess_001, TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);

    EXPECT_CALL(*mockLoopback_, UnregisterAvailableStatusCallback(_)).Times(2)
        .WillOnce(Return(ERR_OPERATION_FAILED))
        .WillOnce(DoAll(SetArgReferee<0>(SUCCESS), Return(SUCCESS)));

    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), ERR_OPERATION_FAILED);
    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), SUCCESS);
    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, UnregisterAvailableStatusCallback_BusinessFailed_RetryAllowed_001,
    TestSize.Level0)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, RegisterAvailableStatusCallback(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));
    EXPECT_EQ(wrapper->RegisterAvailableStatusCallback(), SUCCESS);

    EXPECT_CALL(*mockLoopback_, UnregisterAvailableStatusCallback(_)).Times(2)
        .WillOnce(DoAll(SetArgReferee<0>(ERR_INVALID_PARAM), Return(SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<0>(SUCCESS), Return(SUCCESS)));

    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), ERR_INVALID_PARAM);
    EXPECT_EQ(wrapper->UnregisterAvailableStatusCallback(), SUCCESS);

    wrapper->SetLoopbackProxy(nullptr);
}
}
}
