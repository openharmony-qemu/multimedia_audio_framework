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

#include "audio_zone_unit_test_base.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioZoneVolumeUnitTest : public AudioZoneUnitTestBase {
};

/**
* @tc.name  : Test AudioZoneVolume.
* @tc.number: AudioZoneVolume_001
* @tc.desc  : Test audio zone volume proxy.
*/
HWTEST_F(AudioZoneVolumeUnitTest, AudioZoneVolume_001, TestSize.Level1)
{
    EXPECT_NE(AudioZoneService::GetInstance().SetAudioZoneVolumeProxy(zoneId1_, STREAM_RING, 33, 0), 0);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableSystemVolumeProxy(TEST_PID_1000, zoneId1_, true));

    EXPECT_EQ(AudioZoneService::GetInstance().SetAudioZoneVolumeLevelForZone(zoneId1_, STREAM_RING, 33, 0), 0);
    client1000_->Wait();
    EXPECT_EQ(client1000_->volumeLevel_, 33);
    EXPECT_EQ(AudioZoneService::GetInstance().GetSystemVolumeLevelForZone(zoneId1_, STREAM_RING), 33);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableSystemVolumeProxy(TEST_PID_1000, zoneId1_, false), 0);
}
} // namespace AudioStandard
} // namespace OHOS