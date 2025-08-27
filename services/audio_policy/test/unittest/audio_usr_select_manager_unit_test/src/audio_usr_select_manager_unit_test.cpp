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

#include "audio_usr_select_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioUsrSelectManagerUnitTest::SetUpTestCase(void) {}
void AudioUsrSelectManagerUnitTest::TearDownTestCase(void) {}
void AudioUsrSelectManagerUnitTest::SetUp(void) {}
void AudioUsrSelectManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioUsrSelectManager.
* @tc.number: AudioUsrSelectManagerUnitTest_001
* @tc.desc  : Test SelectInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManagerUnitTest_001, TestSize.Level0)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t uid = 123;
    audioUsrSelectManager.SelectInputDeviceByUid(desc, uid);
    EXPECT_EQ(audioUsrSelectManager.audioUsrSelectMap_.size(), 1);
    EXPECT_EQ(audioUsrSelectManager.audioUsrSelectMap_[uid].size(), 1);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioUsrSelectManagerUnitTest_002
* @tc.desc  : Test GetSelectedInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManagerUnitTest_002, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t uid = 123;
    audioUsrSelectManager.SelectInputDeviceByUid(desc, uid);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor =
        audioUsrSelectManager.GetSelectedInputDeviceByUid(uid);
    EXPECT_NE(audioDeviceDescriptor, nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioUsrSelectManagerUnitTest_003
* @tc.desc  : Test ClearSelectedInputDeviceByUid interface.
*/
HWTEST_F(AudioUsrSelectManagerUnitTest, AudioUsrSelectManagerUnitTest_003, TestSize.Level1)
{
    AudioUsrSelectManager &audioUsrSelectManager = AudioUsrSelectManager::GetAudioUsrSelectManager();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t uid = 123;
    audioUsrSelectManager.SelectInputDeviceByUid(desc, uid);

    audioUsrSelectManager.ClearSelectedInputDeviceByUid(uid);
    EXPECT_EQ(audioUsrSelectManager.audioUsrSelectMap_.size(), 1);
    EXPECT_EQ(audioUsrSelectManager.audioUsrSelectMap_[uid].size(), 1);
}
} // namespace AudioStandard
} // namespace OHOS
