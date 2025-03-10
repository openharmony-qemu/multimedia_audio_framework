/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "ipc_stream_in_server.h"
#include <memory>
#include "ipc_stream_stub.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "renderer_in_server.h"
#include "capturer_in_server.h"
#include <cinttypes>
#include "audio_service_log.h"
#include "audio_errors.h"
#include "ipc_stream.h"
#include "message_parcel.h"
#include "parcel.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class IpcStreamStubUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

void IpcStreamStubUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void IpcStreamStubUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void IpcStreamStubUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void IpcStreamStubUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

constexpr int32_t DEFAULT_STREAM_ID = 10;

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_001
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_RATE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_002
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_LOWPOWER_VOLUME,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_003
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_LOWPOWER_VOLUME,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_004
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_EFFECT_MODE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_005
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_005, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_EFFECT_MODE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_006
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_006, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_PRIVACY_TYPE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_007
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_007, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_PRIVACY_TYPE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_008
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_008, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_OFFLOAD_MODE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_009
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_009, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_UNSET_OFFLOAD_MODE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_010
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_010, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_OFFLOAD_APPROXIMATELY_CACHE_TIME,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_012
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_012, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_UPDATE_SPATIALIZATION_STATE,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_013
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_013, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_STREAM_MANAGER_TYPE,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_014
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_014, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_SILENT_MODE_AND_MIX_WITH_OTHERS,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_015
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_015, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_CLIENT_VOLUME,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_016
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_016, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_MUTE,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_017
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_017, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_REGISTER_THREAD_PRIORITY,
            data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_018
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_018, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::IPC_STREAM_MAX_MSG,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_019
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_019, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::IPC_STREAM_MAX_MSG,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_020
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_020, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::ON_UPDATE_POSITION,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_021
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_021, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::OH_GET_AUDIO_TIME,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_022
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_022, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_LATENCY,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_023
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_023, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_RATE,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_024
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_024, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_DUCK_FACTOR,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_025
 * @tc.desc  : Test IpcStreamStub::OnMiddleCodeRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_025, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->OnMiddleCodeRemoteRequest(IpcStream::IpcStreamMsg::ON_SET_DEFAULT_OUTPUT_DEVICE,
        data, reply, option);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_026
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_026, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::ON_REGISTER_STREAM_LISTENER,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_RESOLVE_BUFFER,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_GET_AUDIO_SESSIONID,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_START,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_STOP,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_RELEASE,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_FLUSH,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_027
 * @tc.desc  : Test IpcStreamStub::OnRemoteRequest.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_027, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);

    EXPECT_NE(ptrIpcStreamStub, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto result = ptrIpcStreamStub->
        OnRemoteRequest(IpcStream::IpcStreamMsg::ON_DRAIN,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::ON_UPDATA_PLAYBACK_CAPTURER_CONFIG,
            data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::OH_GET_AUDIO_POSITION,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);

    result = ptrIpcStreamStub->OnRemoteRequest(IpcStream::IpcStreamMsg::OH_GET_AUDIO_POSITION,
        data, reply, option);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_028
 * @tc.desc  : Test IpcStreamStub::HandleRegisterStreamListener.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_028, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);
    ASSERT_TRUE(ptrIpcStreamStub != nullptr);

    MessageParcel data;
    MessageParcel reply;

    auto result = ptrIpcStreamStub->HandleRegisterStreamListener(data, reply);
    EXPECT_EQ(result, AUDIO_INVALID_PARAM);

    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    data.WriteRemoteObject(impl);
    result = ptrIpcStreamStub->HandleRegisterStreamListener(data, reply);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test IpcStreamStub API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamStub_029
 * @tc.desc  : Test IpcStreamStub::HandleResolveBuffer.
 */
HWTEST(IpcStreamStubUnitTest, IpcStreamStub_029, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, ret);
    ASSERT_TRUE(ptrIpcStreamStub != nullptr);

    MessageParcel data;
    MessageParcel reply;

    auto result = ptrIpcStreamStub->HandleResolveBuffer(data, reply);
    EXPECT_EQ(result, AUDIO_OK);
}
} // namespace AudioStandard
} // namespace OHOS