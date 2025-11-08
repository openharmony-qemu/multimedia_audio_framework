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

#include "audio_policy_service_extended_unit_test.h"
#include "get_server_util.h"

#include <thread>
#include <memory>
#include <vector>
#include "audio_info.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermission = false;
static const std::string INNER_CAPTURER_SINK_LEGACY = "InnerCapturer";
const int32_t CONNECTING_NUMBER = 10;
const int32_t TEST_SESSIONID = MIN_STREAMID + 1010;
const int32_t ROUTER_MAP_ID0 = 1000;
const int32_t ROUTER_MAP_ID1 = 1001;
const int32_t ROUTER_MAP_ID2 = 1002;
const int32_t ROUTER_MAP_ID3 = 1003;
const int32_t ROUTER_MAP_ID4 = 1004;
const int32_t ROUTER_MAP_ID5 = 1005;
const int32_t ROUTER_MAP_ID6 = 1006;
const int32_t VALUE_ZERO = 0;
const int32_t DEFAULT_VOLUME_LEVEL = 7;
const int32_t G_UNKNOWN_PID = -1;
const uint32_t CHANNELS = 2;
const uint32_t RATE = 4;

void AudioPolicyServiceExtendedUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceExtendedUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceExtendedUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest::SetUp start-end");
}
void AudioPolicyServiceExtendedUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest::TearDown start-end");
}

static AudioPolicyServer* GetServerPtr()
{
    return GetServerUtil::GetServerPtr();
}

static void GetPermission()
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
            .processName = "audio_policy_service_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

static const std::vector<DeviceType> deviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_HDMI
};

static const std::vector<StreamSetState> streamSetStates = {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE
};

static const std::vector<bool>isConnecteds = {
    true,
    false
};
static const std::vector<BluetoothOffloadState>flags = {
    NO_A2DP_DEVICE,
    A2DP_NOT_OFFLOAD,
    A2DP_OFFLOAD
};
static const std::vector<AudioPipeType>pipeTypes = {
    PIPE_TYPE_UNKNOWN,
    PIPE_TYPE_NORMAL_OUT,
    PIPE_TYPE_NORMAL_IN,
    PIPE_TYPE_OFFLOAD,
    PIPE_TYPE_MULTICHANNEL
};
static const std::vector<Media::MediaMonitor::PreferredType>preferredTypes = {
    Media::MediaMonitor::MEDIA_RENDER,
    Media::MediaMonitor::CALL_RENDER,
    Media::MediaMonitor::RING_RENDER,
    Media::MediaMonitor::TONE_RENDER,
    Media::MediaMonitor::CALL_CAPTURE,
    Media::MediaMonitor::RECORD_CAPTURE
};
static const std::vector<StreamUsage>streamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX
};
static const std::vector<AudioStreamType>audioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_TYPE_MAX,
    STREAM_ALL
};
static const std::vector<bool>isMutes = {
    true,
    false
};
static const std::vector<DeviceRole>deviceRoles = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX
};
static const std::vector<DeviceFlag>deviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};
static const std::vector<AudioSampleFormat>audioSampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH
};
static const std::vector<AudioScene>audioScenes = {
    AUDIO_SCENE_INVALID,
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING,
    AUDIO_SCENE_MAX
};
static const std::vector<AudioRingerMode>audioRingerModes = {
    RINGER_MODE_SILENT,
    RINGER_MODE_VIBRATE,
    RINGER_MODE_NORMAL
};

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SetDefaultAdapterEnable
* @tc.desc  : Test SetDefaultAdapterEnable.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SetDefaultAdapterEnable, TestSize.Level1)
{
    bool isEnable = false;
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().SetDefaultAdapterEnable(isEnable);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: UnregisterBluetoothListener
* @tc.desc  : Test UnregisterBluetoothListener.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UnregisterBluetoothListener, TestSize.Level1)
{
#ifdef BLUETOOTH_ENABLE
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().UnregisterBluetoothListener();
    );
