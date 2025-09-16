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

#include "audio_policy_service_ext_unit_test.h"
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
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";

void AudioPolicyServiceExtUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceExtUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceExtUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUp start-end");
}
void AudioPolicyServiceExtUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDown start-end");
}

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
* @tc.desc  : Test LoadAudioPolicyConfig interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_001, TestSize.Level1)
{
    AudioPolicyConfigManager::GetInstance().xmlHasLoaded_ = true;
    auto ret = AudioPolicyService::GetAudioPolicyService().LoadAudioPolicyConfig();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_002
* @tc.desc  : Test Init interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_002, TestSize.Level1)
{
    AudioPolicyConfigManager::GetInstance().xmlHasLoaded_ = true;
    auto ret = AudioPolicyService::GetAudioPolicyService().LoadAudioPolicyConfig();
    EXPECT_FALSE(ret);
    ret = AudioPolicyService::GetAudioPolicyService().Init();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_003
* @tc.desc  : Test CreateRecoveryThread interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_003, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().CreateRecoveryThread();
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().RecoveryDevicesThread_);
    AudioPolicyService::GetAudioPolicyService().CreateRecoveryThread();
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_004
* @tc.desc  : Test Deinit interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_004, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = true;
    AudioPolicyService::GetAudioPolicyService().Deinit();
    EXPECT_EQ(false, AudioPolicyService::isBtListenerRegistered);
}

