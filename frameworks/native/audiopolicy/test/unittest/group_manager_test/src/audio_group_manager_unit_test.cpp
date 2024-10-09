/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioGroupManagerUnitTest"
#endif

#include "audio_group_manager_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_log.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr int32_t MAX_VOL = 15;
    constexpr int32_t MIN_VOL = 0;
    std::string networkId = "LocalDevice";
    constexpr int32_t ERROR_62980101 = -62980101;
}

void AudioGroupManagerUnitTest::SetUpTestCase(void) {}
void AudioGroupManagerUnitTest::TearDownTestCase(void) {}
void AudioGroupManagerUnitTest::SetUp(void) {}
void AudioGroupManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioVolume API
* @tc.number: AudioVolume_001
* @tc.desc  : Test AudioVolume manager interface multiple requests
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, AudioVolume_001, TestSize.Level1)
{
    int32_t volume = 0;
    bool mute = true;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ALL, volume);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ALL);
        EXPECT_EQ(volume, ret);

        ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_ALL, mute);
        EXPECT_EQ(SUCCESS, ret);

        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_ALL, isMute);
        EXPECT_EQ(true, isMute);
    }
}

/**
* @tc.name  : Test AudioVolume API
* @tc.number: AudioVolume_002
* @tc.desc  : Test AudioVolume manager interface multiple requests
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, AudioVolume_002, TestSize.Level1)
{
    int32_t volume = 2;
    bool mute = true;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ALARM, volume);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ALARM);
        EXPECT_EQ(volume, ret);

        ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_ALARM, mute);
        EXPECT_EQ(SUCCESS, ret);

        // stream alarm can not set mute
        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_ALARM, isMute);
        EXPECT_EQ(false, isMute);
    }
}

/**
* @tc.name  : Test AudioVolume API
* @tc.number: AudioVolume_003
* @tc.desc  : Test AudioVolume manager interface multiple requests
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, AudioVolume_003, TestSize.Level1)
{
    int32_t volume = 4;
    bool mute = true;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ACCESSIBILITY, volume);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ACCESSIBILITY);
        EXPECT_EQ(volume, ret);

        ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_ACCESSIBILITY, mute);
        EXPECT_EQ(SUCCESS, ret);

        // stream accessibility can not set mute
        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_ACCESSIBILITY, isMute);
        EXPECT_EQ(false, isMute);
    }
}

/**
* @tc.name  : Test AudioVolume API
* @tc.number: AudioVolume_004
* @tc.desc  : Test AudioVolume manager interface multiple requests
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, AudioVolume_004, TestSize.Level1)
{
    int32_t volume = 5;
    bool mute = true;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ULTRASONIC, volume);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ULTRASONIC);
        EXPECT_EQ(volume, ret);

        ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_ULTRASONIC, mute);
        EXPECT_EQ(SUCCESS, ret);

        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_ULTRASONIC, isMute);
        EXPECT_EQ(true, isMute);
    }
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_001
* @tc.desc  : Test setting volume of ringtone stream with max volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, MAX_VOL);
        EXPECT_EQ(SUCCESS, ret);

        int32_t volume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
        EXPECT_EQ(MAX_VOL, volume);
    }
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_002
* @tc.desc  : Test setting volume of ringtone stream with min volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_002, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, MIN_VOL);
        EXPECT_EQ(SUCCESS, ret);

        int32_t volume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
        EXPECT_EQ(MIN_VOL, volume);
    }
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_003
* @tc.desc  : Test setting volume of media stream with max volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_003, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_MUSIC, MAX_VOL);
        EXPECT_EQ(SUCCESS, ret);

        int32_t mediaVol = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_MUSIC);
        EXPECT_EQ(MAX_VOL, mediaVol);

        int32_t ringVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
        EXPECT_EQ(MIN_VOL, ringVolume);
    }
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_004
* @tc.desc  : Test setting volume of alarm stream with error volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_004, TestSize.Level0)
{
    int32_t ErrorVolume = 17;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t FirstVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ALARM);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ALARM, ErrorVolume);
        EXPECT_NE(SUCCESS, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ALARM);
        EXPECT_EQ(FirstVolume, SecondVolume);
    }
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_005
* @tc.desc  : Test setting volume of accessibility stream with error volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_005, TestSize.Level0)
{
    int32_t ErrorVolume = 18;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t FirstVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ACCESSIBILITY);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ACCESSIBILITY, ErrorVolume);
        EXPECT_NE(SUCCESS, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ACCESSIBILITY);
        EXPECT_EQ(FirstVolume, SecondVolume);
    }
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_006
* @tc.desc  : Test setting volume of ultrasonic stream with error volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_006, TestSize.Level0)
{
    int32_t ErrorVolume = -5;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t FirstVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ULTRASONIC);

        auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ULTRASONIC, ErrorVolume);
        EXPECT_NE(SUCCESS, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ULTRASONIC);
        EXPECT_EQ(FirstVolume, SecondVolume);
    }
}

/**
* @tc.name  : Test GetMaxVolume API
* @tc.number: GetMaxVolumeTest_001
* @tc.desc  : Test GetMaxVolume of media stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, GetMaxVolumeTest_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
    int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t mediaVol = audioGroupMngr_->GetMaxVolume(AudioVolumeType::STREAM_MUSIC);
        EXPECT_EQ(MAX_VOL, mediaVol);

        int32_t ringVolume = audioGroupMngr_->GetMaxVolume(AudioVolumeType::STREAM_RING);
        EXPECT_EQ(MAX_VOL, ringVolume);
    }
}

/**
* @tc.name  : Test GetMaxVolume API
* @tc.number: GetMinVolumeTest_001
* @tc.desc  : Test GetMaxVolume of media stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, GetMinVolumeTest_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t mediaVol = audioGroupMngr_->GetMinVolume(AudioVolumeType::STREAM_MUSIC);
        EXPECT_EQ(MIN_VOL, mediaVol);

        int32_t ringVolume = audioGroupMngr_->GetMinVolume(AudioVolumeType::STREAM_RING);
        EXPECT_EQ(MIN_VOL, ringVolume);
    }
}

/**
* @tc.name  : Test SetMute API
* @tc.number: SetMute_001
* @tc.desc  : Test mute functionality of ringtone stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_RING, true);
        EXPECT_EQ(SUCCESS, ret);

        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING, isMute);
        EXPECT_EQ(true, isMute);
    }
}

/**
* @tc.name  : Test SetMute IsStreamMute API
* @tc.number: SetMute_002
* @tc.desc  : Test unmute functionality of ringtone stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_002, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_RING, false);
        EXPECT_EQ(SUCCESS, ret);

        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING, isMute);
        EXPECT_EQ(false, isMute);
    }
}

/**
* @tc.name  : Test SetMute IsStreamMute API
* @tc.number: SetMute_003
* @tc.desc  : Test mute functionality of media stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_003, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_MUSIC, true);
        EXPECT_EQ(SUCCESS, ret);

        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_MUSIC, isMute);
        EXPECT_EQ(true, isMute);
    }
}

/**
* @tc.name  : Test SetMute IsStreamMute API
* @tc.number: SetMute_004
* @tc.desc  : Test unmute functionality of media stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_004, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_MUSIC, false);
        EXPECT_EQ(SUCCESS, ret);

        bool isMute;
        ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING, isMute);
        EXPECT_EQ(false, isMute);
    }
}

/**
* @tc.name  : Test IsVolumeUnadjustable API
* @tc.number: Audio_Group_Manager_IsVolumeUnadjustable_001
* @tc.desc  : Test volume is unadjustable or adjustable functionality
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_IsVolumeUnadjustable_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        auto ret = audioGroupMngr_->IsVolumeUnadjustable();
        AUDIO_INFO_LOG("Is volume unadjustable: %{public}d", ret);
        EXPECT_EQ(false, ret);
    }
}

/**
* @tc.name  : Test AdjustVolumeByStep API
* @tc.number: Audio_Group_Manager_AdjustVolumeByStep_001
* @tc.desc  : Test adjust volume to up by step functionality
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_AdjustVolumeByStep_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int32_t ret = -1;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_MUSIC, 7);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->AdjustVolumeByStep(VolumeAdjustType::VOLUME_UP);
        AUDIO_INFO_LOG("Adjust volume by step: %{public}d", ret);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
* @tc.name  : Test AdjustVolumeByStep API
* @tc.number: Audio_Group_Manager_AdjustVolumeByStep_002
* @tc.desc  : Test adjust volume to down by step functionality
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_AdjustVolumeByStep_002, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int32_t ret = -1;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        ret = audioGroupMngr_->AdjustVolumeByStep(VolumeAdjustType::VOLUME_DOWN);
        AUDIO_INFO_LOG("Adjust volume by step: %{public}d", ret);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
* @tc.name  : Test AdjustSystemVolumeByStep API
* @tc.number: Audio_Group_Manager_AdjustSystemVolumeByStep_001
* @tc.desc  : Test adjust system volume by step to up of STREAM_RECORDING stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_AdjustSystemVolumeByStep_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int32_t FirstVolume = 7;
    int32_t ret = -1;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, FirstVolume);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->AdjustSystemVolumeByStep(AudioVolumeType::STREAM_RING,
                                                        VolumeAdjustType::VOLUME_UP);
        AUDIO_INFO_LOG("Adjust system volume by step: %{public}d", ret);
        EXPECT_EQ(SUCCESS, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
        EXPECT_GT(SecondVolume, FirstVolume);
    }
}

/**
* @tc.name  : Test AdjustSystemVolumeByStep API
* @tc.number: Audio_Group_Manager_AdjustSystemVolumeByStep_002
* @tc.desc  : Test adjust system volume by step to down of STREAM_RECORDING stream
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_AdjustSystemVolumeByStep_002, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int32_t FirstVolume = 7;
    int32_t ret = -1;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, FirstVolume);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->AdjustSystemVolumeByStep(AudioVolumeType::STREAM_RING,
                                                        VolumeAdjustType::VOLUME_DOWN);
        AUDIO_INFO_LOG("Adjust system volume by step: %{public}d", ret);
        EXPECT_EQ(SUCCESS, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
        EXPECT_GT(FirstVolume, SecondVolume);
    }
}

/**
* @tc.name  : Test AdjustSystemVolumeByStep API
* @tc.number: Audio_Group_Manager_AdjustSystemVolumeByStep_003
* @tc.desc  : Test adjust system volume by step to up of STREAM_RING stream when is max volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_AdjustSystemVolumeByStep_003, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int32_t ret = -1;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        auto maxVol = audioGroupMngr_->GetMaxVolume(AudioVolumeType::STREAM_RING);
        ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, maxVol);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->AdjustSystemVolumeByStep(AudioVolumeType::STREAM_RING,
                                                        VolumeAdjustType::VOLUME_UP);
        AUDIO_INFO_LOG("Adjust system volume by step: %{public}d", ret);
        EXPECT_EQ(ERROR_62980101, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
        EXPECT_EQ(maxVol, SecondVolume);
    }
}

/**
* @tc.name  : Test AdjustSystemVolumeByStep API
* @tc.number: Audio_Group_Manager_AdjustSystemVolumeByStep_003
* @tc.desc  : Test adjust system volume by step to down of STREAM_MUSIC stream when is min volume
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_AdjustSystemVolumeByStep_004, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int32_t ret = -1;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        auto minVol = audioGroupMngr_->GetMinVolume(AudioVolumeType::STREAM_MUSIC);
        ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_MUSIC, minVol);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioGroupMngr_->AdjustSystemVolumeByStep(AudioVolumeType::STREAM_MUSIC,
                                                        VolumeAdjustType::VOLUME_DOWN);
        AUDIO_INFO_LOG("Adjust system volume by step: %{public}d", ret);
        EXPECT_EQ(ERROR_62980101, ret);

        int32_t SecondVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_MUSIC);
        EXPECT_EQ(minVol, SecondVolume);
    }
}

/**
* @tc.name  : Test GetSystemVolumeInDb API
* @tc.number: Audio_Group_Manager_GetSystemVolumeInDb_001
* @tc.desc  : Test get volume db with alarm streamtype and speaker devicetype when volume is 3
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_GetSystemVolumeInDb_001, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int vol = 3;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        float db = audioGroupMngr_->GetSystemVolumeInDb(AudioVolumeType::STREAM_ALARM, vol,
                                                        DeviceType::DEVICE_TYPE_SPEAKER);
        AUDIO_INFO_LOG("Get system volume in Db: %{public}f", db);
        EXPECT_LT(SUCCESS, db);
    }
}

/**
* @tc.name  : Test GetSystemVolumeInDb API
* @tc.number: Audio_Group_Manager_GetSystemVolumeInDb_002
* @tc.desc  : Test get volume db when the stream type is changed to voice call
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_GetSystemVolumeInDb_002, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int vol = 3;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        float db = audioGroupMngr_->GetSystemVolumeInDb(AudioVolumeType::STREAM_VOICE_CALL, vol,
                                                        DeviceType::DEVICE_TYPE_SPEAKER);
        AUDIO_INFO_LOG("Get system volume in Db: %{public}f", db);
        EXPECT_LT(SUCCESS, db);
    }
}

/**
* @tc.name  : Test GetSystemVolumeInDb API
* @tc.number: Audio_Group_Manager_GetSystemVolumeInDb_003
* @tc.desc  : Test get volume db wthen the volume is changed to 4
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_GetSystemVolumeInDb_003, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int vol = 4;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        float db = audioGroupMngr_->GetSystemVolumeInDb(AudioVolumeType::STREAM_ALARM, vol,
                                                        DeviceType::DEVICE_TYPE_SPEAKER);
        AUDIO_INFO_LOG("Get system volume in Db: %{public}f", db);
        EXPECT_LT(SUCCESS, db);
    }
}

/**
* @tc.name  : Test GetSystemVolumeInDb API
* @tc.number: Audio_Group_Manager_GetSystemVolumeInDb_004
* @tc.desc  : Test get volume db when the device type is changed to earpiece
* @tc.require: issueI5M1XV
*/
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_GetSystemVolumeInDb_004, TestSize.Level0)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    int vol = 3;
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        float db = audioGroupMngr_->GetSystemVolumeInDb(AudioVolumeType::STREAM_ALARM, vol,
                                                        DeviceType::DEVICE_TYPE_EARPIECE);
        AUDIO_INFO_LOG("Get system volume in Db: %{public}f", db);
        EXPECT_LT(SUCCESS, db);
    }
}

