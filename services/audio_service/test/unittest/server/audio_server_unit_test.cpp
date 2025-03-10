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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    bool ret = audioServer->CreatePlaybackCapturerManager();
    EXPECT_EQ(true, ret);
}

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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = ALL_DEVICES_FLAG;
    std::vector<DeviceType> deviceTypes;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceTypes.push_back(deviceType);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_OFFLOAD;
    bool ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    audioServer->RegisterAudioRendererSinkCallback();
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    bool hibernate = true;
    audioServer->CheckHibernateState(hibernate);
}

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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t errorCode = 0;
    sptr<IRemoteObject> obj = audioServer->CreateIpcOfflineStream(errorCode);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    uint32_t sessionId = 1;
    int32_t ret = audioServer->GenerateSessionId(sessionId);
    EXPECT_EQ(ret, ERROR);
}

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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    AudioPlaybackCaptureConfig config;
    int32_t innerCapId = 0;
    int32_t ret = audioServer->CheckCaptureLimit(config, innerCapId);
    EXPECT_EQ(ret, SUCCESS);
}

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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    int32_t innerCapId = 0;
    int32_t ret = audioServer->SetInnerCapLimit(innerCapId);
    EXPECT_EQ(ret, SUCCESS);
}

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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
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
    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(systemAbilityId, true);
    ASSERT_TRUE(audioServer != nullptr);
    uint32_t devMgrType = 0;
    const std::string adapterName = "test";
    bool force = false;
    audioServer->UnloadHdiAdapter(devMgrType, adapterName, force);
}
} // namespace AudioStandard
} //