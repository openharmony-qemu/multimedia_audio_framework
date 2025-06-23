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

#include "audio_zone_class_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioZoneClassUnitTest::SetUpTestCase(void) {}
void AudioZoneClassUnitTest::TearDownTestCase(void) {}
void AudioZoneClassUnitTest::SetUp(void) {}
void AudioZoneClassUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioZoneUnitTest.
 * @tc.number: AudioZone_001
 * @tc.desc  : Test EnableAudioZoneReport interface.
 */
HWTEST_F(AudioZoneClassUnitTest, AudioZone_001, TestSize.Level1)
{
    std::shared_ptr<AudioZoneClientManager> manager =
        std::make_shared<AudioZoneClientManager>(DelayedSingleton<AudioPolicyServerHandler>::GetInstance());
    AudioZoneContext context;
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, "", context);
    EXPECT_EQ(zone->EnableChangeReport(0, true), SUCCESS);
    EXPECT_EQ(zone->EnableChangeReport(0, false), SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
 