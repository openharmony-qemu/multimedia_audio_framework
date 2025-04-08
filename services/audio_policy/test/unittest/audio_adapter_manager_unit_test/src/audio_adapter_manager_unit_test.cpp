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
#include "audio_policy_utils.h"
#include "audio_adapter_manager_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_interrupt_service.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static AudioAdapterManager *audioAdapterManager_;

void AudioAdapterManagerUnitTest::SetUpTestCase(void) {}
void AudioAdapterManagerUnitTest::TearDownTestCase(void) {}
void AudioAdapterManagerUnitTest::SetUp(void) {}
void AudioAdapterManagerUnitTest::TearDown(void) {}

std::shared_ptr<AudioInterruptService> GetTnterruptServiceTest()
{
    return std::make_shared<AudioInterruptService>();
}
/**
 * @tc.name: IsAppVolumeMute_001
 * @tc.desc: Test IsAppVolumeMute when owned is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsAppVolumeMute_001, TestSize.Level1)
{
    int32_t appUid = 12345;
    bool owned = true;
    bool isMute = true;
    bool result = AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: IsAppVolumeMute_002
 * @tc.desc: Test IsAppVolumeMute when owned is false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsAppVolumeMute_002, TestSize.Level1)
{
    int32_t appUid = 12345;
    bool owned = false;
    bool isMute = true;
    bool result = AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetSystemVolumeLevelWithDevice_001
 * @tc.desc: Test SetSystemVolumeLevelWithDevice when deviceType is different from currentActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSystemVolumeLevelWithDevice_001, TestSize.Level1)
{
    audioAdapterManager_->Init();
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t minVolume = audioAdapterManager_->GetMinVolumeLevel(streamType);
    int32_t maxVolume = audioAdapterManager_->GetMaxVolumeLevel(streamType);
    ASSERT_TRUE(volumeLevel >= minVolume && volumeLevel <= maxVolume);
    int32_t result = audioAdapterManager_->SetSystemVolumeLevelWithDevice(streamType, volumeLevel, deviceType);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: SetSystemVolumeLevelWithDevice_002
 * @tc.desc: Test SetSystemVolumeLevelWithDevice when deviceType is same as currentActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSystemVolumeLevelWithDevice_002, TestSize.Level1)
{
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t minVolume = audioAdapterManager_->GetMinVolumeLevel(streamType);
    int32_t maxVolume = audioAdapterManager_->GetMaxVolumeLevel(streamType);
    ASSERT_TRUE(volumeLevel >= minVolume && volumeLevel <= maxVolume);
    int32_t result = audioAdapterManager_->SetSystemVolumeLevelWithDevice(streamType, volumeLevel, deviceType);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: SetDeviceSafeVolumeh_001
 * @tc.desc: Test SetDeviceSafeVolume when entering the first if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetDeviceSafeVolume_001, TestSize.Level1)
{
    bool safeVolumeCall = true;
    AudioAdapterManager::GetInstance().SetDeviceSafeVolume(STREAM_MUSIC, 5);
    AudioAdapterManager::GetInstance().Init();
    AudioAdapterManager::GetInstance().SetRestoreVolumeFlag(safeVolumeCall);
    AudioAdapterManager::GetInstance().SetDeviceSafeVolume(STREAM_RING, 5);
    EXPECT_TRUE(safeVolumeCall);
}

/**
 * @tc.name: HandleStreamMuteStatus_001
 * @tc.desc: Test HandleStreamMuteStatus when deviceType is not DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleStreamMuteStatus_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, streamUsage, deviceType);
    EXPECT_TRUE(mute);
}

/**
 * @tc.name: HandleStreamMuteStatus_002
 * @tc.desc: Test HandleStreamMuteStatus when deviceType is DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleStreamMuteStatus_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, streamUsage, deviceType);
    EXPECT_TRUE(mute);
}

/**
 * @tc.name: SetOffloadVolume_001
 * @tc.desc: Test SetOffloadVolume.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetOffloadVolume_001, TestSize.Level1)
{
    audioAdapterManager_->Init();
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 1;
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb);

    streamType = STREAM_SPEECH;
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb);

    streamType = STREAM_SYSTEM;
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_DP;
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb);

    streamType = STREAM_SYSTEM;
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    auto interruptServiceTest = GetTnterruptServiceTest();
    audioAdapterManager_->audioServerProxy_ = interruptServiceTest->GetAudioServerProxy();
    EXPECT_NE(audioAdapterManager_->audioServerProxy_, nullptr);
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb);
    EXPECT_EQ(streamType, STREAM_SYSTEM);
}

/**
 * @tc.name: SetOffloadSessionId_001
 * @tc.desc: Test SetOffloadSessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetOffloadSessionId_001, TestSize.Level1)
{
    uint32_t sessionId = MIN_STREAMID - 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);

    sessionId = MAX_STREAMID + 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);

    sessionId = MIN_STREAMID + 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);
}

/**
 * @tc.name: SetDoubleRingVolumeDb_001
 * @tc.desc: Test SetDoubleRingVolumeDb
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetDoubleRingVolumeDb_001, TestSize.Level1)
{
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_RING;
    int32_t volumeLevel = 5;
    audioAdapterManager_->useNonlinearAlgo_ = true;
    int32_t result = audioAdapterManager_->SetDoubleRingVolumeDb(streamType, volumeLevel);
    EXPECT_EQ(result, SUCCESS);

    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    streamType = STREAM_DEFAULT;
    audioAdapterManager_->useNonlinearAlgo_ = true;
    result = audioAdapterManager_->SetDoubleRingVolumeDb(streamType, volumeLevel);
    EXPECT_EQ(result, SUCCESS);

    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    streamType = STREAM_RING;
    audioAdapterManager_->useNonlinearAlgo_ = true;
    result = audioAdapterManager_->SetDoubleRingVolumeDb(streamType, volumeLevel);
    EXPECT_EQ(result, SUCCESS);

    audioAdapterManager_->useNonlinearAlgo_ = false;
    result = audioAdapterManager_->SetDoubleRingVolumeDb(streamType, volumeLevel);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ResetOffloadSessionId_001
 * @tc.desc: Test ResetOffloadSessionId
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, ResetOffloadSessionId_001, TestSize.Level1)
{
    audioAdapterManager_->offloadSessionID_ = 12345;
    audioAdapterManager_->ResetOffloadSessionId();
    EXPECT_FALSE(audioAdapterManager_->offloadSessionID_.has_value());

    audioAdapterManager_->offloadSessionID_.reset();
    audioAdapterManager_->ResetOffloadSessionId();
    EXPECT_FALSE(audioAdapterManager_->offloadSessionID_.has_value());
}
} // namespace AudioStandard
} // namespace OHOS