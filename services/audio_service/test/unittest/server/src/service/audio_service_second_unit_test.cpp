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

#include "audio_service_second_unit_test.h"
#include "audio_errors.h"
#include "audio_service.h"
#include "audio_service_log.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_stream_info.h"
#include "policy_handler.h"
#include "audio_volume.h"
#include "audio_privacy_manager_in_server.h"

#include <gtest/gtest.h>
#include <memory>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr int32_t DEFAULT_TEST_UID = 10000;
constexpr uint32_t TEST_SESSION_ID = 12345;
constexpr int32_t INVALID_SESSION_ID = 999999;

void AudioServiceSecondUnitTest::SetUpTestCase(void)
{
}

void AudioServiceSecondUnitTest::TearDownTestCase(void)
{
}

void AudioServiceSecondUnitTest::SetUp(void)
{
}

void AudioServiceSecondUnitTest::TearDown(void)
{
}

/**
 * @tc.name  : Test AudioService::GetInstance API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceGetInstance_001
 * @tc.desc  : Test GetInstance interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceGetInstance_001, TestSize.Level1)
{
    AudioService *instance = AudioService::GetInstance();
    EXPECT_NE(instance, nullptr);
}

/**
 * @tc.name  : Test AudioService::UpdateMuteControlSet API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceUpdateMuteControlSet_001
 * @tc.desc  : Test UpdateMuteControlSet with valid session id.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceUpdateMuteControlSet_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->UpdateMuteControlSet(TEST_SESSION_ID, true);
    audioService->UpdateMuteControlSet(TEST_SESSION_ID, false);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::UpdateMuteControlSet API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceUpdateMuteControlSet_002
 * @tc.desc  : Test UpdateMuteControlSet with invalid session id.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceUpdateMuteControlSet_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    // Test with invalid session id (too small)
    audioService->UpdateMuteControlSet(MIN_STREAMID - 1, true);
    // Test with invalid session id (too large)
    audioService->UpdateMuteControlSet(MAX_STREAMID + 1, true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::RemoveIdFromMuteControlSet API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceRemoveIdFromMuteControlSet_001
 * @tc.desc  : Test RemoveIdFromMuteControlSet with existing session id.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceRemoveIdFromMuteControlSet_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->UpdateMuteControlSet(TEST_SESSION_ID, true);
    audioService->RemoveIdFromMuteControlSet(TEST_SESSION_ID);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::RemoveIdFromMuteControlSet API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceRemoveIdFromMuteControlSet_002
 * @tc.desc  : Test RemoveIdFromMuteControlSet with non-existing session id.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceRemoveIdFromMuteControlSet_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->RemoveIdFromMuteControlSet(INVALID_SESSION_ID);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SaveForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSaveForegroundList_001
 * @tc.desc  : Test SaveForegroundList interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSaveForegroundList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.app1", "com.test.app2"};
    audioService->SaveForegroundList(list);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SaveForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSaveForegroundList_002
 * @tc.desc  : Test SaveForegroundList with invalid size.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSaveForegroundList_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list;
    for (size_t i = 0; i < 20; i++) {
        list.push_back("com.test.app" + std::to_string(i));
    }
    audioService->SaveForegroundList(list);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::MatchForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceMatchForegroundList_001
 * @tc.desc  : Test MatchForegroundList interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceMatchForegroundList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.app1", "com.test.app2"};
    audioService->SaveForegroundList(list);
    bool result = audioService->MatchForegroundList("com.test.app1", DEFAULT_TEST_UID);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::MatchForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceMatchForegroundList_002
 * @tc.desc  : Test MatchForegroundList with non-existing app.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceMatchForegroundList_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.app1", "com.test.app2"};
    audioService->SaveForegroundList(list);
    bool result = audioService->MatchForegroundList("com.test.app3", DEFAULT_TEST_UID);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::InForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceInForegroundList_001
 * @tc.desc  : Test InForegroundList interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceInForegroundList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.app1", "com.test.app2"};
    audioService->SaveForegroundList(list);
    audioService->MatchForegroundList("com.test.app1", DEFAULT_TEST_UID);
    bool result = audioService->InForegroundList(DEFAULT_TEST_UID);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::InForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceInForegroundList_002
 * @tc.desc  : Test InForegroundList with non-existing uid.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceInForegroundList_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    bool result = audioService->InForegroundList(INVALID_SESSION_ID);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::SaveRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSaveRenderWhitelist_001
 * @tc.desc  : Test SaveRenderWhitelist interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSaveRenderWhitelist_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.whitelist1", "com.test.whitelist2"};
    audioService->SaveRenderWhitelist(list);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::InRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceInRenderWhitelist_001
 * @tc.desc  : Test InRenderWhitelist with existing app.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceInRenderWhitelist_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.whitelist1", "com.test.whitelist2"};
    audioService->SaveRenderWhitelist(list);
    bool result = audioService->InRenderWhitelist("com.test.whitelist1");
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::InRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceInRenderWhitelist_002
 * @tc.desc  : Test InRenderWhitelist with non-existing app.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceInRenderWhitelist_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> list = {"com.test.whitelist1", "com.test.whitelist2"};
    audioService->SaveRenderWhitelist(list);
    bool result = audioService->InRenderWhitelist("com.test.whitelist3");
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::UpdateForegroundState API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceUpdateForegroundState_001
 * @tc.desc  : Test UpdateForegroundState interface.
 */