/**
 * @tc.name  : Test SetRingerMode API
 * @tc.number: Audio_Group_Manager_SetRingerMode_001
 * @tc.desc  : SetRingerMode
 * @tc.require: issueI5M1XV
 */
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_SetRingerMode_001, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRingerMode audioRingerMode = AudioRingerMode::RINGER_MODE_NORMAL;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        ret = audioGroupMngr_->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
        EXPECT_EQ(SUCCESS, ret);
        audioRingerMode = audioGroupMngr_->GetRingerMode();
        EXPECT_EQ(audioRingerMode, AudioRingerMode::RINGER_MODE_SILENT);
    
        ret = audioGroupMngr_->SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
        EXPECT_EQ(SUCCESS, ret);
        audioRingerMode = audioGroupMngr_->GetRingerMode();
        EXPECT_EQ(audioRingerMode, AudioRingerMode::RINGER_MODE_VIBRATE);

        ret = audioGroupMngr_->SetRingerMode(AudioRingerMode::RINGER_MODE_NORMAL);
        EXPECT_EQ(SUCCESS, ret);
        audioRingerMode = audioGroupMngr_->GetRingerMode();
        EXPECT_EQ(audioRingerMode, AudioRingerMode::RINGER_MODE_NORMAL);
    }
}

/**
 * @tc.name  : Test SetRingerModeCallback API
 * @tc.number: Audio_Group_Manager_SetRingerModeCallback_001
 * @tc.desc  : SetRingerModeCallback
 * @tc.require: issueI5M1XV
 */
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_SetRingerModeCallback_001, TestSize.Level0)
{
    int32_t ret = -1;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t clientId = getpid();
        std::shared_ptr<AudioRingerModeCallback> callback = nullptr;
        ret = audioGroupMngr_->SetRingerModeCallback(clientId, callback);
        EXPECT_EQ(ERR_INVALID_PARAM, ret);
        ret = audioGroupMngr_->UnsetRingerModeCallback(clientId);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
 * @tc.name  : Test SetRingerModeCallback API
 * @tc.number: Audio_Group_Manager_SetRingerModeCallback_002
 * @tc.desc  : SetRingerModeCallback
 * @tc.require: issueI5M1XV
 */
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_SetRingerModeCallback_002, TestSize.Level0)
{
    int32_t ret = -1;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);

        int32_t clientId = getpid();
        std::shared_ptr<AudioRingerModeCallback> callback =  make_shared<AudioRingerModeCallbackTest>();
        ret = audioGroupMngr_->SetRingerModeCallback(clientId, callback);
        EXPECT_EQ(SUCCESS, ret);
        ret = audioGroupMngr_->UnsetRingerModeCallback(clientId);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
 * @tc.name  : Test SetMicrophoneMute API
 * @tc.number: Audio_Group_Manager_SetMicrophoneMute_001
 * @tc.desc  : SetMicrophoneMute
 * @tc.require: issueI5M1XV
 */
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_SetMicrophoneMute_001, TestSize.Level0)
{
    int32_t ret = -1;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        
        audioGroupMngr_->SetMicrophoneMutePersistent(false, PolicyType::PRIVACY_POLCIY_TYPE);
        bool isMuteFirst = audioGroupMngr_->IsMicrophoneMute();
        ret = audioGroupMngr_->SetMicrophoneMute(!isMuteFirst);
        EXPECT_EQ(SUCCESS, ret);
        bool isMuteSecond = audioGroupMngr_->IsMicrophoneMute();
        EXPECT_EQ(isMuteSecond, !isMuteFirst);
    }
}

