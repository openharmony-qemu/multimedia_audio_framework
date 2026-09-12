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

#include "audio_policy_server_methods_test.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "audio_loud_volume_manager.h"
#include "audio_volume_utils.h"
#include "audio_record_controller.h"
#include "audio_server_death_recipient.h"
#include "audio_loopback_manager.h"
#include "audio_policy_client_holder.h"
#include "audio_policy_server_handler.h"
#include "stream_flag_info.h"
#include "audio_policy_server.h"

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
    int32_t GetObjectRefCount() { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) { return 0; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return addDeathResult_; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; }
    bool addDeathResult_ = true;
    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

void AudioPolicyServerMethodsTest::SetUpTestCase(void)
{
    GetPermission();
}

void AudioPolicyServerMethodsTest::TearDownTestCase(void) {}

void AudioPolicyServerMethodsTest::SetUp(void)
{
    audioPolicyServer_ = std::make_shared<AudioPolicyServer>(
        systemAbilityId_, runOnCreate_);
}

void AudioPolicyServerMethodsTest::TearDown(void)
{
    audioPolicyServer_ = nullptr;
}

/**
 * @tc.name  : ReloadLoudVolumeMode_001
 * @tc.desc  : Test loudVolumeSupportMode_ is LOUD_VOLUME_NOT_SUPPORT
 */