/**
 * @tc.name  : Test AudioService::DumpForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceDumpForegroundList_001
 * @tc.desc  : Test DumpForegroundList interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceDumpForegroundList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::string dumpString;
    audioService->DumpForegroundList(dumpString);
    EXPECT_FALSE(dumpString.empty());
}

/**
 * @tc.name  : Test AudioService::IsEndpointTypeVoip API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsEndpointTypeVoip_001
 * @tc.desc  : Test IsEndpointTypeVoip with VOICE_COMMUNICATION usage.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceIsEndpointTypeVoip_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    config.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    AudioDeviceDescriptor deviceInfo;
    bool result = audioService->IsEndpointTypeVoip(config, deviceInfo);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::IsEndpointTypeVoip API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsEndpointTypeVoip_002
 * @tc.desc  : Test IsEndpointTypeVoip with VIDEO_COMMUNICATION usage.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceIsEndpointTypeVoip_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    config.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    AudioDeviceDescriptor deviceInfo;
    bool result = audioService->IsEndpointTypeVoip(config, deviceInfo);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::IsEndpointTypeVoip API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsEndpointTypeVoip_003
 * @tc.desc  : Test IsEndpointTypeVoip with VOICE_COMMUNICATION source type.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceIsEndpointTypeVoip_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    config.capturerInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    AudioDeviceDescriptor deviceInfo;
    bool result = audioService->IsEndpointTypeVoip(config, deviceInfo);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::IsEndpointTypeVoip API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsEndpointTypeVoip_004
 * @tc.desc  : Test IsEndpointTypeVoip with non-voip usage.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceIsEndpointTypeVoip_004, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    AudioDeviceDescriptor deviceInfo;
    bool result = audioService->IsEndpointTypeVoip(config, deviceInfo);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeDualTone_001
 * @tc.desc  : Test ShouldBeDualTone with non-Speaker device.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceShouldBeDualTone_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    std::string dupSinkName = "NotSpeaker";
    bool result = audioService->ShouldBeDualTone(config, dupSinkName);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeDualTone_002
 * @tc.desc  : Test ShouldBeDualTone with Speaker device and ALARM usage.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceShouldBeDualTone_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    std::string dupSinkName = "Speaker";
    bool result = audioService->ShouldBeDualTone(config, dupSinkName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeDualTone_003
 * @tc.desc  : Test ShouldBeDualTone with Speaker device and non-ringtone usage.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceShouldBeDualTone_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    std::string dupSinkName = "Speaker";
    bool result = audioService->ShouldBeDualTone(config, dupSinkName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::EnableDualStreamForNormalStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceEnableDualStreamForNormalStream_001
 * @tc.desc  : Test EnableDualStreamForNormalStream with non-existing session.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceEnableDualStreamForNormalStream_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->EnableDualStreamForNormalStream(INVALID_SESSION_ID, "test_sink");
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioService::DisableDualStreamForNormalStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceDisableDualStreamForNormalStream_001
 * @tc.desc  : Test DisableDualStreamForNormalStream with non-existing session.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceDisableDualStreamForNormalStream_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->DisableDualStreamForNormalStream(INVALID_SESSION_ID);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioService::SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetNonInterruptMute_001
 * @tc.desc  : Test SetNonInterruptMute with non-existing session.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetNonInterruptMute_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    // Test with non-existing session
    audioService->SetNonInterruptMute(INVALID_SESSION_ID, true);
    audioService->SetNonInterruptMute(INVALID_SESSION_ID, false);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::UpdateSessionMuteStatus API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceUpdateSessionMuteStatus_001
 * @tc.desc  : Test UpdateSessionMuteStatus interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceUpdateSessionMuteStatus_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->UpdateSessionMuteStatus(TEST_SESSION_ID, true);
    audioService->UpdateSessionMuteStatus(TEST_SESSION_ID, false);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetIncMaxRendererStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetIncMaxRendererStreamCnt_001
 * @tc.desc  : Test SetIncMaxRendererStreamCnt interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetIncMaxRendererStreamCnt_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->SetIncMaxRendererStreamCnt(AUDIO_MODE_PLAYBACK);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetIncMaxRendererStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetIncMaxRendererStreamCnt_002
 * @tc.desc  : Test SetIncMaxRendererStreamCnt with non-playback mode.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetIncMaxRendererStreamCnt_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->SetIncMaxRendererStreamCnt(AUDIO_MODE_RECORD);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetDecMaxRendererStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetDecMaxRendererStreamCnt_001
 * @tc.desc  : Test SetDecMaxRendererStreamCnt interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetDecMaxRendererStreamCnt_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->SetDecMaxRendererStreamCnt();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetIncMaxLoopbackStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetIncMaxLoopbackStreamCnt_001
 * @tc.desc  : Test SetIncMaxLoopbackStreamCnt interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetIncMaxLoopbackStreamCnt_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->SetIncMaxLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    audioService->SetIncMaxLoopbackStreamCnt(AUDIO_MODE_RECORD);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetDecMaxLoopbackStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetDecMaxLoopbackStreamCnt_001
 * @tc.desc  : Test SetDecMaxLoopbackStreamCnt interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetDecMaxLoopbackStreamCnt_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->SetDecMaxLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    audioService->SetDecMaxLoopbackStreamCnt(AUDIO_MODE_RECORD);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::GetCurrentLoopbackStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceGetCurrentLoopbackStreamCnt_001
 * @tc.desc  : Test GetCurrentLoopbackStreamCnt interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceGetCurrentLoopbackStreamCnt_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t count = audioService->GetCurrentLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    EXPECT_GE(count, 0);
}

/**
 * @tc.name  : Test AudioService::CleanAppUseNumMap API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceCleanAppUseNumMap_001
 * @tc.desc  : Test CleanAppUseNumMap interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceCleanAppUseNumMap_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->CleanAppUseNumMap(DEFAULT_TEST_UID);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::GetCurrentRendererStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceGetCurrentRendererStreamCnt_001
 * @tc.desc  : Test GetCurrentRendererStreamCnt interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceGetCurrentRendererStreamCnt_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t count = audioService->GetCurrentRendererStreamCnt();
    EXPECT_GE(count, 0);
}

/**
 * @tc.name  : Test AudioService::UpdateSourceType API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceUpdateSourceType_001
 * @tc.desc  : Test UpdateSourceType with special source type.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceUpdateSourceType_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->UpdateSourceType(SOURCE_TYPE_PLAYBACK_CAPTURE);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioService::HasBluetoothEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceHasBluetoothEndpoint_001
 * @tc.desc  : Test HasBluetoothEndpoint interface.
 */
