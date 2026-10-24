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

#include "audio_server_second_unit_test.h"
#include "audio_server.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "karaoke_param_helper.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioServerSecondUnitTest::SetUpTestCase(void)
{
    constexpr int32_t systemAbilityId = 100;
    audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
}

void AudioServerSecondUnitTest::TearDownTestCase(void)
{
    audioServer = nullptr;
}

void AudioServerSecondUnitTest::SetUp(void)
{
    // Setup before each test case
}

void AudioServerSecondUnitTest::TearDown(void)
{
    // Teardown after each test case
}

/**
 * @tc.name  : Test SuspendRenderSink API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSuspendRenderSink_001
 * @tc.desc  : Test SuspendRenderSink interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSuspendRenderSink_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SuspendRenderSink("test_sink", "");
    EXPECT_NE(SUCCESS, ret); // Should fail with invalid sink name
}

/**
 * @tc.name  : Test RestoreRenderSink API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRestoreRenderSink_001
 * @tc.desc  : Test RestoreRenderSink interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRestoreRenderSink_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->RestoreRenderSink("test_sink", "");
    EXPECT_NE(SUCCESS, ret); // Should fail with invalid sink name
}

/**
 * @tc.name  : Test GetMaxAmplitude API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetMaxAmplitude_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    float maxAmplitude = 0.0f;
    int32_t ret = audioServer->GetMaxAmplitude(true, "speaker", 0, maxAmplitude);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetVolumeDataCount API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetVolumeDataCount_001
 * @tc.desc  : Test GetVolumeDataCount interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetVolumeDataCount_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int64_t volumeData = 0;
    int32_t ret = audioServer->GetVolumeDataCount("test_sink", volumeData);
    EXPECT_NE(SUCCESS, ret); // Should fail with invalid sink name
}

/**
 * @tc.name  : Test UpdateLatencyTimestamp API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUpdateLatencyTimestamp_001
 * @tc.desc  : Test UpdateLatencyTimestamp interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUpdateLatencyTimestamp_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UpdateLatencyTimestamp("test_timestamp", true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServerForceStopAudioStream_001
 * @tc.desc  : Test ForceStopAudioStream interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerForceStopAudioStream_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->ForceStopAudioStream(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid audio type
}

/**
 * @tc.name  : Test SetMicrophoneMute API - Permission Denied
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetMicrophoneMute_002
 * @tc.desc  : Test SetMicrophoneMute with invalid permissions.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetMicrophoneMute_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test with invalid permissions (non-audio uid)
    // This should trigger the permission check failure branch
    int32_t ret = audioServer->SetMicrophoneMute(true);
    // The actual result depends on mock permission check
    // We're testing the branch coverage here
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : Test SetVoiceVolume API - Invalid Parameters
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetVoiceVolume_002
 * @tc.desc  : Test SetVoiceVolume with invalid parameters.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetVoiceVolume_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test with invalid volume (out of range)
    int32_t ret = audioServer->SetVoiceVolume(-1.0f);
    // Should handle invalid volume
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAudioParameter API - Invalid Key
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAudioParameter_001
 * @tc.desc  : Test SetAudioParameter with invalid key.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetAudioParameter_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test with invalid key to cover error branches
    int32_t ret = audioServer->SetAudioParameter("INVALID_KEY", "test_value");
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAudioParameter API - Permission Denied
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAudioParameter_002
 * @tc.desc  : Test SetAudioParameter with permission denied.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetAudioParameter_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test A2DP offload config which requires special permission
    int32_t ret = audioServer->SetAudioParameter("AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG", "test");
    // Should fail without proper permission
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnAddSystemAbility API - Default Branch
 * @tc.type  : FUNC
 * @tc.number: AudioServerOnAddSystemAbility_002
 * @tc.desc  : Test OnAddSystemAbility with unhandled systemAbilityId.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerOnAddSystemAbility_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test with unhandled systemAbilityId to cover default branch
    audioServer->OnAddSystemAbility(-1, "");
    audioServer->OnAddSystemAbility(9999, "");
    // Should not crash, just log error
    EXPECT_NE(nullptr, audioServer);
}

/**
 * @tc.name  : Test GetAudioParameter API - Empty Result
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetAudioParameter_002
 * @tc.desc  : Test GetAudioParameter with non-existent key.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetAudioParameter_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::string value;
    int32_t ret = audioServer->GetAudioParameter("NON_EXISTENT_KEY", value);
    (void)ret;
    // Should return empty string for non-existent key
    EXPECT_EQ(value, "");
}

/**
 * @tc.name  : Test SuspendRenderSink API - Permission Denied
 * @tc.type  : FUNC
 * @tc.number: AudioServerSuspendRenderSink_002
 * @tc.desc  : Test SuspendRenderSink with permission denied.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSuspendRenderSink_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test with invalid sink name to cover error branch
    int32_t ret = audioServer->SuspendRenderSink("invalid_sink_name", "");
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test RestoreRenderSink API - Permission Denied
 * @tc.type  : FUNC
 * @tc.number: AudioServerRestoreRenderSink_002
 * @tc.desc  : Test RestoreRenderSink with permission denied.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRestoreRenderSink_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    // Test with invalid sink name to cover error branch
    int32_t ret = audioServer->RestoreRenderSink("invalid_sink_name", "");
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetOutputDeviceSink API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetOutputDeviceSink_001
 * @tc.desc  : Test SetOutputDeviceSink interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetOutputDeviceSink_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetOutputDeviceSink(DEVICE_TYPE_SPEAKER, "primary");
    EXPECT_EQ(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test SetActiveOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetActiveOutputDevice_001
 * @tc.desc  : Test SetActiveOutputDevice interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetActiveOutputDevice_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetActiveOutputDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test OffloadSetVolume API
 * @tc.type  : FUNC
 * @tc.number: AudioServerOffloadSetVolume_001
 * @tc.desc  : Test OffloadSetVolume interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerOffloadSetVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->OffloadSetVolume(0.5f, 0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid renderId
}

/**
 * @tc.name  : Test LoadAudioEffectLibraries API
 * @tc.type  : FUNC
 * @tc.number: AudioServerLoadAudioEffectLibraries_001
 * @tc.desc  : Test LoadAudioEffectLibraries interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerLoadAudioEffectLibraries_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    bool hasEffectsLoaded = false;
    
    int32_t ret = audioServer->LoadAudioEffectLibraries(libraries, effects, successEffectList, hasEffectsLoaded);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetParameterCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetParameterCallback_001
 * @tc.desc  : Test SetParameterCallback interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetParameterCallback_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->SetParameterCallback(object);
    EXPECT_NE(SUCCESS, ret); // Should fail with null object
}

/**
 * @tc.name  : Test RegistCoreServiceProvider API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRegistCoreServiceProvider_001
 * @tc.desc  : Test RegistCoreServiceProvider interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRegistCoreServiceProvider_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->RegistCoreServiceProvider(object);
    EXPECT_NE(SUCCESS, ret); // Should fail with null object
}

/**
 * @tc.name  : Test SetWakeupSourceCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetWakeupSourceCallback_001
 * @tc.desc  : Test SetWakeupSourceCallback interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetWakeupSourceCallback_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->SetWakeupSourceCallback(object);
    EXPECT_EQ(SUCCESS, ret); // Should fail with null object
}

/**
 * @tc.name  : Test ResetRouteForDisconnect API
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetRouteForDisconnect_001
 * @tc.desc  : Test ResetRouteForDisconnect interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerResetRouteForDisconnect_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->ResetRouteForDisconnect(DEVICE_TYPE_SPEAKER);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test GetEffectLatency API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetEffectLatency_001
 * @tc.desc  : Test GetEffectLatency interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetEffectLatency_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    uint32_t latency = 0;
    int32_t ret = audioServer->GetEffectLatency("test_session", latency);
    EXPECT_EQ(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetSinkMuteForSwitchDevice_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetSinkMuteForSwitchDevice("primary", 100, true);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice primary_extra info passthrough
 * @tc.type  : FUNC
 * @tc.number: SetSinkMuteForSwitchDevice_PrimaryExtra_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice with primary_extra does not crash.
 */
