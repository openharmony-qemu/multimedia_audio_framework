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

#include "audio_policy_server_extended_test.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void GetPermission()
{
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
        .processName = "audio_unit_test",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
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

void AudioPolicyServerUnitTest::SetUpTestCase(void)
{
    GetPermission();
}

void AudioPolicyServerUnitTest::TearDownTestCase(void) {}
void AudioPolicyServerUnitTest::SetUp(void)
{
    audioPolicyServer_ = std::make_shared<AudioPolicyServer>(systemAbilityId_, runOnCreate_);
}

void AudioPolicyServerUnitTest::TearDown(void)
{
    audioPolicyServer_ = nullptr;
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_001
* @tc.desc  : Test OnAddSystemAbility.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_001, TestSize.Level4)
{
    int32_t systemAbilityId = BACKGROUND_TASK_MANAGER_SERVICE_ID;
    std::string deviceId = "test_device_id";
    audioPolicyServer_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(audioPolicyServer_->audioBackgroundManager_.backgroundTaskListener_ != nullptr);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_002
* @tc.desc  : Test RegisterDefaultVolumeTypeListener.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_002, TestSize.Level4)
{
    audioPolicyServer_->interruptService_.reset();
    audioPolicyServer_->RegisterDefaultVolumeTypeListener();
    EXPECT_TRUE(audioPolicyServer_->interruptService_ == nullptr);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_003
* @tc.desc  : Test ChangeVolumeOnVoiceAssistant.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_003, TestSize.Level4)
{
    AudioStreamType streamInFocus = AudioStreamType::STREAM_VOICE_ASSISTANT;
    audioPolicyServer_->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioPolicyServer_->ChangeVolumeOnVoiceAssistant(streamInFocus);
    EXPECT_EQ(streamInFocus, AudioStreamType::STREAM_MUSIC);
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_004
* @tc.desc  : Test RegisterVolumeKeyEvents.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_004, TestSize.Level4)
{
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_MUTE;
    int32_t ret = audioPolicyServer_->RegisterVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_005
* @tc.desc  : Test CreateRendererClient.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_005, TestSize.Level4)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t flag = 1;
    uint32_t sessionId = 1;
    std::string networkId = "";
    audioPolicyServer_->CreateRendererClient(streamDesc, flag, sessionId, networkId);
    EXPECT_EQ(streamDesc->bundleName_, "");
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_006
* @tc.desc  : Test RegisterTracker.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_006, TestSize.Level4)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();

    int32_t modeIn = static_cast<int32_t>(AudioMode::AUDIO_MODE_PLAYBACK);
    AudioStreamChangeInfo streamChangeInfoIn;
    EXPECT_EQ(audioPolicyServer_->RegisterTracker(modeIn, streamChangeInfoIn, nullptr), ERR_INVALID_PARAM);

    modeIn = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD);
    EXPECT_EQ(audioPolicyServer_->RegisterTracker(modeIn, streamChangeInfoIn, nullptr), ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_007
* @tc.desc  : Test UpdateTracker.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_007, TestSize.Level4)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();

    int32_t modeIn = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD);
    AudioStreamChangeInfo streamChangeInfoIn;
    EXPECT_EQ(audioPolicyServer_->UpdateTracker(modeIn, streamChangeInfoIn), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_008
* @tc.desc  : Test RegisterClientDeathRecipient.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_008, TestSize.Level4)
{
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    AudioPolicyServer::DeathRecipientId id = AudioPolicyServer::DeathRecipientId::TRACKER_CLIENT;
    audioPolicyServer_->RegisterClientDeathRecipient(object, id);

    pid_t pid = IPCSkeleton::GetCallingPid();
    audioPolicyServer_->clientDiedListenerState_.push_back(pid);
    audioPolicyServer_->RegisterClientDeathRecipient(object, id);

    id = AudioPolicyServer::DeathRecipientId::LISTENER_CLIENT;
    audioPolicyServer_->RegisterClientDeathRecipient(object, id);

    audioPolicyServer_->clientDiedListenerState_.clear();
    audioPolicyServer_->RegisterClientDeathRecipient(object, id);
    EXPECT_TRUE(audioPolicyServer_->clientDiedListenerState_.empty());
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_009
* @tc.desc  : Test RegisteredStreamListenerClientDied.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_009, TestSize.Level4)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    audioPolicyServer_->audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();

    pid_t pid = 1;
    pid_t uid = 1;
    audioPolicyServer_->lastMicMuteSettingPid_ = 0;
    EXPECT_NO_THROW(audioPolicyServer_->RegisteredStreamListenerClientDied(pid, uid));

    audioPolicyServer_->interruptService_.reset();
    audioPolicyServer_->audioPolicyServerHandler_.reset();
    audioPolicyServer_->lastMicMuteSettingPid_ = pid;
    EXPECT_NO_THROW(audioPolicyServer_->RegisteredStreamListenerClientDied(pid, uid));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_010
