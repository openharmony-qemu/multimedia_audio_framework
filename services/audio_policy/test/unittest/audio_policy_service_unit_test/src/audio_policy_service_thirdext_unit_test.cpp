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
    GetServerUtil::GetServerPtr()->audioPolicyService_.OnReceiveBluetoothEvent(macAddress, deviceName);
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
* @tc.name  : Test GetAudioEffectOffloadFlag.
* @tc.number: GetAudioEffectOffloadFlag_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetAudioEffectOffloadFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetAudioEffectOffloadFlag_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.GetAudioEffectOffloadFlag();
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
* @tc.name  : Test LoadSplitModule.
* @tc.number: LoadSplitModule_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, LoadSplitModule_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest LoadSplitModule_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    const std::string networkIdOne = "";
    const std::string splitArgOne = "";
    int32_t result = GetServerUtil::GetServerPtr()->audioPolicyService_.LoadSplitModule(
        splitArgOne, networkIdOne);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    const std::string networkIdTwo = LOCAL_NETWORK_ID;
    const std::string splitArgTwo = "11.22";
    result = GetServerUtil::GetServerPtr()->audioPolicyService_.LoadSplitModule(
        splitArgTwo, networkIdTwo);
    EXPECT_EQ(ERR_INVALID_HANDLE, result);
}

