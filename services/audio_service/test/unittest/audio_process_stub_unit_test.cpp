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

#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_process_stub.h"
#include "audio_process_in_server.h"
#include "audio_service.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static int32_t DEFAULT_STREAM_ID = 10;

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

class AudioProcessStubUnitTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioProcessStubUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioProcessStubUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_001
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_RESOLVE_BUFFER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_002
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::OH_GET_SESSIONID;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_003
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_START;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_004
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_PAUSE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_005
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_005, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_RESUME;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_006
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_006, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_STOP;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_007
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_007, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_REQUEST_HANDLE_INFO;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_008
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_008, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_RELEASE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_009
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_009, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_REGISTER_PROCESS_CB;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_010
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_010, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_REGISTER_THREAD_PRIORITY;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_011
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_011, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_SET_DEFAULT_OUTPUT_DEVICE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_012
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_012, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::ON_SET_SLITNT_MODE_AND_MIX_WITH_OTHERS;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_013
 * @tc.desc  : Test AudioProcessStub::OnRemoteRequest().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_013, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    uint32_t code = IAudioProcess::IAudioProcessMsg::PROCESS_MAX_MSG;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessStub.
 * @tc.type  : FUNC
 * @tc.number: AudioProcessStub_014
 * @tc.desc  : Test AudioProcessStub::HandleResolveBuffer().
 */
HWTEST_F(AudioProcessStubUnitTest, AudioProcessStub_014, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    sptr<AudioProcessInServer> audioProcessStubPtr = AudioProcessInServer::Create(config, AudioService::GetInstance());

    EXPECT_NE(audioProcessStubPtr, nullptr);

    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(IAudioProcess::GetDescriptor());

    bool ret = audioProcessStubPtr->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = audioProcessStubPtr->HandleResolveBuffer(data, reply);
    EXPECT_EQ(ret, true);
}
}
}