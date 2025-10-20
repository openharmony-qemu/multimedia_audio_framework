/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_POLICY_MANAGER_UNIT_TEST_H
#define AUDIO_POLICY_MANAGER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
class ConcreteAudioRingerModeCallback : public AudioRingerModeCallback {
public:
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override {}
};

class ConcreteAudioManagerMicrophoneBlockedCallback : public AudioManagerMicrophoneBlockedCallback {
public:
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override {}
};

class ConcreteAudioRendererStateChange : public AudioRendererStateChangeCallback {
public:
    void OnRendererStateChange(
        const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override {}
};

class ConcreteAudioCapturerStateChangeCallback : public AudioCapturerStateChangeCallback {
public:
    void OnCapturerStateChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override {}
};

class ConcreteAudioSessionCallback : public AudioSessionCallback {
public:
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override {}
};

class ConcreteHeadTrackingDataRequestedChangeCallback : public HeadTrackingDataRequestedChangeCallback {
public:
    void OnHeadTrackingDataRequestedChange(bool isRequested) override {}
};

class ConcreteAudioDeviceRefiner : public AudioDeviceRefiner {
public:
    virtual int32_t OnAudioOutputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const FetchDeviceInfo &fetchDeviceInfo)
        override { return 0; }
    virtual int32_t OnAudioDupDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const FetchDeviceInfo &fetchDeviceInfo) override
    {
        std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
        if (desc == nullptr) {
            return 0;
        }
        desc->deviceType_ = DEVICE_TYPE_SPEAKER;
        descs.push_back(std::move(desc));
        return 0;
    }
    virtual int32_t OnAudioInputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, SourceType sourceType, int32_t clientUid, AudioPipeType audioPipeType)
        override {return 0;}
    virtual int32_t GetSplitInfoRefined(std::string &splitInfo)
        override {return 0;}
    virtual int32_t OnDistributedOutputChange(bool isRemote)
        override {return 0;}
    virtual int32_t OnDistributedServiceOnline() override {return 0;}
};

class ConcreteAudioDeviceAnahs : public AudioDeviceAnahs {
public:
    int32_t OnExtPnpDeviceStatusChanged(std::string anahsStatus, std::string anahsShowType) override { return 0; }
};

class ConcreteAudioManagerAudioSceneChangedCallback : public AudioManagerAudioSceneChangedCallback {
public:
    void OnAudioSceneChange(const AudioScene audioScene) override {}
};

class AudioPolicyManagerUnitTest : public testing::Test {
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
#endif // AUDIO_POLICY_MANAGER_UNIT_TEST_H
