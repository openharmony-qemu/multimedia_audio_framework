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

#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_policy_server.h"
#include "audio_policy_manager_stub_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
    int32_t systemAbilityId = 3009;
}

void AudioPolicyManagerStubUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerStubUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerStubUnitTest::SetUp(void) {}
void AudioPolicyManagerStubUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_001.
* @tc.desc  : Test SetLowPowerVolumeInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->SetLowPowerVolumeInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_002.
* @tc.desc  : Test GetAudioFocusInfoListInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->interruptService_ = nullptr;
    AudioPolicyManage_->GetAudioFocusInfoListInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_003.
* @tc.desc  : Test QueryEffectSceneModeInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->QueryEffectSceneModeInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_004.
* @tc.desc  : Test SetPlaybackCapturerFilterInfosInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->SetPlaybackCapturerFilterInfosInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    data.WriteInt32(1);
    AudioPolicyManage_->SetPlaybackCapturerFilterInfosInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_005.
* @tc.desc  : Test SetPlaybackCapturerFilterInfosInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(30);
    data.WriteInt32(1);
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->SetPlaybackCapturerFilterInfosInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}


/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_006.
* @tc.desc  : Test CreateAudioInterruptZoneInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->CreateAudioInterruptZoneInternal(data, reply);
    AudioPolicyManage_->RemoveAudioInterruptZonePidsInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_007.
* @tc.desc  : Test CreateAudioInterruptZoneInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_007, TestSize.Level1)
{
    int dsc = 1001;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(dsc);
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->CreateAudioInterruptZoneInternal(data, reply);
    AudioPolicyManage_->RemoveAudioInterruptZonePidsInternal(data, reply);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_008.
* @tc.desc  : Test OnMiddleTenRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_ANAHS_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_ANAHS_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleTenRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_009.
* @tc.desc  : Test OnMiddleNinRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_009, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleNinRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_OUTPUT_DEVICE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_INPUT_DEVICE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_HEAD_TRACKING_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEFAULT_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_QUERY_CLIENT_TYPE_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_010.
* @tc.desc  : Test OnMiddleEigRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_010, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleEigRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::TRIGGER_FETCH_DEVICE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleEigRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::MOVE_TO_NEW_PIPE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleEigRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPTION),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleEigRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_AUDIO_SESSION), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleEigRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_AUDIO_SESSION_ACTIVATED), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleEigRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_011.
* @tc.desc  : Test OnMiddleSevRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_011, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::RELEASE_AUDIO_INTERRUPT_ZONE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_BLUETOOTH_DESCRIPTOR), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::FETCH_OUTPUT_DEVICE_FOR_TRACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SPATIALIZATION_SCENE_TYPE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_REFINER_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_REFINER_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleSevRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_012.
* @tc.desc  : Test OnMiddleSixRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_SPATIAL_DEVICE_STATE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::CREATE_AUDIO_INTERRUPT_ZONE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_AUDIO_INTERRUPT_ZONE_PIDS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::REMOVE_AUDIO_INTERRUPT_ZONE_PIDS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleSixRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_013.
* @tc.desc  : Test OnMiddleFifRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleFifRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_MICROPHONE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFifRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleFifRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_014.
* @tc.desc  : Test OnMiddleFouRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_014, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleFouRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_015.
* @tc.desc  : Test OnMiddleTirRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_015, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SINGLE_STREAM_VOLUME), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_USING_PEMISSION_FROM_PRIVACY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleTirRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_016.
* @tc.desc  : Test OnMiddleSecRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_016, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleSecRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_LOW_POWER_STREM_VOLUME), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleSecRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_LOW_POWRR_STREM_VOLUME), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleSecRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_017.
* @tc.desc  : Test OnMiddleFirRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_017, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_WAKEUP_AUDIOCAPTURER), data, reply, option);\
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::QUERY_MICROPHONE_PERMISSION), data, reply, option);\
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleFirRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_018.
* @tc.desc  : Test OnMiddlesRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_018, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_RINGER_MODE_LEGACY),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddlesRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_019.
* @tc.desc  : Test OnMiddleNinRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_019, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleNinRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
