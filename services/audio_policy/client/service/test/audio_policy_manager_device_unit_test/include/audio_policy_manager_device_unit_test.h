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

#ifndef AUDIO_POLICY_MANAGER_DEVICE_UNIT_TEST_H
#define AUDIO_POLICY_MANAGER_DEVICE_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_policy_manager.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerDeviceChangeCallbackTest : public AudioManagerDeviceChangeCallback {
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override {}
};
class AudioPreferredInputDeviceChangeCallbackTest : public AudioPreferredInputDeviceChangeCallback {
public:
    virtual ~AudioPreferredInputDeviceChangeCallbackTest() = default;
    virtual void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) {}
};

class AudioPreferredOutputDeviceChangeCallbackTest : public AudioPreferredOutputDeviceChangeCallback {
public:
    virtual ~AudioPreferredOutputDeviceChangeCallbackTest() = default;
    virtual void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) {}
};

class ConcreteDeviceChangeWithInfoCallback : public DeviceChangeWithInfoCallback {
    void OnDeviceChangeWithInfo(const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReasonExt reason) override {}
    void OnRecreateStreamEvent(
        const uint32_t sessionId, const int32_t streamFlag, const AudioStreamDeviceChangeReasonExt reason) override {}
};

class AudioPolicyManagerDeviceUnitTest : public testing::Test {
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
#endif // AUDIO_POLICY_MANAGER_DEVICE_UNIT_TEST_H
