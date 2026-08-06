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

#include "audio_service.h"
#include "audio_service_log.h"
#include "core_service_handler.h"
#include "audio_workgroup.h"
#include "audio_system_load_listener.h"
#include "renderer_in_server.h"
#include "capturer_in_server.h"
#include "ipc_stream_in_server.h"
#include "boot_animation_state_info.h"
#include "audio_service_enum.h"

#include "aiut_log.h"

#include "audio_errors.h"
#include "audio_system_manager.h"
#include "audio_process_proxy.h"
#include "pro_renderer_stream_impl.h"
#include "rtg_interface.h"
#include "concurrent_task_client.h"
#include "audio_resource_service.h"
#include "audio_effect_chain_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace testing::ext;

namespace {
} // namespace

namespace OHOS {
namespace AudioStandard {

class AudioServiceCoreAiut : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioServiceCoreAiut::SetUpTestCase(void) {}
void AudioServiceCoreAiut::TearDownTestCase(void) {}
void AudioServiceCoreAiut::SetUp(void) {}
void AudioServiceCoreAiut::TearDown(void) {}

// ==================== AudioService Tests ====================

/**
 * @tc.name  : Test DisableLoopback API
 * @tc.type  : FUNC
 * @tc.number: AudioService_DisableLoopback_NoCrash
 * @tc.desc  : Test DisableLoopback interface does not crash.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_DisableLoopback_NoCrash, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->DisableLoopback();
    AIUT_EXPECT_NE_INFO_LOG(AudioService::GetInstance(), nullptr,
        "AudioService should still be valid after DisableLoopback");
}

/**
 * @tc.name  : Test NotifyStreamVolumeChanged API
 * @tc.type  : FUNC
 * @tc.number: AudioService_NotifyStreamVolumeChanged_MusicAndAlarm
 * @tc.desc  : Test NotifyStreamVolumeChanged interface with various stream types.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_NotifyStreamVolumeChanged_MusicAndAlarm, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    int32_t ret = audioService->NotifyStreamVolumeChanged(STREAM_MUSIC, 0.5f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "NotifyStreamVolumeChanged failed, ret=%d", ret);

    ret = audioService->NotifyStreamVolumeChanged(STREAM_ALARM, 1.0f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "NotifyStreamVolumeChanged alarm failed, ret=%d", ret);
}

/**
 * @tc.name  : Test HasBluetoothEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioService_HasBluetoothEndpoint_ReturnsTrue
 * @tc.desc  : Test HasBluetoothEndpoint returns true.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_HasBluetoothEndpoint_ReturnsTrue, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    bool ret = audioService->HasBluetoothEndpoint();
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "HasBluetoothEndpoint should return true");
}

/**
 * @tc.name  : Test SetDecMaxRendererStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioService_SetDecMaxRendererStreamCnt_DecrementsCounter
 * @tc.desc  : Test SetDecMaxRendererStreamCnt decrements counter.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SetDecMaxRendererStreamCnt_DecrementsCounter, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->SetIncMaxRendererStreamCnt(AUDIO_MODE_PLAYBACK);
    int32_t beforeCnt = audioService->GetCurrentRendererStreamCnt();
    audioService->SetDecMaxRendererStreamCnt();
    int32_t afterCnt = audioService->GetCurrentRendererStreamCnt();
    AIUT_EXPECT_EQ_INFO_LOG(afterCnt, beforeCnt - 1, "SetDecMaxRendererStreamCnt failed, before=%d after=%d",
        beforeCnt, afterCnt);
}

/**
 * @tc.name  : Test GetHibernateState API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetHibernateState_ReturnsOnHibernateValue
 * @tc.desc  : Test GetHibernateState returns onHibernate_ value.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetHibernateState_ReturnsOnHibernateValue, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->onHibernate_ = false;
    bool ret = audioService->GetHibernateState();
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "GetHibernateState should return false");

    audioService->onHibernate_ = true;
    ret = audioService->GetHibernateState();
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "GetHibernateState should return true");
    audioService->onHibernate_ = false;
}

/**
 * @tc.name  : Test GetRemoteDeviceName API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetRemoteDeviceName_InvalidDeviceReturnsFalse
 * @tc.desc  : Test GetRemoteDeviceName interface.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetRemoteDeviceName_InvalidDeviceReturnsFalse, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    std::string deviceId = "test_device";
    std::string deviceName;
    bool ret = audioService->GetRemoteDeviceName(deviceId, deviceName);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "GetRemoteDeviceName should return false for invalid device");
}

/**
 * @tc.name  : Test SaveAdjustStreamVolumeInfo API
 * @tc.type  : FUNC
 * @tc.number: AudioService_SaveAdjustStreamVolumeInfo_NoCrash
 * @tc.desc  : Test SaveAdjustStreamVolumeInfo interface does not crash.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SaveAdjustStreamVolumeInfo_NoCrash, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->SaveAdjustStreamVolumeInfo(0.5f, 1, "2026-01-01", 100);
    AIUT_EXPECT_NE_INFO_LOG(AudioService::GetInstance(), nullptr,
        "AudioService should still be valid after SaveAdjustStreamVolumeInfo");
}

/**
 * @tc.name  : Test GetAllSinkInputs API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetAllSinkInputs_EmptyInTestEnv
 * @tc.desc  : Test GetAllSinkInputs interface does not crash.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetAllSinkInputs_EmptyInTestEnv, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    std::vector<SinkInput> sinkInputs;
    audioService->GetAllSinkInputs(sinkInputs);
    AIUT_EXPECT_TRUE_INFO_LOG(sinkInputs.empty(),
        "sinkInputs should be empty in test env, size=%zu", sinkInputs.size());
}

/**
 * @tc.name  : Test GetPrivacyTypeForNormalStream API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetPrivacyTypeForNormalStream_SessionNotInMap
 * @tc.desc  : Test GetPrivacyTypeForNormalStream with session not in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetPrivacyTypeForNormalStream_SessionNotInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->allRendererMap_.clear();
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
    int32_t ret = audioService->GetPrivacyTypeForNormalStream(99999, privacyType);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "GetPrivacyTypeForNormalStream should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetPrivacyType API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetPrivacyType_SessionNotInMap
 * @tc.desc  : Test GetPrivacyType with session not in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetPrivacyType_SessionNotInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->allRendererMap_.clear();
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
    int32_t ret = audioService->GetPrivacyType(99999, privacyType);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "GetPrivacyType should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetPrivacyTypeForNormalStream API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetPrivacyTypeForNormalStream_ValidSession
 * @tc.desc  : Test GetPrivacyTypeForNormalStream with valid session.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetPrivacyTypeForNormalStream_ValidSession, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    AudioProcessConfig processConfig;
    processConfig.privacyType = AudioPrivacyType::PRIVACY_TYPE_PRIVATE;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    uint32_t sessionId = 200001;
    audioService->allRendererMap_.clear();
    audioService->allRendererMap_[sessionId] = renderer;
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
    int32_t ret = audioService->GetPrivacyTypeForNormalStream(sessionId, privacyType);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetPrivacyTypeForNormalStream should succeed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(privacyType),
        static_cast<int32_t>(AudioPrivacyType::PRIVACY_TYPE_PRIVATE),
        "privacyType mismatch, type=%d", static_cast<int32_t>(privacyType));
    audioService->allRendererMap_.clear();
}

/**
 * @tc.name  : Test SetNonInterruptMuteForProcess API
 * @tc.type  : FUNC
 * @tc.number: AudioService_SetNonInterruptMuteForProcess_MuteAndUnmute
 * @tc.desc  : Test SetNonInterruptMuteForProcess with mute true and false.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SetNonInterruptMuteForProcess_MuteAndUnmute, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->muteSwitchStreams_.clear();
    audioService->mutedSessions_.clear();
    uint32_t sessionId = 200002;

    audioService->SetNonInterruptMuteForProcess(sessionId, true);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->muteSwitchStreams_.count(sessionId), 1,
        "muteSwitchStreams_ should contain sessionId after mute");

    audioService->SetNonInterruptMuteForProcess(sessionId, false);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->muteSwitchStreams_.count(sessionId), 0,
        "muteSwitchStreams_ should not contain sessionId after unmute");
}

/**
 * @tc.name  : Test UpdateSessionMuteStatus API
 * @tc.type  : FUNC
 * @tc.number: AudioService_UpdateSessionMuteStatus_MuteTrueAndFalse
 * @tc.desc  : Test UpdateSessionMuteStatus with mute true and false.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_UpdateSessionMuteStatus_MuteTrueAndFalse, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->muteSwitchStreams_.clear();
    uint32_t sessionId = 200003;

    audioService->UpdateSessionMuteStatus(sessionId, true);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->muteSwitchStreams_.count(sessionId), 1,
        "muteSwitchStreams_ should contain sessionId");

    audioService->UpdateSessionMuteStatus(sessionId, false);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->muteSwitchStreams_.count(sessionId), 0,
        "muteSwitchStreams_ should not contain sessionId");
}

/**
 * @tc.name  : Test IsMuteSwitchStream API
 * @tc.type  : FUNC
 * @tc.number: AudioService_IsMuteSwitchStream_InAndNotInSet
 * @tc.desc  : Test IsMuteSwitchStream with session in and not in set.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_IsMuteSwitchStream_InAndNotInSet, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->muteSwitchStreams_.clear();
    uint32_t sessionId = 200004;

    bool ret = audioService->IsMuteSwitchStream(sessionId);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "IsMuteSwitchStream should return false for non-existent session");

    audioService->muteSwitchStreams_.insert(sessionId);
    ret = audioService->IsMuteSwitchStream(sessionId);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "IsMuteSwitchStream should return true for existing session");

    ret = audioService->IsMuteSwitchStream(sessionId);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "IsMuteSwitchStream should return false after erase");
}

/**
 * @tc.name  : Test GetRendererInServerBySessionId API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetRendererInServerBySessionId_SessionNotInMap
 * @tc.desc  : Test GetRendererInServerBySessionId with session not in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetRendererInServerBySessionId_SessionNotInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->allRendererMap_.clear();
    auto ret = audioService->GetRendererInServerBySessionId(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, nullptr, "GetRendererInServerBySessionId should return nullptr");
}

/**
 * @tc.name  : Test GetRendererInServerBySessionId API
 * @tc.type  : FUNC
 * @tc.number: AudioService_GetRendererInServerBySessionId_ValidSession
 * @tc.desc  : Test GetRendererInServerBySessionId with valid session.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_GetRendererInServerBySessionId_ValidSession, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    uint32_t sessionId = 200005;
    audioService->allRendererMap_.clear();
    audioService->allRendererMap_[sessionId] = renderer;
    auto ret = audioService->GetRendererInServerBySessionId(sessionId);
    AIUT_EXPECT_NE_INFO_LOG(ret, nullptr, "GetRendererInServerBySessionId should return non-null");
    audioService->allRendererMap_.clear();
}

#ifdef HAS_FEATURE_INNERCAPTURER
/**
 * @tc.name  : Test UnloadModernInnerCapSink API
 * @tc.type  : FUNC
 * @tc.number: AudioService_UnloadModernInnerCapSink_NoProvider
 * @tc.desc  : Test UnloadModernInnerCapSink interface.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_UnloadModernInnerCapSink_NoProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    int32_t ret = audioService->UnloadModernInnerCapSink(1);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "UnloadModernInnerCapSink should not succeed without provider, ret=%d", ret);
}

/**
 * @tc.name  : Test UnloadModernOffloadCapSource API
 * @tc.type  : FUNC
 * @tc.number: AudioService_UnloadModernOffloadCapSource_NoProvider
 * @tc.desc  : Test UnloadModernOffloadCapSource interface.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_UnloadModernOffloadCapSource_NoProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    int32_t ret = audioService->UnloadModernOffloadCapSource();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "UnloadModernOffloadCapSource should not succeed, ret=%d", ret);
}
#endif

/**
 * @tc.name  : Test SetAncoRecordingConfigChanged API
 * @tc.type  : FUNC
 * @tc.number: AudioService_SetAncoRecordingConfigChanged_EmptyCapturerMap
 * @tc.desc  : Test SetAncoRecordingConfigChanged with empty capturer map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SetAncoRecordingConfigChanged_EmptyCapturerMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->allCapturerMap_.clear();
    std::vector<std::string> bundleNameList = {"com.test.app1", "com.test.app2"};
    audioService->SetAncoRecordingConfigChanged(bundleNameList);
    AIUT_EXPECT_TRUE_INFO_LOG(audioService->allCapturerMap_.empty(),
        "allCapturerMap_ should remain empty, size=%zu", audioService->allCapturerMap_.size());
}

/**
 * @tc.name  : Test SetAncoRecordingConfigChanged API
 * @tc.type  : FUNC
 * @tc.number: AudioService_SetAncoRecordingConfigChanged_CapturerInMap
 * @tc.desc  : Test SetAncoRecordingConfigChanged with capturer in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SetAncoRecordingConfigChanged_CapturerInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    audioService->allCapturerMap_.clear();
    audioService->allCapturerMap_[200006] = capturer;
    std::vector<std::string> bundleNameList = {"com.test.app1"};
    audioService->SetAncoRecordingConfigChanged(bundleNameList);
    AIUT_EXPECT_FALSE_INFO_LOG(audioService->allCapturerMap_.empty(),
        "allCapturerMap_ should still contain entry after SetAncoRecordingConfigChanged, size=%zu",
        audioService->allCapturerMap_.size());
    audioService->allCapturerMap_.clear();
}

/**
 * @tc.name  : Test UpdateMuteControlSet boundary
 * @tc.type  : FUNC
 * @tc.number: AudioService_UpdateMuteControlSet_MinStreamIdBoundary
 * @tc.desc  : Test UpdateMuteControlSet with MIN_STREAMID boundary.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_UpdateMuteControlSet_MinStreamIdBoundary, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->mutedSessions_.clear();
    audioService->UpdateMuteControlSet(MIN_STREAMID, true);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->mutedSessions_.count(MIN_STREAMID), 1,
        "mutedSessions_ should contain MIN_STREAMID");

    audioService->UpdateMuteControlSet(MIN_STREAMID, false);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->mutedSessions_.count(MIN_STREAMID), 0,
        "mutedSessions_ should not contain MIN_STREAMID");

    audioService->UpdateMuteControlSet(MIN_STREAMID - 1, true);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->mutedSessions_.count(MIN_STREAMID - 1), 0,
        "mutedSessions_ should not contain invalid sessionId");
    audioService->mutedSessions_.clear();
}

/**
 * @tc.name  : Test RemoveIdFromMuteControlSet boundary
 * @tc.type  : FUNC
 * @tc.number: AudioService_RemoveIdFromMuteControlSet_SessionNotInSet
 * @tc.desc  : Test RemoveIdFromMuteControlSet with session not in set.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_RemoveIdFromMuteControlSet_SessionNotInSet, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    audioService->mutedSessions_.clear();
    audioService->RemoveIdFromMuteControlSet(200007);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->mutedSessions_.count(200007), 0,
        "mutedSessions_ should be empty");
}

/**
 * @tc.name  : Test SaveForegroundList boundary
 * @tc.type  : FUNC
 * @tc.number: AudioService_SaveForegroundList_MaxFgListSize
 * @tc.desc  : Test SaveForegroundList with exactly MAX_FG_LIST_SIZE items.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SaveForegroundList_MaxFgListSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    std::vector<std::string> list;
    list.resize(10);
    for (size_t i = 0; i < list.size(); i++) {
        list[i] = "bundle_" + std::to_string(i);
    }
    audioService->SaveForegroundList(list);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->foregroundSet_.size(), 10,
        "foregroundSet_ should have 10 items, size=%zu", audioService->foregroundSet_.size());
    audioService->foregroundSet_.clear();
    audioService->foregroundUidSet_.clear();
}

/**
 * @tc.name  : Test DumpForegroundList with items
 * @tc.type  : FUNC
 * @tc.number: AudioService_DumpForegroundList_WithItems
 * @tc.desc  : Test DumpForegroundList with items in set.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_DumpForegroundList_WithItems, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->foregroundSet_.clear();
    audioService->foregroundSet_.insert("com.test.dump1");
    audioService->foregroundSet_.insert("com.test.dump2");
    std::string dumpString;
    audioService->DumpForegroundList(dumpString);
    AIUT_EXPECT_TRUE_INFO_LOG(dumpString.find("com.test.dump1") != std::string::npos,
        "dumpString should contain dump1");
    AIUT_EXPECT_TRUE_INFO_LOG(dumpString.find("com.test.dump2") != std::string::npos,
        "dumpString should contain dump2");
    audioService->foregroundSet_.clear();
}

/**
 * @tc.name  : Test ForceStopAudioStream with invalid type
 * @tc.type  : FUNC
 * @tc.number: AudioService_ForceStopAudioStream_InvalidType
 * @tc.desc  : Test ForceStopAudioStream with invalid audioType.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_ForceStopAudioStream_InvalidType, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");
    StopAudioType invalidType = static_cast<StopAudioType>(999);
    int32_t ret = audioService->ForceStopAudioStream(invalidType);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "ForceStopAudioStream should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test ForceStopAudioStream with renderers
 * @tc.type  : FUNC
 * @tc.number: AudioService_ForceStopAudioStream_WithRenderersAndCapturers
 * @tc.desc  : Test ForceStopAudioStream with renderers in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_ForceStopAudioStream_WithRenderersAndCapturers, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    audioService->allRendererMap_.clear();
    audioService->allRendererMap_[200008] = renderer;
    int32_t ret = audioService->ForceStopAudioStream(STOP_ALL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "ForceStopAudioStream STOP_ALL failed, ret=%d", ret);

    std::shared_ptr<CapturerInServer> capturer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    audioService->allCapturerMap_.clear();
    audioService->allCapturerMap_[200009] = capturer;
    ret = audioService->ForceStopAudioStream(STOP_RECORD);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "ForceStopAudioStream STOP_RECORD failed, ret=%d", ret);

    audioService->allRendererMap_.clear();
    audioService->allCapturerMap_.clear();
}

/**
 * @tc.name  : Test RestoreSession with valid renderer
 * @tc.type  : FUNC
 * @tc.number: AudioService_RestoreSession_ValidRenderer
 * @tc.desc  : Test RestoreSession with valid renderer in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_RestoreSession_ValidRenderer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    uint32_t sessionId = 200010;
    audioService->allRendererMap_.clear();
    audioService->allCapturerMap_.clear();
    audioService->allRendererMap_[sessionId] = renderer;
    RestoreInfo restoreInfo;
    audioService->RestoreSession(sessionId, restoreInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(audioService->allRendererMap_.empty(),
        "allRendererMap_ should still contain entry after RestoreSession, size=%zu",
        audioService->allRendererMap_.size());
    audioService->allRendererMap_.clear();
}

/**
 * @tc.name  : Test RestoreSession with valid capturer
 * @tc.type  : FUNC
 * @tc.number: AudioService_RestoreSession_ValidCapturer
 * @tc.desc  : Test RestoreSession with valid capturer in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_RestoreSession_ValidCapturer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    uint32_t sessionId = 200011;
    audioService->allRendererMap_.clear();
    audioService->allCapturerMap_.clear();
    audioService->allCapturerMap_[sessionId] = capturer;
    RestoreInfo restoreInfo;
    audioService->RestoreSession(sessionId, restoreInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(audioService->allCapturerMap_.empty(),
        "allCapturerMap_ should still contain entry after RestoreSession, size=%zu",
        audioService->allCapturerMap_.size());
    audioService->allCapturerMap_.clear();
}

/**
 * @tc.name  : Test SetNonInterruptMute with capturer null
 * @tc.type  : FUNC
 * @tc.number: AudioService_SetNonInterruptMute_NullCapturerInMap
 * @tc.desc  : Test SetNonInterruptMute with null capturer in map.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SetNonInterruptMute_NullCapturerInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->allCapturerMap_.clear();
    std::shared_ptr<CapturerInServer> nullCapturer = nullptr;
    audioService->allCapturerMap_[200012] = nullCapturer;
    audioService->SetNonInterruptMute(200012, true);
    AIUT_EXPECT_FALSE_INFO_LOG(audioService->allCapturerMap_.empty(),
        "allCapturerMap_ should still contain null entry after SetNonInterruptMute, size=%zu",
        audioService->allCapturerMap_.size());
    audioService->allCapturerMap_.clear();
}

/**
 * @tc.name  : Test SetLatestMuteState with no callback
 * @tc.type  : FUNC
 * @tc.number: AudioService_SetLatestMuteState_NoCallback
 * @tc.desc  : Test SetLatestMuteState with no registered callback.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_SetLatestMuteState_NoCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->muteStateCallbacks_.clear();
    audioService->muteStateMap_.clear();
    audioService->SetLatestMuteState(200013, true);
    AIUT_EXPECT_EQ_INFO_LOG(audioService->muteStateMap_.count(200013), 0,
        "muteStateMap_ should be empty without callback");
}

/**
 * @tc.name  : Test RegisterMuteStateChangeCallback with no existing callback
 * @tc.type  : FUNC
 * @tc.number: AudioService_RegisterMuteStateChangeCallback_NoExistingCallback
 * @tc.desc  : Test RegisterMuteStateChangeCallback with no existing callback and no mute state.
 */
HWTEST_F(AudioServiceCoreAiut, AudioService_RegisterMuteStateChangeCallback_NoExistingCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioService *audioService = AudioService::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(audioService, nullptr, "audioService is null");

    audioService->muteStateCallbacks_.clear();
    audioService->muteStateMap_.clear();
    bool muteFlag = false;
    uint32_t sessionId = 200014;
    MuteStateChangeCallbck callback = [&muteFlag](bool flag) { muteFlag = flag; };
    audioService->RegisterMuteStateChangeCallback(sessionId, callback);
    AIUT_EXPECT_EQ_INFO_LOG(muteFlag, false, "muteFlag should remain false");
}

// ==================== AudioWorkgroup Tests ====================

/**
 * @tc.name  : Test GetWorkgroupId API
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroup_GetWorkgroupId_ReturnsCorrectId
 * @tc.desc  : Test GetWorkgroupId returns correct id.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroup_GetWorkgroupId_ReturnsCorrectId, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(42);
    int32_t ret = workgroup.GetWorkgroupId();
    AIUT_EXPECT_EQ_INFO_LOG(ret, 42, "GetWorkgroupId should return 42, ret=%d", ret);
}

/**
 * @tc.name  : Test GetThreadsNums API
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroup_GetThreadsNums_InitiallyZero
 * @tc.desc  : Test GetThreadsNums returns correct count.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroup_GetThreadsNums_InitiallyZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(1);
    uint32_t ret = workgroup.GetThreadsNums();
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetThreadsNums should return 0 initially, ret=%u", ret);
}

/**
 * @tc.name  : Test GetCgroupLimitId API
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroup_GetCgroupLimitId_InitiallyMinusOne
 * @tc.desc  : Test GetCgroupLimitId returns correct id.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroup_GetCgroupLimitId_InitiallyMinusOne, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(1);
    int32_t ret = workgroup.GetCgroupLimitId();
    AIUT_EXPECT_EQ_INFO_LOG(ret, -1, "GetCgroupLimitId should return -1 initially, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCgroupLimitParams API
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroup_SetCgroupLimitParams_SetsCorrectValues
 * @tc.desc  : Test SetCgroupLimitParams sets correct values.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroup_SetCgroupLimitParams_SetsCorrectValues, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(1);
    workgroup.SetCgroupLimitParams(100, 200);
    int32_t cgroupId = workgroup.GetCgroupLimitId();
    AIUT_EXPECT_EQ_INFO_LOG(cgroupId, 200, "GetCgroupLimitId should return 200, ret=%d", cgroupId);
}

/**
 * @tc.name  : Test RemoveThread API
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroup_RemoveThread_NoConcurrentTask
 * @tc.desc  : Test RemoveThread interface.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroup_RemoveThread_NoConcurrentTask, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(1);
    int32_t tid = 100;
    int32_t result = workgroup.RemoveThread(tid);
    AIUT_EXPECT_NE_INFO_LOG(result, AUDIO_OK, "RemoveThread should not return AUDIO_OK without ConcurrentTask, ret=%d",
        result);
}

/**
 * @tc.name  : Test Start with equal times
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupStart_004
 * @tc.desc  : Test Start with deadlineTime equal to startTime.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroupStart_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(1);
    int32_t result = workgroup.Start(100, 100);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_ERR, "Start with equal times should return AUDIO_ERR, ret=%d", result);
}

/**
 * @tc.name  : Test Start with deadline less than start
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroup_Start_DeadlineLessThanStart
 * @tc.desc  : Test Start with deadlineTime less than startTime.
 */
HWTEST_F(AudioServiceCoreAiut, AudioWorkgroup_Start_DeadlineLessThanStart, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioWorkgroup workgroup(1);
    int32_t result = workgroup.Start(200, 100);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_ERR, "Start with deadline < start should return AUDIO_ERR, ret=%d", result);
}

// ==================== CoreServiceHandler Tests ====================

/**
 * @tc.name  : Test UpdateSessionOperation with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_UpdateSessionOperation_NullProvider
 * @tc.desc  : Test UpdateSessionOperation returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_UpdateSessionOperation_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.UpdateSessionOperation(1, SESSION_OPERATION_CREATE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "UpdateSessionOperation should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test ReloadCaptureSession with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_ReloadCaptureSession_NullProvider
 * @tc.desc  : Test ReloadCaptureSession returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_ReloadCaptureSession_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.ReloadCaptureSession(1, SESSION_OPERATION_CREATE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "ReloadCaptureSession should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test SetDefaultOutputDevice with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_SetDefaultOutputDevice_NullProvider
 * @tc.desc  : Test SetDefaultOutputDevice returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_SetDefaultOutputDevice_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER, 1, STREAM_USAGE_MEDIA, true, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "SetDefaultOutputDevice should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test RecordBootStateTime with non-boot uid
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_RecordBootStateTime_NonBootUid
 * @tc.desc  : Test RecordBootStateTime returns SUCCESS for non-BOOT_ANIMATION_UID.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_RecordBootStateTime_NonBootUid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    int32_t ret = handler.RecordBootStateTime(1, 9999, 1000);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RecordBootStateTime should return SUCCESS for non-boot uid, ret=%d", ret);
}

/**
 * @tc.name  : Test RecordBootStateTime with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_RecordBootStateTime_NullProvider
 * @tc.desc  : Test RecordBootStateTime returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_RecordBootStateTime_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.RecordBootStateTime(1, BOOT_ANIMATION_UID, 1000);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "RecordBootStateTime should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test GetAdapterNameBySessionId with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_GetAdapterNameBySessionId_NullProvider
 * @tc.desc  : Test GetAdapterNameBySessionId returns empty string when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_GetAdapterNameBySessionId_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    std::string ret = handler.GetAdapterNameBySessionId(1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, std::string(""), "GetAdapterNameBySessionId should return empty, ret=%s", ret.c_str());
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test GetModuleNameBySessionId with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_GetModuleNameBySessionId_NullProvider
 * @tc.desc  : Test GetModuleNameBySessionId returns empty string when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_GetModuleNameBySessionId_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    std::string ret = handler.GetModuleNameBySessionId(1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, std::string(""), "GetModuleNameBySessionId should return empty, ret=%s", ret.c_str());
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test GetProcessDeviceInfoBySessionId with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_GetProcessDeviceInfoBySessionId_NullProvider
 * @tc.desc  : Test GetProcessDeviceInfoBySessionId returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_GetProcessDeviceInfoBySessionId_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo streamInfo;
    bool isUltraFast = false;
    int32_t ret = handler.GetProcessDeviceInfoBySessionId(1, deviceInfo, streamInfo, isUltraFast, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "GetProcessDeviceInfoBySessionId should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test GenerateSessionId with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_GenerateSessionId_NullProvider
 * @tc.desc  : Test GenerateSessionId returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_GenerateSessionId_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    uint32_t ret = handler.GenerateSessionId();
    AIUT_EXPECT_EQ_INFO_LOG(ret, static_cast<uint32_t>(ERROR), "GenerateSessionId should return ERROR, ret=%u", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test SetWakeUpAudioCapturerFromAudioServer with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_SetWakeUpAudioCapturer_NullProvider
 * @tc.desc  : Test SetWakeUpAudioCapturerFromAudioServer returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_SetWakeUpAudioCapturer_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    AudioProcessConfig config;
    int32_t ret = handler.SetWakeUpAudioCapturerFromAudioServer(config);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "SetWakeUpAudioCapturer should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test GetPaIndexByPortName with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_GetPaIndexByPortName_NullProvider
 * @tc.desc  : Test GetPaIndexByPortName returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_GetPaIndexByPortName_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    uint32_t ret = handler.GetPaIndexByPortName("primary");
    AIUT_EXPECT_EQ_INFO_LOG(ret, static_cast<uint32_t>(ERROR),
        "GetPaIndexByPortName should return ERROR when provider is null, ret=%u", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test A2dpOffloadGetRenderPosition with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_A2dpOffloadGetRenderPosition_NullProvider
 * @tc.desc  : Test A2dpOffloadGetRenderPosition returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_A2dpOffloadGetRenderPosition_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    int32_t ret = handler.A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "A2dpOffloadGetRenderPosition should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test SetRendererTarget with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_SetRendererTarget_NullProvider
 * @tc.desc  : Test SetRendererTarget returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_SetRendererTarget_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.SetRendererTarget(1, 0, 1, 100, 200);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "SetRendererTarget should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test StartInjection with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_StartInjection_NullProvider
 * @tc.desc  : Test StartInjection returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_StartInjection_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.StartInjection(1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "StartInjection should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test RemoveIdForInjector with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_RemoveIdForInjector_NullProvider
 * @tc.desc  : Test RemoveIdForInjector does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_RemoveIdForInjector_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.RemoveIdForInjector(1);
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after RemoveIdForInjector");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test ReleaseCaptureInjector with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_ReleaseCaptureInjector_NullProvider
 * @tc.desc  : Test ReleaseCaptureInjector does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_ReleaseCaptureInjector_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.ReleaseCaptureInjector();
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after ReleaseCaptureInjector");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test RebuildCaptureInjector with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_RebuildCaptureInjector_NullProvider
 * @tc.desc  : Test RebuildCaptureInjector does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_RebuildCaptureInjector_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.RebuildCaptureInjector(1);
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after RebuildCaptureInjector");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test OnCheckActiveMusicTime with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_OnCheckActiveMusicTime_NullProvider
 * @tc.desc  : Test OnCheckActiveMusicTime does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_OnCheckActiveMusicTime_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.OnCheckActiveMusicTime("test_reason");
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after OnCheckActiveMusicTime");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test CaptureConcurrentCheck with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_CaptureConcurrentCheck_NullProvider
 * @tc.desc  : Test CaptureConcurrentCheck returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_CaptureConcurrentCheck_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.CaptureConcurrentCheck(1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "CaptureConcurrentCheck should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test NotifySessionStandbyStatus with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandlerNotifySessionStandbyStatus_001
 * @tc.desc  : Test NotifySessionStandbyStatus does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandlerNotifySessionStandbyStatus_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.NotifySessionStandbyStatus(1, true);
    handler.NotifySessionStandbyStatus(1, false);
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after NotifySessionStandbyStatus");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test IsSupportIndependentPipe with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_IsSupportIndependentPipe_NullProvider
 * @tc.desc  : Test IsSupportIndependentPipe returns false when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_IsSupportIndependentPipe_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    bool ret = handler.IsSupportIndependentPipe();
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "IsSupportIndependentPipe should return false");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test NotifyStreamStandby with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_NotifyStreamStandby_NullProvider
 * @tc.desc  : Test NotifyStreamStandby returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_NotifyStreamStandby_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.NotifyStreamStandby(1, 100);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "NotifyStreamStandby should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test NotifyStreamStandbyResume with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_NotifyStreamStandbyResume_NullProvider
 * @tc.desc  : Test NotifyStreamStandbyResume returns ERROR when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_NotifyStreamStandbyResume_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    int32_t ret = handler.NotifyStreamStandbyResume(1, 100);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "NotifyStreamStandbyResume should return ERROR, ret=%d", ret);
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test NotifyDeviceStateChange with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_NotifyDeviceStateChange_NullProvider
 * @tc.desc  : Test NotifyDeviceStateChange does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_NotifyDeviceStateChange_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.NotifyDeviceStateChange("local", OUTPUT_DEVICE, true);
    handler.NotifyDeviceStateChange("local", INPUT_DEVICE, false);
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after NotifyDeviceStateChange");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test NotifySilentStateChange with null provider
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_NotifySilentStateChange_NullProvider
 * @tc.desc  : Test NotifySilentStateChange does not crash when provider is null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_NotifySilentStateChange_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    auto originProvider = handler.iCoreServiceProvider_;
    handler.iCoreServiceProvider_ = nullptr;
    handler.NotifySilentStateChange(1, true);
    handler.NotifySilentStateChange(1, false);
    AIUT_EXPECT_EQ_INFO_LOG(handler.iCoreServiceProvider_, nullptr,
        "provider should still be null after NotifySilentStateChange");
    handler.iCoreServiceProvider_ = originProvider;
}

/**
 * @tc.name  : Test ConfigCoreServiceProvider with null
 * @tc.type  : FUNC
 * @tc.number: CoreServiceHandler_ConfigCoreServiceProvider_NullProvider
 * @tc.desc  : Test ConfigCoreServiceProvider returns ERR_INVALID_PARAM for null.
 */
HWTEST_F(AudioServiceCoreAiut, CoreServiceHandler_ConfigCoreServiceProvider_NullProvider, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &handler = CoreServiceHandler::GetInstance();
    sptr<ICoreServiceProviderIpc> provider = nullptr;
    int32_t ret = handler.ConfigCoreServiceProvider(provider);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "ConfigCoreServiceProvider should return ERR_INVALID_PARAM, ret=%d",
        ret);
}