#endif
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: OnServiceConnected
* @tc.desc  : Test OnServiceConnected.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, OnServiceConnected, TestSize.Level1)
{
    AudioServiceIndex serviceIndex = AudioServiceIndex::HDI_SERVICE_INDEX;
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().OnServiceConnected(serviceIndex);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SetDefaultDeviceLoadFlag
* @tc.desc  : Test SetDefaultDeviceLoadFlag.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SetDefaultDeviceLoadFlag, TestSize.Level1)
{
    bool isLoad = false;
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().SetDefaultDeviceLoadFlag(isLoad);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: RestoreSession
* @tc.desc  : Test RestoreSession.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, RestoreSession, TestSize.Level1)
{
    uint32_t sessionId = 0;
    RestoreInfo restoreInfo = {};
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().RestoreSession(sessionId, restoreInfo);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: UpdateSpatializationSupported
* @tc.desc  : Test UpdateSpatializationSupported.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateSpatializationSupported, TestSize.Level1)
{
    std::string macAddress = "02:7f:3a:9d:1c:5b";
    bool support = false;
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().UpdateSpatializationSupported(macAddress, support);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SubscribeSafeVolumeEvent
* @tc.desc  : Test SubscribeSafeVolumeEvent.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SubscribeSafeVolumeEvent, TestSize.Level1)
{
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().SubscribeSafeVolumeEvent();
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: getFastControlParam
* @tc.desc  : Test getFastControlParam.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, getFastControlParam, TestSize.Level1)
{
    bool ret = AudioPolicyService::GetAudioPolicyService().getFastControlParam();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: NotifyAccountsChanged
* @tc.desc  : Test NotifyAccountsChanged.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, NotifyAccountsChanged, TestSize.Level1)
{
    int id = 1;
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().NotifyAccountsChanged(id);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: GetDistributedRoutingRoleInfo
* @tc.desc  : Test GetDistributedRoutingRoleInfo.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetDistributedRoutingRoleInfo, TestSize.Level1)
{
    DistributedRoutingInfo ret = AudioPolicyService::GetAudioPolicyService().GetDistributedRoutingRoleInfo();
    EXPECT_EQ(ret.descriptor, nullptr);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: GetAndSaveClientType
* @tc.desc  : Test GetAndSaveClientType.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetAndSaveClientType, TestSize.Level1)
{
    uint32_t uid = 1;
    std::string bundleName = "test";
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetAndSaveClientType(uid, bundleName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: UpdateDescWhenNoBTPermission
* @tc.desc  : Test UpdateDescWhenNoBTPermission.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateDescWhenNoBTPermission, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descs.push_back(std::move(audioDeviceDescriptor));
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().UpdateDescWhenNoBTPermission(descs);
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: RegisterDataObserver
* @tc.desc  : Test RegisterDataObserver.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, RegisterDataObserver, TestSize.Level1)
{
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().RegisterDataObserver();
    );
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: RegisterRemoteDevStatusCallback
* @tc.desc  : Test RegisterRemoteDevStatusCallback.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, RegisterRemoteDevStatusCallback, TestSize.Level1)
{
#ifdef FEATURE_DEVICE_MANAGER
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NO_THROW(
        AudioPolicyService::GetAudioPolicyService().RegisterRemoteDevStatusCallback();
    );
#endif
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: RestoreSession_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, RestoreSession_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest RestoreSession_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    const int32_t testID = 0;
    RestoreInfo restoreInfo;
    GetServerPtr()->audioPolicyService_.RestoreSession(testID, restoreInfo);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: GetDistributedRoutingRoleInfo_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetDistributedRoutingRoleInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest GetDistributedRoutingRoleInfo_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    GetServerPtr()->audioPolicyService_.GetDistributedRoutingRoleInfo();
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: GetActiveOutputDeviceDescriptor_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetActiveOutputDeviceDescriptor_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest GetActiveOutputDeviceDescriptor_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    auto res = GetServerPtr()->audioPolicyService_.GetActiveOutputDeviceDescriptor();
    ASSERT_NE(nullptr, res);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: OnMicrophoneBlockedUpdate_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, OnMicrophoneBlockedUpdate_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest OnMicrophoneBlockedUpdate_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    GetServerPtr()->audioPolicyService_.OnMicrophoneBlockedUpdate(DeviceType::DEVICE_TYPE_EARPIECE,
        DeviceBlockStatus::DEVICE_BLOCKED);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: OnMicrophoneBlockedUpdate_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, OnMicrophoneBlockedUpdate_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest OnMicrophoneBlockedUpdate_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    GetServerPtr()->audioPolicyService_.OnMicrophoneBlockedUpdate(DeviceType::DEVICE_TYPE_NONE,
        DeviceBlockStatus::DEVICE_BLOCKED);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: GetAllSinkInputs_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetAllSinkInputs_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest GetAllSinkInputs_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    std::vector<SinkInput> sinkInputs;
    GetServerPtr()->audioPolicyService_.GetAllSinkInputs(sinkInputs);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SubscribeSafeVolumeEvent_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SubscribeSafeVolumeEvent_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest SubscribeSafeVolumeEvent_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    GetServerPtr()->audioPolicyService_.SubscribeSafeVolumeEvent();
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SetAvailableDeviceChangeCallback_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SetAvailableDeviceChangeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest SetAvailableDeviceChangeCallback_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    int32_t clientId = 0;
    const sptr<IRemoteObject> object = new RemoteObjectTestStub();
    int32_t ret = GetServerPtr()->audioPolicyService_.SetAvailableDeviceChangeCallback(clientId,
        AudioDeviceUsage::MEDIA_INPUT_DEVICES, object, true);
    ASSERT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SetAvailableDeviceChangeCallback_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SetAvailableDeviceChangeCallback_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest SetAvailableDeviceChangeCallback_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    int32_t clientId = 0;
    const sptr<IRemoteObject> object = nullptr;
    int32_t ret = GetServerPtr()->audioPolicyService_.SetAvailableDeviceChangeCallback(clientId,
        AudioDeviceUsage::MEDIA_INPUT_DEVICES, object, true);
    ASSERT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: SetQueryClientTypeCallback_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, SetQueryClientTypeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest SetQueryClientTypeCallback_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    const sptr<IRemoteObject> object = new RemoteObjectTestStub();
    int32_t ret = GetServerPtr()->audioPolicyService_.SetQueryClientTypeCallback(object);
    ASSERT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: UpdateDescWhenNoBTPermission_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateDescWhenNoBTPermission_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest UpdateDescWhenNoBTPermission_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "LocalDevice";
    devices.push_back(desc);
    
    GetServerPtr()->audioPolicyService_.UpdateDescWhenNoBTPermission(devices);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: UpdateDescWhenNoBTPermission_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateDescWhenNoBTPermission_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest UpdateDescWhenNoBTPermission_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "LocalDevice";
    devices.push_back(desc);
    
    GetServerPtr()->audioPolicyService_.UpdateDescWhenNoBTPermission(devices);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: UpdateDescWhenNoBTPermission_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateDescWhenNoBTPermission_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest UpdateDescWhenNoBTPermission_003 start");
    ASSERT_NE(nullptr, GetServerPtr());
    GetPermission();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc;
    devices.push_back(desc);
    
    GetServerPtr()->audioPolicyService_.UpdateDescWhenNoBTPermission(devices);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: UpdateCapturerInfoWhenNoPermission_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateCapturerInfoWhenNoPermission_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest UpdateCapturerInfoWhenNoPermission_001 start");
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioDeviceDescriptor inputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo1 = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo1->clientUID = 5000;
    captureChangeInfo1->createrUID = 1001;
    captureChangeInfo1->sessionId = 2001;
    captureChangeInfo1->inputDeviceInfo = inputDeviceInfo;

    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo2 = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo2->clientUID = 1001;
    captureChangeInfo2->createrUID = 1001;
    captureChangeInfo2->sessionId = 2001;
    captureChangeInfo2->inputDeviceInfo = inputDeviceInfo;

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(move(captureChangeInfo1));
    audioCapturerChangeInfos.push_back(move(captureChangeInfo2));
    bool hasBTPermission = false;
    bool hasSystemPermission = false;

    int32_t ret = server->audioPolicyService_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission,
        hasSystemPermission);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: UpdateCapturerInfoWhenNoPermission_002
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceExtendedUnitTest, UpdateCapturerInfoWhenNoPermission_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtendedUnitTest UpdateCapturerInfoWhenNoPermission_002 start");
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);
    
    AudioDeviceDescriptor inputDeviceInfo(AudioDeviceDescriptor::AUDIO_DEVICE_DESCRIPTOR);
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo->clientUID = 5000;
    captureChangeInfo->createrUID = 1001;
    captureChangeInfo->sessionId = 2001;
    captureChangeInfo->inputDeviceInfo = inputDeviceInfo;

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(move(captureChangeInfo));
    bool hasBTPermission = true;
    bool hasSystemPermission = true;

    int32_t ret = server->audioPolicyService_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission,
        hasSystemPermission);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetPreferredOutputStreamType_002
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetPreferredOutputStreamType_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server) ;

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MAX ;
    std::string bundleName ;

    int32_t ret = server->audioPolicyService_.GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetPreferredOutputStreamType_004
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetPreferredOutputStreamType_004, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server) ;

    AudioRendererInfo rendererInfo;
    rendererInfo.rendererFlags = AUDIO_FLAG_VOIP_FAST;
    std::string bundleName ;
    auto& sceneManager = AudioSceneManager::GetInstance();
    sceneManager.SetAudioScenePre(AUDIO_SCENE_PHONE_CALL, 1, 1);

    int32_t ret = server->audioPolicyService_.GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetHardwareOutputSamplingRate_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetHardwareOutputSamplingRate_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    int32_t ret = server->audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(ret, -1);

    desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = DeviceRole::DEVICE_ROLE_NONE;
    ret = server->audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(ret, -1);

    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    ret = server->audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    EXPECT_NE(ret, 48000);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetProcessDeviceInfo_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceExtendedUnitTest, GetProcessDeviceInfo_001, TestSize.Level1)
{
    auto &policyService = AudioPolicyService::GetAudioPolicyService();
    auto config = AudioProcessConfig();
    config.rendererInfo.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
    config.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
    config.capturerInfo.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
    bool lockFlag = false;
    auto deviceInfo = AudioDeviceDescriptor();
    int32_t ret = 0;

    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;

    lockFlag = false;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);

    lockFlag = true;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_MESSAGE;

    lockFlag = false;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);

    lockFlag = true;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);

    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;

    lockFlag = false;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);

    lockFlag = true;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;

    lockFlag = false;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);

    lockFlag = true;
    ret = policyService.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    ASSERT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
