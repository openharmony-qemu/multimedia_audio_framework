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
#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_server.h"
#include "audio_service.h"
#include "system_ability_definition.h"
#include "audio_service_types.h"
#include "audio_system_load_listener.h"
#include "audio_capturer_types.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioServerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test CreatePlaybackCapturerManager API
 * @tc.type  : FUNC
 * @tc.number: CreatePlaybackCapturerManager_001
 * @tc.desc  : Test CreatePlaybackCapturerManager interface using empty case.
 */
HWTEST(AudioServerUnitTest, CreatePlaybackCapturerManager_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CreatePlaybackCapturerManager_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    bool ret = false;
    audioServer->CreatePlaybackCapturerManager(ret);
    EXPECT_EQ(true, ret);
}
#endif

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_001
 * @tc.desc  : Test SetIORoutes interface using empty case, when type is DEVICE_TYPE_USB_ARM_HEADSET,
                deviceType is DEVICE_TYPE_USB_ARM_HEADSET.
 */
HWTEST(AudioServerUnitTest, SetIORoutes_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest SetIORoutes_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = ALL_DEVICES_FLAG;
    std::vector<DeviceType> deviceTypes;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceTypes.push_back(deviceType);
    bool ret = audioServer->SetIORoutes(type, flag, deviceTypes);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test OnAddSystemAbility API
 * @tc.type  : FUNC
 * @tc.number: OnAddSystemAbility_001
 * @tc.desc  : Test OnAddSystemAbility interface using empty case.
 */
HWTEST(AudioServerUnitTest, OnAddSystemAbility_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest OnAddSystemAbility_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    const std::string deviceId = "";
    audioServer->OnAddSystemAbility(LAST_SYS_ABILITY_ID, deviceId);
}

/**
 * @tc.name  : Test InitMaxRendererStreamCntPerUid API
 * @tc.type  : FUNC
 * @tc.number: InitMaxRendererStreamCntPerUid_001
 * @tc.desc  : Test InitMaxRendererStreamCntPerUid interface using empty case.
 */
HWTEST(AudioServerUnitTest, InitMaxRendererStreamCntPerUid_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest InitMaxRendererStreamCntPerUid_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->InitMaxRendererStreamCntPerUid();
}

/**
 * @tc.name  : Test WriteServiceStartupError API
 * @tc.type  : FUNC
 * @tc.number: WriteServiceStartupError_001
 * @tc.desc  : Test WriteServiceStartupError interface using empty case.
 */
HWTEST(AudioServerUnitTest, WriteServiceStartupError_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest WriteServiceStartupError_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->WriteServiceStartupError();
}

/**
 * @tc.name  : Test CheckMaxRendererInstances API
 * @tc.type  : FUNC
 * @tc.number: CheckMaxRendererInstances_001
 * @tc.desc  : Test CheckMaxRendererInstances interface using empty case.
 */
HWTEST(AudioServerUnitTest, CheckMaxRendererInstances_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckMaxRendererInstances_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t ret = audioServer->CheckMaxRendererInstances();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CheckMaxRendererInstances API
 * @tc.type  : FUNC
 * @tc.number: CheckMaxRendererInstances_002
 * @tc.desc  : Test CheckMaxRendererInstances interface using empty case.
 */
HWTEST(AudioServerUnitTest, CheckMaxRendererInstances_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckMaxRendererInstances_002 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioService::GetInstance()->currentRendererStreamCnt_ = 128;
    int32_t ret = audioServer->CheckMaxRendererInstances();
    EXPECT_EQ(ret, ERR_EXCEED_MAX_STREAM_CNT);
}

/**
 * @tc.name  : Test CheckAndWaitAudioPolicyReady API
 * @tc.type  : FUNC
 * @tc.number: CheckAndWaitAudioPolicyReady_001
 * @tc.desc  : Test CheckAndWaitAudioPolicyReady interface using empty case.
 */
HWTEST(AudioServerUnitTest, CheckAndWaitAudioPolicyReady_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckAndWaitAudioPolicyReady_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t ret = audioServer->CheckAndWaitAudioPolicyReady();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CheckAndWaitAudioPolicyReady API
 * @tc.type  : FUNC
 * @tc.number: CheckAndWaitAudioPolicyReady_002
 * @tc.desc  : Test CheckAndWaitAudioPolicyReady interface using empty case.
 */
HWTEST(AudioServerUnitTest, CheckAndWaitAudioPolicyReady_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckAndWaitAudioPolicyReady_002 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->waitCreateStreamInServerCount_ = 6;
    int32_t ret = audioServer->CheckAndWaitAudioPolicyReady();
    EXPECT_EQ(ret, ERR_RETRY_IN_CLIENT);
}

/**
 * @tc.name  : Test CheckAndWaitAudioPolicyReady API
 * @tc.type  : FUNC
 * @tc.number: CheckAndWaitAudioPolicyReady_003
 * @tc.desc  : Test CheckAndWaitAudioPolicyReady interface using empty case.
 */
HWTEST(AudioServerUnitTest, CheckAndWaitAudioPolicyReady_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckAndWaitAudioPolicyReady_003 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->isAudioPolicyReady_ = true;
    int32_t ret = audioServer->CheckAndWaitAudioPolicyReady();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RegisterAudioCapturerSourceCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioCapturerSourceCallback_001
 * @tc.desc  : Test RegisterAudioCapturerSourceCallback interface using empty case.
 */
HWTEST(AudioServerUnitTest, RegisterAudioCapturerSourceCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest RegisterAudioCapturerSourceCallback_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->RegisterAudioCapturerSourceCallback();
}

/**
 * @tc.name  : Test RegisterAudioRendererSinkCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioRendererSinkCallback_001
 * @tc.desc  : Test RegisterAudioRendererSinkCallback interface using empty case.
 */
HWTEST(AudioServerUnitTest, RegisterAudioRendererSinkCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest RegisterAudioRendererSinkCallback_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->RegisterAudioRendererSinkCallback();
}

/**
 * @tc.name  : Test RegisterDataTransferStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterDataTransferStateChangeCallback_001
 * @tc.desc  : Test RegisterDataTransferStateChangeCallback interface using empty case.
 */
HWTEST(AudioServerUnitTest, RegisterDataTransferStateChangeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest RegisterDataTransferStateChangeCallback_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->RegisterDataTransferStateChangeCallback();
}

/**
 * @tc.name  : Test OnRenderSinkStateChange API
 * @tc.type  : FUNC
 * @tc.number: OnRenderSinkStateChange_001
 * @tc.desc  : Test OnRenderSinkStateChange interface using empty case.
 */
HWTEST(AudioServerUnitTest, OnRenderSinkStateChange_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest OnRenderSinkStateChange_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    uint32_t sinkId = 1;
    bool started = true;
    audioServer->OnRenderSinkStateChange(sinkId, started);
}

/**
 * @tc.name  : Test CheckHibernateState API
 * @tc.type  : FUNC
 * @tc.number: CheckHibernateState_001
 * @tc.desc  : Test CheckHibernateState interface using empty case.
 */
HWTEST(AudioServerUnitTest, CheckHibernateState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckHibernateState_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    bool hibernate = true;
    audioServer->CheckHibernateState(hibernate);
}

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test CreateIpcOfflineStream API
 * @tc.type  : FUNC
 * @tc.number: CreateIpcOfflineStream_001
 * @tc.desc  : Test CreateIpcOfflineStream interface using empty case.
 */
HWTEST(AudioServerUnitTest, CreateIpcOfflineStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CreateIpcOfflineStream_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t errorCode = 0;
    sptr<IRemoteObject> obj = nullptr;
    audioServer->CreateIpcOfflineStream(errorCode, obj);
    EXPECT_NE(obj, nullptr);
}

/**
 * @tc.name  : Test GetOfflineAudioEffectChains API
 * @tc.type  : FUNC
 * @tc.number: GetOfflineAudioEffectChains_001
 * @tc.desc  : Test GetOfflineAudioEffectChains interface using empty case.
 */
HWTEST(AudioServerUnitTest, GetOfflineAudioEffectChains_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest GetOfflineAudioEffectChains_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    std::vector<std::string> effectChains = {};
    int32_t ret = audioServer->GetOfflineAudioEffectChains(effectChains);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GenerateSessionId API
 * @tc.type  : FUNC
 * @tc.number: GenerateSessionId_001
 * @tc.desc  : Test GenerateSessionId interface using empty case.
 */
HWTEST(AudioServerUnitTest, GenerateSessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest GenerateSessionId_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    uint32_t sessionId = 1;
    int32_t ret = audioServer->GenerateSessionId(sessionId);
    EXPECT_EQ(ret, ERROR);
}
#endif

/**
 * @tc.name  : Test GetAllSinkInputs API
 * @tc.type  : FUNC
 * @tc.number: GetAllSinkInputs_001
 * @tc.desc  : Test GetAllSinkInputs interface using empty case.
 */
HWTEST(AudioServerUnitTest, GetAllSinkInputs_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest GetAllSinkInputs_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    std::vector<SinkInput> sinkInputs = {};
    audioServer->GetAllSinkInputs(sinkInputs);
}

/**
 * @tc.name  : Test NotifyAudioPolicyReady API
 * @tc.type  : FUNC
 * @tc.number: NotifyAudioPolicyReady_001
 * @tc.desc  : Test NotifyAudioPolicyReady interface using empty case.
 */
HWTEST(AudioServerUnitTest, NotifyAudioPolicyReady_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest NotifyAudioPolicyReady_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->NotifyAudioPolicyReady();
}

/**
 * @tc.name  : Test CheckCaptureLimit API
 * @tc.type  : FUNC
 * @tc.number: CheckCaptureLimit_001
 * @tc.desc  : Test CheckCaptureLimit interface using empty case.
 */
#ifdef HAS_FEATURE_INNERCAPTURER
HWTEST(AudioServerUnitTest, CheckCaptureLimit_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckCaptureLimit_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioPlaybackCaptureConfig config;
    int32_t innerCapId = 0;
    int32_t ret = audioServer->CheckCaptureLimit(config, innerCapId);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test SetInnerCapLimit API
 * @tc.type  : FUNC
 * @tc.number: SetInnerCapLimit_001
 * @tc.desc  : Test SetInnerCapLimit interface using empty case.
 */
HWTEST(AudioServerUnitTest, SetInnerCapLimit_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest SetInnerCapLimit_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t innerCapId = 0;
    int32_t ret = audioServer->SetInnerCapLimit(innerCapId);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
 * @tc.name  : Test ReleaseCaptureLimit API
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureLimit_001
 * @tc.desc  : Test ReleaseCaptureLimit interface using empty case.
 */
HWTEST(AudioServerUnitTest, ReleaseCaptureLimit_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ReleaseCaptureLimit_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t innerCapId = 0;
    int32_t ret = audioServer->ReleaseCaptureLimit(innerCapId);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
 * @tc.name  : Test LoadHdiAdapter API
 * @tc.type  : FUNC
 * @tc.number: LoadHdiAdapter_001
 * @tc.desc  : Test LoadHdiAdapter interface using empty case.
 */
HWTEST(AudioServerUnitTest, LoadHdiAdapter_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest LoadHdiAdapter_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    uint32_t devMgrType = 0;
    std::string adapterName = "test";
    int32_t ret = audioServer->LoadHdiAdapter(devMgrType, adapterName);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test UnloadHdiAdapter API
 * @tc.type  : FUNC
 * @tc.number: UnloadHdiAdapter_001
 * @tc.desc  : Test UnloadHdiAdapter interface using empty case.
 */
HWTEST(AudioServerUnitTest, UnloadHdiAdapter_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest UnloadHdiAdapter_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    uint32_t devMgrType = 0;
    const std::string adapterName = "test";
    bool force = false;
    audioServer->UnloadHdiAdapter(devMgrType, adapterName, force);
}

/**
 * @tc.name  : Test ParseAudioParameter API
 * @tc.type  : FUNC
 * @tc.number: ParseAudioParameter_001
 * @tc.desc  : Test ParseAudioParameter interface.
 */
HWTEST(AudioServerUnitTest, ParseAudioParameter_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ParseAudioParameter_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    EXPECT_FALSE(audioServer->isAudioParameterParsed_.load());
    audioServer->ParseAudioParameter();
    EXPECT_TRUE(audioServer->isAudioParameterParsed_.load());
}

/**
 * @tc.name  : Test ParseAudioParameter API
 * @tc.type  : FUNC
 * @tc.number: ParseAudioParameter_002
 * @tc.desc  : Test ParseAudioParameter interface.
 */
HWTEST(AudioServerUnitTest, ParseAudioParameter_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ParseAudioParameter_002 start");
    int32_t systemAbilityId = 3001;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    EXPECT_FALSE(audioServer->isAudioParameterParsed_.load());
    audioServer->ParseAudioParameter();
    EXPECT_TRUE(audioServer->isAudioParameterParsed_.load());
}

/**
 * @tc.name  : Test OnSystemloadLevel API
 * @tc.type  : FUNC
 * @tc.number: OnSystemloadLevel_001
 * @tc.desc  : Test OnSystemloadLevel interface.
 */
HWTEST(AudioServerUnitTest, OnSystemloadLevel_001, TestSize.Level1)
{
    AudioSystemloadListener audioSystemloadListener;
    AudioService::GetInstance()->currentRendererStreamCnt_ = 10;
    audioSystemloadListener.OnSystemloadLevel(1);
    audioSystemloadListener.OnSystemloadLevel(2);
    audioSystemloadListener.OnSystemloadLevel(3);
    audioSystemloadListener.OnSystemloadLevel(4);
    audioSystemloadListener.OnSystemloadLevel(5);
    audioSystemloadListener.OnSystemloadLevel(6);
    audioSystemloadListener.OnSystemloadLevel(7);
    AudioService::GetInstance()->currentRendererStreamCnt_ = 0;
    audioSystemloadListener.OnSystemloadLevel(7);
    EXPECT_EQ(AudioService::GetInstance()->currentRendererStreamCnt_, 0);
}

/**
 * @tc.name  : Test NeedDelayCreateSource API
 * @tc.type  : FUNC
 * @tc.number: NeedDelayCreateSource_001
 * @tc.desc  : Test NeedDelayCreateSource interface.
 */
HWTEST(AudioServerUnitTest, NeedDelayCreateSource_001, TestSize.Level1)
{
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    EXPECT_EQ(audioServer->NeedDelayCreateSource(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_MMAP), true);
    EXPECT_EQ(audioServer->NeedDelayCreateSource(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_USB), true);
    EXPECT_EQ(audioServer->NeedDelayCreateSource(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_USB), false);
    EXPECT_EQ(audioServer->NeedDelayCreateSource(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_MMAP), false);
    EXPECT_EQ(audioServer->NeedDelayCreateSource(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB), false);
    EXPECT_EQ(audioServer->NeedDelayCreateSource(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_MMAP), false);
}

/**
 * @tc.name  : Test NeedDelayCreateSink API
 * @tc.type  : FUNC
 * @tc.number: NeedDelayCreateSink_001
 * @tc.desc  : Test NeedDelayCreateSink interface.
 */
HWTEST(AudioServerUnitTest, NeedDelayCreateSink_001, TestSize.Level1)
{
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    EXPECT_EQ(audioServer->NeedDelayCreateSink(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_MMAP), true);
    EXPECT_EQ(audioServer->NeedDelayCreateSink(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_USB), true);
    EXPECT_EQ(audioServer->NeedDelayCreateSink(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_USB), false);
    EXPECT_EQ(audioServer->NeedDelayCreateSink(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_MMAP), false);
    EXPECT_EQ(audioServer->NeedDelayCreateSink(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB), false);
    EXPECT_EQ(audioServer->NeedDelayCreateSink(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_MMAP), false);
}

/**
 * @tc.name  : Test AudioSystemloadListener OnSystemloadLevel with high load level
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_OnSystemloadLevel_High_001
 * @tc.desc  : Test AudioSystemloadListener::OnSystemloadLevel with high system load level
 */
HWTEST(AudioServerUnitTest, AudioSystemloadListener_OnSystemloadLevel_High_001, TestSize.Level1)
{
    // Create a mock system load listener
    auto systemLoadListener = std::make_shared<AudioSystemloadListener>();
    EXPECT_NE(systemLoadListener, nullptr);

    // Test with high system load level (should trigger disable spatial audio with delay)
    // We can't easily test the actual functionality without mocking dependencies,
    // but we can verify the function doesn't crash
    systemLoadListener->OnSystemloadLevel(7); // SYSTEM_LOAD_LEVEL_ESCAPE

    // Test with medium-high system load level
    systemLoadListener->OnSystemloadLevel(6); // SYSTEM_LOAD_LEVEL_EMERGENCY
}

/**
 * @tc.name  : Test AudioSystemloadListener OnSystemloadLevel with low load level
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_OnSystemloadLevel_Low_001
 * @tc.desc  : Test AudioSystemloadListener::OnSystemloadLevel with low system load level
 */
HWTEST(AudioServerUnitTest, AudioSystemloadListener_OnSystemloadLevel_Low_001, TestSize.Level1)
{
    // Create a mock system load listener
    auto systemLoadListener = std::make_shared<AudioSystemloadListener>();
    EXPECT_NE(systemLoadListener, nullptr);

    // Test with low system load level (should immediately enable spatial audio)
    systemLoadListener->OnSystemloadLevel(4); // Below control level
}

/**
 * @tc.name  : Test AudioSystemloadListener OnSystemloadLevel with medium load level
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_OnSystemloadLevel_Medium_001
 * @tc.desc  : Test AudioSystemloadListener::OnSystemloadLevel with medium system load level
 */
HWTEST(AudioServerUnitTest, AudioSystemloadListener_OnSystemloadLevel_Medium_001, TestSize.Level1)
{
    // Create a mock system load listener
    auto systemLoadListener = std::make_shared<AudioSystemloadListener>();
    EXPECT_NE(systemLoadListener, nullptr);

    // Test with medium system load level (should schedule delayed enable)
    systemLoadListener->OnSystemloadLevel(5); // Between levels
}

/**
 * @tc.name  : Test AudioSystemloadListener OnSystemloadLevel with empty audio streams
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_OnSystemloadLevel_Empty_001
 * @tc.desc  : Test AudioSystemloadListener::OnSystemloadLevel with empty audio streams
 */
HWTEST(AudioServerUnitTest, AudioSystemloadListener_OnSystemloadLevel_Empty_001, TestSize.Level1)
{
    // Create a mock system load listener
    auto systemLoadListener = std::make_shared<AudioSystemloadListener>();
    EXPECT_NE(systemLoadListener, nullptr);

    // Test with empty audio streams (should return early)
    systemLoadListener->OnSystemloadLevel(3); // Any level when streams are empty
}

/**
 * @tc.name  : Test AudioSystemloadListener OnSystemloadLevel edge cases
 * @tc.type  : FUNC
 * @tc.number: AudioSystemloadListener_OnSystemloadLevel_Edge_001
 * @tc.desc  : Test AudioSystemloadListener::OnSystemloadLevel with edge system load levels
 */
HWTEST(AudioServerUnitTest, AudioSystemloadListener_OnSystemloadLevel_Edge_001, TestSize.Level1)
{
    // Create a mock system load listener
    auto systemLoadListener = std::make_shared<AudioSystemloadListener>();
    EXPECT_NE(systemLoadListener, nullptr);

    // Test with edge system load levels
    systemLoadListener->OnSystemloadLevel(0); // Minimum level
    systemLoadListener->OnSystemloadLevel(10); // Beyond maximum level
}

/**
 * @tc.name  : Test AudioParameterValueCheck with valid parameters
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_001
 * @tc.desc  : Test AudioParameterValueCheck with valid parameters that should pass
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with valid parameters (no forbidden keywords)
    std::string key = "volume_level";
    std::string value = "50";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    // Test with another valid parameter
    key = "sample_rate";
    value = "48000";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with forbidden keyword asr_aec_mode
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_002
 * @tc.desc  : Test AudioParameterValueCheck with forbidden keyword asr_aec_mode
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_002 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with forbidden keyword "asr_aec_mode"
    std::string key = "aec_config";
    std::string value = "asr_aec_mode=1";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with forbidden keyword ASR_AEC
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_003
 * @tc.desc  : Test AudioParameterValueCheck with forbidden keyword ASR_AEC
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_003 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with forbidden keyword "ASR_AEC"
    std::string key = "aec_config";
    std::string value = "ASR_AEC=enabled";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with forbidden keyword TTS_2_DEVICE
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_004
 * @tc.desc  : Test AudioParameterValueCheck with forbidden keyword TTS_2_DEVICE
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_004 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with forbidden keyword "TTS_2_DEVICE"
    std::string key = "routing_config";
    (void)key;
    std::string value = "TTS_2_DEVICE=primary";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with forbidden keyword audio2voicetx
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_005
 * @tc.desc  : Test AudioParameterValueCheck with forbidden keyword audio2voicetx
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_005 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with forbidden keyword "audio2voicetx"
    std::string key = "voice_config";
    (void)key;
    std::string value = "audio2voicetx=true";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with forbidden keyword output_mute
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_006
 * @tc.desc  : Test AudioParameterValueCheck with forbidden keyword output_mute=true/false
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_006, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_006 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with forbidden keyword "output_mute=true"
    std::string key = "mute_config";
    (void)key;
    std::string value = "output_mute=true";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);

    // Test with forbidden keyword "output_mute=false"
    value = "output_mute=false";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with empty strings
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_007
 * @tc.desc  : Test AudioParameterValueCheck with empty key and value strings
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_007 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with empty key and value
    std::string key = "";
    std::string value = "";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    // Test with empty value only
    key = "some_key";
    value = "";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with multiple forbidden keywords
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_008
 * @tc.desc  : Test AudioParameterValueCheck with multiple forbidden keywords in value
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_008, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_008 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with multiple forbidden keywords
    std::string key = "config";
    (void)key;
    std::string value = "asr_aec_mode=1;TTS_2_DEVICE=primary";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with partial keyword match
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_009
 * @tc.desc  : Test AudioParameterValueCheck with partial keyword match (should fail)
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_009, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_009 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with partial match of forbidden keyword
    std::string key = "config";
    (void)key;
    std::string value = "prefix_asr_aec_mode_suffix";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with similar but valid parameters
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_010
 * @tc.desc  : Test AudioParameterValueCheck with similar but valid parameters
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_010, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_010 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with similar but valid parameters
    std::string key = "aec_config";
    std::string value = "aec_mode=1"; // Not "asr_aec_mode"
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    value = "aec_enabled=true"; // Not "asr_aec_mode"
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with special characters
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_011
 * @tc.desc  : Test AudioParameterValueCheck with special characters in value
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_011, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_011 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with special characters (should pass)
    std::string key = "config";
    std::string value = "value@#$%^&*()";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    // Test with special characters including forbidden keyword
    value = "test_asr_aec_mode_test";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with numeric values
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_012
 * @tc.desc  : Test AudioParameterValueCheck with numeric values
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_012, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_012 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with numeric values (should pass)
    std::string key = "volume";
    std::string value = "100";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    value = "0";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    value = "-1";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with long values
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_013
 * @tc.desc  : Test AudioParameterValueCheck with long string values
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_013, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_013 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test with long valid value
    std::string key = "config";
    std::string value(1000, 'a'); // 1000 'a' characters
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);

    // Test with long value containing forbidden keyword
    value = std::string(500, 'a') + "asr_aec_mode" + std::string(500, 'b');
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);
}

/**
 * @tc.name  : Test AudioParameterValueCheck with case sensitivity
 * @tc.type  : FUNC
 * @tc.number: AudioParameterValueCheck_014
 * @tc.desc  : Test AudioParameterValueCheck with case sensitivity
 */
HWTEST(AudioServerUnitTest, AudioParameterValueCheck_014, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioParameterValueCheck_014 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);

    // Test case sensitivity - ASR_AEC should fail
    std::string key = "config";
    std::string value = "ASR_AEC=1";
    bool boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);

    // Test case sensitivity - asr_aec_mode should fail
    value = "asr_aec_mode=1";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_FALSE(boolResult);

    // Test with different case - should pass
    value = "Asr_Aec_Mode=1";
    boolResult = audioServer->AudioParameterValueCheck(key, value);
    EXPECT_TRUE(boolResult);
}

/**
 * @tc.name  : Test CheckRecorderPermission API with loopback mode
 * @tc.type  : FUNC
 * @tc.number: CheckRecorderPermission_Loopback_001
 * @tc.desc  : Test CheckRecorderPermission returns true for loopback mode with with MIC and FAST flag.
 */
HWTEST(AudioServerUnitTest, CheckRecorderPermission_Loopback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckRecorderPermission_Loopback_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    AudioProcessConfig config;
    config.appInfo.appUid = 1000;
    config.callerUid = 1000;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    config.capturerInfo.isLoopback = true;
    bool result = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test CheckRecorderPermission API without loopback mode
 * @tc.type  : FUNC
 * @tc.number: CheckRecorderPermission_NonLoopback_001
 * @tc.desc  : Test CheckRecorderPermission with MIC and FAST flag but no loopback mode.
 */
HWTEST(AudioServerUnitTest, CheckRecorderPermission_NonLoopback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckRecorderPermission_NonLoopback_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    AudioProcessConfig config;
    config.appInfo.appUid = 1000;
    config.callerUid = 1000;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    config.capturerInfo.isLoopback = false;
    bool result = audioServer->CheckRecorderPermission(config);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test CheckRecorderPermission API with different source type
 * @tc.type  : FUNC
 * @tc.number: CheckRecorderPermission_SourceType_001
 * @tc.desc  : Test CheckRecorderPermission with VOICE_COMMUNICATION and loopback mode.
 */
HWTEST(AudioServerUnitTest, CheckRecorderPermission_SourceType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckRecorderPermission_SourceType_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    AudioProcessConfig config;
    config.appInfo.appUid = 1000;
    config.callerUid = 1000;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    config.capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    config.capturerInfo.isLoopback = true;
    bool result = audioServer->CheckRecorderPermission(config);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test CheckRecorderPermission API without FAST flag
 * @tc.type  : FUNC
 * @tc.number: CheckRecorderPermission_FastFlag_001
 * @tc.desc  : Test CheckRecorderPermission with loopback mode but without FAST flag.
 */
HWTEST(AudioServerUnitTest, CheckRecorderPermission_FastFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest CheckRecorderPermission_FastFlag_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    AudioProcessConfig config;
    config.appInfo.appUid = 1000;
    config.callerUid = 1000;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = 0;
    config.capturerInfo.isLoopback = true;
    bool result = audioServer->CheckRecorderPermission(config);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test ReportWakeupEvent API
 * @tc.type  : FUNC
 * @tc.number: ReportWakeupEvent_001
 * @tc.desc  : Test ReportWakeupEvent interface with valid params.
 */
HWTEST(AudioServerUnitTest, ReportWakeupEvent_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ReportWakeupEvent_001 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioWakeupTrackInfo audioWakeupTrackInfo;
    audioWakeupTrackInfo.stage = STAGE_FWK_CREATE_ENTER;
    audioWakeupTrackInfo.result = WAKEUP_RESULT_DEFAULT;
    audioWakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;
    int32_t ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test ReportWakeupEvent API
 * @tc.type  : FUNC
 * @tc.number: ReportWakeupEvent_002
 * @tc.desc  : Test ReportWakeupEvent with different stage codes.
 */
HWTEST(AudioServerUnitTest, ReportWakeupEvent_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ReportWakeupEvent_002 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioWakeupTrackInfo audioWakeupTrackInfo;
    audioWakeupTrackInfo.result = WAKEUP_RESULT_SUCC;
    audioWakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;

    audioWakeupTrackInfo.stage = STAGE_FWK_CREATE_EXIT;
    int32_t ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.stage = STAGE_FWK_START_ENTER;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.stage = STAGE_FWK_START_EXIT;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.stage = STAGE_FWK_END;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test ReportWakeupEvent API
 * @tc.type  : FUNC
 * @tc.number: ReportWakeupEvent_003
 * @tc.desc  : Test ReportWakeupEvent with different results.
 */
HWTEST(AudioServerUnitTest, ReportWakeupEvent_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ReportWakeupEvent_003 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioWakeupTrackInfo audioWakeupTrackInfo;
    audioWakeupTrackInfo.stage = STAGE_FWK_CREATE_ENTER;
    audioWakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;

    audioWakeupTrackInfo.result = WAKEUP_RESULT_DEFAULT;
    int32_t ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.result = WAKEUP_RESULT_SUCC;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.result = WAKEUP_RESULT_FAIL;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test ReportWakeupEvent API
 * @tc.type  : FUNC
 * @tc.number: ReportWakeupEvent_004
 * @tc.desc  : Test ReportWakeupEvent with different error codes.
 */
HWTEST(AudioServerUnitTest, ReportWakeupEvent_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest ReportWakeupEvent_004 start");
    int32_t systemAbilityId = 100;
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioWakeupTrackInfo audioWakeupTrackInfo;
    audioWakeupTrackInfo.stage = STAGE_FWK_CREATE_ENTER;
    audioWakeupTrackInfo.result = WAKEUP_RESULT_FAIL;

    audioWakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;
    int32_t ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_CREATE_ERROR;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    audioWakeupTrackInfo.errCode = WAKEUP_TRACK_FWK_START_ERROR;
    ret = audioServer->ReportWakeupEvent(audioWakeupTrackInfo);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_001
 * @tc.desc  : Test AudioWakeupTrackInfo serialization and deserialization.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_001 start");
    AudioWakeupTrackInfo originalInfo;
    originalInfo.stage = STAGE_FWK_CREATE_ENTER;
    originalInfo.result = WAKEUP_RESULT_DEFAULT;
    originalInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;

    Parcel parcel;
    bool ret = originalInfo.Marshalling(parcel);
    EXPECT_TRUE(ret);

    AudioWakeupTrackInfo *unmarshalledInfo = AudioWakeupTrackInfo::Unmarshalling(parcel);
    EXPECT_NE(nullptr, unmarshalledInfo);
    EXPECT_EQ(unmarshalledInfo->stage, originalInfo.stage);
    EXPECT_EQ(unmarshalledInfo->result, originalInfo.result);
    EXPECT_EQ(unmarshalledInfo->errCode, originalInfo.errCode);
    delete unmarshalledInfo;
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_002
 * @tc.desc  : Test AudioWakeupTrackInfo with different values.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_002 start");
    AudioWakeupTrackInfo originalInfo;
    originalInfo.stage = STAGE_FWK_START_EXIT;
    originalInfo.result = WAKEUP_RESULT_FAIL;
    originalInfo.errCode = WAKEUP_TRACK_FWK_START_ERROR;

    Parcel parcel;
    bool ret = originalInfo.Marshalling(parcel);
    EXPECT_TRUE(ret);

    AudioWakeupTrackInfo *unmarshalledInfo = AudioWakeupTrackInfo::Unmarshalling(parcel);
    EXPECT_NE(nullptr, unmarshalledInfo);
    EXPECT_EQ(unmarshalledInfo->stage, STAGE_FWK_START_EXIT);
    EXPECT_EQ(unmarshalledInfo->result, WAKEUP_RESULT_FAIL);
    EXPECT_EQ(unmarshalledInfo->errCode, WAKEUP_TRACK_FWK_START_ERROR);
    delete unmarshalledInfo;
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_003
 * @tc.desc  : Test AudioWakeupTrackInfo with create error.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_003 start");
    AudioWakeupTrackInfo originalInfo;
    originalInfo.stage = STAGE_FWK_CREATE_EXIT;
    originalInfo.result = WAKEUP_RESULT_FAIL;
    originalInfo.errCode = WAKEUP_TRACK_FWK_CREATE_ERROR;

    Parcel parcel;
    bool ret = originalInfo.Marshalling(parcel);
    EXPECT_TRUE(ret);

    AudioWakeupTrackInfo *unmarshalledInfo = AudioWakeupTrackInfo::Unmarshalling(parcel);
    EXPECT_NE(nullptr, unmarshalledInfo);
    EXPECT_EQ(unmarshalledInfo->stage, STAGE_FWK_CREATE_EXIT);
    EXPECT_EQ(unmarshalledInfo->result, WAKEUP_RESULT_FAIL);
    EXPECT_EQ(unmarshalledInfo->errCode, WAKEUP_TRACK_FWK_CREATE_ERROR);
    delete unmarshalledInfo;
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_004
 * @tc.desc  : Test AudioWakeupTrackInfo with success result.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_004 start");
    AudioWakeupTrackInfo originalInfo;
    originalInfo.stage = STAGE_FWK_START_EXIT;
    originalInfo.result = WAKEUP_RESULT_SUCC;
    originalInfo.errCode = WAKEUP_TRACK_FWK_NO_ERROR;

    Parcel parcel;
    bool ret = originalInfo.Marshalling(parcel);
    EXPECT_TRUE(ret);

    AudioWakeupTrackInfo *unmarshalledInfo = AudioWakeupTrackInfo::Unmarshalling(parcel);
    EXPECT_NE(nullptr, unmarshalledInfo);
    EXPECT_EQ(unmarshalledInfo->stage, STAGE_FWK_START_EXIT);
    EXPECT_EQ(unmarshalledInfo->result, WAKEUP_RESULT_SUCC);
    EXPECT_EQ(unmarshalledInfo->errCode, WAKEUP_TRACK_FWK_NO_ERROR);
    delete unmarshalledInfo;
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_005
 * @tc.desc  : Test AudioWakeupTrackInfo with all stage codes.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_005 start");
    std::vector<AudioWakeupStageCode> stages = {
        STAGE_FWK_CREATE_ENTER,
        STAGE_FWK_CREATE_EXIT,
        STAGE_FWK_START_ENTER,
        STAGE_FWK_START_EXIT,
        STAGE_FWK_END
    };
    for (auto stage : stages) {
        AudioWakeupTrackInfo info;
        info.stage = stage;
        info.result = WAKEUP_RESULT_DEFAULT;
        info.errCode = WAKEUP_TRACK_FWK_NO_ERROR;
        Parcel parcel;
        EXPECT_TRUE(info.Marshalling(parcel));
        AudioWakeupTrackInfo *unmarshalled = AudioWakeupTrackInfo::Unmarshalling(parcel);
        EXPECT_NE(nullptr, unmarshalled);
        EXPECT_EQ(unmarshalled->stage, stage);
        delete unmarshalled;
    }
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_006
 * @tc.desc  : Test AudioWakeupTrackInfo with all error codes.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_006, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_006 start");
    std::vector<AudioWakeupErrorCode> errCodes = {
        WAKEUP_TRACK_FWK_NO_ERROR,
        WAKEUP_TRACK_FWK_CREATE_ERROR,
        WAKEUP_TRACK_FWK_START_ERROR
    };
    for (auto errCode : errCodes) {
        AudioWakeupTrackInfo info;
        info.stage = STAGE_FWK_CREATE_ENTER;
        info.result = WAKEUP_RESULT_FAIL;
        info.errCode = errCode;
        Parcel parcel;
        EXPECT_TRUE(info.Marshalling(parcel));
        AudioWakeupTrackInfo *unmarshalled = AudioWakeupTrackInfo::Unmarshalling(parcel);
        EXPECT_NE(nullptr, unmarshalled);
        EXPECT_EQ(unmarshalled->errCode, errCode);
        delete unmarshalled;
    }
}

/**
 * @tc.name  : Test AudioWakeupTrackInfo Marshalling/Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: AudioWakeupTrackInfoMarshalling_007
 * @tc.desc  : Test AudioWakeupTrackInfo with all results.
 */
HWTEST(AudioServerUnitTest, AudioWakeupTrackInfoMarshalling_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioServerUnitTest AudioWakeupTrackInfoMarshalling_007 start");
    std::vector<AudioWakeupResult> results = {
        WAKEUP_RESULT_DEFAULT,
        WAKEUP_RESULT_SUCC,
        WAKEUP_RESULT_FAIL
    };
    for (auto result : results) {
        AudioWakeupTrackInfo info;
        info.stage = STAGE_FWK_START_EXIT;
        info.result = result;
        info.errCode = WAKEUP_TRACK_FWK_NO_ERROR;
        Parcel parcel;
        EXPECT_TRUE(info.Marshalling(parcel));
        AudioWakeupTrackInfo *unmarshalled = AudioWakeupTrackInfo::Unmarshalling(parcel);
        EXPECT_NE(nullptr, unmarshalled);
        EXPECT_EQ(unmarshalled->result, result);
        delete unmarshalled;
    }
}

} // namespace AudioStandard
} //