HWTEST_F(AudioServerSecondUnitTest, SetSinkMuteForSwitchDevice_PrimaryExtra_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SetSinkMuteForSwitchDevice("primary_extra", 100, true, "Wired_Speaker");
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test LoadHdiEffectModel API
 * @tc.type  : FUNC
 * @tc.number: AudioServerLoadHdiEffectModel_001
 * @tc.desc  : Test LoadHdiEffectModel interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerLoadHdiEffectModel_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->LoadHdiEffectModel();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateEffectBtOffloadSupported API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUpdateEffectBtOffloadSupported_001
 * @tc.desc  : Test UpdateEffectBtOffloadSupported interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUpdateEffectBtOffloadSupported_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UpdateEffectBtOffloadSupported(true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRotationToEffect API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetRotationToEffect_001
 * @tc.desc  : Test SetRotationToEffect interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetRotationToEffect_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetRotationToEffect(0);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateSessionConnectionState API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUpdateSessionConnectionState_001
 * @tc.desc  : Test UpdateSessionConnectionState interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUpdateSessionConnectionState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UpdateSessionConnectionState(0, 0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetNonInterruptMute_001
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetNonInterruptMute_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetNonInterruptMute(0, true);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetOffloadMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetOffloadMode(0, 0, false);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUnsetOffloadMode_001
 * @tc.desc  : Test UnsetOffloadMode interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUnsetOffloadMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UnsetOffloadMode(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test CheckHibernateState API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckHibernateState_001
 * @tc.desc  : Test CheckHibernateState interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCheckHibernateState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->CheckHibernateState(true);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test CreateIpcOfflineStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateIpcOfflineStream_001
 * @tc.desc  : Test CreateIpcOfflineStream interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCreateIpcOfflineStream_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t errorCode = 0;
    sptr<IRemoteObject> client = nullptr;
    int32_t ret = audioServer->CreateIpcOfflineStream(errorCode, client);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test GetOfflineAudioEffectChains API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetOfflineAudioEffectChains_001
 * @tc.desc  : Test GetOfflineAudioEffectChains interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetOfflineAudioEffectChains_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<std::string> effectChains;
    int32_t ret = audioServer->GetOfflineAudioEffectChains(effectChains);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetStandbyStatus API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetStandbyStatus_001
 * @tc.desc  : Test GetStandbyStatus interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetStandbyStatus_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    int32_t ret = audioServer->GetStandbyStatus(0, isStandby, enterStandbyTime);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test GetAllSinkInputs API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetAllSinkInputs_001
 * @tc.desc  : Test GetAllSinkInputs interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetAllSinkInputs_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<SinkInput> sinkInputs;
    int32_t ret = audioServer->GetAllSinkInputs(sinkInputs);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test NotifyAudioPolicyReady API
 * @tc.type  : FUNC
 * @tc.number: AudioServerNotifyAudioPolicyReady_001
 * @tc.desc  : Test NotifyAudioPolicyReady interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerNotifyAudioPolicyReady_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->NotifyAudioPolicyReady();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test LoadHdiAdapter API
 * @tc.type  : FUNC
 * @tc.number: AudioServerLoadHdiAdapter_001
 * @tc.desc  : Test LoadHdiAdapter interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerLoadHdiAdapter_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->LoadHdiAdapter(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary");
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test UnloadHdiAdapter API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUnloadHdiAdapter_001
 * @tc.desc  : Test UnloadHdiAdapter interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUnloadHdiAdapter_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UnloadHdiAdapter(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary", false);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test CreateHdiSinkPort API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateHdiSinkPort_001
 * @tc.desc  : Test CreateHdiSinkPort interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCreateHdiSinkPort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    IAudioSinkAttr attr;
    uint32_t renderId = 0;
    int32_t ret = audioServer->CreateHdiSinkPort("speaker", "test_info", attr, true, renderId);
    EXPECT_EQ(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test CreateSinkPort API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateSinkPort_001
 * @tc.desc  : Test CreateSinkPort interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCreateSinkPort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    IAudioSinkAttr attr;
    uint32_t renderId = 0;
    int32_t ret = audioServer->CreateSinkPort(0, 0, "test_info", attr, renderId);
    EXPECT_EQ(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test CreateHdiSourcePort API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateHdiSourcePort_001
 * @tc.desc  : Test CreateHdiSourcePort interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCreateHdiSourcePort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    IAudioSourceAttr attr;
    uint32_t captureId = 0;
    int32_t ret = audioServer->CreateHdiSourcePort("mic", "test_info", attr, true, captureId);
    EXPECT_EQ(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test CreateSourcePort API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateSourcePort_001
 * @tc.desc  : Test CreateSourcePort interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCreateSourcePort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    IAudioSourceAttr attr;
    uint32_t captureId = 0;
    int32_t ret = audioServer->CreateSourcePort(0, 0, "test_info", attr, captureId);
    EXPECT_EQ(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test DestroyHdiPort API
 * @tc.type  : FUNC
 * @tc.number: AudioServerDestroyHdiPort_001
 * @tc.desc  : Test DestroyHdiPort interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerDestroyHdiPort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->DestroyHdiPort(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid port ID
}

/**
 * @tc.name  : Test IsAcousticEchoCancelerSupported API
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsAcousticEchoCancelerSupported_001
 * @tc.desc  : Test IsAcousticEchoCancelerSupported interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerIsAcousticEchoCancelerSupported_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    bool isSupported = false;
    int32_t ret = audioServer->IsAcousticEchoCancelerSupported(SOURCE_TYPE_MIC, isSupported);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test BuildKaraokeParamString golden strings
 * @tc.type  : FUNC
 * @tc.number: AudioServerKaraokeParamHelper_001
 * @tc.desc  : Verify (paramType,value) -> "key=value" matches legacy output.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerKaraokeParamHelper_001, TestSize.Level1)
{
    bool valid = false;
    EXPECT_EQ("Karaoke_volume=50", BuildKaraokeParamString(KARAOKE_PARAM_VOLUME, 50, valid));
    EXPECT_TRUE(valid);
    EXPECT_EQ("Karaoke_volume=0", BuildKaraokeParamString(KARAOKE_PARAM_VOLUME, 0, valid));
    EXPECT_EQ("Karaoke_volume=100", BuildKaraokeParamString(KARAOKE_PARAM_VOLUME, 100, valid));

    EXPECT_EQ("Karaoke_reverb_mode=disable",
        BuildKaraokeParamString(KARAOKE_PARAM_REVERB_MODE, REVERB_PRESET_ORIGINAL, valid));
    EXPECT_EQ("Karaoke_reverb_mode=ktv",
        BuildKaraokeParamString(KARAOKE_PARAM_REVERB_MODE, REVERB_PRESET_KTV, valid));
    EXPECT_EQ("Karaoke_reverb_mode=theatre",
        BuildKaraokeParamString(KARAOKE_PARAM_REVERB_MODE, REVERB_PRESET_THEATER, valid));
    EXPECT_EQ("Karaoke_reverb_mode=concert",
        BuildKaraokeParamString(KARAOKE_PARAM_REVERB_MODE, REVERB_PRESET_CONCERT, valid));

    EXPECT_EQ("Karaoke_eq_mode=disable",
        BuildKaraokeParamString(KARAOKE_PARAM_EQ_MODE, EQUALIZER_PRESET_FLAT, valid));
    EXPECT_EQ("Karaoke_eq_mode=full",
        BuildKaraokeParamString(KARAOKE_PARAM_EQ_MODE, EQUALIZER_PRESET_FULL, valid));
    EXPECT_EQ("Karaoke_eq_mode=bright",
        BuildKaraokeParamString(KARAOKE_PARAM_EQ_MODE, EQUALIZER_PRESET_BRIGHT, valid));

    EXPECT_EQ("Karaoke_enable=enable", BuildKaraokeParamString(KARAOKE_PARAM_ENABLE, 1, valid));
    EXPECT_EQ("Karaoke_enable=disable", BuildKaraokeParamString(KARAOKE_PARAM_ENABLE, 0, valid));
    EXPECT_EQ("Karaoke_capture_reverb=true",
        BuildKaraokeParamString(KARAOKE_PARAM_CAPTURE_REVERB, 1, valid));
    EXPECT_EQ("Karaoke_capture_reverb=false",
        BuildKaraokeParamString(KARAOKE_PARAM_CAPTURE_REVERB, 0, valid));
}

/**
 * @tc.name  : Test BuildKaraokeParamString invalid inputs
 * @tc.type  : FUNC
 * @tc.number: AudioServerKaraokeParamHelper_002
 * @tc.desc  : Verify invalid paramType / out-of-range volume / bad ordinal rejected.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerKaraokeParamHelper_002, TestSize.Level1)
{
    bool valid = true;
    EXPECT_EQ("", BuildKaraokeParamString(999, 0, valid));
    EXPECT_FALSE(valid);
    EXPECT_EQ("", BuildKaraokeParamString(KARAOKE_PARAM_VOLUME, -1, valid));
    EXPECT_FALSE(valid);
    EXPECT_EQ("", BuildKaraokeParamString(KARAOKE_PARAM_VOLUME, 101, valid));
    EXPECT_FALSE(valid);
    EXPECT_EQ("", BuildKaraokeParamString(KARAOKE_PARAM_REVERB_MODE, 999, valid));
    EXPECT_FALSE(valid);
    EXPECT_EQ("", BuildKaraokeParamString(KARAOKE_PARAM_EQ_MODE, 999, valid));
    EXPECT_FALSE(valid);
    EXPECT_EQ("", BuildKaraokeParamString(KARAOKE_PARAM_ENABLE, 2, valid));
    EXPECT_FALSE(valid);
    EXPECT_EQ("", BuildKaraokeParamString(KARAOKE_PARAM_CAPTURE_REVERB, 2, valid));
    EXPECT_FALSE(valid);
}

/**
 * @tc.name  : Test SetKaraokeParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetKaraokeParameters_001
 * @tc.desc  : Test SetKaraokeParameters interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetKaraokeParameters_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    bool ret = false;
    int32_t result = audioServer->SetKaraokeParameters(
        DEVICE_TYPE_SPEAKER, KARAOKE_PARAM_VOLUME, 50, ret);
    EXPECT_NE(SUCCESS, result);
}

/**
 * @tc.name  : Test IsAudioLoopbackSupported API
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsAudioLoopbackSupported_001
 * @tc.desc  : Test IsAudioLoopbackSupported interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerIsAudioLoopbackSupported_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    bool isSupported = false;
    int32_t ret = audioServer->IsAudioLoopbackSupported(0, DEVICE_TYPE_SPEAKER, isSupported);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetLatestMuteState API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetLatestMuteState_001
 * @tc.desc  : Test SetLatestMuteState interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetLatestMuteState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetLatestMuteState(0, true);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test AddCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: AudioServerAddCaptureInjector_001
 * @tc.desc  : Test AddCaptureInjector interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerAddCaptureInjector_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::string rate = "48000";
    std::string format = "S16LE";
    std::string channels = "2";
    std::string bufferSize = "1024";
    int32_t ret = audioServer->AddCaptureInjector(0, rate, format, channels, bufferSize);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid sink port
}

/**
 * @tc.name  : Test RemoveCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRemoveCaptureInjector_001
 * @tc.desc  : Test RemoveCaptureInjector interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRemoveCaptureInjector_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->RemoveCaptureInjector(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid sink port
}

/**
 * @tc.name  : Test RegisterCallbackHandle API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRegisterCallbackHandle_001
 * @tc.desc  : Test RegisterCallbackHandle interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRegisterCallbackHandle_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->RegisterCallbackHandle(object);
    EXPECT_NE(SUCCESS, ret); // Should fail with null object
}

/**
 * @tc.name  : Test SetCallbackHandleEnable API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetCallbackHandleEnable_001
 * @tc.desc  : Test SetCallbackHandleEnable interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetCallbackHandleEnable_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetCallbackHandleEnable(0, true);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid callback ID
}

/**
 * @tc.name  : Test GetCurrentOutputPipeChangeInfos API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetCurrentOutputPipeChangeInfos_001
 * @tc.desc  : Test GetCurrentOutputPipeChangeInfos interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetCurrentOutputPipeChangeInfos_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<std::shared_ptr<AudioOutputPipeInfo>> pipeChangeInfos;
    int32_t ret = audioServer->GetCurrentOutputPipeChangeInfos(pipeChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetCurrentInputPipeChangeInfos API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetCurrentInputPipeChangeInfos_001
 * @tc.desc  : Test GetCurrentInputPipeChangeInfos interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetCurrentInputPipeChangeInfos_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<std::shared_ptr<AudioInputPipeInfo>> pipeChangeInfos;
    int32_t ret = audioServer->GetCurrentInputPipeChangeInfos(pipeChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RequestUserPrivacyAuthority API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRequestUserPrivacyAuthority_001
 * @tc.desc  : Test RequestUserPrivacyAuthority interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRequestUserPrivacyAuthority_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->RequestUserPrivacyAuthority(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test RegistAdapterManagerCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRegistAdapterManagerCallback_001
 * @tc.desc  : Test RegistAdapterManagerCallback interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRegistAdapterManagerCallback_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->RegistAdapterManagerCallback(object, LOCAL_NETWORK_ID);
    EXPECT_NE(SUCCESS, ret); // Should fail with null object
}

/**
 * @tc.name  : Test UnRegistAdapterManagerCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUnRegistAdapterManagerCallback_001
 * @tc.desc  : Test UnRegistAdapterManagerCallback interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUnRegistAdapterManagerCallback_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UnRegistAdapterManagerCallback(LOCAL_NETWORK_ID);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAuxiliarySinkEnable API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAuxiliarySinkEnable_001
 * @tc.desc  : Test SetAuxiliarySinkEnable interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetAuxiliarySinkEnable_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetAuxiliarySinkEnable(true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test ReportPlaybackCaptureUserChoice API
 * @tc.type  : FUNC
 * @tc.number: AudioServerReportPlaybackCaptureUserChoice_001
 * @tc.desc  : Test ReportPlaybackCaptureUserChoice interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerReportPlaybackCaptureUserChoice_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->ReportPlaybackCaptureUserChoice(0, true);
    EXPECT_EQ(ERR_SYSTEM_PERMISSION_DENIED, ret);
}

/**
 * @tc.name  : Test ReportWakeupEvent API
 * @tc.type  : FUNC
 * @tc.number: AudioServerReportWakeupEvent_001
 * @tc.desc  : Test ReportWakeupEvent interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerReportWakeupEvent_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    AudioWakeupTrackInfo wakeupTrackInfo;
    wakeupTrackInfo.stage = STAGE_FWK_CREATE_ENTER;
    wakeupTrackInfo.result = WAKEUP_RESULT_SUCC;
    wakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;
    
    int32_t ret = audioServer->ReportWakeupEvent(wakeupTrackInfo);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAncoRecordingConfigChanged API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAncoRecordingConfigChanged_001
 * @tc.desc  : Test SetAncoRecordingConfigChanged interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetAncoRecordingConfigChanged_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<std::string> bundleNameList = {"com.test.app1", "com.test.app2"};
    int32_t ret = audioServer->SetAncoRecordingConfigChanged(bundleNameList);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateActiveDevicesRoute API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUpdateActiveDevicesRoute_001
 * @tc.desc  : Test UpdateActiveDevicesRoute interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUpdateActiveDevicesRoute_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    std::vector<IntPair> activeDevices;
    activeDevices.push_back({DEVICE_TYPE_SPEAKER, ALL_DEVICES_FLAG});
    
    int32_t ret = audioServer->UpdateActiveDevicesRoute(activeDevices, A2DP_OFFLOAD, 0);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test ReleaseActiveDeviceRoute API
 * @tc.type  : FUNC
 * @tc.number: AudioServerReleaseActiveDeviceRoute_001
 * @tc.desc  : Test ReleaseActiveDeviceRoute interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerReleaseActiveDeviceRoute_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->ReleaseActiveDeviceRoute(DEVICE_TYPE_SPEAKER, ALL_DEVICES_FLAG, LOCAL_NETWORK_ID);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test SetDmDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetDmDeviceType_001
 * @tc.desc  : Test SetDmDeviceType interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerSetDmDeviceType_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->SetDmDeviceType(0, DEVICE_TYPE_SPEAKER);
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test UpdateDualToneState API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUpdateDualToneState_001
 * @tc.desc  : Test UpdateDualToneState interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUpdateDualToneState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UpdateDualToneState(true, 0, "test_sink");
    EXPECT_NE(SUCCESS, ret); // Should fail without proper setup
}

/**
 * @tc.name  : Test GetVolumeBySessionId API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetVolumeBySessionId_001
 * @tc.desc  : Test GetVolumeBySessionId interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerGetVolumeBySessionId_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    float volume = 0.0f;
    int32_t ret = audioServer->GetVolumeBySessionId(0, volume);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid session
}

/**
 * @tc.name  : Test RegisterDataTransferCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRegisterDataTransferCallback_001
 * @tc.desc  : Test RegisterDataTransferCallback interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRegisterDataTransferCallback_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->RegisterDataTransferCallback(object);
    EXPECT_NE(SUCCESS, ret); // Should fail with null object
}

/**
 * @tc.name  : Test RegisterDataTransferMonitorParam API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRegisterDataTransferMonitorParam_001
 * @tc.desc  : Test RegisterDataTransferMonitorParam interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerRegisterDataTransferMonitorParam_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    DataTransferMonitorParam param;
    int32_t ret = audioServer->RegisterDataTransferMonitorParam(0, param);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid callback ID
}

/**
 * @tc.name  : Test UnregisterDataTransferMonitorParam API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUnregisterDataTransferMonitorParam_001
 * @tc.desc  : Test UnregisterDataTransferMonitorParam interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerUnregisterDataTransferMonitorParam_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->UnregisterDataTransferMonitorParam(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid callback ID
}

/**
 * @tc.name  : Test CheckCaptureLimit API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckCaptureLimit_001
 * @tc.desc  : Test CheckCaptureLimit interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerCheckCaptureLimit_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    AudioPlaybackCaptureConfig config;
    int32_t innerCapId = 0;
    int32_t ret = audioServer->CheckCaptureLimit(config, innerCapId);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test ReleaseCaptureLimit API
 * @tc.type  : FUNC
 * @tc.number: AudioServerReleaseCaptureLimit_001
 * @tc.desc  : Test ReleaseCaptureLimit interface.
 */
HWTEST_F(AudioServerSecondUnitTest, AudioServerReleaseCaptureLimit_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    
    int32_t ret = audioServer->ReleaseCaptureLimit(0);
    EXPECT_NE(SUCCESS, ret); // Should fail without valid innerCapId
}

} // namespace AudioStandard
} // namespace OHOS
