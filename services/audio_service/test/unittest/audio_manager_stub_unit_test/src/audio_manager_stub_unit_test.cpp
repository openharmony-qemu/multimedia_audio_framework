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

#include "audio_manager_stub_unit_test.h"

#include "audio_manager_base.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "audio_device_info.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "policy_handler.h"

#include <locale>
#include <codecvt>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
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
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void AudioManagerStubUnitTest::SetUpTestCase(void) {}

void AudioManagerStubUnitTest::TearDownTestCase(void) {}

void AudioManagerStubUnitTest::SetUp(void)
{
    GetPermission();
}

void AudioManagerStubUnitTest::TearDown(void) {}

namespace OHOS {
class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}

    ~MockIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        if (bExchange) {
            std::u16string descriptor = std::u16string();
            return descriptor;
        } else {
            std::u16string descriptor = std::u16string(u"testDescriptor");
            return descriptor;
        }
    }

    static void SetExchange(bool bEx) {
        bExchange = bEx;
    }
private:
    static bool  bExchange;
};
bool MockIRemoteObject::bExchange = true;
}

#ifdef FEATURE_FILE_IO
/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_010
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_ASR_AEC_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_010, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}
#endif

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_011
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to GET_ASR_NOISE_SUPPRESSION_MODE, Set
*             AsrNoiseSuppressionMode value to Outliers(4).
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_011, TestSize.Level1)
{
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_NOISE_SUPPRESSION_MODE);
    MessageParcel data;
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> audioServerN = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    auto ret = audioServerN ->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_012
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to GET_ASR_NOISE_SUPPRESSION_MODE, Set
*             AsrNoiseSuppressionMode value to Outliers(4).
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_012, TestSize.Level1)
{
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_WHISPER_DETECTION_MODE);
    MessageParcel data;
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> audioServerN = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    auto ret = audioServerN ->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_019
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to GET_ASR_NOISE_SUPPRESSION_MODE, Set
*             AsrNoiseSuppressionMode value to Outliers(4).
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_019, TestSize.Level1)
{
    uint32_t format = static_cast<uint32_t>(6000);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> audioServerN = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    auto ret = audioServerN ->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(305, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_001
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to GET_ASR_NOISE_SUPPRESSION_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_001, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_NOISE_SUPPRESSION_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_002
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SET_ASR_WHISPER_DETECTION_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_002, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_WHISPER_DETECTION_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_003
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to GET_ASR_WHISPER_DETECTION_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_003, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_WHISPER_DETECTION_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_004
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SET_ASR_VOICE_CONTROL_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_004, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_CONTROL_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_005
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SET_ASR_VOICE_MUTE_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_005, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_MUTE_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_006
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to IS_WHISPERING
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_006, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::IS_WHISPERING);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_007
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to GET_EFFECT_OFFLOAD_ENABLED
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_007, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_OFFLOAD_ENABLED);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_NE(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_008
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SUSPEND_RENDERSINK
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_008, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SUSPEND_RENDERSINK);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_009
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to RESTORE_RENDERSINK
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_009, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::RESTORE_RENDERSINK);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_010
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to LOAD_HDI_EFFECT_MODEL
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_010, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_HDI_EFFECT_MODEL);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_013
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_013, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_014
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SET_SINK_MUTE_FOR_SWITCH_DEVICE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_014, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_SINK_MUTE_FOR_SWITCH_DEVICE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_015
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SET_ROTATION_TO_EFFECT
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_015, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ROTATION_TO_EFFECT);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_016
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to UPDATE_SESSION_CONNECTION_STATE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_016, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SESSION_CONNECTION_STATE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_017
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to SET_SINGLE_STREAM_MUTE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_017, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_SINGLE_STREAM_MUTE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFourthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFourthPartCode_018
* @tc.desc  : Test HandleFourthPartCode interface. Set code value to RESTORE_SESSION
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFourthPartCode_018, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::RESTORE_SESSION);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleFourthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_001
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to NOTIFY_STREAM_VOLUME_CHANGED
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_001, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_002
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_SPATIALIZATION_SCENE_TYPE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_002, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_003
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_MAX_AMPLITUDE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_003, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_005
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to RESET_ROUTE_FOR_DISCONNECT
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_005, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_006
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_EFFECT_LATENCY
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_006, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_007
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to UPDATE_LATENCY_TIMESTAMP
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_007, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_008
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_ASR_AEC_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_008, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_AEC_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_009
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_ASR_AEC_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_009, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    AsrAecMode asrAecMode = (static_cast<AsrAecMode>(0));
    auto set = audioServer->SetAsrAecMode(asrAecMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_011
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_ASR_NOISE_SUPPRESSION_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_011, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_NOISE_SUPPRESSION_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_012
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_ASR_WHISPER_DETECTION_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_012, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_WHISPER_DETECTION_MODE);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_013
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_AUDIO_EFFECT_PROPERTY
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_013, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY);
    MessageParcel data;
    data.WriteInt32(2);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_014
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_AUDIO_ENHANCE_PROPERTY
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_014, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY);
    MessageParcel data;
    data.WriteInt32(2);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_015
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_AUDIO_EFFECT_PROPERTY
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_015, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY);
    MessageParcel data;
    data.WriteInt32(2);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
    format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY);
    ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_016
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_AUDIO_ENHANCE_PROPERTY
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_016, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY);
    MessageParcel data;
    data.WriteInt32(2);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
    format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY);
    ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_017
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to UNSET_OFFLOAD_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_017, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UNSET_OFFLOAD_MODE);
    MessageParcel data;
    data.WriteUint32(2);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_018
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_OFFLOAD_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_018, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_OFFLOAD_MODE);
    MessageParcel data;
    data.WriteUint32(2);
    data.WriteInt32(2);
    data.WriteBool(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_001
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to CHECK_REMOTE_DEVICE_STATE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_001, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_002
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to SET_VOICE_VOLUME
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_002, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_003
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to SET_AUDIO_MONO_STATE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_003, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_004
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to SET_AUDIO_BALANCE_VALUE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_004, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_006
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to LOAD_AUDIO_EFFECT_LIBRARIES
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_006, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_AUDIO_EFFECT_LIBRARIES);
    MessageParcel data;
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteString("1");
    data.WriteString("2");
    data.WriteString("3");
    data.WriteString("4");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_008
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to CREATE_AUDIO_EFFECT_CHAIN_MANAGER
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_008, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIO_EFFECT_CHAIN_MANAGER);
    MessageParcel data;
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteString("SCENE_MUSIC");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_NE(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_009
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to SET_OUTPUT_DEVICE_SINK
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_009, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK);
    MessageParcel data;
    data.WriteInt32(24);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_010
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to CREATE_PLAYBACK_CAPTURER_MANAGER
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_010, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_015
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to UPDATE_SPATIALIZATION_STATE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_015, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIALIZATION_STATE);
    MessageParcel data;
    data.WriteBool(0);
    data.WriteBool(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_016
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to UPDATE_SPATIAL_DEVICE_TYPE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_016, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIAL_DEVICE_TYPE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_017
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to OFFLOAD_SET_VOLUME
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_017, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_018
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to NOTIFY_STREAM_VOLUME_CHANGED
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_018, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_001
* @tc.desc  : Test OnRemoteRequest interface. Set code value to AUDIO_SERVER_CODE_MAX
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_001, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_002
* @tc.desc  : Test OnRemoteRequest interface. Set code value to GET_ASR_NOISE_SUPPRESSION_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_002, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_NOISE_SUPPRESSION_MODE);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(0));
    auto set = audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(AUDIO_OK, set);
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_003
* @tc.desc  : Test OnRemoteRequest interface. Set code value to GET_AUDIO_PARAMETER
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_003, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_004
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_AUDIO_PARAMETER
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_004, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_005
* @tc.desc  : Test OnRemoteRequest interface. Set code value to GET_EXTRA_AUDIO_PARAMETERS
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_005, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_EXTRA_AUDIO_PARAMETERS);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(4);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_006
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_EXTRA_AUDIO_PARAMETERS
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_006, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_EXTRA_AUDIO_PARAMETERS);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(0);
    data.WriteString("123");
    data.WriteString("321");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_007
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_MICROPHONE_MUTE
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_007, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_008
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_AUDIO_SCENE
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_008, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}
/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_009
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_AUDIO_SCENE(ERR Int)
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_009, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(1);
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_010
* @tc.desc  : Test OnRemoteRequest interface. Set code value to UPDATE_ROUTE_REQ
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_010, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTE_REQ);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_011
* @tc.desc  : Test OnRemoteRequest interface. Set code value to UPDATE_ROUTES_REQ
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_011, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTES_REQ);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_012
* @tc.desc  : Test OnRemoteRequest interface. Set code value to UPDATE_ROUTES_REQ(ERR Int)
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_012, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTES_REQ);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_013
* @tc.desc  : Test OnRemoteRequest interface. Set code value to UPDATE_DUAL_TONE_REQ
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_013, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_DUAL_TONE_REQ);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteBool(true);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_014
* @tc.desc  : Test OnRemoteRequest interface. Set code value to GET_TRANSACTION_ID
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_014, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_TRANSACTION_ID);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteInt32(1);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_015
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_PARAMETER_CALLBACK
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_015, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    sptr<IRemoteObject> object = new OHOS::MockIRemoteObject();
    OHOS::MockIRemoteObject::SetExchange(true);
    data.WriteRemoteObject(object);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_016
