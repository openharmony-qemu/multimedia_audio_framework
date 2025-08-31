/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "get_server_util.h"
#include "audio_policy_service_thirdext_unit_test.h"
#include "audio_policy_config_manager.h"
#include "audio_server_proxy.h"
#include "nativetoken_kit.h"
#include "dfx_msg_manager.h"
#include "audio_errors.h"
#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermissioned = false;

const int32_t TEST_SESSIONID = MIN_STREAMID + 1010;
const int32_t A2DP_STOPPED = 1;
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_INVALID = 3;
const uint32_t ROTATE = 1;
const int32_t SESSION_ID = 1000001;
const int32_t STATE = 1;
const uint32_t TEST_APP_UID = 1;

const uint32_t MIN_VOLUME_LEVEL = 0;
const uint32_t MAX_VOLUME_LEVEL = 15;

void AudioPolicyServiceFourthUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceFourthUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceFourthUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUp start-end");
}
void AudioPolicyServiceFourthUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDown start-end");
}

static void GetPermission()
{
    if (!g_hasPermissioned) {
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
            .processName = "audio_policy_service_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermissioned = true;
    }
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyServiceTest_Prepare001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest AudioPolicyServiceTest_Prepare001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    // Get permission for test
    GetPermission();

    // Call OnServiceConnected for HDI_SERVICE_INDEX
    GetServerUtil::GetServerPtr()->audioPolicyService_.OnServiceConnected(HDI_SERVICE_INDEX);
}