HWTEST_F(AudioPolicyServerMethodsTest, ReloadLoudVolumeMode_001, TestSize.Level1)
{
    audioPolicyServer_->loudVolumeSupportMode_ = LOUD_VOLUME_NOT_SUPPORT;
    bool ret = false;
    int32_t result = audioPolicyServer_->ReloadLoudVolumeMode(0, 0, ret);
    EXPECT_EQ(result, AUDIO_INVALID_PARAM);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_003
 * @tc.desc  : Test ReloadLoudVolumeMode returns AUDIO_INVALID_PARAM when fails
 */
HWTEST_F(AudioPolicyServerMethodsTest, ReloadLoudVolumeMode_003, TestSize.Level1)
{
    audioPolicyServer_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    bool ret = false;
    int32_t result = audioPolicyServer_->ReloadLoudVolumeMode(0, 0, ret);
    EXPECT_EQ(result, AUDIO_INVALID_PARAM);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : SubscribeVolumeKeyEventsRetryFunc_001
 * @tc.desc  : Test empty failedVolumeKeyEvents_ early return
 */
#ifdef FEATURE_MULTIMODALINPUT_INPUT
HWTEST_F(AudioPolicyServerMethodsTest, SubscribeVolumeKeyEventsRetryFunc_001, TestSize.Level1)
{
    audioPolicyServer_->failedVolumeKeyEvents_.clear();
    audioPolicyServer_->hasSubscribedVolumeKeyEvents_.store(false);
    audioPolicyServer_->SubscribeVolumeKeyEventsRetryFunc();
    EXPECT_TRUE(audioPolicyServer_->hasSubscribedVolumeKeyEvents_.load());
}
#endif

/**
 * @tc.name  : RegisterVoipMuteDeathRecipient_001
 * @tc.desc  : Test callerObj is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterVoipMuteDeathRecipient_001, TestSize.Level1)
{
    int32_t callerUid = 100;
    EXPECT_NO_THROW(audioPolicyServer_->RegisterVoipMuteDeathRecipient(callerUid, nullptr));
}

/**
 * @tc.name  : RegisterVoipMuteDeathRecipient_003
 * @tc.desc  : Test AddDeathRecipient fails
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterVoipMuteDeathRecipient_003, TestSize.Level1)
{
    sptr<RemoteObjectTestStub> obj = new RemoteObjectTestStub();
    obj->addDeathResult_ = false;
    int32_t callerUid = 300;
    audioPolicyServer_->voipMuteDeathRecipients_.clear();
    EXPECT_NO_THROW(audioPolicyServer_->RegisterVoipMuteDeathRecipient(callerUid, obj));
    EXPECT_EQ(audioPolicyServer_->voipMuteDeathRecipients_.size(), 0u);
}

/**
 * @tc.name  : RegisterVoipMuteDeathRecipient_004
 * @tc.desc  : Test successful registration
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterVoipMuteDeathRecipient_004, TestSize.Level1)
{
    sptr<RemoteObjectTestStub> obj = new RemoteObjectTestStub();
    obj->addDeathResult_ = true;
    int32_t callerUid = 400;
    audioPolicyServer_->voipMuteDeathRecipients_.clear();
    EXPECT_NO_THROW(audioPolicyServer_->RegisterVoipMuteDeathRecipient(callerUid, obj));
    EXPECT_EQ(audioPolicyServer_->voipMuteDeathRecipients_.size(), 1u);
    audioPolicyServer_->voipMuteDeathRecipients_.clear();
}

/**
 * @tc.name  : OnVoipMuteCallerDied_002
 * @tc.desc  : Test callerUid not found in map
 */
HWTEST_F(AudioPolicyServerMethodsTest, OnVoipMuteCallerDied_002, TestSize.Level1)
{
    int32_t callerUid = 600;
    audioPolicyServer_->voipMuteDeathRecipients_.clear();
    EXPECT_NO_THROW(audioPolicyServer_->OnVoipMuteCallerDied(callerUid));
    EXPECT_EQ(audioPolicyServer_->voipMuteDeathRecipients_.size(), 0u);
}

/**
 * @tc.name  : SetVolumeControlPriority_001
 * @tc.desc  : Test no system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetVolumeControlPriority_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    std::vector<int32_t> priorities = {0};
    int32_t result = 0;
    int32_t ret = audioPolicyServer_->SetVolumeControlPriority(priorities, result);
    setuid(selfUid);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : SetVolumeControlPriority_002
 * @tc.desc  : Test invalid volume type
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetVolumeControlPriority_002, TestSize.Level1)
{
    std::vector<int32_t> priorities = {9999};
    int32_t result = 0;
    int32_t ret = audioPolicyServer_->SetVolumeControlPriority(priorities, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : SetVolumeControlPriority_003
 * @tc.desc  : Test duplicate volume type
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetVolumeControlPriority_003, TestSize.Level1)
{
    int32_t volType = static_cast<int32_t>(STREAM_MUSIC);
    std::vector<int32_t> priorities = {volType, volType};
    int32_t result = 0;
    int32_t ret = audioPolicyServer_->SetVolumeControlPriority(priorities, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : SetVolumeControlPriority_004
 * @tc.desc  : Test interruptService_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetVolumeControlPriority_004, TestSize.Level1)
{
    audioPolicyServer_->interruptService_.reset();
    std::vector<int32_t> priorities;
    int32_t result = 0;
    int32_t ret = audioPolicyServer_->SetVolumeControlPriority(priorities, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : SetVolumeControlPriority_005
 * @tc.desc  : Test successful path
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetVolumeControlPriority_005, TestSize.Level1)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    std::vector<int32_t> priorities;
    int32_t result = 0;
    int32_t ret = audioPolicyServer_->SetVolumeControlPriority(priorities, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : IsStreamActiveByStreamUsage_001
 * @tc.desc  : Test invalid stream usage
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsStreamActiveByStreamUsage_001, TestSize.Level1)
{
    bool active = false;
    int32_t ret = audioPolicyServer_->IsStreamActiveByStreamUsage(-1, active);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : IsStreamActiveByStreamUsage_002
 * @tc.desc  : Test stream usage exceeds STREAM_USAGE_MAX
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsStreamActiveByStreamUsage_002, TestSize.Level1)
{
    bool active = false;
    int32_t ret = audioPolicyServer_->IsStreamActiveByStreamUsage(
        static_cast<int32_t>(STREAM_USAGE_MAX) + 1, active);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : IsStreamActiveByStreamUsage_003
 * @tc.desc  : Test valid stream usage
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsStreamActiveByStreamUsage_003, TestSize.Level1)
{
    bool active = false;
    int32_t ret = audioPolicyServer_->IsStreamActiveByStreamUsage(
        static_cast<int32_t>(STREAM_USAGE_MEDIA), active);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetMicrophoneMuteCommon_001
 * @tc.desc  : Test SetMicrophoneMuteCommon with no state change
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMuteCommon_001, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_ =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    bool currentMute = false;
    audioPolicyServer_->IsMicrophoneMute(currentMute);
    int32_t ret = audioPolicyServer_->SetMicrophoneMuteCommon(currentMute, true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : SetMicrophoneMuteCommon_002
 * @tc.desc  : Test SetMicrophoneMuteCommon with handler nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMuteCommon_002, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_.reset();
    int32_t ret = audioPolicyServer_->SetMicrophoneMuteCommon(true, false);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : SetMicrophoneMuteCommon_003
 * @tc.desc  : Test SetMicrophoneMuteCommon with state change and handler present
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMuteCommon_003, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_ =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    bool currentMute = false;
    audioPolicyServer_->IsMicrophoneMute(currentMute);
    bool targetMute = !currentMute;
    int32_t ret = audioPolicyServer_->SetMicrophoneMuteCommon(targetMute, false);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : SetMicrophoneMutePersistent_001
 * @tc.desc  : Test no MICROPHONE_CONTROL_PERMISSION
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMutePersistent_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t ret = audioPolicyServer_->SetMicrophoneMutePersistent(true, 0);
    setuid(selfUid);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : SetMicrophoneMutePersistent_002
 * @tc.desc  : Test invalid policy type
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMutePersistent_002, TestSize.Level1)
{
    int32_t ret = audioPolicyServer_->SetMicrophoneMutePersistent(true, 9999);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : SetMicrophoneMutePersistent_003
 * @tc.desc  : Test valid policy type EDM_POLICY_TYPE
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMutePersistent_003, TestSize.Level1)
{
    int32_t typeIn = static_cast<int32_t>(PolicyType::EDM_POLICY_TYPE);
    int32_t ret = audioPolicyServer_->SetMicrophoneMutePersistent(true, typeIn);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : SetMicrophoneMutePersistent_004
 * @tc.desc  : Test valid policy type PRIVACY_POLCIY_TYPE
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMutePersistent_004, TestSize.Level1)
{
    int32_t typeIn = static_cast<int32_t>(PolicyType::PRIVACY_POLCIY_TYPE);
    int32_t ret = audioPolicyServer_->SetMicrophoneMutePersistent(false, typeIn);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : SetMicrophoneMutePersistent_005
 * @tc.desc  : Test valid policy type TEMPORARY_POLCIY_TYPE
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetMicrophoneMutePersistent_005, TestSize.Level1)
{
    int32_t typeIn = static_cast<int32_t>(PolicyType::TEMPORARY_POLCIY_TYPE);
    int32_t ret = audioPolicyServer_->SetMicrophoneMutePersistent(false, typeIn);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : ActivatePreemptMode_001
 * @tc.desc  : Test callingUid != PREEMPT_UID
 */
HWTEST_F(AudioPolicyServerMethodsTest, ActivatePreemptMode_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t ret = audioPolicyServer_->ActivatePreemptMode();
    setuid(selfUid);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : ActivatePreemptMode_002
 * @tc.desc  : Test interruptService_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, ActivatePreemptMode_002, TestSize.Level1)
{
    audioPolicyServer_->interruptService_.reset();
    int32_t selfUid = getuid();
    setuid(AudioPolicyServer::PREEMPT_UID);
    int32_t ret = audioPolicyServer_->ActivatePreemptMode();
    setuid(selfUid);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
 * @tc.name  : ActivatePreemptMode_003
 * @tc.desc  : Test successful path with valid uid and interruptService
 */
HWTEST_F(AudioPolicyServerMethodsTest, ActivatePreemptMode_003, TestSize.Level1)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    int32_t selfUid = getuid();
    setuid(AudioPolicyServer::PREEMPT_UID);
    int32_t ret = audioPolicyServer_->ActivatePreemptMode();
    setuid(selfUid);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : ActivateAudioInterrupt_001
 * @tc.desc  : Test ValidateAudioInterrupt fails
 */
HWTEST_F(AudioPolicyServerMethodsTest, ActivateAudioInterrupt_001, TestSize.Level1)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = 0;
    audioInterrupt.uid = 0;
    int32_t ret = audioPolicyServer_->ActivateAudioInterrupt(
        audioInterrupt, 0, false);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SetRegisterInterruptCallbackFlag_001
 * @tc.desc  : Test interruptService_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetRegisterInterruptCallbackFlag_001, TestSize.Level1)
{
    audioPolicyServer_->interruptService_.reset();
    int32_t ret = audioPolicyServer_->SetRegisterInterruptCallbackFlag(1, true);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
 * @tc.name  : SetRegisterInterruptCallbackFlag_002
 * @tc.desc  : Test successful path
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetRegisterInterruptCallbackFlag_002, TestSize.Level1)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    int32_t ret = audioPolicyServer_->SetRegisterInterruptCallbackFlag(1, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetRegisterStateChangeCallbackFlag_001
 * @tc.desc  : Test interruptService_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetRegisterStateChangeCallbackFlag_001, TestSize.Level1)
{
    audioPolicyServer_->interruptService_.reset();
    int32_t ret = audioPolicyServer_->SetRegisterStateChangeCallbackFlag(true);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
 * @tc.name  : SetRegisterStateChangeCallbackFlag_002
 * @tc.desc  : Test successful path
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetRegisterStateChangeCallbackFlag_002, TestSize.Level1)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    int32_t ret = audioPolicyServer_->SetRegisterStateChangeCallbackFlag(true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : GetCurrentCapturerChangeInfos_002
 * @tc.desc  : Test with nullptr element in capturerInfos
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetCurrentCapturerChangeInfos_002, TestSize.Level1)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ =
        audioPolicyServer_->coreService_->GetEventEntry();
    std::vector<shared_ptr<AudioCapturerChangeInfo>> infos;
    infos.push_back(nullptr);
    int32_t ret = audioPolicyServer_->GetCurrentCapturerChangeInfos(infos);
    EXPECT_EQ(ret, ERR_MEMORY_ALLOC_FAILED);
}

/**
 * @tc.name  : GetAudioDebugInfo_001
 * @tc.desc  : Test eventEntry_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetAudioDebugInfo_001, TestSize.Level1)
{
    audioPolicyServer_->eventEntry_.reset();
    std::shared_ptr<AudioDebugInfo> debugInfo;
    int32_t ret = audioPolicyServer_->GetAudioDebugInfo(1, debugInfo);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : GetAudioDebugInfo_002
 * @tc.desc  : Test no permission and stream not belong to uid
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetAudioDebugInfo_002, TestSize.Level1)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ =
        audioPolicyServer_->coreService_->GetEventEntry();
    int32_t selfUid = getuid();
    setuid(9999);
    std::shared_ptr<AudioDebugInfo> debugInfo = std::make_shared<AudioDebugInfo>();
    int32_t ret = audioPolicyServer_->GetAudioDebugInfo(0, debugInfo);
    setuid(selfUid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : GetAudioDebugInfo_003
 * @tc.desc  : Test audioDebugInfo nullptr creates new object
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetAudioDebugInfo_003, TestSize.Level1)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ =
        audioPolicyServer_->coreService_->GetEventEntry();
    std::shared_ptr<AudioDebugInfo> debugInfo;
    int32_t ret = audioPolicyServer_->GetAudioDebugInfo(0, debugInfo);
    EXPECT_NE(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : GetAudioDebugInfo_004
 * @tc.desc  : Test interruptService_ is nullptr skips focus debug info
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetAudioDebugInfo_004, TestSize.Level1)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ =
        audioPolicyServer_->coreService_->GetEventEntry();
    audioPolicyServer_->interruptService_.reset();
    std::shared_ptr<AudioDebugInfo> debugInfo = std::make_shared<AudioDebugInfo>();
    int32_t ret = audioPolicyServer_->GetAudioDebugInfo(0, debugInfo);
    EXPECT_NE(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : GetAudioDebugInfo_005
 * @tc.desc  : Test interruptService_ not nullptr gets focus debug info
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetAudioDebugInfo_005, TestSize.Level1)
{
    audioPolicyServer_->coreService_ = AudioCoreService::GetCoreService();
    audioPolicyServer_->coreService_->Init();
    audioPolicyServer_->eventEntry_ =
        audioPolicyServer_->coreService_->GetEventEntry();
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioDebugInfo> debugInfo = std::make_shared<AudioDebugInfo>();
    int32_t ret = audioPolicyServer_->GetAudioDebugInfo(0, debugInfo);
    EXPECT_NE(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : SetA2dpDeviceVolume_001
 * @tc.desc  : Test callerUid != UID_BLUETOOTH_SA
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetA2dpDeviceVolume_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t ret = audioPolicyServer_->SetA2dpDeviceVolume("00:00:00:00", 5, true);
    setuid(selfUid);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SetNearlinkDeviceVolume_001
 * @tc.desc  : Test uid check fails
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetNearlinkDeviceVolume_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t ret = audioPolicyServer_->SetNearlinkDeviceVolume(
        "00:00:00:00", static_cast<int32_t>(STREAM_MUSIC), 5, true);
    setuid(selfUid);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : RegisterAppStateObserver_001
 * @tc.desc  : Test appStateObserver_ already registered
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterAppStateObserver_001, TestSize.Level1)
{
    audioPolicyServer_->appStateObserver_ =
        sptr<AppStateObserver>(new AppStateObserver());
    EXPECT_NO_THROW(audioPolicyServer_->RegisterAppStateObserver());
}

/**
 * @tc.name  : RegisterAppStateObserver_002
 * @tc.desc  : Test register failure resets appStateObserver_
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterAppStateObserver_002, TestSize.Level1)
{
    audioPolicyServer_->appStateObserver_ = nullptr;
    EXPECT_NO_THROW(audioPolicyServer_->RegisterAppStateObserver());
}

/**
 * @tc.name  : RegisterPolicyCallbackClient_001
 * @tc.desc  : Test object is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterPolicyCallbackClient_001, TestSize.Level1)
{
    int32_t ret = audioPolicyServer_->RegisterPolicyCallbackClient(nullptr, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : RegisterPolicyCallbackClient_002
 * @tc.desc  : Test handler is nullptr skips AddAudioPolicyClientProxyMap
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterPolicyCallbackClient_002, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_.reset();
    sptr<IRemoteObject> obj = new RemoteObjectTestStub();
    int32_t ret = audioPolicyServer_->RegisterPolicyCallbackClient(obj, 0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RegisterPolicyCallbackClient_003
 * @tc.desc  : Test successful registration with handler
 */
HWTEST_F(AudioPolicyServerMethodsTest, RegisterPolicyCallbackClient_003, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_ =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    sptr<IRemoteObject> obj = new RemoteObjectTestStub();
    int32_t ret = audioPolicyServer_->RegisterPolicyCallbackClient(obj, 0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : IsAudioLoopbackSupported_001
 * @tc.desc  : Test AudioServerProxy returns not supported
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsAudioLoopbackSupported_001, TestSize.Level1)
{
    bool ret = true;
    int32_t result = audioPolicyServer_->IsAudioLoopbackSupported(
        static_cast<int32_t>(LOOPBACK_HARDWARE),
        static_cast<int32_t>(DEVICE_TYPE_SPEAKER), ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : IsCollaborativePlaybackSupportedForDevice_001
 * @tc.desc  : Test no system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsCollaborativePlaybackSupportedForDevice_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    bool isSupported = true;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = audioPolicyServer_->IsCollaborativePlaybackSupportedForDevice(
        device, isSupported);
    setuid(selfUid);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_FALSE(isSupported);
}

/**
 * @tc.name  : IsCollaborativePlaybackSupportedForDevice_002
 * @tc.desc  : Test with system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsCollaborativePlaybackSupportedForDevice_002, TestSize.Level1)
{
    bool isSupported = false;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = audioPolicyServer_->IsCollaborativePlaybackSupportedForDevice(
        device, isSupported);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetActiveHfpDevice_001
 * @tc.desc  : Test no system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetActiveHfpDevice_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t ret = audioPolicyServer_->SetActiveHfpDevice("00:00:00:00");
    setuid(selfUid);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SetActiveHfpDevice_002
 * @tc.desc  : Test with system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetActiveHfpDevice_002, TestSize.Level1)
{
    int32_t ret = audioPolicyServer_->SetActiveHfpDevice("00:00:00:00");
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : SetAudioSessionBehavior_001
 * @tc.desc  : Test interruptService_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetAudioSessionBehavior_001, TestSize.Level1)
{
    audioPolicyServer_->interruptService_.reset();
    int32_t ret = audioPolicyServer_->SetAudioSessionBehavior(0);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
 * @tc.name  : SetAudioSessionBehavior_002
 * @tc.desc  : Test with interruptService_ present
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetAudioSessionBehavior_002, TestSize.Level1)
{
    audioPolicyServer_->interruptService_ = std::make_shared<AudioInterruptService>();
    int32_t ret = audioPolicyServer_->SetAudioSessionBehavior(0);
    EXPECT_NE(ret, ERR_UNKNOWN);
}

/**
 * @tc.name  : IsAudioSeparationEffectSupported_001
 * @tc.desc  : Test no system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsAudioSeparationEffectSupported_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    bool ret = true;
    int32_t result = audioPolicyServer_->IsAudioSeparationEffectSupported(ret);
    setuid(selfUid);
    EXPECT_NE(result, ERR_SYSTEM_PERMISSION_DENIED);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : IsAudioSeparationEffectSupported_002
 * @tc.desc  : Test with system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, IsAudioSeparationEffectSupported_002, TestSize.Level1)
{
    bool ret = false;
    int32_t result = audioPolicyServer_->IsAudioSeparationEffectSupported(ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : SetAudioSeparationEffectEnabled_001
 * @tc.desc  : Test no MANAGE_SYSTEM_AUDIO_EFFECTS permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetAudioSeparationEffectEnabled_001, TestSize.Level1)
{
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t ret = audioPolicyServer_->SetAudioSeparationEffectEnabled(true, 0, 0);
    setuid(selfUid);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SetAudioSeparationEffectEnabled_003
 * @tc.desc  : Test handler is nullptr no change event
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetAudioSeparationEffectEnabled_003, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_.reset();
    int32_t ret = audioPolicyServer_->SetAudioSeparationEffectEnabled(true, 0, 0);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

/**
 * @tc.name  : SetAudioSeparationEffectEnabled_004
 * @tc.desc  : Test with handler present
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetAudioSeparationEffectEnabled_004, TestSize.Level1)
{
    audioPolicyServer_->audioPolicyServerHandler_ =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    int32_t ret = audioPolicyServer_->SetAudioSeparationEffectEnabled(true, 0, 0);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

/**
 * @tc.name  : CreateLoopback_001
 * @tc.desc  : Test loopbackManager_ is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, CreateLoopback_001, TestSize.Level1)
{
    audioPolicyServer_->loopbackManager_.reset();
    sptr<IRemoteObject> cbObject = new RemoteObjectTestStub();
    sptr<IRemoteObject> loopback;
    int32_t ret = 0;
    int32_t result = audioPolicyServer_->CreateLoopback(0, 0, cbObject, loopback, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : CreateLoopback_002
 * @tc.desc  : Test cbObject is nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, CreateLoopback_002, TestSize.Level1)
{
    audioPolicyServer_->loopbackManager_ =
        std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> loopback;
    int32_t ret = 0;
    int32_t result = audioPolicyServer_->CreateLoopback(0, 0, nullptr, loopback, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : CreateLoopback_003
 * @tc.desc  : Test LOOPBACK_TYPE_NORMAL skips permission check
 */
HWTEST_F(AudioPolicyServerMethodsTest, CreateLoopback_003, TestSize.Level1)
{
    audioPolicyServer_->loopbackManager_ =
        std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> cbObject = new RemoteObjectTestStub();
    sptr<IRemoteObject> loopback;
    int32_t ret = 0;
    int32_t type = static_cast<int32_t>(LOOPBACK_TYPE_NORMAL);
    int32_t result = audioPolicyServer_->CreateLoopback(0, type, cbObject, loopback, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : CreateLoopback_004
 * @tc.desc  : Test non-NORMAL type without system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, CreateLoopback_004, TestSize.Level1)
{
    audioPolicyServer_->loopbackManager_ =
        std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> cbObject = new RemoteObjectTestStub();
    sptr<IRemoteObject> loopback;
    int32_t ret = 0;
    int32_t type = static_cast<int32_t>(LOOPBACK_TYPE_GLOBAL_HANDLER);
    int32_t selfUid = getuid();
    setuid(9999);
    int32_t result = audioPolicyServer_->CreateLoopback(0, type, cbObject, loopback, ret);
    setuid(selfUid);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : CreateLoopback_005
 * @tc.desc  : Test non-NORMAL type with system permission
 */
HWTEST_F(AudioPolicyServerMethodsTest, CreateLoopback_005, TestSize.Level1)
{
    audioPolicyServer_->loopbackManager_ =
        std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> cbObject = new RemoteObjectTestStub();
    sptr<IRemoteObject> loopback;
    int32_t ret = 0;
    int32_t type = static_cast<int32_t>(LOOPBACK_TYPE_GLOBAL_HANDLER);
    int32_t result = audioPolicyServer_->CreateLoopback(0, type, cbObject, loopback, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : EnableSystemRecordController_001
 * @tc.desc  : Test show=true with object nullptr
 */
HWTEST_F(AudioPolicyServerMethodsTest, EnableSystemRecordController_001, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    int32_t ret = audioPolicyServer_->EnableSystemRecordController(true, config, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : EnableSystemRecordController_002
 * @tc.desc  : Test show=true with valid object
 */
HWTEST_F(AudioPolicyServerMethodsTest, EnableSystemRecordController_002, TestSize.Level1)
{
    sptr<IRemoteObject> obj = new RemoteObjectTestStub();
    SystemRecordControllerConfig config;
    int32_t ret = audioPolicyServer_->EnableSystemRecordController(true, config, obj);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : GetSystemVolumeLevel_001
 * @tc.desc  : zoneId valid, skip FindAudioZoneByUid.
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetSystemVolumeLevel_001, TestSize.Level1)
{
    int32_t level = -1;
    audioPolicyServer_->GetSystemVolumeLevel(STREAM_MUSIC, 0, level, 0);
    EXPECT_GE(level, 0);
}

/**
 * @tc.name  : GetSystemVolumeLevel_002
 * @tc.desc  : zoneId invalid, uid!=0, go uid reverse lookup.
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetSystemVolumeLevel_002, TestSize.Level1)
{
    int32_t level = -1;
    audioPolicyServer_->GetSystemVolumeLevel(STREAM_MUSIC, 1000, level, INVALID_ZONEID);
    EXPECT_GE(level, 0);
}

/**
 * @tc.name  : SetSystemVolumeDegree_001
 * @tc.desc  : zoneId valid, skip FindAudioZoneByUid.
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetSystemVolumeDegree_001, TestSize.Level1)
{
    int32_t ret = audioPolicyServer_->SetSystemVolumeDegree(STREAM_MUSIC, 30, 0, 0, 0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetSystemVolumeDegree_002
 * @tc.desc  : zoneId invalid, uid!=0, go uid reverse lookup.
 */
HWTEST_F(AudioPolicyServerMethodsTest, SetSystemVolumeDegree_002, TestSize.Level1)
{
    int32_t ret = audioPolicyServer_->SetSystemVolumeDegree(STREAM_MUSIC, 30, 0, 1000, INVALID_ZONEID);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : GetSystemVolumeDegree_001
 * @tc.desc  : zoneId valid, skip FindAudioZoneByUid.
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetSystemVolumeDegree_001, TestSize.Level1)
{
    int32_t degree = -1;
    audioPolicyServer_->GetSystemVolumeDegree(STREAM_MUSIC, 0, degree, 0);
    EXPECT_GE(degree, 0);
}

/**
 * @tc.name  : GetSystemVolumeDegree_002
 * @tc.desc  : zoneId invalid, uid!=0, go uid reverse lookup.
 */
HWTEST_F(AudioPolicyServerMethodsTest, GetSystemVolumeDegree_002, TestSize.Level1)
{
    int32_t degree = -1;
    audioPolicyServer_->GetSystemVolumeDegree(STREAM_MUSIC, 1000, degree, INVALID_ZONEID);
    EXPECT_GE(degree, 0);
}
} // namespace AudioStandard
} // namespace OHOS