* @tc.desc  : Test PermStateChangeCallback.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_010, TestSize.Level4)
{
    Security::AccessToken::PermStateChangeScope scopeInfo;
    sptr<AudioPolicyServer> server = new AudioPolicyServer(systemAbilityId_, runOnCreate_);
    auto callback = std::make_shared<AudioPolicyServer::PerStateChangeCbCustomizeCallback>(scopeInfo, server);
    Security::AccessToken::PermStateChangeInfo result;
    EXPECT_NO_THROW(callback->PermStateChangeCallback(result));

    bool targetMuteState = true;
    uint32_t targetTokenId = 1;
    int32_t appUid = 1;
    EXPECT_NO_THROW(callback->UpdateMicPrivacyByCapturerState(targetMuteState, targetTokenId, appUid));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_011
* @tc.desc  : Test UnsetAvailableDeviceChangeCallback.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_011, TestSize.Level4)
{
    audioPolicyServer_->audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();

    int32_t clientId = 1;
    int32_t usage = static_cast<int32_t>(AudioDeviceUsage::MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(audioPolicyServer_->UnsetAvailableDeviceChangeCallback(clientId, usage), SUCCESS);

    audioPolicyServer_->audioPolicyServerHandler_.reset();
    EXPECT_EQ(audioPolicyServer_->UnsetAvailableDeviceChangeCallback(clientId, usage), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_012
* @tc.desc  : Test SendVolumeKeyEventToRssWhenAccountsChanged.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_012, TestSize.Level4)
{
    audioPolicyServer_->audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    EXPECT_NO_THROW(audioPolicyServer_->SendVolumeKeyEventToRssWhenAccountsChanged());

    audioPolicyServer_->audioPolicyServerHandler_.reset();
    EXPECT_NO_THROW(audioPolicyServer_->SendVolumeKeyEventToRssWhenAccountsChanged());
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_013
* @tc.desc  : Test SetAudioSessionScene.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_013, TestSize.Level4)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    int32_t audioSessionScene = static_cast<int32_t>(AudioSessionScene::MEDIA);
    EXPECT_EQ(audioPolicyServer_->SetAudioSessionScene(audioSessionScene), SUCCESS);

    audioPolicyServer_->interruptService_.reset();
    EXPECT_EQ(audioPolicyServer_->SetAudioSessionScene(audioSessionScene), ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_014
* @tc.desc  : Test GetDefaultOutputDevice.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_014, TestSize.Level4)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();

    int32_t deviceType = static_cast<int32_t>(DeviceType::DEVICE_TYPE_NONE);
    EXPECT_EQ(audioPolicyServer_->GetDefaultOutputDevice(deviceType), SUCCESS);

    audioPolicyServer_->eventEntry_.reset();
    EXPECT_EQ(audioPolicyServer_->GetDefaultOutputDevice(deviceType), ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_015
* @tc.desc  : Test SetDefaultOutputDevice.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_015, TestSize.Level4)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();

    int32_t deviceType = static_cast<int32_t>(DeviceType::DEVICE_TYPE_NONE);
    EXPECT_EQ(audioPolicyServer_->SetDefaultOutputDevice(deviceType), ERR_NOT_SUPPORTED);

    audioPolicyServer_->eventEntry_.reset();
    EXPECT_EQ(audioPolicyServer_->SetDefaultOutputDevice(deviceType), ERR_UNKNOWN);

    audioPolicyServer_->interruptService_.reset();
    EXPECT_EQ(audioPolicyServer_->SetDefaultOutputDevice(deviceType), ERR_UNKNOWN);

    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();
    EXPECT_EQ(audioPolicyServer_->SetDefaultOutputDevice(deviceType), ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_016
* @tc.desc  : Test IsAllowedPlayback.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_016, TestSize.Level4)
{
    int32_t uid = 1;
    int32_t pid = 1;
    bool isAllowed = false;
    EXPECT_EQ(audioPolicyServer_->IsAllowedPlayback(uid, pid, isAllowed), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_017
* @tc.desc  : Test SetCallbackStreamUsageInfo.
*/
HWTEST_F(AudioPolicyServerUnitTest, AudioPolicyServer_017, TestSize.Level4)
{
    audioPolicyServer_->audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::set<int32_t> streamUsages = {static_cast<int32_t>(StreamUsage::STREAM_USAGE_MEDIA),
        static_cast<int32_t>(StreamUsage::STREAM_USAGE_ALARM)};
    EXPECT_EQ(audioPolicyServer_->SetCallbackStreamUsageInfo(streamUsages), SUCCESS);

    audioPolicyServer_->audioPolicyServerHandler_.reset();
    EXPECT_EQ(audioPolicyServer_->SetCallbackStreamUsageInfo(streamUsages), AUDIO_ERR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: GetFastStreamInfo_001
* @tc.desc  : Test AudioPolicyServer interfaces.
*/
HWTEST_F(AudioPolicyServerUnitTest, GetFastStreamInfo_001, TestSize.Level1)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ = audioPolicyServer_->coreService_->GetEventEntry();
    AudioStreamInfo info;
    audioPolicyServer_->audioConfigManager_.OnFastFormatParsed(AudioSampleFormat::SAMPLE_S32LE);
    audioPolicyServer_->GetFastStreamInfo(info, 0);
    ASSERT_EQ(AudioSampleFormat::SAMPLE_S32LE, info.format);
}
} // AudioStandard
} // OHOS