// ==================== AudioSystemloadListener Tests ====================

/**
 * @tc.name  : Test RegisterResSchedSys API
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_RegisterResSchedSys_NoCrash
 * @tc.desc  : Test RegisterResSchedSys does not crash.
 */
HWTEST_F(AudioServiceCoreAiut, AudioSystemloadListener_RegisterResSchedSys_NoCrash, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioSystemloadListener listener;
    listener.RegisterResSchedSys();
    AIUT_EXPECT_EQ_INFO_LOG(AudioService::GetInstance()->currentRendererStreamCnt_, 0,
        "currentRendererStreamCnt_ should be 0 after RegisterResSchedSys, cnt=%d",
        AudioService::GetInstance()->currentRendererStreamCnt_);
}

/**
 * @tc.name  : Test UnregisterResSchedSys API
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_UnregisterResSchedSys_NoCrash
 * @tc.desc  : Test UnregisterResSchedSys does not crash.
 */
HWTEST_F(AudioServiceCoreAiut, AudioSystemloadListener_UnregisterResSchedSys_NoCrash, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioSystemloadListener listener;
    listener.UnregisterResSchedSys();
    AIUT_EXPECT_EQ_INFO_LOG(AudioService::GetInstance()->currentRendererStreamCnt_, 0,
        "currentRendererStreamCnt_ should be 0 after UnregisterResSchedSys, cnt=%d",
        AudioService::GetInstance()->currentRendererStreamCnt_);
}

/**
 * @tc.name  : Test AudioSystemloadListenerHandler constructor
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListenerHandler_ConstructAndDestruct
 * @tc.desc  : Test AudioSystemloadListenerHandler constructor and destructor.
 */
HWTEST_F(AudioServiceCoreAiut, AudioSystemloadListenerHandler_ConstructAndDestruct, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto handler = std::make_shared<AudioSystemloadListenerHandler>();
    AIUT_ASSERT_NE_INFO_LOG(handler, nullptr, "handler is null");
}

} // namespace AudioStandard
} // namespace OHOS
