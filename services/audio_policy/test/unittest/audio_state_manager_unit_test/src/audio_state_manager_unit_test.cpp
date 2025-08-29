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

#include "audio_state_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioStateManagerUnitTest::SetUpTestCase(void) {}
void AudioStateManagerUnitTest::TearDownTestCase(void) {}
void AudioStateManagerUnitTest::SetUp(void) {}
void AudioStateManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_001
* @tc.desc  : Test SetPreferredMediaRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_001, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredMediaRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_002
* @tc.desc  : Test SetAndGetPreferredCallRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_002, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    shared_ptr<AudioDeviceDescriptor> speaker = std::make_shared<AudioDeviceDescriptor>();
    speaker->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 0);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, -1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, -1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 789);
    shared_ptr<AudioDeviceDescriptor> deviceDesc =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, nullptr);
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(789);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, nullptr);
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(790);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, nullptr);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 790);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, speaker);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_003
* @tc.desc  : Test SetPreferredCallCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_003, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredCallCaptureDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredCallCaptureDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_004
* @tc.desc  : Test SetPreferredCallCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_004, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRingRenderDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_005
* @tc.desc  : Test SetPreferredRecordCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_005, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRecordCaptureDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_006
* @tc.desc  : Test SetPreferredToneRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_006, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredToneRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredToneRenderDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_007
* @tc.desc  : Test UpdatePreferredMediaRenderDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_007, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredMediaRenderDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_EQ(AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_008
* @tc.desc  : Test UpdatePreferredCallRenderDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_008, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredCallRenderDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_EQ(AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_009
* @tc.desc  : Test UpdatePreferredCallCaptureDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_009, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredCallCaptureDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_EQ(AudioStateManager::GetAudioStateManager().GetPreferredCallCaptureDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_010
* @tc.desc  : Test UpdatePreferredRecordCaptureDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_010, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredRecordCaptureDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_EQ(AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice()->connectState_,
        ConnectState::CONNECTED);
}
} // namespace AudioStandard
} // namespace OHOS
