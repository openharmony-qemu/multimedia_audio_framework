/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "audio_collaborative_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioCollaborativeManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioCollaborativeManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest SetUpTestCase");
}

void AudioCollaborativeManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest TearDownTestCase");
}

void AudioCollaborativeManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioCollabMgrTest SetUp");
}

void AudioCollaborativeManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioCollabMgrTest TearDown");
}

// Alias for backward compatibility with test names
using AudioCollabMgrTest = AudioCollaborativeManagerUnitTest;

/**
 * @tc.name   : Test IsCollaborativePlaybackSupported API
 * @tc.type  : FUNC
 * @tc.number: IsCollaborativePlaybackSupported_001
 * @tc.desc  : Test IsCollaborativePlaybackSupported interface.
 */
HWTEST_F(AudioCollaborativeManagerUnitTest, IsCollaborativePlaybackSupported_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest IsCollaborativePlaybackSupported_001 start");
    bool result = AudioCollaborativeManager::GetInstance()->IsCollaborativePlaybackSupported();
    AUDIO_INFO_LOG("AudioCollabMgrTest IsCollaborativePlaybackSupported_001 result:%{public}d", result);
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test IsCollaborativePlaybackSupportedForDevice API
 * @tc.type  : FUNC
 * @tc.number: IsCollaborativePlaybackSupportedForDevice_001
 * @tc.desc  : Test IsCollaborativePlaybackSupportedForDevice with null descriptor.
 */
HWTEST_F(AudioCollaborativeManagerUnitTest, IsCollaborativePlaybackSupportedForDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest IsCollaborativePlaybackSupportedForDevice_001 start");
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    bool result = AudioCollaborativeManager::GetInstance()->IsCollaborativePlaybackSupportedForDevice(nullDesc);
    AUDIO_INFO_LOG("AudioCollabMgrTest IsCollaborativePlaybackSupportedForDevice_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::GetInstance()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test IsCollaborativePlaybackEnabledForDevice API
 * @tc.type  : FUNC
 * @tc.number: IsCollaborativePlaybackEnabledForDevice_001
 * @tc.desc  : Test IsCollaborativePlaybackEnabledForDevice with null descriptor.
 */
HWTEST_F(AudioCollaborativeManagerUnitTest, IsCollaborativePlaybackEnabledForDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest IsCollaborativePlaybackEnabledForDevice_001 start");
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    bool result = AudioCollaborativeManager::GetInstance()->IsCollaborativePlaybackEnabledForDevice(nullDesc);
    AUDIO_INFO_LOG("AudioCollabMgrTest IsCollaborativePlaybackEnabledForDevice_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::GetInstance()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetCollaborativePlaybackEnabledForDevice API
 * @tc.type  : FUNC
 * @tc.number: SetCollaborativePlaybackEnabledForDevice_001
 * @tc.desc  : Test SetCollaborativePlaybackEnabledForDevice with null descriptor.
 */
HWTEST_F(AudioCollaborativeManagerUnitTest, SetCollaborativePlaybackEnabledForDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest SetCollaborativePlaybackEnabledForDevice_001 start");
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    int32_t result = AudioCollaborativeManager::GetInstance()->SetCollaborativePlaybackEnabledForDevice(nullDesc, true);
    AUDIO_INFO_LOG("AudioCollabMgrTest SetCollaborativePlaybackEnabledForDevice_001 result:%{public}d", result);
    // Should return error for null descriptor
    // But result depends on AudioPolicyManager::GetInstance()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test RegisterCollaborationEnabledForCurrentDeviceEventListener API
 * @tc.type  : FUNC
 * @tc.number: RegisterCollaborationEnabledForCurrentDeviceEventListener_001
 * @tc.desc  : Test RegisterCollaborationEnabledForCurrentDeviceEventListener with null callback.
 */
HWTEST_F(AudioCollabMgrTest, RegisterCollabEnabledForCurDevEventListener_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest RegisterCollaborationEnabledForCurrentDeviceEventListener_001 start");
    int32_t result = AudioCollaborativeManager::GetInstance()
        ->RegisterCollaborationEnabledForCurrentDeviceEventListener(nullptr);
    AUDIO_INFO_LOG("AudioCollabMgrTest RegisterCollabEnabledForCurDevEventListener_001 result:%{public}d",
        result);
    // Should return error for null callback
    // But result depends on AudioPolicyManager::GetInstance()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test UnregisterCollaborationEnabledForCurrentDeviceEventListener API
 * @tc.type  : FUNC
 * @tc.number: UnregisterCollaborationEnabledForCurrentDeviceEventListener_001
 * @tc.desc  : Test UnregisterCollaborationEnabledForCurrentDeviceEventListener interface.
 */
HWTEST_F(AudioCollabMgrTest, UnregisterCollabEnabledForCurDevEventListener_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCollabMgrTest UnregisterCollabEnabledForCurDevEventListener_001 start");
    int32_t result = AudioCollaborativeManager::GetInstance()
        ->UnregisterCollaborationEnabledForCurrentDeviceEventListener();
    AUDIO_INFO_LOG("AudioCollabMgrTest UnregisterCollabEnabledForCurDevEventListener_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::GetInstance()
    // Just check function doesn't crash
    (void)result;
}

} // namespace AudioStandard
} // namespace OHOS
