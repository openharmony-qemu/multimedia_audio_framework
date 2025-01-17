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

#ifndef LOG_TAG
#define LOG_TAG "AudioEffectVolumeUnitTest"
#endif

#include "audio_effect_volume_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_volume.h"
#include "audio_errors.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
}

void AudioEffectVolumeUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetUpTestCase start ");
}
void AudioEffectVolumeUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: TearDownTestCase start ");
}
void AudioEffectVolumeUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: SetUp start ");
}
void AudioEffectVolumeUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: TearDown start ");
}

/**
* @tc.name   : Test GetSystemVolume API
* @tc.number : GetSystemVolume_001
* @tc.desc   : Test GetSystemVolume interface.
*/
HWTEST(AudioEffectVolumeUnitTest, GetSystemVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetSystemVolume_001 start ");
    const int32_t systemVolumeType = 1;
    const float systemVolume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetSystemVolume(systemVolumeType, systemVolume);
    float result = audioEffectVolume->GetSystemVolume(systemVolumeType);
    EXPECT_EQ(systemVolume, result);

    result = audioEffectVolume->GetSystemVolume(9999);
    EXPECT_EQ(systemVolume, result);
}

/**
* @tc.name   : Test GetStreamVolume API
* @tc.number : GetStreamVolume_001
* @tc.desc   : Test GetStreamVolume interface.
*/
HWTEST(AudioEffectVolumeUnitTest, GetStreamVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetSystemVolume_001 start ");
    const std::string sessionID = "123";
    const float streamVolume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
    float result = audioEffectVolume->GetStreamVolume(sessionID);
    EXPECT_EQ(streamVolume, result);

    result = audioEffectVolume->GetStreamVolume("");
    EXPECT_EQ(1.0, result);
}

/**
* @tc.name   : Test StreamVolumeDelete API
* @tc.number : StreamVolumeDelete_001
* @tc.desc   : Test StreamVolumeDelete interface.
*/
HWTEST(AudioEffectVolumeUnitTest, StreamVolumeDelete_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: StreamVolumeDelete_001 start ");
    const std::string sessionID = "123";
    const float streamVolume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
    float result = audioEffectVolume->GetStreamVolume(sessionID);
    EXPECT_EQ(streamVolume, result);

    int32_t result2 = audioEffectVolume->StreamVolumeDelete(sessionID);
    EXPECT_EQ(0, result2);

    result2 = audioEffectVolume->StreamVolumeDelete(sessionID);
    EXPECT_EQ(0, result2);
}

/**
* @tc.name   : Test GetDspVolume API
* @tc.number : GetDspVolume_001
* @tc.desc   : Test GetDspVolume interface.
*/
HWTEST(AudioEffectVolumeUnitTest, GetDspVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectVolumeUnitTest: GetDspVolume_001 start ");
    const std::string sceneType = "SCENE_MUSIC";
    const float volume = 0.5f;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    audioEffectVolume->SetDspVolume(volume);
    float result = audioEffectVolume->GetDspVolume();
    EXPECT_EQ(volume, result);
}
} // namespace AudioStandard
} // namespace OHOS