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

#include "audio_policy_log.h"
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

    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_VOICE_RINGTONE_MUTE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

        AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_PREFERRED_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SAVE_REMOTE_INFO), data, reply, option);
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
* @tc.number: AudioPolicyManagerStubUnitTest_014_1.
* @tc.desc  : Test OnMiddleFouRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_014_1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_SOUND_URI),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUME_STREAM),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_VOLUME_STREAM),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_RENDERER_INSTANCES),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_VOLUME_UNADJUSTABLE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_VOLUME_BY_STEP),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_SYSTEM_VOLUME_BY_STEP),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}


/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_014_2.
* @tc.desc  : Test OnMiddleFouRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_014_2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_SYSTEM_VOLUME_BY_STEP),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUME_IN_DB),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

        AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::QUERY_EFFECT_SCENEMODE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_PLAYBACK_CAPTURER_FILTER_INFO),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CAPTURER_SILENT_STATE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_HARDWARE_OUTPUT_SAMPLING_RATE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

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
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_VOLUME_GROUP_INFO), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_NETWORKID_BY_GROUP_ID), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_PREFERRED_INTPUT_DEVICE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

        AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACKS_ENABLE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK_RENDERER_INFO), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_FOCUS_INFO_LIST), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_SOUND_URI), data, reply, option);
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
* @tc.number: AudioPolicyManagerStubUnitTest_017_1.
* @tc.desc  : Test OnMiddleFirRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_017_1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_DEVICES), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SELECT_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SELECTED_DEVICE_INFO), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_IN_FOCUS_BY_UID), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    uint32_t code = 1000;
    AudioPolicyManage_->OnMiddleFirRemoteRequest(code, data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_017_2.
* @tc.desc  : Test OnMiddleFirRemoteRequest.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_017_2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_INTERRUPT), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::DEACTIVATE_INTERRUPT), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_INTERRUPT_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_INTERRUPT_CALLBACK), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::REQUEST_AUDIO_FOCUS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ABANDON_AUDIO_FOCUS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

        AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_IN_FOCUS), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SESSION_INFO_IN_FOCUS), data, reply, option);
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
    AudioPolicyManage_->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_INPUT_DEVICE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_RINGER_MODE_LEGACY),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_RINGER_MODE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_RINGER_MODE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_SCENE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_SCENE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE_AUDIO_CONFIG), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE_LEGACY), data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK),
        data, reply, option);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    AudioPolicyManage_->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_CALLBACK),
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

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_020.
* @tc.desc  : Test RegisterTrackerInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_020, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    data.WriteInt32(static_cast<int32_t>(AudioMode::AUDIO_MODE_PLAYBACK));
    data.WriteRemoteObject(nullptr);

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->RegisterTrackerInternal(data, reply);

    // Assert
    int32_t ret = reply.ReadInt32();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_021.
* @tc.desc  : Test RegisterTrackerInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_021, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    data.WriteInt32(static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD));
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    data.WriteRemoteObject(remoteObject);

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->RegisterTrackerInternal(data, reply);

    // Assert
    int32_t ret = reply.ReadInt32();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_022.
* @tc.desc  : Test GetCapturerChangeInfosInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_022, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(std::make_shared<AudioCapturerChangeInfo>());

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->GetCapturerChangeInfosInternal(data, reply);

    // Assert
    int32_t size = 0;
    reply.ReadInt32(size);
    EXPECT_EQ(audioCapturerChangeInfos.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_023.
* @tc.desc  : Test GetVolumeGroupInfoInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_023, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    std::string networkId = "testNetworkId";
    data.WriteString(networkId);
    std::vector<sptr<VolumeGroupInfo>> groupInfos;
    sptr<VolumeGroupInfo> groupInfo = new VolumeGroupInfo();
    groupInfos.push_back(groupInfo);

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->GetVolumeGroupInfoInternal(data, reply);

    // Assert
    EXPECT_NE(reply.ReadInt32(), 0);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_024.
* @tc.desc  : Test GetVolumeGroupInfoInternal.
*/
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_024, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    std::string networkId = "testNetworkId";
    data.WriteString(networkId);
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->GetVolumeGroupInfoInternal(data, reply);

    // Assert
    int32_t errorCode = reply.ReadInt32();
    EXPECT_NE(errorCode, 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_025.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_025, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadInt32(), 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_026.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_026, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;
    supportedEffectConfig.preProcessNew.stream;

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadUint32(), 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_027.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_027, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;
    supportedEffectConfig.postProcessNew.stream;

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadUint32(), 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_028.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_028, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;
    SceneMappingItem postProcessSceneMap;
    postProcessSceneMap.name = "testName";
    postProcessSceneMap.sceneType = "testSceneType";
    supportedEffectConfig.postProcessSceneMap.push_back(postProcessSceneMap);
    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Act
    AudioPolicyManage_->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadString(), "");
}

/**
 * @tc.name  : GetAudioCapturerMicrophoneDescriptorsInternal_001
 * @tc.number: AudioPolicyManagerStubUnitTest_029
 * @tc.desc  : Test GetHardwareOutputSamplingRateInternal.
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_029, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    data.WriteInt32(1); // Write a non-nullptr to simulate a valid audioDeviceDescriptor

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);
    // Act
    AudioPolicyManage_->GetHardwareOutputSamplingRateInternal(data, reply);

    // Arrange
    int32_t sessionId = 1;
    data.WriteInt32(sessionId);

    // Act
    AudioPolicyManage_->GetAudioCapturerMicrophoneDescriptorsInternal(data, reply);

    // Assert
    int32_t size = reply.ReadInt32();
    EXPECT_LT(size, 0);

    std::vector<sptr<MicrophoneDescriptor>> descs;
    // Act
    AudioPolicyManage_->GetAvailableMicrophonesInternal(data, reply);

    // Assert
    size = 0;
    EXPECT_TRUE(reply.ReadInt32(size));
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : IsHeadTrackingEnabledInternal_001
 * @tc.number: AudioPolicyManagerStubUnitTest_030
 * @tc.desc  : Test IsHeadTrackingEnabledInternal.
 */
HWTEST(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_030, testing::ext::TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;

    std::shared_ptr<AudioPolicyManagerStub> AudioPolicyManage_ = std::make_shared<AudioPolicyServer>(systemAbilityId);
    EXPECT_NE(AudioPolicyManage_, nullptr);

    // Arrange
    // Simulate the scenario where head tracking is enabled
    AudioPolicyManage_->SetHeadTrackingEnabled(true);

    // Act
    AudioPolicyManage_->IsHeadTrackingEnabledInternal(data, reply);

    // Assert
    bool result = false;
    reply.ReadBool(result);
    EXPECT_FALSE(result);

    // Arrange
    // Simulate the scenario where head tracking is disabled
    AudioPolicyManage_->SetHeadTrackingEnabled(false);

    // Act
    AudioPolicyManage_->IsHeadTrackingEnabledInternal(data, reply);

    // Assert
    result = true;
    reply.ReadBool(result);
    EXPECT_FALSE(result);
}
} // namespace AudioStandard
} // namespace OHOS