/**
* @tc.name  : Test SetDefaultOutputDevice.
* @tc.number: SetDefaultOutputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetDefaultOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    bool isRunning = true;
    const StreamUsage streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    const uint32_t sessionID = 0;

    int32_t result =
        GetServerUtil::GetServerPtr()->audioPolicyService_.SetDefaultOutputDevice(
            deviceType, sessionID, streamUsage, isRunning);
    EXPECT_EQ(SUCCESS, result);
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioConfigManager_.hasEarpiece_ = true;
    result =
        GetServerUtil::GetServerPtr()->audioPolicyService_.SetDefaultOutputDevice(
            deviceType, sessionID, streamUsage, isRunning);
    EXPECT_EQ(SUCCESS, result);
}

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
    EXPECT_EQ(A2DP_STOPPED, playingState);
    EXPECT_EQ(CONNECTION_STATUS_CONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
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
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
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
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
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
    EXPECT_EQ(CONNECTION_STATUS_CONNECTED,
        audioA2dpOffloadManager_.audioA2dpOffloadFlag_.currentOffloadConnectionState_);
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
* @tc.name  : Test GetAudioEffectProperty.
* @tc.number: GetAudioEffectProperty_001
* @tc.desc  : Test GetAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetAudioEffectProperty_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioEffectPropertyArray propertyArrayTest;
    GetServerUtil::GetServerPtr()->GetAudioEffectProperty(propertyArrayTest);
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
* @tc.name  : Test GetAudioEnhanceProperty.
* @tc.number: GetAudioEnhanceProperty_001
* @tc.desc  : Test GetAudioEnhanceProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetAudioEnhanceProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetAudioEnhanceProperty_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    AudioEnhancePropertyArray propertyArrayTest;
    GetServerUtil::GetServerPtr()->GetAudioEnhanceProperty(propertyArrayTest);
}

/**
* @tc.name  : Test SetAudioConcurrencyCallback.
* @tc.number: SetAudioConcurrencyCallback_001
* @tc.desc  : Test SetAudioConcurrencyCallback interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetAudioConcurrencyCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetAudioConcurrencyCallback_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    sptr<IRemoteObject> objectSptrTest = nullptr;
    GetServerUtil::GetServerPtr()->SetAudioConcurrencyCallback(TEST_SESSIONID, objectSptrTest);
}

/**
* @tc.name  : Test UnsetAudioConcurrencyCallback.
* @tc.number: UnsetAudioConcurrencyCallback_001
* @tc.desc  : Test UnsetAudioConcurrencyCallback interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UnsetAudioConcurrencyCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UnsetAudioConcurrencyCallback_001 start");
    AudioPolicyServer* server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<IRemoteObject> objectSptrTest = nullptr;
    server->SetAudioConcurrencyCallback(TEST_SESSIONID, objectSptrTest);
    server->UnsetAudioConcurrencyCallback(TEST_SESSIONID);
}

/**
* @tc.name  : Test ActivateAudioConcurrency.
* @tc.number: ActivateAudioConcurrency_001
* @tc.desc  : Test ActivateAudioConcurrency interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ActivateAudioConcurrency_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ActivateAudioConcurrency_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    GetServerUtil::GetServerPtr()->ActivateAudioConcurrency(PIPE_TYPE_UNKNOWN);
}

/**
* @tc.name  : Test ResetRingerModeMute.
* @tc.number: ResetRingerModeMute_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ResetRingerModeMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ResetRingerModeMute_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    GetServerUtil::GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = true;
    auto ret = GetServerUtil::GetServerPtr()->audioPolicyService_.ResetRingerModeMute();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test ResetRingerModeMute.
* @tc.number: ResetRingerModeMute_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ResetRingerModeMute_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ResetRingerModeMute_002 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    GetServerUtil::GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = false;
    auto ret = GetServerUtil::GetServerPtr()->audioPolicyService_.ResetRingerModeMute();
    EXPECT_EQ(SUCCESS, ret);
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
    server->audioPolicyService_.audioDeviceLock_.UpdateSessionConnectionState(sessionID, state);
    EXPECT_EQ(nullptr, AudioServerProxy::GetInstance().GetAudioServerProxy());
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
* @tc.name  : Test UpdateEffectBtOffloadSupported.
* @tc.number: UpdateEffectBtOffloadSupported_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateEffectBtOffloadSupported_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateEffectBtOffloadSupported_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    bool isSupported = false;
    server->audioPolicyService_.UpdateEffectBtOffloadSupported(isSupported);
    EXPECT_EQ(nullptr, AudioServerProxy::GetInstance().GetAudioServerProxy());
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
    EXPECT_NE(SUCCESS, result);

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
* @tc.name  : Test DealAudioSceneOutputDevices.
* @tc.number: DealAudioSceneOutputDevices_001
* @tc.desc  : Test IsA2dpOffloadConnected interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DealAudioSceneOutputDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest DealAudioSceneOutputDevices_001 start");
    ASSERT_NE(nullptr, GetServerUtil::GetServerPtr());

    const AudioScene audioScene = AUDIO_SCENE_RINGING;
    std::vector<DeviceType> activeOutputDevices;
    bool haveArmUsbDevice = false;
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioSceneManager_.DealAudioSceneOutputDevices(
        audioScene, activeOutputDevices, haveArmUsbDevice);
    EXPECT_EQ(false, haveArmUsbDevice);

    const AudioScene audioScene2 = AUDIO_SCENE_VOICE_RINGING;
    haveArmUsbDevice = false;
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioSceneManager_.DealAudioSceneOutputDevices(
        audioScene2, activeOutputDevices, haveArmUsbDevice);
    EXPECT_EQ(false, haveArmUsbDevice);

    const AudioScene audioScene3 = AUDIO_SCENE_DEFAULT;
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs {};
    haveArmUsbDevice = false;
    GetServerUtil::GetServerPtr()->audioPolicyService_.audioSceneManager_.DealAudioSceneOutputDevices(
        audioScene3, activeOutputDevices, haveArmUsbDevice);
    EXPECT_TRUE(descs.empty());
    EXPECT_EQ(false, haveArmUsbDevice);
}

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
* @tc.name  : Test IsA2dpOffloadConnected.
* @tc.number: IsA2dpOffloadConnected_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnected_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnected_001 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    EXPECT_EQ(false, server->audioPolicyService_.IsA2dpOffloadConnected());
}

/**
* @tc.name  : Test IsA2dpOffloadConnected.
* @tc.number: IsA2dpOffloadConnected_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnected_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnected_002 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    server->audioPolicyService_.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    ASSERT_NE(nullptr, server->audioPolicyService_.audioA2dpOffloadManager_);
    EXPECT_EQ(false, server->audioPolicyService_.IsA2dpOffloadConnected());
}

/**
* @tc.name  : Test SetA2dpOffloadFlag.
* @tc.number: SetA2dpOffloadFlag_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetA2dpOffloadFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetA2dpOffloadFlag_001 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    BluetoothOffloadState state = NO_A2DP_DEVICE;
    server->audioPolicyService_.SetA2dpOffloadFlag(state);
}
/**
* @tc.name  : Test SetA2dpOffloadFlag.
* @tc.number: SetA2dpOffloadFlag_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetA2dpOffloadFlag_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetA2dpOffloadFlag_002 start");
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_NE(nullptr, server);
    server->audioPolicyService_.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    ASSERT_NE(nullptr, server->audioPolicyService_.audioA2dpOffloadManager_);
    BluetoothOffloadState state = NO_A2DP_DEVICE;
    server->audioPolicyService_.SetA2dpOffloadFlag(state);
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
* @tc.name  : Test ActivateConcurrencyFromServer.
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

/**
* @tc.name  : Test ActivateConcurrencyFromServer.
* @tc.number: ActivateConcurrencyFromServer_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ActivateConcurrencyFromServer_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ActivateConcurrencyFromServer_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    int32_t result = server->audioPolicyService_.ActivateConcurrencyFromServer(pipeType);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test IsAllowedPlayback.
* @tc.number: IsAllowedPlayback_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsAllowedPlayback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsAllowedPlayback_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    const int32_t uid = 0;
    const int32_t pid = 0;
    EXPECT_FALSE(server->audioPolicyService_.IsAllowedPlayback(uid, pid));
}

/**
* @tc.name  : Test IsAllowedPlayback.
* @tc.number: IsAllowedPlayback_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsAllowedPlayback_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsAllowedPlayback_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    const int32_t uid = 1003;
    const int32_t pid = 0;
    EXPECT_TRUE(server->audioPolicyService_.IsAllowedPlayback(uid, pid));
}

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
 * @tc.name  : Test ExcludeOutputDevices
 * @tc.number: ExcludeOutputDevices_001
 * @tc.desc  : Test ExcludeOutputDevices interfaces.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, ExcludeOutputDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ExcludeOutputDevices_001 start");
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

    int32_t ret = server->audioPolicyService_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test ExcludeOutputDevices
 * @tc.number: ExcludeOutputDevices_002
 * @tc.desc  : Test ExcludeOutputDevices interfaces.
 */