/**
 * @tc.name  : Test SetMicrophoneMutePersistent API
 * @tc.number: Audio_Group_Manager_SetMicrophoneMutePersistent_001
 * @tc.desc  : SetMicrophoneMutePersistent
 * @tc.require: issueI5M1XV
 */
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_SetMicrophoneMutePersistent_001, TestSize.Level0)
{
    int32_t ret = -1;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        bool isMutePersistentFirst = audioGroupMngr_->GetPersistentMicMuteState();
        ret = audioGroupMngr_->SetMicrophoneMutePersistent(!isMutePersistentFirst, PolicyType::PRIVACY_POLCIY_TYPE);
        EXPECT_EQ(SUCCESS, ret);
        bool isMutePersistentSecond = audioGroupMngr_->GetPersistentMicMuteState();
        EXPECT_EQ(isMutePersistentFirst, !isMutePersistentSecond);
    }
}

/**
 * @tc.name  : Test GetPersistentMicMuteState API
 * @tc.number: Audio_Group_Manager_GetPersistentMicMuteState_001
 * @tc.desc  : GetPersistentMicMuteState
 * @tc.require: issueI5M1XV
 */
HWTEST(AudioGroupManagerUnitTest, Audio_Group_Manager_GetPersistentMicMuteState_001, TestSize.Level0)
{
    int32_t ret = -1;
    std::vector<sptr<VolumeGroupInfo>> infos;
    AudioSystemManager::GetInstance()->GetVolumeGroups(networkId, infos);
    if (infos.size() > 0) {
        int32_t groupId = infos[0]->volumeGroupId_;
        auto audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
        ret = audioGroupMngr_->SetMicrophoneMutePersistent(true, PolicyType::PRIVACY_POLCIY_TYPE);
        EXPECT_EQ(SUCCESS, ret);
        bool isMutePersistent = audioGroupMngr_->GetPersistentMicMuteState();
        EXPECT_EQ(isMutePersistent, true);
    }
}
} // namespace AudioStandard
} // namespace OHOS
