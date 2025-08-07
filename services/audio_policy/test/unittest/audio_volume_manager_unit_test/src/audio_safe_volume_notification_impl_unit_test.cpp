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

#include "audio_volume_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioSafeNotificationImpl_001
* @tc.desc  : Test CheckWiredActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioSafeNotificationImpl_001, TestSize.Level3)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 10000;
    audioVolumeManager.activeSafeTime_ = 100;
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioSafeNotificationImpl_002
* @tc.desc  : Test CheckWiredActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioSafeNotificationImpl_002, TestSize.Level3)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 10;
    audioVolumeManager.activeSafeTime_ = 10000;
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioSafeNotificationImpl_003
* @tc.desc  : Test CheckBlueToothActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioSafeNotificationImpl_003, TestSize.Level3)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 10000;
    audioVolumeManager.activeSafeTime_ = 1000;
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioSafeNotificationImpl_004
* @tc.desc  : Test CheckBlueToothActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioSafeNotificationImpl_004, TestSize.Level3)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 10;
    audioVolumeManager.activeSafeTime_ = 10000;
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
}
} // namespace AudioStandard
} // namespace OHOS