HWTEST_F(AudioPolicyServiceFourthUnitTest, ExcludeOutputDevices_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ExcludeOutputDevices_002 start");
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

    int32_t ret = server->audioPolicyService_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(SUCCESS, ret);
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

    server->audioPolicyService_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    int32_t result = server->audioPolicyService_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
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

    server->audioPolicyService_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    int32_t result = server->audioPolicyService_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
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
        server->audioPolicyService_.GetExcludedDevices(audioDevUsage);
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
        server->audioPolicyService_.GetExcludedDevices(audioDevUsage);
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

    EXPECT_EQ(DFX_MSG_LOOP_TIMES, manager.reportQueue_.size());
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

    EXPECT_EQ(DFX_MSG_LOOP_TIMES, manager.reportQueue_.size());
    auto upper = manager.reportQueue_.upper_bound(TEST_APP_UID);
    auto lastIt = --upper;
    auto checkValue = static_cast<int>(lastIt->second.renderInfo.back().rendererAction.firstByte);

    EXPECT_EQ(DFX_MSG_ACTION_LOOP_TIMES - (DFX_MSG_ACTION_LOOP_TIMES_2 - DFX_MSG_ACTION_LOOP_TIMES_1), checkValue);
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
    EXPECT_EQ(DFX_MSG_ACTION_LOOP_TIMES_2 + DFX_MSG_ACTION_LOOP_TIMES_1 - DFX_MSG_MAX_ACTION_SIZE, checkValue2);
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
    EXPECT_TRUE(manager.isFull_);

    const int32_t DEFAULT_DFX_REPORT_INTERVAL_MIN = 24 * 60;
    manager.lastReportTime_ -= DEFAULT_DFX_REPORT_INTERVAL_MIN;
    manager.Process(renderMsg);
    manager.CheckReportDfxMsg();
    EXPECT_TRUE(manager.isFull_);
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

} // namespace AudioStandard
} // namespace OHOS
