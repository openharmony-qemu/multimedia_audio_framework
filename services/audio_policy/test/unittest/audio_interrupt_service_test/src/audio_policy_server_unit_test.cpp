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

#include "audio_policy_server_unit_test.h"
#include "audio_policy_server.h"
#include "audio_interrupt_unit_test.h"
#include "audio_info.h"
#include "securec.h"
#include "audio_interrupt_service.h"
#include "audio_device_descriptor.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "audio_bundle_manager.h"
#include "audio_zone_info.h"
#include "audio_unit_test.h"
#include "audio_zone_service.h"
#include "audio_connected_device.h"
#include "audio_policy_server_handler.h"
#include "audio_stream_collector.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermission = false;
bool g_hasServerInit = false;
constexpr int32_t DEFAULT_ZONE_ID = 0;
constexpr int32_t CHANNEL_STEREO = 2;

// Helper functions for GetCurrentRendererChangeInfosForZone tests
std::shared_ptr<AudioRendererChangeInfo> CreateRendererChangeInfo(
    int32_t uid, int32_t sessionId, RendererState state, StreamUsage usage)
{
    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->clientUID = uid;
    info->createrUID = uid;
    info->sessionId = sessionId;
    info->rendererState = state;
    info->rendererInfo.streamUsage = usage;
    info->channelCount = CHANNEL_STEREO;
    return info;
}

void AddRenderersToCollector(const std::vector<std::shared_ptr<AudioRendererChangeInfo>>& renderers)
{
    AudioStreamCollector& collector = AudioStreamCollector::GetAudioStreamCollector();
    std::lock_guard<std::mutex> lock(collector.streamsInfoMutex_);
    for (const auto& renderer : renderers) {
        collector.audioRendererChangeInfos_.push_back(renderer);
    }
}

void ClearRenderersFromCollector()
{
    AudioStreamCollector& collector = AudioStreamCollector::GetAudioStreamCollector();
    std::lock_guard<std::mutex> lock(collector.streamsInfoMutex_);
    collector.audioRendererChangeInfos_.clear();
}

void ResetGetActiveAudioInterruptZoneTestEnv()
{
    AudioConnectedDevice::GetInstance().DelConnectedDevice("RemoteDevice", DEVICE_TYPE_SPEAKER);
    AudioZoneService::GetInstance().DeInit();
}

int32_t PrepareActiveZone(AudioStreamType streamType, bool hasVolumeControllableDevice)
{
    auto interruptService = std::make_shared<AudioInterruptService>();
    ResetGetActiveAudioInterruptZoneTestEnv();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(), interruptService);

    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("GetActiveAudioInterruptZoneTest", context, 0);
    if (hasVolumeControllableDevice) {
        auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
        device->networkId_ = "RemoteDevice";
        device->volumeBehavior_.isVolumeControlDisabled = false;
        AudioConnectedDevice::GetInstance().AddConnectedDevice(device);
        AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId, {device});
    }

    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.streamType = streamType;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.audioFocusType.isPlay = true;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt, AudioFocuState::ACTIVE);
    interruptService->zonesMap_[zoneId] = audioInterruptZone;
    return zoneId;
}

