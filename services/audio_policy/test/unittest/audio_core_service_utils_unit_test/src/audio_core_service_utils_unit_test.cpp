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

#include "audio_core_service_utils_unit_test.h"
#include "audio_scene_manager.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test AudioCoreServiceUtils.
 * @tc.number: AudioCoreServiceUtils_001
 * @tc.desc  : Test AudioCoreServiceUtils::IsDualStreamWhenRingDual()
 */
HWTEST(AudioCoreServiceUtilsTest, AudioCoreServiceUtils_001, TestSize.Level1)
{
    EXPECT_TRUE(AudioCoreServiceUtils::IsDualStreamWhenRingDual(STREAM_RING));
    EXPECT_TRUE(AudioCoreServiceUtils::IsDualStreamWhenRingDual(STREAM_ALARM));
    EXPECT_TRUE(AudioCoreServiceUtils::IsDualStreamWhenRingDual(STREAM_ACCESSIBILITY));
    EXPECT_FALSE(AudioCoreServiceUtils::IsDualStreamWhenRingDual(STREAM_MUSIC));
}

/**
 * @tc.name  : Test AudioCoreServiceUtils.
 * @tc.number: AudioCoreServiceUtils_002
 * @tc.desc  : Test AudioCoreServiceUtils::IsOverRunPlayback()
 */
HWTEST(AudioCoreServiceUtilsTest, AudioCoreServiceUtils_002, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_RECORD;

    EXPECT_FALSE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_STOPPED));
    mode = AUDIO_MODE_PLAYBACK;
    EXPECT_TRUE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_STOPPED));
    EXPECT_TRUE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_RELEASED));
    EXPECT_TRUE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_PAUSED));
    EXPECT_FALSE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_PREPARED));

    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_RINGING, 0, 0);
    EXPECT_FALSE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_PAUSED));
    EXPECT_FALSE(AudioCoreServiceUtils::IsOverRunPlayback(mode, RENDERER_PREPARED));
}

/**
 * @tc.name  : Test AudioCoreServiceUtils.
 * @tc.number: AudioCoreServiceUtils_003
 * @tc.desc  : Test AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker()
 */
HWTEST(AudioCoreServiceUtilsTest, AudioCoreServiceUtils_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>();
    desc1->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc1->networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>();
    desc2->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc2->networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> desc3 = std::make_shared<AudioDeviceDescriptor>();
    desc3->deviceType_ = DEVICE_TYPE_DP;
    desc3->networkId_ = LOCAL_NETWORK_ID;

    EXPECT_FALSE(AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(descs, 0));

    descs.push_back(desc3);
    descs.push_back(desc1);
    EXPECT_FALSE(AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(descs, 0));

    descs.clear();
    descs.push_back(desc1);
    descs.push_back(desc3);
    EXPECT_FALSE(AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(descs, 0));

    descs.clear();
    descs.push_back(desc1);
    descs.push_back(desc2);
    EXPECT_FALSE(AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(descs, 0));

    descs.clear();
    descs.push_back(desc2);
    descs.push_back(desc1);
    EXPECT_TRUE(AudioCoreServiceUtils::IsRingDualToneOnPrimarySpeaker(descs, 0));
}

/**
 * @tc.name  : Test AudioCoreServiceUtils.
 * @tc.number: AudioCoreServiceUtils_004
 * @tc.desc  : Test AudioCoreServiceUtils::NeedDualHalToneInStatus()
 */
HWTEST(AudioCoreServiceUtilsTest, AudioCoreServiceUtils_004, TestSize.Level1)
{
    EXPECT_TRUE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_NORMAL, STREAM_USAGE_ALARM, false, false));
    EXPECT_TRUE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_NORMAL, STREAM_USAGE_ALARM, false, true));
    EXPECT_TRUE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_NORMAL, STREAM_USAGE_ALARM, true, false));
    EXPECT_FALSE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_NORMAL, STREAM_USAGE_ALARM, true, true));

    EXPECT_TRUE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_NORMAL, STREAM_USAGE_MUSIC, false, false));
    EXPECT_TRUE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_SILENT, STREAM_USAGE_ALARM, false, false));
    EXPECT_FALSE(AudioCoreServiceUtils::NeedDualHalToneInStatus(RINGER_MODE_SILENT, STREAM_USAGE_MUSIC, false, false));
}

/**
 * @tc.name  : Test AudioCoreServiceUtils.
 * @tc.number: AudioCoreServiceUtils_005
 * @tc.desc  : Test AudioCoreServiceUtils::IsAlarmOnActive()
 */
HWTEST(AudioCoreServiceUtilsTest, AudioCoreServiceUtils_005, TestSize.Level1)
{
    EXPECT_TRUE(AudioCoreServiceUtils::IsAlarmOnActive(STREAM_USAGE_ALARM, true));
    EXPECT_FALSE(AudioCoreServiceUtils::IsAlarmOnActive(STREAM_USAGE_MUSIC, true));
}
} // namespace AudioStandard
} // namespace OHOS
