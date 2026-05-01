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
}

HWTEST_F(AudioLoopbackWrapperUnitTest, Enable_Control_Success_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, Enable(true, _)).WillOnce(DoAll(SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

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

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_Control_Success_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetStatus(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(LOOPBACK_AVAILABLE_RUNNING), SetArgReferee<1>(SUCCESS), Return(SUCCESS)));

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, LOOPBACK_AVAILABLE_RUNNING);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_Control_NoProxy_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, LOOPBACK_UNAVAILABLE_DEVICE);
}

HWTEST_F(AudioLoopbackWrapperUnitTest, GetStatus_Control_IpcFailed_001, TestSize.Level1)
{
    auto wrapper = std::make_shared<TestAudioLoopbackWrapper>(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        appInfo_);
    wrapper->SetLoopbackProxy(mockLoopback_);

    EXPECT_CALL(*mockLoopback_, GetStatus(_, _)).WillOnce(DoAll(SetArgReferee<0>(LOOPBACK_AVAILABLE_IDLE),
        SetArgReferee<1>(ERR_OPERATION_FAILED), Return(SUCCESS)));

    AudioLoopbackStatus status = wrapper->GetStatus();
    EXPECT_EQ(status, LOOPBACK_UNAVAILABLE_DEVICE);
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
}
}