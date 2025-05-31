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

#include "audio_policy_server_unit_test.h"
#include "media_monitor_manager.h"
#include "input_manager.h"
#include "privacy_kit.h"
#include "tokenid_kit.h"
#include "common_event_manager.h"
#include "audio_policy_log.h"
#include "client_type_manager.h"
#include "dfx_msg_manager.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_200
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_200, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = true;
    audioPolicyServer->isScreenOffOrLock_ = true;
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_201
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_201, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = false;
    audioPolicyServer->isScreenOffOrLock_ = true;
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_202
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_202, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = true;
    audioPolicyServer->isScreenOffOrLock_ = false;
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_203
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_203, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = true;
    audioPolicyServer->isScreenOffOrLock_ = true;
    VolumeUtils::SetPCVolumeEnable(false);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_204
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_204, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = false;
    audioPolicyServer->isScreenOffOrLock_ = false;
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_205
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_205, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = false;
    audioPolicyServer->isScreenOffOrLock_ = false;
    VolumeUtils::SetPCVolumeEnable(false);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_206
* @tc.desc  : Test AudioPolicyServer::ProcessVolumeKeyEvents
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_206, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    audioPolicyServer->volumeApplyToAll_ = true;
    audioPolicyServer->isScreenOffOrLock_ = false;
    VolumeUtils::SetPCVolumeEnable(false);
    auto ret = audioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_207
* @tc.desc  : Test AudioPolicyServer::SetStreamMute
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_207, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = true;
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    auto ret = audioPolicyServer->SetStreamMute(streamType, mute, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_208
* @tc.desc  : Test AudioPolicyServer::UpdateSystemMuteStateAccordingMusicState
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_208, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = true;
    bool isUpdateUi = true;
    VolumeUtils::SetPCVolumeEnable(true);
    audioPolicyServer->UpdateSystemMuteStateAccordingMusicState(streamType, mute, isUpdateUi);
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(STREAM_VOICE_CALL), AudioStreamType::STREAM_MUSIC);
    EXPECT_EQ(VolumeUtils::IsPCVolumeEnable(), true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_209
* @tc.desc  : Test AudioPolicyServer::UpdateSystemMuteStateAccordingMusicState
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_209, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = false;
    bool isUpdateUi = true;
    VolumeUtils::SetPCVolumeEnable(true);
    audioPolicyServer->UpdateSystemMuteStateAccordingMusicState(streamType, mute, isUpdateUi);
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(STREAM_VOICE_CALL), AudioStreamType::STREAM_MUSIC);
    EXPECT_EQ(VolumeUtils::IsPCVolumeEnable(), true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_210
* @tc.desc  : Test AudioPolicyServer::UpdateSystemMuteStateAccordingMusicState
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_210, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_ALL;
    bool mute = false;
    bool isUpdateUi = true;
    VolumeUtils::SetPCVolumeEnable(true);
    audioPolicyServer->UpdateSystemMuteStateAccordingMusicState(streamType, mute, isUpdateUi);
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(STREAM_ALL), AudioStreamType::STREAM_ALL);
    EXPECT_EQ(VolumeUtils::IsPCVolumeEnable(), true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_211
* @tc.desc  : Test AudioPolicyServer::UpdateSystemMuteStateAccordingMusicState
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_211, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = false;
    bool isUpdateUi = true;
    VolumeUtils::SetPCVolumeEnable(false);
    audioPolicyServer->UpdateSystemMuteStateAccordingMusicState(streamType, mute, isUpdateUi);
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(STREAM_MUSIC), AudioStreamType::STREAM_MUSIC);
    EXPECT_EQ(VolumeUtils::IsPCVolumeEnable(), false);
    EXPECT_EQ(audioPolicyServer->GetStreamMuteInternal(STREAM_SYSTEM), false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_212
* @tc.desc  : Test AudioPolicyServer::GetPreferredInputDeviceDescriptors
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_212, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioPolicyServer->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(audioPolicyServer->eventEntry_, nullptr);

    AudioCapturerInfo captureInfo;
    auto ret = audioPolicyServer->GetPreferredInputDeviceDescriptors(captureInfo);
    EXPECT_NE(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_213
* @tc.desc  : Test AudioPolicyServer::SetMicrophoneMutePersistent
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_213, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    bool isMute = true;
    PolicyType type = EDM_POLICY_TYPE;

    auto ret = audioPolicyServer->SetMicrophoneMutePersistent(isMute, type);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_214
* @tc.desc  : Test AudioPolicyServer::DeactivateAudioInterrupt
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_214, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    AudioInterrupt audioInterrupt;
    int32_t zoneID = 0;
    audioPolicyServer->interruptService_ = nullptr;

    auto ret = audioPolicyServer->DeactivateAudioInterrupt(audioInterrupt, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_215
* @tc.desc  : Test AudioPolicyServer::ActivatePreemptMode
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_215, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    EXPECT_NE(static_cast<uid_t>(IPCSkeleton::GetCallingUid()), audioPolicyServer->PREEMPT_UID);

    auto ret = audioPolicyServer->ActivatePreemptMode();
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_216
* @tc.desc  : Test AudioPolicyServer::DeactivatePreemptMode
*/
HWTEST(AudioPolicyUnitTestSecond, AudioPolicyServer_216, TestSize.Level1)
{
    int32_t systemAbilityId = 0;
    auto audioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId);
    ASSERT_NE(audioPolicyServer, nullptr);

    EXPECT_NE(static_cast<uid_t>(IPCSkeleton::GetCallingUid()), audioPolicyServer->PREEMPT_UID);

    auto ret = audioPolicyServer->DeactivatePreemptMode();
    EXPECT_EQ(ret, ERROR);
}
} // AudioStandard
} // OHOS
