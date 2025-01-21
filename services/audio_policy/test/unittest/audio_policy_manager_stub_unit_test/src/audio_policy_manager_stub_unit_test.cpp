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

#include "audio_policy_manager_stub_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_policy_service.h"
#include "inner_event.h"
#include "event_handler.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyManagerStubUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerStubUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerStubUnitTest::SetUp(void) {}
void AudioPolicyManagerStubUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_001
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleTenRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_001, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_ANAHS_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleTenRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_002
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleTenRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_002, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_ANAHS_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleTenRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_003
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleTenRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_003, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUMELEVEL);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleTenRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_004
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_004, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_OUTPUT_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_005
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_005, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_INPUT_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_006
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_006, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_ENABLED_FOR_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_007
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_007, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_ENABLED_FOR_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_008
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_008, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_ENABLED_FOR_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_009
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_009, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_HEAD_TRACKING_ENABLED_FOR_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_010
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_010, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEFAULT_OUTPUT_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_011
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_011, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_QUERY_CLIENT_TYPE_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_012
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_012, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_ENHANCE_PROPERTY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_013
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_013, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_014
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_014, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_015
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_015, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_016
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_016, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_017
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_017, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_018
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleNinRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_018, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_TRACKER);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_019
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_019, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::TRIGGER_FETCH_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_020
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_020, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::MOVE_TO_NEW_PIPE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_021
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_021, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_022
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_022, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_AUDIO_SESSION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_023
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_023, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::DEACTIVATE_AUDIO_SESSION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_024
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_024, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_AUDIO_SESSION_ACTIVATED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_025
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleEigRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_025, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_TRACKER);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_026
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_026, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::RELEASE_AUDIO_INTERRUPT_ZONE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_027
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_027, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALL_DEVICE_ACTIVE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_028
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_028, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CONVERTER_CONFIG);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_029
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_029, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CONVERTER_CONFIG);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_030
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_030, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SPATIALIZATION_SCENE_TYPE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_031
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_031, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_032
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_032, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_REFINER_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_033
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSevRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_033, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_REFINER_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_034
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_034, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_SPATIAL_DEVICE_STATE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_035
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_035, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::CONFIG_DISTRIBUTED_ROUTING_ROLE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_036
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_036, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DISTRIBUTED_ROUTING_ROLE_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_037
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_037, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_DISTRIBUTED_ROUTING_ROLE_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_038
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_038, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::CREATE_AUDIO_INTERRUPT_ZONE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_039
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_039, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_AUDIO_INTERRUPT_ZONE_PIDS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_040
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSixRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_040, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::REMOVE_AUDIO_INTERRUPT_ZONE_PIDS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_041
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFifRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_041, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEVICE_ABSOLUTE_VOLUME_SUPPORTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFifRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_042
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFifRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_042, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_A2DP_DEVICE_VOLUME);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFifRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_043
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFifRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_043, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AVAILABLE_DEVICE_CHANGE_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFifRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_044
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFifRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_044, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AVAILABLE_DEVICE_CHANGE_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFifRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_045
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFouRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_045, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFouRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_046
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleTirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_046, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SINGLE_STREAM_VOLUME);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleTirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_048
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleTirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_048, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_FOCUS_INFO_LIST);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleTirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_049
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSecRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_049, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::RECONFIGURE_CHANNEL);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSecRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_050
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSecRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_050, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_LOW_POWER_STREM_VOLUME);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSecRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_051
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleSecRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_051, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_LOW_POWRR_STREM_VOLUME);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleSecRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_052
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_052, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_INTERRUPT_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_053
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_053, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_INTERRUPT_CALLBACK);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_054
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_054, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::REQUEST_AUDIO_FOCUS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_055
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_055, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::ABANDON_AUDIO_FOCUS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_058
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddleFirRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_058, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SELECT_OUTPUT_DEVICE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddleFirRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_059
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddlesRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_059, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_RINGER_MODE_LEGACY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddlesRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_060
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddlesRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_060, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddlesRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_061
 * @tc.desc  : Test AudioPolicyManagerStub::OnMiddlesRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_061, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE_LEGACY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMiddlesRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_062
 * @tc.desc  : Test AudioPolicyManagerStub::OnMidRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_062, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_STREAM_MUTE_LEGACY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMidRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_063
 * @tc.desc  : Test AudioPolicyManagerStub::OnMidRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_063, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEVICE_ACTIVE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMidRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_064
 * @tc.desc  : Test AudioPolicyManagerStub::OnMidRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_064, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_DEVICE_ACTIVE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMidRemoteRequest(code, data, reply, option);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyManagerStub_065
 * @tc.desc  : Test AudioPolicyManagerStub::OnMidRemoteRequest
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStub_065, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    bool runOnCreate = true;
    auto ptrAudioPolicyManagerStub = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyManagerStub, nullptr);

    uint32_t code = static_cast<uint32_t>(AudioPolicyInterfaceCode::LOAD_SPLIT_MODULE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ptrAudioPolicyManagerStub->OnMidRemoteRequest(code, data, reply, option);
}
} // namespace AudioStandard
} // namespace OHOS