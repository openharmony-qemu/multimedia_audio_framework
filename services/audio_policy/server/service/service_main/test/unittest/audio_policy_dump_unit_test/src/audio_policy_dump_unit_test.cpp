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

#include "audio_policy_dump_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyDumpUnitTest::SetUpTestCase(void) {}
void AudioPolicyDumpUnitTest::TearDownTestCase(void) {}
void AudioPolicyDumpUnitTest::SetUp(void) {}
void AudioPolicyDumpUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_001
* @tc.desc  : Test AllDeviceVolumeInfoDump interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_001, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    std::string dumpString = "";
    audioPolicyDump.AllDeviceVolumeInfoDump(dumpString);
    EXPECT_NE(0, dumpString.size());
}

/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_002
* @tc.desc  : Test GetRingerModeDump interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_002, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    std::string dumpString = "";
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(RINGER_MODE_SILENT);
    audioPolicyDump.GetRingerModeDump(dumpString);
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(RINGER_MODE_VIBRATE);
    audioPolicyDump.GetRingerModeDump(dumpString);
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(static_cast<AudioRingerMode>(3));
    audioPolicyDump.GetRingerModeDump(dumpString);
    EXPECT_NE(0, dumpString.size());
}

/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_003
* @tc.desc  : Test GetRingerModeInfoDump interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_003, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    std::string dumpString = "";
    audioPolicyDump.GetRingerModeInfoDump(dumpString);
    EXPECT_NE(0, dumpString.size());
}


/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_004
* @tc.desc  : Test GetRingerModeType interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_004, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    AudioRingerMode ringerMode = RINGER_MODE_SILENT;
    auto ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
    ringerMode = RINGER_MODE_VIBRATE;
    ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
    ringerMode = RINGER_MODE_NORMAL;
    ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
    ringerMode = static_cast<AudioRingerMode>(3);
    ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
}
} // namespace AudioStandard
} // namespace OHOS