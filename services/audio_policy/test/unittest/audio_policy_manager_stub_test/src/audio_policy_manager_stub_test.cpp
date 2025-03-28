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

#include "audio_policy_manager_stub_test.h"
#include "audio_policy_server.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
    AudioPolicyManagerStub *g_audioPolicyManagerStub = nullptr;
    const int32_t SYSTEM_ABILITY_ID = 3009;
}

void AudioPolicyManagerStubUnitTest::SetUpTestCase(void)
{
    g_audioPolicyManagerStub = new AudioPolicyServer(SYSTEM_ABILITY_ID);
    ASSERT_TRUE(g_audioPolicyManagerStub != nullptr);
}
void AudioPolicyManagerStubUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerStubUnitTest::SetUp(void) {}
void AudioPolicyManagerStubUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_001.
* @tc.desc  : Test SetLowPowerVolumeInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    g_audioPolicyManagerStub->SetLowPowerVolumeInternal(data, reply);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_002.
* @tc.desc  : Test GetAudioFocusInfoListInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    AudioPolicyServer *audioPolicyServer = new AudioPolicyServer(SYSTEM_ABILITY_ID);
    audioPolicyServer->interruptService_ = nullptr;
    audioPolicyServer->GetAudioFocusInfoListInternal(data, reply);
    EXPECT_NE(audioPolicyServer, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_003.
* @tc.desc  : Test QueryEffectSceneModeInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    g_audioPolicyManagerStub->QueryEffectSceneModeInternal(data, reply);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_006.
* @tc.desc  : Test CreateAudioInterruptZoneInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    g_audioPolicyManagerStub->CreateAudioInterruptZoneInternal(data, reply);
    g_audioPolicyManagerStub->RemoveAudioInterruptZonePidsInternal(data, reply);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_007.
* @tc.desc  : Test CreateAudioInterruptZoneInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_007, TestSize.Level1)
{
    int dsc = 1001;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(dsc);
    g_audioPolicyManagerStub->CreateAudioInterruptZoneInternal(data, reply);
    g_audioPolicyManagerStub->RemoveAudioInterruptZonePidsInternal(data, reply);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_008.
* @tc.desc  : Test OnMiddleTenRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_ANAHS_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_ANAHS_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_VOICE_RINGTONE_MUTE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

        g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_PREFERRED_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SAVE_REMOTE_INFO), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleTenRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_009.
* @tc.desc  : Test OnMiddleNinRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_009, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::GET_INPUT_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_HEAD_TRACKING_ENABLED_FOR_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEFAULT_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_QUERY_CLIENT_TYPE_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_010.
* @tc.desc  : Test OnMiddleEigRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_010, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleEigRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::TRIGGER_FETCH_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleEigRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::MOVE_TO_NEW_PIPE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleEigRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::INJECT_INTERRUPTION), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleEigRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_AUDIO_SESSION), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleEigRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_AUDIO_SESSION_ACTIVATED), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleEigRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_011.
* @tc.desc  : Test OnMiddleSevRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_011, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::RELEASE_AUDIO_INTERRUPT_ZONE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_BLUETOOTH_DESCRIPTOR), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::FETCH_OUTPUT_DEVICE_FOR_TRACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SPATIALIZATION_SCENE_TYPE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_REFINER_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_REFINER_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleSevRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_012.
* @tc.desc  : Test OnMiddleSixRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_SPATIAL_DEVICE_STATE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::CREATE_AUDIO_INTERRUPT_ZONE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_AUDIO_INTERRUPT_ZONE_PIDS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSixRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::REMOVE_AUDIO_INTERRUPT_ZONE_PIDS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleSixRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_013.
* @tc.desc  : Test OnMiddleFifRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleFifRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_MICROPHONE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFifRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleFifRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_014_1.
* @tc.desc  : Test OnMiddleFouRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_014_1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_SOUND_URI),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUME_STREAM),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_VOLUME_STREAM),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_RENDERER_INSTANCES),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_VOLUME_UNADJUSTABLE),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_VOLUME_BY_STEP),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_SYSTEM_VOLUME_BY_STEP),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}


/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_014_2.
* @tc.desc  : Test OnMiddleFouRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_014_2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;


    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_SYSTEM_VOLUME_BY_STEP),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUME_IN_DB),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

        g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::QUERY_EFFECT_SCENEMODE),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_HARDWARE_OUTPUT_SAMPLING_RATE),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleFouRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_015.
* @tc.desc  : Test OnMiddleTirRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_015, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SINGLE_STREAM_VOLUME), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_NETWORKID_BY_GROUP_ID), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_PREFERRED_INTPUT_DEVICE_DESCRIPTORS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

        g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACKS_ENABLE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK_RENDERER_INFO), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleTirRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_016.
