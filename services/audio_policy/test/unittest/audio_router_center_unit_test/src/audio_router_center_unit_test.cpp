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

#include "../include/audio_router_center_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioRouterCenterUnitTest::SetUpTestCase(void) {}
void AudioRouterCenterUnitTest::TearDownTestCase(void) {}
void AudioRouterCenterUnitTest::SetUp(void) {}
void AudioRouterCenterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: AudioRouterCenter_001
 * @tc.desc  : Test NeedSkipSelectAudioOutputDeviceRefined interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_001, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_INVALID, descs));

    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(RINGER_MODE_VIBRATE);
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_INVALID, descs));
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_ALARM, descs));

    descs.push_back(std::move(desc));
    descs.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_ALARM, descs));

    descs.front()->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_TRUE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_ALARM, descs));
}
} // namespace AudioStandard
} // namespace OHOS
 