/**
* @tc.name  : Test GetSupportedAudioEffectProperty.
* @tc.number: GetSupportedAudioEffectProperty_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetSupportedAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetSupportedAudioEffectProperty_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioEffectPropertyArray propertyArray;
    GetServerUtil::GetServerPtr()->audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
}

/**
* @tc.name  : Test LoadHdiEffectModel.
* @tc.number: LoadHdiEffectModel_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, LoadHdiEffectModel_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest LoadHdiEffectModel_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    GetServerUtil::GetServerPtr()->audioPolicyService_.LoadHdiEffectModel();
}

/**
* @tc.name  : Test OnReceiveBluetoothEvent.
* @tc.number: OnReceiveBluetoothEvent_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnReceiveBluetoothEvent_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnReceiveBluetoothEvent_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    const std::string macAddress = "11-11-11-11-11-11";
    const std::string deviceName = "deviceName";
}

/**
* @tc.name  : Test WaitForConnectionCompleted.
* @tc.number: WaitForConnectionCompleted_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, WaitForConnectionCompleted_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest WaitForConnectionCompleted_001 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.WaitForConnectionCompleted();
    EXPECT_FALSE(!(audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ =
        CONNECTION_STATUS_CONNECTED));
}
#ifdef AUDIO_POLICY_SERVICE_UNIT_TEST_DIFF
/**
* @tc.name  : Test WaitForConnectionCompleted.
* @tc.number: WaitForConnectionCompleted_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, WaitForConnectionCompleted_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest WaitForConnectionCompleted_001 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.WaitForConnectionCompleted();
    EXPECT_FALSE(!(audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ =
        CONNECTION_STATUS_CONNECTED));
}
#endif
/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_001 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(0);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_001 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(1);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_003
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_001 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(0);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test CheckSpatializationAndEffectState.
* @tc.number: CheckSpatializationAndEffectState_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, CheckSpatializationAndEffectState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest CheckSpatializationAndEffectState_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.audioOffloadStream_.CheckSpatializationAndEffectState();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test UpdateRoute.
* @tc.number: UpdateRoute_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateRoute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateRoute_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    outputDevices.push_back(std::move(audioDeviceDescriptor));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor1->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    outputDevices.push_back(std::move(audioDeviceDescriptor1));
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    outputDevices.push_back(std::move(audioDeviceDescriptor2));
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);
    audioDeviceDescriptor.reset();
    audioDeviceDescriptor1.reset();
    audioDeviceDescriptor2.reset();
    rendererChangeInfo.reset();
}

/**
* @tc.name  : Test UpdateRoute.
* @tc.number: UpdateRoute_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateRoute_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateRoute_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_DP;
    outputDevices.push_back(std::move(audioDeviceDescriptor));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);

    server->audioPolicyService_.audioDeviceCommon_.enableDualHalToneState_ = true;
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);
    audioDeviceDescriptor.reset();
    rendererChangeInfo.reset();
}
#ifdef AUDIO_POLICY_SERVICE_UNIT_TEST_DIFF
/**
* @tc.name  : Test UpdateDefaultOutputDeviceWhenStopping.
* @tc.number: UpdateDefaultOutputDeviceWhenStopping_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateDefaultOutputDeviceWhenStopping_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateDefaultOutputDeviceWhenStopping_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    int32_t uid = getuid();
    GetServerUtil::GetServerPtr()->
        audioPolicyService_.audioDeviceLock_.UpdateDefaultOutputDeviceWhenStopping(uid);
    EXPECT_EQ(SUCCESS, uid);
}
#endif
/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_STOPPED;
    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_002 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_STOPPED;
    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_003 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_004
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_004 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_005
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_005 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_CONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_006
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_006, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_006 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_007
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_007 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_STOPPED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_008
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_008, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_008 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_INVALID;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(A2DP_STOPPED, playingState);
    EXPECT_NE(A2DP_PLAYING, playingState);
}
#ifdef AUDIO_POLICY_SERVICE_UNIT_TEST_DIFF
/**
* @tc.name  : Test GetSupportedAudioEnhanceProperty.
* @tc.number: GetSupportedAudioEnhanceProperty_001
* @tc.desc  : Test GetSupportedAudioEnhanceProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetSupportedAudioEnhanceProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetSupportedAudioEnhanceProperty_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioEnhancePropertyArray propertyArrayTest;
    GetServerUtil::GetServerPtr()->GetSupportedAudioEnhanceProperty(propertyArrayTest);
}
#endif
/**
* @tc.name  : Test SetAudioEffectProperty.
* @tc.number: SetAudioEffectProperty_001
* @tc.desc  : Test SetAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetAudioEffectProperty_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioEffectPropertyArray propertyArrayTest;
    GetServerUtil::GetServerPtr()->SetAudioEffectProperty(propertyArrayTest);
}

/**
* @tc.name  : Test SetAudioEnhanceProperty.
* @tc.number: SetAudioEnhanceProperty_001
* @tc.desc  : Test SetAudioEnhanceProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetAudioEnhanceProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetAudioEnhanceProperty_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioEnhancePropertyArray propertyArrayTest;
    GetServerUtil::GetServerPtr()->SetAudioEnhanceProperty(propertyArrayTest);
}

/**
* @tc.name  : Test ErasePreferredDeviceByType.
* @tc.number: ErasePreferredDeviceByType_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ErasePreferredDeviceByType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ErasePreferredDeviceByType_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioPolicyUtils::GetInstance().isBTReconnecting_ = true;
    const PreferredType preferredType = AUDIO_MEDIA_RENDER;
    auto ret = AudioPolicyUtils::GetInstance().ErasePreferredDeviceByType(preferredType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test ErasePreferredDeviceByType.
* @tc.number: ErasePreferredDeviceByType_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ErasePreferredDeviceByType_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ErasePreferredDeviceByType_002 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    const PreferredType preferredType = AUDIO_MEDIA_RENDER;
    auto ret = AudioPolicyUtils::GetInstance().ErasePreferredDeviceByType(preferredType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test UpdateSessionConnectionState.
* @tc.number: UpdateSessionConnectionState_001
* @tc.desc  : Test UpdateSessionConnectionState interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateSessionConnectionState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateSessionConnectionState_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    int32_t sessionID = SESSION_ID;
    int32_t state = STATE;
    EXPECT_NE(nullptr, AudioServerProxy::GetInstance().GetAudioServerProxy());
}

/**
* @tc.name  : Test IsRingerOrAlarmerDualDevicesRange.
* @tc.number: IsRingerOrAlarmerDualDevicesRange_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsRingerOrAlarmerDualDevicesRange_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsRingerOrAlarmerDualDevicesRange_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    InternalDeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsRingerOrAlarmerDualDevicesRange.
* @tc.number: IsRingerOrAlarmerDualDevicesRange_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsRingerOrAlarmerDualDevicesRange_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsRingerOrAlarmerDualDevicesRange_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    InternalDeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_DP;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
 * @tc.number: ScoInputDeviceFetchedForRecongnition_001
 * @tc.desc  : Test ScoInputDeviceFetchedForRecongnition interfaces.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, ScoInputDeviceFetchedForRecongnition_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ScoInputDeviceFetchedForRecongnition_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    std::string address = "";

    bool handleFlag = false;
    ConnectState connectState = DEACTIVE_CONNECTED;
    int32_t result = server->audioPolicyService_.audioDeviceCommon_.ScoInputDeviceFetchedForRecongnition(handleFlag,
        address, connectState);
    EXPECT_EQ(SUCCESS, result);

    handleFlag = true;
    connectState = VIRTUAL_CONNECTED;
    result = server->audioPolicyService_.audioDeviceCommon_.ScoInputDeviceFetchedForRecongnition(handleFlag,
        address, connectState);
    EXPECT_EQ(SUCCESS, result);
}
#ifdef AUDIO_POLICY_SERVICE_UNIT_TEST_DIFF
/**
* @tc.name  : Test SetRotationToEffect.
* @tc.number: SetRotationToEffect_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetRotationToEffect_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetRotationToEffect_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    uint32_t rotate = ROTATE;
    server->audioPolicyService_.SetRotationToEffect(rotate);
    EXPECT_NE(nullptr, AudioServerProxy::GetInstance().GetAudioServerProxy());
}
#endif

/**
* @tc.name  : Test SelectRingerOrAlarmDevices.
* @tc.number: SelectRingerOrAlarmDevices_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SelectRingerOrAlarmDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SelectRingerOrAlarmDevices_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs1;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    bool result = GetServerUtil::GetServerPtr()
        ->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(descs1, rendererChangeInfo1);
    EXPECT_EQ(false, result);

    vector<std::shared_ptr<AudioDeviceDescriptor>> descs2;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_SPEAKER;
    descs2.push_back(std::move(audioDeviceDescriptor2));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    rendererChangeInfo2->sessionId = TEST_SESSIONID;
    result = GetServerUtil::GetServerPtr()
        ->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(descs2, rendererChangeInfo2);
    EXPECT_EQ(true, result);

    vector<std::shared_ptr<AudioDeviceDescriptor>> descs3;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor3 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descs3.push_back(std::move(audioDeviceDescriptor3));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    rendererChangeInfo3->sessionId = TEST_SESSIONID;
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.enableDualHalToneState_ = true;
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioPolicyManager_.SetRingerMode(
        RINGER_MODE_VIBRATE);
    result = GetServerUtil::GetServerPtr()
        ->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(descs3, rendererChangeInfo3);
    EXPECT_EQ(true, result);

    vector<std::shared_ptr<AudioDeviceDescriptor>> descs4;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor4 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor4->deviceType_ = DEVICE_TYPE_EXTERN_CABLE;
    descs3.push_back(std::move(audioDeviceDescriptor4));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    rendererChangeInfo4->sessionId = TEST_SESSIONID;
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.enableDualHalToneState_ = true;
    result = GetServerUtil::GetServerPtr()
        ->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(descs4, rendererChangeInfo4);
    EXPECT_EQ(false, result);

    rendererChangeInfo1.reset();
    rendererChangeInfo2.reset();
    rendererChangeInfo3.reset();
    rendererChangeInfo4.reset();
    audioDeviceDescriptor2.reset();
    audioDeviceDescriptor3.reset();
    audioDeviceDescriptor4.reset();
}

/**
* @tc.name  : Test SetPreferredDevice.
* @tc.number: SetPreferredDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetPreferredDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetPreferredDevice_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr1 = nullptr;
    int32_t result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_MEDIA_RENDER, audioDeviceDescriptorSptr1);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr2 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr2->deviceType_ = DEVICE_TYPE_NONE;
    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_CALL_RENDER, audioDeviceDescriptorSptr2, -1);
    EXPECT_EQ(SUCCESS, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_CALL_CAPTURE, audioDeviceDescriptorSptr2);
    EXPECT_EQ(SUCCESS, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_RECORD_CAPTURE, audioDeviceDescriptorSptr2);
    EXPECT_EQ(SUCCESS, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_RING_RENDER, audioDeviceDescriptorSptr2);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_TONE_RENDER, audioDeviceDescriptorSptr2);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    uint32_t preferredType = 6;
    PreferredType ERR_PFTYPE = static_cast<PreferredType>(preferredType);
    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        ERR_PFTYPE, audioDeviceDescriptorSptr2);
    EXPECT_EQ(ERR_INVALID_PARAM, result);
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_004
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_004 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(0);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_005
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_005 start");
    EXPECT_NE(nullptr, GetServerUtil::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_;

    audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_ = CONNECTION_STATUS_TIMEOUT;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(0);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test GetA2dpOffloadFlag.
* @tc.number: GetA2dpOffloadFlag_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetA2dpOffloadFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetA2dpOffloadFlag_001 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    EXPECT_EQ(NO_A2DP_DEVICE, server->audioPolicyService_.GetA2dpOffloadFlag());
}

/**
* @tc.name  : Test GetA2dpOffloadFlag.
* @tc.number: GetA2dpOffloadFlag_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetA2dpOffloadFlag_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetA2dpOffloadFlag_002 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    server->audioPolicyService_.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    ASSERT_NE(nullptr, server->audioPolicyService_.audioA2dpOffloadManager_);
    BluetoothOffloadState flag = server->audioPolicyService_.GetA2dpOffloadFlag();
    EXPECT_EQ(NO_A2DP_DEVICE, server->audioPolicyService_.GetA2dpOffloadFlag());
}

/**
* @tc.name  : Test NotifyCapturerRemoved.
* @tc.number: NotifyCapturerRemoved_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, NotifyCapturerRemoved_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest NotifyCapturerRemoved_001 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    uint64_t sessionId = 0;
    EXPECT_EQ(SUCCESS, server->audioPolicyService_.NotifyCapturerRemoved(sessionId));
}

/**
* @tc.name  : Test NotifyCapturerRemoved.
* @tc.number: NotifyCapturerRemoved_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, NotifyCapturerRemoved_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest NotifyCapturerRemoved_002 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    uint64_t sessionId = 0;
    server->audioPolicyService_.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_EQ(SUCCESS, server->audioPolicyService_.NotifyCapturerRemoved(sessionId));
}

#ifdef HAS_FEATURE_INNERCAPTURER
/**
* @tc.name  : Test LoadModernInnerCapSink.
* @tc.number: LoadModernInnerCapSink_001
* @tc.desc  : Test LoadModernInnerCapSink interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, LoadModernInnerCapSink_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    int32_t ret = server->audioPolicyService_.LoadModernInnerCapSink(1);
    EXPECT_EQ(ret, SUCCESS);
}


/**
* @tc.name  : Test UnloadModernInnerCapSink.
* @tc.number: UnloadModernInnerCapSink_001
* @tc.desc  : Test UnloadModernInnerCapSink interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UnloadModernInnerCapSink_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    int32_t ret = server->audioPolicyService_.UnloadModernInnerCapSink(1);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManager_001
* @tc.desc  : Test AudioDeviceManager interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioDeviceManager_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest AudioDeviceManager_001 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    server->audioDeviceManager_.FindConnectedDeviceById(1);
}

/**
 * @tc.name   : Test UnexcludeOutputDevices API
 * @tc.number : UnexcludeOutputDevicesTest_001
 * @tc.desc   : Test UnexcludeOutputDevices interface.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, UnexcludeOutputDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UnexcludeOutputDevicesTest_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);

    int32_t result = server->audioDeviceLock_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UnexcludeOutputDevicesTest_001() result:%{public}d", result);

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test UnexcludeOutputDevices API
 * @tc.number : UnexcludeOutputDevicesTest_002
 * @tc.desc   : Test UnexcludeOutputDevices interface.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, UnexcludeOutputDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UnexcludeOutputDevicesTest_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);

    int32_t result = server->audioDeviceLock_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UnexcludeOutputDevicesTest_002() result:%{public}d", result);

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test GetExcludedDevices API
 * @tc.number : GetExcludedDevicesTest_001
 * @tc.desc   : Test GetExcludedDevices interface.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetExcludedDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetExcludedDevicesTest_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        server->audioDeviceLock_.GetExcludedDevices(audioDevUsage);
    EXPECT_EQ(audioDeviceDescriptors.size(), 0);
}

/**
 * @tc.name   : Test GetExcludedDevices API
 * @tc.number : GetExcludedDevicesTest_002
 * @tc.desc   : Test GetExcludedDevices interface.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetExcludedDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetExcludedDevicesTest_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        server->audioDeviceLock_.GetExcludedDevices(audioDevUsage);
    EXPECT_EQ(audioDeviceDescriptors.size(), 0);
}

/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerPrcess_001
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerPrcess_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();
    const int DFX_MSG_ACTION_LOOP_TIMES = 100;
    std::list<RenderDfxInfo> renderInfo{};
    std::list<InterruptDfxInfo> interruptInfo{};
    for (size_t i = 0; i < DFX_MSG_ACTION_LOOP_TIMES; i++) {
        renderInfo.push_back({});
        interruptInfo.push_back({});
    }

    const int DFX_MSG_LOOP_TIMES = 20;
    DfxMessage renderMsg = {.appUid = TEST_APP_UID, .renderInfo = renderInfo};
    DfxMessage interruptMsg = {.appUid = TEST_APP_UID, .interruptInfo = interruptInfo};
    for (size_t i = 0; i < DFX_MSG_LOOP_TIMES; i++) {
        manager.Process(renderMsg);
        manager.Process(interruptMsg);
    }

    EXPECT_EQ(1, manager.reportQueue_.size());
    bool checkFlag = true;
    for (auto &item : manager.reportQueue_) {
        if (item.second.renderInfo.size() != DFX_MSG_ACTION_LOOP_TIMES ||
            item.second.interruptInfo.size() != DFX_MSG_ACTION_LOOP_TIMES) {
            checkFlag = false;
            break;
        }
    }
    EXPECT_TRUE(checkFlag);
    manager.reportQueue_.clear();
}

/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerPrcess_002
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerPrcess_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();
    const int DFX_MSG_ACTION_LOOP_TIMES = 100;
    const int DFX_MSG_ACTION_LOOP_TIMES_1 = 50;
    const int DFX_MSG_ACTION_LOOP_TIMES_2 = 60;
    std::list<RenderDfxInfo> renderInfo{};
    for (size_t i = 1; i <= DFX_MSG_ACTION_LOOP_TIMES_1; i++) {
        DfxStatAction rendererAction{};
        rendererAction.firstByte = i;
        renderInfo.push_back({.rendererAction = rendererAction});
    }

    std::list<RenderDfxInfo> renderInfo2{};
    for (size_t i = 1; i <= DFX_MSG_ACTION_LOOP_TIMES_2; i++) {
        DfxStatAction rendererAction{};
        rendererAction.firstByte = DFX_MSG_ACTION_LOOP_TIMES_1 + i;
        renderInfo2.push_back({.rendererAction = rendererAction});
    }

    std::list<InterruptDfxInfo> interruptInfo{};
    for (size_t i = 0; i < DFX_MSG_ACTION_LOOP_TIMES; i++) {
        interruptInfo.push_back({});
    }

    const int DFX_MSG_LOOP_TIMES = 20;
    for (size_t i = 0; i < DFX_MSG_LOOP_TIMES; i++) {
        DfxMessage renderMsg = {.appUid = TEST_APP_UID, .renderInfo = renderInfo};
        DfxMessage renderMsg2 = {.appUid = TEST_APP_UID, .renderInfo = renderInfo2};
        DfxMessage interruptMsg = {.appUid = TEST_APP_UID, .interruptInfo = interruptInfo};
        manager.Process(renderMsg);
        manager.Process(renderMsg2);
        manager.Process(interruptMsg);
    }

    EXPECT_EQ(1, manager.reportQueue_.size());
    auto upper = manager.reportQueue_.upper_bound(TEST_APP_UID);
    auto lastIt = --upper;
    auto checkValue = static_cast<int>(lastIt->second.renderInfo.back().rendererAction.firstByte);

    EXPECT_EQ(DFX_MSG_ACTION_LOOP_TIMES, checkValue);
    manager.reportQueue_.clear();
}

/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerPrcess_003
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerPrcess_003, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();
    const int DFX_MSG_ACTION_LOOP_TIMES_1 = 50;
    const int DFX_MSG_ACTION_LOOP_TIMES_2 = 60;
    std::list<RenderDfxInfo> renderInfo{};
    for (size_t i = 1; i <= DFX_MSG_ACTION_LOOP_TIMES_1; i++) {
        DfxStatAction rendererAction{};
        rendererAction.firstByte = i;
        renderInfo.push_back({.rendererAction = rendererAction});
    }

    std::list<RenderDfxInfo> renderInfo2{};
    for (size_t i = 1; i <= DFX_MSG_ACTION_LOOP_TIMES_2; i++) {
        DfxStatAction rendererAction{};
        rendererAction.firstByte = DFX_MSG_ACTION_LOOP_TIMES_1 + i;
        renderInfo2.push_back({.rendererAction = rendererAction});
    }

    DfxMessage renderMsg = {.appUid = TEST_APP_UID, .renderInfo = renderInfo};
    manager.Process(renderMsg);
    DfxMessage renderMsg2 = {.appUid = TEST_APP_UID, .renderInfo = renderInfo2};
    manager.Process(renderMsg2);

    int loopTimes = 0;
    int checkValue1 = 0;
    int checkValue2 = 0;
    const int DFX_MSG_MAX_ACTION_SIZE = 100;
    for (auto &item : manager.reportQueue_) {
        loopTimes++;
        if (loopTimes == 1) {
            checkValue1 = item.second.renderInfo.size();
        } else if (loopTimes == 2) {
            checkValue2 = item.second.renderInfo.size();
        }
    }

    EXPECT_EQ(DFX_MSG_MAX_ACTION_SIZE, checkValue1);
    EXPECT_EQ(0, checkValue2);
    manager.reportQueue_.clear();
}


/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerPrcess_004
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerPrcess_004, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();
    const int DFX_MSG_ACTION_LOOP_TIMES = 100;
    std::list<RenderDfxInfo> renderInfo{};
    std::list<InterruptDfxInfo> interruptInfo{};
    for (size_t i = 0; i < DFX_MSG_ACTION_LOOP_TIMES; i++) {
        renderInfo.push_back({});
        interruptInfo.push_back({});
    }

    const int DFX_MSG_LOOP_TIMES = 20;
    DfxMessage renderMsg = {.appUid = TEST_APP_UID, .renderInfo = renderInfo};
    DfxMessage interruptMsg = {.appUid = TEST_APP_UID, .interruptInfo = interruptInfo};
    for (size_t i = 0; i < DFX_MSG_LOOP_TIMES; i++) {
        manager.Process(renderMsg);
        manager.Process(interruptMsg);
    }

    manager.CheckReportDfxMsg();
    EXPECT_FALSE(manager.isFull_);

    const int32_t DEFAULT_DFX_REPORT_INTERVAL_MIN = 24 * 60;
    manager.lastReportTime_ -= DEFAULT_DFX_REPORT_INTERVAL_MIN;
    manager.Process(renderMsg);
    manager.CheckReportDfxMsg();
    EXPECT_FALSE(manager.isFull_);
    manager.reportQueue_.clear();
}

/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerEnqueue_001
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerEnqueue_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();

    constexpr int32_t BOOTUP_MUSIC_UID = 1003;
    DfxMessage renderMsg = {.appUid = BOOTUP_MUSIC_UID};
    auto ret = manager.Enqueue(renderMsg);
    EXPECT_FALSE(ret);

    DfxMessage renderMsg2 = {.appUid = TEST_APP_UID};
    manager.isFull_ = true;
    ret = manager.Enqueue(renderMsg2);
    EXPECT_FALSE(ret);
    manager.isFull_ = false;
}

/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerAppStateTest_001
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerAppStateTest_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();
    manager.SaveAppInfo({TEST_APP_UID});
    manager.UpdateAppState(TEST_APP_UID, DFX_APP_STATE_START);

    int32_t size = 0;
    int32_t checkSize1 = 1;
    int32_t checkSize2 = 2;
    if (manager.appInfo_.count(TEST_APP_UID)) {
        auto &item = manager.appInfo_[TEST_APP_UID];
        size = item.appStateVec.size();
    }
    EXPECT_EQ(checkSize1, size);

    size = 0;
    manager.UpdateAppState(TEST_APP_UID, DFX_APP_STATE_FOREGROUND);
    if (manager.appInfo_.count(TEST_APP_UID)) {
        auto &item = manager.appInfo_[TEST_APP_UID];
        size = item.appStateVec.size();
    }
    EXPECT_EQ(checkSize1, size);

    manager.UpdateAppState(TEST_APP_UID, DFX_APP_STATE_FOREGROUND, true);
    if (manager.appInfo_.count(TEST_APP_UID)) {
        auto &item = manager.appInfo_[TEST_APP_UID];
        size = item.appStateVec.size();
    }
    EXPECT_EQ(checkSize2, size);
    manager.appInfo_.clear();
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: AudioPolicyConfigManager_001
* @tc.desc  : Test AudioPolicyConfigManager.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyConfigManager_001, TestSize.Level1)
{
    AudioPolicyConfigManager &audioConfigManager_ = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(audioConfigManager_.Init(), false);
    EXPECT_EQ(audioConfigManager_.Init(true), true);

    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    configData.Reorganize();
    std::string version = configData.GetVersion();
    EXPECT_NE(version, "");

    EXPECT_NE(configData.adapterInfoMap.size(), 0);
    EXPECT_NE(configData.deviceInfoMap.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: AudioPolicyConfigManager_002
* @tc.desc  : Test AudioPolicyConfigManager.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyConfigManager_002, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    size_t adapterMapSize = configData.adapterInfoMap.size();
    std::unordered_map<AudioAdapterType, std::pair<size_t, size_t>> adapterSizeMap {};

    for (auto &item : configData.adapterInfoMap) {
        std::pair<size_t, size_t> sizePair = std::make_pair(item.second->deviceInfos.size(),
            item.second->pipeInfos.size());
        adapterSizeMap.insert({item.first, sizePair});
    }

    AudioPolicyConfigManager &audioConfigManager_ = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(audioConfigManager_.Init(true), true);
    configData.Reorganize();

    EXPECT_NE(configData.adapterInfoMap.size(), 0);

    for (auto &item : adapterSizeMap) {
        auto adapterInfoIt = configData.adapterInfoMap.find(item.first);
        EXPECT_NE(adapterInfoIt, configData.adapterInfoMap.end());

        EXPECT_NE(adapterInfoIt->second->adapterName, "");
        EXPECT_NE(adapterInfoIt->second->deviceInfos.size(), 0);
        EXPECT_NE(adapterInfoIt->second->pipeInfos.size(), 0);

        std::pair<size_t, size_t> sizePair = std::make_pair(adapterInfoIt->second->deviceInfos.size(),
            adapterInfoIt->second->pipeInfos.size());
        EXPECT_EQ(item.second, sizePair);

        for (auto &deviceInfo : adapterInfoIt->second->deviceInfos) {
            EXPECT_NE(deviceInfo->supportPipeMap_.size(), 0);
        }

        for (auto &pipeInfo : adapterInfoIt->second->pipeInfos) {
            for (auto &streamPropInfo : pipeInfo->streamPropInfos_) {
                EXPECT_NE(streamPropInfo->supportDeviceMap_.size(), 0);
            }
        }
    }
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: AudioPolicyConfigManager_003
* @tc.desc  : Test AudioPolicyConfigManager.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyConfigManager_003, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    size_t deviceMapSize = configData.deviceInfoMap.size();
    std::unordered_map<std::pair<DeviceType, DeviceRole>, size_t, PairHash> deviceSizeMap;

    for (auto &pair : configData.deviceInfoMap) {
        deviceSizeMap.insert({pair.first, pair.second.size()});
    }

    AudioPolicyConfigManager &audioConfigManager_ = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(audioConfigManager_.Init(true), true);
    configData.Reorganize();

    EXPECT_NE(configData.deviceInfoMap.size(), 0);
    for (auto &pair : deviceSizeMap) {
        auto deviceSetIt = configData.deviceInfoMap.find(pair.first);
        EXPECT_NE(deviceSetIt, configData.deviceInfoMap.end());
        EXPECT_EQ(deviceSetIt->second.size(), pair.second);
    }
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: AudioPolicyConfigManager_004
* @tc.desc  : Test AudioPolicyConfigManager.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyConfigManager_004, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    size_t adapterMapSize = configData.adapterInfoMap.size();
    std::unordered_map<AudioAdapterType, std::pair<size_t, size_t>> adapterSizeMap {};

    for (auto &item : configData.adapterInfoMap) {
        std::pair<size_t, size_t> sizePair = std::make_pair(item.second->deviceInfos.size(),
            item.second->pipeInfos.size());
        adapterSizeMap.insert({item.first, sizePair});
    }

    AudioPolicyConfigManager &audioConfigManager_ = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(audioConfigManager_.Init(true), true);
    configData.Reorganize();

    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;

    bool ret = audioConfigManager_.IsFastStreamSupported(streamInfo, desc);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: AudioPolicyConfigManager_005
* @tc.desc  : Test AudioPolicyConfigManager.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyConfigManager_005, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    size_t adapterMapSize = configData.adapterInfoMap.size();
    std::unordered_map<AudioAdapterType, std::pair<size_t, size_t>> adapterSizeMap {};

    for (auto &item : configData.adapterInfoMap) {
        std::pair<size_t, size_t> sizePair = std::make_pair(item.second->deviceInfos.size(),
            item.second->pipeInfos.size());
        adapterSizeMap.insert({item.first, sizePair});
    }

    AudioPolicyConfigManager &audioConfigManager_ = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(audioConfigManager_.Init(true), true);
    configData.Reorganize();

    AudioStreamInfo streamInfo;
    std::shared_ptr<AdapterDeviceInfo> deviceInfo;
    bool ret = audioConfigManager_.GetFastStreamSupport(streamInfo, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test DFX_MSG_MANAGER
* @tc.number: DfxMsgManagerActionTest_001
* @tc.desc  : Test DFX_MSG_MANAGER interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DfxMsgManagerActionTest_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    auto &manager = DfxMsgManager::GetInstance();
    const int DFX_MSG_ACTION_LOOP_TIMES_1 = 50;
    const int DFX_MSG_ACTION_LOOP_TIMES_2 = 60;
    const int DFX_MSG_ACTION_SPLIT_NUM = 2;
    std::list<RenderDfxInfo> renderInfo{};
    int infoIndex = 0;
    for (size_t i = 1; i <= DFX_MSG_ACTION_LOOP_TIMES_1; i++) {
        DfxStatAction rendererAction{};
        if (i % DFX_MSG_ACTION_SPLIT_NUM == 0) {
            rendererAction.fourthByte = RendererStage::RENDERER_STAGE_START_OK;
            rendererAction.firstByte = ++infoIndex;
        }
        renderInfo.push_back({.rendererAction = rendererAction});
    }

    std::list<RenderDfxInfo> renderInfo2{};
    infoIndex = 0;
    for (size_t i = 1; i <= DFX_MSG_ACTION_LOOP_TIMES_2; i++) {
        DfxStatAction rendererAction{};
        if (i % DFX_MSG_ACTION_SPLIT_NUM == 0) {
            rendererAction.fourthByte = RendererStage::RENDERER_STAGE_START_FAIL;
            rendererAction.firstByte = ++infoIndex;
        }
        renderInfo2.push_back({.rendererAction = rendererAction});
    }

    DfxMessage renderMsg = {.appUid = TEST_APP_UID, .renderInfo = renderInfo};
    DfxMessage renderMsg2 = {.appUid = TEST_APP_UID, .renderInfo = renderInfo2};
    manager.Process(renderMsg);
    manager.Process(renderMsg2);

    int8_t checkValue = 0;
    for (auto &item : manager.reportQueue_) {
        manager.UpdateAction(TEST_APP_UID, item.second.renderInfo);
        checkValue = manager.GetDfxIndexByType(TEST_APP_UID, DfxMsgIndexType::DFX_MSG_IDX_TYPE_RENDER_INFO);
    }

    EXPECT_NE(checkValue, (DFX_MSG_ACTION_LOOP_TIMES_1 / DFX_MSG_ACTION_SPLIT_NUM) +
        (DFX_MSG_ACTION_LOOP_TIMES_2 / DFX_MSG_ACTION_SPLIT_NUM));
    manager.reportQueue_.clear();
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: AudioDeviceDescriptor_001
* @tc.desc  : Test AudioDeviceDescriptor.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioDeviceDescriptor_001, TestSize.Level1)
{
    Parcel parcel;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(audioDeviceDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_NONE;
    audioDeviceDescriptor->MarshallingToDeviceInfo(parcel, false, false, API_10);
    EXPECT_EQ(audioDeviceDescriptor->audioStreamInfo_.size(), 0);

    DeviceStreamInfo streamInfo;
    audioDeviceDescriptor->audioStreamInfo_.push_back(streamInfo);
    audioDeviceDescriptor->MarshallingToDeviceInfo(parcel, false, false, API_10);
    EXPECT_EQ(audioDeviceDescriptor->GetDeviceStreamInfo().samplingRate.size(), 0);

    streamInfo.samplingRate.insert(SAMPLE_RATE_44100);
    streamInfo.channelLayout.insert(CH_LAYOUT_STEREO);
    audioDeviceDescriptor->audioStreamInfo_.push_back(streamInfo);
    audioDeviceDescriptor->MarshallingToDeviceInfo(parcel, false, false, API_10);
    EXPECT_NE(audioDeviceDescriptor->GetDeviceStreamInfo().samplingRate.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: GetStreamPropInfo_001
* @tc.desc  : Test GetStreamPropInfo
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetStreamPropInfo_001, TestSize.Level1)
{
    uint32_t routerFlag = AUDIO_OUTPUT_FLAG_FAST;
    AudioPolicyConfigManager &manager = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    configData.Reorganize();

    std::shared_ptr<PipeStreamPropInfo> propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo->channels_ = AudioChannel::STEREO;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeInfo->streamPropInfos_ = {propInfo};

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->supportPipeMap_.insert({routerFlag, pipeInfo});
    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    auto devicekey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.insert({devicekey, deviceInfoSet});

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = "LocalDevice";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;

    streamDesc->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    manager.GetStreamPropInfo(streamDesc, streamPropInfo);
    streamDesc->routeFlag_ = routerFlag;
    manager.GetStreamPropInfo(streamDesc, streamPropInfo);
    EXPECT_EQ(streamPropInfo->channels_, AudioChannel::CHANNEL_UNKNOW);
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: UpdateBasicStreamInfo_001
* @tc.desc  : Test UpdateBasicStreamInfo
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateBasicStreamInfo_001, TestSize.Level1)
{
    AudioPolicyConfigManager &manager = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);

    streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.channels = MONO;
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);

    pipeInfo = std::make_shared<AdapterPipeInfo>();
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);

    streamDesc->routeFlag_ = (AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST);
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);
    EXPECT_EQ(streamInfo.channels, STEREO);

    streamDesc->routeFlag_ = (AUDIO_OUTPUT_FLAG_VOIP | AUDIO_OUTPUT_FLAG_FAST);
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);
    EXPECT_EQ(streamInfo.channels, STEREO);

    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);

    std::shared_ptr<PipeStreamPropInfo> propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo->channels_ = AudioChannel::STEREO;
    pipeInfo->streamPropInfos_ = {propInfo};
    manager.UpdateBasicStreamInfo(streamDesc, pipeInfo, streamInfo);

    EXPECT_EQ(streamInfo.format, AudioSampleFormat::SAMPLE_S16LE);
    EXPECT_EQ(streamInfo.channels, STEREO);
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: ParseFormat_001
* @tc.desc  : Test ParseFormat
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ParseFormat_001, TestSize.Level1)
{
    AudioPolicyConfigManager &manager = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    EXPECT_EQ(manager.ParseFormat("s16le"), SAMPLE_S16LE);
    EXPECT_EQ(manager.ParseFormat("s24le"), SAMPLE_S24LE);
    EXPECT_EQ(manager.ParseFormat("s32le"), SAMPLE_S32LE);
    EXPECT_EQ(manager.ParseFormat("123"), SAMPLE_S16LE);
}

/**
* @tc.name  : Test AudioPolicyConfigManager.
* @tc.number: CheckDynamicCapturerConfig_001
* @tc.desc  : Test CheckDynamicCapturerConfig
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, CheckDynamicCapturerConfig_001, TestSize.Level1)
{
    AudioPolicyConfigManager &manager = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<PipeStreamPropInfo> info = std::make_shared<PipeStreamPropInfo>();
    deviceDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioModuleInfo moduleInfo = {
        .rate = "8000",
        .format = "s16le",
    };
    manager.dynamicCapturerConfig_[ClassType::TYPE_USB] = moduleInfo;

    manager.CheckDynamicCapturerConfig(streamDesc, info);
    EXPECT_EQ(info->format_, SAMPLE_S16LE);
    EXPECT_EQ(info->sampleRate_, 8000);

    manager.dynamicCapturerConfig_.clear();
    info->format_ = SAMPLE_U8;
    info->sampleRate_ = 0;
    manager.CheckDynamicCapturerConfig(streamDesc, info);
    EXPECT_EQ(info->format_, SAMPLE_U8);
    EXPECT_EQ(info->sampleRate_, 0);

    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_DEFAULT;
    info->format_ = SAMPLE_U8;
    info->sampleRate_ = 0;
    manager.CheckDynamicCapturerConfig(streamDesc, info);
    EXPECT_EQ(info->format_, SAMPLE_U8);
    EXPECT_EQ(info->sampleRate_, 0);
}

/**
* @tc.name  : Test GetMaxVolumeLevel.
* @tc.number: GetMaxVolumeLevel_001
* @tc.desc  : valid parameters, return SUCCESS and volumeLevel is correct.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetMaxVolumeLevel_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    int32_t volumeType = 1;
    int32_t volumeLevel = 0;
    int32_t deviceType = 0;
    int ret = server->GetMaxVolumeLevel(volumeType, volumeLevel, deviceType);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, MAX_VOLUME_LEVEL);
}

/**
* @tc.name  : Test GetMaxVolumeLevel.
* @tc.number: GetMaxVolumeLevel_002
* @tc.desc  : no valid parameters, return SUCCESS and volumeLevel is default value.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetMaxVolumeLevel_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    int32_t volumeType = -1;
    int32_t volumeLevel = 0;
    int32_t deviceType = -1;
    int ret = server->GetMaxVolumeLevel(volumeType, volumeLevel, deviceType);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test GetMinVolumeLevel.
* @tc.number: GetMaxVolumeLevel_001
* @tc.desc  : valid parameters, return SUCCESS and volumeLevel is correct.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetMinVolumeLevel_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    int32_t volumeType = 1;
    int32_t volumeLevel = 0;
    int32_t deviceType = 0;
    int ret = server->GetMinVolumeLevel(volumeType, volumeLevel, deviceType);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, MIN_VOLUME_LEVEL);
}

/**
* @tc.name  : Test GetMinVolumeLevel.
* @tc.number: GetMinVolumeLevel_002
* @tc.desc  : no valid parameters, return SUCCESS and volumeLevel is default value.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetMinVolumeLevel_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    int32_t volumeType = -1;
    int32_t volumeLevel = 0;
    int32_t deviceType = -1;
    int ret = server->GetMinVolumeLevel(volumeType, volumeLevel, deviceType);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, ERR_INVALID_PARAM);
}
} // namespace AudioStandard
} // namespace OHOS