* @tc.desc  : Test OnRemoteRequest interface. Set code value to GET_REMOTE_AUDIO_PARAMETER
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_016, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_REMOTE_AUDIO_PARAMETER);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_017
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_REMOTE_AUDIO_PARAMETER
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_017, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_REMOTE_AUDIO_PARAMETER);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteString("SCENE_MUSIC");
    data.WriteInt32(1);
    data.WriteString("SCENE_MUSIC");
    data.WriteString("SCENE_MUSIC");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_018
* @tc.desc  : Test OnRemoteRequest interface. Set code value to NOTIFY_DEVICE_INFO
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_018, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteString("SCENE_MUSIC");
    data.WriteBool(true);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_019
* @tc.desc  : Test OnRemoteRequest interface. Set code value to UPDATE_SPATIALIZATION_STATE
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_019, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIALIZATION_STATE);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    data.WriteBool(true);
    data.WriteBool(true);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_020
* @tc.desc  : Test OnRemoteRequest interface. Set code value to SET_ASR_VOICE_CONTROL_MODE
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_020, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_CONTROL_MODE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_021
* @tc.desc  : Test OnRemoteRequest interface. Set code value to ERR_TYPE
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_021, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(700);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(305, ret);
}

/**
* @tc.name  : Test OnRemoteRequest API
* @tc.type  : FUNC
* @tc.number: OnRemoteRequest_022
* @tc.desc  : Test OnRemoteRequest interface. Set code value to GET_ALL_SINK_INPUTS
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_022, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_ALL_SINK_INPUTS);
    MessageParcel data;
    data.WriteInterfaceToken(u"IStandardAudioService");
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->OnRemoteRequest(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFifthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFifthPartCode_001
* @tc.desc  : Test HandleFifthPartCode interface. Set code value to CREATE_IPC_OFFLINE_STREAM
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFifthPartCode_001, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_IPC_OFFLINE_STREAM);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleFifthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFifthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFifthPartCode_002
* @tc.desc  : Test HandleFifthPartCode interface. interface. Set code value to GET_OFFLINE_AUDIO_EFFECT_CHAINS
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFifthPartCode_002, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_OFFLINE_AUDIO_EFFECT_CHAINS);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleFifthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFifthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFifthPartCode_003
* @tc.desc  : Test HandleFifthPartCode interface. Set code value to GET_STANDBY_STATUS
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFifthPartCode_003, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_STANDBY_STATUS);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleFifthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFifthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFifthPartCode_004
* @tc.desc  : Test HandleFifthPartCode interface. Set code value to GENERATE_SESSION_ID
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFifthPartCode_004, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GENERATE_SESSION_ID);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleFifthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test HandleFifthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFifthPartCode_005
* @tc.desc  : Test HandleFifthPartCode interface. Set code value to LOAD_HDI_ADAPTER
*/
HWTEST_F(AudioManagerStubUnitTest, HandleFifthPartCode_005, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_HDI_ADAPTER);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleFifthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleFifthPartCode API
* @tc.type  : FUNC
* @tc.number: HandleFifthPartCode_006
* @tc.desc  : Test HandleFifthPartCode interface. Set code value to UNLOAD_HDI_ADAPTER
*/
HWTEST_F(AudioManagerStubUnitTest, OnRemoteRequest_027, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::UNLOAD_HDI_ADAPTER);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleFifthPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_019
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to GET_AUDIO_EFFECT_PROPERTY_V3
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_019, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleThirdPartCode API
* @tc.type  : FUNC
* @tc.number: HandleThirdPartCode_020
* @tc.desc  : Test HandleThirdPartCode interface. Set code value to SET_AUDIO_EFFECT_PROPERTY_V3
*/
HWTEST_F(AudioManagerStubUnitTest, HandleThirdPartCode_020, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3);
    MessageParcel data;
    data.WriteInt32(0);
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleThirdPartCode(format, data, reply, option);
    EXPECT_EQ(ERROR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_019
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to CREATE_AUDIOPROCESS
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_019, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOPROCESS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_020
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to REGISET_POLICY_PROVIDER
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_020, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::REGISET_POLICY_PROVIDER);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_021
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to SET_WAKEUP_CLOSE_CALLBACK
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_021, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::SET_WAKEUP_CLOSE_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name  : Test HandleSecondPartCode API
* @tc.type  : FUNC
* @tc.number: HandleSecondPartCode_022
* @tc.desc  : Test HandleSecondPartCode interface. Set code value to CHECK_HIBERNATE_STATE
*/
HWTEST_F(AudioManagerStubUnitTest, HandleSecondPartCode_022, TestSize.Level1)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t format = static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_HIBERNATE_STATE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = audioServer->HandleSecondPartCode(format, data, reply, option);
    EXPECT_EQ(AUDIO_OK, ret);
}
} // namespace AudioStandard
} // namespace OHOS