* @tc.desc  : Test OnMiddleSecRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_016, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddleSecRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_LOW_POWER_STREM_VOLUME), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleSecRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_LOW_POWRR_STREM_VOLUME), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleSecRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_017_1.
* @tc.desc  : Test OnMiddleFirRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_017_1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;


    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SELECT_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SELECTED_DEVICE_INFO), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_IN_FOCUS_BY_UID), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_017_2.
* @tc.desc  : Test OnMiddleFirRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_017_2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_INTERRUPT), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::DEACTIVATE_INTERRUPT), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_INTERRUPT_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_INTERRUPT_CALLBACK), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::REQUEST_AUDIO_FOCUS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ABANDON_AUDIO_FOCUS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

        g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_IN_FOCUS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleFirRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SESSION_INFO_IN_FOCUS), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_018.
* @tc.desc  : Test OnMiddlesRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_018, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_INPUT_DEVICE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_RINGER_MODE),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_SCENE),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE_AUDIO_CONFIG), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE_LEGACY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_CALLBACK),
        data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddlesRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_019.
* @tc.desc  : Test OnMiddleNinRemoteRequest.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_019, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;


    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY), data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    uint32_t code = 1000;
    g_audioPolicyManagerStub->OnMiddleNinRemoteRequest(code, data, reply, option);
    EXPECT_NE(g_audioPolicyManagerStub, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_020.
* @tc.desc  : Test RegisterTrackerInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_020, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    data.WriteInt32(static_cast<int32_t>(AudioMode::AUDIO_MODE_PLAYBACK));
    data.WriteRemoteObject(nullptr);


    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    // Act
    g_audioPolicyManagerStub->RegisterTrackerInternal(data, reply);

    // Assert
    int32_t ret = reply.ReadInt32();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyManagerStub.
* @tc.number: AudioPolicyManagerStubUnitTest_021.
* @tc.desc  : Test RegisterTrackerInternal.
*/
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_021, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    data.WriteInt32(static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD));
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    data.WriteRemoteObject(remoteObject);


    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    // Act
    g_audioPolicyManagerStub->RegisterTrackerInternal(data, reply);

    // Assert
    int32_t ret = reply.ReadInt32();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_025.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_025, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;


    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    // Act
    g_audioPolicyManagerStub->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadInt32(), 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_026.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_026, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;
    supportedEffectConfig.preProcessNew.stream;


    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    // Act
    g_audioPolicyManagerStub->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadUint32(), 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_027.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_027, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;
    supportedEffectConfig.postProcessNew.stream;


    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    // Act
    g_audioPolicyManagerStub->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadUint32(), 0);
}

/**
 * @tc.name  : Test AudioPolicyManagerStub.
 * @tc.number: AudioPolicyManagerStubUnitTest_028.
 * @tc.desc  : Test QueryEffectSceneModeInternal.
 */
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_028, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // Arrange
    SupportedEffectConfig supportedEffectConfig;
    SceneMappingItem postProcessSceneMap;
    postProcessSceneMap.name = "testName";
    postProcessSceneMap.sceneType = "testSceneType";
    supportedEffectConfig.postProcessSceneMap.push_back(postProcessSceneMap);

    EXPECT_NE(g_audioPolicyManagerStub, nullptr);

    // Act
    g_audioPolicyManagerStub->QueryEffectSceneModeInternal(data, reply);

    // Assert
    EXPECT_EQ(reply.ReadString(), "");
}

/**
 * @tc.name  : IsHeadTrackingEnabledInternal_001
 * @tc.number: AudioPolicyManagerStubUnitTest_030
 * @tc.desc  : Test IsHeadTrackingEnabledInternal.
 */
HWTEST_F(AudioPolicyManagerStubUnitTest, AudioPolicyManagerStubUnitTest_030, testing::ext::TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;

    // Arrange
    // Simulate the scenario where head tracking is enabled
    g_audioPolicyManagerStub->SetHeadTrackingEnabled(true);

    // Act
    g_audioPolicyManagerStub->IsHeadTrackingEnabledInternal(data, reply);

    // Assert
    bool result = false;
    reply.ReadBool(result);
    EXPECT_FALSE(result);

    // Arrange
    // Simulate the scenario where head tracking is disabled
    g_audioPolicyManagerStub->SetHeadTrackingEnabled(false);

    // Act
    g_audioPolicyManagerStub->IsHeadTrackingEnabledInternal(data, reply);

    // Assert
    result = true;
    reply.ReadBool(result);
    EXPECT_FALSE(result);
}
} // namespace AudioStandard
} // namespace OHOS