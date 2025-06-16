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

#include "oh_audio_workgroup_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioWorkgroupUnitTest::SetUpTestCase(void) { }

void OHAudioWorkgroupUnitTest::TearDownTestCase(void) { }

void OHAudioWorkgroupUnitTest::SetUp(void) { }

void OHAudioWorkgroupUnitTest::TearDown(void) { }

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test OHAudioWorkgroup.
 * @tc.number: TestOHAudioWorkgroup_001
 * @tc.desc  : Test OHAudioWorkgroup.
 */

HWTEST(OHAudioWorkgroupUnitTest, TestOHAudioWorkgroup_001, TestSize.Level0)
{
    OH_AudioResourceManager *audioResourceManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioResourceManager, nullptr);
    int32_t test_tid = gettid();
    OH_AudioWorkgroup *audioWorkgroup = nullptr;
    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, "testAudioGroup", &audioWorkgroup);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioWorkgroup, nullptr);

    result = OH_AudioWorkgroup_AddCurrentThread(audioWorkgroup, &test_tid);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioWorkgroup_Start(audioWorkgroup, 20, 40);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioWorkgroup_Stop(audioWorkgroup);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioWorkgroup_RemoveThread(audioWorkgroup, test_tid);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioResourceManager_ReleaseWorkgroup(audioResourceManager, audioWorkgroup);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}
#endif

} // namespace AudioStandard
} // namespace OHOS