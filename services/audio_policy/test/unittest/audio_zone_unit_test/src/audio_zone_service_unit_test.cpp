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

#include "audio_zone_service_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioZoneServiceUnitTest::SetUpTestCase(void) {}
void AudioZoneServiceUnitTest::TearDownTestCase(void) {}
void AudioZoneServiceUnitTest::SetUp(void)
{
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
}
void AudioZoneServiceUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_001
 * @tc.desc  : Test EnableAudioZoneReport interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_001, TestSize.Level1)
{
    EXPECT_EQ(AudioZoneService::GetInstance().EnableAudioZoneReport(0, true), SUCCESS);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableAudioZoneReport(0, false), SUCCESS);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_002
 * @tc.desc  : Test CheckIsZoneValid interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_002, TestSize.Level1)
{
    EXPECT_EQ(AudioZoneService::GetInstance().CheckIsZoneValid(-1), false);
    EXPECT_EQ(AudioZoneService::GetInstance().CheckIsZoneValid(1), false);
    EXPECT_EQ(AudioZoneService::GetInstance().CheckIsZoneValid(1), false);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_003
 * @tc.desc  : Test InjectInterruptToAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_003, TestSize.Level1)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    EXPECT_NE(AudioZoneService::GetInstance().InjectInterruptToAudioZone(0, "", interrupts), 0);
    EXPECT_NE(AudioZoneService::GetInstance().InjectInterruptToAudioZone(0, "0", interrupts), 0);
}
} // namespace AudioStandard
} // namespace OHOS
 