/**
* @tc.name  : Test SafeVolumeEventSubscriber.
* @tc.number: SafeVolumeEventSubscriber_001
* @tc.desc  : Test OnReceiveEvent interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SafeVolumeEventSubscriber_001, TestSize.Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(AUDIO_RESTORE_VOLUME_EVENT);
    matchingSkills.AddEvent(AUDIO_INCREASE_VOLUME_EVENT);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto commonSubscribePtr = std::make_shared<SafeVolumeEventSubscriber>(subscribeInfo,
        [](const EventFwk::CommonEventData&){});
    ASSERT_NE(nullptr, commonSubscribePtr);
    const EventFwk::CommonEventData eventData;
    commonSubscribePtr->OnReceiveEvent(eventData);
    EXPECT_NE(nullptr, commonSubscribePtr->eventReceiver_);
    commonSubscribePtr->eventReceiver_ = nullptr;
    commonSubscribePtr->OnReceiveEvent(eventData);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_005
* @tc.desc  : Test SetSourceOutputStreamMute interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_005, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().Init();
    int32_t uid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().SetSourceOutputStreamMute(uid, true);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_006
* @tc.desc  : Test GetSelectedDeviceInfo interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_006, TestSize.Level1)
{
    int32_t uid = 123, pid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetSelectedDeviceInfo(uid, pid, STREAM_MUSIC);
    EXPECT_EQ("", ret);

    AudioRouteMap::GetInstance().AddRouteMapInfo(uid, "LocalDevice", pid);
    ret = AudioPolicyService::GetAudioPolicyService().GetSelectedDeviceInfo(uid, pid, STREAM_MUSIC);
    EXPECT_EQ("", ret);

    AudioRouteMap::GetInstance().AddRouteMapInfo(uid, "test", pid);
    ret = AudioPolicyService::GetAudioPolicyService().GetSelectedDeviceInfo(uid, pid, STREAM_MUSIC);
    EXPECT_EQ("", ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_07
* @tc.desc  : Test LoadModernOffloadCapSource interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_07, TestSize.Level1)
{
    auto ret0 = AudioPolicyService::GetAudioPolicyService().LoadModernOffloadCapSource();
    EXPECT_EQ(SUCCESS, ret0);

    AudioIOHandle audioIOHandle = 1003;
    AudioPolicyService::GetAudioPolicyService().audioIOHandleMap_.IOHandles_["offloadCapturerSource"] = audioIOHandle;

    auto ret1 = AudioPolicyService::GetAudioPolicyService().LoadModernOffloadCapSource();
    EXPECT_EQ(SUCCESS, ret1);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_008
* @tc.desc  : Test GetOutputDevice interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_008, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter(new AudioRendererFilter());
    audioRendererFilter->uid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetOutputDevice(audioRendererFilter);
    EXPECT_NE(nullptr, audioRendererFilter);
    audioRendererFilter->uid = -1;
    ret = AudioPolicyService::GetAudioPolicyService().GetOutputDevice(audioRendererFilter);
    EXPECT_NE(0, ret.size());
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_009
* @tc.desc  : Test GetInputDevice interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_009, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter(new AudioCapturerFilter());
    audioCapturerFilter->uid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetInputDevice(audioCapturerFilter);
    EXPECT_NE(nullptr, audioCapturerFilter);
    audioCapturerFilter->uid = -1;
    ret = AudioPolicyService::GetAudioPolicyService().GetInputDevice(audioCapturerFilter);
    EXPECT_EQ(0, ret.size());
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_010
* @tc.desc  : Test UpdateA2dpOffloadFlagBySpatialService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_010, TestSize.Level1)
{
    std::string macAddress = "00-15-5D-E6-DE-FC";
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnableMap;
    sessionIDToSpatializationEnableMap.insert({123, false});
    AudioPolicyService::GetAudioPolicyService().UpdateA2dpOffloadFlagBySpatialService(macAddress,
        sessionIDToSpatializationEnableMap);
    EXPECT_EQ(nullptr, AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_);
    AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_ =
        std::make_shared<AudioA2dpOffloadManager>();
    AudioPolicyService::GetAudioPolicyService().UpdateA2dpOffloadFlagBySpatialService(macAddress,
        sessionIDToSpatializationEnableMap);
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_011
* @tc.desc  : Test GetCurrentCapturerChangeInfos interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_011, TestSize.Level1)
{
    AudioConnectedDevice::GetInstance().AddConnectedDevice(std::make_shared<AudioDeviceDescriptor>());
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(std::make_shared<AudioCapturerChangeInfo>());
    auto ret = AudioPolicyService::GetAudioPolicyService().GetCurrentCapturerChangeInfos(
        audioCapturerChangeInfos, false, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_012
* @tc.desc  : Test GetPreferredOutputStreamType interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_012, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::string bundleName = "test";
    AudioPolicyService::GetAudioPolicyService().isFastControlled_ = false;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
    AudioPolicyService::GetAudioPolicyService().isFastControlled_ = true;
    rendererInfo.playerType = PLAYER_TYPE_SOUND_POOL;
    ret = AudioPolicyService::GetAudioPolicyService().GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
    rendererInfo.playerType = PLAYER_TYPE_AV_PLAYER;
    ret = AudioPolicyService::GetAudioPolicyService().GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
    rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    ret = AudioPolicyService::GetAudioPolicyService().GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(AUDIO_FLAG_MMAP, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_014
* @tc.desc  : Test RegisterBluetoothListener interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_014, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().deviceStatusListener_);
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    AudioPolicyService::GetAudioPolicyService().UnregisterBluetoothListener();
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().deviceStatusListener_);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_015
* @tc.desc  : Test CheckSupportedAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_015, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 propertyArray;
    EffectFlag flag = CAPTURE_EFFECT_FLAG;
    auto ret = AudioPolicyService::GetAudioPolicyService().CheckSupportedAudioEffectProperty(propertyArray, flag);
    EXPECT_EQ(AUDIO_OK, ret);
    flag = RENDER_EFFECT_FLAG;
    ret = AudioPolicyService::GetAudioPolicyService().CheckSupportedAudioEffectProperty(propertyArray, flag);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_016
* @tc.desc  : Test SetAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_016, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 propertyArray;
    AudioEffectPropertyV3 tmp;
    tmp.flag = CAPTURE_EFFECT_FLAG;
    propertyArray.property.push_back(tmp);
    tmp.flag = RENDER_EFFECT_FLAG;
    propertyArray.property.push_back(tmp);
    auto ret = AudioPolicyService::GetAudioPolicyService().SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}
} // namespace AudioStandard
} // namespace OHOS