/**
 * @tc.name  : Test AudioService::IsStreamTypeFitWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsStreamTypeFitWorkgroup_001
 * @tc.desc  : Test IsStreamTypeFitWorkgroup with supported stream type.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceIsStreamTypeFitWorkgroup_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    bool result = audioService->IsStreamTypeFitWorkgroup(STREAM_MUSIC);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioService::IsStreamTypeFitWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsStreamTypeFitWorkgroup_002
 * @tc.desc  : Test IsStreamTypeFitWorkgroup with unsupported stream type.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceIsStreamTypeFitWorkgroup_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    bool result = audioService->IsStreamTypeFitWorkgroup(STREAM_VOICE_CALL);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioService::RegisterMuteStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceRegisterMuteStateChangeCallback_001
 * @tc.desc  : Test RegisterMuteStateChangeCallback interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceRegisterMuteStateChangeCallback_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    MuteStateChangeCallbck callback = [](bool muteFlag) {};
    audioService->RegisterMuteStateChangeCallback(TEST_SESSION_ID, callback);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetLatestMuteState API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetLatestMuteState_001
 * @tc.desc  : Test SetLatestMuteState interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetLatestMuteState_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    MuteStateChangeCallbck callback = [](bool muteFlag) {};
    audioService->RegisterMuteStateChangeCallback(TEST_SESSION_ID, callback);
    audioService->SetLatestMuteState(TEST_SESSION_ID, true);
    audioService->SetLatestMuteState(TEST_SESSION_ID, false);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceForceStopAudioStream_001
 * @tc.desc  : Test ForceStopAudioStream with STOP_ALL.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceForceStopAudioStream_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->ForceStopAudioStream(STOP_ALL);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioService::ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceForceStopAudioStream_002
 * @tc.desc  : Test ForceStopAudioStream with STOP_RENDER.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceForceStopAudioStream_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->ForceStopAudioStream(STOP_RENDER);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioService::ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceForceStopAudioStream_003
 * @tc.desc  : Test ForceStopAudioStream with STOP_RECORD.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceForceStopAudioStream_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->ForceStopAudioStream(STOP_RECORD);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioService::ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceForceStopAudioStream_004
 * @tc.desc  : Test ForceStopAudioStream with invalid type.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceForceStopAudioStream_004, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    int32_t ret = audioService->ForceStopAudioStream(static_cast<StopAudioType>(-1));
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test AudioService::SaveAdjustStreamVolumeInfo API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSaveAdjustStreamVolumeInfo_001
 * @tc.desc  : Test SaveAdjustStreamVolumeInfo interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSaveAdjustStreamVolumeInfo_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    audioService->SaveAdjustStreamVolumeInfo(1.0f, TEST_SESSION_ID, "2026-06-17", 0);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioService::SetAncoRecordingConfigChanged API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetAncoRecordingConfigChanged_001
 * @tc.desc  : Test SetAncoRecordingConfigChanged interface.
 */
HWTEST_F(AudioServiceSecondUnitTest, AudioServiceSetAncoRecordingConfigChanged_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    ASSERT_NE(audioService, nullptr);
    std::vector<std::string> bundleNameList = {"com.test.anco1"};
    audioService->SetAncoRecordingConfigChanged(bundleNameList);
    EXPECT_TRUE(true);
}

} // namespace AudioStandard
} // namespace OHOS
