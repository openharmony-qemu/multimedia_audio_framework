/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_POLICY_TEST_H
#define AUDIO_POLICY_TEST_H

#include <audio_system_manager.h>
#include <gtest/gtest.h>
#include <audio_policy_log.h>

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
using namespace std;
using namespace testing;

struct PolicyParam {
    float volume;
    AudioStreamType streamType;
    AudioRingerMode ringerMode;
    ActiveDeviceType actDeviceType;
    DeviceType deviceType;
    DeviceFlag deviceFlag;
    DeviceRole deviceRole;
    AudioScene audioScene;
    bool active;
    bool mute;
    string key;
    string value;
};

class AudioRingerModeCallbackTest : public AudioRingerModeCallback {
public:
    AudioRingerMode ringerMode_;
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;
};

class AudioPolicyTest : public TestWithParam<PolicyParam> {
public:
    AudioPolicyTest() {}

    virtual ~AudioPolicyTest() {}

    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void) override;
    // TearDown: Called after each test cases
    void TearDown(void) override;
};
} // namespace V1_0
} // namespace AudioStandard
} // namespace OHOS
#endif  // AUDIO_POLICY_TEST_H
