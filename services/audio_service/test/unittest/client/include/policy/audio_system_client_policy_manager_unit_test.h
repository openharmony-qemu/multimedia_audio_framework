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

#ifndef AUDIO_SYSTEM_CLIENT_POLICY_MANAGER_UNIT_TEST_H
#define AUDIO_SYSTEM_CLIENT_POLICY_MANAGER_UNIT_TEST_H

#include <gtest/gtest.h>

#include "audio_system_client_policy_manager.h"
#include "audio_policy_manager.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

// Mock class definitions
class MockAudioDistributedRoutingRoleCallback : public AudioDistributedRoutingRoleCallback {
public:
    void OnDistributedRoutingRoleChange(std::shared_ptr<AudioDeviceDescriptor> descriptor,
        const CastType type) override
    {
    }
};

class MockAudioManagerMicrophoneBlockedCallback : public AudioManagerMicrophoneBlockedCallback {
public:
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override
    {
    }
};

class MockAudioManagerAudioSceneChangedCallback : public AudioManagerAudioSceneChangedCallback {
public:
    void OnAudioSceneChange(const AudioScene audioScene) override
    {
    }
};

class MockAudioQueryClientTypeCallback : public AudioQueryClientTypeCallback {
public:
    bool OnQueryClientType(const std::string &bundleName, uint32_t uid) override
    {
        return false;
    }
};

class MockAudioClientInfoMgrCallback : public AudioClientInfoMgrCallback {
public:
    bool OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid) override
    {
        return false;
    }
    bool OnCheckMediaControllerBundle(const std::string &bundleName) override
    {
        return false;
    }
    bool OnQueryIsForceGetZoneDevice(const std::string &bundleName) override
    {
        return false;
    }
    bool OnQueryIsForceGetDevByVolumeType(const std::string &bundleName) override
    {
        return false;
    }
    bool OnQueryUseSystemPreferredDeviceForVolume(const std::string &bundleName) override
    {
        return false;
    }
};

class MockAudioVKBInfoMgrCallback : public AudioVKBInfoMgrCallback {
public:
    bool OnCheckVKBInfo(const std::string &bundleName) override
    {
        return false;
    }
};

class MockAudioQueryBundleNameListCallback : public AudioQueryBundleNameListCallback {
public:
    bool OnQueryBundleNameIsInList(const std::string &bundleName, const std::string &listType) override
    {
        return false;
    }
    void StartVoiceBroadcast(uint32_t sessionId, const std::string &cellularRecordPhoneNum) override {}
};

class MockAudioRingerModeCallback : public AudioRingerModeCallback {
public:
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override
    {
    }
};

class MockAudioQueryAllowedPlaybackCallback : public AudioQueryAllowedPlaybackCallback {
public:
    bool OnQueryAllowedPlayback(int32_t uid, int32_t pid) override
    {
        return false;
    }
};

class MockAudioBackgroundMuteCallback : public AudioBackgroundMuteCallback {
public:
    void OnBackgroundMute(const int32_t uid) override
    {
    }
};

class MockAudioManagerAvailableDeviceChangeCallback : public AudioManagerAvailableDeviceChangeCallback {
public:
    void OnAvailableDeviceChange(const AudioDeviceUsage usage,
        const DeviceChangeAction &deviceChangeAction) override
    {
    }
};

class AudioSystemClientPolicyManagerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SYSTEM_CLIENT_POLICY_MANAGER_UNIT_TEST_H