sptr<AudioPolicyServer> GetPolicyServerUnitTest()
{
    static int32_t systemAbilityId = 3009;
    static bool runOnCreate = false;
    static sptr<AudioPolicyServer> server =
        sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    if (!g_hasServerInit) {
        IdHandler::GetInstance();
        HdiAdapterManager::GetInstance();
        HPAE::IHpaeManager::GetHpaeManager().Init();
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
}

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

void ReleaseServer()
{
    GetPolicyServerUnitTest()->OnStop();
    g_hasServerInit = false;
}

void AudioPolicyUnitTest::SetUpTestCase(void) {}
void AudioPolicyUnitTest::TearDownTestCase(void)
{
    ReleaseServer();
}

void AudioPolicyUnitTest::SetUp(void)
{
    GetPermission();
}

void AudioPolicyUnitTest::TearDown(void) {}

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

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test SetNearlinkDeviceVolume.
* @tc.number: SetNearlinkDeviceVolume_002
* @tc.desc  : Test SetNearlinkDeviceVolume
*/
HWTEST(AudioPolicyUnitTest, SetNearlinkDeviceVolume_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    std::string macAddress = "LocalDevice";
    int32_t streamTypeIn = 1;
    int32_t volume = 0;
    bool updateUi = true;

    int32_t ret = ptrAudioPolicyServer->SetNearlinkDeviceVolume(macAddress, streamTypeIn, volume, updateUi);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    streamTypeIn = 0;
    ret = ptrAudioPolicyServer->SetNearlinkDeviceVolume(macAddress, streamTypeIn, volume, updateUi);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetAudioInterruptCallback_002
* @tc.desc  : Test AudioPolicyServer::SetAudioInterruptCallback
*/
HWTEST(AudioPolicyUnitTest, SetAudioInterruptCallback_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    uint32_t sessionID = 0;
    sptr<IRemoteObject> object = nullptr;
    uint32_t clientUid = 0;
    int32_t zoneID = 0;

    ptrAudioPolicyServer->interruptService_ = nullptr;
    ptrAudioPolicyServer->coreService_ = nullptr;
    int32_t result = ptrAudioPolicyServer->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);

    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->coreService_ = nullptr;
    result = ptrAudioPolicyServer->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);

    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->coreService_ = std::make_shared<AudioCoreService>();
    result = ptrAudioPolicyServer->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: GetDmDeviceType_001
* @tc.desc  : Test AudioPolicyServer::GetDmDeviceType
*/
HWTEST(AudioPolicyUnitTest, GetDmDeviceType_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    uint16_t deviceType = 0;
    int32_t result = ptrAudioPolicyServer->GetDmDeviceType(deviceType);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetSystemVolumeLevelWithDeviceInternal_001
* @tc.desc  : Test AudioPolicyServer::SetSystemVolumeLevelWithDeviceInternal
*/
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevelWithDeviceInternal_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 20;
    bool isUpdateUi = false;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;

    int32_t result = ptrAudioPolicyServer->SetSystemVolumeLevelWithDeviceInternal(
        streamType, volumeLevel, isUpdateUi, deviceType);

    auto &audioVolumeManager = ptrAudioPolicyServer->audioVolumeManager_;
    EXPECT_EQ(result, audioVolumeManager.SetSingleStreamVolumeWithDevice(
        streamType, volumeLevel, isUpdateUi, deviceType));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: GetSystemVolumeLevel_001
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevel
*/
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_001, TestSize.Level1)
{
    int32_t streamType = 1;
    int32_t uid = 0;
    int32_t volumeLevel = 0;

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    int32_t res = ptrAudioPolicyServer->GetSystemVolumeLevel(streamType, uid, volumeLevel);
    EXPECT_EQ(res, SUCCESS);

    uid = -1;
    res = ptrAudioPolicyServer->GetSystemVolumeLevel(streamType, uid, volumeLevel);
    EXPECT_EQ(res, SUCCESS);

    uid = 1;
    res = ptrAudioPolicyServer->GetSystemVolumeLevel(streamType, uid, volumeLevel);
    EXPECT_EQ(res, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetStreamMute_001
* @tc.desc  : Test AudioPolicyServer::SetStreamMute
*/
HWTEST(AudioPolicyUnitTest, SetStreamMute_001, TestSize.Level1)
{
    int32_t streamTypeIn = 1;
    bool mute = false;
    int32_t deviceTypeIn = 1;

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    int32_t res = ptrAudioPolicyServer->SetStreamMute(streamTypeIn, mute, deviceTypeIn);
    EXPECT_EQ(res, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: OnReceiveEvent_001
* @tc.desc  : Test AudioPolicyServer::OnReceiveEvent
*/
HWTEST(AudioPolicyUnitTest, OnReceiveEvent_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);

    EventFwk::CommonEventData eventData;
    OHOS::EventFwk::Want want;
    want.SetAction("usual.event.dms.cast_plugged_changed");
    eventData.SetWant(want);
    eventData.SetData("0");
    audioPolicyServer->OnReceiveEvent(eventData);
    EXPECT_EQ(eventData.GetData(), "0");

    eventData.SetData("1");
    audioPolicyServer->OnReceiveEvent(eventData);
    EXPECT_EQ(eventData.GetData(), "1");
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_001
* @tc.desc  : Test CheckAudioSessionStrategy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_001, TestSize.Level1)
{
    AudioSessionStrategy strategy;
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
    strategy.concurrencyMode = AudioConcurrencyMode::DUCK_OTHERS;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
    strategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_002
* @tc.desc  : Test CheckAudioSessionStrategy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_002, TestSize.Level1)
{
    AudioSessionStrategy strategy;
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    strategy.concurrencyMode = static_cast<AudioConcurrencyMode>(999); // Invalid mode
    EXPECT_FALSE(server->CheckAudioSessionStrategy(strategy));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_003
* @tc.desc  : Test SetAudioManagerInterruptCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_003, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    server->interruptService_ = nullptr;
    int32_t result = server->SetAudioManagerInterruptCallback(0, sptr<RemoteObjectTestStub>::MakeSptr());
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_004
* @tc.desc  : Test UnsetAudioManagerInterruptCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_004, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    server->interruptService_ = nullptr;
    int32_t result = server->UnsetAudioManagerInterruptCallback(0);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_005
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_005, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = static_cast<AudioParamKey>(100);
    std::string condition;
    std::string value;
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_006
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_006, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = PARAM_KEY_STATE;
    std::string condition;
    std::string value;
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_008
* @tc.desc  : Test ArgInfoDump.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_008, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"invalidParam");
    server->ArgInfoDump(dumpString, argQue);
    EXPECT_EQ(dumpString.find("Please input correct param:\n"), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_009
* @tc.desc  : Test DeactivateAudioInterrupt.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_009, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = getpid();
    audioInterrupt.uid = getuid();
    int32_t zoneID = 456;

    int32_t result = server->DeactivateAudioInterrupt(audioInterrupt, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    result = server->DeactivateAudioInterrupt(audioInterrupt, zoneID);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_010
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_010, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = VOLUME;
    std::string condition;
    std::string value;

    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_011
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_011, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = INTERRUPT;
    std::string condition;
    std::string value;

    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_012
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_012, TestSize.Level1)
{
    int32_t strategy = 0;
    auto policyServerTest = GetPolicyServerUnitTest();
    EXPECT_EQ(policyServerTest->ActivateAudioSession(strategy), SUCCESS);
    EXPECT_EQ(policyServerTest->DeactivateAudioSession(), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_013
* @tc.desc  : Test GetStreamInFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_013, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t zoneID = 456;
    int32_t result = 0;
    policyServerTest->GetStreamInFocus(zoneID, result);
    EXPECT_EQ(result, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_014
* @tc.desc  : Test OnRemoveSystemAbility.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_014, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = AVSESSION_SERVICE_ID;
    std::string deviceId = "132456";
    policyServerTest->OnRemoveSystemAbility(systemAbilityId, deviceId);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_015
* @tc.desc  : Test InitMicrophoneMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_015, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    policyServerTest->isInitMuteState_ = true;
    policyServerTest->InitMicrophoneMute();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_016
* @tc.desc  : Test ActivateAudioInterrupt.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_016, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    AudioInterrupt audioInterrupt;
    int32_t zoneID = 456;
    int32_t result = 0;
    policyServerTest->ActivateAudioInterrupt(audioInterrupt, zoneID, result);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioPolicyServer_016A
* @tc.number: AudioPolicyServer_016A
* @tc.desc  : Test ActivateAudioInterrupt updates descriptor voipNoPrivacyFlag.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_016A, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    ASSERT_NE(policyServerTest, nullptr);
    policyServerTest->coreService_ = AudioCoreService::GetCoreService();
    policyServerTest->eventEntry_ =
        std::make_shared<AudioCoreService::EventEntry>(policyServerTest->coreService_);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 816001;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamDesc->GetSessionId();
    audioInterrupt.uid = IPCSkeleton::GetCallingUid();
    audioInterrupt.pid = IPCSkeleton::GetCallingPid();
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    audioInterrupt.audioFocusType.isPlay = false;
    audioInterrupt.behaviorFlags_.voipNoPrivacyFlag = true;

    int32_t ret = policyServerTest->ActivateAudioInterrupt(audioInterrupt, DEFAULT_ZONE_ID, false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(streamDesc->voipNoPrivacyFlag_.load());

    pipeManager->curPipeList_.clear();
}

/**
* @tc.name  : AudioPolicyServer_016B
* @tc.number: AudioPolicyServer_016B
* @tc.desc  : Test SetAudioSessionBehavior updates descriptor voipNoPrivacyFlag by pid.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_016B, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    ASSERT_NE(policyServerTest, nullptr);
    policyServerTest->coreService_ = AudioCoreService::GetCoreService();
    policyServerTest->eventEntry_ =
        std::make_shared<AudioCoreService::EventEntry>(policyServerTest->coreService_);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 816002;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->callerPid_ = IPCSkeleton::GetCallingPid();
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    constexpr uint32_t voipNoPrivacyBehavior = 0x00000001;
    int32_t ret = policyServerTest->SetAudioSessionBehavior(voipNoPrivacyBehavior);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(streamDesc->voipNoPrivacyFlag_.load());

    pipeManager->curPipeList_.clear();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_017
* @tc.desc  : Test SetRingerModeLegacy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_017, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();

    AudioRingerMode audioRingerMode = AudioRingerMode::RINGER_MODE_NORMAL;
    int32_t result = policyServerTest->SetRingerModeLegacy(audioRingerMode);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_018
* @tc.desc  : Test AudioPolicyServer::LoadSplitModule.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_018, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const std::string splitArgs = "";
    const std::string networkId = "";
    ptrAudioPolicyServer->LoadSplitModule(splitArgs, networkId);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_019
* @tc.desc  : Test AudioPolicyServer::IsAudioSessionActivated
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_019, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool isActive = false;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->IsAudioSessionActivated(isActive);
    EXPECT_EQ(isActive, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_020
* @tc.desc  : Test AudioPolicyServer::IsAudioSessionActivated
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_020, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool isActive = false;
    ptrAudioPolicyServer->interruptService_ = nullptr;
    ptrAudioPolicyServer->IsAudioSessionActivated(isActive);
    EXPECT_EQ(isActive, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_022
* @tc.desc  : Test AudioPolicyServer::DeactivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_022, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->DeactivateAudioSession();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_023
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_023, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_024
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_024, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = nullptr;
    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_025
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_025, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();

    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_026
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_026, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();

    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_027
* @tc.desc  : Test AudioPolicyServer::InjectInterruption
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_027, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const std::string networkId = "";
    InterruptEvent event;

    auto ret = ptrAudioPolicyServer->InjectInterruption(networkId, event);

    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_028
* @tc.desc  : Test AudioPolicyServer::UnsetAudioDeviceAnahsCallback
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_028, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    auto ret = ptrAudioPolicyServer->UnsetAudioDeviceAnahsCallback();

    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_029
* @tc.desc  : Test AudioPolicyServer::TriggerFetchDevice
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_029, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    auto ret = ptrAudioPolicyServer->TriggerFetchDevice(reason);

    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_030
* @tc.desc  : Test AudioPolicyServer::UnsetAudioDeviceRefinerCallback
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_030, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    auto ret = ptrAudioPolicyServer->UnsetAudioDeviceRefinerCallback();

    EXPECT_EQ(ret, ERROR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_031
* @tc.desc  : Test AudioPolicyServer::SetHighResolutionExist
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_031, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool highResExist = true;
    auto ret = ptrAudioPolicyServer->SetHighResolutionExist(highResExist);

    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_032
* @tc.desc  : Test AudioPolicyServer::IsHighResolutionExist
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_032, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool ret;
    ptrAudioPolicyServer->IsHighResolutionExist(ret);

    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_033
* @tc.desc  : Test AudioPolicyServer::DisableSafeMediaVolume
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_033, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    auto ret = ptrAudioPolicyServer->DisableSafeMediaVolume();

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_034
* @tc.desc  : Test AudioPolicyServer::SetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_034, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const AudioSpatializationSceneType spatializationSceneType =
        AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT;
    auto ret = ptrAudioPolicyServer->SetSpatializationSceneType(spatializationSceneType);

    EXPECT_EQ(ret, OPEN_PORT_FAILURE);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_035
* @tc.desc  : Test AudioPolicyServer::GetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_035, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t ret = 0;
    ptrAudioPolicyServer->GetSpatializationSceneType(ret);

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_036
* @tc.desc  : Test AudioPolicyServer::GetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_036, TestSize.Level1)
{
    auto ptrAudioPolicyServer = GetPolicyServerUnitTest();

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> descs = nullptr;
    ptrAudioPolicyServer->GetActiveBluetoothDevice(descs);
    EXPECT_EQ(descs, nullptr);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_037
* @tc.desc  : Test AudioPolicyServer::SetCallDeviceActive
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_037, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    InternalDeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    bool active = true;
    std::string address = "";
    int32_t ret = 0;
    ptrAudioPolicyServer->SetCallDeviceActive(deviceType, active, address, ret);

    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_038
* @tc.desc  : Test AudioPolicyServer::SetCallDeviceActive
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_038, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    InternalDeviceType deviceType = DeviceType::DEVICE_TYPE_EARPIECE;
    bool active = true;
    std::string address = "";
    int32_t ret = 0;
    ptrAudioPolicyServer->SetCallDeviceActive(deviceType, active, address, ret);

    EXPECT_NE(ret, ERR_SYSTEM_PERMISSION_DENIED);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetSystemVolumeLevelWithOption_001
* @tc.desc  : Test AudioPolicyServer::SetSystemVolumeLevelWithOption
*/
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevelWithOption_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelWithOption_001 start");
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_NE(ptrAudioPolicyServer, nullptr);

    int32_t volumeLevel = 5;
    bool isUpdateUi = true;
    VolumeUpdateOption info = { isUpdateUi };
    auto ret = ptrAudioPolicyServer->audioVolumeManager_.SetSystemVolumeLevelWithOption(STREAM_VOICE_CALL,
        volumeLevel, DEFAULT_ZONE_ID, info);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetSystemVolumeLevelWithOption_002
* @tc.desc  : Test AudioPolicyServer::SetSystemVolumeLevelWithOption
*/
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevelWithOption_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelWithOption_002 start");
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_NE(ptrAudioPolicyServer, nullptr);

    int32_t volumeLevel = 5;
    bool isUpdateUi = true;
    VolumeUtils::SetPCVolumeEnable(true);
    VolumeUpdateOption info = { isUpdateUi };
    auto ret = ptrAudioPolicyServer->audioVolumeManager_.SetSystemVolumeLevelWithOption(STREAM_VOICE_CALL,
        volumeLevel, DEFAULT_ZONE_ID, info);
    VolumeUtils::SetPCVolumeEnable(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_041
* @tc.desc  : Test AudioPolicyServer::GetSystemActiveVolumeType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_041, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t streamType;
    int32_t clientUid = 0;
    ptrAudioPolicyServer->GetSystemActiveVolumeType(clientUid, streamType);
    EXPECT_EQ(streamType, STREAM_MUSIC);
    clientUid = 1;
    ptrAudioPolicyServer->GetSystemActiveVolumeType(clientUid, streamType);
    EXPECT_EQ(streamType, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_042
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_042, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_ALL;
    int res = ptrAudioPolicyServer->audioVolumeManager_.GetSystemVolumeLevelNoMuteStateInterface(streamType);
    EXPECT_EQ(res, 5);
    streamType = STREAM_MUSIC;
    res = ptrAudioPolicyServer->audioVolumeManager_.GetSystemVolumeLevelNoMuteStateInterface(streamType);
    EXPECT_EQ(res, 5);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_043
* @tc.desc  : Test AudioPolicyServer::GetStreamMute
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_043, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t streamType = 2;
    bool ret;
    ptrAudioPolicyServer->GetStreamMute(streamType, ret);
    EXPECT_EQ(ret, false);
    streamType = 25;
    ret = ptrAudioPolicyServer->GetStreamMute(streamType, ret);
    EXPECT_EQ(ret, false);
    streamType = 1;
    ret = ptrAudioPolicyServer->GetStreamMute(streamType, ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test TranslateErrorCodeer.
* @tc.number: TranslateErrorCode_001
* @tc.desc  : Test TranslateErrorCodeer.
*/
HWTEST(AudioPolicyUnitTest, TranslateErrorCode_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t result = ERR_INVALID_PARAM;
    uint32_t resultForMonitor = ERR_SUBSCRIBE_INVALID_PARAM;
    uint32_t actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = ERR_NULL_POINTER;
    resultForMonitor = ERR_SUBSCRIBE_KEY_OPTION_NULL;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = ERR_MMI_CREATION;
    resultForMonitor = ERR_SUBSCRIBE_MMI_NULL;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = ERR_MMI_SUBSCRIBE;
    resultForMonitor = ERR_MODE_SUBSCRIBE;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = 99999;
    resultForMonitor = 0;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);
}

/**
* @tc.name  : Test UpdateMuteStateAccordingToVolLevel.
* @tc.number: UpdateMuteStateAccordingToVolLevel_001
* @tc.desc  : Test AudioPolicyServer::UpdateMuteStateAccordingToVolLevel
*/
HWTEST(AudioPolicyUnitTest, UpdateMuteStateAccordingToVolLevel_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 1;
    bool mute = true;
    VolInfoForUpdateMute info = { streamType, volumeLevel, mute, 0 };
    ptrAudioPolicyServer->audioVolumeManager_.UpdateMuteStateAccordingToVolLevel(info);
}

/**
* @tc.name  : Test UpdateMuteStateAccordingToVolLevel.
* @tc.number: UpdateMuteStateAccordingToVolLevel_002
* @tc.desc  : Test AudioPolicyServer::UpdateMuteStateAccordingToVolLevel
*/
HWTEST(AudioPolicyUnitTest, UpdateMuteStateAccordingToVolLevel_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 0;
    bool mute = false;
    VolInfoForUpdateMute info = { streamType, volumeLevel, mute, 0 };
    ptrAudioPolicyServer->audioVolumeManager_.UpdateMuteStateAccordingToVolLevel(info);
}

/**
* @tc.name  : Test UpdateMuteStateAccordingToVolLevel.
* @tc.number: UpdateMuteStateAccordingToVolLevel_003
* @tc.desc  : Test AudioPolicyServer::UpdateMuteStateAccordingToVolLevel
*/
HWTEST(AudioPolicyUnitTest, UpdateMuteStateAccordingToVolLevel_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    AudioStreamType streamType = AudioStreamType::STREAM_SYSTEM;
    int32_t volumeLevel = 1;
    bool mute = false;
    VolInfoForUpdateMute info = { streamType, volumeLevel, mute, 0 };
    ptrAudioPolicyServer->audioVolumeManager_.UpdateMuteStateAccordingToVolLevel(info);
}

/**
* @tc.name  : Test MaxOrMinVolumeOption.
* @tc.number: MaxOrMinVolumeOption_001
* @tc.desc  : Test AudioPolicyServer::MaxOrMinVolumeOption
*/
#ifdef FEATURE_MULTIMODALINPUT_INPUT
HWTEST(AudioPolicyUnitTest, MaxOrMinVolumeOption_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t volLevel = 20;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC;
    bool result = ptrAudioPolicyServer->MaxOrMinVolumeOption(volLevel, keyType, streamInFocus, 0);
    EXPECT_FALSE(result);
    result = ptrAudioPolicyServer->MaxOrMinVolumeOption(volLevel, keyType, streamInFocus, 1);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test MaxOrMinVolumeOption.
* @tc.number: MaxOrMinVolumeOption_002
* @tc.desc  : Test AudioPolicyServer::MaxOrMinVolumeOption
*/
HWTEST(AudioPolicyUnitTest, MaxOrMinVolumeOption_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t volLevel = 0;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC;
    bool result = ptrAudioPolicyServer->MaxOrMinVolumeOption(volLevel, keyType, streamInFocus);
    EXPECT_FALSE(result);
}
#endif

/**
* @tc.name  : Test IsVolumeLevelValid.
* @tc.number: IsVolumeLevelValid_001
* @tc.desc  : Test AudioPolicyServer::IsVolumeLevelValid
*/
HWTEST(AudioPolicyUnitTest, IsVolumeLevelValid_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 20;
    bool result = ptrAudioPolicyServer->audioVolumeManager_.IsVolumeLevelValid(streamType, volumeLevel);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test GetSystemVolumeInDb.
* @tc.number: GetSystemVolumeInDb_001
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeInDb
*/
HWTEST(AudioPolicyUnitTest, GetSystemVolumeInDb_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioVolumeType volumeType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 20;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    float actual = 0;
    ptrAudioPolicyServer->GetSystemVolumeInDb(volumeType, volumeLevel, deviceType, actual);
    float result = static_cast<float>(ERR_INVALID_PARAM);
    EXPECT_EQ(actual, result);
}

/**
* @tc.name  : Test IsArmUsbDevice.
* @tc.number: IsArmUsbDevice_001
* @tc.desc  : Test AudioPolicyServer::IsArmUsbDevice
*/
HWTEST(AudioPolicyUnitTest, IsArmUsbDevice_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    bool result = ptrAudioPolicyServer->IsArmUsbDevice(desc);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test IsArmUsbDevice.
* @tc.number: IsArmUsbDevice_002
* @tc.desc  : Test AudioPolicyServer::IsArmUsbDevice
*/
HWTEST(AudioPolicyUnitTest, IsArmUsbDevice_002, TestSize.Level1)
{
    auto ptrAudioPolicyServer = GetPolicyServerUnitTest();
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    bool result = ptrAudioPolicyServer->IsArmUsbDevice(desc);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test MapExternalToInternalDeviceType.
* @tc.number: MapExternalToInternalDeviceType_001
* @tc.desc  : Test AudioPolicyServer::MapExternalToInternalDeviceType
*/
HWTEST(AudioPolicyUnitTest, MapExternalToInternalDeviceType_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc.deviceId_ = 0;
    auto ptrAudioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(desc);
    EXPECT_NE(ptrAudioDeviceDescriptor, nullptr);
    ptrAudioPolicyServer->audioDeviceManager_.connectedDevices_.push_back(ptrAudioDeviceDescriptor);
    ptrAudioPolicyServer->MapExternalToInternalDeviceType(desc);
}

/**
* @tc.name  : Test MapExternalToInternalDeviceType.
* @tc.number: MapExternalToInternalDeviceType_002
* @tc.desc  : Test AudioPolicyServer::MapExternalToInternalDeviceType
*/
HWTEST(AudioPolicyUnitTest, MapExternalToInternalDeviceType_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceRole_ == INPUT_DEVICE;
    auto ptrAudioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(desc);
    EXPECT_NE(ptrAudioDeviceDescriptor, nullptr);
    ptrAudioPolicyServer->audioDeviceManager_.connectedDevices_.push_back(ptrAudioDeviceDescriptor);
    ptrAudioPolicyServer->MapExternalToInternalDeviceType(desc);
}

/**
* @tc.name  : Test MapExternalToInternalDeviceType.
* @tc.number: MapExternalToInternalDeviceType_003
* @tc.desc  : Test AudioPolicyServer::MapExternalToInternalDeviceType
*/
HWTEST(AudioPolicyUnitTest, MapExternalToInternalDeviceType_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    desc.deviceRole_ = INPUT_DEVICE;
    auto ptrAudioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(desc);
    EXPECT_NE(ptrAudioDeviceDescriptor, nullptr);
    ptrAudioPolicyServer->audioDeviceManager_.connectedDevices_.push_back(ptrAudioDeviceDescriptor);
    ptrAudioPolicyServer->MapExternalToInternalDeviceType(desc);
    EXPECT_EQ(desc.deviceType_, DEVICE_TYPE_WIRED_MICIN);
}

/**
* @tc.name  : Test UpdateDeviceInfo.
* @tc.number: UpdateDeviceInfo_001
* @tc.desc  : Test UpdateDeviceInfo.
*/
HWTEST(AudioPolicyUnitTest, UpdateDeviceInfo_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>();
    int32_t command = 1;

    int32_t ret = server->UpdateDeviceInfo(deviceDesc, command);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}

/**
* @tc.name  : Test SetCallbackCapturerInfo.
* @tc.number: SetCallbackCapturerInfo_001
* @tc.desc  : Test AudioPolicyServer::SetCallbackCapturerInfo
*/
HWTEST(AudioPolicyUnitTest, SetCallbackCapturerInfo_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t capturerFlags = 0;
    AudioCapturerInfo audioCapturerInfo(sourceType, capturerFlags);
    int32_t uid = -1;
    int32_t result = ptrAudioPolicyServer->SetCallbackCapturerInfo(audioCapturerInfo, uid);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test SetAudioScene.
* @tc.number: SetAudioScene_001
* @tc.desc  : Test AudioPolicyServer::SetAudioScene
*/
HWTEST(AudioPolicyUnitTest, SetAudioScene_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioScene audioScene = AUDIO_SCENE_CALL_START;
    int32_t result = ptrAudioPolicyServer->SetAudioScene(audioScene);
    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test SetAudioInterruptCallback.
* @tc.number: SetAudioInterruptCallback_001
* @tc.desc  : Test AudioPolicyServer::SetAudioInterruptCallback
*/
HWTEST(AudioPolicyUnitTest, SetAudioInterruptCallback_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    uint32_t sessionID = 0;
    sptr<IRemoteObject> object = nullptr;
    uint32_t clientUid = 0;
    int32_t zoneID = 0;
    int32_t result = ptrAudioPolicyServer->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test GetStreamVolumeInfoMap.
* @tc.number: GetStreamVolumeInfoMap_001
* @tc.desc  : Test AudioPolicyServer::GetStreamVolumeInfoMap
*/
HWTEST(AudioPolicyUnitTest, GetStreamVolumeInfoMap_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    StreamVolumeInfoMap streamVolumeInfos;
    ptrAudioPolicyServer->GetStreamVolumeInfoMap(streamVolumeInfos);
}

/**
* @tc.name  : Test InitPolicyDumpMap.
* @tc.number: InitPolicyDumpMap_001
* @tc.desc  : Test AudioPolicyServer::InitPolicyDumpMap
*/
HWTEST(AudioPolicyUnitTest, InitPolicyDumpMap_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->InitPolicyDumpMap();
}

/**
* @tc.name  : Test AudioDevicesDump.
* @tc.number: AudioDevicesDump_001
* @tc.desc  : Test AudioPolicyServer::AudioDevicesDump
*/
HWTEST(AudioPolicyUnitTest, AudioDevicesDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioDevicesDump(dumpString);
}

/**
* @tc.name  : Test AudioModeDump.
* @tc.number: AudioModeDump_001
* @tc.desc  : Test AudioPolicyServer::AudioModeDump
*/
HWTEST(AudioPolicyUnitTest, AudioModeDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioModeDump(dumpString);
}

/**
* @tc.name  : Test AudioVolumeDump.
* @tc.number: AudioVolumeDump_001
* @tc.desc  : Test AudioPolicyServer::AudioVolumeDump
*/
HWTEST(AudioPolicyUnitTest, AudioVolumeDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioVolumeDump(dumpString);
}

/**
* @tc.name  : Test AudioPolicyParserDump.
* @tc.number: AudioPolicyParserDump_001
* @tc.desc  : Test AudioPolicyServer::AudioPolicyParserDump
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyParserDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioPolicyParserDump(dumpString);
}

/**
* @tc.name  : Test AudioStreamDump.
* @tc.number: AudioStreamDump_001
* @tc.desc  : Test AudioPolicyServer::AudioStreamDump
*/
HWTEST(AudioPolicyUnitTest, AudioStreamDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioStreamDump(dumpString);
}

/**
* @tc.name  : Test XmlParsedDataMapDump.
* @tc.number: XmlParsedDataMapDump_001
* @tc.desc  : Test AudioPolicyServer::XmlParsedDataMapDump
*/
HWTEST(AudioPolicyUnitTest, XmlParsedDataMapDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->XmlParsedDataMapDump(dumpString);
}

/**
* @tc.name  : Test EffectManagerInfoDump.
* @tc.number: EffectManagerInfoDump_001
* @tc.desc  : Test AudioPolicyServer::EffectManagerInfoDump
*/
HWTEST(AudioPolicyUnitTest, EffectManagerInfoDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->EffectManagerInfoDump(dumpString);
}

/**
* @tc.name  : Test MicrophoneMuteInfoDump.
* @tc.number: MicrophoneMuteInfoDump_001
* @tc.desc  : Test AudioPolicyServer::MicrophoneMuteInfoDump
*/
HWTEST(AudioPolicyUnitTest, MicrophoneMuteInfoDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->MicrophoneMuteInfoDump(dumpString);
}

constexpr int32_t TEST_DEVICE_ID = 1001;
static AudioDeviceDescriptor& CurrentActiveDevice()
{
    auto descs = AudioRouterSelectStrategy::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    auto desc = descs.empty() || !descs.back() ? make_shared<AudioDeviceDescriptor>() : descs.back();
    if (desc->deviceId_ == 0) {
        desc->deviceId_ = TEST_DEVICE_ID;
        AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
        AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    }
    desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(desc->deviceId_);
    return *desc;
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_046
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_046, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = true;
    ptrAudioPolicyServer->isScreenOffOrLock_ = true;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, AUDIO_OK);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_048
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_048, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = false;
    ptrAudioPolicyServer->isScreenOffOrLock_ = true;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, AUDIO_OK);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_049
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_049, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = false;
    ptrAudioPolicyServer->isScreenOffOrLock_ = false;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, AUDIO_OK);
}

#ifdef FUNC_NOT_FIND
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_051
* @tc.desc  : Test AudioPolicyServer::SetRingerMode
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_051, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    ptrAudioPolicyServer->coreService_ = std::make_shared<AudioCoreService>();
    auto ret = ptrAudioPolicyServer->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrAudioPolicyServer->SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrAudioPolicyServer->SetRingerMode(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrAudioPolicyServer->SetRingerMode(60);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_053
* @tc.desc  : Test IsHeadTrackingEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_053, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsHeadTrackingEnabled(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_054
* @tc.desc  : Test IsHeadTrackingEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_054, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    bool ret = false;
    server->IsHeadTrackingEnabled(address, ret);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_055
* @tc.desc  : Test SetHeadTrackingEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_055, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->SetHeadTrackingEnabled(true);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_056
* @tc.desc  : Test IsSpatializationSupported.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_056, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsSpatializationSupported(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_057
* @tc.desc  : Test IsSpatializationSupportedForDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_057, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    bool ret = false;
    server->IsSpatializationSupportedForDevice(address, ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_058
* @tc.desc  : Test IsHeadTrackingSupported.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_058, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsHeadTrackingSupported(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_059
* @tc.desc  : Test IsHeadTrackingSupportedForDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_059, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    bool ret;
    server->IsHeadTrackingSupportedForDevice(address, ret);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_060
* @tc.desc  : Test UpdateSpatialDeviceState.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_060, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioSpatialDeviceState audioSpatialDeviceState;
    auto ret = server->UpdateSpatialDeviceState(audioSpatialDeviceState);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_061
* @tc.desc  : Test SetAudioDeviceRefinerCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_061, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAudioDeviceRefinerCallback(object);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_062
* @tc.desc  : Test SetPreferredDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_062, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc;
    int32_t ret = 0;
    server->SetPreferredDevice(PreferredType::AUDIO_CALL_CAPTURE, desc, ret);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_063
* @tc.desc  : Test SetPreferredDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_063, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string networkId = "test";
    VolumeBehavior volumeBehavior;
    int32_t result = server->SetDeviceVolumeBehavior(networkId, DeviceType::DEVICE_TYPE_EARPIECE, volumeBehavior);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_064
* @tc.desc  : Test SetAudioDeviceAnahsCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_064, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAudioDeviceAnahsCallback(object);
    EXPECT_EQ(ret, ERROR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_065
* @tc.desc  : Test GetSupportedAudioEffectProperty.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_065, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioEffectPropertyArray propertyArray;
    auto ret = server->GetSupportedAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_066
* @tc.desc  : Test GetSupportedAudioEffectProperty.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_066, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioEffectPropertyArray propertyArray;
    auto ret = server->SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, AUDIO_OK);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_067
* @tc.desc  : Test GetAudioEffectProperty.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_067, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioEffectPropertyArray propertyArray;
    server->GetAudioEffectProperty(propertyArray);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_068
* @tc.desc  : Test SetDeviceAbsVolumeSupported.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_068, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string macAddress = "test";
    bool support = true;
    auto ret = server->SetDeviceAbsVolumeSupported(macAddress, support, 0);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_069
* @tc.desc  : Test SetA2dpDeviceVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_069, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string macAddress = "test";
    int32_t volume = 1;
    bool updateUi = true;
    auto ret = server->SetA2dpDeviceVolume(macAddress, volume, updateUi);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_070
* @tc.desc  : Test GetAvailableDevices.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_070, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = server->coreService_->GetEventEntry();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetAvailableDevices(AudioDeviceUsage::ALL_CALL_DEVICES, ret);
    EXPECT_NE(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_071
* @tc.desc  : Test GetAvailableDevices.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_071, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t deviceUsage = -1;
    AudioDeviceUsage usge = static_cast<AudioDeviceUsage>(deviceUsage);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetAvailableDevices(usge, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_072
* @tc.desc  : Test SetAvailableDeviceChangeCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_072, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAvailableDeviceChangeCallback(clientId, AudioDeviceUsage::ALL_CALL_DEVICES, object);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_073
* @tc.desc  : Test SetAvailableDeviceChangeCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_073, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    int32_t deviceUsage = -1;
    AudioDeviceUsage usge = static_cast<AudioDeviceUsage>(deviceUsage);
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAvailableDeviceChangeCallback(clientId, usge, object);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_074
* @tc.desc  : Test ConfigDistributedRoutingRole.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_074, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    server->coreService_ = AudioCoreService::GetCoreService();
    auto ret = server->ConfigDistributedRoutingRole(descriptor, CastType::CAST_TYPE_ALL);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_075
* @tc.desc  : Test SetDistributedRoutingRoleCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_075, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->audioPolicyServerHandler_ = nullptr;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetDistributedRoutingRoleCallback(object);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->SetDistributedRoutingRoleCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_076
* @tc.desc  : Test UnsetDistributedRoutingRoleCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_076, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->audioPolicyServerHandler_ = nullptr;
    auto ret = server->UnsetDistributedRoutingRoleCallback();
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->UnsetDistributedRoutingRoleCallback();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_077
* @tc.desc  : Test RegisterPowerStateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_077, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->RegisterPowerStateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_078
* @tc.desc  : Test UnRegisterPowerStateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_078, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->powerStateListener_ = nullptr;
    server->UnRegisterPowerStateListener();

    server->powerStateListener_ = new PowerStateListener(server);
    server->UnRegisterPowerStateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_079
* @tc.desc  : Test RegisterAppStateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_079, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->appStateListener_ = nullptr;
    server->RegisterAppStateListener();

    server->appStateListener_ = new AppStateListener();
    server->RegisterAppStateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_080
* @tc.desc  : Test RegisterSyncHibernateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_080, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->syncHibernateListener_ = nullptr;
    server->RegisterSyncHibernateListener();

    server->syncHibernateListener_ = new SyncHibernateListener(server);
    server->RegisterSyncHibernateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_081
* @tc.desc  : Test UnRegisterSyncHibernateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_081, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->syncHibernateListener_ = nullptr;
    server->UnRegisterSyncHibernateListener();

    server->syncHibernateListener_ = new SyncHibernateListener(server);
    server->UnRegisterSyncHibernateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_082
* @tc.desc  : Test IsSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_082, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsSpatializationEnabled(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_083
* @tc.desc  : Test IsSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_083, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    auto ret = false;
    server->IsSpatializationEnabled(address, ret);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_084
* @tc.desc  : Test SetSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_084, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->SetSpatializationEnabled(true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_085
* @tc.desc  : Test SetSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_085, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    auto ret = server->SetSpatializationEnabled(selectedAudioDevice, true);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_086
* @tc.desc  : Test UpdateTracker.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_086, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioStreamChangeInfo streamChangeInfo;
    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_PAUSED;
    auto ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);

    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;
    ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);

    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_RELEASED;
    ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);

    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_RUNNING;
    ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_093
* @tc.desc  : Test GetNetworkIdByGroupId.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_093, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t groupId = 0;
    std::string networkId = "test";
    auto ret = server->GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(ret, ERROR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_094
* @tc.desc  : Test SetSystemSoundUri.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_094, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string key = "test";
    std::string uri = "test";
    auto ret = server->SetSystemSoundUri(key, uri);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_095
* @tc.desc  : Test GetSystemSoundUri.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_095, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string key = "test";
    std::string ret;
    server->GetSystemSoundUri(key, ret);
    EXPECT_EQ(ret, "");
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_096
* @tc.desc  : Test GetMaxRendererInstances.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_096, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t ret = 0;
    server->isFirstAudioServiceStart_.store(true);
    server->GetMaxRendererInstances(ret);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_097
* @tc.desc  : Test GetMaxRendererInstances.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_097, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t ret = 0;
    server->isFirstAudioServiceStart_.store(false);
    server->GetMaxRendererInstances(ret);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_098
* @tc.desc  : Test GetPreferredOutputDeviceDescriptors.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_098, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioRendererInfo rendererInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    server->GetPreferredOutputDeviceDescriptors(rendererInfo, true, deviceDescs);
    server->GetPreferredOutputDeviceDescriptors(rendererInfo, false, deviceDescs);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_099
* @tc.desc  : Test SetCallbackRendererInfo.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_099, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioRendererInfo rendererInfo;
    server->audioPolicyServerHandler_ = nullptr;
    auto ret = server->SetCallbackRendererInfo(rendererInfo);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->SetCallbackRendererInfo(rendererInfo);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_100
* @tc.desc  : Test SetInputDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_100, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    uint32_t sessionID = 0;
    auto ret = server->SetInputDevice(DeviceType::DEVICE_TYPE_EARPIECE, sessionID,
        SourceType::SOURCE_TYPE_INVALID, true);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_102
* @tc.desc  : Test SetRingerModeInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_102, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    auto ret = server->SetRingerModeInner(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetRingerModeInner(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_103
* @tc.desc  : Test SetRingerModeInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_103, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->SetMicrophoneMuteCommon(true, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_104
* @tc.desc  : Test IsMicrophoneMuteLegacy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_104, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsMicrophoneMuteLegacy(ret);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_105
* @tc.desc  : Test UnsetAudioInterruptCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_105, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    uint32_t sessionID = 0;
    int32_t zoneID = 0;
    server->interruptService_ = nullptr;
    auto ret = server->UnsetAudioInterruptCallback(sessionID, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->UnsetAudioInterruptCallback(sessionID, zoneID);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_106
* @tc.desc  : Test SetQueryClientTypeCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_106, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetQueryClientTypeCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_107
* @tc.desc  : Test SetAudioClientInfoMgrCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_107, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAudioClientInfoMgrCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_108
* @tc.desc  : Test SetQueryBundleNameListCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_108, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetQueryBundleNameListCallback(object);
    EXPECT_NE(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_109
* @tc.desc  : Test RequestAudioFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_109, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    server->interruptService_ = nullptr;
    auto ret = server->RequestAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->RequestAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_120
* @tc.desc  : Test AbandonAudioFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_120, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    server->interruptService_ = nullptr;
    auto ret = server->AbandonAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->AbandonAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_121
* @tc.desc  : Test ProcessRemoteInterrupt.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_121, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::set<int32_t> sessionIds;
    InterruptEventInternal interruptEvent;
    server->interruptService_ = nullptr;
    server->ProcessRemoteInterrupt(sessionIds, interruptEvent);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->ProcessRemoteInterrupt(sessionIds, interruptEvent);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_122
* @tc.desc  : Test GetStreamInFocusByUid.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_122, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 0;
    int32_t zoneID = 0;
    int32_t ret = 0;
    server->GetStreamInFocusByUid(uid, zoneID, ret);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_123
* @tc.desc  : Test GetSessionInfoInFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_123, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioInterrupt audioInterrupt;
    int32_t zoneID = 0;
    server->interruptService_ = nullptr;
    auto ret = server->GetSessionInfoInFocus(audioInterrupt, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->GetSessionInfoInFocus(audioInterrupt, zoneID);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_124
* @tc.desc  : Test GetAudioFocusInfoList.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_124, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList;
    int32_t zoneID = 0;
    server->interruptService_ = nullptr;
    auto ret = server->GetAudioFocusInfoList(focusInfoList, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->GetAudioFocusInfoList(focusInfoList, zoneID);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_125
* @tc.desc  : Test AdjustSystemVolumeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_125, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->AdjustSystemVolumeByStep(AudioVolumeType::STREAM_ALL, VolumeAdjustType::VOLUME_UP);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_126
* @tc.desc  : Test SetStreamMuteInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_126, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    auto ret = server->SetStreamMuteInternal(AudioStreamType::STREAM_ALARM, false, true, deviceType);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetStreamMuteInternal(AudioStreamType::STREAM_ALL, false, true, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_127
* @tc.desc  : Test UpdateSystemMuteStateAccordingMusicState.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_127, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->audioVolumeManager_.UpdateSystemMuteStateAccordingMusicState(
        AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, false, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_128
* @tc.desc  : Test SendMuteKeyEventCbWithUpdateUiOrNot.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_128, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool isUpdateUi = true;
    server->audioPolicyServerHandler_ = nullptr;
    server->audioVolumeManager_.SendMuteKeyEventCbWithUpdateUiOrNot(
        AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    server->audioVolumeManager_.SendMuteKeyEventCbWithUpdateUiOrNot(
        AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_129
* @tc.desc  : Test SetSingleStreamMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_129, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    server->SetSingleStreamMute(AudioStreamType::STREAM_MUSIC, false, true, deviceType);
    server->SetSingleStreamMute(AudioStreamType::STREAM_RING, false, true, deviceType);
    server->SetSingleStreamMute(AudioStreamType::STREAM_RING, true, true, deviceType);
    server->SetSingleStreamMute(AudioStreamType::STREAM_RING, true, false, deviceType);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_130
* @tc.desc  : Test ProcUpdateRingerModeForMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_130, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    server->audioVolumeManager_.ProcUpdateRingerModeForMute(false, true);
    server->audioVolumeManager_.ringerModeManager_.supportVibrator_ = false;
    server->audioVolumeManager_.ProcUpdateRingerModeForMute(true, true);

    server->audioVolumeManager_.ringerModeManager_.supportVibrator_ = true;
    server->audioVolumeManager_.ProcUpdateRingerModeForMute(true, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_131
* @tc.desc  : Test SetSelfAppVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_131, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 0;
    int32_t volumeFlag = 0;
    auto ret = server->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_132
* @tc.desc  : Test IsAppVolumeMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_132, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    bool owned = true;
    bool isMute = false;
    auto ret = server->IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_133
* @tc.desc  : Test SendVolumeKeyEventCbWithUpdateUiOrNot.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_133, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool isUpdateUi = true;
    server->audioPolicyServerHandler_ = nullptr;
    server->audioVolumeManager_.SendVolumeKeyEventCbWithUpdateUiOrNot(
        AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    server->audioVolumeManager_.SendVolumeKeyEventCbWithUpdateUiOrNot(
        AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_134
* @tc.desc  : Test ProcUpdateRingerMode.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_134, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    server->audioVolumeManager_.ringerModeManager_.supportVibrator_ = true;
    server->audioVolumeManager_.ProcUpdateRingerMode();

    server->audioVolumeManager_.ringerModeManager_.supportVibrator_ = false;
    server->audioVolumeManager_.ProcUpdateRingerMode();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_136
* @tc.desc  : Test SetSingleStreamVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_136, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    VolumeUpdateOption option{true, false};
    auto ret = server->audioVolumeManager_.SetSingleStreamVolume(
        AudioStreamType::STREAM_VOICE_ASSISTANT, volumeLevel, option);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->audioVolumeManager_.SetSingleStreamVolume(AudioStreamType::STREAM_RING, volumeLevel, option);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->audioVolumeManager_.SetSingleStreamVolume(AudioStreamType::STREAM_VOICE_RING, volumeLevel, option);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_139
* @tc.desc  : Test GetDevices.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_139, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(server->coreService_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetDevices(DeviceFlag::NONE_DEVICES_FLAG, ret);
    EXPECT_EQ(ret.size(), 0);

    DeviceFlag deviceFlag = static_cast<DeviceFlag>(-1);
    server->GetDevices(DeviceFlag::NONE_DEVICES_FLAG, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_140
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_140, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetDevicesInner(DeviceFlag::ALL_DEVICES_FLAG, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_141
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_141, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetOutputDevice(audioRendererFilter, ret);
    EXPECT_NE(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_142
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_142, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetOutputDevice(audioRendererFilter, ret);
    EXPECT_NE(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_143
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_143, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetInputDevice(audioCapturerFilter, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_144
* @tc.desc  : Test SubscribePowerStateChangeEvents.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_144, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->SubscribePowerStateChangeEvents();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_145
* @tc.desc  : Test OnReceiveEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_145, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    EventFwk::CommonEventData eventData;
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    server->OnReceiveEvent(eventData);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_146
* @tc.desc  : Test SubscribeCommonEventExecute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_146, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->SubscribeCommonEventExecute();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_147
* @tc.desc  : Test CheckSubscribePowerStateChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_147, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->powerStateCallbackRegister_ = true;
    server->CheckSubscribePowerStateChange();
    server->powerStateCallbackRegister_ = false;
    server->CheckSubscribePowerStateChange();
    EXPECT_TRUE(server->powerStateCallbackRegister_);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_148
* @tc.desc  : Test CheckSubscribePowerStateChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_148, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    auto ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_TTS, volumeLevel);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = -1;
    ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    volumeLevel = 6;
    ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_RING, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
    MockNative::Resume();

    volumeLevel = server->audioVolumeManager_.GetMinVolumeLevel(AudioStreamType::STREAM_RING);
    ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_RING, volumeLevel);
#ifdef AUDIO_BUILD_VARIANT_ROOT
    EXPECT_EQ(ret, SUCCESS);
#else
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
#endif
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_149
* @tc.desc  : Test SetAppVolumeMuted.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_149, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;
    auto ret = server->SetAppVolumeMuted(appUid, true, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_150
* @tc.desc  : Test SetAppVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_150, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeLevel = 1;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;
    auto ret = server->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    volumeLevel = server->audioVolumeManager_.GetMinVolumeLevel(AudioStreamType::STREAM_APP);
    ret = server->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_151
* @tc.desc  : Test SetSystemVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_151, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;

    auto ret = server->SetSystemVolumeLevel(AudioStreamType::STREAM_WAKEUP, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = -1;
    ret = server->SetSystemVolumeLevel(AudioStreamType::STREAM_MUSIC, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = server->audioVolumeManager_.GetMinVolumeLevel(AudioStreamType::STREAM_MUSIC);
    ret = server->SetSystemVolumeLevel(AudioStreamType::STREAM_MUSIC, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_152
* @tc.desc  : Test SetSystemVolumeLevelWithDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_152, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;

    auto ret = server->SetSystemVolumeLevelWithDevice(AudioStreamType::STREAM_WAKEUP, volumeLevel,
        DeviceType::DEVICE_TYPE_EARPIECE, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = -1;
    ret = server->SetSystemVolumeLevelWithDevice(AudioStreamType::STREAM_MUSIC, volumeLevel,
        DeviceType::DEVICE_TYPE_EARPIECE, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_153
* @tc.desc  : Test GetSystemActiveVolumeTypeInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_153, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientUid = 0;
    auto ret = server->GetSystemActiveVolumeTypeInternal(clientUid);
    EXPECT_EQ(ret, AudioStreamType::STREAM_MUSIC);

    clientUid = 1;
    ret = server->GetSystemActiveVolumeTypeInternal(clientUid);
    EXPECT_EQ(ret, AudioStreamType::STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_154
* @tc.desc  : Test GetAppVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_154, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeLevel = 1;
    auto ret = server->GetAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_155
* @tc.desc  : Test GetSystemVolumeLevelInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_155, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->audioVolumeManager_.GetSystemVolumeLevelInterface(AudioStreamType::STREAM_ALL);
    server->audioVolumeManager_.GetSystemVolumeLevelInterface(AudioStreamType::STREAM_VOICE_RING);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_156
* @tc.desc  : Test SetLowPowerVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_156, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamId = 0;
    float volume = 0.5f;
    auto ret = server->SetLowPowerVolume(streamId, volume);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_157
* @tc.desc  : Test CheckCanMuteVolumeTypeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_157, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    server->audioVolumeManager_.volumeStep_ = 1;
    auto ret =
        server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_ASSISTANT, volumeLevel);
    EXPECT_EQ(ret, false);

    server->audioVolumeManager_.volumeStep_ = 0;
    ret =
        server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_ASSISTANT, volumeLevel);
    EXPECT_EQ(ret, true);

    ret = server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(ret, true);

    server->audioVolumeManager_.volumeStep_ = 1;
    ret = server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_158
* @tc.desc  : Test CheckCanMuteVolumeTypeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_158, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    server->audioVolumeManager_.volumeStep_ = 1;
    auto ret =
        server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_CALL, volumeLevel);
    EXPECT_EQ(ret, false);

    ret = server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_ALARM, volumeLevel);
    EXPECT_EQ(ret, false);

    ret = server->audioVolumeManager_.CheckCanMuteVolumeTypeByStep(
        AudioVolumeType::STREAM_VOICE_COMMUNICATION, volumeLevel);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_159
* @tc.desc  : Test AdjustVolumeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_159, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->AdjustVolumeByStep(VolumeAdjustType::VOLUME_UP);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->AdjustVolumeByStep(VolumeAdjustType::VOLUME_DOWN);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_160
* @tc.desc  : Test TranslateKeyEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_160, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t resultOfVolumeKey = 0;
    server->SendMonitortEvent(keyType, resultOfVolumeKey);

    keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN;
    server->SendMonitortEvent(keyType, resultOfVolumeKey);

    keyType = OHOS::MMI::KeyEvent::KEYCODE_MUTE;
    server->SendMonitortEvent(keyType, resultOfVolumeKey);

    keyType = -1;
    server->SendMonitortEvent(keyType, resultOfVolumeKey);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_161
* @tc.desc  : Test HandleAudioServerRelatedSystemAbility.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_161, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t systemAbilityId = APP_MGR_SERVICE_ID;
    std::string deviceId = "test";
    server->HandleAudioServerRelatedSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = -1;
    server->HandleAudioServerRelatedSystemAbility(systemAbilityId, deviceId);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_162
* @tc.desc  : Test HandleKvDataShareEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_162, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->isInitMuteState_.store(false);
    server->HandleKvDataShareEvent();

    server->isInitMuteState_.store(true);
    server->HandleKvDataShareEvent();
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_163
* @tc.desc  : Test SubscribeVolumeKeyEvents.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_163, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->hasSubscribedVolumeKeyEvents_.store(false);
    server->SubscribeVolumeKeyEvents();

    server->hasSubscribedVolumeKeyEvents_.store(true);
    server->SubscribeVolumeKeyEvents();
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_164
* @tc.desc  : Test SubscribeOsAccountChangeEvents.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_164, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->accountObserver_ = nullptr;
    server->SubscribeOsAccountChangeEvents();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_165
* @tc.desc  : Test AddAudioServiceOnStart.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_165, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->isFirstAudioServiceStart_.store(true);
    server->AddAudioServiceOnStart();

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(server->coreService_);
    server->isFirstAudioServiceStart_.store(false);
    server->AddAudioServiceOnStart();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_0166
* @tc.desc  : Test SetDeviceConnectionStatus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_166, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->deviceName_ = "Speaker_Out";
    desc->deviceRole_ = OUTPUT_DEVICE;

    bool isConnected = true;
    int32_t ret = server->SetDeviceConnectionStatus(desc, isConnected);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_167
* @tc.desc  : Test GetDirectPlaybackSupport.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_167, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.encoding = ENCODING_PCM;
    streamInfo.format = SAMPLE_S24LE;
    streamInfo.channels = STEREO;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t result = 0;
    server->GetDirectPlaybackSupport(streamInfo, streamUsage, result);
    EXPECT_EQ(result, DIRECT_PLAYBACK_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test AudioPolicyServer.
 * @tc.number: AudioPolicyServer_168
 * @tc.desc  : Test ActivatePreemptMode.
 */
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_168, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t result = server->ActivatePreemptMode();
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioPolicyServer_222
* @tc.desc  : Test GetVADeviceBroker.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_222, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    sptr<IRemoteObject> client;
    int32_t result = server->GetVADeviceBroker(client);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_NE(client, nullptr);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioPolicyServer_223
* @tc.desc  : Test GetVADeviceBroker.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_223, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    sptr<IRemoteObject> controller;
    std::string macAddress = "00:11:22:33:44:55";
    int32_t result = server->GetVADeviceController(macAddress, controller);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test GetActiveAudioInterruptZone.
* @tc.number: GetActiveAudioInterruptZone_001
* @tc.desc  : Test AudioPolicyServer::GetActiveAudioInterruptZone when zoneId is zero.
*/
HWTEST(AudioPolicyUnitTest, GetActiveAudioInterruptZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t zoneId = 0;
    AudioStreamType streamType = STREAM_DEFAULT;

    PrepareActiveZone(STREAM_RING, false);
    audioPolicyServer->GetActiveAudioInterruptZone(zoneId, streamType);
    EXPECT_EQ(zoneId, 0);
    EXPECT_EQ(streamType, STREAM_RING);

    int32_t activeZoneId = PrepareActiveZone(STREAM_RING, true);
    zoneId = 0;
    streamType = STREAM_DEFAULT;
    audioPolicyServer->GetActiveAudioInterruptZone(zoneId, streamType);
    EXPECT_EQ(zoneId, activeZoneId);
    EXPECT_EQ(streamType, STREAM_RING);

    ResetGetActiveAudioInterruptZoneTestEnv();
}

/**
* @tc.name  : Test GetActiveAudioInterruptZone.
* @tc.number: GetActiveAudioInterruptZone_002
* @tc.desc  : Test AudioPolicyServer::GetActiveAudioInterruptZone when zoneId is not zero.
*/
HWTEST(AudioPolicyUnitTest, GetActiveAudioInterruptZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_NE(audioPolicyServer, nullptr);

    audioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioPolicyServer->interruptService_, nullptr);
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt, AudioFocuState::ACTIVE);
    audioPolicyServer->interruptService_->zonesMap_[1] = audioInterruptZone;

    int32_t zoneId = 1;
    AudioStreamType streamType = STREAM_DEFAULT;
    audioPolicyServer->GetActiveAudioInterruptZone(zoneId, streamType);
    EXPECT_EQ(zoneId, 1);
    EXPECT_EQ(streamType, STREAM_MUSIC);

    audioPolicyServer->audioVolumeManager_.needForceControlVolumeType_ = true;
    audioPolicyServer->audioVolumeManager_.forceControlVolumeType_ = STREAM_RING;

    zoneId = 1;
    streamType = STREAM_DEFAULT;
    audioPolicyServer->GetActiveAudioInterruptZone(zoneId, streamType);
    EXPECT_EQ(zoneId, 0);
    EXPECT_EQ(streamType, STREAM_RING);

    audioPolicyServer->audioVolumeManager_.needForceControlVolumeType_ = false;
    audioPolicyServer->audioVolumeManager_.forceControlVolumeType_ = STREAM_DEFAULT;
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_224
* @tc.desc  : Test OnReceiveEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_224, TestSize.Level4)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    EventFwk::CommonEventData eventData;
    OHOS::EventFwk::Want want;
    want.SetAction("usual.event.SCREEN_UNLOCKED");
    eventData.SetWant(want);
    audioPolicyServer->isRingtoneEL2Ready_ = false;
    audioPolicyServer->OnReceiveEvent(eventData);
    EXPECT_EQ(audioPolicyServer->isRingtoneEL2Ready_, true);
    audioPolicyServer->OnReceiveEvent(eventData);
    EXPECT_EQ(audioPolicyServer->isRingtoneEL2Ready_, true);
    audioPolicyServer->interruptService_ = nullptr;
    audioPolicyServer->OnReceiveEvent(eventData);
    EXPECT_EQ(audioPolicyServer->isRingtoneEL2Ready_, true);
}


/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: IsStreamActiveByStreamUsage_001
 * @tc.desc  : AudioPolicyServer::IsStreamActiveByStreamUsage
 */
HWTEST(AudioPolicyUnitTest, IsStreamActiveByStreamUsage_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamUsage = static_cast<int32_t>(StreamUsage::STREAM_USAGE_MUSIC);
    bool isStreamActive = true;

    int32_t ret = server->IsStreamActiveByStreamUsage(streamUsage, isStreamActive);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetVolumeInDbByStream_001
 * @tc.desc  : AudioPolicyServer::GetVolumeInDbByStream
 */
HWTEST(AudioPolicyUnitTest, GetVolumeInDbByStream_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamUsage = static_cast<int32_t>(StreamUsage::STREAM_USAGE_MUSIC);
    int32_t volLevel = 5;
    int32_t deviceType = static_cast<int32_t>(DeviceType::DEVICE_TYPE_SPEAKER);
    float volDb = 0;

    int32_t ret = server->GetVolumeInDbByStream(streamUsage, volLevel, deviceType, volDb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetSupportedAudioVolumeTypes_001
 * @tc.desc  : AudioPolicyServer::GetSupportedAudioVolumeTypes
 */
HWTEST(AudioPolicyUnitTest, GetSupportedAudioVolumeTypes_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::vector<int32_t> audioVolumeTypes = {};

    int32_t ret = server->GetSupportedAudioVolumeTypes(audioVolumeTypes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetAudioVolumeTypeByStreamUsage_001
 * @tc.desc  : AudioPolicyServer::GetAudioVolumeTypeByStreamUsage
 */
HWTEST(AudioPolicyUnitTest, GetAudioVolumeTypeByStreamUsage_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamUsage = static_cast<int32_t>(StreamUsage::STREAM_USAGE_MUSIC);
    int32_t audioVolumeType = static_cast<int32_t>(AudioVolumeType::STREAM_DEFAULT);

    int32_t ret = server->GetAudioVolumeTypeByStreamUsage(streamUsage, audioVolumeType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetStreamUsagesByVolumeType_001
 * @tc.desc  : AudioPolicyServer::GetStreamUsagesByVolumeType
 */
HWTEST(AudioPolicyUnitTest, GetStreamUsagesByVolumeType_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volType = static_cast<int32_t>(AudioVolumeType::STREAM_MUSIC);
    std::vector<int32_t> streamUsages = {};

    int32_t ret = server->GetStreamUsagesByVolumeType(volType, streamUsages);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyUnitTest, SelectPrivateDevice_01, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    int32_t devType = 8;
    std::string macAddress{"11:22:33:44"};
    int32_t result = server->SelectPrivateDevice(devType, macAddress);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: ForceSelectDevice_01
* @tc.desc  : Test ForceSelectDevice.
*/
HWTEST(AudioPolicyUnitTest, ForceSelectDevice_01, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    sptr<AudioRendererFilter> filter{nullptr};
    int32_t result = server->ForceSelectDevice(31, "11:22", filter);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: SetQueryDeviceVolumeBehaviorCallback_001
 * @tc.desc  : AudioPolicyServer::SetQueryDeviceVolumeBehaviorCallback
 */
HWTEST(AudioPolicyUnitTest, SetQueryDeviceVolumeBehaviorCallback_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = nullptr;

    int32_t ret = server->SetQueryDeviceVolumeBehaviorCallback(object);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    object = new RemoteObjectTestStub();

    ret = server->SetQueryDeviceVolumeBehaviorCallback(object);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: SetDeviceVolumeBehavior_001
* @tc.desc  : Test SetDeviceVolumeBehavior.
*/
HWTEST(AudioPolicyUnitTest, SetDeviceVolumeBehavior_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string networkId = "test";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    VolumeBehavior volumeBehavior;
    int32_t result = server->audioDeviceManager_.SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: GetDeviceVolumeBehavior_001
* @tc.desc  : Test GetDeviceVolumeBehavior.
*/
HWTEST(AudioPolicyUnitTest, GetDeviceVolumeBehavior_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string networkId = "test";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    VolumeBehavior volumeBehavior = server->audioDeviceManager_.GetDeviceVolumeBehavior(networkId, deviceType);
    EXPECT_EQ(volumeBehavior.isReady, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: IsIntelligentNoiseReductionEnabledForCurrentDevice_001
* @tc.desc  : Test IsIntelligentNoiseReductionEnabledForCurrentDevice.
*/
HWTEST(AudioPolicyUnitTest, IsIntelligentNoiseReductionEnabledForCurrentDevice_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    bool isSupport = false;
    int32_t ret = server->IsIntelligentNoiseReductionEnabledForCurrentDevice(sourceType, isSupport);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: CheckAndGetApiVersion_001
* @tc.desc  : AudioPolicyServer::CheckAndGetApiVersion.
*/
HWTEST(AudioPolicyUnitTest, CheckAndGetApiVersion_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool hasSystemPermission = false;

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->hasPair_ = false;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;

    int32_t apiVersion = server->CheckAndGetApiVersion(deviceDescs, hasSystemPermission);
    EXPECT_EQ(apiVersion, 0);
    hasSystemPermission = true;
    apiVersion = server->CheckAndGetApiVersion(deviceDescs, hasSystemPermission);
    EXPECT_EQ(apiVersion, 0);
    deviceDescs.push_back(desc);
    apiVersion = server->CheckAndGetApiVersion(deviceDescs, hasSystemPermission);
    EXPECT_EQ(apiVersion, 0);
}

/**
* @tc.name  : Test GetBundleNameFromUidandGetBundleInfoFromUid.
* @tc.number: GetBundleNameFromUidandGetBundleInfoFromUid_001
* @tc.desc  : GetBundleNameFromUidandGetBundleInfoFromUid.
*/
HWTEST(AudioPolicyUnitTest, GetBundleNameFromUidandGetBundleInfoFromUid_001, TestSize.Level1)
{
    std::string callerName = AudioBundleManager::GetBundleNameFromUid(666);
    EXPECT_EQ(callerName, "");
    AppExecFwk::BundleInfo bundleInfo = AudioBundleManager::GetBundleInfoFromUid(666);
    bundleInfo = AudioBundleManager::GetBundleInfoFromUid(666);
    callerName = AudioBundleManager::GetBundleNameFromUid(666);
    EXPECT_EQ(callerName, "");
}

/**
* @tc.name  : Test GetSystemSoundPath.
* @tc.number: GetSystemSoundPath_001
* @tc.desc  : Test GetSystemSoundPath.
*/
HWTEST(AudioPolicyUnitTest, GetSystemSoundPath_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string ret = "";
    server->GetSystemSoundPath(0, ret);
    EXPECT_EQ(ret, "");

    ret = "";
    server->GetSystemSoundPath(1, ret);
    EXPECT_EQ(ret, "");

    ret = "";
    server->GetSystemSoundPath(2, ret);
    EXPECT_EQ(ret, "");

    ret = "";
    server->GetSystemSoundPath(-1, ret);
    EXPECT_EQ(ret, "");
}

/**
* @tc.name  : Test AudioPolicyServer::RestoreDistributedDeviceInfo.
* @tc.number: RestoreDistributedDeviceInfo_001
* @tc.desc  : Test RestoreDistributedDeviceInfo with permission denied.
*/
HWTEST(AudioPolicyUnitTest, RestoreDistributedDeviceInfo_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->RestoreDistributedDeviceInfo();
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RestoreDistributedDeviceInfo.
* @tc.number: RestoreDistributedDeviceInfo_002
* @tc.desc  : Test RestoreDistributedDeviceInfo with coreService_ nullptr and timeout.
*/
HWTEST(AudioPolicyUnitTest, RestoreDistributedDeviceInfo_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    // Test coreService_ nullptr
    server->coreService_ = nullptr;
    int32_t ret = server->RestoreDistributedDeviceInfo();
    EXPECT_EQ(ret, ERROR);

    // Test timeout (canRestore false)
    server->coreService_ = std::make_shared<AudioCoreService>();
    server->isFirstAudioServiceStart_.store(false);
    ret = server->RestoreDistributedDeviceInfo();
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RestoreDistributedDeviceInfo.
* @tc.number: RestoreDistributedDeviceInfo_003
* @tc.desc  : Test RestoreDistributedDeviceInfo success case.
*/
HWTEST(AudioPolicyUnitTest, RestoreDistributedDeviceInfo_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    server->isFirstAudioServiceStart_.store(true);

    int32_t ret = server->RestoreDistributedDeviceInfo();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::UpdateContextForAudioZone.
* @tc.number: UpdateContextForAudioZone_001
* @tc.desc  : Test UpdateContextForAudioZone with invalid zoneId.
*/
HWTEST(AudioPolicyUnitTest, UpdateContextForAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext context = {};

    // Test zoneId = 0 (valid zoneId, but no permission)
    int32_t ret = server->UpdateContextForAudioZone(0, context);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    // Test zoneId < 0 (invalid zoneId)
    ret = server->UpdateContextForAudioZone(-1, context);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::UpdateContextForAudioZone.
* @tc.number: UpdateContextForAudioZone_003
* @tc.desc  : Test UpdateContextForAudioZone success case.
*/
HWTEST(AudioPolicyUnitTest, UpdateContextForAudioZone_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext context = {};
    context.focusStrategy_ = AudioZoneFocusStrategy::LOCAL_FOCUS_STRATEGY;
    context.backStrategy_ = MediaBackStrategy::STOP;

    int32_t zoneId = 1;

    int32_t ret = server->UpdateContextForAudioZone(zoneId, context);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_001
* @tc.desc  : Test GetAudioZoneForApp with empty userId list.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_001, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<int32_t> userIds = {};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(retUserIds.empty());
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_002
* @tc.desc  : Test GetAudioZoneForApp with userId matching zone.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_002, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext zoneContext;
    zoneContext.userId_ = 100;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("MatchZone", zoneContext, 0);
    ASSERT_GT(zoneId, 0);

    std::vector<int32_t> userIds = {100};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 1);
    EXPECT_EQ(retUserIds[0], 100);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_003
* @tc.desc  : Test GetAudioZoneForApp with userId not matching any zone, fallback to mainZone.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_003, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext mainContext;
    mainContext.userId_ = 200;
    AudioZoneService::GetInstance().UpdateContextForAudioZone(DEFAULT_ZONEID, mainContext);

    std::vector<int32_t> userIds = {100};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 1);
    EXPECT_EQ(retUserIds[0], 200);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_004
* @tc.desc  : Test GetAudioZoneForApp with multiple userIds, some match zones.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_004, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext mainContext;
    mainContext.userId_ = 200;
    AudioZoneService::GetInstance().UpdateContextForAudioZone(DEFAULT_ZONEID, mainContext);

    AudioZoneContext zoneContext;
    zoneContext.userId_ = 100;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone", zoneContext, 0);
    ASSERT_GT(zoneId, 0);

    std::vector<int32_t> userIds = {100, 101};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 2);
    EXPECT_EQ(retUserIds[0], 100);
    EXPECT_EQ(retUserIds[1], 200);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_005
* @tc.desc  : Test GetAudioZoneForApp with duplicate userIds.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_005, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext zoneContext;
    zoneContext.userId_ = 100;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("DupZone", zoneContext, 0);
    ASSERT_GT(zoneId, 0);

    std::vector<int32_t> userIds = {100, 100, 100};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 3);
    EXPECT_EQ(retUserIds[0], 100);
    EXPECT_EQ(retUserIds[1], 100);
    EXPECT_EQ(retUserIds[2], 100);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_006
* @tc.desc  : Test GetAudioZoneForApp with mainZone userId = INVALID_USER_ID.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_006, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext mainContext;
    mainContext.userId_ = AudioZoneContext::INVALID_USER_ID;
    AudioZoneService::GetInstance().UpdateContextForAudioZone(DEFAULT_ZONEID, mainContext);

    std::vector<int32_t> userIds = {100};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 1);
    EXPECT_EQ(retUserIds[0], AudioZoneContext::INVALID_USER_ID);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_007
* @tc.desc  : Test GetAudioZoneForApp after zone is released.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_007, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext mainContext;
    mainContext.userId_ = 250;
    AudioZoneService::GetInstance().UpdateContextForAudioZone(DEFAULT_ZONEID, mainContext);

    AudioZoneContext zoneContext;
    zoneContext.userId_ = 100;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TempZone", zoneContext, 0);
    ASSERT_GT(zoneId, 0);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);

    std::vector<int32_t> userIds = {100};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 1);
    EXPECT_EQ(retUserIds[0], 250);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_008
* @tc.desc  : Test GetAudioZoneForApp with dynamic context update.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_008, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext zoneContext;
    zoneContext.userId_ = 100;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("DynamicZone", zoneContext, 0);
    ASSERT_GT(zoneId, 0);

    AudioZoneContext updatedContext;
    updatedContext.userId_ = 150;
    AudioZoneService::GetInstance().UpdateContextForAudioZone(zoneId, updatedContext);

    std::vector<int32_t> userIds = {150};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 1);
    EXPECT_EQ(retUserIds[0], 150);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneForApp.
* @tc.number: GetAudioZoneForApp_009
* @tc.desc  : Test GetAudioZoneForApp with multiple zones matching different userIds.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneForApp_009, TestSize.Level1)
{
    GetPermission();
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    AudioZoneContext zone1Context;
    zone1Context.userId_ = 100;
    int32_t zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("Zone1", zone1Context, 0);
    ASSERT_GT(zoneId1, 0);

    AudioZoneContext zone2Context;
    zone2Context.userId_ = 200;
    int32_t zoneId2 = AudioZoneService::GetInstance().CreateAudioZone("Zone2", zone2Context, 0);
    ASSERT_GT(zoneId2, 0);

    std::vector<int32_t> userIds = {100, 200};
    std::vector<int32_t> retUserIds;
    int32_t ret = server->GetAudioZoneForApp(userIds, retUserIds);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(retUserIds.size(), 2);
    EXPECT_EQ(retUserIds[0], 100);
    EXPECT_EQ(retUserIds[1], 200);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId2);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAllAudioZone.
* @tc.number: GetAllAudioZone_001
* @tc.desc  : Test GetAllAudioZone success case.
*/
HWTEST(AudioPolicyUnitTest, GetAllAudioZone_001, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    int32_t ret = server->GetAllAudioZone(descs);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_GE(descs.size(), 0u);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZone.
* @tc.number: GetAudioZone_001
* @tc.desc  : Test GetAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, GetAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioZoneDescriptor> desc = nullptr;

    // Test zoneId = 0
    int32_t ret = server->GetAudioZone(0, desc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->GetAudioZone(-1, desc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZone.
* @tc.number: GetAudioZone_002
* @tc.desc  : Test GetAudioZone with non-existent zoneId.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZone_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioZoneDescriptor> desc = nullptr;
    int32_t zoneId = 99999; // Non-existent zoneId

    int32_t ret = server->GetAudioZone(zoneId, desc);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZone.
* @tc.number: GetAudioZone_003
* @tc.desc  : Test GetAudioZone with valid zoneId.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZone_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    // First get all zones to find a valid zoneId
    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    server->GetAllAudioZone(descs);

    if (!descs.empty()) {
        std::shared_ptr<AudioZoneDescriptor> desc = nullptr;
        int32_t zoneId = descs[0]->zoneId_;

        int32_t ret = server->GetAudioZone(zoneId, desc);
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_NE(desc, nullptr);
        EXPECT_EQ(desc->zoneId_, zoneId);
    }
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneByName.
* @tc.number: GetAudioZoneByName_001
* @tc.desc  : Test GetAudioZoneByName with empty name.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneByName_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::string emptyName = "";
    int32_t zoneId = -1;

    int32_t ret = server->GetAudioZoneByName(emptyName, zoneId);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneByName.
* @tc.number: GetAudioZoneByName_002
* @tc.desc  : Test GetAudioZoneByName without permission.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneByName_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::string zoneName = "testZone";
    int32_t zoneId = -1;

    int32_t ret = server->GetAudioZoneByName(zoneName, zoneId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneByName.
* @tc.number: GetAudioZoneByName_003
* @tc.desc  : Test GetAudioZoneByName with non-existent zone name.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneByName_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::string zoneName = "nonExistentZone";
    int32_t zoneId = -1;

    int32_t ret = server->GetAudioZoneByName(zoneName, zoneId);
    // According to AudioZoneService::GetAudioZoneByName, returns ERROR when not found
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioZoneByName.
* @tc.number: GetAudioZoneByName_004
* @tc.desc  : Test GetAudioZoneByName with valid zone name.
*/
HWTEST(AudioPolicyUnitTest, GetAudioZoneByName_004, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    // First get all zones to find a valid zone name
    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    server->GetAllAudioZone(descs);

    if (!descs.empty()) {
        std::string zoneName = descs[0]->name_;
        int32_t zoneId = -1;

        int32_t ret = server->GetAudioZoneByName(zoneName, zoneId);
        EXPECT_EQ(ret, SUCCESS);
    }
}

/**
* @tc.name  : Test AudioPolicyServer::BindDeviceToAudioZone.
* @tc.number: BindDeviceToAudioZone_001
* @tc.desc  : Test BindDeviceToAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, BindDeviceToAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    devices.push_back(device);

    // Test zoneId = 0
    int32_t ret = server->BindDeviceToAudioZone(0, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->BindDeviceToAudioZone(-1, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::BindDeviceToAudioZone.
* @tc.number: BindDeviceToAudioZone_002
* @tc.desc  : Test BindDeviceToAudioZone with empty device list.
*/
HWTEST(AudioPolicyUnitTest, BindDeviceToAudioZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices; // Empty list
    int32_t zoneId = 1;

    int32_t ret = server->BindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::BindDeviceToAudioZone.
* @tc.number: BindDeviceToAudioZone_003
* @tc.desc  : Test BindDeviceToAudioZone with device size >= MAX_SIZE.
*/
HWTEST(AudioPolicyUnitTest, BindDeviceToAudioZone_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    // Create MAX_SIZE devices to exceed the limit
    for (int i = 0; i < 1024; i++) {
        auto device = std::make_shared<AudioDeviceDescriptor>();
        device->deviceType_ = DEVICE_TYPE_SPEAKER;
        devices.push_back(device);
    }
    int32_t zoneId = 1;

    int32_t ret = server->BindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::BindDeviceToAudioZone.
* @tc.number: BindDeviceToAudioZone_004
* @tc.desc  : Test BindDeviceToAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, BindDeviceToAudioZone_004, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    devices.push_back(device);
    int32_t zoneId = 1;

    int32_t ret = server->BindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::BindDeviceToAudioZone.
* @tc.number: BindDeviceToAudioZone_005
* @tc.desc  : Test BindDeviceToAudioZone with non-existent zoneId.
*/
HWTEST(AudioPolicyUnitTest, BindDeviceToAudioZone_005, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    devices.push_back(device);
    int32_t zoneId = 99999; // Non-existent zoneId

    int32_t ret = server->BindDeviceToAudioZone(zoneId, devices);
    // According to AudioZoneService::BindDeviceToAudioZone, returns ERROR when zone not found
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::BindDeviceToAudioZone.
* @tc.number: BindDeviceToAudioZone_006
* @tc.desc  : Test BindDeviceToAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, BindDeviceToAudioZone_006, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    // First get all zones to find a valid zoneId
    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    server->GetAllAudioZone(descs);

    if (!descs.empty()) {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
        auto device = std::make_shared<AudioDeviceDescriptor>();
        device->deviceType_ = DEVICE_TYPE_SPEAKER;
        devices.push_back(device);
        int32_t zoneId = descs[0]->zoneId_;

        int32_t ret = server->BindDeviceToAudioZone(zoneId, devices);
        EXPECT_EQ(ret, SUCCESS);
    }
}

/**
* @tc.name  : Test AudioPolicyServer::UnBindDeviceToAudioZone.
* @tc.number: UnBindDeviceToAudioZone_001
* @tc.desc  : Test UnBindDeviceToAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, UnBindDeviceToAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    devices.push_back(device);

    // Test zoneId = 0
    int32_t ret = server->UnBindDeviceToAudioZone(0, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->UnBindDeviceToAudioZone(-1, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::UnBindDeviceToAudioZone.
* @tc.number: UnBindDeviceToAudioZone_002
* @tc.desc  : Test UnBindDeviceToAudioZone with empty device list.
*/
HWTEST(AudioPolicyUnitTest, UnBindDeviceToAudioZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices; // Empty list
    int32_t zoneId = 1;

    int32_t ret = server->UnBindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::UnBindDeviceToAudioZone.
* @tc.number: UnBindDeviceToAudioZone_003
* @tc.desc  : Test UnBindDeviceToAudioZone with device size >= MAX_SIZE.
*/
HWTEST(AudioPolicyUnitTest, UnBindDeviceToAudioZone_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    // Create MAX_SIZE devices to exceed the limit
    for (int i = 0; i < 1024; i++) {
        auto device = std::make_shared<AudioDeviceDescriptor>();
        device->deviceType_ = DEVICE_TYPE_SPEAKER;
        devices.push_back(device);
    }
    int32_t zoneId = 1;

    int32_t ret = server->UnBindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::UnBindDeviceToAudioZone.
* @tc.number: UnBindDeviceToAudioZone_004
* @tc.desc  : Test UnBindDeviceToAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, UnBindDeviceToAudioZone_004, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    devices.push_back(device);
    int32_t zoneId = 1;

    int32_t ret = server->UnBindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::UnBindDeviceToAudioZone.
* @tc.number: UnBindDeviceToAudioZone_005
* @tc.desc  : Test UnBindDeviceToAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, UnBindDeviceToAudioZone_005, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    // First get all zones to find a valid zoneId
    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    server->GetAllAudioZone(descs);

    if (!descs.empty()) {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
        auto device = std::make_shared<AudioDeviceDescriptor>();
        device->deviceType_ = DEVICE_TYPE_SPEAKER;
        devices.push_back(device);
        int32_t zoneId = descs[0]->zoneId_;

        int32_t ret = server->UnBindDeviceToAudioZone(zoneId, devices);
        EXPECT_EQ(ret, SUCCESS);
    }
}

/**
* @tc.name  : Test AudioPolicyServer::EnableAudioZoneChangeReport.
* @tc.number: EnableAudioZoneChangeReport_001
* @tc.desc  : Test EnableAudioZoneChangeReport with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, EnableAudioZoneChangeReport_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool enable = true;

    // Test zoneId = 0
    int32_t ret = server->EnableAudioZoneChangeReport(0, enable);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->EnableAudioZoneChangeReport(-1, enable);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableAudioZoneChangeReport.
* @tc.number: EnableAudioZoneChangeReport_002
* @tc.desc  : Test EnableAudioZoneChangeReport without permission.
*/
HWTEST(AudioPolicyUnitTest, EnableAudioZoneChangeReport_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool enable = true;
    int32_t zoneId = 1;

    int32_t ret = server->EnableAudioZoneChangeReport(zoneId, enable);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableAudioZoneChangeReport.
* @tc.number: EnableAudioZoneChangeReport_003
* @tc.desc  : Test EnableAudioZoneChangeReport with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, EnableAudioZoneChangeReport_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool enable = true;
    int32_t zoneId = 1;

    int32_t ret = server->EnableAudioZoneChangeReport(zoneId, enable);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidToAudioZone.
* @tc.number: AddUidToAudioZone_001
* @tc.desc  : Test AddUidToAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, AddUidToAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 1001;

    // Test zoneId = 0
    int32_t ret = server->AddUidToAudioZone(0, uid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->AddUidToAudioZone(-1, uid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidToAudioZone.
* @tc.number: AddUidToAudioZone_002
* @tc.desc  : Test AddUidToAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, AddUidToAudioZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;

    int32_t ret = server->AddUidToAudioZone(zoneId, uid);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidToAudioZone.
* @tc.number: AddUidToAudioZone_003
* @tc.desc  : Test AddUidToAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, AddUidToAudioZone_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;

    int32_t ret = server->AddUidToAudioZone(zoneId, uid);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidFromAudioZone.
* @tc.number: RemoveUidFromAudioZone_001
* @tc.desc  : Test RemoveUidFromAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, RemoveUidFromAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 1001;

    // Test zoneId = 0
    int32_t ret = server->RemoveUidFromAudioZone(0, uid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->RemoveUidFromAudioZone(-1, uid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidFromAudioZone.
* @tc.number: RemoveUidFromAudioZone_002
* @tc.desc  : Test RemoveUidFromAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, RemoveUidFromAudioZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;

    int32_t ret = server->RemoveUidFromAudioZone(zoneId, uid);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidFromAudioZone.
* @tc.number: RemoveUidFromAudioZone_003
* @tc.desc  : Test RemoveUidFromAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, RemoveUidFromAudioZone_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;

    int32_t ret = server->RemoveUidFromAudioZone(zoneId, uid);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_001
* @tc.desc  : Test AddUidUsagesToAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 1001;
    std::set<int32_t> usages = {1, 2};

    // Test zoneId = 0
    int32_t ret = server->AddUidUsagesToAudioZone(0, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->AddUidUsagesToAudioZone(-1, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_002
* @tc.desc  : Test AddUidUsagesToAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {1, 2};

    int32_t ret = server->AddUidUsagesToAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_003
* @tc.desc  : Test AddUidUsagesToAudioZone with empty usages.
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages; // Empty set

    int32_t ret = server->AddUidUsagesToAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_004
* @tc.desc  : Test AddUidUsagesToAudioZone with usages size > MAX_STREAM_USAGE_COUNT.
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_004, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages;
    // Create usages set larger than MAX_STREAM_USAGE_COUNT (23)
    for (int i = 1; i <= 24; i++) {
        usages.insert(i);
    }

    int32_t ret = server->AddUidUsagesToAudioZone(zoneId, uid, usages);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_005
* @tc.desc  : Test AddUidUsagesToAudioZone with invalid stream usage (STREAM_USAGE_UNKNOWN).
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_005, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {0}; // STREAM_USAGE_UNKNOWN = 0

    int32_t ret = server->AddUidUsagesToAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_006
* @tc.desc  : Test AddUidUsagesToAudioZone with invalid stream usage (> STREAM_USAGE_MAX).
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_006, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {23}; // > STREAM_USAGE_MAX (22)

    int32_t ret = server->AddUidUsagesToAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddUidUsagesToAudioZone.
* @tc.number: AddUidUsagesToAudioZone_007
* @tc.desc  : Test AddUidUsagesToAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, AddUidUsagesToAudioZone_007, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {1, 2}; // STREAM_USAGE_MEDIA, STREAM_USAGE_VOICE_COMMUNICATION

    int32_t ret = server->AddUidUsagesToAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_001
* @tc.desc  : Test RemoveUidUsagesFromAudioZone with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 1001;
    std::set<int32_t> usages = {1, 2};

    // Test zoneId = 0
    int32_t ret = server->RemoveUidUsagesFromAudioZone(0, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->RemoveUidUsagesFromAudioZone(-1, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_002
* @tc.desc  : Test RemoveUidUsagesFromAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {1, 2};

    int32_t ret = server->RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_003
* @tc.desc  : Test RemoveUidUsagesFromAudioZone with empty usages.
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages; // Empty set

    int32_t ret = server->RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_004
* @tc.desc  : Test RemoveUidUsagesFromAudioZone with usages size > MAX_STREAM_USAGE_COUNT.
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_004, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages;
    // Create usages set larger than MAX_STREAM_USAGE_COUNT (23)
    for (int i = 1; i <= 24; i++) {
        usages.insert(i);
    }

    int32_t ret = server->RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_005
* @tc.desc  : Test RemoveUidUsagesFromAudioZone with invalid stream usage (STREAM_USAGE_UNKNOWN).
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_005, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {0}; // STREAM_USAGE_UNKNOWN = 0

    int32_t ret = server->RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_006
* @tc.desc  : Test RemoveUidUsagesFromAudioZone with invalid stream usage (> STREAM_USAGE_MAX).
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_006, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {23}; // > STREAM_USAGE_MAX (22)

    int32_t ret = server->RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveUidUsagesFromAudioZone.
* @tc.number: RemoveUidUsagesFromAudioZone_007
* @tc.desc  : Test RemoveUidUsagesFromAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, RemoveUidUsagesFromAudioZone_007, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    int32_t uid = 1001;
    std::set<int32_t> usages = {1, 2}; // STREAM_USAGE_MEDIA, STREAM_USAGE_VOICE_COMMUNICATION

    int32_t ret = server->RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddStreamToAudioZone.
* @tc.number: AddStreamToAudioZone_001
* @tc.desc  : Test AddStreamToAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, AddStreamToAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    AudioZoneStream stream;
    stream.streamUsage = STREAM_USAGE_MEDIA;
    stream.sourceType = SOURCE_TYPE_MIC;
    stream.isPlay = true;

    int32_t ret = server->AddStreamToAudioZone(zoneId, stream);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddStreamToAudioZone.
* @tc.number: AddStreamToAudioZone_002
* @tc.desc  : Test AddStreamToAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, AddStreamToAudioZone_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    AudioZoneStream stream;
    stream.streamUsage = STREAM_USAGE_MEDIA;
    stream.sourceType = SOURCE_TYPE_MIC;
    stream.isPlay = true;

    int32_t ret = server->AddStreamToAudioZone(zoneId, stream);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveStreamFromAudioZone.
* @tc.number: RemoveStreamFromAudioZone_001
* @tc.desc  : Test RemoveStreamFromAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, RemoveStreamFromAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    AudioZoneStream stream;
    stream.streamUsage = STREAM_USAGE_MEDIA;
    stream.sourceType = SOURCE_TYPE_MIC;
    stream.isPlay = true;

    int32_t ret = server->RemoveStreamFromAudioZone(zoneId, stream);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveStreamFromAudioZone.
* @tc.number: RemoveStreamFromAudioZone_002
* @tc.desc  : Test RemoveStreamFromAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, RemoveStreamFromAudioZone_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    AudioZoneStream stream;
    stream.streamUsage = STREAM_USAGE_MEDIA;
    stream.sourceType = SOURCE_TYPE_MIC;
    stream.isPlay = true;

    int32_t ret = server->RemoveStreamFromAudioZone(zoneId, stream);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::AddStreamsToAudioZone.
* @tc.number: AddStreamsToAudioZone_001
* @tc.desc  : Test AddStreamsToAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, AddStreamsToAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    std::vector<AudioZoneStream> streams;
    AudioZoneStream stream1;
    stream1.streamUsage = STREAM_USAGE_MEDIA;
    stream1.sourceType = SOURCE_TYPE_MIC;
    stream1.isPlay = true;
    streams.push_back(stream1);

    int32_t ret = server->AddStreamsToAudioZone(zoneId, streams);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::AddStreamsToAudioZone.
* @tc.number: AddStreamsToAudioZone_002
* @tc.desc  : Test AddStreamsToAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, AddStreamsToAudioZone_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    std::vector<AudioZoneStream> streams;
    AudioZoneStream stream1;
    stream1.streamUsage = STREAM_USAGE_MEDIA;
    stream1.sourceType = SOURCE_TYPE_MIC;
    stream1.isPlay = true;
    streams.push_back(stream1);

    int32_t ret = server->AddStreamsToAudioZone(zoneId, streams);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveStreamsFromAudioZone.
* @tc.number: RemoveStreamsFromAudioZone_001
* @tc.desc  : Test RemoveStreamsFromAudioZone without permission.
*/
HWTEST(AudioPolicyUnitTest, RemoveStreamsFromAudioZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    std::vector<AudioZoneStream> streams;
    AudioZoneStream stream1;
    stream1.streamUsage = STREAM_USAGE_MEDIA;
    stream1.sourceType = SOURCE_TYPE_MIC;
    stream1.isPlay = true;
    streams.push_back(stream1);

    int32_t ret = server->RemoveStreamsFromAudioZone(zoneId, streams);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::RemoveStreamsFromAudioZone.
* @tc.number: RemoveStreamsFromAudioZone_002
* @tc.desc  : Test RemoveStreamsFromAudioZone with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, RemoveStreamsFromAudioZone_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    std::vector<AudioZoneStream> streams;
    AudioZoneStream stream1;
    stream1.streamUsage = STREAM_USAGE_MEDIA;
    stream1.sourceType = SOURCE_TYPE_MIC;
    stream1.isPlay = true;
    streams.push_back(stream1);

    int32_t ret = server->RemoveStreamsFromAudioZone(zoneId, streams);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::SetZoneDeviceVisible.
* @tc.number: SetZoneDeviceVisible_001
* @tc.desc  : Test SetZoneDeviceVisible without permission.
*/
HWTEST(AudioPolicyUnitTest, SetZoneDeviceVisible_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool visible = true;
    int32_t ret = server->SetZoneDeviceVisible(visible);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::SetZoneDeviceVisible.
* @tc.number: SetZoneDeviceVisible_002
* @tc.desc  : Test SetZoneDeviceVisible with valid parameters.
*/
HWTEST(AudioPolicyUnitTest, SetZoneDeviceVisible_002, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool visible = true;
    int32_t ret = server->SetZoneDeviceVisible(visible);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::SetZoneDeviceVisible.
* @tc.number: SetZoneDeviceVisible_003
* @tc.desc  : Test SetZoneDeviceVisible with visible = false.
*/
HWTEST(AudioPolicyUnitTest, SetZoneDeviceVisible_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool visible = false;
    int32_t ret = server->SetZoneDeviceVisible(visible);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableSystemVolumeProxy.
* @tc.number: EnableSystemVolumeProxy_001
* @tc.desc  : Test EnableSystemVolumeProxy with invalid zoneId (zoneId <= 0).
*/
HWTEST(AudioPolicyUnitTest, EnableSystemVolumeProxy_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool enable = true;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    // Test zoneId = 0
    int32_t ret = server->EnableSystemVolumeProxy(0, deviceType, enable);
    EXPECT_NE(ret, ERR_INVALID_PARAM);

    // Test zoneId < 0
    ret = server->EnableSystemVolumeProxy(-1, deviceType, enable);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableSystemVolumeProxy.
* @tc.number: EnableSystemVolumeProxy_002
* @tc.desc  : Test EnableSystemVolumeProxy without permission.
*/
HWTEST(AudioPolicyUnitTest, EnableSystemVolumeProxy_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    bool enable = true;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t ret = server->EnableSystemVolumeProxy(zoneId, deviceType, enable);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableSystemVolumeProxy.
* @tc.number: EnableSystemVolumeProxy_003
* @tc.desc  : Test EnableSystemVolumeProxy with valid parameters (enable = true).
*/
HWTEST(AudioPolicyUnitTest, EnableSystemVolumeProxy_003, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    bool enable = true;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t ret = server->EnableSystemVolumeProxy(zoneId, deviceType, enable);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableSystemVolumeProxy.
* @tc.number: EnableSystemVolumeProxy_004
* @tc.desc  : Test EnableSystemVolumeProxy with valid parameters (enable = false).
*/
HWTEST(AudioPolicyUnitTest, EnableSystemVolumeProxy_004, TestSize.Level1)
{
    GetPermission();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t zoneId = 1;
    bool enable = false;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t ret = server->EnableSystemVolumeProxy(zoneId, deviceType, enable);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioInterruptForZone.
* @tc.number: GetAudioInterruptForZone_001
* @tc.desc  : Test GetAudioInterruptForZone with invalid zoneId (zoneId < 0).
*/
HWTEST(AudioPolicyUnitTest, GetAudioInterruptForZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::map<AudioInterrupt, int32_t>> retList;

    // Test zoneId < 0
    int32_t ret = server->GetAudioInterruptForZone(-1, retList);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioInterruptForZone.
* @tc.number: GetAudioInterruptForZone_002
* @tc.desc  : Test GetAudioInterruptForZone with valid zoneId.
*/
HWTEST(AudioPolicyUnitTest, GetAudioInterruptForZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::map<AudioInterrupt, int32_t>> retList;
    int32_t zoneId = 0;

    int32_t ret = server->GetAudioInterruptForZone(zoneId, retList);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioInterruptForZone.
* @tc.number: GetAudioInterruptForZone_003
* @tc.desc  : Test GetAudioInterruptForZone with deviceTag.
*/
HWTEST(AudioPolicyUnitTest, GetAudioInterruptForZone_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::map<AudioInterrupt, int32_t>> retList;
    int32_t zoneId = 1;
    std::string deviceTag = "testDevice";

    int32_t ret = server->GetAudioInterruptForZone(zoneId, deviceTag, retList);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::GetAudioInterruptForZone.
* @tc.number: GetAudioInterruptForZone_004
* @tc.desc  : Test GetAudioInterruptForZone with invalid zoneId and deviceTag.
*/
HWTEST(AudioPolicyUnitTest, GetAudioInterruptForZone_004, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::vector<std::map<AudioInterrupt, int32_t>> retList;
    int32_t zoneId = -1;
    std::string deviceTag = "testDevice";

    int32_t ret = server->GetAudioInterruptForZone(zoneId, deviceTag, retList);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::GetCurrentRendererChangeInfosForZone.
* @tc.number: GetCurrentRendererChangeInfosForZone_001
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone with invalid zoneId.
*/
HWTEST(AudioPolicyUnitTest, GetCurrentRendererChangeInfosForZone_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = server->coreService_->GetEventEntry();

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = server->GetCurrentRendererChangeInfosForZone(-1, infos);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer::GetCurrentRendererChangeInfosForZone.
* @tc.number: GetCurrentRendererChangeInfosForZone_002
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone with valid zoneId (primary zone) when no renderers.
*/
HWTEST(AudioPolicyUnitTest, GetCurrentRendererChangeInfosForZone_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    if (server->coreService_ != nullptr) {
        server->eventEntry_ = server->coreService_->GetEventEntry();
    }

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = server->GetCurrentRendererChangeInfosForZone(0, infos);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyServer::GetCurrentRendererChangeInfosForZone.
* @tc.number: GetCurrentRendererChangeInfosForZone_003
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone with non-existent zoneId.
*             Should return SUCCESS with empty infos.
*/
HWTEST(AudioPolicyUnitTest, GetCurrentRendererChangeInfosForZone_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    if (server->coreService_ != nullptr) {
        server->eventEntry_ = server->coreService_->GetEventEntry();
    }

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    const int32_t nonExistentZoneId = 9999;
    int32_t ret = server->GetCurrentRendererChangeInfosForZone(nonExistentZoneId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_TRUE(infos.empty());
}

/**
* @tc.name  : Test AudioPolicyServer::GetCurrentRendererChangeInfosForZone.
* @tc.number: GetCurrentRendererChangeInfosForZone_004
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone with renderer info and zone binding.
*             Create zone, bind UID, add renderer with matching clientUID, verify returned info.
*/
HWTEST(AudioPolicyUnitTest, GetCurrentRendererChangeInfosForZone_004, TestSize.Level1)
{
    GetPermission();
    auto server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioZoneService::GetInstance().Init(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    // Create a test zone
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestRendererZone", context, 0);
    ASSERT_NE(zoneId, ERROR);

    // Bind UID to zone with specific stream usage
    const int32_t testClientUID = 1234;
    const StreamUsage testStreamUsage = STREAM_USAGE_MEDIA;
    std::set<StreamUsage> usages = {testStreamUsage};
    int32_t ret = AudioZoneService::GetInstance().AddUidUsagesToAudioZone(zoneId, testClientUID, usages);
    EXPECT_EQ(SUCCESS, ret);

    // Verify FindAudioZone returns correct zoneId
    int32_t foundZoneId = AudioZoneService::GetInstance().FindAudioZone(testClientUID, testStreamUsage);
    EXPECT_EQ(foundZoneId, zoneId);

    // Create mock renderer change info
    AudioStreamCollector& collector = AudioStreamCollector::GetAudioStreamCollector();
    auto rendererInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererInfo->clientUID = testClientUID;
    rendererInfo->createrUID = testClientUID;
    rendererInfo->sessionId = 2001;
    rendererInfo->clientPid = 3001;
    rendererInfo->rendererState = RENDERER_RUNNING;
    rendererInfo->rendererInfo.streamUsage = testStreamUsage;
    rendererInfo->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    rendererInfo->channelCount = CHANNEL_STEREO;

    // Add renderer to collector
    {
        std::lock_guard<std::mutex> lock(collector.streamsInfoMutex_);
        collector.audioRendererChangeInfos_.push_back(rendererInfo);
    }

    // Get renderer info for the zone
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    ret = server->GetCurrentRendererChangeInfosForZone(zoneId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(infos.size(), 1);
    if (infos.size() > 0) {
        EXPECT_EQ(infos[0]->clientUID, testClientUID);
        EXPECT_EQ(infos[0]->sessionId, 2001);
        EXPECT_EQ(infos[0]->rendererState, RENDERER_RUNNING);
        EXPECT_EQ(infos[0]->rendererInfo.streamUsage, testStreamUsage);
    }

    // Cleanup: remove renderer and release zone
    {
        std::lock_guard<std::mutex> lock(collector.streamsInfoMutex_);
        collector.audioRendererChangeInfos_.clear();
    }
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
* @tc.name  : Test AudioPolicyServer::GetCurrentRendererChangeInfosForZone.
* @tc.number: GetCurrentRendererChangeInfosForZone_005
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone with multiple renderers in different zones.
*             Create two zones with different UIDs, add renderers, verify correct filtering.
*/
HWTEST(AudioPolicyUnitTest, GetCurrentRendererChangeInfosForZone_005, TestSize.Level1)
{
    GetPermission();
    auto server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioZoneService::GetInstance().Init(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    // Setup: create zones and bind UIDs
    AudioZoneContext context;
    int32_t zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("Zone1", context, 0);
    int32_t zoneId2 = AudioZoneService::GetInstance().CreateAudioZone("Zone2", context, 0);
    ASSERT_NE(zoneId1, ERROR);
    ASSERT_NE(zoneId2, ERROR);

    const int32_t uid1 = 1001;
    const int32_t uid2 = 2002;
    AudioZoneService::GetInstance().AddUidUsagesToAudioZone(zoneId1, uid1, {STREAM_USAGE_MEDIA});
    AudioZoneService::GetInstance().AddUidUsagesToAudioZone(zoneId2, uid2, {STREAM_USAGE_MUSIC});

    // Add mock renderers
    auto renderer1 = CreateRendererChangeInfo(uid1, 1001, RENDERER_RUNNING, STREAM_USAGE_MEDIA);
    auto renderer2 = CreateRendererChangeInfo(uid2, 2002, RENDERER_RUNNING, STREAM_USAGE_MUSIC);
    AddRenderersToCollector({renderer1, renderer2});

    // Verify: zone1 only contains renderer1
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos1;
    EXPECT_EQ(server->GetCurrentRendererChangeInfosForZone(zoneId1, infos1), SUCCESS);
    EXPECT_EQ(infos1.size(), 1);
    if (!infos1.empty()) {
        EXPECT_EQ(infos1[0]->clientUID, uid1);
        EXPECT_EQ(infos1[0]->sessionId, 1001);
    }

    // Verify: zone2 only contains renderer2
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos2;
    EXPECT_EQ(server->GetCurrentRendererChangeInfosForZone(zoneId2, infos2), SUCCESS);
    EXPECT_EQ(infos2.size(), 1);
    if (!infos2.empty()) {
        EXPECT_EQ(infos2[0]->clientUID, uid2);
        EXPECT_EQ(infos2[0]->sessionId, 2002);
    }

    // Cleanup
    ClearRenderersFromCollector();
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId2);
}

/**
* @tc.name  : Test AudioPolicyServer::GetCurrentRendererChangeInfosForZone.
* @tc.number: GetCurrentRendererChangeInfosForZone_006
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone with renderer not bound to any zone.
*             Renderer with unbound UID should return to default zone (zoneId=0).
*/
HWTEST(AudioPolicyUnitTest, GetCurrentRendererChangeInfosForZone_006, TestSize.Level1)
{
    GetPermission();
    auto server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioZoneService::GetInstance().Init(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());

    // Create renderer with UID not bound to any zone
    const int32_t unboundUID = 9999;
    AudioStreamCollector& collector = AudioStreamCollector::GetAudioStreamCollector();
    auto renderer = std::make_shared<AudioRendererChangeInfo>();
    renderer->clientUID = unboundUID;
    renderer->createrUID = unboundUID;
    renderer->sessionId = 9999;
    renderer->rendererState = RENDERER_RUNNING;
    renderer->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    renderer->channelCount = CHANNEL_STEREO;

    {
        std::lock_guard<std::mutex> lock(collector.streamsInfoMutex_);
        collector.audioRendererChangeInfos_.push_back(renderer);
    }

    // FindAudioZone should return 0 (default zone) for unbound UID
    int32_t foundZoneId = AudioZoneService::GetInstance().FindAudioZone(unboundUID, STREAM_USAGE_MEDIA);
    EXPECT_EQ(foundZoneId, 0);

    // Query default zone - should get the unbound renderer
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = server->GetCurrentRendererChangeInfosForZone(0, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_GE(infos.size(), 1);
    
    // Verify unbound renderer is in the results
    bool foundUnbound = false;
    for (const auto& info : infos) {
        if (info->clientUID == unboundUID) {
            foundUnbound = true;
            EXPECT_EQ(info->sessionId, 9999);
            break;
        }
    }
    EXPECT_TRUE(foundUnbound);

    // Cleanup
    {
        std::lock_guard<std::mutex> lock(collector.streamsInfoMutex_);
        collector.audioRendererChangeInfos_.clear();
    }
}

/**
* @tc.name  : Test AudioPolicyServer::GetMaxAmplitude.
* @tc.number: GetMaxAmplitude_001
* @tc.desc  : Test GetMaxAmplitude with valid deviceId.
*/
HWTEST(AudioPolicyUnitTest, GetMaxAmplitude_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t deviceId = 0;
    float ret = 0.0f;

    int32_t result = server->GetMaxAmplitude(deviceId, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::IsHeadTrackingDataRequested.
* @tc.number: IsHeadTrackingDataRequested_001
* @tc.desc  : Test IsHeadTrackingDataRequested with valid macAddress.
*/
HWTEST(AudioPolicyUnitTest, IsHeadTrackingDataRequested_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::string macAddress = "AA:BB:CC:DD:EE:FF";
    bool ret = false;

    int32_t result = server->IsHeadTrackingDataRequested(macAddress, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::IsHeadTrackingDataRequested.
* @tc.number: IsHeadTrackingDataRequested_002
* @tc.desc  : Test IsHeadTrackingDataRequested with empty macAddress.
*/
HWTEST(AudioPolicyUnitTest, IsHeadTrackingDataRequested_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::string macAddress = "";
    bool ret = false;

    int32_t result = server->IsHeadTrackingDataRequested(macAddress, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::NotifyAccountsChanged.
* @tc.number: NotifyAccountsChanged_001
* @tc.desc  : Test NotifyAccountsChanged with interruptService_ as nullptr.
*/
HWTEST(AudioPolicyUnitTest, NotifyAccountsChanged_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = nullptr;
    int id = 100;
    int oldId = 0;

    server->NotifyAccountsChanged(id, oldId);
    EXPECT_EQ(server->interruptService_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyServer::NotifyAccountsChanged.
* @tc.number: NotifyAccountsChanged_002
* @tc.desc  : Test NotifyAccountsChanged with valid interruptService_.
*/
HWTEST(AudioPolicyUnitTest, NotifyAccountsChanged_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    int id = 100;
    int oldId = 0;

    server->NotifyAccountsChanged(id, oldId);
    EXPECT_NE(server->interruptService_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyServer::CheckHibernateState.
* @tc.number: CheckHibernateState_001
* @tc.desc  : Test CheckHibernateState with hibernate true.
*/
HWTEST(AudioPolicyUnitTest, CheckHibernateState_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool hibernate = true;
    server->CheckHibernateState(hibernate);
}

/**
* @tc.name  : Test AudioPolicyServer::CheckHibernateState.
* @tc.number: CheckHibernateState_002
* @tc.desc  : Test CheckHibernateState with hibernate false.
*/
HWTEST(AudioPolicyUnitTest, CheckHibernateState_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool hibernate = false;
    server->CheckHibernateState(hibernate);
}

/**
* @tc.name  : Test AudioPolicyServer::UpdateSafeVolumeByS4.
* @tc.number: UpdateSafeVolumeByS4_001
* @tc.desc  : Test UpdateSafeVolumeByS4.
*/
HWTEST(AudioPolicyUnitTest, UpdateSafeVolumeByS4_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->UpdateSafeVolumeByS4();
}

/**
* @tc.name  : Test AudioPolicyServer::CheckConnectedDevice.
* @tc.number: CheckConnectedDevice_001
* @tc.desc  : Test CheckConnectedDevice.
*/
HWTEST(AudioPolicyUnitTest, CheckConnectedDevice_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->CheckConnectedDevice();
}

/**
* @tc.name  : Test AudioPolicyServer::SetDeviceConnectedFlagFalseAfterDuration.
* @tc.number: SetDeviceConnectedFlagFalseAfterDuration_001
* @tc.desc  : Test SetDeviceConnectedFlagFalseAfterDuration.
*/
HWTEST(AudioPolicyUnitTest, SetDeviceConnectedFlagFalseAfterDuration_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->SetDeviceConnectedFlagFalseAfterDuration();
}

/**
* @tc.name  : Test AudioPolicyServer::IsOtherMediaPlaying.
* @tc.number: IsOtherMediaPlaying_001
* @tc.desc  : Test IsOtherMediaPlaying with interruptService_ as nullptr.
*/
HWTEST(AudioPolicyUnitTest, IsOtherMediaPlaying_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = nullptr;
    bool isExistence = true;

    int32_t ret = server->IsOtherMediaPlaying(isExistence);
    EXPECT_EQ(ret, ERR_MEMORY_ALLOC_FAILED);
    EXPECT_EQ(isExistence, false);
}

/**
* @tc.name  : Test AudioPolicyServer::IsOtherMediaPlaying.
* @tc.number: IsOtherMediaPlaying_002
* @tc.desc  : Test IsOtherMediaPlaying with valid interruptService_.
*/
HWTEST(AudioPolicyUnitTest, IsOtherMediaPlaying_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    bool isExistence = false;

    int32_t ret = server->IsOtherMediaPlaying(isExistence);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableMuteSuggestionWhenMixWithOthers.
* @tc.number: EnableMuteSuggestionWhenMixWithOthers_001
* @tc.desc  : Test EnableMuteSuggestionWhenMixWithOthers with interruptService_ as nullptr.
*/
HWTEST(AudioPolicyUnitTest, EnableMuteSuggestionWhenMixWithOthers_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = nullptr;
    bool enable = true;

    int32_t ret = server->EnableMuteSuggestionWhenMixWithOthers(enable);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableMuteSuggestionWhenMixWithOthers.
* @tc.number: EnableMuteSuggestionWhenMixWithOthers_002
* @tc.desc  : Test EnableMuteSuggestionWhenMixWithOthers with valid interruptService_ but no audio session.
*/
HWTEST(AudioPolicyUnitTest, EnableMuteSuggestionWhenMixWithOthers_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    bool enable = true;

    int32_t ret = server->EnableMuteSuggestionWhenMixWithOthers(enable);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test AudioPolicyServer::EnableMuteSuggestionWhenMixWithOthers.
* @tc.number: EnableMuteSuggestionWhenMixWithOthers_003
* @tc.desc  : Test EnableMuteSuggestionWhenMixWithOthers with valid interruptService_ but no audio session.
*/
HWTEST(AudioPolicyUnitTest, EnableMuteSuggestionWhenMixWithOthers_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    bool enable = false;

    int32_t ret = server->EnableMuteSuggestionWhenMixWithOthers(enable);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test AudioPolicyServer::SetVoiceRingtoneMute.
* @tc.number: SetVoiceRingtoneMute_001
* @tc.desc  : Test SetVoiceRingtoneMute with invalid callerUid.
*/
HWTEST(AudioPolicyUnitTest, SetVoiceRingtoneMute_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool isMute = true;

    int32_t ret = server->SetVoiceRingtoneMute(isMute);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::SetVoiceRingtoneMute.
* @tc.number: SetVoiceRingtoneMute_002
* @tc.desc  : Test SetVoiceRingtoneMute with isMute true.
*/
HWTEST(AudioPolicyUnitTest, SetVoiceRingtoneMute_002, TestSize.Level1)
{
    uint64_t tokenId;
    constexpr int perNum = 2;
    const char *perms[perNum] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "call_manager_test",
        .aplStr = "system_core",
    };
    infoInstance.dcaps = nullptr;
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    bool isMute = true;

    int32_t ret = server->SetVoiceRingtoneMute(isMute);
    // Reset token
    GetPermission();
}

/**
* @tc.name  : Test AudioPolicyServer::NotifySessionStateChange.
* @tc.number: NotifySessionStateChange_001
* @tc.desc  : Test NotifySessionStateChange with invalid callerUid.
*/
HWTEST(AudioPolicyUnitTest, NotifySessionStateChange_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 1000;
    int32_t pid = 100;
    bool hasSession = true;

    int32_t ret = server->NotifySessionStateChange(uid, pid, hasSession);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::NotifySessionStateChange.
* @tc.number: NotifySessionStateChange_002
* @tc.desc  : Test NotifySessionStateChange with hasSession true.
*/
HWTEST(AudioPolicyUnitTest, NotifySessionStateChange_002, TestSize.Level1)
{
    uint64_t tokenId;
    constexpr int perNum = 2;
    const char *perms[perNum] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "avsession_test",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 1000;
    int32_t pid = 100;
    bool hasSession = true;

    int32_t ret = server->NotifySessionStateChange(uid, pid, hasSession);
    // Reset token
    GetPermission();
}

/**
* @tc.name  : Test AudioPolicyServer::NotifyFreezeStateChange.
* @tc.number: NotifyFreezeStateChange_001
* @tc.desc  : Test NotifyFreezeStateChange with invalid callerUid.
*/
HWTEST(AudioPolicyUnitTest, NotifyFreezeStateChange_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::set<int32_t> pidList = {100, 200};
    bool isFreeze = true;

    int32_t ret = server->NotifyFreezeStateChange(pidList, isFreeze);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::NotifyFreezeStateChange.
* @tc.number: NotifyFreezeStateChange_002
* @tc.desc  : Test NotifyFreezeStateChange with pidList size > 100.
*/
HWTEST(AudioPolicyUnitTest, NotifyFreezeStateChange_002, TestSize.Level1)
{
    uint64_t tokenId;
    constexpr int perNum = 2;
    const char *perms[perNum] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "rss_test",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::set<int32_t> pidList;
    for (int i = 0; i <= 100; i++) {
        pidList.insert(i);
    }
    bool isFreeze = true;

    int32_t ret = server->NotifyFreezeStateChange(pidList, isFreeze);
    EXPECT_EQ(ret, ERROR);
    // Reset token
    GetPermission();
}

/**
* @tc.name  : Test AudioPolicyServer::NotifyFreezeStateChange.
* @tc.number: NotifyFreezeStateChange_003
* @tc.desc  : Test NotifyFreezeStateChange with valid pidList and isFreeze true.
*/
HWTEST(AudioPolicyUnitTest, NotifyFreezeStateChange_003, TestSize.Level1)
{
    uint64_t tokenId;
    constexpr int perNum = 2;
    const char *perms[perNum] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "rss_test",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::set<int32_t> pidList = {100, 200};
    bool isFreeze = true;

    int32_t ret = server->NotifyFreezeStateChange(pidList, isFreeze);
    // Reset token
    GetPermission();
}

/**
* @tc.name  : Test AudioPolicyServer::NotifyFreezeStateChange.
* @tc.number: NotifyFreezeStateChange_004
* @tc.desc  : Test NotifyFreezeStateChange with valid pidList and isFreeze false.
*/
HWTEST(AudioPolicyUnitTest, NotifyFreezeStateChange_004, TestSize.Level1)
{
    uint64_t tokenId;
    constexpr int perNum = 2;
    const char *perms[perNum] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "rss_test",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    std::set<int32_t> pidList = {100, 200};
    bool isFreeze = false;

    int32_t ret = server->NotifyFreezeStateChange(pidList, isFreeze);
    // Reset token
    GetPermission();
}

/**
* @tc.name  : Test AudioPolicyServer::SetAudioSessionCapturerMuteHint.
* @tc.number: SetAudioSessionCapturerMuteHint_001
* @tc.desc  : Test SetAudioSessionCapturerMuteHint when there is no running capturer in current session.
*/
HWTEST(AudioPolicyUnitTest, SetAudioSessionCapturerMuteHint_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->SetAudioSessionCapturerMuteHint(true);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test AudioPolicyServer::SetAudioSessionCapturerMuteHint.
* @tc.number: SetAudioSessionCapturerMuteHint_002
* @tc.desc  : Test SetAudioSessionCapturerMuteHint policy service path.
*/
HWTEST(AudioPolicyUnitTest, SetAudioSessionCapturerMuteHint_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->SetAudioSessionCapturerMuteHint(false);
    EXPECT_NE(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer::RegisteredTrackerClientDied.
* @tc.number: RegisteredTrackerClientDied_001
* @tc.desc  : Test tracker death path should trigger mute-hint reevaluation and prune stale state.
*/
HWTEST(AudioPolicyUnitTest, RegisteredTrackerClientDied_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    constexpr uint32_t testStreamId = 900001;
    server->audioEffectService_.SetAppConfiguredCapturerMuteHint(testStreamId, true);
    EXPECT_TRUE(server->audioEffectService_.GetEffectiveAppMuteHint(testStreamId));

    constexpr pid_t nonExistPid = 199991;
    constexpr pid_t nonExistUid = 199992;
    server->RegisteredTrackerClientDied(nonExistPid, nonExistUid);

    EXPECT_FALSE(server->audioEffectService_.GetEffectiveAppMuteHint(testStreamId));
}

/**
* @tc.name  : Test AudioPolicyServer::ResetAllProxy.
* @tc.number: ResetAllProxy_001
* @tc.desc  : Test ResetAllProxy with invalid callerUid.
*/
HWTEST(AudioPolicyUnitTest, ResetAllProxy_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->ResetAllProxy();
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer::ResetAllProxy.
* @tc.number: ResetAllProxy_002
* @tc.desc  : Test ResetAllProxy with valid callerUid.
*/
HWTEST(AudioPolicyUnitTest, ResetAllProxy_002, TestSize.Level1)
{
    uint64_t tokenId;
    constexpr int perNum = 2;
    const char *perms[perNum] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "rss_test",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->ResetAllProxy();
    // Reset token
    GetPermission();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: OnReceiveEvent_002
* @tc.desc  : Test OnReceiveEvent.
*/
HWTEST(AudioPolicyUnitTest, OnReceiveEvent_002, TestSize.Level4)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    EventFwk::CommonEventData eventData;
    OHOS::EventFwk::Want want;
    want.SetAction("usual.event.SCREEN_OFF");
    eventData.SetWant(want);
    audioPolicyServer->OnReceiveEvent(eventData);
    EXPECT_EQ(audioPolicyServer->isScreenOn_, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: IsScreenOnAdjustVolume_001
* @tc.desc  : Test IsScreenOnAdjustVolume.
*/
HWTEST(AudioPolicyUnitTest, IsScreenOnAdjustVolume_001, TestSize.Level4)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOn_ = true;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = true;
    bool ret = audioPolicyServer->IsScreenOnAdjustVolume();
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: IsScreenOnAdjustVolume_002
* @tc.desc  : Test IsScreenOnAdjustVolume.
*/
HWTEST(AudioPolicyUnitTest, IsScreenOnAdjustVolume_002, TestSize.Level4)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOn_ = false;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = true;
    bool ret = audioPolicyServer->IsScreenOnAdjustVolume();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: IsScreenOnAdjustVolume_003
* @tc.desc  : Test IsScreenOnAdjustVolume.
*/
HWTEST(AudioPolicyUnitTest, IsScreenOnAdjustVolume_003, TestSize.Level4)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOn_ = true;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = false;
    bool ret = audioPolicyServer->IsScreenOnAdjustVolume();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: IsScreenOnAdjustVolume_004
* @tc.desc  : Test IsScreenOnAdjustVolume.
*/
HWTEST(AudioPolicyUnitTest, IsScreenOnAdjustVolume_004, TestSize.Level4)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOn_ = false;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = false;
    bool ret = audioPolicyServer->IsScreenOnAdjustVolume();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test ProcessVolumeKeyEvents.
* @tc.number: ProcessVolumeKeyEvents_005
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTest, ProcessVolumeKeyEvents_005, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOffOrLock_ = true;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = true;
    audioPolicyServer->isScreenOn_ = true;
    audioPolicyServer->ProcessVolumeKeyEvents(keyType);
}

/**
* @tc.name  : Test ProcessVolumeKeyEvents.
* @tc.number: ProcessVolumeKeyEvents_006
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTest, ProcessVolumeKeyEvents_006, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOffOrLock_ = false;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = false;
    audioPolicyServer->isScreenOn_ = false;
    audioPolicyServer->ProcessVolumeKeyEvents(keyType);
}

/**
* @tc.name  : Test ProcessVolumeKeyEvents.
* @tc.number: ProcessVolumeKeyEvents_007
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTest, ProcessVolumeKeyEvents_007, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOffOrLock_ = true;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = true;
    audioPolicyServer->isScreenOn_ = false;
    audioPolicyServer->ProcessVolumeKeyEvents(keyType);
}

/**
* @tc.name  : Test ProcessVolumeKeyEvents.
* @tc.number: ProcessVolumeKeyEvents_008
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTest, ProcessVolumeKeyEvents_008, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOffOrLock_ = true;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = false;
    audioPolicyServer->isScreenOn_ = false;
    audioPolicyServer->ProcessVolumeKeyEvents(keyType);
}

/**
* @tc.name  : Test ProcessVolumeKeyEvents.
* @tc.number: ProcessVolumeKeyEvents_009
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTest, ProcessVolumeKeyEvents_009, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(audioPolicyServer, nullptr);
    audioPolicyServer->isScreenOffOrLock_ = false;
    audioPolicyServer->screenOffAdjustVolumeEnable_ = true;
    audioPolicyServer->isScreenOn_ = false;
    audioPolicyServer->ProcessVolumeKeyEvents(keyType);
}

/**
 * @tc.name  : Test AudioPolicyServer.
 * @tc.number: SetAudioZoneVolumeConfig_001
 * @tc.desc  : Call SetAudioZoneVolumeConfig with empty/typical params, ensure it returns a non-error code.
 */
HWTEST(AudioPolicyUnitTest, SetAudioZoneVolumeConfig_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string zoneName = "UnitTestZone";
    std::vector<AudioZoneVolumeConfig> configs; // empty vector is a valid quick-check

    int32_t ret = server->SetAudioZoneVolumeConfig(zoneName, configs);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioPolicyServer.
 * @tc.number: TriggerAudioZoneVolumeSync_001
 * @tc.desc  : Call TriggerAudioZoneVolumeSync and expect SUCCESS (no crash, triggers sync).
 */
HWTEST(AudioPolicyUnitTest, TriggerAudioZoneVolumeSync_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->TriggerAudioZoneVolumeSync();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsAudioLoopbackSupported.
 * @tc.number: IsAudioLoopbackSupported_001
 * @tc.desc  : Test AudioPolicyServer::IsAudioLoopbackSupported with normal device type
 */
HWTEST(AudioPolicyUnitTest, IsAudioLoopbackSupported_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t modeIn = 0;
    int32_t deviceType = static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    bool ret = false;
    int32_t result = ptrAudioPolicyServer->IsAudioLoopbackSupported(modeIn, deviceType, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test IsAudioLoopbackSupported.
 * @tc.number: IsAudioLoopbackSupported_002
 * @tc.desc  : Test AudioPolicyServer::IsAudioLoopbackSupported with NEARLINK device type
 */
HWTEST(AudioPolicyUnitTest, IsAudioLoopbackSupported_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t modeIn = 0;
    int32_t deviceType = static_cast<int32_t>(DEVICE_TYPE_NEARLINK);
    bool ret = false;
    int32_t result = ptrAudioPolicyServer->IsAudioLoopbackSupported(modeIn, deviceType, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test IsAudioLoopbackSupported.
 * @tc.number: IsAudioLoopbackSupported_003
 * @tc.desc  : Test AudioPolicyServer::IsAudioLoopbackSupported with different loopback modes
 */
HWTEST(AudioPolicyUnitTest, IsAudioLoopbackSupported_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t modeIn = 1;
    int32_t deviceType = static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    bool ret = false;
    int32_t result = ptrAudioPolicyServer->IsAudioLoopbackSupported(modeIn, deviceType, ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test VerifyUidAndPid.
* @tc.number: VerifyUidAndPid_001
* @tc.desc  : Test AudioPolicyServer::VerifyUidAndPid
*/
HWTEST(AudioPolicyUnitTest, VerifyUidAndPid_001, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    EXPECT_NE(server, nullptr);

    int32_t pid = 3009;
    int32_t uid = 3009;
    int ret = server->VerifyUidAndPid(uid, pid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable for CALLBACK_ROUTE_SELECTOR.
 * @tc.number: SetClientCallbacksEnable_RouteSelector_001
 * @tc.desc  : Test SetClientCallbacksEnable with CALLBACK_ROUTE_SELECTOR enable.
 */
HWTEST(AudioPolicyUnitTest, SetClientCallbacksEnable_RouteSelector_001, TestSize.Level1)
{
    auto policyServer = GetPolicyServerUnitTest();
    EXPECT_NE(policyServer, nullptr);

    int32_t callbackChange = static_cast<int32_t>(CallbackChange::CALLBACK_ROUTE_SELECTOR);
    int32_t result = policyServer->SetClientCallbacksEnable(callbackChange, true);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable for CALLBACK_ROUTE_SELECTOR.
 * @tc.number: SetClientCallbacksEnable_RouteSelector_002
 * @tc.desc  : Test SetClientCallbacksEnable with CALLBACK_ROUTE_SELECTOR disable.
 */
HWTEST(AudioPolicyUnitTest, SetClientCallbacksEnable_RouteSelector_002, TestSize.Level1)
{
    auto policyServer = GetPolicyServerUnitTest();
    EXPECT_NE(policyServer, nullptr);

    int32_t callbackChange = static_cast<int32_t>(CallbackChange::CALLBACK_ROUTE_SELECTOR);
    int32_t result = policyServer->SetClientCallbacksEnable(callbackChange, false);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable for CALLBACK_ROUTE_SELECTOR with uninitialized server.
 * @tc.number: SetClientCallbacksEnable_RouteSelector_003
 * @tc.desc  : Test SetClientCallbacksEnable with CALLBACK_ROUTE_SELECTOR when server is not fully initialized.
 */
HWTEST(AudioPolicyUnitTest, SetClientCallbacksEnable_RouteSelector_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(server, nullptr);

    int32_t callbackChange = static_cast<int32_t>(CallbackChange::CALLBACK_ROUTE_SELECTOR);
    int32_t result = server->SetClientCallbacksEnable(callbackChange, true);
    EXPECT_NE(result, SUCCESS);
}
} // AudioStandard
} // OHOS
